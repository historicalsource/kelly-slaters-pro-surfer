#include "global.h"

#include "osalloc.h"
#include "hwmath.h"
#include "algebra.h"
//!#include "character.h"
#include "mesh_grid.h"
#include "app.h"
#include "geomgr.h"
#include "color.h"
#include "hwrasterize.h"
#include "iri.h"
#include "osdevopts.h"
#include "oserrmsg.h"
#include "profiler.h"
#include "vsplit.h"
#include "wds.h"
#include "vertwork.h"
#include "vertnorm.h"
#include "maxiri.h"
#include "light.h"
#include "pmesh.h"
#include "terrain.h"
#include "vsplit.h"
#include "renderflav.h"
#include "clipflags.h"

#include <algorithm>

#if defined (TARGET_PC)
extern vert_buf non_indexed_workspace;
#endif

#if defined(TARGET_PC)
enum {MAX_VERTS_PER_PRIMITIVE=65536};
#endif

#if defined(TARGET_MKS)
enum {MAX_VERTS_PER_PRIMITIVE=VIRTUAL_MAX_VERTS_PER_PRIMITIVE};  // we increased this because it gets used by particle generators
#endif

extern unsigned short indices[MAX_VERTS_PER_PRIMITIVE];


unsigned short* c_skin_tri_setup_nonprog( face* face_start, int face_count, hw_rasta_vert* vert_begin, unsigned short* index_pool );
unsigned short* c_skin_tri_setup_prog( vr_pmesh* me, int start,int end, hw_rasta_vert* vert_begin, unsigned short* index_pool, int level_of_detail );

extern matrix4x4 w2v;
extern matrix4x4 v2vp;
extern matrix4x4 total_xform;

extern vector3d bone_lights[MAX_BONES][ABSOLUTE_MAX_LIGHTS];


#if defined(TARGET_MKS)
extern "C"
{
void asm_xvertcopy(hw_rasta_vert* dest,hw_rasta_vert* src,int count);
void asm_xform_uni_alpha(hw_rasta_vert* vert_list, int count, matrix4x4* xform, hw_rasta_vert* vert_out, unsigned char universal_alpha);
void asm_xform_offset_into_diffuse(hw_rasta_vert* vert_list, int count, matrix4x4* xform, hw_rasta_vert* vert_out, unsigned char universal_alpha);
void asm_xform_vertex_alpha(hw_rasta_vert* vert_list, int count, matrix4x4* xform, hw_rasta_vert* vert_out );
void asm_xform_skin(hw_rasta_vert* vert_list, int count, matrix4x4 xforms[], hw_rasta_vert* vert_out );


bool asm_clip_verts( hw_rasta_vert* tvlit,
                     int vertex_count,
                     float near_plane_d,
                     float far_plane_d,
                     vector3d* xformed_facing_camera,
                     plane* plane_table,
                     vector3d* camera_origin,
                     int* all_clipped );             // returns whether some_front_clipped

unsigned short* asm_skin_tri_setup_prog( vr_pmesh* me, int start,int end, hw_rasta_vert* vert_begin, unsigned short* index_pool, int level_of_detail );

void flush_cache();

};

#endif

#define XFORM( sv, mat, dv ) \
  dv.x  = mat[0][0]*sv.x+mat[1][0]*sv.y+mat[2][0]*sv.z+mat[3][0];  \
  dv.y  = mat[0][1]*sv.x+mat[1][1]*sv.y+mat[2][1]*sv.z+mat[3][1];  \
  dv.z  = mat[0][2]*sv.x+mat[1][2]*sv.y+mat[2][2]*sv.z+mat[3][2];


void xform_and_light_skin( hw_rasta_vert* vert_list_begin,
                           hw_rasta_vert* vert_list_end,  // must be a pointer into vert_workspace
                           //pmesh_normal* normal_list_begin,
                           bool light,
                           matrix4x4* bones_proj,   // xforms for projection
                           matrix4x4* bones_world,
                           short* wedge_lod_starts,
                           int level_of_detail,
                           use_light_context *lites
                            ); // xforms for lights
void store_xforms_statically();

extern bool all_clipped;
extern bool needs_front_clip;  // in lieu of passing parameters.  Might be slower, but no .h file!


