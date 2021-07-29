// script_lib_character.cpp
//
// This file contains application-specific code and data that makes use of the
// Script Runtime Core to define script library classes and functions that
// provide the interface between the script language and the application.

#include "global.h"

#include "script_lib_signal.h"
//!#include "script_lib_character.h"
#include "script_lib_entity.h"
#include "script_lib_item.h"
#include "script_lib_vector3d.h"
//!#include "character.h"
#include "vm_stack.h"
#include "vm_thread.h"
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


///////////////////////////////////////////////////////////////////////////////
// script library class: character
///////////////////////////////////////////////////////////////////////////////

// pointer to single instance of library class
slc_character_t* slc_character = NEW slc_character_t("character",4,"actor");

// read a character value (by id) from a stream
void slc_character_t::read_value(chunk_file& fs,char* buf)
  {
  // read id
  stringx id;
  serial_in(fs,&id);
  // find character and write value to buffer
  *(vm_character_t*)buf = (vm_character_t)find_instance(id);
  }

// find named instance of character
unsigned slc_character_t::find_instance(const stringx& n) const
  {
  if (n=="NULL") return (unsigned)0;
  const entity* r = entity_manager::inst()->find_entity(entity_id(n.c_str()),IGNORE_FLAVOR,FIND_ENTITY_UNKNOWN_OK);
  if (!r)
    {
    error( "character " + n + " not found" );
    }
  else if (r->get_flavor()!=ENTITY_CHARACTER)
    {
    error( "entity " + n + " is not a character" );
    }
  return (unsigned)r;
  }








// script library function:  character::get_limb(str)
class slf_character_get_limb_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_get_limb_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_character_t me;
      vm_str_t animid;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      int id;
      if ( (id=anim_id_manager::inst()->find_id( parms->animid->c_str() ))==NO_ID ||
          !parms->me->limb_valid(id))
        {
        stack.get_thread()->slf_error( "$" + parms->me->get_id().get_val() +
          ".get_limb('" + *parms->animid +"'): limb not found." );
        }
      limb_body * result = parms->me->limb_ptr(parms->animid->c_str())->get_body();
      SLF_RETURN;
      SLF_DONE;
      }
  };
slf_character_get_limb_t slf_character_get_limb(slc_character,"get_limb(str)");


// script library function:  character::get_nonlimb(str)
class slf_character_get_nonlimb_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_get_nonlimb_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_character_t me;
      vm_str_t animid;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      entity* result = parms->me->nonlimb_ptr( parms->animid->c_str() );
      if ( !result )
        {
        stack.get_thread()->slf_error( "$" + parms->me->get_id().get_val() +
          ".get_nonlimb('" + *parms->animid +"'): nonlimb not found." );
        }
      SLF_RETURN;
      SLF_DONE;
      }
  };
slf_character_get_nonlimb_t slf_character_get_nonlimb(slc_character,"get_nonlimb(str)");





// script library function:  character::get_attack_range()
class slf_character_get_attack_range_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_get_attack_range_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
      rational_t result = parms->me->get_attack_range();
      SLF_RETURN;
      SLF_DONE;
      }
  };
slf_character_get_attack_range_t slf_character_get_attack_range(slc_character,"get_attack_range()");



// script library function:  character::set_hit_points(num)
class slf_character_set_hit_points_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_set_hit_points_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_character_t me;
      vm_num_t hp;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      parms->me->set_hit_points(parms->hp);
      SLF_DONE;
      }
  };
slf_character_set_hit_points_t slf_character_set_hit_points(slc_character,"set_hit_points(num)");


// script library function:  character::get_hit_points()
class slf_character_get_hit_points_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_get_hit_points_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
      rational_t result = parms->me->get_soft_attrib()->get_hit_points();
      SLF_RETURN;
      SLF_DONE;
      }
  };
slf_character_get_hit_points_t slf_character_get_hit_points(slc_character,"get_hit_points()");


// script library function:  character::get_full_hit_points()
class slf_character_get_full_hit_points_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_get_full_hit_points_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
      rational_t result = parms->me->get_full_hit_points();
      SLF_RETURN;
      SLF_DONE;
      }
  };
slf_character_get_full_hit_points_t slf_character_get_full_hit_points(slc_character,"get_full_hit_points()");


// script library function:  character::set_armor_points(num)
class slf_character_set_armor_points_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_set_armor_points_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_character_t me;
      vm_num_t ap;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      parms->me->set_armor_points(parms->ap);
      SLF_DONE;
      }
  };
slf_character_set_armor_points_t slf_character_set_armor_points(slc_character,"set_armor_points(num)");


// script library function:  character::get_armor_points()
class slf_character_get_armor_points_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_get_armor_points_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
      rational_t result = parms->me->get_armor_points();
      SLF_RETURN;
      SLF_DONE;
      }
  };
slf_character_get_armor_points_t slf_character_get_armor_points(slc_character,"get_armor_points()");


// script library function:  character::get_full_armor_points()
class slf_character_get_full_armor_points_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_get_full_armor_points_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
      rational_t result = parms->me->get_full_armor_points();
      SLF_RETURN;
      SLF_DONE;
      }
  };
