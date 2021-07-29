// script_lib.cpp
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.
// This file contains application-specific code and data that makes use of the
// Script Runtime Core to define script library classes and functions that
// provide the interface between the script language and the application.
#include "global.h"

#include "script_library_class.h"
#include "script_lib_entity.h"
//!#include "character.h"
#include "algebra.h"
#include "inputmgr.h"
#include "joystick.h"
#include "commands.h"
#include "chunkfile.h"
//#include "fxman.h"
#include "game.h"
#include "geomgr.h"
#include "fogmgr.h"
#include "light.h"
#include "oserrmsg.h"
#include "ostimer.h"
#include "hwaudio.h"
//!#include "rigid.h"
#include "vm_stack.h"
#include "wds.h"
#include "msgboard.h"
#include "hwmovieplayer.h"
#include "hwaudio.h"

//!#include "character.h"
//#include "projectile.h"
//!#include "limb.h"
#include "vm_thread.h"
#include "terrain.h"
//!#include "attrib.h"
#include "projconst.h"
#include "geomgr.h"
//#include "brain.h"
#include "trigger.h"
#include "localize.h"
//!#include "automap.h"
#include "element.h"
#include "debug_render.h"
#include "text_font.h"
//P #include "optionsfile.h"
#include "osdevopts.h"
#include "collide.h"
#include "time_interface.h"
#include "ai_interface.h"
#include "random.h"

//!typedef character* vm_character_t;  // vm_stack data representation

//#define ENABLE_STREAM_ERRORS

extern game *g_game_ptr;
//H extern bool g_blackscreen;
extern rational_t g_time_dilation;


///////////////////////////////////////////////////////////////////////////////
// Global script library function allows the current thread to be flagged as
// NOT_SUSPENDABLE
///////////////////////////////////////////////////////////////////////////////

// global script library function: allow_suspend_thread(num v)
class slf_allow_suspend_thread_t : public script_library_class::function
{
public:
  // constructor required
  slf_allow_suspend_thread_t(const char* n): script_library_class::function(n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_num_t t;
  };
  // library function execution
  virtual bool operator ()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    stack.get_thread()->set_suspendable( parms->t );
    SLF_DONE;
  }
};
//slf_allow_suspend_thread_t slf_allow_suspend_thread("allow_suspend_thread(num)");

// global script library function: set_camera_priority(num v)
class slf_set_camera_priority_t : public script_library_class::function
{
public:
  // constructor required
  slf_set_camera_priority_t(const char* n): script_library_class::function(n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_num_t t;
  };
  // library function execution
  virtual bool operator ()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    stack.get_thread()->set_camera_priority( parms->t );
    SLF_DONE;
  }
};
//slf_set_camera_priority_t slf_set_camera_priority("set_camera_priority(num)");






///////////////////////////////////////////////////////////////////////////////
// global marky cam functions
///////////////////////////////////////////////////////////////////////////////
// global script library function:  enable_marky_cam(num sync)
class slf_enable_marky_cam_t : public script_library_class::function
{
public:
  // constructor required
  slf_enable_marky_cam_t(const char* n) : script_library_class::function(n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_num_t sync;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    app::inst()->get_game()->enable_marky_cam(true, (bool)parms->sync, stack.get_thread()->get_camera_priority());
    SLF_DONE;
  }
};
//slf_enable_marky_cam_t slf_enable_marky_cam("enable_marky_cam(num)");

// global script library function:  disable_marky_cam(num sync)
class slf_disable_marky_cam_t : public script_library_class::function
{
public:
  // constructor required
  slf_disable_marky_cam_t(const char* n) : script_library_class::function(n) {}
  // library function parameters
  struct parms_t
  {
    vm_num_t sync;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    app::inst()->get_game()->enable_marky_cam(false, (bool)parms->sync, stack.get_thread()->get_camera_priority());
    SLF_DONE;
  }
};
//slf_disable_marky_cam_t slf_disable_marky_cam("disable_marky_cam(num)");


// global script library function:  freeze_hero(num sync)
class slf_freeze_hero_t : public script_library_class::function
{
public:
  // constructor required
  slf_freeze_hero_t(const char* n) : script_library_class::function(n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_num_t freeze;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    app::inst()->get_game()->freeze_hero((bool)parms->freeze);
    SLF_DONE;
  }
};
//slf_freeze_hero_t slf_freeze_hero("freeze_hero(num)");


// global script library function: vector3d set_zoom( num )
class slf_set_zoom_t : public script_library_class::function
{
public:
  // constructor required
  slf_set_zoom_t(const char* n): script_library_class::function(n) { }
  // library function execution

  // library function parameters;
  // accessed by SLF_PARMS
  struct parms_t
  {
    vm_num_t num;
  };
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;
    PROJ_ZOOM = parms->num;
    geometry_manager::inst()->rebuild_view_frame();
    SLF_DONE;
  }
};
//slf_set_zoom_t slf_set_zoom("set_zoom(num)");



///////////////////////////////////////////////////////////////////////////////

#include "profiler.h"


// global script library function: num localize_thread( vector3d s2 )
class slf_localize_thread_t : public script_library_class::function
{
public:
  // constructor required
  slf_localize_thread_t(const char* n): script_library_class::function(n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vector3d loc;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;
    terrain & ter = g_world_ptr->get_the_terrain();
    region_node * rn;
    sector * sec = ter.find_sector( parms->loc );
    if (sec)
    {
      rn = sec->get_region();
      rn->get_data()->add_local_thread(stack.get_thread());
    }
    SLF_DONE;
  }
};
//slf_localize_thread_t slf_localize_thread("localize_thread(vector3d)");

// global script library function: num localize_thread_to_character( vm_character_t s2 )
class slf_localize_thread_to_character_t : public script_library_class::function
{
public:
  // constructor required
  slf_localize_thread_to_character_t(const char* n): script_library_class::function(n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
//!    character *chr;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS_UNUSED;
//!    parms->chr->add_local_thread(stack.get_thread());
    SLF_DONE;
  }
};
//slf_localize_thread_to_character_t slf_localize_thread_to_character("localize_thread_to_character(character)");



// global script library function: num localize_thread_region( str loc )
class slf_localize_thread_region_t : public script_library_class::function
{
public:
  // constructor required
  slf_localize_thread_region_t(const char* n): script_library_class::function(n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_str_t reg;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;
    terrain & ter = g_world_ptr->get_the_terrain();
    region_node * rn;
    rn = ter.find_region(*(parms->reg));
    rn->get_data()->add_local_thread(stack.get_thread());
    SLF_DONE;
  }
};
//slf_localize_thread_region_t slf_localize_thread_region("localize_thread_region(str)");



// global script library function: num globalize_thread( )
class slf_globalize_thread_t : public script_library_class::function
{
public:
  // constructor required
  slf_globalize_thread_t(const char* n): script_library_class::function(n) { }
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    region * r = stack.get_thread()->get_local_region();
    if(r != NULL)
      r->remove_local_thread(stack.get_thread());
    else
    {
/*!      character *chr = stack.get_thread()->get_local_character();
      if ( chr != NULL )
        chr->remove_local_thread(stack.get_thread());
!*/
    }
    SLF_DONE;
  }
};
//slf_globalize_thread_t slf_globalize_thread("globalize_thread()");


// global script library function: suspend_all_ai( )
class slf_suspend_all_ai_t : public script_library_class::function
{
public:
  // constructor required
  slf_suspend_all_ai_t(const char* n): script_library_class::function(n) { }
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    ai_interface::push_disable_all();
    SLF_DONE;
  }
};
//slf_suspend_all_ai_t slf_suspend_all_ai("suspend_all_ai()");


// global script library function: unsuspend_all_ai( )
class slf_unsuspend_all_ai_t : public script_library_class::function
{
public:
  // constructor required
  slf_unsuspend_all_ai_t(const char* n): script_library_class::function(n) { }
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    ai_interface::pop_disable_all();
    SLF_DONE;
  }
};
//slf_unsuspend_all_ai_t slf_unsuspend_all_ai("unsuspend_all_ai()");








/*
extern rational_t g_max_character_cam_dist;
// global script library function: character_clip_dist(num dist)
class slf_character_clip_dist_t : public script_library_class::function
{
public:
  // constructor required
  slf_character_clip_dist_t(const char* n): script_library_class::function(n) {}

  // library function parameters
  struct parms_t
  {
    // parameters
    vm_num_t dist;
  };
  // library function execution
  virtual bool operator ()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    g_max_character_cam_dist = parms->dist;
    SLF_DONE;
  }
};
slf_character_clip_dist_t slf_character_clip_dist("character_clip_dist(num)");
*/

//H extern bool g_disable_screen_saver;
//Hextern bool g_disable_interface;

/*H
class slf_enable_screen_saver_t : public script_library_class::function
{
public:
  // constructor required
  slf_enable_screen_saver_t(const char* n): script_library_class::function(n) {}

  // library function parameters
  struct parms_t
  {
    // parameters
    vm_num_t on;
  };
  // library function execution
  virtual bool operator ()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;

    if(parms->on)
      g_disable_screen_saver = false;
    else
      g_disable_screen_saver = true;

    SLF_DONE;
  }
};
//slf_enable_screen_saver_t slf_enable_screen_saver("enable_screen_saver(num)");
H*/

class slf_enable_pause_t : public script_library_class::function
{
public:
  // constructor required
  slf_enable_pause_t(const char* n): script_library_class::function(n) {}

  // library function parameters
  struct parms_t
  {
    // parameters
    vm_num_t on;
  };
  // library function execution
  virtual bool operator ()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;

    if(parms->on)
      g_game_ptr->set_disable_start_menu(false);
    else
      g_game_ptr->set_disable_start_menu(true);

    SLF_DONE;
  }
};
//slf_enable_pause_t slf_enable_pause("enable_pause(num)");


class slf_enable_interface_t : public script_library_class::function
{
public:
  // constructor required
  slf_enable_interface_t(const char* n): script_library_class::function(n) {}

  // library function parameters
  struct parms_t
  {
    // parameters
    vm_num_t on;
  };
  // library function execution
  virtual bool operator ()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;

    if(parms->on)
      g_game_ptr->set_disable_interface(false);
    else
      g_game_ptr->set_disable_interface(true);

