//-----------------------------------------------------------------------------
// File: wvaparse.cpp
//
// Desc: Helper class for reading and writing a .wav file 
//
// Hist: 6.12.01 - adapated from xbsound.cpp
//
// Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#include "wavparse.h"




//-----------------------------------------------------------------------------
// Name: CRiffChunk()
// Desc: Object constructor.
//-----------------------------------------------------------------------------
CRiffChunk::CRiffChunk()
{
    // Initialize defaults
    m_fccChunkId   = 0;
    m_pParentChunk = NULL;
    m_hFile        = INVALID_HANDLE_VALUE;
    m_dwDataOffset = 0;
    m_dwDataSize   = 0;
    m_dwFlags      = 0;
}




//-----------------------------------------------------------------------------
// Name: Initialize()
// Desc: Initializes the object
//-----------------------------------------------------------------------------
VOID CRiffChunk::Initialize( FOURCC fccChunkId, const CRiffChunk* pParentChunk, 
                             HANDLE hFile )
{
    m_fccChunkId   = fccChunkId;
    m_pParentChunk = pParentChunk;
    m_hFile        = hFile;
}




//-----------------------------------------------------------------------------
// Name: Open()
// Desc: Opens an existing chunk.
//-----------------------------------------------------------------------------
HRESULT CRiffChunk::Open()
{
    RIFFHEADER rhRiffHeader;
    LONG       lOffset = 0;

    // Seek to the first byte of the parent chunk's data section
    if( m_pParentChunk )
    {
        lOffset = m_pParentChunk->m_dwDataOffset;

        // Special case the RIFF chunk
        if( FOURCC_RIFF == m_pParentChunk->m_fccChunkId )
            lOffset += sizeof(FOURCC);
    }
    
    // Read each child chunk header until we find the one we're looking for
    for( ;; )
    {
        if( 0xFFFFFFFF == SetFilePointer( m_hFile, lOffset, NULL, FILE_BEGIN ) )
        {
            // Although 0xfffffff is the return code in case of an error, it
            // also could be a valid file offset. Let's find out.
            if( NO_ERROR != GetLastError() )
                return HRESULT_FROM_WIN32( GetLastError() );
        }

        DWORD dwRead;
        if( 0 == ReadFile( m_hFile, &rhRiffHeader, sizeof(rhRiffHeader), &dwRead, NULL ) )
            return HRESULT_FROM_WIN32( GetLastError() );

        // Check if we found the one we're looking for
        if( m_fccChunkId == rhRiffHeader.fccChunkId )
        {
            // Save the chunk size and data offset
            m_dwDataOffset = lOffset + sizeof(rhRiffHeader);
            m_dwDataSize   = rhRiffHeader.dwDataSize;

            // Success
            m_dwFlags |= RIFFCHUNK_FLAGS_VALID;

            return S_OK;
        }

        lOffset += sizeof(rhRiffHeader) + rhRiffHeader.dwDataSize;
    }
}





