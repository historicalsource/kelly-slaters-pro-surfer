#ifndef _PHYSICAL_INTERFACE_H_
#define _PHYSICAL_INTERFACE_H_

#include "global.h"
#include "ostimer.h"
#include "po.h"
#include "entity_interface.h"
#include "guidance_sys.h"
#include "FrontEndManager.h"

class guidance_system;

const vector3d IGNORE_LOC = vector3d(10e10f,10e10f,10e10f);
class physical_interface : public entity_interface
{
protected:
  unsigned int flags;

  rational_t mass;
  rational_t volume;

  vector3d velocity;
  vector3d angular_velocity;
  vector3d acceleration_factor;
  vector3d acceleration_correction_factor;
  vector3d last_acceleration_correction_factor;
  vector3d last_collision_normal;

  unsigned int collision_flags;

  bool stuck_parent_was_alive_last_frame;

  rational_t slide_factor;
  rational_t bounce_factor;
  rational_t sticky_offset;

  vector3d bounce_pos;
  vector3d bounce_norm;
  entity *bounce_ent;

  rational_t gravity_multiplier;

  void bounce(vector3d hit, vector3d hit_norm, entity *hit_entity = NULL);
  void do_physics(time_value_t t);

  guidance_system *guide_sys;

  rational_t last_elevation;
  rational_t cur_elevation;
  bool effectively_standing;
  vector3d cur_normal;
  rational_t last_floor_offset;

public:
  enum
  {
    _PHYS_ENABLED       = 0x00000001,
    _PHYS_SUSPENDED     = 0x00000002,

    _PHYS_GRAVITY       = 0x00000004,
    _PHYS_NO_INCREMENT  = 0x00000008,

    _PHYS_STANDING      = 0x00000010,
    _PHYS_COLLIDED      = 0x00000020,
    _PHYS_EXT_COLLIDED  = 0x00000040,
    _PHYS_BOUNCED       = 0x00000080,
    _PHYS_IMMOBILE      = 0x00000100,

    _PHYS_BOUNCY        = 0x10000000,
    _PHYS_STICKY        = 0x20000000,
    _PHYS_STICKY_ORIENT = 0x40000000,
    _PHYS_STUCK         = 0x80000000,

    _PHYS_FLAG_MASK     = 0xffffffff
  };

  physical_interface(entity *ent);
  virtual ~physical_interface();

  void set_flag(unsigned int flag, bool val = true)                                           { if(val) flags |= flag; else flags &= ~flag; }
  bool is_flagged(unsigned int flag) const                                                    { return((flags & flag) != 0); }

  void enable(bool val = true)                                                                { set_flag(_PHYS_ENABLED, val); }
  void disable()                                                                              { enable(false); }
  bool is_enabled() const                                                                     { return(is_flagged(_PHYS_ENABLED)); }
                                                                                              
  void suspend(bool val = true)                                                               { set_flag(_PHYS_SUSPENDED, val); }
  void unsuspend()                                                                            { suspend(false); }
  bool is_suspended() const                                                                   { return(is_flagged(_PHYS_SUSPENDED)); }
                                                                                                                                                                                            
  bool is_gravity() const                                                                     { return(is_flagged(_PHYS_GRAVITY)); }
  void set_gravity(bool val = true)                                                           { set_flag(_PHYS_GRAVITY, val); }
                                                                                              
  bool get_collided_last_frame() const                                                        { return(is_flagged(_PHYS_COLLIDED)); }
  void set_collided_last_frame( bool c )                                                      { set_flag(_PHYS_COLLIDED, c); }
                                                                                              
  bool get_ext_collided_last_frame() const                                                    { return(is_flagged(_PHYS_EXT_COLLIDED));}
  void set_ext_collided_last_frame( bool c )                                                  { set_flag(_PHYS_EXT_COLLIDED, c); }

  bool is_bouncy() const                                                                      { return(is_flagged(_PHYS_BOUNCY)); }
  void set_bouncy( bool c )                                                                   { set_flag(_PHYS_BOUNCY, c); }
                                                                                              
  bool is_sticky() const                                                                      { return(is_flagged(_PHYS_STICKY)); }
  void set_sticky( bool c )                                                                   { set_flag(_PHYS_STICKY, c); }
                                                                                              
  bool is_sticky_orient() const                                                               { return(is_flagged(_PHYS_STICKY_ORIENT)); }
  void set_sticky_orient( bool c )                                                            { set_flag(_PHYS_STICKY_ORIENT, c); }

  bool is_increment() const                                                                   { return(!is_flagged(_PHYS_NO_INCREMENT)); }
  void set_increment( bool c )                                                                { set_flag(_PHYS_NO_INCREMENT, !c); }
                                                                                              
  bool is_standing() const                                                                    { return(is_flagged(_PHYS_STANDING)); }
  void set_standing( bool c )                                                                 { set_flag(_PHYS_STANDING, c); }

  bool is_immobile() const                                                                    { return(is_flagged(_PHYS_IMMOBILE)); }
  void set_immobile( bool i )                                                                 { set_flag(_PHYS_IMMOBILE, i); }
                                                                                              
  bool is_stuck() const                                                                       { return(is_flagged(_PHYS_STUCK)); }
  bool has_bounced() const                                                                    { return(is_flagged(_PHYS_BOUNCED)); }

