// interface.cpp :: in-game interface functions

#include "global.h"

#include "wds.h"
// BIGCULL #include "gun.h"
#include "app.h"
#include "game.h"
#include "ostimer.h"
#include "widget.h"
#include "interface.h"
#include "controller.h"
#include "commands.h"
#include "localize.h"
#include "osdevopts.h"
#include "inputmgr.h"
#include "joystick.h"
// BIGCULL #include "damage_interface.h"
// BIGCULL #include "spiderman_controller.h"
#include "widget_entity.h"

#include <cstdarg>

extern game *g_game_ptr;

/*P
chicklet_bar::chicklet_bar( const char *_widget_name, widget *_parent, short _x, short _y, widget_dir_e _dir, const specs_t& _specs )
  : bar_widget( _widget_name, _parent, _x, _y, _dir ), specs( _specs )
{
  // New way
  chicklet_gfx = NEW buffered_bitmap_widget("chicklets", this, 0, 0, 1);

  rational_t tmp_width = ((specs.w + 1) * specs.num_chicks);
  rational_t tmp_height = (specs.h);

  chicklet_gfx->open("interface\\pcinterface");
  chicklet_gfx->set_screen_scale(2.0f, 1.0f);
  chicklet_gfx->set_subrect(specs.u_off, specs.v_off, specs.u_off + tmp_width, specs.v_off + tmp_height);

  //chicklet_gfx->resize( int((specs.w + specs.spacing) * (specs.num_chicks - 1) - specs.spacing), int(specs.h));

  /* Old way
  chicklets = NEW bitmap_widget*[specs.num_chicks];

  int cur_x = 0, cur_y = 0;
  if ( x_fac == -1 || y_fac == -1 )
  {
    cur_x += int( specs.w ) * x_fac;
    cur_y += int( specs.h ) * y_fac;
  }

  for ( int i = 0; i < specs.num_chicks; ++i )
  {
    chicklets[i] = NEW bitmap_widget( "chicklets", this, cur_x, cur_y, CHICKLET_Total,
      ONE_BIG_BLOCK_RENDER_STYLE);

    chicklets[i]->open( "interface\\healthchicklet" );

    chicklets[i]->resize( specs.w, specs.h );
    cur_x += x_fac * ( int( specs.w ) + specs.spacing );
    cur_y += y_fac * ( int( specs.h ) + specs.spacing );
  }* /
}


void chicklet_bar::update( rational_t _val, rational_t _full_val )
{
  assert( _val >= 0.0f && _full_val > 0.0f );

  // cap it
  if ( _val > _full_val )
  {
    _val = _full_val;
  }

  if ( val != _val || full_val != _full_val )
  {
    val = _val;
    full_val = _full_val;

    int num_active = int( rational_t( specs.num_chicks ) * val / full_val);

    set_chicklets( 0, num_active, specs.active );
/* Old Way
    set_chicklets( num_active, specs.num_chicks - 1, CHICKLET_Default );
* /
  }
}


void chicklet_bar::set_chicklets( int first, int last, const chicklet_e& index )
{
  rational_t tmp_width = ((specs.w + 1) * last);
  rational_t tmp_height = (specs.h);

  //chicklet_gfx->resize(tmp_width, tmp_height);
  chicklet_gfx->set_subrect(specs.u_off, specs.v_off, specs.u_off + tmp_width, specs.v_off + tmp_height);

  /* Old way
  for ( int i = first; i <= last; ++i )
  {
      chicklets[i]->set_frame( index );
  }* /
}


//------------------------------------------------------------------

/*** Replaced with NEW graphic for pc version ***

health_bg_widget::health_bg_widget( const char *_widget_name, widget *_parent, short _x, short _y, float _w, float _h, bool vert,
          stringx end_name, stringx stretch_name )
  : widget( _widget_name, _parent, _x, _y ), w( _w ), h( _h )
{
  bar_left = NEW bitmap_widget( "health-bar_left", this, 0, 0, 1 );
  bar_left->open( end_name.c_str() );
  float end_w = bar_left->get_width() * ( h / bar_left->get_height() ); // calculate end piece's width
  bar_left->resize( end_w, h );

  bar_right = NEW bitmap_widget( "health-bar_right", this, 0, 0, 1);
  bar_right->open( end_name.c_str() );
  bar_right->resize( end_w, h );
  bar_right->flip_horiz();

  // health-bar_stretch
  bar_stretch = NEW bitmap_widget( "health-bar_stretch", this, 0, 0, 1);
  bar_stretch->open( stretch_name.c_str() );
  bar_stretch->resize( w - 2 * end_w + 1, h );

  if ( vert )
  {
    bar_left->rotate_to( 90.0f * PI / 180.0f );
    bar_right->rotate_to( 90.0f * PI / 180.0f );
    bar_stretch->rotate_to( 90.0f * PI / 180.0f );

    bar_left->move_to( h, 0 );
    bar_stretch->move_to( h, end_w );
    bar_right->move_to( h, w - end_w );
  }
  else
  {
    bar_left->move_to( 0, 0 );
    int right_x = int( w ) - int( end_w );
    bar_right->move_to( right_x, 0 );
    int stretch_x = int( end_w );
    bar_stretch->move_to( stretch_x, 0 );
  }

}


void health_bg_widget::hide_bar_left()
{
  bar_left->hide();
  bar_left->ignore_parent_showing();
}

void health_bg_widget::hide_bar_right()
{
  bar_right->hide();
  bar_right->ignore_parent_showing();
}

void health_bg_widget::hide_bar_stretch()
{
  bar_stretch->hide();
  bar_stretch->ignore_parent_showing();
}
*/

//------------------------------------------------------------------

