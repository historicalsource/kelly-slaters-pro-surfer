////////////////////////////////////////////////////////////////////////////////

// pmesh.cpp
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.

// possibly the slowest renderer ever built, brought to you courtesy of
// James Fristrom

////////////////////////////////////////////////////////////////////////////////

#include "global.h"

#include "algebra.h"
#include "po.h"
#include "app.h"
#include "bound.h"
#include "clipflags.h"
#include "color.h"
#include "game.h"
#include "geomgr.h"
#include "hwmath.h"
#include "hwrasterize.h"
#include "iri.h"
#include "light.h"
#include "lightmgr.h"
#include "maxiri.h"
#include "osalloc.h"
#include "osdevopts.h"
#include "oserrmsg.h"
#include "pmesh.h"
#include "profiler.h"
#include "project.h"
#include "renderflav.h"
#include "terrain.h"
#include "vertnorm.h"
#include "vertwork.h"
#include "vsplit.h"
#include "wds.h"
#include "widget.h"
#include <algorithm>

extern int global_frame_counter;

// evil
static int how_many_verts;

unsigned short* c_skin_tri_setup_nonprog( face* face_start, int face_count, hw_rasta_vert_lit* vert_begin, unsigned short* index_pool )
{
  face* my_face;
  face* face_end = face_start + face_count;
  unsigned short* index_ptr = index_pool;
  for( my_face = face_start; my_face != face_end; ++my_face )
  {
    int wedge0, wedge1, wedge2;
    wedge0 = my_face->wedge_refs[0];
    wedge1 = my_face->wedge_refs[1];
    wedge2 = my_face->wedge_refs[2];

    *index_ptr++ = wedge0;
    *index_ptr++ = wedge1;
    *index_ptr++ = wedge2;
  }
  // tally these anyway to not fool people in to thinking there's fewer tri's out there
  // than there are.
  return index_ptr;
}

unsigned short* c_skin_tri_setup_prog( vr_pmesh* me, int start, int end, hw_rasta_vert_lit* vert_begin, unsigned short* index_ptr, int level_of_detail )
{
  face* my_face;
  face* face_start = me->faces+start;
  face* face_end = me->faces+end;
  wedge* wedges = me->wedges;

  for ( my_face = face_start;
        my_face != face_end;
        ++my_face )
  {
    if( my_face->level_of_detail <= level_of_detail )
    {
      wedge_ref wr0, wr1, wr2;
      wedge* my_wedge;

      wr0 = my_face->wedge_refs[0];
      my_wedge = wedges + wr0;
      while( my_wedge->level_of_detail > level_of_detail )
      {
        wr0 = my_wedge->lower_detail;
        my_wedge = wedges + wr0;
      }
      assert( wr0 < how_many_verts );
//      hw_rasta_vert_lit* v0 = vert_begin+wr0; // unused -- remove me?
    #if defined(TARGET_MKS)
      if( v0->xyz.z < 0.01f )
        goto front_clipped;
    #endif
      wr1 = my_face->wedge_refs[1];
      my_wedge = wedges + wr1;
      while( my_wedge->level_of_detail > level_of_detail )
      {
        wr1 = my_wedge->lower_detail;
        my_wedge = wedges + wr1;
      }
      assert( wr1 < how_many_verts );
    #if defined(TARGET_MKS)
      if( v1->xyz.z < 0.01f )
        goto front_clipped;
    #endif
      wr2 = my_face->wedge_refs[2];
      my_wedge = wedges + wr2;
      while( my_wedge->level_of_detail > level_of_detail )
      {
        wr2 = my_wedge->lower_detail;
        my_wedge = wedges + wr2;
      }
      assert( wr2 < how_many_verts );

      *index_ptr++ = wr0;
      *index_ptr++ = wr1;
      *index_ptr++ = wr2;
    #if defined(TARGET_MKS)
      }
    #endif
    }
    else
      break;    // we've sorted faces by detail so we know we're done:  but it didn't help
#if defined(TARGET_MKS)
front_clipped:;
#endif
  }

  return index_ptr;
}


////////////////////////////////////////////////////////////////////////////////
// Assembly function prototypes for pmeshasm.src