    SLF_DONE;
  }
};
//slf_enable_interface_t slf_enable_interface("enable_interface(num)");

/*
//unsigned char g_vmu_tone = 0xc0;
//given portnum=[0,1,2,3] and slotnum=[0,1]
#if defined(TARGET_MKS)
//void vmu_beep_on(int port, int slot, char tone = 0);
//void vmu_beep_off(int port, int slot);
//#include "vmu_beep.h"
#endif

class slf_vmu_beep_on_t : public script_library_class::function
{
public:
  // constructor required
  slf_vmu_beep_on_t(const char* n): script_library_class::function(n) {}

  // library function parameters
  struct parms_t
  {
    // parameters
    vm_num_t tone;
  };
  // library function execution
  virtual bool operator ()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
#if defined(TARGET_MKS)
    if (vmu_happy_beep();//vmu_beep_on(0, 0, parms->tone);
#endif

    SLF_DONE;
  }
};
slf_vmu_beep_on_t slf_vmu_beep_on("vmu_beep_on(num)");

class slf_vmu_beep_off_t : public script_library_class::function
{
public:
  // constructor required
  slf_vmu_beep_off_t(const char* n): script_library_class::function(n) {}

  // library function parameters
  // library function execution
  virtual bool operator ()( vm_stack& stack, entry_t entry )
  {
//    SLF_PARMS;
#if defined(TARGET_MKS)
    vmu_beep_off(0, 0);
#endif

    SLF_DONE;
  }
};
slf_vmu_beep_off_t slf_vmu_beep_off("vmu_beep_off()");
*/

////////////////////////////////////////////////////////////////////////////////
// global script library function: wait_animate_fog_color(vector3d,num)
class slf_wait_animate_fog_color_t : public script_library_class::function
{
  public:
    // constructor required
    slf_wait_animate_fog_color_t(const char* n) : script_library_class::function(n) {}
    // library function persistent local vm_stack data;
    // stored on stack (after parms) until function completed;
    // accessed by SLF_SDATA at start of function
    struct sdata_t
    {
      time_value_t clock;
      vector3d     start_color;
    };
    // library function parameters;
    // accessed by SLF_PARMS
    struct parms_t
    {
      vector3d dest_color;
      vm_num_t duration;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_SDATA;
      SLF_PARMS;
      if (entry == FIRST_ENTRY)
      {
        // first entry to this library call with this stack;
        // set up timer
        if(parms->duration==0)
        {
          fog_manager::inst()->set_fog_color( color( parms->dest_color.x,
                                                        parms->dest_color.y,
                                                        parms->dest_color.z,
                                                        1.0f ) );
          SLF_DONE;
        }
        else
        {
          sdata->clock = 0.0f;//.reset();
          color start_color4 = fog_manager::inst()->get_fog_color();
          sdata->start_color = vector3d( start_color4.get_red(),
                                         start_color4.get_green(),
                                         start_color4.get_blue() );
          SLF_RECALL;
        }
      }
      else
      {
        sdata->clock += SCRIPT_TIME_INC;
//        time_value_t time_elapsed = sdata->clock.elapsed();
        float percentage_elapsed = sdata->clock / parms->duration;
        percentage_elapsed = min( (float)percentage_elapsed, 1.0f );
        vector3d fog_triple = (parms->dest_color - sdata->start_color) * percentage_elapsed
                            + sdata->start_color;
        fog_manager::inst()->set_fog_color( color( fog_triple.x,
                                                        fog_triple.y,
                                                        fog_triple.z,
                                                        1.0f ) );
        if( sdata->clock >= parms->duration )
        {
          fog_manager::inst()->set_fog_color(
            color( parms->dest_color.x, parms->dest_color.y, parms->dest_color.z, 1.0f ) );
          SLF_DONE;
        }
        else
        {
          SLF_RECALL;
        }
      }
    }
};
//slf_wait_animate_fog_color_t slf_wait_animate_fog_color("wait_animate_fog_color(vector3d,num)");

// global script library function: wait_animate_fog_distance(vector3d,num)
class slf_wait_animate_fog_distance_t : public script_library_class::function
{
  public:
    // constructor required
    slf_wait_animate_fog_distance_t(const char* n) : script_library_class::function(n) {}
    // library function persistent local vm_stack data;
    // stored on stack (after parms) until function completed;
    // accessed by SLF_SDATA at start of function
    struct sdata_t
    {
      time_value_t clock;
      vm_num_t     start_distance;
    };
    // library function parameters;
    // accessed by SLF_PARMS
    struct parms_t
    {
      vm_num_t dest_distance;
      vm_num_t duration;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_SDATA;
      SLF_PARMS;
      if (entry == FIRST_ENTRY)
      {
        // first entry to this library call with this stack;
        // set up timer
        if(parms->duration==0)
        {
          fog_manager::inst()->set_fog_distance( 0, parms->dest_distance );
          SLF_DONE;
        }
        else
        {
          sdata->clock = 0.0f;
          sdata->start_distance = fog_manager::inst()->get_fog_end_distance();
          SLF_RECALL;
        }
      }
      else
      {
        sdata->clock += SCRIPT_TIME_INC;
//        time_value_t time_elapsed = sdata->clock.elapsed();
        float percentage_elapsed = sdata->clock / parms->duration;
        percentage_elapsed = min( percentage_elapsed, 1.0f );
        rational_t distance = (parms->dest_distance - sdata->start_distance) * percentage_elapsed
                            + sdata->start_distance;
        fog_manager::inst()->set_fog_distance( 0, distance );
        if( sdata->clock >= parms->duration )
        {
          SLF_DONE;
        }
        else
        {
          SLF_RECALL;
        }
      }
    }
};
//slf_wait_animate_fog_distance_t slf_wait_animate_fog_distance("wait_animate_fog_distance(num,num)");


// global script library function: wait_animate_fog_distances(vector3d,num)
class slf_wait_animate_fog_distances_t : public script_library_class::function
{
  public:
    // constructor required
    slf_wait_animate_fog_distances_t(const char* n) : script_library_class::function(n) {}
    // library function persistent local vm_stack data;
    // stored on stack (after parms) until function completed;
    // accessed by SLF_SDATA at start of function
    struct sdata_t
    {
      time_value_t clock;
      vm_num_t     start_near_distance;
      vm_num_t     start_far_distance;
    };
    // library function parameters;
    // accessed by SLF_PARMS
    struct parms_t
    {
      vm_num_t dest_near_distance;
      vm_num_t dest_far_distance;
      vm_num_t duration;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_SDATA;
      SLF_PARMS;
      if (entry == FIRST_ENTRY)
      {
        // first entry to this library call with this stack;
        // set up timer
        if(parms->duration==0)
        {
          fog_manager::inst()->set_fog_distance( parms->dest_near_distance,
                                                       parms->dest_far_distance );
          SLF_DONE;
        }
        else
        {
          sdata->clock = 0.0f;
          sdata->start_near_distance = fog_manager::inst()->get_fog_start_distance();
          sdata->start_far_distance  = fog_manager::inst()->get_fog_end_distance();
          SLF_RECALL;
        }
      }
      else
      {
//        time_value_t time_elapsed = sdata->clock.elapsed();
        sdata->clock += SCRIPT_TIME_INC;

        float percentage_elapsed = sdata->clock / parms->duration;
        percentage_elapsed = min( percentage_elapsed, 1.0f );
        rational_t far_distance = (parms->dest_far_distance - sdata->start_far_distance) * percentage_elapsed
                            + sdata->start_far_distance;
        rational_t near_distance = (parms->dest_near_distance - sdata->start_near_distance) * percentage_elapsed
                            + sdata->start_near_distance;
        fog_manager::inst()->set_fog_distance( near_distance, far_distance );
        if( sdata->clock >= parms->duration )
        {
          SLF_DONE;
        }
        else
        {
          SLF_RECALL;
        }
      }
    }
};
//slf_wait_animate_fog_distances_t slf_wait_animate_fog_distances("wait_animate_fog_distances(num,num,num)");


///////////////////////////////////////////////////////////////////////////////
// Miscellaneous global script library functions
///////////////////////////////////////////////////////////////////////////////

// global script library function: wait_frame()
class slf_wait_frame_t : public script_library_class::function
{
  public:
    // constructor required
    slf_wait_frame_t(const char* n) : script_library_class::function(n) {}
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      if (entry == FIRST_ENTRY)
      {
        // first entry to this library call with this stack;
        // set up timer
        SLF_RECALL;
      }
      else
      {
        SLF_DONE;
      }
    }
};
//slf_wait_frame_t slf_wait_frame("wait_frame()");


// global script library function: delay(num)
class slf_delay_t : public script_library_class::function
{
  public:
    // constructor required
    slf_delay_t(const char* n) : script_library_class::function(n) {}
    // library function persistent local vm_stack data;
    // stored on stack (after parms) until function completed;
    // accessed by SLF_SDATA at start of function
    struct sdata_t
    {
      time_value_t clock;
    };
    // library function parameters;
    // accessed by SLF_PARMS
    struct parms_t
    {
      vm_num_t duration;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_SDATA;
      SLF_PARMS;
      if (entry == FIRST_ENTRY)
      {
        // first entry to this library call with this stack;
        // set up timer
        sdata->clock = 0.0f;
        SLF_RECALL;
      }
      else
      {
        sdata->clock += SCRIPT_TIME_INC;
        if( sdata->clock >= parms->duration )
        {
          SLF_DONE;
        }
        else
        {
          SLF_RECALL;
        }
      }
    }
};
//slf_delay_t slf_delay("delay(num)");

// global script library function: abs_delay(num)
class slf_abs_delay_t : public script_library_class::function
{
  public:
    // constructor required
    slf_abs_delay_t(const char* n) : script_library_class::function(n) {}
    // library function persistent local vm_stack data;
    // stored on stack (after parms) until function completed;
    // accessed by SLF_SDATA at start of function
    struct sdata_t
    {
      game_clock_t clock;
    };
    // library function parameters;
    // accessed by SLF_PARMS
    struct parms_t
    {
      vm_num_t duration;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_SDATA;
      SLF_PARMS;
      if (entry == FIRST_ENTRY)
      {
        // first entry to this library call with this stack;
        // set up timer
        sdata->clock.reset();
        SLF_RECALL;
      }
      else
      {
        if( sdata->clock.elapsed() >= parms->duration )
        {
          SLF_DONE;
        }
        else
        {
          SLF_RECALL;
        }
      }
    }
};
//slf_abs_delay_t slf_abs_delay("abs_delay(num)");

