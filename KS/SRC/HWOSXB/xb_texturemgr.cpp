/*-------------------------------------------------------------------------------------------------------

  XB texture manager implementation.

-------------------------------------------------------------------------------------------------------*/
#include "global.h"

#include "xb_texturemgr.h"
#include "xb_rasterize.h"
#include "project.h"

#include "bitplane.h"
#include "debug.h"
#include "osalloc.h"
#include "oserrmsg.h"
#include "osdevopts.h"

// need in order to use the vert_workspace as a temp buffer
#include "vertwork.h"

#undef free
#undef malloc
#include "ngl.h"


//---------------------------------------------------------------------
// Automatically determines hw_texture format, file extensions.
bool hw_texture::load( const stringx & file_name )
{
//    STUB( "hw_texture::load" ); JIV FIXME
  
  return true;
}

bool hw_texture::load_raw_tga(os_file &the_file)
{
  STUB( "hw_texture::load_raw_tga" );

  return true;
}

//--------------------------------------------------------------
void hw_texture::unload()
{
  STUB( "hw_texture::unload" );
}



DEFINE_SINGLETON(hw_texture_mgr)

hw_texture_mgr::hw_texture_mgr()
{
}
//---------------------------------------------------------------------
hw_texture_mgr::~hw_texture_mgr()
{
  unload_all_textures();
}

//--------------------------------------------------------------
// finds the texture file, creates a new texture object and tells it to load itself.
hw_texture* hw_texture_mgr::load_texture(const stringx& pathname)
{
  // JIV FIXME
  assert( name_list.size() == texture_list.size() );

  hw_texture *newone = new hw_texture;

  bool err = newone->load(pathname);
  assert( err );

  name_list.push_back(pathname);
  texture_list.push_back(newone);

  return newone;
}


//--------------------------------------------------------------------
hw_texture* hw_texture_mgr::texture_loaded(const stringx& fname)
{
  for( int i = name_list.size() - 1; i >= 0; i-- )
    if(name_list[i] == fname)
      return texture_list[i];
  
  return NULL;
}


//--------------------------------------------------------------------
void hw_texture_mgr::unload_all_textures()
{
  for( int i = texture_list.size() - 1; i >= 0; i-- )
  {
    // derived from ref, don't delete?
    texture_list[i] = 0;
  }
  
  texture_list.resize(0);
  name_list.resize(0);

  // JIV FIXME something else?
  assert( name_list.size() == texture_list.size() );
}

