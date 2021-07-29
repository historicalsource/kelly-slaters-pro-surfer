#include "global.h"

#include "physical_interface.h"
#include "entity.h"
#include "bsp_collide.h"
#include "wds.h"
#include "joint.h"
#include "collide.h"
#include "guidance_sys.h"
#include "terrain.h"
#include "profiler.h"
#include "console.h"

const rational_t FRICTION_COEFICIENT = 5;
const rational_t MIN_FRICTION_Y_NORMAL = .5f;

bool first_hit_this_frame = false;


physical_interface::physical_interface(entity *ent)
  : entity_interface(ent),
  flags(0)
{
  mass = 0.0f;
  volume = 0.0f;
  velocity = ZEROVEC;
  angular_velocity = ZEROVEC;
  acceleration_factor = ZEROVEC;
  acceleration_correction_factor = ZEROVEC;
  collision_flags = 0x00000000;

  bounce_factor = 0.5f;
  slide_factor = 0.65f;
  sticky_offset = 0.025f;

  gravity_multiplier = 1.0f;

  guide_sys = NULL;

  stuck_parent_was_alive_last_frame = true;

  last_elevation = 0.0f;
  cur_elevation = 0.0f;
  effectively_standing = false;
  cur_normal = YVEC;
  last_floor_offset = 0.0f;
}

physical_interface::~physical_interface()
{
  if(has_guidance_sys())
    destroy_guidance_sys();
}

bool physical_interface::using_velocity() const
{
  return( !my_entity->is_stationary() );//&& !my_entity->has_parent() );
}

vector3d physical_interface::get_effective_collision_velocity( const vector3d& loc ) const
{
  return((my_entity->get_abs_position() - my_entity->get_last_position()) * (1.0f / g_world_ptr->get_cur_time_inc()));
}

void physical_interface::frame_advance(time_value_t t)
{
  if(has_guidance_sys() && !is_stuck())
    guidance_sys()->frame_advance(t);

  set_flag(_PHYS_BOUNCED, false);

  if(is_bouncy() || is_sticky())
  {
    if ( !is_stuck() )
    {
      region_node *old_region = my_entity->get_primary_region();
      assert(old_region);

      my_entity->update_abs_po_reverse();
      vector3d old_pos = my_entity->get_abs_position();

      do_physics(t);

      vector3d new_pos = my_entity->get_abs_position();
      vector3d delta = new_pos - old_pos;
      delta.normalize();

      entity *hit_entity = NULL;
      vector3d hit, hitn;

/*
      if ( find_intersection( old_pos, new_pos + ((item_owner && item_owner->is_rocket()) ? get_abs_po().get_facing() : ZEROVEC),
                              (use_owner_region && item_owner && item_owner->get_owner() != NULL) ? item_owner->get_owner()->get_primary_region() : get_primary_region(),
                              FI_COLLIDE_WORLD|FI_COLLIDE_BEAMABLE,
                              &vec, &hitn,
                              NULL, &hit_entity))
*/
      // avoid colliding with myself....
      bool collisions_active = my_entity->are_collisions_active();
      my_entity->set_collisions_active(false, false);

      if ( find_intersection( old_pos, new_pos + (delta * (my_entity->get_visual_radius() > 0.0f ? my_entity->get_visual_radius()*0.75f : 0.1f)),
                              old_region,
                              FI_COLLIDE_WORLD|FI_COLLIDE_BEAMABLE,
                              &hit, &hitn,
                              NULL, &hit_entity))
      {
        hitn.normalize();
        bounce(hit, hitn, hit_entity);
      }

      my_entity->set_collisions_active(collisions_active, false);
    }
    else
    {
      if(my_entity->has_parent() && !((entity *)my_entity->link_ifc()->get_parent())->is_alive() && stuck_parent_was_alive_last_frame)
      {
        //stuck = false;
        po the_po = my_entity->get_abs_po();
        my_entity->link_ifc()->set_parent( NULL );
        my_entity->set_rel_po_no_children(the_po);
        set_velocity(ZEROVEC);
        set_gravity(true);
        set_angular_velocity(ZEROVEC);

        set_flag(physical_interface::_PHYS_STUCK, false);
      }

      stuck_parent_was_alive_last_frame = my_entity->has_parent() ? ((entity *)my_entity->link_ifc()->get_parent())->is_alive() : true;
    }
  }
  else
    do_physics(t);

/*
  // this is not necessary i believe
  my_entity->update_abs_po_reverse();
*/
}

void physical_interface::manage_standing(bool force)
{
  compute_elevation();
  my_entity->update_abs_po_reverse();
  const vector3d& cp = my_entity->get_abs_position();

  rational_t floor_delta = cp.y - get_floor_offset() - cur_elevation;
  vector3d vel = get_velocity();

  if ( force || floor_delta < (vel.y <= 0.0f ? 0.05f : 0.0f))
  {
    // force the character onto the floor
    vector3d fp = cp;
    fp.y -= floor_delta;

    po new_po = my_entity->get_abs_po();
    new_po.set_position(fp);

    if(my_entity->has_parent())
      fast_po_mul(new_po, new_po, my_entity->link_ifc()->get_parent()->get_abs_po().inverse());

    my_entity->set_rel_po_no_children(new_po);
    my_entity->update_abs_po_reverse();

    effectively_standing = true;

	  // clear horizontal velocity
    vel.y = vel.x = vel.z = 0;
    set_velocity( vel );
  }
  else
    effectively_standing = false;
}


