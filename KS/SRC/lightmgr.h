// lightmgr.h
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.

#ifndef LIGHTMGR_H
#define LIGHTMGR_H

// each independently lit object has its own light manager, which is used for all its parts
// there's also a global static one that is only used for dynamic lights on terrain

#if defined(TARGET_XBOX)
#include "global.h"
#include "xbglobals.h"
#endif /* TARGET_XBOX JIV DEBUG */


#include "algebra.h"
#include "sphere.h"
#include "light.h"
#include "refptr.h"
#include "hwrasterize.h"

class region;
class use_light_context;

class light_manager : public ref
{
public:
  class light_rec
  {
  public:
    light_source* source; // if null, it's not around anymore and should be faded out and replaced
    vector3d dir_or_pos;  // direction or position, depending on kind of light
    light_properties props; // we can't just use light_source->props because some lights go away (but in that case should we really be rendering them?)
    float current_intensity; // for light transitions (0 to 1)
    light_rec()
      : source(NULL)
      , current_intensity(0.0F)
    {}
  };
	#ifdef TARGET_PS2
  typedef vector<light_rec,malloc_alloc> light_list_t;
	#else
  typedef vector<light_rec> light_list_t;
	#endif
  light_list_t lights;

  color last_ambient;
  color goal_ambient;
  color my_ambient;

  unsigned max_lights;  // number of lights I get to use
  unsigned cur_max_lights;  // adjusted for framerate etc

  sphere bound;      // bounding sphere of object

  bool allow_omni;   // true for terrain, false for entities
  bool dynamic_only; // true for terrain, false for entities

  explicit light_manager(bool is_terrain=false)
    : last_ambient(1.0f,1.0f,1.0f,1.0f)
    , goal_ambient(1.0f,1.0f,1.0f,1.0f)
    , my_ambient(1.0f,1.0f,1.0f,1.0f)
    , max_lights(1)
    , allow_omni(is_terrain)
    , dynamic_only(is_terrain)
  {}

  void set_bound_sphere(const sphere& new_bound) { bound = new_bound; }
  const sphere& get_bound_sphere() const { return bound; }

  // for entities, pass elapsed time.  For terrain, pass a large time such as 10.0f
  void frame_advance(region* reg, time_value_t t, const int playerID );

  void prepare_for_rendering(use_light_context *lites);

  static light_manager* get_static_light_set();

  #ifdef DEBUG
  void dump_debug_info() const;
  #endif

protected:
  int compare_light(int sli);
};



#define ABSOLUTE_MAX_LIGHTS 3
#define MAX_POINT_LIGHTS ((unsigned)2)



class use_light_context
{
public:
  struct point_light
  {
    vector3d pos;
    light_source* light;
    point_light() {}
    point_light(const vector3d& _pos, light_source* ls) : pos(_pos), light(ls) {}
  };
  struct dir_light
  {
    vector3d dir;
    light_source* light;
    dir_light() {}
    dir_light(const vector3d& _dir, light_source* ls)
      : dir(_dir), light(ls)
    {}
  };

  vector<point_light> point_lights;
  vector<light_properties> point_light_props;
  vector<dir_light> dir_lights;
  vector<light_properties> dir_light_props;
  color ambient_factor;
  typedef pair<float,int> light_dist;
  vector<light_dist> point_light_dists;

  use_light_context()
    : point_lights(ABSOLUTE_MAX_LIGHTS)
    , point_light_props(ABSOLUTE_MAX_LIGHTS)
    , dir_lights(ABSOLUTE_MAX_LIGHTS)
    , dir_light_props(ABSOLUTE_MAX_LIGHTS)
  {
  }

  void clear_lights();

  void xform_lights_to_local(const po& world2local, int num_bones, render_flavor_t render_flavor);
  void transform_lights_to_bone_space(const vector3d& pos, const matrix4x4* bones_world, int num_bones);

  bool empty() const
  {
    return dir_lights.empty() && point_light_dists.empty();
  }
};

//extern use_light_context lites;


enum light_type_flavor
{
  ltfPoint=0,
  ltfDir=1,
  ltfNone=-1
};

class light_types
{
  public:
	  signed char lt[4];  // light_type_flavor
};

enum
{
  LIGHT_DIFFUSE =1,      // diffuse and ambient
  LIGHT_ADDITIVE=2       // additive and self-lit
};


// In the below, the only thing the light_props is used for is for the near range.
// All the color stuff is already in the lighting matrices

void c_onelight(hw_rasta_vert* src_vert_list,
              int count,
              light_properties* light_info,
              unsigned flags,
              hw_rasta_vert_lit* dest_vert_list,
              color32 ambientc
              );
void sweetlight(hw_rasta_vert* src_vert_list,
              int count,
              vector3d lighting_table[][ABSOLUTE_MAX_LIGHTS],
              light_properties light_props[ABSOLUTE_MAX_LIGHTS],
              int num_dir_lights,
              int num_point_lights,
              unsigned flags,
              use_light_context::light_dist* my_light_dists,
              hw_rasta_vert_lit* dest_vert_list,
              int alpha,    // obsolete
              int num_bones
              );
void c_sweetlight_inner(hw_rasta_vert* src_vert_list,
              hw_rasta_vert_lit* dest_vert_list,
              matrix4x4* lighting_matrices,  // note:  the first lighting_matrix is the diffuse lighting table, the second is specular, and
                                             // from then on you have the light vector matrices for each bone
              int count,
              vector3d lighting_table[][ABSOLUTE_MAX_LIGHTS],
              light_properties light_props[ABSOLUTE_MAX_LIGHTS],
              light_types lt,
              unsigned flags );

#ifdef TARGET_MKS
extern "C" {
class pmesh_normal;
void asm_onelight(pmesh_normal* normal_list,
              hw_rasta_vert* src_vert_list,
              int count,
              matrix4x4* light_mats,
              color32 ambientc,
              float light_red,
              float light_green,
              float light_blue
              );
void asm_sweetlight_inner(pmesh_normal* normal_list,
                          hw_rasta_vert* src_vert_list,
                          hw_rasta_vert* dest_vert_list,
                          matrix4x4* lighting_matrices,  // note:  the first lighting_matrix is the diffuse lighting table, the second is specular, and 
                                                         // from then on you have the light vector matrices for each bone
                          int count,
                          vector3d lighting_table[][ABSOLUTE_MAX_LIGHTS],
                          light_properties light_props[ABSOLUTE_MAX_LIGHTS],
                          light_types lt,
                          unsigned flags,
                          int alpha );
void asm_sweetlight_inner_dironly(pmesh_normal* normal_list,
                          hw_rasta_vert* src_vert_list,
                          hw_rasta_vert* dest_vert_list,
                          matrix4x4* lighting_matrices,  // note:  the first lighting_matrix is the diffuse lighting table, the second is specular, and 
                                                         // from then on you have the light vector matrices for each bone
                          int count,
                          vector3d lighting_table[][ABSOLUTE_MAX_LIGHTS],
                          light_properties light_props[ABSOLUTE_MAX_LIGHTS],
                          light_types lt,
                          unsigned flags,
                          int alpha );
};
#endif

void prepare_lighting_matrices( color ambient_color,
                                light_properties* light_color_table,
                                int num_dir_lights,
                                int num_point_lights,
                                unsigned flags,
                                int num_bones );


void initialize_lighting_matrices();

extern matrix4x4* light_matrices;


#endif // LIGHTMGR_H
