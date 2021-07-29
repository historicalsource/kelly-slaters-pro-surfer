// script_lib_widget.cpp
//
// This file contains application-specific code and data that makes use of the
// Script Runtime Core to define script library classes and functions that
// provide the interface between the script language and the application.

#include "global.h"

#include "script_lib_widget.h"
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
#include "widget.h"
#include "widget_script.h"
#include "interface.h"


extern game* g_game_ptr;


///////////////////////////////////////////////////////////////////////////////
// script library class: widget
///////////////////////////////////////////////////////////////////////////////


// script library function:  widget::show()
class slf_widget_show_t : public script_library_class::function
{
public:
  // constructor required
  slf_widget_show_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_widget_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    parms->me->show();

    SLF_DONE;
  }
};
//slf_widget_show_t slf_widget_show(slc_widget,"show()");



// script library function:  widget::hide()
class slf_widget_hide_t : public script_library_class::function
{
public:
  // constructor required
  slf_widget_hide_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_widget_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    parms->me->hide();

    SLF_DONE;
  }
};
//slf_widget_hide_t slf_widget_hide(slc_widget,"hide()");





// script library function:  widget::move_to(num,num)
class slf_widget_move_to_t : public script_library_class::function
{
public:
  // constructor required
  slf_widget_move_to_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_widget_t me;
    vm_num_t x;
    vm_num_t y;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    parms->me->move_to(parms->x, parms->y);

    SLF_DONE;
  }
};
//slf_widget_move_to_t slf_widget_move_to(slc_widget,"move_to(num,num)");



// script library function:  widget::move_to(num,num,num,num)
class slf_widget_move_to2_t : public script_library_class::function
{
public:
  // constructor required
  slf_widget_move_to2_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_widget_t me;
    vm_num_t wt;
    vm_num_t dur;
    vm_num_t x;
    vm_num_t y;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    parms->me->move_to(parms->wt, parms->dur, parms->x, parms->y);

    SLF_DONE;
  }
};
//slf_widget_move_to2_t slf_widget_move_to2(slc_widget,"move_to(num,num,num,num)");



// script library function:  widget::set_layer(num)
class slf_widget_set_layer_t : public script_library_class::function
{
public:
  // constructor required
  slf_widget_set_layer_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_widget_t me;
    vm_num_t layer;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    int layer = (int)parms->layer;
    if(layer < widget::RHW0)
      layer = widget::RHW0;
    if(layer >= widget::NUM_RHW_LAYERS)
      layer = widget::NUM_RHW_LAYERS-1;

    parms->me->set_layer((widget::rhw_layer_e)layer);

    SLF_DONE;
  }
};
//slf_widget_set_layer_t slf_widget_set_layer(slc_widget,"set_layer(num)");




// script library function:  widget::scale_to(num)
class slf_widget_scale_to_t : public script_library_class::function
{
public:
  // constructor required
  slf_widget_scale_to_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_widget_t me;
    vm_num_t val;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    parms->me->scale_to(parms->val, parms->val);

    SLF_DONE;
  }
};
//slf_widget_scale_to_t slf_widget_scale_to(slc_widget,"scale_to(num)");



// script library function:  widget::scale_to(num,num,num)
class slf_widget_scale_to2_t : public script_library_class::function
{
public:
  // constructor required
  slf_widget_scale_to2_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_widget_t me;
    vm_num_t wt;
    vm_num_t dur;
    vm_num_t val;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    parms->me->scale_to(parms->wt, parms->dur, parms->val, parms->val);

    SLF_DONE;
  }
};
//slf_widget_scale_to2_t slf_widget_scale_to2(slc_widget,"scale_to(num,num,num)");




// script library function:  widget::fade_to(num)
class slf_widget_fade_to_t : public script_library_class::function
{
public:
  // constructor required
  slf_widget_fade_to_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_widget_t me;
    vm_num_t alpha;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    parms->me->fade_to(parms->alpha);

    SLF_DONE;
  }
};
//slf_widget_fade_to_t slf_widget_fade_to(slc_widget,"fade_to(num)");


// script library function:  widget::fade_to(num,num,num)
class slf_widget_fade_to2_t : public script_library_class::function
{
public:
  // constructor required
  slf_widget_fade_to2_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_widget_t me;
    vm_num_t wt;
    vm_num_t dur;
    vm_num_t alpha;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    parms->me->fade_to(parms->wt, parms->dur, parms->alpha);

    SLF_DONE;
  }
};
//slf_widget_fade_to2_t slf_widget_fade_to2(slc_widget,"fade_to(num,num,num)");



