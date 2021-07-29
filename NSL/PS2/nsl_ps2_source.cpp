#include "common\nsl.h"
#include "ps2\nsl_ps2.h"
#include "ps2\gas_utility.h"
#include "ps2\gas.h"
#include "string.h"
///////////////////////// nsl source api functions ////////////////////////

#ifdef NSL_LOAD_SOURCE_BY_NAME
/*-------------------------------------------------------------------------
  nsl Load Source
-------------------------------------------------------------------------*/
static nslSourceId _nslLoadSource( const char *soundSourceName, const char *streamFilename, const nslStreamInfo *streamInfo )
{
  nslSourceId found; 
  if (!nsl.on) return NSL_INVALID_ID;
  ASSERT_NSL_INITIALIZED();

  if (nsl.sourceSlotsUsedCount == NSL_NUM_SOURCES) 
    return NSL_INVALID_ID;

  char checkName[48];
  strncpy(checkName, soundSourceName, 48);
  checkName[47] = '\0';
  strupr(checkName);

  // Jack it to uppercase
  found = nslGetSource(checkName, false);
  
  if (found != NSL_INVALID_ID)
    return found;

  for (int i=0; i < NSL_NUM_SOURCES; i++)
  {
    
    if (!nsl.sourceSlots[nsl.sourceSlotsUsedCount].used)
      i = nsl.sourceSlotsUsedCount;

    if (!nsl.sourceSlots[i].used)
    {
      // Clear it out.. maybe remove this line later.. 
      _nslClearSourceSlot(i, false);
      
      // Update Cstr (for debugging)
      // Maintain the filename
      strcpy(nsl.sourceSlots[i].fileName, checkName);

      // Send it downstream to GAS
	  if( streamFilename && streamInfo )
		  nslPs2GasRpc(GAS_RPC_OPEN_RAWSTREAM, 
				streamFilename, 
				streamInfo->sampleRate,
				streamInfo->sampleFormat,
				streamInfo->channelCount,
				streamInfo->chunkSize);
      nsl.sourceSlots[i].gasSourceId = nslPs2GasRpc(GAS_RPC_ADD_SOURCE, nsl.sourceSlots[i].fileName, 0, 0, 0, 0);
      
      if (nsl.sourceSlots[i].gasSourceId == (nlUint32)-1) 
      {
        // We failed
        nsl.sourceSlots[i].used = false;
        strcpy(nsl.sourceSlots[i].fileName, "");
        return NSL_INVALID_ID;
      }
      //Get Info about this source
      nsl.sourceSlots[i].looping    = nslPs2GasRpc(GAS_RPC_GET_SOURCE_LOOPING, "", nsl.sourceSlots[i].gasSourceId, 0, 0, 0);
      nsl.sourceSlots[i].length      = (float) nslPs2GasRpc(GAS_RPC_GET_SOURCE_LENGTH, "", nsl.sourceSlots[i].gasSourceId, 0, 0, 0) / 1000.0f;
      nsl.sourceSlots[i].paddedLength= (float) nslPs2GasRpc(GAS_RPC_GET_SOURCE_PADDED_LENGTH, "", nsl.sourceSlots[i].gasSourceId, 0, 0, 0) / 1000.0f;
      nsl.sourceSlots[i].bank       = nslPs2GasRpc(GAS_RPC_GET_SOURCE_BANK, "", nsl.sourceSlots[i].gasSourceId, 0, 0, 0);
      nsl.sourceSlots[i].type       = (nslSourceTypeEnum)nslPs2GasRpc(GAS_RPC_GET_SOURCE_TYPE, "", nsl.sourceSlots[i].gasSourceId, 0, 0, 0);
      nsl.sourceSlots[i].rawVolume  = (float)nslPs2GasRpc(GAS_RPC_GET_SRC_VOLUME, "", nsl.sourceSlots[i].gasSourceId, 0, 0, 0)/100.0f;
      //set myId
      if (nsl.sourceSlots[i].myId == NSL_INVALID_ID) 
        nsl.sourceSlots[i].myId = i;  
      
      nsl.sourceSlots[i].maxDist = NSL_DEFAULT_MAX_SOUND_DIST;
      nsl.sourceSlots[i].minDist = NSL_DEFAULT_MIN_SOUND_DIST;
//      nsl.sourceSlots[i].balance = 0.0f;
      nsl.sourceSlots[i].pitch = 1.0f;


      // Can't have 0 in with ID mask
      nsl.sourceSlots[i].myId += NSL_SOURCE_ID_INCREMENT;

      //Increment the used slots
      nsl.sourceSlotsUsedCount++;
      //Its used;
      nsl.sourceSlots[i].used = true;
      //Return
      return nsl.sourceSlots[i].myId;
    }

  }
  return NSL_INVALID_ID;
}  

