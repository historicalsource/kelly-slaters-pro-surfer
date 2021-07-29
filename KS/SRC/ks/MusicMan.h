#ifndef MUSICMAN_H
#define MUSICMAN_H
#include "singleton.h"
#include "ksnsl.h"

#define MAX_TUBE_MUSIC_DIST 20
#define MAX_SONGS 50
#define MUSIC_VOLUME_ADJUSTMENT_FACTOR 1.0f
class Track
{
  nslSourceId mySrcId;
  nslSoundId mySndId;
  bool paused;
public:
  char songName[30];
  char artistName[30];
  Track();
  ~Track();
  void SetInfo(const char *name, const char *artist, nslSourceId src);
  nslSoundId Play();
  bool IsPlaying();
  void Stop();
  void Pause();
  void Unpause();
};

// This class plays a random source 
class MusicListing
{
private:
  int               totalSources;
  int               current;
  Track             sources[MAX_SONGS];
  nslSoundId        currentSoundId;
  bool              sucessfulLastPlay;
public:
  MusicListing();
  ~MusicListing();
  int         getNumSongs()                   { return totalSources;                    };
  void        disable(int song, bool on)      { enabled[order[song]] = !on;             };
  bool        isDisabled(int song)            { return !enabled[order[song]];           };
  nslSoundId  getSoundId()                    { return currentSoundId;                  };
  char*       getTrackName(int pos)           { if (pos < totalSources) return sources[order[pos]].songName; else return NULL;; };
  char*       getArtistName(int pos)          { if (pos < totalSources) return sources[order[pos]].artistName; else return NULL;; };
  void        setCurrent(int pos)             { current = pos;                          };
  int         getCurrent()                    { return current;                         };
  void        addSource( const char *artist, const char *name, nslSourceId s );
  void        clearSources();
  nslSoundId  playNext();
  nslSoundId  play();
  bool        isPlaying();
  void        stop();
  void        pause()                         { sources[order[current]].Pause();        };
  void        unpause()                       { sources[order[current]].Unpause();      };
  void        shutdown();
  void        randomize();
  void        alphabetize();
  void        swap(int a, int b);
  
  int               order[MAX_SONGS];
  bool              enabled[MAX_SONGS];
};



class MusicMan : public singleton
{
private:
  
  float volume;
  int inited;
  bool paused;
public:
  DECLARE_SINGLETON(MusicMan);

  MusicMan();
  ~MusicMan();
  MusicListing musicTrack;
  void  setCurrent(int cur)     { musicTrack.setCurrent(cur); };
  void  swap (int a, int b)     { musicTrack.swap(a, b); };
  int   getNumSongs()           { return musicTrack.getNumSongs(); };
  char* getSongName(int index)  { return musicTrack.getTrackName(index); };
  char* getArtistName(int index){ return musicTrack.getArtistName(index); };
  bool  isPlaying()             { return musicTrack.isPlaying();  };
  nslSoundId playNext();
  nslSoundId play();
  void loadSources();
  void setVolume(float vol)     { volume = vol*MUSIC_VOLUME_ADJUSTMENT_FACTOR;};
	float getVolume()     { return volume/MUSIC_VOLUME_ADJUSTMENT_FACTOR; }
  void shutdown();
  void pause()                  { if (!paused) { paused = true; musicTrack.pause(); } };
  void unpause()                { if (paused) { paused = false; musicTrack.unpause(); } };
  void init();
  void tick(bool inGame);
  void stop();
  void disable(int which, bool disabled)  { musicTrack.disable(which, disabled); };
  bool isDisabled(int which)    { return musicTrack.isDisabled(which); };
  void alphabetize()            { musicTrack.alphabetize(); };
  void randomize()              { musicTrack.randomize(); };
};

#endif
