// script_lib_scene_anim.cpp
//
// This file contains application-specific code and data that makes use of the
// Script Runtime Core to define script library classes and functions that
// provide the interface between the script language and the application.

#include "global.h"

#include "script_lib_scene_anim.h"
#include "vm_stack.h"
#include "vm_thread.h"
#include "wds.h"


///////////////////////////////////////////////////////////////////////////////
// script library class: scene_anim
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// scene_anim script library class member functions
///////////////////////////////////////////////////////////////////////////////

// script library function:  scene_anim::kill_anim();
class slf_scene_anim_kill_anim_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_scene_anim_kill_anim_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
      {
      }
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_scene_anim_t me;
      };
    // library function execution
    virtual bool operator()( vm_stack& stack, entry_t entry )
      {
      SLF_PARMS;
      g_world_ptr->kill_scene_anim( parms->me );
      SLF_DONE;
      }
  };
//slf_scene_anim_kill_anim_t slf_scene_anim_kill_anim(slc_scene_anim,"kill_anim()");


///////////////////////////////////////////////////////////////////////////////
// Global script functions supporting scene_anim class
///////////////////////////////////////////////////////////////////////////////

// script library function:  load_scene_anim( str name )
class slf_load_scene_anim_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_load_scene_anim_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_str_t name;
      };
    // library function execution
    virtual bool operator()( vm_stack& stack,entry_t entry )
      {
      SLF_PARMS;
      g_world_ptr->load_scene_anim( *parms->name );
      SLF_DONE;
      }
  };
//slf_load_scene_anim_t slf_load_scene_anim("load_scene_anim(str)");

// script library function:  scene_anim play_scene_anim( str name )
class slf_play_scene_anim_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_play_scene_anim_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_str_t name;
      };
    // library function execution
    virtual bool operator()( vm_stack& stack,entry_t entry )
      {
      SLF_PARMS;
      vm_scene_anim_t result = g_world_ptr->play_scene_anim( *parms->name );
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_play_scene_anim_t slf_play_scene_anim("play_scene_anim(str)");


void register_scene_anim_lib()
{
  // pointer to single instance of library class
  slc_scene_anim_t* slc_scene_anim = NEW slc_scene_anim_t("scene_anim",4);

  NEW slf_scene_anim_kill_anim_t(slc_scene_anim,"kill_anim()");
  NEW slf_load_scene_anim_t("load_scene_anim(str)");
  NEW slf_play_scene_anim_t("play_scene_anim(str)");
}
