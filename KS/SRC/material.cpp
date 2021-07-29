////////////////////////////////////////////////////////////////////////////////

// material.cpp
// Copyright (C) 1999-2000 Treyarch, L.L.C.  ALL RIGHTS RESERVED
// the material a face is made of

////////////////////////////////////////////////////////////////////////////////
#include "global.h"

#include "material.h"

#include "osalloc.h"
#include "oserrmsg.h"
#include "ostimer.h"
#include "osdevopts.h"
#include "osfile.h"
#include "filespec.h"
#include "file_finder.h"
#include "profiler.h"
#include "blendmodes.h"

#include <algorithm>


const chunk_flavor CHUNK_MATERIAL("material");
const chunk_flavor CHUNK_TEXTURE("texture");
const chunk_flavor CHUNK_TEXFLAGS("texflags");
const chunk_flavor CHUNK_TEXINFO("texinfo");
const chunk_flavor CHUNK_DETINFO("detinfo");
const chunk_flavor CHUNK_ENVIRONMENT("enviro");
const chunk_flavor CHUNK_DIFFUSE_COLOR("diffuse");
const chunk_flavor CHUNK_UVANIM("uvanim");
const chunk_flavor CHUNK_FLAGS("flags");
const chunk_flavor CHUNK_NAME("name");

#define texture_exists( n ) texture_filename[ n ].length()

////////////////////////////////////////////////////////////////////////////////
//  material
////////////////////////////////////////////////////////////////////////////////

// globals
instance_bank<material> material_bank;

material*     material::last_context_material = NULL;
hw_texture*   material::last_context_texture  = NULL;
map_e         material::last_context_map      = (map_e)-1;
unsigned      material::last_context_ff       = (unsigned)-1;
color32       material::last_context_color    = color32(0,0,0,0);

void skip_material_chunk( chunk_file& io )
{
  for(;;)
  {
    chunk_flavor cf;
    serial_in( io, &cf );
    if ( CHUNK_TEXTURE == cf )
    {
      stringx filename;
      serial_in( io, &filename );
    }
    else if( CHUNK_TEXFLAGS==cf )
    {
      unsigned flags;
      serial_in( io, &flags );
    }
    else if( CHUNK_TEXINFO==cf )
    {
      unsigned mode;
      serial_in( io, &mode );
      float amt;
      serial_in( io, &amt );
    }
    else if( CHUNK_DETINFO==cf )
    {
      float f;
      serial_in( io, &f );
      serial_in( io, &f );
      serial_in( io, &f );
      serial_in( io, &f );
    }
	  else if(CHUNK_ENVIRONMENT == cf)
    {
		  stringx filename;
		  serial_in(io, &filename);
	  }
    else if ( CHUNK_DIFFUSE_COLOR == cf )
    {
      color c;
      serial_in( io, &c );
    }
    else if ( CHUNK_FLAGS == cf)
    {
      unsigned flags;
      serial_in( io, &flags );
    }
    else if ( CHUNK_END == cf )
    {
      break;
    }
  }
}

static char g_debug_material_fname[32]="spankygod";
static char g_debug_texture_fname[32]="blank";


