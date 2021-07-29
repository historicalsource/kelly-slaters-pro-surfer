#ifndef NSL_GC_HEADER
#define NSL_GC_HEADER

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

#ifndef NSL_NO_ASSERT_H
#include <assert.h>
#endif

#include <dolphin/dvd.h>
#include <dolphin/os.h>
#include <dolphin/arq.h>
#include <dolphin/ax.h>

#include "common/nsl.h"
#include "gamecube/nsl_array.h"

enum {
	NSL_NUM_STREAMS = 16
};

// this is a temporary solution
// common/nsl.h should contain macro
// but for now it isn't
#ifndef	NSL_ASSERT
#define	NSL_ASSERT(x) assert(x)
#endif

//
// Other things you can define:
//

// Play sounds immediately rather than using the
// play queue.
//#define NSL_NO_PLAY_QUEUE 1

#ifdef	NSL_LOAD_SOURCE_BY_NAME
typedef	const char* nslMoniker;
#define	NSL_CHECK_MONIKER(moniker) NSL_ASSERT(moniker)
#endif

#ifdef	NSL_LOAD_SOURCE_BY_ALIAS
typedef	nlInt32 nslMoniker;
#define	NSL_CHECK_MONIKER(moniker) NSL_ASSERT((moniker)>=0)
#endif

//#define NSL_VERBOSE 1
//#define NSL_VERBOSE_PLAY 1
//#define NSL_VERBOSE_DROP 1
//#define NSL_VERBOSE_PAUSE 1
//#define NSL_VERBOSE_STREAM 1

// this needs to be defined to 1 or 0, not undefined
#define NSL_ENSURE_WEAK 1

#if defined(DEBUG)
#define	nslDebugPrintf nslPrintf
#else
#define	nslDebugPrintf if(0)
#endif

#if defined(NSL_VERBOSE)
#define	nslVerbosePrintf nslPrintf
#else
#define nslVerbosePrintf if(0)
#endif

#if defined(NSL_VERBOSE) || defined(DEBUG)
#define	nslVerboseDebugPrintf nslPrintf
#else
#define nslVerboseDebugPrintf if(0)
#endif

#if defined(NSL_VERBOSE) || defined(NSL_VERBOSE_PLAY)
#define	nslVerbosePlayPrintf nslPrintf
#else
#define nslVerbosePlayPrintf if(0)
#endif

#if defined(NSL_VERBOSE) || defined(NSL_VERBOSE_DROP)
#define	nslVerboseDropPrintf nslPrintf
#else
#define nslVerboseDropPrintf if(0)
#endif

#if defined(NSL_VERBOSE) || defined(NSL_VERBOSE_STREAM)
#define nslVerboseStreamPrintf nslPrintf
#else
#define nslVerboseStreamPrintf if(0)
#endif

#if defined(NSL_VERBOSE) || defined(NSL_VERBOSE_PAUSE)
#define	nslVerbosePausePrintf nslPrintf
#else
#define nslVerbosePausePrintf if(0)
#endif

/*-------------------------------------------------------------------------
  nsl platform dependent constants and macros
-------------------------------------------------------------------------*/
#define NSL_NUM_SOURCES        512
#define NSL_NUM_EMITTERS       256
#define NSL_NUM_SOUNDS         64
#define NSL_NUM_EMITTER_SOUNDS 16

// the way sound ID works is that higher bits is the unique ID given out 
// by the system every time a new sound is requested while the lower 8 
// bits are used to determine which voice channel the sound is intended for.
#define NSL_ID_INCREMENT  0x00000400
#define NSL_ID_MASK       0xFFFFFC00
#define NSL_SLOT_MASK     0x000003FF

#define NSL_GET_SLOT_FROM_ID(x)	((x)&NSL_SLOT_MASK)

#define NSL_SOURCE_NAME_SIZE	32

#define AR_ADDR_HI(x)  ((u16)(((x)&0xFFFF0000)>>16))
#define AR_ADDR_LO(x)  ((u16)((x)&0x0000FFFF))
#define AR_ADDR(lo,hi) ((((u32)(hi))<<16)|(((u32)(lo))&0x0000FFFF))

/*-------------------------------------------------------------------------
  nsl platform dependent functions
-------------------------------------------------------------------------*/

// this is called when we're waiting on a DVD transaction
typedef void (*nslGCDVDIdleFunc)( void );

void nslGCSetDVDIdleFunc( nslGCDVDIdleFunc f );

