// script_lib_sound_stream.cpp
//
// This file contains application-specific code and data that makes use of the
// Script Runtime Core to define script library classes and functions that
// provide the interface between the script language and the application.

#include "global.h"

#include "script_lib_sound_stream.h"
#include "hwaudio.h"
#include "vm_stack.h"
#include "vm_thread.h"
#include "ostimer.h"
#include "game.h"
#include "localize.h"

#ifdef GCCULL

//#define MAX_QUEUED_STREAMS 1
//int queued_streams = 0;

extern game *g_game_ptr;

///////////////////////////////////////////////////////////////////////////////
// script library class: sound_stream
///////////////////////////////////////////////////////////////////////////////


list<vm_sound_stream_t> g_cine_streams;
list<vm_sound_stream_t> g_stream_list;


static bool thread_find_sound_stream(vm_sound_stream_t str)
{
  list<vm_sound_stream_t>::iterator ss;
  for (ss = g_stream_list.begin(); ss!=g_stream_list.end(); ++ss)
  {
    if (*ss == str)
      return 1;
  }

  for (ss = g_cine_streams.begin(); ss!=g_cine_streams.end(); ++ss)
  {
    if (*ss == str)
      return 1;
  }

  return 0;
}


// script library function:  sound_stream::set_cine()
class slf_sound_stream_set_cine_t : public script_library_class::function
{
public:
  // constructor required
  slf_sound_stream_set_cine_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_sound_stream_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    // Only for Dreamcast,because streams start "instantly" froma hard drive (e.g. PC).
#if defined(TARGET_MKS)
    SLF_PARMS;
    parms->me->set_cine(1);
#else
    SLF_PARMS_UNUSED;
#endif
    SLF_DONE;
  }
};
//slf_sound_stream_set_cine_t slf_sound_stream_set_cine(slc_sound_stream,"set_cine()");


// script library function:  sound_stream::queue()
class slf_sound_stream_queue_t : public script_library_class::function
{
public:
  // constructor required
  slf_sound_stream_queue_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_sound_stream_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    // Only for Dreamcast,because streams start "instantly" froma hard drive (e.g. PC).
#if defined(TARGET_MKS)
    SLF_PARMS;
    //      parms->me->queue();
#else
    SLF_PARMS_UNUSED
#endif
    SLF_DONE;
  }
};
//slf_sound_stream_queue_t slf_sound_stream_queue(slc_sound_stream,"queue()");


// script library function:  sound_stream::play()
class slf_sound_stream_play_t : public script_library_class::function
{
public:
  // constructor required
  slf_sound_stream_play_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function persistent local vm_stack data;
  // stored on stack (after parms) until function completed;
  // accessed by SLF_SDATA at start of function
  struct sdata_t
  {
    time_value_t start_time;
    bool queued;
  };
  // library function parameters;
  // accessed by SLF_PARMS
  struct parms_t
  {
    vm_sound_stream_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
/*
    SLF_SDATA;
*/
    SLF_PARMS;

    parms->me->request_queue();
    SLF_DONE;

#if 0
    if (entry == FIRST_ENTRY)
    {
      // first entry to this library call with this stack;
      parms->me->request_queue();
      SLF_RECALL;
    }
    else
    {
      if( parms->me->is_ready_to_play() )
      {
        /*
          // Note I am leaving this in in release modes since this shouldn't be used in a time critical manner anyway
          list<vm_sound_stream_t>::iterator ss;
          for (ss = g_stream_list.begin(); ss!=g_stream_list.end(); ss++)
          {
          if (*ss == parms->me) break;
          }
          if (ss==g_stream_list.end())
          {
          stack.get_thread()->slf_error( "tried to play invalid stream" );
          }
        */
        parms->me->start(false);
        SLF_DONE;
      }
      else
      {
        SLF_RECALL;
      }
    }
#endif