void serial_in(chunk_file &io, material *m, const stringx &texture_dir, unsigned additional_mat_flags, unsigned additional_tex_flags )
{
	chunk_flavor cf;
	stringx      filename;
  color32      c( 255, 255, 255, 255 );

  int texture_count = 0;

  bool tex_flags_in_material_flag_chunk = true;

  m->set_flags( additional_mat_flags );

  m->set_defaults();
	// read chunks
	while (true)
  {
		serial_in(io, &cf);
	  if(CHUNK_NAME == cf)
    {
			stringx thename;
      serial_in(io, &thename);
			m->material_name=thename;
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
  			filename = texture_dir + filename;

#ifndef TARGET_PS2
				m->diffuse_map[ texture_count ].load(filename);
				m->texture_filename[ texture_count ] = filename;

        // FIXME:  needs to come from exporter
        if(texture_count == MAP_DIFFUSE || texture_count == MAP_DIFFUSE2 )
          m->diffuse_map[texture_count].set_blend_amount( 1.0f );
        else if(texture_count == MAP_ENVIRONMENT )
          m->diffuse_map[texture_count].set_blend_amount( 0.25f );
        else
          m->diffuse_map[texture_count].set_blend_amount( 0.125f );
#endif
			}
      texture_count++;
		}
		else if(CHUNK_ENVIRONMENT == cf)
    {
      // backwardly compatible to the days when ENVIRONMENT was a special case texture
			serial_in(io, &filename);
			filename = texture_dir + filename;
			if(!filename.empty())
      {
				m->texture_filename[ MAP_ENVIRONMENT ] = filename;

#ifndef TARGET_PS2
				m->diffuse_map[ MAP_ENVIRONMENT ].load(filename);

        // FIXME:  needs to come from exporter
        m->diffuse_map[MAP_ENVIRONMENT].set_blend_amount( 0.125f );
#endif
      }
		}
    else if(CHUNK_TEXFLAGS == cf)
    {
      tex_flags_in_material_flag_chunk = false;
      int flags;
      serial_in(io, &flags );

      // it's a rule we told the artists:  you can have material color OR vertex color:
      if( ! (flags & TEX_NO_VERTEX_COLOR ) )
        flags |= TEX_NO_MATERIAL_COLOR;

      m->diffuse_map[ (texture_count-1) ].set_flags( flags );
    }
    else if(CHUNK_TEXINFO == cf )
    {
      int mode;
      serial_in(io, &mode );
//#define WARN_BAD_TEXINFO
#if (defined _DEBUG) && (defined WARN_BAD_TEXINFO)
      if( mode >= NGLBM_MAX_BLEND_MODES )
        warning( "blend mode out of range (%d) for '%s'!", mode, io.get_filename( ).c_str( ) );
#endif
      m->diffuse_map[ (texture_count-1) ].set_blend_mode( mode );
      float amount;
      serial_in(io, &amount );
#if (defined _DEBUG) && (defined WARN_BAD_TEXINFO)
      if( amount < 0.0f || amount > 100.0f )
        warning( "blend amount out of range (%f) for '%s'!", amount, io.get_filename( ).c_str( ) );
#endif
      m->diffuse_map[ (texture_count-1) ].set_blend_amount( amount );
    }
    else if( CHUNK_DETINFO == cf )
    {
      serial_in( io, &m->det_u_scale );
      serial_in( io, &m->det_v_scale );
      serial_in( io, &m->det_range );
      serial_in( io, &m->det_alpha_clamp );
    }
		else if( CHUNK_DIFFUSE_COLOR == cf )
    {
      color c_in;
			serial_in( io, &c_in );
      c = c_in.to_color32();
		}
    else if(CHUNK_UVANIM == cf)
    {
			serial_in(io, &m->u_anim);
			serial_in(io, &m->v_anim);
    }
		else if(CHUNK_FLAGS == cf)
    {
      // FIXME:  CHUNK_FLAGS is a legacy from when material and texture flags were the same
      unsigned flags;
			serial_in(io, &flags);
      additional_mat_flags |= flags;
      m->set_flags( additional_mat_flags );
      additional_tex_flags |= flags;
      if( tex_flags_in_material_flag_chunk )
      {
        for( int i=0; i<MAPS_PER_MATERIAL; i++ )
        {
          m->diffuse_map[i].set_flags( m->diffuse_map[i].get_flags() | additional_tex_flags );
        }
      }
      // environment?
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

  // set the base material to the diffuse color we read
  m->diffuse_map[0].set_diffuse_color( c );

  // FIXME:  this should be set in the exporter
  m->diffuse_map[0].process_vertex_contexts( m->mat_flags );

  for(int i=1; i<MAPS_PER_MATERIAL; ++i)
  {
    if( m->has_diffuse_map(i))
      m->diffuse_map[i].process_vertex_contexts( m->mat_flags );
  }
}

material::material(const material &b)
{
  assert( sizeof(material)==224 );  // if this assertion fires, material has changed and copy
                                    // constructor needs maintenance( I wish we could do this in compile pass )
  int i;
  for( i=0;i<MAPS_PER_MATERIAL;i++ )
  {
    texture_filename[i] = b.texture_filename[i];
    diffuse_map[i] = b.diffuse_map[i];
  }
	mat_flags = b.mat_flags;
  material_name = b.material_name;
  u_anim = b.u_anim;
  v_anim = b.v_anim;

  det_u_scale = b.det_u_scale;
  det_v_scale = b.det_v_scale;
  det_range   = b.det_range;
  det_alpha_clamp = b.det_alpha_clamp;

  for(i=0; i<MAPS_PER_MATERIAL; ++i)
  {
    if(texture_exists(i))
      diffuse_map[i].process_vertex_contexts( mat_flags );
  }
}

material::material( chunk_file& io, const stringx& texture_dir, unsigned additional_mat_flags, unsigned additional_tex_flags )
{
  set_defaults();
  serial_in( io, this, texture_dir, additional_mat_flags, additional_tex_flags );
  build_hash_name();
}

material::material(const stringx &filename, unsigned additional_mat_flags, unsigned additional_tex_flags )
{
  set_defaults();
  texture_filename[0] = filename;

  // texture_filename[1] already initialized to null, same for diffuse_map[1]
#ifndef TARGET_PS2
  diffuse_map[0].load(texture_filename[0]);
  set_flags(get_flags() | additional_mat_flags);
  diffuse_map[0].set_flags( diffuse_map[0].get_flags() | additional_tex_flags );
  diffuse_map[0].process_vertex_contexts( mat_flags );
#endif

  build_hash_name();
}

material::~material()
{
}


void material::set_defaults()
{
  for(int i=0;i<MAPS_PER_MATERIAL;i++)
  {
    texture_filename[i] = stringx();
  }
  mat_flags = 0;
  u_anim = 0.0f;
  v_anim = 0.0f;
}


// WARNING : This function must be maintained if NEW fields are added to material!
void material::build_hash_name()
{
  filespec fspec( texture_filename[0] );
  material_name = fspec.name;
  int i;
  for( i=1;i<MAPS_PER_MATERIAL;i++ )
  {
    material_name += diffuse_map[i].get_filename();
  }

  // Build hash string to append to name.  All numerical values mashed into 8 char string.
  char hasher_string[9] = "12345678";
  float float_hash;
  unsigned int hash;
  float_hash = 2 * mat_flags +
               3 * u_anim +
               5 * v_anim +
               7 * diffuse_map[0].get_blend_mode();
  // bash it to an unsigned int
  hash = *((unsigned int*) &float_hash);

  unsigned int mask = 0x0000000F;
  int shifter = 0;
  for (i=0;i<8;++i)
  {
    hasher_string[i] = 'A' + ((hash&mask)>>shifter);
    shifter+=4;
    mask<<=4;
  }

  // And slap it on
  material_name += stringx(hasher_string);
}


bool material::is_translucent() const
{
  if( diffuse_map[0].is_translucent() )
    return true;

  for(int i=1; i<MAPS_PER_MATERIAL; ++i)
  {
    if( texture_exists(i) && diffuse_map[i].is_translucent())
      return true;
  }

  return false;
}

//extern profiler_timer proftimer_send_context;
//extern profiler_timer proftimer_process_context;

void material::send_context(int frame, map_e map, unsigned int force_flags, color32 force_color )
{
#ifndef TARGET_PS2
  hw_texture* tex = get_texture(frame,map);
  if (last_context_material == this &&
      last_context_map == map &&
      last_context_ff == force_flags &&
      last_context_texture == tex &&
      last_context_color == force_color )
    return;

  last_context_material = this;
  last_context_map = map;
  last_context_ff = force_flags;
  last_context_texture = tex;
  last_context_color = force_color;

  //proftimer_send_context.start();
  diffuse_map[ map ].send_context( frame, force_flags, force_color );
  if( texture_exists(map ) )
    diffuse_map[ map ].send_texture( frame, 0 );
  else
    hw_rasta::inst()->send_texture( hw_texture_mgr::inst()->get_white_texture() );
#endif
}

hw_texture* material::get_texture( int frame, map_e map ) const
{
  if( texture_exists(map ) )
    return diffuse_map[ map ].get_texture( frame );
  return NULL;
}

int material::get_anim_length( int map ) const
{
  return diffuse_map[map].get_anim_length();
}

void material::flush_last_context()
{
  last_context_material = NULL;
}

unsigned material::get_flags() const
{
  return mat_flags;
}

void material::set_flags(unsigned _f)
{
  mat_flags = _f;
//  diffuse_map[0].set_flags(_f);  KILLME.
}

void material::process_vertex_contexts()
{
  //proftimer_process_context.start();
  diffuse_map[0].process_vertex_contexts( mat_flags );

  for(int i=1; i<MAPS_PER_MATERIAL; ++i)
  {
    if( texture_exists(i))
      diffuse_map[i].process_vertex_contexts( mat_flags );
  }

  //proftimer_process_context.stop();
}

void material::set_diffuse_color( color32 c, int m )
{
  diffuse_map[ m ].set_diffuse_color( c );
}

/*
color_and_mat_flag material::get_color_and_mat_flag( int m )
{
  bool mat_flag = (diffuse_map[ m ].get_flags() & TEX_NO_MATERIAL_COLOR)?false:true;
  color_and_mat_flag retval( get_diffuse_color( m ), mat_flag );
  return retval;
}

void material::set_color_and_mat_flag( const color_and_mat_flag& caf, int m )
{
  bool mat_flag = (diffuse_map[ m ].get_flags() & TEX_NO_MATERIAL_COLOR)?false:true;
  if(( caf.c != get_diffuse_color( m ) )||( caf.mat_flag != mat_flag ))
  {
    if( caf.mat_flag )
      diffuse_map[ m ].set_flags( diffuse_map[ m ].get_flags() & (0xffff^TEX_NO_MATERIAL_COLOR) );
    else
      diffuse_map[ m ].set_flags( diffuse_map[ m ].get_flags() | TEX_NO_MATERIAL_COLOR );
    set_diffuse_color( caf.c, m );
    diffuse_map[ m ].process_vertex_contexts( mat_flags );
  }
}
*/
////////////////////////////////////////////////////////////////////////////////
//  anim_texture
////////////////////////////////////////////////////////////////////////////////

anim_texture::~anim_texture()
{
  frame_list.resize(0);
  vc_list.resize(0);
}

void anim_texture::process_ifl(const stringx &found_name)
{
	chunk_file fs;
  fs.open(found_name, os_file::FILE_READ|chunk_file::FILE_TEXT);
	filespec iflspec(found_name);

	while (!fs.at_eof())
  {
		stringx frame_name;
		serial_in(fs, &frame_name);
		if (frame_name.length())
    {
			filespec framespec(frame_name);
      hw_texture* txid = hw_texture_mgr::inst()->texture_loaded(iflspec.path + framespec.name);
      if (txid!=NULL)
        frame_list.push_back(txid);
    	else
		  	frame_list.push_back(hw_texture_mgr::inst()->load_texture(iflspec.path + framespec.name));
    }
	}

  fs.close();
}

void anim_texture::load(const stringx & pathname)
{
	assert(!pathname.empty());

	if (pathname.find(".") != (int)string::npos)
		error(("Invalid filename (don't specify extension): " + pathname).c_str());

	filename = pathname;

  if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_STASH_ONLY))
  {
    filespec pathspec(pathname);
    stringx found_name;

    // cheap hack to fix PVR loads from correct paths
#ifndef TARGET_PS2
	  if (g_file_finder->exists(pathname, ".pvr", &found_name))
    {
	    chunk_file fs;
		  filespec spec(found_name);
      hw_texture* txid = hw_texture_mgr::inst()->texture_loaded(spec.path + spec.name);
      if (txid!=NULL)
        frame_list.push_back(txid);
      else
		    frame_list.push_back(hw_texture_mgr::inst()->load_texture(spec.path + spec.name));
	  }
    else if (g_file_finder->exists(pathname, ".ifl", &found_name))
    {
		  // animating texture found, read in frame list.
      process_ifl(found_name);
	  }
	  else
#endif
    {
      found_name = pathname;
      if( os_developer_options::inst()->is_flagged( os_developer_options::FLAG_USE_TGAS) )
      {
        stringx fname;
        if(!os_file::file_exists(fname = found_name+".tga"))
        {
          found_name = os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR)+"\\"+pathspec.name;
          filename = found_name;
        }
      }

      hw_texture* txid = hw_texture_mgr::inst()->texture_loaded(found_name);
      if (txid!=NULL)
        frame_list.push_back(txid);
      else
  	    frame_list.push_back(hw_texture_mgr::inst()->load_texture(found_name));
    }
  }
  else // stash-file only way
  {
    filespec spec(pathname);
    stringx found_name(spec.name+".ifl");
    if (stash::file_exists(found_name))
    {
		  // animating texture found, read in frame list.
      process_ifl(found_name);
	  }
	  else
    {
      // just load the tga
      found_name = spec.name + ".tga";
//      if (stash::file_exists(found_name) == false)
//        debug_print("Couldn't find " + found_name);

      hw_texture* txid = hw_texture_mgr::inst()->texture_loaded(found_name);
      if (txid!=NULL)
        frame_list.push_back(txid);
      else
  	    frame_list.push_back(hw_texture_mgr::inst()->load_texture(found_name));
    }
    filename = found_name;
  }
  if( (*(frame_list.end()-1))->is_translucent() )
  {
    blend_mode = NGLBM_BLEND;  // otherwise leave it opaque
    // if an official blend mode has been set in the archmaterial it will be overrided later
  }
}

