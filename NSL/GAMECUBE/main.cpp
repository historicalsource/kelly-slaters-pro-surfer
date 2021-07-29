#include <stdlib.h>

#include <dolphin/os.h>
#include <dolphin/dvd.h>
#include <dolphin/vi.h>
#include <dolphin/pad.h>
#include <dolphin/ar.h>
#include <dolphin/arq.h>

#include "common/nsl.h"

static bool osAllocInitialized = false;

static void osSetupHeap( void )
{
	void* arenaLow = NULL;
	void* arenaHigh = NULL;
	OSHeapHandle heap = 0;

	if( osAllocInitialized ) {
		return;
	}

	// get arena boundaries
	arenaLow = OSGetArenaLo( );
	arenaHigh = OSGetArenaHi( );

	// initialize the arena with a maximum of one heap
	// we remember arena low because it can be changed
	// due to the arena's bookkeeping data
	arenaLow = OSInitAlloc( arenaLow, arenaHigh, 1 );
#ifdef DEBUG
	OSReport( "OSInitAlloc: creating heap of size %d\n",  (int) arenaHigh - (int) arenaLow );
#endif
	// set it again
	OSSetArenaLo( arenaLow );
	// 32-byte align
	arenaLow = (void*) OSRoundUp32B( arenaLow );
	arenaHigh = (void*) OSRoundDown32B( arenaHigh );
	// create a heap
	heap = OSCreateHeap( arenaLow, arenaHigh );
	// set it to be the current one
	OSSetCurrentHeap( heap );
	// now our arena has moved after the heap alloc,
	// reset its boundaries
	arenaLow = arenaHigh;
	OSSetArenaLo( arenaLow );

	osAllocInitialized = true;
}

void* operator new( size_t n )
{
	return OSAlloc( OSRoundUp32B( n ) );
}

void* operator new[]( size_t n )
{
	return OSAlloc( OSRoundUp32B( n ) );
}

void operator delete( void* p )
{

	if( p ) {
		OSFree( p );
	}

}

void operator delete[]( void* p )
{

	if( p ) {
		OSFree( p );
	}

}

static void testMultiPlay( void )
{
	nslReset( "origin_b", NSL_LANGUAGE_ENGLISH );

	nslSourceId src = nslLoadSource( "origin_main" );

	nslSourceId srcs[4];

	srcs[0] = nslLoadSource( "thugstp1" );
	srcs[1] = nslLoadSource( "thugstp2" );
	srcs[2] = nslLoadSource( "thugstp3" );
	srcs[3] = nslLoadSource( "thugstp4" );

	nslSoundId snd;
	nslSoundId snds[4] = { NSL_INVALID_ID, NSL_INVALID_ID, NSL_INVALID_ID, NSL_INVALID_ID };

	nlUint32 prev = 3;
	nlUint32 next = 0;

	while( 1 ) {
		PADStatus pads[PAD_MAX_CONTROLLERS];
		
		nslFrameAdvance( 1.0f );

		PADRead( pads );
		PADClamp( pads );
		
		if( pads[0].err != PAD_ERR_NONE ) {
			continue;
		}

		if( pads[0].button & PAD_BUTTON_START ) {
			break;
		}

		if( pads[0].button & PAD_BUTTON_A ) {
		
			if( ( nslGetSoundStatus( snds[next] ) != NSL_SOUNDSTATUS_PLAYING ) &&
					( nslGetSoundStatus( snds[prev] ) != NSL_SOUNDSTATUS_PLAYING ) ) {
				snds[next] = nslAddSound( srcs[next] );
				nslPlaySound( snds[next] );
				prev = next;
				++next;
				
				if( next > 3 ) {
					next = 0;				
				}

			}

		}
		
		if( pads[0].button & PAD_BUTTON_B ) {

			if( nslGetSoundStatus( snd ) != NSL_SOUNDSTATUS_PLAYING ) {
				snd = nslAddSound( src );
				nslPlaySound( snd );
			}

		}
		
	}

}

static void testPause( void )
{
	nslReset( "origin_b", NSL_LANGUAGE_ENGLISH );

	nslSourceId src = nslLoadSource( "origin_main" );
	nslSoundId snd = nslAddSound( src );

	nslPlaySound( snd );

	while( 1 ) {
		PADStatus pads[PAD_MAX_CONTROLLERS];
		
		nslFrameAdvance( 1.0f );

		PADRead( pads );
		PADClamp( pads );
		
		if( pads[0].err != PAD_ERR_NONE ) {
			continue;
		}

		if( pads[0].button & PAD_BUTTON_START ) {
			break;
		}

		if( pads[0].button & PAD_BUTTON_B ) {

			if( nslGetSoundStatus( snd ) != NSL_SOUNDSTATUS_PAUSED ) {
				nslPauseSound( snd );
			}

		}

		if( pads[0].button & PAD_BUTTON_A ) {

			if( nslGetSoundStatus( snd ) == NSL_SOUNDSTATUS_PAUSED ) {
				nslUnpauseSound( snd );
			}

		}
	
	}

}

