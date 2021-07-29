////////////////////////////////////////////////////////////////////////////////
//
//  camera.cpp
//  Copyright (C) 1999-2000 Treyarch, L.L.C.  ALL RIGHTS RESERVED
//
//  A camera
//
//
////////////////////////////////////////////////////////////////////////////////
#include "global.h"

//!#include "character.h"
#include "camera.h"
#include "geomgr.h"
#include "hwmath.h"
#include "wds.h"
#include "game.h"
//!#include "character.h"
#include "terrain.h"
#include "inputmgr.h"
#include "commands.h"
//!#include "attrib.h"
#include "collide.h"
#include "controller.h"
 // BIGCULL #include "gun.h"
#include "time_interface.h"
#include "ai_interface.h"
// BIGCULL #include "ai_senses.h"
#include "msgboard.h"
#include "console.h"
#include "wave.h"
#include "kellyslater_controller.h"
#include "beachdata.h"	// For per-beach parameters


extern bool auto_aimed_attack;
int g_mouselook_x_axis_dir = -1;
int g_mouselook_y_axis_dir = 1;


camera::camera( entity* _parent,
                const entity_id& _id,
                entity_flavor_t _flavor )
  :   entity( _id, _flavor )
{
  if (_parent != NULL)
    link_ifc()->set_parent( _parent );
  camera::set_externally_controlled( false );

  // a microphone on the marky camera
  microphone = NEW mic( this, entity_id::make_unique_id() );

  if(!has_time_ifc())
    create_time_ifc();

  time_ifc()->set_mode(_TIME_ABSOLUTE);
  time_ifc()->set_time_dilation(1.0f);
}

camera::~camera()
{
  delete microphone;
}



void camera::sync( camera& b )
{
  if( is_externally_controlled() ) return;
  if ( link_ifc()->get_parent() )
  {
    static po res;
    fast_po_mul(res, b.get_abs_po(), link_ifc()->get_parent()->get_abs_po().inverse());
//    set_rel_po( b.get_abs_po() * link_ifc()->get_parent()->get_abs_po().inverse() );
    set_rel_po( res );
  }
  else
    set_rel_po( b.get_abs_po() );
}


void camera::adjust_geometry_pipe( bool scene_analyzer )
{
  vector3d look = (const vector3d&)get_abs_po().get_matrix()[2] + get_abs_position();
  const vector3d& up = (const vector3d&)get_abs_po().get_matrix()[1];
  if ( scene_analyzer )
    {
    assert( !is_externally_controlled() );
    entity * c1 = find_camera(entity_id("USER_CAM"));
    set_rel_po(c1->get_abs_po());
    geometry_manager::inst()->set_scene_analyzer( get_abs_position(), look, up );
    }
  else
    geometry_manager::inst()->set_view( get_abs_position(), look, up );
}


void camera::get_look_and_up(vector3d *look, vector3d *up)
{
  if(look != NULL)
    *look = (const vector3d&)get_abs_po().get_matrix()[2];

  if(up != NULL)
    *up = (const vector3d&)get_abs_po().get_matrix()[1];
}


////////////////////////////////////////////////////////////////
// game_camera
////////////////////////////////////////////////////////////////

game_camera::game_camera( const entity_id& _id, entity* _target_entity )
  :   camera( NULL, _id ),
      last_frame_valid ( false ),
      target_entity( _target_entity ),
      temporary_lock( false ),
      ground_pitch_po( po_identity_matrix ),
      crawl_mode( false ),
      crawl_mode_firstperson( false )
{
  ksctrl=NULL;
}


void game_camera::sync( camera& b )
{
  if ( is_externally_controlled() ) return;
  camera::sync( b );
  last_frame_valid = false;
  temporary_lock = false;
  ground_pitch_po = po_identity_matrix;
  crawl_mode = false;
  crawl_mode_firstperson = false;
}


#define MAX_COLLISIONS 5