void mesh_grid::face_swap(region *rgn, face_ref old_face, vr_pmesh *new_mesh, face_ref new_face)
{
  vector<cface_replacement>::iterator si = rgn->sorted.begin();
  vector<cface_replacement>::const_iterator si_end = rgn->sorted.end();

  for( ; si != si_end; ++si)
  {
    if((*si).pP == rgn->my_vr_pmesh && (*si).rF == old_face)
    {
      (*si).set(new_mesh, new_face);
      break;
    }
  }


  si = rgn->sorted_water.begin();
  si_end = rgn->sorted_water.end();

  for( ; si != si_end; ++si)
  {
    if((*si).pP == rgn->my_vr_pmesh && (*si).rF == old_face)
    {
      (*si).set(new_mesh, new_face);
      break;
    }
  }
}


material_ref old_material = -1;


#define MY_MAX_VERTS 4000
hw_rasta_vert my_verts[MY_MAX_VERTS];
wedge_ref my_start_index[MY_MAX_VERTS];

mesh_grid::mesh_grid(terrain *ter, region *rgn)
{
  vr_pmesh *mesh = rgn->get_vr_pmesh();

  materials.clear();
  for(int m=0;m<mesh->materials.size();++m)
  {
    materials.push_back( material_bank.new_instance(mesh->materials[m]) );
  }

  grid_origin = ZEROVEC;
  grid_x_size = 0.0f;
  grid_z_size = 0.0f;

  int i = 0;
  for(i = 0; i < MESH_NUM_GRIDS; ++i)
  {
    mesh_buckets[i] = NULL;
  }

  // get the extents of the region
  rational_t min_x;
  rational_t max_x;
  rational_t min_z;
  rational_t max_z;
  int first_pass = 1;

  int num_verts = mesh->get_num_wedges();
  int num_faces = mesh->get_max_faces();

  for(i = 0; i<num_verts; ++i)
  {
    vector3d pt = mesh->get_xvert_unxform(i);

    if(first_pass || pt.x > max_x)
      max_x = pt.x;
    if(first_pass || pt.x < min_x)
      min_x = pt.x;

    if(first_pass || pt.z > max_z)
      max_z = pt.z;
    if(first_pass || pt.z < min_z)
      min_z = pt.z;

    first_pass = 0;
  }

  grid_origin.x = min_x;
  grid_origin.z = min_z;
  grid_x_size = (rational_t)((max_x - min_x) / (rational_t)MESH_GRID_X);
  grid_z_size = (rational_t)((max_z - min_z) / (rational_t)MESH_GRID_Z);


  int faces_per_bucket[MESH_NUM_GRIDS];
  material_ref cur_materials[MESH_NUM_GRIDS];
  int face_indices[MESH_NUM_GRIDS];

  for(i = 0; i < MESH_NUM_GRIDS; ++i)
  {
    faces_per_bucket[i] = 0;
    face_indices[i] = 0;
    cur_materials[i] = UNINITIALIZED_MATERIAL_REF;
  }

  for(i = 0; i<num_faces; ++i)
    ++faces_per_bucket[sort_face(mesh, &mesh->wedge_index_list[i*3] )];

  for(i = 0; i<MESH_NUM_GRIDS; ++i)
  {
    if(faces_per_bucket[i] > 0)
    {
      mesh_buckets[i] = NEW vr_pmesh();

      mesh_buckets[i]->num_faces = faces_per_bucket[i];
      mesh_buckets[i]->min_faces = mesh_buckets[i]->num_faces;
      mesh_buckets[i]->num_wedges = mesh_buckets[i]->num_faces * 3;

      mesh_buckets[i]->faces = (face*)os_malloc32x( mesh_buckets[i]->num_faces*sizeof(face) );  //vector<face>(nfaces);

      mesh_buckets[i]->wedges = (wedge*)os_malloc32x( mesh_buckets[i]->num_wedges*sizeof(wedge) );  //  hope we get wedge to multiple of 32...

      mesh_buckets[i]->xverts = (hw_rasta_vert*)os_malloc32x( mesh_buckets[i]->num_wedges*sizeof(hw_rasta_vert) );//NEW hw_rasta_vert[nwedges];

      mesh_buckets[i]->vert_refs_for_wedge_ref = NULL;

      mesh_buckets[i]->wedge_lod_starts = NULL;

      mesh_buckets[i]->xverts_for_lod = NEW int[1];
      mesh_buckets[i]->xverts_for_lod[0] = mesh_buckets[i]->num_wedges;

      mesh_buckets[i]->materials.clear();
      for(int m=0;m<mesh->materials.size();++m)
      {
        mesh_buckets[i]->materials.push_back( material_bank.new_instance(mesh->materials[m]) );
      }
    }
  }

  for(i = 0; i<num_faces; ++i)
  {
    int bucket_index = sort_face(mesh, &mesh->wedge_index_list[i*3]);

/*
    material_ref new_material = mesh->faces[i].get_material_ref();

    if ( new_material != cur_materials[bucket_index] )
    {
      // insert value representing face index at which this material starts
      mesh_buckets[i]->material_changes.insert( vr_pmesh::material_map::value_type( new_material, face_indices[bucket_index] ) );
      cur_materials[bucket_index] = new_material;
    }
*/

    int face_index = face_indices[bucket_index];
    int vert_index = face_index*3;

    int mesh_vert_index[3];
    mesh_vert_index[0] = mesh->wedge_index_list[i*3];
    mesh_vert_index[1] = mesh->wedge_index_list[i*3+1];
    mesh_vert_index[2] = mesh->wedge_index_list[i*3+2];

    mesh_buckets[bucket_index]->faces[face_index].my_material     = mesh->reduced_faces[i].my_material;
    mesh_buckets[bucket_index]->faces[face_index].level_of_detail = mesh->reduced_faces[i].level_of_detail;
    mesh_buckets[bucket_index]->faces[face_index].flags           = mesh->reduced_faces[i].flags;

    for(int j=0; j<3; ++j)
    {
      mesh_buckets[bucket_index]->faces[face_index].wedge_refs[j] = vert_index + j;

      mesh_buckets[bucket_index]->wedges[vert_index + j].level_of_detail   = mesh->wedges[mesh_vert_index[j]].level_of_detail;

//      mesh_buckets[bucket_index]->xverts[vert_index + j]   = mesh->xverts[mesh_vert_index[j]];
      memcpy(&mesh_buckets[bucket_index]->xverts[vert_index + j], &mesh->xverts[mesh_vert_index[j]], sizeof(hw_rasta_vert));

//      memcpy(&mesh_buckets[bucket_index]->normals[vert_index + j], &mesh->normals[mesh_vert_index[j]], sizeof(pmesh_normal));
    }

//    mesh_buckets[bucket_index]->faces[face_index].wedge_refs[0] = vert_index;
//    mesh_buckets[bucket_index]->faces[face_index].wedge_refs[1] = vert_index + 1;
//    mesh_buckets[bucket_index]->faces[face_index].wedge_refs[2] = vert_index + 2;

//    mesh_buckets[bucket_index]->wedges[vert_index].level_of_detail   = mesh->wedges[mesh_vert_index[0]].level_of_detail;
//    mesh_buckets[bucket_index]->wedges[vert_index+1].level_of_detail = mesh->wedges[mesh_vert_index[1]].level_of_detail;
//    mesh_buckets[bucket_index]->wedges[vert_index+2].level_of_detail = mesh->wedges[mesh_vert_index[2]].level_of_detail;

//    mesh_buckets[bucket_index]->xverts[vert_index]   = mesh->xverts[mesh_vert_index[0]];
//    mesh_buckets[bucket_index]->xverts[vert_index+1] = mesh->xverts[mesh_vert_index[1]];
//    mesh_buckets[bucket_index]->xverts[vert_index+2] = mesh->xverts[mesh_vert_index[2]];

//    mesh_buckets[bucket_index]->normals[vert_index]   = mesh->normals[mesh_vert_index[0]];
//    mesh_buckets[bucket_index]->normals[vert_index+1] = mesh->normals[mesh_vert_index[1]];
//    mesh_buckets[bucket_index]->normals[vert_index+2] = mesh->normals[mesh_vert_index[2]];

    ++face_indices[bucket_index];
  }


  for(i=0; i<MESH_NUM_GRIDS; ++i)
  {
    if(mesh_buckets[i] != NULL)
    {
      mesh_buckets[i]->optimize();
      mesh_buckets[i]->mark_self_lit_verts();
      mesh_buckets[i]->compute_info();
    }
  }


  for(i = 0; i < MESH_NUM_GRIDS; ++i)
    face_indices[i] = 0;

  for(i = 0; i<num_faces; ++i)
  {
    int bucket_index = sort_face(mesh, &mesh->wedge_index_list[i*3]);
    int face_index = face_indices[bucket_index];

    face_swap(rgn, i, mesh_buckets[bucket_index], face_index);

    ++face_indices[bucket_index];
  }

  compute_grid_extents();

  old_radius = mesh->radius;
  old_mesh_flags = mesh->mesh_flags;


  static int liu = 0;

  if(!liu)
  {
    for(int i=0; i<MY_MAX_VERTS; ++i)
      my_start_index[i] = i;

    liu = 1;
  }


  mat_helpers = NEW mat_render_helper[materials.size()];

  int material_idx = 0;
  for(material_idx = 0; material_idx < materials.size(); ++material_idx)
  {
    for(int i = 0; i<MESH_NUM_GRIDS; ++i)
    {
      mat_helpers[material_idx].vert_index[i] = -1;
      mat_helpers[material_idx].num_verts[i] = -1;
    }
  }

  material_idx = 0;

//  vector<material*>::iterator mat_i = materials.begin();
//  vector<material*>::const_iterator mat_i_end = materials.end();

//  for( ; mat_i != mat_i_end; ++mat_i, ++material_idx)
//  {
    for(i = 0; i<MESH_NUM_GRIDS; ++i)
    {
      if(mesh_buckets[i] != NULL)
      {
        vr_pmesh::material_map::iterator mi = mesh_buckets[i]->material_changes.begin();
        vr_pmesh::material_map::iterator mi_end = mesh_buckets[i]->material_changes.end();

        for( ; mi != mi_end; ++mi)
        {
          int material_idx_check = (*mi).first;

          if(material_idx_check != INVISIBLE_MATERIAL_REF)
          {
            int start_face = (*mi).second;

            int end_face;
            vr_pmesh::material_map::iterator next = mi;
            ++next;
            if ( next == mesh_buckets[i]->material_changes.end() )
              end_face = mesh_buckets[i]->get_max_faces();
            else
              end_face = (*next).second;

            mat_helpers[material_idx_check].vert_index[i] = start_face*3;
            mat_helpers[material_idx_check].num_verts[i] = (end_face - start_face)*3;
          }
        }
      }
    }
//}



}

