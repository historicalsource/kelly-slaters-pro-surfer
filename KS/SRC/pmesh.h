#ifndef PMESH_H
#define PMESH_H

////////////////////////////////////////////////////////////////////////////////

// pmesh.h
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.

// progressive mesh declarations

// currently, all meshes are 'progressive'.  a non-progressive mesh is a
// progressive mesh where the detail_level of all polygons and wedges is zero.
// this is less efficient than a mesh structure that can store tristrips.

////////////////////////////////////////////////////////////////////////////////
#include <vector>

#include "vert.h"
#include "wedge.h"
#include "face.h"
#include "material.h"
#include "instance.h"
#include "visrep.h"
#include "color.h"
#include <map>

#if defined(TARGET_XBOX)
#define XBOX_USE_PMESH_STUFF 0
#endif /* TARGET_XBOX JIV DEBUG */

////////////////////////////////////////////////////////////////////////////////
//  forward class declarations
////////////////////////////////////////////////////////////////////////////////
class matrix4x4;
class geometry_manager;
class region;
class terrain;
class vert_normal;
class use_light_context;

////////////////////////////////////////////////////////////////////////////////
//  pmesh
////////////////////////////////////////////////////////////////////////////////

enum
{
  MX_COLORS,     // the first matrix is used for completely different sets
  MX_BONES       // of calculations than the rest.
};

enum
{
  MAX_BONES=80
};


class pmesh_normal
{
public:
  char x, y, z;
  inline pmesh_normal& operator	= ( const vector3d& v )	    // assignment of a pmesh_normal
  {
    x = (char)v.x; y = (char)v.y; z = (char)v.z;
    return *this;
  }
};


// vertex flags



enum
{
  CXL_DIFFUSELIGHT=1, // diffuse and ambient light (otherwise it's additive and self-lit)
  CXL_NOCLIP      =2
};


class vr_pmesh : public visual_rep
{
// Types
public:
  typedef map<material_ref,face_ref> material_map;

  enum
  {
    NORMAL=0,       // NORMAL means no lighting except for dynamic lights adding offset colors in the additive/specular slot
    FORCE_LIGHT=1,  // FORCE_LIGHT means lighting in the conventional sense:  vertex colors are multiplied by ambient and lighting is added in the diffuse slot
    ALLOW_SELF_LIT_MATERIALS=2, // this isn't supported anymore
    LIT_LAST_FRAME=4
  };

// Constructors
public:
  // empty mesh
  vr_pmesh( unsigned _mesh_flags = 0 );

  virtual ~vr_pmesh();

  // construct a pmesh consisting of a simple one-sided rectangle
  void make_rectangle();
  // construct a pmesh consisting of a collection of triangles
  void make_n_tangle(int n);
  // construct a pmesh consisting of a simple two-sided triangle
  void make_double_sided_triangle();

  static void init_pmesh_system( void );
  static void kill_pmesh_system( void );

// NEWENT File I/O
public:
  vr_pmesh( chunk_file& fs, unsigned _mesh_flags );
private:
  void load( chunk_file& fs );
  // this goes against the principle of serial_in being an
  // external function...we also have an external serial_in,
  // though
  void internal_serial_in( chunk_file& fs );
  bool read_stuff1(chunk_file& fs, chunk_flavor flavor_flav, bool & no_warnings, int & vert_count, int & svert_count, int & morph_wedge_count);
  bool read_stuff2(chunk_file& fs, chunk_flavor flavor_flav, int & last_wedges_lod, int & wedge_count, int & face_count, const stringx& texture_dir, int & vsplit_count);

// Old File I/O
public:
  // only used by debug compile to create visual_weapon.tpm:
  vr_pmesh( const char* pathname,
            unsigned _mesh_flags = NORMAL );
private:
  // whoever said you could copy these things?
  vr_pmesh( const vr_pmesh& );
  vr_pmesh& operator=( const vr_pmesh& );
private:
  void _construct( const stringx& pmesh_filename,
                   unsigned _mesh_flags );

// Misc.
public:
  // access verts, faces, wedges, and materials
  inline vector<vert> & get_verts() { return *verts; }

  // the old face interface is gone, to be replaced with this:
  wedge_ref get_wedge_ref( face_ref faceid, int corner ) const;
  material_ref get_material_ref( face_ref faceid ) const;
  unsigned short get_face_flags( face_ref faceid ) const;
  bool is_walkable( face_ref faceid ) const;
  bool is_notwalkable( face_ref faceid ) const;
  bool is_cosmetic( face_ref faceid ) const;
  bool is_water( face_ref faceid ) const;
  bool is_vegetation( face_ref faceid ) const;

