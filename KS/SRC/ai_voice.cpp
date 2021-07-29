#include "global.h"

#include "wds.h"

#ifdef GCCULL
#include "ai_voice.h"
#include "ai_interface.h"
#include "sound_interface.h"
#include "entity.h"

const pstring ai_voice_box::voice_id[_NUM_VOICE_COMMANDS] =
{
  "SILENCE",

  // vocals
  "NEW_THREAT",
  "LOST_THREAT",
  "NEW_CUE_VIS",
  "NEW_CUE_AUD",
  "INVESTIGATE",
  "THREATEN",
  "COWER",
  "RUN",
  "TAUNT",
  "PAIN",
  "ATTACK",

  // responses
  "NEW_THREAT_RESP",
  "LOST_THREAT_RESP",
  "NEW_CUE_VIS_RESP",
  "NEW_CUE_AUD_RESP",
  "INVESTIGATE_RESP",
  "THREATEN_RESP",
  "COWER_RESP",
  "RUN_RESP",
  "TAUNT_RESP"
};


#define TEMP_SILENCE_TIMER 5.0f


ai_voice_box::ai_voice_box(ai_interface *own)
{
//  LipSyncData  LPData;
//  LipSyncMngr* pLipSync = g_world_ptr->m_pLipSync;

  owner = own;
  silence_timer = 0.0f;

//  if ( pLipSync && !owner->get_my_entity()->is_hero() )
//  {
	// set up the data for the lipsync
//	LPData.pObject = owner->get_my_entity();
//	LPData.VOQFileName = "VOData\\TestSnd.VOQ";
//	LPData.AnimName = "Characters\\LipSync\\LipSync_Talk.anmx";

	// notify the LipSync Manager of the Lipsync data
//	LSHandle = pLipSync->LoadLipSyncData(&LPData);
//  }

}

ai_voice_box::~ai_voice_box()
{
}

void ai_voice_box::copy(ai_voice_box *b)
{
}

entity *ai_voice_box::get_my_entity() const
{
  return(owner->get_my_entity());
}

void ai_voice_box::frame_advance(time_value_t t)
{
	silence_timer -= t;
}

void ai_voice_box::read_data(chunk_file& fs)
{
  stringx label;
  for ( serial_in(fs,&label); label!=chunkend_label; serial_in(fs,&label) )
  {
    handle_chunk(fs, label);
  }
}

void ai_voice_box::handle_chunk(chunk_file& fs, stringx &label)
{
}

bool ai_voice_box::say(eVoiceCommand vcom, rational_t priority )
{
  sg_entry *entry;

  if(silence_timer <= 0.0f || priority > 1.0f)
  {
    assert(vcom >= _VOICE_SILENCE && vcom < _NUM_VOICE_COMMANDS);

    if(vcom != _VOICE_SILENCE && get_my_entity()->has_sound_ifc())
    {
		  if ( check_priority( priority ) )
		  {
			  entry = get_my_entity()->sound_ifc()->play_3d_sound_grp(voice_id[vcom]);
			  if(entry)
			  {
				  sound_priority sp;
				  sp.priority = priority;
				  sp.id = entry->last_id_played;
				  m_priority_data.push_back(sp);

				  silence_timer = TEMP_SILENCE_TIMER;
				  return(true);
			  }
		  }
    }
  }

  return(false);
}

bool ai_voice_box::say(const pstring &snd_id, rational_t priority)
{
  sg_entry *entry;

  if(silence_timer <= 0.0f || priority > 1.0f)
  {
    if(get_my_entity()->has_sound_ifc())
    {
		  if ( check_priority( priority ) )
		  {
			  entry = get_my_entity()->sound_ifc()->play_3d_sound_grp(snd_id);
			  if(entry)
			  {
				  sound_priority sp;
				  sp.priority = priority;
				  sp.id = entry->last_id_played;
				  m_priority_data.push_back(sp);

				  silence_timer = TEMP_SILENCE_TIMER;
				  return(true);
			  }
		  }
    }
  }

  return(false);
}

bool ai_voice_box::say_file(const stringx &snd, rational_t priority)
{
  if(silence_timer <= 0.0f || priority > 1.0f)
  {
    if(get_my_entity()->has_sound_ifc())
    {
		  if ( check_priority( priority ) )
		  {
			  unsigned int id = get_my_entity()->sound_ifc()->play_3d_sound(snd);
			  if(id > 0)
			  {
				  sound_priority sp;
				  sp.priority = priority;
				  sp.id = id;
				  m_priority_data.push_back(sp);

				  silence_timer = TEMP_SILENCE_TIMER;
				  return(true);
			  }
		  }
    }
  }

  return(false);
}

bool ai_voice_box::check_priority( rational_t priority )
{
	if ( !get_my_entity()->has_sound_ifc() )
		return( false );

	if ( m_priority_data.empty() )
		return( true );

	vector<sound_priority>::iterator i;
	for ( i=m_priority_data.begin(); i!=m_priority_data.end(); )
	{
		// NEW sound has higher priority, play the NEW sound( higher number = higher priority )
		if ( priority > i->priority || !get_my_entity()->sound_ifc()->sound_playing(i->id) )
		{
			m_priority_data.erase(i);
			return( true );
		}
    ++i;
	}

	return( false );

}

#endif
