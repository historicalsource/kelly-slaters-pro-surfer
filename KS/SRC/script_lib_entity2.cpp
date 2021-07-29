// script_lib_entity2.cpp
//
// This file contains application-specific code and data that makes use of the
// Script Runtime Core to define script library classes and functions that
// provide the interface between the script language and the application.

#include "global.h"

#include "script_lib_signal.h"
#include "script_lib_entity.h"
#include "entity.h"
#include "vm_stack.h"
#include "vm_thread.h"
#include "wds.h"
//!#include "character.h"
//#include "brain.h"
#include "script_lib_sound_inst.h"
#include "script_lib_sound_stream.h"
//!#include "script_lib_character.h"
#include "script_lib_item.h"
//#include "fxman.h"
#include "app.h"
#include "game.h"
#include "entityflags.h"
// BIGCULL #include "spiderman_common.h"
#include "ai_interface.h"
#include "time_interface.h"
#include "physical_interface.h"
#include "item.h"
#include "pstring.h"
#include "lightmgr.h"
#ifdef GCCULL
#include "ai_voice.h"


class slf_entity_wait_looping_anim_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_wait_looping_anim_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}

  // library function persistent local vm_stack data;
  // stored on stack (after parms) until function completed;
  // accessed by SLF_SDATA at start of function
  struct sdata_t
  {
    entity_anim_tree* anim;
  };

  // library function parameters;
  // accessed by SLF_PARMS
  struct parms_t
  {
    vm_entity_t me;
    vm_str_t anim;
    vm_num_t loop_count;
    vm_num_t reverse;
  };

  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_SDATA;

    SLF_PARMS;

    if (entry == FIRST_ENTRY)
    {
      sdata->anim = parms->me->play_loop_anim( *parms->anim, ANIM_LOOPING|(parms->reverse?ANIM_REVERSE:0), parms->loop_count-1 );
      SLF_RECALL;
    }
    else
    {
      if(sdata->anim->is_finished())
      {
        g_world_ptr->kill_anim(sdata->anim);
        SLF_DONE;
      }
      else
      {
        SLF_RECALL;
      }
    }
  }
};
//slf_entity_wait_looping_anim_t slf_entity_wait_looping_anim(slc_entity, "wait_looping_anim(str,num,num)");



// script library function:  entity::set_alternative_materials( str )
class slf_entity_set_alternative_materials_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_alternative_materials_t( script_library_class* slc, const char* n ) : script_library_class::function( slc, n ) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_str_t n;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    parms->me->set_alternative_materials( stringx(*parms->n) );
    SLF_DONE;
  }
};
//slf_entity_set_alternative_materials_t slf_entity_set_alternative_materials( slc_entity, "set_alternative_materials(str)" );


// script library function:  entity::use_item( str )
class slf_entity_use_item_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_use_item_t( script_library_class* slc, const char* n ) : script_library_class::function( slc, n ) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_str_t item_name;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    stringx item_name = *parms->item_name;
    item_name.to_upper();
    item* it = parms->me->find_item_by_name( item_name );
    parms->me->use_item( it );
    // we must ensure that the item will frame_advance() even when ineligible by the usual rules
    if(it->is_a_gun())
      g_world_ptr->guarantee_active( it );
    SLF_DONE;
  }
};
//slf_entity_use_item_t slf_entity_use_item( slc_entity, "use_item(str)" );



// script library function:  entity::set_current_target( entity )
class slf_entity_set_current_target_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_current_target_t( script_library_class* slc, const char* n ) : script_library_class::function( slc, n ) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_entity_t target;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    parms->me->set_current_target( parms->target );
    SLF_DONE;
  }
};
//slf_entity_set_current_target_t slf_entity_set_current_target( slc_entity, "set_current_target(entity)" );


// script library function:  entity::set_current_target_pos( vector3d )
class slf_entity_set_current_target_pos_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_current_target_pos_t( script_library_class* slc, const char* n ) : script_library_class::function( slc, n ) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vector3d target;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    parms->me->set_current_target_pos( parms->target );
    SLF_DONE;
  }
};
//slf_entity_set_current_target_pos_t slf_entity_set_current_target_pos( slc_entity, "set_current_target_pos(vector3d)" );



// script library function:  entity::set_current_target_norm( vector3d )
class slf_entity_set_current_target_norm_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_current_target_norm_t( script_library_class* slc, const char* n ) : script_library_class::function( slc, n ) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vector3d norm;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
    vector3d norm = parms->norm;
    norm.normalize();
    parms->me->set_current_target_norm( norm );
    SLF_DONE;
  }
};
//slf_entity_set_current_target_norm_t slf_entity_set_current_target_norm( slc_entity, "set_current_target_norm(vector3d)" );