///////////////////////////////////////////////////////////////////////////////
// script library class: timer_widget
///////////////////////////////////////////////////////////////////////////////
// global script library function: timer_widget create_timer_widget()
class slf_create_timer_widget_t : public script_library_class::function
{
public:
  // constructor required
  slf_create_timer_widget_t(const char* n): script_library_class::function(n) { }
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
//    widget::set_rhw_2d_layer( widget::RHW0 );
//		vm_timer_widget_t result = NEW timer_widget( "script timer widget", g_game_ptr->get_script_widget_holder(), 0, 0 );
//    widget::restore_last_rhw_2d_layer();
    vm_timer_widget_t result = g_game_ptr->get_script_widget_holder()->get_timer_widget();
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_create_timer_widget_t slf_create_timer_widget("create_timer_widget()");


// script library function:  timer_widget::get_time_left()
class slf_timer_widget_get_time_left_t : public script_library_class::function
{
public:
  // constructor required
  slf_timer_widget_get_time_left_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_timer_widget_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_num_t result = parms->me->get_time_left();
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_timer_widget_get_time_left_t slf_timer_widget_get_time_left(slc_timer_widget,"get_time_left()");


// script library function:  timer_widget::set_time_left(num)
class slf_timer_widget_set_time_left_t : public script_library_class::function
{
public:
  // constructor required
  slf_timer_widget_set_time_left_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_timer_widget_t me;
    vm_num_t time_left;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    parms->me->set_time_left(parms->time_left);
    SLF_DONE;
  }
};
//slf_timer_widget_set_time_left_t slf_timer_widget_set_time_left(slc_timer_widget,"set_time_left(num)");


// script library function:  timer_widget::freeze()
class slf_timer_widget_freeze_t : public script_library_class::function
{
public:
  // constructor required
  slf_timer_widget_freeze_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_timer_widget_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    parms->me->freeze();
    SLF_DONE;
  }
};
//slf_timer_widget_freeze_t slf_timer_widget_freeze(slc_timer_widget,"freeze()");


// script library function:  timer_widget::run()
class slf_timer_widget_run_t : public script_library_class::function
{
public:
  // constructor required
  slf_timer_widget_run_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_timer_widget_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    parms->me->run();
    SLF_DONE;
  }
};
//slf_timer_widget_run_t slf_timer_widget_run(slc_timer_widget,"run()");


// script library function:  timer_widget::inc_time_left(num)
class slf_timer_widget_inc_time_left_t : public script_library_class::function
{
public:
  // constructor required
  slf_timer_widget_inc_time_left_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_timer_widget_t me;
    vm_num_t time_left_delta;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    parms->me->inc_time_left(parms->time_left_delta);
    SLF_DONE;
  }
};
//slf_timer_widget_inc_time_left_t slf_timer_widget_inc_time_left(slc_timer_widget,"inc_time_left(num)");

// script library function:  timer_widget::set_digit_color(num,num,num,num)
class slf_timer_widget_set_digit_color_t : public script_library_class::function
{
public:
  // constructor required
  slf_timer_widget_set_digit_color_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_timer_widget_t me;
    vm_num_t r;
    vm_num_t g;
    vm_num_t b;
    vm_num_t a;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    parms->me->set_digit_color(color(parms->r, parms->g, parms->b, parms->a));
    SLF_DONE;
  }
};
//slf_timer_widget_set_digit_color_t slf_timer_widget_set_digit_color(slc_timer_widget,"set_digit_color(num,num,num,num)");

// script library function:  timer_widget::set_bg_color(num,num,num,num)
class slf_timer_widget_set_bg_color_t : public script_library_class::function
{
public:
  // constructor required
  slf_timer_widget_set_bg_color_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_timer_widget_t me;
    vm_num_t r;
    vm_num_t g;
    vm_num_t b;
    vm_num_t a;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    parms->me->set_bg_color(color(parms->r, parms->g, parms->b, parms->a));
    SLF_DONE;
  }
};
//slf_timer_widget_set_bg_color_t slf_timer_widget_set_bg_color(slc_timer_widget,"set_bg_color(num,num,num,num)");


/*
// script library function:  timer_widget::use_red_nums()
class slf_timer_widget_use_red_nums_t : public script_library_class::function
{
public:
  // constructor required
  slf_timer_widget_use_red_nums_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_timer_widget_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    parms->me->use_red_nums();
    SLF_DONE;
  }
};
//slf_timer_widget_use_red_nums_t slf_timer_widget_use_red_nums(slc_timer_widget,"use_red_nums()");



// script library function:  timer_widget::use_green_nums()
class slf_timer_widget_use_green_nums_t : public script_library_class::function
{
public:
  // constructor requiwhite
  slf_timer_widget_use_green_nums_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_timer_widget_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    parms->me->use_green_nums();
    SLF_DONE;
  }
};
//slf_timer_widget_use_green_nums_t slf_timer_widget_use_green_nums(slc_timer_widget,"use_green_nums()");
*/

