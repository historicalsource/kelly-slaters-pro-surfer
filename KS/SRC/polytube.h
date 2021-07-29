#ifndef _POLYTUBE_H_
#define _POLYTUBE_H_

#include "global.h"

#include "entity.h"
#include "b_spline.h"

class mat_fac;

class polytube : public entity
{
protected:
  void init();
  virtual void copy_instance_data( const polytube& b );

  bool use_spline;

  b_spline spline;
  mat_fac *my_material;
  rational_t tube_radius;
  int num_sides;
  rational_t tiles_per_meter;
  rational_t max_length;

public:
  // Constructors
  polytube( const entity_id& _id, unsigned int _flags = 0 );

  polytube( chunk_file& fs,
          const entity_id& _id,
          entity_flavor_t _flavor = ENTITY_POLYTUBE,
          unsigned int _flags = 0 );

  virtual ~polytube();

  virtual entity* make_instance( const entity_id& _id,
                                 unsigned int _flags ) const;

  virtual void frame_advance(time_value_t t) {}
  virtual render_flavor_t render_passes_needed() const;
  virtual void render( camera* camera_link, rational_t detail, render_flavor_t flavor, rational_t entity_translucency_pct );

  inline b_spline &get_spline()                                                   { return(spline); }
  inline void set_spline(const b_spline &b)                                       { spline = b; }
  inline void clear()                                                             { spline.clear(); }
  inline void reserve_control_pts(int num)                                        { spline.reserve_control_pts(num); }
  inline void add_control_pt(const vector3d &pt)                                  { spline.add_control_pt(pt); }
  inline void set_control_pt(int index, const vector3d &vec)                      { spline.set_control_pt(index, vec); }
  inline const vector3d &get_control_pt(int index)                                { return(spline.get_control_pt(index)); }
  inline const vector3d &get_curve_pt(int index)                                  { return(spline.get_curve_pt(index)); }
  inline int get_num_control_pts() const                                          { return(spline.get_num_control_pts()); }
  inline int get_num_curve_pts() const                                            { return(spline.get_num_curve_pts()); }
  inline const vector<vector3d> &get_control_pts() const                          { return(spline.get_control_pts()); }
  inline const vector<vector3d> &get_curve_pts() const                            { return(spline.get_curve_pts()); }
  inline void build(int subdiv, b_spline::eSplineType type = b_spline::_B_SPLINE) { spline.build(subdiv, type); }
  inline void rebuild()                                                           { spline.rebuild(); }
  inline int get_sub_divisions() const                                            { return(spline.get_sub_divisions()); }
  inline b_spline::eSplineType get_spline_type() const                            { return(spline.get_spline_type()); }
  inline void set_force_start(bool s)                                             { spline.set_force_start(s); }
  inline bool is_force_start() const                                              { return(spline.is_force_start()); }



  inline mat_fac *get_material() const            { return(my_material); }
  void set_material( const stringx& file );
  void set_material(mat_fac *mat);

  inline rational_t get_tube_radius() const       { return(tube_radius); }
  inline void set_tube_radius(rational_t rad)     { tube_radius = rad; }
  inline int get_num_sides() const                { return(num_sides); }
  inline void set_num_sides(int n)                { num_sides = n; assert(num_sides >= 3 && num_sides <= 60); }
  inline rational_t get_tiles_per_meter() const   { return(tiles_per_meter); }
  inline void set_tiles_per_meter(rational_t t)   { tiles_per_meter = t; assert(tiles_per_meter > 0.0f); }
  inline rational_t get_max_length() const        { return(max_length); }
  inline void set_max_length(rational_t l)        { max_length = l; }

  inline bool uses_spline() const                 { return(use_spline); }
  inline void set_use_spline(bool u)              { use_spline = u; }
};

void render_polytube(const vector<vector3d> &pts, rational_t radius, int num_sides = 3, const color32 &col = color32_white, mat_fac *the_material = NULL, rational_t tiles_per_meter = 1.0f, const matrix4x4 &the_matrix = identity_matrix, rational_t max_length = -1.0f);
inline void render_polytube(const vector<vector3d> &pts, rational_t radius, int num_sides = 3, const color32 &col = color32_white, mat_fac *the_material = NULL, rational_t tiles_per_meter = 1.0f, const po &the_po = po_identity_matrix, rational_t max_length = -1.0f)
{
  render_polytube(pts, radius, num_sides, col, the_material, tiles_per_meter, the_po.get_matrix(), max_length);
}

#endif
