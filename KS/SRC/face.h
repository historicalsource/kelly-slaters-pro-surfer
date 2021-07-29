#ifndef FACE_H
#define FACE_H
////////////////////////////////////////////////////////////////////////////////
/*
  face.h

  triangular mesh faces, expressed as which wedges (wedges are our name for
  what DirectX calls strided vertices) of a mesh they connect, which
  material they express, and what level of detail they exist at

  sometimes I cannot feel my face
  you'll never see me fall from Grace
  my pony that I trained well to race
  and when hunting foxes to give chase
  the winter wind chills in this place
  sometimes I cannot feel my face

*/
////////////////////////////////////////////////////////////////////////////////
#include "wedge.h"
#include "chunkfile.h"
#include "faceflags.h"

const chunk_flavor CHUNK_FACE             ("face");
const chunk_flavor CHUNK_FACE_WITH_FLAGS  ("face_fl");
const chunk_flavor CHUNK_PROGRESSIVE_FACE ("face_p");
const chunk_flavor CHUNK_NUM_FACES        ("nofaces");
const chunk_flavor CHUNK_MESH             ("mesh");

class face 
{
public:
  // empty invalid face
  face() 
    : my_material(UNINITIALIZED_MATERIAL_REF), level_of_detail(USHRT_MAX), flags(0)
  {
    wedge_refs[0] = wedge_refs[1] = wedge_refs[2] = UNINITIALIZED_WEDGE_REF;
  }
  // copy constructor
  face(const face &f)
  {
    wedge_refs[0] = f.wedge_refs[0];
    wedge_refs[1] = f.wedge_refs[1];
    wedge_refs[2] = f.wedge_refs[2];
    my_material = f.my_material;
    level_of_detail = f.level_of_detail;
    flags = f.flags;
  }
  // face as a set of wedges and a material
  face(wedge_ref wa, wedge_ref wb, wedge_ref wc, material_ref _my_material, unsigned short _flags = 0)
    : my_material(_my_material), level_of_detail(0), flags(_flags)
  {
    wedge_refs[0] = wa; wedge_refs[1] = wb; wedge_refs[2] = wc;
  }
#if !defined(NO_SERIAL_IN)
  face(chunk_file &io, const chunk_flavor face_flavor = CHUNK_FACE)
  {
    serial_in(io, this, face_flavor);
  }
#endif
  ~face() { }

  // return a wedge index
  wedge_ref get_wedge_ref(int i) const { return wedge_refs[i]; }

  // return a material index
  material_ref get_material_ref() const { return my_material; }
  void set_material_ref(material_ref r) { my_material=r; }

  unsigned short get_flags() const { return flags; }

  bool is_walkable() const { return (flags & TERFACE_WALKABLE); }
  bool is_notwalkable() const { return( flags & TERFACE_FORCE_NOT_WALKABLE ); } // 0x200
  bool is_cosmetic() const { return (flags & TERFACE_COSMETIC); }
  bool is_water() const { return (flags & TERFACE_WATER); }
  bool is_vegetation() const { return (flags & TERFACE_VEGETATION); }

  unsigned char get_surface_type() const { return((flags & TERFACE_SURFTYPE_MASK) >> 4); }

  bool top_border()    const { return (flags & TERFACE_TOP_BORDER); }
  bool bottom_border() const { return (flags & TERFACE_BOTTOM_BORDER); }
  bool left_border()   const { return (flags & TERFACE_LEFT_BORDER); }
  bool right_border()  const { return (flags & TERFACE_RIGHT_BORDER); }

  const vector3d& get_normal() const { assert(false); return ZEROVEC; }
  const vector3d& get_center() const { assert(false); return ZEROVEC; }
  const rational_t get_radius() const { assert(false); return 0; }

//void operator=(const face &f)
//{
//  wedge_refs[0] = f.wedge_refs[0];
//  wedge_refs[1] = f.wedge_refs[1];
//  wedge_refs[2] = f.wedge_refs[2];
//  my_material = f.my_material;
//  level_of_detail = f.level_of_detail;
//  flags = f.flags;
//  ++face_count;
//}

  bool operator==(const face& f) const
  {
    return( ( wedge_refs[0]==f.wedge_refs[0] )&&
            ( wedge_refs[1]==f.wedge_refs[1] )&&
            ( wedge_refs[2]==f.wedge_refs[2] ) );
  }

public:
  wedge_ref wedge_refs[3];
  material_ref my_material;
  unsigned short level_of_detail;
  unsigned short flags;
  // for undoing the "optimize()" sort when we load face refs later

#if !defined(NO_SERIAL_IN)
  friend void serial_in( chunk_file& io, face* f, const chunk_flavor& face_flavor);
#endif
#if !defined(NO_SERIAL_OUT)
  friend void serial_out( chunk_file& io, const face& f );
#endif

};

class reduced_face
{
public:
  // empty invalid face
  reduced_face()
    : my_material(UNINITIALIZED_MATERIAL_REF), level_of_detail(USHRT_MAX), flags(0)
  {
  }
  // reduced face as a material
  reduced_face( material_ref _my_material, unsigned short _flags = 0)
    : my_material(_my_material), level_of_detail(0), flags(_flags)
  {
  }

  // return a material index
  material_ref get_material_ref() const { return my_material; }
  void set_material_ref(material_ref r) { my_material=r; }

  unsigned short get_flags() const { return flags; }

  bool is_walkable() const { return (flags & TERFACE_WALKABLE); }
  bool is_cosmetic() const { return (flags & TERFACE_COSMETIC); }
  bool is_water() const { return (flags & TERFACE_WATER); }
  bool is_vegetation() const { return (flags & TERFACE_VEGETATION); }

  unsigned char get_surface_type() const { return((flags & TERFACE_SURFTYPE_MASK) >> 4); }

  bool top_border()    const { return (flags & TERFACE_TOP_BORDER); }
  bool bottom_border() const { return (flags & TERFACE_BOTTOM_BORDER); }
  bool left_border()   const { return (flags & TERFACE_LEFT_BORDER); }
  bool right_border()  const { return (flags & TERFACE_RIGHT_BORDER); }

public:
  material_ref my_material;
  unsigned short level_of_detail;
  unsigned short flags;
};

////////////////////////////////////////////////////////////////
//  collision_face
////////////////////////////////////////////////////////////////

/*
struct collision_data
{
  vector3d normal;
  vector3d center;
  rational_t radius;

  collision_data(){}
  collision_data( const vector3d& _normal, const vector3d& _center, rational_t _radius )
  {
    normal = _normal;
    center = _center;
    radius = _radius;
  }
};

class collision_face : public face
{
  public:
    collision_face() : face() {}
    collision_face(const face &f) : face(f)
    {
      _build_collision_data();
    }
    collision_face(wedge_ref wa, wedge_ref wb, wedge_ref wc, material_ref _my_material, unsigned short _flags = 0)
      : face(wa, wb, wc, _my_material, _flags)
    {
      _build_collision_data();
    }

    collision_face(chunk_file &io, const chunk_flavor face_flavor = CHUNK_FACE) 
      : face(io, face_flavor)
    {
      _build_collision_data();
    }
    virtual const vector3d& get_normal() const {return cd.normal; }
    virtual const vector3d& get_center() const {return cd.center; }
    virtual const rational_t get_radius() const {return cd.radius; }

  protected:
    void _build_collision_data();

    collision_data cd;
};
*/

#endif