void physical_interface::compute_elevation()
{
  last_elevation = cur_elevation;
  cur_elevation = g_world_ptr->get_the_terrain().get_elevation( my_entity->get_abs_position(),
                                                                cur_normal,
                                                                my_entity->get_primary_region() );
}


// return desired distance from character's root to floor
rational_t physical_interface::get_floor_offset()
{
  // for now at least, this is entirely determined by the root animation
  entity_anim_tree* a = NULL;

  for(int i=(MAX_ANIM_SLOTS-1); i>=0 && (a == NULL); --i)
//  for(int i=0; i<MAX_ANIM_SLOTS && (a == NULL); ++i)
  {
    a = my_entity->get_anim_tree( i );
    if(a && (a->is_finished() || a->get_floor_offset() <= 0.0f || !a->is_root(get_my_entity())))
      a = NULL;
  }

  if ( a!=NULL && a->is_relative_to_start() )
  {
    vector3d relp = ZEROVEC;
    a->get_current_root_relpos( &relp );
    last_floor_offset = a->get_floor_offset();

#if defined(TARGET_PC)
    if(last_floor_offset == 0.0f)
    {
      static bool warned = false;

      if(!warned)
      {
        warning("Animation '%s' Does not appear to have a floor offset!", a->get_name().c_str());
        warned = true;
      }

      last_floor_offset = 1.0f;
    }
#endif

    last_floor_offset += relp.y;
  }
  return last_floor_offset;
}

void physical_interface::do_physics(time_value_t t)
{
  assert(is_enabled() && !is_suspended() && !my_entity->playing_scene_anim());

  my_entity->set_needs_compute_sector(true);

  vector3d angvel = get_angular_velocity();

  // Physical Representation
  if ( is_increment() && !my_entity->is_stationary() )
  {
    my_entity->update_abs_po_reverse();

    velocity += acceleration_factor;
#ifdef BSP_COLLISIONS
    vector3d posn;
    vector3d v;
//    vector3d pi, n, nu, nv, vp; // unused, remove me?
    po new_po;
    bool valid = false;
    int collisions;

    vector3d vel = get_velocity();
    v = (vel - get_acceleration_correction_factor()) * t;
    if ( my_entity->is_frame_delta_valid() )
    {
      po new_po = my_entity->get_abs_po();
      new_po.set_position(my_entity->get_abs_position() - my_entity->get_frame_delta().get_position());

      if(my_entity->has_parent())
        fast_po_mul(new_po, new_po, my_entity->link_ifc()->get_parent()->get_abs_po().inverse());

      my_entity->set_rel_po_no_children(new_po);

//      my_entity->set_rel_position_no_children( my_entity->get_abs_position() - my_entity->get_frame_delta().get_position() );
      v += my_entity->get_frame_delta().get_position();
    }


    START_PROF_TIMER(proftimer_add_pos_increment);
    valid = add_position_increment( v );
    my_entity->update_abs_po_reverse();
    STOP_PROF_TIMER(proftimer_add_pos_increment);

    if ( my_entity->get_colgeom() && my_entity->get_colgeom()->get_type() == collision_geometry::CAPSULE)
    {
      assert(my_entity->get_colgeom()->get_type() == collision_geometry::CAPSULE);
//      collision_capsule *cap = (collision_capsule *)my_entity->get_colgeom(); // unused, remove me?

      collisions = 0;
      first_hit_this_frame = true;

      if ( valid )
      {
//!          if ( get_flavor() != ENTITY_CHARACTER )
          set_velocity((v / t) + get_acceleration_correction_factor());
        // update the angular component if needed
        new_po = my_entity->get_abs_po();
        po ang_vel_rot;
        ang_vel_rot.set_rot(angvel * t);
        new_po.add_increment(&ang_vel_rot);

        if(my_entity->has_parent())
          fast_po_mul(new_po, new_po, my_entity->link_ifc()->get_parent()->get_abs_po().inverse());

        new_po.fixup();
        my_entity->set_rel_po_no_children(new_po);
      }
      else
      {
        // kill any velocity
        set_velocity(vector3d(0.0f, 0.0f, 0.0f));
      }
    }
    else
    {
      // get the current po
      new_po = my_entity->get_abs_po();
      // update the position component
      posn = my_entity->get_abs_position();
      posn += v;
      new_po.set_position(posn);
      // update the angular component if needed
      //new_po = get_abs_po();
      po ang_vel_rot;
      ang_vel_rot.set_rot(angvel * t);
      new_po.add_increment(&ang_vel_rot);

      if(my_entity->has_parent())
        fast_po_mul(new_po, new_po, my_entity->link_ifc()->get_parent()->get_abs_po().inverse());

      new_po.fixup();
      my_entity->set_rel_po_no_children(new_po);
    }
#else
    vector3d posn = my_entity->get_abs_position();
    vector3d vel;
    get_velocity(&vel);
    posn += (vel - get_acceleration_correction_factor())*t;
    po new_po = my_entity->get_abs_po();
    new_po.set_position(posn);
    po ang_vel_rot;
    ang_vel_rot.set_rot(angvel*t);
    new_po.add_increment(&ang_vel_rot);

    if(my_entity->has_parent())
      fast_po_mul(new_po, new_po, my_entity->link_ifc()->get_parent()->get_abs_po().inverse());

    new_po.fixup();
    my_entity->set_rel_po_no_children(new_po);
#endif
  }
  else
  {
    set_velocity(ZEROVEC);
    set_angular_velocity(ZEROVEC);
  }

  set_last_acceleration_correction_factor(get_acceleration_correction_factor());
  set_acceleration_correction_factor(ZEROVEC);
  acceleration_factor = ZEROVEC;

  my_entity->update_abs_po_reverse();
}

