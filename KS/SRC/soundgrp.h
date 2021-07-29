#ifndef SOUNDGRP_H
#define SOUNDGRP_H

#if 0

#include "hwaudio.h"

struct sound_group
{
  vector<int> indices;
  vector<rational_t> probabilities;
  vector<sound_id_t> sound_ids;
  rational_t pitch_variance;
};

class sound_group_list_t : public map<stringx , sound_group *>
{
public:
  void play_sound_group( const stringx &sref, int idx=-1, int pri=0, const stringx *who = NULL);
  void play_sound_group( const stringx &sref, sound_emitter* emit);
  void play_sound_group_looping( const stringx &sref );
  void kill_sound_group_looping( const stringx &sref );
  bool sound_group_exists( const stringx& sref );
  int  get_num_sound_group_sounds( const stringx& sref );

  void clear();

  friend void serial_in(chunk_file& io, sound_group_list_t * sg);
};

#endif

#endif