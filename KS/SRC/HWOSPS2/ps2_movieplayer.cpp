#include <libgraph.h>
#include <libdma.h>
//#include <assert.h>
#include <sifrpc.h>
#include <sifdev.h>
#include <libgraph.h>
#include <libpkt.h>

#include "ps2_movieplayer.h"

#define MODULEDIR       "host:/usr/local/sce/iop/modules/"
#define bound(val,x)    ((((val) + (x) - 1) / (x))*(x))
#define MAX_WIDTH       720
#define MAX_HEIGHT      576

DEFINE_SINGLETON( movieplayer )


static void* myOutputFunc( struct nvlMovie* m, size_t* s, void* data )
{
  nglTexture  *rvTex;

  assert( m && data );
  rvTex = (nglTexture*)data;
  *s = rvTex->Width * rvTex->Height << 2;
  return rvTex->Data;
}

movieplayer::movieplayer()
{
  movieStarted  = false;
  isPlaying     = false;
  movieSource   = NULL;
  movie         = NULL;
  texture       = NULL;	
  movieBuf      = NULL;
}

movieplayer::~movieplayer()
{
}

void movieplayer::init()
{
  nvlSetMemoryAllocCallback((void *(*)(int, int))KSMemAllocNVL);
  nvlSetMemoryFreeCallback(KSMemFree);
  nvlStreamSetMemoryAllocCallback((void *(*)(int, int))KSMemAllocNVL);
  nvlStreamSetMemoryFreeCallback(KSMemFree);

  nvlStreamSystemInit(0);
  nvlInit();

  hiRes = false;
}

bool _nvlLoadModule(char *mod) 
{
  int counter=0;
  char cd[256];

  // Try the CD
  strcpy(cd, "cdrom0:\\");
  strcat(cd, mod);
  strcat(cd, ";1");
  while ((sceSifLoadModule(cd, 0, NULL) < 0) && (counter < 5))
    counter++;
  

  
  if (counter < 5) return true;
  else  // Try the host
  {
    counter = 0;
    strcpy(cd, "host0:");
    strcat(cd, mod);

    while ((sceSifLoadModule(cd, 0, NULL) < 0) && (counter < 5))
      counter++;
    
    if (counter < 5) return true;
  }

  // Shit, bail
  return false;
}
static bool firstTime = true;
float ps2MovieVolume = 1.0f;
void movieplayer::initHiRes(int width, int height)
{
  bool res;

  hiRes = true;
	
  nglExit();
	if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
	{
		
		nslShutdown();
	}

  nvlMPEGSetMemoryAllocCallback((void *(*)(int, int))KSMemAllocNVL);
  nvlMPEGSetMemoryFreeCallback(KSMemFree);

  ChangeThreadPriority( GetThreadId(), NVLMPEG_PRIORITY );
  sceSifInitRpc( 0 );
  sceSifInitIopHeap();

	// We load these modules elsewhere too, so 
	// don't assert if it fails 

	if (firstTime)
	{
		// Don't load this if the sound already has
		if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
		{
			res = _nvlLoadModule("LIBSD.IRX" );
			if (!res)
				debug_print("Could not load libsd.irx from movieplayer.  Maybe its already loaded");
			res = _nvlLoadModule("SDRDRV.IRX" );
			if (!res)
				debug_print("Could not load sdrdrv.irx from movieplayer.  Maybe its already loaded");
			//assert( res );

		}

/*
		//assert( res );
		res = _nvlLoadModule("SDRDRV.IRX" );
		if (!res)
			debug_print("Could not load sdrdrv.irx from movieplayer.  Maybe its already loaded");
		//assert( res );*/
		firstTime =false;
	}
  
	/* should be loaded by the input manager
	res = _nvlLoadModule("sio2man.irx" );
	if (!res)
		debug_print("Could not load sio2man.irx from movieplayer.  Maybe its already loaded");
	*/
  //assert( res );

  sceGsResetPath();
  sceDmaReset( 1 );
#ifdef TV_PAL
  //sceGsResetGraph( 0, SCE_GS_INTERLACE, SCE_GS_PAL, SCE_GS_FRAME );
#else
  //sceGsResetGraph( 0, SCE_GS_INTERLACE, SCE_GS_NTSC, SCE_GS_FRAME );
#endif
  clearGsMem(0x00, 0x00, 0x00, width, height);
  sceGsSetDefDBuff( &db, SCE_GS_PSMCT32, width, (height/2), SCE_GS_ZNOUSE, 0, SCE_GS_CLEAR );
  FlushCache(0);
}

void movieplayer::shutdown()
{
  if(hiRes)
  {
    //KSNGL_ReInit();
    //nglResetDisplay();
		//if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
		//		nslInit();
  }
  else
  {
    nvlShutdown();
    nvlStreamSystemShutdown();
  }
}

void movieplayer::play( char *filename, bool play_video, bool play_audio, bool loop, int buffersize )
{
  if(hiRes)
  {
		
    if(nvlMPEGLoad(filename, &db))
    {
			//nvlMPEGSetVolume(ps2MovieVolume);
      movieStarted = true;
      isPlaying    = true;
    }
    else
    {
      movieStarted = false;
      isPlaying    = false;
    }

  }
  else
  {
    if(movieBuf)
      free(movieBuf);
    movieBuf = (char *)memalign(64, buffersize);
    assert(movieBuf);

	  movieSource = nvlLoadMovieSource(filename, buffersize, movieBuf);
    if(movieSource)
	    movie = nvlAddMovie( movieSource );
	  if (!movie)
	  {
		  nglPrintf("WARNING: Could not play movie %s!", filename);
      if(movieSource)
		    nvlReleaseMovieSource(movieSource);
		  movieSource = NULL;
      if(movieBuf)
        free(movieBuf);
      movieBuf = NULL;
		  return;
	  }
	  //nglDev->SetRenderState(D3DRS_YUVENABLE, TRUE);
    texture = nglCreateTexture( NGLTF_32BIT, (unsigned int)nvlMovieGetWidth(movie), (unsigned int)nvlMovieGetHeight(movie) );
	  nvlPlayMovie( movie, myOutputFunc, texture, loop );
	  //nvlMovieSetVolume(movie, nslGetVolume(NSL_SOURCETYPE_MOVIE));
	  movieStarted = true;
    isPlaying    = true;
  }
}


