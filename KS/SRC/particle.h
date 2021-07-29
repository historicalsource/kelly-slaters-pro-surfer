#ifndef PARTICLE_H
#define PARTICLE_H
////////////////////////////////////////////////////////////////////////////////
/*
  particle
*/
////////////////////////////////////////////////////////////////////////////////

//  i thought i told you to leave me while i walked down to the beach tell me how does it feel when your heart grows cold
//#include "physent.h"
#include "entity.h"

////////////////////////////////////////////////////////////////////////////////
class particle_generator;
class particle;

class particle_force
{
public:
  enum force_flavor_t
  {
    CONSTANT,
    RESISTANCE,
    TOWARDS_POINT,
  };
  particle_force() 
  {}
  particle_force( force_flavor_t _flavor, 
                  const vector3d& _multipurpose, 
                  rational_t _delta_speed,
                  rational_t _terminal_speed ) 
    : flavor(_flavor),
      multipurpose(_multipurpose),
      delta_speed(_delta_speed),
      terminal_speed(_terminal_speed),
      recip_terminal_speed(1/_terminal_speed) 
  {
    assert(terminal_speed>=0); // so the above recip is kinda wierd
  }
  vector3d get_new_v( const particle& _particle, particle_generator& _pg, time_value_t time_delta );

  force_flavor_t flavor;
  vector3d       multipurpose;
  vector3d       utility;
  rational_t     delta_speed;
  rational_t     terminal_speed;
  rational_t     recip_terminal_speed;
};
  
////////////////////////////////////////////////////////////////////////////////
class particle
{
private:
  vector3d my_pos;  
  vector3d my_velocity;
  vector3d rot_axis;  // we could cheapen this by using a parallel array only
                      // for particles with rotation
  rational_t scale;
  rational_t rotation;
  rational_t rotation_speed;
  time_value_t life_remaining;
  time_value_t total_life;

  friend class particle_generator;
  friend class particle_force;
};

class particle_generator : public entity
{
public:
  particle_generator( const entity_id& _id, unsigned int _flags );
  particle_generator( const entity_id& _id = ANONYMOUS,
                      entity_flavor_t _flavor = ENTITY_PARTICLE_GENERATOR,
                      unsigned int _flags = 0 );
  virtual ~particle_generator();

  void initialize_variables();

  bool always_render;

/////////////////////////////////////////////////////////////////////////////
// entity class identification
public:
  virtual bool is_a_particle_generator() const { return true; }
  virtual bool get_distance_fade_ok() const { return false; } // don't fade out at far plane

// File I/O
public:
  particle_generator( const stringx& _filename,
                      const entity_id& _id = ANONYMOUS,
                      entity_flavor_t _flavor = ENTITY_PARTICLE_GENERATOR,
                      unsigned int _flags = 0 );

  void load();

// Instancing
public:
  virtual entity* make_instance( const entity_id& _id,
                                 unsigned int _flags ) const;
protected:
  void copy_instance_data( const particle_generator& b );

// entity_maker caching interface
public:
  virtual void acquire( unsigned int _flags );

// Misc.
public:
  virtual void frame_advance(time_value_t t);
  virtual void render(camera* camera_link, rational_t detail, render_flavor_t flavor, rational_t entity_translucency_pct);

  virtual void set_created_entity_default_active_status();

  virtual vector3d get_visual_center() const;
  virtual rational_t get_visual_radius() const;
  virtual rational_t get_visual_xz_radius_rel_center() const;

  virtual void po_changed();

  virtual void set_visible( bool a );

  virtual bool is_still_visible() const;  // like is_visible, but when you disappear
  // a particle generator there will still be some lingering particles.  is_still_visible
  // remains true until the last particle is gone.
  virtual render_flavor_t render_passes_needed() const;

  virtual bool is_active() const { return is_visible(); }
  bool is_on() const;

  enum
  {
    GENERATOR_ORIENTED      = 0x00000001,
    DIRECTION_ORIENTED      = 0x00000002,
    ASYNC_ANIMS             = 0x00000004,
    LIFETIME_ANIMS          = 0x00000008,
    LOCAL_SPACE             = 0x00000010,
    PARTICLE_BSP_COLLISIONS = 0x00000020
  };

  // for use by the particle_cleaner
  void destroy_particles();

  virtual vector3d get_last_position() const { return last_position_valid? last_position : get_abs_position(); }
  virtual void frame_done() { last_position = get_abs_position(); }

  virtual bool possibly_active() const    { return(is_still_visible()); }
  virtual bool possibly_aging() const     { return(is_still_visible()); }

#if _ENABLE_WORLD_EDITOR
  stringx dir_name;
  stringx tool_visrep_name;
  void reset_sys(bool full = false);
  bool write(const stringx &file);
#endif


private:
  bool             disable_for_sony_booth;

  stringx          filename;
  particle*        particles;
  int              start_particle;
  int              end_particle;
  int              max_particles;
  time_value_t     particle_life_span;
  rational_t       life_variation;
  int              birthrate;
  rational_t       rate_variation;  // from 0.0f to 1.0f
  rational_t       base_speed;
  rational_t       speed_variation;  // from 0.0f to 1.0f
  rational_t       spread_off_axis;
  rational_t       spread_off_plane;
  rational_t       scale_variation;  // from 0.0f to 1.0f
  time_value_t     grow_for;
  time_value_t     shrink_for;
  time_value_t     fade_in;
  time_value_t     fade_out;
  rational_t       motion_inheritance;  // from 0.0f to 1.0f
  rational_t       generation_radius;  
  rational_t       generation_height;
  time_value_t     rotation_period;    // assumes random axis for now
  rational_t       recip_rotation_period;
  rational_t       rotational_speed_variation;
  rational_t       rotational_start_variation;

  time_value_t     on_for;
  time_value_t     off_for;
  unsigned         flags;

  time_value_t     time_to_next_particle;

  rational_t       rh_grow_for;
  rational_t       rh_shrink_for;
  rational_t       rh_fade_in;
  rational_t       rh_fade_out;

  vector<particle_force> force_list;

  //vector<particle_generator*> *pool;

  vector3d         abs_visual_center;
  vector3d		     visual_center;
  rational_t		   visual_radius;

  vector3d last_position;
  bool last_position_valid;
  
  rational_t       get_base_visual_radius() const;

  friend class particle_force;
  friend class ParticleDialog;
};


#endif
