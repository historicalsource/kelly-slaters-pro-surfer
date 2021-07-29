	#include "global.h"
#include "SoundScript.h"
#include "timer.h"



void ScheduledSoundEvent::clear()
{
	snd = NSL_INVALID_ID;
	myEntity = NULL;
	time = -1;
	event = SS_LAST;	
}


CurrentSoundEvent::CurrentSoundEvent()
{
	eId   = NSL_GLOBAL_EMITTER_ID;
	snd   = NSL_INVALID_ID;
	event = SS_LAST;
}
void CurrentSoundEvent::clear()
{
	if (nslGetSoundStatus(snd) != NSL_SOUNDSTATUS_INVALID)
		nslStopSound(snd);
	if (eId != NSL_GLOBAL_EMITTER_ID)
		nslReleaseEmitter(eId);
	eId = NSL_GLOBAL_EMITTER_ID;
	snd = NSL_INVALID_ID;
	event = SS_LAST;
}


// The event map stuff
EventMapType::EventMapType()
{
	int i;
	numSrcs = 0;
	type = SS_LAST;
	for (i=0; i < MAX_EVENT_SOURCES; i++)
	{
		srcs[i] = NSL_INVALID_ID;
	}
}

EventMapType::EventMapType(EventType myType)
{
	int i;
	type = myType;
	numSrcs = 0;
	for (i=0; i < MAX_EVENT_SOURCES; i++)
	{
		srcs[i] = NSL_INVALID_ID;
	}
}

nslSoundId EventMapType::playEvent(entity *e)
{
	nslSoundId snd = NSL_INVALID_ID;
	nslSourceId src = NSL_INVALID_ID;

	// The case where we have no sources
	// Play the placeholder stuff
	if (numSrcs == 0)
	{
		if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_PLAY_PLACEHOLDER))
		{
			nslSourceId src=nslLoadSource("PLACEHOLDER");
			if (src != NSL_INVALID_ID)
				snd = nslAddSound(src);
			if (nslGetSoundStatus(snd) != NSL_INVALID_ID)
			{
				nslPlaySound(snd);
				return snd;
			}
		}
		return NSL_INVALID_ID;
	}

 	src = srcs[random(numSrcs)];
	
	// If its the place holder
	// only play if we have placeholders turned on
	if ((stricmp(nslGetSourceName(src), "PLACEHOLDER") == 0) && (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_PLAY_PLACEHOLDER)))
		return NSL_INVALID_ID;

	// Play it
	snd = nslAddSound(src);
	if (nslGetSoundStatus(snd) != NSL_SOUNDSTATUS_INVALID)
	{
		if ((EventDampGuard[type]) && (nslGetSoundStatus(snd) != NSL_SOUNDSTATUS_INVALID))
		{
			nslDampenGuardSound(snd);
		}
		if (e)
		{
			nslPlaySound(snd);
		}
		else
		{
			nslPlaySound(snd);
		}
	}
	return snd;
}

bool EventMapType::addSoundMapping(nslSourceId newSrc)
{
	if (numSrcs != MAX_EVENT_SOURCES && newSrc != NSL_INVALID_ID)
		srcs[numSrcs++] = newSrc;
	else 
		return false;
	return true;
}


void EventMapType::clear()
{
	int i;
	for (i=0; i < numSrcs; i++)
		srcs[i] = NSL_INVALID_ID;

	numSrcs = 0;
}

DEFINE_SINGLETON(SoundScriptManager)

// Begin sound manager stuff
bool SoundScriptManager::init()
{
	paused = false;
	nlVector3d v;
	v[0] = v[1] = v[2] = -9999;
  clearEvents();
	createAllMappings();
  return true;
}

void SoundScriptManager::clearEvents()
{ 
	int i;
  for (i=0; i < MAX_TOTAL_EVENTS; i++)
  {
    scheduledLevelEvents[i].clear();
    playingEvents[i].clear();		
  }
	numEvents = 0;
}

nslSoundId SoundScriptManager::playSound(EventType type, entity *e)	
{
	return eventMap[type].playEvent(e);
}

void SoundScriptManager::createAllMappings()
{	
	int i;
	for (i=0; i < SS_LAST; i++)
	{
		eventMap[i].clear();
	}

}
void SoundScriptManager::pause()
{
	paused = true;
}


void SoundScriptManager::unpause()
{
	paused = false;
}
nslSoundId	SoundScriptManager::getSoundId(SSEventId e)
{
	if (playingEvents[e].event == SS_LAST)
		return NSL_INVALID_ID;
	else
		return playingEvents[e].snd;
}