    /*
      if (entry == FIRST_ENTRY)
      {
      // first entry to this library call with this stack;
      sdata->queued = false;
      SLF_RECALL;
      }
      else
      {
      if( !sdata->queued && queued_streams < MAX_QUEUED_STREAMS )
      {
      // set up timer
      sdata->start_time = game_clock::inst()->get_time();
      parms->me->queue();
      queued_streams++;
      sdata->queued = true;
      }
      if( sdata->queued && game_clock::inst()->get_time() >= sdata->start_time + 0.5f * XTICKS_PER_SEC )
      {
      // Note I am leaving this in in release modes since this shouldn't be used in a time critical manner anyway
      list<vm_sound_stream_t>::iterator ss;
      for (ss = g_stream_list.begin(); ss!=g_stream_list.end(); ss++)
      {
      if (*ss == parms->me) break;
      }
      if (ss==g_stream_list.end())
      {
      stack.get_thread()->slf_error( "tried to play invalid stream" );
      }
      parms->me->start(false);
      queued_streams--;
      SLF_DONE;
      }
      else
      {
      SLF_RECALL;
      }
      }
    */
  }
};
//slf_sound_stream_play_t slf_sound_stream_play(slc_sound_stream,"play()");


// script library function:  sound_stream::play_looping()
class slf_sound_stream_play_looping_t : public script_library_class::function
{
public:
  // constructor required
  slf_sound_stream_play_looping_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function persistent local vm_stack data;
  // stored on stack (after parms) until function completed;
  // accessed by SLF_SDATA at start of function
  struct sdata_t
  {
    time_value_t start_time;
    bool queued;
  };
  // library function parameters;
  // accessed by SLF_PARMS
  struct parms_t
  {
    vm_sound_stream_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    /*
    SLF_SDATA;
    */
    SLF_PARMS;

    parms->me->request_queue(false,true);

    SLF_DONE;
    /*
    if (entry == FIRST_ENTRY)
      {
      // first entry to this library call with this stack;
      parms->me->request_queue();
      SLF_RECALL;
      }
    else
      {
      if( parms->me->is_ready_to_play() )
        {
        // Note I am leaving this in in release modes since this shouldn't be used in a time critical manner anyway
        list<vm_sound_stream_t>::iterator ss;
        for (ss = g_stream_list.begin(); ss!=g_stream_list.end(); ss++)
          {
          if (*ss == parms->me) break;
          }
        if (ss==g_stream_list.end())
          {
#ifdef ENABLE_STREAM_ERRORS
          stack.get_thread()->slf_error( "tried to play invalid stream" );
#endif
          SLF_DONE
          }
        parms->me->start(true);
        SLF_DONE;
        }
      else
        {
        SLF_RECALL;
        }
      }
      */
  }
};
//slf_sound_stream_play_looping_t slf_sound_stream_play_looping(slc_sound_stream,"play_looping()");


// script library function:  sound_stream::stop()
class slf_sound_stream_stop_t : public script_library_class::function
{
public:
  // constructor required
  slf_sound_stream_stop_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_sound_stream_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    // Note I am leaving this in in release modes since this shouldn't be used in a time critical manner anyway
    list<vm_sound_stream_t>::iterator ss;
    for (ss = g_stream_list.begin(); ss!=g_stream_list.end(); ss++)
    {
      if (*ss == parms->me) break;
    }
    if (ss==g_stream_list.end())
    {
#ifdef ENABLE_STREAM_ERRORS
      stack.get_thread()->slf_error( "tried to stop invalid stream" );
#endif
      SLF_DONE;
    }
    parms->me->stop();
    SLF_DONE;
  }
};
//slf_sound_stream_stop_t slf_sound_stream_stop(slc_sound_stream,"stop()");


// script library function:  num sound_stream::is_playing()
class slf_sound_stream_is_playing_t : public script_library_class::function
{
public:
  // constructor required
  slf_sound_stream_is_playing_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_sound_stream_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_num_t result = 0;

    if(thread_find_sound_stream(parms->me))
      result = parms->me->is_playing_or_waiting_to_play();

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_sound_stream_is_playing_t slf_sound_stream_is_playing(slc_sound_stream,"is_playing()");


// script library function:  sound_stream::set_ranges(num,num)
class slf_sound_stream_set_ranges_t : public script_library_class::function
{
public:
  // constructor required
  slf_sound_stream_set_ranges_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_sound_stream_t me;
    vm_num_t min;
    vm_num_t max;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    parms->me->set_ranges(parms->min,parms->max);
    SLF_DONE;
  }
};
//slf_sound_stream_set_ranges_t slf_sound_stream_set_ranges(slc_sound_stream,"set_ranges(num,num)");