bool physical_interface::add_position_increment( vector3d& v )
{
  my_entity->update_abs_po_reverse();

#ifdef BSP_COLLISIONS

  int MAX_COLLISIONS = (my_entity == g_world_ptr->get_hero_ptr(g_game_ptr->get_active_player()) || my_entity == g_world_ptr->get_hero_ptr(1)) ? 20 : 5;
  assert(MAX_PLAYERS < 3);
  //g_thiso = this;
  vector3d posn, col_posn;
  vector3d pi, n; // , nu, nv, vp; // unused, remove me?
  rational_t d;
  po new_po, new_collision_po;
  bool valid = false;
  int collisions;

  if ( my_entity->get_colgeom() && my_entity->get_colgeom()->get_type() == collision_geometry::CAPSULE && my_entity->are_collisions_active())
  {
    assert(my_entity->get_colgeom()->get_type() == collision_geometry::CAPSULE);
    collision_capsule *cap = (collision_capsule *)my_entity->get_colgeom();

    assert(cap->get_rel_capsule().base.is_valid() && cap->get_rel_capsule().end.is_valid());

    collisions = 0;
    first_hit_this_frame = true;
//    po temp_po = my_entity->get_abs_po(); // unused, remove me?
    vector3d original_v = v;

    int pass = 0;
    do
    {
      // get the current po
      new_po = my_entity->get_abs_po();
      // update the position component according to this trial
      posn = my_entity->get_abs_position();
      posn += v;
      new_po.set_position(posn);

/*!      if( get_flavor() == ENTITY_CHARACTER )
      {
        character *c = (character *)this;
        if( c->get_controller() && c->get_controller()->is_in_cautious_mode() )
        {
          rational_t floor_y = g_world_ptr->get_the_terrain().get_elevation( posn, vector3d(YVEC), get_region() );
          if( posn.y - floor_y > 5.0f )
          {
            collisions = MAX_COLLISIONS;
            break;
          }
        }
      }
!*/
      // do the same for the collision_geometry, which may be different if this is
      // a character
      new_collision_po = my_entity->get_colgeom_root_po();
      col_posn = new_collision_po.get_position();
      col_posn += v;
      new_collision_po.set_position(col_posn);

      cap->xform(new_collision_po);

      // PEH
      // THIS IS A TEST OF THE EMERGENCY SPHERE REMOVAL SYSTEM
      // CRITTERS ARE ONLY CHECKED AGAINST THEIR BASE INSTEAD OF AT BOTH ENDS OF THEIR CAPSULS

      //      if ( !g_world_ptr->get_the_terrain().in_world(sphere_center = cap->get_abs_capsule().base+v, sphere_radius = cap->get_abs_capsule().radius, v, n, pi) ||
      //           !g_world_ptr->get_the_terrain().in_world(sphere_center = cap->get_abs_capsule().end+v, sphere_radius = cap->get_abs_capsule().radius, v, n, pi ) )

      //      if ( !g_world_ptr->get_the_terrain().in_world(sphere_center = cap->get_abs_capsule().base+v, sphere_radius = cap->get_abs_capsule().radius, v, n, pi))

      valid = true;
/*!      if ( get_flavor()==ENTITY_CHARACTER
           && (((character*)this)->is_moving_around() || this == g_world_ptr->get_hero_ptr()) )
*/
//      #pragma fixme("Make some way of re-enabling check for character moving around (JDB 01-29-01")
      if(1)
      {
		    vector3d sphere_base_center = cap->get_abs_capsule().base;
        vector3d sphere_end_center = cap->get_abs_capsule().end;

        rational_t sphere_radius = cap->get_abs_capsule().radius;

// BETH: for front end purposes
		if(FEDone())
		{

//          debug_print( "add_position_increment: checking " + v3tos(col_posn) );
        if ( !g_world_ptr->get_the_terrain().in_world(sphere_base_center, sphere_radius, v, n, pi) )
        {
//          if(dot((XVEC+ZVEC), v) != 0.0f)
//            int x = 1;
          set_last_collision_normal( n );

          vector3d new_n = sphere_base_center-pi;
/*
          if(sphere_base_center == pi || new_n.length2() <= 0.00001f || !pi.is_valid())
          {
#if _CONSOLE_ENABLE
            console_log("WAY CLOSE: %f (%d) <%.2f, %.2f, %.2f> <%.2f, %.2f, %.2f>!", new_n.length2(), pi.is_valid(), new_n.x, new_n.y, new_n.z, pi.x, pi.y, pi.z);
#endif
          }
*/

//            debug_print( "bottom sphere collision: distance " + v3tos(new_n)
//                         + " normal " + v3tos(n) );
          d = new_n.length2();
          if ( d <= 0.0f )
          {
            // we're imbedded--bail
            v = ZEROVEC;
            set_velocity( ZEROVEC );
            set_ext_collided_last_frame( true );
            return true;
          }

          d = __fsqrt(d);

/*
          if(collisions == 1)
          {
#if _CONSOLE_ENABLE
            console_log("first norm <%.2f, %.2f, %.2f> v <%.2f, %.2f, %.2f>", n.x, n.y, n.z, v.x, v.y, v.z);
#endif
          }
*/

          n = new_n / d;

/*          vector3d old_v = v;*/
          v += (sphere_radius - d + 0.001f)*n;  // Fudge factor to make sure the sphere is pushed firmly into valid space.
          collisions++;


/*
          if(collisions == 2)
          {
#if _CONSOLE_ENABLE
            console_log("second norm <%.2f, %.2f, %.2f> v <%.2f, %.2f, %.2f>", n.x, n.y, n.z, v.x, v.y, v.z);
#endif
          }

          if(v.length() > 3.0f)
          {
#if _CONSOLE_ENABLE
            console_log("FUCK(%d) norm <%.2f, %.2f, %.2f> v <%.2f, %.2f, %.2f>\n newn <%.2f, %.2f, %.2f> pi <%.2f, %.2f, %.2f> d=%f\nold v <%.2f, %.2f, %.2f> sphere <%.2f, %.2f, %.2f> (%.2f)", collisions, n.x, n.y, n.z, v.x, v.y, v.z, new_n.x, new_n.y, new_n.z, pi.x, pi.y, pi.z, d, old_v.x, old_v.y, old_v.z, sphere_base_center.x, sphere_base_center.y, sphere_base_center.z, sphere_radius);
#endif
          }
*/
          valid = false;
        }
        else if ( !g_world_ptr->get_the_terrain().in_world(sphere_end_center, sphere_radius, v, n, pi) )
        {
          set_last_collision_normal( n );

          vector3d new_n = sphere_end_center-pi;

          //                  debug_print( "top sphere collision: distance " + v3tos(new_n)
          //                               + " normal " + v3tos(n) );
          // handle the case where we imbedded ourselves in the world
          //            if (dot(new_n,n)<0) new_n = -new_n;
          // if we get too embedded, new_n will be zero--use old n
          d = new_n.length2();
          if ( d <= 0.0f )
          {
            v = ZEROVEC;
            set_velocity( ZEROVEC );
            set_ext_collided_last_frame( true );
            return true;
          }
          d = __fsqrt(d);

          n = new_n / d;

          v += (sphere_radius - d + 0.001f)*n;  // Fudge factor to make sure the sphere is pushed firmly into valid space.
          collisions++;
          valid = false;
        }

		}// BETH

      }
//!*/
      if ( collisions == MAX_COLLISIONS && pass == 0 && !valid )
      {
        // CTT 03/23/00: TODO: somehow restore the last valid capsule (used to come from actor_angle_mcs)
      }
    } while ( !valid && (collisions < MAX_COLLISIONS) );

    // friction
    if ( collisions > 0 )
    {
      set_last_collision_normal( n );
      set_ext_collided_last_frame( true );
      if (collisions==MAX_COLLISIONS)
        v = ZEROVEC;
      vector3d diff_dir = v-original_v;
      rational_t diff = (v-original_v).length();
      if ( diff > 0 )
      {
        diff_dir /= diff;
        rational_t less_friction_factor = diff_dir.y-MIN_FRICTION_Y_NORMAL;
        if ( less_friction_factor < 0 )
          less_friction_factor = 0;
        else
          less_friction_factor *= (1-MIN_FRICTION_Y_NORMAL);
        rational_t drag = diff*FRICTION_COEFICIENT*less_friction_factor*get_friction_scale();
        if ( drag > 1 )
          drag = 1;
        v -= v*drag;
        posn = my_entity->get_abs_position();
        posn += v;
        new_po.set_position(posn);
      }
    }

    if(my_entity->has_parent())
      fast_po_mul(new_po, new_po, my_entity->link_ifc()->get_parent()->get_abs_po().inverse());
/*
    else if(new_po.get_position().length() < 5.0f && collisions > 0)
    {
      vector3d pos = new_po.get_position();
      vector3d posn = my_entity->get_abs_position();
#if _CONSOLE_ENABLE
      console_log("%s add_pos_increment <%.2f, %.2f, %.2f> -> <%.2f, %.2f, %.2f>\n%d/%d collisions\nvel <%.2f, %.2f, %.2f> (orig <%.2f, %.2f, %.2f>)\nnorm <%.2f, %.2f, %.2f>", my_entity->get_name().c_str(), posn.x, posn.y, posn.z, pos.x, pos.y, pos.z, collisions, MAX_COLLISIONS, v.x, v.y, v.z, original_v.x, original_v.y, original_v.z, n.x, n.y, n.z);
#endif
    }
*/

    my_entity->set_rel_po_no_children(new_po);
    return valid;
  }
  else
  {
    // get the current po
    new_po = my_entity->get_abs_po();
    // update the position component
    posn = my_entity->get_abs_position();
    posn += v;
    new_po.set_position(posn);

    if(my_entity->has_parent())
      fast_po_mul(new_po, new_po, my_entity->link_ifc()->get_parent()->get_abs_po().inverse());

    my_entity->set_rel_po_no_children(new_po);
    return true;
  }

#else

  vector3d posn = my_entity->get_abs_position();
  posn += v;
  po new_po = my_entity->get_abs_po();
  new_po.set_position(posn);

  if(my_entity->has_parent())
    fast_po_mul(new_po, new_po, my_entity->link_ifc()->get_parent()->get_abs_po().inverse());

  my_entity->set_rel_po_no_children(new_po);
  return true;

#endif
}


