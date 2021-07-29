////////////////////////////////////////////////////////////////////////////////
/*
  physent.cpp

    Home of the physical_entity, basic physical primitive of DBTS
    Derived classed include
        Rigid_Body
        Complex_Body
        Particle
*/
#if 0



#include "global.h"

//!#include "character.h"
#include "app.h"
#include "colgeom.h"
#include "game.h"
#include "oserrmsg.h"
#include "pmesh.h"
//#include "amesh.h"
#include "joint.h"
//#include "physent.h"
#include "wds.h"
#include "capsule.h"
#include "colmesh.h"
//!#include "actor.h"
#include "mcs.h"
#include "profiler.h"
//!#include "character.h"
#include "terrain.h"
#include "geomgr.h"
#include "collide.h"
#include "bsp_collide.h"
#include "controller.h"

extern FILE * debug_log_file;

bool first_hit_this_frame = false;
extern rational_t g_y_accel;

//rational_t g_hero_x_max = 5.5f;

//#define VERBOSE_COLLISIONS

const rational_t FRICTION_COEFICIENT = 5;
const rational_t MIN_FRICTION_Y_NORMAL = .5f;
////////////////////////////////////////////////////////////////////////////////
// physical_entity:  Constructors
////////////////////////////////////////////////////////////////////////////////

physical_entity::physical_entity( const entity_id& _id, unsigned int _flags )
  : entity( _id, ENTITY_PHYSICAL, _flags )
{
  initialize_variables();
}


// TODO: rationalize entity flags
physical_entity::physical_entity( const entity_id& _id,
                                  entity_flavor_t _flavor,
                                  unsigned int _flags )
  : entity( _id, _flavor, _flags )
{
  initialize_variables();
}


physical_entity::physical_entity( visual_rep *vrep, entity_flavor_t _flavor )
  : entity( ANONYMOUS, _flavor )
{
  initialize_variables();
  // Visual
  set_flag( EFLAG_GRAPHICS, true );
  set_visible( true );
  my_visrep = vrep;

  // Physical
  set_stationary( true );
}


physical_entity::~physical_entity()
{
  delete damage_capsule;
  // any descendant of entity that overloads add_me_to_region() and
  // remove_me_from_region() (see entity.h) needs to call this cleanup method
  // in its destructor; the reason for this is that if we wait and let the
  // parent class (e.g., entity) call this method, it will end up using the
  // parent's version of remove_me_from_region() (which makes sense when you
  // think about it, since by that point the descendant class has already been
  // destroyed)
  remove_from_terrain();
}


void physical_entity::initialize_variables()
{
  emitter = 0;
  damage_capsule = NULL;
  set_in_use(true);
  aging = true;
  fade_away = false;
  properties_initialized = false;
  set_walkable(false);
  set_physical( true );
  //  set_stationary(false);
  on_the_ground = false;
  velocity = ZEROVEC;
  angular_velocity = ZEROVEC;
  acceleration_factor = ZEROVEC;
  acceleration_correction_factor = ZEROVEC;
  collided_last_frame = false;
  ext_collided_last_frame = false;
  collision_flags = 0x00000000;
  mass = 0;
  volume = 0;
  set_sticky(false);
  set_repulsion(false);
}


///////////////////////////////////////////////////////////////////////////////
// NEWENT File I/O
///////////////////////////////////////////////////////////////////////////////

physical_entity::physical_entity( chunk_file& fs,
                                  const entity_id& _id,
                                  entity_flavor_t _flavor,
                                  unsigned int _flags )
  : entity( fs, _id, _flavor, _flags )
{
  initialize_variables();
  set_visible( true );
  //  set_flag( EFLAG_PHYSICS_MOVING, true );

  // make lowercase version for searching
  stringx tempstr = fs.get_dir();
  tempstr.to_lower();
  // strip off unnecessary portion of path
  stringx::size_type n = tempstr.find( "\\data\\" );
  if ( n != stringx::npos )
    my_dirname = fs.get_dir().substr( n+6 );
  else
    my_dirname = fs.get_dir();

  //my_filename = fs.get_filename();

  // physical_entity-specific data
  chunk_flavor cf;
  bool read_physical_properties = false;
  for ( serial_in(fs,&cf); cf!=CHUNK_END; serial_in(fs,&cf) )
  {
    if ( cf == chunk_flavor("physprop") )
    {
      // read physical properties
      read_physical_properties = true;
      for ( serial_in(fs,&cf); cf!=CHUNK_END; serial_in(fs,&cf) )
      {
        if ( cf == chunk_flavor("mass") )
        {
          rational_t m;
          serial_in( fs, &m );
          set_mass( m );
        }
        else if ( cf == chunk_flavor("moi") )
        {
          vector3d v;
          serial_in( fs, &v );
          set_I_vector( v );
        }
        else if ( cf == chunk_flavor("volume") )
        {
          rational_t v;
          serial_in( fs, &v );
          set_volume( v );
        }
        else if ( cf == chunk_flavor("radius") )
        {
          rational_t r;
          serial_in( fs, &r );
          set_radius( r );
        }
        else if ( cf == chunk_flavor("com") )
        {
          vector3d v;
          serial_in( fs, &v );
          set_c_o_m( v );
        }
        else
          error( fs.get_name() + ": unknown chunk found in physical properties" );
      }
    }
    else
      error( fs.get_name() + ": unknown chunk found in physical_entity" );
  }

  if ( colgeom && !read_physical_properties )
  {
    // estimate physical properties from collision geometry if necessary
    colgeom->estimate_physical_properties( this );
  }
  set_collision_flags( entity::COLLIDE_WITH_TERRAIN | entity::COLLIDE_WITH_ACTORS );
}


