// script_lib_sound_inst.cpp
//
// This file contains application-specific code and data that makes use of the
// Script Runtime Core to define script library classes and functions that
// provide the interface between the script language and the application.

#include "global.h"

#include "script_lib_sound_inst.h"
#include "hwaudio.h"
#include "vm_stack.h"
#include "vm_thread.h"
#include "game.h"
extern game *g_game_ptr;

#ifdef GCCULL


// script library function:  sound_instance::release()
class slf_sound_instance_release_t : public script_library_class::function
{
public:
  // constructor required
  slf_sound_instance_release_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_sound_instance_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    parms->me->release();
    SLF_DONE;
  }
};
//slf_sound_instance_release_t slf_sound_instance_release(slc_sound_instance,"release()");

// script library function:  sound_instance::play()
class slf_sound_instance_play_t : public script_library_class::function
{
public:
  // constructor required
  slf_sound_instance_play_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_sound_instance_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    parms->me->play(false);
    SLF_DONE;
  }
};
//slf_sound_instance_play_t slf_sound_instance_play(slc_sound_instance,"play()");

// script library function:  sound_instance::play_looped()
class slf_sound_instance_play_looped_t : public script_library_class::function
{
public:
  // constructor required
  slf_sound_instance_play_looped_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_sound_instance_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    parms->me->play(true);
    SLF_DONE;
  }
};


// script library function:  sound_instance::queue()
class slf_sound_instance_queue_t : public script_library_class::function
{
public:
  // constructor required
  slf_sound_instance_queue_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_sound_instance_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    parms->me->preload();
    SLF_DONE;
  }
};


// script library function:  sound_instance::stop()
class slf_sound_instance_stop_t : public script_library_class::function
{
public:
  // constructor required
  slf_sound_instance_stop_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_sound_instance_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    parms->me->stop();
    SLF_DONE;
  }
};
//slf_sound_instance_stop_t slf_sound_instance_stop(slc_sound_instance,"stop()");

// script library function:  sound_instance::rewind()
class slf_sound_instance_rewind_t : public script_library_class::function
{
public:
  // constructor required
  slf_sound_instance_rewind_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_sound_instance_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    parms->me->rewind();
    SLF_DONE;
  }
};
//slf_sound_instance_rewind_t slf_sound_instance_rewind(slc_sound_instance,"rewind()");

// script library function:  num sound_instance::is_playing()
class slf_sound_instance_is_playing_t : public script_library_class::function
{
public:
  // constructor required
  slf_sound_instance_is_playing_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_sound_instance_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_num_t result = parms->me->is_playing();
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_sound_instance_is_playing_t slf_sound_instance_is_playing(slc_sound_instance,"is_playing()");

// script library function:  sound_instance::set_pos(num)
class slf_sound_instance_set_pos_t : public script_library_class::function
{
public:
  // constructor required
  slf_sound_instance_set_pos_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_sound_instance_t me;
    vm_num_t new_pos;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    parms->me->set_pos( parms->new_pos );
    SLF_DONE;
  }
};
//slf_sound_instance_set_pos_t slf_sound_instance_set_pos(slc_sound_instance,"set_pos(num)");