slf_character_get_full_armor_points_t slf_character_get_full_armor_points(slc_character,"get_full_armor_points()");


// script library function:  character::is_alive()
class slf_character_is_alive_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_is_alive_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
      rational_t result = parms->me->is_alive();
      SLF_RETURN;
      SLF_DONE;
      }
  };
slf_character_is_alive_t slf_character_is_alive(slc_character,"is_alive()");


// script library function:  character::is_dying()
class slf_character_is_dying_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_is_dying_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
      rational_t result = parms->me->is_dying();
      SLF_RETURN;
      SLF_DONE;
      }
  };
slf_character_is_dying_t slf_character_is_dying(slc_character,"is_dying()");


// script library function:  character::is_alive_or_dying()
class slf_character_is_alive_or_dying_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_is_alive_or_dying_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
      rational_t result = parms->me->is_alive_or_dying();
      SLF_RETURN;
      SLF_DONE;
      }
  };
slf_character_is_alive_or_dying_t slf_character_is_alive_or_dying(slc_character,"is_alive_or_dying()");


// script library function:  item character::get_cur_item();
class slf_character_get_cur_item_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_get_cur_item_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
      vm_item_t result = parms->me->get_cur_item();
      if ( result && result->get_count()==0 )
        result = NULL;
      SLF_RETURN;
      SLF_DONE;
      }
  };
slf_character_get_cur_item_t slf_character_get_cur_item(slc_character,"get_cur_item()");

// script library function:  num character::get_permanent_item_count();
class slf_character_get_permanent_item_count_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_get_permanent_item_count_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
      rational_t result = (rational_t)parms->me->get_permanent_item_count();
      SLF_RETURN;
      SLF_DONE;
      }
  };
slf_character_get_permanent_item_count_t slf_character_get_permanent_item_count(slc_character,"get_permanent_item_count()");

// script library function:  entity character::get_permanent_item(num n);
class slf_character_get_permanent_item_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_get_permanent_item_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_character_t me;
      rational_t     n;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_entity_t result = ((int)parms->n < parms->me->get_permanent_item_count()) ? ((vm_entity_t)parms->me->get_permanent_item((int)parms->n)) : NULL;
      SLF_RETURN;
      SLF_DONE;
      }
  };
slf_character_get_permanent_item_t slf_character_get_permanent_item(slc_character,"get_permanent_item(num)");

// script library function:  character::add_permanent_item(entity e);
class slf_character_add_permanent_item_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_add_permanent_item_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_character_t me;
      vm_entity_t item_ent;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      item *the_item = find_item(parms->item_ent->get_id());
      parms->me->add_permanent_item(the_item);
      SLF_DONE;
      }
  };
slf_character_add_permanent_item_t slf_character_add_permanent_item(slc_character,"add_permanent_item(entity)");

// script library function:  character::remove_permanent_item(entity e);
class slf_character_remove_permanent_item_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_remove_permanent_item_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_character_t me;
      vm_entity_t item_ent;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      item *the_item = find_item(parms->item_ent->get_id());
      parms->me->remove_permanent_item(the_item);
      SLF_DONE;
      }
  };
slf_character_remove_permanent_item_t slf_character_remove_permanent_item(slc_character,"remove_permanent_item(entity)");

// script library function:  entity character::get_permanent_item_by_name(str name);
class slf_character_get_permanent_item_by_name_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_get_permanent_item_by_name_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_character_t me;
      vm_str_t       name;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      entity *result = parms->me->get_permanent_item_by_name(*(parms->name));
      SLF_RETURN;
      SLF_DONE;
      }
  };
slf_character_get_permanent_item_by_name_t slf_character_get_permanent_item_by_name(slc_character,"get_permanent_item_by_name(str)");

// script library function:  character::get_nearest_opponent()
class slf_character_get_nearest_opponent_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_get_nearest_opponent_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
      character * result = parms->me->get_nearest_opponent();
      SLF_RETURN;
      SLF_DONE;
      }
  };
slf_character_get_nearest_opponent_t slf_character_get_nearest_opponent(slc_character,"get_nearest_opponent()");

///////////////////////////////////////////////////////////////////////////////
// script library class: char_group
///////////////////////////////////////////////////////////////////////////////

// pointer to single instance of library class
slc_char_group_t* slc_char_group = NEW slc_char_group_t("char_group",4);

// read a char_group (by id) from a stream
void slc_char_group_t::read_value(chunk_file& fs,char* buf)
  {
  // read id
  stringx id;
  serial_in(fs,&id);
  // find char_group and write value to buffer
  *(vm_char_group_t*)buf = (vm_char_group_t)find_instance(id);
  }

// find named instance of char_group
unsigned slc_char_group_t::find_instance(const stringx& n) const
  {
  const char_group* cg = char_group_manager::inst()->find(char_group_id(n),char_group_manager::FAIL_OK);
  if (!cg)
    {
    error( "char_group " + n + " not found" );
    }
  return (unsigned)cg;
  }


// script library function:  num char_group::size()
class slf_char_group_size_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_char_group_size_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
      vm_num_t result = parms->me->size();
      SLF_RETURN;
      SLF_DONE;
      }
  };