///////////////////////////////////////////////////////////////////////////////
// Old File I/O
///////////////////////////////////////////////////////////////////////////////

physical_entity::physical_entity( const stringx& entity_fname,
                                  const entity_id& _id,
                                  entity_flavor_t _flavor,
                                  bool _active,
                                  bool _stationary,
                                  bool delete_stream )
  : entity( entity_fname, _id, _flavor, SKIP_DELETE_STREAM )
{
  initialize_variables();
  set_visible( true );

  // Physical
  set_active( _active );
  set_flag( EFLAG_PHYSICS_MOVING, true );
  set_stationary( _stationary );

  if ( colgeom )
    colgeom->estimate_physical_properties( this );

  read_info();

  if ( delete_stream )
  {
    assert( my_fs );
    delete my_fs;
    my_fs = NULL;
  }
}


void physical_entity::read_info()
{
  stringx label, vmesh_fname, dummy, entity_flavor;

  set_collision_flags(entity::COLLIDE_WITH_TERRAIN|entity::COLLIDE_WITH_ACTORS);

  my_dirname = my_fs->get_dir();
  //my_filename = my_fs->get_filename();

  set_rel_position(ZEROVEC);
  abs_center = get_abs_position();
  center = ZEROVEC;
}


///////////////////////////////////////////////////////////////////////////////
// Instancing
///////////////////////////////////////////////////////////////////////////////

entity* physical_entity::make_instance( const entity_id& _id,
                                        unsigned int _flags ) const
{
  physical_entity* newpe = NEW physical_entity( _id, _flags );
  newpe->set_flag( EFLAG_MISC_NONSTATIC, true );  // physents are always nonstatic
  newpe->copy_instance_data( *this );
  return (entity*)newpe;
}


void physical_entity::copy_instance_data( const physical_entity& b )
{
  entity::copy_instance_data( b );

  initialize_variables();
  flavor = b.flavor;
  my_dirname = b.my_dirname;
  //my_filename = b.my_filename;
  mass = b.mass;
  volume = b.volume;
  properties_initialized = b.properties_initialized;
  center = b.center;
  abs_center = b.abs_center;
  collision_flags = b.collision_flags;
  on_the_ground = b.on_the_ground;

  set_visible( true );
  set_flag( EFLAG_PHYSICS_MOVING, true );
  set_collision_flags( entity::COLLIDE_WITH_TERRAIN | entity::COLLIDE_WITH_ACTORS );
}


////////////////////////////////////////////////////////////////////////////////
// Misc.
////////////////////////////////////////////////////////////////////////////////

// entity interface virtual functions for setting position need to be overloaded
// in physent so that the center (computed by compute_dynamic_properties()) is
// adjusted appropriately
void physical_entity::po_changed()
{
//!  entity::po_changed();
  abs_center = get_abs_po().slow_xform(center);
}


// these virtual functions allow types descended from entity to be
// recognized when adding them to regions, so that the region class can
// maintain lists of different entity types as desired
void physical_entity::add_me_to_region( region* r )
{
  r->add( this );
}
void physical_entity::remove_me_from_region( region* r )
{
  r->remove( this );
}


