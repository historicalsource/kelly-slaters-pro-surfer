/*
 * Kelly Slater's Pro Surfer
 * (c) Treyarch, LLC, 2001
 *
 * GameCube bootstrap.
 */

#include "global.h"

#ifdef TARGET_GC

#include <dolphin/os.h>
#include <dolphin/dvd.h>
#include <dolphin/pad.h>
#include <dolphin/vi.h>

#include <dolphin/base/PPCArch.h>

#include "app.h"
#include "game.h"
#include "wds.h"
#include "osdevopts.h"
#include "hwosgc/gc_rasterize.h"
#include "file_manager.h"
#include "localize.h"
#include "ksnvl.h"
#include "nsl_gc.h"

#include "gc_arammgr.h"

enum ksGCError {
	KS_GC_NO_ERROR   = 0,
	KS_GC_NO_DISC    = 1,
	KS_GC_COVER_OPEN = 2,
	KS_GC_WRONG_DISC = 3,
	KS_GC_DISC_RETRY = 4,
	KS_GC_DISC_FATAL = 5,
	KS_GC_RESET      = 6
};

static volatile ksGCError ksGCCurrentError = KS_GC_NO_ERROR;

typedef struct {
	int num_lines;
	const char* lines[5];
} ksGCErrorMsg;

// lang = 0,1,...
static ksGCErrorMsg ksGCErrorMsgsEnglish[] = {
	{ 2, { "Please insert the",  "Kelly Slater's Pro Surfer Game Disc.", "", "", "" } },
	{ 1, { "Please close the Disc Cover.", "", "", "", "" } },
	{ 2, { "Please insert the",  "Kelly Slater's Pro Surfer Game Disc.", "", "", "" } },
	{ 5, { "The Game Disc could not be read.", "Please read the",  "Nintendo GameCube(tm)", "Instruction Booklet", "for more information." } },
	{ 5, { "An error has occurred.", "Turn the power OFF and refer to", "the Nintendo GameCube(tm)", "Instruction Booklet for", "further instructions." } }
};

// lang = 2
static ksGCErrorMsg ksGCErrorMsgsFrench[] = {
	{ 2, { "Veuillez insérer le disque:", "Kelly Slater's Pro Surfer.", "", "", "" } },
	{ 1, { "Veuillez fermer le couvercle.", "", "", "", "" } },
	{ 2, { "Veuillez insérer le disque:", "Kelly Slater's Pro Surfer.", "", "", "" } },
	{ 4, { "La lecture du disque a échoué.",  "Veuillez vous référer au manuel",  "d'instructions Nintendo Gamecube", "pour de plus amples informations.", "" } },
	{ 5, { "Une erreur est survenue.", "Eteignez la console et", "référez-vous au manuel", "d'instructions Nintendo Gamecube", "pour de plus amples informations." } }
};

// lang = 3
static ksGCErrorMsg ksGCErrorMsgsGerman[] = {
	{ 3, { "Bitte legen Sie die", "Kelly Slater's Pro Surfer", "Game Disc ein." } },
	{ 1, { "Bitte schließen Sie den Disc-Deckel.", "", "", "", "" } },
	{ 3, { "Bitte legen Sie die", "Kelly Slater's Pro Surfer", "Game Disc ein." } },
	{ 5, { "Diese Game Disc kann", "nicht gelesen werden.", "Bitte lesen Sie die", "Bedienungsanleitung, um weitere", "Informationen zu erhalten." } },
	{ 5, { "Ein Fehler ist aufgetreten.", "Bitte schalten Sie den NINTENDO GAMECUBE", "aus und lesen Sie die", "Bedienungsanleitung, um weitere", "Informationen zu erhalten." } }
};

static ksGCErrorMsg* ksGCErrorMsgs = NULL;
static void (*ksGCOldRetraceCallback)( u32 count ) = NULL;

