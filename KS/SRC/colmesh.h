#ifndef COLMESH_H
#define COLMESH_H


#include "cface.h"
#include "colgeom.h"
//#include "material.h"
#include "vert.h"

class vr_pmesh;
class entity;

typedef vector<cface
	#ifdef TARGET_PS2
               ,malloc_alloc
	#endif
							 > cface_vect;
typedef vector<vert
	#ifdef TARGET_PS2
               ,malloc_alloc
	#endif
							 >  vert_vect;

typedef vector<int
	#ifdef TARGET_PS2
               ,malloc_alloc
	#endif
							 > intvector;

class cg_mesh : public collision_geometry
{
public:
  enum
  {
    FLAG_PIVOT_VALID        = 0x00000001,
    FLAG_GEOM_WARNING_GIVEN = 0x00000002,
    FLAG_ENTITY_COLLISION   = 0x00000004,
    FLAG_CAMERA_COLLISION   = 0x00000008,
  };
  enum
  {
    DISALLOW_WARNINGS=0,
    ALLOW_WARNINGS=1
  };
  cg_mesh();
  cg_mesh( chunk_file& fs, bool allow_warnings = true );  // for reading NEWENT files
  cg_mesh(const char * colgeom_mesh_fname, bool allow_warnings = true);
  cg_mesh(vr_pmesh * my_pmesh, const stringx & mesh_fname, bool allow_warnings = true);
  typedef vector<visual_rep*> VisRepList;
  cg_mesh(const VisRepList& visreps, const stringx & mesh_fname, bool allow_warnings = true);
  virtual ~cg_mesh();
  virtual collision_geometry* make_instance( entity* _owner ) const;
  virtual void xform(po const & the_po);
  virtual void split_xform(po const & po_1, po const & po2, int second_po_start_idx);
  virtual void split_xform(po const & po_1, po const & po2, po const & po3, int second_po_start_idx, int third_po_start_idx);

  void read_from_file( chunk_file& fs, bool allow_warnings );

  // access verts, faces, wedges, and materials
  inline const vert& get_raw_vert( vert_ref i )         const { return raw_verts[i]; }
  inline const vert& get_vert( vert_ref i )             const { return verts[i]; }
  inline const cface& get_cface( face_ref i )           const { return cfaces[i]; }
  inline const material& get_material( material_ref i ) const { return *(materials[i]); }

  // NEW x-sorted cface lookup
  int get_low_index( rational_t val ) const;
  int get_high_index( rational_t val ) const;
  inline const cface_vect& get_cfaces() const { return cfaces; }

  // access pointers to verts, faces, wedges, and materials
  inline const vert* get_raw_vert_ptr( vert_ref i )         const { return &raw_verts[i]; }
  inline const vert* get_vert_ptr( vert_ref i )             const { return &verts[i]; }
  inline const cface* get_cface_ptr( face_ref i )           const { return &cfaces[i]; }
  inline const material* get_material_ptr( material_ref i ) const { return materials[i]; }

  inline void set_vert(int i,vert v)  { verts[i] = v; }
  inline void set_raw_vert(int i,vert v)  { raw_verts[i] = v; }

  inline int get_num_verts()     const { return verts.size(); }
  inline int get_num_cfaces()    const { return cfaces.size(); }
  inline int get_num_materials() const { return materials.size(); }

  virtual void get_closest_point_along_dir( vector3d* target, const vector3d& axis ) const;

  virtual void get_min_extent( vector3d* v ) const;
  virtual void get_max_extent( vector3d* v ) const;

  virtual rational_t get_radius() const { return radius; }
  virtual const vector3d& get_pivot() const { return pivot; }
  inline void set_flag(int flag, bool torf)
  {
    if (torf)
      flags |= flag;
    else
      flags &= (0xFFFFFFFF^flag);
  }
  inline bool get_flag( int flag ) const { return (flags & flag)!=0; }
  inline void set_flags(unsigned int x) { flags = x; }
  inline unsigned int get_flags() const { return flags; }

  virtual bool is_pivot_valid() const      { return (flags & FLAG_PIVOT_VALID); }
  virtual bool is_entity_collision() const { return (flags & FLAG_ENTITY_COLLISION); }
  virtual bool is_camera_collision() const { return (flags & FLAG_CAMERA_COLLISION); }

  virtual unsigned int get_type() const;


  void rebuild_raw_cface_data(const stringx & meshname, bool allow_warnings)
  {
    build_raw_cface_data(meshname, allow_warnings);
  }

  virtual void estimate_physical_properties(entity * rb, rational_t material_density=DENSITY_OF_WATER);

protected:
  void sort_cfaces();

  vert_vect    raw_verts;
  vert_vect    verts;
  cface_vect    cfaces;  // now sorted by center X value
  vector<material*> materials;

  // sorted cface indexing
  rational_t cface_xmin;
  intvector lookup_low;
  intvector lookup_high;

  vector3d          pivot;
  unsigned int      flags;
  int lookup_size;

  rational_t radius;

private:
  void build_raw_cface_data(const stringx & meshname, bool allow_warnings);
  void compute_radius();
};


extern instance_bank<cg_mesh> cg_mesh_bank;


#endif
