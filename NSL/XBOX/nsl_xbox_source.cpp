#include "nsl_xbox.h"

//
//
// nslSource functions 
//
//

////////////////////////////////////////////////////////////////////////////////
//
//  Load source by NAME version
//
////////////////////////////////////////////////////////////////////////////////

#ifdef NSL_LOAD_SOURCE_BY_NAME

// This function only looks at the header of the wave file and extract the date for 
// creating sound buffer.  This can probably move to NSLTool in the future.
void _nslProcessSndEntry( const char *soundSourceName )
{
	if(!sndSystem.loadedSndFile)
  {
    nslPrintf("NSL ERROR:  No Sound File Loaded\n");
		return;	
  }

	char fname[256];
	char *token;
	sprintf(fname, "%s", soundSourceName);
	token = strrchr(fname, '\\');
	if(!token)
		token = fname;
	_str2upper(token);
	
	++sndSystem.numSources;
	nslSource* theSource = &sndSystem.sources[sndSystem.numSources];

	// read wave file header
	unsigned char* read = sndSystem.soundBank.Buf;
	read += theSource->sndEntry.offset;
	theSource->data = read;
		
	// setup the DSBUFFERDESC
  theSource->desc.dwSize = sizeof( DSBUFFERDESC );
	theSource->desc.dwFlags = DSBCAPS_LOCDEFER | DSBCAPS_CTRLVOLUME |	DSBCAPS_CTRLFREQUENCY;
	theSource->desc.dwBufferBytes = 0; // we'll use SetBufferDada()
	theSource->desc.lpwfxFormat = (LPWAVEFORMATEX)&theSource->format;
	theSource->desc.lpMixBins = 0;
	theSource->desc.dwInputMixBin = 0;	
	theSource->isLooped = (theSource->sndEntry.flags & XBOX_SND_FLAGS_LOOPED) ? true : false;

	#if defined (NSLDEBUG)
  sndSystem.soundDataMemoryUsage += theSource->sndEntry.size;
	#endif
	theSource->isLoaded = true;
}

nslSourceId nslLoadSource( const char *soundSourceName )
{
  if(!sndSystem.loadedSndFile)
  {
    nslPrintf("NSL ERROR:  No Sound File Loaded\n");
		return NSL_INVALID_ID;
  }


	char fname[256];
	char *token;
	sprintf(fname, "%s", soundSourceName);
	token = strrchr(fname, '\\');
	if(!token)
		token = fname;
	_str2upper(token);

	// make sure this file is in the .SND file
	nslSourceId found = nslGetSource(token, false);
	if(found == NSL_INVALID_ID)
		nslPrintf("NSL ERROR: Trying to load an invalid source file - %s...", token);

	return found;
}

nslSourceId nslGetSource( const char *soundSourceName, bool fatal )
{
  if(!sndSystem.loadedSndFile)
  {
    nslPrintf("NSL ERROR:  No Sound File Loaded\n");
		return NSL_INVALID_ID;
  }
	char fname[256];
	char *token;
	sprintf(fname, "%s", soundSourceName);
	token = strrchr(fname, '\\');
	if(!token)
		token = fname;
	_str2upper(token);

	bool found = false;
	int i;
	for(i=0; i<=sndSystem.numSources; i++)
	{
		if(strcmp(sndSystem.sources[i].sndEntry.fileName, token) == 0)
		{
			found = true;
			break;
		}
	}

	// couldn't find the source or if it wasn't loaded because the file was missing
	if (!found || !sndSystem.sources[i].isLoaded)
	{
		if (fatal)
			nslFatal("NSL ERROR: Sound not loaded: %s.\n", soundSourceName);

		return NSL_INVALID_ID;
	}
	else
		return ( i );
}

const char *nslGetSourceName( nslSourceId whichSource )
{
  if(!sndSystem.loadedSndFile)
  {
    nslPrintf("NSL ERROR:  No Sound File Loaded\n");
    return "INVALID";
  }
	RETURN_WITH_VALUE_ON_INVALID_ID(whichSource, "INVALID");

	return (sndSystem.sources[whichSource].sndEntry.fileName);
}

