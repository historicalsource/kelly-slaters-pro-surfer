#ifndef IRI_H
#define IRI_H

#include "frame_info.h"

class region;
class portal;
typedef graph<stringx,region*,portal*> region_graph;
typedef region_graph::node region_node;
class light_manager;

#include "forceflags.h"

class instance_render_info
{
public:
  instance_render_info() 
    : render_scale(1.0f, 1.0f, 1.0f)
  { 
    alt_materials = NULL; 
  }

  instance_render_info(
          float _number_of_faces_to_attempt, 
          const po & _local_to_world, 
          time_value_t _age,
          region_node* _my_region,
          unsigned short _ifl_frame_boost = 0,
          color32 _color_scale = color32(255,255,255,255),
          unsigned int _force_flags = 0,
          rational_t _camera_relative_rotation = 0,
          float _particle_scale = 1.0f,
          light_manager* _light_manager = NULL,
          int _locked_frame = -1, //-1 means no frame is locked by default.
          vector<material*> *_alt_materials = NULL
          )
      : number_of_faces_to_attempt( _number_of_faces_to_attempt ),
        local_to_world( _local_to_world ),
        force_flags(_force_flags),
        color_scale( _color_scale ),
        camera_relative_rotation(_camera_relative_rotation),
        my_region( _my_region ),
        //      ifl_frame_boost( _ifl_frame_boost ),
        particle_scale(_particle_scale),
        my_light_set(_light_manager),
        alt_materials( _alt_materials ),
        render_scale(1.0f, 1.0f, 1.0f)
  {
    frame_time_info.set_age(_age);
    frame_time_info.set_ifl_frame_boost(_ifl_frame_boost);
    frame_time_info.set_ifl_frame_locked(_locked_frame);
  }

  instance_render_info(
          float _number_of_faces_to_attempt, 
          const po & _local_to_world, 
          class frame_info &_frame_time_info,
          region_node* _my_region,
          color32 _color_scale = color32(255,255,255,255),
          unsigned int _force_flags = 0,
          rational_t _camera_relative_rotation = 0,
          float _particle_scale = 1.0f,
          light_manager* _light_manager = NULL,
          vector<material*> *_alt_materials = NULL
        )
      : number_of_faces_to_attempt( _number_of_faces_to_attempt ),
        local_to_world( _local_to_world ),
        force_flags(_force_flags),
        color_scale( _color_scale ),
        camera_relative_rotation(_camera_relative_rotation),
        frame_time_info(_frame_time_info),
        my_region( _my_region ),
        particle_scale(_particle_scale),
        my_light_set(_light_manager),
        alt_materials( _alt_materials ),
        render_scale(1.0f, 1.0f, 1.0f)
  {}

  int                     time_to_frame_locked (int period) const { return frame_time_info.time_to_frame_locked(period);}
  float                   get_target_face_count() const {return number_of_faces_to_attempt;}
  const po&               get_local_to_world() const {return local_to_world;}
  const color32&          get_color_scale() const {return color_scale;}
  rational_t              get_camera_relative_rotation() const {return camera_relative_rotation;}
  time_value_t            get_age() const {return frame_time_info.get_age();}
  void                    set_age(time_value_t age) {frame_time_info.set_age(age);}
  unsigned short          get_ifl_frame_boost() { return frame_time_info.get_ifl_frame_boost(); }
  void                    set_ifl_frame_boost(int boost) {frame_time_info.set_ifl_frame_boost(boost); }
    
  bool                    force_translucent() const { return force_flags & FORCE_TRANSLUCENCY; }
  bool                    force_unzbuffered() const { return force_flags & FORCE_UNZBUFFERED; }
  unsigned int            get_force_flags() const { return force_flags; }
  region_node             *get_region() const {return my_region;}
  bool			              get_skip_clip() const {return force_flags & FORCE_SKIP_CLIP;}
  light_manager           *get_light_set() const {return my_light_set;}
  float                   get_particle_scale() const {return particle_scale;}
  bool                    operator<(const instance_render_info& iri) const { return frame < iri.frame;}

  vector<material*>       *get_alt_materials() const { return alt_materials;}
  void                    set_alt_materials( vector<material*> *_alt_materials ) { alt_materials = _alt_materials;}

  const vector3d          &get_render_scale() const { return(render_scale); }
  void                    set_render_scale(const vector3d &s) { render_scale = s; }


public:

  float                   number_of_faces_to_attempt;
  po                      local_to_world;              // keep 8-byte aligned: sizeof has to be divisible by 8, as well
  unsigned/* short*/      force_flags;
  color32                 color_scale;
  rational_t              camera_relative_rotation;
  class frame_info        frame_time_info;
  region_node             *my_region;     // for lighting, ambient and directional
  rational_t              particle_scale; // kludge for particles
  //unsigned short          ifl_frame_boost;
  light_manager           *my_light_set;  // if NULL, sorts lights itself
  vector<material*>       *alt_materials;
  int                     frame;
  vector3d                render_scale;

  //Information for frame locking at renderering.
  instance_render_info& operator = (const instance_render_info &new_guy) 
  {
    if (&new_guy == this)
      return *this;

    // copy all member data
    number_of_faces_to_attempt = new_guy.number_of_faces_to_attempt;
    local_to_world = new_guy.local_to_world;
    force_flags = new_guy.force_flags;
    color_scale = new_guy.color_scale;
    camera_relative_rotation = new_guy.camera_relative_rotation;
    frame_time_info = new_guy.frame_time_info;
    my_region = new_guy.my_region;
    particle_scale = new_guy.particle_scale;
    my_light_set = new_guy.my_light_set;  // if NULL, sorts lights itself
    alt_materials = new_guy.alt_materials;
    frame = new_guy.frame;
    render_scale = new_guy.render_scale;

    return *this;
  }
    

};

#endif