void physical_entity::frame_advance(time_value_t t)
{
  if ( is_active() && is_physical() )
  {
    vector3d angvel;
    get_angular_velocity(&angvel);

    //assert (parent==NULL);

    // Visual Representation
    /*
    if ( my_visrep )
      {
      if ( my_visrep->get_type() == VISREP_AMESH )
        {
        if( ((vr_amesh*)my_visrep)->kill_me() )
          {
          tam_suicide();
          }
        }
      }
    */

    // Physical Representation
    if ( !is_stationary() )
    {
      velocity += acceleration_factor;
      g_y_accel = acceleration_factor.y;
#ifdef BSP_COLLISIONS
      vector3d posn;
      vector3d v;
      vector3d pi, n, nu, nv, vp;
      po new_po;
      bool valid = false;
      int collisions;

      vector3d vel;
      get_velocity(&vel);
      v = (vel - get_acceleration_correction_factor()) * t;
      if ( is_frame_delta_valid() )
      {
        set_rel_position( get_abs_position() - get_frame_delta().get_position() );
        v += get_frame_delta().get_position();
      }
      valid = add_position_increment( v );

      if ( colgeom )
      {
        assert(colgeom->get_type() == collision_geometry::CAPSULE);
        collision_capsule *cap = (collision_capsule *)colgeom;

        collisions = 0;
        first_hit_this_frame = true;

        if ( valid )
        {
//!          if ( get_flavor() != ENTITY_CHARACTER )
            set_velocity((v / t) + get_acceleration_correction_factor());
          // update the angular component if needed
          new_po = get_abs_po();
          po ang_vel_rot;
          ang_vel_rot.set_rot(angvel * t);
          new_po.add_increment(&ang_vel_rot);
          new_po.fixup();
          set_rel_po(new_po);
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
        new_po = get_abs_po();
        // update the position component
        posn = get_abs_position();
        posn += v;
        new_po.set_position(posn);
        // update the angular component if needed
        //new_po = get_abs_po();
        po ang_vel_rot;
        ang_vel_rot.set_rot(angvel * t);
        new_po.add_increment(&ang_vel_rot);
        new_po.fixup();
        set_rel_po(new_po);
      }
#else
      vector3d posn = get_abs_position();
      vector3d vel;
      get_velocity(&vel);
      posn += (vel - get_acceleration_correction_factor())*t;
      po new_po = get_abs_po();
      new_po.set_rel_position(posn);
      po ang_vel_rot;
      ang_vel_rot.set_rot(angvel*t);
      new_po.add_increment(&ang_vel_rot);
      new_po.fixup();
      set_rel_po(new_po);
#endif
    }
    else
    {
      set_velocity(ZEROVEC);
      set_angular_velocity(ZEROVEC);
    }

    set_on_the_ground(false);
    set_last_acceleration_correction_factor(get_acceleration_correction_factor());
    set_acceleration_correction_factor(ZEROVEC);
    acceleration_factor = ZEROVEC;

    //    compute_dynamic_properties();
    entity::frame_advance(t);
  }
}

//physical_entity const * g_thiso;

bool physical_entity::add_position_increment( vector3d& v )
{
#ifdef BSP_COLLISIONS

  int MAX_COLLISIONS = this == g_world_ptr->get_hero_ptr() ? 20 : 5;
  //g_thiso = this;
  vector3d posn, col_posn;
  vector3d pi, n, nu, nv, vp;
//!  rational_t d;
  po new_po, new_collision_po;
  bool valid = false;
  int collisions;

  if ( colgeom && are_collisions_active())
  {
    assert(colgeom->get_type() == collision_geometry::CAPSULE);
    collision_capsule *cap = (collision_capsule *)colgeom;

    assert(cap->get_rel_capsule().base.is_valid() && cap->get_rel_capsule().end.is_valid());

    collisions = 0;
    first_hit_this_frame = true;
    po temp_po = get_abs_po();
    vector3d original_v = v;

    int pass = 0;
    do
    {
      // get the current po
      new_po = get_abs_po();
      // update the position component according to this trial
      posn = get_abs_position();
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
      new_collision_po = get_colgeom_root_po();
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
      {
		    vector3d sphere_base_center = cap->get_abs_capsule().base;
        vector3d sphere_end_center = cap->get_abs_capsule().end;

        rational_t sphere_radius = cap->get_abs_capsule().radius;

//          debug_print( "add_position_increment: checking " + v3tos(col_posn) );
        if ( !g_world_ptr->get_the_terrain().in_world(sphere_base_center, sphere_radius, v, n, pi) )
        {
          vector3d new_n = sphere_base_center-pi;

          set_last_collision_normal( n );

//            debug_print( "bottom sphere collision: distance " + v3tos(new_n)
//                         + " normal " + v3tos(n) );
          d = new_n.length();
          if ( d <= 0.0f )
          {
            // we're imbedded--bail
            v = ZEROVEC;
            set_velocity( ZEROVEC );
            set_ext_collided_last_frame( true );
            return true;
          }

          n = new_n / d;
          v += (sphere_radius - d + 0.001f)*n;  // Fudge factor to make sure the sphere is pushed firmly into valid space.
          collisions++;
          first_hit_this_frame = false;
          valid = false;
        }
        else if ( !g_world_ptr->get_the_terrain().in_world(sphere_end_center, sphere_radius, v, n, pi) )
        {
          vector3d new_n = sphere_end_center-pi;

          set_last_collision_normal( n );
          //                  debug_print( "top sphere collision: distance " + v3tos(new_n)
          //                               + " normal " + v3tos(n) );
          // handle the case where we imbedded ourselves in the world
          //            if (dot(new_n,n)<0) new_n = -new_n;
          // if we get too embedded, new_n will be zero--use old n
          d = new_n.length();
          if ( d <= 0.0f )
          {
            v = ZEROVEC;
            set_velocity( ZEROVEC );
            set_ext_collided_last_frame( true );
            return true;
          }

          n = new_n / d;

          v += (sphere_radius - d + 0.001f)*n;  // Fudge factor to make sure the sphere is pushed firmly into valid space.
          collisions++;
          first_hit_this_frame = false;
          valid = false;
        }
      }
!*/
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
        posn = get_abs_position();
        posn += v;
        new_po.set_position(posn);
      }
    }

    set_rel_po(new_po);
    return valid;
  }
  else
  {
    // get the current po
    new_po = get_abs_po();
    // update the position component
    posn = get_abs_position();
    posn += v;
    new_po.set_position(posn);
    set_rel_po(new_po);
    return true;
  }

#else

  vector3d posn = get_abs_position();
  posn += v;
  po new_po = get_abs_po();
  new_po.set_rel_position(posn);
  set_rel_po(new_po);
  return true;

#endif
}

