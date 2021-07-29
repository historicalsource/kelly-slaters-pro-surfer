// popup.cpp:  widgets used for popup messages


#include "global.h"

//!#include "attrib.h"
#include "wds.h"
//!#include "character.h"
#include "app.h"
#include "game.h"
#include "ostimer.h"
#include "interface.h"
//P #include "pda.h"
//P #include "popup.h"
#include "controller.h"
#include "commands.h"
#include "localize.h"
#include "osdevopts.h"
#include "pfe.h"
#include "inputmgr.h"
#include "mouse.h"
//P #include "osstorage.h"
//P #include "warning.h"

#include <cstdarg>


extern game *g_game_ptr;


#define POPUP_TEXT_COL        color( 87.0f / 255.0f, 142.0f / 255.0f, 242.0f / 255.0f, 1.0f )
#define POPUP_INACTIVE_COL    color( 0.5f, 0.5f, 0.5f, 1.0f )
#define POPUP_ACTIVE_COL      color( 1.0f, 1.0f, 1.0f, 1.0f )



//-------------------------------------------------------------------------------------


popup_widget::popup_widget( const char *_widget_name, widget *_parent, short _x, short _y )
  : widget( _widget_name, _parent, _x, _y )
{
  state = STATE_WAIT;
  display_text = empty_string;
  copy = false;
}





bool popup_widget::is_same_text( const stringx &text )
{
  if ( text == display_text )
    return true;
  else
    return false;
}



//-------------------------------------------------------------------------------------


popup_box::popup_box( const char *_widget_name, widget *_parent, short _x, short _y, rational_t width, rational_t height )
  : widget( _widget_name, _parent, _x, _y )
{
  // bg box
  bg = NEW bitmap_widget( "popup-bg", this, 0, 0, 1 );
  bg->open( os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR)+"\\alpha" );
  bg->resize( width, height );
  bg->set_color( POPUP_BOX_COL );

  // outline
  outline = NEW box_widget( "popup-box", this, 0, 0, width, height );
  outline->set_color( POPUP_OUTLINE_COL );
}


void popup_box::resize( rational_t width, rational_t height )
{
  bg->resize( width, height );
  outline->resize( width, height );
}


//-----------------------------------------------------------------