// script library function:  entity entity::get_current_target()
class slf_get_current_target_t : public script_library_class::function
{
public:
  // constructor required
  slf_get_current_target_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_entity_t result = parms->me->get_current_target();
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_get_current_target_t slf_get_current_target(slc_entity, "get_current_target()");

// script library function:  vector3d entity::get_current_target_pos()
class slf_get_current_target_pos_t : public script_library_class::function
{
public:
  // constructor required
  slf_get_current_target_pos_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vector3d result = parms->me->get_current_target_pos();
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_get_current_target_pos_t slf_get_current_target_pos(slc_entity, "get_current_target_pos()");

// script library function:  vector3d entity::get_current_target_norm()
class slf_get_current_target_norm_t : public script_library_class::function
{
public:
  // constructor required
  slf_get_current_target_norm_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vector3d result = parms->me->get_current_target_norm();
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_get_current_target_norm_t slf_get_current_target_norm(slc_entity, "get_current_target_norm()");

// script library function:  item entity::get_last_item_used();
class slf_entity_get_last_item_used_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_entity_get_last_item_used_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_entity_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_entity_t result = parms->me->get_last_item_used();
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_entity_get_last_item_used_t slf_entity_get_last_item_used(slc_entity,"get_last_item_used()");


/*
// script library function:  entity::connect_to_dread_net()
class slf_entity_connect_to_dread_net_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_connect_to_dread_net_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    parms->me->get_brain()->connect();

    SLF_DONE;
  }
};
//slf_entity_connect_to_dread_net_t slf_entity_connect_to_dread_net(slc_entity,"connect_to_dread_net()");

// script library function:  entity::disconnect_from_dread_net()
class slf_entity_disconnect_from_dread_net_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_disconnect_from_dread_net_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    parms->me->get_brain()->disconnect();

    SLF_DONE;
  }
};
//slf_entity_disconnect_from_dread_net_t slf_entity_disconnect_from_dread_net(slc_entity,"disconnect_from_dread_net()");

// script library function: num entity::is_connected_to_dread_net()
class slf_entity_is_connected_to_dread_net_t: public script_library_class::function
{
public:
  // constructor required
  slf_entity_is_connected_to_dread_net_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    vm_num_t result = parms->me->get_brain()->is_connected();

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_entity_is_connected_to_dread_net_t slf_entity_is_connected_to_dread_net(slc_entity, "is_connected_to_dread_net()");


// script library function: num entity::see_hero()
class slf_entity_see_hero_t: public script_library_class::function {
public:
  // constructor required
  slf_entity_see_hero_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    vm_num_t result = parms->me->get_brain()->is_hero_acquired();

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_entity_see_hero_t slf_entity_see_hero(slc_entity, "see_hero()");


// script library function: num entity::ignore_next_cue()
class slf_entity_ignore_next_cue_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_ignore_next_cue_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    parms->me->get_brain()->ignore_next_cue();

    SLF_DONE;
  }
};
//slf_entity_ignore_next_cue_t slf_entity_ignore_next_cue(slc_entity, "ignore_next_cue()");




// script library function: num entity::get_current_cue_type()
class slf_entity_get_current_cue_type_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_get_current_cue_type_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    vm_str_t result = &dread_net::get_cue_name(parms->me->get_brain()->get_current_cue_type());

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_entity_get_current_cue_type_t slf_entity_get_current_cue_type(slc_entity, "get_current_cue_type()");

// script library function: num entity::get_next_cue_type()
class slf_entity_get_next_cue_type_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_get_next_cue_type_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    vm_str_t result = &dread_net::get_cue_name(parms->me->get_brain()->get_next_cue_type());

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_entity_get_next_cue_type_t slf_entity_get_next_cue_type(slc_entity, "get_next_cue_type()");

// script library function: num entity::get_current_cue_power()
class slf_entity_get_current_cue_power_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_get_current_cue_power_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    vm_num_t result = parms->me->get_brain()->get_current_cue_power();

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_entity_get_current_cue_power_t slf_entity_get_current_cue_power(slc_entity, "get_current_cue_power()");

// script library function: num entity::get_next_cue_power()
class slf_entity_get_next_cue_power_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_get_next_cue_power_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    vm_num_t result = parms->me->get_brain()->get_next_cue_power();

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_entity_get_next_cue_power_t slf_entity_get_next_cue_power(slc_entity, "get_next_cue_power()");



// script library function: num entity::get_current_cue_pos()
class slf_entity_get_current_cue_pos_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_get_current_cue_pos_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    vector3d result = parms->me->get_brain()->get_current_cue_pos();

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_entity_get_current_cue_pos_t slf_entity_get_current_cue_pos(slc_entity, "get_current_cue_pos()");


// script library function: num entity::get_next_cue_pos()
class slf_entity_get_next_cue_pos_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_get_next_cue_pos_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    vector3d result = parms->me->get_brain()->get_next_cue_pos();

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_entity_get_next_cue_pos_t slf_entity_get_next_cue_pos(slc_entity, "get_next_cue_pos()");


// script library function: num entity::get_current_cue_reached()
class slf_entity_get_current_cue_reached_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_get_current_cue_reached_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    vm_num_t result = parms->me->get_brain()->get_current_cue_reached();

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_entity_get_current_cue_reached_t slf_entity_get_current_cue_reached(slc_entity, "get_current_cue_reached()");





// script library function: entity::set_los_defaults()
class slf_entity_set_los_defaults_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_los_defaults_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    parms->me->get_brain()->set_los_defaults();

    SLF_DONE;
  }
};
//slf_entity_set_los_defaults_t slf_entity_set_los_defaults(slc_entity, "set_los_defaults()");


// script library function: entity::set_cue_react_radius_defaults()
class slf_entity_set_cue_react_radius_defaults_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_cue_react_radius_defaults_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    parms->me->get_brain()->set_cue_react_radius_defaults();

    SLF_DONE;
  }
};
//slf_entity_set_cue_react_radius_defaults_t slf_entity_set_cue_react_radius_defaults(slc_entity, "set_cue_react_radius_defaults()");

// script library function: entity::set_min_cue_react_power_defaults()
class slf_entity_set_min_cue_react_power_defaults_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_min_cue_react_power_defaults_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    parms->me->get_brain()->set_min_cue_react_power_defaults();

    SLF_DONE;
  }
};
//slf_entity_set_min_cue_react_power_defaults_t slf_entity_set_min_cue_react_power_defaults(slc_entity, "set_min_cue_react_power_defaults()");



// script library function: entity::set_los_params()
class slf_entity_set_los_params_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_los_params_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_num_t state;
    vm_num_t see_all;
    vm_num_t see_fov_dist;
    vm_num_t see_fov_arc;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    int st = parms->state;
    if(st > BRAIN_REACT_COMBAT)
      st = BRAIN_REACT_COMBAT;
    if(st < BRAIN_REACT_IDLE)
      st = BRAIN_REACT_IDLE;


    parms->me->get_brain()->set_los_params((eReactionState)st, parms->see_all, parms->see_fov_dist, parms->see_fov_arc);

    SLF_DONE;
  }
};
//slf_entity_set_los_params_t slf_entity_set_los_params(slc_entity, "set_los_params(num,num,num,num)");


// script library function: entity::set_cue_react_radius()
class slf_entity_set_cue_react_radius_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_cue_react_radius_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_num_t state;
    vm_num_t rad;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    int st = parms->state;
    if(st > BRAIN_REACT_COMBAT)
      st = BRAIN_REACT_COMBAT;
    if(st < BRAIN_REACT_IDLE)
      st = BRAIN_REACT_IDLE;


    parms->me->get_brain()->set_cue_react_radius((eReactionState)st, parms->rad);

    SLF_DONE;
  }
};
//slf_entity_set_cue_react_radius_t slf_entity_set_cue_react_radius(slc_entity, "set_cue_react_radius(num,num)");

// script library function: entity::set_min_cue_react_power()
class slf_entity_set_min_cue_react_power_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_min_cue_react_power_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_num_t state;
    vm_num_t min;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    int st = parms->state;
    if(st > BRAIN_REACT_COMBAT)
      st = BRAIN_REACT_COMBAT;
    if(st < BRAIN_REACT_IDLE)
      st = BRAIN_REACT_IDLE;


    parms->me->get_brain()->set_min_cue_react_power((eReactionState)st, parms->min);

    SLF_DONE;
  }
};
//slf_entity_set_min_cue_react_power_t slf_entity_set_min_cue_react_power(slc_entity, "set_min_cue_react_power(num,num)");

// script library function: num entity::get_reaction_state()
class slf_entity_get_reaction_state_t: public script_library_class::function
{
public:
  // constructor required
  slf_entity_get_reaction_state_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    vm_num_t result = parms->me->get_brain()->get_reaction_state();

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_entity_get_reaction_state_t slf_entity_get_reaction_state(slc_entity, "get_reaction_state()");


// script library function: num entity::get_min_reaction_state()
class slf_entity_get_min_reaction_state_t: public script_library_class::function
{
public:
  // constructor required
  slf_entity_get_min_reaction_state_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    vm_num_t result = parms->me->get_brain()->get_min_reaction_state();

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_entity_get_min_reaction_state_t slf_entity_get_min_reaction_state(slc_entity, "get_min_reaction_state()");


// script library function: num entity::get_max_reaction_state()
class slf_entity_get_max_reaction_state_t: public script_library_class::function
{
public:
  // constructor required
  slf_entity_get_max_reaction_state_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    vm_num_t result = parms->me->get_brain()->get_max_reaction_state();

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_entity_get_max_reaction_state_t slf_entity_get_max_reaction_state(slc_entity, "get_max_reaction_state()");


// script library function: entity::set_reaction_state()
class slf_entity_set_reaction_state_t: public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_reaction_state_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_num_t state;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    parms->me->get_brain()->set_reaction_state_without_brainstem( (eReactionState)(int)parms->state );

    SLF_DONE;
  }
};
//slf_entity_set_reaction_state_t slf_entity_set_reaction_state(slc_entity, "set_reaction_state(num)");

// script library function: entity::set_min_reaction_state()
class slf_entity_set_min_reaction_state_t: public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_min_reaction_state_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_num_t state;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    int st = parms->state;
    if(st > BRAIN_REACT_COMBAT)
      st = BRAIN_REACT_COMBAT;
    if(st < BRAIN_REACT_IDLE)
      st = BRAIN_REACT_IDLE;

//    parms->me->get_brain()->set_min_reaction_state((eReactionState)st);

    SLF_DONE;
  }
};
//slf_entity_set_min_reaction_state_t slf_entity_set_min_reaction_state(slc_entity, "set_min_reaction_state(num)");

// script library function: entity::set_max_reaction_state()
class slf_entity_set_max_reaction_state_t: public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_max_reaction_state_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_num_t state;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    int st = parms->state;
    if(st > BRAIN_REACT_COMBAT)
      st = BRAIN_REACT_COMBAT;
    if(st < BRAIN_REACT_IDLE)
      st = BRAIN_REACT_IDLE;

//    parms->me->get_brain()->set_max_reaction_state((eReactionState)st);

    SLF_DONE;
  }
};
//slf_entity_set_max_reaction_state_t slf_entity_set_max_reaction_state(slc_entity, "set_max_reaction_state(num)");




// script library function: entity::inc_reaction_state()
class slf_entity_inc_reaction_state_t: public script_library_class::function
{
public:
  // constructor required
  slf_entity_inc_reaction_state_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    parms->me->get_brain()->inc_reaction_state();

    SLF_DONE;
  }
};
//slf_entity_inc_reaction_state_t slf_entity_inc_reaction_state(slc_entity, "inc_reaction_state()");

// script library function: entity::dec_reaction_state()
class slf_entity_dec_reaction_state_t: public script_library_class::function
{
public:
  // constructor required
  slf_entity_dec_reaction_state_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    parms->me->get_brain()->dec_reaction_state();

    SLF_DONE;
  }
};
//slf_entity_dec_reaction_state_t slf_entity_dec_reaction_state(slc_entity, "dec_reaction_state()");





// script library function: entity::set_reaction_ai(num state, str ai)
class slf_entity_set_reaction_ai_t: public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_reaction_ai_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_num_t state;
    vm_str_t ai;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    int st = parms->state;
    if(st > BRAIN_REACT_COMBAT)
      st = BRAIN_REACT_COMBAT;
    if(st < BRAIN_REACT_IDLE)
      st = BRAIN_REACT_IDLE;

    parms->me->get_brain()->set_ai_state((eReactionState)st, *parms->ai);

    SLF_DONE;
  }
};
//slf_entity_set_reaction_ai_t slf_entity_set_reaction_ai(slc_entity, "set_reaction_ai(num,str)");


// script library function: entity::set_guard_pos(vector3d pos, vector3d face)
class slf_entity_set_guard_pos_t: public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_guard_pos_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vector3d pos;
    vector3d face;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    parms->me->get_brain()->set_guard_pos(parms->pos, parms->face);

    SLF_DONE;
  }
};
//slf_entity_set_guard_pos_t slf_entity_set_guard_pos(slc_entity, "set_guard_pos(vector3d,vector3d)");



// script library function: entity::set_reload_time(num t)
class slf_entity_set_reload_time_t: public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_reload_time_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_num_t t;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    parms->me->get_brain()->set_default_reload_timer(parms->t);

    SLF_DONE;
  }
};
//slf_entity_set_reload_time_t slf_entity_set_reload_time(slc_entity, "set_reload_time(num)");

// script library function: entity::set_reload_variance(num t)
class slf_entity_set_reload_variance_t: public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_reload_variance_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_num_t v;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    parms->me->get_brain()->set_reload_timer_variance(parms->v);

    SLF_DONE;
  }
};
//slf_entity_set_reload_variance_t slf_entity_set_reload_variance(slc_entity, "set_reload_variance(num)");


// script library function: num entity::get_reload_time()
class slf_entity_get_reload_time_t: public script_library_class::function
{
public:
  // constructor required
  slf_entity_get_reload_time_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    vm_num_t result = parms->me->get_brain()->get_default_reload_timer();

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_entity_get_reload_time_t slf_entity_get_reload_time(slc_entity, "get_reload_time()");


// script library function: num entity::get_reload_variance()
class slf_entity_get_reload_variance_t: public script_library_class::function
{
public:
  // constructor required
  slf_entity_get_reload_variance_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    vm_num_t result = parms->me->get_brain()->get_reload_timer_variance();

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_entity_get_reload_variance_t slf_entity_get_reload_variance(slc_entity, "get_reload_variance()");


// script library function: entity::set_patrol_id(num id)
class slf_entity_set_patrol_id_t: public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_patrol_id_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_num_t id;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    parms->me->get_brain()->set_patrol_id((int)parms->id);

    SLF_DONE;
  }
};
//slf_entity_set_patrol_id_t slf_entity_set_patrol_id(slc_entity, "set_patrol_id(num)");


// script library function: num entity::get_patrol_id()
class slf_entity_get_patrol_id_t: public script_library_class::function
{
public:
  // constructor required
  slf_entity_get_patrol_id_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    vm_num_t result = parms->me->get_brain()->get_patrol_id();

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_entity_get_patrol_id_t slf_entity_get_patrol_id(slc_entity, "get_patrol_id()");


// script library function:  entity::link_to_dread_net_signaller(signaller sig);
class slf_entity_link_to_dread_net_signaller_t : public script_library_class::function
{
  public:
    // constructor required
    slf_entity_link_to_dread_net_signaller_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_entity_t me;
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
//slf_entity_link_to_dread_net_signaller_t slf_entity_link_to_dread_net_signaller(slc_entity,"link_to_dread_net_signaller(signaller)");

// script library function:  entity::unlink_from_dread_net_signaller(signaller sig);
class slf_entity_unlink_from_dread_net_signaller_t : public script_library_class::function
{
  public:
    // constructor required
    slf_entity_unlink_from_dread_net_signaller_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_entity_t me;
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
//slf_entity_unlink_from_dread_net_signaller_t slf_entity_unlink_from_dread_net_signaller(slc_entity,"unlink_from_dread_net_signaller(signaller)");


//---------------------------------------------------------------
// INVESTIGATE BRAINS
//---------------------------------------------------------------
// script library function:  entity::push_state_investigate()
class slf_entity_push_state_investigate_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_push_state_investigate_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vector3d pos;
    vm_num_t running;
    vm_num_t search;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if(parms->me->get_brain() != NULL)
      parms->me->get_brain()->push_state_investigate( parms->pos, parms->running, parms->search, true );
    SLF_DONE;
  }
};
//slf_entity_push_state_investigate_t slf_entity_push_state_investigate(slc_entity,"push_state_investigate(vector3d,num,num)");

//---------------------------------------------------------------
// GOTO BRAINS
//---------------------------------------------------------------
// script library function:  entity::push_state_goto(vector3d,num,num)
class slf_entity_push_state_goto_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_push_state_goto_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vector3d loc;
    rational_t range;
    vm_num_t running;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if(parms->me->get_brain() != NULL)
      parms->me->get_brain()->push_state_goto(parms->loc,parms->range,parms->running, false);
    SLF_DONE;
  }
};
//slf_entity_push_state_goto_t slf_entity_push_state_goto(slc_entity,"push_state_goto(vector3d,num,num)");

//---------------------------------------------------------------
// GOTO BRAINS
//---------------------------------------------------------------
// script library function:  entity::push_state_path_goto(vector3d,num,num)
class slf_entity_push_state_path_goto_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_push_state_path_goto_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vector3d loc;
    rational_t range;
    vm_num_t running;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if(parms->me->get_brain() != NULL)
      parms->me->get_brain()->push_state_goto(parms->loc,parms->range,parms->running, true);
    SLF_DONE;
  }
};
//slf_entity_push_state_path_goto_t slf_entity_push_state_path_goto(slc_entity,"push_state_path_goto(vector3d,num,num)");

//---------------------------------------------------------------
// GOTO_HERO BRAINS
//---------------------------------------------------------------
// script library function:  entity::push_state_goto_hero()
class slf_entity_push_state_goto_hero_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_push_state_goto_hero_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if(parms->me->get_brain() != NULL)
      parms->me->get_brain()->push_state_goto_hero();
    SLF_DONE;
  }
};
//slf_entity_push_state_goto_hero_t slf_entity_push_state_goto_hero(slc_entity,"push_state_goto_hero()");

//---------------------------------------------------------------
// FACING BRAINS
//---------------------------------------------------------------
// script library function:  entity::push_state_change_facing(vector3d)
class slf_entity_push_state_change_facing_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_push_state_change_facing_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vector3d loc;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if(parms->me->get_brain() != NULL)
      parms->me->get_brain()->push_state_change_facing(parms->loc);
    SLF_DONE;
  }
};
//slf_entity_push_state_change_facing_t slf_entity_push_state_change_facing(slc_entity,"push_state_change_facing(vector3d)");
*/
/*
// script library function:  entity::push_inactive()
class slf_entity_push_inactive_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_push_inactive_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if(parms->me->get_brain() != NULL)
      parms->me->get_brain()->push_inactive();
    SLF_DONE;
  }
};
slf_entity_push_inactive_t slf_entity_push_inactive(slc_entity,"push_inactive()");

// script library function:  entity::pop_inactive()
class slf_entity_pop_inactive_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_pop_inactive_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if(parms->me->get_brain() != NULL)
      parms->me->get_brain()->pop_inactive();
    SLF_DONE;
  }
};
slf_entity_pop_inactive_t slf_entity_pop_inactive(slc_entity,"pop_inactive()");
*/
/*
// script library function: str entity::get_state_info()
class slf_entity_get_state_info_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_get_state_info_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;
    static stringx none = "No Brain";
    vm_str_t result = &none;

    if(parms->me->get_brain() != NULL)
      result = &parms->me->get_brain()->get_state_info();

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_entity_get_state_info_t slf_entity_get_state_info(slc_entity, "get_state_info()");

// script library function:  entity::force_pathing()
class slf_entity_force_pathing_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_force_pathing_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if(parms->me->get_brain() != NULL)
      parms->me->get_brain()->set_forced_path(true);
    SLF_DONE;
  }
};
//slf_entity_force_pathing_t slf_entity_force_pathing(slc_entity,"force_pathing()");

// script library function:  entity::unforce_pathing()
class slf_entity_unforce_pathing_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_unforce_pathing_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if(parms->me->get_brain() != NULL)
      parms->me->get_brain()->set_forced_path(false);
    SLF_DONE;
  }
};
//slf_entity_unforce_pathing_t slf_entity_unforce_pathing(slc_entity,"unforce_pathing()");

// script library function:  entity::set_path_graph(str)
class slf_entity_set_path_graph_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_path_graph_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_str_t path;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    path_graph *graph = g_world_ptr->get_path_graph(*parms->path);
    parms->me->get_brain()->set_current_path_graph(graph);
    SLF_DONE;
  }
};
//slf_entity_set_path_graph_t slf_entity_set_path_graph(slc_entity,"set_path_graph(str)");

// script library function: str entity::get_path_graph_name()
class slf_entity_get_path_graph_name_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_get_path_graph_name_t(script_library_class *slc, const char *n): script_library_class::function(slc, n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    static stringx none("NONE");
    SLF_PARMS;

    #ifndef BUILD_BOOTABLE
    if ( parms->me->is_hero() )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): don't call this function on the hero!" );
    if ( parms->me->get_brain() == NULL )
      stack.get_thread()->slf_error( "$" + parms->me->get_name() + "." + stringx(get_name()) + "(): this entity has no brain!" );
    #endif

    vm_str_t result = &none;

    path_graph *graph = parms->me->get_brain()->get_current_path_graph();
    if(graph != NULL)
      result = &graph->get_id();

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_entity_get_path_graph_name_t slf_entity_get_path_graph_name(slc_entity, "get_path_graph_name()");

// script library function:  entity::add_cue(str)
class slf_entity_add_cue_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_add_cue_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_str_t cue_name;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    assert(g_world_ptr->get_dread_net());

    dread_net::eAVCueType cue = dread_net::get_cue_type(*parms->cue_name);

    if(cue != dread_net::UNDEFINED_AV_CUE)
      g_world_ptr->get_dread_net()->add_cue(cue, parms->me);
    else
      warning("Error raising AV Cue '%s' on '%s': AV Cue does not exist!", parms->cue_name->c_str(), parms->me->get_name().c_str());

    SLF_DONE;
  }
};
//slf_entity_add_cue_t slf_entity_add_cue(slc_entity,"add_cue(str)");
*/

class slf_entity_set_render_scale_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_render_scale_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vector3d s;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    parms->me->set_render_scale( parms->s );
    SLF_DONE;
  }
};
//slf_entity_set_render_scale_t slf_entity_set_render_scale(slc_entity,"set_render_scale(vector3d)");

// script library function:  entity::wait_change_render_scale(vector3d,num)
class slf_entity_wait_change_render_scale_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_wait_change_render_scale_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function persistent local vm_stack data;
  // stored on stack (after parms) until function completed;
  // accessed by SLF_SDATA at start of function
  struct sdata_t
  {
    vector3d old_scale;
    vm_num_t time_elapsed;
  };
  // library function parameters;
  // accessed by SLF_PARMS
  struct parms_t
  {
    vm_entity_t me;
    vector3d new_scale;
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
      sdata->time_elapsed = 0.0f;
      sdata->old_scale = parms->me->get_render_scale();
      SLF_RECALL;
    }
    else
    {
      sdata->time_elapsed += CALC_ENTITY_TIME_DILATION(SCRIPT_TIME_INC, parms->me);
      time_value_t duration = parms->duration;
      if (sdata->time_elapsed > duration)
        sdata->time_elapsed = duration;

      // compute incremental translation
      vector3d scale = parms->new_scale-sdata->old_scale;
      scale *= sdata->time_elapsed / parms->duration;
      scale += sdata->old_scale;

      parms->me->set_render_scale( scale );

      // repeat process until duration elapses
      if ( sdata->time_elapsed < duration)
      {
        // come back next frame
        SLF_RECALL;
      }
      else
      {
        // rotation completed; terminate library function
        SLF_DONE;
      }
    }
  }
};
//slf_entity_wait_change_render_scale_t slf_entity_wait_change_render_scale(slc_entity,"wait_change_render_scale(vector3d,num,num)");

