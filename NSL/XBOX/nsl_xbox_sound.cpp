#include "nsl_xbox.h"
#include "nsl_xbox_source.h"
#include <math.h>

#pragma warning(disable:4244)
#define XBOX_ADPCM_HEADER_SIZE 48
#define M_PI 3.14159f

#define REAR_DAMPEN_VALUE 0.7f
#define CENTER_DAMPEN_VALUE 0.43f

//#undef assert
//
//#if defined(NSLDEBUG)
//#define assert(exp) { if(!(exp)) _nslAssert(#exp, __FILE__, __LINE__); }
//#else
//#define assert(exp) {}
//#endif
inline bool _nslCheckSoundId( nslSoundId id )
{
	nslSoundId checkSlot = NSL_SLOT(id);
	
	if (sndSystem.sounds[checkSlot].id == id)
		return true;
	else
		return false;
}

int _nslFindNewSoundSlot()
{
#if DISABLE_SOUNDS
	return -1;
#else

/*
  int freesounds = 0;

	for (int i=0; i<NSL_NUM_SOUNDS; i++)
		if(i != NSL_INVALID_ID && !sndSystem.sounds[i].isUsed)
    {
      freesounds++;
    }

  nslPrintf( "Free sound slots: %d\n", freesounds );
*/
  
	for (int i=0; i<NSL_NUM_SOUNDS; i++)
		if(i != NSL_INVALID_ID && !sndSystem.sounds[i].isUsed)
			return i;
		
		return -1;
#endif
}

void _nslClearSoundSlot( nslSoundId id )
{
#if !DISABLE_SOUNDS

	nslSound* theSound = &sndSystem.sounds[NSL_SLOT(id)];

	--sndSystem.numSounds;
	if(theSound->eId != NSL_GLOBAL_EMITTER_ID)
		--sndSystem.num3DSounds;
	
	// remove the sound from the emitter
	_nslRemoveSoundFromEmitter(theSound->eId, theSound->id);
	
	if(theSound->pDSBuffer)
		theSound->pDSBuffer->Release();
	theSound->pDSBuffer = NULL;
	
	memset(theSound, 0, sizeof(nslSound));
#if (!DISABLE_NEW_STREAMS && DISABLE_STREAMS)
  theSound->newStreamSlot = -1;
#endif

#endif
}
	
int _nslFindNewStreamSlot()
{
#if DISABLE_STREAMS
	return -1;
#else
	for (int i=0; i<NSL_NUM_STREAM_SOUNDS; ++i)
		if(i != NSL_INVALID_ID && !sndSystem.streams[i].isUsed)
			return i;
		
		return -1;
#endif
}

#if (!DISABLE_NEW_STREAMS && DISABLE_STREAMS)
bool _nslInitNewStream(nslNewStreamId id)
{
	nslNewStreamSound* theStream = &sndSystem.soundStreams[id];
	nslSoundId sndId = theStream->soundId;
	nslSound* theSound = &sndSystem.sounds[NSL_SLOT(sndId)];
	nslSource* theSource = &sndSystem.sources[theSound->srcId];

	DSSTREAMDESC dssd;
	ZeroMemory( &dssd, sizeof(dssd) );
  dssd.dwMaxAttachedPackets = XBoxStreamBufSize / XBoxMinPacketSize;
  dssd.lpwfxFormat          = (LPWAVEFORMATEX)&theSource->format;

	// create a 3D stream sound if the emitter isn't global emitter
	if(theSound->eId != NSL_GLOBAL_EMITTER_ID)
		dssd.dwFlags = /*DSSTREAMCAPS_CTRL3D | */DSSTREAMCAPS_LOCDEFER;
	
	DS_TRY( DirectSoundCreateStream(&dssd, &(theStream->pRenderFilter)) );
	if(theSound->eId != NSL_GLOBAL_EMITTER_ID)
	{
		// we are controlling the distance falloff ourselves...
		//DS_TRY( theStream->pRenderFilter->SetMaxDistance( 1.0f, DS3D_IMMEDIATE ) );
	}
	
  for ( int i = 0; i < XBoxStreamPackets; i++ )
  {
		theStream->dwPacketStatus[i] = XMEDIAPACKET_STATUS_SUCCESS;	
    theStream->isPacketReady[i] = false;
  }

	// set the initial volume of the stream
	// becasue the stream is running in a different thread, we sometimes gets a pop if we don't
	// set the volume right away when the stream is supposed to be mute.
	float newVolume;
	newVolume = theSound->params.volume;
	newVolume *= sndSystem.gameVolume[theSource->sndEntry.type] / 1.0f;
	newVolume *= sndSystem.masterVolume / 1.0f;	
	if (theSound->dampenCount > 0)
		newVolume *= sndSystem.dampenValue;
	//DS_TRY( theStream->pRenderFilter->SetVolume( _toDecibel(newVolume) ));


	// If this sound is not set to play with reverb then send its output
	// straight to the xtalk mix bins rather that via the reverb DSP unit.

	if (!nslGetReverb(theSound->srcId))
	{
		if (newVolume > 0.0f)
		{
			if (theSound->eId != NSL_GLOBAL_EMITTER_ID)
				_set3dVolume( theSound, theStream );
			else
				_setNon3dVolume( theSound, theStream );
		}
		DS_TRY( theStream->pRenderFilter->SetVolume( _toDecibel(newVolume) ));
	}


	theStream->isInitialized = true;
	theStream->shouldDie = false;
	theStream->almostDead = false;

#ifdef NSL_LOAD_SOURCE_BY_NAME
	nslPrintf("NSL INFO: Playing stream %s", sndSystem.sources[sndSystem.sounds[theStream->soundId&0xff].srcId].sndEntry.fileName);
#endif
	
#ifdef NSL_LOAD_SOURCE_BY_ALIAS
	nslPrintf("NSL INFO: Playing stream %d", sndSystem.sources[sndSystem.sounds[theStream->soundId&0xff].srcId].sndEntry.aliasID);
#endif
	
	return true;
}

HRESULT _nslNewProcessRenderer( nslNewStreamId id, DWORD packetIndex, XMEDIAPACKET& xmp )
{
	HRESULT      hr;
	nslNewStreamSound* theStream = &sndSystem.soundStreams[id];
	
	// we didn't read anything, probably end of the file
	if(xmp.dwMaxSize == 0)
		return true;

	xmp.pdwCompletedSize = &theStream->dwCompletedSize[packetIndex];
  xmp.pvBuffer = theStream->pvSourceBuffer[packetIndex];
	xmp.pdwStatus = &theStream->dwPacketStatus[packetIndex];
	hr = theStream->pRenderFilter->Process( &xmp, NULL );
	
	if( FAILED(hr) )
		return false;
	else
		return true;
}

extern "C"
{
  extern BOOL g_fDirectSoundDisableBusyWaitWarning;
}