  bool is_effectively_standing() const                                                        { return(effectively_standing); }
                                                                                              
  rational_t get_mass() const                                                                 { return mass; }
  rational_t get_volume() const                                                               { return volume; }
  void set_mass( rational_t m )                                                               { mass = m; }
  void set_volume( rational_t v )                                                             { volume = v; }
  void set_I_vector( const vector3d& I )                                                      {}
  void set_c_o_m( const vector3d& c )                                                         {}

  rational_t get_bounce_factor() const                                                        { return bounce_factor; }
  rational_t get_slide_factor() const                                                         { return slide_factor; }
  rational_t get_sticky_offset() const                                                        { return sticky_offset; }
  void set_bounce_factor( rational_t m )                                                      { bounce_factor = m; }
  void set_slide_factor( rational_t v )                                                       { slide_factor = v; }
  void set_sticky_offset( rational_t v )                                                      { sticky_offset = v; }

  vector3d get_bounce_pos() const                                                             { return(bounce_pos); }
  vector3d get_bounce_norm() const                                                            { return(bounce_norm); }
  entity *get_bounce_ent() const                                                              { return(bounce_ent); }

  rational_t get_gravity_multiplier() const                                                   { return(gravity_multiplier); }
  void set_gravity_multiplier( rational_t g )                                                 { gravity_multiplier = g; }
  
  void set_velocity( const vector3d& v )                                                      { velocity = v; }
  void set_velocity_with_impact( const vector3d& v )                                          { set_velocity( v ); }
  void set_velocity_without_impact( const vector3d& v )                                       { set_velocity( v ); }
  void set_angular_velocity( const vector3d& v )                                              { angular_velocity = v;}
                                                                                              
  vector3d get_acceleration_factor() const                                                    { return acceleration_factor; }
  void set_acceleration_factor( const vector3d& v )                                           { acceleration_factor = v; }
  void add_acceleration_factor( const vector3d& v )	                                          { acceleration_factor += v; }
                                                                                              
  vector3d get_acceleration_correction_factor() const	                                        { return acceleration_correction_factor; }
  void set_acceleration_correction_factor( const vector3d& v, int mods=0 )	                  { acceleration_correction_factor = v; }
                                                                                              
  vector3d get_last_acceleration_correction_factor() const                                    { return last_acceleration_correction_factor; }
  void set_last_acceleration_correction_factor( const vector3d& v )                           { last_acceleration_correction_factor = v; }
                                                                                              
  vector3d get_velocity() const                                                               { return velocity; }
  vector3d get_angular_velocity() const                                                       { return angular_velocity; }
                                                                                              
  unsigned int get_collision_flags() const                                                    { return collision_flags; }
  void set_collision_flags(unsigned int cf)                                                   { collision_flags = cf; }
                                                                                              
  rational_t get_friction_scale() const                                                       { return(1.0f); }
                                                                                              
  vector3d get_last_collision_normal() const                                                  { return last_collision_normal;}
  void set_last_collision_normal( const vector3d& v );
                                                                                              
  rational_t get_effective_collision_mass( const vector3d& loc, const vector3d& dir ) const   { return get_mass(); }

  bool using_velocity() const;
  vector3d get_effective_collision_velocity( const vector3d& loc ) const;

  // Debug tools
  rational_t compute_energy()
  {
    assert( 0 );
    return 0;
  };

  virtual void frame_advance(time_value_t t);

  // used by frame_advance to add velocity, collisions to clip guys, etc.  handles BSP enforcement
  // returns succes or failure, and clips v appropriately
  bool add_position_increment( vector3d& v );

  void update_unused_velocity(time_value_t increment);

  // Note:  loc has no effect on physical_entity version of this function,
  // which assumes the force is applied to the center of mass.
  // Derived classes such as rigid_body can and do use this information
  // to accurately apply the force to a location other than the c_o_m.
  // Use of IGNORE_LOC dummy value for loc (instead of a separate flag)
  // yielded the most natural syntax for the caller.
  enum force_type
  {
    CONTINUOUS=0,
    INSTANT=1
  };
  virtual void apply_force_increment( const vector3d& f, force_type ft, const vector3d& loc = IGNORE_LOC, int mods=0 );


  virtual bool get_ifc_num(const pstring &att, rational_t &val);
  virtual bool set_ifc_num(const pstring &att, rational_t val);
  virtual bool get_ifc_vec(const pstring &att, vector3d &val);
  virtual bool set_ifc_vec(const pstring &att, const vector3d &val);


  void manage_standing(bool force = false);
  void compute_elevation();
  rational_t get_floor_offset();

  bool has_guidance_sys()         { return(guide_sys != NULL); }
  guidance_system *guidance_sys() { assert(guide_sys); return(guide_sys); }

  guidance_system *create_guidance_sys(guidance_system::eGuidanceSysType type);
  void destroy_guidance_sys();


  // Static collision Methods
  static void resolve_collision( entity * e1, entity * e2, time_value_t t, bool terrain_collision );
  static bool resolve_collision_with_terrain( entity * e1, time_value_t t, po const & velocity_po, const vector3d& abs_base_point );
};



#endif