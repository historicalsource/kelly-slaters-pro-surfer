
#ifndef PHYSENT_H
#define PHYSENT_H

#include "entity.h"
#include "physics.h"
//#include "hwaudio.h"


//class fx_manager;
class world_dynamics_system;
class collision_geometry;
class region;
//!class actor;
class collision_capsule;

//const vector3d IGNORE_LOC = vector3d(10e10f,10e10f,10e10f);

// CLASS entity is a fat interface for all entities.  physical_entity implements a lot of physical stuff.
enum force_type
{
  CONTINUOUS=0,
  INSTANT=1
};

class physical_entity : public entity
{
// Constructors
public:
  physical_entity( const entity_id& _id, unsigned int _flags );

  physical_entity( const entity_id& _id = ANONYMOUS,
                   entity_flavor_t _flavor = ENTITY_PHYSICAL,
                   unsigned int _flags = 0 );

  physical_entity( visual_rep *vrep,
                   entity_flavor_t _flavor = ENTITY_PHYSICAL );

  virtual ~physical_entity();

private:
  void initialize_variables();

/////////////////////////////////////////////////////////////////////////////
// entity class identification
public:
  virtual bool is_a_physical_entity() const { return true; }

// NEWENT File I/O
public:
  physical_entity( chunk_file& fs,
                   const entity_id& _id,
                   entity_flavor_t _flavor = ENTITY_PHYSICAL,
                   unsigned int _flags = 0 );
//    virtual void read_enx( chunk_file& fs );
//    virtual void read_enx( chunk_file& fs, stringx& lstr );
//    virtual bool handle_enx_chunk( chunk_file& fs, stringx& label );


// Old File I/O
public:
  physical_entity( const stringx& entity_fname,
                   const entity_id& _id = ANONYMOUS,
                   entity_flavor_t _flavor = ENTITY_PHYSICAL,
                   bool _active = INACTIVE,
                   bool _stationary = false,
                   bool delete_stream = SKIP_DELETE_STREAM );
private:
  // used by constructors which read in an entity file.
  // Reads whatever info is common to all physical entites.  Currently (10/16/98),
  // this means stripping the entity type header and reading in
  // the visual_representation.
  void read_info();


// Instancing
public:
  virtual entity* make_instance( const entity_id& _id,
                                 unsigned int _flags ) const;
protected:
  void copy_instance_data( const physical_entity& b );


// Misc.
public:
  // entity interface virtual functions for setting position need to be overloaded
  // in physent so that the center (computed by compute_dynamic_properties()) is
  // adjusted appropriately
  virtual void po_changed();

  // advance physical characteristics:  run physics, or generate particles, for example
  virtual void frame_advance(time_value_t t);

  // used by frame_advance to add velocity, collisions to clip guys, etc.  handles BSP enforcement
  // returns succes or failure, and clips v appropriately
  virtual bool add_position_increment( vector3d& v );

  virtual bool using_velocity() const;
  virtual void update_unused_velocity(time_value_t increment);
//    virtual void save_current_position(void);

  // these virtual functions allow types descended from entity to be
  // recognized when adding them to regions, so that the region class can
  // maintain lists of different entity types as desired
  virtual void add_me_to_region( region* r );
  virtual void remove_me_from_region( region* r );


// Visual Representation Methods
public:
  virtual rational_t get_visual_radius() const
  { return my_visrep ? my_visrep->get_radius( get_age() ) : 0; }

  virtual time_value_t get_visrep_ending_time()
  { return my_visrep->get_ending_time(); }

  // rendering info
  // maximum detail level of my mesh
  //int get_max_detail() const;

  //virtual void render( rational_t detail, render_flavor_t flavor );

  virtual void set_fade_away( bool fade ) { fade_away = true; }
  virtual bool get_fade_away() const {return fade_away;}

  //virtual stringx const & get_filename() const { return my_filename; }
  virtual stringx const & get_dirname() const { return my_dirname; }
  virtual bool has_dirname() const { return true; }


// Physical Representation Methods
public:
  virtual rational_t get_mass() const   { return mass; }
  virtual rational_t get_volume() const { return volume; }
  virtual void set_mass( rational_t m )   { mass = m; }
  virtual void set_volume( rational_t v ) { volume = v; }
  virtual void set_I_vector( const vector3d& I ) {}
  virtual void set_c_o_m( const vector3d& c ) {}
  void set_properties_initialized( bool t ) { properties_initialized = t; }

  static void resolve_collision( entity * e1, entity * e2, time_value_t t, bool terrain_collision );
  static bool resolve_collision_with_terrain( entity * e1, time_value_t t, po const & velocity_po, const vector3d& abs_base_point );

