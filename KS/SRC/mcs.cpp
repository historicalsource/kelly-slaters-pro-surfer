////////////////////////////////////////////////////////////////////////////////
/*
  mcs.cpp

  home to various motion_control_systems

*/
////////////////////////////////////////////////////////////////////////////////
#include "global.h"

#include "project.h"  // tells which project we're building

#include "mcs.h"
//!#include "character.h"
#include "hwmath.h"
//!#include "actor.h"
//!#include "character.h"
#include "wds.h"
#include "terrain.h"
//!#include "ladder.h"
//!#include "attrib.h"
//!#include "limb.h"
#include "controller.h"
#include "inputmgr.h"
#include "commands.h"

extern vector3d up_vector;

////////////////////////////////////////////////////////////////////////////////
//   motion_control_system
////////////////////////////////////////////////////////////////////////////////

motion_control_system::motion_control_system() : motion_object()
{
}


motion_control_system::~motion_control_system()
{
}


////////////////////////////////////////////////////////////////////////////////
//   theta_and_psi_mcs
////////////////////////////////////////////////////////////////////////////////
theta_and_psi_mcs::theta_and_psi_mcs(entity* _ent, rational_t _theta, rational_t _psi ) :
  motion_control_system(),
  theta( _theta ),
  psi(_psi)
{
  ent = _ent;
  d_theta_for_next_frame = 0.0f;
  d_psi_for_next_frame = 0.0f;
}

void theta_and_psi_mcs::frame_advance(time_value_t time_inc)
{
  if (input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, PLAYER_FRONT_CROUCH) < 2*AXIS_MAX/3 )
  {
    theta += d_theta_for_next_frame;
    psi += d_psi_for_next_frame;

    if (psi < -PI*0.5f) psi = -PI*0.5f;
    else if (psi > PI*0.5f) psi = PI*0.5f;

    // transform using rotate_around_x and rotate_around_y
    po rotate_around_y;
    rotate_around_y.set_rot( up_vector, theta );
    po rotate_around_x;
    rotate_around_x.set_rot( vector3d(1,0,0), psi );
    po rotate_around = rotate_around_x * rotate_around_y;
    ent->set_rel_orientation( rotate_around.get_orientation() );
  }
}


////////////////////////////////////////////////////////////////////////////////
//   dolly_and_strafe_mcs
////////////////////////////////////////////////////////////////////////////////
void dolly_and_strafe_mcs::frame_advance(time_value_t time_inc)
{
  do_dolly( dolly );
  do_strafe( strafe );
  do_lift( lift );
  dolly = 0;
  strafe = 0;
  lift = 0;
}

void dolly_and_strafe_mcs::do_dolly( rational_t distance )
{
  // get direction facing
  vector3d facing = ent->get_abs_po().get_facing();
  // multiply distance by it
  facing *= distance;
  // add it to position
  ent->set_rel_position( ent->get_abs_position() + facing );
}

void dolly_and_strafe_mcs::do_strafe( rational_t distance )
{
  // the x vector transformed by po should be strafe direction
  vector3d x(1,0,0);

//  vector3d strafe_vec = ent->get_po().non_affine_slow_xform( x );
  vector3d strafe_vec = ent->get_abs_po().fast_8byte_non_affine_xform( x );

  strafe_vec *= distance;
  ent->set_rel_position( ent->get_abs_position() + strafe_vec );
}

void dolly_and_strafe_mcs::do_lift( rational_t distance )
{
  // doing lift in world coordinates
  vector3d lift(0,1,0);
  lift *= distance;
  ent->set_rel_position( ent->get_abs_position() + lift );
}





////////////////////////////////////////////////////////////////////////////////
//   character_ladder_mcs
////////////////////////////////////////////////////////////////////////////////

