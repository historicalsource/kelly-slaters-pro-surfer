// debug_render.cpp
// Copyright (c) 2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.
// rendering functions for drawing stuff for debugging purposes
// this stuff doesn't have to be all that fast

#include "global.h"

#include "debug_render.h"

#include "app.h"
#include "game.h"
#include "sphere.h"
#include "plane.h"
#include "beam.h"
#include "iri.h"
#include "pmesh.h"
#include "vertwork.h"
#include "geomgr.h"
#include "cface.h"
#include "colmesh.h"
#include "entity.h"
#include "terrain.h"  // for partition3
#include "wds.h"
#include "text_font.h"
#include "osdevopts.h"

static vr_pmesh * global_cap_base=NULL;
static vr_pmesh * global_cap_mid=NULL;
static vr_pmesh * global_cap_end=NULL;

#ifndef BUILD_BOOTABLE
static vr_pmesh * global_pmeshes[GLOBAL_PMESH_COUNT];

static vr_pmesh * frustum_pmesh=NULL;
#endif

// No longer supported.  (dc 12/17/01)
/*
#if defined(TARGET_XBOX)
static Font *g_debug_font;
#else
static text_font* g_debug_font;
#endif // TARGET_XBOX JIV DEBUG
*/

bool debug_render_init() // initialize debug render subsystem
{
  global_cap_base = NULL;
  global_cap_mid = NULL;
  global_cap_end = NULL;
/*
  g_debug_font = NULL;

  if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_STASH_ONLY))
  {
#if defined(TARGET_XBOX)
    g_debug_font = NEW Font();
    g_debug_font->load("biofont");
#else
    g_debug_font = NEW text_font("font8x12");
#endif // TARGET_XBOX JIV DEBUG
  }
  else
  {
#if defined(TARGET_XBOX)
    g_debug_font = NEW Font();
    g_debug_font->load("biofont");
#else
    g_debug_font = NEW text_font("font8x12");
#endif // TARGET_XBOX JIV DEBUG
  }
*/

  return true;
}

bool debug_render_done() // shutdown debug render subsystem
{
#ifndef BUILD_BOOTABLE
  if (global_cap_base)
  {
    delete global_cap_base;
    global_cap_base = NULL;
  }
  if (global_cap_mid)
  {
    delete global_cap_mid;
    global_cap_mid = NULL;
  }
  if (global_cap_end)
  {
    delete global_cap_end;
    global_cap_end = NULL;
  }
  for (int i=GLOBAL_PMESH_COUNT; --i>=0; )
  {
    delete global_pmeshes[i];
    global_pmeshes[i] = NULL;
  }

  if (frustum_pmesh)
  {
    delete frustum_pmesh;
    frustum_pmesh = NULL;
  }
#endif

/*
  if (g_debug_font)
  {
    delete g_debug_font;
    g_debug_font = NULL;
  }
*/
  return true;
}


void render_sphere(const vector3d &pos, rational_t radius, color32 col)
{
#ifndef BUILD_BOOTABLE
  if (radius < 0.05f) radius = 0.05f;

  vector3d right(radius, 0, 0), up(0, radius, 0), forward(0, 0, radius);
  po m(right,up,forward,pos);

  instance_render_info iri(global_cap_base->get_max_faces(), m, 0, app::inst()->get_game()->get_current_view_camera()->get_region(), 0, col, FORCE_TRANSLUCENCY );
  global_cap_base->render_instance(RENDER_TRANSLUCENT_PORTION|RENDER_CLIPPED_FULL_DETAIL|RENDER_REAL_POINT_LIGHTS, &iri);
  global_cap_end->render_instance(RENDER_TRANSLUCENT_PORTION|RENDER_CLIPPED_FULL_DETAIL|RENDER_REAL_POINT_LIGHTS, &iri);
#endif
}

void render_capsule(const vector3d &base, const vector3d &end, rational_t radius, color32 col)
{
#ifndef BUILD_BOOTABLE
  if (radius < 0.05f) radius = 0.05f;

  vector3d p0 = base;
  vector3d p1 = end;
  po m_base,m_mid,m_end;

  vector3d xvec, yvec, zvec;
  yvec = p1-p0;
  rational_t r = radius;
  rational_t yvec_len = yvec.length();
  yvec.normalize();
  if (__fabs(yvec.z)>__fabs(yvec.x))
  {
    zvec = cross(XVEC,yvec);
    zvec.normalize();
    xvec = cross(yvec,zvec);
  }
  else
  {
    xvec = cross(yvec,ZVEC);
    xvec.normalize();
    zvec = cross(xvec,yvec);
  }
  m_base = po(xvec*r,yvec*r,zvec*r,p0);
  m_mid = po(xvec*r,yvec*yvec_len,zvec*r,p0);
  m_end = po(xvec*r,yvec*r,zvec*r,p1);

  instance_render_info iri_base (global_cap_base->get_max_faces(), m_base, 0, app::inst()->get_game()->get_current_view_camera()->get_region(), 0, col, FORCE_TRANSLUCENCY );
  global_cap_base->render_instance(RENDER_TRANSLUCENT_PORTION|RENDER_CLIPPED_FULL_DETAIL|RENDER_REAL_POINT_LIGHTS, &iri_base);

  instance_render_info iri_mid (global_cap_mid->get_max_faces(), m_mid, 0, app::inst()->get_game()->get_current_view_camera()->get_region(), 0, col, FORCE_TRANSLUCENCY );
  global_cap_mid->render_instance(RENDER_TRANSLUCENT_PORTION|RENDER_CLIPPED_FULL_DETAIL|RENDER_REAL_POINT_LIGHTS, &iri_mid);

  instance_render_info iri_end (global_cap_end->get_max_faces(), m_end, 0, app::inst()->get_game()->get_current_view_camera()->get_region(), 0, col, FORCE_TRANSLUCENCY );
  global_cap_end->render_instance(RENDER_TRANSLUCENT_PORTION|RENDER_CLIPPED_FULL_DETAIL|RENDER_REAL_POINT_LIGHTS, &iri_end);
#endif
}