class slf_entity_set_render_color_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_render_color_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vector3d loc;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    if(parms->loc.x < 0.0f)
      parms->loc.x = 0.0f;
    if(parms->loc.x > 1.0f)
      parms->loc.x = 1.0f;

    if(parms->loc.y < 0.0f)
      parms->loc.y = 0.0f;
    if(parms->loc.y > 1.0f)
      parms->loc.y = 1.0f;

    if(parms->loc.z < 0.0f)
      parms->loc.z = 0.0f;
    if(parms->loc.z > 1.0f)
      parms->loc.z = 1.0f;


    color32 col = parms->me->get_render_color();
    col.set_red(parms->loc.x * 255);
    col.set_green(parms->loc.y * 255);
    col.set_blue(parms->loc.z * 255);

    parms->me->set_render_color(col);

    SLF_DONE;
  }
};
//slf_entity_set_render_color_t slf_entity_set_render_color(slc_entity,"set_render_color(vector3d)");

class slf_entity_set_render_alpha_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_render_alpha_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_num_t alpha;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    if(parms->alpha < 0.0f)
      parms->alpha = 0.0f;
    if(parms->alpha > 1.0f)
      parms->alpha = 1.0f;

    color32 col = parms->me->get_render_color();
    col.set_alpha(parms->alpha * 255);

    parms->me->set_render_color(col);

    SLF_DONE;
  }
};
//slf_entity_set_render_alpha_t slf_entity_set_render_alpha(slc_entity,"set_render_alpha(num)");