  virtual bool is_uv_animated() const;

  unsigned char get_surface_type( face_ref faceid ) const;

  inline const vector3d& get_xvert_unxform( wedge_ref i ) const { return xverts[i].get_unxform(); }

  inline vector3d compute_face_normal( face_ref faceid ) const
  {
    const vector3d& v0 = get_xvert_unxform( get_wedge_ref( faceid, 0) );
    const vector3d& v1 = get_xvert_unxform( get_wedge_ref( faceid, 1) );
    const vector3d& v2 = get_xvert_unxform( get_wedge_ref( faceid, 2) );
    return cross(v1-v0, v2-v0);//.normalize(); // weighted by twice face area... not unit length!
  }

  inline const vert& get_vert( vert_ref i )             const { return (*verts)[i]; }
//!!    const face& get_face( face_ref i )             const { return faces[i]; }
//!!    const face& get_unsorted_face( face_ref i )    const { return faces[original_face_for_face_slot[i]]; }//faces[faces[i].original_occupant_of_my_face_slot]; }
  inline const wedge& get_wedge( wedge_ref i )          const { return wedges[i]; }
  inline const material& get_material( material_ref i ) const { return *(materials[i]); }
  inline const hw_rasta_vert& get_xvert( wedge_ref i )  const { return xverts[i]; }

  // access pointers to verts, faces, wedges, and materials
  inline const vert     * get_vert_ptr(vert_ref i)          const { return &((*verts)[i]); }
//!!    const face     * get_face_ptr(face_ref i)          const { return &faces[i]; }
//    const face     * get_unsorted_face_ptr(face_ref i)     const { return &faces[faces[i].original_occupant_of_my_face_slot]; }
  inline const wedge    * get_wedge_ptr(wedge_ref i)        const { return &wedges[i]; }
  inline const material * get_material_ptr(material_ref i) const { return materials[i]; }

  inline void set_vert(int i,vert v) { (*verts)[i] = v; }
  inline void set_wedge(int i,wedge & w)  { wedges[i] = w; }
  inline vector3d get_xvert_unxform_pos(int i) const { return xverts[i].xyz; }
  inline void set_xvert_tc(int i, const texture_coord& tex) { xverts[i].tc[0] = tex; }
  inline void set_xvert_unxform_pos(int i,vector3d p) { xverts[i].xyz = p; }
  inline void set_xvert_xform_pos(int i,vector3d p)
  {
#if defined( TARGET_PC) || XBOX_USE_PMESH_STUFF
    xverts[i].xyz = p;
#else
    xverts[i].xyz.x = p.x;
    xverts[i].xyz.y = p.y;
#endif
  }
  inline void set_xvert_unxform_diffuse(int i,color32 c) { xverts[i].diffuse = c; }

  inline int get_num_verts()     const { return verts ? verts->size() : 0; }
  //int get_min_faces()     const { return min_faces; }
  //int get_max_faces()     const { return num_faces; }
  inline int get_num_wedges()    const { return num_wedges; }
  inline int get_num_materials() const { return materials.size(); }

  inline int get_min_faces( time_value_t delta_t = 0) const { return min_faces; }
  inline int get_max_faces( time_value_t delta_t = 0) const { return num_faces; }

  virtual const vector3d& get_center( time_value_t delta_t ) const { return center; }
  virtual rational_t get_radius( time_value_t delta_t ) const { return radius; }

  // compute visual center and radius for skin
  void compute_info( po* bones, int num_bones );

  virtual rational_t compute_xz_radius_rel_center( const po& xform );

  // set mesh that we're morphing to.  not in the constructor because
  // we generally don't know when we're building.
  //void set_morph_result(vr_pmesh* _result) { morph_result = _result; }
  //vr_pmesh* get_morph_result() const { return morph_result; }
  // the morph_result is just a link, we don't own/contain it.

  //void set_morph_duration(time_value_t t) { morph_duration = t; }
  //time_value_t get_morph_duration() const { return morph_duration; }

  // the maximum detail scalar...each increment in detail is
  // a vertex split.  you can roughly estimate the number of polys
  // by multiplying max_detail by 2...
  virtual int get_max_detail()  const { return max_detail; }
  int         get_min_detail()  const { return min_detail; }