// global script library function: dilated_delay(num)
class slf_dilated_delay_t : public script_library_class::function
{
  public:
    // constructor required
    slf_dilated_delay_t(const char* n) : script_library_class::function(n) {}
    // library function persistent local vm_stack data;
    // stored on stack (after parms) until function completed;
    // accessed by SLF_SDATA at start of function
    struct sdata_t
    {
      time_value_t time_remaining;
    };
    // library function parameters;
    // accessed by SLF_PARMS
    struct parms_t
    {
      vm_num_t duration;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_SDATA;
      SLF_PARMS;
      if (entry == FIRST_ENTRY)
      {
        // first entry to this library call with this stack;
        // set up timer
        sdata->time_remaining = parms->duration;
        SLF_RECALL;
      }
      else
      {
        sdata->time_remaining -= CALC_GLOBAL_TIME_DILATION(SCRIPT_TIME_INC);

        if( sdata->time_remaining <= 0.0f )
        {
          SLF_DONE;
        }
        else
        {
          SLF_RECALL;
        }
      }
    }
};
//slf_dilated_delay_t slf_dilated_delay("dilated_delay(num)");

// global script library function: vo_delay(num,num,num,num)
class slf_vo_delay_t : public script_library_class::function
{
  public:
    // constructor required
    slf_vo_delay_t(const char* n) : script_library_class::function(n) {}
    // library function persistent local vm_stack data;
    // stored on stack (after parms) until function completed;
    // accessed by SLF_SDATA at start of function
    struct sdata_t
    {
      time_value_t clock;
      vm_num_t duration;
    };
    // library function parameters;
    // accessed by SLF_PARMS
    struct parms_t
    {
      vm_num_t duration_eng;
      vm_num_t duration_fre;
      vm_num_t duration_ger;
      vm_num_t duration_spa;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_SDATA;
      SLF_PARMS;
      if (entry == FIRST_ENTRY)
      {
        // first entry to this library call with this stack;
        // set up timer
        sdata->clock = 0.0f;
/*P
        switch(app::inst()->get_game()->get_optionsfile()->get_option(GAME_OPT_LANGUAGE))
        {
          case 0:
P*/
            sdata->duration = parms->duration_eng;
/*P
            break;

          case 1:
            sdata->duration = parms->duration_fre;
            break;

          case 2:
            sdata->duration = parms->duration_ger;
            break;

          case 3:
            sdata->duration = parms->duration_spa;
            break;

          default:
            error("Invalid locale for vo_delay");
            break;
        }
P*/
        SLF_RECALL;
      }
      else
      {
        sdata->clock += SCRIPT_TIME_INC;
        if( sdata->clock >= sdata->duration )
        {
          SLF_DONE;
        }
        else
        {
          SLF_RECALL;
        }
      }
    }
};
//slf_vo_delay_t slf_vo_delay("vo_delay(num,num,num,num)");

// global script library function:  area_damage(vector3d pos,num radius,num damage)
class slf_area_damage_t : public script_library_class::function
{
  public:
    // constructor required
    slf_area_damage_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      vector3d p;
      rational_t radius;
      rational_t damage;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_PARMS_UNUSED;
STUBBED(slf_area_damage_t, "script_lib slf_area_damage_t");
/*!      for (int i=0;i<g_world_ptr->get_num_characters();++i)
      {
        character * chr = g_world_ptr->get_character(i);
        vector3d diff = chr->get_abs_position()-parms->p;
        if (diff.length2() < parms->radius*parms->radius)
        {
          chr->apply_damage(parms->damage, vector3d(0,0,0), vector3d(0,0,0), DAMAGE_DIRECT, NULL);
        }
      }
!*/
      SLF_DONE;
    }
};
//slf_area_damage_t slf_area_damage("area_damage(vector3d,num,num)");

// global script library function:  post_message(str msg,num damage)
class slf_post_message2_t : public script_library_class::function
{
  public:
    // constructor required
    slf_post_message2_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_str_t msg;
      rational_t time;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
#if defined (TARGET_PC)
      SLF_PARMS;
      app::inst()->get_game()->get_message_board()->post(*parms->msg,parms->time);
#else
      SLF_PARMS_UNUSED;
#endif
      SLF_DONE;
    }
};
//slf_post_message2_t slf_post_message("post_message(str,num)");


// global script library function:  post_message(num value,num damage)
class slf_post_message_t : public script_library_class::function
{
  public:
    // constructor required
    slf_post_message_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      rational_t value;
      rational_t time;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
#if defined (TARGET_PC)
      SLF_PARMS;
	    char outbuf[80];
	    sprintf(outbuf,"%f", parms->value);
      app::inst()->get_game()->get_message_board()->post(outbuf,parms->time);
#else
      SLF_PARMS_UNUSED;
#endif
      SLF_DONE;
    }
};
//slf_post_message_t slf_post_message2("post_message(num,num)");


// global script library function: num strcmp( str s1, str s2 )
class slf_strcmp_t : public script_library_class::function
{
  public:
    // constructor required
    slf_strcmp_t(const char* n): script_library_class::function(n) { }
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_str_t s1;
      vm_str_t s2;
    };
    // library function execution
    virtual bool operator ()(vm_stack &stack, entry_t entry)
    {
      SLF_PARMS;
      vm_num_t result = strcmp( parms->s1->data(), parms->s2->data() );
      SLF_RETURN;
      SLF_DONE;
    }
};
//slf_strcmp_t slf_strcmp("strcmp(str,str)");


// global script library function: num random( num s2 )
class slf_random_t : public script_library_class::function
{
  public:
    // constructor required
    slf_random_t(const char* n): script_library_class::function(n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_num_t max;
    };
    // library function execution
    virtual bool operator ()(vm_stack &stack, entry_t entry)
    {
      SLF_PARMS;
      vm_num_t result = random((int)parms->max);
      SLF_RETURN;
      SLF_DONE;
    }
};
//slf_random_t slf_random("random(num)");


// global script library function: num get_elevation( vector3d )
class slf_get_elevation_t : public script_library_class::function
{
  public:
    // constructor required
    slf_get_elevation_t(const char* n): script_library_class::function(n) { }
    // library function parameters
    struct parms_t
    {
      // parameters
      vector3d loc;
    };
    // library function execution
    virtual bool operator ()(vm_stack &stack, entry_t entry)
    {
      SLF_PARMS;
      vector3d dummy;
      vm_num_t result = g_world_ptr->get_the_terrain().get_elevation(parms->loc, dummy);
      SLF_RETURN;
      SLF_DONE;
    }
};
//slf_get_elevation_t slf_get_elevation("get_elevation(vector3d)");


// global script library function: add_debug_sphere(vector3d,num);
class slf_add_debug_sphere_t : public script_library_class::function
{
  public:
    // constructor required
    slf_add_debug_sphere_t(const char* n): script_library_class::function(n) { }
    // library function parameters
    struct parms_t
    {
      // parameters
      vector3d pos;
      vm_num_t radius;
    };
    // library function execution
    virtual bool operator ()(vm_stack &stack, entry_t entry)
    {
#ifdef DEBUG
      SLF_PARMS;
      add_debug_sphere(parms->pos,parms->radius);
#else
      SLF_PARMS_UNUSED;
#endif
      SLF_DONE;
    }
};
//slf_add_debug_sphere_t slf_add_debug_sphere("add_debug_sphere(vector3d,num)");

// global script library function: clear_debug_spheres();
class slf_clear_debug_spheres_t : public script_library_class::function
{
  public:
    // constructor required
    slf_clear_debug_spheres_t(const char* n): script_library_class::function(n) { }
    // library function parameters
    struct parms_t
    {
      // parameters
      vector3d pos;
      vm_num_t radius;
    };
    // library function execution
    virtual bool operator ()(vm_stack &stack, entry_t entry)
    {
#ifdef DEBUG
      SLF_PARMS;
      extern void clear_debug_spheres();
      clear_debug_spheres();
#else
      SLF_PARMS_UNUSED;
#endif
      SLF_DONE;
    }
};
//slf_clear_debug_spheres_t slf_clear_debug_spheres("clear_debug_spheres()");

// global script library function: num get_time_inc( )
class slf_get_time_inc_t : public script_library_class::function
{
  public:
    // constructor required
    slf_get_time_inc_t(const char* n): script_library_class::function(n) { }
    // library function parameters
    // library function execution
    virtual bool operator ()(vm_stack &stack, entry_t entry)
    {
      vm_num_t result = g_world_ptr->get_cur_time_inc();
      SLF_RETURN;
      SLF_DONE;
    }
};
//slf_get_time_inc_t slf_get_time_inc("get_time_inc()");


// global script library function: num text_width(str msg)
class slf_text_width_t : public script_library_class::function
{
  public:
    // constructor required
    slf_text_width_t(const char* n): script_library_class::function(n) { }
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_str_t msg;
    };
    // library function execution
    virtual bool operator ()(vm_stack &stack, entry_t entry)
    {
      SLF_PARMS;
      vm_num_t result = (rational_t)font->text_width((*parms->msg).c_str());
      SLF_RETURN;
      SLF_DONE;
    }
};
//slf_text_width_t slf_text_width("text_width(str)");


// global script library function:  bool is_action_button_down( )
class slf_is_action_button_down_t : public script_library_class::function
{
  public:
    // constructor required
    slf_is_action_button_down_t(const char* n) : script_library_class::function(n) {}

    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      vm_num_t result = (input_mgr::inst()->get_control_delta( JOYSTICK_DEVICE, PLAYER_ACTION ) == AXIS_MAX &&
						!app::inst()->get_game()->is_hero_frozen() &&
//!						!g_world_ptr->get_hero_ptr()->get_paralysis_factor() &&
						!g_world_ptr->get_hero_ptr(g_game_ptr->get_active_player())->is_suspended());

//      vm_num_t result = g_world_ptr->get_hero_ptr()->get_sheathe_state()*input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, PLAYER_ACTION );
      SLF_RETURN;
      SLF_DONE;
    }
};
//slf_is_action_button_down_t slf_is_action_button_down("is_action_button_down()");