// script library function:  sound_stream::set_volume(num)
class slf_sound_stream_set_volume_t : public script_library_class::function
{
public:
  // constructor required
  slf_sound_stream_set_volume_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_sound_stream_t me;
    vm_num_t vol;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
#ifdef TARGET_PC
    SLF_PARMS;

    if(parms->me)
    {
      if(!thread_find_sound_stream(parms->me))
      {
        stack.get_thread()->slf_error( "used set_volume on a released stream" );
        SLF_DONE;
      }

      parms->me->set_volume(parms->vol);
    }
#else
    SLF_PARMS_UNUSED;
#endif
    SLF_DONE;
  }
};
//slf_sound_stream_set_volume_t slf_sound_stream_set_volume(slc_sound_stream,"set_volume(num)");


// script library function:  num sound_stream::get_volume()
class slf_sound_stream_get_volume_t : public script_library_class::function
{
public:
  // constructor required
  slf_sound_stream_get_volume_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_sound_stream_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    vm_num_t result = 0.0f;
    if(parms->me)
    {
      if(!thread_find_sound_stream(parms->me))
      {
        stack.get_thread()->slf_error( "used get_volume on a released stream" );
        result = 0.0f;
        SLF_RETURN;
        SLF_DONE;
      }

      result = parms->me->get_volume();
    }

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_sound_stream_get_volume_t slf_sound_stream_get_volume(slc_sound_stream,"get_volume()");


// script library function:  sound_stream::release()
class slf_sound_stream_release_t : public script_library_class::function
{
public:
  // constructor required
  slf_sound_stream_release_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_sound_stream_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    list<vm_sound_stream_t>::iterator ss;
#if defined (TARGET_PC)
    for (ss = g_cine_streams.begin(); ss!=g_cine_streams.end(); ss++)
    {
      if (*ss == parms->me) break;
    }
    if (ss!=g_cine_streams.end())
    {
      stack.get_thread()->slf_error( "used release on a stream created with cine_create_stream" );
    }
#endif

    // Note I am leaving this in in release modes since this shouldn't be used in a time critical manner anyway
    for (ss = g_stream_list.begin(); ss!=g_stream_list.end(); ss++)
    {
      if (*ss == parms->me) break;
    }
    if (ss==g_stream_list.end())
    {
#ifdef ENABLE_STREAM_ERRORS
      stack.get_thread()->slf_error( "tried to release invalid stream" );
#endif
          SLF_DONE;
    }

    if ( thread_find_sound_stream(parms->me) )
    {
      g_stream_list.remove( parms->me );
      g_cine_streams.remove( parms->me );
      parms->me->release();
    }

    SLF_DONE;
  }
};
//slf_sound_stream_release_t slf_sound_stream_release(slc_sound_stream,"release()");