static vector3d collide_with_world( camera * cam, const vector3d& p, rational_t r, const vector3d& v, region_node* start_region )
{
  vector3d tv = v;

  // first see if v sends the sphere origin through any solid surface
  region_node* hit_region = start_region;
  if ( tv.length2() > 0.00001f )
  {
    vector3d hit_loc, hit_normal;
    bool hit = find_intersection( p, p+tv,
                                  start_region,
                                  FI_COLLIDE_WORLD|FI_COLLIDE_CAMERA,
                                  &hit_loc, &hit_normal,
                                  &hit_region, NULL );
    if ( hit )
    {
      // adjust tv to avoid going through surface
      tv = hit_loc - p;
      rational_t d = tv.length();
      tv *= (d-0.01f) / d;
    }
  }

  // now collide sphere with world
  vector3d newp;
  int collisions = 0;
  bool valid = false;
  bool first_hit_this_frame = true;
  do
  {
    // update the position according to this trial
    newp = p;
    newp += tv;
    vector3d n, pi;
    rational_t d;
    if ( !in_world( newp, r, v, hit_region, pi, true ) )
    {
      n = newp - pi;
      d = n.length2();
      if ( d > 0 )
      {
        d = __fsqrt( d );
        n *= 1.0f / d;
        tv = tv + (r+0.0001f-d)*n;
        ++collisions;
      }
      else
        collisions = MAX_COLLISIONS;
      first_hit_this_frame = false;
    }
    else
    {
      valid = true;
    }
  } while ( !valid && collisions<MAX_COLLISIONS );

  if ( valid )
    return newp;
  return p;
}


const rational_t CAMERA_PHYSICAL_RADIUS = 0.3f;
const rational_t CHASE_DISTANCE = 3;
const rational_t ENT_CENTER_ELEVATION = 0.9f;
const rational_t FRAMERATE_FOR_WHICH_CHUCK_THINKS_THE_CAMERA_WAS_TUNED_WHEN_HE_TUNED_IT = 20;
const rational_t TARGET_ELEVATION1 = 0.6f;
const rational_t TARGET_ELEVATION2 = 2.5f;
const rational_t TARGET_DIST_BEHIND1 = -1.6f;
const rational_t FOCUS_DIST_FROM_ENT = 1.0f;
const rational_t CAMERA_MIN_VEL = 0.001f;
const rational_t CAMERA_MAX_VEL = 10.0f;
const rational_t CAMERA_RHO_TIGHTNESS = 0.3f;
const rational_t CAMERA_THETA_TIGHTNESS = 0.15f;
const rational_t CAMERA_Y_TIGHTNESS = 0.1f;
const rational_t TARGET_SLOPE_BEYOND_NORMAL_RANGE = 0.5f;
const rational_t TARGET_MAX_ADD_ELEVATION = 4.0f;

vector3d game_camera::compute_entity_center( rational_t elevation )
{
  // begin by obtaining a legal point slightly above the target entity
  vector3d ent_center = target_entity->get_abs_position();
  ent_center.y += elevation + CAMERA_PHYSICAL_RADIUS;
  vector3d p, n;
  bool hit = g_world_ptr->get_the_terrain().find_intersection( target_entity->get_abs_position(),
                                                               ent_center, p, n );
  if ( hit )
    ent_center = p;
  ent_center.y -= CAMERA_PHYSICAL_RADIUS;
//    assert( g_world_ptr->get_the_terrain().in_world(target_entity->get_abs_position(),
//   f                                                 CAMERA_PHYSICAL_RADIUS,
//                                                    ZEROVEC, n, p) );
//    assert( g_world_ptr->get_the_terrain().in_world(ent_center,
//                                                    CAMERA_PHYSICAL_RADIUS,
//                                                    ZEROVEC, n, p) );

  return ent_center;
}

static vector3d chest(0, 0, 0);

