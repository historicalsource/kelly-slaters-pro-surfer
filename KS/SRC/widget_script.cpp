// widget_script.cpp :: widgets used by scripters

#include "global.h"

#include "game.h"
#include "wds.h"
#include "pmesh.h"
#include "widget.h"
#include "widget_script.h"
#include "script_lib_mfg.h"

#include <cstdarg>



static const short clue_xoff = 4;
static const short clue_yoff = 6;
static const short clue_spacing = 3;

static const float clue_width = 17.0f;
static const float clue_height = 19.0f;


clue_widget::clue_widget( const char *_widget_name, widget *_parent, short _x, short _y ) :
  widget( _widget_name, _parent, _x, _y )
{
  show_while_letterboxed = false;

  int i;
  for ( i = 0; i != NUM_COLOR_CLUES; ++i )
  {
    clue_col[i] = CLUE_Default;
  }

  clue_bar = NEW bitmap_widget( "clue_bar", this, 0, 0, 1 );
  clue_bar->open( "interface\\cluebar" );
  clue_bar->resize( 64.0f, 32.0f );

  int clue_x = clue_xoff, clue_y = clue_yoff;

  for ( i = 0; i != NUM_COLOR_CLUES; ++i )
  {
    clue[i] = NEW bitmap_widget( "clues", this, clue_x, clue_y, 1 );
    clue[i]->open( (os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR)+"\\alpha").c_str() );
    clue[i]->resize( clue_width, clue_height );
    clue[i]->ignore_parent_showing();
    set_clue_col( i, CLUE_Default );

    clue_x += int( clue_width ) + clue_spacing;
  }

  restore_last_rhw_2d_layer();
}

void clue_widget::set_clue_col( int panel, clue_col_e cc )
{
  assert( panel >= 0 && panel < NUM_COLOR_CLUES );
  clue_col[panel] = cc;

  if ( cc == CLUE_Default )
  {
    clue[panel]->hide();
  }
  else if ( cc == CLUE_Red )
  {
    clue[panel]->set_color( color( 1.0f, 0.0f, 0.0f, 1.0f ) );
    clue[panel]->show();
  }
  else if ( cc == CLUE_Green )
  {
    clue[panel]->set_color( color( 0.0f, 1.0f, 0.0f, 1.0f ) );
    clue[panel]->show();
  }
  else if ( cc == CLUE_Blue )
  {
    clue[panel]->set_color( color( 0.0f, 0.0f, 1.0f, 1.0f ) );
    clue[panel]->show();
  }
}


void clue_widget::clear_clues()
{
  for ( int i = 0; i < NUM_COLOR_CLUES; ++i )
  {
    set_clue_col( i, CLUE_Default );
  }
}



//////////////////////////////////////////////////////////////////////////////////////////////////


static const int text_spacing = 2;
static const rational_t digit_scale = 3.75f;
static const rational_t digit_fade_level = 0.6f;
static const rational_t point_fade_level = 0.5f;
static const rational_t timer_bitmap_size = 20.0f;
static rational_t digit_bitmap_width = 190.0f;
static rational_t digit_bitmap_height = 190.0f;