/*
status_widget::status_widget( const char *_widget_name, widget *_parent, short _x, short _y ) :
  widget( _widget_name, _parent, _x, _y )
{
  // bar factors
  float fluid_bar_margin_factor = 1.0f/48.0f;
  float chick_bar_margin_factor = 1.0f/56.0f;
  float bar_height_factor = 5.0f/7.0f;

  // nano bar bg dimensions
  float nano_bg_w = 230.0f;
  float nano_bg_h = 12.0f;

  // health bar bg dimensions
  float health_bg_w = 352.0f;
  float health_bg_h = 20.0f;

  // nano bar bg dimensions
  float armor_bg_w = nano_bg_w;
  float armor_bg_h = nano_bg_h;

  // nano bar bg coords
  short nano_bg_x = short( health_bg_w / 2 - nano_bg_w / 2 );
  short nano_bg_y = 0;

  // health bar bg coords
  short health_bg_x = 0;
  short health_bg_y = (short)nano_bg_h + 1;

  // armor bar bg coords
  short armor_bg_x = nano_bg_x;
  short armor_bg_y = (short)(health_bg_y + health_bg_h + 1);

  // nano fluid bar specs
  float nano_w  = int( nano_bg_w - (2 * nano_bg_w * fluid_bar_margin_factor) + 0.5f );
  if ( int(nano_w) % 2 != 0 ) --nano_w;  // make it even
  float nano_h  = nano_bg_h * bar_height_factor * 0.6f;
  int nano_x    = (int)(nano_bg_x + (nano_bg_w - nano_w)/2);
  int nano_y    = (int)(nano_bg_y + (nano_bg_h - nano_h)/2 - 4);

  // health bar chicklet specs
  chicklet_bar::specs_t h_specs;
  h_specs.num_chicks  = 20;
  h_specs.spacing     = 2;
  float health_margin = health_bg_w * chick_bar_margin_factor * 2.0f;
  float health_spaces = float(h_specs.spacing * (h_specs.num_chicks-1));
  float health_avail  = health_bg_w - health_margin - health_spaces;
  h_specs.w           = int( health_avail / float(h_specs.num_chicks) + 1.5f );

  // make width even
  int health_chick_bar_w = 0;
  //do
  //{
  //  --h_specs.w;
    health_chick_bar_w = (int)((h_specs.w * h_specs.num_chicks) + (h_specs.spacing * (h_specs.num_chicks-1)));
  //}
  //while ( health_chick_bar_w % 2 != 0.0f );
  h_specs.w = 8;
  h_specs.h          = health_bg_h * bar_height_factor;
  h_specs.active     = CHICKLET_Blue;

  // Offset into the texturemap (for the upper-left corner)
  h_specs.u_off = 3;
  h_specs.v_off = 17;

  // health bar chicklet bar coords
  int health_x       = (int)(health_bg_x + (health_bg_w - health_chick_bar_w) / 2.0f);
  int health_y       = (int)(health_bg_y + ((health_bg_h - h_specs.h) / 2.0f) - 2);

  // armor bar chicklet specs
  chicklet_bar::specs_t a_specs;
  a_specs.num_chicks = 5;
  a_specs.spacing    = 1;
  float armor_margin = armor_bg_w * chick_bar_margin_factor * 2.0f;
  float armor_spaces = float(a_specs.spacing * (a_specs.num_chicks-1));
  float armor_avail  = armor_bg_w - armor_margin - armor_spaces;
  a_specs.w          = int( armor_avail / float(a_specs.num_chicks) + 1.5f );
  // make width even
  int armor_chick_bar_w = 0;
  do
  {
    --a_specs.w;
    armor_chick_bar_w = (int)((a_specs.w * a_specs.num_chicks) + (a_specs.spacing * (a_specs.num_chicks-1)));
  }
  while ( armor_chick_bar_w % 2 != 0.0f );
  a_specs.w = 21;
  a_specs.h          = armor_bg_h * bar_height_factor;
  a_specs.active     = CHICKLET_Green;

  // texture offsets
  a_specs.u_off = 38;
  a_specs.v_off = 38;

  // armor bar chicklet bar coords
  int armor_x       = (int)(armor_bg_x + (armor_bg_w - armor_chick_bar_w) / 2.0f + 2);
  int armor_y       = (int)(armor_bg_y + ((armor_bg_h - a_specs.h) / 2.0f));

  // New health/nano/armor background...
  buffered_bitmap_widget *status_bg = NEW buffered_bitmap_widget("status_bg", this, -9, -4, 1);
  status_bg->open( "interface\\pcinterface" );
  status_bg->set_screen_scale(2.0f, 1.0f);
  status_bg->set_subrect(0, 52, 184, 100);

  // nano support arm specs
  //float nano_supp_w = health_bg_w / 8.0f;
  //float nano_supp_h = nano_bg_h * 0.75f;
  //short nano_supp_left_x = nano_bg_x - short( nano_supp_w );
  //short nano_supp_right_x = nano_bg_x + short( nano_bg_w );
  //short nano_supp_y = health_bg_y - short( nano_supp_h ) + 2;

  // armor support arm specs
  //float armor_supp_w = nano_supp_w;
  //float armor_supp_h = nano_supp_h;
  //short armor_supp_left_x = armor_bg_x - short( armor_supp_w / 2.0f );
  //short armor_supp_right_x = armor_bg_x + short( armor_bg_w - armor_supp_w / 2.0f );
  //short armor_supp_y = health_bg_y + health_bg_h - 3;

  // construct nano support arms
  //bitmap_widget *nano_supp_left = NEW buffered_bitmap_widget( "status-supp_left", this, nano_supp_left_x, nano_supp_y, 1 );
  //nano_supp_left->open( "interface\\healthbartopsupport" );
  //nano_supp_left->resize( nano_supp_w, nano_supp_h );

  //bitmap_widget *nano_supp_right = NEW buffered_bitmap_widget( "status-supp_right", this, nano_supp_right_x, nano_supp_y, 1 );
  //nano_supp_right->open( "interface\\healthbartopsupport" );
  //nano_supp_right->resize( nano_supp_w, nano_supp_h );
  //nano_supp_right->flip_horiz();

  // armor support arms
  //bitmap_widget *armor_supp_left = NEW buffered_bitmap_widget( "status-supp_left", this, armor_supp_left_x, armor_supp_y, 1 );
  //armor_supp_left->open( "interface\\healthbartopsupport" );
  //armor_supp_left->resize( armor_supp_w, armor_supp_h );
  //armor_supp_left->flip_vert();

  //bitmap_widget *armor_supp_right = NEW buffered_bitmap_widget( "status-supp_right", this, armor_supp_right_x, armor_supp_y, 1 );
  //armor_supp_right->open( "interface\\healthbartopsupport" );
  //armor_supp_right->resize( armor_supp_w, armor_supp_h );
  //armor_supp_right->flip_horiz();
  //armor_supp_right->flip_vert();

  // construct bgs
  //health_bg_widget *nano_bg = NEW health_bg_widget( "status-nano_bg", this, nano_bg_x, nano_bg_y, nano_bg_w, nano_bg_h, false );
  //health_bg_widget *health_bg = NEW health_bg_widget( "status-health_bg", this, health_bg_x, health_bg_y, health_bg_w, health_bg_h, false );
  //health_bg_widget *armor_bg = NEW health_bg_widget( "status-armor_bg", this, armor_bg_x, armor_bg_y, armor_bg_w, armor_bg_h, false );

  // construct nano empty
  //bitmap_widget *nano_empty = NEW buffered_bitmap_widget( "status-nano_empty", this, nano_x, nano_y, 1 );
  //nano_empty->open( "interface\\healthchicklet" );
  //nano_empty->resize( nano_w, nano_h );

  // construct nano bar
  nano_bar = NEW fluid_bar( "status-nano", this, nano_x, nano_y, WDIR_Right, nano_w, nano_h,
    "interface\\healthchickletnano5" );
  nano_bar->set_fill_rate( 1.0f );
  nano_bar->set_empty_rate( 2.0f );
  nano_bar->use_special_rate( 5.0f );
  nano_bar->get_bar_map()->set_num_frames( 8 );
  nano_bar->get_bar_map()->set_fps( 8 );
  nano_bar->get_bar_map()->play();


  // construct health bar
  health_bar = NEW chicklet_bar( "status-health", this, health_x, health_y, WDIR_Right, h_specs );

  // construct armor bar
  armor_bar = NEW chicklet_bar( "status-armor", this, armor_x, armor_y, WDIR_Right, a_specs );

  if (os_developer_options::inst()->get_int(os_developer_options::INT_DETAIL_LEVEL)>=2)
  {
    // highlights
    bitmap_widget *nano_highlight = NEW bitmap_widget( "status-nano_hi", this, nano_bg_x, nano_y, 1 );
    nano_highlight->open( "interface\\healthbarhilight" );
    nano_highlight->resize( nano_bg_w, nano_h );
  }

  // set initial state
  frame_advance( 0.0f );
}



void status_widget::frame_advance( time_value_t time_inc )
{
  if ( !is_shown() ) return;

STUBBED(status_widget_frame_advance, "status_widget::frame_advance");

/ *!
  entity* hero = g_world_ptr->get_hero_ptr();

  if ( hero )
  {
    rational_t full_val = hero->get_full_nanotech_energy(),
               to_val = hero->get_nanotech_energy();
    nano_bar->set_full_val( full_val );
    nano_bar->set_to_val( to_val );

    health_bar->update( hero->get_hit_points(),
                        hero->get_full_hit_points() );
    armor_bar->update(  hero->get_armor_points(),
                        hero->get_full_armor_points() );
  }
!* /
  widget::frame_advance( time_inc );
}



//------------------------------------------------------------------

static const rational_t ammo_top_g = 0.75f;
static const rational_t ammo_bot_g = 0.2f;

static const float ammo_cell_offset_y = -7.0f;

static const float ammo_widget_w = 40.0f;
static const float ammo_widget_h = 96.0f - ammo_cell_offset_y;
static const float ammo_fluid_w = ammo_widget_w - 18.0f;
static const float ammo_fluid_h = ammo_widget_h - 18.0f;
static const float ammo_fluid_x = 9.0f;
static const float ammo_fluid_y = 10.0f;

static const float ammo_highlight_w = 20.0f;
static const float ammo_highlight_h = 100.0f;
static const short ammo_highlight_x = 32;
static const short ammo_highlight_y = 0;


ammo_widget::ammo_widget( const char *_widget_name, widget *_parent, short _x, short _y ) :
  widget( _widget_name, _parent, _x, _y )
{
  //bg = NEW health_bg_widget( "ammo-health_bg", this, 0, 0, ammo_widget_h, ammo_widget_w, true,
  //  "interface\\ammobarbaseend", "interface\\ammobarbasestretch" ); // width, height reversed because on its side
  //bg->hide_bar_right();

  empty = NEW buffered_bitmap_widget( "ammo-empty", this, ammo_fluid_x-7, ammo_fluid_y-12, 1 );
  //empty->open( "interface\\healthchicklet" );
  empty->open( "interface\\pcinterface" );
  empty->set_subrect( 93, 104, 143, 224);
  empty->set_screen_scale(0.665f, 0.81f);
  //empty->resize( ammo_fluid_h, ammo_fluid_w + 7 );
  //empty->rotate_to( 90.0f * PI / 180.0f );

  bar = NEW fluid_bar( "ammo-fluid", this, ammo_fluid_x, ammo_fluid_y, WDIR_Up, ammo_fluid_w, ammo_fluid_h,
    "textures\\alpha" );
  set_bar_col( 1.0f );
  bar->set_fill_rate( 1.0f );
  bar->set_empty_rate( 2.0f );
  bar->use_special_rate( 2.0f );

  if (os_developer_options::inst()->get_int(os_developer_options::INT_DETAIL_LEVEL)>=2)
  {
    highlight = NEW buffered_bitmap_widget( "ammo-highlight", this, ammo_highlight_x, ammo_highlight_y, 1 );
    highlight->open( "interface\\healthbarhilight" );
    highlight->resize( ammo_highlight_h, ammo_highlight_w );
    highlight->rotate_to( 90.0f * PI / 180.0f );
    highlight->set_color( color( 1.0f, 1.0f, 1.0f, 0.5f ) );
  }

  buffered_bitmap_widget *bullet = NEW buffered_bitmap_widget( "ammo-bullet", this, ammo_fluid_x-30,
	  ammo_fluid_y+60, 1 );
  bullet->open( "interface\\pcinterface" );
  bullet->set_subrect( 154, 198, 255, 255);
  bullet->set_screen_scale(1.0f, 1.0f);
  //empty->resize( ammo_fluid_h, ammo_fluid_w + 7 );
  //empty->rotate_to( 90.0f * PI / 180.0f );

/ *  ammo_cell = NEW vrep_widget( "items\\entities\\ammocell01.tpm", "ammo_cell", this,
    int( ammo_widget_w / 2 ) + 5, int( ammo_widget_h ) + ammo_cell_offset_y );
  ammo_cell->set_radius( 42 );
  ammo_cell->set_rotation(vector3d(0, 0, 1), 25 * PI / 180.0f, 0);
* /
  amount = NEW text_widget( "ammo-amt", this, 20, 60, 0.85f, JUSTIFY_CENTER, 0, "interface\\frontendfont" );
}


void ammo_widget::frame_advance( time_value_t time_inc )
{
  if ( !is_shown() ) return;


STUBBED(ammo_widget_frame_advance, "ammo_widget::frame_advance");
/ *!
  entity* hero = g_world_ptr->get_hero_ptr();

  if ( hero )
  {
    rational_t full_val = hero->get_max_ammo_points(),
               to_val = hero->get_ammo_points();
    bar->set_full_val( full_val );
    bar->set_to_val( to_val );

    amount->set_string( 0 ); // default--don't show amount
    if ( to_val > 0.0f )
    {
      player_controller *hero_cont = (player_controller*)(g_world_ptr->get_hero_controller());
      if ( hero_cont )
      {
        character *chr = hero_cont->get_my_chr();
        if ( chr )
        {
          item *itm = chr->get_cur_item();
          if ( itm && itm->is_a_gun() )
          {
            int usage = ((gun*)itm)->get_ammo_usage();
            if ( usage == 0 )
              usage = 1;
            amount->set_string( stringx(int(to_val/usage)) ); // show shots left
          }
        }
      }
    }

    if ( full_val )
    {
      set_bar_col( to_val / full_val );
    }
  }
!* /
  widget::frame_advance( time_inc );
}


void ammo_widget::set_bar_col( rational_t fullness )
{
  assert( fullness >= 0.0f && fullness <= 1.0f );
  if ( bar )
  {
    color c[4];
    rational_t top_g = ammo_bot_g + ( ammo_top_g - ammo_bot_g ) * fullness;
    c[0] = color( 0.0f, top_g, 0.0f, 1.0f );
    c[1] = color( 0.0f, top_g, 0.0f, 1.0f );
    c[2] = color( 0.0f, ammo_bot_g, 0.0f, 1.0f );
    c[3] = color( 0.0f, ammo_bot_g, 0.0f, 1.0f );
   bar->set_color( c );
  }
}




//------------------------------------------------------------------



/ *!
boss_widget::boss_widget( const char *_widget_name, widget *_parent, short _x, short _y ) :
  widget( _widget_name, _parent, _x, _y )
{
  float hb_bg_w = 230.0f;
  float hb_bg_h = 20.0f;
  //hb_bg = NEW health_bg_widget( "boss-health_bg", this, 0, 0, hb_bg_w, hb_bg_h, false );

  int hb_x            = 8;
  int hb_y            = 4;
  float hb_w          = 214.0f;
  float hb_h          = 12.0f;
  hb_full = NEW bitmap_widget( "boss-full", this, hb_x, hb_y, 1 );
  hb_full->open( "interface\\healthchicklet" );
  hb_full->resize( hb_w, hb_h );
  hb_full->set_color( color( 0.6f, 0.6f, 0.6f, 1.0f ) );

  health_bar = NEW fluid_bar( "boss-bar", this, hb_x, hb_y, WDIR_Right, hb_w, hb_h,
    "textures\\alpha" );
  health_bar->set_color( color( 1.0f, 0.0f, 0.0f, 1.0f ) );
  health_bar->set_fill_rate( 1.0f );
  health_bar->set_empty_rate( 2.0f );
  health_bar->use_special_rate( 3.0f );

  if (os_developer_options::inst()->get_int(os_developer_options::INT_DETAIL_LEVEL)>=2)
  {
    hb_highlight = NEW bitmap_widget( "boss-hi", this, 0, hb_y - 2, 1 );
    hb_highlight->open( "interface\\healthbarhilight" );
    hb_highlight->resize( hb_bg_w, hb_h );
  }

  hide();
  boss = NULL;
}


void boss_widget::show()
{
  if ( boss )
  {
    set_health_bar_color( color( 1.0f, 0.0f, 0.0f, 1.0f ) );
    widget::show();
  }
}


void boss_widget::frame_advance( time_value_t time_inc )
{
  if ( !boss ) return;

  rational_t hp = boss->get_soft_attrib()->get_hit_points();
  if ( hp <= 0 )  // boss widget automatically hides when boss is killed
  {
    hide();
    boss = NULL;
  }
  else
  {
    rational_t full_val = boss->get_full_hit_points(),
               to_val = boss->get_soft_attrib()->get_hit_points();
    health_bar->set_full_val( full_val );
    health_bar->set_to_val( to_val );
  }

  widget::frame_advance( time_inc );
}

void boss_widget::set_health_bar_color( color col )
{
  health_bar->set_color( col );
}

void boss_widget::set_health_bar_val(rational_t val)
{
  health_bar->set_cur_val(val);
}
!* /


//------------------------------------------------------------------


score_widget::score_widget( const char *_widget_name, widget *_parent, short _x, short _y ) :
  widget( _widget_name, _parent, _x, _y )
{
  score = 0;

  score_bg = NEW text_widget( "score-bg", this, 0, 3, 1.0f, JUSTIFY_RIGHT, stringx( itos( score ) ) );
  score_bg->set_color( color( 0.5f, 0.5f, 0.5f, 1.0f ) );

  score_text = NEW text_widget( "score-text", this, 0, 0, 1.0f, JUSTIFY_RIGHT, stringx( itos( score ) ) );
  score_text->set_color( color( 0.75f, 0.75f, 0.75f, 1.0f ) );

  // set initial state of score
  last_score = -1;
  update();
}


void score_widget::set_score( int s )
{
  score = s;
  update();
}


void score_widget::inc_score( int inc )
{
  score += inc;
  if ( score > 9999999 ) score -= 9999999;
  update();
}


void score_widget::update()
{
  assert( score >= 0 );
  if ( score != last_score )
  {
    stringx zeroes = "0000000";
    stringx num = itos( score );
    assert( num.length() <= 7 );
    stringx sc = zeroes.substr( 0, 7-num.length() ) + num;
    score_text->set_string( sc );
    score_bg->set_string( sc );
  }
}




//------------------------------------------------------------------

float g_gun_scale_factor = 0.75f;
float g_glow_alpha = 0.0f;  // 0.3f is value C. Soares likes
float g_glow_scale_factor = 0.0f;  // was 0.75f
int g_gun_offset_x = 14;
int g_gun_offset_y = 20;

#define GLOW_BASE_WH          112.0f
#define GLOW_ACTIVE_WH        GLOW_BASE_WH
#define GLOW_DEFAULT_WH       ( GLOW_BASE_WH * g_glow_scale_factor )

#define GLOW_ACTIVE_COLOR  ( color(0.0f/255.0f, 149.0f/255.0f, 164.0f/255.0f, 0.7f) )
#define GLOW_DEFAULT_COLOR  GLOW_ACTIVE_COLOR //( color(0.0f/255.0f, 64.0f/255.0f, 164.0f/255.0f, g_glow_alpha) )
#define GLOW_MIDWAY_COLOR  ( color(0.0f/255.0f, 223.5f/255.0f, 246.0f/255.0f, 0.7f) )  // 50% brighter

static const short item_x = 23;  // 35
static const short item_y = 333; // 340
static const short item_orig_x = item_x + short( GLOW_BASE_WH ) / 2;
static const short item_orig_y = item_y + short( GLOW_BASE_WH ) / 2;
static const short arrow_y_off = 50;
static const short name_x = item_orig_x;
static const short name_y = item_orig_y - 65;
static const short cur_item_x = item_orig_x + g_gun_offset_x;
static const short cur_item_y = item_orig_y + g_gun_offset_y;

static const rational_t amt_text_act_scale = 1.0f;
static const rational_t amt_text_deact_scale = 0.7f;
static const short amt_text_x_off = 1;
static const short amt_text_y_off = 0;



item_widget::item_widget( const char *_widget_name, widget *_parent, short _x, short _y ) :
  widget( _widget_name, _parent, _x, _y )
{
  item_active = false;
  owner = NULL;
  orientation = 225.0f;

//  int i = 0; // unused -- remove me?

  bg = NEW bitmap_widget( "item-bg", this, item_orig_x - short( GLOW_DEFAULT_WH ) / 2 + g_gun_offset_x,
    item_orig_y - short( GLOW_DEFAULT_WH ) / 2 + g_gun_offset_y, 1 );
  bg->open( "interface\\weaponglow" );
  bg->set_color( GLOW_DEFAULT_COLOR );
  bg->resize( GLOW_DEFAULT_WH, GLOW_DEFAULT_WH );
  bg->set_linear_animation( false );

  // up-down arrows (normal)
  float but_rad = 24;
  up_but = NEW vrep_widget( "interface\\updownarrow.tpm", "item-up_but", this, item_orig_x, item_orig_y - arrow_y_off );
  up_but->set_radius( but_rad );
  up_but->set_rotation( 270 * PI / 180.0f, 0.0f, 0.0f );

  down_but = NEW vrep_widget( "interface\\updownarrow.tpm", "item-down_but", this, item_orig_x, item_orig_y + arrow_y_off );
  down_but->set_radius( but_rad );
  down_but->set_rotation( 270 * PI / 180.0f, 0.0f, PI );

  // up-down arrows (highlight)
  up_but_hi = NEW vrep_widget( "interface\\updownarrowlight.tpm", "item-up_hi", this, item_orig_x, item_orig_y - arrow_y_off );
  up_but_hi->set_radius( but_rad );
  up_but_hi->set_rotation( 270 * PI / 180.0f, 0.0f, 0.0f );

  down_but_hi = NEW vrep_widget( "interface\\updownarrowlight.tpm", "item-down_hi", this, item_orig_x, item_orig_y + arrow_y_off );
  down_but_hi->set_radius( but_rad );
  down_but_hi->set_rotation( 270 * PI / 180.0f, 0.0f, PI );

  up_but_hi->fade_to( 0.0f );
  down_but_hi->fade_to( 0.0f );

  up_but->ignore_parent_showing();
  down_but->ignore_parent_showing();
  up_but_hi->ignore_parent_showing();
  down_but_hi->ignore_parent_showing();

  current_item = NEW vrep_widget( "item-current", this, cur_item_x, cur_item_y, NULL );
  current_item->set_radius(48);
  current_item->fade_to( 0.5f );
  current_item->scale_to( g_gun_scale_factor );
  current_item->set_linear_animation( false );

  set_rhw_2d_layer( RHW6 );

  amount = NEW text_widget( "item-amt", this, cur_item_x + amt_text_x_off, cur_item_y + amt_text_y_off, amt_text_deact_scale, JUSTIFY_CENTER, 0, "interface\\frontendfont" );
  amount->fade_to( 0.5f );

  text_block_widget::block_info_t info;
  info.scale = 0.6f;
  info.max_lines = 3;
  info.line_spacing = 12;
  info.max_width = (short)GLOW_BASE_WH;
  name = NEW text_block_widget( "main-text", this, 0, 0, &info );

  restore_last_rhw_2d_layer();

  turn_selector_off();
}


void item_widget::show()
{
  widget::show();

  // Why is this necessary? It causes the player controller to mess up. If you pause within a jump, or crawling, it messes all up!!!
  // If this is truly necessary, please explain to me why... (JDB 8/30/00)
//  if ( g_world_ptr && g_world_ptr->get_hero_controller() )
//  {
//    ((player_controller*)(g_world_ptr->get_hero_controller()))->clear_c_index();
//  }

  turn_selector_off();
  sort();
}


void item_widget::add_item(item *cur_item)
{
  if ( !is_selector_on() || !owner || owner->get_num_items() == 0 )
  {
    sort(cur_item, false);
  }
}


void item_widget::add_perm_item()
{
//  sort_perm();
}


void item_widget::remove_perm_item()
{
//  sort_perm();
}


void item_widget::next_item(item *cur_item)
{
//// if this gets commented back in, you will also need to deal with item change in pda
//  if(g_world_ptr->get_dread_net())
//    g_world_ptr->get_dread_net()->add_cue(dread_net::CHANGE_INV, g_world_ptr->get_hero_ptr());

  sort(cur_item, true);
}


void item_widget::prev_item(item *cur_item)
{
//// if this gets commented back in, you will also need to deal with item change in pda
//  if(g_world_ptr->get_dread_net())
//    g_world_ptr->get_dread_net()->add_cue(dread_net::CHANGE_INV, g_world_ptr->get_hero_ptr());

  sort(cur_item, true);
}


void item_widget::use_item(item *cur_item)
{
  sort(cur_item, false);
}


//item *g_test_item = NULL;

void item_widget::sort( item *cur_item, bool spin )
{
  if ( !owner ) return;

STUBBED(handheld_item_holster, "item_widget::sort");

/ *!
  item *itm = (cur_item == NULL) ? owner->get_cur_item() : cur_item;
  int num = itm ? itm->get_number() : 0;

  if ( owner->get_num_items() > 0 && itm && num > 0 )
  {
    // set vrep
    current_item->set_vrep( itm->get_vrep() );
    // set amount
    if ( num > 1 && ( !itm->is_a_handheld_item() || !((handheld_item*)itm)->is_a_radio_detonator() ) )
    {
      amount->set_string( stringx(num) );
    }
    else
    {
      amount->set_string( 0 );
    }

    // set name
    stringx n = "$" + itm->get_name();
    n.to_upper();
    name->set_text( n );

    // adjust name pos. as needed
    short name_new_x = name_x - int( name->get_width()/2.0f + 0.5f );
    if ( name_new_x < item_x )
      name_new_x = item_x;
    name->move_to( name_new_x, name_y - name->get_height() );

    // set orientation
    orientation = itm->get_interface_orientation();
    current_item->rotate_to( 0.0f, 0.0f, orientation * PI / 180.0f, vector3d(0, 1, 0) );

    if ( spin )
    {
      spin_item();
    }
    bg->show();
  }
  else
  {
    current_item->set_vrep( NULL );
    amount->set_string( 0 );
    name->set_text( 0 );
    bg->hide();
  }
!* /
}


void item_widget::spin_item()
{
  current_item->rotate_to( 0.0f, 0.0f, (orientation - 359.0f) * PI / 180.0f, vector3d(0, 1, 0) );
  current_item->rotate_to( 0.0f, 0.25f, orientation * PI / 180.0f, vector3d(0, 1, 0) );
}



void item_widget::activation_effect()
{
  if ( item_active ) return;

  bg->flush();
  current_item->flush();
  amount->flush();

  // color shift
  bg->set_color( 0.0f, 0.1f, GLOW_MIDWAY_COLOR );
  bg->set_color( 0.1f, 0.15f, GLOW_ACTIVE_COLOR );
  current_item->fade_to( 1.0f );
  amount->fade_to( 1.0f );

  // scale shift
  bg->scale_to( 0.0f, 0.25f, GLOW_ACTIVE_WH / 64.0f );
  bg->move_to( 0.0f, 0.25f, item_orig_x - short( GLOW_ACTIVE_WH ) / 2, item_orig_y - short( GLOW_ACTIVE_WH ) / 2 );
  current_item->scale_to( 0.0f, 0.25f, 1.0f );
  current_item->move_to( 0.0f, 0.25f, item_orig_x, item_orig_y );
  amount->scale_to( amt_text_act_scale );
  amount->move_to( 0.0f, 0.25f, item_orig_x + amt_text_x_off, item_orig_y + amt_text_y_off );

  spin_item();
  item_active = true;
}



void item_widget::deactivation_effect()
{
  if ( !item_active ) return;

  bg->flush();
  current_item->flush();

  // color shift
  bg->set_color( 0.0f, 0.25f, GLOW_DEFAULT_COLOR );
  current_item->fade_to( 0.5f );
  amount->fade_to( 0.5f );

  // scale shift & move
  bg->scale_to( 0.0f, 0.25f, GLOW_DEFAULT_WH / 64.0f );
  bg->move_to( 0.0f, 0.25f, item_orig_x - short( GLOW_DEFAULT_WH ) / 2 + g_gun_offset_x,
    item_orig_y - short( GLOW_DEFAULT_WH ) / 2 + g_gun_offset_y );
  current_item->scale_to( 0.0f, 0.25f, g_gun_scale_factor );
  current_item->move_to( 0.0f, 0.25f, cur_item_x, cur_item_y );
  amount->scale_to( amt_text_deact_scale );
  amount->move_to( 0.0f, 0.25f, cur_item_x + amt_text_x_off, cur_item_y + amt_text_y_off );

  spin_item();
  item_active = false;
}


void item_widget::set_current_item_color(rational_t r, rational_t g, rational_t b, rational_t a)
{
  current_item->set_color(color(r, g, b, a));
}



void item_widget::turn_selector_on()
{
  up_but->show();
  up_but_hi->show();
  down_but->show();
  down_but_hi->show();

  name->show();
  name->fade_to( 0.0f );
  name->fade_to( 0.0f, 0.2f, 1.0f );

  selector_on = true;
}



void item_widget::turn_selector_off()
{
  up_but->hide();
  up_but_hi->hide();
  down_but->hide();
  down_but_hi->hide();

  name->hide();

  selector_on = false;
}



void item_widget::flash_up_but()
{
  if ( up_but_hi->wevent_run_list_empty() )
  {
    up_but_hi->fade_to( 0.0f, 0.1f, 1.0f );
    up_but_hi->fade_to( 0.2f, 0.1f, 0.0f );

    up_but->fade_to( 0.0f, 0.1f, 0.0f );
    up_but->fade_to( 0.2f, 0.1f, 1.0f );

    g_sound_group_list.play_sound_group( "menu_button" );
  }
}


void item_widget::flash_down_but()
{
  if ( down_but_hi->wevent_run_list_empty()  )
  {
    down_but_hi->fade_to( 0.0f, 0.1f, 1.0f );
    down_but_hi->fade_to( 0.2f, 0.1f, 0.0f );

    down_but->fade_to( 0.0f, 0.1f, 0.0f );
    down_but->fade_to( 0.2f, 0.1f, 1.0f );

    g_sound_group_list.play_sound_group( "menu_button" );
  }
}

//------------------------------------------------------------------

static const short bull_text_offset_y = 30;


bulletin_widget::bulletin_widget( const char *_widget_name, widget *_parent, short _x, short _y ) :
  widget( _widget_name, _parent, _x, _y )
{
  text_scale = 1.0f;
  text_col = color( 1.0f, 1.0f, 1.0f, 1.0f );
  drop_shad_col = color( 0.5f, 0.5f, 0.5f, 1.0f );
  drop_shad_x = 1;
  drop_shad_y = 1;

  set_rhw_3d_layer( RHW2 );
  item = NEW vrep_widget( "bulletin-item", this, 0, 0, NULL );
  item->set_radius(48);
  vector3d rotation_vect(0, 1, 0);
  item->set_rotation(rotation_vect, 180 * PI / 180.0f, 0);
  item->fade_to( 0.7f );
  restore_last_rhw_3d_layer();

  set_rhw_2d_layer( RHW9 );
  drop_shad = NEW text_widget( "bulletin-drop_shad", this, drop_shad_x, bull_text_offset_y + drop_shad_y, text_scale );
  drop_shad->set_color( drop_shad_col );

  text = NEW text_widget( "bulletin-text", this, 0, bull_text_offset_y, text_scale );
  text->set_color( text_col );
  restore_last_rhw_2d_layer();
}


void bulletin_widget::frame_advance( time_value_t time_inc )
{
  if ( !is_shown() ) return;

  widget::frame_advance( time_inc );

  if ( abs_col[0].a == 0.0f )
  {
    hide();
  }
}


void bulletin_widget::initiate( stringx name, visual_rep *vr,
  time_value_t in_time, time_value_t  up_time, time_value_t out_time )
{
  show();

  item->set_vrep( vr );

  name.to_upper();
  stringx disp = "$" + name;
  text->set_string( disp );
  drop_shad->set_string( disp );

  flush();

  // fade
  fade_to( 0.01f );
  fade_to( 0.0f, in_time, 1.0f );
  fade_to( in_time + up_time, out_time, 0.0f );
}





//------------------------------------------------------------------

static const float nano_widget_w = 320.0f;
static short nano_widget_start_x = -short(nano_widget_w);
static short nano_widget_end_x = 20;
static short nano_widget_y = 80;

static const short box_w = 90;
static const short box_h = 90;
static const short box_x = 18;
static const short box_y[] = { 50, 185 };

static const color nano_grey( 16.0f / 255.0f, 16.0f / 255.0f, 16.0f / 255.0f, 0.7f );

nano_widget::nano_widget( const char *_widget_name, widget *_parent, short _x, short _y ) :
  widget( _widget_name, _parent, _x, _y )
{
  stealth_launched = turbo_launched = false;

  set_rhw_2d_layer( RHW6 );

  float bg_h = 120.0f;
  short bg_x = 0;
  short bg_y = 0;

  bg = NEW bitmap_widget( "nano-bg", this, bg_x + short(bg_h), bg_y, 1 );
  bg->open( "interface\\bignanobar" );
  bg->resize( nano_widget_w, bg_h );
  bg->rotate_to( 90.0f * PI / 180.0f );

  int i;
  bitmap_widget *box_bg[2];
  for ( i = 0; i < 2; ++i )
  {
    box_bg[i] = NEW bitmap_widget( "nano-box bg", this, box_x, box_y[i], 1 );
    box_bg[i]->open( "textures\\alpha" );
    box_bg[i]->resize( box_w, box_h );
    box_bg[i]->set_color( nano_grey );

    box[i] = NEW box_widget( "nano-box", this, box_x, box_y[i], box_w, box_h );

    // to block out the box behind the buttons
    block_out[i] = NEW bitmap_widget( "nano-block", this, box_x, box_y[i], 1 );
    block_out[i]->open( "textures\\alpha" );
    block_out[i]->resize( 15, 15);
    block_out[i]->set_color( color( 0.0f, 0.0f, 0.0f, 1.0f ) );

    short but_off_x = -12;
    short but_off_y = -13;

    a_but[i] = NEW bitmap_widget( "nano-a", this, box_x + but_off_x, box_y[i] + but_off_y, 1 );
    a_but[i]->open( "interface\\buta" );

    b_but[i] = NEW bitmap_widget( "nano-b", this, box_x + but_off_x, box_y[i] + but_off_y, 1 );
    b_but[i]->open( "interface\\butb" );

    x_but[i] = NEW bitmap_widget( "nano-x", this, box_x + but_off_x, box_y[i] + but_off_y, 1 );
    x_but[i]->open( "interface\\butx" );

    y_but[i] = NEW bitmap_widget( "nano-y", this, box_x + but_off_x, box_y[i] + but_off_y, 1 );
    y_but[i]->open( "interface\\buty" );

    text_block_widget::block_info_t info;
    info.scale = 0.4f;
    info.max_lines = 2;
    info.line_spacing = 10;
    info.max_width = 1;
    trig[i] = NEW text_block_widget( "nano-trig", this, 0, 0, &info );
  }

  short text_off_x = short( box_w ) / 2;
  short text_off_y = 68;
  turbo_text = NEW text_widget( "nano-turbo_text", this, box_x + text_off_x, box_y[0] + text_off_y, 0.5f,
    JUSTIFY_CENTER, "$TURBO" );
  stealth_text = NEW text_widget( "nano-stealth_text", this, box_x + text_off_x, box_y[1] + text_off_y, 0.5f,
    JUSTIFY_CENTER, "$STEALTH" );


  short stealth_x = box_x + short( box_w ) / 2 - 32;
  short stealth_y = box_y[1] + 4;
  stealth1 = NEW bitmap_widget( "nano-stealth1", this, stealth_x, stealth_y, 1 );
  stealth1->open( "interface\\stealth1" );

  stealth2 = NEW bitmap_widget( "nano-stealth2", this, stealth_x, stealth_y, 1 );
  stealth2->open( "interface\\stealth2" );
  stealth2->set_color( color( 1.0f, 1.0f, 1.0f, 0.0f ) );

  short turbo_x = stealth_x;
  short turbo_y = box_y[0] + 4;
  turbo1 = NEW bitmap_widget( "nano-turbo1", this, turbo_x, turbo_y, 1 );
  turbo1->open( "interface\\turbo1" );

  turbo2 = NEW bitmap_widget( "nano-turbo2", this, turbo_x, turbo_y, 1 );
  turbo2->open( "interface\\turbo2" );
  turbo2->fade_to( 0.0f );

  short spark_offset_x[NUM_NANO_SPARKS] = { 16, 27, 38, 30, 45, 38, 28, 46, 35, 16 };
  short spark_offset_y[NUM_NANO_SPARKS] = { 19, 15, 21, 28, 29, 34, 40, 42, 46, 37 };
  for ( i = 0; i < NUM_NANO_SPARKS; ++i )
  {
    sparks[i] = NEW bitmap_widget( "nano-sparks", this, turbo_x + spark_offset_x[i] - 3,
      turbo_y + spark_offset_y[i] - 3, 1 );
    sparks[i]->open( "interface\\turbospark" );
    sparks[i]->resize( 4.0f, 4.0f );
    sparks[i]->set_origin( 2, 2 );
    sparks[i]->set_color( color( 1.0f, 1.0f, 1.0f, 0.0f ) );
  }

  flare = NEW bitmap_widget( "nano-flare", this, 0, 0, 1 );
  flare->open( "interface\\lensflarebase" );
  flare->ignore_parent();
  flare->hide();

  restore_last_rhw_2d_layer();
}


#define A_COLOR   color( 204.0f/255.0f, 51.0f/255.0f, 51.0f/255.0f, 1.0f )
#define B_COLOR   color( 51.0f/255.0f, 68.0f/255.0f, 153.0f/255.0f, 1.0f )
#define X_COLOR   color( 255.0f/255.0f, 221.0f/255.0f, 0.0f/255.0f, 1.0f )
#define Y_COLOR   color( 0.0f/255.0f, 119.0f/255.0f, 34.0f/255.0f, 1.0f )
#define TRIG_COLOR   color( 0.4f, 0.4f, 0.4f, 1.0f )

void nano_widget::show()
{
  stealth_launched = turbo_launched = false;

  flush();
  widget::show();

  set_color( color( 1.0f, 1.0f, 1.0f, 1.0f ) );

  move_to( nano_widget_start_x, nano_widget_y );

  // fade in with flare effect
  //flare->ignore_parent();
  //fade_to( 0.0f );
  //fade_to( 0.25f, 0.5f, 1.0f );
  //do_flare();

  // set up displayed buttons based on current mapping
  axis_id_t mapping[2];
  mapping[0] = g_game_ptr->get_optionsfile()->get_cont_config( CONFIG_ATTACK ); // turbo
  mapping[1] = g_game_ptr->get_optionsfile()->get_cont_config( CONFIG_KICK ); // stealth

  for ( int i = 0; i < 2; ++i )
  {
    block_out[i]->show();
    a_but[i]->hide();
    b_but[i]->hide();
    x_but[i]->hide();
    y_but[i]->hide();
    trig[i]->hide();

    switch ( mapping[i] )
    {
    case JOY_BTNA:
      {
        a_but[i]->show();
        box[i]->set_color( A_COLOR );
      }
      break;
    case JOY_BTNB:
      {
        b_but[i]->show();
        box[i]->set_color( B_COLOR );
      }
      break;
    case JOY_BTNX:
      {
        x_but[i]->show();
        box[i]->set_color( X_COLOR );
      }
      break;
    case JOY_BTNY:
      {
        y_but[i]->show();
        box[i]->set_color( Y_COLOR );
      }
      break;
    case JOY_BTNR:
      {
        block_out[i]->hide();
        trig[i]->show();
        trig[i]->set_text( "$RightTrigger" );
        trig[i]->move_to( box_x + box_w/2 - trig[i]->get_width()/2,
                                box_y[i] - trig[i]->get_height() - 2 );
        box[i]->set_color( TRIG_COLOR );
      }
      break;
    case JOY_BTNL:
      {
        block_out[i]->hide();
        trig[i]->show();
        trig[i]->set_text( "$LeftTrigger" );
        trig[i]->move_to( box_x + box_w/2 - trig[i]->get_width()/2,
                                box_y[i] - trig[i]->get_height() - 2 );
        box[i]->set_color( TRIG_COLOR );
      }
      break;
    default:
      break;
    }
  }
}


void nano_widget::hide()
{
  move_to( nano_widget_start_x, nano_widget_y );
  widget::hide();
}


void nano_widget::frame_advance( time_value_t time_inc )
{
  if ( !is_shown() ) return;

  animate_stealth();
  animate_turbo();

  widget::frame_advance( time_inc );
}

float g_flare_wait_time = 0.0f;
float g_flare_total_time = 0.5f;
float g_flare_rampup_time = 0.0f;
float g_flare_rampdown_time = 0.5f;
float g_flare_start_scale = 1.0f;
float g_flare_mid_scale = 1.0f;
float g_flare_end_scale = 1.0f;

short g_flare_distance = 400;
short g_flare_offset_x = -20;
short g_flare_offset_y = 18;


void nano_widget::do_flare()
{
  short rampup_distance = short( float(g_flare_distance) * ( g_flare_rampup_time / g_flare_total_time ) );
  short start_offset_y = short( -( flare->get_height() * g_flare_start_scale ) / 2.0f );
  short rampup_offset_y = short( -( flare->get_height() * g_flare_mid_scale ) / 2.0f );
  short end_offset_y = short( -( flare->get_height() * g_flare_end_scale ) / 2.0f );

  flare->move_to( g_flare_offset_x, start_offset_y + g_flare_offset_y );
  flare->move_to( g_flare_wait_time, g_flare_rampup_time, rampup_distance + g_flare_offset_x,
    rampup_offset_y + g_flare_offset_y );
  flare->move_to( g_flare_wait_time + g_flare_rampup_time,g_flare_rampdown_time, g_flare_distance + g_flare_offset_x,
    end_offset_y + g_flare_offset_y );

  flare->scale_to( g_flare_start_scale );
  flare->scale_to( g_flare_wait_time, g_flare_rampup_time, g_flare_mid_scale );
  flare->scale_to( g_flare_wait_time + g_flare_rampup_time, g_flare_rampdown_time, g_flare_end_scale );

  flare->set_color( color( 1.0f, 1.0f, 1.0f, 0.0f ) );
  flare->set_color( g_flare_wait_time, g_flare_rampup_time, color( 1.0f, 1.0f, 1.0f, 1.0f ) );
  flare->set_color( g_flare_wait_time + g_flare_rampup_time, g_flare_rampdown_time, color( 1.0f, 1.0f, 1.0f, 0.0f ) );
}

float g_stealth_rampup = 0.8f;
float g_stealth_on_time = 0.0f;

void nano_widget::animate_stealth()
{
  if ( turbo_launched ) return;

  // wait till previous events are done
  if ( stealth2->wevent_run_list_empty() )
  {
    stealth2->set_color( 0.0f, g_stealth_rampup, color( 1.0f, 1.0f, 1.0f, 0.0f ) );
    stealth2->set_color( g_stealth_rampup + g_stealth_on_time, g_stealth_rampup, color( 1.0f, 1.0f, 1.0f, 1.0f ) );
  }
}

#define RAND_PCT (((float) (random(1000)))/1000)

void nano_widget::animate_turbo()
{
  if ( stealth_launched ) return;

  // wait till previous events are done
  if ( turbo2->wevent_run_list_empty() )
  {
    turbo2->fade_to( 0.0f, 0.1f, 1.0f );
    turbo2->fade_to( 0.1f, 0.1f, 0.0f );
  }

  // sparks
  for ( int i = 0; i < NUM_NANO_SPARKS; ++i )
  {
    if ( sparks[i]->wevent_run_list_empty() )
    {
      float wait = RAND_PCT * 0.25f;
      float rampup = 0.1f;
      float rampdown = RAND_PCT + 0.1f;
      float dur = 0.1f;

      sparks[i]->set_color( wait, rampup, color( 1.0f, 1.0f, 1.0f, 1.0f ) );
      sparks[i]->set_color( wait + rampup + dur, rampdown, color( 1.0f, 1.0f, 1.0f, 0.0f ) );
      sparks[i]->scale_to( wait, rampup, 0.6f );
      sparks[i]->scale_to( wait + rampup + dur, rampdown, 0.1f );
    }
  }
}


void nano_widget::hit_stealth()
{
  if ( is_shown() && g_world_ptr->get_hero_ptr() && !stealth_launched && !turbo_launched )
  {
    stealth_launched = true;
    flush();
    move_to( 0.4f, 0.2f, nano_widget_start_x, nano_widget_y );
    set_next_state( WSTATE_Hide, 0.6f );
    g_sound_group_list.play_sound_group( "menu_button" );

    // flash stealth box
    time_value_t wait = 0.0f;
    for ( int i = 0; i < 3; ++i )
    {
      / *
      stealth_but->set_color( wait, 0.1f, color( 0.2f, 0.2f, 0.2f, 1.0f ) );
      stealth_but->set_color( wait + 0.1f, 0.1f, color( 1.0f, 1.0f, 1.0f, 1.0f ) );

      box[1]->set_color( wait, 0.1f, color( 0.0f, 1.0f, 0.0f, 1.0f ) );
      box[1]->set_color( wait + 0.1f, 0.1f, color( 0.0f, 1.0f, 0.0f, 1.0f ) );
      * /

      wait += 0.2f;
    }
  }
}



void nano_widget::hit_turbo()
{
  if ( is_shown() && g_world_ptr->get_hero_ptr() && !stealth_launched && !turbo_launched )
  {
    turbo_launched = true;
    flush();
    move_to( 0.4f, 0.2f, nano_widget_start_x, nano_widget_y );
    set_next_state( WSTATE_Hide, 0.6f );
    g_sound_group_list.play_sound_group( "menu_button" );

    // flash turbo box
    time_value_t wait = 0.0f;
    for ( int i = 0; i < 3; ++i )
    {
      / *
      turbo_but->set_color( wait, 0.1f, color( 0.2f, 0.2f, 0.2f, 1.0f ) );
      turbo_but->set_color( wait + 0.1f, 0.1f, color( 1.0f, 1.0f, 1.0f, 1.0f ) );

      box[0]->set_color( wait, 0.1f, color( 0.0f, DIM_COLOR * 0.2f, 0.0f, 1.0f ) );
      box[0]->set_color( wait + 0.1f, 0.1f, color( 0.0f, 1.0f, 0.0f, 1.0f ) );
      * /

      wait += 0.2f;
    }
  }
}


//------------------------------------------------------------------

impact_widget::impact_widget(const char *_widget_name, widget *_parent, short _x, short _y) :
  widget(_widget_name, _parent, _x, _y)
{
  static int flash_pos[NUM_DIRS][2] = { { 262, 86 }, { 428, 174 }, { 390, 410 }, { 224, 302 } };

  set_rhw_2d_layer( RHW1 );

  for (int i = 0; i < NUM_DIRS; ++i)
  {
    flash[i] = NEW bitmap_widget("impact", this, flash_pos[i][0], flash_pos[i][1], 1);
    flash[i]->open("interface\\impact");
    flash[i]->rotate_to(i * PI/2);
    amount[i] = 0;
  }

  restore_last_rhw_2d_layer();
}

void impact_widget::show()
{
  flush();
  set_indicator();
}

static rational_t impact_decay_rate = 15;        // determines how quickly the impact graphic fades
static rational_t impact_display_ratio = 30;     // determines alpha translucency based on amount of damage

void impact_widget::frame_advance(time_value_t time_inc)
{
  for (int i = 0; i < NUM_DIRS; ++i)
  {
    amount[i] -= time_inc * impact_decay_rate;
    if (amount[i] < 0) amount[i] = 0;
  }

  set_indicator();

  widget::frame_advance(time_inc);
}

void impact_widget::indicate_damage(rational_t dam, const vector3d &from)
{
  vector3d diff = from - g_world_ptr->get_hero_ptr()->get_abs_position();
  rational_t mag = diff.length();

  int dir;
  if (mag < 0.00001f)
    dir = TOP;
  else
  {
    const vector3d &facing = g_world_ptr->get_hero_ptr()->get_abs_po().get_facing();
    diff /= mag;
    rational_t dp = dot(diff, facing);
    if (dp > 0.707f)  // 0.707 == cos(PI/4)
      dir = TOP;
    else if (dp < -0.707f)
      dir = BOTTOM;
    else if (diff.x * facing.z - diff.z * facing.x > 0) // 2D perp dot product
      dir = RIGHT;
    else
      dir = LEFT;
  }

  if (amount[dir]<0.1f*impact_display_ratio) amount[dir]=0.1f*impact_display_ratio;
  amount[dir] += dam;

  set_indicator();
}

void impact_widget::set_indicator()
{
  bool should_hide = true;

  for (int i = 0; i < NUM_DIRS; ++i)
  {
    rational_t a = amount[i] * (1.0f/impact_display_ratio);
    if (a > 1.0f) a = 1.0f;
    else if (a < 0.0f) a = 0.0f;

    flash[i]->set_color(color(1.0f, 1.0f, 1.0f, a));
    //flash[i]->set_color(color(1.0f, 0.0f, 0.0f, a));

    if (a > 0)
      should_hide = false;
  }

  if (should_hide)
    widget::hide();
  else
    widget::show();
}
*/
//------------------------------------------------------------------
rational_t HEIGHT_CHICKLET_X_SCALE = 16.0f;
rational_t HEIGHT_CHICKLET_Y_SCALE = 15.0f;
rational_t HEIGHT_SPIDER_X_SCALE = 50.0f;
rational_t HEIGHT_SPIDER_Y_SCALE = 40.0f;
rational_t COMPASS_FRAME_X_SCALE = 120.0f;
rational_t COMPASS_FRAME_Y_SCALE = 120.0f;
int HEIGHT_SPIDER_X_OFFSET = -17;
int HEIGHT_SPIDER_Y_OFFSET = -6;
int BAR_HEIGHT = 6;
int BAR_X_OFFSET = 57;
int BAR_Y_OFFSET = -65;
int COMPASS_FRAME_X_OFFSET = -40; // -76
int COMPASS_FRAME_Y_OFFSET = 10; // -47
float COMPASS_FRAME_SCALE = 1.2f;
float COMPASS_ARROW_SCALE = 0.6f;