vmu_selector::vmu_selector( const char *_widget_name, widget *_parent, short _x, short _y )
  : popup_widget( _widget_name, _parent, _x, _y )
{
  menu_state = NO_VMU;

  // rhw layers for popups
  set_rhw_2d_layer( RHW8 );
  set_rhw_3d_layer( RHW9 );

  // bg box
//  const float bg_w = 440.0f;// unused -- remove me?
//  const float bg_h = 330.0f;// unused -- remove me?
//  const short bg_x = (short)(320 - bg_w/2);// unused -- remove me?
//  const short bg_y = (short)(240 - bg_h/2);// unused -- remove me?
//  popup_box *bg = NEW popup_box( "vmu-bg", this, bg_x, bg_y, bg_w, bg_h ); // unused -- remove me?

  // help text
  help_grp[NO_VMU] = NEW help_group( "vmu-help1", this, 320, 0 );
  help_grp[ONE_VMU] = NEW help_group( "vmu-help2", this, 320, 0 );
  help_grp[MULTI_VMUS] = NEW help_group( "vmu-help3", this, 320, 0 );

  const rational_t text_scale = 0.7f, icon_scale = 0.75f;
  const short text_x = 33;
  help_line *help = NEW help_line( "vmu-h1-1", help_grp[NO_VMU], 0, 130, HELP_ICON_NONE, "$NoVMU1", text_scale, icon_scale, text_x );
  help = NEW help_line( "vmu-h1-2", help_grp[NO_VMU], 0, 160, HELP_ICON_NONE, "$NoVMU2", text_scale, icon_scale, text_x );
  help = NEW help_line( "vmu-h1-3", help_grp[NO_VMU], 0, 190, HELP_ICON_NONE, "$NoVMU3", text_scale, icon_scale, text_x );
  help = NEW help_line( "vmu-h1-4", help_grp[NO_VMU], 0, 240, HELP_ICON_A, "$NoVMU4", text_scale, icon_scale, text_x );

  help = NEW help_line( "vmu-h2-1", help_grp[ONE_VMU], 0, 130, HELP_ICON_NONE, "$OneVMU1", text_scale, icon_scale, text_x );
  help = NEW help_line( "vmu-h2-2", help_grp[ONE_VMU], 0, 160, HELP_ICON_A, "$OneVMU2", text_scale, icon_scale, text_x );
  help = NEW help_line( "vmu-h2-3", help_grp[ONE_VMU], 0, 190, HELP_ICON_NONE, "$OneVMU3", text_scale, icon_scale, text_x );
  help = NEW help_line( "vmu-h2-4", help_grp[ONE_VMU], 0, 240, HELP_ICON_NONE, "$OneVMU4", text_scale, icon_scale, text_x );

  help = NEW help_line( "vmu-h3-1", help_grp[MULTI_VMUS], 0, 130, HELP_ICON_LR_Arrows, "$MultiVMU1", text_scale, icon_scale, text_x );
  help = NEW help_line( "vmu-h3-2", help_grp[MULTI_VMUS], 0, 160, HELP_ICON_NONE, "$MultiVMU2", text_scale, icon_scale, text_x );
  help = NEW help_line( "vmu-h3-3", help_grp[MULTI_VMUS], 0, 190, HELP_ICON_NONE, "$MultiVMU3", text_scale, icon_scale, text_x );
  help = NEW help_line( "vmu-h3-4", help_grp[MULTI_VMUS], 0, 240, HELP_ICON_A, "$MultiVMU4", text_scale, icon_scale, text_x );

  // align help text
  help_grp[NO_VMU]->center_and_align();
  help_grp[ONE_VMU]->center_and_align();
  help_grp[MULTI_VMUS]->center_and_align();

  // VMU menu
//  char *names[MAX_STORAGE_UNITS] = { "A1", "A2", "B1", "B2", "C1", "C2", "D1", "D2" };// unused -- remove me?
//  const short x_spacing = 50, vmuy = 315;// unused -- remove me?
  menu = NEW menu_widget( "vmu-menu", this, 0, 0, WMESS_L_Press, WMESS_R_Press );
  for ( int i = 0; i < MAX_STORAGE_UNITS; ++i )
  {
//    short vmux = i * x_spacing + 320 - (MAX_STORAGE_UNITS-1) * x_spacing / 2; // unused -- remove me?
//    vmu_item *item = NEW vmu_item( "vmu-item", menu, vmux, vmuy, names[i] ); // unused -- remove me?
  }
  menu->init();

  // blocks free
  blocks_free = NEW text_widget( "vmu-blocks_free", this, 320, 330 );
  blocks_free->hide();

  // layout tool
//#ifdef DEBUG
//  layout = NEW layout_widget( "pda-layout", this, 0, 0 );
//#else
//  layout = NULL;
//#endif
}


void vmu_selector::show()
{
  popup_widget::show();
  init_vmu_menu();
}


void vmu_selector::init_vmu_menu()
{
  int i, num_avail = 0;
  bool focus_set = false;

  for ( i = 0; i < MAX_STORAGE_UNITS; ++i )
  {
    menu_item_widget *item = menu->find_item_by_index( i );
/*P
    storage_unit *unit = storage_mgr::inst()->get_unit( i );
    if ( unit )
    {
      if ( !focus_set )
      {
        menu->set_default_sel_index( i );
        item->select();
        focus_set = true;
      }
      item->set_skip( false );
      ++num_avail;
    }
    else
    {
      item->set_skip( true );
    }
    storage_mgr::inst()->release_unit( unit );
P*/
  }

  // make sure the menu items get set up properly
  menu->hide();
  if ( num_avail > 0 )
    menu->show();

  // set state and help text
  if ( num_avail == 0 )
  {
    menu_state = NO_VMU;
  }
  else if ( num_avail == 1 )
  {
    menu_state = ONE_VMU;
  }
  else
  {
    menu_state = MULTI_VMUS;
  }

  help_grp[NO_VMU]->hide();
  help_grp[ONE_VMU]->hide();
  help_grp[MULTI_VMUS]->hide();
  help_grp[menu_state]->show();

  g_game_ptr->get_vmu_checker()->init();
}