#if defined(TARGET_MKS)
extern "C" {
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

////////////////////////////////////////////////////////////////////////////////
//  optimization tool, because algebra.h ain't inlining
////////////////////////////////////////////////////////////////////////////////
// well this isn't exactly optimal either, especially on the Dreamcast, because
// it takes absolutely no advantage of the SH4 ftrv instruction! --Sean
////////////////////////////////////////////////////////////////////////////////
// the Dreamcast does its transforming in assembly, using ftrv.  This is an
// optimization tool for certain exceptions.  We do what we can.  -Jamie

#define RHW_XFORM( sv, mat, dv, rhw )  \
  rational_t w = mat[0][3]*sv.x+mat[1][3]*sv.y+mat[2][3]*sv.z+mat[3][3];  \
  rhw = 1/w; \
  dv.x  = mat[0][0]*sv.x+mat[1][0]*sv.y+mat[2][0]*sv.z+mat[3][0];  \
  dv.y  = mat[0][1]*sv.x+mat[1][1]*sv.y+mat[2][1]*sv.z+mat[3][1];  \
  dv.z  = mat[0][2]*sv.x+mat[1][2]*sv.y+mat[2][2]*sv.z+mat[3][2];

#define XFORM( sv, mat, dv ) \
  dv.x  = mat[0][0]*sv.x+mat[1][0]*sv.y+mat[2][0]*sv.z+mat[3][0];  \
  dv.y  = mat[0][1]*sv.x+mat[1][1]*sv.y+mat[2][1]*sv.z+mat[3][1];  \
  dv.z  = mat[0][2]*sv.x+mat[1][2]*sv.y+mat[2][2]*sv.z+mat[3][2];

#define XFORM_VERT( sv, mat, dv ) \
  dv.x  = mat[0][0]*sv.xyz.x+mat[1][0]*sv.xyz.y+mat[2][0]*sv.xyz.z+mat[3][0];  \
  dv.y  = mat[0][1]*sv.xyz.x+mat[1][1]*sv.xyz.y+mat[2][1]*sv.xyz.z+mat[3][1];  \
  dv.z  = mat[0][2]*sv.xyz.x+mat[1][2]*sv.xyz.y+mat[2][2]*sv.xyz.z+mat[3][2];

#define SO3_XFORM( sv, mat, dv ) \
  dv.x  = mat[0][0]*sv.x+mat[1][0]*sv.y+mat[2][0]*sv.z;  \
  dv.y  = mat[0][1]*sv.x+mat[1][1]*sv.y+mat[2][1]*sv.z;  \
  dv.z  = mat[0][2]*sv.x+mat[1][2]*sv.y+mat[2][2]*sv.z;



////////////////////////////////////////////////////////////////////////////////
// globals
////////////////////////////////////////////////////////////////////////////////
instance_bank<vr_pmesh> vr_pmesh_bank;


////////////////////////////////////////////////////////////////////////////////
// statics
////////////////////////////////////////////////////////////////////////////////
geometry_manager* vr_pmesh::l_geometry_pipe = NULL;

#if defined(TARGET_XBOX)
vert_lit_buf vert_workspace;
vert_lit_buf vert_workspace_small;
vert_lit_buf vert_workspace_quad;
vert_buf_xformed vert_workspace_xformed;
vert_buf_xformed vert_workspace_xformed_small;
vert_buf_xformed vert_workspace_xformed_quad;

#else

vert_buf vert_workspace;
#if defined (TARGET_PC)
vert_buf vert_workspace_small;
vert_buf vert_workspace_quad;
vert_buf_xformed vert_workspace_xformed;
vert_buf_xformed vert_workspace_xformed_small;
vert_buf_xformed vert_workspace_xformed_quad;
#elif defined (TARGET_PS2)
vert_buf vert_workspace_quad;
vert_buf_xformed vert_workspace_xformed;
vert_buf_xformed vert_workspace_xformed_quad;
#endif

#endif /* XBOX_USE_PMESH_STUFF JIV DEBUG */

unsigned short vert_utility_buffer[MAX_VERTS_PER_PRIMITIVE];

pmesh_normal* normal_pool;  // dynamic so we can get 32-byte alignment



////////////////////////////////////////////////////////////////////////////////

void xform_and_light_skin( hw_rasta_vert* vert_list_begin,
                           hw_rasta_vert* vert_list_end,  // must be a pointer into vert_workspace
                           bool light,
                           matrix4x4* bones_proj,   // xforms for projection
                           const matrix4x4* bones_world,
                           short* wedge_lod_starts,
                           int level_of_detail,
                           use_light_context *lites
                            ); // xforms for lights

#ifndef TARGET_PC
// on PC we're letting D3D do the xforms
static matrix4x4 total_xform; // ugly nasty global
#endif


////////////////////////////////////////////////////////////////////////////////
//  static lighting info
////////////////////////////////////////////////////////////////////////////////


vector3d bone_lights[MAX_BONES][ABSOLUTE_MAX_LIGHTS];

// permanent
matrix4x4* bones_proj;

////////////////////////////////////////////////////////////////////////////////
void vr_pmesh::init_pmesh_system( void )
{
}

////////////////////////////////////////////////////////////////////////////////
void vr_pmesh::kill_pmesh_system( void )
{
#if defined (TARGET_PC) || XBOX_USE_PMESH_STUFF
  vert_workspace.deconstruct();
  vert_workspace_small.deconstruct();
  vert_workspace_quad.deconstruct();
  vert_workspace_xformed.deconstruct();
  vert_workspace_xformed_small.deconstruct();
  vert_workspace_xformed_quad.deconstruct();
  vert_workspace_quad.deconstruct();
  vert_workspace_xformed.deconstruct();
  vert_workspace_xformed_quad.deconstruct();
#endif
}

////////////////////////////////////////////////////////////////////////////////
render_flavor_t vr_pmesh::render_passes_needed() const
{
  render_flavor_t passes=0;
  if (has_translucent_verts)
    return RENDER_TRANSLUCENT_PORTION; // do it all in translucent pass
    //passes |= RENDER_TRANSLUCENT_PORTION;
  for (int i=get_num_materials(); --i>=0; )
  {
    const material* mptr = get_material_ptr(i);
    if ( mptr->is_translucent() )
      passes |= RENDER_TRANSLUCENT_PORTION;
    else
      passes |= RENDER_OPAQUE_PORTION;
    if(mptr->has_environment_map())
      passes |= RENDER_TRANSLUCENT_PORTION;

    for(int x=1; x<MAPS_PER_MATERIAL; ++x)
    {
      if(mptr->has_diffuse_map(x))
        passes |= RENDER_TRANSLUCENT_PORTION;
    }
  }
  return passes;
}


////////////////////////////////////////////////////////////////////////////////
// vr_pmesh render function
////////////////////////////////////////////////////////////////////////////////


bool g_i_am_an_actor = false;
bool g_environment_maps_enabled = true;
bool g_decal_maps_enabled = true;
bool g_detail_maps_enabled = true;

material* g_material_debug_mask = NULL;

int g_start_render = 1;
int g_end_render   = INT_MAX;
int g_num_rendered = 0;
int skip_pass = 0;

void vr_pmesh::render_instance( render_flavor_t render_flavor, instance_render_info* iri, short *ifl_lookup )
{
#if defined(TARGET_PC) || XBOX_USE_PMESH_STUFF
  g_num_rendered++;
  if( g_num_rendered < g_start_render )
    return;
  if( g_num_rendered > g_end_render )
    return;
  if( render_flavor & skip_pass )
    return;

  color32 color_scale32 = iri->get_color_scale();

  vector<material*>* r_materials;

  if( iri->alt_materials ) r_materials = iri->alt_materials;
  else r_materials = &materials;

  // exit early if we're forcing everything translucent and this isn't the translucent pass
  bool pass_xluc = (render_flavor & RENDER_TRANSLUCENT_PORTION)!=0;
  if(iri->force_translucent() && !pass_xluc )
  {
    return;
  }

  if ( r_materials->empty() )
  {
  #ifndef BUILD_BOOTABLE
    if ( !material_changes.empty() && (*material_changes.begin()).first==INVISIBLE_MATERIAL_REF )
      return;
    if (!os_developer_options::inst()->get_no_warnings())
    {
      stringx composite = error_context::inst()->get_context() + stringx("No materials on mesh in file ") + filename;
      warning( composite.c_str() );
    }
  #endif
    return;
  }

  START_PROF_TIMER( proftimer_render_instance );

  geometry_manager::inst()->set_local_to_world(iri->get_local_to_world().get_matrix());

  if (optimized_verts)
  {
    // render the polys in each material.
    material_map::iterator mi;
    for (mi=material_changes.begin(); mi!=material_changes.end(); ++mi)
    {
      int material_idx = (*mi).first;
      if(material_idx != INVISIBLE_MATERIAL_REF)
      {
        material* mat=(*r_materials)[material_idx];
        if (!mat)
          continue;
        if ( g_material_debug_mask )
          if ( mat != g_material_debug_mask )
            continue;

        bool render_diffuse_map;
        bool render_environment_map;
        bool render_decal_map = false;
        bool render_detail_map = false;
        bool is_xluc = color_scale32.c.a<255 || iri->force_translucent() ||
                       mat->is_translucent() || has_translucent_verts;// || mat->has_diffuse_map(1);
        render_diffuse_map = is_xluc == pass_xluc;
        render_environment_map = pass_xluc && g_environment_maps_enabled && mat->has_environment_map();
        render_decal_map = pass_xluc && g_decal_maps_enabled && mat->has_diffuse_map(MAP_DIFFUSE2);
        render_detail_map = pass_xluc && g_detail_maps_enabled && mat->has_diffuse_map(MAP_DETAIL);

        if(render_diffuse_map || render_environment_map || render_decal_map || render_detail_map)
        {
          // iterate through faces until reaching end of current material
          int start = (*mi).second;
          int end;
          material_map::iterator next = mi;
          ++next;
          if ( next == material_changes.end() )
            end = get_max_faces();
          else
            end = (*next).second;

          unsigned short* index_start = wedge_index_list + start*3;
          int num_indices = (end - start)*3;
          if(num_indices)
          {
            int frame = 0;
            int length = 0;

            unsigned override_color = (color_scale32.to_ulong() != 0xffffffff)?FORCE_COLOR_PARAMETER:0;
            if( render_diffuse_map )
            {
              length = mat->get_anim_length( MAP_DIFFUSE );
              frame = iri->time_to_frame_locked( length );
              // if I'm opaque and I don't have translucent verts and I'm in the translucent pass
              // I need to override translucentness so I can zwrite
              bool need_to_zwrite = false;
              if( (!has_translucent_verts) && (!mat->diffuse_map[0].is_translucent()) && (pass_xluc) )
                need_to_zwrite = true;
              if( need_to_zwrite )
                hw_rasta::inst()->send_start( hw_rasta::PT_OPAQUE_POLYS );
              START_PROF_TIMER( proftimer_instance_sendctx );
              mat->send_context( frame, MAP_DIFFUSE, iri->get_force_flags()|override_color, color_scale32 );
              STOP_PROF_TIMER( proftimer_instance_sendctx );
              START_PROF_TIMER( proftimer_instance_draw );
              hw_rasta::inst()->send_indexed_vertex_list(*optimized_verts, optimized_verts->get_max_size(),
                                                         index_start, num_indices);
              STOP_PROF_TIMER( proftimer_instance_draw );
              if( need_to_zwrite )
                hw_rasta::inst()->send_start( hw_rasta::PT_TRANS_POLYS );
            }

            if( render_detail_map || render_environment_map || render_decal_map)
            {
              // copy optimized verts into workspace so we can mess with it
              optimized_verts->lock(-1);
              int n = optimized_verts->get_max_size();

              vert_workspace.lock( n );
              hw_rasta_vert_lit* dest = vert_workspace.begin();
              for( hw_rasta_vert_lit* src = optimized_verts->begin(); --n>=0; ++src,++dest )
              {
                *dest = *src;
              }
              vert_workspace.unlock();

              optimized_verts->unlock();

              if( render_detail_map )
              {
                length = mat->get_anim_length( MAP_DETAIL );
                frame = iri->time_to_frame_locked( length );
                prepare_for_extra_diffuse_pass(mat, MAP_DETAIL);  // this prepares in vert_workspace

                START_PROF_TIMER( proftimer_instance_sendctx );
                mat->send_context( frame, MAP_DETAIL, iri->get_force_flags()|override_color, color_scale32 );
                STOP_PROF_TIMER( proftimer_instance_sendctx );
                START_PROF_TIMER( proftimer_instance_draw );
                hw_rasta::inst()->send_indexed_vertex_list(vert_workspace, optimized_verts->get_max_size(),
                                                           index_start, num_indices);
                STOP_PROF_TIMER( proftimer_instance_draw );
              }

              if( render_environment_map )
              {
                length = mat->get_anim_length( MAP_ENVIRONMENT );
                frame = iri->time_to_frame_locked( length );
                prepare_for_environment_pass(mat);  // this prepares in vert_workspace

                START_PROF_TIMER( proftimer_instance_sendctx );
                mat->send_context( frame, MAP_ENVIRONMENT, iri->get_force_flags()|override_color, color_scale32 );
                STOP_PROF_TIMER( proftimer_instance_sendctx );
                START_PROF_TIMER( proftimer_instance_draw );
                hw_rasta::inst()->send_indexed_vertex_list(vert_workspace, optimized_verts->get_max_size(),
                                                           index_start, num_indices);
                STOP_PROF_TIMER( proftimer_instance_draw );
              }

              if( render_decal_map )
              {
                length = mat->get_anim_length( MAP_DIFFUSE2 );
                frame = iri->time_to_frame_locked( length );
                prepare_for_extra_diffuse_pass(mat, MAP_DIFFUSE2);  // this prepares in vert_workspace

                START_PROF_TIMER( proftimer_instance_sendctx );
                mat->send_context( frame, MAP_DIFFUSE2, iri->get_force_flags()|override_color, color_scale32 );
                STOP_PROF_TIMER( proftimer_instance_sendctx );

                START_PROF_TIMER( proftimer_instance_draw );
                hw_rasta::inst()->send_indexed_vertex_list(vert_workspace, optimized_verts->get_max_size(),
                                                           index_start, num_indices);
                STOP_PROF_TIMER( proftimer_instance_draw );
              }
            }

          }
        }
      }
    }
  }
  else
  {
    START_PROF_TIMER( proftimer_instance_light_setup );

    // used to transform the lights into the object's local space.
    po world2local = iri->get_local_to_world().inverse();

    light_manager* light_set=NULL;
    unsigned cxl_flags = 0;
    cxl_flags |= CXL_NOCLIP;  // entities don't benefit from side-culling.  I guess we don't clip anything now!

    bool no_lighting;

    if (force_light())
      no_lighting = false;
    else
    {
      no_lighting = true;
      if (!(render_flavor&RENDER_NO_LIGHTS) && !(iri->get_force_flags()&FORCE_NO_LIGHT))
      {
        for (material_map::const_iterator mmi=material_changes.begin();
             !(mmi==material_changes.end());
             ++mmi)
        {
          material_ref material_idx = (*mmi).first;
          if (material_idx == INVISIBLE_MATERIAL_REF)
            continue;
          material* curmat=(*r_materials)[material_idx];
          if (!curmat)
            continue;
          if (!curmat->is_full_self_illum())
          {
            no_lighting = false;
            break;
          }
        }
      }
    }

    if (!no_lighting)
    {
      light_set = iri->get_light_set();
      if (light_set)
        if (!(render_flavor&RENDER_REAL_POINT_LIGHTS))
          cxl_flags |= CXL_DIFFUSELIGHT;
    }
    if (!light_set)
    {
      light_set = light_manager::get_static_light_set();
    }
    use_light_context lites;
    light_set->prepare_for_rendering(&lites);

    size_t num_dir_lights = lites.dir_lights.size();
    size_t num_point_lights = min( ABSOLUTE_MAX_LIGHTS - num_dir_lights, lites.point_light_dists.size() );
    lites.xform_lights_to_local(world2local, 1, render_flavor);

    STOP_PROF_TIMER( proftimer_instance_light_setup );

    int level_of_detail = calculate_integral_detail_level( iri );

    po clip_view2local;
    po clip_world2local; // not used, but better safe than sorry

  #ifdef DEBUG
    if( os_developer_options::inst()->is_flagged( os_developer_options::FLAG_SHOW_NORMALS ) )
      render_normals( iri );
  #endif

    how_many_verts = xverts_for_lod[level_of_detail];

    hw_rasta_vert* workspace_end = xverts + how_many_verts;

    vert_workspace.lock(how_many_verts);

    START_PROF_TIMER( proftimer_clip_xform_light );

    clip_xform_and_light( xverts,
                       workspace_end,
                       cxl_flags,
                       num_dir_lights,
                       num_point_lights,
                       color_scale32.c.a, // wish we could pass a whole color here!! would avoid work later
                       clip_world2local,
                       clip_view2local,
                       render_flavor & (RENDER_RAW_ICON_THROUGHPUT|RENDER_ENTITY_WIDGET),
                       &lites);

    STOP_PROF_TIMER( proftimer_clip_xform_light );

    vert_workspace.unlock();

    // render the polys in each material.
    material_map::iterator mi;
    for (mi=material_changes.begin(); mi!=material_changes.end(); ++mi)
    {
      int material_idx = (*mi).first;
      if(material_idx != INVISIBLE_MATERIAL_REF)
      {
        const material* curmat=(*r_materials)[material_idx];
        if (!curmat)
          continue;

        bool render_diffuse_map;
        bool render_environment_map;

        bool render_extra_map[MAPS_PER_MATERIAL];
        for(int i=0; i<MAPS_PER_MATERIAL; ++i)
          render_extra_map[i] = false;

        bool is_xluc = color_scale32.c.a<255 || iri->force_translucent() ||
                       curmat->is_translucent() || has_translucent_verts;
        render_diffuse_map = is_xluc == pass_xluc;
        render_environment_map = pass_xluc && g_environment_maps_enabled && curmat->has_environment_map();
        render_extra_map[MAP_DIFFUSE2] = pass_xluc && g_decal_maps_enabled && curmat->has_diffuse_map(MAP_DIFFUSE2);
        render_extra_map[MAP_DETAIL] = pass_xluc && g_detail_maps_enabled && curmat->has_diffuse_map(MAP_DETAIL);


        if(render_diffuse_map || render_environment_map || render_extra_map[MAP_DIFFUSE2] || render_extra_map[MAP_DETAIL])
        {
          render_material_clipped_full_detail( iri, material_idx, mi, color_scale32,
            render_diffuse_map, render_extra_map, render_environment_map );
        }
      }
    }
  }
  STOP_PROF_TIMER( proftimer_render_instance );
#endif // TARGET_PC
}


int vr_pmesh::calculate_integral_detail_level(instance_render_info* iri)
{
  // calculate detail level
  assert( iri->get_target_face_count() <= get_max_faces());
  int temp = max( (int)iri->get_target_face_count(),get_min_faces());
  float target_face_count = temp;
  int level_of_detail = (int)iri->get_target_face_count();
  if(get_max_faces()!=get_min_faces())
  {
    float target_detail = (target_face_count - get_min_faces())/
                          (get_max_faces() - get_min_faces());
    target_detail *= get_max_detail();
    level_of_detail = (int)target_detail;
  }
  else
    level_of_detail = 0;
  return level_of_detail;
}

class face_info
{
public:
  rational_t min_z2;
  rational_t max_z2;
  const face* my_face;

  face_info() {}
  face_info( rational_t _min_z2, rational_t _max_z2, const face* _my_face )
    : min_z2(_min_z2),
      max_z2(_max_z2),
      my_face(_my_face)
  {}

  bool operator<(const face_info& fi) const
  {
    return min_z2 < fi.min_z2;
  }
};

//-----------------------------------------------------------------------------------------

geometry_manager* geometry_manager_inst;

void vr_pmesh::render_material_clipped_full_detail(
      instance_render_info* iri,
      material_ref material_idx,
      material_map::iterator mi,
      color32 color_scale32,
      bool render_diffuse_map,
      bool render_extra_map[MAPS_PER_MATERIAL],
      bool render_environment_map )
{
#if defined(TARGET_PC) || XBOX_USE_PMESH_STUFF
  vector<material*>* r_materials;

  if( iri->alt_materials ) r_materials = iri->alt_materials;
  else r_materials = &materials;

  material* mat = (*r_materials)[material_idx];

  XDEBUG_MSG("vr_pmesh::render_material");

  unsigned short* indices_it = vert_utility_buffer;

  START_PROF_TIMER( proftimer_rgn_trisetup );
  int i;
  int end_face;
  material_map::iterator next = mi;
  ++next;
  if ( next == material_changes.end() )
    end_face = get_max_faces();
  else
    end_face = (*next).second;
  int num_indices;
  wedge_ref* start_index;
  // iterate through faces until reaching end of current material
  if( wedge_index_list == NULL )
  {
    for ( i=(*mi).second; i<end_face; ++i )
    {
      const face* my_face = &faces[i];

      // a backface cull here might be nice, but you can only do it for
      // tris who don't cross the front clip plane
      //clockwise backface exclusion
      int xv0,xv1,xv2;
      xv0 = my_face->wedge_refs[0];
      xv1 = my_face->wedge_refs[1];
      xv2 = my_face->wedge_refs[2];
      *indices_it++ = xv0;
      *indices_it++ = xv1;
      *indices_it++ = xv2;
    }

    // Avoiding MKS l_divs...
    assert (sizeof(*indices_it)==2);
    num_indices = ( ((char *) indices_it) - ((char *) vert_utility_buffer) ) >> 1;
    start_index = vert_utility_buffer;
  }
  else
  {
    int start_face = (*mi).second;
    start_index = wedge_index_list + start_face*3;
    num_indices = (end_face - start_face)*3;
  }
  STOP_PROF_TIMER( proftimer_rgn_trisetup );

  START_PROF_TIMER( proftimer_rgn_draw );
  if(num_indices)
  {
    int frame = 0;

    unsigned override_color = (color_scale32.to_ulong() != 0xffffffff)?FORCE_COLOR_PARAMETER:0;
    if(render_diffuse_map)
    {
      frame = iri->time_to_frame_locked(mat->get_anim_length(MAP_DIFFUSE));
      mat->send_context( frame, MAP_DIFFUSE, iri->get_force_flags()|override_color, color_scale32 );

      hw_rasta::inst()->send_indexed_vertex_list(vert_workspace, get_num_wedges(),
          start_index, num_indices, hw_rasta::SEND_VERT_FRONT_CLIP );

    }

    if( render_extra_map[MAP_DETAIL] )
    {
      frame = iri->time_to_frame_locked(mat->get_anim_length(MAP_DETAIL));
      prepare_for_extra_diffuse_pass(mat, MAP_DETAIL);  // this prepares in vert_workspace

      START_PROF_TIMER( proftimer_instance_sendctx );
      mat->send_context( frame, MAP_DETAIL, iri->get_force_flags()|override_color, color_scale32 );
      STOP_PROF_TIMER( proftimer_instance_sendctx );

      START_PROF_TIMER( proftimer_instance_draw );
      hw_rasta::inst()->send_indexed_vertex_list(vert_workspace, get_num_wedges(),
          start_index, num_indices, hw_rasta::SEND_VERT_FRONT_CLIP );
      STOP_PROF_TIMER( proftimer_instance_draw );
    }

    if(render_environment_map)
    {
      frame = iri->time_to_frame_locked(mat->get_anim_length(MAP_ENVIRONMENT));
      prepare_for_environment_pass(mat);
      mat->send_context( frame, MAP_ENVIRONMENT, iri->get_force_flags()|override_color, color_scale32 );

      hw_rasta::inst()->send_indexed_vertex_list(vert_workspace, get_num_wedges(),
          start_index, num_indices, hw_rasta::SEND_VERT_FRONT_CLIP );
    }

    if( render_extra_map[MAP_DIFFUSE2] )
    {
      frame = iri->time_to_frame_locked(mat->get_anim_length(MAP_DIFFUSE2));
      prepare_for_extra_diffuse_pass(mat, MAP_DIFFUSE2);  // this prepares in vert_workspace

      START_PROF_TIMER( proftimer_instance_sendctx );
      mat->send_context( frame, MAP_DIFFUSE2, iri->get_force_flags()|override_color, color_scale32 );
      STOP_PROF_TIMER( proftimer_instance_sendctx );

      START_PROF_TIMER( proftimer_instance_draw );
      hw_rasta::inst()->send_indexed_vertex_list(vert_workspace, get_num_wedges(),
          start_index, num_indices, hw_rasta::SEND_VERT_FRONT_CLIP );
      STOP_PROF_TIMER( proftimer_instance_draw );
    }

    if( render_environment_map || render_extra_map[MAP_DIFFUSE2] )
      undo_uv_damage();  // possible optimization:  you only need to do this if you're drawing
                         // more materials


    XDEBUG_MSG("vr_pmesh::render_material return");
  }
  STOP_PROF_TIMER( proftimer_rgn_draw );
#endif // TARGET_PC
}


void vr_pmesh::prepare_for_environment_pass(material *mat)
{
#if defined(TARGET_PC) || XBOX_USE_PMESH_STUFF
  vert_workspace.lock(-1);

#pragma todo("Finish work on the environment mapping. It's better, but still needs work, gotta talk to Alex (JDB)")

  matrix4x4 local_to_world = geometry_manager::inst()->xforms[ geometry_manager::XFORM_LOCAL_TO_WORLD ];
  matrix4x4 world_to_screen = geometry_manager::inst()->xforms[ geometry_manager::XFORM_WORLD_TO_VIEW ];

  vector3d cam_pos = app::inst()->get_game()->get_current_view_camera()->get_abs_position();

  int n=get_num_wedges();
  hw_rasta_vert* normalv=xverts;
  for (hw_rasta_vert_lit* destv=(hw_rasta_vert_lit *)vert_workspace.begin();
       --n>=0;
       ++destv, ++normalv)
  {
    // get camera space vertex normal
    vector3d result_norm;
    vector3d result_pos;

    vector3d norm = normalv->get_normal();
    SO3_XFORM( norm, local_to_world, result_norm );
    XFORM_VERT((*destv), local_to_world, result_pos);

    vector3d v2cam_norm = cam_pos - result_pos;
    v2cam_norm.normalize();

    rational_t dot_prod = dot(v2cam_norm, result_norm);
    vector3d reflection = (result_norm * (2.0f * (dot_prod / result_norm.length2()))) - v2cam_norm;

    vector3d result;
    SO3_XFORM( reflection, world_to_screen, result);
    destv->tc[0].x = (result.x * 0.5f) + 0.5f;  // from the DirectX SDK
    destv->tc[0].y = -(result.y * 0.5f) + 0.5f;

    if(dot_prod < 0.0f)
      dot_prod = 0.0f;

    destv->diffuse.c.a = (unsigned char)((mat->get_environment_blend() * 255.0f /* * dot_prod */));
  }

  vert_workspace.unlock();
#endif // TARGET_PC
}

void vr_pmesh::undo_uv_damage()
{
#if defined(TARGET_PC) || XBOX_USE_PMESH_STUFF
  // yuck yuck yuck
  vert_workspace.lock(-1);

  int n=get_num_wedges();
  hw_rasta_vert* srcv=xverts;
  for (hw_rasta_vert_lit* destv=(hw_rasta_vert_lit *)vert_workspace.begin();
       --n>=0;
       ++destv, ++srcv)
  {
    destv->tc[0] = srcv->tc[0];
    destv->diffuse.c.a = srcv->diffuse.c.a;
  }

  vert_workspace.unlock();

#endif // TARGET_PC
}

void vr_pmesh::undo_skin_uv_damage()
{
#if defined(TARGET_PC) || XBOX_USE_PMESH_STUFF
  // yuck yuck yuck
  vert_workspace.lock(-1);

  int n=get_num_wedges();
  hw_rasta_vert* srcv=xverts;
  for (hw_rasta_vert_lit* destv=(hw_rasta_vert_lit *)vert_workspace.begin();
       --n>=0;
       ++destv, ++srcv)
  {
    destv->tc[0] = srcv->tc[0];
    destv->diffuse.c.a = 255;
  }

  vert_workspace.unlock();

#endif // TARGET_PC
}

void vr_pmesh::prepare_for_extra_diffuse_pass(material *mat, int _map)
{
#if defined(TARGET_PC) || XBOX_USE_PMESH_STUFF
/*
//  assert(_map > 0);
#ifdef TARGET_PC
  if( optimized_verts )
  {
    // copy optimized verts into workspace so we can mess with it
    optimized_verts->lock(-1);
    int n = optimized_verts->get_max_size();
    vert_workspace.lock( n );
    hw_rasta_vert_lit* dest = vert_workspace.begin();
    for( hw_rasta_vert_lit* src = optimized_verts->begin(); --n>=0; ++src,++dest )
    {
      *dest = *src;
    }
    optimized_verts->unlock();
  }
#endif
*/
  vert_workspace.lock(-1);

  switch(_map)
  {
    case MAP_DIFFUSE2:
    {
      int n=get_num_wedges();
      unsigned char alpha = (unsigned char)((mat->get_blend_amount(MAP_DIFFUSE2) * 255.0f));

      for (hw_rasta_vert_lit* destv=(hw_rasta_vert_lit *)vert_workspace.begin(); --n>=0; ++destv)
      {
        destv->tc[0] = destv->tc[MAP_DIFFUSE2];
        destv->diffuse.c.a = alpha;
      }
    }
    break;

    case MAP_DETAIL:
    {
      static matrix4x4 the_xform;
      the_xform = geometry_manager::inst()->xforms[geometry_manager::XFORM_LOCAL_TO_WORLD] * geometry_manager::inst()->xforms[geometry_manager::XFORM_WORLD_TO_SCREEN];

      const rational_t max = mat->get_det_range();
      const rational_t min = 0.0f;
      unsigned char a = (unsigned char)(mat->get_blend_amount(MAP_DETAIL) * 255 * mat->get_det_alpha_clamp() );

      rational_t delta = (min - max);

      int n=get_num_wedges();
      for (hw_rasta_vert_lit* destv=(hw_rasta_vert_lit *)vert_workspace.begin(); --n>=0; ++destv)
      {
        destv->tc[0].x *= mat->get_det_u_scale();
        destv->tc[0].y *= mat->get_det_v_scale();

        rational_t z = the_xform[0][2]*destv->xyz.x+the_xform[1][2]*destv->xyz.y+the_xform[2][2]*destv->xyz.z+the_xform[3][2];

        if(z >= max)
          destv->diffuse.c.a = 0;
        else if(z <= min)
          destv->diffuse.c.a = a;
        else
        {
          rational_t factor = 1.0f + ((z - min) / delta);
          factor *= factor;
          destv->diffuse.c.a = (unsigned char)(factor * (rational_t)a);
        }
      }
    }
    break;
  }

#endif // TARGET_PC
  vert_workspace.unlock();
}

//#pragma fixme( "how come I can't un-const bones_world without breaking the renderer?" )
po bones_world[MAX_BONES];

void vr_pmesh::render_skin( render_flavor_t render_flavor,
                         const instance_render_info* iri,
                         const po* _bones_world,
                         int num_bones )
{
#if defined(TARGET_PC) || XBOX_USE_PMESH_STUFF
  memcpy( bones_world, _bones_world, num_bones*sizeof(po) );
  // exit early if we're forcing everything translucent and this isn't the translucent pass
  bool pass_xluc = (render_flavor & RENDER_TRANSLUCENT_PORTION)!=0;
  if (iri->force_translucent() && !pass_xluc)
  {
    assert(false);
    return;
  }

  START_PROF_TIMER( proftimer_render_skin );

  vector<material*>   *r_materials;

  if( iri->alt_materials ) r_materials = iri->alt_materials;
  else r_materials = &materials;

  // transform the directional lights into view space.

  XDEBUG_MSG("pmesh::render");

#ifdef PROFILING_ON
  float detail_diff = get_max_faces() - iri->get_target_face_count();

  if(!(render_flavor & RENDER_TRANSLUCENT_PORTION))
  {
    ADD_PROF_COUNT( profcounter_lod_tri_estimate, detail_diff );
  }
#endif

  // we are letting D3D do the xforms now, but skins are going to be in world space
  // after we are done with them.  If we could get them into camera space it'd save
  // D3D from doing some work.  Just set this to exactly the inverse of the world_to_view
  // matrix and mix the world_to_view matrix in with the bone matrices.
  geometry_manager::inst()->set_local_to_world( identity_matrix );

  // calculate detail level
  assert( iri->get_target_face_count() <= get_max_faces());
  assert( get_min_faces() <= get_max_faces());
  int temp = max( (int)iri->get_target_face_count(),get_min_faces());
  float target_face_count = temp;
  int level_of_detail = get_max_detail();
  float detail_fraction = 0;

  if(get_max_faces()!=get_min_faces())
  {
    float target_detail = (target_face_count - get_min_faces())/
                          (get_max_faces() - get_min_faces());
    target_detail *= get_max_detail();
    level_of_detail = (int)target_detail;
    detail_fraction = target_detail - level_of_detail;
  }
  // else it's a simple mesh, highest detail = 0


  // prepare bones:  a bone starts out going from bone-local to waist-local
  // (in case of the waist, it's identity).
  // but the xform we need is waist-local (unanimated) to waist-local (animated)
  // so we need to start with waist-local to bone-local
  // which we can find using the inverse of the bone's pivot
  int i;
  int bwsize = num_bones;
  for(i=0;i<bwsize;++i)
  {
    po pivot_po = get_bone_pivot(i);
    matrix4x4 pivot_xform = pivot_po.get_matrix();
    matrix4x4 pivot_xform_invert = pivot_xform.inverse();
    bones_proj[i] = pivot_xform_invert * ((matrix4x4 *)bones_world)[i];
    ((po*)&bones_world[i])->invert(); // for lights.  FIXME:  doesn't take into account NEW skin paradigm
    *((po*)&bones_world[i]) = bones_world[i].get_matrix() * pivot_xform;
  }

  bool light=(render_flavor & RENDER_NO_LIGHTS)==0 &&
             (iri->get_force_flags()&FORCE_NO_LIGHT)==0;
  // we need to light vertices for each material independently instead!
  if (light)
    for (int i=get_num_materials(); --i>=0; )
    {
      if ( get_material_ptr( i )->is_full_self_illum() )
      {
        light=false;
        break;
      }
    }
  if (iri->get_force_flags()&FORCE_LIGHT)
    light=true;

  // transform lights to bone space
  const vector3d& center_pos=iri->get_local_to_world().get_position(); // center of object
  light_manager* light_set = iri->get_light_set();
  if (!light || !light_set)
  {
    light_set = light_manager::get_static_light_set();
  }
  // This object is only updated by light_manager::prepare_for_rendering
  use_light_context lites;
  light_set->prepare_for_rendering(&lites);

  lites.transform_lights_to_bone_space(center_pos, (matrix4x4 *)bones_world, num_bones);

  how_many_verts = xverts_for_lod[level_of_detail];
#ifdef _DEBUG
	if( filename == "kravenkraven" ) {
		//DebugBreak( );
	}
#endif

  hw_rasta_vert* workspace_end = xverts + how_many_verts;

  // prepare xverts to see if they're in the model for this level of detail
  // we don't need to mask and or if no wedge_lod_starts,
  // because CLIP_NOTINLOD will never get set.

  vert_workspace.lock(how_many_verts);

  xform_and_light_skin( xverts,
                        workspace_end,
                        light, // do lighting?
                        bones_proj,
                        (matrix4x4 *)bones_world,
                        wedge_lod_starts,
                        level_of_detail,
                        &lites );

  vert_workspace.unlock();

  material_map::iterator mi;

  color32 color_scale32 = iri->get_color_scale();

  for ( mi=material_changes.begin(); mi!=material_changes.end(); ++mi)
  {
    int material_idx = (*mi).first;
    assert(material_idx>=0);
    // in case the material set doesn't match the mesh (can happen with bad variant data)
    if ( material_idx >= (int)r_materials->size() )
      material_idx = 0;

    if (material_idx != INVISIBLE_MATERIAL_REF)
    {
      const material* curmat=(*r_materials)[material_idx];
      if (!curmat)
        continue;

      bool render_diffuse_map;
      bool render_environment_map;
      bool render_decal_map = false;
      bool render_detail_map = false;
      bool is_xluc = color_scale32.c.a<255 || iri->force_translucent() ||
                     curmat->is_translucent() || has_translucent_verts;
      render_diffuse_map = is_xluc == pass_xluc;
      render_environment_map = /*pass_xluc &&*/ g_environment_maps_enabled;
      render_decal_map = /*pass_xluc &&*/ g_decal_maps_enabled && curmat->has_diffuse_map(MAP_DIFFUSE2);
      render_detail_map = /*pass_xluc &&*/  g_detail_maps_enabled && curmat->has_diffuse_map(MAP_DETAIL);
      if(!curmat->has_environment_map())
        render_environment_map = false;

      START_PROF_TIMER( proftimer_skin_trisetup );

      XDEBUG_MSG("vr_pmesh::render_material");

      // iterate through faces until reaching end of current material
      int end;
      material_map::iterator next = mi;
      ++next;
      if ( next == material_changes.end() )
        end = get_max_faces();
      else
        end = (*next).second;
      unsigned short* index_ptr = vert_utility_buffer;

      vert_workspace.lock(-1); // don't clear! (OW! I want to get rid of this lock, it's very painful)
      hw_rasta_vert_lit* vert_workspace_begin = (hw_rasta_vert_lit*)vert_workspace.begin();

      // Special case the non-progressive case
      wedge_ref* index_start = vert_utility_buffer;
      int num_indices;
      if( wedge_index_list )
      {
        index_start = wedge_index_list + (*mi).second*3;
        num_indices = (end - (*mi).second)*3;
      }
      else
      {
	      if (level_of_detail>=get_max_detail() )
	      {
	        index_ptr = c_skin_tri_setup_nonprog( faces+(*mi).second,
	                                    end - (*mi).second,
	                                    vert_workspace_begin,
	                                    index_ptr );
	      }
	      else
	      {
	      #if defined(TARGET_MKS)
	        index_ptr = asm_skin_tri_setup_prog( this,
	                                  (*mi).second,
	                                  end,
	                                  vert_workspace_begin,
	                                  index_ptr,
	                                  level_of_detail );
	      #else
          index_ptr = c_skin_tri_setup_prog( this,
	                                (*mi).second,
	                                end,
	                                vert_workspace_begin,
	                                index_ptr,
	                                level_of_detail );
	      #endif
	      }
	      // Avoiding MKS l_divs...
	      // num_indices = index_ptr - index_start;
	      assert (sizeof(*index_ptr)==2);
	      num_indices = ( ((char *) index_ptr) - ((char *) index_start) ) >> 1;
	    }

      STOP_PROF_TIMER( proftimer_skin_trisetup );

      vert_workspace.unlock();

      START_PROF_TIMER( proftimer_skin_draw );

      if(num_indices)
      {
        int frame = 0;
        if(render_diffuse_map)
        {
          material* mat = (*r_materials)[material_idx];
          unsigned override_color = (color_scale32.to_ulong() != 0xffffffff)?FORCE_COLOR_PARAMETER:0;

          frame = iri->time_to_frame_locked(mat->get_anim_length(MAP_DIFFUSE));
          mat->send_context( frame, MAP_DIFFUSE, iri->get_force_flags()|override_color, color_scale32 );
//          g_game_ptr->get_blank_material()->send_context( frame, MAP_DIFFUSE, iri->get_force_flags()|override_color, color_scale32 );
          hw_rasta::inst()->send_indexed_vertex_list(vert_workspace, get_num_wedges(),
              index_start, num_indices,
              iri->get_skip_clip()?0:hw_rasta::SEND_VERT_FRONT_CLIP );

          if( !pass_xluc )
            hw_rasta::inst()->send_start( hw_rasta::PT_TRANS_POLYS );
          if( render_detail_map || render_environment_map || render_decal_map )
          {
            if( render_detail_map )
            {
              frame = iri->time_to_frame_locked(mat->get_anim_length(MAP_DETAIL));
              prepare_for_extra_diffuse_pass(mat, MAP_DETAIL);  // this prepares in vert_workspace

              START_PROF_TIMER( proftimer_instance_sendctx );
              mat->send_context( frame, MAP_DETAIL, iri->get_force_flags()|override_color, color_scale32 );
              STOP_PROF_TIMER( proftimer_instance_sendctx );

              START_PROF_TIMER( proftimer_instance_draw );
              hw_rasta::inst()->send_indexed_vertex_list(vert_workspace, get_num_wedges(),
                  index_start, num_indices,
                  iri->get_skip_clip()?0:hw_rasta::SEND_VERT_FRONT_CLIP );
              STOP_PROF_TIMER( proftimer_instance_draw );
            }

            if( render_environment_map )
            {
              frame = iri->time_to_frame_locked(mat->get_anim_length(MAP_ENVIRONMENT));
              prepare_for_environment_pass(mat);  // this prepares in vert_workspace

              START_PROF_TIMER( proftimer_instance_sendctx );
              mat->send_context( frame, MAP_ENVIRONMENT, iri->get_force_flags()|override_color, color_scale32 );
              STOP_PROF_TIMER( proftimer_instance_sendctx );

              START_PROF_TIMER( proftimer_instance_draw );
              hw_rasta::inst()->send_indexed_vertex_list(vert_workspace, get_num_wedges(),
                  index_start, num_indices,
                  iri->get_skip_clip()?0:hw_rasta::SEND_VERT_FRONT_CLIP );
              STOP_PROF_TIMER( proftimer_instance_draw );
            }

            if( render_decal_map )
            {
              frame = iri->time_to_frame_locked(mat->get_anim_length(MAP_DIFFUSE2));
              prepare_for_extra_diffuse_pass(mat, MAP_DIFFUSE2);  // this prepares in vert_workspace

              START_PROF_TIMER( proftimer_instance_sendctx );
              mat->send_context( frame, MAP_DIFFUSE2, iri->get_force_flags()|override_color, color_scale32 );
              STOP_PROF_TIMER( proftimer_instance_sendctx );

              START_PROF_TIMER( proftimer_instance_draw );
              hw_rasta::inst()->send_indexed_vertex_list(vert_workspace, get_num_wedges(),
                  index_start, num_indices,
                  iri->get_skip_clip()?0:hw_rasta::SEND_VERT_FRONT_CLIP );
              STOP_PROF_TIMER( proftimer_instance_draw );
            }
          }
          if( !pass_xluc )
            hw_rasta::inst()->send_start( hw_rasta::PT_OPAQUE_POLYS );
        }
      }

      STOP_PROF_TIMER( proftimer_skin_draw );
      XDEBUG_MSG("vr_pmesh::render_material return");
    }
  }

  XDEBUG_MSG("vr_pmesh::render return");
  if( os_developer_options::inst()->is_flagged( os_developer_options::FLAG_SHOW_NORMALS ) )
    render_skin_normals( iri, bones_world );
  STOP_PROF_TIMER( proftimer_render_skin );
#endif // TARGET_PC
}

void vr_pmesh::clip_xform_and_light( hw_rasta_vert* vert_list_begin,
                      hw_rasta_vert* vert_list_end,
                      unsigned cxl_flags,
                      int num_dir_lights,
                      int num_point_lights,
                      int alpha,
                      const po& clip_world2view,
                      const po& clip_local2view,
                      bool icon_render,
                      use_light_context *lites
                      )
{
#if defined(TARGET_PC) || XBOX_USE_PMESH_STUFF
//  needs_front_clip = false;
  unsigned lighting_flags = (cxl_flags&CXL_DIFFUSELIGHT) ? LIGHT_DIFFUSE : LIGHT_ADDITIVE;
  prepare_lighting_matrices( (cxl_flags&CXL_DIFFUSELIGHT) ? lites->ambient_factor : color(1.0f,1.0f,1.0f,1.0f),
                             &lites->dir_light_props[0],
                             num_dir_lights,
                             num_point_lights,
                             lighting_flags,
                             1 );
//  color32 ambient_factor32 = lites->ambient_factor.to_color32(); // unused -- remove me?


  int vert_list_count = vert_list_end - vert_list_begin;
  if(os_developer_options::inst()->is_flagged( os_developer_options::FLAG_LIGHTING ) &&
     (num_dir_lights | num_point_lights) )
  {
    // this extra pass should be ok because (1) not many models go through this path and
    // (2) most of those that do, are small enough that all the verts will fit in the cache
    hw_rasta_vert* tvlit;
    hw_rasta_vert_lit* dest;
    // strip out normal as we copy
    for (tvlit=vert_list_begin, dest=vert_workspace.begin(); tvlit!=vert_list_end; ++tvlit,++dest)
    {
      dest->set_xyz(tvlit->get_unxform());
      dest->tc[0] = tvlit->tc[0];
      dest->tc[1] = tvlit->tc[1];
    }
    // now lighting will copy the diffuse colors (cache should be nice and warm too)
    sweetlight(vert_list_begin,
               vert_list_count,
               bone_lights,
               &lites->dir_light_props[0],
               num_dir_lights,
               num_point_lights,
               lighting_flags,
               &lites->point_light_dists[0],
               vert_workspace.begin(),
               alpha,
               1);
  }
  else
  {
    hw_rasta_vert* tvlit;
    hw_rasta_vert_lit* dest;
    // strip out normal as we copy
/*    if (alpha<255)
      for (tvlit=vert_list_begin, dest=vert_workspace.begin(); tvlit!=vert_list_end; ++tvlit,++dest)
      {
        dest->set_xyz(tvlit->get_unxform());
        dest->tc[0] = tvlit->tc[0];
        dest->tc[1] = tvlit->tc[1];
        dest->diffuse.i = tvlit->diffuse.i;
        dest->diffuse.c.a = (alpha*dest->diffuse.c.a)>>8;
      }
    else*/
      for (tvlit=vert_list_begin, dest=vert_workspace.begin(); tvlit!=vert_list_end; ++tvlit,++dest)
      {
        dest->set_xyz(tvlit->get_unxform());
        dest->tc[0] = tvlit->tc[0];
        dest->tc[1] = tvlit->tc[1];
        dest->diffuse.i = tvlit->diffuse.i;
      }
  }
#endif // TARGET_PC
}


void xform_and_light_skin( hw_rasta_vert* vert_list_begin,
                           hw_rasta_vert* vert_list_end,  // must be a pointer into vert_workspace
                           bool light,
                           matrix4x4* bones_proj,   // xforms for projection
                           const matrix4x4* bone_spaces,  // xforms for lights
                           short* wedge_lod_starts,
                           int level_of_detail,
                           use_light_context *lites)
{
#if defined(TARGET_PC) || XBOX_USE_PMESH_STUFF
  START_PROF_TIMER( proftimer_skin_lite );
  color32 ambient_factor32 = lites->ambient_factor.to_color32();
  size_t num_dir_lights = lites->dir_lights.size();
  size_t num_point_lights = min( ABSOLUTE_MAX_LIGHTS-num_dir_lights,lites->point_light_dists.size() );

//  light = false;  // temp

  if(light)
  {
    prepare_lighting_matrices( lites->ambient_factor,
                               &lites->dir_light_props[0],
                               num_dir_lights,
                               num_point_lights,
                               LIGHT_DIFFUSE,
                               0 );   // <<<< That should be num_bones from higher parameter
  }

  STOP_PROF_TIMER( proftimer_skin_lite );

//  enum { STRIDE=256 };
  enum { STRIDE=512 };

  hw_rasta_vert* tile_begin, *tile_end;
  hw_rasta_vert_lit *tile_dest;
  short* tile_wedge_begin;
  for( tile_begin = vert_list_begin,
      tile_dest = (hw_rasta_vert_lit *)vert_workspace.begin(),
      tile_wedge_begin = wedge_lod_starts;
      tile_begin != vert_list_end;
      tile_begin = tile_end,
      tile_wedge_begin += STRIDE,
      tile_dest += STRIDE)           // if it goes over, we're done anyway
  {
    tile_end = tile_begin + STRIDE;
    if( tile_end > vert_list_end )
      tile_end = vert_list_end;

    int tile_count = tile_end - tile_begin;

    START_PROF_TIMER( proftimer_skin_xform );
    register hw_rasta_vert* tvlit;
    //if( !os_developer_options::inst()->is_flagged( os_developer_options::FLAG_D3D_GEOMETRY ) )
    {
      // xform
      register hw_rasta_vert_lit* dvit = tile_dest;
      for (tvlit=tile_begin; tvlit!=tile_end; ++tvlit,++dvit)
      {
        float rhw;
        dvit->diffuse = 0xffffffff; // <-temp.   = tvlit->diffuse;
        dvit->tc[0] = tvlit->tc[0];
        dvit->tc[1] = tvlit->tc[1];
        vector3d &xyzbone1 = dvit->xyz;
        vector3d &xyz = tvlit->xyz;
        xyzbone1 = vector3d(0,0,0);
#if defined(TARGET_PS2)
        const matrix4x4& bm = bones_proj[tvlit->bone_ids[0] ];
        RHW_XFORM( xyz, bm, xyzbone1, rhw );
        xyzbone1 *= rhw;
#else
        for( int i=0;i<tvlit->num_bones;i++ )
        {
          const matrix4x4& bm = bones_proj[tvlit->bone_ids[i] ];
          vector3d this_xyzbone;
          RHW_XFORM( xyz, bm, this_xyzbone, rhw );
          this_xyzbone *= rhw;
          this_xyzbone *= tvlit->bone_weights[i];
          xyzbone1 += this_xyzbone;
        }
#endif
      }
    }
    STOP_PROF_TIMER( proftimer_skin_xform );

#if defined(TARGET_PC) || XBOX_USE_PMESH_STUFF
    START_PROF_TIMER( proftimer_skin_lite );
    // light first while local coordinates still intact
    if(light && os_developer_options::inst()->is_flagged(os_developer_options::FLAG_LIGHTING))
    {
      if((num_point_lights==0)&&(num_dir_lights==1))
        c_onelight(tile_begin,
                   tile_count,
                   &lites->dir_light_props[0],
                   LIGHT_DIFFUSE,
                   tile_dest,
                   ambient_factor32);
      else
        sweetlight(tile_begin,
                   tile_count,
                   bone_lights,
                   &lites->dir_light_props[0],
                   num_dir_lights,
                   num_point_lights,
                   LIGHT_DIFFUSE,
                   &lites->point_light_dists[0],
                   tile_dest,
                   0,
                   MAX_BONES);
    }
    STOP_PROF_TIMER( proftimer_skin_lite );
#endif // TARGET_PC

  }  // end of tiling loop
#endif // TARGET_PC
}


void vr_pmesh::render_normals( const instance_render_info* iri )
{
}

void vr_pmesh::render_skin_normals( const instance_render_info* iri,
                              const po* bones_world )
{

#if defined(TARGET_PC) || XBOX_USE_PMESH_STUFF
  static unsigned short retarded_index_list[8192];

  // create matrices that will transform a normal into world space, taking into account limb and animation
/*  matrix4x4 model_to_world[ MAX_BONES ];
  int i;
  for( i=0; i<get_num_bones(); i++ )
  {

    po pivot_po = get_bone_pivot(i);
    matrix4x4 pivot_xform = pivot_po.get_matrix();
    pivot_xform = pivot_xform.inverse();
    bones_proj[i] = pivot_xform * ((matrix4x4 *)bones_world)[i];

    po pivot_po = get_bone_pivot(i);
    matrix4x4 bone_to_model = pivot_po.get_matrix();
    matrix4x4 model_to_bone = bone_to_model;
    model_to_bone.inverse();
    model_to_world[i] = model_to_bone * ((matrix4x4*)bones_world)[i];
  }
*/
  const matrix4x4& world_to_screen = geometry_manager::inst()->get_world_to_screen();

  // go through the vert list and for every skin vert
  // render it (using its default bone) and its normal
  hw_rasta_vert*    src = xverts;
  vert_workspace.lock( 8172 );
  hw_rasta_vert_lit* dest = vert_workspace.begin();
  hw_rasta_vert*    src_end = xverts+how_many_verts;

  int i = 0;
  for( ;src != src_end; src++ )
  {
    retarded_index_list[i] = i;  i++;
    retarded_index_list[i] = i;  i++;
    retarded_index_list[i] = i;  i++;
    float rhw;

    // source vert "same" as skin
    dest->diffuse = 0xffffffff;
    dest->tc[0] = dest->tc[1] = vector2d( 0, 0 );
    vector3d xyzbone1 = dest->xyz;
    vector3d &xyz = src->xyz;
    xyzbone1 = vector3d(0,0,0);
    for( int i=0;i<src->num_bones;i++ )
    {
      const matrix4x4& bm = bones_proj[src->bone_ids[i] ];
      vector3d this_xyzbone;
      RHW_XFORM( xyz, bm, this_xyzbone, rhw );
      this_xyzbone *= rhw;
      this_xyzbone *= src->bone_weights[i];
      xyzbone1 += this_xyzbone;
    }
    dest->xyz = xyzbone1;

    // next vert same as last vert, slightly offset
    dest++;
    dest->diffuse = 0xffffffff;
    dest->tc[0] = dest->tc[1] = vector2d( 0.0f, 0.0f );
    dest->xyz = xyzbone1 + vector3d(0.01f, 0.01f, 0.01f);

    // next vert has normal
    dest++;
    dest->diffuse = 0xffffffff;
    dest->tc[0] = dest->tc[1] = vector2d( 0.0f, 0.0f );
    vector3d norm = src->get_normal();
    norm *= 0.1f;
#if 0
    // method 1
    vector3d endnorm = src->xyz + norm;
    RHW_XFORM( endnorm, bones_proj[ src->bone_ids[0] ], xyzbone1, rhw );
    dest->xyz = xyzbone1;
#else
    // method 2
    // transform normal without translation
    vector3d endnorm;
    SO3_XFORM( norm, bones_proj[ src->bone_ids[0] ], endnorm );
    // good puppy
    endnorm += xyzbone1;
    dest->xyz = endnorm;
#endif
    dest++;
  }
  vertex_context vc;
  vc.set_cull_mode( vertex_context::CNONE );
  hw_rasta::inst()->send_context( vc );

  hw_rasta::inst()->send_texture( hw_texture_mgr::inst()->get_white_texture() );
  if(dest != vert_workspace.begin())
  {
    int vert_count = dest - vert_workspace.begin();
    vert_workspace.unlock();
    hw_rasta::inst()->send_indexed_vertex_list(vert_workspace, vert_count,
                                               retarded_index_list, vert_count );
  }
#endif
}


//#pragma fixme( "rewrite anim length system to understand diff't per-layer anim lengths. -mkv 4/6/01" )
// Compute the "animation length" of the materials on a pmesh
// by simply taking the maximum of all animations across all layers
// of all materials
int vr_pmesh::get_anim_length() const
{
  int count = get_num_materials( );
  int length = -1;

  for( int i = 0; i < count; i++ ) {

    for( int j = 0; j < MAPS_PER_MATERIAL; j++ ) {
      int l = materials[i]->get_anim_length( j );
      length = max( length, l );
    }

  }

  return length;
}


void vr_pmesh::clear_lighting()
{
#ifdef TARGET_MKS
  // MKS does its lighting in-place in the original mesh data
  // and so it needs cleared out to draw a non-lit version, otherwise it gets
  // overbrightened by the additional specular.  Can't we turn off the specular
  // render state in Kamui?
  hw_rasta_vert* wp = xverts;
  for(int i=num_wedges; --i>=0; ++wp)
  {
    wp->specular.i &= 0xFF000000;    // black
  }
#endif
}

void vr_pmesh::anim_uvs( time_value_t t )
{

  // don't update more than once per frame. why this is being
  // called multiple times per frame sort of boggles me
  if( uvanim_update_frame == global_frame_counter ) {
    return;
  }

  uvanim_update_frame = global_frame_counter;

  // no point being here if we're not animated (any layer possible)
  if( !is_uv_animated( ) ) {
    return;
  }

  // somehow it boggles me that this could happen--remove?
  if( wedge_index_list == NULL ) {
#if (defined DEBUG) && (defined TARGET_PC)
    OutputDebugString( "skipping animation because of NULL wedge_index_list\n" );
#endif
    return;
  }

  // setup our scratch buffer for marking already advanced texcoords
  int md = get_max_detail( );
  int num_verts = xverts_for_lod[ md ];
  unsigned short* vert_done = vert_utility_buffer;
  memset( vert_done, 0, num_verts * sizeof( short ) );

  // now actually advance the uv values
  material_map::iterator mi;

  for( mi = material_changes.begin( ); mi != material_changes.end( ); mi++ ) {
    int material_idx = (*mi).first;

    // don't animate invisible materials--does this ever happen?
    if( material_idx == INVISIBLE_MATERIAL_REF ) {
#if (defined DEBUG) && (defined TARGET_PC)
      OutputDebugString( "skipping animation of invisible material\n" );
#endif
      continue;
    }

    material* mat = materials[material_idx];

    // this *particular* material might not have any anim
    if( !( mat->u_anim || mat->v_anim ) ) {
      continue;
    }

    material_map::iterator next = mi;
    next++;
    int end = 0;

    if( next == material_changes.end() ) {
      end = get_max_faces( );
    } else {
      end = (*next).second;
    }

    int face_idx = (*mi).second;
    unsigned short* index_start = wedge_index_list + ( face_idx * 3 );
    int num_indices = ( end - face_idx ) * 3;

    if( !num_indices ) {
      continue;
    }

    for( int cn = 0; cn < num_indices; cn++ ) {
      int ix = index_start[cn];

      if( vert_done[ix] ) {
        continue;
      }

      vert_done[ix] = TRUE;

      rational_t& x = xverts[ix].tc[0].x;
      x += ( mat->u_anim * t );
      rational_t& y = xverts[ix].tc[0].y;
      y += ( mat->v_anim * t );

      const rational_t UV_ANIM_CLAMP = 64.0f;
      x = fmodf( x, UV_ANIM_CLAMP );
      y = fmodf( y, UV_ANIM_CLAMP );
    }

  }

}

bool vr_pmesh::is_uv_animated() const
{
  vector<material*>::const_iterator mi;

  for ( mi=materials.begin(); mi!=materials.end(); ++mi )
  {
    if ( (*mi)->u_anim || (*mi)->v_anim )
      return true;
  }

  return false;
}


// we should change this so it uses a 256-entry lookup table instead of calling pow() 3 times per vertex!

void set_global_brightness( rational_t brightness )
{
  const rational_t  k = 1.0f / 255.0f;

  assert( brightness >= -1.0f && brightness <= 1.0f );
  if( brightness >= -.009f && brightness <= .009f ) return;
  if( brightness > .0f ) brightness = 1 - brightness;
  else brightness *= -1000.0f;
  for( instance_bank<vr_pmesh>::pref_set::iterator si = vr_pmesh_bank.refs_by_ptr.begin(), sie = vr_pmesh_bank.refs_by_ptr.end(); si != sie; ++si )
  {
    if( *si && (*si)->ptr )
    {
      hw_rasta_vert *xv = (*si)->ptr->xverts;
      for( int i = 0, j = (*si)->ptr->get_num_wedges(); i < j; ++i, ++xv )
      {
        if( brightness <= 1.0f )
        {
          xv->diffuse.set_red  ( (unsigned char)(255.0f * (rational_t)pow(k * (rational_t)xv->diffuse.get_red  (), brightness)) );
          xv->diffuse.set_green( (unsigned char)(255.0f * (rational_t)pow(k * (rational_t)xv->diffuse.get_green(), brightness)) );
          xv->diffuse.set_blue ( (unsigned char)(255.0f * (rational_t)pow(k * (rational_t)xv->diffuse.get_blue (), brightness)) );
        }
        else
        {
          xv->diffuse.set_red  ( (unsigned char)(255.0f / pow(brightness, 1.0f - k * (rational_t)xv->diffuse.get_red  ())) );
          xv->diffuse.set_green( (unsigned char)(255.0f / pow(brightness, 1.0f - k * (rational_t)xv->diffuse.get_green())) );
          xv->diffuse.set_blue ( (unsigned char)(255.0f / pow(brightness, 1.0f - k * (rational_t)xv->diffuse.get_blue ())) );
        }
      }
    }
  }
}


void undo_global_brightness_set( rational_t previous_brightness_value )
{
  const rational_t k = 1.0f / 255.0f;
  instance_bank<vr_pmesh>::pref_set::iterator si, sie;
  int i, j;

  assert( previous_brightness_value >= -1.0f && previous_brightness_value <= 1.0f );
  if( previous_brightness_value >= -.009f && previous_brightness_value <= .009f ) return;


  if( previous_brightness_value > .0f )
  {
    previous_brightness_value = 1.0f / ( 1.0f - previous_brightness_value );
    for( si = vr_pmesh_bank.refs_by_ptr.begin(), sie = vr_pmesh_bank.refs_by_ptr.end(); si != sie; ++si )
    {
      if( *si && (*si)->ptr )
      {
        hw_rasta_vert *xv = (*si)->ptr->xverts;
        for( i = 0, j = (*si)->ptr->get_num_wedges(); i < j; ++i, ++xv )
        {
          xv->diffuse.set_red  ( (unsigned char)(255.0f * (rational_t)pow(k * (rational_t)xv->diffuse.get_red  (), previous_brightness_value)) );
          xv->diffuse.set_green( (unsigned char)(255.0f * (rational_t)pow(k * (rational_t)xv->diffuse.get_green(), previous_brightness_value)) );
          xv->diffuse.set_blue ( (unsigned char)(255.0f * (rational_t)pow(k * (rational_t)xv->diffuse.get_blue (), previous_brightness_value)) );
        }
      }
    }
  }
  else
  {
    previous_brightness_value = -1000 * previous_brightness_value;
    for( si = vr_pmesh_bank.refs_by_ptr.begin(), sie = vr_pmesh_bank.refs_by_ptr.end(); si != sie; ++si )
    {
      if( *si && (*si)->ptr )
      {
        hw_rasta_vert *xv = (*si)->ptr->xverts;
        for( i = 0, j = (*si)->ptr->get_num_wedges(); i < j; ++i, ++xv )
        {
          xv->diffuse.set_red  ( (unsigned char)(255.0f * (1.0f - log(k * (rational_t)xv->diffuse.get_red  ()) / log(previous_brightness_value))) );
          xv->diffuse.set_green( (unsigned char)(255.0f * (1.0f - log(k * (rational_t)xv->diffuse.get_green()) / log(previous_brightness_value))) );
          xv->diffuse.set_blue ( (unsigned char)(255.0f * (1.0f - log(k * (rational_t)xv->diffuse.get_blue ()) / log(previous_brightness_value))) );
        }
      }
    }
  }
}


inline unsigned char multiply_clip_color( unsigned char col, rational_t factor )
{
  rational_t frv = (rational_t)col * factor;
  if( frv > 255 ) frv = 255;
  return (unsigned char)frv;
}


void set_global_linear_brightness( rational_t brightness )
{
#define BRIGHTNESS_SCALE_FACTOR 2.0f

  assert( brightness >= -1.0f && brightness <= 1.0f );
  if( brightness < .0f ) brightness += 1.0f;
  else if( brightness > .0f ) brightness = BRIGHTNESS_SCALE_FACTOR * ( brightness + 1.0f );
  else return;
  for( instance_bank<vr_pmesh>::pref_set::iterator si = vr_pmesh_bank.refs_by_ptr.begin(), sie = vr_pmesh_bank.refs_by_ptr.end(); si != sie; ++si )
  {
    if( *si && (*si)->ptr )
    {
      hw_rasta_vert *xv = (*si)->ptr->xverts;
      for( int i = 0, j = (*si)->ptr->get_num_wedges(); i < j; ++i, ++xv )
      {
        xv->diffuse.set_red  ( multiply_clip_color(xv->diffuse.get_red  (), brightness) );
        xv->diffuse.set_green( multiply_clip_color(xv->diffuse.get_green(), brightness) );
        xv->diffuse.set_blue ( multiply_clip_color(xv->diffuse.get_blue (), brightness) );
      }
    }
  }
}

#if defined(TARGET_PC) || XBOX_USE_PMESH_STUFF
unsigned vr_pmesh::optdisallow = 0;
#endif

////////////////////////////////////////////////////////////////////////////////
// statics
////////////////////////////////////////////////////////////////////////////////
extern pmesh_normal *normal_pool;

#if defined (TARGET_PC) 
//extern vert_buf non_indexed_workspace;
#endif

//extern instance_render_info* viri;
extern matrix4x4* bones_proj;

const int MAX_N_TANGLE_FACES  = 100;
const int MAX_N_TANGLE_VERTS  = MAX_N_TANGLE_FACES*3;
const int MAX_N_TANGLE_WEDGE_REFS = MAX_N_TANGLE_FACES*3;
int * global_n_tangle_xverts_for_lod;
wedge * global_n_tangle_wedges;
hw_rasta_vert * global_n_tangle_rasta_verts;
vert_ref * global_n_tangle_vert_refs;
face * global_n_tangle_faces;
wedge_ref * global_n_tangle_wedge_refs;
reduced_face * global_n_tangle_reduced_faces;

// Big ol' hack to avoid parameter passing for the moment.
bool making_n_tangle = false;

enum { MAX_ATTENUATION=16384 };

////////////////////////////////////////////////////////////////////////////////
// vr_pmesh
////////////////////////////////////////////////////////////////////////////////

void initialize_mesh_stuff()
{
//  int sub_before_1=0, sub_after_1=0, sub_before_2=0, sub_after_2=0;

#if defined (TARGET_PC) || XBOX_USE_PMESH_STUFF
  normal_pool = (pmesh_normal*)os_malloc( MAX_VERTS_PER_PRIMITIVE * sizeof(pmesh_normal) );
#else
  normal_pool = NULL;
#endif

  bones_proj = (matrix4x4*)os_malloc( MAX_BONES * sizeof(matrix4x4) );


//  int temp1 = MAX_VERTS_PER_PRIMITIVE;
//  int temp2 = VIRTUAL_MAX_VERTS_PER_PRIMITIVE;
  // account for vertex and
//  int offset =  temp1 * ( sizeof( hw_rasta_vert ) + sizeof(vert_normal) )
//            -   temp2 * ( sizeof( hw_rasta_vert ) + sizeof(vert_normal) );
//P  memtrack::set_total_alloced_offset( offset  + sub_after_1-sub_before_1 + sub_after_2-sub_before_2);

  // the "Default" context distinguishes game allocations from overhead;
  // in this, it serves the same purpose as the total_alloced_offset value
//P  g_memory_context.push_context( "Default" );

  global_n_tangle_wedges = (wedge*)os_malloc( MAX_N_TANGLE_VERTS*sizeof(wedge) );
  global_n_tangle_rasta_verts = (hw_rasta_vert*)os_malloc( MAX_N_TANGLE_VERTS*sizeof(hw_rasta_vert) );
  global_n_tangle_vert_refs = (vert_ref*)os_malloc( MAX_N_TANGLE_VERTS*sizeof(vert_ref) );
  global_n_tangle_faces = (face*)os_malloc( MAX_N_TANGLE_FACES*sizeof(face) );
  global_n_tangle_wedge_refs = (wedge_ref*)os_malloc( MAX_N_TANGLE_WEDGE_REFS*sizeof(wedge_ref) );
  global_n_tangle_reduced_faces = (reduced_face*)os_malloc( MAX_N_TANGLE_FACES*sizeof(reduced_face) );
  global_n_tangle_xverts_for_lod = (int*)os_malloc( (MAX_N_TANGLE_VERTS+1)*sizeof(int) );
}

////////////////////////////////////////////////////////////////////////////////
// vr_pmesh
////////////////////////////////////////////////////////////////////////////////
vr_pmesh::vr_pmesh( unsigned _mesh_flags )
         : visual_rep(VISREP_PMESH)
         , min_faces(0)
         , max_detail(0)
{
  has_translucent_verts = false;
  mesh_flags = _mesh_flags | LIT_LAST_FRAME;
  xverts = NULL;
  uvanim_update_frame = 0;
  wedges = NULL;
  vert_refs_for_wedge_ref = NULL;
  wedge_lod_starts = NULL;
  original_face_for_face_slot = NULL;
  faces = NULL;
  reduced_faces = NULL;
  wedge_index_list = NULL;
  num_faces = 0;
  num_wedges = 0;
  pivot = ZEROVEC;
  pivot_valid = false;
  xverts_for_lod = NULL;

  materials.resize(0);
  verts = NEW vector<vert>;
#if defined(TARGET_PC) || XBOX_USE_PMESH_STUFF
  optimized_verts = NULL;
#endif
}


vr_pmesh::vr_pmesh(const char * pmesh_filename, unsigned _mesh_flags ) : visual_rep( VISREP_PMESH )
{
#if defined(TARGET_PC) || XBOX_USE_PMESH_STUFF
  optimized_verts = NULL;
#endif
  _construct(stringx(pmesh_filename), _mesh_flags);
}


vr_pmesh::~vr_pmesh()
{
#if defined(TARGET_PC) || XBOX_USE_PMESH_STUFF
  if (optimized_verts)
    delete optimized_verts;
#endif
  if (reduced_faces && reduced_faces != global_n_tangle_reduced_faces)
    delete[] reduced_faces;
  if( wedge_index_list && wedge_index_list != global_n_tangle_wedge_refs) delete[] wedge_index_list;
//  texture_manager::inst()->delete_texture("sample_shadow");
//  assert( verts==NULL );  // shrink memory footprint should have been called before now
  if( verts ) delete verts;  // one limb body and some terrain portals still have verts by now, but their
  // memory cost is low
  if( xverts_for_lod && xverts_for_lod != global_n_tangle_xverts_for_lod ) delete[] xverts_for_lod;
  if( original_face_for_face_slot ) delete original_face_for_face_slot;
  if( xverts && xverts!=global_n_tangle_rasta_verts ) os_free( xverts );
  if( wedges && wedges!=global_n_tangle_wedges ) os_free( wedges );
  if( faces && faces!=global_n_tangle_faces) os_free( faces );
  if( vert_refs_for_wedge_ref ) os_free( vert_refs_for_wedge_ref );
  if( wedge_lod_starts ) os_free( wedge_lod_starts );

  for (int i=materials.size(); --i>=0; )
  {
    delete materials[i];
  }
  material_changes.clear();
}

///////////////////////////////////////////////////////////////////////////////
// NEWENT File I/O
///////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
int g_debug_thingy;
#endif

vr_pmesh::vr_pmesh( chunk_file& fs, unsigned _mesh_flags )
:   visual_rep( VISREP_PMESH )
{
  has_translucent_verts = false;
  progressive = false;
  mesh_flags = _mesh_flags | LIT_LAST_FRAME;
  verts = NULL;
  xverts = NULL;
  uvanim_update_frame = 0;
  wedges = NULL;
  faces = NULL;
  wedge_index_list = NULL;
  reduced_faces = NULL;
  vert_refs_for_wedge_ref = NULL;
  wedge_lod_starts = NULL;
  original_face_for_face_slot = NULL;
#if defined(TARGET_PC) || XBOX_USE_PMESH_STUFF
  optimized_verts = NULL;
#endif
  filename = fs.get_filename();
#ifdef _DEBUG
  if( filename == stringx("spidermanspidey") )
  {
    g_debug_thingy++;
  }
#endif
  load( fs );
}


void vr_pmesh::load( chunk_file& fs )
{
  pivot = ZEROVEC;
  pivot_valid = false;
  internal_serial_in( fs );
	debug_print( "loaded mesh '%s' has v/w/f/m %d/%d/%d/%d", filename.c_str( ), verts->size( ), num_wedges, num_faces, materials.size( ) );
}

void serial_in( chunk_file& fs, vr_pmesh* mesh )
{
  mesh->internal_serial_in( fs );
}

void vr_pmesh::internal_serial_in( chunk_file& fs )
{
  error_context::inst()->push_context(fs.get_name());
  stringx next_string;
  int vert_count=0;
  int svert_count=0;
  int wedge_count=0;
  int face_count=0;
  int vsplit_count=0;
  int morph_wedge_count=0;
  pivot = ZEROVEC;
  pivot_valid = false;
  stringx texture_dir;
  bool no_warnings = false;
  max_detail = 0;
  min_detail = INT_MAX;

  int last_wedges_lod = 0;

#ifndef TARGET_PS2
  if( os_file::directory_exists( fs.get_dir()+os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) ) ) // does texture directory exist below?
  {
#endif
    // yes, use it
    stringx ostexdir(os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR));
    texture_dir = fs.get_dir()+ostexdir+"\\";
#ifndef TARGET_PS2
  }
  else
  {
    // no, use one back
    stringx dir=fs.get_dir();
    int im_the_last_slash = dir.rfind('\\');
    int second_to_last_slash = dir.rfind('\\',im_the_last_slash-1);
    texture_dir = dir.substr(0,second_to_last_slash+1) + os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR)+stringx("\\");
  }
