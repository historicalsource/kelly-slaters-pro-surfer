#ifndef POPUP_H
#define POPUP_H

#include "widget.h"
//P #include "pda.h"
#include "commands.h"


//-------------------------------------------------------------------------------------

// popup_widget contains a state which can be set and checked for further action (wait, retry, and proceed)

class popup_widget : public widget
{
public:
  popup_widget( const char *_widget_name, widget *_parent, short _x, short _y );
  virtual ~popup_widget() {}
  virtual void show() { widget::show(); state = STATE_WAIT; }
  bool is_waiting() const { return (state == STATE_WAIT); }
  bool requests_retry() const { return (state == STATE_RETRY); }
  bool requests_proceed() const { return (state == STATE_PROCEED); }

  virtual void set_text( const stringx &text ) { display_text = text; }
  virtual const stringx &get_text() const { return display_text; }
  virtual bool is_same_text( const stringx &text );

  virtual void set_wait( time_value_t _wait ) {}

  void set_copy( bool _copy ) { copy = _copy; }
  bool is_a_copy() const { return copy; }

protected:
  enum popup_state_e
  {
    STATE_WAIT,
    STATE_RETRY,
    STATE_PROCEED,
  };
  popup_state_e state;
  stringx display_text;
  bool copy;
};



//-------------------------------------------------------------------------------------


#define POPUP_OUTLINE_COL   color( 87.0f / 255.0f, 142.0f / 255.0f, 242.0f / 255.0f, 1.0f )
#define POPUP_BOX_COL       color( 16.0f/255.0f, 30.0f/255.0f, 77.0f/255.0f, 1.0f )

// an outlined box with standardized colors

class popup_box : public widget
{
public:
  popup_box( const char *_widget_name, widget *_parent, short _x, short _y, rational_t width, rational_t height );
  virtual ~popup_box() {}
  void resize( rational_t width, rational_t height );
protected:
  bitmap_widget *bg;
  box_widget *outline;
};


//-------------------------------------------------------------------------------------

class vmu_selector : public popup_widget
{
public:
  enum vmu_menu_state_e
  {
    NO_VMU,
    ONE_VMU,
    MULTI_VMUS,
    NUM_VMU_STATES,
  };

  vmu_selector( const char *_widget_name, widget *_parent, short _x, short _y );
  virtual ~vmu_selector() {}

  virtual void show();
  virtual void frame_advance( time_value_t time_inc );
  void init_vmu_menu();

  class vmu_item : public menu_item_widget
  {
  public:
    vmu_item( const char *_widget_name, widget *_parent, short _x, short _y, const char *_item_desc );
    virtual ~vmu_item() {}

    virtual void show();
    virtual void select( bool initial = false );
    virtual void deselect( bool initial );

  protected:
    text_widget *item_text;
  };

protected:
//#ifdef DEBUG
//  layout_widget *layout;
//#endif
  menu_widget *menu;
  text_widget *blocks_free;
  help_group *help_grp[NUM_VMU_STATES];
  vmu_menu_state_e menu_state;
};


//-------------------------------------------------------------------------------------

#define QUESTION_TEXT_SCALE   0.7f


// has popup_box as background, requests retry or proceed (waits for specified command);
// you should construct lines (which are added to help_grp) then call init() to size and
// arrange everything.  Popup draws centered on x, y.

class question_popup : public popup_widget
{
public:   // NUM_GAME_CONTROLS as command indicates given action (proceed or retry) is not possible
  question_popup( const char *_widget_name, widget *_parent, short _x, short _y,
      game_control_t _retry_command = NUM_GAME_CONTROLS, game_control_t _proceed_command = NUM_GAME_CONTROLS );
  virtual ~question_popup() {}

  // lines are automatically centered (including icons)
  void construct_line( help_icon_id_t icon_id, stringx help_text, short y,
                 rational_t text_scale = HELP_TEXT_SCALE,
                 rational_t icon_scale = HELP_ICON_SCALE,
                 short help_x = HELP_TEXT_X );              // see pda.h

  // call this after constructing lines; first two parameters are for help_group::center_and_align
  void init( short x_off = 0, short start_ln_num = 0, short horiz_margin = 50, short vert_margin = 40 );

  virtual void frame_advance( time_value_t time_inc );

protected:
  game_control_t retry_command, proceed_command;
  help_group *help_grp;
  popup_box *bg;
};


//-------------------------------------------------------------------------------------

// notice popup gives user a boxed single line piece of text
// popup is centered on x, y

class notice_popup : public popup_widget
{
public:
  notice_popup( const char *_widget_name, widget *_parent, short _x, short _y );
  virtual ~notice_popup() {}
  virtual void set_text( const stringx &text );
protected:
  text_widget *text_line;
  popup_box *bg;
};



//-------------------------------------------------------------------------------------

// warning popup shows user a text block
// proceeds after process timer has elapsed
// popup is centered on x, y

class warning_popup : public popup_widget
{
public:
  warning_popup( const char *_widget_name, widget *_parent, short _x, short _y );
  virtual ~warning_popup() {}
  virtual void frame_advance( time_value_t time_inc );
  virtual void set_text( const stringx &text );
protected:
  text_block_widget *text_block;
  popup_box *bg;
};

//-------------------------------------------------------------------------------------

// controller_removed_popup gives user a text block and waits for reinsertion of controller,
// after warning process's timer has elapsed
// popup is centered on x, y

class controller_removed_popup : public warning_popup
{
public:
  controller_removed_popup( const char *_widget_name, widget *_parent, short _x, short _y )
    : warning_popup( _widget_name, _parent, _x, _y ) {}
  virtual ~controller_removed_popup() {}
  virtual void frame_advance( time_value_t time_inc );
};


