#ifndef _HW_GC_MOVIEPLAYER_H_
#define _HW_GC_MOVIEPLAYER_H_

#include "singleton.h"

#include "nvl_gc.h"

class movieplayer : public singleton
{
	public:
    DECLARE_SINGLETON( movieplayer )

		movieplayer();
    ~movieplayer();

    void init();
    void shutdown();

    void play( const char *filename, bool play_video = true, bool play_audio = true );
		void stop();

		bool is_playing();
    void start_frame();
    void end_frame();

		void set_volume( float vol );

	private:
    nvlMovieSource* cur_mov_src;
    nvlMovie* cur_mov;
    bool needs_flip;
    
    static void* output_func( struct nvlMovie* m, size_t* s, void* data );
};

#endif	// _HW_GC_MOVIEPLAYER_H_