hw_texture* anim_texture::get_texture(int frame) const
{
  frame %= frame_list.size();
  return frame_list[frame];
}

bool anim_texture::is_translucent() const
{
  if (((tex_flags&TEX_PUNCH_THROUGH)!=0)||(get_blend_mode()==NGLBM_PUNCHTHROUGH)||(get_blend_mode()==NGLBM_OPAQUE))
    return false; // it's see-through but not rendered like the other translucent polys
  return frame_list[0]->is_translucent() || ((tex_flags&TEX_ADDITIVE_ALPHA)||(get_blend_mode()==NGLBM_ADDITIVE));
}


int anim_texture::get_anim_length() const
{
  return frame_list.size();
}


void anim_texture::process_vertex_contexts( unsigned mat_flags )
{
  // this creates and processes the vertex contexts for each frame
  hw_texture* new_texture;
  vc_list.resize( frame_list.size() );
  for (int i=frame_list.size(); --i>=0; )
  {
    new_texture = get_texture(i);
    vc_list[i] = vertex_context();
    vc_list[i].set_translucent( true );//is_translucent());
//!    vc_list[i].set_texture(new_texture);
		if((blend_mode == NGLBM_ADDITIVE ) || (tex_flags & TEX_ADDITIVE_ALPHA))  // NEW and old formats supported
	  {
      // this isn't a strict additive mode, but because of legacy
      // concepts of additive blending and alpha usage, we do this
      vc_list[i].set_src_alpha_mode(vertex_context::SRCALPHA);
      vc_list[i].set_dst_alpha_mode(vertex_context::ONE);
			vc_list[i].set_fog(false);  // fog messes with additive alpha
	  }
		else
	  {
      if(( blend_mode == NGLBM_PUNCHTHROUGH )||( tex_flags & TEX_PUNCH_THROUGH))  // NEW and old formats
        vc_list[i].set_punchthrough( true );
      else
        vc_list[i].set_punchthrough( false );
      if( blend_mode == NGLBM_OPAQUE )
  			vc_list[i].set_dst_alpha_mode(vertex_context::ZERO);
      else
			  vc_list[i].set_dst_alpha_mode(vertex_context::INVSRCALPHA);
	    vc_list[i].set_fog(true);
	    if (!is_translucent())
	      if (!(mat_flags & MAT_FULL_SELF_ILLUM))
					if (mat_flags & MAT_ALLOW_ADDITIVE_LIGHT)
						vc_list[i].set_specular(true);
	  }
	  if (mat_flags & MAT_NO_FOG)
	  {
	    vc_list[i].set_fog(false);
	  }
    vc_list[i].set_tex_clamp_mode(vertex_context::clamp_mode_t(
      ((tex_flags&TEX_CLAMP_U) ? vertex_context::CLAMP_U : 0) |
      ((tex_flags&TEX_CLAMP_V) ? vertex_context::CLAMP_V : 0)
      ));

		if( !( mat_flags & MAT_BACKFACE_DEFAULT ) ) {

			if( mat_flags & MAT_NO_BACKFACE_CULL ) {
				vc_list[i].set_cull_mode( vertex_context::CNONE );
			}

		}

#ifdef TARGET_PC
    vc_list[i].set_diffuse( diffuse_color );

    if( tex_flags & TEX_NO_VERTEX_COLOR ) {
      vc_list[i].set_vertex_colored( false );
    } else {
      vc_list[i].set_vertex_colored( true );
    }
    if( tex_flags & TEX_NO_MATERIAL_COLOR ) {
      vc_list[i].set_material_colored( false );
    } else {
      vc_list[i].set_material_colored( true );
    }
#endif
  }
}

void anim_texture::send_context( int frame, unsigned force_flags, color32 force_color )
{
  if (frame_list.empty())
    return;
  frame %= frame_list.size();
  hw_rasta::inst()->send_context( vc_list[frame], force_flags, force_color );
}

void anim_texture::send_texture( int frame, int stage )
{
  assert( stage>=0 );
  assert( stage<MAX_TEXTURE_COORDS );
  hw_rasta::inst()->send_texture( frame_list[frame], stage );
}

