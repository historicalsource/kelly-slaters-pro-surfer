#ifndef RIGID_H
#define RIGID_H

//#include "physent.h"
//!class actor;

class physics_debug_info
{
public:
  rational_t initial_height;
};

class rigid_body: public physical_entity
{
public:
  rigid_body( const entity_id& _id, unsigned int _flags );

  rigid_body( const entity_id& _id = ANONYMOUS,
              entity_flavor_t _flavor = ENTITY_RIGID_BODY,
              unsigned int _flags = 0 );

  virtual ~rigid_body();

private:
  void initialize_variables();

/////////////////////////////////////////////////////////////////////////////
// entity class identification
public:
  virtual bool is_a_rigid_body() const { return true; }

// NEWENT File I/O
public:
  rigid_body( chunk_file& fs,
              const entity_id& _id,
              entity_flavor_t _flavor = ENTITY_RIGID_BODY,
              unsigned int _flags = 0,
              actor* _my_actor = NULL );

// Old File I/O
public:
  rigid_body( const char* colgeom_fname,
              actor* _my_actor,
              bool _active = true );

  
// Instancing
public:
  virtual entity* make_instance( const entity_id& _id,
                                 unsigned int _flags ) const;
protected:
  void copy_instance_data( const rigid_body& b );


// Misc.
public:
  virtual void frame_advance(float t);

  virtual void apply_force_increment( const vector3d& f, force_type ft, const vector3d& loc, int mods=0 );

  actor * get_my_actor() const { return my_actor; }

  virtual void get_effective_collision_velocity( vector3d* target, const vector3d& loc) const;
  virtual rational_t get_effective_collision_mass( const vector3d& loc, const vector3d& dir) const;

  virtual bool is_active() const { assert(0); return 0; }

  void get_I_vector(vector3d* target) const { *target=vector3d(I_x, I_y, I_z); }
  virtual void set_I_vector( const vector3d& Iv ) { I_x = Iv.x; I_y = Iv.y; I_z = Iv.z; }

  void get_c_o_m(vector3d* target) const { *target = c_o_m; }
  virtual void set_c_o_m( const vector3d& v ) { c_o_m=v; }


  // Debugging Info
  virtual rational_t compute_energy();
private:
  virtual rational_t get_I_about_axis(vector3d axis) const;

  rational_t I_x, I_y, I_z;
  // center of mass
  vector3d c_o_m;

  actor * my_actor;

  physics_debug_info dinfo;
};

#endif