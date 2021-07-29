// script_lib_character2.cpp
//
// This file contains stuff that wouldn't fit into script_lib_character.cpp
// due to Metrowerks out-of-memory error during the compile phase.

#include "global.h"

#include "script_lib_signal.h"
//!#include "script_lib_character.h"
#include "script_lib_entity.h"
#include "script_lib_item.h"
#include "script_lib_vector3d.h"
//!#include "character.h"
#include "vm_stack.h"
#include "vm_thread.h"
//#include "brain.h"
#include "algebra.h"
#include "inputmgr.h"
//!#include "actor.h"
#include "commands.h"
#include "chunkfile.h"
#include "game.h"
//!#include "char_group.h"
#include "wds.h"
#include "oserrmsg.h"
//!#include "attrib.h"
//!#include "limb.h"
#include "path.h"


// script library function:  character::set_scripted_death()
class slf_character_set_scripted_death_t : public script_library_class::function
{
public:
  // constructor required
  slf_character_set_scripted_death_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_character_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    parms->me->set_scripted_death();
    SLF_DONE;
  }
};
slf_character_set_scripted_death_t slf_character_set_scripted_death(slc_character,"set_scripted_death()");


// script library function:  character::enable_radio()
class slf_character_enable_radio_t : public script_library_class::function
{
public:
  // constructor required
  slf_character_enable_radio_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_character_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    assert(!parms->me->is_hero() && parms->me->get_brain() != NULL);
    parms->me->get_brain()->enable_radio();
    SLF_DONE;
  }
};
slf_character_enable_radio_t slf_character_enable_radio(slc_character,"enable_radio()");

// script library function:  character::disable_radio()
class slf_character_disable_radio_t : public script_library_class::function
{
public:
  // constructor required
  slf_character_disable_radio_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_character_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    assert(!parms->me->is_hero() && parms->me->get_brain() != NULL);
    parms->me->get_brain()->disable_radio();
    SLF_DONE;
  }
};
slf_character_disable_radio_t slf_character_disable_radio(slc_character,"disable_radio()");

// script library function: num character::is_radio_enabled()
class slf_character_is_radio_enabled_t: public script_library_class::function
{
public:
  // constructor required
  slf_character_is_radio_enabled_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_character_t me;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;
    assert(!parms->me->is_hero() && parms->me->get_brain() != NULL);
    vm_num_t result = parms->me->get_brain()->radio_enabled();
    SLF_RETURN;
    SLF_DONE;
  }
};
slf_character_is_radio_enabled_t slf_character_is_radio_enabled(slc_character, "is_radio_enabled()");








// script library function:  char_group::connect_to_dread_net()
class slf_char_group_connect_to_dread_net_t : public script_library_class::function
{
public:
  // constructor required
  slf_char_group_connect_to_dread_net_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_char_group_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vector<character *>::iterator i = parms->me->begin();
    while(i != parms->me->end())
    {
      assert(!(*i)->is_hero() && (*i)->get_brain() != NULL);
      (*i)->get_brain()->connect();
    }
    SLF_DONE;
  }
};
slf_char_group_connect_to_dread_net_t slf_char_group_connect_to_dread_net(slc_char_group,"connect_to_dread_net()");

// script library function:  char_group::disconnect_from_dread_net()
class slf_char_group_disconnect_from_dread_net_t : public script_library_class::function
{
public:
  // constructor required
  slf_char_group_disconnect_from_dread_net_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_char_group_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vector<character *>::iterator i = parms->me->begin();
    while(i != parms->me->end())
    {
      assert(!(*i)->is_hero() && (*i)->get_brain() != NULL);
      (*i)->get_brain()->disconnect();
    }
    SLF_DONE;
  }
};
slf_char_group_disconnect_from_dread_net_t slf_char_group_disconnect_from_dread_net(slc_char_group,"disconnect_from_dread_net()");

// script library function: num char_group::num_connected_to_dread_net()
class slf_char_group_num_connected_to_dread_net_t: public script_library_class::function
{
public:
  // constructor required
  slf_char_group_num_connected_to_dread_net_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_char_group_t me;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;
    vm_num_t result = 0;

    vector<character *>::iterator i = parms->me->begin();
    while(i != parms->me->end())
    {
      assert(!(*i)->is_hero() && (*i)->get_brain() != NULL);
      if((*i)->get_brain()->is_connected())
        result += 1;
    }

    SLF_RETURN;
    SLF_DONE;
  }
};
slf_char_group_num_connected_to_dread_net_t slf_char_group_num_connected_to_dread_net(slc_char_group, "num_connected_to_dread_net()");

// script library function:  char_group::enable_radio()
class slf_char_group_enable_radio_t : public script_library_class::function
{
public:
  // constructor required
  slf_char_group_enable_radio_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_char_group_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vector<character *>::iterator i = parms->me->begin();
    while(i != parms->me->end())
    {
      assert(!(*i)->is_hero() && (*i)->get_brain() != NULL);
      (*i)->get_brain()->enable_radio();
    }
    SLF_DONE;
  }
};
slf_char_group_enable_radio_t slf_char_group_enable_radio(slc_char_group,"enable_radio()");

// script library function:  char_group::disable_radio()
class slf_char_group_disable_radio_t : public script_library_class::function
{
public:
  // constructor required
  slf_char_group_disable_radio_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_char_group_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vector<character *>::iterator i = parms->me->begin();
    while(i != parms->me->end())
    {
      assert(!(*i)->is_hero() && (*i)->get_brain() != NULL);
      (*i)->get_brain()->disable_radio();
    }
    SLF_DONE;
  }
};
slf_char_group_disable_radio_t slf_char_group_disable_radio(slc_char_group,"disable_radio()");

// script library function:  char_group::link_to_dread_net_signaller(signaller sig);
class slf_char_group_link_to_dread_net_signaller_t : public script_library_class::function
{
  public:
    // constructor required
    slf_char_group_link_to_dread_net_signaller_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_char_group_t me;
      vm_signaller_t sig;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_PARMS;
      g_world_ptr->get_dread_net()->link(parms->sig, parms->me);
      SLF_DONE;
    }
};
slf_char_group_link_to_dread_net_signaller_t slf_char_group_link_to_dread_net_signaller(slc_char_group,"link_to_dread_net_signaller(signaller)");

// script library function:  char_group::unlink_from_dread_net_signaller(signaller sig);
class slf_char_group_unlink_from_dread_net_signaller_t : public script_library_class::function
{
  public:
    // constructor required
    slf_char_group_unlink_from_dread_net_signaller_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_char_group_t me;
      vm_signaller_t sig;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_PARMS;
      g_world_ptr->get_dread_net()->unlink(parms->sig, parms->me);
      SLF_DONE;
    }
};
slf_char_group_unlink_from_dread_net_signaller_t slf_char_group_unlink_from_dread_net_signaller(slc_char_group,"unlink_from_dread_net_signaller(signaller)");

