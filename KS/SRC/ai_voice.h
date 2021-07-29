#ifndef _AI_VOICE_H_
#define _AI_VOICE_H_

#ifdef GCCULL

#include "global.h"
#include "ostimer.h"
#include "simple_classes.h"

class ai_interface;
class entity;
//class sound_interface;
class pstring;
//class sg_entry;

typedef enum eVoiceCommand
{
  _VOICE_SILENCE,

  // vocals
  _VOICE_NEW_THREAT,
  _VOICE_LOST_THREAT,
  _VOICE_NEW_CUE_VIS,
  _VOICE_NEW_CUE_AUD,
  _VOICE_INVESTIGATE,
  _VOICE_THREATEN,
  _VOICE_COWER,
  _VOICE_RUN,
  _VOICE_TAUNT,
  _VOICE_PAIN,
  _VOICE_ATTACK,

  // responses
  _VOICE_NEW_THREAT_RESPONSE,
  _VOICE_LOST_THREAT_RESPONSE,
  _VOICE_NEW_CUE_VIS_RESPONSE,
  _VOICE_NEW_CUE_AUD_RESPONSE,
  _VOICE_INVESTIGATE_RESPONSE,
  _VOICE_THREATEN_RESPONSE,
  _VOICE_COWER_RESPONSE,
  _VOICE_RUN_RESPONSE,
  _VOICE_TAUNT_RESPONSE,

  _NUM_VOICE_COMMANDS
};



class ai_voice_box
{
private:
	struct sound_priority
	{
		unsigned int id;
		rational_t priority;		// priority of that sound
	};

	vector<sound_priority>	m_priority_data;

	bool	check_priority( rational_t priority );	// determines if the NEW requested sound can override the current playing sound

protected:
  friend class ai_interface;

  ai_interface *owner;

//  int			LSHandle;	// handle to lip sync

  void read_data(chunk_file& fs);
  virtual void handle_chunk(chunk_file& fs, stringx &label);

  virtual void frame_advance(time_value_t t);

  static const pstring voice_id[_NUM_VOICE_COMMANDS];

  // quick hack to quiet them down a bit
  simple_timer silence_timer;

public:
  ai_voice_box(ai_interface *own);
  virtual ~ai_voice_box();

  void copy(ai_voice_box *b);

  ai_interface *get_ai_interface() const    { return(owner); }
  inline entity *get_my_entity() const;

  bool say(eVoiceCommand vcom = _VOICE_SILENCE, rational_t priority = 1.0f);
  bool say(const pstring &snd_id, rational_t priority = 1.0f);
  bool say_file(const stringx &snd, rational_t priority = 1.0f);
};

#endif

#endif
