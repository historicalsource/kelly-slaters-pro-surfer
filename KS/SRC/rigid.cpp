////////////////////////////////////////////////////////////////////////////////
/*
  rigid.cpp

    Home of the rigid_body, basic object of physics
*/
////////////////////////////////////////////////////////////////////////////////
#include "global.h"

//!#include "character.h"
//!#include "actor.h"
#include "colgeom.h"
#include "colmesh.h"
#include "osdevopts.h"
//!#include "rigid.h"
#include "po.h"
#include "pmesh.h"
//#include "amesh.h"
#include "hwmath.h"
#include "visrep.h"
#include "debug.h"
#include "oserrmsg.h"

////////////////////////////////////////////////////////////////////////////////
// rigid_body functions
////////////////////////////////////////////////////////////////////////////////

rigid_body::rigid_body( const entity_id& _id, unsigned int _flags )
  :   entity( _id, ENTITY_RIGID_BODY, _flags )
{
  initialize_variables();
}


rigid_body::rigid_body( const entity_id& _id,
                        entity_flavor_t _flavor,
                        unsigned int _flags )
  :   entity( _id, _flavor, _flags )
{
  initialize_variables();
}


rigid_body::~rigid_body()
{
}


void rigid_body::initialize_variables()
{
  c_o_m = ZEROVEC;
  I_x = I_y = I_z = 0;
}


///////////////////////////////////////////////////////////////////////////////
// NEWENT File I/O
///////////////////////////////////////////////////////////////////////////////

rigid_body::rigid_body( chunk_file& fs,
                        const entity_id& _id,
                        entity_flavor_t _flavor,
                        unsigned int _flags,
                        actor* _my_actor )
:   entity( fs, _id, _flavor, _flags )
{
  initialize_variables();
  set_flag( EFLAG_GRAPHICS, true );
  set_flag( EFLAG_GRAPHICS_VISIBLE, true );
  dinfo.initial_height = get_abs_position().y;
  my_actor = _my_actor;
}


///////////////////////////////////////////////////////////////////////////////
// Old File I/O
///////////////////////////////////////////////////////////////////////////////

// TODO: rationalize entity flags passed via constructor
rigid_body::rigid_body( const char* colgeom_fname,
                       actor* _my_actor,
                       bool _active )
:   entity( entity_id(), ENTITY_RIGID_BODY )
{
  set_active( _active );
  initialize_variables();
  colgeom = NEW cg_mesh(colgeom_fname, cg_mesh::ALLOW_WARNINGS);
  colgeom->set_owner(this);

  // For DEBUG <<<< ing purposes
  stringx fname = stringx(colgeom_fname);
  assert(fname.find( ".txtmesh" )!=string::npos);
  my_visrep = vr_pmesh_bank.new_instance(fname, 0);
  set_flag(EFLAG_GRAPHICS,true);
  set_flag(EFLAG_GRAPHICS_VISIBLE,true);
  dinfo.initial_height = get_abs_position().y;
  my_actor = _my_actor;
  colgeom->estimate_physical_properties(this);
}


///////////////////////////////////////////////////////////////////////////////
// Instancing
///////////////////////////////////////////////////////////////////////////////

entity* rigid_body::make_instance( const entity_id& _id,
                                   unsigned int _flags ) const
{
  rigid_body* newrb = NEW rigid_body( _id, _flags );
  newrb->copy_instance_data( *this );
  return (entity*)newrb;
}


void rigid_body::copy_instance_data( const rigid_body& b )
{
  entity::copy_instance_data( b );

  initialize_variables();
  dinfo.initial_height = get_abs_position().y;

  set_flag( EFLAG_GRAPHICS, true );
  set_flag( EFLAG_GRAPHICS_VISIBLE, true );
}


///////////////////////////////////////////////////////////////////////////////
// Misc.
///////////////////////////////////////////////////////////////////////////////

// frame_advance
//
// assumes a nontrivial time increment t
void rigid_body::frame_advance(time_value_t t)
{
  if ( is_active() )
  {
    entity::frame_advance(t);
    if(!get_parent())
    {
      po old_po = get_abs_po();

      // a free body will rotate about its center_of_mass.
      vector3d c = old_po.non_affine_slow_xform(c_o_m);
      vector3d new_c = my_po.non_affine_slow_xform(c_o_m);
      vector3d posn_offset = c-new_c;
      vector3d posn = get_abs_position();
      posn += posn_offset;

      my_po.set_rel_position(posn);
    }
  }
}



