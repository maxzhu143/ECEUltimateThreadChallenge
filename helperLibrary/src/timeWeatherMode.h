#ifndef TIME_WEATHER_MODE_H
#define TIME_WEATHER_MODE_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <math.h>
class CheapYellowDisplay;

#ifndef ATLANTA_TZ
#define ATLANTA_TZ "EST5EDT,M3.2.0/2,M11.1.0/2"
#endif

#ifndef ATLANTA_LAT
#define ATLANTA_LAT 33.75f
#endif

#ifndef ATLANTA_LON
#define ATLANTA_LON -84.39f
#endif

enum ScreenMode
{
  SCREEN_MODE_SPOTIFY = 0,
  SCREEN_MODE_CLOCK_WEATHER = 1
};

ScreenMode currentScreenMode = SCREEN_MODE_SPOTIFY;
String currentTimeString = "--:--";
String currentWeatherString = "--";
unsigned long nextClockWeatherDraw = 0;
unsigned long nextWeatherRefreshTime = 0;

const unsigned long CLOCK_DRAW_INTERVAL_MS = 1000;
const unsigned long WEATHER_REFRESH_INTERVAL_MS = 10UL * 60UL * 1000UL;

// New settings
bool useFahrenheit = false;
bool use24HourClock = false;

String weatherCodeToText(int code)
{
  switch (code)
  {
  case 0:
    return "Clear";
  case 1:
  case 2:
  case 3:
    return "Cloudy";
  case 45:
  case 48:
    return "Fog";
  case 51:
  case 53:
  case 55:
    return "Drizzle";
  case 56:
  case 57:
    return "Frz Drzl";
  case 61:
  case 63:
  case 65:
    return "Rain";
  case 66:
  case 67:
    return "Frz Rain";
  case 71:
  case 73:
  case 75:
    return "Snow";
  case 77:
    return "Snow";
  case 80:
  case 81:
  case 82:
    return "Showers";
  case 85:
  case 86:
    return "Snow";
  case 95:
    return "Storm";
  case 96:
  case 99:
    return "T-Storm";
  default:
    return "Weather";
  }
}

void setupAtlantaTime()
{
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  setenv("TZ", ATLANTA_TZ, 1);
  tzset();
}

void updateCurrentTimeString()
{
  time_t now;
  struct tm timeinfo;
  char buf[20];

  time(&now);
  localtime_r(&now, &timeinfo);

  if (timeinfo.tm_year < (2024 - 1900))
  {
    currentTimeString = "Syncing...";
    return;
  }

  if (use24HourClock)
  {
    strftime(buf, sizeof(buf), "%H:%M", &timeinfo);
    currentTimeString = String(buf);
  }
  else
  {
    strftime(buf, sizeof(buf), "%I:%M %p", &timeinfo);
    if (buf[0] == '0')
    {
      currentTimeString = String(buf + 1);
    }
    else
    {
      currentTimeString = String(buf);
    }
  }
}

void updateAtlantaWeather()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    currentWeatherString = "No WiFi";
    return;
  }

  WiFiClientSecure weatherClient;
  weatherClient.setInsecure();

  HTTPClient http;
  String url = String("https://api.open-meteo.com/v1/forecast?latitude=") +
               String(ATLANTA_LAT, 4) +
               "&longitude=" + String(ATLANTA_LON, 4) +
               "&current=temperature_2m,weather_code&temperature_unit=celsius";

  DG2("Fetching Atlanta weather...", url);

  if (!http.begin(weatherClient, url))
  {
    currentWeatherString = "Wx Err";
    return;
  }

  int httpCode = http.GET();
  if (httpCode != 200)
  {
    DG2("Weather HTTP error: ", httpCode);
    currentWeatherString = "Wx Err";
    http.end();
    return;
  }

  DynamicJsonDocument doc(4096);
  String payload = http.getString();
  http.end();
  DeserializationError err = deserializeJson(doc, payload);

  if (err)
  {
    DG2("Weather JSON parse failed: ", err.c_str());
    currentWeatherString = "Wx Err";
    return;
  }

  float tempC = doc["current"]["temperature_2m"] | NAN;
  int weatherCode = doc["current"]["weather_code"] | -1;

  if (isnan(tempC) || weatherCode < 0)
  {
    currentWeatherString = "Wx Err";
    return;
  }

  float displayTemp = tempC;
  char unitChar = 'C';

  if (useFahrenheit)
  {
    displayTemp = (tempC * 9.0f / 5.0f) + 32.0f;
    unitChar = 'F';
  }

  int roundedTemp = (int)round(displayTemp);
  currentWeatherString = String(roundedTemp) + "\xB0" + String(unitChar) + "  " + weatherCodeToText(weatherCode);
  DG2("Atlanta weather: ", currentWeatherString);
}

void toggleTemperatureUnit()
{
  useFahrenheit = !useFahrenheit;
  updateAtlantaWeather();
}

void toggleClockFormat()
{
  use24HourClock = !use24HourClock;
  updateCurrentTimeString();
}

void initTimeWeatherMode()
{
  setupAtlantaTime();
  updateCurrentTimeString();
  updateAtlantaWeather();
  nextClockWeatherDraw = 0;
  nextWeatherRefreshTime = millis() + WEATHER_REFRESH_INTERVAL_MS;
}

void toggleScreenMode(CheapYellowDisplay &display)
{
  if (currentScreenMode == SCREEN_MODE_SPOTIFY)
  {
    currentScreenMode = SCREEN_MODE_CLOCK_WEATHER;
    DG("Switched to CLOCK/WEATHER mode");
    updateCurrentTimeString();
    display.drawClockWeatherScreen(currentTimeString.c_str(), currentWeatherString.c_str(), "Atlanta, GA");
  }
  else
  {
    currentScreenMode = SCREEN_MODE_SPOTIFY;
    DG("Switched to SPOTIFY mode");
    display.showDefaultScreen();
  }
}

void drawClockWeatherScreen(CheapYellowDisplay &display)
{
  if (millis() < nextClockWeatherDraw)
    return;

  nextClockWeatherDraw = millis() + CLOCK_DRAW_INTERVAL_MS;
  updateCurrentTimeString();

  if (millis() > nextWeatherRefreshTime || currentWeatherString == "--")
  {
    updateAtlantaWeather();
    nextWeatherRefreshTime = millis() + WEATHER_REFRESH_INTERVAL_MS;
  }

  display.drawClockWeatherScreen(currentTimeString.c_str(), currentWeatherString.c_str(), "Atlanta, GA");
}

#endif