// global script library function:  bool get_cur_time_inc( )
class slf_get_cur_time_inc_t : public script_library_class::function
{
  public:
    // constructor required
    slf_get_cur_time_inc_t(const char* n) : script_library_class::function(n) {}

    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      vm_num_t result = g_world_ptr->get_cur_time_inc();
      SLF_RETURN;
      SLF_DONE;
    }
};
//slf_get_cur_time_inc_t slf_get_cur_time_inc("get_cur_time_inc()");


// global script library function:  bool TARGET_PC( )
class slf_TARGET_PC_t : public script_library_class::function
{
  public:
    // constructor required
    slf_TARGET_PC_t(const char* n) : script_library_class::function(n) {}

    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
#if defined (TARGET_PC)
      vm_num_t result = true;
#else
      vm_num_t result = false;
#endif
      SLF_RETURN;
      SLF_DONE;
    }
};
//slf_TARGET_PC_t slf_TARGET_PC("TARGET_PC()");


/*H
////////////////////////////////////////////////////////////////////////////////
// global script library function:  blackscreen_on()
class slf_blackscreen_on_t : public script_library_class::function
{
  public:
    // constructor required
    slf_blackscreen_on_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      g_blackscreen = true;
      SLF_DONE;
    }
};
//slf_blackscreen_on_t slf_blackscreen_on("blackscreen_on()");


////////////////////////////////////////////////////////////////////////////////
// global script library function:  blackscreen_off()
class slf_blackscreen_off_t : public script_library_class::function
{
  public:
    // constructor required
    slf_blackscreen_off_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      g_blackscreen = false;
      SLF_DONE;
    }
};
//slf_blackscreen_off_t slf_blackscreen_off("blackscreen_off()");
H*/

////////////////////////////////////////////////////////////////////////////////
// global script library function:  letterbox_on()
class slf_letterbox_on_t : public script_library_class::function
{
  public:
    // constructor required
    slf_letterbox_on_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
//P      app::inst()->get_game()->letterbox_on();
      SLF_DONE;
    }
};
//slf_letterbox_on_t slf_letterbox_on("letterbox_on()");

////////////////////////////////////////////////////////////////////////////////
// global script library function:  letterbox_off()
class slf_letterbox_off_t : public script_library_class::function
{
  public:
    // constructor required
    slf_letterbox_off_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
//P      app::inst()->get_game()->letterbox_off();
      SLF_DONE;
    }
};
//slf_letterbox_off_t slf_letterbox_off("letterbox_off()");

////////////////////////////////////////////////////////////////////////////////
// global script library function:  letterbox_on(str)
class slf_letterbox_cout_t : public script_library_class::function
{
  public:
    // constructor required
    slf_letterbox_cout_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_str_t n;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_PARMS_UNUSED;
//P      app::inst()->get_game()->letterbox_cout(localize_text(*parms->n));
      SLF_DONE;
    }
};
//slf_letterbox_cout_t slf_letterbox_cout("letterbox_cout(str)");


////////////////////////////////////////////////////////////////////////////////
// global script library function:  fade_out()
class slf_fade_out_t : public script_library_class::function
{
  public:
    // constructor required
    slf_fade_out_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
//P      app::inst()->get_game()->fade_out();
      SLF_DONE;
    }
};
//slf_fade_out_t slf_fade_out("fade_out()");


////////////////////////////////////////////////////////////////////////////////
// global script library function:  fade_in()
class slf_fade_in_t : public script_library_class::function
{
  public:
    // constructor required
    slf_fade_in_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
//P      app::inst()->get_game()->fade_in();
      SLF_DONE;
    }
};
//slf_fade_in_t slf_fade_in("fade_in()");



////////////////////////////////////////////////////////////////////////////////
// global script library function:  set_region_active(str,num)
//
// If the given num is zero, the named region will be ineligible for rendering, etc.;
// otherwise, the named region will behave normally.
class slf_set_region_active_t : public script_library_class::function
{
  public:
    // constructor required
    slf_set_region_active_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_str_t region_name;
      vm_num_t v;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_PARMS;
      region_node* rn = g_world_ptr->get_the_terrain().find_region( *parms->region_name );
      if ( !rn )
      {
        stack.get_thread()->slf_error( "set_region_active(): region '" + *parms->region_name + "' not found" );
      }
      region* r = rn->get_data();
      r->set_locked( false );
      if ( parms->v )
        r->set_active( true );
      else
      {
        r->set_active( false );
        r->set_locked( true );
      }
      SLF_DONE;
    }
};
//slf_set_region_active_t slf_set_region_active("set_region_active(str,num)");

////////////////////////////////////////////////////////////////////////////////
// global script library function:  set_fog_table_gamma(num)
class slf_set_fog_table_gamma_t : public script_library_class::function
{
  public:
    // constructor required
    slf_set_fog_table_gamma_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_num_t g;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_PARMS;
    //#if defined(TARGET_MKS)
      fog_manager::inst()->set_fog_table_gamma( parms->g );
    //#endif
      SLF_DONE;
    }
};
//slf_set_fog_table_gamma_t slf_set_fog_table_gamma("set_fog_table_gamma(num)");

/*H
////////////////////////////////////////////////////////////////////////////////
class slf_game_segue_from_intro_t : public script_library_class::function
{
  public:
    // constructor required
    slf_game_segue_from_intro_t(const char* n) : script_library_class::function(n) {}
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      app::inst()->get_game()->segue_from_intro( 0.75f, 1.0f );
      SLF_DONE;
    }
};
//slf_game_segue_from_intro_t slf_game_segue_from_intro("game_segue_from_intro()");

////////////////////////////////////////////////////////////////////////////////
class slf_game_segue_from_demo_t : public script_library_class::function
{
  public:
    // constructor required
    slf_game_segue_from_demo_t(const char* n) : script_library_class::function(n) {}
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      app::inst()->get_game()->segue_from_demo();
      SLF_DONE;
    }
};
//slf_game_segue_from_demo_t slf_game_segue_from_demo("game_segue_from_demo()");

////////////////////////////////////////////////////////////////////////////////
class slf_game_segue_from_intro_parms_t : public script_library_class::function
{
  public:
    // constructor required
    slf_game_segue_from_intro_parms_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_num_t xlu;
      vm_num_t dur;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_PARMS;
      app::inst()->get_game()->segue_from_intro( parms->xlu, parms->dur );
      SLF_DONE;
    }
};
//slf_game_segue_from_intro_parms_t slf_game_segue_from_intro_parms("game_segue_from_intro_parms(num,num)");

H*/

// global script library function: num get_detail_level()
class slf_get_detail_level_t : public script_library_class::function
{
  public:
    // constructor required
    slf_get_detail_level_t(const char* n): script_library_class::function(n) { }
    // library function execution
    virtual bool operator ()(vm_stack &stack, entry_t entry)
    {
      #ifdef TARGET_PC
      vm_num_t result = os_developer_options::inst()->get_int(os_developer_options::INT_DETAIL_LEVEL);
      #else
      vm_num_t result = 2;
      #endif
      SLF_RETURN;
      SLF_DONE;
    }
};
//slf_get_detail_level_t slf_get_detail_level("get_detail_level()");


// global script library function: num was_start_pressed()
class slf_was_start_pressed_t : public script_library_class::function
{
  public:
    // constructor required
    slf_was_start_pressed_t(const char* n): script_library_class::function(n) { }
    // library function execution
    virtual bool operator ()(vm_stack &stack, entry_t entry)
    {
      vm_num_t result = app::inst()->get_game()->was_start_pressed();
      SLF_RETURN;
      SLF_DONE;
    }
};
//slf_was_start_pressed_t slf_was_start_pressed("was_start_pressed()");

// global script library function: num was_A_pressed()
class slf_was_A_pressed_t : public script_library_class::function
{
  public:
    // constructor required
    slf_was_A_pressed_t(const char* n): script_library_class::function(n) { }
    // library function execution
    virtual bool operator ()(vm_stack &stack, entry_t entry)
    {
      vm_num_t result = app::inst()->get_game()->was_A_pressed();
      SLF_RETURN;
      SLF_DONE;
    }
};
//slf_was_A_pressed_t slf_was_A_pressed("was_A_pressed()");

// global script library function: num was_A_pressed()
class slf_was_B_pressed_t : public script_library_class::function
{
  public:
    // constructor required
    slf_was_B_pressed_t(const char* n): script_library_class::function(n) { }
    // library function execution
    virtual bool operator ()(vm_stack &stack, entry_t entry)
    {
      vm_num_t result = app::inst()->get_game()->was_B_pressed();
      SLF_RETURN;
      SLF_DONE;
    }
};
//slf_was_B_pressed_t slf_was_B_pressed("was_B_pressed()");


////////////////////////////////////////////////////////////////////////////////
// PURUPURU PACK FU
////////////////////////////////////////////////////////////////////////////////
/*
class slf_vibrate_purupuru_t : public script_library_class::function
{

  public:
    // constructor required
    slf_vibrate_purupuru_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_num_t vibrator_flag;
      vm_num_t vibrator_power;
      vm_num_t vibrator_freq;
      vm_num_t vibrator_inc;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_PARMS;
printf("this function not supported.  Jason Bare told me so");
//      input_device* joy=input_mgr::inst()->get_device(JOYSTICK_DEVICE);
//		  if (joy && joy->is_vibrator_present())
//        joy->vibrate( parms->vibrator_flag, parms->vibrator_power, parms->vibrator_freq, parms->vibrator_inc );
      SLF_DONE;
    }
};
//slf_vibrate_purupuru_t slf_vibrate_purupuru("vibrate_purupuru(num,num,num,num)");
*/



