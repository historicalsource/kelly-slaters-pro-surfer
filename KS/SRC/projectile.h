#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "ostimer.h"
//#include "physent.h"

enum projectile_phase_t {
	PHASE_Launch,
	PHASE_Travel,
	PHASE_Hit,
	PHASE_Dead
};

enum projectile_hit_t {
	HIT_None,
	HIT_Character,
	HIT_Terrain,
	HIT_Fizzle
};

class projectile;

typedef pair<entity *,rational_t>                 range_t;
typedef list<range_t>                             range_list_t;
typedef list<range_t>::iterator                   range_iterator_t;
typedef list<range_t>::const_iterator             const_range_iterator_t;
typedef list<projectile *>                        projectile_list_t;
typedef list<projectile *>::iterator              projectile_iterator_t;
typedef list<projectile *>::const_iterator        const_projectile_iterator_t;
typedef list<entity *>                            entity_list_t;
typedef list<entity *>::iterator                  entity_iterator_t;
typedef list<entity *>::const_iterator            const_entity_iterator_t;
typedef list<script_object::instance *>           instance_list_t;
typedef list<script_object::instance *>::iterator instance_iterator_t;

class ranged_attack 
{
public:
	ranged_attack( entity *a, const stringx& script_object, const stringx& script_func);
	projectile *create_projectile(stringx entity_name);
	entity     *get_attacker() const { return attacker; }
	int         get_id() const { return id; }
private:
	void remove_projectile(projectile *p);
	entity                   *attacker;
	projectile_list_t        projectile_list;
	instance_list_t          instance_list;
	// script to be played immediately
	script_object           *cast_so;
	script_object::instance *cast_inst;
	int                      cast_func;
	vm_thread               *cast_vm;
	int                      id;
	static int               next_id;
	friend class projectile; // so that projectiles can remove themselves from the projectile_list
};

#define PROJECTILE_AutoTarget 0x00000001  // projectile will adjust initial vector to hit target
#define PROJECTILE_Seek       0x00000002  // projectile will adjust "in flight" vector to hit target

class projectile : public physical_entity
{
public:
  projectile( const entity_id& _id, unsigned int _flags );
  projectile( const stringx& entity_fname, const entity_id& _id = ANONYMOUS );
  ~projectile();

private:
  void initialize_variables();
  void compute_colgeom();
  void read_info();
  void set_parent( ranged_attack* p );

// Instancing
public:
  virtual entity* make_instance( const entity_id& _id,
                                 unsigned int _flags ) const;
protected:
  void copy_instance_data( const projectile& b );

/////////////////////////////////////////////////////////////////////////////
// entity class identification
public:
  virtual bool is_a_projectile() const { return true; }

// Miscellaneous
public:
  ranged_attack* get_parent() const { return parent; }
  void launch( entity* s, entity* t );
  void frame_advance( time_value_t t );
  bool set_launch_vector();
  void orient();
  void collide( time_value_t t );

private:
  ranged_attack* parent;
  unsigned int flags;
  entity* source;
  entity* target;
  projectile_hit_t hit;
  entity* hit_entity;
  rational_t launch_velocity;
  rational_t launch_angle;
  vector3d old_position;
  bool in_flight;
  time_value_t total_life;
  time_value_t fizzle_time;
  // script to be played upon projectile prep
  script_object*           prep_so;
  script_object::instance* prep_inst;
  int                      prep_func;
  vm_thread*               prep_vm;
  // script to be played upon projectile launch
  script_object*           launch_so;
  script_object::instance* launch_inst;
  int                      launch_func;
  vm_thread*               launch_vm;
  // script to be played upon projectile hit
  script_object*           hit_so;
  script_object::instance* hit_inst;
  int                      hit_func;
  vm_thread*               hit_vm;

  friend class ranged_attack;
};

#define TARGET_Myself   0x00000001 // target invoker (if any)
#define TARGET_Friends  0x00000002 // friendly characters
#define TARGET_Enemies  0x00000004 // enemy characters (if no invoker, all characters fall in this class)
#define TARGET_Entities 0x00000008 // entities besides characters

enum target_shape_t {
	SHAPE_Sphere,
	SHAPE_XZCylinder,
	SHAPE_Cone
};

class target {
public:
  virtual ~target() {}
	virtual void invoke(entity *invoker, vector3d &p) = 0;
	const_range_iterator_t begin() { return (const_range_iterator_t)range_list.begin(); }
	const_range_iterator_t end()   { return (const_range_iterator_t)range_list.end(); }
protected:
	target_shape_t shape;
	unsigned int   flags;
	entity        *invoker;
	range_list_t   range_list;
};

class sphere_target: public target {
public:
	sphere_target(unsigned int f, rational_t r) { shape = SHAPE_Sphere; flags = f; radius2 = r * r; }
	void invoke(entity *invoker, vector3d &p);
private:
	rational_t radius2;
};

class xzcylinder_target: public target {
public:
	xzcylinder_target(unsigned int f, rational_t r, rational_t h) { shape = SHAPE_XZCylinder; flags = f; radius = r; height = h; }
	void invoke(entity *invoker, vector3d &p);
private:
	rational_t radius;
	rational_t height;
};

class cone_target: public target {
public:
	cone_target(unsigned int f, vector3d &v, rational_t a, rational_t d) { shape = SHAPE_Cone; flags = f; facing = v; assert((a >= 0.0f) && (a < 180.0f)); cos_half_angle = (rational_t)fast_cos((double)((180.0f - a) * 0.5f * PI / 360.0f)); distance2 = d * d; }
	void invoke(entity *invoker, vector3d &p);
private:
	vector3d   facing;
	rational_t cos_half_angle;
	rational_t distance2;
};

#endif