void render_cylinder(const vector3d &base, const vector3d &end, rational_t radius, color32 col)
{
#ifndef BUILD_BOOTABLE
  if (radius < 0.05f) radius = 0.05f;

  vector3d p0 = base;
  vector3d p1 = end;
  po m_base,m_mid,m_end;

  vector3d xvec, yvec, zvec;
  yvec = p1-p0;
  rational_t r = radius;
  rational_t yvec_len = yvec.length();
  if (yvec_len<1e-5f) return;
  yvec.normalize();
  if (__fabs(yvec.z)>__fabs(yvec.x))
  {
    zvec = cross(XVEC,yvec);
    zvec.normalize();
    xvec = cross(yvec,zvec);
  }
  else
  {
    xvec = cross(yvec,ZVEC);
    xvec.normalize();
    zvec = cross(xvec,yvec);
  }
  m_base = po(xvec*r,yvec*1e-9f,zvec*r,p0);
  m_mid = po(xvec*r,yvec*yvec_len,zvec*r,p0);
  m_end = po(xvec*r,yvec*1e-9f,zvec*r,p1);

  instance_render_info iri_base(global_cap_base->get_max_faces(), m_base, 0, app::inst()->get_game()->get_current_view_camera()->get_region(), 0, col, FORCE_TRANSLUCENCY );
  global_cap_base->render_instance(RENDER_TRANSLUCENT_PORTION|RENDER_CLIPPED_FULL_DETAIL|RENDER_REAL_POINT_LIGHTS, &iri_base);

  instance_render_info iri_mid(global_cap_mid->get_max_faces(), m_mid, 0, app::inst()->get_game()->get_current_view_camera()->get_region(), 0, col, FORCE_TRANSLUCENCY );
  global_cap_mid->render_instance(RENDER_TRANSLUCENT_PORTION|RENDER_CLIPPED_FULL_DETAIL|RENDER_REAL_POINT_LIGHTS, &iri_mid);

  instance_render_info iri_end(global_cap_end->get_max_faces(), m_end, 0, app::inst()->get_game()->get_current_view_camera()->get_region(), 0, col, FORCE_TRANSLUCENCY );
  global_cap_end->render_instance(RENDER_TRANSLUCENT_PORTION|RENDER_CLIPPED_FULL_DETAIL|RENDER_REAL_POINT_LIGHTS, &iri_end);
#endif
}


void render_triangle(const vector3d &pt1, const vector3d &pt2, const vector3d &pt3,
                     color32 col, bool double_sided)
{
  static const uint16 debug_render_indices[6]=
  {
    0,1,2,   // front
    0,2,1,   // back
  };
  uint32 num_indices=double_sided ? 6 : 3;
  uint32 num_verts=3;

  app::inst()->get_game()->get_blank_material()->send_context(0, MAP_DIFFUSE, FORCE_TRANSLUCENCY);
#ifdef TARGET_MKS
  const matrix4x4& world_to_viewport = geometry_manager::inst()->xforms[geometry_manager::XFORM_WORLD_TO_SCREEN];

  vert_workspace_xformed_quad.lock(3);
  hw_rasta_vert_xformed * vert_it = vert_workspace_xformed.begin();
  hw_rasta_vert_xformed * vert_begin = vert_it;

  vector4d pp[3];
  pp[0]=vector4d(pt1,1.0f);
  pp[1]=vector4d(pt2,1.0f);
  pp[2]=vector4d(pt3,1.0f);

  int i;
  for (i=0; i<3; ++i)
  {
    pp[i]=xform4d(world_to_viewport,pp[i]);
    pp[i].w=1.0f/pp[i].w;
    pp[i].x *= pp[i].w;
    pp[i].y *= pp[i].w;
    pp[i].z *= pp[i].w;

    vert_it->set_xyz_rhw(pp[i]);
    vert_it->diffuse = col;
    ++vert_it;
  }

  vert_begin[0].tc[0] = texture_coord(0.0f,0.0f);
  vert_begin[1].tc[0] = texture_coord(0.5f,0.86f);
  vert_begin[2].tc[0] = texture_coord(1.0f,0.0f);

  vert_workspace_xformed.unlock();

  // send it to the card
  hw_rasta::inst()->send_indexed_vertex_list( vert_workspace_xformed, num_verts,
    debug_render_indices, num_indices,
    hw_rasta::SEND_VERT_FRONT_CLIP);
#else
  geometry_manager::inst()->set_local_to_world(identity_matrix);

  vert_workspace.lock(3);
  hw_rasta_vert_lit* vert_it = (hw_rasta_vert_lit*)vert_workspace.begin();

  vert_it->set_xyz(pt1);
  vert_it->diffuse = col;
  vert_it->tc[0] = texture_coord(0.0f,0.0f);
  ++vert_it;

  vert_it->set_xyz(pt2);
  vert_it->diffuse = col;
  vert_it->tc[0] = texture_coord(0.5f,0.86f);
  ++vert_it;

  vert_it->set_xyz(pt3);
  vert_it->diffuse = col;
  vert_it->tc[0] = texture_coord(1.0f,0.0f);
  ++vert_it;

  vert_workspace.unlock();

  // send it to the card (using list because it could be double-sided)
  hw_rasta::inst()->send_indexed_vertex_list(vert_workspace, num_verts,
                                             debug_render_indices, num_indices);
#endif
}

