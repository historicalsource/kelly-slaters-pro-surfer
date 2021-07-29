#ifndef LIMB_H
#define LIMB_H

#include "ostimer.h"
//!#include "rigid.h"
#include "entity.h"
#include "project.h"

class hinge;
class po;
//!class actor;
class limb;
class vr_pmesh;

enum
{
  LEFT_ARM = 0,
  RIGHT_ARM = 1
};

typedef pair<limb *, rational_t> limb_dist_t;


struct limb_body_meat
{
  rational_t mass;
  rational_t volume;
  vector3d c_o_m;
  vector3d I;
  vector3d min_extent;
  vector3d max_extent;
  rational_t shock_damper;
  rational_t l_shock_damper;
  rational_t h_shock_damper;
  rational_t free_body;

  limb_body_meat() {}
  limb_body_meat( const char* pathname, unsigned ) {}
};


class limb_body : public entity
{
public:
  limb_body( limb * _my_limb, const entity_id& _id, unsigned int _flags );
  limb_body( limb * _my_limb, const entity_id& _id = ANONYMOUS,
             entity_flavor_t _flavor = ENTITY_LIMB_BODY,
             unsigned int _flags = 0 );
  virtual ~limb_body();

/////////////////////////////////////////////////////////////////////////////
// entity class identification
public:
  virtual bool is_a_limb_body() const { return true; }

// NEWENT File I/O
public:
  limb_body( limb * _my_limb,
             chunk_file& fs,
             const entity_id& _id,
             entity_flavor_t _flavor,
             unsigned int _flags );

// Old File I/O
public:
  limb_body( limb * _my_limb,
             const stringx& colgeom_fname,
             vr_pmesh* single_mesh ,
             const entity_id& id_name = ANONYMOUS,
             entity_flavor_t _flavor = ENTITY_LIMB_BODY  );


// Instancing
public:
  virtual entity* make_instance( limb * _my_limb,
                                 const entity_id& _id,
                                 unsigned int _flags ) const;
protected:
  void copy_instance_data( const limb_body& b );
  void compute_limb_properties();

// Misc.
public:
  virtual void frame_advance(time_value_t t);

  // This is in duplication of the information in rigid_body, but this is OK
  // because rigid_body is not derived from limb_body.
  virtual void get_c_o_m(vector3d* target) const { *target=my_meat->c_o_m;}
  virtual void set_c_o_m( const vector3d& c )    { my_meat->c_o_m = c; }
  virtual rational_t get_mass() const     { return my_meat->mass;}
  virtual void set_mass( rational_t m )   { my_meat->mass = m; }
  virtual rational_t get_volume() const   { return my_meat->volume; }
  virtual void set_volume( rational_t v ) { my_meat->volume = v; }
  virtual void get_I_vector( vector3d* target ) const   { *target=my_meat->I; }
  virtual void set_I_vector( const vector3d& I ) { my_meat->I = I; }

  const vector3d& get_min_extent() const { return my_meat->min_extent; }
  void set_min_extent( const vector3d& v ) { my_meat->min_extent = v; }
  const vector3d& get_max_extent() const { return my_meat->max_extent; }
  void set_max_extent( const vector3d& v ) { my_meat->max_extent = v; }
  rational_t get_shock_damper() const { return my_meat->shock_damper; }
  void set_shock_damper( rational_t s ) { my_meat->shock_damper = s; }
  rational_t get_l_shock_damper() const { return my_meat->l_shock_damper; }
  void set_l_shock_damper( rational_t s ) { my_meat->l_shock_damper = s; }
  rational_t get_h_shock_damper() const { return my_meat->h_shock_damper; }
  void set_h_shock_damper( rational_t s ) { my_meat->h_shock_damper = s; }
  rational_t get_free_body() const { return my_meat->free_body; }
  void set_free_body( rational_t s ) { my_meat->free_body = s; }

  virtual region_node* get_region() const;

  // enum of local signal ids (for coding convenience and readability)
  enum signal_id_t
  {
    // a descendant class uses the following line to append its local signal ids after the parent's
    PARENT_SYNC_DUMMY = entity::N_SIGNALS - 1,
    #define MAC(label,str)  label,
    #include "limb_body_signals.h"
    #undef MAC
    N_SIGNALS
  };