#endif // NSL_LOAD_SOURCE_BY_NAME

////////////////////////////////////////////////////////////////////////////////
// End of NAME version
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//
//  Load source by ALLIAS version
//
////////////////////////////////////////////////////////////////////////////////

#ifdef NSL_LOAD_SOURCE_BY_ALIAS

// This function only looks at the header of the wave file and extract the date for 
// creating sound buffer.  This can probably move to NSLTool in the future.
void _nslProcessSndEntry( nlUint16 aliasID )
{
  if(!sndSystem.loadedSndFile)
  {
    nslPrintf("NSL ERROR:  No Sound File Loaded\n");
		return;
  }
	char fname[256];
	char *token;
	sprintf(fname, "%d", aliasID);
	token = strrchr(fname, '\\');
	if(!token)
		token = fname;
	_str2upper(token);
	
	++sndSystem.numSources;
	nslSource* theSource = &sndSystem.sources[sndSystem.numSources];

	// read wave file header
	unsigned char* read = sndSystem.soundBank.Buf;
	read += theSource->sndEntry.offset;
	theSource->data = read;
		
	// setup the DSBUFFERDESC
  theSource->desc.dwSize = sizeof( DSBUFFERDESC );
	theSource->desc.dwFlags = DSBCAPS_LOCDEFER | DSBCAPS_CTRLVOLUME |	DSBCAPS_CTRLFREQUENCY;
	theSource->desc.dwBufferBytes = 0; // we'll use SetBufferDada()
	theSource->desc.lpwfxFormat = (LPWAVEFORMATEX)&theSource->format;
#if 0
	theSource->desc.dwMixBinMask = 0;
	theSource->desc.dwInputMixBinMask = 0;
#else
	theSource->desc.lpMixBins = 0;
	theSource->desc.dwInputMixBin = 0;	
#endif
	theSource->isLooped = (theSource->sndEntry.flags & XBOX_SND_FLAGS_LOOPED) ? true : false;
	#ifdef NSLDEBUG
//	if (theSource->isLooped)
//		nslPrintf("NSL INFO: %s is a looped sound", theSource->sndEntry.fileName);
	#endif

	#if defined (NSLDEBUG)
  sndSystem.soundDataMemoryUsage += theSource->sndEntry.size;
	#endif
	theSource->isLoaded = true;
}

nslSourceId nslLoadSource( nlUint16 soundSourceID )
{
  if(!sndSystem.loadedSndFile)
  {
    nslPrintf("NSL ERROR:  No Sound File Loaded\n");
		return NSL_INVALID_ID;
  }

/*
  char fname[256];
	char *token;

  sprintf(fname, "%s", soundSourceName);
	token = strrchr(fname, '\\');
	if(!token)
		token = fname;
	_str2upper(token);
*/

	// make sure this file is in the .SND file
	nslSourceId found = nslGetSource(soundSourceID, false);
	if(found == NSL_INVALID_ID)
		nslPrintf("NSL ERROR: Trying to load an invalid source file - %d...", soundSourceID);

	return found;
}

nslSourceId nslGetSource( nlUint16 soundSourceID, bool fatal )
{
  if(!sndSystem.loadedSndFile)
  {
    nslPrintf("NSL ERROR:  No Sound File Loaded\n");
		return NSL_INVALID_ID;
  }

	bool found = false;
	int i;
	for(i=0; i<=sndSystem.numSources; i++)
	{
		if(sndSystem.sources[i].sndEntry.aliasID == soundSourceID)
		{
			found = true;
			break;
		}
	}

	// couldn't find the source or if it wasn't loaded because the file was missing
	if (!found || !sndSystem.sources[i].isLoaded)
	{
		if (fatal)
			nslFatal("NSL ERROR: Sound not loaded: %d.\n", soundSourceID);

		return NSL_INVALID_ID;
	}
	else
		return ( i );
}

