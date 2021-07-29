// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#if defined(TARGET_XBOX)
#include "osdevopts.h"
#endif /* TARGET_XBOX JIV DEBUG */

#include "GameData.h"
#include "ksnsl.h"
#include "RumbleManager.h"
#include "MusicMan.h"

DEFINE_SINGLETON(StoredConfigData)


int StoredConfigData::getSize()
{
  return sizeof(ksConfigData);
}

void initKSConfigStruct(ksConfigData *cfg)
{
	cfg->masterVolume = 
	cfg->musicVolume = 
	cfg->sfxVolume = 
	cfg->ambientVolume =
	cfg->voiceVolume = 1.0f;
	
	cfg->scoreDisplay = true;
	cfg->audioMode = NSL_SPEAKER_STEREO;
  
	strcpy(cfg->camera[0], "BEACH_CAM0");
	strcpy(cfg->camera[1], "BEACH_CAM1");

	for (int i=0; i < MAX_SONGS; i++)
	{
		cfg->order[i]		= i;
		cfg->enabled[i] = true;
	}

	cfg->rumbleOn[0] =
	cfg->rumbleOn[1] = true;

}
void StoredConfigData::init()
{
	initKSConfigStruct(&cData);
}

void *StoredConfigData::getGameConfig()
{
  if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
  {
		
    cData.masterVolume = nslGetMasterVolume();
    cData.ambientVolume = nslGetVolume(NSL_SOURCETYPE_AMBIENT);
    cData.sfxVolume = nslGetVolume(NSL_SOURCETYPE_SFX);
    cData.musicVolume = nslGetVolume(NSL_SOURCETYPE_MUSIC);
    cData.voiceVolume = nslGetVolume(NSL_SOURCETYPE_VOICE);
    cData.audioMode =  nslGetSpeakerMode();
    memcpy(&cData.enabled, &MusicMan::inst()->musicTrack.enabled, sizeof(bool)*MAX_SONGS);
    memcpy(&cData.order, &MusicMan::inst()->musicTrack.order, sizeof(int)*MAX_SONGS);
  }
  cData.rumbleOn[0] = rumbleMan.isOn(0);
  cData.rumbleOn[1] = rumbleMan.isOn(1);
	cData.scoreDisplay = frontendmanager.score_display;
  // We don't get the camera since it should already be set
  return &cData;
}

void StoredConfigData::setGameConfig(void *cfg)
{
  if (cfg)
    cData = *(ksConfigData *)cfg;

  if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
  {
    nslSetMasterVolume(cData.masterVolume);
#ifndef TARGET_XBOX
    nslSetSpeakerMode(cData.audioMode);
#endif
    nslSetVolume(NSL_SOURCETYPE_MUSIC, cData.musicVolume);
    nslSetVolume(NSL_SOURCETYPE_SFX, cData.sfxVolume);
    nslSetVolume(NSL_SOURCETYPE_VOICE, cData.voiceVolume);
    nslSetVolume(NSL_SOURCETYPE_AMBIENT, cData.ambientVolume);
    memcpy(&MusicMan::inst()->musicTrack.enabled, &cData.enabled, sizeof(bool)*MAX_SONGS);
    memcpy(&MusicMan::inst()->musicTrack.order, &cData.order,     sizeof(int)*MAX_SONGS);

  }
	frontendmanager.enableScoreDisplay(cData.scoreDisplay);
  rumbleMan.turnOn(cData.rumbleOn[0],0);
  rumbleMan.turnOn(cData.rumbleOn[1],1);
}

void StoredConfigData::setLastCamera(int hero, const char *cam)
{
  strcpy(cData.camera[hero], cam);
}

#if defined(TARGET_XBOX)
StoredConfigData StoredConfigData::operator=(const StoredConfigData &s)
#else
StoredConfigData StoredConfigData::operator=(StoredConfigData s)
#endif /* TARGET_XBOX JIV DEBUG */
{
  cData.masterVolume = s.cData.masterVolume;
  cData.ambientVolume = s.cData.ambientVolume;
  cData.sfxVolume = s.cData.sfxVolume;
  cData.voiceVolume = s.cData.voiceVolume;
  cData.musicVolume = s.cData.musicVolume;
  cData.rumbleOn[0] = s.cData.rumbleOn[0];
  cData.rumbleOn[1] = s.cData.rumbleOn[1];
  cData.audioMode   = s.cData.audioMode;
  strcpy(cData.camera[0], s.cData.camera[0]);
	strcpy(cData.camera[1], s.cData.camera[1]);

#if defined(TARGET_XBOX)
  // JIV FIXME recursive otherwise
  StoredConfigData kludge;

  memcpy( &kludge, this, sizeof kludge );

  return kludge;
#endif /* TARGET_XBOX JIV DEBUG */
}


char *StoredConfigData::getLastCamera(int hero)
{
  return cData.camera[hero];
}
