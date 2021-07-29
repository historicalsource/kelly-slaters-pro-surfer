#ifndef RSG_H
#define RSG_H

#include "pstring.h"
#include "random.h"

class sg_entry
{
public:
	pstring name;
	rational_t probability;

  rational_t pitch;
  rational_t pitch_var;

  rational_t volume;
  rational_t volume_var;

  rational_t min_dist;
  rational_t max_dist;

  bool is_voice;
  unsigned int last_id_played;

	bool flagged; // temporary, used by sound_group::get_sound only

  sg_entry()
  {
    probability = 1.0f;
    pitch = 1.0f;
    volume = 1.0f;
    volume_var = 0.0f;
    pitch_var = 0.0f;
    min_dist = -1.0f;
    max_dist = -1.0f;
    is_voice = false;
    last_id_played = 0;
  }

  void copy(const sg_entry &b)
  {
    name = b.name;
    probability = b.probability;
    pitch = b.pitch;
    pitch_var = b.pitch_var;
    volume = b.volume;
    volume_var = b.volume_var;

    min_dist = b.min_dist;
    max_dist = b.max_dist;

    is_voice = b.is_voice;
  }

  sg_entry(const sg_entry& b)
  {
    copy(b);
  }

  sg_entry& operator=(const sg_entry &b) 
  {
		copy( b );
    return *this;
  }

  inline static rational_t variance(rational_t num, rational_t var = 0.0f)
  {
    return(num + (PLUS_MINUS_ONE * var));
  }
};

class sound_group
{
public:
	pstring name;
	int maxhistory;
//	int history[8];
//  int history_index;
	vector<sg_entry> entries;
	vector<sg_entry*> pool;

	// Create from a text file.
	sound_group();
	sound_group(const sound_group &b)
  {
    copy(b);
  } 

  void copy(const sound_group &b);

  void clear_history();
  void init_pool();

  sound_group& operator=(const sound_group &b) 
  {
		copy( b );
    return *this;
  }

	// Picks the next random string and cycles the history.
	sg_entry* get_next();

	friend void serial_in( chunk_file& fs, sound_group* group );
};

// Read a single group from a file, including the soundgrp tag.
void serial_in( chunk_file& fs, sound_group* group );

// Read a whole list from a file.  Assumes that the sound_groups: chunk has already 
// been read, and scans for a chunkend.
void serial_in( chunk_file& fs, vector<sound_group>& groups );

#endif
