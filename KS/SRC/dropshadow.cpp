////////////////////////////////////////////////////////////////////////////////

// dropshadow.cpp
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED. 

// This class sucks.  The shadow could be a static pre-built vr_pmesh instead.

////////////////////////////////////////////////////////////////////////////////
#include "global.h"

//!#include "character.h"
#include "dropshadow.h"
#include "vertwork.h"
#include "geomgr.h"
#include "iri.h"
#include "algebra.h"
#include "game.h"

#define ERDMAN_SHADOW // incompatible with ATI Rage Pro chipset on PC

DEFINE_SINGLETON( vr_dropshadow )

#ifdef ERDMAN_SHADOW
enum { nsegments = 9 }; // 12
static float costab[nsegments], sintab[nsegments];
static unsigned short dropshadow_indices[nsegments*3];  
#endif

vr_dropshadow::vr_dropshadow() 
  : visual_rep( VISREP_DROPSHADOW )
{
  my_material = 0;
  for (int i=0; i<nsegments; ++i)
  {
    float rads=i*PI*2.0f/nsegments;

    rational_t sinx, cosx;
    fast_sin_cos_approx( rads, &sinx, &cosx );

    costab[i] = 1.5f*cosx;
    sintab[i] = 1.5f*sinx;
    dropshadow_indices[i*3+0] = 0;
    dropshadow_indices[i*3+1] = i+1;
    dropshadow_indices[i*3+2] = i+2;
  }
  dropshadow_indices[(nsegments-1)*3+2] = 1; // wrap around
}

vr_dropshadow::~vr_dropshadow()
{
}