#if 0 // BIGCULL
spiderman_compass_widget::spiderman_compass_widget(const char *_widget_name, widget *_parent, short _x, short _y )
 : widget(_widget_name, _parent, _x, _y)
{
	//compass_bar = NEW vrep_widget( "interface\\spidey_compass_bar.tpm", "interface-compass_bar", this, 0, 0 );
	//compass_frame = NEW vrep_widget( "interface\\entities\\spidey_compass_frame.txtmesh", "interface-compass_frame", this, 0, 0 );
	//compass_frame->scale_to(1.8f);
	//compass_frame->set_rotation(DEG_TO_RAD(45.0f), 0.0f, 0.0f);
  set_rhw_3d_layer( RHW0 );
  compass_frame = NEW vrep_widget( "interface\\entities\\spidey_compass_frame.txtmesh", "interface-compass_frame", this, 0, 0 );
  compass_frame->scale_to(COMPASS_FRAME_SCALE);//(1.8f);
	compass_frame->move_to(COMPASS_FRAME_X_OFFSET, COMPASS_FRAME_Y_OFFSET);
  compass_frame->set_rotation(DEG_TO_RAD(45.0f), 0.0f, 0.0f);
	restore_last_rhw_3d_layer();

  set_rhw_3d_layer( RHW1 );
  compass_arrow = NEW vrep_widget( "interface\\entities\\spidey_compass_arrow.txtmesh", "interface-compass_arrow", this, 0, 0 );
  compass_arrow->set_rotation(DEG_TO_RAD(45.0f), 0.0f, 0.0f);
  compass_arrow->scale_to(COMPASS_ARROW_SCALE);//(0.9f);
	compass_arrow->move_to(COMPASS_FRAME_X_OFFSET, COMPASS_FRAME_Y_OFFSET);
	restore_last_rhw_3d_layer();

/*  char compass_frame_str[256];
	strcpy(compass_frame_str, "interface\\");
	strcat(compass_frame_str, os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR).c_str());
	strcat(compass_frame_str, "\\spidercompass");
	set_rhw_2d_layer( RHW0 );
	compass_frame = NEW bitmap_widget("spidercompass",this, 0, 0, 1);
	compass_frame->open(compass_frame_str);
	compass_frame->move_to(COMPASS_FRAME_X_OFFSET, COMPASS_FRAME_Y_OFFSET);
	compass_frame->resize(COMPASS_FRAME_X_SCALE, COMPASS_FRAME_Y_SCALE);
	restore_last_rhw_2d_layer();
*/
//  compass_ud_arrow = NEW vrep_widget( "interface\\spidey_compass_arrowupdown.tpm", "interface-compass_ud_arrow", this, 75, 0 );
//  compass_ud_arrow->set_rotation(DEG_TO_RAD(90.0f), 0.0f, 0.0f);
  //compass_ud_arrow->set_color(color(1.0f, 1.0f, 1.0f, 0.0f));

  char height_chicklet_str[256];
  strcpy(height_chicklet_str, "interface\\");
  strcat(height_chicklet_str, os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR).c_str());
  strcat(height_chicklet_str, "\\heightchicklet");
  set_rhw_2d_layer( RHW0 );
  height_chicklet = NEW bitmap_widget( "height-chicklet", this, 0, 0, 1 );
  height_chicklet->open( height_chicklet_str );
