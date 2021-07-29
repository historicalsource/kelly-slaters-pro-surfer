// script_lib_entity_widget.cpp
//
// This file contains application-specific code and data that makes use of the
// Script Runtime Core to define script library classes and functions that
// provide the interface between the script language and the application.

#include "global.h"

#include "script_lib_entity.h"
#include "script_lib_entity_widget.h"
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
#include "widget_entity.h"


extern game *g_game_ptr;





// global script library function: entity_widget create_entity_widget(str)
class slf_create_entity_widget_t : public script_library_class::function
{
public:
  // constructor required
  slf_create_entity_widget_t(const char* n): script_library_class::function(n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
		vm_str_t filename;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;
    widget::set_rhw_3d_layer( widget::RHW3 );
		vm_entity_widget_t result = NEW entity_widget( "script ent widg", (widget*)(g_game_ptr->get_script_widget_holder()), 0, 0, parms->filename->c_str() );
    widget::restore_last_rhw_3d_layer();
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_create_entity_widget_t slf_create_entity_widget("create_entity_widget(str)");





// script library function:  entity_widget::get_entity()
class slf_entity_widget_get_entity_t : public script_library_class::function
{
public:
  // constructor required
  slf_entity_widget_get_entity_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_entity_widget_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_entity_t result = parms->me->get_ent();
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_entity_widget_get_entity_t slf_entity_widget_get_entity(slc_entity_widget,"get_entity()");


void register_entity_widget_lib()
{
  // pointer to single instance of library class
  slc_entity_widget_t* slc_entity_widget = NEW slc_entity_widget_t("entity_widget",4,"widget");

  NEW slf_create_entity_widget_t("create_entity_widget(str)");
  NEW slf_entity_widget_get_entity_t(slc_entity_widget,"get_entity()");
}