void game_camera::frame_advance( time_value_t t )
{
  assert(t>=0 && t<1e9f);
  if ( is_externally_controlled() )
  {
    last_frame_valid = false;
    return;
  }

  assert(target_entity);

  targ_ent_pos = target_entity->get_abs_position();
  vector3d targ_ent_face = target_entity->get_abs_po().get_facing();
  po last_frame_po = get_abs_po();
  vector3d hit_loc, hit_normal;

  targ_ent_elev = targ_ent_pos.y;

  // position camera will look at
  vector3d p0 = targ_ent_pos;

  // do character-specific stuff, like elevation adjustment and crawl check
/*!  character *target_char = (target_entity->get_flavor()==ENTITY_CHARACTER)? static_cast<character*>(target_entity) : NULL;
  if ( target_char )
  {
    // is the character physical?
    if ( !target_char->is_physical() )
    {
      // if not, use his waist for the target elevation
      assert( target_char->has_limb_tree() );
      targ_ent_elev = target_char->limb_ptr(WAIST)->get_body()->get_abs_position().y;
    }
    // crawl check
    if ( target_char->get_player_controller() )
    {
      int cur_anim_index = target_char->get_player_controller()->get_current_animation_index();
      if ( cur_anim_index == ANIM_TO_CRAWL
        || cur_anim_index == ANIM_CLIMB_TO_CRAWL
        || cur_anim_index == ANIM_CLIMB_CRAWL_TRANS
        || cur_anim_index == ANIM_LAUNCH_UP_GRAB
        || cur_anim_index == ANIM_LAUNCH_F_GRAB
        )
      {
        crawl_mode = true;
      }
      else
      {
        crawl_mode = false;
      }

      if ( target_char->get_player_controller()->is_in_crawl_mode()
        && !( cur_anim_index == ANIM_TO_CRAWL
           || cur_anim_index == ANIM_CLIMB_TO_CRAWL
           || cur_anim_index == ANIM_CLIMB_CRAWL_TRANS
           || cur_anim_index == ANIM_LAUNCH_UP_GRAB
           || cur_anim_index == ANIM_LAUNCH_F_GRAB
           )
        )
      {
        if ( !crawl_mode_firstperson )
        {
          crawl_mode_firstperson = true;
          invalidate();
        }
      }
      else
      {
        crawl_mode_firstperson = false;
      }
    }
    else
    {
      crawl_mode = false;
      crawl_mode_firstperson = false;
    }

    // use the floor as a stable elevation reference if it isn't too far from the character's root
    rational_t floor_elev = target_char->get_cur_elevation() + 1.1f;
    if ( __fabs(floor_elev-targ_ent_elev) < 1.0f )
      targ_ent_elev = floor_elev;

    // when gun is drawn, bring us up a little more
    // Mattel wants the elevated view at all times (bug #1858) ! if ( target_char->get_weapon_type() == WEAPON_TYPE_GUN )
    targ_ent_elev += 0.3f;

    // elevation special cases
    if ( target_char->get_player_controller() )
    {
      hero_state_t state = target_char->get_player_controller()->get_hero_state();
      if ( state == STATE_CLIMB
        || state == STATE_FALLING
        || state == STATE_BROAD_JUMP
        || state == STATE_CLIMB_1M_LEDGE
        || state == ADVENTURE_JUMP
        || state == ADVENTURE_JUMP_FORWARD
        || state == ADVENTURE_JUMP_BACKWARD
        )
      {
        // use root elevation
        targ_ent_elev = targ_ent_pos.y;
      }
      /*
      else if ( state == STATE_CLIMB_LEDGE || state == STATE_JUMP_TO_LEDGE )
      {
        // move it up
        targ_ent_elev = targ_ent_pos.y + 0.4f;
      }
      else if ( state == STATE_CLIMB_OFF_LEDGE )
      {
        // move it down
        targ_ent_elev = targ_ent_pos.y - 0.5f;
      }
      * /
    }

    // position camera will look at
    p0.y = targ_ent_elev + 0.7f;
    static rational_t old_p0y;
    if ( last_frame_valid )
    {
      rational_t dy = p0.y - old_p0y;
      if ( __fabs(dy) > CAMERA_MIN_VEL*t )
        p0.y = old_p0y + dy * 0.2f*FRAMERATE_FOR_WHICH_CHUCK_THINKS_THE_CAMERA_WAS_TUNED_WHEN_HE_TUNED_IT*t;
    }
    old_p0y = p0.y;

    // elevation smoothing
    static rational_t old_elev;
    if ( last_frame_valid )
    {
      rational_t dy = targ_ent_elev - old_elev;
      if ( __fabs(dy) > CAMERA_MIN_VEL*t )
        targ_ent_elev = old_elev + dy * 0.1f*FRAMERATE_FOR_WHICH_CHUCK_THINKS_THE_CAMERA_WAS_TUNED_WHEN_HE_TUNED_IT*t;
    }
    old_elev = targ_ent_elev;

    // crawl check
    if ( crawl_mode )
        return;

    if ( crawl_mode_firstperson )
    {
      first_person_cam( target_char, t );
      return;
    }
  }
!*/
  bool use_last_pos = false;

  if ( !targ_ent_pos.is_valid() )
  {
    // camera went crazy!  reset it to player's position
    warning("camera target went out of the world!!");
    targ_ent_pos = target_entity->get_abs_position();
    //use_last_pos = true;
  }

  if ( target_entity->get_sector() == NULL )
  {
    // use last frame position if current focus is invalid
    use_last_pos = true;
  }

  p0 += targ_ent_face;

  // position of camera itself
  vector3d p1;
  if ( use_last_pos )
    p1 = last_frame_po.get_position();
  else
  {
    p1 = targ_ent_pos;
    p1.y = targ_ent_elev + 0.7f;
    p1 -= target_entity->get_abs_po().get_facing() * (CHASE_DISTANCE - 1.0f);
  }

  // fire a ray upwards to determine max height
  vector3d ceiling = targ_ent_pos;
  ceiling.y += 3;
  if ( !use_last_pos
    && find_intersection( targ_ent_pos, ceiling,
                          target_entity->get_region(),
                          FI_COLLIDE_WORLD|FI_COLLIDE_CAMERA,
                          &hit_loc, &hit_normal )
    )
  {
    ceiling = hit_loc;
    ceiling.y -= CAMERA_PHYSICAL_RADIUS;
    if ( p0.y > ceiling.y )
      p0.y = ceiling.y;
    if ( p1.y > ceiling.y )
      p1.y = ceiling.y;
  }

  chest = targ_ent_pos;
  chest.y += 0.7f;
  if ( chest.y > ceiling.y )
    chest.y = ceiling.y;

  // collide with stuff
  if ( !use_last_pos
    && find_intersection( targ_ent_pos, p1,
                          target_entity->get_region(),
                          FI_COLLIDE_WORLD|FI_COLLIDE_CAMERA,
                          &hit_loc, &hit_normal )
    )
  {
    // push away from the wall
    p1 = hit_loc + (hit_normal * CAMERA_PHYSICAL_RADIUS);

    if ( hit_normal.y > -0.5f )
    {
      vector3d diff = p1 - p0;
      rational_t d = diff.length2();
      if ( d > 0.0001f )
      {
        // pull up as we get closer
        d = __fsqrt(d);
        rational_t up = CHASE_DISTANCE - d;
        p1.y += up;
      }
    }
  }

  if ( p1.y > ceiling.y )
    p1.y = ceiling.y;

  if ( !use_last_pos )
    p1 = collide_with_world( this, p1, CAMERA_PHYSICAL_RADIUS, vector3d(0,0,0), get_region() );

  assert(t>=0 && t<1e2f);
  // smoothing
  blend( p0, p1, t );
}