/*!
character_ladder_mcs::character_ladder_mcs( character* _ent )
{
  owner = _ent;
  ent = _ent;

  cur_ladder = NULL;
  going_up = false;

  cur_anim = NULL;

  last_phase = phase = PHASE_NONE;

  next_anim_start_time = 0;
}


character_ladder_mcs::~character_ladder_mcs()
{
}

void character_ladder_mcs::set_phase(phase_t p)
{
  last_phase = phase;
  phase = p;
}


#define MAX_TRANS_VEL    4.0f
#define MAX_TRANS_ANGVEL 3.14f  // should this be PI instead?

void character_ladder_mcs::frame_advance( time_value_t time_inc )
{
  assert( cur_ladder );

  next_anim_start_time = 0;

  bool process_next_phase = false;

  do
    {
    process_next_phase = false;

    switch ( phase )
      {
      // lerp into mount position and orientation
      case MOUNT_TRANSITION:
        process_next_phase = mount_transition(time_inc);
        break;

        // play mount animation
      case MOUNT:
        process_next_phase = mount(time_inc);
        break;

        // wait for mount animation to finish
      case WAIT_MOUNT:
        process_next_phase = wait_mount(time_inc);
        break;

        // play upward climb animation
      case CLIMB_UP:
        process_next_phase = climb_up(time_inc);
        break;

        // play downward climb animation
      case CLIMB_DOWN:
        process_next_phase = climb_down(time_inc);
        break;

        // wait for climb animation to finish
      case ADVANCE_CLIMB:
        process_next_phase = advance_climb(time_inc);
        break;

        // play dismount animation
      case DISMOUNT:
        process_next_phase = dismount(time_inc);
        break;

        // wait for dismount animation to end, then we're done
      case WAIT_DISMOUNT:
        process_next_phase = wait_dismount(time_inc);
        break;
      }
    } while ( process_next_phase );

  owner->set_velocity( ZEROVEC );
  owner->set_angular_velocity( ZEROVEC );

  // Avoid playing hit animation when finished climbing...
  if(owner->is_hero())
    owner->set_hit_applied(0);
}


void character_ladder_mcs::controller_disengage()
{
  cur_ladder = NULL;
  set_active( false );

  // re-enable normal operation
  owner->set_velocity( ZEROVEC );
  owner->set_angular_velocity( ZEROVEC );

  owner->show_dropshadow();

  // re-enable player control, if necessary
  owner->get_controller()->set_active( controller_active_state );

  owner->set_physical( owner_physical_state );
  owner->set_collisions_active( owner_collision_state );
}


void character_ladder_mcs::controller_engage( ladder* lad )
{
  cur_ladder = lad;

  assert(cur_ladder);

  owner_physical_state = owner->is_physical();
  owner_collision_state = owner->are_collisions_active();

  controller_active_state = owner->get_controller()->is_active();
  owner->get_controller()->set_active( false );

  owner->set_physical(false);
  owner->set_collisions_active(false);

  vector3d top_mount_pos = cur_ladder->get_top_mount_position();
  vector3d bottom_mount_pos = cur_ladder->get_bottom_mount_position();

  if ( cur_ladder->get_ladder_type() == ladder::ZIP_LINE || (top_mount_pos - owner->get_position()).length2() <= (bottom_mount_pos - owner->get_position()).length2() )
  {
    mount_pos = top_mount_pos;

    end_pos = cur_ladder->get_bottom_dismount_position();

    vector3d ladder_facing = (cur_ladder->get_ladder_type() == ladder::HORIZONTAL_LADDER) ? cur_ladder->get_po().get_facing() : -cur_ladder->get_po().get_facing();
    mount_theta = safe_atan2( -ladder_facing.x, ladder_facing.z );

    going_up = false;
  }
  else
  {
    mount_pos = bottom_mount_pos;

    end_pos = cur_ladder->get_top_dismount_position();

    vector3d ladder_facing = -cur_ladder->get_po().get_facing();
    mount_theta = safe_atan2( -ladder_facing.x, ladder_facing.z );

    going_up = true;
  }

  set_active( true );

  set_phase(MOUNT_TRANSITION);

  // set up transition variables
  owner_pos = owner->get_position();
  vector3d owner_facing = owner->get_po().get_facing();
  owner_theta = safe_atan2( -owner_facing.x, owner_facing.z );

  // player control disabled until transition is complete
  owner->get_controller()->set_active( false );

  // actor velocity, etc. disabled (but not VSIM) until end of climbing
  owner->set_velocity( ZEROVEC );
  owner->set_angular_velocity( ZEROVEC );
}



bool character_ladder_mcs::climb_up( time_value_t time_inc )
{
  switch ( cur_ladder->get_ladder_type() )
  {
    case ladder::VERTICAL_LADDER:
      return v_ladder_climb_up( time_inc );
      break;

    case ladder::HORIZONTAL_LADDER:
      return h_ladder_climb_up( time_inc );
      break;

    case ladder::ZIP_LINE:
      return zip_line_climb_up( time_inc );
      break;

    default:
      error( "Bad ladder type!" );
      break;
  }
  return false;
}

bool character_ladder_mcs::climb_down( time_value_t time_inc )
{
  switch(cur_ladder->get_ladder_type())
  {
    case ladder::VERTICAL_LADDER:
      return(v_ladder_climb_down( time_inc ));
      break;

    case ladder::HORIZONTAL_LADDER:
      return(h_ladder_climb_down( time_inc ));
      break;

    case ladder::ZIP_LINE:
      return(zip_line_climb_down( time_inc ));
      break;

    default:
      error(stringx("Bad ladder type!"));
      break;
  }

  return(false);
}

bool character_ladder_mcs::advance_climb( time_value_t time_inc )
{
  switch(cur_ladder->get_ladder_type())
  {
    case ladder::VERTICAL_LADDER:
      return(v_ladder_advance_climb( time_inc ));
      break;

    case ladder::HORIZONTAL_LADDER:
      return(h_ladder_advance_climb( time_inc ));
      break;

    case ladder::ZIP_LINE:
      return(zip_line_advance_climb( time_inc ));
      break;

    default:
      error(stringx("Bad ladder type!"));
      break;
  }

  return(false);
}

bool character_ladder_mcs::mount( time_value_t time_inc )
{
  switch(cur_ladder->get_ladder_type())
  {
    case ladder::VERTICAL_LADDER:
      return(v_ladder_mount( time_inc ));
      break;

    case ladder::HORIZONTAL_LADDER:
      return(h_ladder_mount( time_inc ));
      break;

    case ladder::ZIP_LINE:
      return(zip_line_mount( time_inc ));
      break;

    default:
      error(stringx("Bad ladder type!"));
      break;
  }

  return(false);
}

bool character_ladder_mcs::wait_mount( time_value_t time_inc )
{
  switch(cur_ladder->get_ladder_type())
  {
    case ladder::VERTICAL_LADDER:
      return(v_ladder_wait_mount( time_inc ));
      break;

    case ladder::HORIZONTAL_LADDER:
      return(h_ladder_wait_mount( time_inc ));
      break;

    case ladder::ZIP_LINE:
      return(zip_line_wait_mount( time_inc ));
      break;

    default:
      error(stringx("Bad ladder type!"));
      break;
  }

  return(false);
}

bool character_ladder_mcs::dismount( time_value_t time_inc )
{
  switch(cur_ladder->get_ladder_type())
  {
    case ladder::VERTICAL_LADDER:
      return(v_ladder_dismount( time_inc ));
      break;

    case ladder::HORIZONTAL_LADDER:
      return(h_ladder_dismount( time_inc ));
      break;

    case ladder::ZIP_LINE:
      return(zip_line_dismount( time_inc ));
      break;

    default:
      error(stringx("Bad ladder type!"));
      break;
  }

  return(false);
}

bool character_ladder_mcs::wait_dismount( time_value_t time_inc )
{
  switch(cur_ladder->get_ladder_type())
  {
    case ladder::VERTICAL_LADDER:
      return(v_ladder_wait_dismount( time_inc ));
      break;

    case ladder::HORIZONTAL_LADDER:
      return(h_ladder_wait_dismount( time_inc ));
      break;

    case ladder::ZIP_LINE:
      return(zip_line_wait_dismount( time_inc ));
      break;

    default:
      error(stringx("Bad ladder type!"));
      break;
  }

  return(false);
}

bool character_ladder_mcs::mount_transition( time_value_t time_inc )
{
  switch(cur_ladder->get_ladder_type())
  {
    case ladder::VERTICAL_LADDER:
      return(v_ladder_mount_transition( time_inc ));
      break;

    case ladder::HORIZONTAL_LADDER:
      return(h_ladder_mount_transition( time_inc ));
      break;

    case ladder::ZIP_LINE:
      return(zip_line_mount_transition( time_inc ));
      break;

    default:
      error(stringx("Bad ladder type!"));
      break;
  }

  return(false);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// vertical ladder
////////////////////////////////////////////////////////////////////////////////////////////////////
bool character_ladder_mcs::v_ladder_climb_up( time_value_t time_inc )
{
  if ( next_anim_start_time < 0 )
  {
    // force completion of absolute (character) motion in current anim
    owner->get_anim()->finish_absolute_motion();
    next_anim_start_time = 0;
  }

//  owner->play_anim( owner->get_climbup_anim_filename(), next_anim_start_time );

  kill_cur_anim();

  if ( using_right_arm )
    cur_anim = owner->play_anim( cur_ladder->get_climb_up_r_anim(), next_anim_start_time, 0 );
  else
    cur_anim = owner->play_anim( cur_ladder->get_climb_up_l_anim(), next_anim_start_time, 0 );

  using_right_arm = !using_right_arm;
  set_phase( ADVANCE_CLIMB );

  return false;
}

bool character_ladder_mcs::v_ladder_climb_down( time_value_t time_inc )
{
  if ( next_anim_start_time < 0 )
  {
    // force completion of absolute (character) motion in current anim
    owner->get_anim()->finish_absolute_motion();
    next_anim_start_time = 0;
  }

//  owner->play_anim( owner->get_climbdown_anim_filename(), next_anim_start_time );

  kill_cur_anim();

  if(using_right_arm)
    cur_anim = owner->play_anim( cur_ladder->get_climb_down_r_anim(), next_anim_start_time, ANIM_LOOPING );
  else
    cur_anim = owner->play_anim( cur_ladder->get_climb_down_l_anim(), next_anim_start_time, ANIM_LOOPING );

  owner->set_physical(true);

  using_right_arm = !using_right_arm;
  set_phase(ADVANCE_CLIMB);

  return(false);
}

bool character_ladder_mcs::v_ladder_advance_climb( time_value_t time_inc )
{
  assert( owner->get_anim() );

  switch(last_phase)
  {
    case CLIMB_UP:
    {
      float dist = (owner->get_position() - start_pos).length();
      // note that this formula assumes forward animation (no ANIM_REVERSE flag)
      entity_anim_tree* a = owner->get_anim_tree( ANIM_PRIMARY );
      if ( a == NULL
        || a->is_finished()
        || a->get_time()+time_inc > a->get_duration()
        )
      {
        // set next animation start time so that we have a seamless transition
        if ( a!=NULL && !a->is_finished() )
          next_anim_start_time = -a->get_time_remaining();
        else
          next_anim_start_time = 0.0f;

        if ( dist>=travel_dist || travel_dist-dist<=0.1f )
          set_phase( DISMOUNT );
        else
          set_phase( last_phase );
        return true;
      }
    }
    break;

    case CLIMB_DOWN:
    {
      // check if at top or bottom, otherwise keeop climbing

      // give gravity a boost to look better
      owner->apply_force_increment(force_dir * (GRAVITY * owner->get_mass() * time_inc)*cur_ladder->get_gravity_booster(), entity::CONTINUOUS);

      next_anim_start_time = 0.0f;

      float dist = (owner->get_position() - start_pos).length();

      if(dist >= travel_dist || (travel_dist-dist) <= 0.1f)
      {
        set_phase(DISMOUNT);
        return true;
      }
    }
    break;

    default:
      assert(0);
      break;
  }

  return false;
}

bool character_ladder_mcs::v_ladder_mount( time_value_t time_inc )
{
  owner->hide_dropshadow();

  kill_cur_anim();

  if ( going_up )
  {
    cur_anim = owner->play_anim( cur_ladder->get_mount_bottom_anim(), 0.0f, 0 );
  }
  else
  {
    cur_anim = owner->play_anim( cur_ladder->get_mount_top_anim(), 0.0f, 0 );
  }

  using_right_arm = true;
  set_phase(WAIT_MOUNT);

  return(false);
}

bool character_ladder_mcs::v_ladder_wait_mount( time_value_t time_inc )
{
  entity_anim_tree* a = owner->get_anim_tree( ANIM_PRIMARY );
  assert( a != NULL );
  if ( a->is_finished() )
  {
    if ( going_up )
      set_phase( CLIMB_UP );
    else
      set_phase( CLIMB_DOWN );
    start_pos = owner->get_position();
    force_dir = end_pos - start_pos;
    travel_dist = force_dir.length();
    assert(travel_dist);
    force_dir /= travel_dist;
    return true;
  }
  return false;
}

bool character_ladder_mcs::v_ladder_dismount( time_value_t time_inc )
{
  if ( next_anim_start_time < 0 )
  {
    // force completion of absolute (character) motion in current anim
    owner->get_anim()->finish_absolute_motion();
    next_anim_start_time = 0;
  }

  // make sure we're in the right place
  owner->set_rel_position(end_pos);

  kill_cur_anim();

  if ( going_up )
  {
    if(using_right_arm)
      cur_anim = owner->play_anim( cur_ladder->get_dismount_top_r_anim(), next_anim_start_time, 0 );
    else
      cur_anim = owner->play_anim( cur_ladder->get_dismount_top_l_anim(), next_anim_start_time, 0 );
  }
  else
  {
    if(using_right_arm)
      cur_anim = owner->play_anim( cur_ladder->get_dismount_bottom_r_anim(), next_anim_start_time, 0 );
    else
      cur_anim = owner->play_anim( cur_ladder->get_dismount_bottom_l_anim(), next_anim_start_time, 0 );
  }

  set_phase(WAIT_DISMOUNT);

  return(false);
}

bool character_ladder_mcs::v_ladder_wait_dismount( time_value_t time_inc )
{
  entity_anim_tree* a = owner->get_anim_tree( ANIM_PRIMARY );
  if ( a==NULL || a->is_finished() )
  {
    kill_cur_anim();
    controller_disengage();
  }
  return false;
}

bool character_ladder_mcs::v_ladder_mount_transition( time_value_t time_inc )
{
  // update position
  bool reached_pos = false;
  vector3d deltap = mount_pos - owner_pos;
  rational_t d = deltap.length();
  if ( d > MAX_TRANS_VEL*time_inc )
    deltap = deltap * (MAX_TRANS_VEL*time_inc / d);
  else
    reached_pos = true;
  owner_pos += deltap;

  // update facing
  bool reached_theta = false;
  rational_t diff = (mount_theta - owner_theta);
  if ( diff > PI )
    diff -= 2 * PI;
  else if ( diff < -PI )
    diff += 2 * PI;
  rational_t max_diff = MAX_TRANS_ANGVEL * time_inc;
  if ( diff > max_diff )
    diff = max_diff;
  else if ( diff < -max_diff )
    diff = -max_diff;
  else
    reached_theta = true;
  owner_theta += diff;

  // move character
  po climber_po = po_identity_matrix;
  climber_po.set_rotate_y( owner_theta );
  climber_po.set_rel_position( owner_pos );
  owner->set_rel_po(climber_po);
  owner->set_velocity(ZEROVEC);
  owner->set_angular_velocity(ZEROVEC);

  if ( reached_pos && reached_theta )
    {
    set_phase(MOUNT);
    return(true);
    }

  return(false);
}
















////////////////////////////////////////////////////////////////////////////////////////////////////
// horizontal ladder
////////////////////////////////////////////////////////////////////////////////////////////////////
bool character_ladder_mcs::h_ladder_climb_up( time_value_t time_inc )
{
  assert(0);

  return(false);
}

bool character_ladder_mcs::h_ladder_climb_down( time_value_t time_inc )
{
  if ( next_anim_start_time < 0 )
  {
    // force completion of absolute (character) motion in current anim
    owner->get_anim()->finish_absolute_motion();
    next_anim_start_time = 0;
  }

//  owner->play_anim( owner->get_climbdown_anim_filename(), next_anim_start_time );

  kill_cur_anim();

  if(using_right_arm)
    cur_anim = owner->play_anim( cur_ladder->get_climb_down_r_anim(), next_anim_start_time, ANIM_LOOPING );
  else
    cur_anim = owner->play_anim( cur_ladder->get_climb_down_l_anim(), next_anim_start_time, ANIM_LOOPING );

  using_right_arm = !using_right_arm;
  set_phase(ADVANCE_CLIMB);

  return false;
}

bool character_ladder_mcs::h_ladder_advance_climb( time_value_t time_inc )
{
  assert( owner->get_anim() );

  switch(last_phase)
  {
    case CLIMB_DOWN:
    {
      float dist = (owner->get_position() - start_pos).length();
      // note that this formula assumes forward animation (no ANIM_REVERSE flag)
      entity_anim_tree* a = owner->get_anim_tree( ANIM_PRIMARY );
      if ( a == NULL
        || a->is_finished()
        || a->get_time()+time_inc > a->get_duration()
        )
      {
        // set next animation start time so that we have a seamless transition
        if ( a!=NULL && !a->is_finished() )
          next_anim_start_time = -a->get_time_remaining();
        else
          next_anim_start_time = 0.0f;

        if ( dist>=travel_dist || travel_dist-dist<=0.1f )
          set_phase( DISMOUNT );
        else
          set_phase( last_phase );

        return true;
      }
    }
    break;

    default:
      assert(0);
      break;
  }

  return false;
}

bool character_ladder_mcs::h_ladder_mount( time_value_t time_inc )
{
//  owner->hide_dropshadow();

  kill_cur_anim();

  cur_anim = owner->play_anim( cur_ladder->get_mount_top_anim(), 0.0f, 0 );

  using_right_arm = true;
  set_phase(WAIT_MOUNT);

  return false;
}

bool character_ladder_mcs::h_ladder_wait_mount( time_value_t time_inc )
{
  entity_anim_tree* a = owner->get_anim_tree( ANIM_PRIMARY );
  assert( a != NULL );
  if ( a->is_finished() )
  {
    set_phase( CLIMB_DOWN );
    start_pos = owner->get_position();
    force_dir = end_pos - start_pos;
    travel_dist = force_dir.length();
    assert(travel_dist);
    force_dir /= travel_dist;
    return true;
  }
  return false;
}

bool character_ladder_mcs::h_ladder_dismount( time_value_t time_inc )
{
  if ( next_anim_start_time < 0 )
  {
    // force completion of absolute (character) motion in current anim
    owner->get_anim()->finish_absolute_motion();
    next_anim_start_time = 0;
  }

  // make sure we're in the right place
  owner->set_rel_position(end_pos);

  kill_cur_anim();

  if(using_right_arm)
    cur_anim = owner->play_anim( cur_ladder->get_dismount_bottom_r_anim(), next_anim_start_time, 0 );
  else
    cur_anim = owner->play_anim( cur_ladder->get_dismount_bottom_l_anim(), next_anim_start_time, 0 );

  set_phase(WAIT_DISMOUNT);

  return false;
}

bool character_ladder_mcs::h_ladder_wait_dismount( time_value_t time_inc )
{
  entity_anim_tree* a = owner->get_anim_tree( ANIM_PRIMARY );
  if ( a==NULL || a->is_finished() )
  {
    kill_cur_anim();
    controller_disengage();
  }
  return false;
}

bool character_ladder_mcs::h_ladder_mount_transition( time_value_t time_inc )
{
  // update position
  bool reached_pos = false;
  vector3d deltap = mount_pos - owner_pos;
  rational_t d = deltap.length();
  if ( d > MAX_TRANS_VEL*time_inc )
    deltap = deltap * (MAX_TRANS_VEL*time_inc / d);
  else
    reached_pos = true;
  owner_pos += deltap;

  // update facing
  bool reached_theta = false;
  rational_t diff = (mount_theta - owner_theta);
  if ( diff > PI )
    diff -= 2 * PI;
  else if ( diff < -PI )
    diff += 2 * PI;
  rational_t max_diff = MAX_TRANS_ANGVEL * time_inc;
  if ( diff > max_diff )
    diff = max_diff;
  else if ( diff < -max_diff )
    diff = -max_diff;
  else
    reached_theta = true;
  owner_theta += diff;

  // move character
  po climber_po = po_identity_matrix;
  climber_po.set_rotate_y( owner_theta );
  climber_po.set_rel_position( owner_pos );
  owner->set_rel_po(climber_po);
  owner->set_velocity(ZEROVEC);
  owner->set_angular_velocity(ZEROVEC);

  if ( reached_pos && reached_theta )
  {
    set_phase(MOUNT);
    return true;
  }

  return false;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// zip line
////////////////////////////////////////////////////////////////////////////////////////////////////
bool character_ladder_mcs::zip_line_climb_up( time_value_t time_inc )
{
  assert(0);
  return false;
}

bool character_ladder_mcs::zip_line_climb_down( time_value_t time_inc )
{
  if ( next_anim_start_time < 0 )
  {
    // force completion of absolute (character) motion in current anim
    owner->get_anim()->finish_absolute_motion();
    next_anim_start_time = 0;
  }

//  owner->play_anim( owner->get_climbdown_anim_filename(), next_anim_start_time );

  kill_cur_anim();

  cur_anim = owner->play_anim( cur_ladder->get_climb_down_r_anim(), next_anim_start_time, ANIM_LOOPING );

  owner->set_physical(true);
  owner->set_gravity(false);

  using_right_arm = !using_right_arm;
  set_phase(ADVANCE_CLIMB);

  return false;
}

bool character_ladder_mcs::zip_line_advance_climb( time_value_t time_inc )
{
  assert( owner->get_anim() );

  switch(last_phase)
  {
    case CLIMB_DOWN:
    {
      // check if at top or bottom, otherwise keeop climbing

      owner->apply_force_increment(force_dir * (GRAVITY * owner->get_mass() * time_inc)*cur_ladder->get_gravity_booster(), entity::CONTINUOUS);

      next_anim_start_time = 0.0f;

      float dist = (owner->get_position() - start_pos).length();

      if(dist >= travel_dist)
      {
        set_phase(DISMOUNT);
        return true;
      }
    }
    break;

    default:
      assert(0);
      break;
  }

  return false;
}

bool character_ladder_mcs::zip_line_mount( time_value_t time_inc )
{
//  owner->hide_dropshadow();

  kill_cur_anim();

  if ( going_up )
  {
  assert(0);
  }
  else
  {
    cur_anim = owner->play_anim( cur_ladder->get_mount_top_anim(), 0.0f, 0 );
  }

  using_right_arm = true;
  set_phase(WAIT_MOUNT);

  return(false);
}

bool character_ladder_mcs::zip_line_wait_mount( time_value_t time_inc )
{
  entity_anim_tree* a = owner->get_anim_tree( ANIM_PRIMARY );
  assert( a != NULL );
  if ( a->is_finished() )
  {
    set_phase( CLIMB_DOWN );
    start_pos = owner->get_position();
    force_dir = end_pos - start_pos;
    travel_dist = force_dir.length();
    assert(travel_dist);
    force_dir /= travel_dist;
    return true;
  }
  return false;
}

bool character_ladder_mcs::zip_line_dismount( time_value_t time_inc )
{
  if ( next_anim_start_time < 0 )
  {
    // force completion of absolute (character) motion in current anim
    owner->get_anim()->finish_absolute_motion();
    next_anim_start_time = 0;
  }

  // make sure we're in the right place
  owner->set_rel_position(end_pos);

  kill_cur_anim();

  if ( going_up )
  {
    assert(0);
  }
  else
  {
    if ( using_right_arm )
      cur_anim = owner->play_anim( cur_ladder->get_dismount_bottom_r_anim(), next_anim_start_time, 0 );
    else
      cur_anim = owner->play_anim( cur_ladder->get_dismount_bottom_l_anim(), next_anim_start_time, 0 );
  }

  set_phase(WAIT_DISMOUNT);

  return(false);
}

bool character_ladder_mcs::zip_line_wait_dismount( time_value_t time_inc )
{
  entity_anim_tree* a = owner->get_anim_tree( ANIM_PRIMARY );
  if ( a==NULL || a->is_finished() )
  {
    kill_cur_anim();
    controller_disengage();
  }
  return false;
}

bool character_ladder_mcs::zip_line_mount_transition( time_value_t time_inc )
{
  // update position
  bool reached_pos = false;
  vector3d deltap = mount_pos - owner_pos;
  rational_t d = deltap.length();
  if ( d > MAX_TRANS_VEL*time_inc )
    deltap = deltap * (MAX_TRANS_VEL*time_inc / d);
  else
    reached_pos = true;
  owner_pos += deltap;

  // update facing
  bool reached_theta = false;
  rational_t diff = (mount_theta - owner_theta);
  if ( diff > PI )
    diff -= 2 * PI;
  else if ( diff < -PI )
    diff += 2 * PI;
  rational_t max_diff = MAX_TRANS_ANGVEL * time_inc;
  if ( diff > max_diff )
    diff = max_diff;
  else if ( diff < -max_diff )
    diff = -max_diff;
  else
    reached_theta = true;
  owner_theta += diff;

  // move character
  po climber_po = po_identity_matrix;
  climber_po.set_rotate_y( owner_theta );
  climber_po.set_rel_position( owner_pos );
  owner->set_rel_po(climber_po);
  owner->set_velocity(ZEROVEC);
  owner->set_angular_velocity(ZEROVEC);

  if ( reached_pos && reached_theta )
  {
    set_phase(MOUNT);
    return true;
  }

  return false;
}

!*/

