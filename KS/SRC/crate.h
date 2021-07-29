#ifndef _CRATE_H_
#define _CRATE_H_


//#include "physent.h"
#include "entity.h"


class region;

enum
{
  CRATE_NONE = 0,
  CRATE_PUSH,
  CRATE_STACK,
  CRATE_UNSTACK,
  CRATE_PULL,
  CRATE_CLIMB
};


//class crate : public physical_entity
class crate : public entity
{
private:
  bool              available, stacking, unstacking;
  crate             *nether;
  vector2d          u_left, l_right, offset;
  vector2d          target_xz;
  stringx           *push_anim_filename, *stack_anim_filename, *pull_anim_filename, *unstack_anim_filename;
  entity_anim_tree  *pAnim;

  void check_po();
  void init_variables();
  void set_facing( const vector2d& facing );
public:
  crate(  const entity_id&  _id = ANONYMOUS,
          entity_flavor_t   _flavor = ENTITY_ENTITY,
          unsigned int      _flags = 0 );

  crate(  chunk_file&       fs,
          const entity_id&  _id,
          entity_flavor_t   _flavor,
          unsigned int      _flags );

  virtual ~crate();

  virtual entity *make_instance( const entity_id& _id, unsigned int _flags ) const;
protected:
  void copy_instance_data( const crate& b );

/////////////////////////////////////////////////////////////////////////////
// entity class identification
public:
  virtual bool is_a_crate() const { return true; }

public:
  void stack_it();
  void unstack_it();
  void fix_po();

  virtual void force_region( region_node* r );
  virtual bool handle_enx_chunk( chunk_file& fs, stringx &label );

  void set_area( const vector2d& upper_left, const vector2d& lower_right, const vector2d& area_offset );
  void get_area( vector2d *upper_left, vector2d *lower_right ) { *upper_left = u_left; *lower_right = l_right;}

  void set_target_xz( rational_t x, rational_t z ) { target_xz.x = x; target_xz.y = z;}
  void set_target_xz_increment( rational_t x, rational_t z );

  bool is_available() { return available;}
  void set_available( bool torf ) { available = torf; set_walkable( torf );}

  bool is_stacking() const { return stacking;}
  void set_stacking( bool torf ) { stacking = torf;}
  bool is_unstacking() const { return unstacking;}
  void set_unstacking( bool torf ) { unstacking = torf;}

  void start_pushing( const vector2d& xz_dir );
  void start_stacking( const vector2d& xz_dir );
  void start_unstacking( const vector2d& xz_dir );
  void start_pulling( const vector2d& xz_dir );

  const stringx& get_push_anim_filename();
  const stringx& get_pull_anim_filename();
  const stringx& get_stack_anim_filename();
  const stringx& get_unstack_anim_filename();

  friend int get_crate( region *r,
                        const po& p,
                        rational_t delta_dist,
                        rational_t delta_y,
                        rational_t delta_ang,
                        crate **found,
                        vector3d *start_pos = NULL,
                        vector3d *start_facing = NULL
                      );

  virtual void frame_advance( time_value_t t );


  friend void scan_for_stacked_crates( region *r );
};


crate *get_crate( region *r, const vector3d& position, bool check_available = true, bool any = false );


// Miscellanies useful functions
inline bool is_approx( const rational_t x, const rational_t y, const rational_t max_error )
{
  if( fabs(x - y) <= max_error ) return true;
  else return false;
}
bool is_axis_aligned( const vector3d& v, rational_t max_error );
rational_t round( rational_t arg );


#endif
