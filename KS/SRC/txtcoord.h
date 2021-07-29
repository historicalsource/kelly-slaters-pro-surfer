#ifndef TXTCOORD_H
#define TXTCOORD_H
////////////////////////////////////////////////////////////////////////////////
/*
  txtcoord.h

  texture_coordinate class
  I probably should have defined this as a vector2dr
  but there isn't a whole lot to it anyhow.
*/
////////////////////////////////////////////////////////////////////////////////
#include "chunkfile.h"
#include "meshrefs.h"
#include "hwmath.h"
#include "algebra.h"

#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////
//  class forward declarations
////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//  texture_coord
////////////////////////////////////////////////////////////////////////////////
class texture_coord : public vector2dr
{
public:
  // uninitialized texture_coord
  texture_coord() {}

  // u v coordinate
  texture_coord(float _u,float _v) : vector2dr(_u,_v) {}

  // do I really need to comment these functions?
  rational_t get_u() const { return x; }
  rational_t get_v() const { return y; }

  // these functions are from tpm creation utility:
  friend void read_nice_mesh(stringx);
  friend void read_base_mesh(stringx);
  friend void process_corner( FILE* fp,
                     vert_ref* vert_id,
                     wedge_ref* wedge_id,
                     face_ref* face_id);
  #if !defined(NO_SERIAL_IN)
  friend void serial_in( chunk_file& io, texture_coord* tc );
  #endif
  #if !defined(NO_SERIAL_OUT)
  friend void serial_out( chunk_file& io, const texture_coord& tc );
  #endif
};

////////////////////////////////////////////////////////////////////////////////
const chunk_flavor CHUNK_TEXTURE_COORD("txtcoord");

#endif
