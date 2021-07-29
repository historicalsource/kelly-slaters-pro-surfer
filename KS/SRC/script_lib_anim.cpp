// script_lib_anim.cpp
//
// This file contains application-specific code and data that makes use of the
// Script Runtime Core to define script library classes and functions that
// provide the interface between the script language and the application.

#include "global.h"

#include "script_lib_anim.h"
#include "entity_anim.h"
#include "vm_stack.h"
#include "vm_thread.h"
#include "wds.h"
#include "file_finder.h"


///////////////////////////////////////////////////////////////////////////////
// script library class: anim
///////////////////////////////////////////////////////////////////////////////

// script library function:  void anim::pause();
class slf_anim_pause_t : public script_library_class::function
{
public:
  // constructor required
  slf_anim_pause_t( script_library_class* slc, const char* n )
  : script_library_class::function( slc, n )
  {
  }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_anim_t me;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    parms->me->set_flag( ANIM_SUSPENDED );
    SLF_DONE;
  }
};
//slf_anim_pause_t slf_anim_pause(slc_anim,"pause()");


// script library function:  void anim::set_timescale();
class slf_anim_set_timescale_t : public script_library_class::function
{
public:
  // constructor required
  slf_anim_set_timescale_t( script_library_class* slc, const char* n )
  : script_library_class::function( slc, n )
  {
  }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_anim_t me;
    vm_num_t speed;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    parms->me->set_timescale_factor(parms->speed >= 0.0f ? parms->speed : 0.0f);
    SLF_DONE;
  }
};
//slf_anim_set_timescale_t slf_anim_set_timescale(slc_anim,"set_timescale(num)");


// script library function:  void anim::set_time();
class slf_anim_set_time_t : public script_library_class::function
{
public:
  // constructor required
  slf_anim_set_time_t( script_library_class* slc, const char* n )
  : script_library_class::function( slc, n )
  {
  }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_anim_t me;
    vm_num_t time;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;

    if(parms->time < 0.0f || parms->time >= parms->me->get_duration())
      parms->me->set_time(parms->me->get_duration());
    else
      parms->me->set_time(parms->time);

    // this forces the animation to that frame
    parms->me->frame_advance(0.0f);

    SLF_DONE;
  }
};
//slf_anim_set_time_t slf_anim_set_time(slc_anim,"set_time(num)");


// script library function:  void anim::play();
class slf_anim_play_t : public script_library_class::function
{
public:
  // constructor required
  slf_anim_play_t( script_library_class* slc, const char* n )
  : script_library_class::function( slc, n )
  {
  }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_anim_t me;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    parms->me->clear_flag( ANIM_SUSPENDED );
    parms->me->attach();
    SLF_DONE;
  }
};
//slf_anim_play_t slf_anim_play(slc_anim,"play()");


// script library function:  void anim::wait_finished();
class slf_anim_wait_finished_t : public script_library_class::function
{
public:
  // constructor required
  slf_anim_wait_finished_t( script_library_class* slc, const char* n )
  : script_library_class::function( slc, n )
  {
  }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_anim_t me;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    if ( entry==FIRST_ENTRY && parms->me->is_flagged(ANIM_LOOPING) )
    {
      // revert animation to non-looping playback mode
      parms->me->clear_flag( ANIM_LOOPING );
      SLF_RECALL;
    }
    // wait for animation to finish playing
    else if ( !parms->me->is_finished() )
      SLF_RECALL;
    else
      SLF_DONE;
  }
};
//slf_anim_wait_finished_t slf_anim_wait_finished(slc_anim,"wait_finished()");


// script library function:  void anim::kill_anim();
class slf_anim_kill_anim_t : public script_library_class::function
{
public:
  // constructor required
  slf_anim_kill_anim_t( script_library_class* slc, const char* n )
  : script_library_class::function( slc, n )
  {
  }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_anim_t me;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    g_world_ptr->kill_anim( parms->me );
    SLF_DONE;
  }
};
//slf_anim_kill_anim_t slf_anim_kill_anim(slc_anim,"kill_anim()");


///////////////////////////////////////////////////////////////////////////////
// global script library functions for animation
///////////////////////////////////////////////////////////////////////////////

// global script library function: void load_anim(str)
class slf_load_anim_t : public script_library_class::function
{
public:
  // constructor required
  slf_load_anim_t(const char* n)
  : script_library_class::function(n)
  {
  }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_str_t filename;
  };
  // library function execution
  virtual bool operator()( vm_stack &stack, entry_t entry )
  {
    SLF_PARMS;
    stringx fname;
    if ( file_finder_exists( *parms->filename, entity_track_tree::extension( ), &fname ) )
      g_world_ptr->get_ett_manager()->load( fname );
    else
    {
      stack.get_thread()->slf_error( "load_anim(): couldn't find " + *parms->filename );
    }
    SLF_DONE;
  }
};
//slf_load_anim_t slf_load_anim("load_anim(str)");






void register_anim_lib()
{
  slc_anim_t* slc_anim = NEW slc_anim_t("anim",4);

  NEW slf_anim_pause_t(slc_anim,"pause()");
  NEW slf_anim_set_timescale_t(slc_anim,"set_timescale(num)");
  NEW slf_anim_set_time_t(slc_anim,"set_time(num)");
  NEW slf_anim_play_t(slc_anim,"play()");
  NEW slf_anim_wait_finished_t(slc_anim,"wait_finished()");
  NEW slf_anim_kill_anim_t(slc_anim,"kill_anim()");
  NEW slf_load_anim_t("load_anim(str)");
}