static void testPitch( void )
{
	nslReset( "cath_c", NSL_LANGUAGE_ENGLISH );

	nslSourceId src = nslLoadSource( "rain_amb" );
	nslSoundId snd = nslAddSound( src );
	nslPlaySound( snd );
	
	while( 1 ) {
		PADStatus pads[PAD_MAX_CONTROLLERS];
		
		nslFrameAdvance( 1.0f );

		PADRead( pads );
		PADClamp( pads );
		
		if( pads[0].err != PAD_ERR_NONE ) {
			continue;
		}

		if( pads[0].button & PAD_BUTTON_START ) {
			break;
		}

		if( nslGetSoundStatus( snd ) == NSL_SOUNDSTATUS_PLAYING ) {
			float pitch = ( pads[0].stickY / 72.0f ) + 1.0f;
			float volume = ( pads[0].substickY / 59.0f ) + 1.0f;
			nslSetSoundParam( snd, NSL_SOUNDPARAM_PITCH, pitch );
			nslSetSoundParam( snd, NSL_SOUNDPARAM_VOLUME, volume );
		}

	}

}

static void testDistance( void )
{
	nlVector3d pos = { 0.0f, 0.0f, 0.0f };

	nslReset( "cath_c", NSL_LANGUAGE_ENGLISH );

	nslSourceId src = nslLoadSource( "rain_amb" );

	nslSoundId snd = nslAddSound( src );
	nslSetSoundParam( snd, NSL_SOUNDPARAM_MINDIST, 0.0f );
	nslSetSoundParam( snd, NSL_SOUNDPARAM_MAXDIST, 10.0f );
	nslEmitterId emit = nslCreateEmitter( pos );
	nslSetSoundEmitter( emit, snd );

	nslSetListenerPosition( pos );
	nlVector3d fwd = { 0.0f, 0.0f, 1.0f };
	nlVector3d up = { 0.0f, 1.0f, 0.0f };
	nslSetListenerOrientation( fwd, up );

	nslPlaySound( snd );

	while( 1 ) {
		PADStatus pads[PAD_MAX_CONTROLLERS];

		nslFrameAdvance( 1.0f );

		PADRead( pads );
		PADClamp( pads );

		if( pads[0].err != PAD_ERR_NONE ) {
			continue;
		}

		if( pads[0].button & PAD_BUTTON_START ) {
			break;
		}

		// update emitter position based on stick
		s8 stickY = pads[0].stickY;
		float y = ( stickY / 72.0f ) * 10.0f;
		s8 stickX = pads[0].stickX;
		float x = ( stickX / 72.0f ) * 10.0f;
		nlVector3d where = { x, 0.0f, y };

		nslSetEmitterPosition( emit, where );
	}

}

static void testDampen( void )
{
	nslReset( "cath_c", NSL_LANGUAGE_ENGLISH );

	nslSourceId src = nslLoadSource( "rain_amb" );
	nslSoundId snd = nslAddSound( src );

	u32 button = 0;

	while( 1 ) {
		PADStatus pads[PAD_MAX_CONTROLLERS];

		nslFrameAdvance( 1.0f );

		PADRead( pads );
		PADClamp( pads );

		if( pads[0].err != PAD_ERR_NONE ) {
			continue;
		}

		if( pads[0].button & PAD_BUTTON_START ) {
			break;
		}

		if( pads[0].button & PAD_BUTTON_A ) {

			if( nslGetSoundStatus( snd ) != NSL_SOUNDSTATUS_PLAYING ) {
				nslPlaySound( snd );
			}

		}

		if( ( pads[0].button & PAD_BUTTON_B ) && ! ( button & PAD_BUTTON_B ) ) {
			nslDampenAllSounds( 0.25f );
		} else if( ( button & PAD_BUTTON_B ) && ! ( pads[0].button & PAD_BUTTON_B ) ) {
			nslUndampenAllSounds( );
		}

		button = pads[0].button;
	}

}

