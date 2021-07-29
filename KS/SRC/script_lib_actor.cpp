// script_lib_actor.cpp
//
// This file contains application-specific code and data that makes use of the
// Script Runtime Core to define script library classes and functions that
// provide the interface between the script language and the application.

#include "global.h"

#include "script_lib_actor.h"
//!#include "actor.h"
#include "script_lib_entity.h"
#include "vm_stack.h"
#include "vm_thread.h"


///////////////////////////////////////////////////////////////////////////////
// script library class: actor
///////////////////////////////////////////////////////////////////////////////

// pointer to single instance of library class
slc_actor_t* slc_actor = NEW slc_actor_t("actor",4,"entity");

// read an actor value (by id) from a stream
void slc_actor_t::read_value(chunk_file& fs,char* buf)
{
  // read id
  stringx id;
  serial_in(fs,&id);
  // find actor and write value to buffer
  *(vm_actor_t*)buf = (vm_actor_t)find_instance(id);
}

// find named instance of actor
unsigned slc_actor_t::find_instance(const stringx& n) const
{
  if (n=="NULL") return (unsigned)0;
  const entity* r = entity_manager::inst()->find_entity(entity_id(n.c_str()),IGNORE_FLAVOR,FIND_ENTITY_UNKNOWN_OK);
  if (!r)
    {
    error( "actor " + n + " not found" );
    }
  else if (r->get_flavor()!=ENTITY_PHYSICAL && r->get_flavor()!=ENTITY_ACTOR && r->get_flavor()!=ENTITY_CHARACTER)
    {
    error( "entity " + n + " is not of script class: actor" );
    }
  return (unsigned)r;
}


// global script library function: actor to_actor(entity)
class slf_to_actor_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_to_actor_t( const char* n )
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
      vm_actor_t result = NULL;
      if ( parms->e->is_an_actor() )
        result = static_cast<vm_actor_t>( parms->e );
      else
      {
        stack.get_thread()->slf_error( "to_actor(): " + parms->e->get_id().get_val() + " is not a actor" );
      }
      SLF_RETURN;
      SLF_DONE;
      }
  };
slf_to_actor_t slf_to_actor("to_actor(entity)");