#include <string.h>
timer_widget::timer_widget( const char *_widget_name, widget *_parent, short _x, short _y )
  : widget( _widget_name, _parent, _x, _y )
{
  script_calls_left.resize(0);
  script_calls_made.resize(0);

//  const rational_t fade_level = 0.5f; // unused -- remove me?
  time_left = 0.0f;

  int i;
  char bomb_number_str[256];
  strcpy(bomb_number_str, "interface\\");
  strcat(bomb_number_str, os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR).c_str());
  strcat(bomb_number_str, "\\bombnumber0");
  int num_pos = strlen(bomb_number_str)-1;

  for ( i = 0; i < 10; ++i )
  {
    widget::set_rhw_2d_layer(widget::RHW8);
    digits[i] = NEW bitmap_widget( "timer-digit", this, 0, 0, 1 );
    widget::restore_last_rhw_2d_layer();

    digits[i]->resize( digit_bitmap_width, digit_bitmap_height);
    digits[i]->open( bomb_number_str );
//    digits[i]->scale_to( digit_scale );
    digits[i]->fade_to( digit_fade_level );
    bomb_number_str[num_pos]++; // increment to next number
/*  digits[i] = NEW vrep_widget( "timer-digits", this, 0, 0, NULL );
    digits[i]->set_rotation( 90.0f * PI / 180.0f, 180.0f * PI / 180.0f, 0.0f );
    digits[i]->scale_to( digit_scale );
*/
  }

  widget::set_rhw_2d_layer(widget::RHW8);

  colon1 = NEW bitmap_widget( "timer-colon1", this, 0, 0, 1 );
  colon1->open( (os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR)+"\\alpha").c_str() );
  colon1->resize( timer_bitmap_size, timer_bitmap_size );

  colon2 = NEW bitmap_widget( "timer-colon2", this, 0, 0, 1 );
  colon2->open( (os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR)+"\\alpha").c_str() );
  colon2->resize( timer_bitmap_size, timer_bitmap_size );

  point = NEW bitmap_widget( "timer-point", this, 0, 0, 1 );
  point->open( (os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR)+"\\alpha").c_str() );
  point->resize( timer_bitmap_size, timer_bitmap_size );

  widget::restore_last_rhw_2d_layer();

  widget::set_rhw_2d_layer(widget::RHW0);

  bg = NEW bitmap_widget( "timer-bg", this, 0, 0, 1 );
  bg->open( (os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR)+"\\alpha").c_str() );
  bg->resize( 200, 75 );

  widget::restore_last_rhw_2d_layer();
/*
  color point_color( 1.0f, 1.0f, 1.0f, point_fade_level );
  colon1->set_color( point_color );
  colon2->set_color( point_color );
  point->set_color( point_color );
*/
  color digit_color( 1.0f, 1.0f, 1.0f, 1.0f );
  set_digit_color(digit_color);

  color bg_color( 0.0f, 0.0f, 0.0f, 1.0f );
  set_bg_color(bg_color);

  // load the digit_vreps so we will simply do a copy when we load them again
  /*
  for ( i = 0; i < 10; ++i )
  {
#ifdef NGL
    nglSetMeshPath( "interface\\entities\\" );
    stringx filename = stringx(i) + "bombnumber";
    digit_mesh[i] = nglGetMesh( (char*)filename.c_str() );
#else
    stringx filename = "interface\\entities\\" + stringx(i) + "bombnumber.txtmesh";
    digit_vreps[i] = load_new_visual_rep( filename, 0 );
    if( digit_vreps[i] && digit_vreps[i]->get_type()==VISREP_PMESH )
    {
      static_cast<vr_pmesh*>(digit_vreps[i])->shrink_memory_footprint();
    }
#endif
  }
*/
/*
  for ( i = 0; i < 10; ++i )
  {
    stringx filename = "interface\\entities\\" + stringx(i) + "redbombnumber.txtmesh";
    red_vreps[i] = load_new_visual_rep( filename, 0 );
    if( red_vreps[i] && red_vreps[i]->get_type()==VISREP_PMESH )
    {
      static_cast<vr_pmesh*>(red_vreps[i])->shrink_memory_footprint();
    }
  }
*/

  minutes = 0;
  seconds1 = 0;
  seconds2 = 0;
  tenths = 0;

  resize_timer();
}


timer_widget::~timer_widget()
{
  script_calls_left.resize(0);
  script_calls_made.resize(0);
}