//  height_chicklet->move_to(HEIGHT_CHICKLET_X_OFFSET, HEIGHT_CHICKLET_Y_OFFSET);
  height_chicklet->resize(HEIGHT_CHICKLET_X_SCALE, HEIGHT_CHICKLET_Y_SCALE);
  restore_last_rhw_2d_layer();


/*  for(int i=0; i < COMPASS_NUM_BARS; ++i)
  {
//    compass_bar[i] = NEW vrep_widget( "interface\\entities\\spidey_compass_bar.txtmesh", "interface-compass_bar", this, 75, BAR_OFFSET + ((-COMPASS_NUM_BARS * BAR_HEIGHT) + i*(BAR_HEIGHT * 2)) );
    compass_bar[i] = NEW bitmap_widget( "interface\\heightchicklet", "interface-compass_bar", this, 75, BAR_OFFSET + ((-COMPASS_NUM_BARS * BAR_HEIGHT) + i*(BAR_HEIGHT * 2)) );
    compass_bar[i]->set_color(color(0.0f, 0.0f, 1.0f, 1.0f));
    if(i == 0)
      compass_bar[i]->set_color(color(0.0f, 1.0f, 1.0f, 1.0f));
//    compass_bar[i]->scale_to(0.25f);
  }
*/
  char height_spider_str[256];
  strcpy(height_spider_str, "interface\\");
  strcat(height_spider_str, os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR).c_str());
  strcat(height_spider_str, "\\heightspider");
  set_rhw_2d_layer( RHW1 );
  height_spider = NEW bitmap_widget( "height-spider", this, 0, 0, 1 );
  height_spider->open( height_spider_str );
