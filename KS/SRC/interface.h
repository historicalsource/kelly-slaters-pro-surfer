#ifndef INTERFACE_H
#define INTERFACE_H

#include "widget.h"

/*P

enum chicklet_e
{
  CHICKLET_Default,
  CHICKLET_Blue,
  CHICKLET_Green,
  CHICKLET_Total
};

class chicklet_bar : public bar_widget
{
public:
  struct specs_t
  {
    short num_chicks;
    short spacing;                // pixels between each chicklet
    float w, h;                   // of each chicklet
    chicklet_e active;            // chicklet bitmap to use when chicklet is active
    float u_off, v_off;           // Offsets into the texturemap for (usually 0, 0)
  };

  chicklet_bar( const char *_widget_name, widget *_parent, short _x, short _y, widget_dir_e _dir, const specs_t& _specs );
  virtual ~chicklet_bar() {}

  void update( rational_t _val, rational_t _full_val );

protected:
  void set_chicklets( int first, int last, const chicklet_e& index );
  //bitmap_widget **chicklets;
//  buffered_bitmap_widget *chicklet_gfx;

  specs_t specs;
};
P*/

//-------------------------------------------------------------------------------------

/*/ background for health bars
class health_bg_widget : public widget
{
public:
  health_bg_widget( const char *_widget_name, widget *_parent, short _x, short _y, float _w, float _h, bool vert,
        stringx end_name = "interface\\healthbarbaseend",
        stringx stretch_name = "interface\\healthbarbasestretch" );
  virtual ~health_bg_widget() {}

  void hide_bar_left();
  void hide_bar_right();
  void hide_bar_stretch();

protected:
  bitmap_widget *bar_left;
  bitmap_widget *bar_right;
  bitmap_widget *bar_stretch;

  float w, h;
};
*/
/*
//-------------------------------------------------------------------------------------
// the combined health/armor/nano display thing
class status_widget : public widget
{
public:
  status_widget( const char *_widget_name, widget *_parent, short _x, short _y );
  virtual ~status_widget() {}

  virtual void frame_advance( time_value_t time_inc );

protected:
  fluid_bar *nano_bar;
  chicklet_bar *health_bar, *armor_bar;
};
*/

//-------------------------------------------------------------------------------------
/*
class ammo_widget : public widget
{
public:
  ammo_widget( const char *_widget_name, widget *_parent, short _x, short _y );
  virtual ~ammo_widget() {}
  
  virtual void frame_advance( time_value_t time_inc );

protected:
  void set_bar_col( rational_t fullness );
  //health_bg_widget *bg;
  fluid_bar *bar;
  buffered_bitmap_widget *empty, *highlight;
  vrep_widget *ammo_cell;
  text_widget *amount;
};
*/

//-------------------------------------------------------------------------------------
/*!
// combined health/armor bar for bosses; up when invoked by scripter
class boss_widget : public widget
{
public:
  boss_widget( const char *_widget_name, widget *_parent, short _x, short _y );
  virtual ~boss_widget() {}

  virtual void show();
  void set_boss( character *_boss ) { boss = _boss; show(); }
  virtual void frame_advance( time_value_t time_inc );

  void set_health_bar_color(color col);
  void set_health_bar_val(rational_t val);

protected:
  fluid_bar *health_bar;
  //health_bg_widget *hb_bg;
  bitmap_widget *hb_full, *hb_highlight;

  character *boss;
};
!*/

