#define DEBUG_PRINT 0

#define DG(x) do { if (DEBUG_PRINT) Serial.println(x); } while(0)
#define DG2(x, y) do { if (DEBUG_PRINT) { Serial.print(x); Serial.println(y); } } while(0)

#define YELLOW_DISPLAY
#define ESP_DRD_USE_SPIFFS true

// Standard Libraries
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <FS.h>
#include "SPIFFS.h"

WiFiClientSecure client;
#define SPOTIFY_MARKET "CA"

// Additional Libraries
#include <WiFiManager.h>
#include <ESP_DoubleResetDetector.h>
#include <SpotifyArduino.h>
#include <SpotifyArduinoCert.h>
#include <ArduinoJson.h>
#include "WifiManagerHandler.h"
#include "refreshToken.h"
#include "spotifyDisplay.h"
#include "spotifyLogic.h"
#include "configFile.h"
//can add this for debugging or prior to working screen
//#include "serialPrint.h"
#include "cheapYellowLCD.h"
#include "timeWeatherMode.h"

// GPIO assignments
#define PLAY_PAUSE_BUTTON_PIN 22
#define RESET_CONFIG_BUTTON_PIN 27
#define VOLUME_POT_PIN 34

const unsigned long PLAY_PAUSE_DEBOUNCE_MS = 50;
const unsigned long RESET_SHORT_PRESS_MIN_MS = 50;
const unsigned long RESET_HOLD_MS = 5000;
const unsigned long VOLUME_READ_INTERVAL_MS = 150;
const int VOLUME_STEP_THRESHOLD = 2;

bool lastPlayPauseReading = HIGH;
bool stablePlayPauseState = HIGH;
unsigned long lastPlayPauseDebounceTime = 0;

int lastSentVolume = -1;
unsigned long nextVolumeReadTime = 0;

unsigned long resetHoldStart = 0;
bool resetHoldActive = false;
bool resetTriggered = false;

const unsigned long MULTI_PRESS_WINDOW_MS = 500;
unsigned long lastShortPressTime = 0;
int shortPressCount = 0;

CheapYellowDisplay cyd;
SpotifyDisplay *spotifyDisplay = &cyd;

void drawWifiManagerMessage(WiFiManager *myWiFiManager)
{
  spotifyDisplay->drawWifiManagerMessage(myWiFiManager);
}

void eraseAllWifiAndSpotifyInfo()
{
  WiFiManager wm;
  wm.resetSettings();
  WiFi.disconnect(true, true);

  if (SPIFFS.exists("/spotify_diy_config.json"))
  {
    if (SPIFFS.remove("/spotify_diy_config.json"))
    {
      Serial.println("Deleted /spotify_diy_config.json");
    }
    else
    {
      Serial.println("Failed to delete /spotify_diy_config.json");
    }
  }
  else
  {
    Serial.println("Config file not found");
  }

  delay(500);
  ESP.restart();
}

void handlePlayPauseButton()
{
  bool reading = digitalRead(PLAY_PAUSE_BUTTON_PIN);

  if (reading != lastPlayPauseReading)
  {
    lastPlayPauseDebounceTime = millis();
  }

  if ((millis() - lastPlayPauseDebounceTime) > PLAY_PAUSE_DEBOUNCE_MS)
  {
    if (reading != stablePlayPauseState)
    {
      stablePlayPauseState = reading;
    if (stablePlayPauseState == LOW)
    {
      DG("play/pause button pressed");
      updateCurrentlyPlaying(true);

      bool success = togglePlayback();
      DG2("togglePlayback result: ", success ? "success" : "failed");
    }
    }
  }

  lastPlayPauseReading = reading;
}

void handleVolumePot()
{
  if (millis() < nextVolumeReadTime)
    return;

  nextVolumeReadTime = millis() + VOLUME_READ_INTERVAL_MS;

  long total = 0;
  for (int i = 0; i < 5; i++)
  {
    total += analogRead(VOLUME_POT_PIN);
  }
  int raw = total / 5;
  int volume = map(raw, 0, 4095, 0, 100);

  if (volume < 0) volume = 0;
  if (volume > 100) volume = 100;

  if (lastSentVolume < 0 || abs(volume - lastSentVolume) >= VOLUME_STEP_THRESHOLD)
  {
    Serial.print("Volume: ");
    Serial.println(volume);

    if (spotify.setVolume(volume))
    {
      lastSentVolume = volume;
      forceSpotifyRefreshSoon();
    }
    else
    {
      Serial.println("Failed to set volume");
    }
  }
}