//  height_spider->move_to(HEIGHT_CHICKLET_X_OFFSET, height_spider_Y_OFFSET);
  height_spider->resize(HEIGHT_SPIDER_X_SCALE, HEIGHT_SPIDER_Y_SCALE);
  restore_last_rhw_2d_layer();

//  compass_spider = NEW vrep_widget( "interface\\entities\\spidey_compass_spider.txtmesh", "interface-compass_spider", this, 75, BAR_Y_OFFSET );

/*  compass_spider = NEW bitmap_widget( "interface\\height_spider", "interface-compass_spider", this, 75, BAR_OFFSET );
  compass_spider->scale_to(0.5f);
  compass_spider->set_color(color(1.0f, 0.0f, 0.0f, 0.75f));

  restore_last_rhw_2d_layer();
*/
  //elevation = NEW text_widget( "interface-compass_elevation", this, 35, 0, 0.5f, JUSTIFY_LEFT, 0, "interface\\frontendfont" );
  //elevation->set_color( color( 0.0f, 1.0f, 0.0f, 1.0f ) );

//  distance = NEW text_widget( "interface-compass_distance", this, 0, 55, 0.85f, "interface\\frontendfont" );
//  distance->set_color( color( 0.0f, 1.0f, 0.0f, 1.0f ) );

  destination = ZEROVEC;
}