//BIGCULL  extern bool update_dat_sheeit;
short MAGIC_OFFSET= 97;
short digit_y     = -23;
//#pragma todo("Figure out Thad's code and get the timer widget working more correctly. (GT--3/28/01)")
void timer_widget::render()
{
  // override default widget::render behaviour
//  const short digit_x[] = { -224, -66, 75, 224 };
#if 0 //BIGCULL
if (update_dat_sheeit)
{
  resize_timer();
}
#endif //BIGCULL

  const short digit_x[] = { -224-MAGIC_OFFSET, -66-MAGIC_OFFSET, 75-MAGIC_OFFSET, 224-MAGIC_OFFSET };

  bg->render();
  colon1->render();
  colon2->render();
  point->render();

  digits[minutes]->move_to( digit_x[0]*abs_S[0], digit_y );
  digits[minutes]->render();
  digits[seconds1]->move_to( digit_x[1]*abs_S[0], digit_y );
  digits[seconds1]->render();
  digits[seconds2]->move_to( digit_x[2]*abs_S[0], digit_y );
  digits[seconds2]->render();
  digits[tenths]->move_to( digit_x[3]*abs_S[0], digit_y );
  digits[tenths]->render();
}


void timer_widget::set_digit_color(color col)
{
  colon1->set_color( col );
  colon2->set_color( col );
  point->set_color( col );

  for ( int i = 0; i < 10; ++i )
    digits[i]->set_color( col );
}


void timer_widget::set_bg_color(color col)
{
  bg->set_color( col );
}


void timer_widget::update_scale()
{
  widget::update_scale();
  resize_timer();
}


void timer_widget::resize_timer()
{
  for ( int i = 0; i < 10; ++i )
  {
    digits[i]->resize( digit_bitmap_width, digit_bitmap_height);
  }

  colon1->resize( timer_bitmap_size, timer_bitmap_size );
  colon2->resize( timer_bitmap_size, timer_bitmap_size );
  point->resize( timer_bitmap_size, timer_bitmap_size );

  colon1->move_to( -155*abs_S[0], -54*abs_S[0] );
  colon2->move_to( -155*abs_S[0], 37*abs_S[0] );
  point->move_to( 140*abs_S[0], 70*abs_S[0] );

//  bg->resize( 200, 75 );
  bg->resize( 660, 240 );
  bg->move_to( -330*abs_S[0], -117*abs_S[0] );
}


void timer_widget::show()
{
  if ( is_shown() ) return;

  running = true;
  updated = false;

  widget::show();
}

void timer_widget::frame_advance( time_value_t time_inc )
{
  if ( !is_shown() ) return;

  if ( running && time_left > 0.0f )
  {
    time_left -= time_inc;
    if ( time_left < 0.0f )
    {
      time_left = 0.0f;
    }
    updated = false;
  }

  if ( !updated )
  {
    // format text for render
    minutes = (int)(time_left / 60);
    seconds1 = (int(time_left) % 60) / 10;
    seconds2 = (int(time_left) % 60) % 10;
    tenths = (int)(rational_t(time_left - int(time_left)) * 10);
    updated = true;
  }

  vector<timer_func>::iterator sci = script_calls_left.begin();
  while(sci != script_calls_left.end())
  {
    if(time_left <= (*sci).time)
    {
      if ( (*sci).function.length() > 0 )
      {
        stringx actual_name = (*sci).function + "()";
        actual_name.to_lower();

        script_object* so = g_world_ptr->get_current_level_global_script_object();
        script_object::instance* inst = g_world_ptr->get_current_level_global_script_object_instance();

        if ( so!=NULL && inst!=NULL )
        {
          int fidx = so->find_func( actual_name );
          if ( fidx >= 0 )
            inst->add_thread( &so->get_func(fidx) );
          else
            warning( "Timer widget: script function '" + actual_name + "' not found" );
        }
      }

      script_calls_made.push_back((*sci));
      sci = script_calls_left.erase(sci);
    }
    else
      ++sci;
  }

  widget::frame_advance( time_inc );
}