#endif

  for(;;)
  {
    chunk_flavor flavor_flav;
    bool succeeded = false;
    serial_in( fs, &flavor_flav );

    if ((succeeded = read_stuff1(fs, flavor_flav, no_warnings, vert_count, svert_count, morph_wedge_count))==false)
      succeeded = read_stuff2(fs, flavor_flav, last_wedges_lod, wedge_count, face_count, texture_dir, vsplit_count);

    if( CHUNK_EOF == flavor_flav )
    {
      succeeded = true;
      break;
    }
    if( CHUNK_END == flavor_flav )
    {
      succeeded = true;
      break;
    }
    if( succeeded == false )
    {
      stringx composite = stringx("Unknown chunk type: ")+ flavor_flav.to_stringx() +
                         stringx(" in file ") + fs.get_name();
      error( composite.c_str() );
    }
  }

  if (materials.empty())
  {
    vector<stringx>::const_iterator str;
    vector<stringx> const & stack = error_context::inst()->get_stack();
    for (str = stack.begin(); str!=stack.end(); ++str)
      if (*str=="NOWARN") break;
    if (str==stack.end() && !os_developer_options::inst()->get_no_warnings())
    {
      stringx composite = error_context::inst()->get_context() + stringx(" No materials on mesh in file.");
      warning( composite.c_str() );
    }
  }

  if ( wedge_count == 0 )
    if (!os_developer_options::inst()->get_no_warnings())
      warning( error_context::inst()->get_context() + " invalid mesh (no wedges)" );

  for(int l=last_wedges_lod;l<=max_detail;++l)
  {
    xverts_for_lod[ l ] = wedge_count;
  }

  // minimum faces is the number of polys that will be drawn at detail level 0
  min_faces = 0;
  for(int i=get_max_faces(); --i>=0; )
  {
    if(faces[i].level_of_detail==0)
      ++min_faces;
  }
  optimize();  // deferred until later, because collision geometry needs it unsorted
               // because collision geometry relies on indices of faces being where it expects
  mark_self_lit_verts();
  compute_info();

  // throw away progressive shit if we're not progressive
  if( get_max_faces()==get_min_faces())
  {
	  os_free( wedge_lod_starts );
	  wedge_lod_starts = NULL;
	}

  #if defined(TARGET_PC) && 0
  {
//P    int after = memtrack::get_total_alloced();
    if(after<8774407)
    {
      debug_print(atot((fs.get_name() + ": " + itos(after) + "\n").c_str())  );
    }
  }
  #endif
  error_context::inst()->pop_context();
}