/*P
////////////////////////////////////////////////////////////////////////////////
class slf_lock_spell_invocations_t : public script_library_class::function
{
  public:
    // constructor required
    slf_lock_spell_invocations_t(const char* n) : script_library_class::function(n) {}
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
//P	    app::inst()->get_game()->lock_spell_invocations();
      SLF_DONE;
    }
};
//slf_lock_spell_invocations_t slf_lock_spell_invocations("lock_spell_invocations()");

////////////////////////////////////////////////////////////////////////////////
class slf_unlock_spell_invocations_t : public script_library_class::function
{
  public:
    // constructor required
    slf_unlock_spell_invocations_t(const char* n) : script_library_class::function(n) {}
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
	    app::inst()->get_game()->unlock_spell_invocations();
      SLF_DONE;
    }
};
//slf_unlock_spell_invocations_t slf_unlock_spell_invocations("unlock_spell_invocations()");

////////////////////////////////////////////////////////////////////////////////
class slf_lock_utility_spell_invocations_t : public script_library_class::function
{
  public:
    // constructor required
    slf_lock_utility_spell_invocations_t(const char* n) : script_library_class::function(n) {}
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
	    app::inst()->get_game()->lock_utility_spell_invocations();
      SLF_DONE;
    }
};
//slf_lock_utility_spell_invocations_t slf_lock_utility_spell_invocations("lock_utility_spell_invocations()");

////////////////////////////////////////////////////////////////////////////////
class slf_unlock_utility_spell_invocations_t : public script_library_class::function
{
  public:
    // constructor required
    slf_unlock_utility_spell_invocations_t(const char* n) : script_library_class::function(n) {}
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
	    app::inst()->get_game()->unlock_utility_spell_invocations();
      SLF_DONE;
    }
};
//slf_unlock_utility_spell_invocations_t slf_unlock_utility_spell_invocations("unlock_utility_spell_invocations()");
P*/
/*H
////////////////////////////////////////////////////////////////////////////////
class slf_lock_hero_weapon_t : public script_library_class::function
{
  public:
    // constructor required
    slf_lock_hero_weapon_t(const char* n) : script_library_class::function(n) {}
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
	    app::inst()->get_game()->lock_hero_weapon();
      SLF_DONE;
    }
};
//slf_lock_hero_weapon_t slf_lock_hero_weapon("lock_hero_weapon()");

////////////////////////////////////////////////////////////////////////////////
class slf_unlock_hero_weapon_t : public script_library_class::function
{
  public:
    // constructor required
    slf_unlock_hero_weapon_t(const char* n) : script_library_class::function(n) {}
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
	    app::inst()->get_game()->unlock_hero_weapon();
      SLF_DONE;
    }
};
//slf_unlock_hero_weapon_t slf_unlock_hero_weapon("unlock_hero_weapon()");
//////////////////////////////////////////////////////////////////////////////////
class slf_slew_game_time_t : public script_library_class::function
{
  public:
    // constructor required
    slf_slew_game_time_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_num_t new_slew;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_PARMS;
	    slew_time( parms->new_slew );
      SLF_DONE;
    }
};
//slf_slew_game_time_t slf_slew_game_time("slew_game_time(num)");

////////////////////////////////////////////////////////////////////////////////
class slf_normal_game_time_t : public script_library_class::function
{
  public:
    // constructor required
    slf_normal_game_time_t(const char* n) : script_library_class::function(n) {}
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
	    normal_time();
      SLF_DONE;
    }
};
//slf_normal_game_time_t slf_normal_game_time("normal_game_time()");
H*/


//////////////////////////////////////////////////////////////////////////////////
// global script library function:  num get_control_trigger(num)
class slf_get_control_trigger_t : public script_library_class::function
{
  public:
    // constructor required
    slf_get_control_trigger_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_num_t id;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_PARMS;
      vm_num_t result = input_mgr::inst()->get_control_trigger( JOYSTICK_DEVICE, parms->id );
      SLF_RETURN;
      SLF_DONE;
    }
};
//slf_get_control_trigger_t slf_get_control_trigger("get_control_trigger(num)");


//////////////////////////////////////////////////////////////////////////////////
// global script library function:  num get_control_state( JOYSTICK_DEVICE,num)
class slf_get_control_state_t : public script_library_class::function
{
  public:
    // constructor required
    slf_get_control_state_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_num_t id;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_PARMS;
      vm_num_t result = input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, parms->id );
      SLF_RETURN;
      SLF_DONE;
    }
};
//slf_get_control_state_t slf_get_control_state("get_control_state(num)");


// global script library function: num wait_keypress(str key)
class slf_wait_keypress_t : public script_library_class::function
{
  public:
    // constructor required
    slf_wait_keypress_t(const char* n): script_library_class::function(n) { }
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_str_t key;
    };
    // library function execution
    virtual bool operator ()(vm_stack &stack, entry_t entry)
    {
      SLF_PARMS_UNUSED;
      if ( input_mgr::inst()->get_control_delta( JOYSTICK_DEVICE, PLAYER_VSIM_FREE_MODE ) == AXIS_MAX )
      {
        SLF_DONE;
      }
      else
      {
        SLF_RECALL;
      }
    }
};
//slf_wait_keypress_t slf_wait_keypress("wait_keypress(str)");

/*H
////////////////////////////////////////////////////////////////////////////////
// INGAME_MAP SUPPORT FUNCTIONS
//   USED BY SCRIPTERS TO MAINTAIN A LIST OF MAP LOCATIONS
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class slf_ingame_map_clear_list_t : public script_library_class::function
{
  public:
    // constructor required
    slf_ingame_map_clear_list_t(const char* n) : script_library_class::function(n) {}
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      app::inst()->get_game()->ingame_map_clear_list();
      SLF_DONE;
    }
};
//slf_ingame_map_clear_list_t slf_ingame_map_clear_list("ingame_map_clear_list()");

////////////////////////////////////////////////////////////////////////////////
class slf_ingame_map_show_item_t : public script_library_class::function
{
  public:
    // constructor required
    slf_ingame_map_show_item_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_str_t id;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_PARMS;
      app::inst()->get_game()->ingame_map_show_item( *parms->id );
      SLF_DONE;
    }
};
//slf_ingame_map_show_item_t slf_ingame_map_show_item("ingame_map_show_item(str)");

////////////////////////////////////////////////////////////////////////////////
class slf_ingame_map_hide_item_t : public script_library_class::function
{
  public:
    // constructor required
    slf_ingame_map_hide_item_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_str_t id;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_PARMS;
      app::inst()->get_game()->ingame_map_hide_item( *parms->id );
      SLF_DONE;
    }
};
//slf_ingame_map_hide_item_t slf_ingame_map_hide_item("ingame_map_hide_item(str)");

////////////////////////////////////////////////////////////////////////////////
class slf_ingame_map_remove_item_t : public script_library_class::function
{
  public:
    // constructor required
    slf_ingame_map_remove_item_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_str_t id;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_PARMS;
      app::inst()->get_game()->ingame_map_remove_item( *parms->id );
      SLF_DONE;
    }
};
//slf_ingame_map_remove_item_t slf_ingame_map_remove_item("ingame_map_remove_item(str)");

////////////////////////////////////////////////////////////////////////////////
class slf_ingame_map_add_item_t : public script_library_class::function
{
  public:
    // constructor required
    slf_ingame_map_add_item_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_str_t id;
      vm_num_t where_x;
      vm_num_t where_y;
      vm_str_t texture;
      vm_str_t description;
      vm_num_t visible;
      vm_str_t map;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_PARMS;
      app::inst()->get_game()->ingame_map_add_item( *parms->id, parms->where_x, parms->where_y, *parms->texture, *parms->description, parms->visible, *parms->map );
      SLF_DONE;
    }
};
//slf_ingame_map_add_item_t slf_ingame_map_add_item("ingame_map_add_item(str,num,num,str,str,num,str)");

////////////////////////////////////////////////////////////////////////////////
class slf_ingame_map_add_item1_t : public script_library_class::function
{
  public:
    // constructor required
    slf_ingame_map_add_item1_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_str_t id;
      vector3d where;
      vm_str_t texture;
      vm_str_t description;
      vm_num_t visible;
	    vm_str_t map;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_PARMS;
      vm_num_t where_x;
      vm_num_t where_y;

      automap* curmap = g_world_ptr->get_current_map();
      vector2d map_where = curmap->get_map_coord( g_world_ptr->get_hero_ptr()->get_abs_position() );
      where_x = map_where.x;
      where_y = map_where.y;
      app::inst()->get_game()->ingame_map_add_item( *parms->id, where_x, where_y, *parms->texture, *parms->description, parms->visible, *parms->map );

      SLF_DONE;
    }
};
//slf_ingame_map_add_item1_t slf_ingame_map_add_item1("ingame_map_add_item(str,vector3d,str,str,num,str)");
H*/

class slf_apply_radius_damage_t : public script_library_class::function
{
public:
  // constructor required
  slf_apply_radius_damage_t(const char* n): script_library_class::function(n) {}

  // library function parameters
  struct parms_t
  {
    // parameters
    vector3d center;
    vm_num_t radius;
    vm_num_t bio_damage;
    vm_num_t mechanical_damage;
  };
  // library function execution
  virtual bool operator ()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    g_world_ptr->apply_radius_damage(parms->center, parms->radius, parms->bio_damage, parms->mechanical_damage);
    SLF_DONE;
  }
};
//slf_apply_radius_damage_t slf_apply_radius_damage("apply_radius_damage(vector3d,num,num,num)");


//////////////////////////////////////////////////////////////////////////////////
// global script library function:  num los_check(vector3d,vector3d)
class slf_los_check_t : public script_library_class::function
{
  public:
    // constructor required
    slf_los_check_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      vector3d p0;
      vector3d p1;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_PARMS;
      vm_num_t result = 1;

      sector *sect = g_world_ptr->get_the_terrain().find_sector(parms->p0);
      if(sect && sect->get_region())
      {
        vector3d hit_pos;
        vector3d hit_norm;
        entity *hit_entity;
        bool hit = find_intersection( parms->p0,
                                      parms->p1,
                                      sect->get_region(),
                                      FI_COLLIDE_WORLD | FI_COLLIDE_BEAMABLE,
                                      &hit_pos,
                                      &hit_norm,
                                      NULL,
                                      &hit_entity
                                    );

        if(hit)
          result = 0;
      }
      else
        result = 0;

      SLF_RETURN;
      SLF_DONE;
    }
};
//slf_los_check_t slf_los_check("los_check(vector3d,vector3d)");



#if _CONSOLE_ENABLE

#include "console.h"

class slf_add_to_console_t : public script_library_class::function
{
  public:
    // constructor required
    slf_add_to_console_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_str_t log;
    };

    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_PARMS;

      console_log( parms->log->c_str() );

      SLF_DONE;
    }
};
//slf_add_to_console_t slf_add_to_console("add_to_console(str)");