void render_quad(const vector3d &pt1, const vector3d &pt2, const vector3d &pt3, const vector3d &pt4,
                 color32 col, bool double_sided)
{
  app::inst()->get_game()->get_blank_material()->send_context(0, MAP_DIFFUSE, FORCE_TRANSLUCENCY);
  static const uint16 debug_render_indices[12]=
  {
    0,1,2, 0,2,3,      // front
    0,2,1, 0,3,2,      // back
  };
  uint32 num_indices=double_sided ? 12 : 6;
  uint32 num_verts=4;

#ifdef TARGET_MKS
  const matrix4x4& world_to_viewport = geometry_manager::inst()->xforms[geometry_manager::XFORM_WORLD_TO_SCREEN];

  vert_workspace_xformed.lock(4);
  hw_rasta_vert_xformed * vert_it = vert_workspace_xformed.begin();
  hw_rasta_vert_xformed * vert_begin = vert_it;

  vector4d pp[4];
  pp[0]=vector4d(pt1,1.0f);
  pp[1]=vector4d(pt2,1.0f);
  pp[2]=vector4d(pt3,1.0f);
  pp[3]=vector4d(pt4,1.0f);

  int i;
  for (i=0; i<4; ++i)
  {
    pp[i]=xform4d(world_to_viewport,pp[i]);
    pp[i].w=1.0f/pp[i].w;
    pp[i].x *= pp[i].w;
    pp[i].y *= pp[i].w;
    pp[i].z *= pp[i].w;

    vert_it->set_xyz_rhw(pp[i]);
    vert_it->diffuse = col;
    ++vert_it;
  }

  vert_begin[0].tc[0] = texture_coord(0.0f,0.0f);
  vert_begin[1].tc[0] = texture_coord(0.0f,1.0f);
  vert_begin[2].tc[0] = texture_coord(1.0f,1.0f);
  vert_begin[3].tc[0] = texture_coord(1.0f,0.0f);

  vert_workspace_xformed.unlock();

  // send it to the card
  hw_rasta::inst()->send_indexed_vertex_list( vert_workspace_xformed, num_verts,
    debug_render_indices, num_indices,
    hw_rasta::SEND_VERT_FRONT_CLIP);
#else
  geometry_manager::inst()->set_local_to_world(identity_matrix);

  vert_workspace.lock(4);
  hw_rasta_vert_lit * vert_it = (hw_rasta_vert_lit*)vert_workspace.begin();

  vert_it->xyz = pt1;
  vert_it->diffuse = col;
  vert_it->tc[0] = texture_coord(0.0f,0.0f);
  ++vert_it;

  vert_it->xyz = pt2;
  vert_it->diffuse = col;
  vert_it->tc[0] = texture_coord(0.0f,1.0f);
  ++vert_it;

  vert_it->xyz = pt3;
  vert_it->diffuse = col;
  vert_it->tc[0] = texture_coord(1.0f,1.0f);
  ++vert_it;

  vert_it->xyz = pt4;
  vert_it->diffuse = col;
  vert_it->tc[0] = texture_coord(1.0f,0.0f);
  ++vert_it;

  vert_workspace.unlock();

  // send it to the card (using list because it could be double-sided)
  hw_rasta::inst()->send_indexed_vertex_list(vert_workspace, num_verts,
    debug_render_indices, num_indices);
#endif
}


void render_quad_2d(rational_t l, rational_t t, rational_t r, rational_t b, rational_t z, color32 col)
{
#ifdef TARGET_PS2
  nglQuad q;
  nglInitQuad( &q );
  nglSetQuadZ( &q, z );
  nglSetQuadRect( &q, l, t, r, b );
  nglSetQuadColor( &q, NGL_RGBA32( (u_int) col.get_red(), (u_int) col.get_green(), (u_int) col.get_blue(), (u_int) col.get_alpha() ) );
  nglListAddQuad( &q );
#else
	#ifndef TARGET_GC
  vert_workspace_xformed.lock(4);
  hw_rasta_vert_xformed * vert_begin = vert_workspace_xformed.begin();

  #ifdef TARGET_PC
  rational_t invw = 1.0f;
  z = geometry_manager::inst()->fix_z_coord(z);
  #else
  rational_t invw;
  if (z < 1e-15f)
    invw = 1e15f;
  else
    invw = 1.0f/z;
  #endif

  vert_begin[0].set_xyz_rhw(vector3d(l,t,z),invw);
  vert_begin[0].diffuse=col;
  vert_begin[0].tc[0] = texture_coord(0.0f,0.0f);

  vert_begin[1].set_xyz_rhw(vector3d(r,t,z),invw);
  vert_begin[1].diffuse=col;
  vert_begin[1].tc[0] = texture_coord(1.0f,0.0f);

  vert_begin[2].set_xyz_rhw(vector3d(l,b,z),invw);
  vert_begin[2].diffuse=col;
  vert_begin[2].tc[0] = texture_coord(0.0f,1.0f);

  vert_begin[3].set_xyz_rhw(vector3d(r,b,z),invw);
  vert_begin[3].diffuse=col;
  vert_begin[3].tc[0] = texture_coord(1.0f,1.0f);

  vert_workspace_xformed.unlock();

  // send it to the card
  app::inst()->get_game()->get_blank_material()->send_context(0, MAP_DIFFUSE, FORCE_TRANSLUCENCY);
  hw_rasta::inst()->send_vertex_strip(vert_workspace_xformed, 4);
	#endif
#endif
}