static void testOverload( void )
{
	nslReset( "origin_b", NSL_LANGUAGE_ENGLISH );

	nlVector3d pos = { 0.0f, 0.0f, 0.0f };
	nlVector3d fwd = { 0.0f, 0.0f, -1.0f };
	nlVector3d up = { 0.0f, 1.0f, 0.0f };

	nslSetListenerPosition( pos );
	nslSetListenerOrientation( fwd, up );

	nslSourceId src = nslLoadSource( "explo1" );
	
	nslSoundId sid = NSL_INVALID_ID;

	while( 1 ) {
		PADStatus pads[PAD_MAX_CONTROLLERS];

		nslFrameAdvance( 1.0f );

		PADRead( pads );
		PADClamp( pads );

		if( pads[0].err != PAD_ERR_NONE ) {
			continue;
		}

		if( pads[0].button & PAD_BUTTON_START ) {
			break;
		}

		if( pads[0].button & PAD_BUTTON_A ) {

			if( nslGetSoundStatus( sid ) == NSL_SOUNDSTATUS_PLAYING ) {
				continue;
			}

			// update emitter position based on stick
			s8 stickY = pads[0].stickY;
			float y = ( stickY / 72.0f ) * 10.0f;
			s8 stickX = pads[0].stickX;
			float x = ( stickX / 72.0f ) * 10.0f;
			nlVector3d where = { x, 0.0f, y };

			sid = nslAddSound( src );

			if( sid == NSL_INVALID_ID ) {
				continue;
			}

			nslSetSoundParam( sid, NSL_SOUNDPARAM_MINDIST, 0.0f );
			nslSetSoundParam( sid, NSL_SOUNDPARAM_MAXDIST, 10.0f );
			nslPlaySound3D( sid, where );
				
			if( pads[0].button & PAD_TRIGGER_Z ) {

				// yes, of course this is bad, we're trying to
				// make the system crash by overloading it with
				// play requests that can't be fulfilled, etc.
				if( nslGetSoundStatus( sid ) == NSL_SOUNDSTATUS_PLAYING ) {
					nslPauseSound( sid );
				}

			}

		}

	}

}

static void testSlayerPlay( void )
{
	nslReset( "city_q", NSL_LANGUAGE_ENGLISH );

	nslSourceId src = nslLoadSource( "laser1" );
	nslSoundId snd = NSL_INVALID_ID;

	while( 1 ) {
		PADStatus pads[PAD_MAX_CONTROLLERS];
		
		nslFrameAdvance( 1.0f );

		PADRead( pads );
		PADClamp( pads );
		
		if( pads[0].err != PAD_ERR_NONE ) {
			continue;
		}

		if( pads[0].button & PAD_BUTTON_START ) {
			break;
		}

		if( pads[0].button & PAD_BUTTON_A ) {
			snd = nslAddSound( src );
				
			if( snd != NSL_INVALID_ID ) {
				nslPlaySound( snd );
			} else {
				nslPrintf( "invalid sound id\n" );
			}

		}
		
	}

}

static void testStereo( void )
{
	nslReset( "city_b", NSL_LANGUAGE_ENGLISH );
	
	nslSourceId src = nslLoadSource( "city_b_1" );
	nslSoundId snd = nslAddSound( src );
	nslPlaySound( snd );
	
	while( 1 ) {
		PADStatus pads[PAD_MAX_CONTROLLERS];

		nslFrameAdvance( 1.0f );

		if( nslGetSoundStatus( snd ) != NSL_SOUNDSTATUS_PLAYING ) {
			src = nslLoadSource( "city_main" );
			snd = nslAddSound( src );
			nslPlaySound( snd );
		}

		PADRead( pads );
		PADClamp( pads );

		if( pads[0].err != PAD_ERR_NONE ) {
			continue;
		}

		if( pads[0].button & PAD_BUTTON_A ) {
			nslStopSound( snd );
		}

	}

}

