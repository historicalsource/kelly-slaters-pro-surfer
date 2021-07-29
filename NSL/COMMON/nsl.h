/*************************************************************************
 * nsl.h
 *-----------------------------------------------------------------------*
 * Platform independent header for the New Sound Library (NSL).  Platform
 * specific extensions are defined in nsl_<platform>.h.
 *-----------------------------------------------------------------------*
 * Treyarch LLC Copyright 2001
 *************************************************************************/

#ifndef NSL_HEADER
#define NSL_HEADER

#define NSL
#define NSL_MAX_STR_LENGTH  32

// These should be defined in the project settings/compiler options
//
// #define NSL_LOAD_SOURCE_BY_ALIAS
// #define NSL_LOAD_SOURCE_BY_NAME

#include "ProjectOptions.h"


#if defined(NSL_LOAD_SOURCE_BY_ALIAS) && defined(NSL_LOAD_SOURCE_BY_NAME)
#error both NSL_LOAD_SOURCE_BY_ALIAS and NSL_LOAD_SOURCE_BY_NAME are defined!
#endif

#if !defined(NSL_LOAD_SOURCE_BY_ALIAS) && !defined(NSL_LOAD_SOURCE_BY_NAME)
#error none of NSL_LOAD_SOURCE_BY_ALIAS and NSL_LOAD_SOURCE_BY_NAME is defined!
#endif

#if defined(DEBUG) || defined(_DEBUG)
#define NSLDEBUG
#endif
// include platform-specific type definitions
#if defined (TARGET_PC)
#define NSL_PC
#ifndef NL_PC
#define NL_PC 1
#endif
#elif defined (SN_TARGET_PS2)
#define NSL_PS2
#ifndef NL_PS2
#define NL_PS2 1
#endif
#elif defined (TARGET_PS2)
#define NSL_PS2
#ifndef NL_PS2
#define NL_PS2 1
#endif
#elif defined (TARGET_XBOX)
#define NSL_XBOX
#ifndef NL_XBOX
#define NL_XBOX 1
#endif
#elif defined (TARGET_GC)
#define NSL_GC
#ifndef NL_GC
#define NL_GC 1
#endif
#elif defined (NSL_SOUND_TOOL)
// any sound-tool only definitions here
#else
#error Unknown platform
#endif

#include "nl.h"

// include platform-specific extensions
#if defined (TARGET_PC)
#include "../pc/nsl_pc_ext.h"
#pragma warning(disable:4786)     //  STL crap - symbol greater than 255 character,
#pragma warning(disable:4530)     //  STL crap - wants to have exception handling
#elif defined (SN_TARGET_PS2)
#elif defined (TARGET_PS2)
#elif defined (TARGET_XBOX)
#include "../xbox/nsl_xbox_ext.h"
#pragma warning(disable:4786)     //  STL crap - symbol greater than 255 character,
#pragma warning(disable:4530)     //  STL crap - wants to have exception handling
#elif defined (TARGET_GC)
#elif defined (NSL_SOUND_TOOL)
#else
#error Unknown platform
#endif

/*-------------------------------------------------------------------------
  nsl platform independent type definitions
-------------------------------------------------------------------------*/

typedef nlUint32 nslSourceId;
typedef nlUint32 nslSoundId;
typedef nlUint32 nslEmitterId;

#define NSL_DEFAULT_MIN_SOUND_DIST 10.0f
#define NSL_DEFAULT_MAX_SOUND_DIST 30.0f

typedef enum _nslSpeakerModeEnum
{
  NSL_SPEAKER_STEREO,
  NSL_SPEAKER_MONO,
  NSL_SPEAKER_HEADPHONE,
  NSL_SPEAKER_PROLOGIC,
  NSL_SPEAKER_DOLBY_51,

  NSL_SPEAKER_Z     // keep this as the last entry
} nslSpeakerModeEnum;

typedef enum _nslOutputModeEnum
{
  NSL_OUTPUT_ANALOG,
  NSL_OUTPUT_DIGITAL,
  NSL_OUTPUT_BOTH,

  NSL_OUTPUT_Z        // keep this as the last entry
} nslOutputModeEnum;

