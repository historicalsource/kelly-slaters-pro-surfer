#ifndef NSL_PS2_HEADER
#define NSL_PS2_HEADER

#include <string.h>
#include <stdio.h>
#include "common/nsl.h"
#include "ps2/gas.h"
#include "ps2/gas_utility.h"
#include "ps2/fifo_queue.h"





const nlUint32 NSL_PS2_SMALL_STRING_LENGTH = 255;



// float to int conversion
inline int nslFTOI( float input )  // doesn't round according to C standard
{
  register float output;
  __asm__ volatile ("cvt.w.s %0, %1" : "=f" (output) : "f" (input) );
  return (int&)output;
//  f += 3 << 22;
//  int n=((*(int*)&f)&0x007fffff) - 0x00400000;
//  return n;
}


/*-------------------------------------------------------------------------
  nsl platform dependent constants and macros
-------------------------------------------------------------------------*/
#ifdef SPIDERMAN
#define NSL_NUM_SOURCES       512
#define NSL_NUM_EMITTERS      128
#define NSL_NUM_SOUNDS_PER_EMITTERS 16
#define NSL_MAX_QUEUED_SOUNDS 64
#define NSL_NUM_SOUNDS        196


#define NSL_ID_BITS	  		10
#define NSL_ID_INCREMENT  0x00000400
#define NSL_ID_MASK	  		0xFFFFFC00
#define NSL_SLOT_MASK     0x000003FF


#define NSL_SOURCE_ID_BITS       16
#define NSL_SOURCE_ID_INCREMENT  0x00010000
#define NSL_SOURCE_ID_MASK       0xFFFF0000
#define NSL_SOURCE_SLOT_MASK     0x0000FFFF

#define NSL_GET_SLOT_FROM_ID( x ) ( x&NSL_SLOT_MASK )
#define NSL_GET_SOURCE_SLOT_FROM_ID( x ) ( x&NSL_SOURCE_SLOT_MASK )


#else
#define NSL_NUM_SOURCES       512
#define NSL_NUM_EMITTERS      256
#define NSL_NUM_SOUNDS_PER_EMITTERS 16
#define NSL_MAX_QUEUED_SOUNDS 64
#define NSL_NUM_SOUNDS        256


#define NSL_ID_BITS	  		10
#define NSL_ID_INCREMENT  0x00000400
#define NSL_ID_MASK	  		0xFFFFFC00
#define NSL_SLOT_MASK     0x000003FF
#define NSL_GET_SLOT_FROM_ID( x ) ( x&NSL_SLOT_MASK )

#define NSL_SOURCE_ID_BITS       16
#define NSL_SOURCE_ID_INCREMENT  0x00010000
#define NSL_SOURCE_ID_MASK       0xFFFF0000
#define NSL_SOURCE_SLOT_MASK     0x0000FFFF

#define NSL_GET_SLOT_FROM_ID( x ) ( x&NSL_SLOT_MASK )
#define NSL_GET_SOURCE_SLOT_FROM_ID( x ) ( x&NSL_SOURCE_SLOT_MASK )



#endif

// Ok.. so now this is the short sound based on 
// seconds
#define NSL_SMALL3D_SOUND_SIZE .5f  // Dont ask

// the way sound ID works is that higher bits is the unique ID given out 
// by the system every time a new sound is requested while the lower 8 
// bits are used to determine which voice channel the sound is intended for.
// The number of bits puts a limit on how many simultaneous sounds/emitters
// we can have

void _nslCheckSoundIdValidity(nslSoundId checkId);
void _nslCheckSourceIdValidity(nslSourceId checkId);
void _nslCheckEmitterIdValidity(nslEmitterId checkId);
void _nslInitialized();

#define ASSERT_SOUND_ID_VALID(checkId) \
  _nslCheckSoundIdValidity(checkId)

#define ASSERT_SOURCE_ID_VALID(checkId) \
  _nslCheckSourceIdValidity(checkId)

#define ASSERT_EMITTER_ID_VALID(checkId) \
  _nslCheckEmitterIdValidity(checkId)

#define ASSERT_NSL_INITIALIZED() \
  _nslInitialized()

enum nslPs2CdDvdMode
{
  NSL_PS2_CD_MODE  = 0, // GAS uses these numbers in its init to tell the mode
  NSL_PS2_DVD_MODE = 1
};

void nslPreInitProviewModePS2(bool on);
void nslPreInitCdDvdModePS2(nslPs2CdDvdMode mode);
void nslPreInitSetGasNamePS2(const char *name);
void nslPreInitSetModDirPS2(const char *directory);
void nslSetHostStreamingPS2(bool on);

/*-------------------------------------------------------------------------
  nsl implementation data types
-------------------------------------------------------------------------*/
struct nslSource 
{
  bool          used;
	bool          looping;

#ifdef NSL_LOAD_SOURCE_BY_NAME

