// fogmgr.cpp
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED. 
#include "global.h"

#include "fogmgr.h"
#include "hwrasterize.h"
#include "geomgr.h"
#include "osdevopts.h"
#include "projconst.h"

DEFINE_SINGLETON(fog_manager)


fog_manager::fog_manager()
{
  computed_fog_end_distance = 0;
  fog_of_war = 0;
  specified_fog_table_gamma = 1;
  if( os_developer_options::inst()->is_flagged( os_developer_options::FLAG_FOG_OVERRIDE ) )
  {
    set_fog_distance( 0, os_developer_options::inst()->get_int( os_developer_options::INT_FOG_DISTANCE ) );
    set_fog_color( color( os_developer_options::inst()->get_int( os_developer_options::INT_FOG_RED   )/255.0f,
                          os_developer_options::inst()->get_int( os_developer_options::INT_FOG_GREEN )/255.0f,
                          os_developer_options::inst()->get_int( os_developer_options::INT_FOG_BLUE  )/255.0f,
                          1.0f ) );
  }
  else
  {
    set_fog_distance( 0, 160.0f );
    set_fog_color( color( 0.0f, 0.0f, 0.0f, 1.0f ) );
  }
}

fog_manager::~fog_manager()
{
}

void fog_manager::set_fog_color(const color& _color)
{
  fog_color = _color;

  os_developer_options::inst()->set_int(os_developer_options::INT_FOG_RED  , _color.r*255);
  os_developer_options::inst()->set_int(os_developer_options::INT_FOG_GREEN, _color.g*255);
  os_developer_options::inst()->set_int(os_developer_options::INT_FOG_BLUE , _color.b*255);

  hw_rasta::inst()->set_fog_color( _color );
}

void fog_manager::set_fog_distance(rational_t start_dist, rational_t end_dist)
{
  specified_fog_start_distance = start_dist;
  specified_fog_end_distance = end_dist;
}

const rational_t MIN_FOG_END_DIST = 20;

rational_t g_debug_fog_override = 0.0;

void fog_manager::update_fog(bool forced)
{
  float last_computed_fog_end_distance = computed_fog_end_distance;
  if ( fog_of_war > 0 )
  {
    rational_t r = 1.0f - (0.5f * fog_of_war);
    computed_fog_start_distance = specified_fog_start_distance * r;
    if ( specified_fog_end_distance > MIN_FOG_END_DIST )
    {
      if ( specified_fog_end_distance*r < MIN_FOG_END_DIST )
        computed_fog_end_distance = MIN_FOG_END_DIST;
      else
        computed_fog_end_distance = specified_fog_end_distance * r;
    }
    else
      computed_fog_end_distance = specified_fog_end_distance;

#if 0//defined( TARGET_MKS )
    hw_rasta::inst()->set_fog_table_gamma( specified_fog_table_gamma * r );
#endif
  }
  else
  {
    computed_fog_start_distance = specified_fog_start_distance;
    computed_fog_end_distance = specified_fog_end_distance;
    //set_fog_table_gamma( specified_fog_table_gamma );
  }

  if( last_computed_fog_end_distance != computed_fog_end_distance || forced)
  {
    if( g_debug_fog_override )
      PROJ_FAR_PLANE_D = g_debug_fog_override;
    else
      PROJ_FAR_PLANE_D = computed_fog_end_distance;
    
	  geometry_manager::inst()->rebuild_view_frame();
	//  os_developer_options::inst()->set_int(  os_developer_options::INT_FOG_DISTANCE, _dist );
	  hw_rasta::inst()->set_fog_dist( computed_fog_start_distance, computed_fog_end_distance );
#ifdef NGL
/*	This call screws up the clip planes (dc 05/16/01)
    nglSetPerspectiveMatrix( proj_field_of_view_in_degrees(), 
                            nglGetScreenWidth()/2, 
                            nglGetScreenHeight()/2, 
                            PROJ_NEAR_PLANE_D, 
                            PROJ_FAR_PLANE_D );
*/
#endif
    //  restore();
	}
}


void fog_manager::set_fog_table_gamma( rational_t g )
{
  specified_fog_table_gamma = g;
#if defined( TARGET_MKS )
  if ( hw_rasta::inst()->get_fog_table_gamma() != g )
    hw_rasta::inst()->set_fog_table_gamma( g );
#endif
}


const rational_t FOW_VEL = 1.0f;
void fog_manager::set_fog_of_war( rational_t target, rational_t time_inc )
{
  rational_t d = target - fog_of_war;
  if ( d > 0 )
  {
    fog_of_war += FOW_VEL * time_inc;
    if ( fog_of_war > target )
      fog_of_war = target;
  }
  else
  {
    fog_of_war -= FOW_VEL * time_inc * 2;  // allow fog to recede faster
    if ( fog_of_war < target )
      fog_of_war = target;
  }
}