// script library function:  vector3d entity::get_render_color()
class slf_get_render_color_t : public script_library_class::function
{
public:
  // constructor required
  slf_get_render_color_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vector3d result;

    color32 col = parms->me->get_render_color();
    result.x = col.get_red()    / 255.0f;
    result.y = col.get_green()  / 255.0f;
    result.z = col.get_blue()   / 255.0f;

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_get_render_color_t slf_get_render_color(slc_entity, "get_render_color()");


// script library function:  vector3d entity::get_render_alpha()
class slf_get_render_alpha_t : public script_library_class::function
{
public:
  // constructor required
  slf_get_render_alpha_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    color32 col = parms->me->get_render_color();
    vm_num_t result = col.get_alpha() / 255.0f;

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_get_render_alpha_t slf_get_render_alpha(slc_entity, "get_render_alpha()");

// script library function:  entity::wait_change_render_color(vector3d,num)
class slf_entity_wait_change_render_color_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_wait_change_render_color_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function persistent local vm_stack data;
  // stored on stack (after parms) until function completed;
  // accessed by SLF_SDATA at start of function
  struct sdata_t
  {
//    game_clock_t clock;
    vector3d old_color;
    vm_num_t old_alpha;
    vm_num_t time_elapsed;
  };
  // library function parameters;
  // accessed by SLF_PARMS
  struct parms_t
  {
    vm_entity_t me;
    vector3d new_color;
    vm_num_t new_alpha;
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
//      sdata->clock.reset();
      sdata->time_elapsed = 0.0f;

      color32 col = parms->me->get_render_color();

      sdata->old_color = vector3d(col.get_red() / 255.0f, col.get_green() / 255.0f, col.get_blue() / 255.0f);
      sdata->old_alpha = col.get_alpha() / 255.0f;

      SLF_RECALL;
    }
    else
    {
      sdata->time_elapsed += CALC_ENTITY_TIME_DILATION(SCRIPT_TIME_INC, parms->me);
      time_value_t duration = parms->duration;
      if (sdata->time_elapsed > duration)
      {
        sdata->time_elapsed = duration;
      }

      // compute incremental translation
      vector3d cur_color = parms->new_color-sdata->old_color;
      cur_color *= sdata->time_elapsed / parms->duration;
      cur_color += sdata->old_color;

      rational_t cur_alpha = parms->new_alpha-sdata->old_alpha;
      cur_alpha *= sdata->time_elapsed / parms->duration;
      cur_alpha += sdata->old_alpha;

      color32 col;
      col.set_red(cur_color.x * 255.0f);
      col.set_green(cur_color.y * 255.0f);
      col.set_blue(cur_color.z * 255.0f);
      col.set_alpha(cur_alpha * 255.0f);

      parms->me->set_render_color( col );

      // repeat process until duration elapses
      if ( sdata->time_elapsed < duration)
      {
        // come back next frame
        SLF_RECALL;
      }
      else
      {
        // rotation completed; terminate library function
        SLF_DONE;
      }
    }
  }
};
//slf_entity_wait_change_render_color_t slf_entity_wait_change_render_color(slc_entity,"wait_change_render_color(vector3d,num,num)");


/*
//---------------------------------------------------------------
//---------------------------------------------------------------
// IDLE BRAINS
//---------------------------------------------------------------
// script library function:  entity::push_state_idle(num,num,num)
class slf_entity_push_state_idle_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_push_state_idle_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_num_t inner;
    vm_num_t outer;
    vm_num_t arc;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if(parms->me->get_brain() != NULL)
      parms->me->get_brain()->push_state_idle( parms->inner, parms->outer, parms->arc );
    SLF_DONE;
  }
};
slf_entity_push_state_idle_t slf_entity_push_state_idle(slc_entity,"push_state_idle(num,num,num)");

// script library function:  entity::push_state_safe_idle()
class slf_entity_push_state_safe_idle_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_push_state_safe_idle_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if(parms->me->get_brain() != NULL)
      parms->me->get_brain()->push_state_safe_idle();
    SLF_DONE;
  }
};
slf_entity_push_state_safe_idle_t slf_entity_push_state_safe_idle(slc_entity,"push_state_safe_idle()");

//---------------------------------------------------------------
// PATROLLER BRAINS
//---------------------------------------------------------------
// script library function:  entity::push_state_patroller_path()
class slf_entity_push_state_patroller_path_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_push_state_patroller_path_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_num_t running;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if(parms->me->get_brain() != NULL)
      parms->me->get_brain()->push_state_patroller_path( parms->running );
    SLF_DONE;
  }
};
slf_entity_push_state_patroller_path_t slf_entity_push_state_patroller_path(slc_entity,"push_state_patroller_path(num)");

// script library function:  entity::push_state_patroller_ring()
class slf_entity_push_state_patroller_ring_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_push_state_patroller_ring_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_num_t running;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if(parms->me->get_brain() != NULL)
      parms->me->get_brain()->push_state_patroller_ring( parms->running );
    SLF_DONE;
  }
};
slf_entity_push_state_patroller_ring_t slf_entity_push_state_patroller_ring(slc_entity,"push_state_patroller_ring(num)");

class slf_entity_push_state_patroller_pingpong_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_push_state_patroller_pingpong_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_num_t running;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if(parms->me->get_brain() != NULL)
      parms->me->get_brain()->push_state_patroller_pingpong( parms->running );
    SLF_DONE;
  }
};
slf_entity_push_state_patroller_pingpong_t slf_entity_push_state_patroller_pingpong(slc_entity,"push_state_patroller_pingpong(num)");

// script library function:  entity::add_waypoint(vector3d)
class slf_entity_add_waypoint_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_add_waypoint_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vector3d loc;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if(parms->me->get_brain() != NULL)
      parms->me->get_brain()->add_waypoint(parms->loc);
    SLF_DONE;
  }
};
slf_entity_add_waypoint_t slf_entity_add_waypoint(slc_entity,"add_waypoint(vector3d)");

// script library function:  entity::clear_waypoints()
class slf_entity_clear_waypoints_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_clear_waypoints_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if(parms->me->get_brain() != NULL)
      parms->me->get_brain()->clear_waypoints();
    SLF_DONE;
  }
};
slf_entity_clear_waypoints_t slf_entity_clear_waypoints(slc_entity,"clear_waypoints()");

//---------------------------------------------------------------
// SLUGGER BRAINS
//---------------------------------------------------------------
// script library function:  entity::push_state_slugger(num,num)
class slf_entity_push_state_slugger_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_push_state_slugger_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_num_t rt;
    vm_num_t na;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if(parms->me->get_brain() != NULL)
      parms->me->get_brain()->push_state_slugger();
    SLF_DONE;
  }
};
slf_entity_push_state_slugger_t slf_entity_push_state_slugger(slc_entity,"push_state_slugger(num,num)");

//---------------------------------------------------------------
// SHOOTER BRAINS
//---------------------------------------------------------------
// script library function:  entity::push_state_shooter(num,num)
class slf_entity_push_state_shooter_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_push_state_shooter_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_num_t rt;
    vm_num_t na;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if(parms->me->get_brain() != NULL)
      parms->me->get_brain()->push_state_shooter();
    SLF_DONE;
  }
};
slf_entity_push_state_shooter_t slf_entity_push_state_shooter(slc_entity,"push_state_shooter(num,num)");

//---------------------------------------------------------------
// CHAINGUNNER BRAINS
//---------------------------------------------------------------
// script library function:  entity::push_state_chaingunner(num,num,num,num)
class slf_entity_push_state_chaingunner_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_push_state_chaingunner_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_num_t rt;
    vm_num_t na;
    vm_num_t ad;
    vm_num_t ap;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if(parms->me->get_brain() != NULL)
      parms->me->get_brain()->push_state_chaingunner(parms->rt,parms->na,parms->ad,parms->ap);
    SLF_DONE;
  }
};
slf_entity_push_state_chaingunner_t slf_entity_push_state_chaingunner(slc_entity,"push_state_chaingunner(num,num,num,num)");

//---------------------------------------------------------------
// GRENADIER BRAINS
//---------------------------------------------------------------
// script library function:  entity::push_state_grenadier(num,num)
class slf_entity_push_state_grenadier_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_push_state_grenadier_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_num_t rt;
    vm_num_t na;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if(parms->me->get_brain() != NULL)
      parms->me->get_brain()->push_state_grenadier();
    SLF_DONE;
  }
};
slf_entity_push_state_grenadier_t slf_entity_push_state_grenadier(slc_entity,"push_state_grenadier(num,num)");

//---------------------------------------------------------------
// GRENADIER BRAINS
//---------------------------------------------------------------
// script library function:  entity::push_state_mechatomo(num,num)
class slf_entity_push_state_mechatomo_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_push_state_mechatomo_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vector3d pt;
    vm_num_t rg;
    vm_num_t close;
    vm_num_t farr;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if(parms->me->get_brain() != NULL)
      parms->me->get_brain()->push_state_mechatomo();
    SLF_DONE;
  }
};
slf_entity_push_state_mechatomo_t slf_entity_push_state_mechatomo(slc_entity,"push_state_mechatomo(vector3d,num,num,num)");

//---------------------------------------------------------------
// UTILITY BRAINS
//---------------------------------------------------------------

// script library function:  entity::pop_state()
class slf_entity_pop_state_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_pop_state_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    if(parms->me->get_brain() != NULL)
      parms->me->get_brain()->pop_state();
    SLF_DONE;
  }
};
slf_entity_pop_state_t slf_entity_pop_state(slc_entity,"pop_state()");
*/