void timer_widget::set_time_left( rational_t _time_left )
{
  if(_time_left > time_left)
  {
    vector<timer_func>::iterator sci = script_calls_made.begin();
    while(sci != script_calls_made.end())
    {
      if(_time_left > (*sci).time)
      {
        script_calls_left.push_back((*sci));
        sci = script_calls_made.erase(sci);
      }
      else
        ++sci;
    }
  }
  else if(_time_left < time_left)
  {
    vector<timer_func>::iterator sci = script_calls_left.begin();
    while(sci != script_calls_left.end())
    {
      if(_time_left <= (*sci).time)
      {
        script_calls_made.push_back((*sci));
        sci = script_calls_left.erase(sci);
      }
      else
        ++sci;
    }
  }

  time_left = _time_left;
  if ( time_left < 0.0f )
    time_left = 0.0f;
  updated = false;
}


void timer_widget::inc_time_left( rational_t time_delta )
{
  set_time_left(time_left + time_delta);
}


/*
void timer_widget::use_red_nums()
{
  using_red_nums = true;

  color timer_red( 208.0f / 255.0f, 48.0f / 255.0f, 48.0f / 255.0f, point_fade_level );
  colon1->set_color( timer_red );
  colon2->set_color( timer_red );
  point->set_color( timer_red );
}


void timer_widget::use_green_nums()
{
  using_red_nums = false;

  color timer_green( 0.0f, 119.0f / 255.0f, 32.0f / 255.0f, point_fade_level );
  colon1->set_color( timer_green );
  colon2->set_color( timer_green );
  point->set_color( timer_green );
}
*/

void timer_widget::add_script_function(const stringx &func, time_value_t time)
{
  if(time >= time_left)
  {
    script_calls_made.push_back(timer_func(func, time));
    script_calls_left.reserve(script_calls_made.size());
  }
  else if(time < time_left)
  {
    script_calls_left.push_back(timer_func(func, time));
    script_calls_made.reserve(script_calls_left.size());
  }
}

void timer_widget::remove_script_function(time_value_t start, time_value_t end)
{
  vector<timer_func>::iterator sci = script_calls_made.begin();
  while(sci != script_calls_made.end())
  {
    if((*sci).time >= start && (*sci).time <= end)
      sci = script_calls_made.erase(sci);
    else
      ++sci;
  }

  sci = script_calls_left.begin();
  while(sci != script_calls_left.end())
  {
    if((*sci).time >= start && (*sci).time <= end)
      sci = script_calls_left.erase(sci);
    else
      ++sci;
  }
}

void timer_widget::set_layer( rhw_layer_e rhw_layer )
{
  if(rhw_layer > (NUM_RHW_LAYERS-2))
    rhw_layer = (rhw_layer_e)(NUM_RHW_LAYERS-2);

  for ( int i = 0; i < 10; ++i )
    digits[i]->set_layer((rhw_layer_e)(RHW1+rhw_layer));

  colon1->set_layer((rhw_layer_e)(RHW1+rhw_layer));
  colon2->set_layer((rhw_layer_e)(RHW1+rhw_layer));
  point->set_layer((rhw_layer_e)(RHW1+rhw_layer));
  bg->set_layer((rhw_layer_e)(RHW0+rhw_layer));
}


//////////////////////////////////////////////////////////////////////////////////////////////////

script_widget_holder_t::script_widget_holder_t( const char *_widget_name, widget *_parent, short _x, short _y )
  :  widget( _widget_name, _parent, _x, _y )
{
  set_rhw_2d_layer( RHW0 );
  set_rhw_3d_layer( RHW0 );

  my_timer_widget = NEW timer_widget( "timer", this, 320, 210 );
  my_timer_widget->ignore_parent_showing();
  running = true;

  restore_last_rhw_3d_layer();
  restore_last_rhw_2d_layer();
}

void script_widget_holder_t::frame_advance(time_value_t time_inc)
{
  if (this->running)
    widget::frame_advance(time_inc);
}