void vmu_selector::frame_advance( time_value_t time_inc )
{
  input_mgr::inst()->scan_devices();

  // proceed?
  if ( input_mgr::inst()->get_control_delta( JOYSTICK_DEVICE, BUTTON_A ) == AXIS_MAX )
  {
    if ( menu_state != NO_VMU )
    {
      syvars->cur_storage_unit_slot = (menu->get_sel_item())->get_index();
      syvars->storage_unit_exists = true;
    }
    state = STATE_PROCEED;  // leave the vmu_selector
  }
  // or change vmu focus
  else if ( menu_state == MULTI_VMUS )
  {
    message_id_t message = 0;

    if ( input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, PFE_LEFTRIGHT ) == AXIS_MIN &&
         input_mgr::inst()->get_control_delta( JOYSTICK_DEVICE, PFE_LEFTRIGHT ) == AXIS_MIN )
    {
      message |= WMESS_L_Press;
    }
    if ( input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, PFE_LEFTRIGHT ) == AXIS_MAX &&
         input_mgr::inst()->get_control_delta( JOYSTICK_DEVICE, PFE_LEFTRIGHT ) == AXIS_MAX )
    {
      message |= WMESS_R_Press;
    }

    menu->message_handler( message );
  }

  popup_widget::frame_advance( time_inc );
}


//-----------------------------------------------------------------

#define VMU_TEXT_SCALE    0.7f

vmu_selector::vmu_item::vmu_item( const char *_widget_name, widget *_parent, short _x, short _y, const char *_item_desc )
  : menu_item_widget( _widget_name, _parent, _x, _y, _item_desc )
{
  item_text = NEW text_widget( "vmu_item-text", this, 0, 0, VMU_TEXT_SCALE, JUSTIFY_CENTER, item_desc );
}


void vmu_selector::vmu_item::show()
{
  menu_item_widget::show();

  if ( skip )
  {
    set_color( POPUP_INACTIVE_COL );
  }
  else
  {
    set_color( POPUP_ACTIVE_COL );
  }
}

void vmu_selector::vmu_item::select( bool initial )
{
  // flash selected vmu
  if ( !skip )
  {
    item_text->flash_on( 0.2f, 0.2f );
    menu_item_widget::select( initial );
  }
}

void vmu_selector::vmu_item::deselect( bool initial )
{
  item_text->flash_off();
  menu_item_widget::deselect( initial );
}


//-----------------------------------------------------------------

question_popup::question_popup( const char *_widget_name, widget *_parent, short _x, short _y,
      game_control_t _retry_command, game_control_t _proceed_command )
  : popup_widget( _widget_name, _parent, _x, _y )
{
  retry_command = _retry_command;
  proceed_command = _proceed_command;

  bg = NEW popup_box( "question-bg", this, 0, 0, 0, 0 );
  help_grp = NEW help_group( "question-hlpgrp", this, 0, 0 );
}


void question_popup::construct_line( help_icon_id_t icon_id, stringx help_text, short y,
                 rational_t text_scale,
                 rational_t icon_scale,
                 short help_x )
{
  help_line *help_ln = NEW help_line( "question-line", help_grp, 0, y, icon_id, help_text,
    text_scale, icon_scale, help_x, true );
  // phony call to convince gnu that we use this, and to not warn us about it.
  // I am assuming, of course, that help_line registers itself with something that later
  // uses/removes it.
  help_ln->get_width();
}


