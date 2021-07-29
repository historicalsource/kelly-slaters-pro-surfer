// scene_anim.cpp

#include "global.h"

#include "wds.h"
//!#include "character.h"
#include "scene_anim.h"
#include "controller.h"
//#include "brain.h"
#include "app.h"
#include "game.h"

#include "physical_interface.h"
#include "ai_interface.h"
#include "aligned.h"

void scene_anim::text_load( const stringx& filename )
{
  chunk_file fs;
  chunk_flavor cf;
  unsigned short tracks, cn;

  fs.open( filename );

  serial_in( fs, &cf );
  if( cf != chunk_flavor("tracks") )
    error( "Invalid scene file " + filename );

  serial_in( fs, &tracks );

  for( cn = 0; cn < tracks; ++cn )
  {
    stringx entity_name;

    serial_in( fs, &cf );

    if( cf != chunk_flavor("entity") )
      error( "Invalid scene file " + filename );

    serial_in( fs, &entity_name );

    entity_track_tree* et = new entity_track_tree;
    serial_in( fs, et );

    track_tree_map[ entity_name ] = et;
  }

  fs.close( );
}

// needs to be kept in sync with tools (terrainexp/sceneanmexp.cpp)
#ifndef TARGET_GC
#define SNMX_TAG     0x584d4e53
#define SNMX_VERSION 0x00020000
#else
#define SNMX_TAG     0x534e4d58
#define SNMX_VERSION 0x00000200
#endif

typedef struct _snmx_header {
  uint32 tag;
  uint32 version;
  uint32 child_count;
  uint32 pad0;
} snmx_header;

void scene_anim::binary_load_from_stash( const stringx& filename )
{
  filespec spec( filename );
  pstring p( spec.name.c_str( ) );
  p.concatinate( spec.ext.c_str( ) );

  stash snmx_file;
  unsigned char* snmx = NULL;
  int size = 0;
  stash_index_entry* stash_header = NULL;

  bool b = snmx_file.get_memory_image( p, snmx, size, stash_header );

  if( !b ) {
    error( "couldn't get memory image for " + filename );
    return;
  }

  // fun time!
  snmx_header* header = (snmx_header*) snmx;

	#ifdef TARGET_GC
	fixupuint32(&header->child_count);
	#endif

  if( header->tag != SNMX_TAG ) {
    error( "supposed SNMX file " + filename + " has invalid header tag" );
  }

  if( header->version != SNMX_VERSION ) {
    error( "unsupported SNMX version in %s (%d)", filename.c_str( ), header->version );
  }

  // advance buffer
  snmx += sizeof( snmx_header );

  for( int i = 0 ; i < (int) header->child_count; i++ ) {
		#ifdef TARGET_GC
		unsigned char *idptr=(unsigned char *) snmx;
		for ( int j=0; j<(sizeof(pstring)/sizeof(int64)); j++ )
		{
			fixup(idptr,sizeof(int64));
			idptr += sizeof(int64);
		}
		#endif
    pstring* p = (pstring*) snmx;
    snmx += sizeof( pstring );
    uint32 size;
    size = *( (uint32*) snmx );
		#ifdef TARGET_GC
		fixupuint32(&size);
		#endif
    snmx += sizeof( uint32 );
    uint32 pad;
    pad = *( (uint32*) snmx );
    snmx += sizeof( uint32 );
    entity_track_tree* et = (entity_track_tree*) snmx;
    entity_track_tree_from_binary( et, filename.c_str( ) );
    stringx s( p->c_str( ) );
    track_tree_map[s] = et;
    snmx += size;
  }

}