slf_char_group_size_t slf_char_group_size(slc_char_group,"size()");

// script library function:  char_group::add(character c)
class slf_char_group_add_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_char_group_add_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_char_group_t me;
      vm_character_t  c;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      parms->me->add(parms->c);
      SLF_DONE;
      }
  };
slf_char_group_add_t slf_char_group_add(slc_char_group,"add(character)");


// script library function:  num char_group::min_distance(vector3d)
class slf_char_group_min_distance_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_char_group_min_distance_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_char_group_t me;
      vector3d pos;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_num_t result = parms->me->min_distance(parms->pos);
      SLF_RETURN;
      SLF_DONE;
      }
  };
slf_char_group_min_distance_t slf_char_group_min_distance(slc_char_group,"min_distance(vector3d)");

// script library function:  num char_group::max_distance(vector3d)
class slf_char_group_max_distance_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_char_group_max_distance_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_char_group_t me;
      vector3d pos;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_num_t result = parms->me->max_distance(parms->pos);
      SLF_RETURN;
      SLF_DONE;
      }
  };
slf_char_group_max_distance_t slf_char_group_max_distance(slc_char_group,"max_distance(vector3d)");

// script library function:  num char_group::num_damaged()
class slf_char_group_num_damaged_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_char_group_num_damaged_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
      vm_num_t result = parms->me->num_damaged();
      SLF_RETURN;
      SLF_DONE;
      }
  };
slf_char_group_num_damaged_t slf_char_group_num_damaged(slc_char_group,"num_damaged()");

// script library function:  num char_group::num_alive()
class slf_char_group_num_alive_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_char_group_num_alive_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
      vm_num_t result = parms->me->num_alive();
      SLF_RETURN;
      SLF_DONE;
      }
  };
slf_char_group_num_alive_t slf_char_group_num_alive(slc_char_group,"num_alive()");


// script library function:  char_group_iterator char_group::begin()
class slf_char_group_begin_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_char_group_begin_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
      vm_char_group_iterator_t result = parms->me->begin();
      SLF_RETURN;
      SLF_DONE;
      }
  };
slf_char_group_begin_t slf_char_group_begin(slc_char_group,"begin()");


// script library function:  char_group_iterator char_group::end()
class slf_char_group_end_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_char_group_end_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
      vm_char_group_iterator_t result = parms->me->end();
      SLF_RETURN;
      SLF_DONE;
      }
  };
slf_char_group_end_t slf_char_group_end(slc_char_group,"end()");


// script library function:  char_group::suspend();
class slf_char_group_suspend_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_char_group_suspend_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
      parms->me->suspend();
      SLF_DONE;
      }
  };
slf_char_group_suspend_t slf_char_group_suspend(slc_char_group,"suspend()");


// script library function:  char_group::unsuspend();
class slf_char_group_unsuspend_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_char_group_unsuspend_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
      parms->me->unsuspend();
      SLF_DONE;
      }
  };
slf_char_group_unsuspend_t slf_char_group_unsuspend(slc_char_group,"unsuspend()");


////////////////////////////////////////////////////////////////////////////////
// script library class:  char_group_iterator
////////////////////////////////////////////////////////////////////////////////

// pointer to single instance of library class
slc_char_group_iterator_t* slc_char_group_iterator = NEW slc_char_group_iterator_t("char_group_iterator",sizeof(vm_char_group_iterator_t));


// script library function:  char_group_iterator char_group_iterator::operator++()
class slf_char_group_iterator_inc_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_char_group_iterator_inc_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_char_group_iterator_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_char_group_iterator_t result = ++parms->me;
      SLF_RETURN;
      SLF_DONE;
      }
  };
slf_char_group_iterator_inc_t slf_char_group_iterator_inc(slc_char_group_iterator,"operator++()");

// script library function:  num char_group_iterator::operator==(char_group_iterator)
class slf_char_group_iterator_is_equal_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_char_group_iterator_is_equal_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_char_group_iterator_t me;
      vm_char_group_iterator_t you;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_num_t result = parms->me==parms->you;
      SLF_RETURN;
      SLF_DONE;
      }
  };
slf_char_group_iterator_is_equal_t slf_char_group_iterator_is_equal(slc_char_group_iterator,"operator==(char_group_iterator)");

// script library function:  num char_group_iterator::operator!=(char_group_iterator)
class slf_char_group_iterator_not_equal_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_char_group_iterator_not_equal_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_char_group_iterator_t me;
      vm_char_group_iterator_t you;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_num_t result = parms->me!=parms->you;
      SLF_RETURN;
      SLF_DONE;
      }
  };
slf_char_group_iterator_not_equal_t slf_char_group_iterator_not_equal(slc_char_group_iterator,"operator!=(char_group_iterator)");

// script library function:  character char_group_iterator::get_char()
class slf_char_group_iterator_get_char_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_char_group_iterator_get_char_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_char_group_iterator_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_character_t result = *(parms->me);
      SLF_RETURN;
      SLF_DONE;
      }
  };
slf_char_group_iterator_get_char_t slf_char_group_iterator_get_char(slc_char_group_iterator,"get_char()");