void question_popup::init( short x_off, short start_ln_num, short horiz_margin, short vert_margin )
{
  help_grp->center_and_align( x_off, start_ln_num );

  short group_w = help_grp->get_width();
  short group_h = help_grp->get_height();

  short box_w = group_w + 2 * horiz_margin;
  short box_h = group_h + 2 * vert_margin;
  short box_x = -box_w / 2;
  short box_y = -box_h / 2;

  help_grp->move_to( 0, -group_h/2 );
  bg->move_to( box_x, box_y );
  bg->resize( box_w, box_h );

#if defined(TARGET_PC)
  SetCursor( LoadCursor(NULL, IDC_ARROW) );
#endif
}



void question_popup::frame_advance( time_value_t time_inc )
{
  if ( retry_command != NUM_GAME_CONTROLS && input_mgr::inst()->get_control_delta( JOYSTICK_DEVICE, retry_command ) == AXIS_MAX )
  {
//    g_sound_group_list.play_sound_group( "menu_button" );
    state = STATE_RETRY;
  }
  else if ( proceed_command != NUM_GAME_CONTROLS && input_mgr::inst()->get_control_delta( JOYSTICK_DEVICE, proceed_command ) == AXIS_MAX )
  {
//    g_sound_group_list.play_sound_group( "menu_button" );
    state = STATE_PROCEED;
  }

  input_device *devi;
  devi = input_mgr::inst()->get_device( MOUSE_DEVICE );
  if( devi )
  {
    rational_t mx = 0;
    rational_t my = 0;
    mx = devi->get_axis_state( MSE_X );
    my = devi->get_axis_state( MSE_Y );
    axis_id_t aidt = devi->get_axis_id( MSE_BTN0 );
    if( aidt != INVALID_AXIS_ID )
    {
      if( devi->get_axis_delta( aidt ) > 0 )
      {
        if( mx >= (get_abs_x()-(120/2)) )
        {
          if( mx <= (get_abs_x()+(120/2)) )
          {
            if( my < (get_abs_y()) )
            {
//              g_sound_group_list.play_sound_group( "menu_button" );
              state = STATE_RETRY;
            }
            if( my > (get_abs_y()) )
            {
//              g_sound_group_list.play_sound_group( "menu_button" );
              state = STATE_PROCEED;
            }
          }
        }
      }
    }
  }

  if( (state == STATE_RETRY) || (state == STATE_PROCEED) )
  {
#if defined(TARGET_PC)
    SetCursor( 0 );
#endif
  }

  popup_widget::frame_advance( time_inc );
}


//-----------------------------------------------------------------


loading_screen::loading_screen( const char *_widget_name, widget *_parent, short _x, short _y, const stringx &file_prefix )
  : popup_widget( _widget_name, _parent, _x, _y )
{
  bg = NEW background_widget( "load_screen-bg", this, 0, 0, file_prefix );
  text_widget *text = NEW text_widget( "load_screen-text", this, 320, 400, 1.2f, JUSTIFY_CENTER, "$Loading" );
  text->get_width(); // phony call to prove that it's used
}


void loading_screen::show()
{
  popup_widget::show();
  fade_to( 0.0f );
  fade_to( 0.0f, 1.0f, 1.0f );
}

void loading_screen::frame_advance( time_value_t time_inc )
{
  popup_widget::frame_advance( time_inc );
  if ( !is_faded() )
  {
    state = STATE_PROCEED;
  }
}


//-------------------------------------------------------------------------------------

static const rational_t notice_height = 80.0f;
static const rational_t notice_margin = 20.0f;


notice_popup::notice_popup( const char *_widget_name, widget *_parent, short _x, short _y )
  : popup_widget( _widget_name, _parent, _x, _y )
{
  bg = NEW popup_box( "notice-bg", this, 0, 0, 5, notice_height );
  text_line = NEW text_widget( "notice-text", this, 0, -8, 0.8f );
}



void notice_popup::set_text( const stringx &text )
{
  popup_widget::set_text( text );

  text_line->set_string( text );
  // resize bg based on text width
  rational_t width = text_line->get_width() + notice_margin * 2.0f;
  bg->resize( width, notice_height );
  bg->move_to( -width/2 , -notice_height/2 ); // center bg
}

//-------------------------------------------------------------------------------------