bool vr_pmesh::read_stuff1(chunk_file& fs, chunk_flavor flavor_flav, bool & no_warnings, int & vert_count, int & svert_count, int & morph_wedge_count)
{
  bool succeeded = false;
  if( chunk_flavor("nobones") == flavor_flav )
  {
    succeeded = true;
    int nobones;
    serial_in( fs, &nobones );
    for(int i=0;i<nobones;++i)
    {
      chunk_flavor mo_flavor;
      serial_in( fs, &mo_flavor );
      assert( chunk_flavor("bone")==mo_flavor );
      bone new_bone;
      for(;;)
      {
        serial_in( fs, &mo_flavor );
        if( chunk_flavor("bonename") == mo_flavor )
        {
          serial_in( fs, &new_bone.name );
        }
        else if( chunk_flavor("pivot") == mo_flavor )
        {
          serial_in( fs, &new_bone.pivot );
        }
        else if( CHUNK_END == mo_flavor )
          break;
      }
      bones.push_back( new_bone );
    }
  }
  if( CHUNK_MESH == flavor_flav )
  {
    succeeded = true;
  }
  if( chunk_flavor("meshfile") == flavor_flav )
  {
    succeeded = true;
    // ignore
    stringx meshfilename;
    serial_in( fs, &meshfilename );
  }
  if( CHUNK_NO_WARNINGS == flavor_flav )
    {
    succeeded = true;
    no_warnings = true;
  }
  if( CHUNK_PIVOT == flavor_flav )
  {
    succeeded = true;
    vector3d local_pivot;
    serial_in( fs, &local_pivot );
    pivot = local_pivot;
    pivot_valid = true;
  }
  if( CHUNK_NUM_VERTS == flavor_flav )
  {
    succeeded = true;
    int nverts;
    serial_in( fs, &nverts );
    delete verts;
    verts = NEW vector<vert>(nverts);
    xverts_for_lod = NEW int[nverts+1];
  }
  if( CHUNK_VERT == flavor_flav )
  {
    succeeded = true;
    vert vert_chunk(fs);
    if (is_pivot_valid())
    {
      vert_chunk = vert(vert_chunk.get_point()-pivot);
    }
    (*verts)[vert_count] = vert_chunk;
    ++vert_count;
  }
  if( CHUNK_SKIN == flavor_flav )
  {
    succeeded = true;
    int num_bones;
    serial_in( fs, &num_bones );
    (*verts)[svert_count].num_bones = num_bones;
    for(int i=0;i<num_bones;i++)
    {
      int bone_id;
      float weight;
      serial_in( fs, &bone_id );
      (*verts)[svert_count].bone_ids[i] = bone_id;
      serial_in( fs, &weight );
      (*verts)[svert_count].bone_weights[i] = weight;
    }
    ++svert_count;
  }
  if( CHUNK_WEDGE_REF == flavor_flav )
  {
    assert(false); // removed this --Sean
  }
  if( CHUNK_NUM_WEDGES == flavor_flav )
  {
    succeeded = true;
    serial_in( fs, &num_wedges );
    os_free( wedges );
    wedges = (wedge*)os_malloc( num_wedges*sizeof(wedge) );  //  hope we get wedge to multiple of 32...
    os_free( xverts );
    xverts = (hw_rasta_vert*)os_malloc( num_wedges*sizeof(hw_rasta_vert) );//NEW hw_rasta_vert[nwedges];
    os_free( vert_refs_for_wedge_ref );
    vert_refs_for_wedge_ref = (vert_ref*)os_malloc( num_wedges*sizeof(vert_ref) );
    os_free( wedge_lod_starts );
    wedge_lod_starts = (short*)os_malloc( num_wedges*sizeof(short) );
  }

//#pragma fixme( "no wonder text mesh loading is so slow, these should be else if. -mkv 5/11/01" )
	if( CHUNK_MESH_FLAGS == flavor_flav ) {
    succeeded = true;
    unsigned int flags = 0;
    serial_in( fs, &flags );
    // ignore these
  }

  return succeeded;
}