// this one is a little wierd, but useful... coords are from -1,-1 (top left) to +1,+1 (bottom right)
// so pos = (0,0) and size = (1,1) will draw a fullscreen quad
void render_quad_2d(const vector2d& pos, const vector2d& size, rational_t z, color32 col)
{
  const matrix4x4& viewport = geometry_manager::inst()->get_viewport_xform();
  vector3d p1 = xform3d_1(viewport,vector3d(pos.x,-pos.y,1));
  vector3d s1 = xform3d_0(viewport,vector3d(size.x,-size.y,0));
  render_quad_2d(p1.x-s1.x,p1.y-s1.y,p1.x+s1.x,p1.y+s1.y, z, col);
}

void render_plane(const plane& pln, color32 front_color, color32 back_color, float size)
{
  vector3d norm = pln.get_unit_normal();
  vector3d ctr = pln.closest_point(geometry_manager::inst()->get_camera_pos());
  vector3d xvec, zvec;
  if (__fabs(norm.z)>__fabs(norm.x))
  {
    zvec = cross(XVEC,norm);
    zvec.normalize();
    xvec = cross(norm,zvec);
  }
  else
  {
    xvec = cross(norm,ZVEC);
    xvec.normalize();
    zvec = cross(xvec,norm);
  }
  xvec *= size; // make it really big!
  zvec *= size;
  vector3d p0,p1,p2,p3;
  p0 = ctr - xvec - zvec;
  p1 = ctr - xvec + zvec;
  p2 = ctr + xvec + zvec;
  p3 = ctr + xvec - zvec;
  //if (front_color!=color32(0,0,0,0))
    render_quad(p0,p1,p2,p3, front_color);
  //if (back_color!=color32(0,0,0,0))
    render_quad(p0,p3,p2,p1, back_color);
}

void render_box(const vector3d& center,
                const vector3d& halfsizex,
                const vector3d& halfsizey,
                const vector3d& halfsizez,
                color32 col)
{
  app::inst()->get_game()->get_blank_material()->send_context(0, MAP_DIFFUSE, FORCE_TRANSLUCENCY);
  static const uint16 debug_render_indices[6*6]=
  {
    0,1,3, 0,3,2,
    5,6,7, 5,4,6,
    1,7,3, 1,5,7,
    4,2,6, 4,0,2,
    6,2,3, 6,3,7,
    0,5,1, 0,4,5,
  };
  uint32 num_indices=6*6;
  uint32 num_verts=8;

#ifdef TARGET_MKS
  const matrix4x4& world_to_viewport = geometry_manager::inst()->xforms[geometry_manager::XFORM_WORLD_TO_SCREEN];

  vert_workspace_xformed.lock(8);
  hw_rasta_vert_xformed * vert_it = vert_workspace_xformed.begin();
  hw_rasta_vert_xformed * vert_begin = vert_it;

  vector4d pp[8];
  pp[0]=vector4d(center+halfsizex+halfsizey+halfsizez,1.0f);
  pp[1]=vector4d(center-halfsizex+halfsizey+halfsizez,1.0f);
  pp[2]=vector4d(center+halfsizex-halfsizey+halfsizez,1.0f);
  pp[3]=vector4d(center-halfsizex-halfsizey+halfsizez,1.0f);
  pp[4]=vector4d(center+halfsizex+halfsizey-halfsizez,1.0f);
  pp[5]=vector4d(center-halfsizex+halfsizey-halfsizez,1.0f);
  pp[6]=vector4d(center+halfsizex-halfsizey-halfsizez,1.0f);
  pp[7]=vector4d(center-halfsizex-halfsizey-halfsizez,1.0f);

  int i;
  for (i=0; i<8; ++i)
  {
    pp[i]=xform4d(world_to_viewport,pp[i]);
    pp[i].w=1.0f/pp[i].w;
    pp[i].x *= pp[i].w;
    pp[i].y *= pp[i].w;
    pp[i].z *= pp[i].w;

    vert_it->set_xyz_rhw(pp[i]);
    vert_it->tc[0] = texture_coord(0,0);
    vert_it->diffuse = col;
    ++vert_it;
  }

  vert_workspace_xformed.unlock();

  // send it to the card
  hw_rasta::inst()->send_indexed_vertex_list( vert_workspace_xformed, num_verts,
    debug_render_indices, num_indices,
    hw_rasta::SEND_VERT_FRONT_CLIP);
#else
  geometry_manager::inst()->set_local_to_world(identity_matrix);

  vert_workspace.lock(8);
  hw_rasta_vert_lit* vert_it = (hw_rasta_vert_lit*)vert_workspace.begin();

  vert_it->xyz = vector3d(center+halfsizex+halfsizey+halfsizez);
  vert_it->tc[0] = texture_coord(0,0);
  vert_it->diffuse = col;
  ++vert_it;
  vert_it->xyz = vector3d(center-halfsizex+halfsizey+halfsizez);
  vert_it->tc[0] = texture_coord(0,0);
  vert_it->diffuse = col;
  ++vert_it;
  vert_it->xyz = vector3d(center+halfsizex-halfsizey+halfsizez);
  vert_it->tc[0] = texture_coord(0,0);
  vert_it->diffuse = col;
  ++vert_it;
  vert_it->xyz = vector3d(center-halfsizex-halfsizey+halfsizez);
  vert_it->tc[0] = texture_coord(0,0);
  vert_it->diffuse = col;
  ++vert_it;
  vert_it->xyz = vector3d(center+halfsizex+halfsizey-halfsizez);
  vert_it->tc[0] = texture_coord(0,0);
  vert_it->diffuse = col;
  ++vert_it;
  vert_it->xyz = vector3d(center-halfsizex+halfsizey-halfsizez);
  vert_it->tc[0] = texture_coord(0,0);
  vert_it->diffuse = col;
  ++vert_it;
  vert_it->xyz = vector3d(center+halfsizex-halfsizey-halfsizez);
  vert_it->tc[0] = texture_coord(0,0);
  vert_it->diffuse = col;
  ++vert_it;
  vert_it->xyz = vector3d(center-halfsizex-halfsizey-halfsizez);
  vert_it->tc[0] = texture_coord(0,0);
  vert_it->diffuse = col;
  ++vert_it;

  vert_workspace.unlock();

  // send it to the card
  hw_rasta::inst()->send_indexed_vertex_list(vert_workspace, num_verts,
                                             debug_render_indices, num_indices);
#endif
}