static rational_t lookat_get_h_rot(entity *targetting_ent, vector3d pos)
{
  rational_t ang = 0.0f;

  vector3d face = targetting_ent->get_abs_po().get_facing();
  vector3d delta = (pos - targetting_ent->get_abs_position());
  delta.y = face.y = 0.0f;
  delta.normalize();
  face.normalize();

  if(face.length2() < 0.9f || delta.length2() < 0.9f)
    return(0.0f);

  rational_t dot_prod = dot(face, delta);

  if(dot_prod <= 1.0f && dot_prod >= -1.0f)
  {
    ang = fast_acos(dot_prod);

    rational_t cross = ((delta.z*face.x) - (delta.x*face.z));
    if((cross < 0.0f && ang > 0.0f) || (cross > 0.0f && ang < 0.0f))
      ang = -ang;
  }

  return(ang);
}

static rational_t lookat_get_v_rot(entity *targetting_ent, vector3d pos)
{
  rational_t ang = 0.0f;

  vector3d face = targetting_ent->get_abs_po().get_facing();
  vector3d delta = (pos - targetting_ent->get_abs_position());

  delta.normalize();
  face.normalize();

  if((face.y >= 0.0f && delta.y >= 0.0f) || (face.y <= 0.0f && delta.y <= 0.0f))
  {
      ang = asin(delta.y) - asin(face.y);
  }
  else
  {
      ang = asin(__fabs(face.y)) + asin(__fabs(delta.y));

      if(delta.y < face.y)
        ang = -ang;
  }

  return(ang);
}

// script library function:  entity::wait_lookat2(ent,ent,vector3d,num)
class slf_entity_wait_lookat2_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_wait_lookat2_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function persistent local vm_stack data;
  // stored on stack (after parms) until function completed;
  // accessed by SLF_SDATA at start of function
  struct sdata_t
  {
//    game_clock_t clock;
    rational_t hrads_sec;
    rational_t vrads_sec;
  };

  // library function parameters;
  // accessed by SLF_PARMS
  struct parms_t
  {
    vm_entity_t me;
    vm_entity_t hent;
    vm_entity_t vent;
    vector3d pt;
    vm_num_t duration;
  };

  void do_rot(sdata_t *sdata, parms_t *parms, rational_t tdelta)
  {
    // compute incremental rotation
    float inc_rads = 0.0f;

    inc_rads = sdata->hrads_sec * tdelta;
    po rot_po = po_identity_matrix;
    rot_po.set_rotate_y(inc_rads);
    po tmp_po = parms->hent->get_abs_po();
    vector3d pos;
    if(parms->hent->link_ifc()->get_parent() != NULL)
      pos = parms->hent->get_rel_position();
    else
      pos = parms->hent->get_abs_position();
    tmp_po.set_position(ZEROVEC);
    tmp_po.add_increment(&rot_po);
    if(parms->hent->link_ifc()->get_parent() != NULL)
    {
      fast_po_mul(tmp_po, tmp_po, parms->hent->link_ifc()->get_parent()->get_abs_po().inverse());
//      tmp_po = tmp_po * parms->hent->link_ifc()->get_parent()->get_abs_po().inverse();
    }
    tmp_po.fixup();
    tmp_po.set_position(pos);
    parms->hent->set_rel_po(tmp_po);

    inc_rads = sdata->vrads_sec * tdelta;
    rot_po = po_identity_matrix;
    rot_po.set_rot(parms->vent->get_abs_po().get_x_facing(), inc_rads);
    tmp_po = parms->vent->get_abs_po();
    if(parms->vent->link_ifc()->get_parent() != NULL)
      pos = parms->vent->get_rel_position();
    else
      pos = parms->vent->get_abs_position();
    tmp_po.set_position(ZEROVEC);
    tmp_po.add_increment(&rot_po);
    if(parms->vent->link_ifc()->get_parent() != NULL)
    {
      fast_po_mul(tmp_po, tmp_po, parms->vent->link_ifc()->get_parent()->get_abs_po().inverse());
//      tmp_po = tmp_po * parms->vent->link_ifc()->get_parent()->get_abs_po().inverse();
    }
    tmp_po.fixup();
    tmp_po.set_position(pos);
    parms->vent->set_rel_po(tmp_po);
  }

  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_SDATA;
    SLF_PARMS;
    parms->me->check_nonstatic();

    if (entry == FIRST_ENTRY)
    {
      // first entry to this library call with this stack;
      // set up timer
      if(parms->duration > 0.0f)
      {
//        sdata->clock.reset();
        sdata->hrads_sec = lookat_get_h_rot(parms->me, parms->pt) / parms->duration;
        sdata->vrads_sec = lookat_get_v_rot(parms->me, parms->pt) / parms->duration;

/*
#if defined(BUILD_DEBUG) || defined(BUILD_FASTDEBUG)
        new_LineInfo info;
        info.clear();
        info.StartCoords = parms->me->get_abs_position();
        info.EndCoords = parms->pt;
        render_lineinfo(&info, 0);

        info.StartCoords = parms->vent->get_abs_position();
        info.EndCoords = info.StartCoords + parms->vent->get_abs_po().get_facing()*5;
        render_lineinfo(&info, 1);

        info.StartCoords = parms->hent->get_abs_position();
        info.EndCoords = info.StartCoords + parms->hent->get_abs_po().get_facing()*5;
        render_lineinfo(&info, 2);
#endif
*/
        SLF_RECALL;
      }
      else
      {
        sdata->hrads_sec = lookat_get_h_rot(parms->me, parms->pt);
        sdata->vrads_sec = lookat_get_v_rot(parms->me, parms->pt);

        do_rot(sdata, parms, 1.0f);

        SLF_DONE;
      }
    }
    else
    {
      // update timer
      time_value_t tdelta = CALC_ENTITY_TIME_DILATION(SCRIPT_TIME_INC, parms->me);
      time_value_t time_left = parms->duration - tdelta;
      if (time_left < 0)
      {
        tdelta += time_left;
        time_left = 0;
      }

      do_rot(sdata, parms, tdelta);

/*
#if defined(BUILD_DEBUG) || defined(BUILD_FASTDEBUG)
      new_LineInfo info;
      info.clear();
      parms->hent->update_abs_po();
      parms->vent->update_abs_po();
      parms->me->update_abs_po();
      info.StartCoords = parms->vent->get_abs_position();
      info.EndCoords = info.StartCoords + parms->vent->get_abs_po().get_facing()*5;
      render_lineinfo(&info, 1);
      info.StartCoords = parms->hent->get_abs_position();
      info.EndCoords = info.StartCoords + parms->hent->get_abs_po().get_facing()*5;
      render_lineinfo(&info, 2);
      info.StartCoords = parms->me->get_abs_position();
      info.EndCoords = info.StartCoords + parms->me->get_abs_po().get_facing()*5;
      render_lineinfo(&info, 3);
#endif
*/

      // repeat process until original duration elapses
      if (time_left)
      {
        // decrease duration to reflect incremental change
        parms->duration = time_left;
        // come back next frame
        SLF_RECALL;
      }
      else
      {
        // rotation completed; terminate library function
        SLF_DONE;
      }
    }
  }
};




