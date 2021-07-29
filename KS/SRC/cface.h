#ifndef CFACE_H
#define CFACE_H
////////////////////////////////////////////////////////////////////////////////
/*
  cface.h

  triangular mesh faces, expressed as verts of a mesh they connect.
  These faces are for use in collison meshes (as opposed to 'face'
  wchich is used by visual meshes).  They contain face normal information.

*/
////////////////////////////////////////////////////////////////////////////////

#include "meshrefs.h"
#include "algebra.h"
#include "stringx.h"

class vert;
class cg_mesh;
class face; // DEBUG

extern int global_cface_count;

const unsigned short CFACE_FLAG_COSMETIC   = 0x0001;
const unsigned short CFACE_FLAG_WATER      = 0x0002;
const unsigned short CFACE_FLAG_VEGETATION = 0x0004;

class cface
  {
  public:
    // empty invalid cface
    cface() 
      {
      vert_refs[0] = vert_refs[1] = vert_refs[2] = UNINITIALIZED_VERT_REF;
      flags = 0;
      global_cface_count++;
      }

    // cface as a set of verts and a material
    cface(vert_ref wa,vert_ref wb,vert_ref wc)
      {
      vert_refs[0] = wa;
      vert_refs[1] = wb;
      vert_refs[2] = wc;
      flags = 0;
      global_cface_count++;
      }

    // copy constructor
    cface(const cface &f)
      {
      memcpy(this,&f,sizeof(cface));
      global_cface_count++;
      }

    ~cface(){global_cface_count--;}

    // return a vert index
    inline vert_ref get_vert_ref(int i) const {return vert_refs[i];}

    inline const vector3d& get_normal() const {return normal;}
    inline void set_normal(vector3d v){normal = v;}
    inline const vector3d& get_raw_normal() const {return raw_normal;}
    inline void set_raw_normal(vector3d v){raw_normal = v;}
    inline const vector3d& get_center() const {return center;}
    inline void set_center(vector3d c) {center = c;}
    inline const vector3d& get_raw_center() const {return raw_center;}
    inline void set_raw_center(vector3d c) {raw_center = c;}
    inline rational_t get_radius() const {return radius;}
    inline void set_radius(rational_t r) {radius = r;}
    inline material_ref get_my_material()const {return my_material;}

    // Flags access
    inline void set_flag(unsigned short fl, bool c)
      {
      if (c)
        flags |= fl;
      else
        flags &= (fl^0xFFFF);
      };
    inline bool is_flagged(unsigned short fl) const {return flags&fl;}
    inline void set_flags(unsigned short fl){flags = fl;}
    inline unsigned short get_flags(){return flags;}

  private:
    vert_ref vert_refs[3];
    material_ref my_material;
    vector3d raw_normal;

#ifdef TARGET_PS2
    vector3d normal __attribute__((aligned(16)));
#else
    vector3d normal;
#endif

    vector3d raw_center;
    vector3d center;
    rational_t radius;
    unsigned short flags;

  friend class cg_mesh;
  friend void read_nice_mesh(stringx);
  friend void read_base_mesh(stringx);
  friend void read_splits(stringx);
  };

#endif