// script library function:  num sound_instance::get_pos()
class slf_sound_instance_get_pos_t : public script_library_class::function
{
public:
  // constructor required
  slf_sound_instance_get_pos_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_sound_instance_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_num_t result = parms->me->get_pos();
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_sound_instance_get_pos_t slf_sound_instance_get_pos(slc_sound_instance,"get_pos()");

// script library function:  num sound_instance::get_scaled_pos()
class slf_sound_instance_get_scaled_pos_t : public script_library_class::function
{
public:
  // constructor required
  slf_sound_instance_get_scaled_pos_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_sound_instance_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_num_t result = parms->me->get_scaled_pos();
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_sound_instance_get_scaled_pos_t slf_sound_instance_get_scaled_pos(slc_sound_instance,"get_scaled_pos()");

// script library function:  sound_instance::set_volume(num volume)
class slf_sound_instance_set_volume_t : public script_library_class::function
{
public:
  // constructor required
  slf_sound_instance_set_volume_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_sound_instance_t me;
    vm_num_t volume;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if (parms->volume < 0.0f)
    {
      warning("Bad scripter!  set_volume call has crazy value %g (needs to be in 0..1)\n", parms->volume);
      parms->volume = 0.0f;
    }
    if (parms->volume > 1.0f)
    {
      warning("Bad scripter!  set_volume call has crazy value %g (needs to be in 0..1)\n", parms->volume);
      parms->volume = 1.0f;
    }
    parms->me->set_volume(parms->volume);
    SLF_DONE;
  }
};
//slf_sound_instance_set_volume_t slf_sound_instance_set_volume(slc_sound_instance,"set_volume(num)");

// script library function:  num sound_instance::get_volume()
class slf_sound_instance_get_volume_t : public script_library_class::function
{
public:
  // constructor required
  slf_sound_instance_get_volume_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_sound_instance_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_num_t result = parms->me->get_volume();
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_sound_instance_get_volume_t slf_sound_instance_get_volume(slc_sound_instance,"get_volume()");


/////////////////////////////////// sound_instance::set_ranges(num,num) /////////////////
// script library function
class slf_sound_instance_set_ranges_t : public script_library_class::function
{
public:
  // constructor required
  slf_sound_instance_set_ranges_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_sound_instance_t me;
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




///////////////////////////////////////////////////////////////////////////////
// Global sound functions
///////////////////////////////////////////////////////////////////////////////

class slf_sound_instance_dampen_guard_t : public script_library_class::function
{
public:
  // constructor required
  slf_sound_instance_dampen_guard_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  struct parms_t
  {
    // parameters
		vm_sound_instance_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
		parms->me->dampen_guard();
    SLF_DONE;
  }

};


class slf_sound_instance_dampen_t : public script_library_class::function
{
public:
  // constructor required
  slf_sound_instance_dampen_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  struct parms_t
  {
    // parameters
		vm_sound_instance_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
		parms->me->dampen();
    SLF_DONE;
  }

};

class slf_sound_instance_undampen_t : public script_library_class::function
{
public:
  // constructor required
  slf_sound_instance_undampen_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  struct parms_t
  {
    // parameters
		vm_sound_instance_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
		parms->me->undampen();
    SLF_DONE;
  }

};

class slf_dampen_all_t : public script_library_class::function
{
public:
  // constructor required
  slf_dampen_all_t(const char* n) : script_library_class::function(n) {}
  struct parms_t
  {
    // parameters
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    sound_device::inst()->dampen_all_instances();
    SLF_DONE;
  }

};
class slf_undampen_all_t : public script_library_class::function
{
public:
  // constructor required
  slf_undampen_all_t(const char* n) : script_library_class::function(n) {}
  struct parms_t
  {
    // parameters
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    sound_device::inst()->undampen_all_instances();
    SLF_DONE;
  }

};
class slf_set_dampen_value_t : public script_library_class::function
{
public:
  // constructor required
  slf_set_dampen_value_t(const char* n) : script_library_class::function(n) {}
  struct parms_t
  {
    // parameters
		vm_num_t n;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    sound_device::inst()->set_dampen_value(parms->n);
    SLF_DONE;
  }

};

// global script library function:  num load_sound(str)
class slf_load_sound_t : public script_library_class::function
{
public:
  // constructor required
  slf_load_sound_t(const char* n) : script_library_class::function(n) {}
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
    vm_num_t result = sound_device::inst()->load_sound(*parms->n);
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_load_sound_t slf_load_sound("load_sound(str)");

// global script library function:  play_sound(str)
class slf_play_sound_t : public script_library_class::function
{
public:
  // constructor required
  slf_play_sound_t(const char* n) : script_library_class::function(n) {}
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
    sound_device::inst()->play_sound(*parms->n);
    SLF_DONE;
  }
};

////////////////////////// play_3d_sound(str,vector3d,num,num,num,num) ///////////////////
// global script library function
class slf_play_3d_sound_t : public script_library_class::function
{
public:
  // constructor required
  slf_play_3d_sound_t(const char* n) : script_library_class::function(n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_str_t name;
    vector3d position;
    vm_num_t volume;
    vm_num_t pitch;
    vm_num_t min;
    vm_num_t max;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    sound_device::inst()->play_3d_sound( *parms->name, parms->position,
      parms->volume, parms->pitch, parms->min, parms->max );
    SLF_DONE;
  }
};

/////////////////////////////////// sound_instance create_sound(str) ////////////////////
// global script library function
class slf_create_sound_t : public script_library_class::function
{
public:
  // constructor required
  slf_create_sound_t(const char* n) : script_library_class::function(n) {}
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
    vm_sound_instance_t result = sound_device::inst()->create_sound(*parms->n);
    SLF_RETURN;
    SLF_DONE;
  }
};


/////////////////////////////////// play_music( str ) ///////////////////////////////////
// global script library function
class slf_play_music_t : public script_library_class::function
{
public:
  // constructor required
  slf_play_music_t(const char* n) : script_library_class::function(n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_str_t music_name;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    filespec spec(parms->music_name->c_str());
    pstring pmusic_name(spec.name.c_str());
    g_game_ptr->play_music(pmusic_name, false);
    SLF_DONE;
  }
};


/////////////////////////////////// play_music_now( str ) ///////////////////////////////
// global script library function
class slf_play_music_now_t : public script_library_class::function
{
public:
  // constructor required
  slf_play_music_now_t(const char* n) : script_library_class::function(n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_str_t music_name;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    filespec spec(parms->music_name->c_str());
    pstring pmusic_name(spec.name.c_str());
    g_game_ptr->play_music(pmusic_name, true);
    SLF_DONE;
  }
};


/////////////////////////////////// stop_music() ////////////////////////////////////////
// global script library function
class slf_stop_music_t : public script_library_class::function
{
public:
  // constructor required
  slf_stop_music_t(const char* n) : script_library_class::function(n) {}

  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    g_game_ptr->stop_music();
    SLF_DONE;
  }
};


/////////////////////////////////// stop_all_sounds() ////////////////////////////////////////
// global script library function
class slf_stop_all_sounds_t : public script_library_class::function
{
public:
  // constructor required
  slf_stop_all_sounds_t(const char* n) : script_library_class::function(n) {}

  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    sound_device::inst()->stop_all_instances();
    SLF_DONE;
  }
};