void physical_interface::update_unused_velocity(time_value_t increment)
{
  if ( !using_velocity() )
  {
    if (my_entity->get_movement_info() && my_entity->get_movement_info()->frame_delta_valid)
    {
      velocity = my_entity->get_movement_info()->frame_delta.get_position()/increment; //(get_abs_position()-last_position)/increment;
    }
    else
      velocity = ZEROVEC;
  }
}

// loc is ignored here.  They are used by derived versions of this function.
// Makes a sort of "fat interface".
void physical_interface::apply_force_increment( const vector3d& f,
                                             force_type ft,
                                             const vector3d& loc, int mods )
{
  if(get_mass() != 0.0f){
  vector3d a = f/get_mass();
  acceleration_factor += a;
  //assert(a.y<5);

  //set_velocity(get_velocity()+a);
  if (ft==CONTINUOUS)
    set_acceleration_correction_factor(get_acceleration_correction_factor() + 0.5f*a);
  }

  effectively_standing = false;
}

void physical_interface::set_last_collision_normal( const vector3d& v )
{
  last_collision_normal = v;

#ifdef _DEBUG
  last_collision_normal.normalize();
  assert(last_collision_normal.length2() >= 0.001f);
#endif
}












///////////////////////////////////////////////
void physical_interface::resolve_collision( entity * e1,
                                         entity * e2,
                                         time_value_t t,
                                         bool terrain_collision )
{
  assert(e1->get_abs_position() != e2->get_abs_position());
  for (unsigned i=0;i<hit_list.size();++i)
  {
    vector3d hit_loc = hit_list[i];
    vector3d ecv1, ecv2;

    ecv1 = e1->has_physical_ifc() ? e1->physical_ifc()->get_effective_collision_velocity(hit_loc) : ZEROVEC;
    ecv2 = e2->has_physical_ifc() ? e2->physical_ifc()->get_effective_collision_velocity(hit_loc) : ZEROVEC;

    vector3d vel = ecv1-ecv2;
    bool valid = (dot(vel,normal_list1[i])>=0 && dot(vel,normal_list2[i])<=0);
    if (valid)
    {
      vector3d impact_dir = hit_loc - e1->get_abs_position();
      impact_dir.y = 0;
      if ( impact_dir.length2() > SMALL_DIST )
        impact_dir.normalize();
      else
        impact_dir = ZVEC;

      vector3d eff_vel = impact_dir * dot( vel, impact_dir );

      vector3d dir = vel;
      if ( dir.length2() > SMALL_DIST )
        dir.normalize();
      else
        dir = normal_list1[i];

      rational_t m1 = e1->has_physical_ifc() ? e1->physical_ifc()->get_effective_collision_mass( hit_loc, impact_dir ) : 1.0f;
      rational_t m2 = e2->has_physical_ifc() ? e2->physical_ifc()->get_effective_collision_mass( hit_loc, impact_dir ) : 1.0f;
      assert (m1);
      assert (m2);

      // handle collision w/ a physically inactive by making the other object have zero mass
      if(!terrain_collision && (e1->has_physical_ifc() || e2->has_physical_ifc()))
      {
        vector3d impulse = ZEROVEC;
        bool e1_only = ( e2->is_stationary() || !e2->has_physical_ifc() || e2->physical_ifc()->is_immobile() );//|| (e2->get_flavor()==ENTITY_CHARACTER && ((character *)e2)->get_paralysis_factor()));
        bool e2_only = ( e1->is_stationary() || !e1->has_physical_ifc() || e1->physical_ifc()->is_immobile() );//|| (e1->get_flavor()==ENTITY_CHARACTER && ((character *)e1)->get_paralysis_factor()));

        if ( e2_only )
        {
          impulse = 2.0f * m2 * eff_vel;
        }
        else if ( e1_only )
        {
          impulse = 2.0f * m1 * eff_vel;
        }
        else if ( eff_vel.length2() > SMALL_DIST )
        {
          impulse = m1 * m2 / (m1+m2)*eff_vel;
        }

        // Now we clip the entities' positions so they no longer intersect
        //rational_t imbedded_dist = (e1->get_colgeom()->get_radius()+e2->get_colgeom()->get_radius()) - xz_norm(e1->get_abs_position()-e2->get_abs_position());
        vector3d diff;
        rational_t imbedded_dist = (e1->get_colgeom()->get_core_radius()+e2->get_colgeom()->get_core_radius()) - collision_geometry::distance_between_cores(e1->get_colgeom(), e2->get_colgeom(), &diff);
        imbedded_dist += 0.001f;   // fudge it a little to prevent multiple collisions
        if ( imbedded_dist < 0.0f )
          imbedded_dist = 0.0f;

        rational_t clip_out1,clip_out2;
        if ( e2_only )
        {
          clip_out2 = imbedded_dist;

/*
          if(e2->get_colgeom()->get_type() == collision_geometry::CAPSULE)
          {
            impact_dir.y = 0.0f;
            impact_dir.normalize();
          }
*/

          vector3d incv = clip_out2 * impact_dir;
          e2->physical_ifc()->add_position_increment( incv );
          e2->update_abs_po_reverse();
        }
        else if ( e1_only )
        {
/*
          if(e1->get_colgeom()->get_type() == collision_geometry::CAPSULE)
          {
            impact_dir.y = 0.0f;
            impact_dir.normalize();
          }
*/

          clip_out1 = imbedded_dist;
          vector3d incv = -clip_out1 * impact_dir;
          e1->physical_ifc()->add_position_increment( incv );
          e1->update_abs_po_reverse();
        }
        else
        {
/*
          if(e1->get_colgeom()->get_type() == collision_geometry::CAPSULE && e2->get_colgeom()->get_type() == collision_geometry::CAPSULE)
          {
            impact_dir.y = 0.0f;
            impact_dir.normalize();
          }
*/
          clip_out1 = imbedded_dist * (m2 / (m1+m2));
          clip_out2 = imbedded_dist * (m1 / (m1+m2));
          vector3d incv = -clip_out1 * impact_dir;
          e1->physical_ifc()->add_position_increment( incv );
          e1->update_abs_po_reverse();
          incv = clip_out2 * impact_dir;
          e2->physical_ifc()->add_position_increment( incv );
          e2->update_abs_po_reverse();
        }

        valid = false;
      }
      else
        terrain_collision = true;

      //     if((e1->get_flavor() == ENTITY_CHARACTER && ((character *)e1)->get_paralysis_factor()) || (e2->get_flavor() == ENTITY_CHARACTER && ((character *)e2)->get_paralysis_factor()))
      //        valid = false;


      if ( valid )
      {
        // it terrain_collision, create a temporary contact joint.
        // note b2 inactive ==> b1 active since they had a valid collision.
        // entities are assumed to be bodies for the moment. <<<<
        if (terrain_collision )
        {
          linear_joint * nj = NEW linear_joint( e1, /*! NULL,!*/ 0, 0, normal_list2[i], normal_list2[i],
                                                 hit_loc, hit_loc, 2, 0.4f, true );
          nj->frame_advance(t);
          delete nj;
        }
/*!        else if ( impulse.length2() > SMALL_DIST )
        {
          if ( e1->is_active() && !e2_only )
            e1->apply_force_increment( -impulse, entity::INSTANT, hit_loc );
          if ( e2->is_active() && !e1_only )
            e2->apply_force_increment( impulse, entity::INSTANT, hit_loc );
        #if defined(TARGET_PC)
          if (debug_log_file)
            fprintf(debug_log_file,"Bounce impulse = (%f, %f, %f)\n",impulse.x,impulse.y,impulse.z);
        #endif
          break;
        }
!*/
      }
    }
  }
}