static const rational_t warning_horiz_margin = 30.0f;
static const rational_t warning_vert_margin = 20.0f;


warning_popup::warning_popup( const char *_widget_name, widget *_parent, short _x, short _y )
  : popup_widget( _widget_name, _parent, _x, _y )
{
  bg = NEW popup_box( "warning-bg", this, 0, 0, 0.0f, 0.0f );

  text_block_widget::block_info_t info;
  info.scale = 1.0f;
  info.max_lines = 10;
  info.line_spacing = 22;
  info.max_width = 320;
  info.just = JUSTIFY_CENTER;
  info.col = POPUP_ACTIVE_COL;
  text_block = NEW text_block_widget( "warning-text", this, 0, 0, &info );
}


void warning_popup::frame_advance( time_value_t time_inc )
{
  popup_widget::frame_advance( time_inc );
  if ( is_shown() )
  {
    if ( g_game_ptr->get_process_timer() == 0.0f )
      state = STATE_PROCEED;
  }
}



void warning_popup::set_text( const stringx &text )
{
  popup_widget::set_text( text );

  text_block->set_text( text );
  text_block->move_to( 0, -text_block->get_height()/2 );

  // resize bg based on text width
  rational_t width = text_block->get_width() + warning_horiz_margin * 2.0f;
  rational_t height = text_block->get_height() + warning_vert_margin * 2.0f;
  bg->resize( width, height );
  bg->move_to( -width/2, -height/2 ); // center bg
}


//-------------------------------------------------------------------------------------


extern bool g_controller_inserted;

void controller_removed_popup::frame_advance( time_value_t time_inc )
{
  warning_popup::frame_advance( time_inc );

  if ( is_shown() )
  {
    // has timer run out? check whether controller is still removed
    if ( requests_proceed() && !g_controller_inserted )
      state = STATE_WAIT;
  }
}


//-------------------------------------------------------------------------------------


static const rational_t yesno_horiz_margin = 30.0f;
static const rational_t yesno_vert_margin = 20.0f;


yesno_popup::yesno_popup( const char *_widget_name, widget *_parent, short _x, short _y )
  : popup_widget( _widget_name, _parent, _x, _y )
{
  bg = NEW popup_box( "warning-bg", this, 0, 0, 0.0f, 0.0f );

  text_block_widget::block_info_t info;
  info.scale = 1.0f;
  info.max_lines = 10;
  info.line_spacing = 22;
  info.max_width = 320;
  info.just = JUSTIFY_CENTER;
  info.col = POPUP_ACTIVE_COL;
  text_block = NEW text_block_widget( "warning-text", this, 0, 0, &info );

  help_grp = NEW help_group( "yesno-hlpgrp", this, 0, 0 );
  construct_line( HELP_ICON_A, "$Yes", 0, 1.0f, 1.0f, 40 );
  construct_line( HELP_ICON_B, "$No", 30, 1.0f, 1.0f, 40 );
}


void yesno_popup::construct_line( help_icon_id_t icon_id, stringx help_text, short y,
                 rational_t text_scale,
                 rational_t icon_scale,
                 short help_x )
{
  help_line *help_ln = NEW help_line( "yesno-line", help_grp, 0, y, icon_id, help_text,
    text_scale, icon_scale, help_x, true );
  help_ln->get_text()->set_color( POPUP_ACTIVE_COL );
}


void yesno_popup::frame_advance( time_value_t time_inc )
{
  if ( is_shown() )
  {
    if ( input_mgr::inst()->get_control_delta( JOYSTICK_DEVICE, BUTTON_A ) == AXIS_MAX )
    {
//      g_sound_group_list.play_sound_group( "menu_button" );
      state = STATE_PROCEED;
    }
    else if ( input_mgr::inst()->get_control_delta( JOYSTICK_DEVICE, BUTTON_B ) == AXIS_MAX )
    {
//      g_sound_group_list.play_sound_group( "menu_button" );
      state = STATE_RETRY;
    }

    popup_widget::frame_advance( time_inc );
  }
}