nslSourceId nslLoadSource( const char *soundSourceName )
{
	return _nslLoadSource( soundSourceName, NULL, NULL );
}

nslSourceId nslStreamLoadSource( const char *soundSourceName, const char *streamFilename, const nslStreamInfo *info )
{
	return _nslLoadSource( soundSourceName, streamFilename, info );
}

#endif 

#ifdef NSL_LOAD_SOURCE_BY_ALIAS

static nslSourceId _nslLoadSource( nlUint16 soundSourceID, const char *streamFilename, const nslStreamInfo *streamInfo )
{
  nslSourceId found; 
  if (!nsl.on) return NSL_INVALID_ID;
  ASSERT_NSL_INITIALIZED();

  if (nsl.sourceSlotsUsedCount == NSL_NUM_SOURCES) 
    return NSL_INVALID_ID;

  found = nslGetSource(soundSourceID, false);
  
  if (found != NSL_INVALID_ID)
    return found;

  for (int i=0; i < NSL_NUM_SOURCES; i++)
  {
    
    if (!nsl.sourceSlots[nsl.sourceSlotsUsedCount].used)
      i = nsl.sourceSlotsUsedCount;

    if (!nsl.sourceSlots[i].used)
    {
      // Clear it out.. maybe remove this line later.. 
      _nslClearSourceSlot(i, false);
      
      nsl.sourceSlots[i].aliasID = soundSourceID;

      // Send it downstream to GAS
	  if( streamFilename && streamInfo )
		  nslPs2GasRpc(GAS_RPC_OPEN_RAWSTREAM, 
				streamFilename, 
				streamInfo->sampleRate,
				streamInfo->sampleFormat,
				streamInfo->channelCount,
				streamInfo->chunkSize);
      nsl.sourceSlots[i].gasSourceId = nslPs2GasRpc(GAS_RPC_ADD_SOURCE, "", nsl.sourceSlots[i].aliasID, 0, 0, 0);
      
      if (nsl.sourceSlots[i].gasSourceId == (nlUint32)-1) 
      {
        // We failed
        nsl.sourceSlots[i].used = false;
        nsl.sourceSlots[i].aliasID = NSL_INVALID_ID;
        return NSL_INVALID_ID;
      }
      //Get Info about this source
      nsl.sourceSlots[i].looping    = nslPs2GasRpc(GAS_RPC_GET_SOURCE_LOOPING, "", nsl.sourceSlots[i].gasSourceId, 0, 0, 0);
      nsl.sourceSlots[i].length      = (float) nslPs2GasRpc(GAS_RPC_GET_SOURCE_LENGTH, "", nsl.sourceSlots[i].gasSourceId, 0, 0, 0) / 1000.0f;
      nsl.sourceSlots[i].paddedLength= (float) nslPs2GasRpc(GAS_RPC_GET_SOURCE_PADDED_LENGTH, "", nsl.sourceSlots[i].gasSourceId, 0, 0, 0) / 1000.0f;
      nsl.sourceSlots[i].bank       = nslPs2GasRpc(GAS_RPC_GET_SOURCE_BANK, "", nsl.sourceSlots[i].gasSourceId, 0, 0, 0);
      nsl.sourceSlots[i].type       = (nslSourceTypeEnum)nslPs2GasRpc(GAS_RPC_GET_SOURCE_TYPE, "", nsl.sourceSlots[i].gasSourceId, 0, 0, 0);
      nsl.sourceSlots[i].rawVolume  = (float)nslPs2GasRpc(GAS_RPC_GET_SRC_VOLUME, "", nsl.sourceSlots[i].gasSourceId, 0, 0, 0)/100.0f;
      //set myId
      if (nsl.sourceSlots[i].myId == NSL_INVALID_ID) 
        nsl.sourceSlots[i].myId = i;  
      
      nsl.sourceSlots[i].maxDist = NSL_DEFAULT_MAX_SOUND_DIST;
      nsl.sourceSlots[i].minDist = NSL_DEFAULT_MIN_SOUND_DIST;
//      nsl.sourceSlots[i].balance = 0.0f;
      nsl.sourceSlots[i].pitch = 1.0f;


      // Can't have 0 in with ID mask
      nsl.sourceSlots[i].myId += NSL_SOURCE_ID_INCREMENT;

      //Increment the used slots
      nsl.sourceSlotsUsedCount++;
      //Its used;
      nsl.sourceSlots[i].used = true;
      //Return
      return nsl.sourceSlots[i].myId;
    }

  }
  return NSL_INVALID_ID;
}  