//extern profiler_counter profcounter_path_collision;

bool physical_interface::resolve_collision_with_terrain( entity* e1, time_value_t t, const po& velocity_po, const vector3d& abs_base_point )
{
  vector3d hit_loc;
  vector3d vel = ZEROVEC;
  int lesser=-1;
  rational_t o=1.0f;
  unsigned i;
  for(i=0;i<hit_list.size();++i)
  {
    rational_t n;
    hit_loc = hit_list[i];
    vel = e1->has_physical_ifc() ? e1->physical_ifc()->get_effective_collision_velocity(hit_loc) : ZEROVEC;
  #if defined(VERBOSE_COLLISIONS)
    debug_print( "hit " + itos(i) + " point " + v3tos(hit_loc)
                 + " normal " + v3tos(normal_list2[i]) );
  #endif

    // Velocity po is used when the local space of the collided-with object is skewed from
    // world space.  This is an articact of using this collision algorithm for collisions with
    // Static entities instead of just with terrain meshes (which are never rotated).
    // It's needed because such collisions are detected in the local space of the static object.
//    vector3d rel_vel =  // unused, remove me?
    velocity_po.non_affine_inverse_xform( vel );
    vector3d rel_pos = velocity_po.inverse_xform( abs_base_point ); //e1->get_abs_position()  -- ERROR? PTA
  #if defined(VERBOSE_COLLISIONS)
    debug_print( "abs_pos: " + v3tos(abs_base_point) + " rel_pos: " + v3tos(rel_pos) );
  #endif

    if ( dot(rel_pos-hit_list[i],normal_list2[i]) >= 0 )
    {
      // Experiment, clip to the one we're least behind
      //n = -dot(e1->get_updated_closest_point_along_dir(normal_list2[i])-hit_list[i],normal_list2[i]);
      // if we have position history (e.g. with capsules) take advantage of it for a more accurate response
      n = -dot(rel_pos-hit_list[i],normal_list2[i]);
  #if defined(VERBOSE_COLLISIONS)
      debug_print("depth " + ftos(n) );
  #endif
      if( n<o )
      {
        lesser = i;
        o = n;
      }
    }
  }

  if( lesser >= 0 )
  {
    i = lesser;
  #if defined(VERBOSE_COLLISIONS)
    debug_print( "picked " + itos(i) );
  #endif
    hit_loc = hit_list[i];
    vel = e1->has_physical_ifc() ? e1->physical_ifc()->get_effective_collision_velocity(hit_loc) : ZEROVEC;

    //vector3d rel_vel = // unused, remove me?
    velocity_po.non_affine_inverse_xform( vel );
    vector3d dir = vel;
    if ( dir.length2() > SMALL_DIST )
      dir.normalize();
    else
      dir = normal_list1[i];

    assert (e1->has_physical_ifc() ? e1->physical_ifc()->get_effective_collision_mass( hit_loc, dir ) : 1.0f );

    // terrain_collision: create a temporary contact joint.
    vector3d normal;
    vector3d abs_hit_loc = velocity_po.slow_xform( hit_loc );
    bool doit = true;
    if ( e1->get_colgeom()->get_type() == collision_geometry::CAPSULE )
    {
      collision_capsule *cap = (collision_capsule *)e1->get_colgeom();
      vector3d line_norm = cap->get_end() - cap->get_base(), diff = abs_hit_loc - cap->get_base();
      rational_t proj = dot( diff, line_norm ) / line_norm.length2();
      if ( proj < 0.0f )
        normal = abs_hit_loc - cap->get_base();
      else if ( proj < 1.0f )
        normal = diff - proj * line_norm;
      else
        normal = abs_hit_loc - cap->get_end();

      rational_t d = normal.length();
  #if defined(VERBOSE_COLLISIONS)
      debug_print("resolve_collision: hit_loc " + v3tos(abs_hit_loc)
                   + " depth " + ftos(d) );
  #endif
      if ( d < SMALL_DIST )
      {
        normal = -dir;
      }
      else
      {
        normal /= d;
        normal = -normal;
        vector3d abs_normal = velocity_po.non_affine_slow_xform( normal_list2[i] );
        if ( dot(normal,abs_normal) < -0.9f )
        {
          normal = -normal;
          d = -d;
        }
        d -= 0.001f;              // fudge factor
      }

  #if defined(VERBOSE_COLLISIONS)
      debug_print( "distance " + ftos(d) + " direction " + v3tos(normal) );
  #endif
      if ( e1->has_physical_ifc() )
      {
        vector3d pinc = normal * (cap->get_core_radius() - d);
        e1->physical_ifc()->add_position_increment( pinc );
        e1->update_abs_po_reverse();
        e1->update_colgeom();

        doit = false;
      }
    }
    else
      normal = velocity_po.non_affine_inverse_xform( normal_list2[i] );


    if ( doit && normal.length2() > SMALL_DIST )
    {
      linear_joint * nj = NEW linear_joint( e1, /*! NULL, !*/ 0, 0, normal, normal,
                                             abs_hit_loc, abs_hit_loc, 2, 0.0f, true );
      nj->frame_advance( t );
      delete nj;
    }

    //profcounter_path_collision.add_count(1);
    return 1;
  }

  return 0;
}




