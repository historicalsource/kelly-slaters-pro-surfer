#ifndef BILLBOARD_H
#define BILLBOARD_H
////////////////////////////////////////////////////////////////////////////////

// billboard.h
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.

// billboard sprites;  sprites that you put in the world

////////////////////////////////////////////////////////////////////////////////
#include "visrep.h"
#include "instance.h"
//#include "material.h"
#include "rect.h"
#include "matfac.h"

class vr_billboard : public visual_rep
{
public:
  vr_billboard();
  virtual ~vr_billboard();

  vr_billboard( chunk_file& fs, bool _instanced );
  vr_billboard( const stringx& fname, unsigned );

  static void flush(); // flush buffered rendering at end of frame

  void internal_serial_in( chunk_file& fs );

  enum flags_t
  {
    AXIS_LOCK            =0x01, // enables axis locking
    DISABLE_PERSPECTIVE  =0x02,
    FORCE_NEAR           =0x04, // enables use of z_push_factor
    AXIS_LOCK_POS        =0x08, // uses direction from camera to instance instead of camera facing
    AXIS_LOCK_HALFPOS    =0x10, // uses blend between with and without AXIS_LOCK_POS
    AXIS_LOCK_LOCAL      =0x20, // axis is specified in local space for each instance instead of world space
    AXIS_LOCK_PERSPECTIVE=0x40, // draws billboard along axis with perspective
  };

#ifdef NGL_PS2
  virtual void render_instance( nglMesh *mesh, int *num_quads,
                                render_flavor_t flavor,
                                instance_render_info* iri,
                                short *ifl_lookup = NULL);
  virtual void render_batch(nglMesh *mesh, int *num_quads,
                            render_flavor_t flavor,
                            instance_render_info* viri,
                            int num_instances,
                            short *ifl_lookup = NULL);
#endif
  virtual void render_instance(render_flavor_t flavor,
                               instance_render_info* iri,
                               short *ifl_lookup = NULL);
  virtual void render_batch(render_flavor_t flavor,
                            instance_render_info* viri,
                            int num_instances,
                            short *ifl_lookup = NULL);

  virtual const vector3d& get_center( time_value_t delta_t ) const;
  virtual rational_t get_radius(time_value_t delta_t) const;
  virtual rational_t compute_xz_radius_rel_center( const po& xform );

  virtual time_value_t get_ending_time() const;

  virtual float time_value_to_frame(time_value_t t);

	void set_width(rational_t w) { width = w; }
	void set_height(rational_t h) { height = h; }
	void set_z_push_factor(rational_t z) { z_push_factor = z; }
  rational_t get_width() const { return width; }
  rational_t get_height() const { return height; }
  rational_t get_z_push_factor() const { return z_push_factor; }

  int get_billboard_flags() const { return flags; }
  void set_billboard_flags(int fl) { flags = fl; }
  const mat_fac& get_mat() { return my_material; }
  mat_fac& get_alterable_mat() { return my_material; }

  const vector3d &get_axis_lock() const { return(axis_lock); } // axis it rotates around

protected:
//  po start_po;        // member rather than leaving on stack for 8-byte alignment
	mat_fac my_material;
  vector3d axis_lock; // axis it rotates around
  rational_t width;
  rational_t height;
  rational_t z_push_factor; // how much to move it toward the camera (only affects zbuffer, and only true billboards with FORCE_NEAR flag set)
  unsigned flags;

public:
  virtual int get_anim_length() const;

  virtual render_flavor_t render_passes_needed() const;
};


extern instance_bank<vr_billboard> vr_billboard_bank;

#endif