// script library function:  character::force_active(num v);
class slf_character_force_active_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_force_active_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_character_t me;
      rational_t a;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      parms->me->force_active( parms->a );
      if ( parms->a && !parms->me->is_active() )
        {
        // if we're trying to force a guy active when he is not already so,
        // we have to wait a frame
        SLF_RECALL;
        }
      else
        {
        SLF_DONE;
        }
      }
  };
slf_character_force_active_t slf_character_force_active(slc_character,"force_active(num)");

// script library function:  character::unforce_active();
class slf_character_unforce_active_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_unforce_active_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
      parms->me->unforce_active();
      SLF_DONE;
      }
  };
slf_character_unforce_active_t slf_character_unforce_active(slc_character,"unforce_active()");

// script library function:  void character::set_soft_attrib_num(str name, num value);
class slf_character_set_soft_attrib_num_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_set_soft_attrib_num_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_character_t me;
      vm_str_t name;
      vm_num_t value;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      int idx = parms->me->get_soft_attrib()->get_idx(*parms->name);
      parms->me->get_soft_attrib()->set_rational_t(idx,parms->value);
      SLF_DONE;
      }
  };
slf_character_set_soft_attrib_num_t slf_character_set_soft_attrib_num(slc_character,"set_soft_attrib_num(str,num)");

// script library function:  void character::get_soft_attrib_num(str name);
class slf_character_get_soft_attrib_num_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_get_soft_attrib_num_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_character_t me;
      vm_str_t name;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      int idx = parms->me->get_soft_attrib()->get_idx(*parms->name);
      vm_num_t result = parms->me->get_soft_attrib()->get_rational_t(idx);
      SLF_RETURN;
      SLF_DONE;
      }
  };
slf_character_get_soft_attrib_num_t slf_character_get_soft_attrib_num(slc_character,"get_soft_attrib_num(str)");


// script library function:  void character::get_hard_attrib_num(str name);
class slf_character_get_hard_attrib_num_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_get_hard_attrib_num_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_character_t me;
      vm_str_t name;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      int idx = parms->me->get_hard_attrib()->get_idx(*parms->name);
      vm_num_t result = parms->me->get_hard_attrib()->get_rational_t(idx);
      SLF_RETURN;
      SLF_DONE;
      }
  };
slf_character_get_hard_attrib_num_t slf_character_get_hard_attrib_num(slc_character,"get_hard_attrib_num(str)");


// script library function:  void character::get_hard_attrib_str(str name);
class slf_character_get_hard_attrib_str_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_get_hard_attrib_str_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_character_t me;
      vm_str_t name;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      int idx = parms->me->get_hard_attrib()->get_idx(*parms->name);
      vm_str_t result = &parms->me->get_hard_attrib()->get_string(idx);
      SLF_RETURN;
      SLF_DONE;
      }
  };
slf_character_get_hard_attrib_str_t slf_character_get_hard_attrib_str(slc_character,"get_hard_attrib_str(str)");

//--------------------------------------------------------------
//  void pickup_ent(entity ent);
//  void drop_ent(vector3d here);

// script library function:  character::pickup_ent(entity e);
class slf_character_pickup_ent_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_pickup_ent_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_character_t me;
      vm_entity_t ent;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      const entity * ent = entity_manager::inst()->find_entity(parms->ent->get_id(),IGNORE_FLAVOR,FIND_ENTITY_UNKNOWN_OK);
      parms->me->pickup_ent(ent);
      SLF_DONE;
      }
  };
slf_character_pickup_ent_t slf_character_pickup_ent(slc_character,"pickup_ent(entity)");


// script library function:  character::heal(num)
class slf_character_heal_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_heal_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_character_t me;
      vm_num_t d;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      parms->me->heal(parms->d);
      SLF_DONE;
      }
  };
slf_character_heal_t slf_character_heal(slc_character,"heal(num)");


// script library function:  character::resurrect()
class slf_character_resurrect_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_resurrect_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
      parms->me->resurrect();
      SLF_DONE;
      }
  };
slf_character_resurrect_t slf_character_resurrect(slc_character,"resurrect()");

// script library function:  character::play_sound_group(str name);
class slf_character_play_sound_group_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_play_sound_group_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_character_t me;
      vm_str_t sound_name;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_num_t result = true;
      parms->me->play_sound_group(*parms->sound_name);
      SLF_RETURN;
      SLF_DONE;
      }
  };
slf_character_play_sound_group_t slf_character_play_sound_group(slc_character,"play_sound_group(str)");


// script library function:  character::play_sound_group_prioritized(str name, num pri);
class slf_character_play_sound_group_prioritized_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_play_sound_group_prioritized_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_character_t me;
      vm_str_t sound_name;
      vm_num_t sound_pri;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      parms->me->play_sound_group(*parms->sound_name,-1,parms->sound_pri);
      SLF_DONE;
      }
  };
slf_character_play_sound_group_prioritized_t slf_character_play_sound_prioritized_group(slc_character,"play_sound_group_prioritized(str,num)");


// script library function:  character::play_sound_group_prioritized_who(str name, num pri, str who);
class slf_character_play_sound_group_prioritized_who_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_play_sound_group_prioritized_who_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_character_t me;
      vm_str_t sound_name;
      vm_num_t sound_pri;
      vm_str_t sound_who;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      parms->me->play_sound_group(*parms->sound_name,-1,parms->sound_pri,parms->sound_who);
      SLF_DONE;
      }
  };
