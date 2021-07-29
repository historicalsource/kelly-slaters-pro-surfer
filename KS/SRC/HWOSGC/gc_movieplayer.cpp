#include "global.h"

#include <string.h>
#include "osassert.h"

#include <stdio.h>
#include <stdlib.h>

#include "gc_movieplayer.h"
#include <dolphin/dsp.h>

//Ack!! This is supposed to be an OS layer, it shouldn't have to include
//game specific stuff like this -mjd.
#include "career.h"

DEFINE_SINGLETON( movieplayer )

void* movieplayer::output_func( struct nvlMovie* m, size_t* s, void* data )
{
	void* fb = NULL;
	movieplayer* player = (movieplayer*) data;

  *s = nglGetScreenWidth() * nglGetScreenHeight() * 2;
	nglGCGetFrameBuffers( NULL, NULL, &fb );
	player->needs_flip = true;
	VIWaitForRetrace( );

  return fb;
}

movieplayer::movieplayer( void )
{
 	cur_mov_src = NULL;
  cur_mov = NULL;
  needs_flip = false;

  init( );
}

void movieplayer::init( void )
{
	// empty
}

movieplayer::~movieplayer( void )
{
  shutdown( );
}

void movieplayer::shutdown( void )
{
	// empty
}

void movieplayer::play( const char *filename, bool play_video, bool play_audio )
{
	// FIXME: I hate you.
	memcpy(&g_career->cfg, StoredConfigData::inst()->getGameConfig(), sizeof(ksConfigData));

	rational_t volume = nslGetVolume( NSL_SOURCETYPE_MOVIE ) * nslGetMasterVolume();

	// Normally we nslShutdown() here, but instead we'll let 
	// the KS engine handle this itself.
	nvlInit( );

  // FIXME: better buffer overflow support
  char name[256]= "";

  strcpy( name, filename );

  cur_mov = NULL;
  cur_mov_src = nvlLoadMovieSource( name );

	if( cur_mov_src == NULL )
	{
	  stop();
		return;
	}

  cur_mov = nvlAddMovie( cur_mov_src, NVL_IMAGE_FORMAT_YUV422 );

  if( cur_mov == NULL )
  {
    stop();
    return;
  }

  s32 movie_x= (nglGetScreenWidth() - nvlMovieGetWidth( cur_mov ) ) / 2;
  s32 movie_y= (nglGetScreenHeight() - nvlMovieGetHeight( cur_mov ) ) / 2;

	nvlMovieSetVolume( cur_mov, volume );

  nvlMovieSetParametersGC( cur_mov,
                           movie_x,
                           movie_y,
                           nglGetScreenWidth(),
                           nglGetScreenHeight()
                         );

  nvlPlayMovie( cur_mov, movieplayer::output_func, this, 0 );
}

void movieplayer::stop( void )
{

	if( cur_mov ) {
	  nvlStopMovie( cur_mov );
	  cur_mov = NULL;
	}

	if( cur_mov_src ) {
		nvlReleaseMovieSource( cur_mov_src );
		cur_mov_src = NULL;
	}

	nvlShutdown( );
	// Normally we would nslInit() here, but KS
	// does that after playing each movie.

	//We need DSP for the memory card
	//don't ask
	if( os_developer_options::inst()->is_flagged( os_developer_options::FLAG_NO_AUDIO ) )
		DSPInit();

	// FIXME: Hiss.
	StoredConfigData::inst()->setGameConfig(&(g_career->cfg));
}

void movieplayer::start_frame( void )
{

	if( cur_mov )
	{
    nvlAdvance( );
  }

}

void movieplayer::end_frame( void )
{

	// NOTE: this is actually useless now--we've gone
	// back to single-buffering. But I'll leave it
	// here because it's quaint.
	if( needs_flip == true )
	{
		nglGCFlipXFB( );
		needs_flip = false;
	}

}

void movieplayer::set_volume( float volume )
{
	assert( cur_mov );

	if( cur_mov )
	{
		nvlMovieSetVolume( cur_mov, volume );
	}

}

bool movieplayer::is_playing( void )
{

	if( cur_mov == NULL )
	{
		return false;
	}

	nvlResult r = nvlMovieStatus( cur_mov );

	return ( r == NVL_RESULT_PLAYING || r == NVL_RESULT_PAUSED );
}