typedef enum _nslSourceTypeEnum
{
  NSL_SOURCETYPE_SFX = 0, // the 1st entry needs to be zero in order to iterate thru this list
  NSL_SOURCETYPE_AMBIENT,
  NSL_SOURCETYPE_MUSIC,
  NSL_SOURCETYPE_VOICE,
  NSL_SOURCETYPE_MOVIE,

  NSL_SOURCETYPE_USER1,
  NSL_SOURCETYPE_USER2,

  NSL_SOURCETYPE_Z    // keep this as the last entry
} nslSourceTypeEnum;

typedef enum _nslSoundParamEnum
{
  NSL_SOUNDPARAM_VOLUME,
  NSL_SOUNDPARAM_PITCH,
  NSL_SOUNDPARAM_MINDIST,
  NSL_SOUNDPARAM_MAXDIST,

  NSL_SOUNDPARAM_Z      // keep this as the last entry
} nslSoundParamEnum;

typedef enum _nslSoundStatusEnum
{
  NSL_SOUNDSTATUS_INVALID = 0,
  NSL_SOUNDSTATUS_QUEUING,
  NSL_SOUNDSTATUS_READY,
  NSL_SOUNDSTATUS_PLAYING,
  NSL_SOUNDSTATUS_PAUSED
} nslSoundStatusEnum;

typedef enum _nslLanguageEnum
{
  // (defined using a typedef so it can be used to index into
  //  an array of strings without explicit casts)

  // special language for use mostly in the nslSoundTool for language inspecific sounds
  NSL_LANGUAGE_NONE = 0,      // keep this as the first entry

  // The "Big 4" european languages
  NSL_LANGUAGE_ENGLISH,              // NSL default
  NSL_LANGUAGE_FRENCH,
  NSL_LANGUAGE_GERMAN,
  NSL_LANGUAGE_SPANISH,

  // Asian languages
  NSL_LANGUAGE_JAPANESE,
  NSL_LANGUAGE_CANTONESE,
  NSL_LANGUAGE_MANDARIN,

  // Phony languages to support Spider-Man's cheats.  Don't ask... <sigh>
  NSL_LANGUAGE_GG_ENGLISH,
  NSL_LANGUAGE_GG_FRENCH,
  NSL_LANGUAGE_GG_GERMAN,
  NSL_LANGUAGE_GG_SPANISH,

  NSL_LANGUAGE_Z      // keep this as the last entry
} nslLanguageEnum; // keep this enum parallel with the nslLanguageStr array in nsl.cpp

typedef enum _nslPlatformEnum
{
  // (defined using a typedef so it can be used to index into
  //  an array of strings without explicit casts)
  NSL_PLATFORM_PS2,
  NSL_PLATFORM_XBOX,
  NSL_PLATFORM_GAMECUBE,
  NSL_PLATFORM_PC,

  NSL_PLATFORM_Z      // keep this as the last entry
} nslPlatformEnum; // keep this enum parallel with the nslPlatformStr array in nsl.cpp


extern const char *nslLanguageStr[]; // defined in nsl.cpp
extern const char *nslPlatformStr[]; // defined in nsl.cpp
extern const char *nslSourceTypesStr[];

/*-------------------------------------------------------------------------
  nsl platform independent constants and macros
-------------------------------------------------------------------------*/


// Sources, sounds and emitters all share the same invalid id
#define NSL_INVALID_ID 0
#define NSL_GLOBAL_EMITTER_ID 0     // (currently use the same id as invalid)


/*---------------------------------------------------------------------------------------------------------
  System Callback API.

  Allows the game to override various system operations within NSL.  Defaults are provided.
---------------------------------------------------------------------------------------------------------*/
// File buffer for callback functions to fill.
struct nslFileBuf
{
  unsigned char* Buf;
  unsigned int Size;
};