void physical_interface::bounce(vector3d hit, vector3d hit_norm, entity *hit_entity)
{
  bounce_pos = hit;
  bounce_norm = hit_norm;
  bounce_ent = hit_entity;

  if(is_sticky())
  {
    set_velocity(ZEROVEC);
    set_gravity(false);
    set_angular_velocity(ZEROVEC);

    if(is_sticky_orient())
    {
      po my_po = po_identity_matrix;
      my_po.set_facing(hit_norm);

      vector3d new_pos = hit + (hit_norm*sticky_offset);
      my_po.set_position(new_pos);

      my_entity->set_rel_po_no_children(my_po);
    }
    else
    {
      vector3d new_pos = hit - (my_entity->get_abs_po().get_facing()*sticky_offset);
      my_entity->set_rel_position_no_children(new_pos);
    }

    if(hit_entity != NULL && my_entity->has_link_ifc())
    {
      po relpo = my_entity->get_abs_po() * hit_entity->get_abs_po().inverse();
      my_entity->link_ifc()->set_parent( hit_entity );
      my_entity->set_rel_po_no_children( relpo );

      stuck_parent_was_alive_last_frame = hit_entity->is_alive();
    }

    set_flag(_PHYS_STUCK, true);
  }
  else if(is_bouncy())
  {
    vector3d vel, dir;
    vel = get_velocity();

    rational_t vel_len = vel.length();

    if(vel_len > 0.0f)
    {
      // reverse it
      vel *= -1;

      // calculate NEW bounce direction
      rational_t inv_vel_len = 1.0f / vel_len;

      dir.x = vel.x*inv_vel_len;
      dir.y = vel.y*inv_vel_len;
      dir.z = vel.z*inv_vel_len;

      my_entity->set_rel_position_no_children(hit + (dir * (my_entity->get_visual_radius() > 0.0f ? my_entity->get_visual_radius()*0.75f : 0.1f)));

/*
      po the_po;
      rational_t dot_prod = dot(dir, hit_norm);
      vector3d cross_prod = cross(dir, hit_norm);

      the_po.set_rot((cross_prod), -2.0f*fast_acos(dot_prod));
      vel = the_po.non_affine_slow_xform(vel);
*/
/*
	    // Mirror code
	    vector3d y = ray to reflect;
	    vector3d u = normal to reflect by;
	    vector3d reflection = (u * (2.0f * (dot(y, u) / u.length2()))) - y;
*/
  	  vel = ((hit_norm * (2.0f * (dot(dir, hit_norm) / hit_norm.length2()))) - dir)*vel_len;

      // cheapo, better would be to make it multiply by bounce_factor in dir of normal and slide_factor in other 2 ortho dir's
      vel.x *= slide_factor;
      vel.y *= bounce_factor;
      vel.z *= slide_factor;

      set_velocity(vel);
    }
    else
      my_entity->set_rel_position_no_children(hit + (hit_norm * (my_entity->get_visual_radius() > 0.0f ? my_entity->get_visual_radius()*0.75f : 0.1f)));
  }

  set_flag(_PHYS_BOUNCED, true);
  my_entity->raise_signal(entity::BOUNCED);
}



