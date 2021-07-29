#ifndef _MESH_GRID_H_
#define _MESH_GRID_H_

#include "pmesh.h"

#define USE_JDB_MESH_GRIDS 0

#define MESH_GRID_X			(int)3
#define MESH_GRID_Z			(int)3
#define MESH_NUM_GRIDS		MESH_GRID_X*MESH_GRID_Z

//#define MAX_MESH_GRID_X		(int)10
//#define MAX_MESH_GRID_Z		(int)10
//#define MAX_MESH_NUM_GRIDS	MAX_MESH_GRID_X*MAX_MESH_GRID_Z

#define assert_bucket(x, z)		    assert((int)x >= 0 && (int)x < MESH_GRID_X && (int)z >= 0 && (int)z < MESH_GRID_Z)
#define mesh_bucket_index(x, z)		(int)(((int)z*MESH_GRID_X)+(int)x)

class mesh_grid
{
private:

protected:
  struct mesh_grid_sphere
    {
    rational_t x;  
    rational_t z;  
    rational_t rad;  
    };

  struct mat_render_helper
    {
    short vert_index[MESH_NUM_GRIDS];
    short num_verts[MESH_NUM_GRIDS];
    short material_offset;
    short total_verts;
    };


  char mesh_visible[MESH_NUM_GRIDS];
  vr_pmesh *mesh_buckets[MESH_NUM_GRIDS];
  mesh_grid_sphere mesh_bucket_spheres[MESH_NUM_GRIDS];

  vector<material*>  materials;
 
  mat_render_helper *mat_helpers;

  float old_radius;
  unsigned old_mesh_flags;

  vector3d grid_origin;
  rational_t grid_x_size;
  rational_t grid_z_size;

public:
  mesh_grid(terrain *ter, region *rgn);
  ~mesh_grid();

  void face_swap(region *rgn, face_ref old_face, vr_pmesh *new_mesh, face_ref new_face);

  void render(render_flavor_t flavor, region_node* rn);

  int sort_face(vr_pmesh *mesh, wedge_ref *wedge_refs);
  void compute_grid_extents();

/*
  void clip_xform_and_light(vr_pmesh *grid_mesh,
                            int xvert_offset,
                            unsigned lighting_flags,
                            unsigned cxl_flags, 
                            int dir_lights,
                            int num_point_lights,
                            int alpha,
                            const po& clip_world2view,
                            const po& clip_local2view);
*/

void clip_xform_and_light(hw_rasta_vert* my_verts,
                          //pmesh_normal *my_norms,
                          int num_verts,
                          int workspace_offset,
                          unsigned lighting_flags,
                          unsigned cxl_flags, 
                          int dir_lights,
                          int num_point_lights,
                          int alpha,
                          const po& clip_world2view,
                          const po& clip_local2view);

/*
void render_material_clipped_full_detail( 
    vr_pmesh *mesh,
    int xvert_offset,
    instance_render_info* iri,
	material_ref material_idx, 
	vr_pmesh::material_map::iterator mi, 
	color32 color_scale32,
	bool render_diffuse_map, 
	bool render_environment_map );
void render_material_entity_noclip( vr_pmesh *mesh, int xvert_offset, material_ref material_idx, vr_pmesh::material_map::iterator mi,int level_of_detail, instance_render_info* iri );
*/

};

#endif