static void testOscorpBX( void )
{
	nslReset( "oscorpbx", NSL_LANGUAGE_ENGLISH );

	nslSourceId srcs[6];
	nslSoundId snds[4];

	nslFrameAdvance( 1.0f );

	srcs[0] = nslLoadSource( "supsld_ftsteps1" );
	srcs[1] = nslLoadSource( "supsld_ftsteps2" );
	srcs[2] = nslLoadSource( "supsld_ftsteps3" );
	srcs[3] = nslLoadSource( "supsld_ftsteps4" );
	srcs[4] = nslLoadSource( "osc_bx_in" );
	srcs[5] = nslLoadSource( "spideysense" );

	nslFrameAdvance( 1.0f );

	snds[0] = nslAddSound( srcs[0] );
	snds[1] = nslAddSound( srcs[3] );
	snds[2] = nslAddSound( srcs[1] );
	snds[3] = nslAddSound( srcs[3] );
	nslPlaySound( snds[0] );
	nslPlaySound( snds[1] );
	nslPlaySound( snds[2] );
	nslPlaySound( snds[3] );

	nslFrameAdvance( 1.0f );

	snds[0] = nslAddSound( srcs[2] );
	nslPlaySound( snds[0] );
	
	nslFrameAdvance( 1.0f );

	snds[0] = nslAddSound( srcs[1] );
	nslPlaySound( snds[0] );
	
	nslFrameAdvance( 1.0f );

	snds[0] = nslAddSound( srcs[0] );
	nslPlaySound( snds[0] );
	
	nslFrameAdvance( 1.0f );

	snds[0] = nslAddSound( srcs[4] );
	snds[1] = nslAddSound( srcs[5] );
	nslPlaySound( snds[0] );
	nslPlaySound( snds[1] );

	nslFrameAdvance( 1.0f );

	snds[0] = nslAddSound( srcs[5] );
	nslPlaySound( snds[0] );
	
	nslFrameAdvance( 1.0f );

	while( 1 ) {
		PADStatus pads[PAD_MAX_CONTROLLERS];

		nslFrameAdvance( 1.0f );

		PADRead( pads );
		PADClamp( pads );

		if( pads[0].err != PAD_ERR_NONE ) {
			continue;
		}

	}

}

static float randf( float x, float y )
{
  float f = (float) rand( );
  f /= RAND_MAX;
  return x + ( f * ( y - x ) );
}

static float variance( float x, float v )
{
	return x + randf( -v, v );
}

static void testPopping( void )
{
	nslReset( "origin_c", NSL_LANGUAGE_ENGLISH );

	nlVector3d up = { 0.0f, 1.0f, 0.0f };
	nlVector3d fwd = { 0.0f, 0.0f, -1.0f };
	nlVector3d pos = { 0.0f, 0.0f, 0.0f };
	nslSetListenerOrientation( fwd, up );
	nslSetListenerPosition( pos );

	nslSourceId srcs[4];

	srcs[0] = nslLoadSource( "sm_ftsteps1" );
	srcs[1] = nslLoadSource( "sm_ftsteps2" );
	srcs[2] = nslLoadSource( "sm_ftsteps3" );
	srcs[3] = nslLoadSource( "sm_ftsteps4" );

	nslEmitterId emitter = nslCreateEmitter( pos );

	nslSoundId snd = NSL_INVALID_ID;
	int i = 0;

	while( 1 ) {
		PADStatus pads[PAD_MAX_CONTROLLERS];

		PADRead( pads );
		PADClamp( pads );

		if( pads[0].err != PAD_ERR_NONE ) {
			continue;
		}

		// update emitter position based on stick
		s8 stickX = pads[0].stickX;
		float x = ( stickX / 72.0f ) * 10.0f;
		s8 stickY = pads[0].stickY;
		float y = ( stickY / 72.0f ) * 10.0f;
		nlVector3d where = { x, 0.0f, -y };
		nslSetEmitterPosition( emitter, where );

		if( pads[0].button & PAD_BUTTON_A ) {

			if( nslGetSoundStatus( snd ) != NSL_SOUNDSTATUS_PLAYING ) {
				snd = nslAddSound( srcs[i++] );
				nslSetSoundParam( snd, NSL_SOUNDPARAM_VOLUME, 1.0f );
				nslSetSoundParam( snd, NSL_SOUNDPARAM_PITCH, variance( 1.0f, 0.15f ) );
				nslSetSoundParam( snd, NSL_SOUNDPARAM_MINDIST, 5.0f );
				nslSetSoundParam( snd, NSL_SOUNDPARAM_MAXDIST, 10.0f );
				nslSetSoundEmitter( emitter, snd );
				nslPlaySound( snd );

				if( i > 3 ) {
					i = 0;
				}

			}

		}

		nslFrameAdvance( 1.0f );
	}

}

int main( int argc, char* argv[] )
{
	osSetupHeap( );

	OSInit( );
	DVDInit( );
	VIInit( );
	PADInit( );

	ARInit( NULL, 0 );
	ARQInit( );

	nslSetSpeakerMode( NSL_SPEAKER_DOLBY_51 );

	nslInit( );

	//testMultiPlay( );
	//testPause( );
	//testPitch( );
	//testDistance( );
	//testOverload( );
	//testSlayerPlay( );
	//testDampen( );
	//testStereo( );
	//testOscorpBX( );
	testPopping( );

	nslShutdown( );

	return 0;
}