slf_character_play_sound_group_prioritized_who_t slf_character_play_sound_prioritized_who_group(slc_character,"play_sound_group_prioritized_who(str,num,str)");


// script library function:  void character::paralyze()
class slf_character_paralyze_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_paralyze_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
      parms->me->paralyze();
      SLF_DONE;
      }
  };
slf_character_paralyze_t slf_character_paralyze(slc_character,"paralyze()");


// script library function:  void character::unparalyze()
class slf_character_unparalyze_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_unparalyze_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
      parms->me->unparalyze();
      SLF_DONE;
      }
  };
slf_character_unparalyze_t slf_character_unparalyze(slc_character,"unparalyze()");


// script library function:  character::I_win();
class slf_character_I_win_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_I_win_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
      parms->me->I_win();
      SLF_DONE;
      }
  };
slf_character_I_win_t slf_character_I_win(slc_character,"I_win()");

// script library function:  character::I_lose();
class slf_character_I_lose_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_I_lose_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
      parms->me->I_lose();
      SLF_DONE;
      }
  };
slf_character_I_lose_t slf_character_I_lose(slc_character,"I_lose()");


// script library function:  character::malor();
class slf_character_malor_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_malor_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
      if (!(parms->me->get_id().get_val()=="HERO"))
        {
        stack.get_thread()->slf_error( "$" + parms->me->get_id().get_val() +
          ".malor(): tried to 'malor' non-hero; try 'adjust_position' instead");
        }
      g_world_ptr->malor();
      SLF_DONE;
      }
  };
slf_character_malor_t slf_character_malor(slc_character,"malor()");


// script library function:  character::adjust_position(vector3d);
class slf_character_adjust_position_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_adjust_position_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_character_t me;
      vector3d pos;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      parms->me->adjust_position(parms->pos);
      SLF_DONE;
      }
  };
slf_character_adjust_position_t slf_character_adjust_position(slc_character,"adjust_position(vector3d)");



// script library function:  character::force_look_at(vector3d v);
class slf_character_force_look_at_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_force_look_at_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_character_t me;
      vector3d v;
      vm_entity_t e;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      parms->me->set_forced_look_at_target(parms->v, parms->e);
      SLF_DONE;
      }
  };
slf_character_force_look_at_t slf_character_force_look_at(slc_character,"force_look_at(vector3d,entity)");


// script library function:  character::free_look_at();
class slf_character_free_look_at_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_free_look_at_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
      parms->me->free_look_at();
      SLF_DONE;
      }
  };
slf_character_free_look_at_t slf_character_free_look_at(slc_character,"free_look_at()");


// script library function:  character::get_last_enemy_hit()
class slf_character_get_last_enemy_hit_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_get_last_enemy_hit_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
      vm_character_t result = parms->me->get_last_enemy_hit();
      SLF_RETURN;
      SLF_DONE;
      }
  };
slf_character_get_last_enemy_hit_t slf_character_get_last_enemy_hit(slc_character,"get_last_enemy_hit()");


// script library function:  character::set_death_fade(num)
class slf_character_set_death_fade_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_set_death_fade_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_character_t me;
      vm_num_t t;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      parms->me->set_death_fade( parms->t );
      SLF_DONE;
      }
  };
slf_character_set_death_fade_t slf_character_set_death_fade(slc_character,"set_death_fade(num)");


///////////////////////////////////////////////////////////////////////////////
// Functions that suspend a thread waiting for a condition to be met
///////////////////////////////////////////////////////////////////////////////

// script library function:  character::wait_for_damage()
class slf_character_wait_for_damage_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_wait_for_damage_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
      if ( parms->me->get_soft_attrib()->get_hit_points() < parms->me->get_full_hit_points() )
        SLF_DONE;
      else
        SLF_RECALL;
      }
  };
slf_character_wait_for_damage_t slf_character_wait_for_damage(slc_character,"wait_for_damage()");

// script library function:  character::wait_for_death()
class slf_character_wait_for_death_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_wait_for_death_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
      if ( parms->me->get_soft_attrib()->get_hit_points() <= 0 )
        SLF_DONE;
      else
        SLF_RECALL;
      }
  };
slf_character_wait_for_death_t slf_character_wait_for_death(slc_character,"wait_for_death()");

// script library function:  char_group::wait_for_damage()
class slf_char_group_wait_for_damage_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_char_group_wait_for_damage_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
      if ( parms->me->num_damaged() > 0 )
        SLF_DONE;
      else
        SLF_RECALL;
      }
  };
slf_char_group_wait_for_damage_t slf_char_group_wait_for_damage(slc_char_group,"wait_for_damage()");

// script library function:  char_group::wait_for_all_dead()
class slf_char_group_wait_for_all_dead_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_char_group_wait_for_all_dead_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
      if ( parms->me->num_alive() == 0 )
        SLF_DONE;
      else
        SLF_RECALL;
      }
  };
slf_char_group_wait_for_all_dead_t slf_char_group_wait_for_all_dead(slc_char_group,"wait_for_all_dead()");

