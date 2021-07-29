#include "global.h"

#ifndef _HW_PS2_MOVIEPLAYER_H_
#define _HW_PS2_MOVIEPLAYER_H_

#ifndef SINGLETON_H
#include "singleton.h"
#endif

#include "nvlMPEG_ps2.h"

enum MWE_PLY_STAT
{
  MWE_PLY_STAT_STOP = 0,    // stoped
  MWE_PLY_STAT_PREP = 1,    // preparing
  MWE_PLY_STAT_PLAYING = 2, // playing
  MWE_PLY_STAT_PLAYEND = 3, // end of playing
  MWE_PLY_STAT_ERROR = 4    // error was occured
};

struct movie_data 
{
	int video_mode;
	int fbuf_mode;
	int frm_count;
	int latency;
	int bps;
	int scr_width;
	int scr_height;
	int work_area;
};


class movieplayer : public singleton
{
  public:
    DECLARE_SINGLETON( movieplayer )

    movieplayer();
    ~movieplayer();

    void init();
    void initHiRes(int width, int height);
    void shutdown();

    void play( char *filename, bool play_video = true, bool play_audio = true, bool loop = false, int buffersize = 0x100000 );
    void pause( bool paused=true );
    void stop();
    void restart_with( const char *filename, bool play_video = true, bool play_audio = true );

    bool is_playing();
    void start_frame(bool listInit=true);
    void end_frame(bool flip=true, bool listSend=true);

	  void posit( int left, int top, int width, int height, int z );
    void set_volume( float vol );
    void set_brightness( float br );
    void set_br_offset( float offset );

    MWE_PLY_STAT get_stat() const { return MWE_PLY_STAT_ERROR; }

  private:
    void clearGsMem( int r, int g, int b, int disp_width, int disp_height );

    bool              hiRes;
    sceGsDBuff        db;

    char              *movieBuf;
	  nvlMovieSource    *movieSource;
	  nvlMovie          *movie;
	  nglTexture        *texture;
	  nglQuad           frame;
    bool              movieStarted;
    bool              isPlaying;
};


#endif	// _HW_PS2_MOVIEPLAYER_H_