class slf_console_exec_t : public script_library_class::function
{
  public:
    // constructor required
    slf_console_exec_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_str_t cmd;
    };

    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_PARMS;

      console_process( parms->cmd->c_str() );

      SLF_DONE;
    }
};
//slf_console_exec_t slf_console_exec("console_exec(str)");

#endif  // _CONSOLE_ENABLE



#define MAX_STATIC_TO_STR_STRINGS 30
static stringx script_strings[MAX_STATIC_TO_STR_STRINGS];
static int     script_strings_cnt;
static stringx script_game_string;
stringx        script_string_none = "none";
stringx        script_return_string;

void script_strings_deconstruct()
{
  for( int i=0; i<MAX_STATIC_TO_STR_STRINGS; i++ )
  {
    script_strings[ i ] = stringx();
  }
  script_game_string = stringx();
  script_string_none = stringx();
  script_return_string = stringx();
}


// script library function: str str_append(str,str,num)
class slf_str_append_t : public script_library_class::function
{
public:
  // constructor required
  slf_str_append_t(const char *n): script_library_class::function(n) { }

  // library function parameters
  struct parms_t
  {
    // parameters
    vm_str_t s1;
    vm_str_t s2;
    vm_num_t space;
  };

  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    static int str_app_counter = 0;
    const char* spacer = " ";
    stringx s = *parms->s1;

    for(int i=0; i<(int)parms->space; ++i)
      s += spacer;

    s += *parms->s2;
    script_strings[str_app_counter] = s;

    vm_str_t result = &script_strings[str_app_counter];

    str_app_counter++;
    if(str_app_counter >= MAX_STATIC_TO_STR_STRINGS)
      str_app_counter = 0;

    SLF_RETURN;
    SLF_DONE;
  }
};

// script library function: str to_str_vector3d(vector3d)
class slf_to_str_vector3d_t : public script_library_class::function
{
public:
  // constructor required
  slf_to_str_vector3d_t(const char *n): script_library_class::function(n) { }

  // library function parameters
  struct parms_t
  {
    // parameters
    vector3d v;
  };

  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    script_strings[script_strings_cnt] = stringx(stringx::fmt, "<%.3f, %.3f, %.3f>", parms->v.x, parms->v.y, parms->v.z);
    vm_str_t result = &script_strings[script_strings_cnt];
    ++script_strings_cnt;
    if(script_strings_cnt >= MAX_STATIC_TO_STR_STRINGS)
      script_strings_cnt = 0;

    SLF_RETURN;
    SLF_DONE;
  }
};

// script library function: str to_str(num)
class slf_to_str_num_t : public script_library_class::function
{
public:
  // constructor required
  slf_to_str_num_t(const char *n): script_library_class::function(n) { }

  // library function parameters
  struct parms_t
  {
    // parameters
    vm_num_t n;
  };

  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    script_strings[script_strings_cnt] = stringx(parms->n);
    vm_str_t result = &script_strings[script_strings_cnt];
    ++script_strings_cnt;
    if(script_strings_cnt >= MAX_STATIC_TO_STR_STRINGS)
      script_strings_cnt = 0;

    SLF_RETURN;
    SLF_DONE;
  }
};



// script library function: str to_int_str(num)
class slf_to_str_int_t : public script_library_class::function
{
public:
  // constructor required
  slf_to_str_int_t(const char *n): script_library_class::function(n) { }

  // library function parameters
  struct parms_t
  {
    // parameters
    vm_num_t n;
  };

  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    script_strings[script_strings_cnt] = stringx(((int)parms->n));
    vm_str_t result = &script_strings[script_strings_cnt];
    ++script_strings_cnt;
    if(script_strings_cnt >= MAX_STATIC_TO_STR_STRINGS)
      script_strings_cnt = 0;

    SLF_RETURN;
    SLF_DONE;
  }
};



// script library function: add_3d_debug_str(vector3d,vector3d,num,str)
class slf_add_3d_debug_str_t : public script_library_class::function
{
public:
  // constructor required
  slf_add_3d_debug_str_t(const char *n): script_library_class::function(n) { }

  // library function parameters
  struct parms_t
  {
    // parameters
    vector3d pos;
    vector3d col;
    vm_num_t dur;
    vm_str_t s1;
  };

  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
#ifdef BUILD_BOOTABLE
    SLF_PARMS_UNUSED;
#else
    SLF_PARMS;

    add_3d_text(parms->pos, color32(parms->col.x * 255, parms->col.y * 255, parms->col.z * 255), parms->dur, *parms->s1);
#endif

    SLF_DONE;
  }
};

// script library function: add_2d_debug_str(vector3d,vector3d,num,str)
class slf_add_2d_debug_str_t : public script_library_class::function
{
public:
  // constructor required
  slf_add_2d_debug_str_t(const char *n): script_library_class::function(n) { }

  // library function parameters
  struct parms_t
  {
    // parameters
    vector3d pos;
    vector3d col;
    vm_num_t dur;
    vm_str_t s1;
  };

  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
#ifdef BUILD_BOOTABLE
    SLF_PARMS_UNUSED;
#else
    SLF_PARMS;

    add_2d_text(parms->pos, color32(parms->col.x * 255, parms->col.y * 255, parms->col.z * 255), parms->dur, *parms->s1);
#endif

    SLF_DONE;
  }
};



static vector3d script_bsp_hit_pos = ZEROVEC;
static vector3d script_bsp_hit_norm = YVEC;
static entity  *script_bsp_hit_ent = NULL;

//////////////////////////////////////////////////////////////////////////////////
// global script library function:  num bsp_check(vector3d,vector3d,num)
class slf_bsp_check_t : public script_library_class::function
{
  public:
    // constructor required
    slf_bsp_check_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      vector3d p0;
      vector3d p1;
      vm_num_t flags;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_PARMS;
      vm_num_t result = 0;

      script_bsp_hit_pos = ZEROVEC;
      script_bsp_hit_norm = YVEC;
      script_bsp_hit_ent = NULL;

      region_node *reg = NULL;
      sector *sect = g_world_ptr->get_the_terrain().find_sector(parms->p0);
      if(sect && sect->get_region())
        reg = sect->get_region();
      else
      {
        vector3d tmp = parms->p0;
        parms->p0 = parms->p1;
        parms->p1 = tmp;

        sect = g_world_ptr->get_the_terrain().find_sector(parms->p0);
        if(sect && sect->get_region())
          reg = sect->get_region();
      }

      if(reg)
      {
        result = find_intersection( parms->p0,
                                    parms->p1,
                                    reg,
                                    (unsigned int)parms->flags,
                                    &script_bsp_hit_pos,
                                    &script_bsp_hit_norm,
                                    NULL,
                                    &script_bsp_hit_ent
                                  );
      }

      SLF_RETURN;
      SLF_DONE;
    }
};

//////////////////////////////////////////////////////////////////////////////////
// global script library function:  vector3d get_bsp_check_hit_pos()
class slf_get_bsp_check_hit_pos_t : public script_library_class::function
{
  public:
    // constructor required
    slf_get_bsp_check_hit_pos_t(const char* n) : script_library_class::function(n) {}
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      vector3d result = script_bsp_hit_pos;
      SLF_RETURN;
      SLF_DONE;
    }
};

//////////////////////////////////////////////////////////////////////////////////
// global script library function:  vector3d get_bsp_check_hit_norm()
class slf_get_bsp_check_hit_norm_t : public script_library_class::function
{
  public:
    // constructor required
    slf_get_bsp_check_hit_norm_t(const char* n) : script_library_class::function(n) {}
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      vector3d result = script_bsp_hit_norm;
      SLF_RETURN;
      SLF_DONE;
    }
};

//////////////////////////////////////////////////////////////////////////////////
// global script library function:  entity get_bsp_check_hit_ent()
class slf_get_bsp_check_hit_ent_t : public script_library_class::function
{
  public:
    // constructor required
    slf_get_bsp_check_hit_ent_t(const char* n) : script_library_class::function(n) {}
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      vm_entity_t result = static_cast<vm_entity_t>(script_bsp_hit_ent);
      SLF_RETURN;
      SLF_DONE;
    }
};

extern profiler_timer proftimer_collide_entity_entity_int;

// global script library function: num entity_col_check(entity entity1, entity entity2)
class slf_entity_col_check_t : public script_library_class::function
{
  public:
    // constructor required
    slf_entity_col_check_t(const char* n): script_library_class::function(n) { }
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_entity_t entity1;
      vm_entity_t entity2;
    };
    // library function execution
    virtual bool operator ()(vm_stack &stack, entry_t entry)
    {
      SLF_PARMS;
      proftimer_collide_entity_entity_int.start();
      vm_num_t result = g_world_ptr->entity_entity_collision_check(parms->entity1, parms->entity2, 0.0f);
      proftimer_collide_entity_entity_int.stop();
      SLF_RETURN;
      SLF_DONE;
    }
};


// script library function: play_sound_vol(str,num)
class slf_play_sound_vol_t : public script_library_class::function
{
public:
  // constructor required
  slf_play_sound_vol_t(const char *n): script_library_class::function(n) { }

  // library function parameters
  struct parms_t
  {
    // parameters
    vm_str_t snd;
    vm_num_t vol;
  };

  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS_UNUSED;

    warning("Function play_sound_vol(str,num) not available!");

    SLF_DONE;
  }
};






//////////////////////////////////////////////////////////////////////////////////
// global script library function:  num get_global_time_dilation()
class slf_get_global_time_dilation_t : public script_library_class::function
{
  public:
    // constructor required
    slf_get_global_time_dilation_t(const char* n) : script_library_class::function(n) {}
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      vm_num_t result = g_time_dilation;
      SLF_RETURN;
      SLF_DONE;
    }
};

// script library function: set_global_time_dilation(num)
class slf_set_global_time_dilation_t : public script_library_class::function
{
public:
  // constructor required
  slf_set_global_time_dilation_t(const char *n): script_library_class::function(n) { }

  // library function parameters
  struct parms_t
  {
    // parameters
    vm_num_t d;
  };

  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    g_time_dilation = parms->d;
    if(g_time_dilation < 0.0f)
      g_time_dilation = 0.0f;