int flash_count =0;
#pragma todo("Perhaps implement a non-frame-rate-dependent flashing effect? (GT--3/29/01)")
void spiderman_compass_widget::render()
{
static color c(1.0f, 1.0f, 1.0f, 1.0f);
  flash_count++;
  if (flash_count == 3)
  {
    c = color(1.0f, 0.0f, 0.0f, 1.0f); // was 0, 0, 1
  }
  else if (flash_count >= 6)
  {
    c = color(1.0f, 1.0f, 1.0f, 1.0f);
    flash_count = 0;
  }
  for(int i=0; i < COMPASS_NUM_BARS; ++i)
  {

    if (i == height_chicklet_destination_height)
      height_chicklet->set_color(c);
    else
      height_chicklet->set_color(color(1.0f, 1.0f, 1.0f, 1.0f));
    height_chicklet->move_to(BAR_X_OFFSET, BAR_Y_OFFSET + ((-COMPASS_NUM_BARS * BAR_HEIGHT) + i*(BAR_HEIGHT * 2)));
    height_chicklet->render();
  }
  height_spider->render();
  compass_frame->render();
  compass_arrow->render();
}

bool do_foo = false;

void spiderman_compass_widget::frame_advance( time_value_t time_inc )
{
  if(!is_shown()) return;

  if (do_foo)
  {
  	compass_frame->move_to(COMPASS_FRAME_X_OFFSET, COMPASS_FRAME_Y_OFFSET);
  	compass_arrow->move_to(COMPASS_FRAME_X_OFFSET, COMPASS_FRAME_Y_OFFSET);
    compass_frame->scale_to(COMPASS_FRAME_SCALE);//(0.9f);
    compass_arrow->scale_to(COMPASS_ARROW_SCALE);//(0.9f);
    height_chicklet->resize(HEIGHT_CHICKLET_X_SCALE, HEIGHT_CHICKLET_Y_SCALE);
    height_spider->resize(HEIGHT_SPIDER_X_SCALE, HEIGHT_SPIDER_Y_SCALE);
    do_foo = false;
  }

  vector3d dir = destination - g_world_ptr->get_hero_ptr()->get_abs_position();
//  rational_t y_dist = dir.y;
  dir.y = 0.0f;
  rational_t dir_distance = dir.length();

  if(dir_distance > 0.0f)
    dir /= dir_distance;
  else
    dir = g_world_ptr->get_hero_ptr()->get_abs_po().get_facing();

  vector3d cam_facing = app::inst()->get_game()->get_current_view_camera()->get_abs_po().get_facing();
  cam_facing.y = 0.0f;
  cam_facing.normalize();
  if(cam_facing.length2() < 0.1f)
    cam_facing = ZVEC;

  rational_t ang = 0.0f;
  rational_t dotp = dot(dir, cam_facing);
  if(dotp < 1.0f && dotp > -1.0f)
  {
    ang = fast_acos(dotp);
    vector3d axis = cross(dir, cam_facing);
    if(axis.y < 0.0f)
      ang = -ang;
  }

//  compass_arrow->rotate_to(0.0f, 0.0f, ang, ZVEC);
  vector3d temp_vec = ZVEC;
  compass_arrow->set_rotation( temp_vec, ang, 0.0f);
//  compass_arrow->set_rotation(DEG_TO_RAD(60.0f), ang, 0.0f);

/*
  rational_t y_delta = (y_dist / 5.0f);
  if(y_delta > 25.0f)
    y_delta = 25.0f;
  if(y_delta < -25.0f)
    y_delta = -25.0f;

//  compass_ud_arrow->set_rotation(DEG_TO_RAD(90.0f * (y_delta / 25.0f)), 0.0f, 0.0f);

  if(__fabs(y_delta) > 5.0f)
  {
    rational_t alpha = ((__fabs(y_delta)-5.0f) / 20.0f)*3.0f;
    if(alpha > 1.0f)
      alpha = 1.0f;
    compass_ud_arrow->set_color(color(1.0f, 1.0f, 1.0f, alpha));
  }
  else
    compass_ud_arrow->set_color(color(1.0f, 1.0f, 1.0f, 0.0f));
*/

  int bar_min = BAR_Y_OFFSET + ((-COMPASS_NUM_BARS * BAR_HEIGHT) + (COMPASS_NUM_BARS - 1)*(BAR_HEIGHT*2));
  int bar_max = BAR_Y_OFFSET + (-COMPASS_NUM_BARS * BAR_HEIGHT);
  int deltaY = (int)(bar_max + ((bar_min - bar_max) * ((g_spiderman_controller_ptr->get_max_height() -
	  g_world_ptr->get_hero_ptr()->get_abs_position().y) / (g_spiderman_controller_ptr->get_max_height() -
	  g_spiderman_controller_ptr->get_min_height()))))+HEIGHT_SPIDER_Y_OFFSET;
  /*if(deltaY > bar_min)
    deltaY = bar_min;
  if(deltaY < bar_max)
    deltaY = bar_max;*/
//  compass_spider->move_to(75, deltaY);
  height_spider->move_to(BAR_X_OFFSET+HEIGHT_SPIDER_X_OFFSET, deltaY);


//  compass_arrow->move_to(0, y_delta);
//  compass_arrow->set_rotation(DEG_TO_RAD(60.0f-y_dist), 0.0f, 0.0f);

//  distance->set_string( stringx(stringx::fmt, "%.2fm", dir_distance) );

  //elevation->set_string( stringx(stringx::fmt, "%.2fm", (-y_dist)) );
  //elevation->move_to(35, y_delta);
  //rational_t red = (__fabs((rational_t)y_delta) / 25.0f);
  //elevation->set_color( color( red, 1.0f - red, 0.0f, 1.0f ) );

  widget::frame_advance(time_inc);
}