// p0 is the desired camera focus, p1 is the desired camera position
void game_camera::blend(vector3d p0, vector3d p1, time_value_t t)
{
  assert(t>=0 && t<1e2f);
  vector3d target = p1;

  if ( last_frame_valid )
  {
    vector3d adjusted_pos = get_abs_position();
    vector3d delta = target - adjusted_pos;
    vector3d t0 = adjusted_pos-targ_ent_pos;
    vector3d t1 = target-targ_ent_pos;
    rational_t rho0 = t0.xz_length();
    rational_t rho1 = t1.xz_length();
    rational_t theta0 =  safe_atan2(-t0.x,t0.z);
    rational_t theta1 =  safe_atan2(-t1.x,t1.z);
    rational_t delta_rho = rho1-rho0;
    rational_t delta_theta = theta1 - theta0;

    if (delta_theta<-PI)     delta_theta += 2*PI;
    else if (delta_theta>PI) delta_theta -= 2*PI;

    rational_t target_rho, target_theta;
    if (__fabs(delta_rho) > CAMERA_MIN_VEL*t)
      target_rho = delta_rho* CAMERA_RHO_TIGHTNESS*FRAMERATE_FOR_WHICH_CHUCK_THINKS_THE_CAMERA_WAS_TUNED_WHEN_HE_TUNED_IT*t;
    else
      target_rho = 0;

    if (__fabs(delta_theta*rho0) > CAMERA_MIN_VEL*t)
      target_theta = delta_theta* CAMERA_THETA_TIGHTNESS*FRAMERATE_FOR_WHICH_CHUCK_THINKS_THE_CAMERA_WAS_TUNED_WHEN_HE_TUNED_IT*t;
    else
      target_theta = 0;

    rational_t cam_cos, cam_sin;
    fast_sin_cos_approx(target_theta+theta0, &cam_sin, &cam_cos);
    target.x = (target_rho+rho0)*-cam_sin + targ_ent_pos.x;
    target.z = (target_rho+rho0)* cam_cos + targ_ent_pos.z;
    /*
      if ( xz_norm(delta) > CAMERA_MIN_VEL*t )
      {
      target.x = delta.x * CAMERA_XZ_TIGHTNESS*FRAMERATE_FOR_WHICH_CHUCK_THINKS_THE_CAMERA_WAS_TUNED_WHEN_HE_TUNED_IT*t + adjusted_pos.x;
      target.z = delta.z * CAMERA_XZ_TIGHTNESS*FRAMERATE_FOR_WHICH_CHUCK_THINKS_THE_CAMERA_WAS_TUNED_WHEN_HE_TUNED_IT*t + adjusted_pos.z;
      }
    */
    //  if ( __fabs(delta.y) > CAMERA_MIN_VEL*t )
    target.y = get_abs_position().y + (delta.y * CAMERA_Y_TIGHTNESS*FRAMERATE_FOR_WHICH_CHUCK_THINKS_THE_CAMERA_WAS_TUNED_WHEN_HE_TUNED_IT*t);

    // impose speed limit
    delta = target - get_abs_position();
    rational_t d = delta.length();
    if ( d > CAMERA_MAX_VEL*t )
      target = delta * (CAMERA_MAX_VEL*t / d) + get_abs_position();

    #ifdef DEBUG
    assert(target.is_valid());
    #endif

    // it is possible for the world collision to be unresolved, in which case
    // new_target will equal get_abs_position() and the camera is stuck;  we will
    // allow the camera to move anyway so long as the destination is not clipped
    //  if ( new_target!=get_abs_position() )
    //    target = new_target;

    //  vector3d tmpn, tmppi;
    //  if ( !g_world_ptr->get_the_terrain().in_world(get_abs_position(), CAMERA_PHYSICAL_RADIUS, ZEROVEC, tmpn, tmppi) )
    //    my_po = last_frame_po;      // it's kludgetastic!

    // make sure we can actually see the target
    vector3d hit_loc, hit_normal;
    if ( find_intersection( chest, target,
                            target_entity->get_region(),
                            FI_COLLIDE_WORLD|FI_COLLIDE_CAMERA,
                            &hit_loc, &hit_normal ) )
    {
      vector3d diff = chest - hit_loc;
      if (diff.length2() > sqr(1.25f))
      {
        diff.normalize();
        target = p1; // now teleport straight to target   //hit_loc + (diff * CAMERA_PHYSICAL_RADIUS);
      }
    }
  }

  // camera target resulting from computed movement and collision with world
  target = collide_with_world( this, target, CAMERA_PHYSICAL_RADIUS, target-get_abs_position(), get_region() );

  if (!target.is_valid())
  {
    // camera went crazy!  reset it to player's position
    warning("camera went out of the world!!");
    target = target_entity->get_abs_position();
  }

  // set the final values
  set_rel_position(target);
  look_at(p0);

  last_frame_valid = true;
  last_frame_po = get_abs_po(); // my_po
}