void render_beam( const vector3d &pt1, const vector3d &pt2, color32 col, rational_t thickness )
{
  // point1 to point2
  vector3d vec = pt2 - pt1;

  // length of beam
  float len = vec.length();

  if (len<1e-9f)
    return;

  vector3d normvec = vec/len;

  camera* curcam = app::inst()->get_game()->get_current_view_camera();

  vector3d linevec = (pt1+pt2)*0.5f-curcam->get_abs_po().get_position();
  //curcam->get_abs_po().get_z_facing();
  vector3d cx = cross(linevec, normvec);
  if (cx.length2()<1e-9f) return; // beam is along camera z axis, I guess we could render it with a small quad
  cx.set_length(thickness*0.5f);

  vector3d p0,p1,p2,p3;

  #if 0

  vector3d cy = normvec * (thickness*0.5f);
  p0 = pt1 - cx - cy;
  p1 = pt1 + cx - cy;
  p2 = pt2 + cx + cy;
  p3 = pt2 - cx + cy;

  #else

  p0 = pt1 - cx;
  p1 = pt1 + cx;
  p2 = pt2 + cx;
  p3 = pt2 - cx;

  #endif

  render_quad(p0,p1,p2,p3, col);
}

void render_beam_box( const vector3d &tl, const vector3d &br, color32 col, rational_t thickness )
{
  render_beam( vector3d(tl.x, tl.y, tl.z), vector3d(br.x, tl.y, tl.z), col, thickness );
  render_beam( vector3d(br.x, tl.y, tl.z), vector3d(br.x, tl.y, br.z), col, thickness );
  render_beam( vector3d(br.x, tl.y, br.z), vector3d(tl.x, tl.y, br.z), col, thickness );
  render_beam( vector3d(tl.x, tl.y, br.z), vector3d(tl.x, tl.y, tl.z), col, thickness );

  render_beam( vector3d(tl.x, br.y, tl.z), vector3d(br.x, br.y, tl.z), col, thickness );
  render_beam( vector3d(br.x, br.y, tl.z), vector3d(br.x, br.y, br.z), col, thickness );
  render_beam( vector3d(br.x, br.y, br.z), vector3d(tl.x, br.y, br.z), col, thickness );
  render_beam( vector3d(tl.x, br.y, br.z), vector3d(tl.x, br.y, tl.z), col, thickness );

  render_beam( vector3d(tl.x, tl.y, tl.z), vector3d(tl.x, br.y, tl.z), col, thickness );
  render_beam( vector3d(br.x, tl.y, tl.z), vector3d(br.x, br.y, tl.z), col, thickness );
  render_beam( vector3d(br.x, tl.y, br.z), vector3d(br.x, br.y, br.z), col, thickness );
  render_beam( vector3d(tl.x, tl.y, br.z), vector3d(tl.x, br.y, br.z), col, thickness );
}

void render_beam_cube( const vector3d &pt, rational_t radius, color32 col, rational_t thickness )
{
  rational_t sqrt3over3 = 0.577350269f; // sqrt(3)/3
  vector3d delta(sqrt3over3, sqrt3over3, sqrt3over3);

  render_beam_box(pt-(delta * radius), pt+(delta * radius), col, thickness);
}


#ifndef BUILD_BOOTABLE

void render_frustum(const camera& cam, const color32& col)
{
  if (!frustum_pmesh) return;
  const po& view2world = cam.get_abs_po();

  // compute basis for camera-oriented XZ space.  we want the basis for the scene camera,
  // which isn't necessarily the current_view camera, if we happen to be using the scene_analyzer
  // cam.
  static rational_t near_adj = PROJ_FAR_PLANE_D/PROJ_NEAR_PLANE_D;
  rational_t tanfovx = tan(PROJ_FOV_ZOOM_HALF)*PROJ_NEAR_PLANE_D;

  vector3d p0 = vector3d(-tanfovx,-tanfovx*PROJ_ASPECT,PROJ_NEAR_PLANE_D);
  vector3d p1 = vector3d(-tanfovx, tanfovx*PROJ_ASPECT,PROJ_NEAR_PLANE_D);
  vector3d p2 = vector3d( tanfovx, tanfovx*PROJ_ASPECT,PROJ_NEAR_PLANE_D);
  vector3d p3 = vector3d( tanfovx,-tanfovx*PROJ_ASPECT,PROJ_NEAR_PLANE_D);
  vector3d p4 = p0*near_adj;
  vector3d p5 = p1*near_adj;
  vector3d p6 = p2*near_adj;
  vector3d p7 = p3*near_adj;

  frustum_pmesh->set_xvert_unxform_pos(0,p0);
  frustum_pmesh->set_xvert_unxform_pos(1,p1);
  frustum_pmesh->set_xvert_unxform_pos(2,p2);
  frustum_pmesh->set_xvert_unxform_pos(3,p3);
  frustum_pmesh->set_xvert_unxform_pos(4,p4);
  frustum_pmesh->set_xvert_unxform_pos(5,p5);
  frustum_pmesh->set_xvert_unxform_pos(6,p6);
  frustum_pmesh->set_xvert_unxform_pos(7,p7);

  instance_render_info firi(frustum_pmesh->get_max_faces(), view2world, 0, NULL, 0,
                            col, FORCE_TRANSLUCENCY );

  frustum_pmesh->render_instance(RENDER_OPAQUE_PORTION
			  | RENDER_TRANSLUCENT_PORTION
			  | RENDER_NO_LIGHTS
			  | RENDER_CLIPPED_FULL_DETAIL, &firi);
}


