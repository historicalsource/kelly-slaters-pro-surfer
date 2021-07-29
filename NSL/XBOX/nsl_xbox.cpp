//
// Sound Library - API
//
// Treyarch LLC, June 2001
//
// (designed by Wade Brainerd, Jamie Fristrom, Andy Chien, and Greg Taylor)
//

#include "nsl_xbox.h"

nslSystem sndSystem;

//
//
// System functions
//
//
void nslFatal( const char* Format, ... )
{
#if defined(BUILD_BOOTABLE)
	return;
#else
	if (sndSystem.callbacks.CriticalError)
		sndSystem.callbacks.CriticalError(Format);
	else
	{
		va_list vlist;
		va_start(vlist, Format);
		char fmtbuff[2048];
		vsprintf(fmtbuff, Format, vlist);
		OutputDebugString(fmtbuff);
		OutputDebugString("\n");
		__asm{ int 3 };
	}
#endif
}

void nslPrintf( const char* format, ...)
{
#if defined(BUILD_BOOTABLE)
	return;
#else
	va_list vlist;
	va_start(vlist, format);
	char fmtbuff[2048];
	vsprintf(fmtbuff, format, vlist);
	if (sndSystem.callbacks.DebugPrint)
	{
		sndSystem.callbacks.DebugPrint(fmtbuff);
	}
	else
	{
		OutputDebugString(fmtbuff);
		OutputDebugString("\n");				
	}
#endif
}

void _nslAssert(const char* exp_str, const char* file_name, int line)
{
#if defined(BUILD_BOOTABLE)
	return;
#else
	char fmtbuff[2048];
	sprintf(fmtbuff, "%s in %s:%d\n", exp_str, file_name, line);
	nslFatal(fmtbuff);
#endif
}


HANDLE nslOpenStream( const char* FileName )
{
  char lang[32];

  if ( sndSystem.language != NSL_LANGUAGE_NONE )
  {
    strcpy( lang, nslLanguageStr[sndSystem.language] );
    strcat( lang, "\\" );
  }
  else
  {
    lang[0] = 0;
  }

	// try to open language specific file
	char tmp[512];
	if (sndSystem.rootDir)
		sprintf(tmp, "%s\\%s%s", sndSystem.rootDir, lang, FileName);
	else
		sprintf(tmp, "%s%s", lang, FileName);

	HANDLE f = CreateFile( tmp, GENERIC_READ,
                         FILE_SHARE_READ, NULL, OPEN_EXISTING,
                         FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING,
                         NULL );
	
	if (f == INVALID_HANDLE_VALUE)
  {
		// then try to open generic file
		char tmp[512];
		if (sndSystem.rootDir)
			sprintf(tmp, "%s\\%s", sndSystem.rootDir, FileName);
		else
			sprintf(tmp, "%s", FileName);

		f = CreateFile( tmp,
                    GENERIC_READ,
                    FILE_SHARE_READ, NULL,
                    OPEN_EXISTING,
                    FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING,
                    NULL );
		
  	if (f == INVALID_HANDLE_VALUE)
			return f;
  }
	
  return f;
}