class lod_info
{
public:
  float ranges[4];
  stringx names[4];

  lod_info( void )
  {

    for( int i = 0; i < 3; i++ ) {
      ranges[i] = 0.0f;
    }

  }
};

static const chunk_flavor CHUNK_LOD( "lod" );

void serial_in( chunk_file& fs, lod_info* lod )
{
  chunk_flavor cf;
  int level;
  float range;
  stringx name;

  for( serial_in( fs, &cf ); cf != CHUNK_END; serial_in( fs, &cf ) ) {

    if( cf == chunk_flavor( "level" ) ) {
      serial_in( fs, &level );
    } else if( cf == chunk_flavor( "range" ) ) {
      serial_in( fs, &range );
    } else if( cf == chunk_flavor( "name" ) ) {
      serial_in( fs, &name );
    } else {
      error( "unknown chunk parsing lod tag" );
    }

  }

  lod->ranges[level] = range;
  lod->names[level] = name;
}

bool vr_pmesh::read_stuff2(chunk_file& fs, chunk_flavor flavor_flav, int & last_wedges_lod, int & wedge_count, int & face_count, const stringx& texture_dir, int & vsplit_count)
{
  bool succeeded = false;
  if( CHUNK_WEDGE == flavor_flav )
  {
    succeeded = true;
    // set defaults
    wedge w;
    int lod_start;
    hw_rasta_vert my_vert_info;
    vector3d my_normal;
    my_vert_info.boneid() = 0;

    my_vert_info.diffuse = color32(255,255,255,255);
    w.lower_detail = UNINITIALIZED_WEDGE_REF;
    w.level_of_detail = 0;

    // wedges always begin with vert ref

    vert_ref my_vert_ref;
    serial_in( fs, &my_vert_ref);

    int tex_coord_counter = 0;

    // but then they're flexible
    for(;;)
    {
      chunk_flavor cf;
      bool success = false;
      serial_in( fs, &cf );
      if( cf==CHUNK_TEXTURE_COORD)
      {
        texture_coord tc;
        serial_in( fs, &tc );

        if(tex_coord_counter >= MAX_TEXTURE_COORDS)
          error("Maximum of allowed texture coords on a pmesh is %d", MAX_TEXTURE_COORDS);

        my_vert_info.tc[tex_coord_counter] = vector2d(tc.x, tc.y);
        tex_coord_counter++;
        success = true;
      }
      if(cf==CHUNK_NORMAL)
      {
        serial_in( fs, &my_normal);
        success = true;
      }
      if(cf==CHUNK_COLOR)
      {
        color temp_color;
        serial_in( fs, &temp_color );
        my_vert_info.diffuse = temp_color.to_color32();
        success = true;
      }
      if(cf==chunk_flavor("detail"))
      {
        int level_of_detail;
        serial_in( fs, &level_of_detail );
        w.level_of_detail = level_of_detail;
        serial_in( fs, &w.lower_detail );
        if(w.level_of_detail==0)
          ++min_detail;
        if(level_of_detail>0)
          progressive=true;
        // make a note of how many wedges to xform for the highest level of detail
        max_detail = verts->size();
        success = true;
      }
      if(cf==chunk_flavor("lodstart"))
      {
        serial_in( fs, &lod_start );
        if( lod_start > SHRT_MAX )
          lod_start = SHRT_MAX;
        else
          lod_start = lod_start;
      }
      if(cf==chunk_flavor("bone"))
      {
        int bone;
        serial_in( fs, &bone );
        my_vert_info.boneid() = bone;
        success = true;
      }

      if(cf==CHUNK_END)
      {
        success = true;
        break;
      }
      if( success == false )
      {
      }
    }
    assert( verts );
    my_vert_info.xyz.x = (*verts)[ my_vert_ref ].point.x;
    my_vert_info.xyz.y = (*verts)[ my_vert_ref ].point.y;
    my_vert_info.xyz.z = (*verts)[ my_vert_ref ].point.z;
    int num_bones = (*verts)[ my_vert_ref ].num_bones;
    assert( num_bones <= MAX_SKIN_BONES );
    my_vert_info.num_bones = num_bones;
//KILL    int best_bone = 0;
//    float highest_weight = 0.0f;
    for( int i=0; i<num_bones; i++ )
    {
      my_vert_info.bone_ids[i] = (*verts)[ my_vert_ref ].bone_ids[i];
      my_vert_info.bone_weights[i] = (*verts)[ my_vert_ref ].bone_weights[i];
    }
//KILL    my_vert_info.boneid() = (*verts)[ my_vert_ref ].bone_ids[best_bone];

    assert(my_vert_info.xyz.is_valid());

    // every time level of detail changes, make a note so we know
    // how many verts to xform for that level of detail
    if( w.level_of_detail != last_wedges_lod )
    {
      for(int l=last_wedges_lod;l<w.level_of_detail;++l)
      {
        xverts_for_lod[ l ] = wedge_count;
      }
      last_wedges_lod = w.level_of_detail;
    }

    vert_refs_for_wedge_ref[wedge_count] = my_vert_ref;
    wedges[wedge_count] = w;
    xverts[wedge_count] = my_vert_info;

    xverts[wedge_count].set_normal(my_normal);

    wedge_lod_starts[wedge_count] = lod_start;

    if( my_vert_info.diffuse.c.a < 255 )
    {
      has_translucent_verts = true;
    }

    ++wedge_count;
  }
  if( CHUNK_NUM_FACES == flavor_flav )
  {
    succeeded = true;
    serial_in( fs, &num_faces );
    faces = (face*)os_malloc( num_faces*sizeof(face) );  //vector<face>(nfaces);
  }
  if ( flavor_flav == CHUNK_FACE ||
            flavor_flav == CHUNK_FACE_WITH_FLAGS ||
            flavor_flav == CHUNK_PROGRESSIVE_FACE )
  {
    succeeded = true;
    faces[face_count] = face( fs, flavor_flav );
    ++face_count;
  }
  if( CHUNK_MATERIAL == flavor_flav )
  {
    succeeded = true;
    material * new_material;
    // this used to be a material_bank instance but I found it annoying for development purposes. -JDF
    new_material = NEW  material( fs,
                                 texture_dir,
                                 ((mesh_flags&FORCE_LIGHT)?
                                   0 :
                                   MAT_ALLOW_ADDITIVE_LIGHT ),
                                  0);
    materials.push_back( new_material );
  }
  if( CHUNK_VSPLIT == flavor_flav )
  {
    succeeded = true;
    vsplit current_vs( fs );
    int level_of_detail = vsplit_count+1;
	  int face_ids0 = current_vs.new_face_ids[0];
	  int face_ids1 = current_vs.new_face_ids[1];
	  if( face_ids0 != (int)UNINITIALIZED_FACE_REF )
		  faces[ current_vs.new_face_ids[0] ].level_of_detail = level_of_detail;
	  if( face_ids1 != (int)UNINITIALIZED_FACE_REF )
		  faces[ current_vs.new_face_ids[1] ].level_of_detail = level_of_detail;
    for(int j=0;j<(int)current_vs.wedge_splits.size();++j)
    {
      wedge_split current_ws = current_vs.wedge_splits[j];
      wedges[current_ws.new_wedge_id].lower_detail = current_ws.old_wedge_id;
      wedges[current_ws.new_wedge_id].level_of_detail = level_of_detail;
    }
    ++vsplit_count;
    max_detail = vsplit_count;
  }
  if( CHUNK_LOD == flavor_flav )
  {
    lod_info lod;
    serial_in( fs, &lod );
    succeeded = true;
//#pragma todo( "do something novel with lod info. -mkv 4/6/01" )
  }
  return succeeded;
}