    SLF_DONE;
  }
};


// global script library function: wait_set_global_time_dilation(num,num)
class slf_wait_set_global_time_dilation_t : public script_library_class::function
{
  public:
    // constructor required
    slf_wait_set_global_time_dilation_t(const char* n) : script_library_class::function(n) {}
    // library function persistent local vm_stack data;
    // stored on stack (after parms) until function completed;
    // accessed by SLF_SDATA at start of function
    struct sdata_t
    {
      time_value_t clock;
      vm_num_t     start_dilation;
    };

    // library function parameters;
    // accessed by SLF_PARMS
    struct parms_t
    {
      vm_num_t dest_dilation;
      vm_num_t duration;
    };

    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_SDATA;
      SLF_PARMS;
      if (entry == FIRST_ENTRY)
      {
        // first entry to this library call with this stack;
        // set up timer
        if(parms->duration==0)
        {
          g_time_dilation = parms->dest_dilation;
          SLF_DONE;
        }
        else
        {
          sdata->clock = 0.0f;
          sdata->start_dilation = g_time_dilation;
          SLF_RECALL;
        }
      }
      else
      {
        sdata->clock += SCRIPT_TIME_INC;
//        time_value_t time_elapsed = sdata->clock.elapsed();
        float percentage_elapsed = sdata->clock / parms->duration;
        percentage_elapsed = min( percentage_elapsed, 1.0f );

        g_time_dilation = ((parms->dest_dilation - sdata->start_dilation) * percentage_elapsed) + sdata->start_dilation;

        if( sdata->clock >= parms->duration )
        {
          SLF_DONE;
        }
        else
        {
          SLF_RECALL;
        }
      }
    }
};
//slf_wait_set_global_time_dilation_t slf_wait_set_global_time_dilation("wait_set_global_time_dilation(num,num)");

// global script library function: num load_level(str level_name)
class slf_load_level_t : public script_library_class::function
{
  public:
    // constructor required
    slf_load_level_t(const char* n): script_library_class::function(n) { }
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_str_t level_name;
    };
    // library function execution
    virtual bool operator ()(vm_stack &stack, entry_t entry)
    {
      SLF_PARMS;
      g_game_ptr->load_new_level(*parms->level_name);
      SLF_DONE;
    }
};

extern char g_movie_name[256];

// global script library function: num play_movie(str movie_name, num continue_level)
class slf_play_movie_t : public script_library_class::function
{
  public:
    // constructor required
    slf_play_movie_t(const char* n): script_library_class::function(n) { }
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_str_t movie_name;
      vm_num_t continue_level;
    };
    // library function execution
    virtual bool operator ()(vm_stack &stack, entry_t entry)
    {
      SLF_PARMS;
//#ifdef TARGET_PC
      g_game_ptr->play_movie(parms->movie_name->c_str(),true);
/*#elif defined TARGET_PS2
      strcpy(g_movie_name, "movies\\");
      strcat(g_movie_name, parms->movie_name->c_str());
      strcat(g_movie_name, ".PSS");
      movieplayer::inst()->play( g_movie_name, true, true );
      sound_device::inst()->partial_shutdown();
      sound_device::inst()->partial_init();
#endif*/
      SLF_DONE;
    }
};



// global script library function: set_game_info_num(str att, num val)
class slf_set_game_info_num_t : public script_library_class::function
{
  public:
    // constructor required
    slf_set_game_info_num_t(const char* n): script_library_class::function(n) { }
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_str_t att;
      vm_num_t val;
    };
    // library function execution
    virtual bool operator ()(vm_stack &stack, entry_t entry)
    {
      SLF_PARMS;
      pstring p(parms->att->c_str());
      g_game_ptr->get_game_info().set_num(p, parms->val);
      SLF_DONE;
    }
};



// global script library function:  num get_game_info_num(str)
class slf_get_game_info_num_t : public script_library_class::function
{
  public:
    // constructor required
    slf_get_game_info_num_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_str_t att;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_PARMS;

      vm_num_t result = 0;

      pstring p(parms->att->c_str());
      g_game_ptr->get_game_info().get_num(p, result);

      SLF_RETURN;
      SLF_DONE;
    }
};


// global script library function: set_game_info_str(str att, str val)
class slf_set_game_info_str_t : public script_library_class::function
{
  public:
    // constructor required
    slf_set_game_info_str_t(const char* n): script_library_class::function(n) { }
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_str_t att;
      vm_str_t val;
    };
    // library function execution
    virtual bool operator ()(vm_stack &stack, entry_t entry)
    {
      SLF_PARMS;
      pstring p(parms->att->c_str());
      g_game_ptr->get_game_info().set_str(p, *parms->val);
      SLF_DONE;
    }
};



// global script library function:  str get_game_info_str(str)
class slf_get_game_info_str_t : public script_library_class::function
{
  public:
    // constructor required
    slf_get_game_info_str_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_str_t att;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_PARMS;


      script_game_string = empty_string;
      pstring p(parms->att->c_str());
      g_game_ptr->get_game_info().get_str(p, script_game_string);

      vm_str_t result = &script_game_string;

      SLF_RETURN;
      SLF_DONE;
    }
};

// global script library function: wait_fps_test(num min_fps, num delay, vector3d pos, vector3d facing)
class slf_wait_fps_test_t : public script_library_class::function
{
  public:
    // constructor required
    slf_wait_fps_test_t(const char* n) : script_library_class::function(n) {}
    // library function persistent local vm_stack data;
    // stored on stack (after parms) until function completed;
    // accessed by SLF_SDATA at start of function
    struct sdata_t
    {
      game_clock_t clock;
      po initial_cam_po;
      vector3d initial_target;
      rational_t initial_roll;
      vm_num_t avg_fps;
      int num_frames;
      bool disable_marky;
    };
    // library function parameters;
    // accessed by SLF_PARMS
    struct parms_t
    {
      vm_num_t min_fps;
      vm_num_t duration;
      vector3d pos;
      vector3d facing;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_SDATA;
      SLF_PARMS;
      if (entry == FIRST_ENTRY)
      {
        // first entry to this library call with this stack;
        // set up timer
        sdata->clock.reset();

      #if defined(TARGET_PC)
        if(!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_SHOW_FPS))
        {
          sdata->avg_fps = 0.0f;
          sdata->num_frames = 0;

          sdata->disable_marky = !g_world_ptr->is_marky_cam_enabled();
          if(sdata->disable_marky)
          {
            g_game_ptr->enable_marky_cam(true);
          }

          sdata->initial_cam_po = g_world_ptr->get_marky_cam_ptr()->get_rel_po();
          sdata->initial_target = sdata->initial_cam_po.get_facing();
          sdata->initial_roll = g_world_ptr->get_marky_cam_ptr()->get_roll();

          g_world_ptr->get_marky_cam_ptr()->camera_set_target(parms->pos+parms->facing);
          g_world_ptr->get_marky_cam_ptr()->camera_set_roll(0.0f);

          po the_po = po_identity_matrix;
          the_po.set_facing(parms->facing);
          the_po.set_position(parms->pos);
          g_world_ptr->get_marky_cam_ptr()->set_rel_po(the_po);
        }
      #endif

        SLF_RECALL;
      }
      else
      {
        if( sdata->clock.elapsed() >= parms->duration )
        {
          vm_num_t result = 1.0f;

        #if defined(TARGET_PC)
          if(!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_SHOW_FPS))
          {
            if(sdata->num_frames > 0)
            {
              sdata->avg_fps /= (float)sdata->num_frames;

              if(sdata->avg_fps < parms->min_fps)
                result = 0.0f;
  //              error("Demo Error:\n\nMinimum FPS setting has not been met by this PC.");
            }

            g_world_ptr->get_marky_cam_ptr()->set_rel_po(sdata->initial_cam_po);
            g_world_ptr->get_marky_cam_ptr()->camera_set_target(sdata->initial_target);
            g_world_ptr->get_marky_cam_ptr()->camera_set_roll(sdata->initial_roll);

            if(sdata->disable_marky)
            {
              g_game_ptr->enable_marky_cam(false);
              sdata->disable_marky = false;
            }
          }
        #endif

          SLF_RETURN;
          SLF_DONE;
        }
        else
        {
        #if defined(TARGET_PC)
          if(!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_SHOW_FPS))
          {
            sdata->avg_fps += g_game_ptr->get_theoretical_fps();
            sdata->num_frames++;
          }
        #endif

          SLF_RECALL;
        }
      }
    }
};
//slf_wait_fps_test_t slf_wait_fps_test("wait_fps_test(num)");


// global script library function:  freeze_universe(num)
class slf_freeze_universe_t : public script_library_class::function
{
  public:
    // constructor required
    slf_freeze_universe_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_num_t freeze;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_PARMS_UNUSED;

      //g_game_ptr->enable_physics((parms->freeze != 0.0f));

      SLF_DONE;
    }
};

 /*
           sdata->start_color = vector3d( start_color4.get_red(),
                                         start_color4.get_green(),
                                         start_color4.get_blue() );
*/
////////////////////////////////////////////////////////////////////////////////
// global script library function: color get_fog_color()
class slf_get_fog_color_t : public script_library_class::function
{
  public:
    // constructor required
    slf_get_fog_color_t(const char* n) : script_library_class::function(n) {}
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      vector3d result = vector3d( fog_manager::inst()->get_fog_color().get_red(),
                                fog_manager::inst()->get_fog_color().get_green(),
                                fog_manager::inst()->get_fog_color().get_blue() );
      SLF_RETURN;
      SLF_DONE;
    }
};

////////////////////////////////////////////////////////////////////////////////
// global script library function: num get_fog_distance()
class slf_get_fog_distance_t : public script_library_class::function
{
  public:
    // constructor required
    slf_get_fog_distance_t(const char* n) : script_library_class::function(n) {}
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      vm_num_t result = fog_manager::inst()->get_fog_end_distance();
      SLF_RETURN;
      SLF_DONE;
    }
};



////////////////////////////////////////////////////////////////////////////////
// global script library function: num systime_month()
class slf_systime_month_t : public script_library_class::function
{
  public:
    // constructor required
    slf_systime_month_t(const char* n) : script_library_class::function(n) {}
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      vm_num_t result = g_game_ptr->get_systime().month;

      SLF_RETURN;
      SLF_DONE;
    }
};