// script library function:  entity::wait_lookat(vector3d,num)
class slf_entity_wait_lookat_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_wait_lookat_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function persistent local vm_stack data;
  // stored on stack (after parms) until function completed;
  // accessed by SLF_SDATA at start of function
  struct sdata_t
  {
//    game_clock_t clock;
    rational_t rads_left;
    rational_t rads_sec;
    vector3d axis;
  };

  // library function parameters;
  // accessed by SLF_PARMS
  struct parms_t
  {
    vm_entity_t me;
    vector3d pt;
    vm_num_t duration;
  };

  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_SDATA;
    SLF_PARMS;
    parms->me->check_nonstatic();

    if (entry == FIRST_ENTRY)
    {
      // first entry to this library call with this stack;
      // set up timer
//      sdata->clock.reset();

      parms->me->update_abs_po();
//      vector3d pt = parms->pt;
//      if(parms->me->has_parent())
//        pt = parms->me->link_ifc()->get_parent()->get_abs_po().inverse().slow_xform(pt);

      vector3d dir = (parms->pt - parms->me->get_abs_position());
      dir.normalize();
      vector3d facing = parms->me->get_abs_po().get_facing();

      sdata->axis = cross(facing, dir);
      sdata->axis.normalize();
      sdata->rads_left = fast_acos(dot(dir, facing));
      if(parms->duration > 0.0f)
        sdata->rads_sec = sdata->rads_left / parms->duration;
      else
        sdata->rads_sec = sdata->rads_left;

#if 0 // BIGCULL
#if defined(BUILD_DEBUG) || defined(BUILD_FASTDEBUG)
      new_LineInfo info;
      info.clear();

      info.StartCoords = parms->me->get_abs_position();
      info.EndCoords = parms->pt;
      render_lineinfo(&info, 0);

      info.StartCoords = parms->me->get_abs_position();
      info.EndCoords = parms->me->get_abs_position() + facing*4;
      render_lineinfo(&info, 1);

      info.StartCoords = parms->me->get_abs_position();
      info.EndCoords = parms->me->get_abs_position() + sdata->axis*4;
      render_lineinfo(&info, 2);
#endif
#endif  // BIGCULL

      SLF_RECALL;
    }
    else
    {
      // update timer
      time_value_t tdelta = CALC_ENTITY_TIME_DILATION(SCRIPT_TIME_INC, parms->me);
      time_value_t time_left = parms->duration - tdelta;
      if (time_left < 0)
      {
        tdelta += time_left;
        time_left = 0;
      }

      float inc_rads = 0.0f;

      // compute incremental rotation
      inc_rads = sdata->rads_sec * tdelta;

      po rot;
      vector3d axis = sdata->axis;
      rot.set_rot(axis, inc_rads);
      parms->me->set_frame_delta(rot,tdelta);

      vector3d pos = parms->me->get_rel_position();
      parms->me->set_rel_position(ZEROVEC);

      fast_po_mul(rot, parms->me->get_rel_po(), rot );
      parms->me->set_rel_po(rot);

//      parms->me->set_rel_po(parms->me->get_rel_po()*rot);
      parms->me->set_rel_position(pos);

#if 0 // BIGCULL
#if defined(BUILD_DEBUG) || defined(BUILD_FASTDEBUG)
      parms->me->update_abs_po();
      new_LineInfo info;
      info.clear();
      info.StartCoords = parms->me->get_abs_position();
      info.EndCoords = parms->me->get_abs_position() + parms->me->get_abs_po().get_facing()*6;
      render_lineinfo(&info, 3);
#endif
#endif  // BIGCULL
      // repeat process until original duration elapses
      if (time_left)
      {
        // decrease duration to reflect incremental change
        parms->duration = time_left;
        // decrease rads by proportional amount; ratio rads/duration is preserved
        sdata->rads_left -= inc_rads;
        // come back next frame
        SLF_RECALL;
      }
      else
      {
        // rotation completed; terminate library function
        SLF_DONE;
      }
    }
  }
};


class slf_entity_set_scale_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_scale_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_num_t scale;
  };

  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    if(parms->scale > 0.0f)
    {
      rational_t old_scale = parms->me->get_rel_po().get_scale();
      assert(old_scale > 0.0f);

      rational_t scale_diff = parms->scale / old_scale;
      assert(scale_diff > 0.0f);

      po scale_po = po_identity_matrix;
      scale_po.set_scale(vector3d(scale_diff, scale_diff, scale_diff));

      po rel_po = parms->me->get_rel_po();
      vector3d rel_pos = rel_po.get_position();
      rel_po.set_position(ZEROVEC);

      fast_po_mul(rel_po, rel_po, scale_po);
      //rel_po = rel_po * scale_po;

      rel_po.set_position(rel_pos);
      parms->me->set_rel_po(rel_po);
    }

    SLF_DONE;
  }
};
//slf_entity_set_scale_t slf_entity_set_scale(slc_entity,"set_scale(num)");

// script library function:  entity::wait_set_scale(num,num)
class slf_entity_wait_set_scale_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_wait_set_scale_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function persistent local vm_stack data;
  // stored on stack (after parms) until function completed;
  // accessed by SLF_SDATA at start of function
  struct sdata_t
  {
//    game_clock_t clock;
    rational_t start_scale;
    rational_t cur_time;
  };

  // library function parameters;
  // accessed by SLF_PARMS
  struct parms_t
  {
    vm_entity_t me;
    vm_num_t scale;
    vm_num_t duration;
  };

  void do_scale(sdata_t *sdata, parms_t *parms, rational_t scale)
  {
    rational_t old_scale = parms->me->get_rel_po().get_scale();
    assert(old_scale > 0.0f);

    rational_t scale_diff = scale / old_scale;
    assert(scale_diff > 0.0f);

    po scale_po = po_identity_matrix;
    scale_po.set_scale(vector3d(scale_diff, scale_diff, scale_diff));

    po rel_po = parms->me->get_rel_po();
    vector3d rel_pos = rel_po.get_position();
    rel_po.set_position(ZEROVEC);
    fast_po_mul(rel_po, rel_po, scale_po);
//    rel_po = rel_po * scale_po;
    rel_po.set_position(rel_pos);
    parms->me->set_rel_po(rel_po);
  }

  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_SDATA;
    SLF_PARMS;
    parms->me->check_nonstatic();

    if (entry == FIRST_ENTRY)
    {
      // first entry to this library call with this stack;
      // set up timer
//      sdata->clock.reset();
      if(parms->scale > 0.0f)
      {
        if(parms->duration > 0.0f)
        {
          sdata->start_scale = parms->me->get_rel_po().get_scale();
          assert(sdata->start_scale > 0.0f);

          sdata->cur_time = 0.0f;

          SLF_RECALL;
        }
        else
        {
          do_scale(sdata, parms, parms->scale);
          SLF_DONE;
        }
      }
      else
      {
        SLF_DONE;
      }
    }
    else
    {
      // update timer
      sdata->cur_time += CALC_ENTITY_TIME_DILATION(SCRIPT_TIME_INC, parms->me);
      if (sdata->cur_time >= parms->duration)
      {
        do_scale(sdata, parms, parms->scale);
        SLF_DONE;
      }
      else
      {
        do_scale(sdata, parms, sdata->start_scale + ((parms->scale - sdata->start_scale)*(sdata->cur_time / parms->duration)));

        // come back next frame
        SLF_RECALL;
      }
    }
  }
};















// script library function:  num entity::get_ifc_num(str)
class slf_entity_get_ifc_num_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_get_ifc_num_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_str_t att;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    static pstring att;
    att = *parms->att;

    vm_num_t result = 0.0f;
    if(!parms->me->get_ifc_num(att, result))
      warning("No such value '%s' for entity::get_ifc_num()", parms->att->c_str());

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_entity_get_ifc_num_t slf_entity_get_ifc_num(slc_entity, "get_ifc_num(str)");

class slf_entity_set_ifc_num_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_ifc_num_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_str_t att;
    vm_num_t val;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    static pstring att;
    att = *parms->att;

    if(!parms->me->set_ifc_num(att, parms->val))
      warning("No such value '%s' for entity::set_ifc_num()", parms->att->c_str());

    SLF_DONE;
  }
};
//slf_entity_set_ifc_num_t slf_entity_set_ifc_num(slc_entity,"set_ifc_num(str,num)");


// script library function:  vec entity::get_ifc_vec(str)
class slf_entity_get_ifc_vec_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_get_ifc_vec_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_str_t att;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    static pstring att;
    att = *parms->att;

    vector3d result = ZEROVEC;
    if(!parms->me->get_ifc_vec(att, result))
      warning("No such value '%s' for entity::get_ifc_vec()", parms->att->c_str());

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_entity_get_ifc_vec_t slf_entity_get_ifc_vec(slc_entity, "get_ifc_vec(str)");

class slf_entity_set_ifc_vec_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_ifc_vec_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_str_t att;
    vector3d val;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    static pstring att;
    att = *parms->att;

    if(!parms->me->set_ifc_vec(att, parms->val))
      warning("No such value '%s' for entity::set_ifc_vec()", parms->att->c_str());

    SLF_DONE;
  }
};
//slf_entity_set_ifc_num_t slf_entity_set_ifc_num(slc_entity,"set_ifc_num(str,num)");

extern stringx script_return_string;

// script library function:  num entity::get_ifc_str(str)
class slf_entity_get_ifc_str_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_get_ifc_str_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_str_t att;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    static stringx script_return_string = empty_string;

    static pstring att;
    att = *parms->att;

    if(!parms->me->get_ifc_str(att, script_return_string))
      warning("No such script_return_stringue '%s' for entity::get_ifc_str()", parms->att->c_str());

    vm_str_t result = &script_return_string;

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_entity_get_ifc_str_t slf_entity_get_ifc_str(slc_entity, "get_ifc_str(str)");

class slf_entity_set_ifc_str_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_set_ifc_str_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_str_t att;
    vm_str_t val;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    static pstring att;
    att = *parms->att;

    if(!parms->me->set_ifc_str(att, *parms->val))
      warning("No such value '%s' for entity::set_ifc_str()", parms->att->c_str());

    SLF_DONE;
  }
};
//slf_entity_set_ifc_str_t slf_entity_set_ifc_str(slc_entity,"set_ifc_str(str,str)");