static void render_cgmesh(const cg_mesh *colgeom, const po &my_po, color32 col)
{
  for (vector<cface>::const_iterator i = colgeom->get_cfaces().begin();
       i != colgeom->get_cfaces().end();
       ++i)
  {
    render_triangle(my_po.slow_xform(colgeom->get_raw_vert((*i).get_vert_ref(0)).get_point()),
                    my_po.slow_xform(colgeom->get_raw_vert((*i).get_vert_ref(1)).get_point()),
                    my_po.slow_xform(colgeom->get_raw_vert((*i).get_vert_ref(2)).get_point()),
                    col);
  }
}


void render_colgeom(const collision_geometry *colgeom, color32 col, const entity *ent)
{
  if(colgeom)
  {
    switch(colgeom->get_type())
    {
      case collision_geometry::CAPSULE:
        render_capsule(((collision_capsule *)colgeom)->get_abs_capsule(), col);
        break;

      case collision_geometry::MESH:
        render_cgmesh((cg_mesh *)colgeom, ent != NULL ? ent->get_abs_po() : po_identity_matrix, col);
        break;
    }
  }
  else if(ent)
    render_sphere(ent->get_abs_position(), ent->get_radius(), col);
}


int FIRST_SHOW_PLANE = 0;
int LAST_SHOW_PLANE = GLOBAL_PMESH_COUNT-1;

#ifndef BUILD_BOOTABLE

static void render_global_pmeshes()
{
  instance_render_info iri(2, po_identity_matrix, 0, NULL, 0, color32(255,255,255,255), 0, 0, FORCE_SKIP_CLIP );
  for (int i=FIRST_SHOW_PLANE; i<=LAST_SHOW_PLANE; ++i)
  {
    global_pmeshes[i]->render_instance(RENDER_OPAQUE_PORTION
            | RENDER_TRANSLUCENT_PORTION
            | RENDER_NO_LIGHTS
            | RENDER_CLIPPED_FULL_DETAIL, &iri);
  }
}
#endif

void render_debug_planes(vector<partition3 *> plane_list, vector<int> hit_dir_list, vector3d p)
{
#ifndef BUILD_BOOTABLE
  // Note:  I like the rendering method in render_plane() better.  --Sean

  // WARNING:  some hyperplane normals may have been temporarily inverted here.
  // find_sector calls will not work, other issues may arise.
  partition3 h;
  vr_pmesh * my_pmesh;
  vector3d pc[4], base_pc[4];
  base_pc[0] = p+5.0f*XVEC+5.0f*ZVEC+3.0f*YVEC;
  base_pc[1] = p+5.0f*XVEC-5.0f*ZVEC+1.5f*YVEC;
  base_pc[2] = p-5.0f*XVEC-5.0f*ZVEC-2.0f*YVEC;
  base_pc[3] = p-5.0f*XVEC+5.0f*ZVEC-0.5f*YVEC;

  for (int j=FIRST_SHOW_PLANE; j<(int)plane_list.size() && j<=LAST_SHOW_PLANE; ++j)
  {
    h = *plane_list[j];
    my_pmesh = global_pmeshes[j];

    pc[0] = h.get_closest_point(base_pc[0]);
    pc[1] = h.get_closest_point(base_pc[1]);
    pc[2] = h.get_closest_point(base_pc[2]);
    pc[3] = h.get_closest_point(base_pc[3]);

    // Draw the nearest corner of the square out to a vert of a poly that belongs to the plane.
    /*    // THIS STOPPED WORKING WHEN WE FLIPPED THE PLANE NORMALS, OF COURSE
    int faceref_idx = h.get_first_faceref();
    int ref = g_world_ptr->get_the_terrain().get_facerefs()[faceref_idx].get_face_ref();
    sector* sec = g_world_ptr->get_the_terrain().find_sector( p );
    if (sec)
    {
      cg_mesh * ter_pmesh = sec->get_region()->get_data()->get_cg_mesh();
      vector3d poly_p = ter_pmesh->get_vert(ter_pmesh->get_cface(ref).get_vert_ref(0)).get_point();

      rational_t d_min = 10000.0;
      int close_idx = -1;
      for (int i=0;i<4;++i)
      {
        rational_t d = norm(pc[i]-poly_p);
        if (d<d_min)
        {
          close_idx = i;
          d_min = d;
        }
      }
      assert(close_idx!=-1);
      pc[close_idx] = poly_p;
    }
    */
    my_pmesh->set_xvert_unxform_pos(0, pc[0]);
    my_pmesh->set_xvert_unxform_pos(1, pc[1]);
    my_pmesh->set_xvert_unxform_pos(2, pc[2]);
    my_pmesh->set_xvert_unxform_pos(3, pc[3]);

    int r = 255-(254/plane_list.size())*j; // 12*(j%20);
    int g = 255-(254/plane_list.size())*j; // 50*(j/20);
    int b = 255-(254/plane_list.size())*j; // 12*(j%20);
    if (r<0) r=0;
    if (g<0) g=0;
    if (b<0) b=0;
    my_pmesh->set_xvert_unxform_diffuse(0,color32(r,g,b,255));
    my_pmesh->set_xvert_unxform_diffuse(1,color32(r,g,b,255));
    my_pmesh->set_xvert_unxform_diffuse(2,color32(r,g,b,255));
    my_pmesh->set_xvert_unxform_diffuse(3,color32(r,g,b,255));

    for (int i=0;i<my_pmesh->get_num_wedges();++i)
    {
      wedge w = my_pmesh->get_wedge(i);
      //          w.prepare(see_my_colmesh->get_verts());
      my_pmesh->set_wedge(i,w);
    }
  }
  render_global_pmeshes();
#endif
}

