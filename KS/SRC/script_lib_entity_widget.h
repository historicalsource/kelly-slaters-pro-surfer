// script_lib_entity_widget.h
#ifndef _SCRIPT_LIB_ENTITY_WIDGET_H
#define _SCRIPT_LIB_ENTITY_WIDGET_H

#include "script_library_class.h"


class entity_widget;


///////////////////////////////////////////////////////////////////////////////
// script library class: entity_widget
///////////////////////////////////////////////////////////////////////////////

class slc_entity_widget_t : public script_library_class
  {
  public:
    // constructor required
    slc_entity_widget_t(const char* n,int sz,const char* p=NULL) : script_library_class(n,sz,p) {}
  };

// vm_stack data representation
typedef entity_widget* vm_entity_widget_t;

// pointer to single instance of library class
extern slc_entity_widget_t* slc_entity_widget;

#endif  // _SCRIPT_LIB_ENTITY_WIDGET_H
