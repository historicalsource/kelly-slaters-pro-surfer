#include "global.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
mat_fac

  a platform independent facade to the material class that lets you load ngl materials from text file
  chunks ArchEngine style
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "global.h"

#include "aggvertbuf.h"
#include "material.h"
#include "osdevopts.h"
#include "game.h"
#include "wds.h" // unfortunately.... sigh

#include "matfac.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
static char g_debug_material_fname[32]="spankygod";
static char g_debug_texture_fname[32]="blank";

////////////////////////////////////////////////////////////////////////////////////////////////////
mat_fac::mat_fac()
{
#ifdef NGL

  // the rest wasn't neccessary, as 99% of nglMaterial is not valid without an associated flag. -WTB
  // this is just for sanity, to eliminate dangling pointers/variables etc.
  memset( &m_nglmat, 0, sizeof(nglMaterial) );
#if NGL > 0x010700
  m_nglmat.Flags = NGLMAT_BILINEAR_FILTER | NGLMAT_ALPHA;
#else
  m_nglmat.Flags = NGLMAT_BILINEAR_FILTER | NGLMAT_PERSPECTIVE_CORRECT | NGLMAT_ALPHA;
#endif
	// particle stuff seem to assume this default blending mode
	m_nglmat.MapBlendMode = NGLBM_BLEND;
	m_nglmat.LightMapBlendMode = NGLBM_BLEND;
	m_nglmat.DetailMapBlendMode = NGLBM_BLEND;
	m_nglmat.EnvironmentMapBlendMode = NGLBM_BLEND;

#else
  m_pcmat = NULL;
#endif
}

mat_fac::mat_fac(const mat_fac& b)
{
#ifndef NGL
  m_pcmat = NULL;
#endif

  *this = b;
}

mat_fac::~mat_fac()
{
#ifdef NGL
  if ( m_nglmat.Map )
    nglReleaseTexture( m_nglmat.Map );
#else
  if(m_pcmat != NULL)
  {
    material_bank.delete_instance( m_pcmat );
    m_pcmat = NULL;
  }
#endif
}

mat_fac& mat_fac::operator=(const mat_fac& b)
{
#ifdef NGL
  memcpy(&m_nglmat, &b.m_nglmat, sizeof(m_nglmat));
  if ( m_nglmat.Map )
    nglAddTextureRef( m_nglmat.Map );
#else
  if(m_pcmat != NULL)
    material_bank.delete_instance( m_pcmat );
  
  m_pcmat = material_bank.new_instance( b.m_pcmat );
#endif

  return(*this);
}

void mat_fac::load_material(const stringx &file)
{
  filespec pather( file );
  if(pather.path.empty())
    pather.path = os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR)+"\\";

#ifdef NGL
  if ( m_nglmat.Map )
    nglReleaseTexture( m_nglmat.Map );

  nglSetTexturePath( (char*)pather.path.c_str() );
  m_nglmat.Map = nglLoadTextureA( (char*)pather.name.c_str() );
  m_nglmat.Flags |= NGLMAT_TEXTURE_MAP;
  m_nglmat.MapBlendMode = NGLBM_PUNCHTHROUGH;
  m_nglmat.MapBlendModeConstant = 255;
