#ifndef __GAMEDATAH__
#define __GAMEDATAH__


#if defined(TARGET_XBOX)
#include "ksnsl.h"
#endif /* TARGET_XBOX JIV DEBUG */
#include "MusicMan.h"



class StoredGameData
{
public:
  int getSize();
  void *getGameData();
  void setGameData(void *);
  void setConfig(char *shortname);
private:
  
};


typedef struct {
  float               masterVolume, ambientVolume, musicVolume, voiceVolume, sfxVolume;
  bool                rumbleOn[2];
  bool                scoreDisplay;
  nslSpeakerModeEnum  audioMode;
  char                camera[2][20];
  int                 order[MAX_SONGS];
  bool                enabled[MAX_SONGS];
} ksConfigData;

void initKSConfigStruct(ksConfigData *cfg);


class StoredConfigData : singleton
{
public:
  StoredConfigData() {};
  StoredConfigData(const StoredConfigData &s) {*this = s;};
#if defined(TARGET_XBOX)
  StoredConfigData operator=(const StoredConfigData &s);
#else
  StoredConfigData operator=(StoredConfigData s);
#endif /* TARGET_XBOX JIV DEBUG */
  int getSize();
  void *getGameConfig();
  void setGameConfig(void *);
  void setLastCamera(int hero, const char *camera);
  char *getLastCamera(int hero);
  void init();
  DECLARE_SINGLETON(StoredConfigData)
private:
  ksConfigData cData;
  
};


#endif