mesh_grid::~mesh_grid()
{
  int i;

  for (i=0;i<materials.size();++i)
  {
    material_bank.delete_instance( materials[i] );
  }

  for(i = 0; i<MESH_NUM_GRIDS; ++i)
  {
    if(mesh_buckets[i] != NULL)
    {
      delete mesh_buckets[i];
      mesh_buckets[i] = NULL;
    }
  }

  if(mat_helpers)
  {
    delete []mat_helpers;
    mat_helpers = NULL;
  }

}


void mesh_grid::render(render_flavor_t flavor, region_node* rn)
{
  old_material = -1;

  int i;

  instance_render_info iri( 0,
                            po_identity_matrix.get_matrix(),
                            0.0F,
                            rn );

  // exit early if we're forcing everything translucent and this isn't the translucent pass
#if !defined(TARGET_MKS)
  if(iri.force_translucent() && ( (!(flavor & RENDER_TRANSLUCENT_PORTION)) && (!(flavor & RENDER_RAW_ICON_THROUGHPUT))) )
  {
    assert(false);
    return;
  }
#endif


  geometry_manager::inst()->set_local_to_world(iri.get_local_to_world().get_matrix());

  store_xforms_statically();

  po world2local = iri.get_local_to_world().inverse(); // this inverse didn't used to be here.  bug?  see pmesh.cpp, which *does* have the inverse call.

  light_manager* light_set = iri.get_light_set();

/////////////
  if ((flavor&RENDER_NO_LIGHTS) || (iri.get_force_flags()&FORCE_NO_LIGHT))
  {
    lites.clear_lights();
  }
  else
  {
    if( !light_set )
    {
      region_node *my_region = iri.get_region();

      bool found;

      if(my_region)
        found = my_region->get_data()->has_affect_terrain_lights();
      else
        found = true;

      if(found)
      {
        lites.reset_lights( iri.get_region(), iri.get_local_to_world(),
                            old_radius, !(old_mesh_flags & FORCE_LIGHT) );
      }
      else
      {
        lites.clear_lights();
      }
    }
    else
    {
      light_set->compute(iri.get_local_to_world().get_abs_position(),old_radius);
    }
  }



  size_t ndirlights=lites.dir_lights.size();
  size_t npointlights = min( ABSOLUTE_MAX_LIGHTS - ndirlights, lites.light_dists.size() );


  int j;
  assert( (ndirlights + npointlights) <= ABSOLUTE_MAX_LIGHTS);

  for(j=0;j<ndirlights;++j)
  {
    bone_lights[0][j] = lites.dir_lights[j].dir;  // ndirlights already xformed by reset_lights or use_light_set
  }

  for(j=0;j<npointlights;++j)
  {
    int light_idx = lites.light_dists[j].second;

    XFORM( lites.world_point_lights[light_idx].pos, world2local.get_matrix(), bone_lights[0][ndirlights+j] );

    lites.light_props[ ndirlights+j ] = lites.props_of_point_lights[light_idx];

    if( os_developer_options::inst()->is_flagged( os_developer_options::FLAG_FAKE_POINT_LIGHTS ) && (!(flavor&RENDER_REAL_POINT_LIGHTS)) )
    {
      rational_t distance = bone_lights[0][ndirlights+j].length()+0.0001f;

      bone_lights[0][ndirlights+j] /= distance;  // normalized and inverted

      distance /= lites.light_props[ndirlights+j].get_near_range();

      if( distance<1 ) distance=1;

      rational_t invdist = 1.0F/distance;

      lites.light_props[ ndirlights+j ].diffuse_color *= invdist;  // attenuated
      lites.light_props[ ndirlights+j ].additive_color *= invdist;
    }
  }


  color32 color_scale32 = iri.get_color_scale();

  bool entity_needs_clipping = false;
  all_clipped = false;
  po clip_view2local;
  po clip_world2local; // right now this never gets used, but let's be safe
  unsigned cxl_flags = 0;

  if( flavor&RENDER_CLIPPED_FULL_DETAIL )
  {
    cxl_flags |= CXL_NOCLIP;  // wait...what DO we clip?
    needs_front_clip = true;
  }
  else
  {
    if( (!iri.get_skip_clip()) && (!(flavor&RENDER_RAW_ICON_THROUGHPUT)) )
    {
      needs_front_clip = true;
      cxl_flags |= CXL_NOCLIP;
    }
    else
    {
      cxl_flags |= CXL_NOCLIP;
      needs_front_clip = false;
    }
  }

  if ((old_mesh_flags & FORCE_LIGHT))
    cxl_flags |= CXL_DIFFUSELIGHT;

  if( os_developer_options::inst()->is_flagged( os_developer_options::FLAG_FAKE_POINT_LIGHTS ) && (!(flavor&RENDER_REAL_POINT_LIGHTS)) )
  {
    ndirlights += npointlights;
    npointlights = 0;
  }

  color ambcolor = (cxl_flags&CXL_DIFFUSELIGHT) ? lites.ambient_factor : color(1.0f,1.0f,1.0f,1.0f);

  // total_xform transform from object space to the viewport.
  total_xform = w2v * v2vp;

  unsigned lighting_flags = (cxl_flags&CXL_DIFFUSELIGHT)? LIGHT_DIFFUSE : LIGHT_ADDITIVE;
  prepare_lighting_matrices( ambcolor,
                             lites.light_props,
                             ndirlights,
                             npointlights,
                             lighting_flags,
                             1 );


  ///// end onetime

  //////// begin multipletime
  for(i = 0; i<MESH_NUM_GRIDS; ++i)
  {
    mesh_visible[i] = (mesh_buckets[i] != NULL); // && g_world_ptr->sphere_in_frustrum(mesh_bucket_spheres[i].x, mesh_bucket_spheres[i].z, mesh_bucket_spheres[i].rad))

#if 0
    if((i%2 == 1 && (i / MESH_GRID_X)%2 == 0) || (i%2 == 0 && (i / MESH_GRID_X)%2 == 1))
      mesh_visible[i] = 0;
#endif
  }


  vector<material*>::iterator mat_i = materials.begin();
  vector<material*>::const_iterator mat_i_end = materials.end();

  int material_offset = 0;
  int material_idx = 0;
  int nmaterials=materials.size();
  for (int mat=nmaterials; --mat>=0; )
  {
    int total_offset = 0;

    for(int i = 0; i<MESH_NUM_GRIDS; ++i)
    {
      if(mesh_visible[i] && mat_helpers[mat].vert_index[i] != -1)
      {

        profiler_start_timer(profiler::PTM_GENERIC1);

        clip_xform_and_light(&mesh_buckets[i]->xverts[mat_helpers[mat].vert_index[i]],
                           //&mesh_buckets[i]->normals[mat_helpers[mat].vert_index[i]],
                           mat_helpers[mat].num_verts[i],
                           material_offset+total_offset,
                           lighting_flags,
                           cxl_flags,
                           ndirlights,
                           npointlights,
                           color_scale32.c.a,
                           clip_world2local,
                           clip_view2local );

        profiler_stop_timer(profiler::PTM_GENERIC1);

        total_offset += mat_helpers[mat].num_verts[i];
      }
    }

    mat_helpers[mat].material_offset = material_offset;
    mat_helpers[mat].total_verts = total_offset;
    material_offset += total_offset;
  }

  mat_i = materials.begin();
  material_idx = 0;
  for( ; mat_i != mat_i_end; ++mat_i, ++material_idx)
  {
    bool render_diffuse_map;
    bool render_environment_map;

  #if defined(TARGET_MKS)
    render_diffuse_map = true;
    render_environment_map = true;
  #else
    bool is_xluc = iri.force_translucent() || (*mat_i && (*mat_i)->is_translucent());
    bool pass_xluc = (flavor & (RENDER_TRANSLUCENT_PORTION|RENDER_RAW_ICON_THROUGHPUT))!=0;

    render_diffuse_map = is_xluc == pass_xluc;
    render_environment_map = pass_xluc;
  #endif

    if(!(*mat_i)->has_environment_map())
      render_environment_map = false;

    profiler_start_timer( profiler::PTM_RGN_DRAW );

    if(render_diffuse_map || render_environment_map)
    {
      if( needs_front_clip )
      {
        if(mat_helpers[material_idx].total_verts)
        {
          int frame = iri.time_to_frame_locked((*mat_i)->get_anim_length());
          if(render_diffuse_map)
          {
#pragma todo( "parametrize frame with respect to material (ifls on diffuse2)? -mkv" )
            profiler_start_timer(profiler::PTM_GENERIC4);
            (*mat_i)->send_context(frame, DIFFUSE, iri.get_force_flags());
            profiler_stop_timer(profiler::PTM_GENERIC4);

            profiler_start_timer(profiler::PTM_GENERIC5);
            vert_workspace.verts += mat_helpers[material_idx].material_offset;

            geometry_manager::inst()->draw_indexed_primitive(PRIMITIVE_TRILIST, vert_workspace, mat_helpers[material_idx].total_verts, &my_start_index[0], mat_helpers[material_idx].total_verts, hw_rasta::SEND_VERT_FRONT_CLIP );

            vert_workspace.verts -= mat_helpers[material_idx].material_offset;
            profiler_stop_timer(profiler::PTM_GENERIC5);
          }

          if(render_environment_map)
          {
            profiler_start_timer(profiler::PTM_GENERIC4);
            (*mat_i)->send_context(frame, ENVIRONMENT, iri.get_force_flags());
            profiler_stop_timer(profiler::PTM_GENERIC4);

            profiler_start_timer(profiler::PTM_GENERIC5);
            vert_workspace.verts += mat_helpers[material_idx].material_offset;

            geometry_manager::inst()->draw_indexed_primitive(PRIMITIVE_TRILIST, vert_workspace, mat_helpers[material_idx].total_verts, &my_start_index[0], mat_helpers[material_idx].total_verts, hw_rasta::SEND_VERT_FRONT_CLIP );

            vert_workspace.verts -= mat_helpers[material_idx].material_offset;
            profiler_stop_timer(profiler::PTM_GENERIC5);
          }
        }
      }
      else
      {
        error("WTF. mesh grids don't allow for no needs_front_clip?????");
      }
    }

    profiler_stop_timer( profiler::PTM_RGN_DRAW );
  }
}




