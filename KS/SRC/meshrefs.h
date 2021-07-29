#ifndef MESHREFS_H
#define MESHREFS_H
////////////////////////////////////////////////////////////////////////////////
/*
  meshrefs.h

  the various kind of indices in a mesh.
  these all are currently implemented as indices into a pool.
  they could later be replaced with pointers for speed
*/
////////////////////////////////////////////////////////////////////////////////
typedef unsigned short vert_ref;
enum {UNINITIALIZED_VERT_REF=0xffff};

typedef unsigned short wedge_ref;
enum {UNINITIALIZED_WEDGE_REF=0xffff};

typedef unsigned face_ref;
enum {UNINITIALIZED_FACE_REF=0xffffffff};

typedef short material_ref;
enum
  {
  UNINITIALIZED_MATERIAL_REF = -1,
  INVISIBLE_MATERIAL_REF =     -2
  };

typedef unsigned bone_idx;

#endif