  // Note:  loc has no effect on physical_entity version of this function,
  // which assumes the force is applied to the center of mass.
  // Derived classes such as rigid_body can and do use this information
  // to accurately apply the force to a location other than the c_o_m.
  // Use of IGNORE_LOC dummy value for loc (instead of a separate flag)
  // yielded the most natural syntax for the caller.
  virtual void apply_force_increment( const vector3d& f, force_type ft, const vector3d& loc = vector3d(10e10f,10e10f,10e10f), int mods=0 );

  virtual void set_velocity( const vector3d& v ) { velocity = v; }
  // This is for assessing impact damage (as in falling)
  virtual void set_velocity_with_impact( const vector3d& v ) { set_velocity( v ); }
  virtual void set_velocity_without_impact( const vector3d& v ) { set_velocity( v ); }
  virtual void set_angular_velocity( const vector3d& v ) { angular_velocity = v;}

  virtual const vector3d& get_acceleration_factor() const   { return acceleration_factor; }
  virtual void set_acceleration_factor( const vector3d& v ) { acceleration_factor = v; }
  virtual void add_acceleration_factor( const vector3d& v )	{ acceleration_factor += v; }

  virtual const vector3d& get_acceleration_correction_factor()      	{ return acceleration_correction_factor; }
  virtual void set_acceleration_correction_factor( const vector3d& v, int mods=0 )	{ acceleration_correction_factor = v; }

  virtual const vector3d& get_last_acceleration_correction_factor()         { return last_acceleration_correction_factor; }
  virtual void set_last_acceleration_correction_factor( const vector3d& v ) { last_acceleration_correction_factor = v; }

  virtual void get_velocity(vector3d* target) const { *target=velocity; }
  vector3d get_velocity() const { return velocity;}
  virtual void get_angular_velocity(vector3d* target) const { *target=angular_velocity; }

  virtual void get_effective_collision_velocity( vector3d* target, const vector3d& loc ) const;
  virtual rational_t get_effective_collision_mass( const vector3d& loc, const vector3d& dir ) const;

  virtual collision_geometry * get_colgeom() const { return colgeom; }
  
  virtual bool is_on_the_ground() const { return on_the_ground; }
  virtual void set_on_the_ground( bool s ) { on_the_ground = s; }
  
  virtual unsigned int get_collision_flags() { return collision_flags; }
  virtual void set_collision_flags(unsigned int cf) { collision_flags = cf; }

  virtual bool get_collided_last_frame() { return collided_last_frame; }
  virtual void set_collided_last_frame( bool c ) { collided_last_frame = c; }

  virtual bool get_ext_collided_last_frame() const { return ext_collided_last_frame;}
  virtual void set_ext_collided_last_frame( bool c ) { ext_collided_last_frame = c;}

  virtual vector3d get_last_position() const;
  virtual void frame_done() { last_position = get_abs_position(); }
  
  // Debug tools
  virtual rational_t compute_energy()
  {
    assert( 0 );
    return 0;
  };

  // collision geometry maintenence
  virtual void invalidate_colgeom();

  virtual void phys_render( time_value_t t=0.0f, bool shadow = false ) {}

  const vector3d& get_center() const;
  
  // This gets a special collsion capsule for purposes of being damaged.
  virtual collision_capsule* get_damage_capsule() { return damage_capsule; }
  virtual collision_capsule* get_updated_damage_capsule();

  virtual rational_t get_friction_scale() const;
  virtual void suspend();

  vector3d get_last_collision_normal() const { return last_collision_normal;}
  void set_last_collision_normal( const vector3d& v ) { last_collision_normal = v;}

// Internal Methods
protected:
  virtual void compute_dynamic_properties();

  virtual rational_t get_I_about_axis( const vector3d& axis )
  {
    assert(0);
    return 0;
  };


// Visual Data
protected:
  bool aging;
  bool fade_away;

  stringx my_dirname;
  //stringx my_filename;


// Physical Data
protected:
  vector3d velocity;
  vector3d angular_velocity;
  vector3d acceleration_factor;
  vector3d acceleration_correction_factor;
  vector3d last_acceleration_correction_factor;
  vector3d last_position;
  rational_t mass;
  rational_t volume;
  bool properties_initialized;

  vector3d last_collision_normal;

  // Computed data each frame
  vector3d center;
  vector3d abs_center;

  unsigned int collision_flags;

  bool on_the_ground;
  bool collided_last_frame;
  bool ext_collided_last_frame;

  collision_capsule * damage_capsule;

  // Friends
//  friend class fx_manager;
  friend class world_dynamics_system;
//!  friend class actor;
};

inline physical_entity* find_physical_entity( const entity_id& id , bool unknown_ok = FIND_ENTITY_UNKNOWN_NOT_OK)
{
  return (physical_entity*)entity_manager::inst()->find_entity( id, ENTITY_PHYSICAL, unknown_ok);
}

#endif
