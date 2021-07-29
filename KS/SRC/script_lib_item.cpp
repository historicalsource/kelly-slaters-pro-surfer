// script_lib_item.cpp
//
// This file contains application-specific code and data that makes use of the
// Script Runtime Core to define script library classes and functions that
// provide the interface between the script language and the application.

#include "global.h"

#include "script_lib_item.h"
#include "item.h"
// BIGCULL #include "handheld_item.h"
// BIGCULL #include "thrown_item.h"
#include "script_lib_entity.h"
#include "vm_stack.h"
#include "vm_thread.h"
#include "wds.h"
#include "entityflags.h"
// BIGCULL #include "gun.h"
#include "entity_maker.h"


///////////////////////////////////////////////////////////////////////////////
// script library class: item
///////////////////////////////////////////////////////////////////////////////

// read a item value (by id) from a stream
void slc_item_t::read_value( chunk_file& fs, char* buf )
  {
  // read id
  stringx id;
  serial_in(fs,&id);
  // find item and write value to buffer
  *(vm_item_t*)buf = (vm_item_t)find_instance(id);
  }

// find named instance of item
unsigned slc_item_t::find_instance( const stringx& n ) const
  {
  if ( n == "NULL" )
    return (unsigned)0;
  const entity* r = entity_manager::inst()->find_entity( entity_id(n.c_str()), IGNORE_FLAVOR, FIND_ENTITY_UNKNOWN_OK );
  if ( !r )
    error( "item " + n + " not found\n" );
  else if ( r->get_flavor() != ENTITY_ITEM )
    error( "entity " + n + " is not a item\n" );
  return (unsigned)r;
  }


// script library function:  item::set_count( num count );
class slf_item_set_count_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_item_set_count_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
      {
      }
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_item_t me;
      vm_num_t count;
      };
    // library function execution
    virtual bool operator()( vm_stack& stack, entry_t entry )
      {
      SLF_PARMS;
      parms->me->set_count( parms->count );
      SLF_DONE;
      }
  };
//slf_item_set_count_t slf_item_set_count(slc_item,"set_count(num)");


// script library function:  num item::get_count();
class slf_item_get_count_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_item_get_count_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
      {
      }
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_item_t me;
      };
    // library function execution
    virtual bool operator()( vm_stack& stack, entry_t entry )
      {
      SLF_PARMS;
      vm_num_t result = parms->me->get_count();
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_item_get_count_t slf_item_get_count(slc_item,"get_count()");






#if 0 // BIGCULL

// script library function:  item::set_launch_force( num launch_force );
class slf_item_set_launch_force_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_item_set_launch_force_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
      {
      }
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_item_t me;
      vm_num_t launch_force;
      };
    // library function execution
    virtual bool operator()( vm_stack& stack, entry_t entry )
      {
      SLF_PARMS;

      assert(parms->me->is_a_thrown_item());
      ((thrown_item *)parms->me)->set_launch_force( parms->launch_force );

      SLF_DONE;
      }
  };
//slf_item_set_launch_force_t slf_item_set_launch_force(slc_item,"set_launch_force(num)");


// script library function:  num item::get_launch_force();
class slf_item_get_launch_force_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_item_get_launch_force_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
      {
      }
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_item_t me;
      };
    // library function execution
    virtual bool operator()( vm_stack& stack, entry_t entry )
      {
      SLF_PARMS;

      assert(parms->me->is_a_thrown_item());
      vm_num_t result = ((thrown_item *)parms->me)->get_launch_force();

      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_item_get_launch_force_t slf_item_get_launch_force(slc_item,"get_launch_force()");


// script library function:  item::set_launch_vec( vector3d launch_vec );
class slf_item_set_launch_vec_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_item_set_launch_vec_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
      {
      }
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_item_t me;
      vector3d launch_vec;
      };
    // library function execution
    virtual bool operator()( vm_stack& stack, entry_t entry )
      {
      SLF_PARMS;

      assert(parms->me->is_a_thrown_item());
      ((thrown_item *)parms->me)->set_launch_vec( parms->launch_vec );

      SLF_DONE;
      }
  };
