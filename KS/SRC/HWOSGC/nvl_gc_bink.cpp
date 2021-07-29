#include <dolphin/AI.h>
#include <dolphin/AX.h>

#include "nvl_gc.h"

#include "rad.h"
#include "bink.h"

#include "gc_arammgr.h"

struct nvlMovieSource {
	bool used;
	char name[256];
	HBINK bink;
};

struct nvlMovie {
	bool used;

	u32 x, y;
	u32 width, height;
	nvlMovieSource* source;
	nvlResult state;
	nvlOutputBufferCallback output;
	void* user;
};

static nvlMovieSource sources[NVL_MAX_MOVIE_NUM];
static nvlMovie movies[NVL_MAX_MOVIE_NUM];

static void _nvlClear( void )
{

	for( int i = 0; i < NVL_MAX_MOVIE_NUM; ++i ) {
		sources[i].used = false;
		movies[i].used = false;	
	}

}

static void* _nvlAllocWrapper( u32 bytes )
{
	return nglMemAlloc( bytes, 32 );
}

static void _nvlFreeWrapper( void* p )
{
	nglMemFree( p );
}

static void* _nvlAramAlloc( u32 bytes )
{
	aram_id_t id = aram_mgr::allocate( bytes );
	u32 u = aram_mgr::id_to_address( id );
	
	return (void*) u;
}

static void _nvlAramFree( void* p )
{
	u32 u = (u32) p;
	anim_id_t id = aram_mgr::address_to_id( u );

	aram_mgr::deallocate( id );
}

void nvlInit( void )
{
	AIInit( NULL );
	AXInit( );

	RADSetMemory( _nvlAllocWrapper, _nvlFreeWrapper );
	RADSetAudioMemory( _nvlAramAlloc, _nvlAramFree );

	_nvlClear( );
}

void nvlReset( void )
{
	nvlShutdown( );
	nvlInit( );
}

void nvlShutdown( void )
{
	AXQuit( );
	AIReset( );
}

void nvlReloadMovies( void )
{

	for( int i = 0; i < NVL_MAX_MOVIE_NUM; ++i ) {
		nvlMovie* movie = &movies[i];

		if( movie->used ) {
			nvlMovieSource* source = NULL;
			HBINK bink = NULL;
			int frame = 0;

			source = movie->source;
			assert( source );
			bink = source->bink;
			assert( bink );
			frame = bink->FrameNum;
			BinkClose( bink );
			bink = BinkOpen( source->name, 0 );

			// this can happen if someone pops the
			// disc cover, necessitating a reload,
			// and then puts in the wrong disc,
			// so that when they close the cover
			// and we're all systems go (busy), we
			// reload, but then it's the wrong disc,
			// so the open fails.			
			if( bink == NULL ) {
				source->used = false;
				source->bink = NULL;

				movie->used = false;
				movie->state = NVL_RESULT_ERROR;

				continue;			
			}

			BinkGoto( bink, frame, 0 );
			source->bink = bink;
		}

	}

}

static s32 _nvlRender( nvlMovie* movie )
{
	size_t size = 0;
	void* buf = NULL;
	HBINK bink;
	nvlMovieSource* source = NULL;

	assert( movie );
	assert( movie->output );
	buf = movie->output( movie, &size, movie->user );
	assert( buf );
	source = movie->source;
	assert( source );
	bink = source->bink;
	assert( bink );
	// FIXME: offsets from nvlMovieParametersGC() or some such

	u32 flags = BINKSURFACEYUY2 | BINKCOPYALL;
	
	// FIXME: awful hack because of ATVI incompetence. I'd
	// use nglScreenHeight(), but that's going to be > 480
	// for PAL.
	if( bink->Height < 480 ) {
		flags |= BINKCOPY2XH;
	}

	s32 r = BinkCopyToBuffer( bink,
														OSCachedToUncached( buf ),
														bink->Width * 2, bink->Height,
														0, 0,
														flags );
	assert( r == 0 );

	return r;
}

void nvlAdvance( void )
{

	for( int i = 0; i < NVL_MAX_MOVIE_NUM; ++i ) {
		nvlMovie* movie = &movies[i];

		if( movie->used && movie->state == NVL_RESULT_PLAYING ) {
			nvlMovieSource* source = movie->source;

			assert( source );

			s32 w = BinkWait( source->bink );

			if( w == 0 ) {
				BinkDoFrame( source->bink );
				_nvlRender( movie );

				if( nvlMovieGetFrame( movie ) == nvlMovieGetFrameNum( movie ) ) {
					// stop
					movie->state = NVL_RESULT_SUCCESS;
				} else {
					BinkNextFrame( source->bink );
				}

			}

		}

	}

}

void nvlSetFramebufferGC( int w, int h )
{
	// empty
}

nvlMovieSource* _nvlFindMovieSource( void )
{

	for( int i = 0; i < NVL_MAX_MOVIE_NUM; ++i ) {
		nvlMovieSource* source = &sources[i];

		if( source->used == false ) {
			source->used = true;
			return source;		
		}

	}

	return NULL;
}

static void _nvlCanonicalName( const char* filename, char* canon )
{
	const char* fp = filename;
	char* cp = canon;
	
	while( *fp ) {

		if( *fp == '\\' ) {
			*cp = '/';
		} else {
			*cp = *fp;
		}

		++fp;
		++cp;
	}

	*cp = '\0';

	// canonicalize suffix
	cp = strrchr( canon, '.' );

	if( cp ) {
		++cp;
		strcpy( cp, "bik" );
	} else {
		strcat( canon, ".bik" );
	}

}

