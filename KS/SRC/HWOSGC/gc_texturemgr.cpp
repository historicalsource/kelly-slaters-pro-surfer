#include "global.h"

#include "gc_texturemgr.h"

bool hw_texture::load( const stringx& file_name )
{
  return false;
}

bool hw_texture::load_raw_tga( os_file& the_file )
{
  return false;
}

void hw_texture::unload( void )
{

}

DEFINE_SINGLETON( hw_texture_mgr )

hw_texture_mgr::hw_texture_mgr( )
{

}

hw_texture_mgr::~hw_texture_mgr( )
{

}

hw_texture* hw_texture_mgr::load_texture( const stringx& pathname )
{
  return NULL;
}

hw_texture* hw_texture_mgr::texture_loaded( const stringx& fname )
{
  return NULL;
}

void hw_texture_mgr::unload_all_textures( void )
{

}