nslSourceId nslLoadSource( nlUint16 soundSourceID )
{
	return _nslLoadSource( soundSourceID, NULL, NULL );
}

nslSourceId nslStreamLoadSource( nlUint16 soundSourceID, const char *streamFilename, const nslStreamInfo *info )
{
	return _nslLoadSource( soundSourceID, streamFilename, info );
}

#endif

nslSourceId nslGetSourceByIndex(int index)
{
	if (index < NSL_NUM_SOURCES)
	{
		if (nsl.sourceSlots[index].used)
		{
			return nsl.sourceSlots[index].myId;
		}
	}
	return NSL_INVALID_ID;
}


#ifdef NSL_LOAD_SOURCE_BY_NAME

/*-------------------------------------------------------------------------
  nsl Get Source
-------------------------------------------------------------------------*/
nslSourceId	nslGetSource( const char *soundSourceName, bool fatal )
{
  if (!nsl.on) return 0;
  ASSERT_NSL_INITIALIZED();
  char checkName[48];
  strncpy(checkName, soundSourceName, 48);
  checkName[47] = '\0';
  strupr(checkName);
  for (unsigned int i=0; i < nsl.sourceSlotsUsedCount; i++) 
  {
    if (strcmp(checkName, nsl.sourceSlots[i].fileName)==0) 
    {
      return nsl.sourceSlots[i].myId;
    }
  }

  if (fatal)
  {
    nslFatal("nslGetSource: bad source name\n");
  }
  return NSL_INVALID_ID;
}

#endif


#ifdef NSL_LOAD_SOURCE_BY_ALIAS

nslSourceId	nslGetSource( nlUint16 soundSourceID, bool fatal )
{
  if (!nsl.on) return 0;
  ASSERT_NSL_INITIALIZED();

  for (unsigned int i=0; i < nsl.sourceSlotsUsedCount; i++) 
  {
    if (nsl.sourceSlots[i].aliasID == soundSourceID) 
    {
      return nsl.sourceSlots[i].myId;
    }
  }

  if (fatal)
  {
    nslFatal("nslGetSource: bad source name\n");
  }
  return NSL_INVALID_ID;
}

#endif


/*-------------------------------------------------------------------------
  nsl Get Source Name
-------------------------------------------------------------------------*/

#ifdef NSL_LOAD_SOURCE_BY_NAME

const char *nslGetSourceName( nslSourceId whichSource )
{
  ASSERT_SOURCE_ID_VALID( whichSource );
  return nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(whichSource)].fileName;
}


#endif


#ifdef NSL_LOAD_SOURCE_BY_ALIAS

nlUint16 nslGetSourceName( nslSourceId whichSource )
{
  ASSERT_SOURCE_ID_VALID( whichSource );
  return nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(whichSource)].aliasID;
}

#endif
/*-------------------------------------------------------------------------
  nsl Release All Sources
-------------------------------------------------------------------------*/
void _nslReleaseAllSources()
{
  if (!nsl.on) return;
  ASSERT_NSL_INITIALIZED();
  // Do we now torch all source, sounds and emitters?
  // What is the difference between releasee and stop. 
  // Seems like we should have a release and stop procedure
  nslReleaseAllSounds();
  for (int i=0; i < NSL_NUM_EMITTERS; i++) {
    _nslClearEmitterSlot(i, false, NSL_CLEAR_RESET);
  }

  for (int i=0; i < NSL_NUM_SOURCES; i++)
    nslPs2GasRpc(GAS_RPC_REMOVE_SOURCE, "", 0, 0, 0, 0);
}


