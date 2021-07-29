#ifndef DROPSHADOW_H
#define DROPSHADOW_H

#include "visrep.h"
//#include "material.h"

// the dropshadow is a transparent square with a dark
// translucent circle in the middle.  It's your responsibility
// to orient and scale it correctly.  Its identity orientation is
// flat.

extern vector3d global_ZEROVEC;

class vr_dropshadow : public visual_rep, public singleton
{
public:
  vr_dropshadow();
  virtual ~vr_dropshadow();

  virtual void render_instance(render_flavor_t flavor,                             
                            instance_render_info* iri,
                            short *ifl_lookup = NULL);

  virtual rational_t get_radius(time_value_t delta_t) const { return 1.0f; }
  virtual const vector3d& get_center( time_value_t delta_t ) const { return global_ZEROVEC; }

  void purge();
  void reload();

  DECLARE_SINGLETON(vr_dropshadow)

protected:
  material* my_material;
};

#endif
