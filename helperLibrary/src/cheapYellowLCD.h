#include "spotifyDisplay.h"

#include "touchScreen.h"

#include <TFT_eSPI.h>
// A library for checking if the reset button has been pressed twice
// Can be used to enable config mode
// Can be installed from the library manager (Search for "ESP_DoubleResetDetector")
// https://github.com/khoih-prog/ESP_DoubleResetDetector

#include <JPEGDEC.h>
// Library for decoding Jpegs from the API responses
//
// Can be installed from the library manager (Search for "JPEGDEC")
// https://github.com/bitbank2/JPEGDEC

// -------------------------------
// Putting this stuff outside the class because
// I can't easily pass member functions in as callbacks for jpegdec

// -------------------------------

TFT_eSPI tft = TFT_eSPI();
JPEGDEC jpeg;

const char *ALBUM_ART = "/album.jpg";

// This next function will be called during decoding of the jpeg file to
// render each block to the Matrix.  If you use a different display
// you will need to adapt this function to suit.
int JPEGDraw(JPEGDRAW *pDraw)
{
  // Stop further decoding as image is running off bottom of screen
  if (pDraw->y >= tft.height())
    return 0;

  tft.pushImage(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, pDraw->pPixels);
  return 1;
}

fs::File myfile;

void *myOpen(const char *filename, int32_t *size)
{
  myfile = SPIFFS.open(filename);
  *size = myfile.size();
  return &myfile;
}
void myClose(void *handle)
{
  if (myfile)
    myfile.close();
}
int32_t myRead(JPEGFILE *handle, uint8_t *buffer, int32_t length)
{
  if (!myfile)
    return 0;
  return myfile.read(buffer, length);
}
int32_t mySeek(JPEGFILE *handle, int32_t position)
{
  if (!myfile)
    return 0;
  return myfile.seek(position);
}

class CheapYellowDisplay : public SpotifyDisplay
{
public:
  void displaySetup(SpotifyArduino *spotifyObj)
  {

    spotify_display = spotifyObj;

    touchSetup(spotifyObj);

    DG("cyd display setup");
    setWidth(320);
    setHeight(240);

    setImageHeight(150);
    setImageWidth(150);

    // Start the tft display and set it to black
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
  }

  void showDefaultScreen()
  {
    tft.fillScreen(TFT_BLACK);

    drawTouchButtons(false, false);
  }

  void displayTrackProgress(long progress, long duration)
  {

    //  Serial.print("Elapsed time of song (ms): ");
    //  Serial.print(progress);
    //  Serial.print(" of ");
    //  Serial.println(duration);
    //  Serial.println();

    float percentage = ((float)progress / (float)duration) * 100;
    int clampedPercentage = (int)percentage;
    // Serial.println(clampedPercentage);
    int barXWidth = map(clampedPercentage, 0, 100, 0, screenWidth - 40);
    // Serial.println(barXWidth);

    int progressStartY = 150 + 5;

    // Draw outer Rectangle, in theory we only need to do this once!
    tft.drawRect(19, progressStartY, screenWidth - 38, 20, TFT_WHITE);

    // Draw the white portion of the filled bar
    tft.fillRect(20, progressStartY + 1, barXWidth, 18, TFT_WHITE);

    // Fill whats left black
    tft.fillRect(20 + barXWidth, progressStartY + 1, (screenWidth - 20) - (20 + barXWidth), 18, TFT_BLACK);
  }

