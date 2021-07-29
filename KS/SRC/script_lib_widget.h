// script_lib_widget.h
#ifndef _SCRIPT_LIB_WIDGET_H
#define _SCRIPT_LIB_WIDGET_H

#include "script_library_class.h"



class widget;
class timer_widget;
class text_block_widget;
class bitmap_widget;
class background_widget;
class fluid_bar;


///////////////////////////////////////////////////////////////////////////////
// script library class: widget
///////////////////////////////////////////////////////////////////////////////

class slc_widget_t : public script_library_class
  {
  public:
    // constructor required
    slc_widget_t(const char* n,int sz,const char* p=NULL) : script_library_class(n,sz,p) {}
  };

// vm_stack data representation
typedef widget* vm_widget_t;

// pointer to single instance of library class
extern slc_widget_t* slc_widget;


///////////////////////////////////////////////////////////////////////////////
// script library class: timer_widget
///////////////////////////////////////////////////////////////////////////////

class slc_timer_widget_t : public script_library_class
  {
  public:
    // constructor required
    slc_timer_widget_t(const char* n,int sz,const char* p=NULL) : script_library_class(n,sz,p) {}
  };

// vm_stack data representation
typedef timer_widget* vm_timer_widget_t;

// pointer to single instance of library class
extern slc_timer_widget_t* slc_timer_widget;


///////////////////////////////////////////////////////////////////////////////
// script library class: text_block_widget
///////////////////////////////////////////////////////////////////////////////

class slc_text_block_widget_t : public script_library_class
  {
  public:
    // constructor required
    slc_text_block_widget_t(const char* n,int sz,const char* p=NULL) : script_library_class(n,sz,p) {}
  };

// vm_stack data representation
typedef text_block_widget* vm_text_block_widget_t;

// pointer to single instance of library class
extern slc_text_block_widget_t* slc_text_block_widget;


///////////////////////////////////////////////////////////////////////////////
// script library class: bitmap_widget
///////////////////////////////////////////////////////////////////////////////

class slc_bitmap6_widget_t : public script_library_class
  {
  public:
    // constructor required
    slc_bitmap6_widget_t(const char* n,int sz,const char* p=NULL) : script_library_class(n,sz,p) {}
  };

// vm_stack data representation
typedef background_widget* vm_bitmap6_widget_t;

// pointer to single instance of library class
extern slc_bitmap6_widget_t* slc_bitmap6_widget;



///////////////////////////////////////////////////////////////////////////////
// script library class: bitmap_widget
///////////////////////////////////////////////////////////////////////////////

class slc_bitmap_widget_t : public script_library_class
  {
  public:
    // constructor required
    slc_bitmap_widget_t(const char* n,int sz,const char* p=NULL) : script_library_class(n,sz,p) {}
  };

// vm_stack data representation
typedef bitmap_widget* vm_bitmap_widget_t;

// pointer to single instance of library class
extern slc_bitmap_widget_t* slc_bitmap_widget;


///////////////////////////////////////////////////////////////////////////////
// script library class: fluid_bar_widget
///////////////////////////////////////////////////////////////////////////////

class slc_fluid_bar_widget_t : public script_library_class
  {
  public:
    // constructor required
    slc_fluid_bar_widget_t(const char* n,int sz,const char* p=NULL) : script_library_class(n,sz,p) {}
  };

// vm_stack data representation
typedef fluid_bar* vm_fluid_bar_widget_t;

// pointer to single instance of library class
extern slc_fluid_bar_widget_t* slc_fluid_bar_widget;

#endif  // _SCRIPT_LIB_WIDGET_H
