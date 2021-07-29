#ifndef	__LIBWAVE_H_INCLUDED__
#define	__LIBWAVE_H_INCLUDED__

	//	libWave, Treyarch Corporation
	//	code: malkia@treyarch.com

	// Loaded .WAV (or future .AIFF) files will be placed in such structure
	typedef struct {
		int   chan;	// 1-mono 2-stereo
		int   freq; // 11025/22050/44100
		int   bits; // 8bits/16bits
		void *data; // raw PCM wave data
		int   size; // in bytes
	} WAVESource;

	// This structure will contain compiled WAVE file for given platform and given format
	// When given platform needs stereo to be splitted, one should call WAVE_StereoSplit
	// WAVE_StereoSplit will allocated and return left and right channels of the wave file
	// Later if you want to interleave those channels (for example 2048 bytes buffer) you
	// should call WAVE_StereoInterleave - it will return the interleaved stereo.
	//
	// Use WAVETarget to specify your desired output format
	// Set   <chan>, <freq>, <bits> and <dataFormat> to specify your target format prefferences
	// Leave <chan>, <freq>, <bits> fields to zero and they will default to values from WAVESource fields
	// Certain <dataFormats> may require internal change to <bits> (for example all ADPCM codecs need 16bit data)
	// If you want your stereo wave data to be interleaved (PS2/GAMECUBE)
	// Use <interleavedStereoBlockSize> to setup such (usual values are like 2048)
	typedef struct {
		/////// INPUT
		int		chan;							// 1-mono, 2-stereo
		int		freq;							// frequency in Khz
		int		bits;							// number of bits per sample (may not be adequate)
		int		dataFormat;						// WAVE_TARGET_ttt_fff
		int		dataAlignment;					// If you want your data to be sector size aligned - use this field
		int		dataInterleavedStereoChunkSize;	// Chunk size (half-buffer size) - does not matter for mono, but must be valid for stereo

		/////// DATA OUTPUT
		void*	data;			// pointer to the converted wave file data
		int		dataSamples;	// number of samples in the data
		int		dataSize;		// final size of the data

		/////// USERDATA OUTPUT
		void*	userData1;		// user data - for example IMAADPCM HEADER
		int		userData1Size;	// size of that user data
		void*	userData2;		// user data - for example IMAADPCM HEADER
		int		userData2Size;	// size of that user data
	} WAVETarget;

	// Various platform specific formats
	// Use WAVETarget->dataFormat to specify format
	enum {
		WAVE_TARGET_NONE,

		WAVE_TARGET_PS2_PCM,	// Does not convert leaves the file PCM8 or PCM16
		WAVE_TARGET_PS2_VAG,	// VAG: PS2 ADPCM

		WAVE_TARGET_XBOX_PCM,
		WAVE_TARGET_XBOX_IMAADPCM,

		WAVE_TARGET_GAMECUBE_PCM,
		WAVE_TARGET_GAMECUBE_ADPCM,
	};

	// Gamecube specific header
	// XBOX IMAADPCM header is not included, if one needs it
	// He should include WINDOWS.H file
	// PS2 does not need such header
	// Headers like this or IMAADPCM header are stored in userData[userDataSize] memory in WAVETarget
	typedef struct
	{
		// start context
		short coef[16];
		unsigned	gain;
		unsigned	pred_scale;
		short		yn1;
		short		yn2;
		// loop context
		unsigned	loop_pred_scale;
		short		loop_yn1;
		short		loop_yn2;
	} WAVE_GAMECUBE_ADPCMINFO;

	enum {
		WAVE_GAMECUBE_ADPCMINFO_SIZE = 64, // GAMECUBE version will store WAVE_GAMECUBE_ADPCMINFO in front of each stream with the size of 64 bytes
	};

	// NOTE: With few exceptions, all functions are returning 0 on fail, 
	//       and non-zero on success.

	// functions opearating on WAVESource
	int	WAVE_SourceInit(                 WAVESource *source ); // clears the fields of WAVESource, will not free already allocated data
	int	WAVE_SourceFree(                 WAVESource *source ); // frees allocated wave data in WAVESource, then clears WAVESource
	int	WAVE_SourceLoad(                 WAVESource *source, const char *file ); // `file' must be valid filename
	int	WAVE_SourceCopy(           const WAVESource *source, WAVESource *dest ); // `dest' will be freed before copying to
	int	WAVE_SourceIsValid(		   const WAVESource *source );
	int	WAVE_SourceConvert(        const WAVESource *source, WAVESource *dest ); // `dest' must contain desired wave data format
	int	WAVE_SourceGetSampleSize(  const WAVESource *source ); // returns: 8bitMono:1, 8bitStereo:2, 16bitMono:2, 16bitStereo:4
	int	WAVE_SourceGetSampleCount( const WAVESource *source ); // returns: number of samples in wave data

	// functions operating on WAVETarget
	int	WAVE_TargetDump(						  const WAVETarget* target, const char *targetName );
	int	WAVE_TargetInit(								WAVETarget *target );
	int WAVE_TargetFree(								WAVETarget *target );
	int WAVE_TargetSetDataFormat(						WAVETarget *target, int targetFmt );
	int	WAVE_TargetSetDataAlignment(					WAVETarget *target, int dataAlignment );
	int	WAVE_TargetSetDataInterleavedStereoChunkSize(	WAVETarget *target, int dataInterleavedStereoChunkSize );
  void* WAVE_TargetGetData(                             WAVETarget *target );
	int	WAVE_TargetGetDataSize(                         WAVETarget *target );
	int	WAVE_TargetGetDataSamples(                      WAVETarget *target );
	int	WAVE_TargetSetMono(								WAVETarget *target );
	int	WAVE_TargetSetFrequency(						WAVETarget *target, int freq );
	int	WAVE_TargetGetFrequency(						WAVETarget *target );
	
	// encoders converting WAVESource to WAVETarget
	typedef int (* WAVE_ENCODE_FUNC)( const WAVESource *source, WAVETarget *target );
	int	WAVE_EncodeGAMECUBE_ADPCM(    const WAVESource *source, WAVETarget *target );
	int	WAVE_EncodeXBOX_IMAADPCM(     const WAVESource *source, WAVETarget *target );
	int	WAVE_EncodePS2_VAG(           const WAVESource *source, WAVETarget *target );
	int	WAVE_Encode(			      const WAVESource *source, WAVETarget *target );

	// Split and stereo interleaving helper functions
	int	WAVE_StereoSplit( const WAVESource *source, void **left, void **right ); // returns allocated left and right channels containg splitted stereo tracks
	int	WAVE_StereoInterleave( int sizeInBytes, const void *L, const void *R, int blockSizeInBytes, void *D ); // interleaves stereo tracks L and R into D given interleave block size
	int	WAVE_StereoInterleaveGetSize( int nonInterleavedStereoSize, int interleavedStereoBlockSize ); // calculates the size of interleaved stereo wave data

	// simple memory management function replacements (realloc, calloc, malloc, free)
  void* WAVE_MemoryRealloc( void* mem, int newSize ); 
  void* WAVE_MemoryCalloc( int elCount, int elSize );
  void* WAVE_MemoryAlloc( int size );
	int WAVE_MemoryFree( void *ptr );

	// libWaveError.cpp wave reporting - errors, messages, warnings, internals
	enum {
		WAVE_REPORT_VERBOSE_LEVEL_SHOW_FATALS = -1,
		WAVE_REPORT_VERBOSE_LEVEL_SHOW_ERRORS,
		WAVE_REPORT_VERBOSE_LEVEL_SHOW_WARNINGS,
		WAVE_REPORT_VERBOSE_LEVEL_SHOW_MESSAGES,
		WAVE_REPORT_VERBOSE_LEVEL_SHOW_INTERNALS,
		WAVE_REPORT_VERBOSE_LEVEL_SHOW_HEADERS,
	};
	int		WAVE_ReportVerboseLevelPush( int level );
	int		WAVE_ReportVerboseLevelPop( void );
	int		WAVE_ReportFatal( const char *, ... );
	int		WAVE_ReportError( const char *, ... );
	int		WAVE_ReportWarning( const char *, ... );
	int		WAVE_ReportMessage( const char *, ... );
	int		WAVE_ReportInternal( const char *, ... ); 
	int		WAVE_ReportHeaderPop( void ); // try not to this one and
	int		WAVE_ReportHeaderPush( const char *, ... ); // this one
	struct	WAVE_CReportHeader // better use this class defined it in begining of each block
	{
			// Instead of manually pushing/popping
			// Use this class once at the begining of error block
			WAVE_CReportHeader( const char *, ...);
		   ~WAVE_CReportHeader();
	};
	struct	WAVE_CReportVerboseLevel 
	{
			WAVE_CReportVerboseLevel( int level );
		   ~WAVE_CReportVerboseLevel();
	};

	#define WAVE_ReportHeader		WAVE_CReportHeader		 __wave_reportHeader
	#define WAVE_ReportVerboseLevel	WAVE_CReportVerboseLevel __wave_reportVeboseLevel

#endif