//--------------------------------------------------------------
void vr_dropshadow::purge()
{
  if( my_material )
    material_bank.delete_instance( my_material );
  my_material = 0;
}
void vr_dropshadow::reload()
{
#ifndef ERDMAN_SHADOW
  my_material = material_bank.new_instance(material( os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\dropshadow" ));
#endif
}
//--------------------------------------------------------------


void vr_dropshadow::render_instance(render_flavor_t flavor,
                                 instance_render_info* iri)
{
#ifdef ERDMAN_SHADOW // Erdman's theory (more polys but less texture, and no texture graininess due to VQ3 compression)

  extern game *g_game_ptr;
  g_game_ptr->get_blank_material()->send_context(0, DIFFUSE, FORCE_TRANSLUCENCY);  

  // make a 12-tri list in object space

  vector3d p[nsegments+1];
  p[0] = vector3d(0, 0, 0);
  for (int i=0; i<nsegments; ++i)
  {
    p[i+1] = vector3d(costab[i], sintab[i], 0);
  }

  static const color32 diffuse_color_0 = color32(0,0,0,240);
  static const color32 diffuse_color = color32(0,0,0,0);
 
 #ifdef TARGET_MKS
  const matrix4x4& m = iri->local_to_world.get_matrix() * geometry_manager::inst()->xforms[geometry_manager::XFORM_WORLD_TO_SCREEN];

  vector4d pp[nsegments+1];
  for(int j=0;j<nsegments+1;++j)
  {
    pp[j] = xform4d(m, vector4d(p[j],1));
    float invw = pp[j].w;
    if (invw)
      invw = 1.0F/invw;
    pp[j].x *= invw;
    pp[j].y *= invw;
    pp[j].z *= invw;
    pp[j].w = invw;
  }

  vert_workspace_xformed.lock(nsegments+1);
  hw_rasta_vert_xformed * vert_it = vert_workspace_xformed.begin();
  hw_rasta_vert_xformed * vert_begin = vert_it;

  vert_it->set_xyz_rhw(pp[0]);
  vert_it->tc[0] = texture_coord(0,0);
  vert_it->diffuse = diffuse_color_0;
  ++vert_it;

  for (int k=1; k<nsegments+1; ++k)
  {
    vert_it->set_xyz_rhw(pp[k]);
    vert_it->tc[0] = texture_coord(0,0);
    vert_it->diffuse = diffuse_color;
    ++vert_it;
  }
  
  vert_workspace_xformed.unlock();
 
  // send it to the card  

  hw_rasta::inst()->send_indexed_vertex_list( vert_workspace_xformed, nsegments+1, 
      dropshadow_indices, nsegments*3, hw_rasta::SEND_VERT_FRONT_CLIP );
 #else
  geometry_manager::inst()->set_local_to_world(iri->local_to_world.get_matrix()); 

  vert_workspace.lock(nsegments+1);
  hw_rasta_vert_lit * vert_it = (hw_rasta_vert_lit*)vert_workspace.begin();
//  hw_rasta_vert_lit * vert_begin = vert_it; // unused -- remove me?

  vert_it->set_xyz(p[0]);
  vert_it->tc[0] = texture_coord(0,0);
  vert_it->diffuse = diffuse_color_0;
  ++vert_it;

  for (int k=1; k<nsegments+1; ++k)
  {
    vert_it->set_xyz(p[k]);
    vert_it->tc[0] = texture_coord(0,0);
    vert_it->diffuse = diffuse_color;
    ++vert_it;
  }
  
  vert_workspace.unlock();
 
  // send it to the card  

  hw_rasta::inst()->send_indexed_vertex_list( vert_workspace, nsegments+1, 
      dropshadow_indices, nsegments*3 );
 #endif
  
#else

  // send it to the card
  my_material->send_context(0);  
 
  static const unsigned short dropshadow_indices[6] = { 0,1,2, 2,1,3 };
  
  // make a 2-tri strip in object space

  vector3d p[4];
  p[0] = vector3d( -1,-1, 0);
  p[1] = vector3d(  1,-1, 0);
  p[2] = vector3d( -1, 1, 0);
  p[3] = vector3d(  1, 1, 0);

  static const color32 diffuse_color = color32(255,255,255,255);
  //static const color32 specular_color = color32(0,0,0,0);
  
 #ifdef TARGET_MKS
  const matrix4x4& m = iri->local_to_world.get_matrix() * geometry_manager::inst()->xforms[geometry_manager::XFORM_WORLD_TO_SCREEN];

  vector4d pp[4];
  for(int j=0;j<4;++j)
  {
    pp[j] = xform4d(m, vector4d(p[j],1));
    float invw = pp[j].w;
    if (invw)
      invw = 1.0F/invw;
    pp[j].x *= invw;
    pp[j].y *= invw;
    pp[j].z *= invw;
    pp[j].w = invw;
  }

  vert_workspace_xformed.lock(4);
  hw_rasta_vert_xformed * vert_it = vert_workspace_xformed.begin();
  hw_rasta_vert_xformed * vert_begin = vert_it;

  vert_it->set_xyz_rhw(pp[0]);
  vert_it->tc[0] = texture_coord(0,1);
  vert_it->diffuse = diffuse_color;
  ++vert_it;
  
  vert_it->set_xyz_rhw(pp[1]);
  vert_it->tc[0] = texture_coord(0,0);
  vert_it->diffuse = diffuse_color;
  ++vert_it;

  vert_it->set_xyz_rhw(pp[2]);
  vert_it->tc[0] = texture_coord(1,1);
  vert_it->diffuse = diffuse_color;
  ++vert_it;

  vert_it->set_xyz_rhw(pp[3]);
  vert_it->tc[0] = texture_coord(1,0);
  vert_it->diffuse = diffuse_color;
  ++vert_it;

  vert_workspace_xformed.unlock();
 
  // we're using a list instead of a strip because we can't clip strips yet
  hw_rasta::inst()->send_indexed_vertex_list( vert_workspace_xformed, 4, 
      dropshadow_indices, 6, hw_rasta::SEND_VERT_FRONT_CLIP );
 #else
  geometry_manager::inst()->set_local_to_world(iri->local_to_world.get_matrix()); 

  vert_workspace.lock(4);
  hw_rasta_vert_lit* vert_it = vert_workspace.begin();
  hw_rasta_vert_lit* vert_begin = vert_it;

  vert_it->set_xyz(p[0]);
  vert_it->tc[0] = texture_coord(0,1);
  vert_it->diffuse = diffuse_color;
  ++vert_it;
  
  vert_it->set_xyz(p[1]);
  vert_it->tc[0] = texture_coord(0,0);
  vert_it->diffuse = diffuse_color;
  ++vert_it;

  vert_it->set_xyz(p[2]);
  vert_it->tc[0] = texture_coord(1,1);
  vert_it->diffuse = diffuse_color;
  ++vert_it;

  vert_it->set_xyz(p[3]);
  vert_it->tc[0] = texture_coord(1,0);
  vert_it->diffuse = diffuse_color;
  ++vert_it;

  vert_workspace.unlock();
 
  hw_rasta::inst()->send_vertex_strip(vert_workspace, 4);
 #endif
  
#endif
}

