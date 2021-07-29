#ifndef WIDGET_ENTITY_H
#define WIDGET_ENTITY_H

#include "widget.h"

//-------------------------------------------------------------------------------------

#if 1 //ndef __MSL_STL
typedef vector<entity_anim_tree*> pentity_anim_tree_vector;
#else

typedef simplestaticallocator<entity_anim_tree *> pentity_anim_tree_allocator;
typedef vector<entity_anim_tree*,pentity_anim_tree_allocator> pentity_anim_tree_vector;
#endif




class entity;

class entity_widget: public widget
{
public:
  entity_widget( const char *_widget_name, widget *_parent, short _x, short _y, const char *ent_file = NULL );
  virtual ~entity_widget();

  virtual void show();

  void add_anim( entity_anim_tree* new_anim );
  void kill_anim( entity_anim_tree* the_anim );

  void frame_advance_entity( entity *e, time_value_t time_inc );
  virtual void frame_advance( time_value_t time_inc );
  void render_entity( entity *e, camera* camera_link );
  virtual void render( camera* camera_link );

  virtual void update_pos();
  virtual void update_scale();
  virtual void update_rot();

  virtual void rotate_to(time_value_t wt, time_value_t d, rational_t a, vector3d _axis) { axis = _axis; rotate_wevent *e; e = NEW rotate_wevent( this, wt, d, a ); add_wevent( e ); }
  void set_rotation( rational_t _ax, rational_t _ay, rational_t _az );
  void set_rotation(vector3d &u, rational_t a, rational_t s) { axis = u; angle = a; rps = s; }
  void update_entity_po();

  void set_ent( entity *_ent );
  entity *get_ent() const { return ent; }

  rational_t   get_ax() const { return ax; }
  rational_t   get_ay() const { return ay; }
  rational_t   get_az() const { return az; }
  virtual rational_t    get_width() { return get_ent()->get_radius(); }
  virtual rational_t    get_height() { return get_ent()->get_radius(); }

	#ifdef WIDGETCULL
  typedef vector<entity_anim_tree*> anim_list;
	#endif

  virtual void set_layer( rhw_layer_e rhw_layer );

protected:
  void calc_largest_z( entity *e );
  entity *ent;
  pentity_anim_tree_vector anims;
  vector3d axis;          // axis of rotation
  rational_t ax, ay, az;  // specific axis angle in radians
  rational_t   rps;       // radians per second

  rational_t   rhw_midpoint;           // midpoint in rhw layer
  rational_t   z;                      // for pc sorting
  rational_t   rhw_half_range;         // half of range of possible rhw values
  matrix4x4    rot_matrix;
  rational_t   largest_z;              // we need one for whole conglom for sorting in pmesh.cpp
};

#endif // WIDGET_ENTITY_H