void _nslClearNewStreamSlot( nslNewStreamId id )
{
	nslNewStreamSound& theStream = sndSystem.soundStreams[id];
	nslSound& theSound = sndSystem.sounds[NSL_SLOT(theStream.soundId)];
 
 	if ( theSound.isQueued && !theSound.isPlaying )
 	{
 		sndSystem.streamer.ReleaseStream( id );
 
 		_nslClearSoundSlot( theStream.soundId );
 		theStream.soundId = NSL_INVALID_ID;
 		theStream.isInitialized = false;
 		theStream.shouldDie = false;
 		theStream.almostDead = false;
 		theStream.startPlay = false;
 		theStream.fileLength = 0;
 		theStream.fileOffset = 0;
 
 		--sndSystem.numStreamSounds;

    return;
 	}

  if (!theStream.almostDead)
  {
    theStream.almostDead = true;

#ifdef NSL_LOAD_SOURCE_BY_NAME
    nslPrintf("NSL INFO: Clearing stream %s", sndSystem.sources[sndSystem.sounds[theStream.soundId&0xff].srcId].sndEntry.fileName);
#endif

#ifdef NSL_LOAD_SOURCE_BY_ALIAS
    nslPrintf("NSL INFO: Clearing stream %d", sndSystem.sources[sndSystem.sounds[theStream.soundId&0xff].srcId].sndEntry.aliasID);
#endif

/*
    Warning: Busy-waiting for the voice to turn off
    Warning: This warning is being generated because you've called a method that requires
    Warning: a hardware voice to be stopped.  Stopping a voice is an asynchronous
    Warning: operation, so any function that requires the voice to be stopped must block
    Warning: until it is.  To see an example of this, call IDirectSoundBuffer::Stop and
    Warning: immediately follow it with a call to IDirectSoundBuffer::GetStatus.  Chances
    Warning: are, you'll see that DSBSTATUS_PLAYING is still set.  For a list of methods
    Warning: that can potentially block in this method, consult the documentation.  To
    Warning: query the playing status of a buffer or stream, call the GetStatus method on
    Warning: the object you wish to check.  To disable this warning, set the global
    Warning: variable "g_fDirectSoundDisableBusyWaitWarning" to TRUE.
*/
	  if(theStream.pRenderFilter)
	  {
      // Well you can't stop the freakin' stream play it quick then
      theStream.pRenderFilter->SetVolume( DSBVOLUME_MIN );
		  // theStream.pRenderFilter->SetPitch(4096);
      // g_fDirectSoundDisableBusyWaitWarning = TRUE;
      //REFERENCE_TIME now;
      //sndSystem.pDS->GetTime( &now );
      theStream.pRenderFilter->FlushEx( 0, DSSTREAMFLUSHEX_ASYNC );
	  }
  }

  if (theStream.almostDead)
  {
	  if (theStream.pRenderFilter)
	  {
      DWORD status;

      theStream.pRenderFilter->GetStatus( &status );

      // Check if it is finally dead
      if (status & DSSTREAMSTATUS_PLAYING)
      {
        nslPrintf( "Stream %d is still playing...", id );
      }
      else
      {
        sndSystem.streamer.ReleaseStream( id );

		    theStream.pRenderFilter->Release();
		    theStream.pRenderFilter = NULL;

	      _nslClearSoundSlot( theStream.soundId );
	      theStream.soundId = NSL_INVALID_ID;
	      theStream.isInitialized = false;
        theStream.shouldDie = false;
        theStream.almostDead = false;
	      theStream.startPlay = false;
	      theStream.fileLength = 0;
	      theStream.fileOffset = 0;
	      
	      --sndSystem.numStreamSounds;
      }
    }
  }
}
#endif // !DISABLE_NEW_STREAMS

#if !DISABLE_STREAMS
void _nslClearStreamSlot( nslStreamId id )
{
#if !DISABLE_STREAMS
	nslStreamSound* theStream = &sndSystem.streams[id];

#ifdef NSL_LOAD_SOURCE_BY_NAME
  nslPrintf("NSL INFO: Clearning stream %s", sndSystem.sources[sndSystem.sounds[theStream->soundId&0xff].srcId].sndEntry.fileName);
#endif

#ifdef NSL_LOAD_SOURCE_BY_ALIAS
  nslPrintf("NSL INFO: Clearning stream %d", sndSystem.sources[sndSystem.sounds[theStream->soundId&0xff].srcId].sndEntry.aliasID);
#endif

	CloseHandle(theStream->hFile);
	theStream->hFile = INVALID_HANDLE_VALUE;

	if(theStream->pRenderFilter)
	{
		theStream->pRenderFilter->Flush();
		theStream->pRenderFilter->Release();
		theStream->pRenderFilter = NULL;
	}
	
	_nslClearSoundSlot( theStream->soundId );
	theStream->soundId = NSL_INVALID_ID;
	theStream->isUsed = false;
	theStream->isInitialized = false;
	theStream->startPlay = false;
	theStream->shouldDie = false;
	theStream->almostDone = false;
	theStream->endStream = false;
	theStream->fileLength = 0;
	theStream->fileOffset = 0;
	
	--sndSystem.numStreamSounds;
#endif
}

bool _nslFindFreePacket( nslStreamId id, DWORD* pdwPacketIndex )
{
#if !DISABLE_STREAMS	
	for( int index = 0; index < FILESTRM_PACKET_COUNT; index++ )
	{
		if( XMEDIAPACKET_STATUS_PENDING != sndSystem.streams[id].adwPacketStatus[index] )
		{
			if( pdwPacketIndex )
				(*pdwPacketIndex) = index;
			
			return true;
		}
	}
#endif
	return false;
}

bool _nslProcessSource( nslStreamId id, DWORD packetIndex, XMEDIAPACKET& xmp )
{
#if !DISABLE_STREAMS
	DWORD        dwTotalSourceUsed   = 0;
	DWORD        dwSourceUsed = 0;
	nslStreamSound* theStream = &sndSystem.streams[id];
	
	ZeroMemory( &xmp, sizeof(xmp) );
	xmp.pvBuffer         = (BYTE *)theStream->pvSourceBuffer + packetIndex * FILESTRM_PACKET_BYTES;
	xmp.dwMaxSize        = FILESTRM_PACKET_BYTES;
	xmp.pdwCompletedSize = &dwSourceUsed;
	
	nslSound* theSound = &sndSystem.sounds[NSL_SLOT(theStream->soundId)];

	// Read from the source
	DWORD dwActualRead = xmp.dwMaxSize;
	DWORD dwFilePos = SetFilePointer( theStream->hFile, 0, 0, FILE_CURRENT );
		
	if ( (theStream->fileOffset+theStream->fileLength) - dwFilePos < dwActualRead )
		dwActualRead = (theStream->fileOffset+theStream->fileLength) - dwFilePos;
		
	ReadFile( theStream->hFile, xmp.pvBuffer, dwActualRead, xmp.pdwCompletedSize, NULL );
	
	if( dwActualRead < xmp.dwMaxSize )
	{
		if (sndSystem.sources[theSound->srcId].sndEntry.flags & XBOX_SND_FLAGS_LOOPED)
			// SetFilePointer( theStream->hFile, XBOX_ADPCM_HEADER_SIZE, 0, FILE_BEGIN );
    	SetFilePointer( theStream->hFile, sndSystem.sources[theSound->srcId].sndEntry.offset, 0, FILE_BEGIN );
		else
		{
			theStream->almostDone = true;
		}
	}	

	xmp.dwMaxSize = dwSourceUsed;
	return true;

#else
	return false;
#endif
}

