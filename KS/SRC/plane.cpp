#include "global.h"

#include "plane.h"

plane::plane( const vector3d& _normal, fp d )
{
  unit_normal = _normal;
  fp len=unit_normal.length();
  unit_normal /= len;
  
  odistance = d/len;
}

plane::plane( fp a, fp b, fp c, fp d )
{
  unit_normal = vector3d(a,b,c);
  fp len=unit_normal.length();
  unit_normal /= len;
  
  odistance = d/len;
}

plane::plane( const vector3d& _point_on_surface, const vector3d& _normal )
{
  unit_normal = _normal;
  unit_normal.normalize();
  
  odistance = dot( _point_on_surface, unit_normal );
}

plane::plane( const vector3d& _point_on_surface1,
              const vector3d& _point_on_surface2,
              const vector3d& _point_on_surface3 )
{
  unit_normal = cross(_point_on_surface3-_point_on_surface1,_point_on_surface2-_point_on_surface1);
  unit_normal.normalize();
  
  odistance = dot( _point_on_surface1, unit_normal );
}