void yesno_popup::set_text( const stringx &text )
{
  popup_widget::set_text( text );

  const int help_spacing = 20;

  text_block->set_text( text );

  // width & height of text
  rational_t width = text_block->get_width() + yesno_horiz_margin * 2.0f;
  rational_t height = text_block->get_height() + yesno_vert_margin * 2.0f +
                      help_grp->get_height() + help_spacing;

  // position text
  int text_block_y = (int)(-height/2.0f + yesno_vert_margin);
  text_block->move_to( 0, -height/2.0f + yesno_vert_margin );
  help_grp->center_and_align();
  help_grp->move_to( 0, text_block_y + text_block->get_height() + help_spacing );

  // resize bg
  bg->resize( width, height );
  bg->move_to( -width/2, -height/2 ); // center bg
}


//-------------------------------------------------------------------------------------


void cancel_popup::frame_advance( time_value_t time_inc )
{
  if ( is_shown() )
  {
    if ( (removed && !g_game_ptr->get_vmu_checker()->cur_vmu_was_removed()) ||
         (different && !g_game_ptr->get_vmu_checker()->cur_vmu_is_different()) ||
         (full && !g_game_ptr->get_vmu_checker()->cur_vmu_is_full()) )
    {
      state = STATE_PROCEED;
      hide();
      g_game_ptr->get_vmu_checker()->pre_save_vmu_warnings();
    }
    else
    {
      if ( input_mgr::inst()->get_control_delta( JOYSTICK_DEVICE, BUTTON_A ) == AXIS_MAX )
      {
//        g_sound_group_list.play_sound_group( "menu_button" );
        state = STATE_RETRY;
        g_game_ptr->get_vmu_checker()->set_skip_save( true );
      }
      else if ( input_mgr::inst()->get_control_delta( JOYSTICK_DEVICE, BUTTON_B ) == AXIS_MAX )
      {
//        g_sound_group_list.play_sound_group( "menu_button" );
        state = STATE_PROCEED;
        g_game_ptr->get_vmu_checker()->set_skip_save( false );
      }
    }

    popup_widget::frame_advance( time_inc );
  }
}


//-------------------------------------------------------------------------------------

// N.B. popup_holder is not a widget!
popup_holder::popup_holder()  // popups should be unparented, but added to list (see end of constructor)
{
  // rhw layers for popups
  widget::set_rhw_2d_layer( widget::RHW_OVER_PFE1 );
  widget::set_rhw_3d_layer( widget::RHW_OVER_PFE2 );

  hero_dies_popup = construct_hero_dies_popup();
  quit_confirm_popup = construct_quit_confirm_popup();
#ifdef TARGET_MKS
  my_notice_popup = construct_notice_popup();
#else
  my_notice_popup = NULL;
#endif
  my_warning_popup = construct_warning_popup();
  my_controller_removed_popup = construct_controller_removed_popup();
  my_yesno_popup = construct_yesno_popup();
  my_cancel_popup = construct_cancel_popup();
  my_cancel_save_vmu_removed_popup = construct_cancel_save_vmu_removed_popup();
  my_cancel_save_vmu_different_popup = construct_cancel_save_vmu_different_popup();
  my_cancel_save_vmu_full_popup = construct_cancel_save_vmu_full_popup();
}


popup_holder::~popup_holder()
{
  widget_list_t::iterator it;
  for ( it = popup_widgets.begin(); it != popup_widgets.end(); ++it )
  {
    if ( *it )
    {
      delete *it;
    }
  }
}


question_popup *popup_holder::construct_hero_dies_popup( bool copy )
{
  widget::set_rhw_2d_layer( widget::RHW_OVER_PFE1 );
  widget::set_rhw_3d_layer( widget::RHW_OVER_PFE2 );
  question_popup *popup = NEW question_popup( "hero dies", NULL, 320, 220, BUTTON_A, BUTTON_B );
  popup->construct_line( HELP_ICON_A, "$TryAgain", 0, 0.8f, 0.75f, 35 );
  popup->construct_line( HELP_ICON_B, "$Quit", 40, 0.8f, 0.75f, 35 );
  popup->init( -10 );
  popup->set_copy( copy );
  popup_widgets.push_back( popup );
  widget::restore_last_rhw_2d_layer();
  widget::restore_last_rhw_3d_layer();
  return popup;
}