/////////////////////////////////// set_master_volume( num ) ///////////////////////////////////
// global script library function
class slf_set_master_volume_t : public script_library_class::function
{
public:
  // constructor required
  slf_set_master_volume_t(const char* n) : script_library_class::function(n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_num_t new_volume;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    assert(parms->new_volume >= 0.0f && parms->new_volume <= 1.0f && "Bad scripter!  Volume must be in range 0..1");
    sound_device::inst()->set_master_volume(parms->new_volume);
    SLF_DONE;
  }
};


/////////////////////////////////// num is_music_playing() //////////////////////////////
// global script library function
class slf_is_music_playing_t : public script_library_class::function
{
public:
  // constructor required
  slf_is_music_playing_t(const char* n) : script_library_class::function(n) {}

  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    vm_num_t result = g_game_ptr->is_music_playing();
    SLF_RETURN;
    SLF_DONE;
  }
};


/////////////////////////////////// num is_music_queued() ///////////////////////////////
// global script library function
class slf_is_music_queued_t : public script_library_class::function
{
public:
  // constructor required
  slf_is_music_queued_t(const char* n) : script_library_class::function(n) {}

  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    vm_num_t result = g_game_ptr->is_music_queued();
    SLF_RETURN;
    SLF_DONE;
  }
};




void register_sound_inst_lib()
{
  // pointer to single instance of library class
  slc_sound_instance_t* slc_sound_instance = NEW slc_sound_instance_t("sound_instance",4);

  NEW slf_sound_instance_release_t(slc_sound_instance,"release()");
  NEW slf_sound_instance_play_t(slc_sound_instance,"play()");
  NEW slf_sound_instance_play_looped_t(slc_sound_instance,"play_looped()");
  NEW slf_sound_instance_queue_t(slc_sound_instance,"queue()");
  NEW slf_sound_instance_stop_t(slc_sound_instance,"stop()");
  NEW slf_sound_instance_rewind_t(slc_sound_instance,"rewind()");
  NEW slf_sound_instance_is_playing_t(slc_sound_instance,"is_playing()");
  NEW slf_sound_instance_set_pos_t(slc_sound_instance,"set_pos(num)");
  NEW slf_sound_instance_get_pos_t(slc_sound_instance,"get_pos()");
  NEW slf_sound_instance_get_scaled_pos_t(slc_sound_instance,"get_scaled_pos()");
  NEW slf_sound_instance_set_volume_t(slc_sound_instance,"set_volume(num)");
  NEW slf_sound_instance_get_volume_t(slc_sound_instance,"get_volume()");
  NEW slf_sound_instance_set_ranges_t(slc_sound_instance,"set_ranges(num,num)");
  NEW slf_load_sound_t("load_sound(str)");
  NEW slf_play_sound_t("play_sound(str)");
  NEW slf_play_3d_sound_t("play_3d_sound(str,vector3d,num,num,num,num)");
  NEW slf_create_sound_t("create_sound(str)");
  NEW slf_play_music_t("play_music(str)");
  NEW slf_play_music_now_t("play_music_now(str)");
  NEW slf_stop_music_t("stop_music()");
  NEW slf_stop_all_sounds_t("stop_all_sounds()");
  NEW slf_set_master_volume_t("set_master_volume(num)");
  NEW slf_is_music_playing_t("is_music_playing()");
  NEW slf_is_music_queued_t("is_music_queued()");

	NEW slf_undampen_all_t("undampen_all_instances()");
	NEW slf_dampen_all_t("dampen_all_instances()");
	NEW slf_set_dampen_value_t("set_dampen_value(num)");

	NEW slf_sound_instance_undampen_t(slc_sound_instance,"undampen()");
	NEW slf_sound_instance_dampen_t(slc_sound_instance,"dampen()");
  NEW slf_sound_instance_dampen_guard_t(slc_sound_instance,"dampen_guard()");
}

#endif