HRESULT _nslProcessRenderer( nslStreamId id, DWORD packetIndex, XMEDIAPACKET& xmp )
{
#if !DISABLE_STREAMS
	HRESULT      hr;
	nslStreamSound* theStream = &sndSystem.streams[id];
	
	// we didn't read anything, probably end of the file
	if(xmp.dwMaxSize == 0)
		return true;

	xmp.pdwCompletedSize = NULL;
	xmp.pdwStatus = &theStream->adwPacketStatus[packetIndex];	
	hr = theStream->pRenderFilter->Process( &xmp, NULL );
	
	if( FAILED(hr) )
		return false;
	else
		return true;

#else
	return false;
#endif
}

// if this function returns false it means the stream is done and doesn't require looping
bool _nslProcessStream( nslStreamId id )
{
#if DISABLE_STREAMS
	return false;
#else
	DWORD dwPacketIndex;
	
	while( !sndSystem.streams[id].almostDone && _nslFindFreePacket( id, &dwPacketIndex ) )
	{
		XMEDIAPACKET xmp;	
				
		// Read from the source filter
		if(!_nslProcessSource( id, dwPacketIndex, xmp ) )
			return false;
		
		// Send the data to the renderer
		if (!_nslProcessRenderer( id, dwPacketIndex, xmp ) )
			return false;
	}	

	if (sndSystem.streams[id].almostDone)
	{
		bool done = true;

		// IDSoundStream->GetStatus seems very unreliable...
		for(int i=0; i<FILESTRM_PACKET_COUNT; i++)
		{
			if (sndSystem.streams[id].adwPacketStatus[i] != XMEDIAPACKET_STATUS_SUCCESS)
			{
				done = false;
				break;
			}
		}

		if (done)
			sndSystem.streams[id].endStream = true;
	}

	return true;
#endif
}