nvlMovieSource* nvlLoadMovieSource( const char* filename, int size, void* buf )
{
	nvlMovieSource* source = NULL;
	char name[256];

	assert( filename );
	
	source = _nvlFindMovieSource( );
	
	if( !source ) {
		return NULL;
	}

	_nvlCanonicalName( filename, name );
	source->bink = BinkOpen( name, 0 );

	if( source->bink == NULL ) {
		source->used = false;
		return NULL;	
	}

	strncpy( source->name, name, sizeof( source->name ) );

	return source;
}

void nvlReleaseMovieSource( nvlMovieSource* source )
{
	assert( source );
	
	if( !source->used ) {
		return;
	}

	if( source->bink ) {
		BinkClose( source->bink );
	}

	source->bink = NULL;
	source->used = false;
}

static nvlMovie* _nvlFindMovie( void )
{

	for( int i = 0; i < NVL_MAX_MOVIE_NUM; ++i ) {
		nvlMovie* movie = &movies[i];

		if( movie->used == false ) {
			movie->used = true;
			return movie;		
		}

	}

	return NULL;

}

nvlMovie* nvlAddMovie( nvlMovieSource* source, nvlImageFormat format )
{
	nvlMovie* movie = NULL;
	
	assert( source );
	
	if( !source ) {
		// FIXME: bad
		return NULL;	
	}

	movie = _nvlFindMovie( );
	
	if( !movie ) {
		return NULL;	
	}

	movie->source = source;
	movie->state = NVL_RESULT_SUCCESS;

	return movie;
}

nvlResult nvlPlayMovie( nvlMovie* movie, nvlOutputBufferCallback cb, void* user, int looping )
{
	assert( movie );

	movie->output = cb;
	movie->user = user;

	// we're just setting a flag
	movie->state = NVL_RESULT_PLAYING;

	return movie->state;
}

void nvlStopMovie( nvlMovie* movie )
{
	assert( movie );

	movie->state = NVL_RESULT_SUCCESS;
	movie->used = false;
	movie->source = NULL;
}

nvlResult nvlPauseMovie( nvlMovie* movie )
{
	nvlMovieSource* source = NULL;
	int r = 0;

	assert( movie );

	movie->state = NVL_RESULT_PAUSED;
	source = movie->source;
	assert( source );
	r = BinkPause( source->bink, 1 );

	return ( r ? NVL_RESULT_PAUSED : NVL_RESULT_PLAYING );
}

nvlResult nvlUnpauseMovie( nvlMovie* movie )
{
	nvlMovieSource* source = NULL;
	int r = 0;

	assert( movie );

	// FIXME: hack
	movie->state = ( nvlMovieGetFrame( movie ) > 0 ) ? NVL_RESULT_PLAYING : NVL_RESULT_SUCCESS;
	source = movie->source;
	assert( source );
	r = BinkPause( source->bink, 0 );

	return ( r ? NVL_RESULT_PAUSED : NVL_RESULT_PLAYING );
}

nvlResult nvlMovieStatus( nvlMovie* movie )
{
	assert( movie );

	return movie->state;
}

int nvlMovieGetWidth( nvlMovie* movie )
{
	nvlMovieSource* source = NULL;
	HBINK bink;

	assert( movie );
	source = movie->source;
	assert( source );
	bink = source->bink;
	assert( bink );

	return bink->Width;
}

int nvlMovieGetHeight( nvlMovie* movie )
{
	nvlMovieSource* source = NULL;
	HBINK bink;

	assert( movie );
	source = movie->source;
	assert( source );
	bink = source->bink;
	assert( bink );

	return bink->Height;
}

int nvlMovieGetFrameNum( nvlMovie* movie )
{
	nvlMovieSource* source = NULL;
	HBINK bink;

	assert( movie );
	source = movie->source;
	assert( source );
	bink = source->bink;
	assert( bink );

	return bink->Frames;
}

int nvlMovieGetFrame( nvlMovie* movie )
{
	nvlMovieSource* source = NULL;
	HBINK bink;

	assert( movie );
	source = movie->source;
	assert( source );
	bink = source->bink;
	assert( bink );

	return bink->FrameNum;
}

void nvlStopAllMovies( void )
{
	
	for( int i = 0; i < NVL_MAX_MOVIE_NUM; ++i ) {
		nvlMovie* movie = &movies[i];
		
		if( movie->used ) {
			nvlStopMovie( movie );		
		}
	
	}
 
}

void nvlPauseAllMovies( void )
{
	
	for( int i = 0; i < NVL_MAX_MOVIE_NUM; ++i ) {
		nvlMovie* movie = &movies[i];

		if( movie->used ) {
			nvlPauseMovie( movie );		
		}

	}

}

void nvlUnpauseAllMovies( void )
{
	
	for( int i = 0; i < NVL_MAX_MOVIE_NUM; ++i ) {
		nvlMovie* movie = &movies[i];
		
		if( movie->used ) {
			nvlUnpauseMovie( movie );		
		}
	
	}
 
}

void nvlMovieSetVolume( nvlMovie* movie, float volume )
{
	nvlMovieSource* source = NULL;
	
	assert( movie );
	source = movie->source;
	assert( source );
	s32 vp = (s32) ( volume * 32768.0f );
	BinkSetVolume( source->bink, 0, vp );
}

void nvlMovieSetParametersGC( nvlMovie* movie, int x, int y, int w, int h )
{
	movie->x = x;
	movie->y = y;
	movie->width = w;
	movie->height = h;
}