question_popup *popup_holder::construct_quit_confirm_popup( bool copy )
{
  widget::set_rhw_2d_layer( widget::RHW_OVER_PFE1 );
  widget::set_rhw_3d_layer( widget::RHW_OVER_PFE2 );
  question_popup *popup = NEW question_popup( "quit confirm", NULL, 320, 220, BUTTON_B, BUTTON_A );
  popup->construct_line( HELP_ICON_NONE, "$AreYouSureQuit", 0, 0.8f, 0.75f, 35 ); // centered
  popup->construct_line( HELP_ICON_A, "$Proceed",50, 0.8f, 0.75f, 35 );
  popup->construct_line( HELP_ICON_B, "$Cancel", 90, 0.8f, 0.75f, 35 );
  popup->init( -10, 1 );
  popup->set_copy( copy );
  popup_widgets.push_back( popup );
  widget::restore_last_rhw_2d_layer();
  widget::restore_last_rhw_3d_layer();
  return popup;
}


notice_popup *popup_holder::construct_notice_popup( bool copy )
{
  widget::set_rhw_2d_layer( widget::RHW_OVER_PFE1 );
  widget::set_rhw_3d_layer( widget::RHW_OVER_PFE2 );
  notice_popup *popup = NEW notice_popup( "notice", NULL, 320, 220 );
  popup->set_copy( copy );
  popup_widgets.push_back( popup );
  widget::restore_last_rhw_2d_layer();
  widget::restore_last_rhw_3d_layer();
  return popup;
}


warning_popup *popup_holder::construct_warning_popup( bool copy )
{
  widget::set_rhw_2d_layer( widget::RHW_OVER_PFE1 );
  widget::set_rhw_3d_layer( widget::RHW_OVER_PFE2 );
  warning_popup *popup = NEW warning_popup( "warning", NULL, 320, 220 );
  popup->set_copy( copy );
  popup_widgets.push_back( popup );
  widget::restore_last_rhw_2d_layer();
  widget::restore_last_rhw_3d_layer();
  return popup;
}


controller_removed_popup *popup_holder::construct_controller_removed_popup( bool copy )
{
  widget::set_rhw_2d_layer( widget::RHW_OVER_PFE1 );
  widget::set_rhw_3d_layer( widget::RHW_OVER_PFE2 );
  controller_removed_popup *popup = NEW controller_removed_popup( "cont removed", NULL, 320, 220 );
  popup->set_copy( copy );
  popup_widgets.push_back( popup );
  widget::restore_last_rhw_2d_layer();
  widget::restore_last_rhw_3d_layer();
  return popup;
}


yesno_popup *popup_holder::construct_yesno_popup( bool copy )
{
  widget::set_rhw_2d_layer( widget::RHW_OVER_PFE1 );
  widget::set_rhw_3d_layer( widget::RHW_OVER_PFE2 );
  yesno_popup *popup = NEW yesno_popup( "yesno", NULL, 320, 220 );
  popup->set_copy( copy );
  popup_widgets.push_back( popup );
  widget::restore_last_rhw_2d_layer();
  widget::restore_last_rhw_3d_layer();
  return popup;
}


cancel_popup *popup_holder::construct_cancel_popup( bool copy )
{
  widget::set_rhw_2d_layer( widget::RHW_OVER_PFE1 );
  widget::set_rhw_3d_layer( widget::RHW_OVER_PFE2 );
  cancel_popup *popup = NEW cancel_popup( "yesno", NULL, 320, 220 );
  popup->set_copy( copy );
  popup_widgets.push_back( popup );
  widget::restore_last_rhw_2d_layer();
  widget::restore_last_rhw_3d_layer();
  return popup;
}



