#ifndef NSL_XBOX_SOURCE_HEADER
#define NSL_XBOX_SOURCE_HEADER

#include "../common/nsl.h"

typedef struct nslSoundParam
{
  float	volume;
	float	pitch;
  float	minDist;
  float	maxDist;	
} _nslSoundParam;

static const DWORD nslXBoxSndVersion = 0x0100;

#define NSL_XBOX_SND_BANK_EXT ".XSD"
#define NSL_XBOX_SND_HEADER_EXT ".XSH"
#define NSL_XBOX_STREAM_SND_EXT ".XSS"
#define STREAM_FILENAME "STREAMS.XSS"

enum nslXBoxSndFlagsEnum
{
	XBOX_SND_FLAGS_INVALID		= 0,
	XBOX_SND_FLAGS_LOOPED		= 1,
	XBOX_SND_FLAGS_LOOPPOINTS	= 2,
	XBOX_SND_FLAGS_STREAM		= 4,
	XBOX_SND_FLAGS_REVERB		= 8,
};

struct nslXBoxSndHeader
{
  DWORD version;
  DWORD reserved;
	DWORD entryCnt;
};

struct nslXBoxSndEntry
{
#ifdef NSL_LOAD_SOURCE_BY_ALIAS
  WORD aliasID;
#endif

#ifdef NSL_LOAD_SOURCE_BY_NAME
	char	fileName[NSL_MAX_STR_LENGTH];
#endif

	int		channels;
  DWORD offset;
  DWORD size;
  DWORD loopStart;
  DWORD loopEnd;
  DWORD flags;

	int samples;

  nslSourceTypeEnum type;
	nslSoundParam params;

	WAVEFORMATEX format;
};

#endif