class slf_ai_goto_t : public script_library_class::function
{
public:
  // constructor required
  slf_ai_goto_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vector3d pos;
    vm_num_t radius;
    vm_num_t fast;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    if(parms->me->has_ai_ifc())
      parms->me->ai_ifc()->goto_position(parms->pos, parms->radius, (parms->fast != 0.0f), true, false);

    SLF_DONE;
  }
};
//slf_ai_goto_t slf_ai_goto(slc_entity,"ai_goto(vector3d,num,num)");

class slf_force_ai_goto_t : public script_library_class::function
{
public:
  // constructor required
  slf_force_ai_goto_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vector3d pos;
    vm_num_t radius;
    vm_num_t fast;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    if(parms->me->has_ai_ifc())
      parms->me->ai_ifc()->goto_position(parms->pos, parms->radius, (parms->fast != 0.0f), true, true);

    SLF_DONE;
  }
};
//slf_force_ai_goto_t slf_force_ai_goto(slc_entity,"force_ai_goto(vector3d,num,num)");



class slf_set_ai_goal_num_t : public script_library_class::function
{
public:
  // constructor required
  slf_set_ai_goal_num_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_str_t goal;
    vm_str_t str;
    vm_num_t num;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    if(parms->me->has_ai_ifc())
    {
      static pstring goal_id;
      static pstring att;

      goal_id = *parms->goal;
      att = *parms->str;

      parms->me->ai_ifc()->set_goal_num(goal_id, att, parms->num);
    }

    SLF_DONE;
  }
};
//slf_set_ai_goal_num_t slf_set_ai_goal_num(slc_entity,"set_ai_goal_num(str,str,num)");


class slf_get_ai_goal_num_t : public script_library_class::function
{
public:
  // constructor required
  slf_get_ai_goal_num_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_str_t goal;
    vm_str_t str;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    vm_num_t result = 0.0f;
    if(parms->me->has_ai_ifc())
    {
      static pstring goal_id;
      static pstring att;

      goal_id = *parms->goal;
      att = *parms->str;

      parms->me->ai_ifc()->get_goal_num(goal_id, att, result);
    }

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_get_ai_goal_num_t slf_get_ai_goal_num(slc_entity,"get_ai_goal_num(str,str)");





class slf_set_ai_goal_str_t : public script_library_class::function
{
public:
  // constructor required
  slf_set_ai_goal_str_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_str_t goal;
    vm_str_t str;
    vm_str_t val;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    if(parms->me->has_ai_ifc())
    {
      static pstring goal_id;
      static pstring att;

      goal_id = *parms->goal;
      att = *parms->str;

      parms->me->ai_ifc()->set_goal_str(goal_id, att, *parms->val);
    }

    SLF_DONE;
  }
};
//slf_set_ai_goal_str_t slf_set_ai_goal_str(slc_entity,"set_ai_goal_str(str,str,str)");

class slf_get_ai_goal_str_t : public script_library_class::function
{
public:
  // constructor required
  slf_get_ai_goal_str_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_str_t goal;
    vm_str_t str;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    script_return_string = empty_string;
    if(parms->me->has_ai_ifc())
    {
      static pstring goal_id;
      static pstring att;

      goal_id = *parms->goal;
      att = *parms->str;

      parms->me->ai_ifc()->get_goal_str(goal_id, att, script_return_string);
    }

    vm_str_t result = &script_return_string;

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_get_ai_goal_str_t slf_get_ai_goal_str(slc_entity,"get_ai_goal_str(str,str)");





class slf_set_ai_loco_num_t : public script_library_class::function
{
public:
  // constructor required
  slf_set_ai_loco_num_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_str_t str;
    vm_num_t num;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    if(parms->me->has_ai_ifc())
    {
      static pstring att;

      att = *parms->str;
      parms->me->ai_ifc()->set_loco_num(att, parms->num);
    }

    SLF_DONE;
  }
};
//slf_set_ai_loco_num_t slf_set_ai_loco_num(slc_entity,"set_ai_loco_num(str,num)");


class slf_get_ai_loco_num_t : public script_library_class::function
{
public:
  // constructor required
  slf_get_ai_loco_num_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_str_t str;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    vm_num_t result = 0.0f;
    if(parms->me->has_ai_ifc())
    {
      static pstring att;

      att = *parms->str;
      parms->me->ai_ifc()->get_loco_num(att, result);
    }

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_get_ai_loco_num_t slf_get_ai_loco_num(slc_entity,"get_ai_loco_num(str)");





class slf_set_time_dilation_t : public script_library_class::function
{
public:
  // constructor required
  slf_set_time_dilation_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_num_t num;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    if(!parms->me->has_time_ifc())
      parms->me->create_time_ifc();

    parms->me->time_ifc()->set_time_dilation(parms->num);

    SLF_DONE;
  }
};
//slf_set_time_dilation_t slf_set_time_dilation(slc_entity,"set_time_dilation(num)");


class slf_get_time_dilation_t : public script_library_class::function
{
public:
  // constructor required
  slf_get_time_dilation_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    vm_num_t result = g_time_dilation;
    if(parms->me->has_time_ifc())
      result = parms->me->time_ifc()->get_time_dilation();

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_get_time_dilation_t slf_get_time_dilation(slc_entity,"get_time_dilation()");






class slf_set_time_mode_t : public script_library_class::function
{
public:
  // constructor required
  slf_set_time_mode_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_num_t num;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    if(!parms->me->has_time_ifc())
      parms->me->create_time_ifc();

    parms->me->time_ifc()->set_mode((eTimeMode)((int)parms->num));

    SLF_DONE;
  }
};
//slf_set_time_mode_t slf_set_time_mode(slc_entity,"set_time_mode(num)");


class slf_get_time_mode_t : public script_library_class::function
{
public:
  // constructor required
  slf_get_time_mode_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    vm_num_t result = _TIME_WORLD;
    if(parms->me->has_time_ifc())
      result = parms->me->time_ifc()->get_mode();

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_get_time_mode_t slf_get_time_mode(slc_entity,"get_time_mode()");


class slf_set_ambient_factor_t : public script_library_class::function
{
public:
  // constructor required
  slf_set_ambient_factor_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vector3d col;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    light_manager *mgr = parms->me->get_light_set();
    if(mgr)
    {
      mgr->my_ambient.r = parms->col.x;
      mgr->my_ambient.g = parms->col.y;
      mgr->my_ambient.b = parms->col.z;
    }

    SLF_DONE;
  }
};
//slf_set_ambient_factor_t slf_set_ambient_factor(slc_entity,"set_ambient_factor(vector3d)");









class slf_create_script_data_interface_t : public script_library_class::function
{
public:
  // constructor required
  slf_create_script_data_interface_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

#ifdef ECULL
    if(!parms->me->has_script_data_ifc())
      parms->me->create_script_data_ifc();
#endif

    SLF_DONE;
  }
};
//slf_create_script_data_interface_t slf_create_script_data_interface(slc_entity,"create_script_data_interface()");


class slf_create_physical_interface_t : public script_library_class::function
{
public:
  // constructor required
  slf_create_physical_interface_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    if(!parms->me->has_physical_ifc())
    {
      parms->me->create_physical_ifc();
      parms->me->region_update_poss_active();

      if(parms->me->is_stationary())
      {
        // It seems like stationary is set to true no matter what it is in the scene file, so you might think
        // this warning isn't valid
        // but the stationary flag can be turned off in the scene export, and in the sin file.
        // so, yes this warning is valid, because if it is stationary it shares colgeom, which is bad for physical
        // interface...  - WB, JB, JF
//#pragma todo("Turn this warning back on after Soares is done filming.")
        warning("Entity '%s' is stationary. Please remove stationary status from the entity in the SC/SIN/SCN file.", parms->me->get_name().c_str());
        parms->me->set_stationary(false);
      }
    }

    SLF_DONE;
  }
};
//slf_create_physical_interface_t slf_create_physical_interface(slc_entity,"create_physical_interface()");

class slf_physical_ifc_apply_force_t : public script_library_class::function
{
public:
  // constructor required
  slf_physical_ifc_apply_force_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vector3d dir;
    vm_num_t force;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    if(parms->me->has_physical_ifc())
    {
      vector3d tmp = parms->dir;
      tmp.normalize();
      tmp *= parms->force;

      parms->me->physical_ifc()->apply_force_increment(tmp, physical_interface::INSTANT);
    }

    SLF_DONE;
  }
};
//slf_physical_ifc_apply_force_t slf_physical_ifc_apply_force(slc_entity,"physical_ifc_apply_force(vector3d,num)");


class slf_entity_ai_say_file_t: public script_library_class::function
{
public:
  // constructor required
  slf_entity_ai_say_file_t( script_library_class* slc, const char* n ) : script_library_class::function( slc, n ) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_str_t n;
	vm_num_t pri;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
	if ( parms->me->has_ai_ifc() )
		parms->me->ai_ifc()->get_voice()->say_file(*parms->n, parms->pri);
    SLF_DONE;
  }
};
//slf_entity_ai_say_file_t slf_entity_set_alternative_materials( slc_entity, "set_alternative_materials(str)" );