  static void register_signals();
  virtual signal_list* construct_signal_list() { return NEW signal_list( N_SIGNALS, (signal*)NULL ); }
  static unsigned short get_signal_id( const char *name );

  limb * get_my_limb() const {return my_limb;}
//!  actor *get_my_actor() const;

  /*
  Note that this feature is somewhat of a kludge, and that the system probably
  needs to be rewritten in the future (e.g., allow signals to carry data,
  allow multiple signal tracks running on a given node, etc.).
  */
  virtual void raise_signal( signal_list::size_t idx ) const;

  void set_my_limb(limb * mine){my_limb = mine;}
private:
  limb * my_limb;

  limb_body_meat * my_meat;
};

extern instance_bank<limb_body_meat> limb_body_meat_bank;


class limb
{
public:
  limb( limb_body* _lb, /*! actor* _my_actor,!*/ limb* _parent );
  limb( chunk_file& fs, /*! actor* _my_actor,!*/ limb* _parent );
  ~limb();

  void read_enx_data( chunk_file& fs );

  // copy limb tree
  limb* make_instance(/*! actor* _my_actor,!*/ limb* _parent );

  void advance_hinges(time_value_t t);
  void phys_render(time_value_t t = 0.0f, unsigned int recurse_depth = INT_MAX, bool shadow = false);
  const po& get_abs_po() const { return lb->get_abs_po(); }
  const po& get_rel_po() const { return lb->get_rel_po();}
  void set_rel_po(po const & p) { lb->set_rel_po(p); }
  const vector3d& get_abs_position() const { return lb->get_abs_position(); }
  anim_id_t get_id() const { return lb->get_anim_id(); }
  limb_body * get_body() const { return lb; }
  limb * get_parent() const { return parent; }
  limb *get_first_child() const { return !children.empty() ? children[0] : NULL; }
  const vector<limb*>& get_children() const { return children; }
  hinge * get_hinge(int i) const
  {
    assert(hinges.size()>i);
    return hinges[i];
  }
  int get_num_hinges() const
  {
    return hinges.size();
  }
  hinge * get_linear_hinge(int i) const
  {
    assert(linear_hinges.size()>i);
    return linear_hinges[i];
  }
  int get_num_linear_hinges() const
  {
    return linear_hinges.size();
  }
  void place_self_in_map(vector<limb *> & limb_map);

  rational_t get_thickness_below_origin() const;
  void recompute_relpos_from_pivot();

  void shock( const vector3d& accel, const vector3d& internal_accel, rational_t time_inc, bool ignore_damping = false, bool impulse = false );
  void local_linear_shock( const vector3d& accel, rational_t time_inc, bool ignore_damping=false, bool impulse = false );
  void local_angular_shock( const vector3d& accel, rational_t time_inc, bool ignore_damping=false, bool impulse = false );

  void kill_hinge_velocities();

  limb_dist_t find_closest_limb(vector3d p);

  void delete_colgeom();

  void add_to_single_mesh();

  void set_actuator_strike_scales(rational_t ss);

//!  actor * get_my_actor() const {return my_actor;}
//!  void set_my_actor( actor* a );

  bool any_strike_in_progress() const;

  // t is a fraction between 0 and 1 which the hinges will use to interpolate between last_second_val and this_second_val
  void set_hinges_from_old_times(time_value_t t);

  // set a whole branch of the limb tree visible or invisible
  void set_branch_visible( bool );

  // restore the hinge values from a keyframe animation
  void restore_hinges_from_po();

  enum
  {
    USE_DAMPING=0,
    IGNORE_DAMPING=1
  };
  enum
  {
    NOT_IMPULSE=0,
    IS_IMPULSE=1
  };
private:
  limb_body * lb;
  vector<hinge* > hinges;
  vector<hinge* > linear_hinges;

  vector<limb *> children;
  limb * parent;
//!  actor * my_actor;
  stringx sound_name;
  bool save_colmesh;
  vector3d base_relpos;

//!friend class actor;
  };

#endif
