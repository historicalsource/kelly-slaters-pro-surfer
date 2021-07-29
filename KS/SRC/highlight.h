#ifndef HIGHLIGHT_H
#define HIGHLIGHT_H

#include "visrep.h"
//#include "material.h"

extern vector3d global_ZEROVEC;

class highlight : public visual_rep, public singleton
{
public:
  highlight();
  virtual ~highlight();

  virtual void render_instance( render_flavor_t flavor, instance_render_info *iri );

  virtual rational_t get_radius( time_value_t delta_t ) const { return 1.0f;}
  virtual const vector3d& get_center( time_value_t delta_t ) const { return global_ZEROVEC;}

  void rotate( rational_t delta_t );

  void purge();
  void reload();

  void set_current_spin_angle( rational_t spin_angle ) { current_spin_angle = spin_angle;}
  void add_current_spin_angle_increment( rational_t spin_increment );
  rational_t get_current_spin_angle() const { return current_spin_angle;}

  DECLARE_SINGLETON( highlight )

protected:
  material* my_material;
  rational_t  current_spin_angle;
};

#endif
