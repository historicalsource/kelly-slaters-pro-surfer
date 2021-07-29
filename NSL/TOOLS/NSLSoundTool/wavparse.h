//-----------------------------------------------------------------------------
// File: wvaparse.h
//
// Desc: Helper class for reading and writing a .wav file 
//
// Hist: 6.12.01 - adapated from xbsound.h
//
// Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef XBSOUND_H
#define XBSOUND_H

#include "windows.h"
#include <mmsystem.h>

//-----------------------------------------------------------------------------
// FourCC definitions
//-----------------------------------------------------------------------------
const DWORD FOURCC_WAVE   = 'EVAW';
const DWORD FOURCC_FORMAT = ' tmf';
const DWORD FOURCC_DATA   = 'atad';
const DWORD FOURCC_WAVESAMPLE = 'pmsw';

//-----------------------------------------------------------------------------
// Name: RIFFHEADER
// Desc: For parsing WAV files
//-----------------------------------------------------------------------------
struct RIFFHEADER
{
    FOURCC  fccChunkId;
    DWORD   dwDataSize;
};

#define RIFFCHUNK_FLAGS_VALID   0x00000001


struct WAVESAMPLE
{
    ULONG   cbSize;
    USHORT  usUnityNote;
    SHORT   sFineTune;
    LONG    lGain;
    ULONG   ulOptions;
    ULONG   cSampleLoops;
};

struct WAVESAMPLE_LOOP
{
    ULONG cbSize;
    ULONG ulLoopType;
    ULONG ulLoopStart;
    ULONG ulLoopLength;
};


//-----------------------------------------------------------------------------
// Name: class CRiffChunk
// Desc: RIFF chunk utility class
//-----------------------------------------------------------------------------
class CRiffChunk
{
    FOURCC            m_fccChunkId;       // Chunk identifier
    const CRiffChunk* m_pParentChunk;     // Parent chunk
    HANDLE            m_hFile;
    DWORD             m_dwDataOffset;     // Chunk data offset
    DWORD             m_dwDataSize;       // Chunk data size
    DWORD             m_dwFlags;          // Chunk flags

public:
    CRiffChunk();

    // Initialization
    VOID    Initialize( FOURCC fccChunkId, const CRiffChunk* pParentChunk,
                        HANDLE hFile );
    HRESULT Open();
    BOOL    IsValid()     { return !!(m_dwFlags & RIFFCHUNK_FLAGS_VALID); }

    // Data
    HRESULT ReadData( LONG lOffset, VOID* pData, DWORD dwDataSize );

    // Chunk information
    FOURCC  GetChunkId()  { return m_fccChunkId; }
    DWORD   GetDataSize() { return m_dwDataSize; }
};




//-----------------------------------------------------------------------------
// Name: class CWaveFile
// Desc: Wave file utility class
//-----------------------------------------------------------------------------
class CWaveFile
{
    HANDLE      m_hFile;            // File handle
    CRiffChunk  m_RiffChunk;        // RIFF chunk
    CRiffChunk  m_FormatChunk;      // Format chunk
    CRiffChunk  m_DataChunk;        // Data chunk
    CRiffChunk  m_WaveSampleChunk;  // Wave Sample chunk

public:
    CWaveFile();
    ~CWaveFile();

    // Initialization
    HRESULT Open( const CHAR* strFileName );
    VOID    Close();

    // Create a new wav file
    HRESULT Create( const CHAR * szFileName, 
                    WAVEFORMATEX * pwfx, 
                    DWORD dwFormatSize, 
                    BYTE * pbData, 
                    DWORD dwDuration,
                    WAVESAMPLE * pws,
                    WAVESAMPLE_LOOP * pwsl );

    // File format
    HRESULT GetFormat( WAVEFORMATEX* pwfxFormat, DWORD dwFormatSize );

    // File data
    HRESULT ReadSample( DWORD dwPosition, VOID* pBuffer, DWORD dwBufferSize, 
                        DWORD* pdwRead );

    // File properties
    VOID    GetDuration( DWORD* pdwDuration ) { *pdwDuration = m_DataChunk.GetDataSize(); }

    // Wave Sample chunk
    HRESULT GetWaveSample( WAVESAMPLE * pws, WAVESAMPLE_LOOP * pwsl );
};




#endif // XBSOUND_H