static ksGCError KSStatusToError( s32 status )
{
	ksGCError error = KS_GC_NO_ERROR;

	switch( status ) {
	case DVD_STATE_NO_DISK:
		error = KS_GC_NO_DISC;
		break;
	case DVD_STATE_COVER_OPEN:
		error = KS_GC_COVER_OPEN;
		break;
	case DVD_STATE_WRONG_DISK:
		error = KS_GC_WRONG_DISC;
		break;
	case DVD_STATE_RETRY:
		error = KS_GC_DISC_RETRY;
		break;
	case DVD_STATE_FATAL_ERROR:
		error = KS_GC_DISC_FATAL;
		break;
	default:
		error = KS_GC_NO_ERROR;
		break;
	}

	return error;
}

static void KSGCErrorSetupLang( void )
{

	switch( ksGlobalTextLanguage ) {
	case LANGUAGE_ENGLISH:
		ksGCErrorMsgs = ksGCErrorMsgsEnglish;
		break;
	case LANGUAGE_PIG_LATIN:
		ksGCErrorMsgs = ksGCErrorMsgsEnglish;
		break;
	case LANGUAGE_FRENCH:
		ksGCErrorMsgs = ksGCErrorMsgsFrench;
		break;
	case LANGUAGE_GERMAN:
		ksGCErrorMsgs = ksGCErrorMsgsGerman;
		break;
	default:
		ksGCErrorMsgs = ksGCErrorMsgsEnglish;
		break;
	}

}

static void KSGCDisplayError( ksGCError error )
{
	stringx error_msg;

	if( error == KS_GC_NO_ERROR ) {
		return;
	}

	KSGCErrorSetupLang( );

	ksGCErrorMsg* msg = &ksGCErrorMsgs[ error - 1 ];

	nglListInit( );
	nglListBeginScene( );

	nglSetClearFlags( NGLCLEAR_COLOR | NGLCLEAR_Z );
	nglSetClearColor( 0, 0, 0, 0 );
	nglSetClearZ( 1.0f );

	int text_height = 0;
	int text_width = 0;
	unsigned int w = 0;
	unsigned int h = 0;
	const int border = 5;

	for( int i = 0; i < msg->num_lines; ++i ) {
		error_msg = msg->lines[i];
		TextString::MakeReplacements( error_msg );
		nglGetStringDimensions( nglSysFont, &w, &h, "\2[1.75]%s", error_msg.c_str( ) );

		if( w > text_width ) {
			text_width = w;
		}

		text_height += ( h + border );
	}

	int screen_height = nglGetScreenHeight( );
	int screen_width = nglGetScreenWidth( );

	int text_y = ( screen_height / 2 ) - ( text_height / 2 );
	int text_x = 0;

	for( int i = 0; i < msg->num_lines; ++i ) {
		error_msg = msg->lines[i];
		TextString::MakeReplacements( error_msg );
		nglGetStringDimensions( nglSysFont, &w, &h, "\2[1.75]%s", error_msg.c_str( ) );

		text_x = ( screen_width / 2 ) - ( w / 2 );
		nglListAddString( nglSysFont, text_x, text_y, 0.2f, 0xFFFFFFFF, "\2[1.75]%s", error_msg.c_str( ) );
		// increment, add some space
		text_y += ( h + border );
	}

	nglListEndScene( );
	nglListSend( true );
}

void KSGCReset( bool ipl )
{
	// Bad!
	GXSetDrawDoneCallback( NULL );
	VISetPostRetraceCallback( NULL );

	// 4.1 & 4.2 of Reset Guidelines.
	PADRecalibrate( PAD_CHAN0_BIT | PAD_CHAN1_BIT | PAD_CHAN2_BIT | PAD_CHAN3_BIT );

	// FIXME: 4.3 Ensure writing to Memory card is complete.

	// 4.4 Ensure graphics and sound are completed.
	nvlStopAllMovies( );
	nvlShutdown( );
	nslReleaseAllSounds( );
	nslShutdown( );

	// These services are boot-strapped from aram_mgr, but no
	// release method is provided.
	ARQReset( );
	ARReset( );

	// FIXME: We should probably do something with nglExit().
	GXAbortFrame( );
	GXDrawDone( );

	VISetBlack( TRUE );
	VIFlush( );
	VIWaitForRetrace( );

	if( ipl ) {
		OSResetSystem( OS_RESET_HOTRESET, OSGetResetCode( ), TRUE );
	} else {
		OSResetSystem( OS_RESET_RESTART, OSGetResetCode( ), FALSE );
	}

}

