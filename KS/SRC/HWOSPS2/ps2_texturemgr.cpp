/*-------------------------------------------------------------------------------------------------------

  PS2 texture manager implementation.

-------------------------------------------------------------------------------------------------------*/
#include "global.h"

#include "ps2_texturemgr.h"
#include "ps2_rasterize.h"
#include "project.h"

#include "bitplane.h"
#include "debug.h"
#include "osalloc.h"
#include "oserrmsg.h"
#include "osdevopts.h"

#include <libgraph.h>

// need in order to use the vert_workspace as a temp buffer
#include "vertwork.h"
#include "ngl_ps2.h"


//---------------------------------------------------------------------
// Automatically determines hw_texture format, file extensions.
bool hw_texture::load( const stringx & file_name )
{
  return false;
}

bool hw_texture::load_raw_tga(os_file &the_file)
{
  return false;
}


//--------------------------------------------------------------
void hw_texture::unload()
{
}



DEFINE_SINGLETON(hw_texture_mgr)

hw_texture_mgr::hw_texture_mgr()
{
}
//---------------------------------------------------------------------
hw_texture_mgr::~hw_texture_mgr()
{
}

//--------------------------------------------------------------
// finds the texture file, creates a new texture object and tells it to load itself.
hw_texture* hw_texture_mgr::load_texture(const stringx& pathname)
{
  return(NULL);
}


//--------------------------------------------------------------------
hw_texture* hw_texture_mgr::texture_loaded(const stringx& fname)
{
  return(NULL);
}


//--------------------------------------------------------------------
void hw_texture_mgr::unload_all_textures()
{
}