// script library function:  timer_widget::add_function(str,num)
class slf_timer_widget_add_function_t : public script_library_class::function
{
public:
  // constructor requiwhite
  slf_timer_widget_add_function_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_timer_widget_t me;
    vm_str_t func;
    vm_num_t time;
  };

  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    parms->me->add_script_function(*parms->func, parms->time);
    SLF_DONE;
  }
};
//slf_timer_widget_add_function_t slf_timer_widget_add_function(slc_timer_widget,"add_function(str,num)");

// script library function:  timer_widget::remove_function(num,num)
class slf_timer_widget_remove_function_t : public script_library_class::function
{
public:
  // constructor requiwhite
  slf_timer_widget_remove_function_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_timer_widget_t me;
    vm_num_t start;
    vm_num_t end;
  };

  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    parms->me->remove_script_function(parms->start, parms->end);
    SLF_DONE;
  }
};
//slf_timer_widget_remove_function_t slf_timer_widget_remove_function(slc_timer_widget,"remove_function(num,num)");

// script library function:  timer_widget::clear_functions()
class slf_timer_widget_clear_functions_t : public script_library_class::function
{
public:
  // constructor requiwhite
  slf_timer_widget_clear_functions_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_timer_widget_t me;
    vm_num_t start;
    vm_num_t end;
  };

  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    parms->me->remove_script_function(-1.0f, FLT_MAX);
    SLF_DONE;
  }
};
//slf_timer_widget_clear_functions_t slf_timer_widget_clear_functions(slc_timer_widget,"clear_functions()");










///////////////////////////////////////////////////////////////////////////////
// script library class: text_block_widget
///////////////////////////////////////////////////////////////////////////////
// global script library function: text_block_widget create_text_block_widget(str,str)
class slf_create_text_block_widget_t : public script_library_class::function
{
public:
  // constructor required
  slf_create_text_block_widget_t(const char* n): script_library_class::function(n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
		vm_str_t font;
		vm_str_t string;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    text_block_widget::block_info_t info;
    info.typeface = *parms->font;
    info.line_spacing = 20;
    info.max_lines = 5;
    info.max_width = 620;
    info.col = color( 1.0f, 1.0f, 1.0f, 1.0f );
    info.text = *parms->string;
    info.break_substring = empty_string;

    widget::set_rhw_2d_layer( widget::RHW0 );
		vm_text_block_widget_t result = NEW text_block_widget( "script text_block widg", (widget*)(g_game_ptr->get_script_widget_holder()), 0, 0, &info );
    widget::restore_last_rhw_2d_layer();

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_create_text_block_widget_t slf_create_text_block_widget("create_text_block_widget(str,str)");

// script library function:  text_block_widget::set_text(str)
class slf_text_block_widget_set_text_t : public script_library_class::function
{
public:
  // constructor requiwhite
  slf_text_block_widget_set_text_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_text_block_widget_t me;
    vm_str_t text;
  };

  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    parms->me->set_text(*parms->text);

    SLF_DONE;
  }
};
//slf_text_block_widget_set_text_t slf_text_block_widget_set_text(slc_text_block_widget,"set_text(str)");

// script library function:  text_block_widget::set_scale(num)
class slf_text_block_widget_set_scale_t : public script_library_class::function
{
public:
  // constructor requiwhite
  slf_text_block_widget_set_scale_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_text_block_widget_t me;
    vm_num_t val;
  };

  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

/*
    text_block_widget::block_info_t info = parms->me->get_block_info();
    info.scale = parms->val;

    parms->me->set_block_info(&info);
*/
    parms->me->set_scale(parms->val);
    SLF_DONE;
  }
};
//slf_text_block_widget_set_scale_t slf_text_block_widget_set_scale(slc_text_block_widget,"set_scale(num)");


// script library function:  text_block_widget::set_max_lines(num)
class slf_text_block_widget_set_max_lines_t : public script_library_class::function
{
public:
  // constructor requiwhite
  slf_text_block_widget_set_max_lines_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_text_block_widget_t me;
    vm_num_t val;
  };

  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    text_block_widget::block_info_t info = parms->me->get_block_info();
    info.max_lines = (uint8)parms->val;

    parms->me->set_block_info(&info);

    SLF_DONE;
  }
};
//slf_text_block_widget_set_max_lines_t slf_text_block_widget_set_max_lines(slc_text_block_widget,"set_max_lines(num)");


// script library function:  text_block_widget::set_line_spacing(num)
class slf_text_block_widget_set_line_spacing_t : public script_library_class::function
{
public:
  // constructor requiwhite
  slf_text_block_widget_set_line_spacing_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_text_block_widget_t me;
    vm_num_t val;
  };

  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