////////////////////////////////////////////////////////////////////////////////////////////////////
// auto-aim arm control (for one-handed weapons only)
////////////////////////////////////////////////////////////////////////////////////////////////////
/*!
auto_aim_arm_mcs::auto_aim_arm_mcs(entity* _ent) : motion_control_system()
{
  assert(_ent->get_flavor() == ENTITY_CHARACTER);
  ent = _ent;
  aim_type = ARM_ONLY;
}

void auto_aim_arm_mcs::set_active( bool yorn )
{
  if ( aim_type == PROCEDURAL_ARM_ONLY && is_active() && !yorn )
  {
    int i;
    for ( i=0; i<MAX_ANIM_SLOTS; ++i )
    {
      entity_anim_tree* a = ent->get_anim_tree( i );
      if ( a != NULL )
        a->attach();
    }
  }
  motion_control_system::set_active( yorn );
}


void auto_aim_arm_mcs::frame_advance( time_value_t time_inc )
{
  if( aim_type == PROCEDURAL_ARM_ONLY )
    frame_advance1( time_inc );
  else
    frame_advance2( time_inc );
}


void auto_aim_arm_mcs::frame_advance1(time_value_t time_inc)
{
  character *chr = (character *)ent;

  // a ZEROVEC target norm signifies that there is no valid target
  if ( chr->get_current_target_norm() == ZEROVEC )
    return;

  po alter_arm_po = po_identity_matrix;
  po tmp_po = po_identity_matrix;

  po upper_arm_po = po_identity_matrix;
  po lower_arm_po = po_identity_matrix;
  po hand_po = po_identity_matrix;

  // animation will screw up the matrices, need to detach it.
  entity_anim* a;
  if ( (a = chr->limb_ptr(RIGHT_UPPER_ARM)->get_body()->get_anim())!=NULL )
  {
    a->detach();
  }
  if ( (a = chr->limb_ptr(RIGHT_LOWER_ARM)->get_body()->get_anim())!=NULL )
  {
    a->detach();
  }
  if ( (a = chr->limb_ptr(RIGHT_HAND)->get_body()->get_anim())!=NULL )
  {
    a->detach();
  }

  upper_arm_po = chr->limb_ptr(RIGHT_UPPER_ARM)->get_body()->get_rel_po();
  lower_arm_po = chr->limb_ptr(RIGHT_LOWER_ARM)->get_body()->get_rel_po();
  hand_po = chr->limb_ptr(RIGHT_HAND)->get_body()->get_rel_po();

  alter_arm_po = po_identity_matrix;
  upper_arm_po.set_rel_orientation(alter_arm_po.get_abs_orientation());

  alter_arm_po = po_identity_matrix;
  lower_arm_po.set_rel_orientation(alter_arm_po.get_abs_orientation());

  // orient the hand better (gangsta style!) (horizontal gun)
  // (normal style (vertical gun) screws up the skin on the elbow)
  alter_arm_po = po_identity_matrix;
  alter_arm_po.set_rotate_x(-1.57f);
  hand_po.set_rel_orientation(alter_arm_po.get_abs_orientation());


  // Multiply Po's to get correct orientation
  alter_arm_po = po_identity_matrix;
#if defined(TARGET_MKS)
      asm_po_mul( &upper_arm_po, &chr->limb_ptr(RIGHT_SHOULDER)->get_body()->get_po(), &alter_arm_po );
#else
      alter_arm_po = upper_arm_po * chr->limb_ptr(RIGHT_SHOULDER)->get_body()->get_po();
#endif

  vector3d start = alter_arm_po.get_position();

#if defined(TARGET_MKS)
      asm_po_mul( &lower_arm_po, &alter_arm_po, &tmp_po );
      asm_po_mul( &hand_po, &tmp_po, &alter_arm_po );
#else
      tmp_po = lower_arm_po * alter_arm_po;
      alter_arm_po = hand_po * tmp_po;
#endif

  vector3d hand_pos = alter_arm_po.get_position();


  // Calculate the XZ rotation needed
  vector3d face = hand_pos - start;
  vector3d targ = chr->get_current_target_pos() - start;

  // get xz facing
  face.y = 0.0f;
  targ.y = 0.0f;
  face.normalize();
  targ.normalize();

  rational_t dot_prod = dot(targ, face);
  if(dot_prod < 1.0f && dot_prod > -1.0f)  // shouldn't this be <= and >= ? --Sean
  {
    rational_t ang = fast_acos(dot_prod);

    alter_arm_po.set_rotate_y(-ang);
    upper_arm_po.set_rel_orientation(alter_arm_po.get_abs_orientation());

    // Multiply Po's to get NEW orientation
    alter_arm_po = po_identity_matrix;
    #if defined(TARGET_MKS)
        asm_po_mul( &upper_arm_po, &chr->limb_ptr(RIGHT_SHOULDER)->get_body()->get_po(), &alter_arm_po );
    #else
        alter_arm_po = upper_arm_po * chr->limb_ptr(RIGHT_SHOULDER)->get_body()->get_po();
    #endif

    start = alter_arm_po.get_position();

    #if defined(TARGET_MKS)
        asm_po_mul( &lower_arm_po, &alter_arm_po, &tmp_po );
        asm_po_mul( &hand_po, &tmp_po, &alter_arm_po );
    #else
        tmp_po = lower_arm_po * alter_arm_po;
        alter_arm_po = hand_po * tmp_po;
    #endif

    hand_pos = alter_arm_po.get_position();
  }


  // calculate the up-down (Z) rotation needed
  face = hand_pos - start;
  targ = chr->get_current_target_pos() - start;

  face.normalize();
  targ.normalize();

  dot_prod = dot(targ, face);
  if(dot_prod < 1.0f && dot_prod > -1.0f)
  {
    rational_t ang = fast_acos(dot_prod);

    // do we need to rotate up or down?
    if(face.y < targ.y)
      ang = -ang;

    alter_arm_po = po_identity_matrix;
    alter_arm_po.set_rotate_z(ang);
    upper_arm_po = alter_arm_po * upper_arm_po;
  }

  // set all teh updated Po's now
  chr->limb_ptr(RIGHT_UPPER_ARM)->get_body()->set_rel_po(upper_arm_po);
  chr->limb_ptr(RIGHT_LOWER_ARM)->get_body()->set_rel_po(lower_arm_po);
  chr->limb_ptr(RIGHT_HAND)->get_body()->set_rel_po(hand_po);
}


/*
  static beam *targ_beam = NULL;
  static beam *arm_beam = NULL;

  if(arm_beam == NULL)
  {
    arm_beam = g_world_ptr->add_beam( entity_id::make_unique_id(), (unsigned int)(EFLAG_ACTIVE|EFLAG_MISC_NONSTATIC) );
    targ_beam = g_world_ptr->add_beam( entity_id::make_unique_id(), (unsigned int)(EFLAG_ACTIVE|EFLAG_MISC_NONSTATIC) );

    arm_beam->set_created_entity_default_active_status();
    targ_beam->set_created_entity_default_active_status();

    targ_beam->set_color(color32(255, 0, 0, 96));
    arm_beam->set_color(color32(0, 255, 0, 96));

    targ_beam->set_flag(beam::NO_CLIP_TO_HERO);
    arm_beam->set_flag(beam::NO_CLIP_TO_HERO);
  }



  arm_beam->set_point_to_point(start, start + face*2);
  targ_beam->set_point_to_point(start, start + targ*2);
  arm_beam->compute_sector(g_world_ptr->get_the_terrain());
  targ_beam->compute_sector(g_world_ptr->get_the_terrain());
*/
/*
    ô ô
     ¿    New auto aim arm using animations!!!
    \_/
* /


float y_arm_rot = 0;

void auto_aim_arm_mcs::frame_advance2( time_value_t time_inc )
{
#define MIN_DELTA_COS_ANG   1.5e-4f
#define MAX_DELTA_COS_ANG   1.0f
#define MAX_ROTATION_SPEED  5.0f * .0174533f
  character   *chr = (character *)ent;
  chr->update_abs_po_including_limbs();

//  vector3d    face = chr->get_po().get_facing();
//  vector3d    targ = chr->get_current_target_pos() - chr->get_position();
  vector3d    face = chr->limb_ptr(RIGHT_HAND)->get_body()->get_position() - chr->limb_ptr(RIGHT_UPPER_ARM)->get_body()->get_position();
  vector3d    targ;

  // a ZEROVEC target norm signifies that there is no valid target
  if ( chr->get_current_target_norm() == ZEROVEC )
    targ = chr->get_po().get_facing();
  else
    targ = chr->get_current_target_pos() - chr->limb_ptr(RIGHT_UPPER_ARM)->get_body()->get_position();

  rational_t  ang, dotpr, tmp;

  vector3d hface( face );
  hface.y = .0f;
  hface.normalize();
  vector3d htarg( targ );
  htarg.y = .0f;
  htarg.normalize();
  dotpr = dot( hface, htarg );
  tmp = __fabs( dotpr - 1 );
  if ( tmp <= MAX_DELTA_COS_ANG )
  {
    entity* lb;
    vector3d pos;
    po p1, newpo;

    if ( tmp >= MIN_DELTA_COS_ANG )  // adjust horizontal angle
    {
      // determine delta angle for aiming
      ang = -fast_acos( dotpr );
      //tmp = MAX_ROTATION_SPEED * time_inc;
      //if ( ang > tmp ) ang = tmp;
      if ( htarg.z*hface.x - htarg.x*hface.z > .0f )
        ang = -ang;

      // we will distribute half to the abdomen and half to the chest
      if ( aim_type == ARM_ONLY )
        ang *= 0.3333f;
      else
        ang *= 0.5f;

      p1.set_rotate_y( ang );

      if ( aim_type == ARM_ONLY )
      {
        // get arm po
        lb = chr->limb_ptr(RIGHT_UPPER_ARM)->get_body();
        pos = lb->get_rel_position();
        newpo = lb->get_po();

        // rotate
        newpo.set_rel_position( ZEROVEC );
        newpo.add_increment( &p1 );

        // transform result to local space and apply to arm
        newpo = newpo * lb->get_parent()->get_po().inverse();
        newpo.fixup();
        newpo.set_rel_position( pos );
        lb->set_rel_po( newpo );
      }

      // get chest po
      lb = chr->limb_ptr(CHEST)->get_body();
      pos = lb->get_rel_position();
      newpo = lb->get_po();

      // rotate
      newpo.set_rel_position( ZEROVEC );
      newpo.add_increment( &p1 );

      // transform result to local space and apply to chest
      newpo = newpo * lb->get_parent()->get_po().inverse();
      newpo.fixup();
      newpo.set_rel_position( pos );
      lb->set_rel_po( newpo );

      // get abdomen po
      lb = lb->get_parent();
      pos = lb->get_rel_position();
      newpo = lb->get_po();

      // rotate
      newpo.set_rel_position( ZEROVEC );
      newpo.add_increment( &p1 );

      // transform result to local space and apply to abdomen
      newpo = newpo * lb->get_parent()->get_po().inverse();
      newpo.fixup();
      newpo.set_rel_position( pos );
      lb->set_rel_po( newpo );
    }

    if ( aim_type == ARM_ONLY )
    {
      // now do the vertical
      chr->update_abs_po_including_limbs();

      vector3d axis = chr->get_po().get_x_facing();
      axis.normalize();

      // project onto axis plane for good angle
      face = face - axis * dot(face,axis);
      targ = targ - axis * dot(targ,axis);

      face.normalize();
      targ.normalize();

      dotpr = dot( face, targ );
      tmp = __fabs( dotpr - 1 );

      if ( tmp>=MIN_DELTA_COS_ANG && tmp<=MAX_DELTA_COS_ANG )
      {
        // determine delta angle for aiming
        ang = -fast_acos( dotpr );

    //    tmp = MAX_ROTATION_SPEED * time_inc;
    //    if ( ang > tmp ) ang = tmp;

        if ( targ.y > face.y )
          ang = -ang;

        // we will distribute half to the abdomen and half to the chest
        if ( aim_type == ARM_ONLY )
          ang *= 0.3333f;
        else
          ang *= 0.5f;

        po p1;
        p1.set_rot( axis, ang );

        if ( aim_type == ARM_ONLY )
        {
          // get arm po
          lb = chr->limb_ptr(RIGHT_UPPER_ARM)->get_body();
          pos = lb->get_rel_position();
          newpo = lb->get_po();

          // rotate
          newpo.set_rel_position( ZEROVEC );
          newpo.add_increment( &p1 );

          // transform result to local space and apply to arm
          newpo = newpo * lb->get_parent()->get_po().inverse();
          newpo.fixup();
          newpo.set_rel_position( pos );
          lb->set_rel_po( newpo );
        }

        // get chest po
        lb = chr->limb_ptr(CHEST)->get_body();
        pos = lb->get_rel_position();
        newpo = lb->get_po();

        // rotate
        newpo.set_rel_position( ZEROVEC );
        newpo.add_increment( &p1 );

        // transform result to local space and apply to chest
        newpo = newpo * lb->get_parent()->get_po().inverse();
        newpo.fixup();
        newpo.set_rel_position( pos );
        lb->set_rel_po( newpo );

        // get abdomen po
        lb = lb->get_parent();
        pos = lb->get_rel_position();
        newpo = lb->get_po();

        // rotate
        newpo.set_rel_position( ZEROVEC );
        newpo.add_increment( &p1 );

        // transform result to local space and apply to abdomen
        newpo = newpo * lb->get_parent()->get_po().inverse();
        newpo.fixup();
        newpo.set_rel_position( pos );
        lb->set_rel_po( newpo );
      }
    }
  }
#undef MIN_DELTA_COS_ANG
#undef MAX_DELTA_COS_ANG
#undef MAX_ROTATION_SPEED
}
!*/