// global script library function:  sound_stream create_stream(str)
class slf_create_stream_t : public script_library_class::function
{
public:
  // constructor required
  slf_create_stream_t(const char* n) : script_library_class::function(n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_str_t n;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    // localize VO stream (only applies to streams whose filename includes "_VO\\")
    stringx stream_filename = localize_VO_stream( *parms->n );
    vm_sound_stream_t result = sound_device::inst()->create_stream( stream_filename );
    g_stream_list.push_back(result);
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_create_stream_t slf_create_stream("create_stream(str)");


// script library function:  wait_play_stream(str name)
class slf_wait_play_stream_t : public script_library_class::function
{
public:
  // constructor required
  slf_wait_play_stream_t(const char* n) : script_library_class::function(n) {}
  // library function persistent local vm_stack data;
  // stored on stack (after parms) until function completed;
  // accessed by SLF_SDATA at start of function
  struct sdata_t
  {
    sound_stream* strm;
  };
  // library function parameters;
  // accessed by SLF_PARMS
  struct parms_t
  {
    vm_str_t name;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_SDATA;
    SLF_PARMS;
    // FIRST TIME IN:
    if ( entry == FIRST_ENTRY )
    {
      // create the sound stream;
      // localize VO stream (only applies to streams whose filename includes "_VO\\")
      stringx stream_filename = localize_VO_stream( *parms->name );
      sdata->strm = sound_device::inst()->create_stream( stream_filename );
      g_stream_list.push_back( sdata->strm );
      // play the sound stream
      sdata->strm->request_queue();
    }
    // SUBSEQUENT PASSES:
    else if ( thread_find_sound_stream(sdata->strm) )
    {
      if ( sdata->strm->is_playing() )
      {
        // stream is still playing; wait for it to finish
        SLF_RECALL;
      }
      else
      {
        // stream has finished playing; kill it
        g_stream_list.remove( sdata->strm );
        g_cine_streams.remove( sdata->strm );
        sdata->strm->release();
      }
    }
    SLF_DONE;
  }
};
//slf_wait_play_stream_t slf_wait_play_stream("wait_play_stream(str)");

/************************************ out of date **********************************

// global script library function:  sound_stream play_music(str)
class slf_play_music_t : public script_library_class::function
{
public:
  // constructor required
  slf_play_music_t(const char* n) : script_library_class::function(n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_str_t n;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_sound_stream_t music = sound_device::inst()->create_stream(*parms->n);
    g_stream_list.push_back(music);
    music->request_queue(false,true);
    g_game_ptr->set_music_stream( music );
    g_game_ptr->update_music_applied_volume( 0.0f, true );
    SLF_DONE;
  }
};
//slf_play_music_t slf_play_music("play_music(str)");
*/

// script library function:  sound_stream::music_fade_down(num,num)
class slf_music_fade_down_t : public script_library_class::function
{
public:
  // constructor required
  slf_music_fade_down_t(const char* n) : script_library_class::function(n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_num_t time;
    vm_num_t vol;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    if(!thread_find_sound_stream(g_game_ptr->get_music_stream()))
    {
      stack.get_thread()->slf_error( "used music_fade_down on a released stream" );
      SLF_DONE;
    }

    g_game_ptr->music_fade_down( parms->time, parms->vol );
    SLF_DONE;
  }
};
//slf_music_fade_down_t slf_music_fade_down("music_fade_down(num,num)");


// script library function:  sound_stream::music_fade_up(num,num)
class slf_music_fade_up_t : public script_library_class::function
{
public:
  // constructor required
  slf_music_fade_up_t(const char* n) : script_library_class::function(n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_num_t time;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    if(!thread_find_sound_stream(g_game_ptr->get_music_stream()))
    {
      stack.get_thread()->slf_error( "used music_fade_up on a released stream" );
      SLF_DONE;
    }

    g_game_ptr->music_fade_up( parms->time );
    SLF_DONE;
  }
};
//slf_music_fade_up_t slf_music_fade_up("music_fade_up(num)");


// Special create and release stream dunctions that retain a global list of which ones are in use,
// so they can be killed en mass.  This is designed for user termination of cut-scenes.

// script library function:  wait_play_cine_stream(str name)
class slf_wait_play_cine_stream_t : public script_library_class::function
{
public:
  // constructor required
  slf_wait_play_cine_stream_t(const char* n) : script_library_class::function(n) {}
  // library function persistent local vm_stack data;
  // stored on stack (after parms) until function completed;
  // accessed by SLF_SDATA at start of function
  struct sdata_t
  {
    sound_stream* strm;
  };
  // library function parameters;
  // accessed by SLF_PARMS
  struct parms_t
  {
    vm_str_t name;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_SDATA;
    SLF_PARMS;
    // FIRST TIME IN:
    if ( entry == FIRST_ENTRY )
    {
      // create the sound stream;
      // localize VO stream (only applies to streams whose filename includes "_VO\\")
      stringx stream_filename = localize_VO_stream( *parms->name );
      sdata->strm = sound_device::inst()->create_stream( stream_filename );
      g_stream_list.push_back( sdata->strm );
      g_cine_streams.push_back( sdata->strm );
      // play the sound stream
      sdata->strm->request_queue();
    }
    // SUBSEQUENT PASSES:
    else if ( thread_find_sound_stream(sdata->strm) )
    {
      if ( sdata->strm->is_playing() )
      {
        // stream is still playing; wait for it to finish
        SLF_RECALL;
      }
      else
      {
        // stream has finished playing; kill it
        g_stream_list.remove( sdata->strm );
        g_cine_streams.remove( sdata->strm );
        sdata->strm->release();
      }
    }
    SLF_DONE;
  }
};
//slf_wait_play_cine_stream_t slf_wait_play_cine_stream("wait_play_cine_stream(str)");


// global script library function:  sound_stream cine_create_stream(str)
class slf_cine_create_stream_t : public script_library_class::function
{
public:
  // constructor required
  slf_cine_create_stream_t(const char* n) : script_library_class::function(n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_str_t n;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    // localize VO stream (only applies to streams whose filename includes "_VO\\")
    stringx stream_filename = localize_VO_stream( *parms->n );
    vm_sound_stream_t result = sound_device::inst()->create_stream( stream_filename );
    g_stream_list.push_back(result);
    g_cine_streams.push_back(result);
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_cine_create_stream_t slf_cine_create_stream("cine_create_stream(str)");

// script library function:  sound_stream::cine_release()
class slf_sound_stream_cine_release_t : public script_library_class::function
{
public:
  // constructor required
  slf_sound_stream_cine_release_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_sound_stream_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
#if defined (TARGET_PC)
    list<vm_sound_stream_t>::iterator ss;
    for (ss = g_cine_streams.begin(); ss!=g_cine_streams.end(); ss++)
    {
      if (*ss == parms->me) break;
    }
    if (ss==g_cine_streams.end())
    {
      error( "used cine_release on a stream not created with cine_create_stream" );
    }
#endif
    g_stream_list.remove( parms->me );
    g_cine_streams.remove( parms->me );
    parms->me->release();
    SLF_DONE;
  }
};
//slf_sound_stream_cine_release_t slf_sound_stream_cine_release(slc_sound_stream,"cine_release()");


// global script library function:  num cine_release_all()
class slf_cine_release_all_t : public script_library_class::function
{
public:
  // constructor required
  slf_cine_release_all_t(const char* n) : script_library_class::function(n) {}
  // library function parameters
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    list<vm_sound_stream_t>::iterator ss;
    for (ss = g_cine_streams.begin(); ss!=g_cine_streams.end(); ss++)
    {
      if ((*ss)->is_playing())
        (*ss)->stop();
      g_stream_list.remove( *ss );
      (*ss)->release();
    }
    g_cine_streams.resize(0);
    SLF_DONE;
  }
};
//slf_cine_release_all_t slf_cine_release_all("cine_release_all()");


// script library function:  num sound_stream::cine_is_playing()
class slf_sound_stream_cine_is_playing_t : public script_library_class::function
{
public:
  // constructor required
  slf_sound_stream_cine_is_playing_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_sound_stream_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    list<vm_sound_stream_t>::iterator ss;
    for (ss = g_cine_streams.begin(); ss!=g_cine_streams.end(); ss++)
    {
      if (*ss == parms->me) break;
    }
    vm_num_t result;
    if (ss==g_cine_streams.end())
    {
      result = false;
    }
    else
    {
      result = parms->me->is_playing();
    }
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_sound_stream_cine_is_playing_t slf_sound_stream_cine_is_playing(slc_sound_stream,"cine_is_playing()");


// global script library function:  sound_stream_release(sound_stream)
class slf_sound_stream_release_c_t : public script_library_class::function
{
public:
  // constructor required
  slf_sound_stream_release_c_t(const char* n) : script_library_class::function(n) {}

  // library function parameters
  struct parms_t
  {
    // parameters
    vm_sound_stream_t snd;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    if(parms->snd->is_playing())
    {
      SLF_RECALL;
    }
    else
    {
      // Note I am leaving this in in release modes since this shouldn't be used in a time critical manner anyway
      list<vm_sound_stream_t>::iterator ss;
      for (ss = g_stream_list.begin(); ss!=g_stream_list.end(); ss++)
      {
        if (*ss == parms->snd) break;
      }
      if (ss==g_stream_list.end())
      {
#ifdef ENABLE_STREAM_ERRORS
        error( "tried to stop invalid stream" );
#endif
          SLF_DONE;
      }

#if defined (TARGET_PC)
      for (ss = g_cine_streams.begin(); ss!=g_cine_streams.end(); ss++)
      {
        if (*ss == parms->snd) break;
      }
      if (ss!=g_cine_streams.end())
      {
        stack.get_thread()->slf_error( "used release on a stream created with cine_create_stream" );
      }
#endif

      parms->snd->stop();


      // Note I am leaving this in in release modes since this shouldn't be used in a time critical manner anyway
      if (ss==g_stream_list.end())
      {
#ifdef ENABLE_STREAM_ERRORS
        stack.get_thread()->slf_error( "tried to release invalid stream" );
#endif
        SLF_DONE;
      }

      g_stream_list.remove( parms->snd );
      g_cine_streams.remove( parms->snd );
      parms->snd->release();
      SLF_DONE;
    }
  }
};
//slf_sound_stream_release_c_t slf_sound_stream_release_c("sound_stream_release_c(sound_stream)");


// global script library function:  sound_stream_release(sound_stream)
class slf_cine_sound_stream_release_c_t : public script_library_class::function
{
public:
  // constructor required
  slf_cine_sound_stream_release_c_t(const char* n) : script_library_class::function(n) {}

  // library function parameters
  struct parms_t
  {
    // parameters
    vm_sound_stream_t snd;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    list<vm_sound_stream_t>::iterator ss;
    for (ss = g_cine_streams.begin(); ss!=g_cine_streams.end(); ss++)
    {
      if (*ss == parms->snd) break;
    }

    bool playing;
    if (ss==g_cine_streams.end())
    {
      playing = false;
    }
    else
    {
      playing = parms->snd->is_playing();
    }

    if(playing)
    {
      SLF_RECALL;
    }
    else
    {
      // Note I am leaving this in in release modes since this shouldn't be used in a time critical manner anyway
      list<vm_sound_stream_t>::iterator ss;
      for (ss = g_stream_list.begin(); ss!=g_stream_list.end(); ss++)
      {
        if (*ss == parms->snd) break;
      }
      if (ss==g_stream_list.end())
      {
#ifdef ENABLE_STREAM_ERRORS
        stack.get_thread()->slf_error( "tried to stop invalid stream" );
#endif
          SLF_DONE;
      }

      parms->snd->stop();

#if defined (TARGET_PC)
      for (ss = g_cine_streams.begin(); ss!=g_cine_streams.end(); ss++)
      {
        if (*ss == parms->snd) break;
      }
      if (ss==g_cine_streams.end())
      {
        stack.get_thread()->slf_error( "used cine_release on a stream not created with cine_create_stream" );
      }
#endif
      g_stream_list.remove( parms->snd );
      g_cine_streams.remove( parms->snd );
      parms->snd->release();
      SLF_DONE;
    }
  }
};
//slf_cine_sound_stream_release_c_t slf_cine_sound_stream_release_c("cine_sound_stream_release_c(sound_stream)");


// global script library function:  sound_stream_auto_release(sound_stream)
class slf_sound_stream_auto_release_c_t : public script_library_class::function
{
public:
  // constructor required
  slf_sound_stream_auto_release_c_t(const char* n) : script_library_class::function(n) {}
  // library function persistent local vm_stack data;
  // stored on stack (after parms) until function completed;
  // accessed by SLF_SDATA at start of function
  struct sdata_t
  {
    time_value_t start_time;
    bool queued;
    bool sound_started;
  };
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_sound_stream_t snd;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_SDATA;
    SLF_PARMS;

    parms->snd->request_queue(true);
    sdata->sound_started = false;
    SLF_DONE;
  /*
    if (entry == FIRST_ENTRY)
      {
      parms->snd->request_queue();
      sdata->sound_started = false;

      SLF_RECALL;
      }
    else if(!sdata->sound_started)
      {
      if( parms->snd->is_ready_to_play() )
        {
        // Note I am leaving this in in release modes since this shouldn't be used in a time critical manner anyway
        list<vm_sound_stream_t>::iterator ss;

        for (ss = g_stream_list.begin(); ss!=g_stream_list.end(); ss++)
          {
          if (*ss == parms->snd) break;
          }

        if (ss==g_stream_list.end())
          {
#ifdef ENABLE_STREAM_ERRORS
          stack.get_thread()->slf_error( "tried to play invalid stream" );
#endif
          SLF_DONE;
          }

        parms->snd->start(false);
        sdata->sound_started = true;
        }

      SLF_RECALL;
      }
    else if(parms->snd->is_playing())
      {
      SLF_RECALL;
      }
    else
      {
      // Note I am leaving this in in release modes since this shouldn't be used in a time critical manner anyway
      list<vm_sound_stream_t>::iterator ss;
      for (ss = g_stream_list.begin(); ss!=g_stream_list.end(); ss++)
        {
        if (*ss == parms->snd) break;
        }
      if (ss==g_stream_list.end())
        {
#ifdef ENABLE_STREAM_ERRORS
        stack.get_thread()->slf_error( "tried to stop invalid stream" );
#endif
          SLF_DONE;
        }

      parms->snd->stop();

#if defined (TARGET_PC)
      for (ss = g_cine_streams.begin(); ss!=g_cine_streams.end(); ss++)
        {
        if (*ss == parms->snd) break;
        }
      if (ss!=g_cine_streams.end())
        {
        stack.get_thread()->slf_error( "used release on a stream created with cine_create_stream" );
        }
#endif

      // Note I am leaving this in in release modes since this shouldn't be used in a time critical manner anyway
      for (ss = g_stream_list.begin(); ss!=g_stream_list.end(); ss++)
        {
        if (*ss == parms->snd) break;
        }
      if (ss==g_stream_list.end())
        {
#ifdef ENABLE_STREAM_ERRORS
        stack.get_thread()->slf_error( "tried to release invalid stream" );
#endif
          SLF_DONE;
        }

      g_stream_list.remove( parms->snd );
      g_cine_streams.remove( parms->snd );
      parms->snd->release();
      SLF_DONE;
      }
      */
  }
};
//slf_sound_stream_auto_release_c_t slf_sound_stream_auto_release_c("sound_stream_auto_release_c(sound_stream)");


// global script library function:  cine_sound_stream_auto_release(sound_stream)
class slf_cine_sound_stream_auto_release_c_t : public script_library_class::function
{
public:
  // constructor required
  slf_cine_sound_stream_auto_release_c_t(const char* n) : script_library_class::function(n) {}
  // library function persistent local vm_stack data;
  // stored on stack (after parms) until function completed;
  // accessed by SLF_SDATA at start of function
  struct sdata_t
  {
    time_value_t start_time;
    bool queued;
    bool sound_started;
  };
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_sound_stream_t snd;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_SDATA;
    SLF_PARMS;
    parms->snd->request_queue(true);
    sdata->sound_started = false;
    SLF_DONE;
    /*
    if (entry == FIRST_ENTRY)
      {
      parms->snd->request_queue();
      sdata->sound_started = false;

      SLF_RECALL;
      }
    else if(!sdata->sound_started)
      {
      if( parms->snd->is_ready_to_play() )
        {
        // Note I am leaving this in in release modes since this shouldn't be used in a time critical manner anyway
        list<vm_sound_stream_t>::iterator ss;

        for (ss = g_stream_list.begin(); ss!=g_stream_list.end(); ss++)
          {
          if (*ss == parms->snd) break;
          }

        if (ss==g_stream_list.end())
          {
#ifdef ENABLE_STREAM_ERRORS
          stack.get_thread()->slf_error( "tried to play invalid stream );
#endif
          SLF_DONE;
          }

        parms->snd->start(false);
        sdata->sound_started = true;
        }

      SLF_RECALL;
      }
    else
      {
      list<vm_sound_stream_t>::iterator ss;
      for (ss = g_cine_streams.begin(); ss!=g_cine_streams.end(); ss++)
        {
        if (*ss == parms->snd) break;
        }

      bool playing;
      if (ss==g_cine_streams.end())
        {
        playing = false;
        }
      else
        {
        playing = parms->snd->is_playing();
        }

      if(playing)
        {
        SLF_RECALL;
        }
      else
        {
        // Note I am leaving this in in release modes since this shouldn't be used in a time critical manner anyway
        list<vm_sound_stream_t>::iterator ss;
        for (ss = g_stream_list.begin(); ss!=g_stream_list.end(); ss++)
          {
          if (*ss == parms->snd) break;
          }

        if (ss==g_stream_list.end())
          {
#ifdef ENABLE_STREAM_ERRORS
          stack.get_thread()->slf_error( "tried to stop invalid stream" );
#endif
          SLF_DONE;
          }

        parms->snd->stop();

#if defined (TARGET_PC)
        for (ss = g_cine_streams.begin(); ss!=g_cine_streams.end(); ss++)
          {
          if (*ss == parms->snd) break;
          }
        if (ss==g_cine_streams.end())
          {
          stack.get_thread()->slf_error( "used cine_release on a stream not created with cine_create_stream" );
          }
#endif

        // Note I am leaving this in in release modes since this shouldn't be used in a time critical manner anyway
        for (ss = g_stream_list.begin(); ss!=g_stream_list.end(); ss++)
          {
          if (*ss == parms->snd) break;
          }
        if (ss==g_stream_list.end())
          {
#ifdef ENABLE_STREAM_ERRORS
          stack.get_thread()->slf_error( "tried to release invalid stream" );
#endif
          SLF_DONE;
          }

        g_stream_list.remove( parms->snd );
        g_cine_streams.remove( parms->snd );
        parms->snd->release();
        SLF_DONE;
        }
      }
    */
  }
};
//slf_cine_sound_stream_auto_release_c_t slf_cine_sound_stream_auto_release_c("cine_sound_stream_auto_release_c(sound_stream)");









void register_sound_stream_lib()
{
  // pointer to single instance of library class
  slc_sound_stream_t* slc_sound_stream = NEW slc_sound_stream_t("sound_stream",4);

  NEW slf_sound_stream_set_cine_t(slc_sound_stream,"set_cine()");
  NEW slf_sound_stream_queue_t(slc_sound_stream,"queue()");
  NEW slf_sound_stream_play_t(slc_sound_stream,"play()");
  NEW slf_sound_stream_play_looping_t(slc_sound_stream,"play_looping()");
  NEW slf_sound_stream_stop_t(slc_sound_stream,"stop()");
  NEW slf_sound_stream_is_playing_t(slc_sound_stream,"is_playing()");
  NEW slf_sound_stream_set_ranges_t(slc_sound_stream,"set_ranges(num,num)");
  NEW slf_sound_stream_set_volume_t(slc_sound_stream,"set_volume(num)");
  NEW slf_sound_stream_get_volume_t(slc_sound_stream,"get_volume()");
  NEW slf_sound_stream_release_t(slc_sound_stream,"release()");
  NEW slf_create_stream_t("create_stream(str)");
  NEW slf_wait_play_stream_t("wait_play_stream(str)");
//  NEW slf_play_music_t("play_music(str)"); /* out of date */
  NEW slf_music_fade_down_t("music_fade_down(num,num)");
  NEW slf_music_fade_up_t("music_fade_up(num)");
  NEW slf_wait_play_cine_stream_t("wait_play_cine_stream(str)");
  NEW slf_cine_create_stream_t("cine_create_stream(str)");
  NEW slf_sound_stream_cine_release_t(slc_sound_stream,"cine_release()");
  NEW slf_cine_release_all_t("cine_release_all()");
  NEW slf_sound_stream_cine_is_playing_t(slc_sound_stream,"cine_is_playing()");
  NEW slf_sound_stream_release_c_t("sound_stream_release_c(sound_stream)");
  NEW slf_cine_sound_stream_release_c_t("cine_sound_stream_release_c(sound_stream)");
  NEW slf_sound_stream_auto_release_c_t("sound_stream_auto_release_c(sound_stream)");
  NEW slf_cine_sound_stream_auto_release_c_t("cine_sound_stream_auto_release_c(sound_stream)");
}

#endif