  char				  fileName[FILENAME_LENGTH];

#endif

#ifdef NSL_LOAD_SOURCE_BY_ALIAS

  int aliasID;

#endif
  int           bank;
  nslSourceId   myId;               // the current id of this source slot (for error-checking)
  float         length;
  float         paddedLength;
  nslSourceTypeEnum	type;
  nlUint32        gasSourceId;
  float         rawVolume;
//  float         balance;
  float         pitch;
  float         minDist;
  float         maxDist;
  
};


struct nslSound
{
public:
	bool isReady;
  bool          used;
  bool          inRange;
  bool          isReallyReady;
  
  //  Flags
  bool          isPlaying;        // True regardless of range
  bool          isReallyPlaying;  // Is GAS playing the sound?
  bool          looping;     // Is this a looping sound?
  bool          isQueuing;

  short         left;             // Post processing volume values
  short         right;
  short         old_left;
  short         old_right;
  
  nlUint32        gasInstanceId;
  nlInt32         pauseCount;      // -1 if guarded, 0 if unpaused, >0 if paused
  nlInt32         dampenCount;     // -1 if guarded, 0 if undampened, +1 if dampened
  nslSourceId   myId;             // the current id of this sound slot (for error-checking)
  nslSourceId		mySource;
  nslEmitterId  myEmitter;
 
  float         rawVolume;        // volume value before modification (position, etc.)
  float         positionalVolume; 

  float         angle;            // Angle from emitter in radians
  
  float         pitch;
//  float         balance;
  float         minDist;
  float         maxDist;

};
/*
struct nslSourceInfo
{
  char name[30];
  bool looping;
  nslSourceTypeEnum sourceType;
};
*/


struct nslEmitter
{
	
  bool          used;
  bool          autoRelease;
  bool          isALineEmitter;
  
  nslEmitterId	myId;    // the current id of this emitter slot (for error-checking)
  nlVector3d    startPosition; // only used on line-emitters
  nlVector3d    endPosition;
  nlVector3d    position;

  fifo_queue<nslSoundId>    emittedSounds;
};


struct nslSystem
{
	// hardware
	
	// resources
	// any pointer should allocated only once during initialization -
	// they should be viewed as static arrays
  nslSystem() { firstInit = 1; on = false; initialized = 0; };
	nlMatrix4x4 					listenerPo;

	nslSource					  sourceSlots[NSL_NUM_SOURCES];
	nslSound						soundSlots[NSL_NUM_SOUNDS];
  nslEmitter					emitterSlots[NSL_NUM_EMITTERS];
  
  nlUint32              sourceSlotsUsedCount;
  nlUint32              soundSlotsUsedCount;
  nlUint32              emitterSlotsUsedCount;
	nlInt32               firstInit;
	nlInt32               initialized;
  nlUint32              numBanks;

  fifo_queue<nslSoundId>  queuedSounds;
	// states/flags
  nslSpeakerModeEnum			speakerMode;
	nslOutputModeEnum				outputMode;
  nslLanguageEnum         language;
  nslPs2CdDvdMode         cdDvdMode;


  // For the timer
  nlUint64            lastClock;
  
  bool                on;
  bool                proview;
  bool                finalizeSourcesEnabled;

	float	      				sfxVolume;
	float	      				ambientVolume;
	float	      				musicVolume;
	float	      				voiceVolume;
  float               movieVolume;
  float               user1Volume;
  float               user2Volume;
  float               masterVolume;
//	float		      			balance;
  float               dampenLevel;
  char                rootDir[32];
};

extern nslSystem nsl;

#ifdef _DEBUGNSL_
extern bool NSLasserted;
#endif

// Tells the generic reset functionality what type of clear to do
typedef enum {
  NSL_CLEAR_INIT,
  NSL_CLEAR_RESET,
  NSL_CLEAR_FREE,
} _nslClearBehaviour;


/*-------------------------------------------------------------------------
  nsl internal functions
-------------------------------------------------------------------------*/

// Some utility stuff for iterating through emitters, sounds, sources, etc
typedef int (*_nslEmitterCallback)( nslEmitter* emitter, void *userData );
// call cb for every active emitter
void _nslEmitterForEach( _nslEmitterCallback cb, void *userData  );


typedef int (*_nslSoundCallback)( nslSound* emitter, void *userData  );
// call cb for every active sound
void _nslSoundForEach( _nslSoundCallback cb, void *userData  );

typedef int (*_nslSourceCallback)( nslSource* emitter, void *userData  );
// call cb for every active sound
void _nslSourceForEach( _nslSourceCallback cb, void *userData  );



bool _nslLoadModule(char *);

void				_nslReleaseAllSources();
void				_nslReleaseAllEmitters();

void _nslResetInternal( _nslClearBehaviour clearStyle );