bool _nslInitStream(nslStreamId id)
{
	nslStreamSound* theStream = &sndSystem.streams[id];
	nslSoundId sndId = theStream->soundId;
	nslSound* theSound = &sndSystem.sounds[NSL_SLOT(sndId)];
	nslSource* theSource = &sndSystem.sources[theSound->srcId];
	char lang[32];
	char strFileName[128];

  lang[0] = 0;
  if (sndSystem.language != NSL_LANGUAGE_NONE)
  {
    strcpy( lang, nslLanguageStr[sndSystem.language] );
    strcat( lang, "\\" );
  }
	
/*
#ifdef NSL_LOAD_SOURCE_BY_NAME
    if (sndSystem.rootDir)
		  sprintf(strFileName, "%s\\%s%s%s", sndSystem.rootDir, lang, theSource->sndEntry.fileName, NSL_XBOX_STREAM_SND_EXT);
    else
		  sprintf(strFileName, "%s%s%s", lang, theSource->sndEntry.fileName, NSL_XBOX_STREAM_SND_EXT);
#endif

#ifdef NSL_LOAD_SOURCE_BY_ALIAS
    if (sndSystem.rootDir)
		  sprintf(strFileName, "%s\\%s%d%s", sndSystem.rootDir, lang, theSource->sndEntry.aliasID, NSL_XBOX_STREAM_SND_EXT);
    else
		  sprintf(strFileName, "%s%d%s", lang, theSource->sndEntry.aliasID, NSL_XBOX_STREAM_SND_EXT);
#endif
*/

  // New streams implementation
  if (sndSystem.rootDir)
		sprintf(strFileName, "%s\\%s%s", sndSystem.rootDir, lang, STREAM_FILENAME);
  else
		sprintf(strFileName, "%s%s", lang, STREAM_FILENAME);

	// check to see if the file exists
	DWORD ret = GetFileAttributes(strFileName);
	if (ret == -1)
	{
		nslPrintf("NSL ERROR: Cannot find stream sound %s", strFileName);
		return false;
	}
		
	theStream->hFile = CreateFile(strFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	// theStream->fileLength = GetFileSize(theStream->hFile, NULL);
	internalAssert(theStream->fileLength != -1);
	theStream->fileLength = theSource->sndEntry.offset + theSource->sndEntry.size;
	// SetFilePointer(theStream->hFile, XBOX_ADPCM_HEADER_SIZE, 0, FILE_BEGIN);
	SetFilePointer(theStream->hFile, theSource->sndEntry.offset, 0, FILE_BEGIN);
	internalAssert(theSource->sndEntry.format.wFormatTag == WAVE_FORMAT_XBOX_ADPCM);
	DSSTREAMDESC dssd;
	ZeroMemory( &dssd, sizeof(dssd) );
  dssd.dwMaxAttachedPackets = FILESTRM_PACKET_COUNT;
  dssd.lpwfxFormat          = (LPWAVEFORMATEX)&theSource->format;
	
	// create a 3D stream sound if the emitter isn't global emitter
	if(theSound->eId != NSL_GLOBAL_EMITTER_ID)
		dssd.dwFlags =/* DSSTREAMCAPS_CTRL3D |*/ DSSTREAMCAPS_LOCDEFER;

	
	DS_TRY( DirectSoundCreateStream(&dssd, &(theStream->pRenderFilter)) );
	if(theSound->eId != NSL_GLOBAL_EMITTER_ID)
	{
		// we are controlling the distance falloff ourselves...
		//DS_TRY( theStream->pRenderFilter->SetMaxDistance( 1.0f, DS3D_IMMEDIATE ) );
	}
	memset(theStream->pvSourceBuffer, 0, FILESTRM_PACKET_BYTES*FILESTRM_PACKET_COUNT);
	internalAssert(theStream->pvSourceBuffer);
	
  for( int i = 0; i < FILESTRM_PACKET_COUNT; i++ )
		theStream->adwPacketStatus[i] = XMEDIAPACKET_STATUS_SUCCESS;	

	// set the initial volume of the stream
	// becasue the stream is running in a different thread, we sometimes gets a pop if we don't
	// set the volume right away when the stream is supposed to be mute.
	float newVolume;
	newVolume = theSound->params.volume;
	newVolume *= sndSystem.gameVolume[theSource->sndEntry.type] / 1.0f;
	newVolume *= sndSystem.masterVolume / 1.0f;	
	newVolume *= sndSystem.dampenLevel / 1.0f;	
	/*DS_TRY( theStream->pRenderFilter->SetVolume( _toDecibel(newVolume) ));


	// If this sound is not set to play with reverb then send its output
	// straight to the xtalk mix bins rather that via the reverb DSP unit.

	if (!nslGetReverb(theSound->srcId))
	{
		DSMIXBINS dsmixbins;

		DSMIXBINVOLUMEPAIR dsmbvp[] =
		{
			{DSMIXBIN_XTLK_FRONT_LEFT, DSBVOLUME_MAX},
			{DSMIXBIN_XTLK_BACK_LEFT, DSBVOLUME_MAX},
			{DSMIXBIN_XTLK_FRONT_RIGHT, DSBVOLUME_MAX},
			{DSMIXBIN_XTLK_BACK_RIGHT, DSBVOLUME_MAX},
		};									

		dsmixbins.dwMixBinCount = 4;
		dsmixbins.lpMixBinVolumePairs = dsmbvp;

		theStream->pRenderFilter->SetMixBins( &dsmixbins );
	}*/
	if (newVolume > 0.0f)
	{
		if (theEmitter->id != NSL_GLOBAL_EMITTER_ID)
			_set3dVolume( theSound, theSound->myStream );
		else
			_setNon3dVolume( theSound, theSound->myStream );
	}
	DS_TRY( theSound->myStream->pRenderFilter->SetVolume( _toDecibel(newVolume) ));

	theStream->isInitialized = true;
	theSound->isReady = true;
  theSound->isPlaying = true;

#ifdef NSL_LOAD_SOURCE_BY_NAME
	nslPrintf("NSL INFO: Playing stream %s", sndSystem.sources[sndSystem.sounds[theStream->soundId&0xff].srcId].sndEntry.fileName);
#endif
	
#ifdef NSL_LOAD_SOURCE_BY_ALIAS
	nslPrintf("NSL INFO: Playing stream %d", sndSystem.sources[sndSystem.sounds[theStream->soundId&0xff].srcId].sndEntry.aliasID);
#endif
	
	return true;
}

DWORD WINAPI _nslStreamThreadProc( LPVOID lpParameter )
{
#if !DISABLE_STREAMS
	//nslStreamId* id = (nslStreamId*)lpParameter;
  /*
  // To process approximately once per frame, we can sleep
  // for 1000 ms / 60 fps between calls to Process.
  */
  DWORD dwQuantum = 1000 / 60;
	
  /*
  // Alternately, the minimum time between processing is
  // determined by how much data we're sending to the stream
  // at once:
  // 2048 * 16 samples per packet * 16 packets = 524288 samples
  // 524288 samples / 44100 samples per second = 11.8 seconds
  //
  // DWORD dwQuantum = 11000;
  */
	
	nslStreamSound* theStream;
	nslSoundId sndId;
	nslSound* theSound;
	nslSource* theSource;
	nslEmitter* theEmitter;
	nlVector3d distVec;
	float newVolume=1.0f, dist;
	int i;

  for( ; ; )
  {
    if (sndSystem.killThread == true)
    {
      sndSystem.killThread = false;
      ExitThread(0);
    }
		if(sndSystem.numStreamSounds == 0)
			Sleep( dwQuantum );
		else
		{
			// i = 0 is not used because it's easier for indexing
			for(i=1; i<NSL_NUM_STREAM_SOUNDS; i++)
			{	
				theStream = &sndSystem.streams[i];

				if (!theStream->isUsed)
					continue;

				// if the stream is stopped externally, stop it
				if(theStream->shouldDie || theStream->endStream)
				{
					_nslClearStreamSlot( i );				
					continue;
				}

				if (!theStream->isInitialized)
				{
					if(!_nslInitStream(i))
						_nslClearStreamSlot(i);
				}
				
				sndId = theStream->soundId;
				theSound = &sndSystem.sounds[NSL_SLOT(sndId)];
				theSource = &sndSystem.sources[theSound->srcId];
				theEmitter = &sndSystem.emitters[theSound->eId];

				if(theStream->startPlay && _nslProcessStream(i))
				{
					if (theSound->eId != NSL_GLOBAL_EMITTER_ID)
					{
						// calculate listener's position relative to this emitter
						nlVector3dSub(distVec, theEmitter->pos, sndSystem.listenerPos);
						dist = nlVector3dLength(distVec);
						
						if ( dist < theSound->params.minDist )
							newVolume = 1.0f;
						else
						{
							if ( dist > theSound->params.maxDist )
								newVolume = 0.0f;
							else
							{
								if ( abs( theSound->params.maxDist - theSound->params.minDist ) > 0.001 )
									newVolume = ( 1.0f - ( ( dist - theSound->params.minDist ) * ( 1.0f / ( theSound->params.maxDist - theSound->params.minDist ) ) ) );
								else
									newVolume = 1.0f;
							}			
						}
						
						newVolume *= theSound->params.volume / 1.0f;
					}
					else
					{
						newVolume = theSound->params.volume;
					}
					newVolume *= sndSystem.gameVolume[theSource->sndEntry.type] / 1.0f;
					newVolume *= sndSystem.masterVolume / 1.0f;
					
					if (newVolume > 0.0f)
					{
						if (theEmitter->id != NSL_GLOBAL_EMITTER_ID)
							_set3dVolume( theSound, theStream );
						else
							_setNon3dVolume( theSound, theStream );
					}
					DS_TRY( theStream->pRenderFilter->SetVolume( _toDecibel(newVolume) ));

				/*	DS_TRY( theStream->pRenderFilter->SetVolume( _toDecibel(newVolume) ));
					if (theSound->eId != NSL_GLOBAL_EMITTER_ID)
					{
						DS_TRY( theStream->pRenderFilter->SetPosition( theEmitter->pos[0], theEmitter->pos[1], theEmitter->pos[2], DS3D_DEFERRED ) );
					}*/
				}
			} // for(i=1; i<NSL_NUM_STREAM_SOUNDS; i++)

			Sleep( dwQuantum );
		} // if(sndSystem.numStreamSounds == 0)
  } // for (;;)
#endif
	return 0;
}

#endif

nslSoundId nslAddSound( nslSourceId soundSource )
{
#if DISABLE_SOUNDS
	return NSL_INVALID_ID;
#else
	if (soundSource == NSL_INVALID_ID || !sndSystem.sources[soundSource].isLoaded) 
		return NSL_INVALID_ID;
	
	nslSystem &s = sndSystem;

	int newSoundSlot = _nslFindNewSoundSlot();
	if(newSoundSlot == -1)
	{
#ifdef NSL_LOAD_SOURCE_BY_NAME
		nslPrintf("NSL ERROR: We ran out of sound slots.  %s is not added.", s.sources[soundSource].sndEntry.fileName);
#endif

#ifdef NSL_LOAD_SOURCE_BY_ALIAS
		nslPrintf("NSL ERROR: We ran out of sound slots.  %d is not added.", s.sources[soundSource].sndEntry.aliasID);
#endif

		return NSL_INVALID_ID;
	}
	++s.id;
	nslSoundId id = NSL_CALC_ID(s.id, newSoundSlot);

	// setup the default sound parameters
	nslSound* theSound = &s.sounds[newSoundSlot];
	internalAssert( !theSound->isUsed );
	nslSource* theSource = &s.sources[soundSource];
	theSound->isUsed = true;
	theSound->id = id;
	theSound->eId = NSL_GLOBAL_EMITTER_ID;
	theSound->srcId = soundSource;

	_nslAddSoundToEmitter(NSL_GLOBAL_EMITTER_ID, theSound->id);
	memcpy(&theSound->params, &(theSource->sndEntry.params), sizeof(nslSoundParam));
	++s.numSounds;
	
	// if it's a streamed sound, acquire a stream slot
	if (theSource->sndEntry.flags & XBOX_SND_FLAGS_STREAM)
	{
		#if DISABLE_STREAMS

#if !DISABLE_NEW_STREAMS
	theSound->isReady = false;
    theSound->newStreamSlot = sndSystem.streamer.CreateStream( sndSystem.hStreamFile, theSource->sndEntry.offset, theSource->sndEntry.size, !!(theSource->sndEntry.flags & XBOX_SND_FLAGS_LOOPED) );
  
    //internalAssert( theSound->newStreamSlot >= 0 );
    if (theSound->newStreamSlot >= 0)
    {
			nslNewStreamSound& theStream = s.soundStreams[theSound->newStreamSlot];
			theStream.shouldDie = false;
      s.streamer.streams[theSound->newStreamSlot].pBuffer = s.soundStreams[theSound->newStreamSlot].pvSourceBuffer[0];
      s.streamer.streams[theSound->newStreamSlot].streaming = true;
      theStream.soundId = theSound->id;
		  ++s.numStreamSounds;
    }
    else
    {
	  _nslClearSoundSlot(theSound->id);
      return NSL_INVALID_ID;
    }
    
#else
		_nslClearSoundSlot(theSound->id);
		return NSL_INVALID_ID;
#endif

		#else
		
		internalAssert(s.numStreamSounds <= NSL_NUM_STREAM_SOUNDS);
		int newStreamSlot = _nslFindNewStreamSlot();
		internalAssert( newStreamSlot != -1 ); // we are out of stream slots
		if (newStreamSlot == -1)
		{
#ifdef NSL_LOAD_SOURCE_BY_NAME
			nslPrintf("NSL ERROR: Ran out of stream slot.  %s is not added.", theSource->sndEntry.fileName);
#endif

#ifdef NSL_LOAD_SOURCE_BY_ALIAS
			nslPrintf("NSL ERROR: Ran out of stream slot.  %d is not added.", theSource->sndEntry.aliasID);
#endif

			_nslClearSoundSlot(theSound->id);
			return NSL_INVALID_ID;
		}
		theSound->streamSlot = newStreamSlot;
		nslStreamSound* theStream = &s.streams[newStreamSlot];
		theStream->soundId = theSound->id;
		theStream->isUsed = true;

		++s.numStreamSounds;

#ifdef NSL_LOAD_SOURCE_BY_NAME
    nslPrintf("NSL INFO: Added Stream %s...", theSource->sndEntry.fileName);
#endif

#ifdef NSL_LOAD_SOURCE_BY_ALIAS
    nslPrintf("NSL INFO: Added Stream %d...", theSource->sndEntry.aliasID);
#endif

		#endif // DISABLE_STREAMS
	}
	// otherwise, it's ready to be played immediately
	// Lock the buffer and copy in the data.
	else
	{
		DS_TRY( s.pDS->CreateSoundBuffer( &theSource->desc, &theSound->pDSBuffer, NULL ) );
		DS_TRY( theSound->pDSBuffer->SetBufferData( theSource->data, theSource->sndEntry.size ) );
		DS_TRY( theSound->pDSBuffer->SetLoopRegion( 0, theSource->sndEntry.size ) );
		DS_TRY( theSound->pDSBuffer->SetCurrentPosition( 0 ) );
	}
	theSound->isQueued = true;
	
	return id;

#endif // DISABLE_SOUNDS
}

nslSoundId  nslAddSoundWithOffset( nslSourceId soundSource, float seconds )
{
	nslPrintf("STUB: nslAddSoundWithOffset\n");
	return	nslAddSound(soundSource);
}

float nslGetSoundPlaybackPosition( nslSoundId s )
{
	nslPrintf("STUB: nslGetSoundPosition\n");
	return 0.0f;
}


void nslPlaySound( nslSoundId soundToPlay )
{
#if !DISABLE_SOUNDS
	RETURN_ON_INVALID_ID(soundToPlay);
	CHECK_ID(soundToPlay);
	nslSound* theSound = &sndSystem.sounds[NSL_SLOT(soundToPlay)];
	internalAssert(theSound->isUsed && "Trying to play a sound that wasn't added.");

	// normal sound is ready immediately
#if !DISABLE_STREAMS
	if (!theSound->streamSlot)
#elif !DISABLE_NEW_STREAMS
	if (theSound->newStreamSlot < 0)
#endif
		theSound->isReady = true;	
#if !DISABLE_STREAMS
	else
	{
		// theSound->isPlaying = true;
		sndSystem.streams[theSound->streamSlot].startPlay = true;	
	}
#endif
#if (!DISABLE_NEW_STREAMS && DISABLE_STREAMS)
	else
  {
    sndSystem.soundStreams[theSound->newStreamSlot].startPlay = true;
/*
		if (theSound->eId != NSL_GLOBAL_EMITTER_ID)
		{
			// calculate listener's position relative to this emitter
			nlVector3dSub(distVec, theEmitter->pos, sndSystem.listenerPos);
			dist = nlVector3dLength(distVec);
			
			if ( dist < theSound->params.minDist )
				newVolume = 1.0f;
			else
			{
				if ( dist > theSound->params.maxDist )
					newVolume = 0.0f;
				else
				{
					if ( abs( theSound->params.maxDist - theSound->params.minDist ) > 0.001 )
						newVolume = ( 1.0f - ( ( dist - theSound->params.minDist ) * ( 1.0f / ( theSound->params.maxDist - theSound->params.minDist ) ) ) );
					else
						newVolume = 1.0f;
				}			
			}
			
			newVolume *= theSound->params.volume / 1.0f;
		}
		else
		{
			newVolume = theSound->params.volume;
		}
		newVolume *= sndSystem.gameVolume[theSource->sndEntry.type] / 1.0f;
		newVolume *= sndSystem.masterVolume / 1.0f;
		
		DS_TRY( theStream->pRenderFilter->SetVolume( _toDecibel(newVolume) ));
		if (theSound->eId != NSL_GLOBAL_EMITTER_ID)
		{
			DS_TRY( theStream->pRenderFilter->SetPosition( theEmitter->pos[0], theEmitter->pos[1], theEmitter->pos[2], DS3D_DEFERRED ) );
		}
*/
  }
#endif // !DISABLE_NEW_STREAMS

#endif
}

void nslPlaySound3D( nslSoundId soundToPlay, const nlVector3d &pos )
{
#if !DISABLE_SOUNDS
	RETURN_ON_INVALID_ID(soundToPlay);
	CHECK_ID(soundToPlay);
	internalAssert(sndSystem.sounds[NSL_SLOT(soundToPlay)].isUsed && "Trying to play a sound that wasn't added.");
	nslEmitterId eId = nslCreateEmitter( pos );
	nslSetSoundEmitter( eId, soundToPlay );
	nslSetEmitterAutoRelease( eId );
	nslPlaySound( soundToPlay );
#endif
}

void nslStopSound( nslSoundId soundToStop )
{
#if !DISABLE_SOUNDS
	RETURN_ON_INVALID_ID(soundToStop);
	CHECK_ID(soundToStop);
	nslSound* theSound = &sndSystem.sounds[NSL_SLOT(soundToStop)];
	
  if( !theSound->isUsed || theSound->isDead )
		return;
	
	// stop the sound
	#if !DISABLE_STREAMS
	if( theSound->streamSlot != NSL_INVALID_ID )
		sndSystem.streams[theSound->streamSlot].shouldDie = true;
	else
	#endif
	{
		// stopping the buffer doesn't guarantee the buffer is ready to be released immediately...
		// mark it dead and have it clear up in the next frame in nslFrameAdvance()
#if !DISABLE_NEW_STREAMS
    if ( theSound->newStreamSlot >= 0 )
    {
	  // sndSystem.streamer.ReleaseStream( theSound->newStreamSlot );
      sndSystem.soundStreams[ theSound->newStreamSlot ].shouldDie = true;
    }
    else
    {
#endif
		  if ( theSound->isPlaying)
		  {
        if ( theSound->eId != NSL_GLOBAL_EMITTER_ID )
        {
          --sndSystem.num3DSounds;
        }
			  DS_TRY( theSound->pDSBuffer->Stop() );
		  }
		  theSound->isDead = true;
      // theSound->isUsed = false;
#if !DISABLE_NEW_STREAMS
    }
#endif
	}
#endif // DISABLE_SOUNDS
}

void nslReleaseAllSounds()
{
#if !DISABLE_SOUNDS
	// stop all the sounds
	nslSound *theSound;
	for(int i=0; i<NSL_NUM_SOUNDS; i++)
	{
		theSound = &sndSystem.sounds[i];
		if(!theSound->isUsed)
			continue;

#if !DISABLE_STREAMS
		if(theSound->streamSlot != NSL_INVALID_ID)
#elif !DISABLE_NEW_STREAMS
		if(theSound->newStreamSlot >= 0)
#endif
		{
			#if !DISABLE_NEW_STREAMS && (DISABLE_STREAMS)
			// This is to allow the stream to really die
			// Please leave it in
			// sndSystem.streamer.ReleaseStream( theSound->newStreamSlot );
				sndSystem.soundStreams[ theSound->newStreamSlot ].shouldDie = true;
        while (theSound->newStreamSlot>=0)
				{
          nslPrintf("killing %d", theSound->newStreamSlot);
					nslFrameAdvance(0.01f); Sleep(10);
				}
			  continue;
			#else
			// let the thread kill the stream itself
			sndSystem.streams[theSound->streamSlot].shouldDie = true;
			Sleep(100);
			#endif
		}
		else
		{
			if (theSound->pDSBuffer)
			{
				DS_TRY( theSound->pDSBuffer->Stop() );
				DS_TRY( theSound->pDSBuffer->SetBufferData( NULL, 0 ) );			
			}
			_nslClearSoundSlot( i );
		}
	}

#if (!DISABLE_NEW_STREAMS && DISABLE_STREAMS)
  // Shutdown the stream file
  CloseHandle( sndSystem.hStreamFile );
#endif

#endif
}

nslSoundStatusEnum nslGetSoundStatus( nslSoundId whichSound )
{
#if DISABLE_SOUNDS
	return NSL_SOUNDSTATUS_INVALID;
#else
	nslSystem &s = sndSystem;
	
  nslSoundId checkSlot = NSL_SLOT(whichSound);
  nslSoundId upCheckId = (whichSound & NSL_ID_MASK);
	
  // ERROR CASES
  if (whichSound == NSL_INVALID_ID)
    return NSL_SOUNDSTATUS_INVALID;
  if (!(s.sounds[checkSlot].isUsed) || s.sounds[checkSlot].isDead)
    return NSL_SOUNDSTATUS_INVALID;
  if (!(checkSlot < NSL_NUM_SOUNDS))
    return NSL_SOUNDSTATUS_INVALID;
  if (!(whichSound == s.sounds[checkSlot].id))
    return NSL_SOUNDSTATUS_INVALID;
  if (!(upCheckId > 0))
    return NSL_SOUNDSTATUS_INVALID;
	
  if (s.sounds[checkSlot].isPaused > 0)
    return NSL_SOUNDSTATUS_PAUSED;
	
  // PLAY CASE
  if (s.sounds[checkSlot].isUsed && s.sounds[checkSlot].isPlaying && (s.sounds[checkSlot].isPaused < 1))
    return NSL_SOUNDSTATUS_PLAYING;
	
	
  // READY CASE
  if (s.sounds[checkSlot].isReady)
    return NSL_SOUNDSTATUS_READY;
	
	
  // QUEUING CASE (THE DEFAULT)
  if (s.sounds[checkSlot].isQueued)
    return NSL_SOUNDSTATUS_QUEUING;
	
  return NSL_SOUNDSTATUS_INVALID;
#endif
}

bool nslIsSoundReady( nslSoundId whichSound )
{
#if DISABLE_SOUNDS
	return false;
#else
	RETURN_WITH_VALUE_ON_INVALID_ID(whichSound, false);
	CHECK_ID2(whichSound, false);
	nslSound *theSound = &sndSystem.sounds[NSL_SLOT(whichSound)];
	
	if(theSound->isUsed)
		return (theSound->isReady);
	else
		return false;
#endif
}

bool nslIsSoundPlaying( nslSoundId whichSound )
{
#if DISABLE_SOUNDS
	return false;
#else
	RETURN_WITH_VALUE_ON_INVALID_ID(whichSound, false);
	CHECK_ID2(whichSound, false);
	nslSoundId id = whichSound;
	nslSound* theSound = &sndSystem.sounds[NSL_SLOT(whichSound)];
	
	if(theSound->id == id && theSound->isUsed && !theSound->isDead)
		return (theSound->isPlaying);
	else
		return false;
#endif
}

void nslPauseSound( nslSoundId soundToPause )
{
#if !DISABLE_SOUNDS
	RETURN_ON_INVALID_ID(soundToPause);
	CHECK_ID(soundToPause);
	nslSound* theSound = &sndSystem.sounds[NSL_SLOT(soundToPause)];
	if (!theSound->isPauseGuarded && theSound->isPlaying && !theSound->isDead)
	{
		theSound->isPaused++;
#if !DISABLE_STREAMS
		if(theSound->streamSlot != NSL_INVALID_ID)
			sndSystem.streams[theSound->streamSlot].pRenderFilter->Pause(DSSTREAMPAUSE_PAUSE);
#elif !DISABLE_NEW_STREAMS
		if(theSound->newStreamSlot >= 0)
			sndSystem.soundStreams[theSound->newStreamSlot].pRenderFilter->Pause(DSSTREAMPAUSE_PAUSE);
#endif
		else
			theSound->pDSBuffer->Stop();
	}
	else
		theSound->isPauseGuarded = false;
#endif
}

void nslUnpauseSound( nslSoundId soundToUnpause )
{
#if !DISABLE_SOUNDS
	RETURN_ON_INVALID_ID(soundToUnpause);
	CHECK_ID(soundToUnpause);
	nslSound* theSound = &sndSystem.sounds[NSL_SLOT(soundToUnpause)];
	if(--theSound->isPaused <= 0 && theSound->isPlaying && !theSound->isDead)
	{
		theSound->isPaused = 0;
#if !DISABLE_STREAMS
		if(theSound->streamSlot != NSL_INVALID_ID)
			sndSystem.streams[theSound->streamSlot].pRenderFilter->Pause(DSSTREAMPAUSE_RESUME);
#elif !DISABLE_NEW_STREAMS
		if(theSound->newStreamSlot >= 0)
			sndSystem.soundStreams[theSound->newStreamSlot].pRenderFilter->Pause(DSSTREAMPAUSE_RESUME);
#endif
		else
			DS_TRY( theSound->pDSBuffer->Play( 0, 0, sndSystem.sources[theSound->srcId].isLooped ? DSBPLAY_LOOPING : 0 ) );
	}
#endif
}

void nslPauseAllSounds()
{
#if !DISABLE_SOUNDS
	for(int i=0; i<NSL_NUM_SOUNDS; i++)
		nslPauseSound(sndSystem.sounds[i].id);
#endif
}

void nslUnpauseAllSounds()
{
#if !DISABLE_SOUNDS
	for(int i=0; i<NSL_NUM_SOUNDS; i++)
		nslUnpauseSound(sndSystem.sounds[i].id);
#endif
}

void nslPauseGuardSound( nslSoundId soundToGuard )
{
#if !DISABLE_SOUNDS
	RETURN_ON_INVALID_ID(soundToGuard);
	CHECK_ID(soundToGuard);
	nslSound* theSound = &sndSystem.sounds[NSL_SLOT(soundToGuard)];
	theSound->isPauseGuarded = true;
	theSound->isPaused = 0;
#endif
}


void nslDampenGuardSound( nslSoundId soundToGuard )
{
#if !DISABLE_SOUNDS
	RETURN_ON_INVALID_ID(soundToGuard);
	CHECK_ID(soundToGuard);
	nslSound *theSound = &sndSystem.sounds[NSL_SLOT(soundToGuard)];
	theSound->dampenCount = -1;
#endif
}

void nslDampenAllSounds( float dampenLevel )
{
#if !DISABLE_SOUNDS
	int i;
	sndSystem.dampenValue = dampenLevel;
	for (i=0; i < NSL_NUM_SOUNDS; i++)
	{
		sndSystem.sounds[i].dampenCount++;
	}
#endif
}

void nslUndampenAllSounds()
{
#if !DISABLE_SOUNDS
	int i;
	sndSystem.dampenValue = 1.0f;
	for (i=0; i < NSL_NUM_SOUNDS; i++)
	{
		sndSystem.sounds[i].dampenCount = 0;
	}
#endif
}

void nslSetSoundRange( nslSoundId soundToSet, float minDist, float maxDist )
{
#if !DISABLE_SOUNDS
	RETURN_ON_INVALID_ID(soundToSet);
	CHECK_ID(soundToSet);
	nslSound *theSound = &sndSystem.sounds[NSL_SLOT(soundToSet)];
	if (minDist < 0.0f)
		;
	else
		theSound->params.minDist = minDist;

	if (maxDist < minDist)
		theSound->params.maxDist = minDist;
	else if (maxDist < 0.0f)
		;
	else
		theSound->params.maxDist = maxDist;
#endif
}

void nslSetSoundParam( nslSoundId soundToSet, nslSoundParamEnum whichParam, float newVal )
{
#if !DISABLE_SOUNDS
	RETURN_ON_INVALID_ID(soundToSet);
	CHECK_ID(soundToSet);
	nslSound* theSound = &sndSystem.sounds[NSL_SLOT(soundToSet)];
	if(!theSound->isUsed)
		return;

	switch (whichParam)
	{
		case NSL_SOUNDPARAM_VOLUME:	
			theSound->params.volume = newVal;			
			break;

		case NSL_SOUNDPARAM_PITCH:
			theSound->params.pitch = newVal;
			int newFreq;
			newFreq = (float)sndSystem.sources[theSound->srcId].sndEntry.format.nSamplesPerSec * newVal;
			if ( newFreq < DSBFREQUENCY_MIN )
				newFreq = DSBFREQUENCY_MIN;
			if ( newFreq > DSBFREQUENCY_MAX )
				newFreq = DSBFREQUENCY_MAX;
			
			if ( theSound->pDSBuffer )
				DS_TRY( theSound->pDSBuffer->SetFrequency( newFreq ) );			
			break;

		case NSL_SOUNDPARAM_MINDIST:
			if(newVal >= 0.0f)	// in Spidey, if newVal is invalid it gets newVal from source data
				theSound->params.minDist = newVal;
			break;

		case NSL_SOUNDPARAM_MAXDIST:
			if(newVal >= 0.0f)	// in Spidey, if newVal is invalid it gets newVal from source data
				theSound->params.maxDist = newVal;
			break;

		default:
			nslFatal("unknown parameter\n");
			break;
	}
#endif // DISABLE_SOUNDS
}

float nslGetSoundParam( nslSoundId whichSound, nslSoundParamEnum whichParam )
{
#if DISABLE_SOUNDS
	return 0.0f;
#else
	RETURN_WITH_VALUE_ON_INVALID_ID(whichSound, 0.0f);
	CHECK_ID2(whichSound, 0.0f);
	nslSound* theSound = &sndSystem.sounds[NSL_SLOT(whichSound)];
	if(!theSound->isUsed)
		return 0.0f;
	
	switch (whichParam)
	{
	#define MAC(a, b) case a##: return (theSound->params.##b##); break;
		#include "nsl_params_mac.h"
		#undef MAC

		default:
			nslFatal("unknown parameter\n");
			break;
	}

	return 0.0f;
#endif
}


/******************** CODE RIPPED AND MODIFIED FROM PS2 VERSION **********************/

float calc_arc_volume( float arcStart, float arcEnd, float currAngle )
{
  // assumes no zero-crossings
  if ( arcStart == arcEnd )
    return 0.0f;
  if ( currAngle > arcStart && currAngle <= arcEnd )
  {
    float range = M_PI / (arcEnd - arcStart);
    float basedAngle = currAngle - arcStart;

    return (sinf( range * basedAngle ));
  }
  return 0.0f;
}


void _setNon3dVolume( nslSound *snd, _nslStreamSound *strm )
{
	float lf  = 1.0f;
	float rf  = 1.0f;
	float  c  = 0.7f * CENTER_DAMPEN_VALUE;
	float ls  = 0.3f * REAR_DAMPEN_VALUE;
	float rs  = 0.3f * REAR_DAMPEN_VALUE;
	float sub = 0.3f;
  DSMIXBINS dsmb;
  DSMIXBINVOLUMEPAIR dsmbvp[8] = {
    {DSMIXBIN_FRONT_LEFT,    _toDecibel(lf)}, // left channel
    {DSMIXBIN_FRONT_RIGHT,   _toDecibel(rf)}, // right channel
    {DSMIXBIN_FRONT_CENTER,  _toDecibel(c)}, // left channel
    {DSMIXBIN_FRONT_CENTER,  _toDecibel(c)}, // right channel
    {DSMIXBIN_BACK_LEFT,     _toDecibel(ls)}, // left channel
    {DSMIXBIN_BACK_RIGHT,    _toDecibel(rs)}, // right channel
    {DSMIXBIN_LOW_FREQUENCY, _toDecibel(sub)}, // left channel
    {DSMIXBIN_LOW_FREQUENCY, _toDecibel(sub)}// right channel
  };
  dsmb.dwMixBinCount = 8;
  dsmb.lpMixBinVolumePairs = dsmbvp;
  if (strm == NULL)
    snd->pDSBuffer->SetMixBins(&dsmb);
  else
  {
    strm->pRenderFilter->SetMixBins(&dsmb);
  }
}


/********************************************************************************
 * bool _set3dVolume(nslSoundId whichSound)
 * sets the left and right values of the sound indicated by 
 * which sound.  
 * it returns true if the sound should really be played
 * false otherwise
 ********************************************************************************/

bool _set3dVolume( nslSound *snd, _nslStreamSound *strm )
{
  unsigned short vol = 0;
  float angle = 0;
  float volumeModifier = 1.0;
  
  // calculate the xz-distance to the sound
  nslEmitter *e = &sndSystem.emitters[snd->eId];

  float xDist = e->pos[0] - sndSystem.listenerPo[0][3];
  float yDist = e->pos[1] - sndSystem.listenerPo[1][3];
  float zDist = e->pos[2] - sndSystem.listenerPo[2][3];
  float xzDistanceSq = xDist*xDist + zDist*zDist;

  angle = _setSoundAngle( snd );
  float checkValue = snd->params.minDist * 0.5f;
  checkValue *= checkValue;
  if (checkValue > 100.0f)
    checkValue = 100.0f;


  // if we get REALLY close to the microphone (50% of min distance), 
  // start drifting towards center, rather than being on either side.
  if (xzDistanceSq < checkValue && checkValue > 0.0f)
  {
    float closenessValue = xzDistanceSq / checkValue;

    // new new new!  Exciting hackey hack to keep sounds close to the 
    // microphone to make things happy and nice.
    if (angle <= M_PI) // right side
    {
      angle = ((angle) * (closenessValue));                 
    }
    else					// left side
    {
      angle = ((M_PI * 2.0f) * (1.0f - (closenessValue))) + 
                   ((angle) * (closenessValue));
      if (angle >= M_PI * 1.9f)
        angle = 0.0f;
    }

  }

  // do math here 
  // to figure out how much volume given angle and distance information
  float lf = 0.0f, rf = 0.0f, c = 0.0f, ls = 0.0f, rs = 0.0f, sub = 0.25f;
  bool in_front = (angle > 1.5f * M_PI || angle <= M_PI * 0.5f);

  float clippedAngle = ( angle + ((M_PI * 3.5f) / 8.0f) );
  if ( clippedAngle > 2.0f * M_PI )
	  clippedAngle -= 2.0f * M_PI;

  c =  calc_arc_volume( (M_PI *  0.0) / 8.0, (M_PI *  8.0) / 8.0, clippedAngle) * CENTER_DAMPEN_VALUE;
  ls = calc_arc_volume( (M_PI *  7.0) / 8.0, (M_PI * 12.0) / 8.0, angle )       * REAR_DAMPEN_VALUE;
  rs = calc_arc_volume( (M_PI *  4.0) / 8.0, (M_PI *  9.0) / 8.0, angle )       * REAR_DAMPEN_VALUE;
  lf = calc_arc_volume( (M_PI *  9.5) / 8.0, (M_PI * 16.0) / 8.0, angle );
  rf = calc_arc_volume( (M_PI *  0.0) / 8.0, (M_PI *  6.5) / 8.0, angle );

  DSMIXBINS dsmb;
  DSMIXBINVOLUMEPAIR dsmbvp[8] = {
		{DSMIXBIN_LOW_FREQUENCY, _toDecibel(sub)},// right channel
		{DSMIXBIN_I3DL2,         _toDecibel(0)},
    {DSMIXBIN_FRONT_LEFT,    _toDecibel(lf)}, // left channel
    {DSMIXBIN_FRONT_RIGHT,   _toDecibel(rf)}, // right channel
    {DSMIXBIN_FRONT_CENTER,  _toDecibel(c)}, // right channel
    {DSMIXBIN_BACK_LEFT,     _toDecibel(ls)}, // left channel
    {DSMIXBIN_BACK_RIGHT,    _toDecibel(rs)}, // right channel
    {DSMIXBIN_LOW_FREQUENCY, _toDecibel(sub)}, // left channel
    
  };
  dsmb.dwMixBinCount = 8;
  dsmb.lpMixBinVolumePairs = dsmbvp;
  if (strm == NULL)
    snd->pDSBuffer->SetMixBins(&dsmb);
  else
  {
    strm->pRenderFilter->SetMixBins(&dsmb);
  }
	
  return true;
}

// PROBABLY SLOW!!!!
float _setSoundAngle( nslSound *snd ) 
{
  // We need to calculate the angle
  nlVector3d forward,toSound, up, newUp;
  float angle = 0.0f;

  // cachy cache in pointers
  nslEmitter *e = &sndSystem.emitters[snd->eId];

  // Setup a few Vectors
  forward[0] = 0;
  forward[1] = 0; 
  forward[2] = 1;
  up[0] = 0;
  up[1] = 1; 
  up[2] = 0; 

  toSound[0] = e->pos[0]-sndSystem.listenerPo[0][3];
  toSound[1] = e->pos[1]-sndSystem.listenerPo[1][3];
  toSound[2] = e->pos[2]-sndSystem.listenerPo[2][3];

  // Normalize toSound
  // A SQUARE ROOT!
  float length = sqrtf(toSound[0]*toSound[0] + toSound[2]*toSound[2]);

  // Nothin too small...
  if (length < .0001) 
  {
	  // bail out early if the toSound is directly up.
	  return 0.0;
  }
  toSound[0] /= length; 
  toSound[1] = 0.0f;
  toSound[2] /= length; 
  
  // Calc the new forward
  //nlTransformVector(forward, nsl.listenerPo, forward);
  forward[0] = sndSystem.listenerPo[0][0]*forward[0] + sndSystem.listenerPo[1][0]*forward[1] + sndSystem.listenerPo[2][0]*forward[2];
  forward[1] = sndSystem.listenerPo[0][1]*forward[0] + sndSystem.listenerPo[1][1]*forward[1] + sndSystem.listenerPo[2][1]*forward[2];
  forward[2] = sndSystem.listenerPo[0][2]*forward[0] + sndSystem.listenerPo[1][2]*forward[1] + sndSystem.listenerPo[2][2]*forward[2];

  // Normalize Forward
	XGVec3Normalize(NL2XGVEC3(forward), NL2XGVEC3(forward) );

  // Get the cos(theta)
  //float cosTheta = toSound[0]*forward[0]+toSound[1]*forward[1]+toSound[2]*forward[2];
  float cosTheta = XGVec3Dot(NL2XGVEC3(toSound), NL2XGVEC3(forward));

  if (cosTheta >= .999) {
    angle = 0;
  } 
  else if (cosTheta <= -.999) 
  {
    angle = M_PI;
  } 
  else 
  {
    // We find the angle.. 
    // Not my code, really..
    //nlCrossProduct3d(newUp, toSound, forward);
    XGVec3Cross(NL2XGVEC3(newUp), NL2XGVEC3(toSound), NL2XGVEC3(forward));

    // Normalize newUp
		XGVec3Normalize(NL2XGVEC3(newUp), NL2XGVEC3(newUp));

    //nlTransformVector(up, nsl.listenerPo, up);
    //XGVec3TransformCoord(&(XGVECTOR3&)up, &(XGVECTOR3&)up, &(XGMATRIX&)sndSystem.listenerPo);
		up[0] = sndSystem.listenerPo[0][0]*up[0] + sndSystem.listenerPo[1][0]*up[1] + sndSystem.listenerPo[2][0]*up[2];
		up[1] = sndSystem.listenerPo[0][1]*up[0] + sndSystem.listenerPo[1][1]*up[1] + sndSystem.listenerPo[2][1]*up[2];
		up[2] = sndSystem.listenerPo[0][2]*up[0] + sndSystem.listenerPo[1][2]*up[1] + sndSystem.listenerPo[2][2]*up[2];

    // Normalize up
		XGVec3Normalize(NL2XGVEC3(up), NL2XGVEC3(up) );

    //float cosPhi = nlDotProduct3d( up, newUp );
    float cosPhi = XGVec3Dot(NL2XGVEC3(up), NL2XGVEC3(newUp));
    angle = acosf(cosTheta);
    if (cosPhi >= 0.0) 
      angle = -angle;

    while (angle < 0.0f) 
    {
      angle += (M_PI*2.0f);
    }
  }
  return angle;
}