guidance_system *physical_interface::create_guidance_sys(guidance_system::eGuidanceSysType type)
{
  assert(!guide_sys);

  switch(type)
  {
    case guidance_system::_GUIDANCE_GENERIC:
    {
      guide_sys = NEW guidance_system(this);
    }
    break;

    case guidance_system::_GUIDANCE_ROCKET:
    {
      guide_sys = NEW rocket_guidance_sys(this);
    }
    break;

    default:
      assert(0);
      break;
  }

  return(guide_sys);
}

void physical_interface::destroy_guidance_sys()
{
  assert(guide_sys);
  delete guide_sys;
  guide_sys = NULL;
}

bool physical_interface::get_ifc_num(const pstring &att, rational_t &val)
{
  IFC_INTERNAL_GET_MACRO("MASS", mass);
  IFC_INTERNAL_GET_MACRO("VOLUME", volume);
  IFC_INTERNAL_GET_MACRO("SLIDE_FACTOR", slide_factor);
  IFC_INTERNAL_GET_MACRO("BOUNCE_FACTOR", bounce_factor);
  IFC_INTERNAL_GET_MACRO("STICKY_OFFSET", sticky_offset);

  IFC_INTERNAL_GET_MACRO("BOUNCY", is_bouncy());
  IFC_INTERNAL_GET_MACRO("STICKY", is_sticky());
  IFC_INTERNAL_GET_MACRO("ENABLED", is_enabled());
  IFC_INTERNAL_GET_MACRO("SUSPENDED", is_suspended());
  IFC_INTERNAL_GET_MACRO("GRAVITY", is_gravity());
  IFC_INTERNAL_GET_MACRO("GRAVITY_MULTIPLIER", get_gravity_multiplier());
  IFC_INTERNAL_GET_MACRO("HAS_BOUNCED", has_bounced());
  IFC_INTERNAL_GET_MACRO("IS_STUCK", is_stuck());
  IFC_INTERNAL_GET_MACRO("STICKY_ORIENT", is_sticky_orient());

  return(false);
}