////////////////////////////////////////////////////////////////
// MARKY_CAMERA
////////////////////////////////////////////////////////////////
marky_camera::marky_camera( const entity_id& id ) : game_camera( id, NULL )
{
  target = ZEROVEC;
  do_collide_with_world = false;
  roll = 0.0f;
  reset_priority();
}

void marky_camera::frame_advance( time_value_t t )
{
  microphone->frame_advance(t);
  microphone->update_abs_po_reverse();

  if( is_externally_controlled() ) return;
  if ( do_collide_with_world )
  {
    // current get_abs_position() is merely desired position;
    // start by clipping it to the world
    vector3d pos = get_abs_position();
    vector3d start;
    if ( last_frame_valid )
    {
      start = last_frame_pos;
      /*
        // impose speed limit
        pos -= start;
        rational_t d = norm( pos );
        if ( d > CAMERA_MAX_VEL*t )
        pos *= CAMERA_MAX_VEL*t / d;
        pos += start;
      */
    }
    else
    {
      if ( link_ifc()->get_parent() )
      {
        //        start = get_parent()->get_abs_po().slow_xform( target );
        start = link_ifc()->get_parent()->get_abs_po().fast_8byte_xform( target );
      }
      else
        start = target;
    }
    vector3d end = pos - start;
    rational_t d = end.length();
    region_node* my_region = get_region();
    if ( d > 0.00001f )
    {
      // push desired position out to account for camera radius
      end = end * ((d + CAMERA_PHYSICAL_RADIUS) / d) + start;
      // clip to world (note that my_region can be changed in the process, if the line segment passes through a portal)
      vector3d hit_loc, hit_normal;
      bool hit = find_intersection( start, end,
                                    my_region,
                                    FI_COLLIDE_WORLD|FI_COLLIDE_CAMERA,
                                    &hit_loc, &hit_normal,
                                    &my_region, NULL );
      if ( hit )
        pos = hit_loc + (start-pos)*(CAMERA_PHYSICAL_RADIUS/d);
    }

    // now collide resulting camera sphere with world
    pos = collide_with_world( this, start, CAMERA_PHYSICAL_RADIUS, pos-start, my_region );

    // set resulting camera position
    last_frame_pos = pos;
    last_frame_valid = true;
    if ( link_ifc()->get_parent() )
    {
      //      set_rel_position( get_parent()->get_abs_po().inverse_xform( pos ) );
      set_rel_position( link_ifc()->get_parent()->get_abs_po().fast_inverse_xform( pos ) );
    }
    else
      set_rel_position( pos );
  }

  make_po();
}

