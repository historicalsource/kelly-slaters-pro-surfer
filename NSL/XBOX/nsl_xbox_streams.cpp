////////////////////////////////////////////////////////////////////////////////
//
//  XBox streams implementation
//
////////////////////////////////////////////////////////////////////////////////


#include "nsl_xbox.h"
#include "nsl_xbox_streams.h"

const BufferAlignment = 4096;


CXBoxStreams::CXBoxStreams()
{
  // none used initially
  currentstream = -1;

  for (int i = 0; i < MaxXBoxStreams; i++)
  {
    streams[i].used = 0;
    streams[i].bufSize = XBoxStreamBufSize;
    streams[i].streaming = false;
    streams[i].shouldDie = false;
    streams[i].ready = false;
  }

  overlapped.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
}

CXBoxStreams::~CXBoxStreams()
{
  CloseHandle( overlapped.hEvent );
}

int CXBoxStreams::FindUnusedStream()
{
  int stream;

  for (stream = 0; stream < MaxXBoxStreams; stream++)
  {
    if ( !streams[stream].used )
    {
      return stream;
    }
  }

  // Not found - return -1;
  return -1;
}

// CreateStream - requires that the hFile is already opened for asynchronous read
// if no stream available - returns -1
int CXBoxStreams::CreateStream( HANDLE hFile, int offset, int size, bool looped )
{
  int stream = FindUnusedStream();

  // mark as being used and set required parameters
  if (stream >= 0)
  {
    streams[stream].used = true;
    streams[stream].hFile = hFile;
    streams[stream].offset = offset;
    streams[stream].startpos = offset;
    streams[stream].size = size;
    streams[stream].left = size;
    streams[stream].streaming = true;
    streams[stream].ready = false;
    streams[stream].priority = 0;
    streams[stream].finished = false;
    streams[stream].endreached = false;
    streams[stream].shouldDie = false;
    streams[stream].looped = looped;
  }

  return stream;
}

void CXBoxStreams::ReleaseStream( int stream )
{
  if (currentstream == stream)
  {
    //nslPrintf("killing current %d", stream);
    streams[currentstream].shouldDie = true;
  }
  else
  {
    nslPrintf("killing %d", stream);
    streams[stream].used = false;
    streams[stream].streaming = false;
  }
}

void CXBoxStreams::FrameAdvance( float elapsedTime )
{
  // Check to see if we are processing a stream
  if (currentstream >= 0)
  {
    DWORD dwBytesTransferred;
    BOOL bIsReadDone = GetOverlappedResult( streams[currentstream].hFile, &overlapped, &dwBytesTransferred, FALSE );

    // If the read isn't complete, keep rendering
    if( !bIsReadDone )
    {
      int err = GetLastError();
      internalAssert( err == ERROR_IO_INCOMPLETE );
    }
    else
    {
      // If we get here, the read is complete.
      // Note that m_Overlapped.hEvent has also been reset to non-signalled by
      // GetOverlappedResult(), so we don't have to reset it.
      // nslPrintf( "Stream %d finished transferring %d bytes\n", currentstream, dwBytesTransferred );

      if ( streams[currentstream].used && streams[currentstream].shouldDie )
      {
        streams[currentstream].used = false;
        streams[currentstream].streaming = false;
        return;
      }

      streams[currentstream].ready = true;

      if ( streams[currentstream].endreached )
      {
        if ( streams[currentstream].looped )
        {
          streams[currentstream].transferred = streams[currentstream].left;
          streams[currentstream].left = streams[currentstream].size;
          streams[currentstream].offset = streams[currentstream].startpos;
          streams[currentstream].endreached = false;
        }
        else
        {
					streams[currentstream].transferred = streams[currentstream].left;
          streams[currentstream].finished = true;
        }
      }
      else
      {
        streams[currentstream].transferred = dwBytesTransferred;
        streams[currentstream].offset += dwBytesTransferred;
        streams[currentstream].left -= dwBytesTransferred;
      }

      currentstream = -1;
    }
  }
  else
  {
    int stream;
    int pick = -1;

    // Pick a new stream that is not ready
    for (stream = 0; stream < MaxXBoxStreams; ++stream)
    {
      if (streams[stream].streaming && !streams[stream].ready && !streams[stream].finished)
      {
        // Find the one that will need to be processed sooner
        if ((pick < 0) || (streams[pick].priority > streams[stream].priority))
        {
          pick = stream;
        }
      }
    }

    // Send a new streaming command if found a stream that is waiting
    if (pick >= 0)
    {
      overlapped.Offset = streams[pick].offset;
      overlapped.OffsetHigh = 0;

      // nslPrintf( "Stream %d, offset %d\n", pick, streams[pick].offset );
      int size = streams[pick].bufSize;

      if ( streams[pick].left < size )
      {
        size = ((streams[pick].left + BufferAlignment - 1) / BufferAlignment) * BufferAlignment;
        streams[pick].endreached = true;
      }
	  if (size > 0)
	  {
		  internalAssert( (streams[pick].offset % BufferAlignment) == 0 );
		  internalAssert( (size % BufferAlignment) == 0 );
		  BOOL bComplete = ReadFile( streams[pick].hFile,
									 streams[pick].pBuffer,
									 size,
									 NULL, &overlapped );

		  internalAssert( !bComplete );
	  }
      currentstream = pick;
      // nslPrintf( "picked %d", pick );
    }
  }
}