void vr_pmesh::_construct(const stringx& pmesh_filename, unsigned _mesh_flags )
{
  has_translucent_verts = false;
  mesh_flags = _mesh_flags | LIT_LAST_FRAME;

  // <<<< most of the asserts in this file should give warnings about
  //      where file is corrupt to user, that is if this remains a text file
  //      that people touch
  verts = NULL;
  xverts = NULL;
  uvanim_update_frame = 0;
  wedges = NULL;
  vert_refs_for_wedge_ref = NULL;
  wedge_lod_starts = NULL;
  original_face_for_face_slot = NULL;
  faces = NULL;
  wedge_index_list = NULL;
  reduced_faces = NULL;
  pivot = ZEROVEC;
  pivot_valid = false;
  xverts_for_lod = NULL;

  chunk_file fs;

  // use ".txtmesh" extension no matter what
  stringx adjusted_pmesh_filename;
  stringx::size_type dot_pos = pmesh_filename.find( 0, '.' );
  if( dot_pos != stringx::npos )
    adjusted_pmesh_filename = pmesh_filename.substr( 0, dot_pos );
  else
    adjusted_pmesh_filename = pmesh_filename;
  adjusted_pmesh_filename += ".txtmesh";

  fs.open(adjusted_pmesh_filename);
  internal_serial_in( fs );
  filename = pmesh_filename;
}