// Structure of function pointers for NSL system callbacks.
struct nslSystemCallbackStruct
{
  bool (*ReadFile)( const char* fileName, nslFileBuf* file, unsigned int align );
  void (*ReleaseFile)( nslFileBuf* file );
  void (*CriticalError)( const char* text );  // Fatal, non recoverable
  void (*Error)( const char* text);           // Error, but we can continue
  void (*Warning) ( const char* text);        // Might be wrong
  void (*DebugPrint)( const char* text );
  void* (*MemAlloc)( unsigned int size, unsigned int align );
  void (*MemFree)( void* ptr );
};

/*---------------------------------------------------------------------------------------
  Actual procedures we call
---------------------------------------------------------------------------------------*/
void nslPrintf( const char* format, ...);
void nslFatal( const char* format, ... );   // Fatal, non recoverable
void nslError( const char* format, ... );   // Error, but we can continue
void nslWarning( const char* format, ... ); // Might be wrong
bool nslReadFile( const char* fileName, nslFileBuf* file, unsigned int align = 1 );
void nslReleaseFile( nslFileBuf* file );
void* nslMemAlloc( unsigned int size, unsigned int align = 1 );
void nslMemFree( void* ptr );


/*---------------------------------------------------------------------------------------------------------
  nslSetSystemCallbacks

  Overrides default system operations.  Passing in a structure of function pointers will override the
  current system callbacks and cause NSL to use the new ones for all operations.

  NULL can passed in any of the pointers, which will cause the NSL default implementation to be used.
---------------------------------------------------------------------------------------------------------*/
void nslSetSystemCallbacks( nslSystemCallbackStruct* Callbacks );
void nslSetRootDir( const char *);





/*-------------------------------------------------------------------------
  nsl system functions
-------------------------------------------------------------------------*/
#ifdef NSLDEBUG
void nslToggleOnOff();
#endif
bool nslInit( );
// returns true on success
bool nslReset( const char *soundListfile, nslLanguageEnum language = NSL_LANGUAGE_Z );
// Doesn't load a bank, just cleans up
void nslReset( );
// Returns true on success
bool nslPushBank( const char *soundListfile, nslLanguageEnum language = NSL_LANGUAGE_Z );
// returns false if there are no banks to pop
bool nslPopBank();
void nslShutdown();
void nslFrameAdvance( float timeElapsed );

// between 0.0 and 1.0
void  nslSetMasterVolume( float newVolume );
float nslGetMasterVolume();

void  nslSetVolume( nslSourceTypeEnum typeOfSound, float newVolume );
float nslGetVolume( nslSourceTypeEnum typeOfSound);

nslSourceTypeEnum nslSourceTypeStringToEnum( const char *stringToLookup );

void nslSetListenerPo( const nlMatrix4x4 &positionAndOrientation );
void nslGetListenerPo( nlMatrix4x4 *dest );

void nslSetListenerPosition( const nlVector3d &newPosition );
void nslSetListenerOrientation( const nlVector3d &facingVector, const nlVector3d &upVector );


void nslSetSpeakerMode( nslSpeakerModeEnum newMode );
nslSpeakerModeEnum nslGetSpeakerMode( );
void nslSetOutputMode( nslOutputModeEnum newMode );


/*-------------------------------------------------------------------------
  nsl source functions
-------------------------------------------------------------------------*/

typedef enum
{
  NSL_STREAM_FORMAT_INVALID,
  NSL_STREAM_FORMAT_ADPCM,
  NSL_STREAM_FORMAT_PCM16,
  NSL_STREAM_FORMAT_PCM8
} nslStreamFormatEnum;

typedef struct
{
  int         sampleRate;
  nslStreamFormatEnum sampleFormat;
  int         channelCount;
  int         chunkSize;
} nslStreamInfo;

#ifdef NSL_LOAD_SOURCE_BY_NAME
nslSourceId nslLoadSource( const char *soundSourceName );
nslSourceId nslStreamLoadSource( const char *soundSourceName, const char *streamFilename, const nslStreamInfo *info );
nslSourceId nslGetSource( const char *soundSourceName, bool fatal );
const char *nslGetSourceName( nslSourceId whichSource );
#endif