/*-------------------------------------------------------------------------
  nsl implementation data types
-------------------------------------------------------------------------*/
#define	NSL_SOURCE_INFO_STREAM        0x00000001	
#define NSL_SOURCE_INFO_STEREO_STREAM	0x00000002
#define NSL_SOURCE_INFO_LOOPING       0x00000004
#define	NSL_SOURCE_INFO_REVERB        0x00000008

/* Sundry load-time data for a given source. */
struct nslSourceInfo
{
	nlUint32 alias;
	char name[180];

	nlUint32 flags;
	nlUint32 offset;
	nlUint32 samples;
	nlUint32 frequency;
	nslSourceTypeEnum	type;
	float volume;

	AXPBADPCM adpcm;
	AXPBADPCMLOOP loop;
};

/* Forward declarations. */
struct nslSource;
struct nslSound;
struct nslEmitter;

/* Run-time source information. */
struct nslSource
{
	nslSourceId id;
	nslSourceInfo* info;

	float volume;
	float pitch;
	float minDist;
	float maxDist;
	
	bool loaded;
	bool used;
	
	int	streamId;	// here streams are opened with nslStreamLoadSource
};

struct nslSound
{
	nslSourceId id;
	
	nslSource* source;
	nslEmitter* emitter;

	bool used;
	bool isPlaying;
	bool isReallyPlaying;
	bool needVolumeUpdate;
	bool frameDelayed;
#ifdef DEBUG
	bool dropped;
#endif

	nlInt32 pauseCount;   // -1 if guarded, 0 if unpaused, >0 if paused
	nlInt32 dampenCount;  // -1 if guarded, 0 if undampened, +1 if dampened

	float rawVolume;
	float angle;
	float pitch;
	float minDist;
	float maxDist;

	AXVPB* vpb;

	int	commonStreamId;	// here streams are opened with normal call nslAddSound() (if they are streamed that is)
};

struct nslEmitter
{
	nslEmitterId id;

	nlVector3d position;
	nslSound* sounds[NSL_NUM_EMITTER_SOUNDS];
	nlUint32 numSounds;

	bool autoRelease;
	bool used;
};

struct nslSystem
{
	nlMatrix4x4 listenerPo;

	nslSource sources[NSL_NUM_SOURCES];
	nslSound sounds[NSL_NUM_SOUNDS];
	nslEmitter emitters[NSL_NUM_EMITTERS];
	nslSourceInfo* infos;

	nlUint32 sourceCount;
	nlUint32 soundCount;
	nlUint32 emitterCount;
	nlUint32 infoCount;

	nsl_array<nslSoundId> playQueue;

	nslSpeakerModeEnum speakerMode;
	nslOutputModeEnum outputMode;

	float sfxVolume;
	float ambientVolume;
	float musicVolume;
	float voiceVolume;
	float movieVolume;
	float masterVolume;
	float dampenLevel;

	nslSound* stereoSound;

	char root[32];
	nslLanguageEnum lang;

	nlUint32 aramBaseAddress;
	nlUint32 aramSize; 
	nlUint32 aramNullAddress;
	nlUint32 aramUsableAddress;

	bool initialized;
	bool on;

	// FIXME: lovely anonymous struct
	struct {
		nlUint32 audioBufferStart;
		nlUint32 audioBufferSize;
	} streams[ NSL_NUM_STREAMS ];

	nslSourceInfo streamSourceInfo;
};

// Our global nslSystem object for affecting system
// state from other translation modules (source files).
extern nslSystem nsl;

// Macros for trivial exit from public functions
// that could be called when NSL isn't initialized. 
#define NSL_ENSURE_ON() \
	do { \
		if( !nsl.on ) { \
			return; \
		} \
		if( nsl.initialized <= 0 ) { \
			assert( 0 ); \
			return; \
		} \
	} while( 0 )

#define NSL_ENSURE_ON_VAL(x) \
	do { \
		if( !nsl.on ) { \
			return (x); \
		} \
		if( nsl.initialized <= 0 ) { \
			assert( 0 ); \
			return (x); \
		} \
	} while( 0 )
	
#define NSL_ENSURE_SID(x) \
	do { \
		if( (x) == NSL_INVALID_ID ) { \
			return; \
		} \
		nlUint32 __tmp_sid = NSL_GET_SLOT_FROM_ID((x)); \
		assert( __tmp_sid < NSL_NUM_SOUNDS ); \
		nslSound* __tmp_sound = &nsl.sounds[__tmp_sid]; \
		assert( NSL_ENSURE_WEAK || (x) == __tmp_sound->id ); \
		if( (x) != __tmp_sound->id ) { \
			return; \
		} \
	} while( 0 )