/*
    text_block_widget::block_info_t info = parms->me->get_block_info();
    info.line_spacing = parms->val;

    parms->me->set_block_info(&info);
*/
    parms->me->set_line_spacing(parms->val);

    SLF_DONE;
  }
};
//slf_text_block_widget_set_line_spacing_t slf_text_block_widget_set_line_spacing(slc_text_block_widget,"set_line_spacing(num)");


// script library function:  text_block_widget::set_max_width(num)
class slf_text_block_widget_set_max_width_t : public script_library_class::function
{
public:
  // constructor requiwhite
  slf_text_block_widget_set_max_width_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_text_block_widget_t me;
    vm_num_t val;
  };

  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    text_block_widget::block_info_t info = parms->me->get_block_info();
    info.max_width = (short)parms->val;

    parms->me->set_block_info(&info);

    SLF_DONE;
  }
};
//slf_text_block_widget_set_max_width_t slf_text_block_widget_set_max_width(slc_text_block_widget,"set_max_width(num)");


// script library function:  text_block_widget::set_color(vector3d,num)
class slf_text_block_widget_set_color_t : public script_library_class::function
{
public:
  // constructor requiwhite
  slf_text_block_widget_set_color_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_text_block_widget_t me;
    vector3d col;
    vm_num_t alpha;
  };

  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

/*
    text_block_widget::block_info_t info = parms->me->get_block_info();
    info.col = color(parms->col.x, parms->col.y, parms->col.z, parms->alpha);

    parms->me->set_block_info(&info);
*/
    parms->me->set_color(color(parms->col.x, parms->col.y, parms->col.z, parms->alpha));

    SLF_DONE;
  }
};
//slf_text_block_widget_set_color_t slf_text_block_widget_set_color(slc_text_block_widget,"set_color(vector3d,num)");






///////////////////////////////////////////////////////////////////////////////
// script library class: bitmap_widget
///////////////////////////////////////////////////////////////////////////////
// global script library function: bitmap_widget create_bitmap_widget(str,str)
class slf_create_bitmap_widget_t : public script_library_class::function
{
public:
  // constructor required
  slf_create_bitmap_widget_t(const char* n): script_library_class::function(n) { }
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

    widget::set_rhw_2d_layer( widget::RHW0 );
		vm_bitmap_widget_t result = NEW bitmap_widget( "script bitmap widg", (widget*)(g_game_ptr->get_script_widget_holder()), 0, 0 );
    result->open(parms->filename->c_str());
    widget::restore_last_rhw_2d_layer();

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_create_bitmap_widget_t slf_create_bitmap_widget("create_bitmap_widget(str)");

// script library function:  bitmap_widget::set_color(vector3d,num)
class slf_bitmap_widget_set_color_t : public script_library_class::function
{
public:
  // constructor requiwhite
  slf_bitmap_widget_set_color_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_bitmap_widget_t me;
    vector3d col;
  };

  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    parms->me->set_color(parms->col.x, parms->col.y, parms->col.z);

    SLF_DONE;
  }
};
//slf_bitmap_widget_set_color_t slf_text_block_widget_set_color(slc_bitmap_widget,"set_color(vector3d,num)");




// script library function:  bitmap_widget::flip_horiz()
class slf_bitmap_widget_flip_horiz_t : public script_library_class::function
{
public:
  // constructor requiwhite
  slf_bitmap_widget_flip_horiz_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_bitmap_widget_t me;
  };

  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    parms->me->flip_horiz();

    SLF_DONE;
  }
};
//slf_bitmap_widget_flip_horiz_t slf_text_block_widget_flip_horiz(slc_bitmap_widget,"flip_horiz()");




// script library function:  bitmap_widget::flip_vert()
class slf_bitmap_widget_flip_vert_t : public script_library_class::function
{
public:
  // constructor requiwhite
  slf_bitmap_widget_flip_vert_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_bitmap_widget_t me;
  };

  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    parms->me->flip_vert();

    SLF_DONE;
  }
};
//slf_bitmap_widget_flip_vert_t slf_text_block_widget_flip_vert(slc_bitmap_widget,"flip_vert()");




// script library function:  bitmap_widget::resize(num,num)
class slf_bitmap_widget_resize_t : public script_library_class::function
{
public:
  // constructor requiwhite
  slf_bitmap_widget_resize_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_bitmap_widget_t me;
    vm_num_t w;
    vm_num_t h;
  };

  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    parms->me->resize(parms->w, parms->h);

    SLF_DONE;
  }
};
//slf_bitmap_widget_resize_t slf_text_block_widget_resize(slc_bitmap_widget,"resize(num,num)");