int mesh_grid::sort_face(vr_pmesh *mesh, wedge_ref *wedge_refs)
{
  assert(mesh);
  assert(wedge_refs);

  vector3d pts[3];
  pts[0] = mesh->get_xvert_unxform(wedge_refs[0]);
  pts[1] = mesh->get_xvert_unxform(wedge_refs[1]);
  pts[2] = mesh->get_xvert_unxform(wedge_refs[2]);

  rational_t min_x=pts[2].x;
  rational_t max_x=min_x;
  rational_t min_z=pts[2].z;
  rational_t max_z=min_z;

  for(int i=2; --i>=0; )
  {
    if(pts[i].x > max_x)
      max_x = pts[i].x;
    if(pts[i].x < min_x)
      min_x = pts[i].x;

    if(pts[i].z > max_z)
      max_z = pts[i].z;
    if(pts[i].z < min_z)
      min_z = pts[i].z;
  }

  int x = (int)(((min_x - grid_origin.x) / grid_x_size) - 0.001f);
  int z = (int)(((min_z - grid_origin.z) / grid_z_size) - 0.001f);

  assert_bucket(x, z);

  return mesh_bucket_index(x, z);
}

void mesh_grid::compute_grid_extents()
{
  for(int i = 0; i<MESH_NUM_GRIDS; ++i)
  {
    if(mesh_buckets[i] != NULL)
    {
      int num_verts = mesh_buckets[i]->get_num_wedges();
      int first_pass = 1;

      vector3d ptn = mesh_buckets[i]->get_xvert_unxform(num_verts-1);

      rational_t min_x=ptn.x;
      rational_t max_x=min_x;
      rational_t min_z=ptn.z;
      rational_t max_z=min_z;

      for(int j = num_verts-1; --j>=0; )
      {
        vector3d pt = mesh_buckets[i]->get_xvert_unxform(j);

        if(pt.x > max_x)
          max_x = pt.x;
        if(pt.x < min_x)
          min_x = pt.x;

        if(pt.z > max_z)
          max_z = pt.z;
        if(pt.z < min_z)
          min_z = pt.z;
      }

      mesh_bucket_spheres[i].x = min_x + ((max_x - min_x) * 0.5F);
      mesh_bucket_spheres[i].z = min_z + ((max_z - min_z) * 0.5F);

      float a = (max_x - mesh_bucket_spheres[i].x);
      float b = (max_z - mesh_bucket_spheres[i].z);

      mesh_bucket_spheres[i].rad =  __fsqrt(a*a + b*b);
    }
  }
}



