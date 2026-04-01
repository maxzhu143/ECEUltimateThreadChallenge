SpotifyDisplay *sp_Display;

SpotifyArduino spotify(client, NULL, NULL);

bool albumArtChanged = false;
bool currentPlaybackState = false;

long songStartMillis;
long songDuration;

char lastTrackUri[200];
char lastTrackContextUri[200];

unsigned long delayBetweenRequests = 5000;
unsigned long requestDueTime;

unsigned long delayBetweenProgressUpdates = 500;
unsigned long progressDueTime;

bool playbackCommandInFlight = false;
unsigned long playbackCommandTime = 0;
const unsigned long PLAYBACK_STATE_LOCKOUT_MS = 3000;

void reconnectClient() {
  client.stop();
  delay(100);
  client.setCACert(spotify_server_cert);
}

void spotifySetup(SpotifyDisplay *theDisplay, const char *clientId, const char *clientSecret)
{
  sp_Display = theDisplay;
  client.setCACert(spotify_server_cert);
  spotify.lateInit(clientId, clientSecret);

  albumArtChanged = false;
  currentPlaybackState = false;
  songStartMillis = 0;
  songDuration = 0;
  requestDueTime = 0;
  progressDueTime = 0;

  lastTrackUri[0] = '\0';
  lastTrackContextUri[0] = '\0';
}

bool isSameTrack(const char *trackUri)
{
  if (trackUri == NULL)
  {
    return false;
  }
  return strcmp(lastTrackUri, trackUri) == 0;
}

void setTrackUri(const char *trackUri)
{
  if (trackUri == NULL)
  {
    lastTrackUri[0] = '\0';
    return;
  }
  strncpy(lastTrackUri, trackUri, sizeof(lastTrackUri) - 1);
  lastTrackUri[sizeof(lastTrackUri) - 1] = '\0';
}

void setTrackContextUri(const char *trackContext)
{
  if (trackContext == NULL)
  {
    lastTrackContextUri[0] = '\0';
  }
  else
  {
    strncpy(lastTrackContextUri, trackContext, sizeof(lastTrackContextUri) - 1);
    lastTrackContextUri[sizeof(lastTrackContextUri) - 1] = '\0';
  }
}

void spotifyRefreshToken(const char *refreshToken)
{
  spotify.setRefreshToken(refreshToken);

  
  DG("Refreshing Access Tokens");
  if (!spotify.refreshAccessToken())
  {
    DG("Failed to get access tokens");
  }
}

bool forceSpotifyRefreshSoon()
{
  requestDueTime = 0;
  progressDueTime = 0;
  return true;
}

bool togglePlayback()
{
  if (playbackCommandInFlight) return false;
  playbackCommandInFlight = true;
  playbackCommandTime = millis();

  bool success = false;

  client.stop();
  delay(100);
  client.setCACert(spotify_server_cert);

  if (currentPlaybackState)
  {
    DG("Pausing Spotify playback");
    success = spotify.pause();

    if (success)
    {
      currentPlaybackState = false;
      songStartMillis = 0;
    }
  }
  else
  {
    DG("Resuming Spotify playback");

    // First try the normal resume call
    success = spotify.play();

    // If normal resume fails, try restarting the last known context/track
    // Use whichever of these functions your SpotifyArduino library supports.
    if (!success && lastTrackContextUri[0] != '\0')
    {
      DG2("spotify.play() failed, trying context: ", lastTrackContextUri);
      success = spotify.playAdvanced(lastTrackContextUri, lastTrackUri);
      // If your library does NOT have playAdvanced(context, track),
      // use its equivalent "play context" function here instead.
    }
    else if (!success && lastTrackUri[0] != '\0')
    {
      DG2("spotify.play() failed, trying track: ", lastTrackUri);
      success = spotify.playAdvanced(NULL, lastTrackUri);
      // If your library does NOT have playAdvanced(NULL, track),
      // use its equivalent "play track" function here instead.
    }

    if (success)
    {
      currentPlaybackState = true;
      songStartMillis = millis();
    }
  }

  DG2("togglePlayback result: ", success ? "success" : "failed");

  requestDueTime = millis() + 1000;
  progressDueTime = millis() + 1000;

  playbackCommandInFlight = false;
  return success;
}

void handleCurrentlyPlaying(CurrentlyPlaying currentlyPlaying)
{
  if (millis() > playbackCommandTime + PLAYBACK_STATE_LOCKOUT_MS)
  {
    currentPlaybackState = currentlyPlaying.isPlaying;
  }

  if (currentlyPlaying.trackUri != NULL)
  {
    if (!isSameTrack(currentlyPlaying.trackUri))
    {
      setTrackUri(currentlyPlaying.trackUri);
      setTrackContextUri(currentlyPlaying.contextUri);
      sp_Display->printCurrentlyPlayingToScreen(currentlyPlaying);
    }

    albumArtChanged = sp_Display->processImageInfo(currentlyPlaying);
    sp_Display->displayTrackProgress(currentlyPlaying.progressMs, currentlyPlaying.durationMs);

    if (currentlyPlaying.isPlaying)
    {
      songStartMillis = millis() - currentlyPlaying.progressMs;
      songDuration = currentlyPlaying.durationMs;
    }
    else
    {
      songStartMillis = 0;
      songDuration = currentlyPlaying.durationMs;
    }
  }
  else
  {
    setTrackUri(NULL);
    setTrackContextUri(NULL);
    songStartMillis = 0;
    songDuration = 0;
  }
}

void updateProgressBar()
{
  if (songStartMillis != 0 && millis() > progressDueTime)
  {
    long songProgress = millis() - songStartMillis;
    if (songProgress > songDuration)
    {
      songProgress = songDuration;
    }
    sp_Display->displayTrackProgress(songProgress, songDuration);
    progressDueTime = millis() + delayBetweenProgressUpdates;
  }
}

void updateCurrentlyPlaying(boolean forceUpdate)
{
  if (forceUpdate || millis() > requestDueTime)
  {
    if (forceUpdate)
    {
      DG("forcing an update");
    }

    DG("getting currently playing song:");
    int status = spotify.getCurrentlyPlaying(handleCurrentlyPlaying, SPOTIFY_MARKET);
    if (status == 200)
    {
      DG("Successfully got currently playing");
      if (albumArtChanged || forceUpdate)
      {
        sp_Display->clearImage();
        int displayImageResult = sp_Display->displayImage();

        if (displayImageResult)
        {
          albumArtChanged = false;
        }
        else
        {
          DG2("failed to display image: ", displayImageResult);
        }
      }
    }
    else if (status == 204)
    {
      currentPlaybackState = false;
      songStartMillis = 0;
      songDuration = 0;

      DG("No active playback device/content right now");
    }
    else
    {
      DG2("Error: ", status);
    }

    requestDueTime = millis() + delayBetweenRequests;
  }
}