// script library function:  bitmap_widget::set_subrect(num,num,num,num)
class slf_bitmap_widget_set_subrect_t : public script_library_class::function
{
public:
  // constructor requiwhite
  slf_bitmap_widget_set_subrect_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_bitmap_widget_t me;
    vm_num_t l;
    vm_num_t t;
    vm_num_t r;
    vm_num_t b;
  };

  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    parms->me->set_subrect(parms->l, parms->t, parms->r, parms->b);

    SLF_DONE;
  }
};
//slf_bitmap_widget_set_subrect_t slf_text_block_widget_set_subrect(slc_bitmap_widget,"set_subrect(num,num,num,num)");


// script library function:  bitmap_widget::get_width()
class slf_bitmap_widget_get_width_t : public script_library_class::function
{
public:
  // constructor required
  slf_bitmap_widget_get_width_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_bitmap_widget_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_num_t result = parms->me->get_width();
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_bitmap_widget_get_width_t slf_bitmap_widget_get_width(slc_bitmap_widget,"get_width()");


// script library function:  bitmap_widget::get_height()
class slf_bitmap_widget_get_height_t : public script_library_class::function
{
public:
  // constructor required
  slf_bitmap_widget_get_height_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_bitmap_widget_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_num_t result = parms->me->get_height();
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_bitmap_widget_get_height_t slf_bitmap_widget_get_height(slc_bitmap_widget,"get_height()");









///////////////////////////////////////////////////////////////////////////////
// script library class: bitmap6_widget
///////////////////////////////////////////////////////////////////////////////
// global script library function: bitmap6_widget create_bitmap6_widget(str,str)
class slf_create_bitmap6_widget_t : public script_library_class::function
{
public:
  // constructor required
  slf_create_bitmap6_widget_t(const char* n): script_library_class::function(n) { }
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

    widget::set_rhw_2d_layer( widget::RHW0 );
		vm_bitmap6_widget_t result = NEW background_widget( "script bitmap6 widg", (widget*)(g_game_ptr->get_script_widget_holder()), 0, 0, *parms->filename );
    widget::restore_last_rhw_2d_layer();

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_create_bitmap6_widget_t slf_create_bitmap6_widget("create_bitmap6_widget(str)");




///////////////////////////////////////////////////////////////////////////////
// script library class: fluid_bar_widget
///////////////////////////////////////////////////////////////////////////////
// global script library function: fluid_bar_widget create_fluid_bar_widget(num)
class slf_create_fluid_bar_widget_t : public script_library_class::function
{
public:
  // constructor required
  slf_create_fluid_bar_widget_t(const char* n): script_library_class::function(n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
		vm_num_t dir;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS;

    widget::set_rhw_2d_layer( widget::RHW0 );

    widget::widget_dir_e dir = widget::WDIR_Right;
    switch((int)parms->dir)
    {
      case 0:
        dir = widget::WDIR_Left;
        break;

      case 1:
        dir = widget::WDIR_Right;
        break;

      case 2:
        dir = widget::WDIR_Up;
        break;

      case 3:
        dir = widget::WDIR_Down;
        break;
    }

    vm_fluid_bar_widget_t result = NEW fluid_bar( "script fluid_bar widg", (widget*)(g_game_ptr->get_script_widget_holder()), 0, 0, dir, 40, 10, os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR)+"\\alpha" );

    widget::restore_last_rhw_2d_layer();

    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_create_fluid_bar_widget_t slf_create_fluid_bar_widget("create_fluid_bar_widget(str)");

// script library function:  fluid_bar_widget::set_color(vector3d,num)
class slf_fluid_bar_widget_set_color_t : public script_library_class::function
{
public:
  // constructor requiwhite
  slf_fluid_bar_widget_set_color_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_fluid_bar_widget_t me;
    vector3d col;
  };

  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    parms->me->set_color(parms->col.x, parms->col.y, parms->col.z);

    SLF_DONE;
  }
};
//slf_fluid_bar_widget_set_color_t slf_text_block_widget_set_color(slc_fluid_bar_widget,"set_color(vector3d,num)");




// script library function:  fluid_bar_widget::resize(num,num)
class slf_fluid_bar_widget_resize_t : public script_library_class::function
{
public:
  // constructor requiwhite
  slf_fluid_bar_widget_resize_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_fluid_bar_widget_t me;
    vm_num_t w;
    vm_num_t h;
  };

  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    parms->me->resize(parms->w, parms->h);

    SLF_DONE;
  }
};
//slf_fluid_bar_widget_resize_t slf_text_block_widget_resize(slc_fluid_bar_widget,"resize(num,num)");




// script library function:  fluid_bar_widget::set_full_val(num)
class slf_fluid_bar_widget_set_full_val_t : public script_library_class::function
{
public:
  // constructor requiwhite
  slf_fluid_bar_widget_set_full_val_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_fluid_bar_widget_t me;
    vm_num_t v;
  };

  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    parms->me->set_full_val(parms->v);

    SLF_DONE;
  }
};
//slf_bitmap_widget_set_full_val_t slf_text_block_widget_set_full_val(slc_bitmap_widget,"set_full_val(num)");


