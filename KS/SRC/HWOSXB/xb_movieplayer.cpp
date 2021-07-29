#include "global.h"
#include "singleton.h"
#include "hwaudio.h"
#include "xb_movieplayer.h"


DEFINE_SINGLETON( movieplayer )

#ifndef TRUE
#define TRUE	true
#endif

#ifndef FALSE
#define FALSE	 false
#endif

#define NVL_USE_BINK

static int texIdx = 0;

static void* myOutputFunc( struct nvlMovie* m, size_t* s, void* data )
{
	nglTexture  **rvTex;
	assert( m && data );
	if( m )
	{
		rvTex = (nglTexture**)data;
#ifdef NVL_USE_BINK
		*s = rvTex[texIdx]->DataPitch;
#else
		*s = rvTex[texIdx]->Width * rvTex[texIdx]->Height << 2;
#endif
	}
	bool  res =	nglLockTextureXB( rvTex[texIdx] );
	assert( res );
	
#ifndef NVL_USE_BINK
	texIdx ^= 1;
#endif
	return rvTex[texIdx]->Data;
}

movieplayer::movieplayer()
{
	movie_started = false;
	movie_src = NULL;
	movie = NULL;
	for(int i=0; i<2; ++i)
		frame_texture[i] = NULL;
}

movieplayer::~movieplayer()
{
}

void movieplayer::init()
{
	nvlInit(nslGetDirectSoundObjXbox());
}

void movieplayer::shutdown()
{
	nvlShutdown();
}

void movieplayer::play( const char *filename, bool play_video, bool play_audio, bool loop)
{
	texIdx = 0;
	movie_src = nvlLoadMovieSource(filename);
	
	if (!movie_src)
		return;
	
#ifdef NVL_USE_BINK
	movie = nvlAddMovie( movie_src, NVL_IMAGE_FORMAT_RGBA32 );
#else
	movie = nvlAddMovie( movie_src );
#endif
	if (!movie)
	{
		nglPrintf("WARNING: Could not play movie %s!", filename);
		nvlReleaseMovieSource(movie_src);
		movie_src = NULL;
		return;
	}
	
#ifndef NVL_USE_BINK
	nglDev->SetRenderState(D3DRS_YUVENABLE, TRUE);
#endif
	
	for(int i=0; i<2; ++i)
	{
#ifdef NVL_USE_BINK
		frame_texture[i] = nglCreateTexture( NGLTF_32BIT | NGLTF_LINEAR, (unsigned int)nvlMovieGetWidth(movie), (unsigned int)nvlMovieGetHeight(movie) );
#else
		frame_texture[i] = nglCreateTexture( NGLTF_YUY2 | NGLTF_LINEAR, (unsigned int)nvlMovieGetWidth(movie), (unsigned int)nvlMovieGetHeight(movie) );
#endif
		nglLockTextureXB( frame_texture[i] );
		memset( frame_texture[i]->Data, 0, frame_texture[i]->DataPitch * frame_texture[i]->Height );
		nglUnlockTextureXB( frame_texture[i] );
	}
	nvlPlayMovie( movie, myOutputFunc, &frame_texture[0], loop );
	nvlMovieSetVolume(movie, nslGetVolume(NSL_SOURCETYPE_MOVIE)*nslGetMasterVolume());
	movie_started = true;
}

void movieplayer::stop()
{
	if(!movie)
		return;
	
	nvlStopMovie(movie);
	movie = NULL;
	for(int i=0; i<2; ++i)
	{
		nglUnlockTextureXB(frame_texture[i]);
		nglDestroyTexture(frame_texture[i]);
		frame_texture[i] = NULL;		
	}
	nvlReleaseMovieSource(movie_src);
	movie_src = NULL;
	nglDev->SetRenderState(D3DRS_YUVENABLE, FALSE);
	movie_started = false;
}

void movieplayer::pause( bool paused )
{
}

void movieplayer::restart_with(const char *filename, bool play_video, bool play_audio)
{
}

void movieplayer::start_frame(bool list_init)
{
	if (movie_started && list_init)
		nglListInit();
}

void movieplayer::end_frame(bool flip, bool list_send)
{
	if (!movie_started)
		return;
	
	nglUnlockTextureXB( frame_texture[texIdx] );
	nglInitQuad( &frame );
	nglSetQuadBlend( &frame, NGLBM_OPAQUE, 0 );
	nglSetQuadMapFlags(&frame, NGLMAP_CLAMP_U | NGLMAP_CLAMP_V | NGLMAP_BILINEAR_FILTER);
	nglSetQuadTex( &frame, frame_texture[texIdx] );
	nglSetQuadRect( &frame, 0.0f, 0.0f, nglGetScreenWidth(),  nglGetScreenHeight());
	nglSetQuadColor( &frame, 0xFFFFFFFF );
	nglSetQuadUV( &frame, 0.0f, 0.0f, 1, 1 );
	nglListAddQuad( &frame );

	if (list_send)
		nglListSend( flip );
}

void movieplayer::posit(int left, int top, int width, int height, int z )
{
	assert(0 && "unsupported function");
	// not supported at the moment
}

void movieplayer::set_volume(float vol)
{
	assert(0 && "unsupported function");
	// not supported at the moment
}

void movieplayer::set_brightness(float br)
{
	assert(0 && "unsupported function");
	// not supported at the moment
}

void movieplayer::set_br_offset( float offset )
{
	assert(0 && "unsupported function");
	// not supported at the moment
}

void* movieplayer::load_file(const char* fname)
{
	return(NULL);
}

bool movieplayer::is_playing()
{
	if(movie_started)
		//return (nvlMovieGetTime(movie) + 10000000 < nvlMovieGetDuration(movie));
		return (nvlMovieStatus(movie) == NVL_RESULT_PLAYING);
	else
		return false;
}