/*-------------------------------------------------------------------------
  nsl Set Source Range
-------------------------------------------------------------------------*/
void nslSetSourceRange( nslSourceId sourceToSet, float minDist, float maxDist)
{
  if (!nsl.on) return;
  ASSERT_NSL_INITIALIZED();
  ASSERT_SOURCE_ID_VALID(sourceToSet);
  nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(sourceToSet)].minDist = minDist;
  nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(sourceToSet)].maxDist = maxDist;

}

/*-------------------------------------------------------------------------
  nsl Get Source Length
-------------------------------------------------------------------------*/
float nslGetSourceLength( nslSourceId s)
{
  // SIZE is the length in 1/1000ths of a second
  return ((float)nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(s)].length);
}

/*-------------------------------------------------------------------------
  nsl Get Source Padded Length
-------------------------------------------------------------------------*/
float nslGetSourcePaddedLength( nslSourceId s)
{
  // SIZE is the length in 1/1000ths of a second
  return ((float)nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(s)].paddedLength);
}

/*-------------------------------------------------------------------------
  nsl Set Source Param
-------------------------------------------------------------------------*/
void nslSetSourceParam( nslSourceId sourceToSet, nslSoundParamEnum whichParam, 
                        float newVal )
{
  if (!nsl.on) return;
  ASSERT_NSL_INITIALIZED();
  ASSERT_SOURCE_ID_VALID(sourceToSet);
  switch (whichParam) {
    case NSL_SOUNDPARAM_VOLUME: 
      nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(sourceToSet)].rawVolume=newVal; break;
    case NSL_SOUNDPARAM_PITCH:
      nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(sourceToSet)].pitch=newVal; break;
    case NSL_SOUNDPARAM_MINDIST:
      nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(sourceToSet)].minDist=newVal; break;
    case NSL_SOUNDPARAM_MAXDIST:
      nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(sourceToSet)].maxDist=newVal; break;
    default: nslFatal(false && "Unhandled sourcetype"); break;
  }

}


/*-------------------------------------------------------------------------
  nsl Get Source Param
-------------------------------------------------------------------------*/
float nslGetSourceParam( nslSourceId whichSource, nslSoundParamEnum whichParam )
{
  if (!nsl.on) return 0.0;
  ASSERT_NSL_INITIALIZED();
  ASSERT_SOURCE_ID_VALID(whichSource);
  switch (whichParam) {
    case NSL_SOUNDPARAM_VOLUME: 
      return nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(whichSource)].rawVolume; break;
    case NSL_SOUNDPARAM_PITCH:
      return nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(whichSource)].pitch; break;
    case NSL_SOUNDPARAM_MINDIST:
      return nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(whichSource)].minDist; break;
    case NSL_SOUNDPARAM_MAXDIST:
      return nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(whichSource)].maxDist; break;
    default: nslFatal(false && "Unhandled sourcetype"); break;
  }

  return 0.0f;
}


void nslSetReverb(nslSourceId whichSource, bool new_setting)
{

#ifdef NSL_LOAD_SOURCE_BY_NAME

	nslPs2GasRpc(	GAS_RPC_SET_REVERB,
					nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(whichSource)].fileName,
					0,
					new_setting,
					0, 0);

#elif defined NSL_LOAD_SOURCE_BY_ALIAS

	nslPs2GasRpc(	GAS_RPC_SET_REVERB,
					"",
					nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(whichSource)].aliasID,
					new_setting,
					0, 0);

#endif

}

bool nslGetReverb(nslSourceId whichSource)
{

#ifdef NSL_LOAD_SOURCE_BY_NAME

	return nslPs2GasRpc(	GAS_RPC_GET_REVERB,
							nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(whichSource)].fileName,
							0, 0, 0, 0);

#elif defined NSL_LOAD_SOURCE_BY_ALIAS

	return nslPs2GasRpc(	GAS_RPC_GET_REVERB,
							"",
							nsl.sourceSlots[NSL_GET_SOURCE_SLOT_FROM_ID(whichSource)].aliasID,
							0, 0, 0);

#endif

}



/////////////////////////// internal functions ////////////////////////////

// none yet.
