#ifndef SOUNDSCRIPTH
#define SOUNDSCRIPTH


#include "global.h"
#include "sounddata.h"
#include "entity.h"

#define MAX_TOTAL_EVENTS 100
#define MAX_EVENT_SOURCES 30
#define SS_PLAY_NOW -999999
typedef int SSEventId;

class EventMapType
{
public:
	EventMapType();
	EventMapType(EventType myType);
	~EventMapType() {};
	bool addSoundMapping(nslSourceId newSrc);
	nslSoundId playEvent(entity *e);
	void clear();
	EventType type;
private:
	int	numSrcs;
	nslSourceId srcs[MAX_EVENT_SOURCES];
};

class ScheduledSoundEvent
{
public:
	void clear();
  float time;
  EventType event;
	nslSoundId snd;
	entity *myEntity;
};

class  CurrentSoundEvent
{
public:
	CurrentSoundEvent();
	void clear();
  EventType event;
	nslSoundId snd;
	entity *myEntity;
	float fadeInTime;
	float fadeOutTime;
	bool fadingOut;
	nslEmitterId eId;
};

class SoundScriptManager : public singleton
{
private:
	EventMapType				eventMap[SS_LAST];
  ScheduledSoundEvent scheduledLevelEvents[MAX_TOTAL_EVENTS];
	CurrentSoundEvent		playingEvents[MAX_TOTAL_EVENTS];
	int numEvents;
	bool paused;
public:
	DECLARE_SINGLETON(SoundScriptManager);
	nslSoundId	getSoundId(SSEventId e);
	void        pause();
	void        unpause();
  bool        init();
  void        tick(float t);
  void        clearEvents();
	void        endEvent(SSEventId e, float fadeOutTime=0);
	SSEventId	  startEvent(EventType etype, entity *e=NULL, float fadeInTime=0);
	bool        addEvent(EventType type, float time=SS_PLAY_NOW, entity *e=NULL);
	SSEventId   playEvent(EventType type, entity *e=NULL, float fadeInTime=0);
	nslSoundId  playSound(EventType type, entity *e=NULL);
	bool        createMapping(EventType t, nslSourceId src);
	void        createAllMappings();
};


#endif