// script library function:  fluid_bar_widget::set_val(num)
class slf_fluid_bar_widget_set_val_t : public script_library_class::function
{
public:
  // constructor requiwhite
  slf_fluid_bar_widget_set_val_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_fluid_bar_widget_t me;
    vm_num_t v;
  };

  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    parms->me->set_to_val(parms->v);

    SLF_DONE;
  }
};
//slf_bitmap_widget_set_val_t slf_text_block_widget_set_val(slc_bitmap_widget,"set_val(num)");


// script library function:  fluid_bar_widget::set_abs_val(num)
class slf_fluid_bar_widget_set_abs_val_t : public script_library_class::function
{
public:
  // constructor requiwhite
  slf_fluid_bar_widget_set_abs_val_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_fluid_bar_widget_t me;
    vm_num_t v;
  };

  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    parms->me->set_to_val(parms->v);
    parms->me->set_cur_val(parms->v);

    SLF_DONE;
  }
};
//slf_bitmap_widget_set_abs_val_t slf_text_block_widget_set_abs_val(slc_bitmap_widget,"set_abs_val(num)");


// script library function:  fluid_bar_widget::set_fill_rate(num)
class slf_fluid_bar_widget_set_fill_rate_t : public script_library_class::function
{
public:
  // constructor requiwhite
  slf_fluid_bar_widget_set_fill_rate_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_fluid_bar_widget_t me;
    vm_num_t v;
  };

  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    parms->me->set_fill_rate(parms->v);

    SLF_DONE;
  }
};
//slf_bitmap_widget_set_fill_rate_t slf_text_block_widget_set_fill_rate(slc_bitmap_widget,"set_fill_rate(num)");

// script library function:  fluid_bar_widget::set_empty_rate(num)
class slf_fluid_bar_widget_set_empty_rate_t : public script_library_class::function
{
public:
  // constructor requiwhite
  slf_fluid_bar_widget_set_empty_rate_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_fluid_bar_widget_t me;
    vm_num_t v;
  };

  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;

    parms->me->set_empty_rate(parms->v);

    SLF_DONE;
  }
};
//slf_bitmap_widget_set_empty_rate_t slf_text_block_widget_set_empty_rate(slc_bitmap_widget,"set_empty_rate(num)");


// script library function:  fluid_bar_widget::get_width()
class slf_fluid_bar_widget_get_width_t : public script_library_class::function
{
public:
  // constructor required
  slf_fluid_bar_widget_get_width_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_fluid_bar_widget_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_num_t result = parms->me->get_width();
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_fluid_bar_widget_get_width_t slf_fluid_bar_widget_get_width(slc_fluid_bar_widget,"get_width()");


// script library function:  fluid_bar_widget::get_height()
class slf_fluid_bar_widget_get_height_t : public script_library_class::function
{
public:
  // constructor required
  slf_fluid_bar_widget_get_height_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_fluid_bar_widget_t me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_num_t result = parms->me->get_height();
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_fluid_bar_widget_get_height_t slf_fluid_bar_widget_get_height(slc_fluid_bar_widget,"get_height()");











///////////////////////////////////////////////////////////////////////////////
// interface functions:  boss bar
///////////////////////////////////////////////////////////////////////////////

// global script library function:  activate_boss_bar(character chr)
class slf_activate_boss_bar_t : public script_library_class::function
{
public:
  // constructor required
  slf_activate_boss_bar_t(const char* n): script_library_class::function(n) { }
  // library function parameters
  struct parms_t
  {
    // parameters
//!    vm_character_t chr;
  };
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    SLF_PARMS_UNUSED;
//!    app::inst()->get_game()->get_interface_widget()->activate_boss_widget( parms->chr );
    SLF_DONE;
  }
};
//slf_activate_boss_bar_t slf_activate_boss_bar("activate_boss_bar(character)");


// global script library function:  hide_boss_bar()
class slf_hide_boss_bar_t : public script_library_class::function
{
public:
  // constructor required
  slf_hide_boss_bar_t(const char* n): script_library_class::function(n) { }
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
//!    app::inst()->get_game()->get_interface_widget()->hide_boss_widget();
    SLF_DONE;
  }
};
//slf_hide_boss_bar_t slf_hide_boss_bar("hide_boss_bar()");



///////////////////////////////////////////////////////////////////////////////
// interface functions:  color clues
///////////////////////////////////////////////////////////////////////////////

