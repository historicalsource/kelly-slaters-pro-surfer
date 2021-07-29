#ifndef GASDEFINESH
#define GASDEFINESH

// we don't have RAM for these in proview mode.
#define PROVIEW_MAX_CD_STEREO_STREAMS 0

#ifdef KELLY_SLATER
#define MAX_CD_STEREO_STREAMS 2
#elif defined (SPIDERMAN)
#define MAX_CD_STEREO_STREAMS 1
#elif defined (NSL_SOUND_TOOL)
#define MAX_CD_STEREO_STREAMS 2 // to make sure it reserves the max space
#else
#define MAX_CD_STEREO_STREAMS 1
#endif
#define MAX_CD_MONO_STREAMS 6
#define MAX_CD_STREAMS (MAX_CD_MONO_STREAMS + MAX_CD_STEREO_STREAMS)

#define PS2_CD_SECTOR_SIZE 2048

// note: keep iop_buffer_half an even multiple of spu_buffer_size
// eck! this could break stuff if any of these defines are used inappropriately -mjd
#define CD_STEREO_BUFFER_SECTORS ( 160 )
#define CD_STEREO_BUFFER_SECTORS_N_CHUNK (CD_STEREO_BUFFER_SECTORS / IOP_N_BUFFER)
#define CD_STEREO_BUFFER_SIZE ( PS2_CD_SECTOR_SIZE * CD_STEREO_BUFFER_SECTORS )
#define CD_STEREO_BUFFER_N_CHUNK ( PS2_CD_SECTOR_SIZE * CD_STEREO_BUFFER_SECTORS_N_CHUNK )

#define CD_MONO_BUFFER_SECTORS ( 40 )
#define CD_MONO_BUFFER_SECTORS_N_CHUNK (CD_MONO_BUFFER_SECTORS / IOP_N_BUFFER)
#define CD_MONO_BUFFER_SIZE ( PS2_CD_SECTOR_SIZE * CD_MONO_BUFFER_SECTORS )
#define CD_MONO_BUFFER_N_CHUNK ( PS2_CD_SECTOR_SIZE * CD_MONO_BUFFER_SECTORS_N_CHUNK )


#endif