////////////////////////////////////////////////////////////////////////////////
// global script library function: num systime_day()
class slf_systime_day_t : public script_library_class::function
{
  public:
    // constructor required
    slf_systime_day_t(const char* n) : script_library_class::function(n) {}
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      vm_num_t result = g_game_ptr->get_systime().day;

      SLF_RETURN;
      SLF_DONE;
    }
};

////////////////////////////////////////////////////////////////////////////////
// global script library function: num systime_day_of_week()
class slf_systime_day_of_week_t : public script_library_class::function
{
  public:
    // constructor required
    slf_systime_day_of_week_t(const char* n) : script_library_class::function(n) {}
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      vm_num_t result = g_game_ptr->get_systime().weekday;

      SLF_RETURN;
      SLF_DONE;
    }
};

////////////////////////////////////////////////////////////////////////////////
// global script library function: num systime_year()
class slf_systime_year_t : public script_library_class::function
{
  public:
    // constructor required
    slf_systime_year_t(const char* n) : script_library_class::function(n) {}
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      vm_num_t result = g_game_ptr->get_systime().year;

      SLF_RETURN;
      SLF_DONE;
    }
};

////////////////////////////////////////////////////////////////////////////////
// global script library function: num systime_hour()
class slf_systime_hour_t : public script_library_class::function
{
  public:
    // constructor required
    slf_systime_hour_t(const char* n) : script_library_class::function(n) {}
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      vm_num_t result = g_game_ptr->get_systime().hour;

      SLF_RETURN;
      SLF_DONE;
    }
};

////////////////////////////////////////////////////////////////////////////////
// global script library function: num systime_minute()
class slf_systime_minute_t : public script_library_class::function
{
  public:
    // constructor required
    slf_systime_minute_t(const char* n) : script_library_class::function(n) {}
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      vm_num_t result = g_game_ptr->get_systime().minute;

      SLF_RETURN;
      SLF_DONE;
    }
};

////////////////////////////////////////////////////////////////////////////////
// global script library function: num systime_second()
class slf_systime_second_t : public script_library_class::function
{
  public:
    // constructor required
    slf_systime_second_t(const char* n) : script_library_class::function(n) {}
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      vm_num_t result = g_game_ptr->get_systime().second;

      SLF_RETURN;
      SLF_DONE;
    }
};



void register_global_lib()
{
  NEW slf_allow_suspend_thread_t("allow_suspend_thread(num)");
  NEW slf_set_camera_priority_t("set_camera_priority(num)");
  NEW slf_enable_marky_cam_t("enable_marky_cam(num)");
  NEW slf_disable_marky_cam_t("disable_marky_cam(num)");
  NEW slf_freeze_hero_t("freeze_hero(num)");
  NEW slf_set_zoom_t("set_zoom(num)");
  NEW slf_localize_thread_t("localize_thread(vector3d)");
  NEW slf_localize_thread_to_character_t("localize_thread_to_character(character)");
  NEW slf_localize_thread_region_t("localize_thread_region(str)");
  NEW slf_globalize_thread_t("globalize_thread()");
//H  NEW slf_enable_screen_saver_t("enable_screen_saver(num)");
  NEW slf_enable_pause_t("enable_pause(num)");
  NEW slf_enable_interface_t("enable_interface(num)");
  NEW slf_wait_animate_fog_color_t("wait_animate_fog_color(vector3d,num)");
  NEW slf_wait_animate_fog_distance_t("wait_animate_fog_distance(num,num)");
  NEW slf_wait_animate_fog_distances_t("wait_animate_fog_distances(num,num,num)");
  NEW slf_wait_frame_t("wait_frame()");
  NEW slf_delay_t("delay(num)");
  NEW slf_abs_delay_t("abs_delay(num)");
  NEW slf_dilated_delay_t("dilated_delay(num)");
  NEW slf_vo_delay_t("vo_delay(num,num,num,num)");
  NEW slf_area_damage_t("area_damage(vector3d,num,num)");
  NEW slf_post_message2_t("post_message(str,num)");
  NEW slf_post_message_t("post_message(num,num)");
  NEW slf_strcmp_t("strcmp(str,str)");
  NEW slf_random_t("random(num)");
  NEW slf_get_elevation_t("get_elevation(vector3d)");
  NEW slf_add_debug_sphere_t("add_debug_sphere(vector3d,num)");
  NEW slf_clear_debug_spheres_t("clear_debug_spheres()");
  NEW slf_get_time_inc_t("get_time_inc()");
  NEW slf_text_width_t("text_width(str)");
  NEW slf_is_action_button_down_t("is_action_button_down()");
  NEW slf_get_cur_time_inc_t("get_cur_time_inc()");
  NEW slf_TARGET_PC_t("TARGET_PC()");
//H  NEW slf_blackscreen_on_t("blackscreen_on()");
//H  NEW slf_blackscreen_off_t("blackscreen_off()");
  NEW slf_letterbox_on_t("letterbox_on()");
  NEW slf_letterbox_off_t("letterbox_off()");
  NEW slf_letterbox_cout_t("letterbox_cout(str)");
  NEW slf_fade_out_t("fade_out()");
  NEW slf_fade_in_t("fade_in()");
  NEW slf_set_region_active_t("set_region_active(str,num)");
  NEW slf_set_fog_table_gamma_t("set_fog_table_gamma(num)");
/*H
  NEW slf_game_segue_from_intro_t("game_segue_from_intro()");
  NEW slf_game_segue_from_demo_t("game_segue_from_demo()");
  NEW slf_game_segue_from_intro_parms_t("game_segue_from_intro_parms(num,num)");
H*/
  NEW slf_get_detail_level_t("get_detail_level()");
  NEW slf_was_start_pressed_t("was_start_pressed()");
  NEW slf_was_A_pressed_t("was_A_pressed()");
  NEW slf_was_B_pressed_t("was_B_pressed()");
//  NEW slf_vibrate_purupuru_t("vibrate_purupuru(num,num,num,num)");
//P  NEW slf_lock_spell_invocations_t("lock_spell_invocations()");
//P  NEW slf_unlock_spell_invocations_t("unlock_spell_invocations()");
//P  NEW slf_lock_utility_spell_invocations_t("lock_utility_spell_invocations()");
//P  NEW slf_unlock_utility_spell_invocations_t("unlock_utility_spell_invocations()");
//H  NEW slf_lock_hero_weapon_t("lock_hero_weapon()");
//H  NEW slf_unlock_hero_weapon_t("unlock_hero_weapon()");
//H  NEW slf_slew_game_time_t("slew_game_time(num)");
//H  NEW slf_normal_game_time_t("normal_game_time()");
  NEW slf_get_control_trigger_t("get_control_trigger(num)");
  NEW slf_los_check_t("los_check(vector3d,vector3d)");
  NEW slf_get_control_state_t("get_control_state(num)");
  NEW slf_wait_keypress_t("wait_keypress(str)");
/*H
  NEW slf_ingame_map_clear_list_t("ingame_map_clear_list()");
  NEW slf_ingame_map_show_item_t("ingame_map_show_item(str)");
  NEW slf_ingame_map_hide_item_t("ingame_map_hide_item(str)");
  NEW slf_ingame_map_remove_item_t("ingame_map_remove_item(str)");
  NEW slf_ingame_map_add_item_t("ingame_map_add_item(str,num,num,str,str,num,str)");
  NEW slf_ingame_map_add_item1_t("ingame_map_add_item(str,vector3d,str,str,num,str)");
H*/
  NEW slf_apply_radius_damage_t("apply_radius_damage(vector3d,num,num,num)");

  NEW slf_str_append_t("str_append(str,str,num)");
  NEW slf_to_str_vector3d_t("to_str(vector3d)");
  NEW slf_to_str_num_t("to_str(num)");
  NEW slf_to_str_int_t("to_int_str(num)");

  NEW slf_add_3d_debug_str_t("add_3d_debug_str(vector3d,vector3d,num,str)");
  NEW slf_add_2d_debug_str_t("add_2d_debug_str(vector3d,vector3d,num,str)");

  NEW slf_bsp_check_t("col_check(vector3d,vector3d,num)");
  NEW slf_entity_col_check_t("entity_col_check(entity,entity)");

  NEW slf_get_bsp_check_hit_pos_t("get_col_hit_pos()");
  NEW slf_get_bsp_check_hit_norm_t("get_col_hit_norm()");
  NEW slf_get_bsp_check_hit_ent_t("get_col_hit_ent()");

  NEW slf_get_global_time_dilation_t("get_global_time_dilation()");
  NEW slf_set_global_time_dilation_t("set_global_time_dilation(num)");
  NEW slf_wait_set_global_time_dilation_t("wait_set_global_time_dilation(num,num)");

  NEW slf_play_sound_vol_t("play_sound_vol(str,num)");
  NEW slf_load_level_t("load_level(str)");
  NEW slf_play_movie_t("play_movie(str,num)");

  NEW slf_set_game_info_num_t("set_game_info_num(str,num)");
  NEW slf_get_game_info_num_t("get_game_info_num(str)");

  NEW slf_set_game_info_str_t("set_game_info_str(str,str)");
  NEW slf_get_game_info_str_t("get_game_info_str(str)");

  NEW slf_freeze_universe_t("freeze_universe(num)");


  NEW slf_wait_fps_test_t("wait_fps_test(num,num,vector3d,vector3d)");

#if _CONSOLE_ENABLE
  NEW slf_add_to_console_t("add_to_console(str)");
  NEW slf_console_exec_t("console_exec(str)");
#endif  // _CONSOLE_ENABLE

  NEW slf_get_fog_color_t("get_fog_color()");
  NEW slf_get_fog_distance_t("get_fog_distance()");


  NEW slf_systime_month_t("systime_month()");
  NEW slf_systime_day_t("systime_day()");
  NEW slf_systime_day_of_week_t("systime_day_of_week()");
  NEW slf_systime_year_t("systime_year()");
  NEW slf_systime_hour_t("systime_hour()");
  NEW slf_systime_minute_t("systime_minute()");
  NEW slf_systime_second_t("systime_second()");

  NEW slf_suspend_all_ai_t("suspend_all_ai()");
  NEW slf_unsuspend_all_ai_t("unsuspend_all_ai()");
}