#else
  if(m_pcmat != NULL)
    material_bank.delete_instance( m_pcmat );
  
  m_pcmat = material_bank.new_instance(pather.path+pather.name );
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void serial_in( chunk_file& io, mat_fac* m, const stringx& texture_dir, unsigned additional_flags )
{
#ifdef NGL
  chunk_flavor cf;
  stringx      filename;
  int texture_count = 0;
  unsigned mat_flags;

  // read chunks
  while (true) 
  {
    serial_in(io, &cf);
    if(CHUNK_NAME == cf)
    {
      stringx thename;
      serial_in(io, &thename);
      if (thename == g_debug_material_fname)
        debug_print("loading material %s",thename.c_str());
    }
    else if(CHUNK_TEXTURE == cf)
    {
      serial_in(io, &filename);
      if(!filename.empty())
      {
        if( stricmp( filename.c_str(), g_debug_texture_fname )==0 )
        {
          debug_print("loading material texture %s",filename.c_str() );
        }
        // go up one directory

        nglSetTexturePath( (char*)texture_dir.c_str() );
        switch( texture_count )
        {
        case MAP_DIFFUSE:
//#pragma fixme("Once nglLoadTextureA takes a const char*")
          m->m_nglmat.Map = nglLoadTextureA( (char*)filename.c_str() );
          m->m_nglmat.Flags |= NGLMAT_TEXTURE_MAP;
          break;
        case MAP_ENVIRONMENT:
          m->m_nglmat.EnvironmentMap = nglLoadTextureA( (char*)filename.c_str() );
          m->m_nglmat.Flags |= NGLMAT_ENVIRONMENT_MAP;
          break;
        case MAP_DIFFUSE2:
          m->m_nglmat.LightMap = nglLoadTextureA( (char*)filename.c_str() );
          m->m_nglmat.Flags |= NGLMAT_LIGHT_MAP;
          break;
        case MAP_DETAIL:
          m->m_nglmat.DetailMap = nglLoadTextureA( (char*)filename.c_str() );
          m->m_nglmat.Flags |= NGLMAT_DETAIL_MAP;
          break;
        }
      }
      texture_count++;
    }
    else if(CHUNK_ENVIRONMENT == cf)
    {
      // a little backwards compatibility
      serial_in(io, &filename);
    }
    else if(CHUNK_TEXFLAGS == cf)
    {
      // a little backwards compatibility
      int flags;
      serial_in(io, &flags );
    }
    else if(CHUNK_TEXINFO == cf )
    {
      int mode;
      float amount;
      serial_in(io, &mode );
      serial_in(io, &amount );
      if(( mode != NGLBM_OPAQUE )&&( mode != NGLBM_PUNCHTHROUGH ))
      {
        m->m_nglmat.Flags |= NGLMAT_ALPHA;
      }
      switch( (texture_count-1) )
      {
      case MAP_DIFFUSE:
        m->m_nglmat.MapBlendMode = mode;
        m->m_nglmat.MapBlendModeConstant = (int)(amount*255);
        break;
      case MAP_DIFFUSE2:
        m->m_nglmat.LightMapBlendMode = mode;
        m->m_nglmat.LightMapBlendModeConstant = (int)(amount*255);
        break;
      case MAP_DETAIL:
        m->m_nglmat.DetailMapBlendMode = mode;
        m->m_nglmat.DetailMapBlendModeConstant = (int)(amount*255);
        break;
      case MAP_ENVIRONMENT:
        m->m_nglmat.EnvironmentMapBlendMode = mode;
        m->m_nglmat.EnvironmentMapBlendModeConstant = (int)(amount*255);
        break;
      }
    }
    else if( CHUNK_DETINFO == cf )
    {
      serial_in( io, &m->m_nglmat.DetailMapUScale );
      serial_in( io, &m->m_nglmat.DetailMapVScale );
      serial_in( io, &m->m_nglmat.DetailMapRange );
      serial_in( io, &m->m_nglmat.DetailMapAlphaClamp );
    }
    else if( CHUNK_DIFFUSE_COLOR == cf )
    {
      color c_in;
      serial_in( io, &c_in );
#ifndef NGL_XBOX	// Xbox process this at meshcvt time thus doesn't allow modification to material color
      c_in /= 2;
      m->m_nglmat.Color = c_in.to_color32().to_ulong();
#endif
    }
    else if(CHUNK_UVANIM == cf)
    {
      int ignored;
      serial_in(io, &ignored );
      serial_in(io, &ignored );
    }
    else if(CHUNK_FLAGS == cf)
    {
      // for backwards compatibility with James old stuff
      serial_in(io, &mat_flags);
    }
    else if(CHUNK_END == cf)
    {
      break;
    }
    else 
    {
      error("Unknown chunk type \'%s\' in %s",cf.c_str(),io.get_filename().c_str() );
    }
  }

  if( mat_flags & TEX_ADDITIVE_ALPHA )
  {
    // for backwards compatibility
    m->m_nglmat.MapBlendMode = NGLBM_ADDITIVE;
  }
#else
  if(m->m_pcmat != NULL)
    material_bank.delete_instance( m->m_pcmat );
  
  m->m_pcmat = material_bank.new_instance(material( io, texture_dir, 0, 0 ));
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int mat_fac::get_anim_length() const
{
#ifdef NGL
  return 1;
#else
  return m_pcmat ? m_pcmat->get_anim_length() : 1;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool mat_fac::is_translucent() const
{
#ifdef NGL
  return(( m_nglmat.MapBlendMode != NGLBM_OPAQUE ) && ( m_nglmat.MapBlendMode != NGLBM_PUNCHTHROUGH ));
#else
  return m_pcmat ? m_pcmat->is_translucent() : false;
#endif
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void mat_fac::send_context( int frame,
                            map_e map,
                            unsigned force_flags,
                            color32 force_color )
{
#ifdef NGL
#else
  if(m_pcmat)
    m_pcmat->send_context( frame, map, force_flags, force_color );
  else
    g_game_ptr->get_blank_material()->send_context( frame, map, force_flags, force_color );
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool mat_fac::has_texture( void ) const
{
#ifdef NGL
  return (m_nglmat.Map ? true: false );
#else
  return true;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int mat_fac::get_original_width( int frame, map_e map ) const
{
#ifdef NGL
  if ( m_nglmat.Map )
    return m_nglmat.Map->Width; 
  else
    return 0;
#else
  hw_texture* tex = get_texture( frame, map );
  return tex->get_original_width();
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int mat_fac::get_original_height( int frame, map_e map ) const
{
#ifdef NGL
  if ( m_nglmat.Map )
    return m_nglmat.Map->Height; 
  else
    return 0;
#else
  hw_texture* tex = get_texture( frame, map );
  return tex->get_original_height();
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned mat_fac::get_blend_mode( int m ) const
{
#ifdef NGL
  return m_nglmat.MapBlendMode;
#else
  return m_pcmat->get_blend_mode();
#endif
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void mat_fac::set_blend_mode( unsigned _mode, int m )
{
#ifdef NGL
  m_nglmat.MapBlendMode = _mode;
  if(( _mode == NGLBM_OPAQUE )||( _mode == NGLBM_PUNCHTHROUGH ))
    m_nglmat.Flags &= (0xffffffff^NGLMAT_ALPHA);
  else
    m_nglmat.Flags |= NGLMAT_ALPHA;
#else
  m_pcmat->set_blend_mode( _mode );
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// non NGL only functiosn
#ifndef NGL
extern world_dynamics_system * g_world_ptr;
////////////////////////////////////////////////////////////////////////////////////////////////////
aggregate_vert_buf* mat_fac::find_mat_buf( int last_frame, unsigned force_flags )
{
  aggregate_vert_buf* matbuf = g_world_ptr->get_matvertbufs().find( m_pcmat, last_frame, force_flags );
  return matbuf;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
hw_texture* mat_fac::get_texture( int frame, map_e map ) const
{
  return m_pcmat->get_texture( frame, map ); 
}
#endif