// global script library function:  give_color_clue( num panel )
class slf_give_color_clue_t : public script_library_class::function
{
public:
  // constructor required
  slf_give_color_clue_t(const char* n) : script_library_class::function(n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_num_t panel;
    vm_num_t color;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS_UNUSED;
//    app::inst()->get_game()->get_script_widget_holder()->get_clue_widget()->set_clue_col( int( parms->panel ), clue_col_e( int(parms->color) ) );
    SLF_DONE;
  }
};
//slf_give_color_clue_t slf_give_color_clue("give_color_clue(num,num)");


// global script library function:  clear_color_clues()
class slf_clear_color_clues_t : public script_library_class::function
{
public:
  // constructor required
  slf_clear_color_clues_t(const char* n) : script_library_class::function(n) {}
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
//    app::inst()->get_game()->get_script_widget_holder()->get_clue_widget()->clear_clues();
    SLF_DONE;
  }
};
//slf_clear_color_clues_t slf_clear_color_clues("clear_color_clues()");



// global script library function:  show_clues_while_letterboxed()
class slf_show_clues_while_letterboxed_t : public script_library_class::function
{
public:
  // constructor required
  slf_show_clues_while_letterboxed_t(const char* n) : script_library_class::function(n) {}
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
//    app::inst()->get_game()->get_script_widget_holder()->get_clue_widget()->set_show_while_letterboxed( true );
    SLF_DONE;
  }
};
//slf_show_clues_while_letterboxed_t slf_show_clues_while_letterboxed("show_clues_while_letterboxed()");


///////////////////////////////////////////////////////////////////////////////
// interface functions:  score
///////////////////////////////////////////////////////////////////////////////

// global script library function:  inc_score(num inc)
class slf_inc_score_t : public script_library_class::function
{
public:
  // constructor required
  slf_inc_score_t(const char* n) : script_library_class::function(n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_num_t inc;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS_UNUSED;
//    app::inst()->get_game()->get_interface_widget()->inc_score( int( parms->inc ) );
    SLF_DONE;
  }
};
//slf_inc_score_t slf_inc_score("inc_score(num)");


// global script library function:  clear_score( num panel )
class slf_clear_score_t : public script_library_class::function
{
public:
  // constructor required
  slf_clear_score_t(const char* n) : script_library_class::function(n) {}
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
//    app::inst()->get_game()->get_interface_widget()->clear_score();
    SLF_DONE;
  }
};
//slf_clear_score_t slf_clear_score("clear_score()");



///////////////////////////////////////////////////////////////////////////////
// script library class: hide_all_widgets
///////////////////////////////////////////////////////////////////////////////
// global script library function: hide_all_widgets()
class slf_hide_all_widgets_t : public script_library_class::function
{
public:
  // constructor required
  slf_hide_all_widgets_t(const char* n): script_library_class::function(n) { }
  // library function execution
  virtual bool operator ()(vm_stack &stack, entry_t entry)
  {
    widget *widg = (widget*)(g_game_ptr->get_script_widget_holder());
    widget_list_t::iterator i = widg->get_children().begin();
    widget_list_t::iterator i_end = widg->get_children().end();

    while(i != i_end)
    {
      (*i)->hide();
      ++i;
    }

    SLF_DONE;
  }
};
//slf_hide_all_widgets_t slf_hide_all_widgets("hide_all_widgets()");