void _nslClearSystemData( _nslClearBehaviour clearStyle );
void _nslClearSourceSlot( nlUint32 whichSlot, bool resetId );
void _nslClearSoundSlot( nlUint32 whichSlot, bool resetId );
void _nslClearEmitterSlot( nlUint32 whichSlot, bool resetId, _nslClearBehaviour clearStyle );
void _nslLoadSourceData(nslFileBuf file);
bool _set3dVolume(nslSoundId whichSound);
bool _setSoundPosVolume(nslSoundId whichSound, float *distSq );
void _setSoundAngle(nslSoundId whichSound);

inline void _nslSafeStrncpy( char *targetString, char *sourceString, nlUint32 amountToCopy)
{
  strncpy( targetString, sourceString, amountToCopy );
  targetString[amountToCopy-1] = '\0';
}

inline void _nslCheckSoundIdValidity(nslSoundId checkId)
{
// unused?   nlUint32 gasId = nsl.soundSlots[checkId&NSL_SLOT_MASK].gasInstanceId;
// unused?   nlUint32 negativeOne = (nlUint32)-1;
  nslSoundId checkSlot = (checkId & NSL_SLOT_MASK);
  nslSoundId upCheckId = (checkId & NSL_ID_MASK);
  if (checkId == NSL_INVALID_ID)
    nslFatal("Bad Sound ID 0x%x\n", checkId);
  if (!(nsl.soundSlots[checkSlot].used))
    nslFatal("Bad Sound ID 0x%x\n", checkId);
  if (!(checkSlot < NSL_NUM_SOUNDS))
    nslFatal("Bad Sound ID 0x%x\n", checkId);
  if (!(checkId == nsl.soundSlots[checkSlot].myId))
    nslFatal("Bad Sound ID 0x%x\n", checkId);
  if (!(upCheckId > 0))
    nslFatal("Bad Sound ID 0x%x\n", checkId);
  if (checkSlot > NSL_NUM_SOUNDS)
    nslFatal("Bad Sound ID 0x%x SLOT NUMBER ABOVE SLOTS\n", checkId);
  
}

inline void _nslCheckSourceIdValidity(nslSourceId checkId)
{
  // Changed to deal with bad sources a bit softer
  // Just print an error and ignore
  nslSourceId checkSlot = (checkId & NSL_SOURCE_SLOT_MASK);
  nslSourceId upCheckId = (checkId & NSL_SOURCE_ID_MASK);
  nlUint32 gasId = nsl.sourceSlots[checkSlot].gasSourceId;

  nlUint32 negativeOne = (nlUint32)-1;
  if (!(checkId != NSL_INVALID_ID)) 
  {
    nslFatal("This source id is the INVALID id");
  }
  if (!(gasId != negativeOne))
  {
    nslFatal("This source has a bad GAS id");
  }
  if (!(checkSlot < NSL_NUM_SOURCES))
  {
    nslFatal("This source is above the max number of sources");
  }
  if (!(checkId == nsl.sourceSlots[checkSlot].myId))
  {
    nslFatal("This ID passed does not match the id for the source slot");
  }
  if (!(upCheckId > 0))
  {
    nslFatal("THE INSTANCE PART OF THE ID IS NOT > 0");
  }
  if (checkSlot > NSL_NUM_SOURCES)
  {
    nslFatal("THE SLOT PART IS MORE THAN THE NUMBER OF TOTAL SLOTS");
  }
  return;
}
inline void _nslInitialized() {
  if (!(nsl.initialized == 1))
    nslFatal("NSL NOT INITIALIZED");
}
inline void _nslCheckEmitterIdValidity(nslEmitterId checkId)
{
  if (checkId == NSL_GLOBAL_EMITTER_ID) return;
  nslEmitterId checkSlot = (checkId & NSL_SLOT_MASK);
  nslEmitterId upCheckId = checkId&NSL_ID_MASK;
  if (!(checkId != NSL_INVALID_ID))
    nslFatal("Bad Emitter ID 0x%x\n", checkId);
  if (!(checkSlot < NSL_NUM_EMITTERS))
    nslFatal("Bad Emitter ID 0x%x\n", checkId);
  if (!(checkId == nsl.emitterSlots[checkSlot].myId))
    nslFatal("Bad Emitter ID 0x%x\n", checkId);
  if (!(upCheckId > 0))
    nslFatal("Bad Emitter ID 0x%x\n", checkId);
  if (checkSlot > NSL_NUM_EMITTERS)
    nslFatal("Bad Sound ID 0x%x SLOT NUMBER ABOVE SLOTS\n", checkId);
}



#ifdef _DEBUGNSL_
  int checkSoundId(nslSoundId);
  int checkSourceId(nslSourceId);
  int checkEmitterId(nslEmitterId);
#endif

void nslEnableFinalizeSourcesPS2( bool enable );

#endif
