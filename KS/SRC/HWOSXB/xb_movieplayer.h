#ifndef _HW_XBOX_MOVIEPLAYER_H_
#define _HW_XBOX_MOVIEPLAYER_H_

#ifndef SINGLETON_H
#include "singleton.h"
#endif

#include "ngl_xbox.h"
#include "nvl_xbox.h"

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


void movie_pre_init( const movie_data *pData = NULL );

class file_manager;

class movieplayer : public singleton
{
public:
	DECLARE_SINGLETON( movieplayer )
		
	movieplayer();
	~movieplayer();
	
	void init();
	void shutdown();
	
	void play( const char *filename, bool play_video = true, bool play_audio = true, bool loop = false );
	void pause( bool paused=true );
	void stop();
  void restart_with( const char *filename, bool play_video = true, bool play_audio = true );
		
	bool is_playing();
  void start_frame(bool list_init = true);
  void end_frame(bool flip = true, bool list_send = true);
		
	void posit( int left, int top, int widht, int height, int z );
	void set_volume( float vol );
	void set_brightness( float br );
	void set_br_offset( float offset );
		
	file_manager *	fileman;
private:
	bool movie_started;
	nvlMovieSource* movie_src;
	nvlMovie* movie;
	nglTexture* frame_texture[2];	
	nglQuad frame;
	void *load_file( const char *fname );
};


#endif	// _HW_XBOX_MOVIEPLAYER_H_