static void KSGCRetraceCallback( u32 count )
{
	static BOOL reset = FALSE;

	ksGCCurrentError = KS_GC_NO_ERROR;

	if( ksGCOldRetraceCallback ) {
		ksGCOldRetraceCallback( count );
	}

	s32 status = DVDGetDriveStatus( );
	ksGCCurrentError = KSStatusToError( status );

	if( OSGetResetButtonState( ) ) {
		reset = TRUE;
	} else if( reset ) {
		ksGCCurrentError = KS_GC_RESET;
	}

}

static int KSGCCheckB( void )
{
	// which controller we're looking at
	static int which = -1;

	PADStatus pads[PAD_MAX_CONTROLLERS];

	VIWaitForRetrace( );
	PADRead( pads );

	if( which == -1 ) {

		// find the an attached controller with the b button pressed
		for( int i = 0; i < PAD_MAX_CONTROLLERS; ++i ) {

			if( pads[i].err == PAD_ERR_NONE ) {

				if( pads[i].button & PAD_BUTTON_B ) {
					which = i;

					return 1;
				}

			}

		}

		return 0;
	}

	assert( which >= 0 );

	if( pads[which].err == PAD_ERR_NONE ) {

		if( pads[which].button & PAD_BUTTON_B ) {
			return 1;
		} else {
			return 0;
		}

	} else {
		return 0;
	}

}

// This is a very unfortunate global variable
// referenced from the PAL60 UI code.
bool ksGCQueryPAL60 = false;

void system_idle( void )
{
	static ksGCError last_error = KS_GC_NO_ERROR;

	if( ksGCCurrentError == KS_GC_RESET ) {
		KSGCReset( false );
	} else {

		// there is now an error
		if( ksGCCurrentError != KS_GC_NO_ERROR && last_error == KS_GC_NO_ERROR ) {
			nvlPauseAllMovies( );
			nslPauseAllSounds( );
		}

		KSGCDisplayError( ksGCCurrentError );

		// no error anymore
		if( ksGCCurrentError == KS_GC_NO_ERROR && last_error != KS_GC_NO_ERROR ) {
			nvlReloadMovies( );
			nvlUnpauseAllMovies( );
			nslUnpauseAllSounds( );

			// clear the EFB
			nglListInit( );
			nglListBeginScene( );
			nglSetClearFlags( NGLCLEAR_COLOR | NGLCLEAR_Z );
			nglSetClearColor( 0, 0, 0, 0 );
			nglSetClearZ( 1.0f );
			nglListEndScene( );
			nglListSend( true );
		}

	}

	if( last_error != ksGCCurrentError ) {
		last_error = ksGCCurrentError;
	}

}

static void application_startup( void )
{
	master_clock::create_inst( );
	g_master_clock_is_up = true;

#ifdef PROFILING_ON
	profiler::create_inst( );
#endif

	app::create_inst( );
}

#ifdef USER_MKV

// I'm in a fae mood today.
static const int __max_evil_entries = ( 3 * 60 );
static float __evil_entries[__max_evil_entries];
static int __evil_counter = 0;

static void application_run( void )
{
	int __evil_i;

	for( __evil_i = 0; __evil_i < __max_evil_entries; ++__evil_i ) {
		__evil_entries[__evil_i] = 0.0f;
	}

	while( !g_game_ptr->get_i_quit( ) )
	{
		master_clock::inst( )->tick( );

		if( ksGCCurrentError == KS_GC_NO_ERROR ) {
			OSTime __evil_then = OSGetTime( );
			app::inst( )->tick( );
			OSTime __evil_now = OSGetTime( );
			OSTime __evil_diff = __evil_now - __evil_then;
			float __evil_fps = (float) OS_TIMER_CLOCK / (float) __evil_diff;
			__evil_entries[__evil_counter] = __evil_fps;

			if( ++__evil_counter >= __max_evil_entries ) {
				char __evil_buf[64];
				float __evil_sum = 0.0f;
				int __evil_num = 0;
				
				for( __evil_i = 0; __evil_i < __max_evil_entries; ++__evil_i ) {
					__evil_sum += __evil_entries[__evil_i];
					++__evil_num;
				}

				float __evil_avg_fps = __evil_sum / (float) __evil_num;

				snprintf( __evil_buf, sizeof( __evil_buf ), "FPS: %f (%f).\n", __evil_fps, __evil_avg_fps );
				OSReport( __evil_buf );
				__evil_counter = 0;
			}

		}

		system_idle( );
	}

}