void register_widget_lib()
{
  // pointer to single instance of library class
  slc_widget_t* slc_widget = NEW slc_widget_t("widget",4);

  NEW slf_widget_show_t(slc_widget,"show()");
  NEW slf_widget_hide_t(slc_widget,"hide()");
  NEW slf_widget_move_to_t(slc_widget,"move_to(num,num)");
  NEW slf_widget_move_to2_t(slc_widget,"move_to(num,num,num,num)");
  NEW slf_widget_set_layer_t(slc_widget,"set_layer(num)");
  NEW slf_widget_scale_to_t(slc_widget,"scale_to(num)");
  NEW slf_widget_scale_to2_t(slc_widget,"scale_to(num,num,num)");
  NEW slf_widget_fade_to_t(slc_widget,"fade_to(num)");
  NEW slf_widget_fade_to2_t(slc_widget,"fade_to(num,num,num)");

  // pointer to single instance of library class
  slc_timer_widget_t* slc_timer_widget = NEW slc_timer_widget_t("timer_widget",4,"widget");

  NEW slf_create_timer_widget_t("create_timer_widget()");
  NEW slf_timer_widget_get_time_left_t(slc_timer_widget,"get_time_left()");
  NEW slf_timer_widget_set_time_left_t(slc_timer_widget,"set_time_left(num)");
  NEW slf_timer_widget_freeze_t(slc_timer_widget,"freeze()");
  NEW slf_timer_widget_run_t(slc_timer_widget,"run()");
  NEW slf_timer_widget_inc_time_left_t(slc_timer_widget,"inc_time_left(num)");
//  NEW slf_timer_widget_use_red_nums_t(slc_timer_widget,"use_red_nums()");
//  NEW slf_timer_widget_use_green_nums_t(slc_timer_widget,"use_green_nums()");
  NEW slf_timer_widget_add_function_t(slc_timer_widget,"add_function(str,num)");
  NEW slf_timer_widget_remove_function_t(slc_timer_widget,"remove_function(num,num)");
  NEW slf_timer_widget_clear_functions_t(slc_timer_widget,"clear_functions()");
  NEW slf_timer_widget_set_digit_color_t(slc_timer_widget,"set_digit_color(num,num,num,num)");
  NEW slf_timer_widget_set_bg_color_t(slc_timer_widget,"set_bg_color(num,num,num,num)");

  slc_text_block_widget_t* slc_text_block_widget = NEW slc_text_block_widget_t("text_block_widget",4,"widget");
  NEW slf_create_text_block_widget_t("create_text_block_widget(str,str)");
  NEW slf_text_block_widget_set_text_t(slc_text_block_widget,"set_text(str)");
  NEW slf_text_block_widget_set_scale_t(slc_text_block_widget,"set_scale(num)");
  NEW slf_text_block_widget_set_max_lines_t(slc_text_block_widget,"set_max_lines(num)");
  NEW slf_text_block_widget_set_line_spacing_t(slc_text_block_widget,"set_line_spacing(num)");
  NEW slf_text_block_widget_set_max_width_t(slc_text_block_widget,"set_max_width(num)");
  NEW slf_text_block_widget_set_color_t(slc_text_block_widget,"set_color(vector3d,num)");


  slc_bitmap_widget_t* slc_bitmap_widget = NEW slc_bitmap_widget_t("bitmap_widget",4,"widget");
  NEW slf_create_bitmap_widget_t("create_bitmap_widget(str)");
  NEW slf_bitmap_widget_set_color_t(slc_bitmap_widget,"set_color(vector3d)");
  NEW slf_bitmap_widget_flip_horiz_t(slc_bitmap_widget,"flip_horiz()");
  NEW slf_bitmap_widget_flip_vert_t(slc_bitmap_widget,"flip_vert()");
  NEW slf_bitmap_widget_resize_t(slc_bitmap_widget,"resize(num,num)");
  NEW slf_bitmap_widget_set_subrect_t(slc_bitmap_widget,"set_subrect(num,num,num,num)");
  NEW slf_bitmap_widget_get_width_t(slc_bitmap_widget,"get_width()");
  NEW slf_bitmap_widget_get_height_t(slc_bitmap_widget,"get_height()");

  NEW slc_bitmap6_widget_t("bitmap6_widget",4,"widget");
  NEW slf_create_bitmap6_widget_t("create_bitmap6_widget(str)");

  slc_fluid_bar_widget_t* slc_fluid_bar_widget = NEW slc_fluid_bar_widget_t("fluid_bar_widget",4,"widget");
  NEW slf_create_fluid_bar_widget_t("create_fluid_bar_widget(num)");
  NEW slf_fluid_bar_widget_set_color_t(slc_fluid_bar_widget,"set_color(vector3d)");
  NEW slf_fluid_bar_widget_resize_t(slc_fluid_bar_widget,"resize(num,num)");
  NEW slf_fluid_bar_widget_set_full_val_t(slc_fluid_bar_widget,"set_full_val(num)");
  NEW slf_fluid_bar_widget_set_val_t(slc_fluid_bar_widget,"set_val(num)");
  NEW slf_fluid_bar_widget_set_abs_val_t(slc_fluid_bar_widget,"set_abs_val(num)");
  NEW slf_fluid_bar_widget_set_fill_rate_t(slc_fluid_bar_widget,"set_fill_rate(num)");
  NEW slf_fluid_bar_widget_set_empty_rate_t(slc_fluid_bar_widget,"set_empty_rate(num)");
  NEW slf_fluid_bar_widget_get_width_t(slc_fluid_bar_widget,"get_width()");
  NEW slf_fluid_bar_widget_get_height_t(slc_fluid_bar_widget,"get_height()");


  NEW slf_activate_boss_bar_t("activate_boss_bar(character)");
  NEW slf_hide_boss_bar_t("hide_boss_bar()");
  NEW slf_give_color_clue_t("give_color_clue(num,num)");
  NEW slf_clear_color_clues_t("clear_color_clues()");
  NEW slf_show_clues_while_letterboxed_t("show_clues_while_letterboxed()");
  NEW slf_inc_score_t("inc_score(num)");
  NEW slf_clear_score_t("clear_score()");

  NEW slf_hide_all_widgets_t("hide_all_widgets()");
}
