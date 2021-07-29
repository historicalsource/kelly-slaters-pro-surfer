#ifndef VSPLIT_H
#define VSPLIT_H
////////////////////////////////////////////////////////////////////////////////
/*
  vsplit.h
  structures describing the vertex and wedge splits for a given progressive
  mesh.  only used for reading and writing progressive mesh files
*/
////////////////////////////////////////////////////////////////////////////////
#include "chunkfile.h"
#include "meshrefs.h"

// any time a wedge splits, at least two wedge_split structures are created
// for it, and each of the wedge_splits owns a subset of the faces around
// the original wedge
class wedge_split
  {
  public:
    // uninitialized wedge_split
    wedge_split() : old_wedge_id(UNINITIALIZED_WEDGE_REF) {}

    //
    wedge_split(wedge_ref _old_wedge,wedge_ref _new_wedge_id) :
        old_wedge_id(_old_wedge)
          {
          new_wedge_id = _new_wedge_id;
          }

    wedge_ref old_wedge_id;
    wedge_ref new_wedge_id;
    vector<face_ref> faces_for_split;
  };

// for any given vert split there's only one vsplit structure
class vsplit
  {
  public:
    vsplit(void) : old_vert(UNINITIALIZED_VERT_REF) {}
#if !defined(NO_SERIAL_IN)
    vsplit( chunk_file& io ) { serial_in( io, this );}
#endif

    vert_ref old_vert;
    vert_ref new_vert_ids[2];
    face_ref new_face_ids[2];
    vector<wedge_split> wedge_splits;
    // its possible to have 2 (but no more than 2) wedge_splits for the
    // same old_wedge_id
#if !defined(NO_SERIAL_OUT)
    friend void serial_out( chunk_file& io, const vsplit& v );
#endif
#if !defined(NO_SERIAL_IN)
    friend void serial_in( chunk_file& io, vsplit* v );
#endif
  };

#if !defined(NO_SERIAL_OUT)
void serial_out( chunk_file& io, const vsplit& v );
#endif
#if !defined(NO_SERIAL_IN)
void serial_in( chunk_file& io, vsplit* v );
#endif

const chunk_flavor CHUNK_VSPLIT("vsplit");


#endif