// global script library function:  wait_prox_or_damaged( entity e1, char_group g1, num r );
// wait until e1 is within range of g1
class slf_wait_prox_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_wait_prox_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_entity_t e1;
      vm_char_group_t g1;
      vm_num_t r;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      if ( parms->g1->min_distance2(parms->e1->get_abs_position()) < parms->r*parms->r )
        SLF_DONE;
      else
        SLF_RECALL;
      }
  };
slf_wait_prox_t slf_wait_prox("wait_prox(entity,char_group,num)");

// global script library function:  wait_prox_or_damaged( entity e1, vector3d p, num r, character c1 );
// wait until e1 is within sphere or c1 is damaged
class slf_wait_prox_or_damaged1_t : public script_library_class::function
{
  public:
    // constructor required
    slf_wait_prox_or_damaged1_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_entity_t e1;
      vector3d p;
      vm_num_t r;
      vm_character_t c1;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_PARMS;
      if ( (parms->e1->get_abs_position()-parms->p).length2() < parms->r*parms->r ||
           parms->c1->get_soft_attrib()->get_hit_points() < parms->c1->get_full_hit_points() )
        SLF_DONE;
      else
        SLF_RECALL;
    }
};
slf_wait_prox_or_damaged1_t slf_wait_prox_or_damaged1("wait_prox_or_damaged(entity,vector3d,num,character)");

// global script library function:  wait_prox_or_damaged( entity e1, vector3d p, num r, char_group g1 );
// wait until e1 is within sphere or g1 is damaged
class slf_wait_prox_or_damaged2_t : public script_library_class::function
{
  public:
    // constructor required
    slf_wait_prox_or_damaged2_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_entity_t e1;
      vector3d p;
      vm_num_t r;
      vm_char_group_t g1;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_PARMS;
      if ( (parms->e1->get_abs_position()-parms->p).length2() < parms->r*parms->r ||
           parms->g1->num_damaged() > 0 )
        SLF_DONE;
      else
        SLF_RECALL;
    }
};
slf_wait_prox_or_damaged2_t slf_wait_prox_or_damaged2("wait_prox_or_damaged(entity,vector3d,num,char_group)");

// global script library function:  wait_prox_or_damaged( entity e1, entity e2, num r, character c1 );
// wait until e1 is within range of e2 or c1 is damaged
class slf_wait_prox_or_damaged3_t : public script_library_class::function
{
  public:
    // constructor required
    slf_wait_prox_or_damaged3_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_entity_t e1;
      vm_entity_t e2;
      vm_num_t r;
      vm_character_t c1;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_PARMS;
      if ( (parms->e1->get_abs_position()-parms->e2->get_abs_position()).length2() < parms->r*parms->r ||
           parms->c1->get_soft_attrib()->get_hit_points() < parms->c1->get_full_hit_points() )
        SLF_DONE;
      else
        SLF_RECALL;
    }
};
slf_wait_prox_or_damaged3_t slf_wait_prox_or_damaged3("wait_prox_or_damaged(entity,entity,num,character)");

// global script library function:  wait_prox_or_damaged( entity e1, vector3d p1, num r1, vector3d p2, num r2, character c1 );
// wait until e1 is within a sphere or c1 is damaged
class slf_wait_prox_or_damaged4_t : public script_library_class::function
{
  public:
    // constructor required
    slf_wait_prox_or_damaged4_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_entity_t e1;
      vector3d p1;
      vm_num_t r1;
      vector3d p2;
      vm_num_t r2;
      vm_character_t c1;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_PARMS;
      if ( (parms->e1->get_abs_position()-parms->p1).length2() < parms->r1*parms->r1 ||
           (parms->e1->get_abs_position()-parms->p2).length2() < parms->r2*parms->r2 ||
           parms->c1->get_soft_attrib()->get_hit_points() < parms->c1->get_full_hit_points() )
        SLF_DONE;
      else
        SLF_RECALL;
    }
};
slf_wait_prox_or_damaged4_t slf_wait_prox_or_damaged4("wait_prox_or_damaged(entity,vector3d,num,vector3d,num,character)");

// global script library function:  wait_prox_or_damaged_or_dead( entity e1, entity e2, num r, character c1, character c2 );
// wait until e1 is within range of e2 or c1 is damaged or c2 is dead
class slf_wait_prox_or_damaged_or_dead1_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_wait_prox_or_damaged_or_dead1_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_entity_t e1;
      vm_entity_t e2;
      vm_num_t r;
      vm_character_t c1;
      vm_character_t c2;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      if ( (parms->e1->get_abs_position()-parms->e2->get_abs_position()).length2() < parms->r*parms->r ||
           parms->c1->get_soft_attrib()->get_hit_points() < parms->c1->get_full_hit_points() ||
           parms->c2->get_soft_attrib()->get_hit_points() <= 0 )
        SLF_DONE;
      else
        SLF_RECALL;
      }
  };
slf_wait_prox_or_damaged_or_dead1_t slf_wait_prox_or_damaged_or_dead1("wait_prox_or_damaged_or_dead(entity,entity,num,character,character)");