//-----------------------------------------------------------------------------
// Name: Read()
// Desc: Reads from the file
//-----------------------------------------------------------------------------
HRESULT CRiffChunk::ReadData( LONG lOffset, VOID* pData, DWORD dwDataSize )
{
    // Seek to the offset
    DWORD dwPosition = SetFilePointer( m_hFile, m_dwDataOffset+lOffset, NULL, FILE_BEGIN );

    if( 0xFFFFFFFF == dwPosition )
    {
        // Although 0xfffffff is the return code in case of an error, it
        // also could be a valid file offset. Let's find out.
        if( NO_ERROR != GetLastError() )
            return HRESULT_FROM_WIN32( GetLastError() );
    }

    // Read from the file
    DWORD dwRead;
    if( 0 == ReadFile( m_hFile, pData, dwDataSize, &dwRead, NULL ) )
        return HRESULT_FROM_WIN32( GetLastError() );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CWaveFile()
// Desc: Object constructor.
//-----------------------------------------------------------------------------
CWaveFile::CWaveFile()
{
    m_hFile = INVALID_HANDLE_VALUE;
}




//-----------------------------------------------------------------------------
// Name: ~CWaveFile()
// Desc: Object destructor.
//-----------------------------------------------------------------------------
CWaveFile::~CWaveFile()
{
    Close();
}




//-----------------------------------------------------------------------------
// Name: Open()
// Desc: Initializes the object.
//-----------------------------------------------------------------------------
HRESULT CWaveFile::Open( const CHAR* strFileName )
{
    // If we're already open, close
    Close();
    
    // Open the file
    m_hFile = CreateFile( strFileName, GENERIC_READ, FILE_SHARE_READ, NULL, 
                          OPEN_EXISTING, 0L, NULL );
    if( INVALID_HANDLE_VALUE == m_hFile )
        return HRESULT_FROM_WIN32( GetLastError() );

    // Initialize the chunk objects
    m_RiffChunk.Initialize( FOURCC_RIFF, NULL, m_hFile );
    m_FormatChunk.Initialize( FOURCC_FORMAT, &m_RiffChunk, m_hFile );
    m_DataChunk.Initialize( FOURCC_DATA, &m_RiffChunk, m_hFile );
    m_WaveSampleChunk.Initialize( FOURCC_WAVESAMPLE, &m_RiffChunk, m_hFile );

    HRESULT hr = m_RiffChunk.Open();
    if( FAILED(hr) )
        return hr;

    hr = m_FormatChunk.Open();
    if( FAILED(hr) )
        return hr;

    hr = m_DataChunk.Open();
    if( FAILED(hr) )
        return hr;

    // Validate the file type
    FOURCC fccType;
    hr = m_RiffChunk.ReadData( 0, &fccType, sizeof(fccType) );
    if( FAILED(hr) )
        return hr;

    if( FOURCC_WAVE != fccType )
        return HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Create
// Desc: Creates a wav file with the given name, wave format, sample data,
//       and WaveSample chunk (if specified).  WaveSample loop point struct is
//       only written out if pws is passed in and has cSampleLoops > 0
//-----------------------------------------------------------------------------
HRESULT 
CWaveFile::Create( const CHAR * szFileName, 
                   WAVEFORMATEX * pwfx, 
                   DWORD dwFormatSize, 
                   BYTE * pbData, 
                   DWORD dwDuration,
                   WAVESAMPLE * pws,
                   WAVESAMPLE_LOOP * pwsl )
{
    DWORD dwWritten;
    RIFFHEADER rh;
    DWORD cbRiffDataSize;
    
    // Calculate size of RIFF chunk
    cbRiffDataSize = sizeof( FOURCC ) +             // FormType
                     sizeof( RIFFHEADER ) +         // FORMAT chunk Header
                     dwFormatSize +                 // FORMAT chunk Data
                     sizeof( RIFFHEADER ) +         // DATA chunk Header
                     dwDuration;                    // DATA chunk Data
    if( pws )
    {
        cbRiffDataSize += sizeof( RIFFHEADER ) +    // WAVESAMPLE chunk Header
                          sizeof( WAVESAMPLE );     // WAVESAMPLE chunk Data
        if( pws->cSampleLoops && pwsl )
        {
            cbRiffDataSize += sizeof( WAVESAMPLE_LOOP );    // WAVESAMPLE Loop point
        }
    }

    // Create the file
    m_hFile = CreateFile( szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL );
    if( INVALID_HANDLE_VALUE == m_hFile )
        return HRESULT_FROM_WIN32( GetLastError() );

    // Write the RIFF chunk header
    rh.fccChunkId = FOURCC_RIFF;
    rh.dwDataSize = cbRiffDataSize;
    WriteFile( m_hFile, &rh, sizeof( RIFFHEADER ), &dwWritten, NULL );
    WriteFile( m_hFile, &FOURCC_WAVE, sizeof( FOURCC ), &dwWritten, NULL );

    // Write the FORMAT chunk
    rh.fccChunkId = FOURCC_FORMAT;
    rh.dwDataSize = dwFormatSize;
    WriteFile( m_hFile, &rh, sizeof( RIFFHEADER ), &dwWritten, NULL );
    WriteFile( m_hFile, pwfx, dwFormatSize, &dwWritten, NULL );

    // Write the DATA chunk
    rh.fccChunkId = FOURCC_DATA;
    rh.dwDataSize = dwDuration;
    WriteFile( m_hFile, &rh, sizeof( RIFFHEADER ), &dwWritten, NULL );
    WriteFile( m_hFile, pbData, dwDuration, &dwWritten, NULL );

    if( pws )
    {
        // Write out WAVESAMPLE chunk with loop point
        rh.fccChunkId = FOURCC_WAVESAMPLE;
        rh.dwDataSize = sizeof( WAVESAMPLE ) + sizeof( WAVESAMPLE_LOOP );
        WriteFile( m_hFile, &rh, sizeof( RIFFHEADER ), &dwWritten, NULL );
        WriteFile( m_hFile, pws, sizeof( WAVESAMPLE ), &dwWritten, NULL );
        if( pws->cSampleLoops && pwsl )
            WriteFile( m_hFile, pwsl, sizeof( WAVESAMPLE_LOOP ), &dwWritten, NULL );
    }
    
    // Close the file
    CloseHandle( m_hFile );
    m_hFile = INVALID_HANDLE_VALUE;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: GetFormat()
// Desc: Gets the wave file format
//-----------------------------------------------------------------------------
HRESULT CWaveFile::GetFormat( WAVEFORMATEX* pwfxFormat, DWORD dwFormatSize )
{
    DWORD dwValidSize = m_FormatChunk.GetDataSize();

    if( NULL == pwfxFormat || 0 == dwFormatSize )
        return E_INVALIDARG;

    // Read the format chunk into the buffer
    HRESULT hr = m_FormatChunk.ReadData( 0, pwfxFormat, min(dwFormatSize, dwValidSize) );
    if( FAILED(hr) )
        return hr;

    // Zero out remaining bytes, in case enough bytes were not read
    if( dwFormatSize > dwValidSize )
        ZeroMemory( (BYTE*)pwfxFormat + dwValidSize, dwFormatSize - dwValidSize );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ReadSample()
// Desc: Reads data from the audio file.
//-----------------------------------------------------------------------------
HRESULT CWaveFile::ReadSample( DWORD dwPosition, VOID* pBuffer, 
                               DWORD dwBufferSize, DWORD* pdwRead )
{                                   
    // Don't read past the end of the data chunk
    DWORD dwDuration;
    GetDuration( &dwDuration );

    if( dwPosition + dwBufferSize > dwDuration )
        dwBufferSize = dwDuration - dwPosition;

    HRESULT hr = S_OK;
    if( dwBufferSize )
        hr = m_DataChunk.ReadData( (LONG)dwPosition, pBuffer, dwBufferSize );

    if( pdwRead )
        *pdwRead = dwBufferSize;

    return hr;
}




//-----------------------------------------------------------------------------
// Name: GetWaveSample
// Desc: Attempts to read the WaveSample chunk
//-----------------------------------------------------------------------------
HRESULT CWaveFile::GetWaveSample( WAVESAMPLE * pws, WAVESAMPLE_LOOP * pwsl )
{
    HRESULT hr;

    // Look for sample points defined in a wave sample chunk
    hr = m_WaveSampleChunk.Open();
    if( FAILED( hr ) )
        return E_FAIL;

    // OK, we've got a wave sample chunk, read the data 
    hr = m_WaveSampleChunk.ReadData( 0, pws, sizeof( WAVESAMPLE ) );
    if( FAILED( hr ) )
        return hr;

    if( pws->cSampleLoops == 1 )
    {
        // We've got a loop defined, so read that
        hr = m_WaveSampleChunk.ReadData( pws->cbSize, pwsl, sizeof( WAVESAMPLE_LOOP ) );
        if( FAILED( hr ) ) 
            return hr;
    }

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: Close()
// Desc: Closes the object
//-----------------------------------------------------------------------------
VOID CWaveFile::Close()
{
    if( m_hFile != INVALID_HANDLE_VALUE )
    {
        CloseHandle( m_hFile );
        m_hFile = INVALID_HANDLE_VALUE;
    }
}