//-------------------------------------------------------------------------------------
/*
class score_widget : public widget
{
public:
  score_widget( const char *_widget_name, widget *_parent, short _x, short _y );
  virtual ~score_widget() {}

  int get_score() const { return score; }
  void set_score( int s );
  void inc_score( int inc );
  void clear_score() { set_score( 0 ); }
  void update();

protected:
  text_widget *score_text, *score_bg;
  int score, last_score;
};


//-------------------------------------------------------------------------------------

class item_widget : public widget
{
public:
  item_widget( const char *_widget_name, widget *_parent, short _x, short _y );
  virtual ~item_widget() {}

  virtual void show();

  virtual void add_perm_item();
  virtual void remove_perm_item();

  virtual void add_item(item *cur_item = NULL);
  virtual void next_item(item *cur_item = NULL);
  virtual void prev_item(item *cur_item = NULL);
  virtual void use_item(item *cur_item = NULL);

  virtual void sort(item *cur_item = NULL, bool spin = false);

  void set_owner( entity *_owner ) { owner = _owner; }
  void activation_effect();
  void deactivation_effect();
  virtual void set_current_item_color(rational_t r = 1.0f, rational_t g = 1.0f, rational_t b = 1.0f, rational_t a = 1.0f);
  void turn_selector_on();
  void turn_selector_off();
  bool is_selector_on() { return ( selector_on ); }
  void flash_up_but();
  void flash_down_but();
  void spin_item();

private:
  entity *owner;
  bitmap_widget *bg;
  vrep_widget *current_item;
  vrep_widget *up_but, *down_but, *up_but_hi, *down_but_hi;
  text_widget *amount;
  text_block_widget *name;
  bool item_active, selector_on;
  rational_t orientation;  // orientation on y in degrees (from .enx file)
};

//-------------------------------------------------------------------------------------


class bulletin_widget : public widget
{
public:
  bulletin_widget( const char *_widget_name, widget *_parent, short _x, short _y );
  virtual ~bulletin_widget() {}

  virtual void frame_advance( time_value_t time_inc );
  void initiate( stringx name, visual_rep *vr, time_value_t in_time, time_value_t up_time, time_value_t out_time );
  void set_text_col( color _text_col ) { text_col = _text_col; }
  void set_text_scale( float _text_scale ) { text_scale = _text_scale; }
  void set_drop_shad_col( color _drop_shad_col ) { drop_shad_col = _drop_shad_col; }
  void set_drop_shad_offsets( short _x, short _y ) { drop_shad_x = _x; drop_shad_y = _y; }

protected:
  vrep_widget *item;
  text_widget *text, *drop_shad;
  color text_col, drop_shad_col;
  float text_scale;
  short drop_shad_x, drop_shad_y;
};


//-------------------------------------------------------------------------------------

#define NUM_NANO_SPARKS 10

class nano_widget : public widget
{
public:
  nano_widget( const char *_widget_name, widget *_parent, short _x, short _y );
  virtual ~nano_widget() {}

  virtual void show();
  virtual void hide();

  virtual void frame_advance( time_value_t time_inc );

  void hit_stealth();
  void hit_turbo();

  bool is_active() const { return ( is_shown() && next_state == WSTATE_None ); }

protected:
  void do_flare();
  void animate_stealth();
  void animate_turbo();

  bitmap_widget *bg;
  bitmap_widget *block_out[2], *a_but[2], *b_but[2], *x_but[2], *y_but[2];
  text_block_widget *trig[2];
  text_widget *stealth_text, *turbo_text;
  box_widget *box[2];

  // cool stuff
  bitmap_widget *flare;
  bitmap_widget *stealth1, *stealth2;
  bitmap_widget *turbo1, *turbo2;
  bitmap_widget *sparks[NUM_NANO_SPARKS];

  bool stealth_launched, turbo_launched;
};

//-------------------------------------------------------------------------------------

class impact_widget : public widget
{
public:
  impact_widget( const char *_widget_name, widget *_parent, short _x, short _y );
  virtual ~impact_widget() {}

  virtual void show();
  virtual void frame_advance( time_value_t time_inc );

  void indicate_damage(rational_t dam, const vector3d &from);

protected:
  void set_indicator();

  enum { TOP = 0, RIGHT, BOTTOM, LEFT, NUM_DIRS };
  bitmap_widget *flash[NUM_DIRS];
  rational_t amount[NUM_DIRS];
};


//-------------------------------------------------------------------------------------

class layout_widget;
*/
//#define COMPASS_NUM_BARS  20
#define COMPASS_NUM_BARS  15