void clear_planes()
{
#ifndef BUILD_BOOTABLE
  int i;
  vr_pmesh * my_pmesh;
  for (int j=0;j<GLOBAL_PMESH_COUNT; ++j)
  {
    my_pmesh = global_pmeshes[j];

    if(!my_pmesh)
      continue;

    my_pmesh->set_xvert_unxform_pos(0, ZEROVEC);
    my_pmesh->set_xvert_unxform_pos(1, ZEROVEC);
    my_pmesh->set_xvert_unxform_pos(2, ZEROVEC);
    my_pmesh->set_xvert_unxform_pos(3, ZEROVEC);

    for (i=0;i<my_pmesh->get_num_wedges();++i)
    {
      wedge w = my_pmesh->get_wedge(i);
      //          w.prepare(see_my_colmesh->get_verts());
      my_pmesh->set_wedge(i,w);
    }
  }
#endif
}


#define MAX_SPHERES   32
static sphere g_spheres[MAX_SPHERES];
int g_num_spheres = 0;
extern rational_t g_capsule_translucency;

void add_debug_sphere(vector3d pos,rational_t radius)
{
  if (g_num_spheres >= MAX_SPHERES)
    return;

  g_spheres[g_num_spheres]=sphere(pos,radius);
  ++g_num_spheres;
}

void clear_debug_spheres()
{
  g_num_spheres = 0;
}

void render_debug_spheres()
{
  for (int i = 0; i < g_num_spheres; ++i)
  {
    render_sphere(g_spheres[i].get_center(),g_spheres[i].get_radius(),color32(255, 255, 255, 255*g_capsule_translucency ));
  }
}

int hist_count = 0;
int cur_hist = 0;
enum { CAPSULE_HISTORY_DEPTH = 20 };
extern rational_t g_capsule_translucency;

static collision_capsule capsule_history[CAPSULE_HISTORY_DEPTH];

void clear_capsule_history()
{
  cur_hist=0;
  hist_count=0;
}

void add_capsule_history(const collision_capsule& cap)
{
  capsule_history[cur_hist++] = cap;
  ++hist_count;
  if (hist_count>=CAPSULE_HISTORY_DEPTH)
    hist_count = CAPSULE_HISTORY_DEPTH;
  if (cur_hist==CAPSULE_HISTORY_DEPTH) cur_hist = 0;
}

void render_capsule_history()
{
  rational_t xluc_scale = 0.5f;
  for (int i=cur_hist, j = hist_count; j>=0 && hist_count-j<CAPSULE_HISTORY_DEPTH; --i, --j)
  {
    g_capsule_translucency = xluc_scale;
    capsule_history[i].render();
      xluc_scale *= 0.9f;
    if (i==0) i=CAPSULE_HISTORY_DEPTH;
  }
  g_capsule_translucency = 0.5f;
}

rational_t g_render_marker_y_angle = 0.0f;
#define _MAX_RENDER_MARKER_ANGLE_SPEED  180.0f

void frame_advance_markers(time_value_t t)
{
  g_render_marker_y_angle += _MAX_RENDER_MARKER_ANGLE_SPEED * t;
  while(g_render_marker_y_angle >= 360.0f)
    g_render_marker_y_angle -= 360.0f;
}

void render_marker(  camera* camera_link, vector3d pos, color32 col, rational_t scale)
{
  entity *target = g_world_ptr->get_entity("TOOL_MARKER");
  if(target)
  {
    po my_po = po_identity_matrix;
    po tmp1 = po_identity_matrix;

    my_po.set_position(pos);

    tmp1 = po_identity_matrix;
    tmp1.set_scale( vector3d(scale, scale, scale) );
    my_po.add_increment( &tmp1 );

    tmp1 = po_identity_matrix;
    tmp1.set_rotate_y(DEG_TO_RAD(g_render_marker_y_angle));
    my_po.add_increment( &tmp1 );

    target->set_rel_po(my_po);

    rational_t alpha = (rational_t)col.c.a * (1.0f / 255.0f);
    col.c.a = 255;
    target->set_render_color(col);

    target->set_visible(true);
    target->render(camera_link,target->get_max_polys(),RENDER_TRANSLUCENT_PORTION | ((target->get_flags()&EFLAG_MISC_NO_CLIP_NEEDED)?0:RENDER_CLIPPED_FULL_DETAIL), alpha);
    target->set_visible(false);
  }
}


class debug_text
{
public:
  debug_text()
  {
    _3d_text = false;
    str = empty_string;
    col = color32(164, 164, 164, 255);
    pos = ZEROVEC;
    duration = 0.0f;
  }

  void copy(const debug_text &d)
  {
    _3d_text = d._3d_text;
    str = d.str;
    col = d.col;
    pos = d.pos;
    duration = d.duration;
  }