void marky_camera::make_po()
{
  assert( !is_externally_controlled() );
  po my_po = get_rel_po();
  my_po.set_facing( target ); //my_po

  if(__fabs(roll) >= 0.001f)
  {
    po tmp_po;
    tmp_po.set_rotate_z(roll);

    fast_po_mul(my_po, tmp_po, my_po);
//    my_po = tmp_po*my_po;
  }

  set_rel_po(my_po);
}

void marky_camera::sync( camera& b )
{
  if( is_externally_controlled() ) return;
  game_camera::sync( b );
}

void marky_camera::camera_set_target( const vector3d& pos )
{
  target = pos;
}

void marky_camera::camera_set_roll( rational_t angle )
{
  roll = angle;
}

// logarithmically transitions to the target parameters.  returns true when there.
const rational_t POS_THRESHOLD      = 0.01f;
const rational_t TARGET_THRESHOLD   = 0.01f;
const rational_t ROLL_THRESHOLD     = 0.01f;

bool marky_camera::camera_slide_to( const vector3d& new_pos, const vector3d& new_target, float new_roll, float speed )
{
  assert( !is_externally_controlled() );
  if ( speed < 1.0f )
    error( "slide_to requires that speed be >1.0f" );
  float inv_speed = 1.0f / speed;

  // position
  vector3d pos = get_rel_position();
  pos += ( new_pos - pos ) * inv_speed;
  set_rel_position( pos );

  // target
  target += ( new_target - target ) * inv_speed;

  // roll
  roll += ( new_roll - roll ) * inv_speed;

  return  __fabs( ( new_pos - pos ).length() ) < POS_THRESHOLD
    &&  __fabs( ( new_target - target ).length() ) < TARGET_THRESHOLD
    &&  __fabs( new_roll - roll ) < ROLL_THRESHOLD;
}

bool marky_camera::camera_slide_to_orbit( const vector3d& center, rational_t range, rational_t theta, rational_t psi, rational_t speed )
{
  vector3d new_target = center;
  vector3d new_pos;
  assert( !is_externally_controlled() );
  if ( link_ifc()->get_parent() )
    new_pos = center - link_ifc()->get_parent()->get_abs_position();
  else
    new_pos = center;
  // sorry, psi is just height for now.
  rational_t cos_theta, sin_theta, sin_psi;
  fast_sin_cos_approx(theta, &sin_theta, &cos_theta);
  sin_psi = fast_sin(psi);

  new_pos += vector3d( range*cos_theta, range*sin_psi, range*sin_theta  );
  return camera_slide_to( new_pos, new_target, roll, speed );
}

void marky_camera::camera_orbit( const vector3d& center, rational_t range, rational_t theta, rational_t psi )
{
  target = center;
  vector3d new_pos;
  assert( !is_externally_controlled() );
  if ( link_ifc()->get_parent() )
    new_pos = center - link_ifc()->get_parent()->get_abs_position();
  else
    new_pos = center;
  //  new_pos += vector3d( cos( theta ), sin( psi ), sin( theta ) + cos( psi )  );
  // sorry, psi is just height for now.
  rational_t cos_theta, sin_theta, sin_psi;
  fast_sin_cos_approx(theta, &sin_theta, &cos_theta);
  sin_psi = fast_sin(psi);

  new_pos += vector3d( range*cos_theta, range*sin_psi, range*sin_theta  );
  set_rel_position( new_pos );
}


vector3d marky_camera::camera_get_target( )
{
  // not implemented
  assert(0);
  return vector3d(0,0,0);
}