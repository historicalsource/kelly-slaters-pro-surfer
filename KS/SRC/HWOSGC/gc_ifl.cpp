#include "gc_ifl.h"

int g_gc_ifl_lock_texture;

bool gc_ifl::init(char *ifl_name)
{
  assert( !initialized );

  g_gc_ifl_lock_texture = -1;
  
  plain_ifl = NULL;
  mram_tex[0] = NULL;
  mram_tex[0] = NULL;
  num_frames = 0;
  frame_offset = NULL;
  common_size = -1;

  if( !stash::using_stash() )
  {
    plain_ifl = nglLoadTexture( ifl_name );
    if( plain_ifl != NULL )
      initialized = true;
		else
			return false;
    mram_tex[0] = plain_ifl->Frames[0];
    mram_tex[1] = plain_ifl->Frames[1];
    num_frames = plain_ifl->NFrames;
    active_tex = 0;
    current_frame = 0;
    return true;
  }
 
  nglFileBuf filebuf;
  filespec spec( ifl_name );
  spec.ext = ".ifl";
  stringx name( spec.name + spec.ext );
  bool b = KSReadFile( name.c_str(), &filebuf, 0 );
  if( !b ) return false;

  uint32 *offsets = (uint32 *) malloc( sizeof( uint32 ) * 1024 );
  
 // handy parser macro
#define endofbuf( a ) ( (a) - (char*)filebuf.Buf >= (int)filebuf.Size )

  char* Pos = (char*)filebuf.Buf;
  while ( !endofbuf( Pos ) )
  {
    // parse out the filename (no extension).
    char* Line = Pos;
    char FileName[256]; //yuck
    while ( !endofbuf( Line ) && !isspace( *Line ) ) Line++;
    u_int Count = mini( Line - Pos, 255 );
    if ( !Count ) break;
    strncpy( FileName, Pos, Count );
    FileName[Count] = '\0';
    char* Ext = strchr( FileName, '.' );
    if ( Ext ) *Ext = '\0';

    // Process Texture
    {
      pstring pname( stringx( FileName ) + stringx( ".gct" ) );
      int size;
      stash_index_entry *anim_header = NULL;
      stash gct_file;
      unsigned char *ofs;
      bool b = gct_file.get_memory_image( pname, ofs, size, anim_header );
      
      if( !b )
      {
        nglPrintf( "Could not find gct: %s\n", pname.c_str() );
      }
      
      assert( b );
      assert( anim_header->in_aram() );
      offsets[ num_frames ] = (unsigned int) ofs;

      if( common_size == -1 )
        common_size = size;
      else
        assert( common_size == size );
      
      if( aram_id == aram_id_t( INVALID_ARAM_ID ) )
        aram_id = gct_file.get_current_aram_id();
      else
        assert( aram_id == gct_file.get_current_aram_id() );
    }
    
    num_frames++;
    assert( num_frames <= 1024 );
    
    // skip any whitespace
    Pos = Line;
    while ( !endofbuf( Pos ) && isspace( *Pos ) ) Pos++;
  }

#undef endofbuf

  frame_offset = (uint32 *) malloc( sizeof( uint32 ) * num_frames );
  memcpy( frame_offset, offsets, num_frames * sizeof( uint32 ) );
  free( offsets );
  KSReleaseFile( &filebuf );

  // Finished getting all the ifl info
  // Load first two textures;
  mram_tex_buf[0] = (unsigned char *) malloc( common_size );
  mram_tex_buf[1] = (unsigned char *) malloc( common_size );
  
  aram_mgr::aram_read( aram_id, frame_offset[0], mram_tex_buf[0], common_size );
  aram_mgr::aram_read( aram_id, frame_offset[1], mram_tex_buf[1], common_size );
  stringx fake_tex_name;
  fake_tex_name = spec.name + "1_gc_ifl";
  mram_tex[0] = nglLoadTextureInPlace( nglFixedString( fake_tex_name.c_str() ), NGLTEX_GCT, mram_tex_buf[0], common_size );
  fake_tex_name = spec.name + "2_gc_ifl";
  mram_tex[1] = nglLoadTextureInPlace( nglFixedString( fake_tex_name.c_str() ), NGLTEX_GCT, mram_tex_buf[1], common_size );
  
  assert( mram_tex[0] );
  assert( mram_tex[1] );
 
  active_tex = 0;
  current_frame = 0;
  
  initialized = true;
  return initialized;
}

void gc_ifl::deinit()
{
  if( !initialized ) return;
  
  if( plain_ifl )
  {
    nglReleaseTexture( plain_ifl );
    initialized = false;
    return;
  }

  aram_mgr::aram_sync();

  assert( frame_offset );
  free( frame_offset );
  frame_offset = NULL;
 
  assert( mram_tex[0] );
  nglReleaseTexture( mram_tex[0] );
  assert( mram_tex[1] );
  nglReleaseTexture( mram_tex[1] );
  assert( mram_tex_buf[0] );
  free( mram_tex_buf[0] );
  mram_tex_buf[0] = NULL;
  assert( mram_tex_buf[1] );
  free( mram_tex_buf[1] );
  mram_tex_buf[1] = NULL;
  
  initialized = false;
}

nglTexture *gc_ifl::get_frame(int frame)
{
  //assert( initialized );
  if( !initialized )
		return &nglWhiteTex;
	
	if( g_gc_ifl_lock_texture != -1 )
    frame = g_gc_ifl_lock_texture;
  
  frame = frame % num_frames;

  if( frame == current_frame )
    return mram_tex[ active_tex ];

  if( frame == ( ( current_frame + 1 ) % num_frames ) )
  {
    if( plain_ifl )
    {
      mram_tex[ active_tex ] = plain_ifl->Frames[ ( frame + 1 ) % num_frames ];
      active_tex = ( active_tex ? 0 : 1 );
      current_frame = frame;
    }
    else
    {
      aram_mgr::aram_read_async( aram_id, frame_offset[ frame ], mram_tex_buf[ active_tex ], common_size );
      active_tex = ( active_tex ? 0 : 1 );
      current_frame = frame;
    }
  }
  else
  {
    if( plain_ifl )
    {
      active_tex = 0;
      current_frame = frame;
      mram_tex[0] = plain_ifl->Frames[ current_frame ];
      mram_tex[1] = plain_ifl->Frames[ ( current_frame + 1 ) % num_frames ];
    }
    else
    {
      current_frame = frame;
      aram_mgr::aram_read_async( aram_id, frame_offset[ frame ], mram_tex_buf[ 0 ], common_size );
      aram_mgr::aram_read_async( aram_id, frame_offset[ ( frame + 1 ) % num_frames ], mram_tex_buf[ 1 ], common_size );
      active_tex = 0;
    }
  }
    
  return mram_tex[ active_tex ];
}
  