void scene_anim::binary_load_from_os( const stringx& filename )
{
  os_file fs;

  fs.open( filename, os_file::FILE_READ );

  snmx_header header;

  fs.read( &header.tag, sizeof( header.tag ) );

  if( header.tag != SNMX_TAG ) {
    error( "supposed SNMX file " + filename + " has invalid header tag" );
    return;
  }

  fs.read( &header.version, sizeof( header.version ) );

  if( header.version != SNMX_VERSION ) {
    error( "unsupported SNMX version in %s (%d)", filename.c_str( ), header.version );
  }

  fs.read( &header.child_count, sizeof( header.child_count ) );
  fs.read( &header.pad0, sizeof( header.pad0 ) );
	#ifdef TARGET_GC
	fixupuint32(&header.child_count);
	#endif

  for( int i = 0; i < (int) header.child_count; i++ ) {
	  pstring p;
	  fs.read( &p, sizeof( pstring ) );
#ifdef TARGET_GC
		unsigned char *idptr=(unsigned char *) &p;
		for ( int j=0; j<(sizeof(pstring)/sizeof(int64)); j++ )
		{
			fixup(idptr,sizeof(int64));
			idptr += sizeof(int64);
		}
#endif
    uint32 size;
    fs.read( &size, sizeof( uint32 ) );
#ifdef TARGET_GC
		fixupuint32(&size);
#endif
	  uint32 pad;
	  fs.read( &pad, sizeof( pad ) );
	  // this memory is freed in a class-specific destructor (or at least it should be :) )
	  entity_track_tree* et = (entity_track_tree*) malloc( size );
	  fs.read( et, size );
	  entity_track_tree_from_binary( et, filename.c_str( ) );
    stringx s( p.c_str( ) );
    track_tree_map[s] = et;
  }

  fs.close( );
}

void scene_anim::binary_load( const stringx& filename )
{

  if( stash::using_stash( ) ) {
    binary_load_from_stash( filename );
  } else {
    binary_load_from_os( filename );
  }

}

void scene_anim::load( const stringx& filename )
{
  filespec spec( filename );

  if( spec.ext == ".snm" || spec.ext == ".SNM" ) {
    text_load( filename );
  } else {
    binary_load( filename );
  }

}


void scene_anim::play( scene_anim_list_t &animlist, scene_anim_handle_t handle, bool reverse, float start_time )
{
  track_tree_list_t::iterator it;

  // disable brains
  ai_interface::push_disable_all();

  for( it = track_tree_map.begin(); it != track_tree_map.end(); it++ )
  {
    stringx s = (*it).first;
    s.to_upper();
    entity_track_tree *track = (*it).second;
    entity *ep;

    if ( s == stringx( "CAMERA" ) )
    {
      // camera anim support
      ep = g_world_ptr->get_marky_cam_ptr();
      app::inst()->get_game()->enable_marky_cam( true );
      ep->set_externally_controlled( true );
    }
    else
      ep = entity_manager::inst()->find_entity(entity_id( s.c_str() ),IGNORE_FLAVOR);

    if ( ep )
    {
/*!      if( ep->is_a_character() )
        ((character*)ep)->clear_render_translucent_flag();
!*/
      // find track
      scene_anims_info info;

	  entity_anim_tree* my_anim_tree;
	  if(reverse)
	      my_anim_tree = ep->play_anim( ANIM_SCENE, s, *track, start_time, ANIM_AUTOKILL|ANIM_NONCOSMETIC|ANIM_PO_FIXUP|ANIM_REVERSE );
	  else
	      my_anim_tree = ep->play_anim( ANIM_SCENE, s, *track, start_time, ANIM_AUTOKILL|ANIM_NONCOSMETIC|ANIM_PO_FIXUP );

      // disable physics
      if(ep->has_physical_ifc())
        ep->physical_ifc()->suspend();

      ep->invalidate_frame_delta();

      ep->set_playing_scene_anim(true);

      // add to anim list
      scene_anim_list_t::iterator i = animlist.begin();
      scene_anim_list_t::iterator i_end = animlist.begin();
      for ( ; i!=i_end; ++i )
        if( (*i).ent == NULL )
          break;

      info.ent = ep;
      info.entity_up_vec = info.ent->get_abs_po().get_y_facing();
      info.entity_up_vec.normalize();
      info.handle = handle;
      info.name = my_anim_tree->get_name();
	  //BETH
	  info.anim_tree = my_anim_tree;

      if ( i == i_end )
        animlist.push_back( info );
      else
        (*i) = info;
    }
  }
}

void scene_anim::kill()
{
}

scene_anim::~scene_anim()
{
  track_tree_list_t::iterator it;

  for( it = track_tree_map.begin(); it != track_tree_map.end(); ++it )
  {
    entity_track_tree *track = (*it).second;
    delete track;
  }

  track_tree_map.clear();
}