//slf_item_set_launch_vec_t slf_item_set_launch_vec(slc_item,"set_launch_vec(vector3d)");


// script library function:  vector3d item::get_launch_vec();
class slf_item_get_launch_vec_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_item_get_launch_vec_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
      {
      }
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_item_t me;
      };
    // library function execution
    virtual bool operator()( vm_stack& stack, entry_t entry )
      {
      SLF_PARMS;

      assert(parms->me->is_a_thrown_item());
      vector3d result = ((thrown_item *)parms->me)->get_launch_vec();

      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_item_get_launch_vec_t slf_item_get_launch_vec(slc_item,"get_launch_vec()");


// script library function:  num item::get_visual_item();
class slf_item_get_visual_item_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_item_get_visual_item_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
      {
      }
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_item_t me;
      };
    // library function execution
    virtual bool operator()( vm_stack& stack, entry_t entry )
      {
      SLF_PARMS;

      assert(parms->me->is_a_handheld_item() && ((handheld_item *)parms->me)->get_visual_item() != NULL);
      vm_entity_t result = ((handheld_item *)parms->me)->get_visual_item();

      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_item_get_visual_item_t slf_item_get_visual_item(slc_item,"get_visual_item()");


// script library function:  num item::get_last_grenade_spawned();
class slf_item_get_last_grenade_spawned_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_item_get_last_grenade_spawned_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
      {
      }
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_item_t me;
      };
    // library function execution
    virtual bool operator()( vm_stack& stack, entry_t entry )
      {
      SLF_PARMS;

      assert(parms->me->is_a_thrown_item() && ((thrown_item *)parms->me)->get_last_grenade_spawned() != NULL);
      vm_entity_t result = ((thrown_item *)parms->me)->get_last_grenade_spawned();

      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_item_get_last_grenade_spawned_t slf_item_get_last_grenade_spawned(slc_item,"get_last_grenade_spawned()");


// script library function:  num item::get_last_grenade_armed();
class slf_item_get_last_grenade_armed_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_item_get_last_grenade_armed_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
      {
      }
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_item_t me;
      };
    // library function execution
    virtual bool operator()( vm_stack& stack, entry_t entry )
      {
      SLF_PARMS;

      assert(parms->me->is_a_thrown_item() && ((thrown_item *)parms->me)->get_last_grenade_armed() != NULL);
      vm_entity_t result = ((thrown_item *)parms->me)->get_last_grenade_armed();

      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_item_get_last_grenade_armed_t slf_item_get_last_grenade_armed(slc_item,"get_last_grenade_armed()");


// script library function:  num item::get_last_grenade_detonated();
class slf_item_get_last_grenade_detonated_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_item_get_last_grenade_detonated_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
      {
      }
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_item_t me;
      };
    // library function execution
    virtual bool operator()( vm_stack& stack, entry_t entry )
      {
      SLF_PARMS;

      assert(parms->me->is_a_thrown_item() && ((thrown_item *)parms->me)->get_last_grenade_detonated() != NULL);
      vm_entity_t result = ((thrown_item *)parms->me)->get_last_grenade_detonated();

      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_item_get_last_grenade_detonated_t slf_item_get_last_grenade_detonated(slc_item,"get_last_grenade_detonated()");


// script library function:  item::set_drawn_limb( str limb );
class slf_item_set_drawn_limb_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_item_set_drawn_limb_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
      {
      }
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_item_t me;
      vm_str_t limb;
      };
    // library function execution
    virtual bool operator()( vm_stack& stack, entry_t entry )
      {
      SLF_PARMS;

      assert(parms->me->is_a_handheld_item());
      ((handheld_item *)parms->me)->set_drawn_limb_name( *parms->limb );
      ((handheld_item *)parms->me)->draw();

      SLF_DONE;
      }
  };
