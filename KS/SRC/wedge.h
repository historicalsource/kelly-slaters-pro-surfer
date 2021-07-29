#ifndef WEDGE_H
#define WEDGE_H
////////////////////////////////////////////////////////////////////////////////
/*
  wedge.h

  a wedge is a half-assed strided vertex

  you can do a vertex like this:

  array<vector> vertex_pool;
  array<texture_coordinate> texture_coordinate_pool;
  array<vector> vertex_normal_pool;

  class strided_vertex
  {
    int index_into_vector_pool;
    int index_into_texture_coordinate_pool;
    int index_into_vertex_normal_pool;
  }

  our wedge is like a strided vertex that only has a vector_pool.
  the actual texture_coordinate and vertex_normal is kept with
  the wedge.

  we may switch to a more fully assed strided vertex, or we
  may switch to straight up surface_verts with more duplicated
  information.  The first takes less memory, the second is
  probably faster.
*/
////////////////////////////////////////////////////////////////////////////////
#include "color.h"
#include "chunkfile.h"
#include "meshrefs.h"
#include "txtcoord.h"
#include "algebra.h"
//#include "hwrasterize.h"
#include "vert.h"

////////////////////////////////////////////////////////////////////////////////
// forward class declarations
////////////////////////////////////////////////////////////////////////////////
class vr_pmesh;
class region;
class terrain;

////////////////////////////////////////////////////////////////////////////////
// wedge
////////////////////////////////////////////////////////////////////////////////
class wedge
  {
  public:
    // uninitialized wedge
    wedge() : level_of_detail(0)  {}

    // wedge with local coordinate, texture coordinate, vertex normal, and
    // reference to the next wedge as detail gets lower
    wedge(
           const texture_coord& _uv0,
           const texture_coord& _uv1,
           const vector3d& _normal,
           wedge_ref next_lower_detail,
           const color& _color = color(1.0f,1.0f,1.0f,1.0f) ) :
         level_of_detail(0), lower_detail(next_lower_detail) {}

    // next wedge as detail gets lower, index into my meshes wedge pool
    wedge_ref get_lower_detail() const  {return lower_detail;}
    short     get_lod() const {return level_of_detail;}

  public:
    short level_of_detail;     // "   "
    wedge_ref lower_detail;  // for progressive mesh only
    // currently 40 bytes

  friend void swap_verts(vert_ref v1,vert_ref v2);
  friend class vr_pmesh;
  friend void read_splits(stringx);

#if !defined(NO_SERIAL_OUT)
  friend void serial_out( chunk_file& io, const wedge& w );
#endif
#if !defined(NO_SERIAL_IN)
  friend void serial_in( chunk_file& io, wedge* w, vert_ref* vr);
#endif  

  // The following added for debugging purposes only
#if !defined(NO_SERIAL_IN)
  friend void serial_in( chunk_file& fs, region * r );
#endif

  friend class region;
  friend class terrain;
  };

extern const chunk_flavor CHUNK_WEDGE;
extern const chunk_flavor CHUNK_NUM_WEDGES;
extern const chunk_flavor CHUNK_NORMAL;
extern const chunk_flavor CHUNK_NO_WARNINGS;

void skip_wedge( chunk_file& fs );

#endif