  // spit polys to display.  you must be between
  // ras_begin_new_scene and ras_page_flip calls
  // attempted_number_of_polys is the number of polys you're
  // going to try to hit when you're rendering.
	virtual void render_instance( render_flavor_t render_flavor, instance_render_info* iri, short *ifl_lookup = NULL );

  //virtual void render_batch(render_flavor_t flavor,
  //                          instance_render_info* viri,
  //                          int num_instances);

  virtual void render_skin(render_flavor_t render_flavor,
                           const instance_render_info* iri,
                           const po* bones,
                           int num_bones);

  void render_normals(const instance_render_info* iri);

  void render_skin_normals(const instance_render_info* iri,
                           const po* bones);

  inline const vector3d& get_pivot() const { return pivot; }
  inline bool is_pivot_valid() const { return pivot_valid; }

//!!    void munge_pmesh( const vr_pmesh& newmesh, const vector3d& offset, bone_idx bone );
//!!    bool is_already_munged() const {return already_munged;}

  void shrink_memory_footprint();

  void build_collision_data();

  inline bool get_has_translucent_verts() const { return has_translucent_verts; }

  // only use this if you know what you're doing!
  inline void set_has_translucent_verts( bool v ) { has_translucent_verts = v; }

  inline bool force_light() const { return mesh_flags & FORCE_LIGHT; }

  inline void set_mesh_flags(unsigned _mesh_flags) { mesh_flags = _mesh_flags; }
  inline unsigned get_mesh_flags() const { return mesh_flags; }

  virtual void set_light_method( light_method_t lmt );

  class bone
  {
    public:
      stringx name;
      po      pivot;
  };

	void clip_xform_and_light( hw_rasta_vert* vert_list_begin,
                    hw_rasta_vert* vert_list_end,
//                      pmesh_normal* normal_list_begin,
                    unsigned clip_flags,
                    int dir_lights,
                    int num_point_lights,
                    int alpha,
                    const po& world2local,
                    const po& view2local,
                    bool icon_render,
                    use_light_context *lites);

  inline int get_num_bones() const { return bones.size(); }
  inline const stringx&  get_bone_name( int bone_no );
  inline const po& get_bone_pivot( int bone_no );

  // internal utility functions
	int calculate_integral_detail_level(instance_render_info* iri);

#if defined(TARGET_PC)  || XBOX_USE_PMESH_STUFF
  void optimize_static_mesh_for_d3d();
  bool is_optimized_static_mesh_for_d3d() const { return optimized_verts!=NULL; }

  static unsigned optdisallow;
  static bool allow_optimize_static_mesh_for_d3d() { return optdisallow==0; }
  static void push_optimize_static_mesh_for_d3d_disallowed() { ++optdisallow; }
  static void pop_optimize_static_mesh_for_d3d_disallowed() { assert(optdisallow); --optdisallow; }
#endif

  void prepare_for_environment_pass(material *mat);  // prepare vert workspace for environment map render
  void prepare_for_extra_diffuse_pass(material *mat, int _map);  // prepare vert workspace for light map render
  void undo_uv_damage();
  void undo_skin_uv_damage();

protected:
  vector<vert>      *verts;

protected:
  hw_rasta_vert     *xverts;  // this is the standard vert that programmers are used to seeing

#if defined(TARGET_PC)
  vert_buf*         optimized_verts; // for holding verts after optimize_static_mesh_for_d3d()
#endif
#if XBOX_USE_PMESH_STUFF
  vert_lit_buf*         optimized_verts; // for holding verts after optimize_static_mesh_for_d3d()
#endif
  wedge             *wedges;   // this is a parallel array to the verts containing extra info
  int               min_faces;
  // table of how many xverts to xform for a given level of detail
  int*              xverts_for_lod;
  int               num_wedges;  // also num_xverts

  face              *faces;          // face includes which three wedges it contains until that information is split into
  reduced_face      *reduced_faces;  // reduced faces and wedge_index_list.  Then reduced_face[i] corresponds to
  int               num_faces;       // wedge_index_list[i*3], [i*3+1], and [i*3+2].

  vector<material*> materials;
  vert_ref*         vert_refs_for_wedge_ref;  // which vert goes with which wedge
  wedge_ref*        wedge_index_list;
  short*            wedge_lod_starts;        // table of the highest lod for which a given wedge is visible
protected:

  face_ref*         original_face_for_face_slot;

  // where the face switches to the next material
  material_map      material_changes;

  vector<bone>      bones;