  debug_text(const debug_text &d)
  {
    copy(d);
  }

  debug_text(bool _3d, const vector3d& p, color32 c, time_value_t dur, const stringx &s)
  {
    _3d_text = _3d;
    str = s;
    col = c;
    pos = p;
    duration = dur;
  }

  debug_text& operator=(const debug_text &b)
  {
		copy( b );
    return *this;
  }

  bool _3d_text;
  stringx str;
  color32 col;
  vector3d pos;
  time_value_t duration;
};

#define MAX_DEBUG_TEXT_STRINGS 25

vector<debug_text> debug_strings;

void add_3d_text(const vector3d& pos, color32 col, time_value_t dur, const stringx &str)
{
  debug_strings.reserve(MAX_DEBUG_TEXT_STRINGS);

  vector<debug_text>::iterator i = debug_strings.begin();
  vector<debug_text>::iterator i_end = debug_strings.end();
  while(i != i_end)
  {
    if((*i).duration <= 0.0f)
    {
      (*i)._3d_text = true;
      (*i).str = str;
      (*i).pos = pos;
      (*i).col = col;
      (*i).duration = dur;
      return;
    }

    ++i;
  }

  debug_strings.push_back(debug_text(true, pos, col, dur, str));
}

void add_2d_text(const vector3d& pos, color32 col, time_value_t dur, const stringx &str)
{
  debug_strings.reserve(MAX_DEBUG_TEXT_STRINGS);

  vector<debug_text>::iterator i = debug_strings.begin();
  vector<debug_text>::iterator i_end = debug_strings.end();
  while(i != i_end)
  {
    if((*i).duration <= 0.0f)
    {
      (*i)._3d_text = false;
      (*i).str = str;
      (*i).pos = pos;
      (*i).col = col;
      (*i).duration = dur;
      return;
    }

    ++i;
  }

  debug_strings.push_back(debug_text(false, pos, col, dur, str));
}

void frame_advance_debug_text(time_value_t t)
{
  vector<debug_text>::iterator i = debug_strings.begin();
  vector<debug_text>::iterator i_end = debug_strings.end();
  while(i != i_end)
  {
    if((*i).duration >= 0.0f)
      (*i).duration -= t;
    if((*i).duration < 0.0f)
      (*i).duration = 0.0f;

    ++i;
  }
}

// No longer supported.  (dc 12/17/01)
/*
void render_debug_text()
{
  if( os_developer_options::inst()->is_flagged( os_developer_options::FLAG_SHOW_FPS ) )
  {
    vector<debug_text>::iterator i = debug_strings.begin();
    vector<debug_text>::iterator i_end = debug_strings.end();
    while(i != i_end)
    {
      if((*i).duration > 0.0f)
      {
        if((*i)._3d_text)
        {
          print_3d_text((*i).pos, (*i).col, (*i).str.c_str());
        }
        else
        {
          render_text((*i).str, vector2di((*i).pos.x, (*i).pos.y), (*i).col);
        }
      }

      ++i;
    }
  }
}
*/




#endif // !BUILD_BOOTABLE

// No longer supported.  (dc 12/17/01)
/*
// print out debugging text (should not be used for real interface stuff!)
void render_text(const stringx& str, const vector2di& pos, color32 col, float depth, float size, text_font* fnt)
{
#if defined(TARGET_XBOX)
  g_debug_font->render(str,col,true,pos.x,pos.y,depth,Font::HORIZJUST_CENTER,Font::VERTJUST_CENTER, true,size);
#else
  if (!fnt)
    fnt = g_debug_font;
  fnt->render(str,pos,col,depth,size);
#endif // TARGET_XBOX JIV DEBUG
}
*/

// can only be called from within rendering loop
// this is for debugging purposes only, do not call in the actual released game!


extern bool g_camera_out_of_world;

// Jason wants a parm to allow disabling the visibility check
void print_3d_text(const vector3d& wpos, color32 col, const char *format, ...)
{
//#if defined(TARGET_PC)
  rational_t fontSize = 14;
//#else
//  rational_t fontSize = 18; // TV's make text very blurry, but the horizontal direction is far more important
//#endif

  camera* cam = app::inst()->get_game()->get_current_view_camera();

  vector3d cam_face = cam->get_abs_po().get_facing();
  vector3d cam_pos = cam->get_abs_position();
  // if camera outside the world, disable visibility check
  bool vis_check = g_camera_out_of_world;

  vector3d dir = (wpos - cam_pos);
  if(dir == ZEROVEC)
    return;
  dir.normalize();

  if(dot(cam_face, dir) > 0.0f && (vis_check || visibility_check(cam_pos, wpos, NULL)))
  {
    vector3d pos = xform3d_1(geometry_manager::inst()->xforms[geometry_manager::XFORM_EFFECTIVE_WORLD_TO_VIEW],wpos);

    if(pos.z > 0.0f)
    {
      pos = xform3d_1_homog(geometry_manager::inst()->xforms[geometry_manager::XFORM_VIEW_TO_SCREEN],pos);

      char string[2048] = "";
      va_list args;
      va_start(args, format);
      vsprintf(string, format, args);
      va_end(args);

      char *newline;
      char *strptr = string;

      // would be nice if this rendered the text centered
      while((newline = strchr(strptr, '\n')) != NULL)
      {
//        int len = newline - strptr; // unused -- remove me?
        *newline = '\0';

        hw_rasta::inst()->print(strptr, vector2di(pos.x, pos.y), col);
        pos.y += fontSize;

        strptr = newline+1;
      }

      hw_rasta::inst()->print(strptr, vector2di(pos.x, pos.y), col);
    }
  }
}

