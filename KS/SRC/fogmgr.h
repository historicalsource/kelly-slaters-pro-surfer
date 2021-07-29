#ifndef FOGMGR_H
#define FOGMGR_H

////////////////////////////////////////////////////////////////////////////////

// fogmgr.h
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED. 

////////////////////////////////////////////////////////////////////////////////

#include "singleton.h"
#include "color.h"

class fog_manager : public singleton
{
public:
  DECLARE_SINGLETON(fog_manager)

  // the fog manager always uses the current d3d device 
  fog_manager();
  ~fog_manager();

  color get_fog_color() const { return fog_color; }
  void set_fog_color(const color& _color);

  rational_t get_fog_start_distance() const { return specified_fog_start_distance; }
  rational_t get_fog_end_distance() const { return specified_fog_end_distance; }
  void set_fog_distance(rational_t start_dist, rational_t end_dist);
  void set_fog_table_gamma( rational_t g );

  void set_fog_of_war( rational_t target, rational_t time_inc );
  rational_t get_fog_of_war() const { return fog_of_war; }

  void update_fog(bool forced = false);    

protected:
  color fog_color;
  // fog distance user specifies:
  rational_t specified_fog_start_distance;
  rational_t specified_fog_end_distance;
  rational_t specified_fog_table_gamma;
  // fog distance we choose based on fog of war and possibly framerate
  rational_t computed_fog_start_distance;
  rational_t computed_fog_end_distance;
  rational_t fog_of_war;

};

#endif // FOGMGR_H