  bool              has_translucent_verts;
  bool              progressive;             // progressive meshes aren't necessarily progressive.

  int uvanim_update_frame;

  vector3d          center;    // center of bounding box of verts, in same space as verts are)
  rational_t        radius;    // radius from apparent center

  vector3d          pivot;
  int               pivot_valid;

  stringx filename;		// for debugging

  unsigned mesh_flags;

  virtual void compute_info();

  void rescale_verts(rational_t s);

  void optimize();
  void mark_self_lit_verts();

public:

  int  max_detail;
  int  min_detail;

  // render all faces of a given material.  generally, for any given execution
  // only one of these functions will be the one in use
  void    render_material_entity_noclip( material_ref sr,material_map::iterator mi,
                                         //time_value_t time,
                                         int level_of_detail, instance_render_info* iri );

  void    render_material_clipped( material_ref sr,
                                   vector<bool>* clipped_verts,
                                   color32 color_scale ) const;

	void 		render_material_clipped_full_detail(
			instance_render_info* iri,
			material_ref material_idx,
			material_map::iterator mi,
			color32 color_scale32,
//			rational_t frame,
			bool render_diffuse_map,
			bool render_extra_map[MAPS_PER_MATERIAL],
			bool render_environment_map );

  void 		render_one_material_clipped_full_detail(
			instance_render_info* iri,
			material_ref material_idx,
			material_map::iterator mi,
			color32 color_scale32,
//			rational_t frame,
			bool render_diffuse_map,
			bool render_environment_map,
      int i,
      hw_rasta_vert* & tvlit,
      vert_normal* & normalit);

  void    render_material_entity_progressive( material_ref sr,
                                             int detail_level,
                                             float detail_fraction,
                                             float morph_fraction,
                                             time_value_t time,
                                             instance_render_info* iri,
                                             color32 color_scale = color32(255,255,255,255) );
  void    render_one_material_entity_progressive( material_ref sr,
                                             int detail_level,
                                             float detail_fraction,
                                             float morph_fraction,
                                             time_value_t time,
                                             color32 color_scale,
										     int i,
    									     hw_rasta_vert* & tvlit,
										     vert_normal* & normalit);

  virtual int get_anim_length() const;

  // dot producting vertices to see where they fall
  void clip_verts(const po& world2local, const po& view2local, hw_rasta_vert* start, int vcount) const;

  void clear_lighting();

  void anim_uvs( time_value_t t );

  virtual render_flavor_t render_passes_needed() const;
  virtual bool get_distance_fade_ok() const { return !has_translucent_verts; }

  static geometry_manager* l_geometry_pipe;

  friend class cg_mesh;
  friend class beam;

  friend unsigned short* c_skin_tri_setup_prog( vr_pmesh* me, int start, int end, hw_rasta_vert_lit* vert_begin, unsigned short* index_ptr, int level_of_detail );
  friend void set_global_brightness( rational_t brightness );
  friend void undo_global_brightness_set( rational_t previous_brightness_value );
  friend void set_global_linear_brightness( rational_t brightness );

  friend void serial_in( chunk_file& fs, vr_pmesh* mesh );
  // The following added for debugging purposes only
  friend void serial_in( chunk_file& fs, region * r );
  friend class region;
  friend class terrain;
};

void serial_in( chunk_file& fs, vr_pmesh* mesh );

const chunk_flavor CHUNK_VERT_REF("vertref");
const chunk_flavor CHUNK_WEDGE_REF("wedgeref");
const chunk_flavor CHUNK_TMESH("tmesh");
const chunk_flavor CHUNK_MESH_FLAGS( "meshflag" );

extern instance_bank<vr_pmesh> vr_pmesh_bank;

void initialize_mesh_stuff();

void set_global_brightness( rational_t brightness );

// This function will NOT work properly if set_global_linear_brightness has been used
void undo_global_brightness_set( rational_t previous_brightness_value );

void set_global_linear_brightness( rational_t brightness );

#if defined(TARGET_PC)  || XBOX_USE_PMESH_STUFF
enum { MAX_VERTS_PER_PRIMITIVE=65536 };
#elif defined(TARGET_MKS)
enum { MAX_VERTS_PER_PRIMITIVE=VIRTUAL_MAX_VERTS_PER_PRIMITIVE };  // we increased this because it gets used by particle generators
#else
enum { MAX_VERTS_PER_PRIMITIVE=65536 };
#endif

#endif // PMESH_H
