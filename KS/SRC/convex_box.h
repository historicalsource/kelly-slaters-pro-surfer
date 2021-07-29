#ifndef CONVEX_BOX_H
#define CONVEX_BOX_H

#include "global.h"
#include "bound.h"

class convex_box
{
public:
  vector4d planes[6];
  bounding_box bbox;

  convex_box( ) { }
  convex_box( const convex_box& );
  ~convex_box( ) { }

  friend void serial_in( chunk_file& fs, convex_box* box );

  inline bool point_inside(const vector3d &pt, const vector3d &box_pos) const
  {
    vector3d delta = pt - box_pos;

    for( int i = 0; i < 6; ++i ) 
    {
      if( ( ( planes[i].x * delta.x ) + ( planes[i].y * delta.y ) + ( planes[i].z * delta.z ) - planes[i].w ) > 0.0f ) 
        return false;
    }

    return true;
  }
};

#endif