bool nslReadFile( const char* FileName, nslFileBuf* File, unsigned int Align )
{
  char lang[32];

  if ( sndSystem.language != NSL_LANGUAGE_NONE )
  {
    strcpy( lang, nslLanguageStr[sndSystem.language] );
    strcat( lang, "\\" );
  }
  else
  {
    lang[0] = 0;
  }

	if (sndSystem.callbacks.ReadFile)
  {
		char tmp[512];

    // try to open language specific sound
		if (sndSystem.rootDir)
			sprintf(tmp, "%s\\%s%s", sndSystem.rootDir, lang, FileName);
		else
			sprintf(tmp, "%s%s", lang, FileName);

    if ( sndSystem.callbacks.ReadFile(tmp, File, Align) )
    {
      return true;
    }
    else
    {
      // then try to open generic file
		  if (sndSystem.rootDir)
			  sprintf(tmp, "%s\\%s", sndSystem.rootDir, FileName);
		  else
			  sprintf(tmp, "%s", FileName);

  		return ( sndSystem.callbacks.ReadFile(tmp, File, Align) );
    }
  }
	else
  {
		// try to open language specific file
		char tmp[512];
		if (sndSystem.rootDir)
			sprintf(tmp, "%s\\%s%s", sndSystem.rootDir, lang, FileName);
		else
			sprintf(tmp, "%s%s", lang, FileName);

		HANDLE f = CreateFile(tmp, GENERIC_READ, FILE_SHARE_READ, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		
		if (f == INVALID_HANDLE_VALUE)
    {
		  // then try to open generic file
		  char tmp[512];
		  if (sndSystem.rootDir)
			  sprintf(tmp, "%s\\%s", sndSystem.rootDir, FileName);
		  else
			  sprintf(tmp, "%s", FileName);

		  f = CreateFile(tmp, GENERIC_READ, FILE_SHARE_READ, NULL,
			  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		  
  		if (f == INVALID_HANDLE_VALUE)
			  return false;
    }
		
		// get file size
		File->Size = GetFileSize(f, NULL);
		
		// read file
    File->Buf = (nlUint8*)nslMemAlloc(File->Size);
		nlUint32 bytesRead;
		if (!ReadFile(f, File->Buf, File->Size, (DWORD*)&bytesRead, NULL))
		{
			nslFatal("NSL ERROR: Can't read \"%s\" !\n", FileName);
      nslMemFree(File->Buf);
			return false;
		}
		if (bytesRead != File->Size)
		{
			nslFatal("NSL ERROR: Can't read \"%s\" !\n", FileName);
      nslMemFree(File->Buf);
			return false;
		}
		CloseHandle(f);
		
    return true;
  }
}

void nslReleaseFile( nslFileBuf* File )
{
	if (sndSystem.callbacks.ReleaseFile)
		sndSystem.callbacks.ReleaseFile( File );
	else
  {
    nslMemFree(File->Buf);
		File->Buf = NULL;
    File->Size = 0;
  }
}

void* nslMemAlloc( unsigned int Size, unsigned int Align )
{
	if (sndSystem.callbacks.MemAlloc)
		return (sndSystem.callbacks.MemAlloc(Size, Align));
	else
		return malloc(Size);
}

void nslMemFree( void* Ptr )
{
	if (sndSystem.callbacks.MemFree)
		sndSystem.callbacks.MemFree(Ptr);
	else
		free(Ptr);
}

void nslSetSystemCallbacks( nslSystemCallbackStruct* Callbacks )
{
	memcpy(&sndSystem.callbacks, Callbacks, sizeof(nslSystemCallbackStruct));
}

void nslSetRootDir( const char* newDir)
{
  sprintf(sndSystem.rootDir, newDir);
  if (sndSystem.rootDir[strlen(sndSystem.rootDir)-1] == '\\')
    sndSystem.rootDir[strlen(sndSystem.rootDir)-1] = '\0';
}

static bool _nslHardwareInit()
{
  // Create the DirectSound object, set the coop level and get the device's caps.
  HRESULT hr = DirectSoundCreate( NULL, &sndSystem.pDS, NULL );
  if ( FAILED(hr) )
	{
		nslPrintf("NSL ERROR: DirectSoundCreate failed\n");
		return false;
	}

	// turn off DSound rolloff factor and do it manually
	sndSystem.pDS->SetRolloffFactor(DS3D_MINROLLOFFFACTOR, DS3D_IMMEDIATE);
	DirectSoundUseLightHRTF();
	if (!nslLoadDSPImageXbox("nslDSP.bin", DSFX_IMAGELOC_UNUSED, 1))
  {
    nslPrintf("NSL ERROR: Could not find nslDSP.bin\n");
    int count = sndSystem.pDS->Release();
    internalAssert( count == 0 );
    sndSystem.pDS=NULL;
    return false;
  }
	
	return true;
}

bool nslInit()
{
	int i;

	if (sndSystem.initialized)
	{
		nslPrintf("NSL WARNING: Already Initialized");
		return true;
	}

	// init hardware
	if ( !_nslHardwareInit() )
		return false;

	nslSystem &s = sndSystem;
  s.killThread = false;
	// init resources
	s.masterVolume = 1.0f;
  //s.rootDir[0] = '\0';
	for(i=0; i<NSL_SOURCETYPE_Z; i++)
		s.gameVolume[i] = 1.0f;
	s.numStreamSounds = 
	s.numSources =
	s.numSounds =
	s.numEmitters =
	s.num3DSounds = 0;
	// create global emitter
	s.emitters[NSL_GLOBAL_EMITTER_ID].isUsed = true;
	++s.numEmitters;
	s.dampenValue = 1.0f;
	// create perminant stream buffers

#if (!DISABLE_NEW_STREAMS && DISABLE_STREAMS)
  for (i = 0; i < NSL_NUM_SOUNDS; i++)
  {
    s.sounds[i].newStreamSlot = -1;
  }

  for (i = 0; i < MaxXBoxStreams; i++)
  {
    memset( &s.soundStreams[i], 0, sizeof(nslNewStreamSound) );

    s.soundStreams[i].currentPacket = -1;

    for (int j = 0; j < XBoxStreamPackets; j++)
    {
      // allocate memory for the XMEDIAPACKETS
      s.soundStreams[i].pvSourceBuffer[j] =
        XPhysicalAlloc( XBoxStreamBufSize,
											  MAXULONG_PTR,
											  0,
											  PAGE_READWRITE | PAGE_NOCACHE );
    }
  }
#endif

#if !DISABLE_STREAMS
	for(i=0; i<NSL_NUM_STREAM_SOUNDS; i++)
		s.streams[i].pvSourceBuffer = XPhysicalAlloc( FILESTRM_PACKET_BYTES * FILESTRM_PACKET_COUNT,
																								MAXULONG_PTR,
																								0,
																								PAGE_READWRITE | PAGE_NOCACHE );
#endif

#if !DISABLE_STREAMS
	s.hWorkderThread = CreateThread( NULL, 0, _nslStreamThreadProc, 0, 0, NULL );
#endif

  // Use english by default
  s.language = NSL_LANGUAGE_ENGLISH;
	s.initialized = true;

	return true;
}

void _str2upper( char *str )
{
	int length = strlen(str);
	for(int i=0; i<length; ++i)
		str[i] = toupper(str[i]);
}

inline float _toDecibel( float f )
{
	float decibel_scale;
	decibel_scale = logf(f*f)*1000.0F*0.43429448f;
	if (decibel_scale<DSBVOLUME_MIN) 
				decibel_scale = DSBVOLUME_MIN;
	else if (decibel_scale>DSBVOLUME_MAX) 
				decibel_scale = DSBVOLUME_MAX;	
	
	return (float)decibel_scale;
}

bool nslReset( const char *soundListfile, nslLanguageEnum language )
{
	if (!sndSystem.initialized)
		return false;
	static char soundBankfile[256];
	char *token;

	nslSystem &s = sndSystem;
	nslFileBuf listBuf;

  // only change the language if they explicitly specify it, else keep the old one
  if (language != NSL_LANGUAGE_Z)
  {
    sndSystem.language = language;
  }

	// clear the previous sound information
	nslReleaseAllSounds();
	_nslReleaseAllEmitters();
	_nslReleaseAllSources();
	sndSystem.numEmitters = 0;

  strcpy(soundBankfile, soundListfile);

	internalAssert(sndSystem.numStreamSounds == 0);
	internalAssert(sndSystem.numEmitters <= 1);
	internalAssert(sndSystem.numSounds == 0);
	internalAssert(sndSystem.numSources == 0);
	internalAssert(sndSystem.num3DSounds == 0);	
	s.loadedSndFile = false;
	s.id = 0;
  strcpy(sndSystem.bankFileName, soundBankfile);

  if ( _stricmp(soundBankfile + strlen(soundBankfile) - strlen(".XSH"), ".XSH") != 0)
  {
    strcat(soundBankfile, ".XSH");
  }
  else
  {
    sndSystem.bankFileName[ strlen(sndSystem.bankFileName) - strlen(".XSH") ] = 0;
  }

	// try to load up header file
	if ( !(nslReadFile( soundBankfile, &listBuf, 0)) )
	{
		nslPrintf( "NSL ERROR: Couldn't load %s", soundBankfile );
		//internalAssert(0);
		return false;
	}

#if (!DISABLE_NEW_STREAMS && DISABLE_STREAMS)
  // try to open stream file
	// token = strrchr(soundBankfile, '.');
	// strcpy(token, NSL_XBOX_STREAM_SND_EXT);
  s.hStreamFile = nslOpenStream( STREAM_FILENAME );
#endif

	// try to load the sound bank
	token = strrchr(soundBankfile, '.');
	strcpy(token, NSL_XBOX_SND_BANK_EXT);
	if ( !(nslReadFile( soundBankfile, &s.soundBank, 0)) )
	{
		nslReleaseFile( &listBuf );
		nslPrintf( "NSL ERROR: Couldn't load %s", soundBankfile );
		return false;		
	}

	// parse the header file and create a reference map
	nslXBoxSndHeader* theHeader = (nslXBoxSndHeader*)listBuf.Buf;
	if( theHeader->version != nslXBoxSndVersion )
	{
		nslFatal( "NSL ERROR: Snd bank is version %d, expecting %d\n", theHeader->version, nslXBoxSndVersion );
		nslReleaseFile( &listBuf );
		nslReleaseFile( &s.soundBank );
		return false;
	}
	s.loadedSndFile = true;
	
	nslXBoxSndEntry* curEntry = (nslXBoxSndEntry*)(listBuf.Buf + sizeof(nslXBoxSndHeader));
	nslSource *theSource;
	DWORD i;
	for( i=0; i<theHeader->entryCnt; i++ )
	{
		theSource = &s.sources[i+1];
		memset( theSource, 0, sizeof(nslSource) );
		theSource->id = i+1;	// zero is reserved for invalid id
		memcpy( &(theSource->sndEntry), &(curEntry[i]), sizeof(nslXBoxSndEntry) );
		memcpy( &(theSource->format.wfx), &(curEntry[i].format), sizeof(WAVEFORMATEX) );
		theSource->format.wSamplesPerBlock = 64;

#ifdef NSL_LOAD_SOURCE_BY_NAME
		_str2upper(theSource->sndEntry.fileName);
		_nslProcessSndEntry(theSource->sndEntry.fileName);
#endif

#ifdef NSL_LOAD_SOURCE_BY_ALIAS
		_nslProcessSndEntry(theSource->sndEntry.aliasID);
#endif
	}

	nslReleaseFile( &listBuf );
  return true;
}

void nslReset(  )
{
  internalAssert(0 && "NOT DEFINED")
}
void nslShutdown()
{
  if (!sndSystem.initialized)
    return;
  
	
	
	
	// shutdown sounds
	nslReleaseAllSounds();

	// shutdown emitters
	_nslReleaseAllEmitters();
	sndSystem.numEmitters = 0;
	// shutdown sources
	_nslReleaseAllSources();

	#if !DISABLE_STREAMS
	// clear stream memory
	for(int i=0; i<NSL_NUM_STREAM_SOUNDS; i++)
		if(sndSystem.streams[i].pvSourceBuffer)
			XPhysicalFree(sndSystem.streams[i].pvSourceBuffer);
	#endif

#if (!DISABLE_NEW_STREAMS && DISABLE_STREAMS)
	nslSystem &s = sndSystem;

  for (int i = 0; i < MaxXBoxStreams; i++)
  {
    s.soundStreams[i].currentPacket = -1;

    for (int j = 0; j < XBoxStreamPackets; j++)
    {
      // free allocated memory for the XMEDIAPACKETS
      XPhysicalFree( s.soundStreams[i].pvSourceBuffer[j] );
    }

    memset( &s.soundStreams[i], 0, sizeof(nslNewStreamSound) );
  }
#endif

	// shutdown hardware
  int count;
  if ( sndSystem.pDS )
  {
    count = sndSystem.pDS->Release();
		// Removed because other things can have references (like movies!)
    //internalAssert( count == 0 );
    sndSystem.pDS=NULL;
  }

#if !DISABLE_STREAMS
  sndSystem.killThread = true;
  while (sndSystem.killThread) Sleep(10);
  CloseHandle(sndSystem.hWorkderThread);
#endif

	sndSystem.initialized = false;
}

#if (DISABLE_STREAMS) && (!DISABLE_NEW_STREAMS)

void nslNewStreamFrameAdvance( nslSound * theSound, float newVolume )
{
	nslSystem &s = sndSystem;

  nslNewStreamSound& theStream = s.soundStreams[theSound->newStreamSlot];
  XBoxSingleStream& xboxStream = s.streamer.streams[theSound->newStreamSlot];

  if ( theStream.shouldDie )
  {
    void _nslClearNewStreamSlot( nslNewStreamId id );
    _nslClearNewStreamSlot( theSound->newStreamSlot );

    return;
  }

  if ( xboxStream.ready )
  {
	  theSound->isReady = true;
  }

  // New streams process goes here
  if ( theStream.startPlay &&
       !theStream.isInitialized )
  {
    // Should initialize this one as soon as it's ready
    if ( xboxStream.ready )
    {
      bool _nslInitNewStream(nslNewStreamId id);
      _nslInitNewStream( theSound->newStreamSlot );
      theStream.isInitialized = true;
      theSound->isPlaying = true;
      theSound->isQueued = false;
    }
    else
    {
      // nslPrintf( "Warning: stream not ready yet!\n" );
    }
  }

  if ( theStream.startPlay &&
       theStream.isInitialized )
  {
    if ( xboxStream.ready )
    {
      theStream.startPlay = false;
      theStream.currentPacket = 0;
      theStream.playingPacket = 0;
      theStream.lastPacket = -1;
    }
  }

  if ( !theStream.startPlay &&
       theStream.isInitialized )
  {
    if ( theStream.lastPacket >= 0 )
    {
      // finished playing last packet
      if ( ( theStream.dwPacketStatus[theStream.lastPacket] == XMEDIAPACKET_STATUS_SUCCESS ) &&
           ( !theStream.isPacketReady[ theStream.currentPacket ] ) )
      {
        // s.streamer.ReleaseStream( theSound->newStreamSlot );
        void _nslClearNewStreamSlot( nslNewStreamId id );
        _nslClearNewStreamSlot( theSound->newStreamSlot );
        return;
      }
    }

    // Load next packet if free
    if ( xboxStream.ready )
    {
      if (!theStream.isPacketReady[ theStream.currentPacket ] && (theStream.lastPacket < 0) )
      {
        // nslPrintf( "Loaded %d", theStream.currentPacket );
        theStream.isPacketReady[ theStream.currentPacket ] = true;
        theStream.dwTransferred[ theStream.currentPacket ] = xboxStream.transferred;
        if ( xboxStream.finished )
        {
          theStream.lastPacket = theStream.currentPacket;
          // s.streamer.ReleaseStream( theSound->newStreamSlot );
        }
      }

      if ( !xboxStream.finished )
      {
        int nextPacket = (theStream.currentPacket + 1) % XBoxStreamPackets;

        if ( theStream.dwPacketStatus[ nextPacket ] == XMEDIAPACKET_STATUS_SUCCESS )
        {
          // Start streaming next packet
          // nslPrintf( "Streaming %d", nextPacket );
          theStream.currentPacket = nextPacket;
          xboxStream.ready = false;
          xboxStream.pBuffer = theStream.pvSourceBuffer[theStream.currentPacket];
        }
      }
    }

    // Start playing next packet if ready
    if ( theStream.isPacketReady[theStream.playingPacket] &&
         theStream.dwPacketStatus[(theStream.playingPacket + XBoxStreamPackets - 2) % XBoxStreamPackets] == XMEDIAPACKET_STATUS_SUCCESS )
    {
      XMEDIAPACKET& xmp = theStream.xmp;
	    ZeroMemory( &xmp, sizeof(xmp) );
	    xmp.dwMaxSize = theStream.dwTransferred[ theStream.playingPacket ];
      //nslPrintf( "cur: %d, play: %d, transf: %d",
      //           theStream.currentPacket,
      //           theStream.playingPacket,
      //           xmp.dwMaxSize );

      // Set the volume/pos here

			if (newVolume > 0.0f)
			{
				if (theSound->eId != NSL_GLOBAL_EMITTER_ID)
					_set3dVolume( theSound, &theStream );
				else
					_setNon3dVolume( theSound, &theStream );
			}
			DS_TRY( theStream.pRenderFilter->SetVolume( _toDecibel(newVolume) ));


		/*	DS_TRY( theStream.pRenderFilter->SetVolume( (long)_toDecibel(newVolume) ));
			if (theSound->eId != NSL_GLOBAL_EMITTER_ID)
			{
				nslEmitter* theEmitter = &sndSystem.emitters[theSound->eId];
				DS_TRY( theStream.pRenderFilter->SetPosition( theEmitter->pos[0], theEmitter->pos[1], theEmitter->pos[2], DS3D_DEFERRED ) );
			}*/
      
      HRESULT _nslNewProcessRenderer( nslNewStreamId id, DWORD packetIndex, XMEDIAPACKET& xmp );
      if ( _nslNewProcessRenderer( theSound->newStreamSlot,
                                   theStream.playingPacket,
                                   theStream.xmp ) )
      {
        theStream.isPacketReady[theStream.playingPacket] = false;
        theStream.playingPacket = (theStream.playingPacket + 1) % XBoxStreamPackets;
      }
      else
      {
        nslPrintf( "nslNewProcessRenderer didn't complete\n" );
      }
    }
  }
}
#endif

void nslFrameAdvance( float TimeElapsed )
{
	if (!sndSystem.initialized || !sndSystem.loadedSndFile)
		return;
	
	nslSystem &s = sndSystem;
	DWORD status;

	// update emitters
	nslEmitter *theEmitter;
	nslSound *theSound;
	nslSound *prevSound;
	nlVector3d distVec;
	float dist;	
	float newVolume = 1.0f;
	int i;

#if !DISABLE_NEW_STREAMS
  s.streamer.FrameAdvance( TimeElapsed );
#endif

	for(i=0; i<NSL_NUM_EMITTERS; ++i)
	{
		theEmitter = &sndSystem.emitters[i];

		if (!theEmitter->isUsed)
			continue;
		
		// if all of an emitter's sounds are dead and it is set to auto-release, free it up
		if (theEmitter->isAutoRelease && theEmitter->soundList == NULL)
			nslReleaseEmitter(i);

		// if the emitter was released externally, it should only be killed if the sound queue is empty
		if (theEmitter->isDead && theEmitter->soundList == NULL)
		{
			_nslClearEmitter(i);
			continue;
		}

		// skip the emitter positional calculation if it doesn't even have active sounds
		if (!theEmitter->soundList)
			continue;

		// update the emitter's sound queue
		theSound = theEmitter->soundList;
		prevSound = NULL;

		// calculate listener's position relative to this emitter
		nlVector3dSub(distVec, theEmitter->pos, s.listenerPos);
		dist = nlVector3dLength(distVec);
		while(theSound)
		{	
			internalAssert( theSound->eId == i);

      // streamed sound are handled in its own thread (USED TO BE!)
#if !DISABLE_STREAMS
			if (!theSound->streamSlot)
#elif !DISABLE_NEW_STREAMS
      if (theSound->newStreamSlot < 0)
#endif
			{
				if(theSound->isUsed && !theSound->isDead)
				{				
					if(theSound->isPaused <= 0)
					{
						if(theSound->isPlaying)
						{
							// check to see if it's done playing
							DS_TRY( theSound->pDSBuffer->GetStatus( &status ) );

							// kill the sound if it's done playing
							if (!(status&DSBSTATUS_PLAYING) && !s.sources[theSound->srcId].isLooped)
							{
								nslStopSound(theSound->id);
							}
						} 
						else // the sound is not playing, check and see if it needs to be played
						{
							if(theSound->isQueued && theSound->isReady )
							{
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
    
									theSound->pDSBuffer->SetMixBins( &dsmixbins );
								}

								DS_TRY( theSound->pDSBuffer->Play( 0, 0, s.sources[theSound->srcId].isLooped ? DSBPLAY_LOOPING : 0 ) );								
								theSound->isPlaying = true;
							}
						}
					} // theSound->isPaused <= 0
				}
				else	// the sound is either dead or no longer used, clear its buffer
				{
					_nslClearSoundSlot( theSound->id );
					if(prevSound)
						theSound = prevSound;
					else
						theSound = theEmitter->soundList;					
				}
			} // if(!theSound->streamSlot)

			// update the volume of this sound if it wasn't killed
			if (theSound)
			{
				// update sound volume according to the emitter's position relative to the listener
				// except for the global emitter
				if (i != NSL_GLOBAL_EMITTER_ID)
				{
					if ( dist < theSound->params.minDist )
						newVolume = 1.0f;
					else
					{
						if ( dist > theSound->params.maxDist )
							newVolume = 0.0f;
						else
						{
							if ( fabs( theSound->params.maxDist - theSound->params.minDist ) > 0.001f )
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
				newVolume *= s.gameVolume[s.sources[theSound->srcId].sndEntry.type] / 1.0f;
				newVolume *= s.masterVolume / 1.0f;
				if (theSound->dampenCount > 0)
					newVolume *= sndSystem.dampenValue;
				// the stream sound's volume setting happens in worker's thread
#if !DISABLE_STREAMS
				if (!theSound->streamSlot)
#elif !DISABLE_NEW_STREAMS
        if (theSound->newStreamSlot < 0)
#endif
				{
					if (theSound->pDSBuffer)
					{
						if (newVolume > 0.0f)
						{
							if (theSound->eId != NSL_GLOBAL_EMITTER_ID)
								_set3dVolume( theSound, NULL );
							else
								_setNon3dVolume( theSound, NULL );
						}
						DS_TRY( theSound->pDSBuffer->SetVolume( _toDecibel(newVolume) ) );
					}
				}

#if (DISABLE_STREAMS) && !(DISABLE_NEW_STREAMS)
        if (theSound->newStreamSlot >= 0)
        {
          nslNewStreamFrameAdvance( theSound, newVolume );
        }
#endif
			}

			prevSound = theSound;
			if(theSound)
				theSound = theSound->next;
		} // while(currentSound)
	} // for emitters

	// commit the 3D sound calculation
	sndSystem.pDS->CommitDeferredSettings();
	
	DirectSoundDoWork();	
}

void nslSetMasterVolume( float NewVolume )
{
	internalAssert( NewVolume >= 0.0f && NewVolume <= 1.0f );
	sndSystem.masterVolume = NewVolume;
}

float nslGetMasterVolume()
{
	return sndSystem.masterVolume;
}

void nslSetVolume( nslSourceTypeEnum typeOfSound, float newVolume )
{
	internalAssert( newVolume >= 0.0f && newVolume <= 1.0f );
	sndSystem.gameVolume[typeOfSound] = newVolume;
}

float nslGetVolume( nslSourceTypeEnum typeOfSound)
{
	internalAssert( typeOfSound < NSL_SOURCETYPE_Z );
	return ( sndSystem.gameVolume[typeOfSound] );
}

void nslSetListenerPo( const nlMatrix4x4 &positionAndOrientation )
{
	// To be implemented...  But using nslSetListenerPosition and nslSetListenerOrientation would be quicker...
	memcpy(&sndSystem.listenerPo, positionAndOrientation, sizeof(nlMatrix4x4));
}

void nslGetListenerPo( nlMatrix4x4 *dest )
{
	// To be implemented...  But using nslSetListenerPosition and nslSetListenerOrientation would be quicker...
	memcpy(dest, &sndSystem.listenerPo, sizeof(nlMatrix4x4));
}

void nslSetListenerPosition( const nlVector3d &newPosition )
{
	sndSystem.listenerPo[0][3] = sndSystem.listenerPos[0] = newPosition[0];
	sndSystem.listenerPo[1][3] = sndSystem.listenerPos[1] = newPosition[1];
	sndSystem.listenerPo[2][3] = sndSystem.listenerPos[2] = newPosition[2];
	sndSystem.pDS->SetPosition( newPosition[0], newPosition[1], newPosition[2], DS3D_DEFERRED );
}

void nslSetListenerOrientation( const nlVector3d &facingVector, const nlVector3d &upVector )
{
	sndSystem.pDS->SetOrientation( facingVector[0], facingVector[1], facingVector[2],
																 upVector[0], upVector[1], upVector[2],
																 DS3D_DEFERRED );
}

void nslSetSpeakerMode( nslSpeakerModeEnum newMode )
{
	internalAssert( newMode >= 0 && newMode < NSL_SPEAKER_Z );
	sndSystem.speakerMode = newMode;
}

nslSpeakerModeEnum nslGetSpeakerMode( )
{
#if 0 // for some reason the speaker modes don't exist in dsound.h
	LPDWORD pdwSpeakerConfig;
	DS_TRY( sndSystem.pDS->GetSpeakerConfig( pdwSpeakerConfig ) );

	switch(*pdwSpeakerConfig)
	{
		case DSSPEAKER_ANALOG_MONO:
			sndSystem.speakerMode = NSL_SPEAKER_MONO;
			break;
		case DSSPEAKER_ANALOG_SURROUND:
			sndSystem.speakerMode = NSL_SPEAKER_PROLOGIC;
			break;
		case DSSPEAKER_DIGITAL_SURROUND:
		case DSSPEAKER_DIGITAL_AC3:
		case DSSPEAKER_DIGITAL_DTS:
			sndSystem.speakerMode = NSL_SPEAKER_DOLBY_51;
			break;
		case DSSPEAKER_ANALOG_STEREO:
		default:
			sndSystem.speakerMode = NSL_SPEAKER_STEREO;
			break;			
	}
#endif
	return (sndSystem.speakerMode);
}

void nslSetOutputMode( nslOutputModeEnum newMode )
{
	internalAssert( newMode >= 0 && newMode < NSL_OUTPUT_Z);
	sndSystem.outputMode = newMode;

	switch(newMode)
	{
		case NSL_SPEAKER_MONO:
			DirectSoundOverrideSpeakerConfig(DSSPEAKER_MONO);
			break;
		case NSL_SPEAKER_STEREO:
		case NSL_SPEAKER_HEADPHONE:
			DirectSoundOverrideSpeakerConfig(DSSPEAKER_STEREO);
			break;
		case NSL_SPEAKER_PROLOGIC:
		case NSL_SPEAKER_DOLBY_51:
			DirectSoundOverrideSpeakerConfig(DSSPEAKER_SURROUND);
			break;
		default:
			break;
	}
}