// apply_force_increment
//
// Applies a delta-t force increment of magnitude f at position loc in world space.
// This can be used for impulses as well as steady forces, because they are similar
// in a discrete world.  Continuous forces require a correction factor.
void rigid_body::apply_force_increment( const vector3d& f, force_type ft, const vector3d& loc, int mods )
{
  // Do the linear part
  // According to several reliable sources, we have been told that in fact
  // the amount of force applied to the c_o_m is proportional to the
  // cosine of the angle between the force vector and the vector from the
  // center of mass to the point the force is applied.  So we need to scale
  // what we pass to physical_entity::apply_force_increment by
  // dot(f, my_po.inverse_xform(loc)-c_o_m), likely normalized somehow. --Sean
  entity::apply_force_increment(f,ft,loc);

  if ( loc.x != IGNORE_LOC.x )
    {
    // Then the angular part
    // First do it relative to the LCS...
    vector3d rel_r = my_po.inverse_xform(loc)-c_o_m;
    vector3d rel_f = my_po.non_affine_inverse_xform(f);

    vector3d rel_torque = cross(rel_f,rel_r);  // left-handed coord system thing...

    // 7/5/2000 Sean
    // We need to preserve the radius part of the equation!  Avoid normalizing
    // the original rel_torque because we need to use it later.
    // However:  this hasn't been tested yet and it is likely to still have
    // bugs such as rel_torque not being quite the right length (perhaps need
    // to normalize rel_f before cross or something)

    vector3d rel_torque_norm = rel_torque;
    rel_torque_norm.normalize();

    rational_t eff_I = get_I_about_axis(rel_torque_norm);
    if ( eff_I == 0.0f )
    {
      #ifdef DEBUG
      debug_print( UF_PA, "Bug here." );
      #endif
      eff_I = 0.001f;
    }

    vector3d rel_ang_accel = rel_torque/eff_I;

    // Then flip it back to WCS and apply.
    vector3d ang_accel = my_po.non_affine_slow_xform(rel_ang_accel);
    vector3d angvel;
    get_angular_velocity(&angvel);
    set_angular_velocity(angvel + ang_accel);
  }
}


// dir should be normalized
rational_t rigid_body::get_effective_collision_mass( const vector3d& loc, const vector3d& dir ) const
{
  assert (__fabs(dir.length()-1)<.01f);

  vector3d r = my_po.inverse_xform(loc)-c_o_m;
  vector3d rel_dir = my_po.non_affine_inverse_xform(dir);
  vector3d axis = cross(r,rel_dir);
  rational_t eff_r_mag = axis.length();

  rational_t I;

  // normalize the axis, and compute I
  if (eff_r_mag>SMALL_DIST)
  {
    axis = axis/eff_r_mag;
    I = get_I_about_axis(axis);
  }
  else
  {
    I = 1;
  }

  rational_t effective_mass = 1/(1/get_mass() + eff_r_mag*eff_r_mag/I);
  return effective_mass;
}

void rigid_body::get_effective_collision_velocity( vector3d* target, const vector3d& loc ) const
{
  vector3d c_o_m = my_po.slow_xform(c_o_m);
  vector3d angvel;
  get_angular_velocity(&angvel);
  vector3d tan_vel = -cross(angvel,loc-c_o_m);
  vector3d vel;
  get_velocity(&vel);
  *target = vel + tan_vel;
}


// Returns an approximation to the moment of inertia of the mesh about axis.
// Strategy (descibed above also):
//    b) Compute an I about each principal axis through the c_o_m (I_x, I_y, I_z) and
//    then for a given axis v (v_x, v_y, v_z), estimate:
//        I_v = v_x^2*I_x + v_y^2*I_y + v_z^2*I_z
//
//  Assumes axis is normalized.
//
//  ACTUALLY:
//    It turns out that you can reprsent the moment of inertia over all axes by a "tensor"
//  which oyu simply apply to the axis to get the result.  Sweet.  Gotta see it ehrn its
//  implemented.  So, more to come here.
//
rational_t rigid_body::get_I_about_axis(vector3d axis) const
{
  return axis.x*axis.x*I_x + axis.y*axis.y*I_y + axis.z*axis.z*I_z;
}

extern FILE * debug_log_file;
rational_t rigid_body::compute_energy()
{
  vector3d vel;
  get_velocity(&vel);

  if (is_active())
  {
    rational_t delta_h;
    rational_t potential_energy = (delta_h = (get_abs_position().y-dinfo.initial_height))*GRAVITY*get_mass();
    if (!os_developer_options::inst()->get_gravity_enabled())
      potential_energy = 0;
    vector3d angvel;
    get_angular_velocity(&angvel);
    rational_t I = get_I_about_axis(angvel/angvel.length());
    rational_t angular_kinetic_energy = 0.5f*I*angvel.length2();
    rational_t linear_kinetic_energy = 0.5f*get_mass()*vel.length2();
    rational_t outval = linear_kinetic_energy + angular_kinetic_energy + potential_energy;
#if defined(TARGET_PC)
    if (debug_log_file)
      fprintf(debug_log_file," ke = %f, ake = %f, pe = %f, e = %f, h = %f, v = %f\n",
              linear_kinetic_energy,angular_kinetic_energy,potential_energy,
              outval,delta_h,vel.z);
#endif
    if (outval<0)
      outval = outval-1;
    return outval;
  }
  else
    return 0;
}