#ifdef NSL_LOAD_SOURCE_BY_ALIAS
nslSourceId nslLoadSource( nlUint16 soundSourceID );
nslSourceId nslStreamLoadSource( nlUint16 soundSourceID, const char *streamFilename, const nslStreamInfo *info );
nslSourceId nslGetSource( nlUint16 soundSourceID, bool fatal );
nlUint16    nslGetSourceName( nslSourceId whichSource );
#endif
nslSourceId nslGetSourceByIndex(int index);
float       nslGetSourceLength( nslSourceId whichSource );
float       nslGetPaddedSourceLength( nslSourceId whichSource );

float       nslGetSourceParam( nslSourceId whichSource, nslSoundParamEnum whichParam );
void        nslSetSourceParam( nslSourceId soundToSet, nslSoundParamEnum whichParam, float newVal );

void nslSetReverb(nslSourceId whichSource, bool new_setting);
bool nslGetReverb(nslSourceId whichSource);


/*-------------------------------------------------------------------------
  nsl sound (aka SoundInstance) functions
-------------------------------------------------------------------------*/

// all 'level' parameters are in range 0 to 1,
// with 1.0 representing the "natural" value
// (pitch would go above 1, in the case of increasing
// pitch above it's natural, base pitch)
nslSoundId  nslAddSound( nslSourceId soundSource );
nslSoundId  nslStreamAddSound( nslSourceId soundSource, int streamOffset, int streamSamples );
nslSoundId  nslAddSoundWithOffset( nslSourceId soundSource, float seconds );
void        nslPlaySound( nslSoundId soundToPlay );
void        nslPlaySound3D( nslSoundId soundToPlay, const nlVector3d &pos );
void        nslStopSound( nslSoundId soundToStop );
void        nslReleaseAllSounds();

nslSoundStatusEnum nslGetSoundStatus( nslSoundId whichSound );

float       nslGetSoundPlaybackPosition( nslSoundId s );

bool        nslIsSoundReady( nslSoundId whichSound );
bool        nslIsSoundPlaying( nslSoundId whichSound );

void        nslPauseSound( nslSoundId whichSound );
void        nslUnpauseSound( nslSoundId whichSound );
void        nslPauseAllSounds();
void        nslUnpauseAllSounds();
void        nslPauseGuardSound( nslSoundId soundToGuard );

void        nslDampenGuardSound( nslSoundId soundToGuard );
void        nslDampenAllSounds( float dampenLevel );
void        nslUndampenAllSounds();

void        nslSetSoundRange( nslSoundId soundToSet, float minDist, float maxDist);

void        nslSetSoundParam( nslSoundId soundToSet, nslSoundParamEnum whichParam, float newVal );

float       nslGetSoundParam( nslSoundId whichSound, nslSoundParamEnum whichParam );


/*-------------------------------------------------------------------------
  nsl emitter functions
-------------------------------------------------------------------------*/

nslEmitterId nslCreateEmitter( const nlVector3d &position );
nslEmitterId nslCreateLineEmitter( const nlVector3d &startPosition, const nlVector3d &endPosition );
void         nslReleaseEmitter( nslEmitterId emitterToRelease );
void         nslSetEmitterPosition( nslEmitterId emitterToSet, const nlVector3d &newPosition );
void         nslSetEmitterAutoRelease( nslEmitterId emitterToSet );

// all instances start with the default 'global' emitter
// this re-assigns an instance to a different one
void         nslSetSoundEmitter( nslEmitterId soundEmitter, nslSoundId soundInstance );


/*-------------------------------------------------------------------------
  Internal functions
  - perhaps move these to an nsl_internal.h?
-------------------------------------------------------------------------*/
int _nslLocalizedPath( const char* file_path, const char* file_name, nslLanguageEnum lang,
                       nslPlatformEnum platform, char* found_filename, char* path_addendum,
                       bool *used_lang = 0 );


#endif