// construct a pmesh consisting of a simple one-sided rectangle
void vr_pmesh::make_rectangle()
{
  pivot = ZEROVEC;
  pivot_valid = false;
  max_detail = 0;
  min_detail = INT_MAX;

  // 4 verts
  int nverts = 4;
  delete verts;
  verts = NEW vector<vert>(nverts);
  delete[] xverts_for_lod;
  xverts_for_lod = NEW int[nverts+1];

  // 4 wedges
  num_wedges = 4;
  os_free( wedges );
  wedges = (wedge*)os_malloc( num_wedges*sizeof(wedge) );  //  hope we get wedge to multiple of 32...
  os_free( xverts );
  xverts = (hw_rasta_vert*)os_malloc( num_wedges*sizeof(hw_rasta_vert) );//NEW hw_rasta_vert[nwedges];
  os_free( vert_refs_for_wedge_ref );
  vert_refs_for_wedge_ref = (vert_ref*)os_malloc( num_wedges*sizeof(vert_ref) );
  os_free( wedge_lod_starts );
  wedge_lod_starts = NULL;
  wedge w;
  hw_rasta_vert my_vert_info;
  my_vert_info.xyz.x=my_vert_info.xyz.y=my_vert_info.xyz.z=0.0F;
  vector3d my_normal( 1, 0, 0 );
  my_vert_info.boneid() = 0;
  my_vert_info.diffuse = color32(255,255,255,255);
  w.lower_detail = UNINITIALIZED_WEDGE_REF;
  w.level_of_detail = 0;
  int i;
  for ( i=0; i<4; ++i )
  {
    vert_refs_for_wedge_ref[i] = i;
    wedges[i] = w;
    xverts[i] = my_vert_info;
    xverts[i].set_normal(my_normal);
  }

  // look right, but may not be.....
  xverts[0].tc[0] = texture_coord(0,0);
  xverts[1].tc[0] = texture_coord(0,1);
  xverts[2].tc[0] = texture_coord(1,1);
  xverts[3].tc[0] = texture_coord(1,0);

  // 2 faces
  num_faces = 2;
  os_free( faces );
  faces = (face*)os_malloc( num_faces*sizeof(face) );  //vector<face>(nfaces);
  face* f = faces;
  f->wedge_refs[0] = 0;
  f->wedge_refs[1] = 1;
  f->wedge_refs[2] = 2;
  f->my_material = 0;
  f->level_of_detail = 0;
  f->flags = 0;
  ++f;
  f->wedge_refs[0] = 2;
  f->wedge_refs[1] = 3;
  f->wedge_refs[2] = 0;
  f->my_material = 0;
  f->level_of_detail = 0;
  f->flags = 0;

  // 1 material
#ifndef TARGET_PS2
/*
  for (i=materials.size(); --i>=0; )
    delete materials[i];
  material_changes.resize(0);
*/

  if(materials.empty())
    materials.push_back( NEW material( *app::inst()->get_game()->get_blank_material() ) );
#endif

  xverts_for_lod[0] = 4;
  min_faces = 2;
  optimize();  // deferred until later, because collision geometry needs it unsorted  <<<< why?
  mark_self_lit_verts();
  compute_info();
}

// construct a pmesh consisting of a simple two-sided triangle
void vr_pmesh::make_double_sided_triangle()
{
  //already_munged = false;
  pivot = ZEROVEC;
  pivot_valid = false;
  max_detail = 0;
  min_detail = INT_MAX;

  // 3 verts
  int nverts = 3;
  delete verts;
  verts = NEW vector<vert>(nverts);
  xverts_for_lod = NEW int[nverts+1];

  // 4 wedges
  num_wedges = 3;
  os_free( wedges );
  wedges = (wedge*)os_malloc( num_wedges*sizeof(wedge) );  //  hope we get wedge to multiple of 32...
  os_free( xverts );
  xverts = (hw_rasta_vert*)os_malloc( num_wedges*sizeof(hw_rasta_vert) );//NEW hw_rasta_vert[nwedges];
  os_free( vert_refs_for_wedge_ref );
  vert_refs_for_wedge_ref = (vert_ref*)os_malloc( num_wedges*sizeof(vert_ref) );
  os_free( wedge_lod_starts );
  wedge w;
  hw_rasta_vert my_vert_info;
  my_vert_info.xyz.x=my_vert_info.xyz.y=my_vert_info.xyz.z=0.0F;
  vector3d my_normal( 1, 0, 0 );
  my_vert_info.boneid() = 0;
  my_vert_info.diffuse = color32(255,255,255,255);
  w.lower_detail = UNINITIALIZED_WEDGE_REF;
  w.level_of_detail = 0;
  int i;
  for ( i=0; i<3; ++i )
  {
    vert_refs_for_wedge_ref[i] = i;
    wedges[i] = w;
    xverts[i] = my_vert_info;
    xverts[i].set_normal(my_normal);
  }

  // look right, but may not be.....
  xverts[0].tc[0] = texture_coord(0,0);
  xverts[1].tc[0] = texture_coord(0,1);
  xverts[2].tc[0] = texture_coord(1,0);

  // 2 faces
  num_faces = 2;
  faces = (face*)os_malloc( num_faces*sizeof(face) );  //vector<face>(nfaces);
  face* f = faces;
  f->wedge_refs[0] = 0;
  f->wedge_refs[1] = 1;
  f->wedge_refs[2] = 2;
  f->my_material = 0;
  f->level_of_detail = 0;
  f->flags = 0;
  ++f;
  f->wedge_refs[0] = 2;
  f->wedge_refs[1] = 1;
  f->wedge_refs[2] = 0;
  f->my_material = 0;
  f->level_of_detail = 0;
  f->flags = 0;

  // 1 material
#ifndef TARGET_PS2
/*
  for (i=materials.size(); --i>=0; )
    delete materials[i];
  material_changes.resize(0);
*/

  if(materials.empty())
     materials.push_back( NEW material( *app::inst()->get_game()->get_blank_material() ) );
//    materials.push_back( material_bank.new_instance( app::inst()->get_game()->get_blank_material() ) );
#endif

  xverts_for_lod[0] = 3;
  min_faces = 2;
  optimize();  // deferred until later, because collision geometry needs it unsorted  <<<< why?
  mark_self_lit_verts();
  compute_info();
}


// construct a pmesh consisting of a simple one-sided rectangle

void vr_pmesh::make_n_tangle(int n)
{
  assert (n<MAX_N_TANGLE_FACES);
  making_n_tangle = true;

  pivot = ZEROVEC;
  pivot_valid = false;
  max_detail = 0;
  min_detail = INT_MAX;

  // 3*n verts
  int nverts = 3*n; // 1+2*n;
  if( verts )
  {
    delete verts;
    verts = NULL;
  }
  xverts_for_lod = global_n_tangle_xverts_for_lod; //NEW int[nverts+1];

  // 4 wedges
  num_wedges = nverts;
  wedges = global_n_tangle_wedges;
  xverts = global_n_tangle_rasta_verts;
  os_free( vert_refs_for_wedge_ref );
  os_free( wedge_lod_starts );
  wedge w;
  hw_rasta_vert my_vert_info;
  my_vert_info.xyz.x=my_vert_info.xyz.y=my_vert_info.xyz.z=0.0F;
  vector3d my_normal( 1, 0, 0 );
  my_vert_info.boneid() = 0;
  my_vert_info.diffuse = color32(255,255,255,255);
  w.lower_detail = UNINITIALIZED_WEDGE_REF;
  w.level_of_detail = 0;
  int i;
  for ( i=0; i<nverts; ++i )
  {
    wedges[i] = w;
    xverts[i] = my_vert_info;
    xverts[i].set_normal(my_normal);
  }

  // 2 faces
  num_faces = n;
  faces = global_n_tangle_faces; //(face*)os_malloc( num_faces*sizeof(face) );  //vector<face>(nfaces);
  face* f = faces;

  xverts[0].tc[0] = texture_coord(0,0);
  int i3=0;
  for (i=0;i<num_faces;++i,i3+=3)
  {
    // look right, but may not be.....
    xverts[1+2*i].tc[0] = texture_coord(0,1);
    xverts[2+2*i].tc[0] = texture_coord(1,1);

    f->wedge_refs[0] = i3;
    f->wedge_refs[1] = i3+1;
    f->wedge_refs[2] = i3+2;
    f->my_material = 0;
    f->level_of_detail = 0;
    f->flags = 0;
    ++f;
  }

  // 1 material
#ifndef TARGET_PS2
/*
  for (i=materials.size(); --i>=0; )
    delete materials[i];
  material_changes.resize(0);
*/

  if(materials.empty())
    materials.push_back( NEW material( *app::inst()->get_game()->get_blank_material() ) );
//    materials.push_back( material_bank.new_instance( app::inst()->get_game()->get_blank_material() ) );
#endif

  xverts_for_lod[0] = nverts;
  min_faces = n;
  optimize();  // deferred until later, because collision geometry needs it unsorted  <<<< why?
  mark_self_lit_verts();
  compute_info();
  making_n_tangle = false;
}