#define NSL_ENSURE_SID_VAL(x,y) \
	do { \
		if( (x) == NSL_INVALID_ID ) { \
			return (y); \
		} \
		nlUint32 __tmp_sid = NSL_GET_SLOT_FROM_ID((x)); \
		assert( __tmp_sid < NSL_NUM_SOUNDS ); \
		nslSound* __tmp_sound = &nsl.sounds[__tmp_sid]; \
		assert( NSL_ENSURE_WEAK || (x) == __tmp_sound->id ); \
		if( (x) != __tmp_sound->id ) { \
			return (y); \
		} \
	} while( 0 )

#define NSL_ENSURE_EID(x) \
	do { \
		nlUint32 __tmp_eid = NSL_GET_SLOT_FROM_ID((x)); \
		assert( __tmp_eid < NSL_NUM_EMITTERS ); \
		nslEmitter* __tmp_emitter = &nsl.emitters[__tmp_eid]; \
		assert( NSL_ENSURE_WEAK || (x) == __tmp_emitter->id ); \
		if( (x) != __tmp_emitter->id ) { \
			return; \
		} \
	} while( 0 )

#define NSL_ENSURE_RID(x) \
	do { \
		nlUint32 __tmp_rid = NSL_GET_SLOT_FROM_ID((x)); \
		assert( __tmp_rid < NSL_NUM_SOURCES ); \
		nslSource* __tmp_source = &nsl.sources[__tmp_rid]; \
		assert( NSL_ENSURE_WEAK || (x) == __tmp_source->id ); \
		if( (x) != __tmp_source->id ) { \
			return; \
		} \
	} while( 0 )

#define NSL_ENSURE_RID_VAL(x,y) \
	do { \
		nlUint32 __tmp_rid = NSL_GET_SLOT_FROM_ID((x)); \
		assert( __tmp_rid < NSL_NUM_SOURCES ); \
		nslSource* __tmp_source = &nsl.sources[__tmp_rid]; \
		assert( NSL_ENSURE_WEAK || (x) == __tmp_source->id ); \
		if( (x) != __tmp_source->id ) { \
			return (y); \
		} \
	} while( 0 )

/*-------------------------------------------------------------------------
  nsl internal functions
-------------------------------------------------------------------------*/
void _nslReleaseAllSources( void );
void _nslReleaseSound( nlUint32 slot );
void _nslReleaseAllSounds( void );
void _nslReleaseAllEmitters( void );
void _nslUpdateVolume( nslSound* sound );
void _nslReallyUpdateVolume( nslSound* sound );
void _nslUpdateVolumeOfType( nslSourceTypeEnum type );
void _nslUpdateVolumeAll( void );
void _nslUpdateVolumeGlobal( nslSound* sound, nslSource* source );
void _nslUpdateVolume3D( nslSound* sound, nslSource* source );

void _nslEmitterRemoveSound( nslEmitter* emitter, nslSound* sound );

void _nslCalcTwoChannel( nslSound* sound, nslSource* source,	f32& left, f32& right );
int	 _nslGetStreamId( nslSound *sound, int *_isCommon = 0 );

nslSourceInfo* _nslGetSourceInfo( nslMoniker aliasOrName );

int _nslCalcDecibelAtten( float v )
{
	int i = 0;

	if( v != 0.0f ) {
		float f = 10.0f * log10f( v );
		// arbitrary MIX API scaling factor
		i = (int) ( 25.0f * f );
	} else {
		i = -960;
	}

	return i;
}

inline char* _nslSafeStrncpy( char* dst, const char* src, size_t n )
{
	char* s = strncpy( dst, src, n );
	dst[ n - 1 ] = '\0';
	return s;
}

inline int _nslStrCaseCmp( const char* s1, const char* s2 )
{
	while( *s1 && *s2 ) {
		char c1 = (char)tolower( *s1 );
		char c2 = (char)tolower( *s2 );
	
		if( c1 < c2 ) {
			return -1;
		} else if( c1 > c2 ) {
			return 1;
		} else {
			s1++;
			s2++;
		}
	
	}

	if( *s1 ) {
		return 1;
	} else if( *s2 ) {
		return -1;
	} else {
		return 0;
	}

}

#endif