void movieplayer::pause( bool paused )
{
}

void movieplayer::stop()
{
  if(hiRes)
  {
    if(movieStarted)
    {
      nvlMPEGStop();
      movieStarted = false;
      isPlaying    = false;
    }
  }
  else
  {
	  if(movie)
    {
	    nvlStopMovie(movie);
	    movie = NULL;
      nglDestroyTexture(texture);
      texture = NULL;
	    nvlReleaseMovieSource(movieSource);
	    movieSource = NULL;
      free(movieBuf);
      movieBuf = NULL;
	    movieStarted = false;
    }
  }

  movieStarted = false;
  isPlaying    = false;
}

void movieplayer::restart_with( const char *filename, bool play_video = true, bool play_audio = true )
{
}

bool movieplayer::is_playing()
{
  if(hiRes)
    return isPlaying;
  else
  {
	  if(movieStarted)
		  return (nvlMovieStatus(movie) == NVL_RESULT_PLAYING && nvlMovieGetFrame(movie) < nvlMovieGetFrameNum(movie)-1);
	  else
		  return false;
  }
}

void movieplayer::start_frame(bool listInit)
{
  if(!hiRes)
  {
	  if (movieStarted && listInit)
		  nglListInit();
  }
	else
		nvlMPEGSetVolume(ps2MovieVolume);

}

void movieplayer::end_frame(bool flip, bool listSend)
{
  if(hiRes && isPlaying)
    isPlaying = nvlMPEGAdvance();
  else if (isPlaying)
  {
	  nglInitQuad( &frame );
	  nglSetQuadBlend( &frame, NGLBM_OPAQUE, 0 );
	  nglSetQuadMapFlags(&frame, NGLMAP_CLAMP_U | NGLMAP_CLAMP_V | NGLMAP_BILINEAR_FILTER);
	  nglSetQuadTex( &frame, texture );
	  nglSetQuadRect( &frame, 0.0f, 0.0f, nglGetScreenWidth(),  nglGetScreenHeight());
	  //nglSetQuadColor( &frame, 0xFFFFFFFF );
	  //nglSetQuadUV( &frame, 0.0f, 0.0f, (float)nvlMovieGetWidth(movie), (float)nvlMovieGetHeight(movie) );
    nglSetQuadZ(&frame, 800.0f);
	  nglListAddQuad( &frame );
    if(listSend)
	    nglListSend( flip );
  }
}

void movieplayer::set_volume( float vol )
{
  if(hiRes)
    nvlMPEGSetVolume(vol);
}

void movieplayer::set_brightness( float br )
{
}

void movieplayer::set_br_offset( float offset )
{
}

void movieplayer::clearGsMem( int r, int g, int b, int disp_width, int disp_height )
{
  const u_long    giftag_clear[2] = { SCE_GIF_SET_TAG(0, 1, 0, 0, 0, 1), 0x000000000000000eL };
  sceGifPacket    packet;
  u_long128       packetBase[6] __attribute__ ((aligned(16)));

  sceDmaChan* dmaGif = sceDmaGetChan(SCE_DMA_GIF);
  SCE_GIF_CLEAR_TAG( &db.giftag0 );
  db.giftag0.NLOOP = 8;
  db.giftag0.EOP = 1;
  db.giftag0.NREG = 1;
  db.giftag0.REGS0 = 0xe;

  sceGsSetDefDrawEnv( &db.draw0, SCE_GS_PSMCT32, disp_width, bound(disp_height/2, 32)*2 + bound(disp_height, 32)*2, 0, 0 );
  *(u_long *)&db.draw0.xyoffset1 = SCE_GS_SET_XYOFFSET_1( 0, 0 );
  FlushCache( 0 );
  sceGsSyncPath( 0, 0 );
  sceGsPutDrawEnv( &db.giftag0 );
  sceGifPkInit( &packet, /*(u_long128*)*/packetBase );
  sceGifPkReset( &packet );
  sceGifPkEnd(&packet, 0, 0, 0);
  sceGifPkOpenGifTag( &packet, *(u_long128*)&giftag_clear );
  sceGifPkAddGsAD( &packet, SCE_GS_PRIM, SCE_GS_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 0, 0) );
  sceGifPkAddGsAD( &packet, SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(r, g, b, 0, 0) );
  sceGifPkAddGsAD( &packet, SCE_GS_XYZ2, SCE_GS_SET_XYZ2(0 << 4,0 << 4, 0) );
  sceGifPkAddGsAD( &packet, SCE_GS_XYZ2, SCE_GS_SET_XYZ2( MAX_WIDTH << 4, MAX_HEIGHT*5 << 4, 0) );
  sceGifPkCloseGifTag( &packet );
  sceGifPkTerminate( &packet );
  FlushCache( 0 );
  sceGsSyncPath( 0, 0 );
  sceDmaSend( dmaGif, (u_long128*)(u_int)packet.pBase );
  sceGsSyncPath( 0, 0 );
}