// global script library function:  wait_prox_or_damaged_or_dead( entity e1, entity e2, num r, character c1, char_group g1 );
// wait until e1 is within range of e2 or c1 is damaged or g1 is all dead
class slf_wait_prox_or_damaged_or_dead2_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_wait_prox_or_damaged_or_dead2_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_entity_t e1;
      vm_entity_t e2;
      vm_num_t r;
      vm_character_t c1;
      vm_char_group_t g1;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      if ( (parms->e1->get_abs_position()-parms->e2->get_abs_position()).length2() < parms->r*parms->r ||
           parms->c1->get_soft_attrib()->get_hit_points() < parms->c1->get_full_hit_points() ||
           parms->g1->num_alive() == 0 )
        SLF_DONE;
      else
        SLF_RECALL;
      }
  };
slf_wait_prox_or_damaged_or_dead2_t slf_wait_prox_or_damaged_or_dead2("wait_prox_or_damaged_or_dead(entity,entity,num,character,char_group)");

// global script library function:  wait_prox_or_damaged_or_dead( entity e1, char_group g1, num r, char_group g2, character c1 );
// wait until e1 is within range of g1 or g2 is damaged or c1 is dead
class slf_wait_prox_or_damaged_or_dead3_t : public script_library_class::function
{
  public:
    // constructor required
    slf_wait_prox_or_damaged_or_dead3_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_entity_t e1;
      vm_char_group_t g1;
      vm_num_t r;
      vm_char_group_t g2;
      vm_character_t c1;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_PARMS;
      if ( parms->g1->min_distance2(parms->e1->get_abs_position()) < parms->r*parms->r ||
           parms->g2->num_damaged() > 0 ||
           parms->c1->get_soft_attrib()->get_hit_points() <= 0 )
        SLF_DONE;
      else
        SLF_RECALL;
    }
};
slf_wait_prox_or_damaged_or_dead3_t slf_wait_prox_or_damaged_or_dead3("wait_prox_or_damaged_or_dead(entity,char_group,num,char_group,character)");

// global script library function:  wait_prox_or_damaged_or_dead( entity e1, char_group g1, num r, char_group g2, char_group g3 );
// wait until e1 is within range of g1 or g2 is damaged or g3 is all dead
class slf_wait_prox_or_damaged_or_dead4_t : public script_library_class::function
{
  public:
    // constructor required
    slf_wait_prox_or_damaged_or_dead4_t(const char* n) : script_library_class::function(n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_entity_t e1;
      vm_char_group_t g1;
      vm_num_t r;
      vm_char_group_t g2;
      vm_char_group_t g3;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_PARMS;
      if ( parms->g1->min_distance2(parms->e1->get_abs_position()) < parms->r*parms->r ||
           parms->g2->num_damaged() > 0 ||
           parms->g3->num_alive() == 0 )
        SLF_DONE;
      else
        SLF_RECALL;
    }
};
slf_wait_prox_or_damaged_or_dead4_t slf_wait_prox_or_damaged_or_dead4("wait_prox_or_damaged_or_dead(entity,char_group,num,char_group,char_group)");




// script library function:  character::switch_variant(str variant_name)
class slf_character_switch_variant_t : public script_library_class::function
{
  public:
    // constructor required
    slf_character_switch_variant_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
    {
      // parameters
      vm_character_t me;
      vm_str_t name;
    };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
    {
      SLF_PARMS;
      parms->me->switch_variant(*parms->name);
      SLF_DONE;
    }
};
slf_character_switch_variant_t slf_character_switch_variant(slc_character,"switch_variant(str)");

// script library function:  character::enable_lod(num on_off)
class slf_character_enable_lod_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_enable_lod_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_character_t me;
      vm_num_t on_off;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;

      if(parms->on_off)
        parms->me->set_flag(EFLAG_MISC_NO_LOD, 0);
      else
        parms->me->set_flag(EFLAG_MISC_NO_LOD, 1);

      SLF_DONE;
      }
  };
slf_character_enable_lod_t slf_character_enable_lod(slc_character,"enable_lod(num)");


// script library function:  character::is_standing()
class slf_character_is_standing_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_character_is_standing_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
      vm_num_t result = parms->me->is_effectively_standing();
      SLF_RETURN;
      SLF_DONE;
      }
  };
slf_character_is_standing_t slf_character_is_standing(slc_character,"is_standing()");


