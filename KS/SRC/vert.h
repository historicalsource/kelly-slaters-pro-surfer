#ifndef VERT_H
#define VERT_H
////////////////////////////////////////////////////////////////////////////////
/*
  vert.h

  the local-coordinate portion of a vertex
*/
////////////////////////////////////////////////////////////////////////////////
#include "algebra.h"
#include "chunkfile.h"

#include "maxskinbones.h"

class vert
{
  public:
    vert() : point(0,0,0) { num_bones=0; }
    vert( chunk_file& io );

    vert(const vector3d& _point) : point(_point) { num_bones=0; }
    const vector3d& get_point() const { return point; }

//  private:  // RUDEST E3 HACK OF ALL TIME, COMMENTED OUT private SPEC
    // *this* is the rudest E3 hack?  You haven't been around long.  -JF

#ifdef TARGET_PS2
    vector3d point __attribute__((aligned(16)));
#else
    vector3d point;
#endif

    int num_bones;
    unsigned char bone_ids[MAX_SKIN_BONES];
    float         bone_weights[MAX_SKIN_BONES];

    friend inline void serial_in( chunk_file& io, vert* v );
};

#if !defined(NO_SERIAL_IN)
inline void serial_in(chunk_file& io,vert* v) 
{
  memset( v, 0, sizeof( *v ) );
  serial_in(io, &v->point);
}

inline vert::vert( chunk_file& io )
{
  serial_in(io, this);
}
#endif /* NO_SERIAL_IN */

const chunk_flavor CHUNK_VERT       ("vert");
const chunk_flavor CHUNK_NUM_VERTS  ("noverts");
const chunk_flavor CHUNK_PIVOT      ("pivot");
const chunk_flavor CHUNK_SKIN       ("skin");

#endif