void SoundScriptManager::tick(float t)
{
	if (paused) return;
	if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
	{
		for (int i=0; i < MAX_TOTAL_EVENTS; i++)
		{
			if ((TIMER_GetLevelSec() > scheduledLevelEvents[i].time) && (scheduledLevelEvents[i].time > 0) && 
					(scheduledLevelEvents[i].event != SS_LAST) && (scheduledLevelEvents[i].snd == NSL_INVALID_ID))
			{
				
				playEvent(scheduledLevelEvents[i].event, scheduledLevelEvents[i].myEntity);
				scheduledLevelEvents[i].clear();
				
			}
			int status = nslGetSoundStatus(playingEvents[i].snd);

			if (playingEvents[i].snd != NSL_INVALID_ID &&  status == NSL_SOUNDSTATUS_INVALID)
			{
				playingEvents[i].clear();		
			}
			else if (status != NSL_SOUNDSTATUS_INVALID)
			{
	 			// Update volumes
				float vol = nslGetSoundParam(playingEvents[i].snd, NSL_SOUNDPARAM_VOLUME);
				if (playingEvents[i].fadingOut && vol > 0.0f && playingEvents[i].fadeOutTime > 0)
				{
					vol -= (t / playingEvents[i].fadeOutTime);
					if (vol <= 0.0f)
						playingEvents[i].clear();
					else 
						nslSetSoundParam(playingEvents[i].snd, NSL_SOUNDPARAM_VOLUME, vol);

				}
				else if ((playingEvents[i].fadingOut && vol > 0) || (!playingEvents[i].fadingOut && vol < 1.0f))
				{
					if (!playingEvents[i].fadingOut && vol < 1.0f && playingEvents[i].fadeInTime > 0)
						vol += (t / playingEvents[i].fadeInTime);
					if (vol >= 1.0f)
						vol = 1.0f;
					nslSetSoundParam(playingEvents[i].snd, NSL_SOUNDPARAM_VOLUME, vol);
				}

				if (playingEvents[i].myEntity != NULL && playingEvents[i].eId !=  NSL_GLOBAL_EMITTER_ID)
				{
					vector3d v;
					nlVector3d newv;
					v = playingEvents[i].myEntity->get_abs_position();
					newv[0] = v.x;
					newv[1] = v.y;
					newv[2] = v.z;
					nslSetEmitterPosition(playingEvents[i].eId, newv);
				}
			}
		}
	}
}

void SoundScriptManager::endEvent(SSEventId id, float fadeOutTime)
{
	assert (fadeOutTime >= 0);
	if (id >=0 && id < MAX_TOTAL_EVENTS)
	{
		if(fadeOutTime==0)
			playingEvents[id].clear();
		else
		{
			playingEvents[id].fadingOut = true;
			playingEvents[id].fadeOutTime = fadeOutTime;
		}

	}
}

SSEventId SoundScriptManager::startEvent(EventType etype, entity *e, float fadeInTime)
{
	return playEvent(etype, e, fadeInTime);
}

SSEventId SoundScriptManager::playEvent(EventType etype, entity *e, float fadeInTime)
{
	if (paused) 
		return -1;

	if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
	{
		int eventIdx = -1, i = 0;
		
		// Stop the previously playing one
		for (i=0; i < MAX_TOTAL_EVENTS; i++)
		{
			if (eventIdx < 0 && playingEvents[i].event == SS_LAST)
			{
				eventIdx = i;
			}
			
			if (EventFreq[etype] != SS_MANY && playingEvents[i].event == etype)
			{
				if ((playingEvents[i].myEntity == e && EventFreq[etype] == SS_ONCE_PER_ENTITY) ||
				    (	EventFreq[etype] == SS_ONCE))
					endEvent(i);
			}
		}

		// Sucessfully found a free slot!
		if (eventIdx >= 0)
		{
			nlVector3d v;
			playingEvents[eventIdx].fadingOut = false;
			// If we found a slot, play!
			assert (fadeInTime >= 0);


			playingEvents[eventIdx].fadeInTime = fadeInTime;
			playingEvents[eventIdx].event = etype;
			playingEvents[eventIdx].snd = eventMap[etype].playEvent(e);
			playingEvents[eventIdx].myEntity = e;
			playingEvents[eventIdx].fadingOut = false;
			if (playingEvents[eventIdx].snd == NSL_INVALID_ID)
			{
				playingEvents[eventIdx].clear();
				return -1;
			}
			else if (playingEvents[eventIdx].fadeInTime > 0)
				nslSetSoundParam(playingEvents[eventIdx].snd, NSL_SOUNDPARAM_VOLUME, 0.0f);
			
			if (e != NULL)
			{
				v[0] = e->get_abs_position().x;
				v[1] = e->get_abs_position().y;
				v[2] = e->get_abs_position().z;
				playingEvents[eventIdx].eId = nslCreateEmitter(v);
				nslSetSoundEmitter(playingEvents[eventIdx].eId, playingEvents[eventIdx].snd);
			}


			// Return the id
			return eventIdx;
		}
		else 
			return -1;
	}
	else
		return -1;
}

// This schedules an event for the future
bool SoundScriptManager::addEvent(EventType type, float time, entity *e)
{	
	if (paused) 
		return false;	

	if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
	{
		int i, eventIdx=-1;
		assert( type < SS_LAST );
		{
			i = 0;
			while (i < MAX_TOTAL_EVENTS && eventIdx == -1 )
			{
				if (scheduledLevelEvents[i].time == -1)
				{
					eventIdx = i;
				}
				i++;
			}
			if (eventIdx != -1)
			{
					scheduledLevelEvents[eventIdx].snd = NSL_INVALID_ID;
					scheduledLevelEvents[eventIdx].myEntity = e;
					scheduledLevelEvents[eventIdx].event = type;
					scheduledLevelEvents[eventIdx].time = time;	
			}
		}
	}
	return true;
}


bool SoundScriptManager::createMapping(EventType t, nslSourceId src)
{
	eventMap[t].type = t;
	return eventMap[t].addSoundMapping(src);
}