cancel_save_vmu_removed_popup *popup_holder::construct_cancel_save_vmu_removed_popup( bool copy )
{
  widget::set_rhw_2d_layer( widget::RHW_OVER_PFE1 );
  widget::set_rhw_3d_layer( widget::RHW_OVER_PFE2 );
  cancel_save_vmu_removed_popup *popup = NEW cancel_save_vmu_removed_popup( "yesno", NULL, 320, 220 );
  popup->set_copy( copy );
  popup_widgets.push_back( popup );
  widget::restore_last_rhw_2d_layer();
  widget::restore_last_rhw_3d_layer();
  return popup;
}


cancel_save_vmu_different_popup *popup_holder::construct_cancel_save_vmu_different_popup( bool copy )
{
  widget::set_rhw_2d_layer( widget::RHW_OVER_PFE1 );
  widget::set_rhw_3d_layer( widget::RHW_OVER_PFE2 );
  cancel_save_vmu_different_popup *popup = NEW cancel_save_vmu_different_popup( "yesno", NULL, 320, 220 );
  popup->set_copy( copy );
  popup_widgets.push_back( popup );
  widget::restore_last_rhw_2d_layer();
  widget::restore_last_rhw_3d_layer();
  return popup;
}

cancel_save_vmu_full_popup *popup_holder::construct_cancel_save_vmu_full_popup( bool copy )
{
  widget::set_rhw_2d_layer( widget::RHW_OVER_PFE1 );
  widget::set_rhw_3d_layer( widget::RHW_OVER_PFE2 );
  cancel_save_vmu_full_popup *popup = NEW cancel_save_vmu_full_popup( "yesno", NULL, 320, 220 );
  popup->set_copy( copy );
  popup_widgets.push_back( popup );
  widget::restore_last_rhw_2d_layer();
  widget::restore_last_rhw_3d_layer();
  return popup;
}




popup_widget *popup_holder::create_popup( popup_widget *popup )
{
  popup_widget *new_popup = NULL;
  if ( popup )
  {
    if ( popup == (popup_widget*)hero_dies_popup )
    {
      new_popup = (popup_widget*)construct_hero_dies_popup( true );
    }
    else if ( popup == (popup_widget*)quit_confirm_popup )
    {
      new_popup = (popup_widget*)construct_quit_confirm_popup( true );
    }
    else if ( popup == (popup_widget*)my_notice_popup )
    {
      new_popup = (popup_widget*)construct_notice_popup( true );
    }
    else if ( popup == (popup_widget*)my_warning_popup )
    {
      new_popup = (popup_widget*)construct_warning_popup( true );
    }
    else if ( popup == (popup_widget*)my_controller_removed_popup )
    {
      new_popup = (popup_widget*)construct_controller_removed_popup( true );
    }
    else if ( popup == (popup_widget*)my_yesno_popup )
    {
      new_popup = (popup_widget*)construct_yesno_popup( true );
    }
    else if ( popup == (popup_widget*)my_cancel_popup )
    {
      new_popup = (popup_widget*)construct_cancel_popup( true );
    }
    else if ( popup == (popup_widget*)my_cancel_save_vmu_removed_popup )
    {
      new_popup = (popup_widget*)construct_cancel_save_vmu_removed_popup( true );
    }
    else if ( popup == (popup_widget*)my_cancel_save_vmu_different_popup )
    {
      new_popup = (popup_widget*)construct_cancel_save_vmu_different_popup( true );
    }
    else if ( popup == (popup_widget*)my_cancel_save_vmu_full_popup )
    {
      new_popup = (popup_widget*)construct_cancel_save_vmu_full_popup( true );
    }
    else  // if we can't find the popup in our current holdings, just return passed pointer
    {
      new_popup = popup;
    }
  }
  return new_popup;
}




void popup_holder::destroy_popup( popup_widget *popup )
{
  if ( !popup->is_a_copy() ) return;

  widget_list_t::iterator it = popup_widgets.begin();

  for ( ; it != popup_widgets.end(); ++it )
  {
    if ( *it == (widget*)popup )
    {
      delete popup;
      popup_widgets.erase( it );
      break;
    }
  }
}