#else

static void application_run( void )
{

	while( !g_game_ptr->get_i_quit( ) )
	{
		master_clock::inst( )->tick( );

		if( ksGCCurrentError == KS_GC_NO_ERROR ) {
			app::inst( )->tick( );
		}

		system_idle( );
	}

}


#endif

static void application_shutdown( void )
{
	app::delete_inst( );

#ifdef PROFILING_ON
	profiler::delete_inst( );
#endif

	master_clock::delete_inst( );
	g_master_clock_is_up = false;
}

void KSHeapError( const char* Text )
{
	error(Text);
}

void KSCriticalError( const char* Text )
{
	OSHalt( const_cast<char*>( Text ) );
  asm( trap );
}

void KSDebugPrint( const char* Text )	//--- can't use nglPrintf from here, because it causes an infinite loop.
{
	OSReport( (char *) Text );
}

void* KSMemAllocate( u_int Size, u_int Align, const char *file, int line )
{
  return arch_memalign( Align, Size,file,line );
}

void* KSMemAlloc( u_int Size, u_int Align )
{
  return KSMemAllocate(Size,Align,"KSMemAlloc",0 );
}

void* KSMemAllocNGL( u_int Size, u_int Align )
{
	if ( Size==sizeof(nglMesh) ) // bigger
	{
  	return KSMemAllocate(Size,Align,"KSMemAllocNGL - nglMesh?",0 );
	}
	if ( Size==sizeof(nglTexture) ) // 128
	{
    return KSMemAllocate(Size,Align,"KSMemAllocNGL - nglTexture?",0 );
	}
  return KSMemAllocate(Size,Align,"KSMemAllocNGL",0 );
}

void* KSMemAllocNSL( u_int Size, u_int Align )
{
  return KSMemAllocate(Size,Align,"KSMemAllocNSL",0 );
}

void* KSMemAllocNVL( u_int Align, u_int Size )
{
  return KSMemAllocate(Size,Align,"KSMemAllocNVL",0 );
}
void KSMemFree( void* Ptr )
{
	arch_free(Ptr);
}

bool KSReadFile( const char* FileName, nglFileBuf* File, u_int Align )
{
  bool was_locked = false;
  if(os_file::is_system_locked())
  {
    os_file::system_unlock();
    was_locked = true;
  }
  bool rv=world_dynamics_system::wds_readfile(FileName,&File->Buf,&File->Size,Align);
  if(was_locked)
    os_file::system_lock();
  return rv;
}

bool KSReadFileNGL( const char* FileName, nglFileBuf* File, u_int Align )
{
	// Casting away const because of consistency problems among declarations of KSReadFile.
	// To be fixed later. (dc 12/06/01)
	return KSReadFile( (char*) FileName, File, Align );
}

void KSReleaseFile( nglFileBuf* File )
{
	world_dynamics_system::wds_releasefile(&File->Buf);
  memset( File, 0, sizeof(nglFileBuf) );
}

nglSystemCallbacks ngl_callbacks={
	&KSReadFileNGL,
	&KSReleaseFile,
	&KSCriticalError,
	&KSDebugPrint,
	&KSMemAllocNGL,
	&KSMemFree
};

nslSystemCallbackStruct nsl_callbacks = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&KSMemAlloc,
	&KSMemFree
};