void vr_pmesh::shrink_memory_footprint()
{
  if( vert_refs_for_wedge_ref )
    os_free( vert_refs_for_wedge_ref );
  vert_refs_for_wedge_ref = NULL;
  if( verts )
    delete verts;
  verts = NULL;
}

void vr_pmesh::compute_info()
{
  int i;
  bounding_box box;

  //float radius2 = 0.0f;
  for(i = get_num_wedges(); --i>=0; )
  {
    const vector3d& p = xverts[i].xyz;
    assert(p.is_valid());
    box.accumulate(p);
    //radius2 = max(p.length2(), radius2);
  }
  //radius = __fsqrt(radius2);
  center = box.center();
  assert(center.is_valid());
  float radius_rel_center2 = 0.0f;
  for(i = get_num_wedges(); --i>=0; )
  {
    vector3d delta = xverts[i].xyz - center;
    radius_rel_center2 = max( delta.length2(), radius_rel_center2 );
  }
  radius = __fsqrt(radius_rel_center2);
}


// compute visual center and radius for skin
void vr_pmesh::compute_info( po* bones, int num_bones )
{
  int i;

  // compute visual center
  bounding_box box;
  for ( i=get_num_wedges(); --i>=0; )
  {
    const hw_rasta_vert& v = xverts[i];
    assert( v.xyz.is_valid() && v.boneid()<num_bones );
    vector3d p = bones[v.boneid()].fast_8byte_xform( v.xyz );
    box.accumulate( p );
  }
  center = box.center();
  assert( center.is_valid() );

  float radius_rel_center2 = 0.0f;
  for ( i=get_num_wedges(); --i>=0; )
  {
    const hw_rasta_vert& v = xverts[i];
    vector3d p = bones[v.boneid()].fast_8byte_xform( v.xyz );
    p -= center;
    radius_rel_center2 = max( p.length2(), radius_rel_center2 );
  }
  radius = __fsqrt( radius_rel_center2 );
}


rational_t vr_pmesh::compute_xz_radius_rel_center( const po& xform )
{
#if defined(TARGET_PC)  || XBOX_USE_PMESH_STUFF
  if (optimized_verts)
    return radius * xform.get_scale();
#endif

  rational_t rad2 = 0.0f;
  int i;
  vector3d ctr = xform.non_affine_slow_xform( center );
  for ( i=get_num_wedges(); --i>=0; )
  {
    vector3d v = xform.non_affine_slow_xform( xverts[i].xyz );
    v -= ctr;
    rad2 = max( v.xz_length2(), rad2 );
  }
  return __fsqrt(rad2);
}


#ifdef DEBUG
static vr_pmesh* fmc_this;
#endif

//int face_material_compare(const void *v1, const void *v2);
static int face_material_compare(const void *v1, const void *v2)
{
  face *f1 = (face *)v1, *f2 = (face *)v2;
  if( f1->get_material_ref()==f2->get_material_ref())
  {
    return f1->level_of_detail - f2->level_of_detail;
  }
  return f1->get_material_ref() - f2->get_material_ref();
}


// move this step into exporters to decrease load time...or implement Wade's evil heap load
void vr_pmesh::optimize()
{
  int i;
  // adjusting texture coordinates to eliminate warble

  // Tool for unsorting the list as needed.  This is the inverse of its final value (see below).
  vector<unsigned short> local_original_occupant_list(get_max_faces());

  #ifdef DEBUG
  fmc_this = this;
  #endif
  qsort(&faces[0], get_max_faces(), sizeof(face), face_material_compare);  // how depressingly old-tech
  // find where materials change:  parallel to materials
  //  maybe we should store material_change in material?
  material_changes.clear();
  material_ref cur_material = UNINITIALIZED_MATERIAL_REF;
  for ( i=0; i<get_max_faces(); ++i )
  {
    material_ref new_material = faces[i].get_material_ref();
    if ( new_material != cur_material )
    {
      // insert value representing face index at which this material starts
      material_changes.insert( material_map::value_type( new_material, i ) );
      cur_material = new_material;
    }
  }

  // simplify primitives for non-progressive meshes:
  if(get_min_faces()==get_max_faces())
  {
//    assert(wedge_index_list == NULL);
//    assert(reduced_faces == NULL);
    if (making_n_tangle)
    {
      wedge_index_list = global_n_tangle_wedge_refs; //NEW wedge_ref[ num_faces*3 ];
      reduced_faces = global_n_tangle_reduced_faces; //NEW reduced_face[ num_faces ];
    }
    else
    {
      delete[] wedge_index_list;
      delete[] reduced_faces;
      wedge_index_list = NEW wedge_ref[ num_faces*3 ];
      reduced_faces = NEW reduced_face[ num_faces ];
    }
    for(i=0;i<num_faces;++i)
    {
      for(int j=0;j<3;++j)
        wedge_index_list[i*3+j] = faces[i].get_wedge_ref(j);
      reduced_faces[i].my_material = faces[i].my_material;
      reduced_faces[i].flags = faces[i].flags;
      reduced_faces[i].level_of_detail = 0;
    }
    if (!making_n_tangle)
    {
      os_free( faces );
    }
    faces = NULL;
  }
  else
  {
    wedge_index_list = NULL;
  }

}

void vr_pmesh::mark_self_lit_verts()
{
  if(!materials.empty())
  {
    int i;
    for( i=get_num_wedges(); --i>=0; )
      xverts[i].clip_flags &= NORMAL_MASK;
  }
}

#if defined(TARGET_PC) || XBOX_USE_PMESH_STUFF
void vr_pmesh::optimize_static_mesh_for_d3d()
{
  if (optimized_verts) return; // already done this!
  if (is_uv_animated()) return;
  // this is present to allow higher level functions to override whether lower
  // level functions may optimize their meshes, since the higher functions may
  // know more about how the object will end up being used, for instance
  // skinned or lit models are not candidates for optimization, however we
  // can't tell how the model will end up being used from down here.  So this
  // allows the functions that *do* know to notify us.
  if (!allow_optimize_static_mesh_for_d3d()) return;

  unsigned how_many_verts = xverts_for_lod[get_max_detail()];
  if (!how_many_verts) return;

#if XBOX_USE_PMESH_STUFF
  optimized_verts = NEW vert_lit_buf(how_many_verts);
#else
  optimized_verts = NEW vert_buf(how_many_verts);
#endif /* XBOX_USE_PMESH_STUFF JIV DEBUG */
  // copy vert info into optimized_verts
  optimized_verts->lock(how_many_verts);

  hw_rasta_vert* tvlit;
  hw_rasta_vert_lit* dest;
  hw_rasta_vert* workspace_end = xverts + how_many_verts;
  // strip out normal as we copy
  for (tvlit=xverts, dest=optimized_verts->begin(); tvlit!=workspace_end; ++tvlit,++dest)
  {
    dest->set_xyz(tvlit->get_unxform());
    for(int i=0; i<MAX_TEXTURE_COORDS; ++i)
      dest->tc[i] = tvlit->tc[i];
    dest->diffuse.i = tvlit->diffuse.i;
  }

  optimized_verts->unlock();
  optimized_verts->optimize();
  os_free( wedges );
  wedges = NULL;

  // don't delete xverts:  we need the normals for environment mapping

  os_free( vert_refs_for_wedge_ref );
  vert_refs_for_wedge_ref = NULL;
  os_free( wedge_lod_starts );
  wedge_lod_starts = NULL;

  int how_many_faces = get_max_faces();
  assert(how_many_faces);
#if 1
  unsigned short* new_index_list = NEW unsigned short[how_many_faces*3];
  unsigned short* new_indices = new_index_list;
  #ifdef DEBUG
  memset(new_index_list,0xfe,how_many_faces*3*sizeof(unsigned short));
  #endif
  material_map::iterator mi;
  #if defined(DEBUG) && 0
  if (how_many_faces>=8 && how_many_faces<=8)
  {
    debug_print("rearranging %d indices for vertex cache coherence", how_many_faces*3);
    debug_print("original faces");
    for (int n=0; n<how_many_faces; ++n)
    {
      debug_print("%d,%d,%d", wedge_index_list[n*3+0], wedge_index_list[n*3+1], wedge_index_list[n*3+2]);
    }
  }
  #endif
  for (mi=material_changes.begin(); mi!=material_changes.end(); ++mi)
  {
    int material_idx = (*mi).first;
    // iterate through faces until reaching end of current material
    int start = (*mi).second;
    int end;
    material_map::iterator next = mi;
    ++next;
    if ( next == material_changes.end() )
      end = how_many_faces;
    else
      end = (*next).second;

    unsigned short* index_start = wedge_index_list + start*3;
    int num_faces = (end - start);
    const unsigned short* match = new_indices;
    // Begin of crappy O(n log n) algorithm to search for best next face
    // that will make optimal use of the vertex cache.  This could be improved a lot.
    if (num_faces) // must copy first face
    {
      for (int k=0; k<3; ++k)
      {
        *new_indices = *index_start;
        ++index_start;
        ++new_indices;
      }
      --num_faces;
    }
    for (int m=num_faces; --m>=0; )
    {
      unsigned short* index = index_start;
      unsigned short* best_index = NULL;
      unsigned max_count=0;
      for (int j=num_faces; --j>=0; index+=3)
      {
        if (index[0] == 0xffff)
        {
          if (index_start == index)
          {
            index_start+=3;
            --num_faces;
          }
          continue;
        }
        if (best_index == NULL)
          best_index = index;
        unsigned count=0;
        if (index[0] == match[0]) ++count;
        if (index[0] == match[1]) ++count;
        if (index[0] == match[2]) ++count;
        if (index[1] == match[0]) ++count;
        if (index[1] == match[1]) ++count;
        if (index[1] == match[2]) ++count;
        if (index[2] == match[0]) ++count;
        if (index[2] == match[1]) ++count;
        if (index[2] == match[2]) ++count;
        if (count > max_count)
        {
          max_count = count;
          best_index = index;
        }
      }
      assert(best_index!=NULL);
      match = new_indices;
      // should maybe rotate the indices to get the best match, if I get time later
      for (int k=0; k<3; ++k)
      {
        *new_indices = *best_index;
        *best_index = 0xffff;
        ++best_index;
        ++new_indices;
      }
    }
  }
  #if defined(DEBUG) && 0
  if (how_many_faces>=6 && how_many_faces<=12)
  {
    debug_print("NEW faces");
    for (int n=0; n<how_many_faces; ++n)
    {
      assert(new_index_list[n*3+0]<=0x0fff &&
             new_index_list[n*3+1]<=0x0fff &&
             new_index_list[n*3+2]<=0x0fff);
      debug_print("%d,%d,%d", new_index_list[n*3+0], new_index_list[n*3+1], new_index_list[n*3+2]);
    }
  }
  #endif
  memcpy(wedge_index_list,new_index_list,how_many_faces*3*sizeof(unsigned short));
  delete [] new_index_list;
#endif
}
#endif // TARGET_PC

void vr_pmesh::rescale_verts(rational_t s)
{
  int i;
  if( verts )
  {
    for (i=verts->size(); --i>=0; )
    {
      (*verts)[i] = vert(s*((*verts)[i].get_point()));
    }
    for (i=get_num_wedges(); --i>=0; )
    {
      xverts[i].xyz = (*verts)[ vert_refs_for_wedge_ref[i] ].point;
    }
  }
}


const stringx& vr_pmesh::get_bone_name( int x )
{
  return bones[x].name;
}

#ifdef DEBUG
//static int debug_hack;
#endif

const po& vr_pmesh::get_bone_pivot( int x )
{
  return bones[x].pivot;
}


void vr_pmesh::set_light_method( light_method_t lmt )
{
  if(lmt==LIGHT_METHOD_DIFFUSE)
  {
    mesh_flags = (mesh_flags & ~1) | FORCE_LIGHT; // isn't there a named constant we can use instead of ~1?
  }
  else
  {
    mesh_flags = (mesh_flags & ~1) | NORMAL;
  }
  // this code relies on textures that aren't lit not
  // being shared with meshes whose textures are lit:
  // this should be the case, because skin, entity,
  // and level textures are all kept in their own
  // subdirectories
  for( int i=0 ; i<(int)materials.size() ; ++i )
  {
    if(lmt==LIGHT_METHOD_DIFFUSE)
	    materials[i]->set_flags( materials[i]->get_flags() & (0xffff^MAT_ALLOW_ADDITIVE_LIGHT) );
	  else
	    materials[i]->set_flags( materials[i]->get_flags()| MAT_ALLOW_ADDITIVE_LIGHT );

	  materials[i]->process_vertex_contexts();
  }

}

wedge_ref vr_pmesh::get_wedge_ref( face_ref fr, int corner ) const
{
  // if this stuff is too slow we could make progressive and non-progressive meshes two different
  // subclasses of vr_pmesh...
  if(faces)
    return faces[fr].get_wedge_ref(corner);
  return wedge_index_list[ fr*3+corner ];
}

bool vr_pmesh::is_water( face_ref fr ) const
{
  if(faces)
    return faces[fr].is_water();
  return reduced_faces[fr].is_water();
}

bool vr_pmesh::is_walkable( face_ref fr ) const
{
  if(faces)
    return faces[fr].is_walkable();
  return reduced_faces[fr].is_walkable();
}

bool vr_pmesh::is_notwalkable( face_ref fr ) const
{
  if(faces)
    return faces[fr].is_notwalkable();
  return reduced_faces[fr].is_walkable();
}

unsigned char vr_pmesh::get_surface_type( face_ref faceid ) const
{
  if(faces)
    return faces[faceid].get_surface_type();
  return reduced_faces[faceid].get_surface_type();
}

bool vr_pmesh::is_cosmetic( face_ref fr ) const
{
  if(faces)
    return faces[fr].is_cosmetic();
  return reduced_faces[fr].is_cosmetic();
}

material_ref vr_pmesh::get_material_ref( face_ref fr ) const
{
  if(faces)
    return faces[fr].get_material_ref();
  return reduced_faces[fr].get_material_ref();
}

unsigned short vr_pmesh::get_face_flags( face_ref fr ) const
{
  if(faces)
    return faces[fr].get_flags();
  return reduced_faces[fr].get_flags();
}