nlUint16 nslGetSourceName( nslSourceId whichSource )
{
  if(!sndSystem.loadedSndFile)
  {
    nslPrintf("NSL ERROR:  No Sound File Loaded\n");
		return NSL_INVALID_ID;
  }

	return (sndSystem.sources[whichSource].sndEntry.aliasID);
}


#endif // NSL_LOAD_SOURCE_BY_ALIAS

////////////////////////////////////////////////////////////////////////////////
// End of ALIAS version
////////////////////////////////////////////////////////////////////////////////


float nslGetPaddedSourceLength( nslSourceId whichSource )
{
	nslPrintf("STUB:  nslGetPaddedSourceLength\n");
	return 0.0;
}

float nslGetSourceLength( nslSourceId whichSource )
{
	nslSource &theSource = sndSystem.sources[whichSource];

	return ((float) theSource.sndEntry.samples) / ((float) theSource.sndEntry.format.nSamplesPerSec);
}


nslSourceId nslGetSourceByIndex(int index)
{
	if (index < NSL_NUM_SOUND_SOURCES)
	{
		if (sndSystem.sources[index].isLoaded)
		{
			return sndSystem.sources[index].id;
		}
	}
	return NSL_INVALID_ID;
}


float nslGetSourceParam( nslSourceId whichSource, nslSoundParamEnum whichParam )
{
	if(!sndSystem.loadedSndFile)
		return 0.0f;		
	nslSoundParam* theParams = &sndSystem.sources[whichSource].sndEntry.params;
	
	switch(whichParam)
	{
		case NSL_SOUNDPARAM_VOLUME:
			return (theParams->volume);
			break;
		case NSL_SOUNDPARAM_PITCH:
			return (theParams->pitch);
			break;
		case NSL_SOUNDPARAM_MINDIST:
			return (theParams->minDist);
			break;
		case NSL_SOUNDPARAM_MAXDIST:
			return (theParams->maxDist);
			break;
		default:
			nslFatal("NSL ERROR: Unknown Parameter.");
			break;
	}
	
	return 0.0f;
}

void nslSetSourceParam( nslSourceId sourceToSet, nslSoundParamEnum whichParam, float newVal )
{
	if(!sndSystem.loadedSndFile)
		return;	
	nslSoundParam* theParams = &sndSystem.sources[sourceToSet].sndEntry.params;

	switch(whichParam)
	{
		case NSL_SOUNDPARAM_VOLUME:
			theParams->volume = newVal;
			break;
		case NSL_SOUNDPARAM_PITCH:
			theParams->pitch = newVal;
			break;
		case NSL_SOUNDPARAM_MINDIST:
			theParams->minDist = newVal;
			break;
		case NSL_SOUNDPARAM_MAXDIST:
			theParams->maxDist = newVal;
			break;
		default:
			nslFatal("NSL ERROR: Unknown Parameter.");
			break;
	}
}

void _nslReleaseAllSources()
{
	if(!sndSystem.loadedSndFile)
		return;

	nslSystem &s = sndSystem;
	nslSource *src;
	for(int i=0; i<NSL_NUM_SOUND_SOURCES; ++i)
	{
		src = &s.sources[i];
		if (src->isLoaded)
		{
			#ifdef NSLDEBUG
			s.soundDataMemoryUsage -= src->sndEntry.size;
			#endif
			memset(src, 0, sizeof(nslSource));
			--s.numSources;
		}
	}
	#ifdef NSLDEBUG
	internalAssert( s.soundDataMemoryUsage == 0 );
	internalAssert( s.numSources == 0);
	#endif

	nslReleaseFile( &sndSystem.soundBank );
}


void nslSetReverb(nslSourceId whichSource, bool new_setting)
{
	if (new_setting)
	{
		sndSystem.sources[whichSource].sndEntry.flags |= XBOX_SND_FLAGS_REVERB;
	}
	else
	{
		sndSystem.sources[whichSource].sndEntry.flags &= ~XBOX_SND_FLAGS_REVERB;
	}
}

bool nslGetReverb(nslSourceId whichSource)
{
	return (sndSystem.sources[whichSource].sndEntry.flags & XBOX_SND_FLAGS_REVERB) > 0;
}