//slf_item_set_drawn_limb_t slf_item_set_drawn_limb(slc_item,"set_drawn_limb(str)");




// script library function:  item::use();
class slf_item_use_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_item_use_t( script_library_class* slc, const char* n )
    :   script_library_class::function( slc, n )
      {
      }
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_item_t me;
      };
    // library function execution
    virtual bool operator()( vm_stack& stack, entry_t entry )
      {
      SLF_PARMS;

      assert(parms->me->is_a_handheld_item());
      ((handheld_item *)parms->me)->apply_effects( ((handheld_item *)parms->me)->get_owner() );

      SLF_DONE;
      }
  };
//slf_item_use_t slf_item_use(slc_item,"use()");


#endif // BIGCULL




///////////////////////////////////////////////////////////////////////////////
// Global script library functions for class item
///////////////////////////////////////////////////////////////////////////////

// global script library function: item create_item( str name )
class slf_create_item_t : public script_library_class::function
{
public:
  // constructor required
  slf_create_item_t( const char* n ) : script_library_class::function(n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_str_t name;
  };
  // library function execution
  virtual bool operator ()( vm_stack &stack, entry_t entry )
  {
    SLF_PARMS;
    entity* result;
    stringx entity_name = get_fname_wo_ext( *parms->name );
    entity_name.to_upper();
    stringx entity_dir = get_dir( *parms->name );
    result = g_entity_maker->create_entity_or_subclass( entity_name,
                                                  entity_id::make_unique_id(),
                                                  po_identity_matrix,
                                                  entity_dir,
                                                  ACTIVE_FLAG | NONSTATIC_FLAG );
    if ( result->get_flavor() != ENTITY_ITEM )
    {
      stack.get_thread()->slf_error( ": entity " + entity_name + " is not an item" );
    }
    result->set_created_entity_default_active_status();
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_create_item_t slf_create_item("create_item(str)");


// global script library function: item to_item(entity)
class slf_to_item_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_to_item_t( const char* n )
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
      vm_item_t result = NULL;
      if ( parms->e->is_an_item() )
        result = static_cast<vm_item_t>( parms->e );
      else
        stack.get_thread()->slf_error( "to_item(): " + parms->e->get_id().get_val() + " is not a item" );
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_to_item_t slf_to_item("to_item(entity)");





void register_item_lib()
{
  // pointer to single instance of library class
  slc_item_t* slc_item = NEW slc_item_t("item",4,"entity");

  NEW slf_item_set_count_t(slc_item,"set_count(num)");
  NEW slf_item_get_count_t(slc_item,"get_count()");
//BIGCULL  NEW slf_item_set_launch_force_t(slc_item,"set_launch_force(num)");
//BIGCULL  NEW slf_item_get_launch_force_t(slc_item,"get_launch_force()");
//BIGCULL  NEW slf_item_set_launch_vec_t(slc_item,"set_launch_vec(vector3d)");
  //BIGCULLNEW slf_item_get_launch_vec_t(slc_item,"get_launch_vec()");
//BIGCULL  NEW slf_item_get_visual_item_t(slc_item,"get_visual_item()");
  //BIGCULLNEW slf_item_get_last_grenade_spawned_t(slc_item,"get_last_grenade_spawned()");
  //BIGCULLNEW slf_item_get_last_grenade_armed_t(slc_item,"get_last_grenade_armed()");
  //BIGCULLNEW slf_item_get_last_grenade_detonated_t(slc_item,"get_last_grenade_detonated()");
//BIGCULL  NEW slf_item_set_drawn_limb_t(slc_item,"set_drawn_limb(str)");
//BIGCULL  NEW slf_item_use_t(slc_item,"use()");
  NEW slf_create_item_t("create_item(str)");
  NEW slf_to_item_t("to_item(entity)");
}