#if 0 // BIGCULL
class spiderman_compass_widget : public widget
{
protected:
  vrep_widget *compass_arrow;
//  vrep_widget *compass_ud_arrow;
//  vrep_widget *compass_spider;
//  vrep_widget *compass_bar[COMPASS_NUM_BARS];
//  bitmap_widget *compass_frame;
  bitmap_widget *height_spider;
  bitmap_widget *height_chicklet;
  vrep_widget *compass_frame;
  text_widget *elevation;
  text_widget *distance;
  vector3d destination;
  int height_chicklet_destination_height;

public:
  spiderman_compass_widget(const char *_widget_name, widget *_parent, short _x, short _y );
  virtual ~spiderman_compass_widget() {}

  virtual void render();
  virtual void frame_advance( time_value_t time_inc );

  void set_dest(vector3d dest);
};

#endif

//-------------------------------------------------------------------------------------
// the combined health/webbing/face display thing
class status_widget : public widget
{
public:
  status_widget( const char *_widget_name, widget *_parent, short _x, short _y );
  virtual ~status_widget() {}

  virtual void frame_advance( time_value_t time_inc );

protected:
  fluid_bar *health_bar;
  fluid_bar *webbing_bar;
  bitmap_widget *status_web_background;
  bitmap_widget *status_health_and_web;
  //text_widget *web_cartridges;
  //text_widget *web_cartridges_bg;
  //entity_widget *spidey_head;
  //entity_widget *silver_spidey;
};


class interface_widget : public widget
{
public:
  interface_widget( const char *_widget_name, widget *_parent, short _x, short _y );
  virtual ~interface_widget() {}

  virtual void show();
  virtual void frame_advance( time_value_t time_inc );
  virtual void render();

//!  void activate_boss_widget( character *chr );
//!  void hide_boss_widget();
//!  void set_boss_widget_health_bar_color(color col);
//!  void set_boss_widget_health_bar_val(rational_t val);

//  void set_score( int s );
//  int get_score() const;
//  void inc_score( int inc );
//  void clear_score();
//  void hit_stealth();
//  void hit_turbo();
//  bool nano_is_active() const { return my_nano_widget->is_active(); }
//  void start_nano();
//  void cancel_nano();
//  void indicate_damage(rational_t dam, const vector3d &from);

//  item_widget *get_item_widget() { return ( my_item_widget ); }
//  bulletin_widget *get_bulletin_widget() { return ( my_bulletin_widget ); }
  void set_cur_reticle( bitmap_widget *reticle ) { cur_reticle = reticle; }
  bitmap_widget *get_cur_reticle() const { return(cur_reticle); }

  // BIGCULLspiderman_compass_widget *get_compass_widget()  { return(my_compass_widget); }
  // BIGCULLvoid show_compass()                             { my_compass_widget->show(); compass_on = true; }
  // BIGCULLvoid hide_compass()                             { my_compass_widget->hide(); compass_on = false; }
  // BIGCULLvoid set_compass_dest(vector3d dest)            { my_compass_widget->set_dest(dest); }

protected:
  bitmap_widget         *cur_reticle;
  status_widget         *my_status_widget;
//  ammo_widget           *my_ammo_widget;
//!  boss_widget           *my_boss_widget;
//  score_widget          *my_score_widget;
//  item_widget           *my_item_widget;
//  bulletin_widget       *my_bulletin_widget;
//  nano_widget           *my_nano_widget;
//  impact_widget         *my_impact_widget;
//  layout_widget         *my_layout_widget;
  // BIGCULLbool compass_on;
  // BIGCULL spiderman_compass_widget *my_compass_widget;
};


#endif // INTERFACE_H