void spiderman_compass_widget::set_dest(vector3d dest)
{
  destination = dest;

//  for(int i=0; i < COMPASS_NUM_BARS; ++i)
//    compass_bar[i]->set_color(color(0.0f, 0.0f, 1.0f, 1.0f));

  rational_t min = g_spiderman_controller_ptr->get_min_height();
  rational_t max = g_spiderman_controller_ptr->get_max_height();

  int num = (int)(COMPASS_NUM_BARS * ((max - destination.y) / (max - min)));
  if(num < 0)
    num = 0;
  if(num >= COMPASS_NUM_BARS)
    num = COMPASS_NUM_BARS - 1;

  height_chicklet_destination_height = num;

//  compass_bar[num]->set_color(color(0.0f, 1.0f, 1.0f, 1.0f));
}


#endif // BIGCULL

// these need to be globals only while debugging (to make it easy to modify them in the debugger)
rational_t STATUS_WEB_BACKGROUND_SCALE = 100.0f;
rational_t STATUS_HEALTH_AND_WEB_X_SCALE = 150.0f;
rational_t STATUS_HEALTH_AND_WEB_Y_SCALE = 65.0f;

short STATUS_WEB_BACKGROUND_X_OFFSET = 0;
short STATUS_WEB_BACKGROUND_Y_OFFSET = 0;

short STATUS_HEALTH_AND_WEB_X_OFFSET = 70;
short STATUS_HEALTH_AND_WEB_Y_OFFSET = 4;

short WEB_X_OFFSET = 83;
short WEB_Y_OFFSET = 45;

short HEALTH_X_OFFSET = 83;
short HEALTH_Y_OFFSET =  23;

short HEALTH_BAR_LENGTH =   125;
short WEB_BAR_LENGTH =      107;
short STATUS_BAR_THICKNESS =  6;

#if 0 // BIGCULL
status_widget::status_widget( const char *_widget_name, widget *_parent, short _x, short _y ) :
  widget( _widget_name, _parent, _x, _y )
{
/*
  set_rhw_2d_layer( RHW2 );
  // construct health bar
  health_bar = NEW fluid_bar( "status-health", this, 102, 59, WDIR_Right, 125, 6, os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR)+"\\alpha" );
  health_bar->set_fill_rate( 1.0f );
  health_bar->set_empty_rate( 2.0f );
  health_bar->use_special_rate( 5.0f );
  health_bar->set_color( color( 0.9f, 0.0f, 0.0f, 1.0f ) );
  // construct webbing bar
  //webbing_bar = NEW fluid_bar( "status-web", this, WEB_X_OFFSET+2, WEB_Y_OFFSET+10, WDIR_Down, 5, 75, os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR)+"\\alpha" );
  //webbing_bar->set_fill_rate( 10.0f );
  //webbing_bar->set_empty_rate( 10.0f );
  //webbing_bar->use_special_rate( 5.0f );
  //webbing_bar->set_color( color( 0.0f, 0.0f, 0.9f, 1.0f ) );
  restore_last_rhw_2d_layer();

  set_rhw_2d_layer( RHW1 );
  web_cartridges_bg = NEW text_widget( "status-web_carts", this, 59, 129, 1.5f, 0, "interface\\frontendfont" );
  web_cartridges_bg->set_color( color( 0.0f, 0.0f, 0.0f, 1.0f ) );
  web_cartridges = NEW text_widget( "status-web_carts", this, 60, 130, 1.5f, 0, "interface\\frontendfont" );
  web_cartridges->set_color( color( 0.9f, 0.0f, 0.0f, 1.0f ) );
  restore_last_rhw_2d_layer();

  bitmap_widget *bg;
  set_rhw_2d_layer( RHW0 );
  bg = NEW bitmap_widget( "status-health_bg", this, 0, 0, 1 );
  bg->open( (stringx("interface\\")+os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR)+"\\infobar").c_str() );
//  bg->resize( 150+4, 10+4 );
//  bg->set_color( color( 0.6f, 0.6f, 0.6f, 1.0f ) );
*/

/*
  bg = NEW bitmap_widget( "status-health_bg", this, 20, 0, 1 );
  bg->open( "textures\\alpha" );
  bg->resize( 150+4, 10+4 );
  bg->set_color( color( 0.6f, 0.6f, 0.6f, 1.0f ) );
*/

  //bg = NEW bitmap_widget( "status-web_bg", this, WEB_X_OFFSET+0, WEB_Y_OFFSET+8, 1 );
  //bg->open( (os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR)+"\\alpha").c_str() );
  //bg->resize( 5+4, 75+4 );
  //bg->set_color( color( 0.6f, 0.6f, 0.6f, 1.0f ) );
//  restore_last_rhw_2d_layer();

  //set_rhw_2d_layer( RHW1 );
/*
  bg = NEW bitmap_widget( "status-health_bg_empty", this, 22, 2, 1 );
  bg->open( "textures\\alpha" );
  bg->resize( 150, 10 );
  bg->set_color( color( 0.0f, 0.0f, 0.0f, 1.0f ) );
*/

  //bg = NEW bitmap_widget( "status-web_bg_empty", this, WEB_X_OFFSET+2, WEB_Y_OFFSET+10, 1 );
  //bg->open( (os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR)+"\\alpha").c_str() );
  //bg->resize( 5, 75 );
  //bg->set_color( color( 0.0f, 0.0f, 0.0f, 1.0f ) );
  //restore_last_rhw_2d_layer();

  set_rhw_2d_layer( RHW1 );
  char status_web_background_str[256];
  strcpy(status_web_background_str, "interface\\");
  strcat(status_web_background_str, os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR).c_str());
  strcat(status_web_background_str, "\\infobarhead");
  status_web_background = NEW bitmap_widget( "info-web", this, 0, 0, 1 );
  status_web_background->open( status_web_background_str );
  status_web_background->move_to(STATUS_WEB_BACKGROUND_X_OFFSET, STATUS_WEB_BACKGROUND_Y_OFFSET);
  status_web_background->resize(STATUS_WEB_BACKGROUND_SCALE, STATUS_WEB_BACKGROUND_SCALE);
  restore_last_rhw_2d_layer();

  set_rhw_2d_layer( RHW2 );
  char status_health_and_web_str[256];
  strcpy(status_health_and_web_str, "interface\\");
  strcat(status_health_and_web_str, os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR).c_str());
  strcat(status_health_and_web_str, "\\infobarbars");
  status_health_and_web = NEW bitmap_widget( "info-bars", this, 0, 0, 1 );
  status_health_and_web->open( status_health_and_web_str );
  status_health_and_web->move_to(STATUS_HEALTH_AND_WEB_X_OFFSET, STATUS_HEALTH_AND_WEB_Y_OFFSET);
  status_health_and_web->resize(STATUS_HEALTH_AND_WEB_X_SCALE, STATUS_HEALTH_AND_WEB_Y_SCALE);
  restore_last_rhw_2d_layer();

  set_rhw_2d_layer( RHW3 );
  // construct health bar
  health_bar = NEW fluid_bar( "status-health", this, HEALTH_X_OFFSET, HEALTH_Y_OFFSET, WDIR_Right,
    HEALTH_BAR_LENGTH, STATUS_BAR_THICKNESS, os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR)+"\\alpha" );
  health_bar->set_fill_rate( 1.0f );
  health_bar->set_empty_rate( 2.0f );
  health_bar->use_special_rate( 5.0f );