class slf_entity_ai_say_group_t: public script_library_class::function
{
public:
  // constructor required
  slf_entity_ai_say_group_t( script_library_class* slc, const char* n ) : script_library_class::function( slc, n ) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t me;
    vm_str_t n;
	vm_num_t pri;
  };
  // library function execution
  virtual bool operator()( vm_stack& stack, entry_t entry )
  {
    SLF_PARMS;
	if ( parms->me->has_ai_ifc() )
	{
		static pstring temp;
		temp = pstring (*parms->n);
		parms->me->ai_ifc()->get_voice()->say(temp, parms->pri);
	}
    SLF_DONE;
  }
};
//slf_entity_ai_say_group_t slf_entity_set_alternative_materials( slc_entity, "set_alternative_materials(str)" );



void register_entity2_lib(slc_entity_t* slc_entity)
{
  NEW slf_entity_wait_looping_anim_t(slc_entity, "wait_looping_anim(str,num,num)");
  NEW slf_entity_set_alternative_materials_t( slc_entity, "set_alternative_materials(str)" );
  NEW slf_entity_use_item_t( slc_entity, "use_item(str)" );
  NEW slf_entity_set_current_target_t( slc_entity, "set_current_target(entity)" );
  NEW slf_entity_set_current_target_pos_t( slc_entity, "set_current_target_pos(vector3d)" );
  NEW slf_entity_set_current_target_norm_t( slc_entity, "set_current_target_norm(vector3d)" );
  NEW slf_get_current_target_t(slc_entity, "get_current_target()");
  NEW slf_get_current_target_pos_t(slc_entity, "get_current_target_pos()");
  NEW slf_get_current_target_norm_t(slc_entity, "get_current_target_norm()");
  NEW slf_entity_get_last_item_used_t(slc_entity,"get_last_item_used()");
  NEW slf_entity_ai_say_file_t(slc_entity, "ai_say_file(str,num)");
  NEW slf_entity_ai_say_group_t(slc_entity, "ai_say_group(str,num)");
//  NEW slf_entity_connect_to_dread_net_t(slc_entity,"connect_to_dread_net()");
//  NEW slf_entity_disconnect_from_dread_net_t(slc_entity,"disconnect_from_dread_net()");
//  NEW slf_entity_is_connected_to_dread_net_t(slc_entity, "is_connected_to_dread_net()");
//  NEW slf_entity_see_hero_t(slc_entity, "see_hero()");
//  NEW slf_entity_ignore_next_cue_t(slc_entity, "ignore_next_cue()");
//  NEW slf_entity_get_current_cue_type_t(slc_entity, "get_current_cue_type()");
//  NEW slf_entity_get_next_cue_type_t(slc_entity, "get_next_cue_type()");
//  NEW slf_entity_get_current_cue_power_t(slc_entity, "get_current_cue_power()");
//  NEW slf_entity_get_next_cue_power_t(slc_entity, "get_next_cue_power()");
//  NEW slf_entity_get_current_cue_pos_t(slc_entity, "get_current_cue_pos()");
//  NEW slf_entity_get_next_cue_pos_t(slc_entity, "get_next_cue_pos()");
//  NEW slf_entity_get_current_cue_reached_t(slc_entity, "get_current_cue_reached()");
//  NEW slf_entity_set_los_defaults_t(slc_entity, "set_los_defaults()");
//  NEW slf_entity_set_cue_react_radius_defaults_t(slc_entity, "set_cue_react_radius_defaults()");
//  NEW slf_entity_set_min_cue_react_power_defaults_t(slc_entity, "set_min_cue_react_power_defaults()");
//  NEW slf_entity_set_los_params_t(slc_entity, "set_los_params(num,num,num,num)");
//  NEW slf_entity_set_cue_react_radius_t(slc_entity, "set_cue_react_radius(num,num)");
//  NEW slf_entity_set_min_cue_react_power_t(slc_entity, "set_min_cue_react_power(num,num)");
//  NEW slf_entity_get_reaction_state_t(slc_entity, "get_reaction_state()");
//  NEW slf_entity_get_min_reaction_state_t(slc_entity, "get_min_reaction_state()");
//  NEW slf_entity_get_max_reaction_state_t(slc_entity, "get_max_reaction_state()");
//  NEW slf_entity_set_reaction_state_t(slc_entity, "set_reaction_state(num)");
//  NEW slf_entity_set_min_reaction_state_t(slc_entity, "set_min_reaction_state(num)");
//  NEW slf_entity_set_max_reaction_state_t(slc_entity, "set_max_reaction_state(num)");
//  NEW slf_entity_inc_reaction_state_t(slc_entity, "inc_reaction_state()");
//  NEW slf_entity_dec_reaction_state_t(slc_entity, "dec_reaction_state()");
//  NEW slf_entity_set_reaction_ai_t(slc_entity, "set_reaction_ai(num,str)");
//  NEW slf_entity_set_guard_pos_t(slc_entity, "set_guard_pos(vector3d,vector3d)");
//  NEW slf_entity_set_reload_time_t(slc_entity, "set_reload_time(num)");
//  NEW slf_entity_set_reload_variance_t(slc_entity, "set_reload_variance(num)");
//  NEW slf_entity_get_reload_time_t(slc_entity, "get_reload_time()");
//  NEW slf_entity_get_reload_variance_t(slc_entity, "get_reload_variance()");
//  NEW slf_entity_set_patrol_id_t(slc_entity, "set_patrol_id(num)");
//  NEW slf_entity_get_patrol_id_t(slc_entity, "get_patrol_id()");
//  NEW slf_entity_link_to_dread_net_signaller_t(slc_entity,"link_to_dread_net_signaller(signaller)");
//  NEW slf_entity_unlink_from_dread_net_signaller_t(slc_entity,"unlink_from_dread_net_signaller(signaller)");
//  NEW slf_entity_push_state_investigate_t(slc_entity,"push_state_investigate(vector3d,num,num)");
//  NEW slf_entity_push_state_goto_t(slc_entity,"push_state_goto(vector3d,num,num)");
//  NEW slf_entity_push_state_path_goto_t(slc_entity,"push_state_path_goto(vector3d,num,num)");
//  NEW slf_entity_push_state_goto_hero_t(slc_entity,"push_state_goto_hero()");
//  NEW slf_entity_push_state_change_facing_t(slc_entity,"push_state_change_facing(vector3d)");
//  NEW slf_entity_get_state_info_t(slc_entity, "get_state_info()");
//  NEW slf_entity_force_pathing_t(slc_entity,"force_pathing()");
//  NEW slf_entity_unforce_pathing_t(slc_entity,"unforce_pathing()");
//  NEW slf_entity_set_path_graph_t(slc_entity,"set_path_graph(str)");
//  NEW slf_entity_get_path_graph_name_t(slc_entity, "get_path_graph_name()");
//  NEW slf_entity_add_cue_t(slc_entity,"add_cue(str)");
  NEW slf_entity_set_render_color_t(slc_entity,"set_render_color(vector3d)");
  NEW slf_entity_set_render_alpha_t(slc_entity,"set_render_alpha(num)");
  NEW slf_get_render_color_t(slc_entity, "get_render_color()");
  NEW slf_get_render_alpha_t(slc_entity, "get_render_alpha()");
  NEW slf_entity_wait_change_render_color_t(slc_entity,"wait_change_render_color(vector3d,num,num)");
  NEW slf_entity_wait_lookat_t(slc_entity,"wait_lookat(vector3d,num)");
  NEW slf_entity_wait_lookat2_t(slc_entity,"wait_lookat2(entity,entity,vector3d,num)");
  NEW slf_entity_set_scale_t(slc_entity,"set_scale(num)");
  NEW slf_entity_wait_set_scale_t(slc_entity,"wait_set_scale(num,num)");

  NEW slf_entity_get_ifc_num_t(slc_entity, "get_ifc_num(str)");
  NEW slf_entity_set_ifc_num_t(slc_entity, "set_ifc_num(str,num)");
  NEW slf_entity_get_ifc_vec_t(slc_entity, "get_ifc_vec(str)");
  NEW slf_entity_set_ifc_vec_t(slc_entity, "set_ifc_vec(str,vector3d)");
  NEW slf_entity_get_ifc_str_t(slc_entity, "get_ifc_str(str)");
  NEW slf_entity_set_ifc_str_t(slc_entity, "set_ifc_str(str,str)");

  NEW slf_ai_goto_t(slc_entity,"ai_goto(vector3d,num,num)");
  NEW slf_force_ai_goto_t(slc_entity,"force_ai_goto(vector3d,num,num)");
  NEW slf_set_ai_goal_num_t(slc_entity,"set_ai_goal_num(str,str,num)");
  NEW slf_get_ai_goal_num_t(slc_entity,"get_ai_goal_num(str,str)");
  NEW slf_set_ai_goal_str_t(slc_entity,"set_ai_goal_str(str,str,str)");
  NEW slf_get_ai_goal_str_t(slc_entity,"get_ai_goal_str(str,str)");
  NEW slf_set_ai_loco_num_t(slc_entity,"set_ai_loco_num(str,num)");
  NEW slf_get_ai_loco_num_t(slc_entity,"get_ai_loco_num(str)");

  NEW slf_set_time_dilation_t(slc_entity,"set_time_dilation(num)");
  NEW slf_get_time_dilation_t(slc_entity,"get_time_dilation()");
  NEW slf_set_time_mode_t(slc_entity,"set_time_mode(num)");
  NEW slf_get_time_mode_t(slc_entity,"get_time_mode()");

  NEW slf_set_ambient_factor_t(slc_entity,"set_ambient_factor(vector3d)");

  NEW slf_create_physical_interface_t(slc_entity,"create_physical_interface()");
  NEW slf_physical_ifc_apply_force_t(slc_entity,"physical_ifc_apply_force(vector3d,num)");

  NEW slf_create_script_data_interface_t(slc_entity,"create_script_data_interface()");
}

#endif
