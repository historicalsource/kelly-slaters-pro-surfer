#ifndef _SOUND_INTERFACE_H_
#define _SOUND_INTERFACE_H_

#if 0
#include "global.h"
#include "entity_interface.h"
#include "sound_group.h"
#include "hwaudio.h"

class pstring;
//class sound_emitter;

class shared_sound_group
{
protected:
  friend class sound_interface;

  unsigned int ref_count;

	// Sound groups provide lists of sounds and the ability to draw from
	// them randomly, with a built in history checker.  Be careful to add groups to
	// the map<> if they're ever loaded outside game.
	vector<sound_group> sound_groups;
	map<pstring,sound_group*> sound_group_map;

	// Example:
	// sg_entry* entry = g_game_ptr->get_sound_group_entry( "spidey_jump" );
	// sound_device::inst()->play_sound( entry->name, entry->volume, entry->pitch );
	// spidey->sound_ifc()->emitter->play_sound( entry->name, entry->volume, entry->pitch );

  // Returns the next entry in a sound group.
	sg_entry* get_sound_group_entry( const pstring& name )
  {
	  map<pstring,sound_group*>::iterator it = sound_group_map.find( name );
    return( it == sound_group_map.end() ? NULL : ( *it ).second->get_next() );
  }

  shared_sound_group() : ref_count(1)   { }
  ~shared_sound_group()                 { assert(ref_count == 0); }

  shared_sound_group *add_ref()         { ++ref_count; return(this); }
  void del_ref()                        { --ref_count; if(ref_count == 0) delete this; }
};

class sound_interface : public entity_interface
{
protected:
	sound_emitter* emitter;

  shared_sound_group *snd_grp;

  int max_voices;
  vector<unsigned int> voices;

  void add_voice(unsigned int id);

public:
	sound_interface(entity* _my_entity);
	virtual ~sound_interface();

  void copy(sound_interface *b);

  sound_emitter *get_emitter() const { return(emitter); }

  virtual void frame_advance(time_value_t t);

	// Returns the next entry in a sound group.
  sg_entry* get_sound_group_entry( const pstring& name )  { return(snd_grp != NULL ? snd_grp->get_sound_group_entry(name) : NULL); }

  // plays a non-positional sound through the sound_device
  void play_sound(const stringx &snd, rational_t volume = 1.0f, rational_t pitch = 1.0f);
  void play_sound(sound_id_t snd, rational_t volume = 1.0f, rational_t pitch = 1.0f);
  sg_entry *play_sound_grp(const pstring &snd_grp, rational_t volume = 1.0f, rational_t pitch = 1.0f);

  // plays a positional sound through the emitter
  unsigned int play_3d_sound(const stringx &snd, rational_t volume = 1.0f, rational_t pitch = 1.0f);
  unsigned int play_3d_sound(sound_id_t snd, rational_t volume = 1.0f, rational_t pitch = 1.0f);
  sg_entry *play_3d_sound_grp(const pstring &snd_grp, rational_t volume = 1.0f, rational_t pitch = 1.0f);
  sg_entry *play_looping_3d_sound_grp(const pstring &snd_grp, rational_t volume = 1.0f, rational_t pitch = 1.0f);

  bool sound_playing(unsigned int id) const { return(emitter->sound_playing(id)); }
  void kill_sound(unsigned int id) { emitter->kill_sound(id); }

	void read_enx_data( chunk_file& fs, stringx& lstr );
};
#endif

#endif
