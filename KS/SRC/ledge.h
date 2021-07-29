#ifndef _LEDGE_H_
#define _LEDGE_H_


class region;


class ledge
{
private:
  float     l_x;
  float     l_z;
  float     r_x;
  float     r_z;
  float     y;
  float     n_x;
  float     n_z;
  int       type;
public:
  ledge( const int _type, const vector3d& _0, const vector3d& _1, const vector3d& _2, const region *r );
  int get_type() const { return type;}
  vector3d get_left_edge() const { return vector3d( l_x, y, l_z );}
  vector3d get_right_edge() const { return vector3d( r_x, y, r_z );}
  vector3d get_out_normal() const { return vector3d( n_x, .0f, n_z );}

  // returns true if l2 has been merged to l1
  friend bool merge_ledge( ledge *l1, const ledge *l2 );

//!  friend void share_portal_ledges();
};

#endif