#pragma todo("Fix this hack to swap the health and web bars...it's just not right (GT--4/3/01)")
  health_bar->set_color( color( 0.0f, 0.0f, 0.9f, 1.0f ) ); // should be .9, 0, 0
  // construct webbing bar
  webbing_bar = NEW fluid_bar( "status-web", this, WEB_X_OFFSET, WEB_Y_OFFSET, WDIR_Right,
    WEB_BAR_LENGTH, STATUS_BAR_THICKNESS, os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR)+"\\alpha" );
  webbing_bar->set_fill_rate( 10.0f );
  webbing_bar->set_empty_rate( 10.0f );
  webbing_bar->use_special_rate( 5.0f );
  webbing_bar->set_color( color( 0.9f, 0.0f, 0.0f, 1.0f ) );// should be 0, 0, .9
  restore_last_rhw_2d_layer();


/*
  set_rhw_3d_layer( RHW3 );
  spidey_head = NEW entity_widget("status-spidey_head", this, 50, 75, "interface\\entities\\spidey_head.ent");
  spidey_head->set_rotation(DEG_TO_RAD(30.0f), DEG_TO_RAD(180.0f), 0.0f);
  silver_spidey = NEW entity_widget("status-silver_spidey", this, 40, 115, "interface\\entities\\silverspider.ent");
  silver_spidey->set_rotation(DEG_TO_RAD(90.0f), 0.0f, DEG_TO_RAD(225.0f));
  restore_last_rhw_3d_layer();
*/
  // set initial state
  frame_advance( 0.0f );
}

bool update_dat_sheeit = false;

//#include "simple_classes.h"
//simple_oscillator head_osc(DEG_TO_RAD(180.0f), DEG_TO_RAD(5.0f), 0.5f);
//rational_t ang = 0.0f;
void status_widget::frame_advance( time_value_t time_inc )
{
  if ( !is_shown() ) return;

  if (update_dat_sheeit)
  {
    status_web_background->move_to(STATUS_WEB_BACKGROUND_X_OFFSET, STATUS_WEB_BACKGROUND_Y_OFFSET);
    status_health_and_web->move_to(STATUS_HEALTH_AND_WEB_X_OFFSET, STATUS_HEALTH_AND_WEB_Y_OFFSET);
    status_web_background->resize(STATUS_WEB_BACKGROUND_SCALE, STATUS_WEB_BACKGROUND_SCALE);
    status_health_and_web->resize(STATUS_HEALTH_AND_WEB_X_SCALE, STATUS_HEALTH_AND_WEB_Y_SCALE);

    health_bar->move_to(HEALTH_X_OFFSET, HEALTH_Y_OFFSET);
    health_bar->resize(HEALTH_BAR_LENGTH, STATUS_BAR_THICKNESS);
    webbing_bar->move_to(WEB_X_OFFSET, WEB_Y_OFFSET);
    webbing_bar->resize(WEB_BAR_LENGTH, STATUS_BAR_THICKNESS);
    update_dat_sheeit = false;
  }
//  head_osc.frame_advance(time_inc);
//  spidey_head->set_rotation(DEG_TO_RAD(30.0f), head_osc.get_val(), 0.0f);

  entity* hero = g_world_ptr->get_hero_ptr();
/*
  ang += time_inc*15.0f;
  while(ang >= 360.0f)
    ang -= 360.0f;
  silver_spidey->set_rotation(DEG_TO_RAD(90.0f), 0.0f, DEG_TO_RAD(225.0f+0.0f));
*/
  if ( hero )
  {
    rational_t full_val = hero->damage_ifc()->get_max_hit_points(),
               to_val = hero->damage_ifc()->get_hit_points();
    health_bar->set_full_val( full_val );
    health_bar->set_to_val( to_val );

    full_val = g_spiderman_controller_ptr->get_max_webbing(),
    to_val = g_spiderman_controller_ptr->get_webbing();
    webbing_bar->set_full_val( full_val );
    webbing_bar->set_to_val( to_val );

//    web_cartridges->set_string( stringx("x") + stringx(g_spiderman_controller_ptr->get_webbing_carts()) );
//    web_cartridges_bg->set_string( stringx("x") + stringx(g_spiderman_controller_ptr->get_webbing_carts()) );

/*
    health_bar->resize(125, 6);
    health_bar->move_to(113, 61);

    webbing_bar->resize(5, 75);
    webbing_bar->move_to(32, 80);
*/

/*
    entity *head = ((conglomerate *)hero)->get_member("BIP01 HEAD");
    head->update_abs_po_reverse();
    vector3d body_face = hero->get_abs_po().get_z_facing();
    body_face.y = 0.0f;
    body_face.normalize();
    vector3d head_face = -head->get_abs_po().get_z_facing();
    head_face.y = 0.0f;
    head_face.normalize();
    vector3d up_face = hero->get_abs_po().get_y_facing();

    ang = acos(dot(head_face, body_face));

    if(dot(up_face, cross(body_face, head_face)) > 0.0f)
      ang = -ang;

    ang += DEG_TO_RAD(180.0f);

    spidey_head->set_rotation(0.0f, ang, 0.0f);
*/
  }

  widget::frame_advance( time_inc );
}


int compass_widget_X_OFFSET = 530;
int compass_widget_Y_OFFSET = 410;

#endif // BIGCULL

//#include "widget_entity.h"
//entity_widget *ent_widg;
interface_widget::interface_widget( const char *_widget_name, widget *_parent, short _x, short _y ) :
  widget( _widget_name, _parent, _x, _y )
{
  cur_reticle = NULL;

  // set rhw layers
  set_rhw_2d_layer( RHW1 );
  set_rhw_3d_layer( RHW3 ); // in interface, vreps should draw over bitmaps

//  my_impact_widget = NEW impact_widget( "interface-impact", this, 0, 0 );
//  my_impact_widget->hide();

//  empty = NEW buffered_bitmap_widget( "ammo-empty", this, ammo_fluid_x-7, ammo_fluid_y-12, 1 );
  //empty->open( "interface\\healthchicklet" );

  // BIGCULL my_status_widget = NEW status_widget( "interface-status", this, 0, 0 ); // 145, 402
//  my_status_widget->hide();

//  my_ammo_widget = NEW ammo_widget( "interface-ammo", this, 540, 331 );
//  my_ammo_widget->hide();

//!  my_boss_widget = NEW boss_widget( "interface-boss", this, 32, 40 );

//  my_score_widget = NEW score_widget( "interface-score", this, 604, 32 );
//  my_score_widget->hide();

//  my_item_widget = NEW item_widget( "interface-item", this, 0, 0 );
//  my_item_widget->hide();

//  my_bulletin_widget = NEW bulletin_widget( "interface-bulletin", this, 320, 340 );
//  my_bulletin_widget->hide();

//  my_nano_widget = NEW nano_widget( "interface-nano", this, nano_widget_start_x, nano_widget_y );
//  my_nano_widget->hide();

//  my_layout_widget = NEW layout_widget( "interface-layout", this, 0, 0 );
//  my_nano_widget->hide();


  // BIGCULL my_compass_widget = NEW spiderman_compass_widget("interface-compass", this, compass_widget_X_OFFSET , compass_widget_Y_OFFSET );
  // BIGCULL my_compass_widget->hide();
// BIGCULL  compass_on = false;

  /*
    widget::set_rhw_3d_layer( widget::RHW3 );
  ent_widg = NEW entity_widget( "testent", this, 320, 240, "circleconsole" );
  ent_widg->scale_to( 0.5f );
    widget::restore_last_rhw_3d_layer();
    */
}


void interface_widget::show()
{
  widget::show();

#if 0 // BIGCULL
  if(compass_on)
    my_compass_widget->show();
  else
    my_compass_widget->hide();
#endif // BIGCULL
}
bool do_that_thing = false;

void interface_widget::frame_advance( time_value_t time_inc )
{
  if ( !is_shown() ) return;

/*
  if ( os_developer_options::inst()->is_flagged(os_developer_options::FLAG_WIDGET_TOOLS) &&
       input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, ACTION_ZOOMIN ) == AXIS_MAX &&
       input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, INVENTORY ) == AXIS_MAX )
  {
    my_layout_widget->show();
  }
*/
#if 0 // BIGCULL
  if (do_that_thing)
  {
    my_compass_widget->move_to(compass_widget_X_OFFSET, compass_widget_Y_OFFSET);
    do_that_thing = false;
  }
  my_compass_widget->frame_advance(time_inc);
#endif // BIGCULL
  if ( cur_reticle )
    cur_reticle->frame_advance( time_inc );

  widget::frame_advance( time_inc );
}


void interface_widget::render()
{
  if ( cur_reticle )
  {
    cur_reticle->render();
  }

  widget::render();
}


/*
void interface_widget::hit_stealth()
{
  my_nano_widget->hit_stealth();
}


void interface_widget::hit_turbo()
{
  my_nano_widget->hit_turbo();
}

void interface_widget::start_nano()
{
  if ( !my_nano_widget->is_active() )
  {
    my_nano_widget->show();
    my_nano_widget->move_to( 0.2f, 0.4f, nano_widget_end_x, nano_widget_y );
  }
}


void interface_widget::cancel_nano()
{
  if ( my_nano_widget->is_active() )
  {
    my_nano_widget->move_to( 0.0f, 0.2f, nano_widget_start_x, nano_widget_y );
    my_nano_widget->set_next_state( WSTATE_Hide, 0.2f );
  }
}

void interface_widget::indicate_damage(rational_t dam, const vector3d &from)
{
  my_impact_widget->indicate_damage(dam, from);
}
*/

/*!void interface_widget::activate_boss_widget( character *chr )
{
  my_boss_widget->set_boss( chr );
  my_boss_widget->frame_advance( 0.0f );
}

void interface_widget::hide_boss_widget()
{
  my_boss_widget->hide();
}

void interface_widget::set_boss_widget_health_bar_color(color col)
{
  my_boss_widget->set_health_bar_color(col);
}

void interface_widget::set_boss_widget_health_bar_val(rational_t val)
{
  my_boss_widget->set_health_bar_val(val);
}
!*/
/*
void interface_widget::set_score( int s )
{
  assert( my_score_widget );
  my_score_widget->set_score( s );
}

int interface_widget::get_score() const
{
  assert( my_score_widget );
  return my_score_widget->get_score();
}

void interface_widget::inc_score( int inc )
{
  assert( my_score_widget );
  my_score_widget->inc_score( inc );
}

void interface_widget::clear_score()
{
  assert( my_score_widget );
  my_score_widget->clear_score();
}
*/