void ks_verify_callback(GXWarningLevel level,
												 u32            id,
                         char*          msg)
{
	if ( id==94 ) return;
	warning( "GX verify error level %d, number %d\n",level,id );
	while ( strlen(msg)>79 )
	{
		char tmsg[81];
		strncpy(tmsg,msg,79);
		msg+=79;
		tmsg[79]=0;
		warning("%s",tmsg);
	}
	warning("%s",msg);
	if ( level<GX_WARN_MEDIUM )
	{
		asm( trap );
	}
}

union f2i_u
{
	double f;
	long long i;
};

#define FP_EX_ENABLE_MASK  0x0090

static void enable_fpu_exceptions( void )
{
#ifdef DEBUG
	register double control;
	union f2i_u d;

	PPCMtmsr( PPCMfmsr( ) | MSR_FE0 | MSR_FE1 );

	// Copy out of the fpscr
	asm { mffs control }

	// Copy out of the register -- we need this in the union for conversion
	d.f = control;
	// Turn on the bits specified by the mode
	d.i |= FP_EX_ENABLE_MASK;

	// Put back into the control register
	// It needs to be in a register to copy into the FPSCR
	control = d.f;

	// copy into the fpscr
	asm { mtfsf 255, control }
#endif
}

int main( int argc, char* argv[] )
{
	OSInit( );
	OSInitFastCast( );
	DVDInit( );
	VIInit( );
	PADInit( );
  CARDInit();

	// once to set things up
	KSGCCheckB( );

	aram_mgr::initialize( 0x600000, 0xa00000, 128 );

#ifdef DEBUG
  enable_fpu_exceptions( );
#endif

	nglSetSystemCallbacks( &ngl_callbacks );
	nslSetSystemCallbacks( &nsl_callbacks );

	// now start actually polling
	int b_held = KSGCCheckB( );

	os_alloc_init( );

	os_developer_options::create_inst( );

	error_context::create_inst( );

	nglSetBufferSize( NGLBUF_GX_FIFO, ( 512 * 1024 ) );
	nglSetBufferSize( NGLBUF_LIST_WORK, ( 512 * 1024 ) );

	b_held &= KSGCCheckB( );

	nglInit( );

	ksGCOldRetraceCallback = VISetPostRetraceCallback( KSGCRetraceCallback );
	nslGCSetDVDIdleFunc( system_idle );

	b_held &= KSGCCheckB( );

#ifdef DEBUG
	GXSetVerifyCallback( ks_verify_callback );
#else
  g_debug.assert_screen = true;
#endif

  low_level_console_init( );

	// KS proper.
	application_startup( );
	low_level_console_flush( );

	b_held &= KSGCCheckB( );
	int tv_mode = nglGetTVMode( );

	// If we're a PAL display and EURGB60 mode bit is set or B is held...
	if( ( tv_mode == NGLTV_PAL ) && ( OSGetEuRgb60Mode( ) || b_held ) ) {
		ksGCQueryPAL60 = true;
	}

	application_run( );
	application_shutdown( );

	low_level_console_release( );

	error_context::delete_inst( );

	os_developer_options::delete_inst( );

	os_alloc_shutdown( );

	return 0;
}

extern "C" char* strlwr( char* s )
{
	char* _s = s;

	assert( s );

	while( *s ) {
		*s = tolower( *s );
		s++;
	}

	return _s;
}

extern "C" char* strupr( char* s )
{
	char* _s = s;

	assert( s );

	while( *s ) {
		*s = toupper( *s );
		s++;
	}

	return _s;
}

extern "C" int strnicmp( const char* s1, const char* s2, int n )
{
	assert( s1 );
	assert( s2 );

	while( *s1 && *s2 && n-- ) {
		char c1 = tolower( *s1 );
		char c2 = tolower( *s2 );

		if( c1 < c2 ) {
			return -1;
		} else if( c1 > c2 ) {
			return 1;
		} else {
			s1++;
			s2++;
		}

	}

	if( n <= 0 ) {
		return 0;
	}

	if( *s1 ) {
		return 1;
	} else if( *s2 ) {
		return -1;
	} else {
		return 0;
	}

}

#endif // TARGET_GC
