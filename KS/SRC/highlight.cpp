///////////////////////////////////////////////////////////////////////////////////////////
// stolen from dropshadow
///////////////////////////////////////////////////////////////////////////////////////////

#include "global.h"

#include "highlight.h"
#include "vertwork.h"
#include "geomgr.h"
#include "iri.h"
#include "algebra.h"
#include "osdevopts.h"


#define HIGHLIGHT_TEXTURE_NAME  (os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\lockon")


DEFINE_SINGLETON( highlight )


highlight::highlight() : visual_rep( VISREP_HIGHLIGHT )
{
  my_material = NULL;
  current_spin_angle = .0f;
}


highlight::~highlight()
{
}


void highlight::purge()
{
  if( my_material ) material_bank.delete_instance( my_material );
  my_material = NULL;
}


void highlight::reload()
{
  my_material = material_bank.new_instance( material(HIGHLIGHT_TEXTURE_NAME) );
}


void highlight::render_instance( render_flavor_t flavor, instance_render_info *iri )
{
  // need to make this send object-space verts instead of screen-space!

  vector3d p[4];
  p[0] = vector3d( -1, -1, 0 );
  p[1] = vector3d(  1, -1, 0 );
  p[2] = vector3d( -1,  1, 0 );
  p[3] = vector3d(  1,  1, 0 );

  static const color32 diffuse_color = color32( 255, 255, 255, 255 );
  //static const color32 specular_color = color32( 0, 0, 0, 0 );
  
  my_material->send_context(0);  

//  unsigned short indices[6] = { 0,1,2, 2,1,3 }; // unsigned variable -- remove me?

 #ifdef TARGET_MKS
  matrix4x4 m = iri->local_to_world.get_matrix() * geometry_manager::inst()->xforms[geometry_manager::XFORM_WORLD_TO_SCREEN];

  vector4d pp[4];
  for( int j=0; j < 4; ++j )
  {
    pp[j] = xform4d(m, vector4d(p[j],1));
    float invw = pp[j].w;
    if (invw) invw = 1.0F / invw;
    pp[j].x *= invw;
    pp[j].y *= invw;
    pp[j].z *= invw;
    pp[j].w = invw;
  }

  vert_workspace_xformed_quad.lock(4);
  hw_rasta_vert_xformed *vert_it = vert_workspace_xformed_quad.begin();
  hw_rasta_vert_xformed *vert_begin = vert_it;

  vert_it->reset();
  vert_it->set_xyz_rhw(pp[0]);
  vert_it->tc[0] = texture_coord(0,1);
  vert_it->diffuse = diffuse_color;
  ++vert_it;
  
  vert_it->reset();
  vert_it->set_xyz_rhw(pp[1]);
  vert_it->tc[0] = texture_coord(0,0);
  vert_it->diffuse = diffuse_color;
  ++vert_it;

  vert_it->reset();
  vert_it->set_xyz_rhw(pp[2]);
  vert_it->tc[0] = texture_coord(1,1);
  vert_it->diffuse = diffuse_color;
  ++vert_it;

  vert_it->reset();
  vert_it->set_xyz_rhw(pp[3]);
  vert_it->tc[0] = texture_coord(1,0);
  vert_it->diffuse = diffuse_color;
  ++vert_it;

  vert_workspace_xformed_quad.unlock();

  // we're sending as a list instead of as a strip because it will need clipping and we can't clip strips
  // so it would have to be converted anyway, we may as well just convert it up front
  hw_rasta::inst()->send_indexed_vertex_list(vert_workspace_xformed_quad, 4, indices, 6, hw_rasta::SEND_VERT_FRONT_CLIP );
 #elif defined (TARGET_PC)
  geometry_manager::inst()->set_local_to_world(iri->local_to_world.get_matrix());

  vert_workspace_quad.lock(4);
  hw_rasta_vert_lit *vert_it = vert_workspace_quad.begin();
//  hw_rasta_vert_lit *vert_begin = vert_it; // unused -- remove me?

  vert_it->reset();
  vert_it->set_xyz(p[0]);
  vert_it->tc[0] = texture_coord(0,1);
  vert_it->diffuse = diffuse_color;
  ++vert_it;
  
  vert_it->reset();
  vert_it->set_xyz(p[1]);
  vert_it->tc[0] = texture_coord(0,0);
  vert_it->diffuse = diffuse_color;
  ++vert_it;

  vert_it->reset();
  vert_it->set_xyz(p[2]);
  vert_it->tc[0] = texture_coord(1,1);
  vert_it->diffuse = diffuse_color;
  ++vert_it;

  vert_it->reset();
  vert_it->set_xyz(p[3]);
  vert_it->tc[0] = texture_coord(1,0);
  vert_it->diffuse = diffuse_color;
  ++vert_it;

  vert_workspace_quad.unlock();

  hw_rasta::inst()->send_vertex_strip(vert_workspace_quad, 4);
 #else // ps2
  geometry_manager::inst()->set_local_to_world(iri->local_to_world.get_matrix());

  vert_workspace_quad.lock(4);
  hw_rasta_vert *vert_it = vert_workspace_quad.begin();

  vert_it->reset();
  vert_it->set_xyz(p[0]);
  vert_it->tc[0] = texture_coord(0,1);
  vert_it->diffuse = diffuse_color;
  ++vert_it;
  
  vert_it->reset();
  vert_it->set_xyz(p[1]);
  vert_it->tc[0] = texture_coord(0,0);
  vert_it->diffuse = diffuse_color;
  ++vert_it;

  vert_it->reset();
  vert_it->set_xyz(p[2]);
  vert_it->tc[0] = texture_coord(1,1);
  vert_it->diffuse = diffuse_color;
  ++vert_it;

  vert_it->reset();
  vert_it->set_xyz(p[3]);
  vert_it->tc[0] = texture_coord(1,0);
  vert_it->diffuse = diffuse_color;
  ++vert_it;

  vert_workspace_quad.unlock();

  hw_rasta::inst()->send_vertex_strip(vert_workspace_quad, 4);
 #endif
}


void highlight::add_current_spin_angle_increment( rational_t spin_increment )
{
  current_spin_angle += spin_increment;
  if( current_spin_angle >= 2 * PI ) current_spin_angle -= 2 * PI;
}