void mesh_grid::clip_xform_and_light(hw_rasta_vert* my_vertsX,
                                     int num_verts,
                                     int workspace_offset,
                                     unsigned lighting_flags,
                                     unsigned cxl_flags,
                                     int ndirlights,
                                     int npointlights,
                                     int alpha,
                                     const po& clip_world2view,
                                     const po& clip_local2view)
{
  hw_rasta_vert* vert_list_begin = &my_vertsX[0];
  hw_rasta_vert* vert_list_end = vert_list_begin + num_verts;

  color32 ambient_factor32 = lites.ambient_factor.to_color32();

  enum { STRIDE=384 };

#if defined(TARGET_MKS)

  // one last attempt at tiling, and hey, this time it worked, in l1c_keep, saving us maybe 1.5f msec
  hw_rasta_vert* tile_begin, *tile_dest, *tile_end;

  profiler_start_timer( profiler::PTM_ENTITY_XFORM );

  //assert(((vert_workspace.begin()+workspace_offset)+grid_mesh->xverts_for_lod[0]) < vert_workspace.end());

  for(  tile_begin = vert_list_begin,
        tile_dest = (vert_workspace.begin()+workspace_offset);

        tile_begin < vert_list_end;

        tile_begin = tile_end
        )
  {
    tile_end = tile_begin + STRIDE;

    if( tile_end > vert_list_end )
      tile_end = vert_list_end;

    // Avoiding MKS l_divs...
    // int tile_count = tile_end - tile_begin
    assert (sizeof(*tile_end)==32);
    int tile_count = ( ((char *) tile_end) - ((char *) tile_begin) ) >> 5;

    if((ndirlights==1)&&(npointlights==0)&&(cxl_flags&CXL_DIFFUSELIGHT))
    {
      asm_onelight(NULL,
                  tile_begin,
                  tile_count,
                  light_matrices,
                  ambient_factor32,
                  lites.light_props->diffuse_color.r,
                  lites.light_props->diffuse_color.g,
                  lites.light_props->diffuse_color.b );
    }
    else
    {
      c_sweetlight(// tile_normal_begin,
                  tile_begin,
                  tile_count,
                  bone_lights,
                  lites.light_props,
                  ndirlights,
                  npointlights,
                  lighting_flags,
                  &(*lites.light_dists.begin()),
                  (vert_workspace.begin()+workspace_offset),
                  alpha,
                  1
                  );
    }

    if((!(cxl_flags&CXL_DIFFUSELIGHT)))
    {
      if( alpha < 255 )
        asm_xform_uni_alpha( tile_begin, tile_count, &total_xform, tile_dest, alpha );
      else
        asm_xform_vertex_alpha( tile_begin, tile_count, &total_xform, tile_dest );
    }
    else
      asm_xform_offset_into_diffuse( tile_begin, tile_count, &total_xform, tile_dest, alpha );

    tile_dest += STRIDE;
  }

  profiler_stop_timer( profiler::PTM_ENTITY_XFORM );

#else
// All on PC side.

  profiler_start_timer( profiler::PTM_ENTITY_CLIP );

  if( cxl_flags&CXL_NOCLIP )
  {
    hw_rasta_vert* xvertptr;
    for (xvertptr = vert_list_begin; xvertptr!=vert_list_end; ++xvertptr )
    {
      xvertptr->clip_flags &= NORMAL_MASK;
    }
  }
  else
  {
    int vert_list_count = vert_list_end-vert_list_begin;
    //assert (sizeof(*vert_list_end)==32);
    //int vert_list_count = ( ((char *) vert_list_end) - ((char *) vert_list_end) ) >> 5;
//    grid_mesh->clip_verts( clip_world2view, clip_local2view, vert_list_begin, vert_list_count );
  }

  profiler_stop_timer( profiler::PTM_ENTITY_CLIP );

  if( !os_developer_options::inst()->is_flagged( os_developer_options::FLAG_D3D_GEOMETRY ) )
  {
    hw_rasta_vert* tvlit;
    hw_rasta_vert* dest;

    for(tvlit=vert_list_begin, dest=(vert_workspace.begin()+workspace_offset);tvlit!=vert_list_end;tvlit++,dest++)
    {
      vector3d xyz,xyz2;
      rational_t rhw;

      // xyz2 is screen coords/z/rhw.
      xyz = tvlit->xyz;

      rational_t w = total_xform[0][3]*xyz.x+total_xform[1][3]*xyz.y+total_xform[2][3]*xyz.z+total_xform[3][3];

      //if (w==0.0f)
      //  w=1.0f;

    #if defined(__X86_CPU)
      __asm
      {
        fld1;
        fdiv  w;
        mov   edi,tvlit; // use the time during fdiv to prefetch the next vert using the integer unit
        add   edi,32;
        mov   eax,[edi];
        fstp  rhw;
      }
    #else
      rhw = 1/w;
    #endif

      xyz2.x  = total_xform[0][0]*xyz.x+total_xform[1][0]*xyz.y+total_xform[2][0]*xyz.z+total_xform[3][0];
      xyz2.y  = total_xform[0][1]*xyz.x+total_xform[1][1]*xyz.y+total_xform[2][1]*xyz.z+total_xform[3][1];
      xyz2.z  = total_xform[0][2]*xyz.x+total_xform[1][2]*xyz.y+total_xform[2][2]*xyz.z+total_xform[3][2];

      *dest = *tvlit;
      dest->set_xyz_rhw(vector3d(xyz2.x*rhw,xyz2.y*rhw,xyz2.z*rhw),rhw);
    }

    int vert_list_count = vert_list_end-vert_list_begin;
    //assert (sizeof(*vert_list_end)==32);
    //int vert_list_count = ( ((char *) vert_list_end) - ((char *) vert_list_end) ) >> 5;
    if(os_developer_options::inst()->is_flagged( os_developer_options::FLAG_LIGHTING ) )
    {
      c_sweetlight(// normal_list_begin,
              vert_list_begin,
              vert_list_count,
              bone_lights,
              lites.light_props,
              ndirlights,
              npointlights,
              lighting_flags,
              lites.light_dists.begin(),
              (vert_workspace.begin()+workspace_offset),
              alpha,
              1
              );
    }
    // wow, you really don't give a shit about windows framerate, do you?
    hw_rasta_vert* vert_workspace_end = (vert_workspace.begin()+workspace_offset)+(vert_list_count);
    if( alpha < 255 )
    {
      for(tvlit=(vert_workspace.begin()+workspace_offset);tvlit!=vert_workspace_end;++tvlit)
      {
        tvlit->diffuse.c.a = alpha;
      }
    }
  }
#endif


}

