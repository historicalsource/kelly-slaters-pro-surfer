////////////////////////////////////////////////////////////////////////////////
//
//  XBox streams header
//
////////////////////////////////////////////////////////////////////////////////


#ifndef NSL_XBOX_STREAMS_H
#define NSL_XBOX_STREAMS_H


// JPL - changed for testing.
//const MaxXBoxStreams = 12;
const MaxXBoxStreams = 6;


// This value is hard-coded assuming an ADPCM frame of 36 samples and 16
// bit stereo (900 frames per packet) - from MS example
// I don't think it will be ALWAYS stereo - bad assumption! -- Asen
const XBoxStreamPackets = 3;
const XBoxMinPacketSize = 36;
//const XBoxStreamBufSize = 256 * 2 * 2 * 36;
// NOTE THAT NOT BUFFERED I/O SHOULD HAVE SIZE AND OFFSET ALIGNED BY 4K!!!!!!!!!
// ALSO IT SHOULD BE ALIGNE BY THE ADPCM FRAME OF 36 SAMPLES
const XBoxStreamBufSize = 36 * 1024;


struct XBoxSingleStream
{
  // buffers are static for now
  int bufSize;
  void * pBuffer;

  // file information
  HANDLE hFile;
  int transferred;
  int offset;
  int startpos;
  int size;
  int totalsize;
  int left;
  int priority;

  // stream flags
  bool ready;
  bool streaming;
  bool used;
  bool finished;
  bool shouldDie;
  bool looped;
  bool endreached;
};

class CXBoxStreams
{
protected:
  OVERLAPPED overlapped;
  int FindUnusedStream();

public:
  int currentstream;
  XBoxSingleStream streams[MaxXBoxStreams];

  CXBoxStreams();
  ~CXBoxStreams();

  int CreateStream( HANDLE hFile, int offset, int size, bool looped );
  // void SetBuffer( int stream, void * pBuffer, int bufsize );
  void ReleaseStream( int stream );
  void StartStreaming( int stream );
  void StopStreaming( int stream );

  void FrameAdvance( float elapsedTime );
};


#endif
