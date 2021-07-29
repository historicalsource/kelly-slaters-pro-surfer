/* Some code (heavily modified) from Game Programming Gems
 * "Portions Copyright (C) Dante Treglia II, 2000"
 */
#ifndef _B_SPLINE_H
#define _B_SPLINE_H

#include "global.h"

class b_spline
{
public:
  typedef enum
  {
    _DIRECT,
    _CATMULL_ROM,
    _B_SPLINE
  } eSplineType;

protected:
	vector<vector3d> control_pts;
  vector<vector3d> curve_pts;
  int sub_divisions;
  eSplineType spline_type;
  bool force_start;

public:
  b_spline()
  {
    sub_divisions = 5;
    spline_type = _B_SPLINE;
    force_start = true;
  }

  inline b_spline &operator=(const b_spline &b)
  {
    control_pts = b.control_pts;
    curve_pts = b.curve_pts;

    sub_divisions = b.sub_divisions;
    spline_type = b.spline_type;
    force_start = b.force_start;

    return(*this);
  }

  b_spline(const b_spline &b)
  {
    (*this) = b;
  }

	~b_spline()
  {
  }

  void rebuild();

  void build(int subdiv, eSplineType type = _B_SPLINE)
  {
    sub_divisions = subdiv;
    spline_type = type;

    rebuild();
  }
	
  inline void clear()
  {
    control_pts.resize(0);
    curve_pts.resize(0);
  }

  inline void build(const vector<vector3d> &pts, int sub_divisions, eSplineType type = _B_SPLINE)
  {
    clear();

    control_pts = pts;

    build(sub_divisions, type);
  }

  inline void reserve_control_pts(int num)
  {
    clear();
    control_pts.reserve(num);
  }

  inline void add_control_pt(const vector3d &pt)
  {
    control_pts.push_back(pt);
  }

  inline void set_control_pt(int index, const vector3d &pt)
  {
    assert(index >= 0 && index < (int) control_pts.size());
    control_pts[index] = pt;
  }
  inline const vector3d &get_control_pt(int index)
  {
    assert(index >= 0 && index < (int) control_pts.size());
    return(control_pts[index]);
  }

  inline const vector3d &get_curve_pt(int index)
  {
    assert(index >= 0 && index < (int) curve_pts.size());
    return(curve_pts[index]);
  }

  inline int get_num_control_pts() const                  { return(control_pts.size()); }
  inline int get_num_curve_pts() const                    { return(curve_pts.size()); }
  inline const vector<vector3d> &get_control_pts() const  { return(control_pts); }
  inline const vector<vector3d> &get_curve_pts() const    { return(curve_pts); }
  inline int get_sub_divisions() const                    { return(sub_divisions); }
  inline eSplineType get_spline_type() const              { return(spline_type); }
  inline bool is_force_start() const                      { return(force_start); }
  inline void set_force_start(bool s)                     { force_start = s; }
};

#endif