bool physical_interface::set_ifc_num(const pstring &att, rational_t val)
{
  IFC_INTERNAL_SET_MACRO("MASS", mass);
  IFC_INTERNAL_SET_MACRO("VOLUME", volume);
  IFC_INTERNAL_SET_MACRO("SLIDE_FACTOR", slide_factor);
  IFC_INTERNAL_SET_MACRO("BOUNCE_FACTOR", bounce_factor);
  IFC_INTERNAL_SET_MACRO("STICKY_OFFSET", sticky_offset);

  IFC_INTERNAL_FUNC_MACRO("BOUNCY", set_bouncy(val != 0.0f));
  IFC_INTERNAL_FUNC_MACRO("STICKY", set_sticky(val != 0.0f));
  IFC_INTERNAL_FUNC_MACRO("ENABLED", enable(val != 0.0f));
  IFC_INTERNAL_FUNC_MACRO("SUSPENDED", suspend(val != 0.0f));
  IFC_INTERNAL_FUNC_MACRO("GRAVITY", set_gravity(val != 0.0f));
  IFC_INTERNAL_FUNC_MACRO("GRAVITY_MULTIPLIER", set_gravity_multiplier(val));
  IFC_INTERNAL_FUNC_MACRO("STICKY_ORIENT", set_sticky_orient(val != 0.0f));

  return(false);
}

bool physical_interface::get_ifc_vec(const pstring &att, vector3d &val)
{
  IFC_INTERNAL_GET_MACRO("VELOCITY", get_velocity());
  IFC_INTERNAL_GET_MACRO("ANGULAR_VELOCITY", get_angular_velocity());
  IFC_INTERNAL_GET_MACRO("BOUNCE_POS", bounce_pos);
  IFC_INTERNAL_GET_MACRO("BOUNCE_NORM", bounce_norm);

  return(false);
}

bool physical_interface::set_ifc_vec(const pstring &att, const vector3d &val)
{
  IFC_INTERNAL_FUNC_MACRO("VELOCITY", set_velocity(val));
  IFC_INTERNAL_FUNC_MACRO("ANGULAR_VELOCITY", set_angular_velocity(val));

  return(false);
}