  void printCurrentlyPlayingToScreen(CurrentlyPlaying currentlyPlaying)
  {
    // Clear the text
    int textStartY = 150 + 30;
    tft.fillRect(0, textStartY, screenWidth, screenHeight - textStartY, TFT_BLACK);
      
    tft.setTextSize(1);
    tft.setTextFont(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.drawCentreString(currentlyPlaying.trackName, screenCenterX, textStartY, 2);
    tft.drawCentreString(currentlyPlaying.artists[0].artistName, screenCenterX, textStartY + 18, 2);
    tft.drawCentreString(currentlyPlaying.albumName, screenCenterX, textStartY + 36, 2);
  }

  void checkForInput()
  {
    if (millis() > touchScreenCoolDownTime && handleTouched())
    {
      drawTouchButtons(previousTrackStatus, nextTrackStatus);
      if (previousTrackStatus)
      {
        spotify_display->previousTrack();
      }
      else if (nextTrackStatus)
      {
        spotify_display->nextTrack();
      }
      drawTouchButtons(false, false);
      requestDueTime = 0;                                               // Some button has been pressed and acted on, it surely impacts the status so force a refresh
      touchScreenCoolDownTime = millis() + touchScreenCoolDownInterval; // Cool the touch off
    }
  }

  // Image Related
  void clearImage()
  {
    int imagePosition = screenCenterX - (imageWidth / 2);
    tft.fillRect(imagePosition, 0, imageWidth, imageHeight, TFT_BLACK);
    
  }

  boolean processImageInfo(CurrentlyPlaying currentlyPlaying)
  {
    SpotifyImage currentlyPlayingMedImage = currentlyPlaying.albumImages[currentlyPlaying.numImages - 2];
    if (!albumDisplayed || !isSameAlbum(currentlyPlayingMedImage.url))
    {
      // We have a differenent album than we currently have displayed
      albumDisplayed = false;
      setImageHeight(currentlyPlayingMedImage.height / 2); // medium image is 300, we are going to scale it to half
      setImageWidth(currentlyPlayingMedImage.width / 2);
      setAlbumArtUrl(currentlyPlayingMedImage.url);
      return true;
    }

    return false;
  }

  int displayImage()
  {
    int imageStatus = displayImageUsingFile(_albumArtUrl);
    DG2("imageStatus: ", imageStatus);
    if (imageStatus == 1)
    {
      albumDisplayed = true;
      return imageStatus;
    }

    return imageStatus;
  }

  void drawWifiManagerMessage(WiFiManager *myWiFiManager)
  {
    DG("Entered Conf Mode");
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawCentreString("Entered Conf Mode:", screenCenterX, 5, 2);
    tft.drawString("Connect to the following WIFI AP:", 5, 28, 2);
    tft.setTextColor(TFT_BLUE, TFT_BLACK);
    tft.drawString(myWiFiManager->getConfigPortalSSID(), 20, 48, 2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Password:", 5, 64, 2);
    tft.setTextColor(TFT_BLUE, TFT_BLACK);
    tft.drawString("thing123", 20, 82, 2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.drawString("If it doesn't AutoConnect, use this IP:", 5, 110, 2);
    tft.setTextColor(TFT_BLUE, TFT_BLACK);
    tft.drawString(WiFi.softAPIP().toString(), 20, 128, 2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
  }

  void drawRefreshTokenMessage()
  {
    DG("Refresh Token Mode");
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawCentreString("Refresh Token Mode:", screenCenterX, 5, 2);
    tft.drawString("You need to authorize this device to use", 5, 28, 2);
    tft.drawString("your spotify account.", 5, 46, 2);

    tft.drawString("Visit the following address and follow", 5, 82, 2);
    tft.drawString("the instrucitons:", 5, 100, 2);
    tft.setTextColor(TFT_BLUE, TFT_BLACK);
    tft.drawString(WiFi.localIP().toString(), 10, 128, 2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
  }
    
void drawClockWeatherScreen(const char *timeText, const char *weatherText, const char *locationText)
{
  tft.fillScreen(TFT_BLACK);

  tft.drawFastHLine(0, 0, screenWidth, TFT_WHITE);
  tft.drawFastHLine(0, 1, screenWidth, TFT_DARKGREY);
  tft.drawFastHLine(0, screenHeight - 2, screenWidth, TFT_DARKGREY);
  tft.drawFastHLine(0, screenHeight - 1, screenWidth, TFT_WHITE);

  String fullTime = String(timeText);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString(fullTime, screenCenterX, 52, 7);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawCentreString(locationText, screenCenterX, 105, 2);
  tft.drawFastHLine(30, 138, screenWidth - 60, TFT_DARKGREY);

  uint16_t weatherColor = TFT_CYAN;
  String wx = String(weatherText);

  if (wx.indexOf("Rain") >= 0 || wx.indexOf("Drzl") >= 0 || wx.indexOf("Shower") >= 0)
    weatherColor = TFT_BLUE;
  else if (wx.indexOf("Storm") >= 0)
    weatherColor = TFT_MAGENTA;
  else if (wx.indexOf("Snow") >= 0)
    weatherColor = TFT_WHITE;
  else if (wx.indexOf("Clear") >= 0)
    weatherColor = TFT_YELLOW;
  else if (wx.indexOf("Fog") >= 0)
    weatherColor = TFT_LIGHTGREY;
  else if (wx.indexOf("Cloud") >= 0 || wx.indexOf("Overcast") >= 0)
    weatherColor = TFT_SILVER;

  tft.setTextColor(weatherColor, TFT_BLACK);
  tft.drawCentreString(weatherText, screenCenterX, 155, 4);
}

    
private:
  unsigned long touchScreenCoolDownInterval = 200; // How long after a touch press do we accept another (0.2 seconds). There is also an APi request inbetween
  unsigned long touchScreenCoolDownTime;           // time when cool down has expired

  int displayImageUsingFile(char *albumArtUrl)
  {

    // In this example I reuse the same filename
    // over and over, maybe saving the art using
    // the album URI as the name would be better
    // as you could save having to download them each
    // time, but this seems to work fine.
    if (SPIFFS.exists(ALBUM_ART) == true)
    {
      DG("Removing existing image");
      SPIFFS.remove(ALBUM_ART);
    }

    fs::File f = SPIFFS.open(ALBUM_ART, "w+");
    if (!f)
    {
      DG("file open failed");
      return -1;
    }

    // Spotify uses a different cert for the Image server, so we need to swap to that for the call
    client.setCACert(spotify_image_server_cert);
    bool gotImage = spotify_display->getImage(albumArtUrl, &f);

    // Swapping back to the main spotify cert
    client.setCACert(spotify_server_cert);

    // Make sure to close the file!
    f.close();

    if (gotImage)
    {
      return drawImagefromFile(ALBUM_ART);
    }
    else
    {
      return -2;
    }
  }

  int drawImagefromFile(const char *imageFileUri)
  {
    unsigned long lTime = millis();
    lTime = millis();
    jpeg.open((const char *)imageFileUri, myOpen, myClose, myRead, mySeek, JPEGDraw);
    jpeg.setPixelType(1);
    int imagePosition = screenCenterX - (imageWidth / 2);
    // decode will return 1 on sucess and 0 on a failure
    int decodeStatus = jpeg.decode(imagePosition, 0, JPEG_SCALE_HALF);
    // jpeg.decode(45, 0, 0);
    jpeg.close();
    DG2("Time taken to decode and display Image (ms): ", String(millis() - lTime));

    return decodeStatus;
  }
    
  void drawTouchButtons(bool backStatus, bool forwardStatus)
  {

    int buttonCenterY = 75;
    int leftButtonCenterX = 40;
    int rightButtonCenterX = screenWidth - leftButtonCenterX;

    // Draw back Button
    tft.fillCircle(leftButtonCenterX, buttonCenterY, 16, TFT_BLACK);
    tft.drawCircle(leftButtonCenterX, buttonCenterY, 16, TFT_WHITE);
    if (backStatus)
    {
      tft.fillCircle(leftButtonCenterX, buttonCenterY, 15, TFT_GREEN);
    }
    else
    {
      tft.drawCircle(leftButtonCenterX, buttonCenterY, 15, TFT_WHITE);
    }

    tft.fillTriangle(leftButtonCenterX - 4, buttonCenterY, leftButtonCenterX + 6, buttonCenterY - 10, leftButtonCenterX + 6, buttonCenterY + 10, TFT_WHITE);
    tft.drawRect(leftButtonCenterX - 6, buttonCenterY - 10, 2, 20, TFT_WHITE);

    // Draw forward Button
    tft.fillCircle(rightButtonCenterX, buttonCenterY, 16, TFT_BLACK);
    tft.drawCircle(rightButtonCenterX, buttonCenterY, 16, TFT_WHITE);
    if (forwardStatus)
    {
      tft.fillCircle(rightButtonCenterX, buttonCenterY, 15, TFT_GREEN);
    }
    else
    {
      tft.drawCircle(rightButtonCenterX, buttonCenterY, 15, TFT_WHITE);
    }

    tft.fillTriangle(rightButtonCenterX + 4, buttonCenterY, rightButtonCenterX - 6, buttonCenterY - 10, rightButtonCenterX - 6, buttonCenterY + 10, TFT_WHITE);
    tft.drawRect(rightButtonCenterX + 6, buttonCenterY - 10, 2, 20, TFT_WHITE);
  }
};