void handleResetConfigButton()
{
  bool pressed = (digitalRead(RESET_CONFIG_BUTTON_PIN) == LOW);

  if (pressed)
  {
    if (!resetHoldActive)
    {
      resetHoldActive = true;
      resetTriggered = false;
      resetHoldStart = millis();
      Serial.println("Reset/mode button hold started");
    }
    else if (!resetTriggered && (millis() - resetHoldStart >= RESET_HOLD_MS))
    {
      resetTriggered = true;
      Serial.println("Reset button held for 5 seconds");
      eraseAllWifiAndSpotifyInfo();
    }
  }
  else
  {
    if (resetHoldActive && !resetTriggered)
    {
      unsigned long heldTime = millis() - resetHoldStart;

      if (heldTime >= RESET_SHORT_PRESS_MIN_MS && heldTime < RESET_HOLD_MS)
      {
        shortPressCount++;
        lastShortPressTime = millis();
      }
    }

    resetHoldActive = false;
    resetTriggered = false;
  }

  if (shortPressCount > 0 && (millis() - lastShortPressTime > MULTI_PRESS_WINDOW_MS))
  {
    handleCompletedShortPressAction();
  }
}

void handleCompletedShortPressAction()
{
  if (shortPressCount == 1)
  {
    Serial.println("Single short press: toggling screen mode");
    toggleScreenMode(cyd);

    if (currentScreenMode == SCREEN_MODE_SPOTIFY)
    {
      forceSpotifyRefreshSoon();
      updateCurrentlyPlaying(true);
    }
  }
  else if (shortPressCount == 2)
  {
    Serial.println("Double short press: toggling C/F");
    toggleTemperatureUnit();

    if (currentScreenMode == SCREEN_MODE_CLOCK_WEATHER)
    {
      drawClockWeatherScreen(cyd);
    }
  }
  else if (shortPressCount >= 3)
  {
    Serial.println("Triple short press: toggling 12/24 hour");
    toggleClockFormat();

    if (currentScreenMode == SCREEN_MODE_CLOCK_WEATHER)
    {
      drawClockWeatherScreen(cyd);
    }
  }

  shortPressCount = 0;
}

void setup()
{
  Serial.begin(115200);

  pinMode(PLAY_PAUSE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(RESET_CONFIG_BUTTON_PIN, INPUT_PULLUP);
  analogReadResolution(12);

  bool forceConfig = false;

  drd = new DoubleResetDetector(DRD_TIMEOUT, DRD_ADDRESS);
  if (drd->detectDoubleReset())
  {
    Serial.println(F("Forcing config mode as there was a Double reset detected"));
    forceConfig = true;
  }

  spotifyDisplay->displaySetup(&spotify);

  bool spiffsInitSuccess = SPIFFS.begin(false) || SPIFFS.begin(true);
  if (!spiffsInitSuccess)
  {
    Serial.println("SPIFFS initialisation failed!");
    while (1)
      yield();
  }
  Serial.println("\r\nInitialisation done.");

  refreshToken[0] = '\0';
  if (!fetchConfigFile(refreshToken, clientId, clientSecret))
  {
    forceConfig = true;
  }

  setupWiFiManager(forceConfig, refreshToken, &saveConfigFile, &drawWifiManagerMessage);

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  spotifySetup(spotifyDisplay, clientId, clientSecret);
  initTimeWeatherMode();

  pinMode(0, INPUT);
  bool forceRefreshToken = digitalRead(0) == LOW;
  if (forceRefreshToken)
  {
    Serial.println("GPIO 0 is low, forcing refreshToken");
  }

  if (forceRefreshToken || refreshToken[0] == '\0')
  {
    spotifyDisplay->drawRefreshTokenMessage();
    Serial.println("Launching refresh token flow");
    if (launchRefreshTokenFlow(&spotify, clientId, refreshToken))
    {
      Serial.print("Refresh Token: ");
      Serial.println(refreshToken);
      saveConfigFile(refreshToken, clientId, clientSecret);
    }
  }

  spotifyRefreshToken(refreshToken);
  spotifyDisplay->showDefaultScreen();
  updateCurrentlyPlaying(true);
}

void loop()
{
  drd->loop();

  handlePlayPauseButton();
  //handleVolumePot();
  handleResetConfigButton();

  if (currentScreenMode == SCREEN_MODE_SPOTIFY)
  {
    spotifyDisplay->checkForInput();
    updateCurrentlyPlaying(false);
    updateProgressBar();
  }
  else
  {
    drawClockWeatherScreen(cyd);
  }
}
