#ifndef VISREP_H
#define VISREP_H

#include "color.h"
#include "po.h"

class matrix4x4;

// delta_t is always time since the visual_rep first started going
// some visual_reps--animations that aren't supposed to loop, for example--
// have ending times.  Those that don't have an ending time of END_OF_TIME.

#include "ostimer.h"
#include "hwmath.h"
#include "renderflav.h"

// this bugs:
enum visrep_t
{
  VISREP_PMESH, 
  VISREP_BILLBOARD,
  VISREP_DROPSHADOW,
  VISREP_HIGHLIGHT = VISREP_DROPSHADOW,
  VISREP_KRMESH
};

class instance_render_info;



// to expound further on clipping:
// normal for entities and terrain is to cull polys outside of view frustum, and to scissor polys that cross front
//   exactly right
// one optimization, if you know most of the entity is inside your view, is to just do the front clip (FORCE_JUST_FRONT_CLIP)
// if you know the entity is clear of the front, but think it might be mostly outside your view, you can skip the front clip
//   (FORCE_SKIP_FRONT_CLIP)
// if the entity is one you don't particularly care how good it looks when it crosses the front clip plane (character variant
//   decorations) then you can do a TRIVIAL_FRONT_CLIP to make sure it doesn't hash the frame buffer:  this is how skins
//   always render
// if you know the entity is mostly in the view AND clear of the front clip plane, skip everything:  FORCE_SKIP_CLIP
// there, did I get everything?
// I put these flags in the .h file just for completeness sake:  I may or may not get to implementing all of them.  Definitely
//  implemented are FORCE_SKIP_CLIP and FORCE_TRIVIAL_FRONT_CLIP


enum light_method_t
{
  LIGHT_METHOD_DIFFUSE = 0,                 // standard lighting :  diffuse * ambient_factor + diffuse * lighting -> diffuse slot
  LIGHT_METHOD_ADDITIVE_DYNAMIC_ONLY = 1    // terrain/terrain entity lighting:  diffuse untouched.  lighting -> specular/offset/additive slot
};

class visual_rep
{
public:

  visual_rep(visrep_t _type, bool _instanced = true);

  virtual ~visual_rep()
  {}

	virtual void render_instance(render_flavor_t render_flavor, 
                               instance_render_info* iri,
                               short *ifl_lookup = NULL) = 0;

  virtual void render_batch(render_flavor_t flavor,
                            instance_render_info* viri,
                            int num_instances);

  // only vr_pmesh has a skin option
  virtual void render_skin(render_flavor_t render_flavor,
                           const instance_render_info* iri, 
                           const po* bones,
                           int num_bones);
  
  virtual int get_min_faces(time_value_t delta_t=0) const;
  virtual int get_max_faces(time_value_t delta_t=0) const;

  rational_t get_min_detail_distance() const { return min_detail_dist; }
  rational_t get_max_detail_distance() const { return max_detail_dist; }
  void       set_min_detail_distance(rational_t mdd) { min_detail_dist=mdd; }
  void       set_max_detail_distance(rational_t mdd) { max_detail_dist=mdd; }

  virtual time_value_t get_ending_time() const;
  virtual float time_value_to_frame(time_value_t t);

  virtual rational_t get_radius(time_value_t delta_t=0) const = 0;
  virtual rational_t compute_xz_radius_rel_center(const po& xform);

  virtual const vector3d& get_center(time_value_t delta_t=0) const = 0;
  
  virtual bool kill_me();
  
  virtual void set_light_method(light_method_t lmt);

  visrep_t get_type() const { return type; }

  bool is_instanced() const { return instanced; }
  void set_instanced(bool i) { instanced = i; }

  virtual void set_distance_fade_ok(bool v);
  virtual bool get_distance_fade_ok() const;

  void set_distance_fade_min_pct(rational_t v) {}
  rational_t get_distance_fade_min_pct() const { return 0; }
  void set_distance_fade_max_pct(rational_t v) {}
  rational_t get_distance_fade_max_pct() const { return 0.07f; }

private:
  visrep_t type;
  rational_t min_detail_dist;  // distance above which visrep is at minimum detail
  rational_t max_detail_dist;  // distance below which visrep is at maximum detail
  bool instanced;

public:
  virtual int get_anim_length() const;
  virtual bool is_uv_animated() const;

  virtual render_flavor_t render_passes_needed() const;
};

// for loading visual_reps at the beginning of the level:
visual_rep* load_new_visual_rep( chunk_file& fs,
                                 const stringx& visrep_name,
                                 visrep_t _flavor, 
                                 unsigned additional_flags );
visual_rep* load_new_visual_rep( const stringx& visrep_name, unsigned additional_flags );
visual_rep* new_visrep_instance( visual_rep* vrep );
void        unload_visual_rep( visual_rep* discard );

// for making a noninstanced copy of a visrep.
visual_rep* new_visrep_copy( visual_rep* vrep );

// for finding visual_reps mid-game, where you aren't allowed to
// do disk access.  This function fails if the visual_rep hasn't
// been loaded.
visual_rep* find_visual_rep( const stringx& visrep_name );

// very useful little tidbit for converting a name to a type
visrep_t visual_rep_name_to_type( const stringx& visrep_name );

#endif