//-------------------------------------------------------------------------------------

// yesno_popup gives user a text block and a yes/no option and takes a for proceed and b for retry
// popup is centered on x, y

class yesno_popup : public popup_widget
{
public:
  yesno_popup( const char *_widget_name, widget *_parent, short _x, short _y );
  virtual ~yesno_popup() {}
  virtual void frame_advance( time_value_t time_inc );
  virtual void set_text( const stringx &text );

  void construct_line( help_icon_id_t icon_id, stringx help_text, short y,
                 rational_t text_scale = HELP_TEXT_SCALE,
                 rational_t icon_scale = HELP_ICON_SCALE,
                 short help_x = HELP_TEXT_X );              // see pda.h

protected:
  text_block_widget *text_block;
  help_group *help_grp;
  popup_box *bg;
};


//-------------------------------------------------------------------------------------

class cancel_popup : public yesno_popup
{
public:
  cancel_popup( const char *_widget_name, widget *_parent, short _x, short _y )
    : yesno_popup( _widget_name, _parent, _x, _y )
  {
    removed = different = full = non_save_full = false;
  }
  virtual ~cancel_popup() {}
  virtual void frame_advance( time_value_t time_inc );
protected:
  bool removed, different, full, non_save_full;
};



//-------------------------------------------------------------------------------------

class cancel_save_vmu_removed_popup : public cancel_popup
{
public:
  cancel_save_vmu_removed_popup( const char *_widget_name, widget *_parent, short _x, short _y )
    : cancel_popup( _widget_name, _parent, _x, _y )
  {
    different = full = non_save_full = false;
    removed = true;
  }
  virtual ~cancel_save_vmu_removed_popup() {}
};



//-------------------------------------------------------------------------------------

class cancel_save_vmu_different_popup : public cancel_popup
{
public:
  cancel_save_vmu_different_popup( const char *_widget_name, widget *_parent, short _x, short _y )
    : cancel_popup( _widget_name, _parent, _x, _y )
  {
    removed = full = non_save_full = false;
    different = true;
  }
  virtual ~cancel_save_vmu_different_popup() {}
};



//-------------------------------------------------------------------------------------

class cancel_save_vmu_full_popup : public cancel_popup
{
public:
  cancel_save_vmu_full_popup( const char *_widget_name, widget *_parent, short _x, short _y )
    : cancel_popup( _widget_name, _parent, _x, _y )
  {
    removed = different = non_save_full = false;
    full = true;
  }
  virtual ~cancel_save_vmu_full_popup() {}
};



//-------------------------------------------------------------------------------------


class loading_screen : public popup_widget
{
public:
  loading_screen( const char *_widget_name, widget *_parent, short _x, short _y,
    const stringx &file_prefix = "interface\\maxsteelmenu0" );
  virtual ~loading_screen() {}
  virtual void show();
  virtual void frame_advance( time_value_t time_inc );
protected:
  background_widget *bg;
};


//-------------------------------------------------------------------------------------


// popup widgets are unparented, are instead held in this thing, which creates and destroys them,
// and provides pointers to them

class popup_holder
{
public:
  popup_holder();
  virtual ~popup_holder();

  question_popup *get_hero_dies_popup() const { return hero_dies_popup; }
  question_popup *get_quit_confirm_popup() const { return quit_confirm_popup; }
  notice_popup *get_notice_popup() const { return my_notice_popup; }
  warning_popup *get_warning_popup() const { return my_warning_popup; }
  controller_removed_popup *get_controller_removed_popup() const { return my_controller_removed_popup; }
  yesno_popup *get_yesno_popup() const { return my_yesno_popup; }
  cancel_popup *get_cancel_popup() const { return my_cancel_popup; }
  cancel_popup *get_cancel_save_vmu_removed_popup() const { return my_cancel_save_vmu_removed_popup; }
  cancel_popup *get_cancel_save_vmu_different_popup() const { return my_cancel_save_vmu_different_popup; }
  cancel_popup *get_cancel_save_vmu_full_popup() const { return my_cancel_save_vmu_full_popup; }

  widget_list_t &get_widgets() { return popup_widgets; }

  // for copying popups
  popup_widget *create_popup( popup_widget *popup );
  void destroy_popup( popup_widget *popup );


protected:
  question_popup *construct_hero_dies_popup( bool copy = false );
  question_popup *construct_quit_confirm_popup( bool copy = false );
  notice_popup *construct_notice_popup( bool copy = false );
  warning_popup *construct_warning_popup( bool copy = false );
  controller_removed_popup *construct_controller_removed_popup( bool copy = false );
  yesno_popup *construct_yesno_popup( bool copy = false );
  cancel_popup *construct_cancel_popup( bool copy = false );
  cancel_save_vmu_removed_popup *construct_cancel_save_vmu_removed_popup( bool copy = false );
  cancel_save_vmu_different_popup *construct_cancel_save_vmu_different_popup( bool copy = false );
  cancel_save_vmu_full_popup *construct_cancel_save_vmu_full_popup( bool copy = false );

  widget_list_t popup_widgets;
  question_popup *hero_dies_popup;
  question_popup *quit_confirm_popup;
  notice_popup *my_notice_popup;
  warning_popup *my_warning_popup;
  controller_removed_popup *my_controller_removed_popup;
  yesno_popup *my_yesno_popup;
  cancel_popup *my_cancel_popup;
  cancel_popup *my_cancel_save_vmu_removed_popup;
  cancel_popup *my_cancel_save_vmu_different_popup;
  cancel_popup *my_cancel_save_vmu_full_popup;
};



#endif // POPUP_H