////////////////////////////////////////////////////////////////////////////////
// physical_entity:  Physical Representation Methods
////////////////////////////////////////////////////////////////////////////////


// loc is ignored here.  They are used by derived versions of this function.
// Makes a sort of "fat interface".
void physical_entity::apply_force_increment( const vector3d& f,
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
}


void physical_entity::get_effective_collision_velocity( vector3d* target, const vector3d& loc ) const
{
  vector3d vel;
  vel = (get_abs_position() - get_last_position()) * (1.0f / g_world_ptr->get_cur_time_inc());
  *target = vel;
}


rational_t physical_entity::get_effective_collision_mass( const vector3d& loc,
                                                          const vector3d& dir ) const
{
  return get_mass();
}


void physical_entity::resolve_collision( entity * e1,
                                         entity * e2,
                                         time_value_t t,
                                         bool terrain_collision )
{
  for (int i=0;i<hit_list.size();++i)
  {
    vector3d hit_loc = hit_list[i];
    vector3d ecv1, ecv2;
    e1->get_effective_collision_velocity(&ecv1,hit_loc);
    e2->get_effective_collision_velocity(&ecv2,hit_loc);
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

      rational_t m1 = e1->get_effective_collision_mass( hit_loc, impact_dir );
      rational_t m2 = e2->get_effective_collision_mass( hit_loc, impact_dir );
      assert (m1);
      assert (m2);

      // handle collision w/ a physically inactive by making the other object have zero mass
/*!      vector3d impulse = ZEROVEC;
      bool e1_only = ( e2->is_stationary() || (e2->get_flavor()==ENTITY_CHARACTER && ((character *)e2)->get_paralysis_factor()));
      bool e2_only = ( e1->is_stationary() || (e1->get_flavor()==ENTITY_CHARACTER && ((character *)e1)->get_paralysis_factor()));
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
      if ( imbedded_dist < 0 )
        imbedded_dist = 0;
      rational_t clip_out1,clip_out2;
      if ( e2_only )
      {
        clip_out2 = imbedded_dist;
        vector3d incv = clip_out2 * impact_dir;
        e2->add_position_increment( incv );
      }
      else if ( e1_only )
      {
        clip_out1 = imbedded_dist;
        vector3d incv = -clip_out1 * impact_dir;
        e1->add_position_increment( incv );
      }
      else
      {
        clip_out1 = imbedded_dist * m2 / (m1+m2);
        clip_out2 = imbedded_dist * m1 / (m1+m2);
        vector3d incv = -clip_out1 * impact_dir;
        e1->add_position_increment( incv );
        incv = clip_out2 * impact_dir;
        e2->add_position_increment( incv );
      }
!*/
      //     if((e1->get_flavor() == ENTITY_CHARACTER && ((character *)e1)->get_paralysis_factor()) || (e2->get_flavor() == ENTITY_CHARACTER && ((character *)e2)->get_paralysis_factor()))
      //        valid = false;


      if ( valid )
      {
        // it terrain_collision, create a temporary contact joint.
        // note b2 inactive ==> b1 active since they had a valid collision.
        // entities are assumed to be bodies for the moment. <<<<
        if ( terrain_collision )
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

extern profiler_counter profcounter_path_collision;

bool physical_entity::resolve_collision_with_terrain( entity* e1, time_value_t t, const po& velocity_po, const vector3d& abs_base_point )
{
  vector3d hit_loc;
  vector3d vel = ZEROVEC;
  int lesser=-1;
  float o=1.0f;
  int i;
  for(i=0;i<hit_list.size();++i)
  {
    float n;
    hit_loc = hit_list[i];
    e1->get_effective_collision_velocity( &vel, hit_loc );
  #if defined(VERBOSE_COLLISIONS)
    debug_print( "hit " + itos(i) + " point " + v3tos(hit_loc)
                 + " normal " + v3tos(normal_list2[i]) );
  #endif

    // Velocity po is used when the local space of the collided-with object is skewed from
    // world space.  This is an articact of using this collision algorithm for collisions with
    // Static entities instead of just with terrain meshes (which are never rotated).
    // It's needed because such collisions are detected in the local space of the static object.
    vector3d rel_vel = velocity_po.non_affine_inverse_xform( vel );
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
    e1->get_effective_collision_velocity( &vel, hit_loc );
    vector3d rel_vel = velocity_po.non_affine_inverse_xform( vel );
    vector3d dir = vel;
    if ( dir.length2() > SMALL_DIST )
      dir.normalize();
    else
      dir = normal_list1[i];
    rational_t m1 = e1->get_effective_collision_mass( hit_loc, dir );
    assert (m1);

    // terrain_collision: create a temporary contact joint.
    vector3d normal;
    vector3d abs_hit_loc = velocity_po.slow_xform( hit_loc );
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
/*!      if ( e1->get_flavor() == ENTITY_CHARACTER )
      {
        vector3d pinc = normal * (cap->get_core_radius() - d);
        ((character *)e1)->add_position_increment( pinc );
        e1->update_colgeom();
      }
      else
        assert(0);
!*/
    }
    else
      normal = velocity_po.non_affine_inverse_xform( normal_list2[i] );


    if ( normal.length2() > SMALL_DIST )
    {
      linear_joint * nj = NEW linear_joint( e1, /*! NULL, !*/ 0, 0, normal, normal,
                                             abs_hit_loc, abs_hit_loc, 2, 0.0f, true );
      nj->frame_advance( t );
      delete nj;
    }

    profcounter_path_collision.add_count(1);
    return 1;
  }

  return 0;
}


collision_capsule* physical_entity::get_updated_damage_capsule()
{
  if (get_damage_capsule() && !get_damage_capsule()->is_valid())
  {
    damage_capsule->xform(get_abs_po());
  }
  return get_damage_capsule();
}


const vector3d& physical_entity::get_center() const
{
  if (properties_initialized)
    return abs_center;
  else
    return get_abs_position();
}


void physical_entity::compute_dynamic_properties()
{
  abs_center = get_abs_position();
  center = get_rel_position();
  properties_initialized = true;
}


void physical_entity::invalidate_colgeom()
{
  entity::invalidate_colgeom();

  if (damage_capsule)
    damage_capsule->invalidate();
}


bool physical_entity::using_velocity() const
{
  return ( !is_stationary() && !has_parent() );
}


void physical_entity::update_unused_velocity(time_value_t increment)
{
  if ( !using_velocity() /*! && get_flavor() != ENTITY_CHARACTER !*/)
  {
    if (mi && mi->frame_delta_valid)
    {
      velocity = mi->frame_delta.get_position()/increment; //(get_abs_position()-last_position)/increment;
    }
    else
      velocity = ZEROVEC;
  }
}


rational_t physical_entity::get_friction_scale() const
{
  return 1.0f;
}

vector3d physical_entity::get_last_position() const
{
  return last_position;
}

/*
void physical_entity::read_enx( chunk_file& fs )
{
  entity::read_enx( fs );
}
*/

/*
void physical_entity::read_enx( chunk_file& fs, stringx& lstr )
{
  entity::read_enx( fs, lstr );
}
*/
/*
bool physical_entity::handle_enx_chunk( chunk_file& fs, stringx& label )
{
  return(entity::handle_enx_chunk( fs, label ));
}
*/


void physical_entity::suspend()
{
  if (!suspended)
  {
    velocity.x = 0;
    velocity.z = 0;
  }

  entity::suspend();
}

#endif