// CTT 02/29/00: TEMPORARY:
// The following script functions represent quick kludge solutions to early design needs:
//   character::get_current_target();
//   num character::get_ammo_points();
//   character::set_ammo_points( num );
/*
// script library function:  character character::get_current_target()
class slf_character_get_current_target_t : public script_library_class::function
{
public:
  // constructor required
  slf_character_get_current_target_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
    vm_entity_t result = parms->me->get_current_target();
    SLF_RETURN;
    SLF_DONE;
  }
};
slf_character_get_current_target_t slf_character_get_current_target(slc_character,"get_current_target()");

// script library function:  vector3d character::get_current_target_pos()
class slf_character_get_current_target_pos_t : public script_library_class::function
{
public:
  // constructor required
  slf_character_get_current_target_pos_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
    vm_vector3d_t result = parms->me->get_current_target_pos();
    SLF_RETURN;
    SLF_DONE;
  }
};
slf_character_get_current_target_pos_t slf_character_get_current_target_pos(slc_character,"get_current_target_pos()");

// script library function:  vector3d character::get_current_target_norm()
class slf_character_get_current_target_norm_t : public script_library_class::function
{
public:
  // constructor required
  slf_character_get_current_target_norm_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
    vm_vector3d_t result = parms->me->get_current_target_norm();
    SLF_RETURN;
    SLF_DONE;
  }
};
slf_character_get_current_target_norm_t slf_character_get_current_target_norm(slc_character,"get_current_target_norm()");
*/
// script library function:  num character::get_ammo_points()
class slf_character_get_ammo_points_t : public script_library_class::function
{
public:
  // constructor required
  slf_character_get_ammo_points_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
    vm_num_t result = parms->me->get_ammo_points();
    SLF_RETURN;
    SLF_DONE;
  }
};
slf_character_get_ammo_points_t slf_character_get_ammo_points(slc_character,"get_ammo_points()");

// script library function:  character::set_ammo_points(num)
class slf_character_set_ammo_points_t : public script_library_class::function
{
public:
  // constructor required
  slf_character_set_ammo_points_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_character_t me;
    vm_num_t ap;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    parms->me->set_ammo_points( parms->ap );
    SLF_DONE;
  }
};
slf_character_set_ammo_points_t slf_character_set_ammo_points(slc_character,"set_ammo_points(num)");

// script library function:  num character::is_stealth()
class slf_character_is_stealth_t : public script_library_class::function
{
public:
  // constructor required
  slf_character_is_stealth_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
    vm_num_t result = parms->me->is_stealth();
    SLF_RETURN;
    SLF_DONE;
  }
};
slf_character_is_stealth_t slf_character_is_stealth(slc_character,"is_stealth()");

// script library function:  num character::set_stealth(num)
class slf_character_set_stealth_t : public script_library_class::function
{
public:
  // constructor required
  slf_character_set_stealth_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_character_t me;
    vm_num_t ap;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_num_t result = parms->me->set_stealth( parms->ap );
    SLF_RETURN;
    SLF_DONE;
  }
};
slf_character_set_stealth_t slf_character_set_stealth(slc_character,"set_stealth(num)");

// script library function:  character::set_stealth_timer()
class slf_character_set_stealth_timer_t : public script_library_class::function
{
public:
  // constructor required
  slf_character_set_stealth_timer_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
    parms->me->set_stealth_timer();
    SLF_DONE;
  }
};
slf_character_set_stealth_timer_t slf_character_set_stealth_timer(slc_character,"set_stealth_timer()");


// script library function:  num character::is_turbo()
class slf_character_is_turbo_t : public script_library_class::function
{
public:
  // constructor required
  slf_character_is_turbo_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
    vm_num_t result = parms->me->is_turbo();
    SLF_RETURN;
    SLF_DONE;
  }
};
slf_character_is_turbo_t slf_character_is_turbo(slc_character,"is_turbo()");

// script library function:  num character::set_turbo(num)
class slf_character_set_turbo_t : public script_library_class::function
{
public:
  // constructor required
  slf_character_set_turbo_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_character_t me;
    vm_num_t ap;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_num_t result = parms->me->set_turbo( parms->ap );
    SLF_RETURN;
    SLF_DONE;
  }
};
slf_character_set_turbo_t slf_character_set_turbo(slc_character,"set_turbo(num)");

// script library function:  character::set_turbo_timer()
class slf_character_set_turbo_timer_t : public script_library_class::function
{
public:
  // constructor required
  slf_character_set_turbo_timer_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
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
    parms->me->set_turbo_timer();
    SLF_DONE;
  }
};
slf_character_set_turbo_timer_t slf_character_set_turbo_timer(slc_character,"set_turbo_timer()");


// global script library function:  hero_set_action_go_to_range( const vector3d& target, rational_t range )
class slf_hero_set_action_go_to_range_t : public script_library_class::function
{
public:
  // constructor required
  slf_hero_set_action_go_to_range_t(const char* n) : script_library_class::function(n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vector3d tgt;
    rational_t range;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
//!    g_world_ptr->get_hero_ptr()->set_action_go_to_range(parms->tgt,parms->range);
    SLF_DONE;
  }
};
slf_hero_set_action_go_to_range_t slf_hero_set_action_go_to_range("hero_set_action_go_to_range(vector3d,num)");

// global script library function: character to_character(entity)
class slf_to_character_t : public script_library_class::function
{
public:
  // constructor required
  slf_to_character_t( const char* n )
    :   script_library_class::function( n )
  {
  }
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_t e;
  };
  // library function execution
  virtual bool operator()( vm_stack &stack, entry_t entry )
  {
    SLF_PARMS;
    vm_character_t result = NULL;
    if ( parms->e->is_a_character() )
      result = static_cast<vm_character_t>( parms->e );
    else
      stack.get_thread()->slf_error( "to_character(): " + parms->e->get_id().get_val() + " is not a character" );
    SLF_RETURN;
    SLF_DONE;
  }
};
slf_to_character_t slf_to_character("to_character(entity)");






