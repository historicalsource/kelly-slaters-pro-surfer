#include "global.h"

#include "sound_interface.h"
#include "entity_interface.h"
#include "time_interface.h"
#include "entity.h"
#include "pstring.h"

#if 0

sound_interface::sound_interface(entity* _my_entity)
  : entity_interface(_my_entity)
{/*
	emitter = sound_device::inst()->create_emitter();
  emitter->set_position(my_entity->get_abs_position());
  snd_grp = NULL;
  max_voices = -1;*/
}

sound_interface::~sound_interface()
{/*
	emitter->release();

  delete emitter;

  if(snd_grp)
  {
    snd_grp->del_ref();
    snd_grp = NULL;
  }*/
}

void sound_interface::copy(sound_interface *b)
{/*
  if(snd_grp)
  {
    snd_grp->del_ref();
    snd_grp = NULL;
  }

  if(b->snd_grp)
    snd_grp = b->snd_grp->add_ref();
    */
}


void sound_interface::play_sound(const stringx &snd, rational_t volume, rational_t pitch)
{
	//sound_device::inst()->play_sound( snd, volume, pitch*ENTITY_TIME_DILATION(my_entity) );
}

void sound_interface::play_sound(sound_id_t snd, rational_t volume, rational_t pitch)
{
	//sound_device::inst()->play_sound( snd, volume, pitch*ENTITY_TIME_DILATION(my_entity) );
}

sg_entry *sound_interface::play_sound_grp(const pstring &snd_grp, rational_t volume, rational_t pitch)
{
	/*sg_entry* entry = get_sound_group_entry( snd_grp );
  if(entry)
		sound_device::inst()->play_sound( entry->name, entry->variance(entry->volume, entry->volume_var)*volume, entry->variance(entry->pitch, entry->pitch_var)*pitch*ENTITY_TIME_DILATION(my_entity) );

  return(entry);*/
  return(NULL);
}



unsigned int sound_interface::play_3d_sound(const stringx &snd, rational_t volume, rational_t pitch)
{
	//return(emitter->play_sound( snd, volume, pitch*ENTITY_TIME_DILATION(my_entity) ));
  return 1;
}

unsigned int sound_interface::play_3d_sound(sound_id_t snd, rational_t volume, rational_t pitch)
{
	//return(emitter->play_sound( snd, volume, pitch*ENTITY_TIME_DILATION(my_entity) ));
  return 1;
}

sg_entry *sound_interface::play_3d_sound_grp(const pstring &snd_grp, rational_t volume, rational_t pitch)
{/*
	sg_entry* entry = get_sound_group_entry( snd_grp );
  if(entry)
  {
    unsigned int id = emitter->play_sound( entry->name, entry->variance(entry->volume, entry->volume_var)*volume, entry->variance(entry->pitch, entry->pitch_var)*pitch*ENTITY_TIME_DILATION(my_entity), entry->min_dist, entry->max_dist );
    if(entry->is_voice)
    {
	    add_voice(id);
      entry->last_id_played = id;
    }
  }

  return(entry);
  */
  return NULL;
}

sg_entry *sound_interface::play_looping_3d_sound_grp(const pstring &snd_grp, rational_t volume, rational_t pitch)
{/*
	sg_entry* entry = get_sound_group_entry( snd_grp );
  if(entry)
  {
    sound_instance *loop_sound = emitter->create_sound(entry->name);
    if(loop_sound)
    {
      loop_sound->set_ranges( (entry->min_dist >= 0.0f) ? entry->min_dist : loop_sound->get_min_dist(), (entry->max_dist >= 0.0f) ? entry->max_dist : loop_sound->get_max_dist() );
      loop_sound->set_volume( entry->variance( entry->volume, entry->volume_var ) );
	    loop_sound->set_pitch( entry->variance( entry->pitch, entry->pitch_var ) );
      loop_sound->play(true);

      entry->last_id_played = loop_sound->get_id();
    }
    else
      entry->last_id_played = 0;
  }

  return(entry);*/
  return NULL;
}

void sound_interface::add_voice(unsigned int id)
{/*
  if(max_voices > 0)
  {
    while((int)voices.size() >= max_voices)
    {
      vector<unsigned int>::iterator i = voices.begin();
      while(i != voices.end())
      {
        unsigned int id = *i;
        i = voices.erase(i);

        if(emitter->kill_sound(id))
          break;
      }
    }

    voices.push_back(id);
  }*/
}

void sound_interface::frame_advance(time_value_t t)
{/*
  emitter->set_position( my_entity->get_abs_position() );

  if(max_voices > 0 && !voices.empty())
  {
    vector<unsigned int>::iterator i = voices.begin();
    while(i != voices.end())
    {
      if(emitter->sound_playing(*i))
        ++i;
      else
        i = voices.erase(i);
    }
  }*/
}



void sound_interface::read_enx_data( chunk_file& fs, stringx& lstr )
{/*
  serial_in( fs, &lstr );

  while(lstr != chunkend_label)
  {
    if(lstr == "max_voices")
    {
      serial_in( fs, &max_voices );
    }
    else if(lstr == "sound_groups:")
    {
      if(snd_grp == NULL)
        snd_grp = NEW shared_sound_group();

			serial_in( fs, snd_grp->sound_groups );

	    // build the sound group map.
      snd_grp->sound_group_map.resize(0);
	    for ( unsigned i = 0; i < snd_grp->sound_groups.size(); i++ )
		    snd_grp->sound_group_map.insert( pair<const pstring,sound_group*>( snd_grp->sound_groups[i].name, &snd_grp->sound_groups[i] ) );
    }

    serial_in( fs, &lstr );
	}

  if(max_voices > 0)
    voices.reserve(max_voices+1);*/
}

#endif