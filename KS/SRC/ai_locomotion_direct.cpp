#include "global.h"

#include "ai_locomotion_direct.h"
#include "ai_interface.h"
#include "entity.h"
#include "collide.h"
#include "hard_attrib_interface.h"
#include "animation_interface.h"
#include "wds.h"
#include "profiler.h"


ai_locomotion_direct::ai_locomotion_direct(ai_interface *own)
  : ai_locomotion(own)
{
  type = LOCOMOTION_DIRECT;
}

ai_locomotion_direct::~ai_locomotion_direct()
{
}

ai_locomotion *ai_locomotion_direct::make_copy(ai_interface *own)
{
  ai_locomotion_direct *loco = NEW ai_locomotion_direct(own);
  loco->copy(this);
  return((ai_locomotion *)loco);
}

void ai_locomotion_direct::copy(ai_locomotion_direct *b)
{
  ai_locomotion::copy((ai_locomotion *)b);
}


bool ai_locomotion_direct::set_destination(const vector3d &pos, rational_t radius, bool fast, bool path_find, bool force_finish)
{
  if(ai_locomotion::set_destination(pos, radius, fast, path_find, force_finish))
  {
    local_dest = target_pos;

    has_been_stuck_for_how_long = 0.0f;
    previous_pos = ZEROVEC;

    repulsion_timer = 0.0f;
    repulsion_local_dest = local_dest;

    path_tries = 0;
    if(path_find)
      set_goto_path(0.5f, false);
    else
    {
      clear_path();
      use_path = false;
    }

    return(true);
  }
  else
    return(false);
}


bool ai_locomotion_direct::process_movement(time_value_t t)
{
  bool dest_reached = true;

  if(in_service)
  {
    dest_reached = false;

    rational_t temp_target_range = tgtrange;
    const vector3d& herenow = get_my_entity()->get_abs_position();

    local_dest = target_pos;
    region_node* dest_region = NULL;

    if ( use_path )
    {
      if ( !get_current_path()->get_next_way_point( herenow, last_pos, temp_target_range, &local_dest, &dest_region, xz_movement_only() ) )
      {
        clear_path();
        use_path = false;
        local_dest = target_pos;
      }

      start_pos = herenow;
    }

    bool crossed_pt = crossed_point(target_pos, herenow, temp_target_range, !use_path || xz_movement_only());

    if(crossed_pt)
      dest_reached = true;
    else
    {
      last_pos = herenow;

      // repulsion
/*
      if ( repulsion_timer > 0.0f )
      {
        repulsion_timer -= t;
        local_dest = repulsion_local_dest;
      }

      if ( get_my_entity()->is_active() && !(path_forced || corpus->forced_path()) && repulsion_timer <= 0.0f )
      {
        if ( process_repulsion(delta_t) )
          return false;
      }
*/
      // GUESSTIMATE FOR 15deg EXPRESSED IN RADS, CHIAROSCURO ON PRESSBOARD

      vector3d pt_to_face = local_dest;

      rational_t target_angle = get_xz_rotation_to_point( pt_to_face );

      if(__fabs( target_angle ) > DEG_TO_RAD(5.0f))
      {
        rational_t turn_factor = 1.0f + (1.0f / ((target_pos - herenow).xz_length2() - (temp_target_range*temp_target_range)));

        rational_t pin = ((get_my_entity()->has_hard_attrib_ifc() ? get_my_entity()->hard_attrib_ifc()->get_max_turn_rate() : 5.0f) * turn_factor) * t;

        if(!running_speed )
          pin *= 0.75f;

        if( target_angle > pin )
          target_angle = pin;
        if( target_angle < -pin )
          target_angle = -pin;

        apply_rotation( target_angle );
      }

      // send character toward local_dest at appropriate speed
      vector3d delta = local_dest - herenow;

      // Cancel the y component of the movement on active characters
      // This is beacuse they are now allowed to climb walls due to sliding up them (Y axis movement).
      // Normally an animation moves them in the XZ only. This will make their translation act
      // like a normal animation (JDB 9/11/00)
      if(xz_movement_only())
        delta.y = 0.0f;

      rational_t d = delta.length2();

      if ( d > SMALL_DIST )
      {
        rational_t md;
        if (get_my_entity()->has_hard_attrib_ifc())
          md = t * (running_speed ? get_my_entity()->hard_attrib_ifc()->get_ai_run_speed() : get_my_entity()->hard_attrib_ifc()->get_ai_walk_speed());
        else
          md = (running_speed? 6.0f : 2.0f) * t;

        if ( (md*md) > d )
          md = __fsqrt(d);

        delta *= md / d;

        vector3d thepos = herenow+delta;
        get_my_entity()->set_rel_position_no_children(thepos);

        get_my_entity()->set_frame_delta_trans(delta, t);
        get_my_entity()->update_abs_po_reverse();

        conditional_compute_sector(get_my_entity()->get_primary_region(), last_pos, get_my_entity()->get_abs_position());
      }
    }
  }

  return(dest_reached); // no pop
}

void ai_locomotion_direct::going_out_of_service()
{
  get_my_entity()->kill_anim( AI_ANIM_LOCOMOTION );

  ai_locomotion::going_out_of_service();
}

//--------------------------------------------------------------
void ai_locomotion_direct::going_into_service()
{
  ai_locomotion::going_into_service();

  // START NEW ANIM ON THE WAY IN
  const char* anim_to_walk = "WALK";
  const char* anim_to_run = "RUN";

  get_ai_interface()->play_animation((running_speed ? anim_to_run : anim_to_walk), AI_ANIM_LOCOMOTION, (ANIM_RELATIVE_TO_START | ANIM_LOOPING | ANIM_NONCOSMETIC | ANIM_TWEEN));

  playing_face_anim = false;
}

void ai_locomotion_direct::handle_chunk(chunk_file& fs, stringx &label)
{
  ai_locomotion::handle_chunk(fs, label);
}




