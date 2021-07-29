#ifndef WIDGET_H
#define WIDGET_H

#include "hwrasterize.h"
#include "rect.h"
//P #include "pfe_element.h"
#include "osdevopts.h"
#include "entity.h"
#include "txtcoord.h"
#include "aggvertbuf.h"
#include "matfac.h"

#define WIDGET_RHW_INC    1.0f

enum wevent_type_e {
	WEVENT_None,							// wevent does nothing
	WEVENT_Color,						// wevent will do a linear interpolation to this color
	WEVENT_Move,							// wevent will do a linear interpolation move to this point
	WEVENT_Rotate,
	WEVENT_Scale
};


class widget;
class wevent;
class typeface_def;

typedef list<widget *> widget_list_t;
typedef list<wevent *> wevent_list_t;


class wevent
{
public:
	wevent( wevent_type_e t, widget *w, time_value_t wt, time_value_t d )
  {
    type = t;
    owner = w;
    wait_time = wt;
    duration = d;
    elapsed = 0.0f;
  }
  bool has_been_activated() { return ( elapsed >= wait_time ); }
  bool no_time_left() { return ( elapsed >= wait_time + duration ); }
  time_value_t active_time_elapsed();
  time_value_t active_time_left();
  time_value_t total_time_left();
  time_value_t time_till_active();
  rational_t get_lerp( time_value_t t );
  void set_time_left( time_value_t t );
  wevent_type_e get_type() { return type; }
  void frame_advance( time_value_t time_inc );
  virtual void do_wevent( rational_t ) = 0;
protected:
	wevent_type_e type;
  widget     *owner;
	time_value_t wait_time;           // time to wait before activation (when passed by elapsed, activate)
	time_value_t duration;
	time_value_t elapsed;             // starts counting when added
};

class move_wevent: public wevent
{
public:
  move_wevent( widget *w, time_value_t wt, time_value_t d, int _x, int _y ) :
      wevent( WEVENT_Move, w, wt, d ) { x = rational_t(_x); y = rational_t(_y); }
  virtual void do_wevent( rational_t );
protected:
	rational_t x;
	rational_t y;
};

class color_wevent: public wevent
{
public:
  color_wevent( widget *w, time_value_t wt, time_value_t d, color _mycolor ) :
      wevent( WEVENT_Color, w, wt, d ), mycolor( _mycolor ) {}
  virtual void do_wevent( rational_t );
protected:
	color mycolor;
};

class rotate_wevent: public wevent
{
public:
  rotate_wevent( widget *w, time_value_t wt, time_value_t d, rational_t _angle ) :
      wevent( WEVENT_Rotate, w, wt, d ), angle( _angle ) {}
  virtual void do_wevent( rational_t );
protected:
	rational_t angle;
};

class scale_wevent: public wevent
{
public:
  scale_wevent( widget *w, time_value_t wt, time_value_t d, rational_t _sx, rational_t _sy ) :
      wevent( WEVENT_Scale, w, wt, d ), sx( _sx ), sy( _sy ) {}
  virtual void do_wevent( rational_t );
protected:
	rational_t sx;
	rational_t sy;
};

//-------------------------------------------------------------------------------------

// message values
typedef unsigned int message_id_t;

#define WMESS_L_Press     0x00000001
#define WMESS_L_Held      0x00000002
#define WMESS_L_Release   0x00000004

#define WMESS_R_Press     0x00000008
#define WMESS_R_Held      0x00000010
#define WMESS_R_Release   0x00000020

#define WMESS_U_Press     0x00000040
#define WMESS_U_Held      0x00000080
#define WMESS_U_Release   0x00000100

#define WMESS_D_Press     0x00000200
#define WMESS_D_Held      0x00000400
#define WMESS_D_Release   0x00000800

#define WMESS_A_Press     0x00001000
#define WMESS_A_Held      0x00002000
#define WMESS_A_Release   0x00004000

#define WMESS_B_Press     0x00008000
#define WMESS_B_Held      0x00010000
#define WMESS_B_Release   0x00020000

#define WMESS_X_Press     0x00040000
#define WMESS_X_Held      0x00080000
#define WMESS_X_Release   0x00100000

#define WMESS_Y_Press     0x00200000
#define WMESS_Y_Held      0x00400000
#define WMESS_Y_Release   0x00800000

#define WMESS_LT_Press    0x01000000
#define WMESS_LT_Held     0x02000000
#define WMESS_LT_Release  0x04000000

#define WMESS_RT_Press    0x08000000
#define WMESS_RT_Held     0x10000000
#define WMESS_RT_Release  0x20000000

#define WMESS_START_Press 0x40000000

#define WMESS_OVERFLOW    0x80000000

#define WMESS_MS_Move     0x00000001
#define WMESS_MS_Press    0x00000002
#define WMESS_MS_Release  0x00000004

// flag values
#define WFLAG_Shown                   0x00000001
#define WFLAG_Open                    0x00000002
#define WFLAG_Ignore_Parent           0x00000004
#define WFLAG_Ignore_Parent_Showing   0x00000008
#define WFLAG_Override_Ignore_Showing 0x00000010  // if widget was ignoring parent showing,
                                                  // this flag indicates it was showing when
                                                  // parent was hidden, which enables it to show
                                                  // again when parent is shown (e.g. if boss bar
                                                  // should be up coming out of pause mode)


// Shown & hidden states for widgets are the equivalent of active & inactive.  Hidden widgets
// should be neither rendered nor advanced.  Widgets default to hidden when constructed so that
// initializations can be done in the show function.

class widget
{
public:

  enum widget_type_e
  {
    WTYPE_Bitmap,
    WTYPE_Text,
    WTYPE_TextBlock,
    WTYPE_Vrep,
    WTYPE_Entity,
    WTYPE_Menu,
    WTYPE_HelpLine,
    WTYPE_HelpGroup,
    WTYPE_PDAPage,
    WTYPE_Other,
  };

  enum widget_dir_e    // direction in which to "fill"
  {
    WDIR_Left,
    WDIR_Right,
    WDIR_Up,
    WDIR_Down,
  };

  enum widget_state_e
  {
    WSTATE_None,
    WSTATE_Show,
    WSTATE_Hide,
  };

  enum rhw_layer_e  // see top of widget.cpp for explanation
  {
    RHW0,
    RHW1,
    RHW2,
    RHW3,
    RHW4,
    RHW5,
    RHW6,
    RHW7,
    RHW8,
    RHW9,
    RHW_OVER_PFE1,
    RHW_OVER_PFE2,
    NUM_RHW_LAYERS
  };

  widget( const char *_widget_name, widget *_parent, short _x, short _y );
  virtual ~widget();

  void set_flag( unsigned int f, bool v )
  {
    if ( v )
      flags |= f;
    else
      flags &= ~f;
  }

  virtual void show();
  virtual void hide();
  virtual void ignore_parent() { set_flag( WFLAG_Ignore_Parent, true ); }
  virtual void obey_parent() { set_flag( WFLAG_Ignore_Parent, false ); }
  virtual void ignore_parent_showing() { set_flag( WFLAG_Ignore_Parent_Showing, true ); }
  virtual void obey_parent_showing() { set_flag( WFLAG_Ignore_Parent_Showing, false ); }

  virtual void frame_advance( time_value_t time_inc );
  virtual void render();
  virtual void message_handler( message_id_t message, message_id_t overflow = 0, rational_t parm0 = 0, rational_t parm1 = 0 );
  static void prepare_to_render();
  static void finish_render();

  virtual void add_child( widget *child );
  void remove_child( widget *child );
  widget *get_parent() const { return ( parent ); }

	void           add_wevent( wevent *e );
  virtual void   flush();
  bool           wevent_run_list_empty() const { return ( wevent_run_list.empty() ); }

  bool is_shown() const  { return ( flags & WFLAG_Shown ); }
  bool ignoring_parent() const { return ( flags & WFLAG_Ignore_Parent ); }
  bool ignoring_parent_showing() const { return ( flags & WFLAG_Ignore_Parent_Showing ); }
  bool override_ignore_showing() const { return ( flags & WFLAG_Override_Ignore_Showing ); } // see note on flag, above
  // a timed way to hide and show...
  void set_next_state( widget_state_e state, time_value_t wait_time ) { next_state = state; state_wait_time = wait_time; }
  // getting rid of this...
  void set_linear_animation( bool _linear_animation ) { linear_animation = _linear_animation; }

  // sometimes we want to use current projection matrix
  void set_use_proj_matrix( bool _use_proj_matrix ) { use_proj_matrix = _use_proj_matrix; }

  // animation and wevent functions
	virtual void   move_to( short _x, short _y );
  virtual void   move_to( time_value_t wt, time_value_t d, short _x, short _y );
	virtual void   scale_to( rational_t hs, rational_t vs );
  virtual void   scale_to( time_value_t wt, time_value_t d, rational_t hs, rational_t vs );
  virtual void   scale_to( rational_t s ) { scale_to( s, s ); }
  virtual void   scale_to( time_value_t wt, time_value_t d, rational_t s ) { scale_to( wt, d, s, s ); }
	virtual void   rotate_to( rational_t a );
  virtual void   rotate_to( time_value_t wt, time_value_t d, rational_t a );
	virtual void   set_color( color c );
  virtual void   set_color( color c[4] );
  virtual void   set_color( time_value_t wt, time_value_t d, color c );
  virtual void   set_color( rational_t r, rational_t g, rational_t b );
  virtual void   fade_to( rational_t alpha );
  virtual void   fade_to( time_value_t wt, time_value_t d, rational_t alpha );
  bool is_faded() const;

	virtual void   set_subrect( int x0, int y0, int x1, int y1 ) { subrect = rectf( x0, y0, x1, y1 ); }
	virtual void   set_origin( short ox, short oy ) { orig_x = ox; orig_y = oy; }

	virtual void   transform( rational_t v[2], color &c, int index );
  void           ndc( rational_t v[2] );

  const stringx& get_name() const { return ( widget_name ); }
  widget_type_e get_type() const { return ( type ); }
  short get_x() const { return( x ); }
  short get_y() const { return( y ); }
  short get_abs_x() const { return( abs_x ); }
  short get_abs_y() const { return( abs_y ); }
  rational_t get_abs_scale( int i ) const { return( abs_S[i] ); }
  rational_t get_abs_angle() const { return( abs_angle ); }
  color get_abs_col( int i ) const { return( abs_col[i] ); }
  virtual rational_t    get_width() { return ( 1 ); }
  virtual rational_t    get_height() { return ( 1 ); }

  widget_list_t &get_children() { return children; }
  widget *get_first_child();
  widget *get_prev_child( widget *start );
  widget *get_next_child( widget *start );

  widget *find_child_by_name( stringx name ); // uses match with widget_name

  virtual void set_layer( rhw_layer_e rhw_layer ) {};

  virtual void update_pos();
  virtual void update_scale();
  virtual void update_rot();
  virtual void update_col();

  static void reset_rhw_2d_val() { rhw_2d_val[rhw_2d_layer] = rhw_layer_ranges[rhw_2d_layer][0]; }
  static void set_rhw_2d_layer( rhw_layer_e rhw_layer )
  {
    last_rhw_2d_layer = rhw_2d_layer;
    rhw_2d_layer = rhw_layer;
    reset_rhw_2d_val();
  }
  static void set_rhw_3d_layer( rhw_layer_e rhw_layer )
  {
    last_rhw_3d_layer = rhw_3d_layer;
    rhw_3d_layer = rhw_layer;
  }
  static void restore_last_rhw_2d_layer() { rhw_2d_layer = last_rhw_2d_layer; }
  static void restore_last_rhw_3d_layer() { rhw_3d_layer = last_rhw_3d_layer; }
  static rational_t get_next_rhw_2d_val();

  friend class   wevent;
  friend class   move_wevent;
  friend class   color_wevent;
  friend class   rotate_wevent;
  friend class   scale_wevent;

  // used in vr_pmesh::render_instance for 3d widget rendering
  static rational_t cur_rhw_midpoint;
  static rational_t cur_rhw_half_range;
  static rational_t cur_largest_z; // used in pmesh.cpp for DC sorting
  static matrix4x4 cur_special_w_xform;  // for sorting calcs on DC
  static bool rendering_prepared;

  static rational_t calc_z( const vector3d &xyz );  // for DC sorting calcs in pmesh.cpp

  // for pc sorting:  convert DC rhw to PC z
  static rational_t get_pc_z( rational_t _rhw );

protected:
  stringx widget_name;
  widget_type_e type;
  widget *parent;
  widget_list_t children;
  unsigned int flags;
	wevent_list_t wevent_run_list;      // list of currently running wevents (ordered by time of activation)
  bool linear_animation;   // Is animation linear or time active / time left?
  bool use_proj_matrix;   // use current projection matrix when rendering?
  widget_state_e next_state;
  time_value_t state_wait_time;

  // base vals are those to be added to/multiplied by local vals to get final rendering vals ( abs_xxx )
  // position
  short x, y;             // local x, y (with reference to parent x, y)
  short abs_x, abs_y, base_x, base_y;
  short orig_x, orig_y;   // 2D origin offset

  // rotation
	rational_t angle;            // 2D angle of rotation
  rational_t abs_angle, base_angle;
	rational_t R[2][2];				  // 2D rotation into widget space

  // color
	color col[4];           // vertex color info; first element is used when color is uniform
  color abs_col[4], base_col[4];

  // scale
	rational_t S[2];             // 2D scale factor
  rational_t abs_S[2], base_S[2];
	rectf subrect;          // subrect of widget which is actually displayed

  // rhw stuff
  // For the DC, widgets are assigned an rhw layer to determine where they sort.
  // This assignment is made when a widget is constructed, and is based on the
  // values below (2d for bitmap & text widgets, 3d for vrep and entity widgets).
  // The 2d value for a layer is incremented with each NEW bitmap or text widget
  // until reset with reset_rhw_2d_val(), or until 2d layer is reset.
  // 3d widgets are distributed throughout their specified layer.
  static const rational_t rhw_layer_ranges[NUM_RHW_LAYERS][2]; // see widget.cpp for more comments
  static rhw_layer_e rhw_2d_layer, last_rhw_2d_layer;
  static rhw_layer_e rhw_3d_layer, last_rhw_3d_layer;
  static rational_t rhw_2d_val[NUM_RHW_LAYERS]; // all these are reset when unparented widget is constructed

  matrix4x4 calc_special_w_xform( const matrix4x4 &cur_mat, int _x, int _y ); // for special DC sorting calcs
};


//-------------------------------------------------------------------------------------


#define WSUBTYPE_None   0   // see subtype field below

// to select an item as focus, call its select function
// warning:  when menu is shown, focus will be reset to default_sel_index in menu
class menu_item_widget : public widget
{
public:
  menu_item_widget( const char *_widget_name, widget *_parent, short _x, short _y, const char *_item_desc = NULL );
  virtual ~menu_item_widget() {}

  virtual void select( bool initial = false );
  virtual void deselect( bool initial ) { selected = false; }

  virtual void set_skip( bool _skip ) { skip = _skip; }

  int get_index() const { return ( index ); }
  const stringx &get_item_desc() const { return ( item_desc ); }
  bool is_selected() const { return ( selected ); }
  bool skip_it() const { return ( skip ); }

  unsigned char get_subtype() const { return subtype; }
  virtual rational_t    get_width();
  virtual rational_t    get_height();

protected:
  int index;  // every item assigned a number for its place in the menu sequence
  stringx item_desc;  // different field from widget_name, so it can be used for display purposes
  bool selected, skip;
  unsigned char subtype;  // use of this for menu items avoids adding more entries in the generic enum for WTYPES
};

// generic container for menus (has focus, movement through items)
// to select an item as focus, call its select function
// warning:  when menu is shown, focus will be reset to default_sel_index
class menu_widget : public widget
{
public:
  menu_widget( const char *_widget_name, widget *_parent, short _x, short _y, message_id_t _to_prev = WMESS_U_Press, message_id_t _to_next = WMESS_D_Press );
  virtual ~menu_widget() {}

  virtual void init();  // required before menu will work; do after all children added (also done by show)

  virtual void add_child( widget *child ) { widget::add_child( child ); ++num_items; }
  virtual void show();
  virtual void message_handler( message_id_t message, message_id_t overflow = 0, rational_t parm0 = 0, rational_t parm1 = 0 );

  virtual menu_item_widget *get_prev_item() const; // will skip items marked to skip; does wraparound
  virtual menu_item_widget *get_next_item() const; // will skip items marked to skip; does wraparound
  virtual menu_item_widget *find_item_by_index( int index ) const;
  virtual menu_item_widget *find_item_by_desc( stringx &desc ) const;   // uses match with item_desc

  virtual menu_item_widget *get_sel_item() const { return sel_item; }
  virtual int get_num_items() const { return num_items; }

  void set_default_sel_index( int index ) { default_sel_index = index; }
  int get_default_sel_index() const { return default_sel_index; }

  message_id_t get_to_prev() const { return to_prev; }
  message_id_t get_to_next() const { return to_next; }

  bool get_change_made() const { return change_made; }
  void set_change_made( bool _change_made ) { change_made = _change_made; }

  virtual rational_t    get_width();
  virtual rational_t    get_height();

  friend class menu_item_widget;

protected:
  int num_items;  // used to set index for each menu item
  menu_item_widget *sel_item; // the current focus (shd. correspond to which item has selected flag set)
  int default_sel_index; // sel_item set to this whenever menu is shown (this defaults itself to first item in list)
  message_id_t to_prev, to_next;
  bool change_made; // indicates message handler acted; cleared when level is shown
};


//-------------------------------------------------------------------------------------


class bitmap_widget: public widget
{
public:
  bitmap_widget( const char *_widget_name, widget *_parent, short _x, short _y, int n = 1 );
  virtual ~bitmap_widget();
  bool          open(const char *name);
  virtual void  scale_to( rational_t hs, rational_t vs );
  virtual void  scale_to( time_value_t wt, time_value_t d, rational_t hs, rational_t vs ) { widget::scale_to( wt, d, hs, vs ); }
  virtual void  scale_to( rational_t s ) { scale_to( s, s ); }
  virtual void  scale_to( time_value_t wt, time_value_t d, rational_t s ) { widget::scale_to( wt, d, s, s ); }
  void          set_tc();
  void          resize( rational_t width, rational_t height );
	virtual void  set_subrect(int x0, int y0, int x1, int y1) { subrect = rectf(x0, y0, x1, y1); set_tc(); }
  void          play();
  void          pause();
  void          stop();
  void          set_num_frames( int _nframes ) { nframes = _nframes; }
  void          set_fps( int fps );
  void          set_frame( int _index ) { assert( index >= 0 && index < nframes ); index = _index; }
  void          flip_horiz();
  void          flip_vert();
  virtual rational_t    get_width() { return ( w ); }   // width before scaling
  virtual rational_t    get_height() { return ( h ); }  // height before scaling
  bool          is_open() { return ( flags & WFLAG_Open ); }

  virtual void set_layer( rhw_layer_e rhw_layer );

  virtual void  frame_advance( time_value_t time_inc );
  virtual void  render();
protected:
  rational_t         w;                    // width
  rational_t         h;                    // height
  rational_t         hw;					// half width
  rational_t         hh;					// half height
  int           nframes;				// number of frames of animation this bitmap possesses

  mat_fac       mat;

  int           index;				// the index into the current frame
  bool          playing;				// is the animation currently being played?
  time_value_t  frame_delay;          // how long do we play each frame for?
  time_value_t  frame_time;           // current time accumulated on this frame
  time_value_t  last_time;            // time stamp of last frame
  texture_coord tc[4];
  int           uv_order[4];    // for changing texture orientation
  rational_t    rhw, z;
};

//-------------------------------------------------------------------------------------

/*
class buffered_bitmap_widget: public bitmap_widget
{
public:
  buffered_bitmap_widget( const char *_widget_name, widget *_parent, short _x, short _y, int n = 1 );
  virtual void  render();
  void set_screen_scale(rational_t xdim, rational_t ydim);
protected:
  rational_t scr_scale_x;
  rational_t scr_scale_y;
};
*/

//-------------------------------------------------------------------------------------


#define DEFAULT_TEXT_SCALE    0.7f

enum text_widget_justification
{
  JUSTIFY_LEFT = 0,
  JUSTIFY_CENTER,
  JUSTIFY_RIGHT
};

class text_widget: public widget
{
public:
  text_widget( const char *_widget_name, widget *_parent, short _x, short _y, rational_t scale = DEFAULT_TEXT_SCALE,
               const stringx &str = 0, stringx typeface = "interface\\maxfont00" );
  text_widget( const char *_widget_name, widget *_parent, short _x, short _y, stringx typeface );
	virtual ~text_widget();
  void init( stringx &typeface );

  void set_string( stringx const &_prelocalized_text );  // always pass prelocalized text!
//P  void set_justif( text_widget_justification justification ) { text->set_justif( justification ); }

  virtual void show() { /*P  text->show_yourself();  P*/ widget::show(); }
  virtual void hide() { /*P  text->hide_yourself();  P*/ widget::hide(); }
  virtual void frame_advance( time_value_t time_inc );
	virtual void render();
  virtual void flush();

  const stringx &get_prelocalized_text() const { return prelocalized_text; }
  virtual rational_t get_width() const;
  virtual rational_t get_height() const;

  void set_rhw( rational_t _rhw );  // replaces rhw assigned in constructor and decrements rhw counter,
                                    // since it got incremented by get_next_rhw_2d_val

  virtual void set_layer( rhw_layer_e rhw_layer );

protected:
  friend class text_block_widget;
  typeface_def *text_font;
  stringx m_tout;

  stringx prelocalized_text;
  rational_t rhw;
  rational_t z;
};

//-------------------------------------------------------------------------------------

// block is parent to list of text_widgets (all children of block must be text_widgets)

class text_block_widget : public widget
{
public:
  class block_info_t
  {
  public:
    block_info_t();
    rational_t scale;
    text_widget_justification just;
    stringx typeface;
    uint8 max_lines;    // max_lines indicates how many rhw numbers will be assigned to this block, so be sparing
    short line_spacing; // in pixels
    short max_width;    // lines will break if longer than this; if -1, do not use
    stringx break_substring;    // should be "" if not used
    stringx text;
    color col;
  };


  text_block_widget( const char *_widget_name, widget *_parent, short _x, short _y, block_info_t *info_ptr );
  virtual ~text_block_widget();

  void set_block_info( block_info_t *info_ptr );
  const block_info_t &get_block_info() const { return block_info; }

  void set_text( stringx const &text );
  const stringx &get_text() const { return block_info.text; }

  virtual void set_color( color c );
  virtual void set_scale( rational_t s );
  virtual void set_line_spacing( rational_t s );


  virtual rational_t get_width() const;
  virtual rational_t get_height() const;

  uint8 get_num_lines_used() const { return num_lines_used; }

  virtual void set_layer( rhw_layer_e rhw_layer );

protected:
  void clear_lines();
  void fill_lines();
  block_info_t block_info;
  uint8 num_lines_used;
  rational_t first_rhw;   // first of a block reserved based on block_info.max_lines
  typeface_def *text_font;
};

//-------------------------------------------------------------------------------------


class vrep_widget: public widget
{
public:
#ifdef NGL
  vrep_widget( const char *_widget_name, widget *_parent, short _x, short _y, nglMesh* _mesh );
#else
  vrep_widget( const char *_widget_name, widget *_parent, short _x, short _y, visual_rep* vr );
#endif

  vrep_widget( char *filename, const char *_widget_name, widget *_parent, short _x, short _y );
  virtual ~vrep_widget();

  virtual void show();
  virtual void frame_advance( time_value_t time_inc );
  virtual void render();
  virtual void   rotate_to(time_value_t wt, time_value_t d, rational_t a, vector3d _axis) { axis = _axis; rotate_wevent *e; e = NEW rotate_wevent( this, wt, d, a ); add_wevent( e ); }

#ifdef NGL
  void         set_mesh(nglMesh* mesh);
#else
  void         set_vrep(visual_rep *vr);
  void         set_vrep( const stringx &filename );
#endif

  void         set_radius(rational_t r) { screen_radius = r; }
   // eventually change to allow continuous rotation to be done on multiple axes
  void         set_rotation(vector3d &u, rational_t a, rational_t s);
  void         set_rotation( rational_t _ax, rational_t _ay, rational_t _az );
  rational_t   get_ax() const { return ax; }
  rational_t   get_ay() const { return ay; }
  rational_t   get_az() const { return az; }
  virtual rational_t    get_width() { return screen_radius; }
  virtual rational_t    get_height() { return screen_radius; }

  virtual void update_pos();
  virtual void update_scale();
  virtual void update_rot();

  virtual void set_layer( rhw_layer_e rhw_layer );

protected:
  void init();
  void update_mat();                   // updates local-to_world matrix
  void calc_largest_z();

#ifdef NGL
  nglMesh* mesh;
#else
  visual_rep  *vrep;
#endif

  rational_t   screen_radius;
  vector3d     axis;                   // axis of rotation
  rational_t   ax, ay, az;             // specific axis angle in radians
  rational_t   rps;                    // radians per second

  rational_t   rhw_midpoint;           // starting rhw value
  rational_t   z;                      // for pc sorting
  rational_t   rhw_half_range;         // half of range of possible rhw values
  matrix4x4    rot_matrix;
  matrix4x4    mat;                    // local-to-world matrix
  matrix4x4    special_w_xform;        // for sorting calcs
  rational_t   largest_z;              // used in pmesh.cpp for DC sorting
};



//-------------------------------------------------------------------------------------

// covers whole screen with 6 bitmaps
// a number 0-5 is added to file_prefix to get bitmap_widget file names

class background_widget : public widget
{
public:
  background_widget( const char *_widget_name, widget *_parent, short _x, short _y, const stringx &file_prefix );
  virtual ~background_widget() {}
protected:
};


//-------------------------------------------------------------------------------------


class box_widget : public widget
{
public:
  box_widget( const char *_widget_name, widget *_parent, short _x, short _y, rational_t _w, rational_t _h,
    const stringx& corner_name = "interface\\roundcorner", const stringx& side_name = (os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR)+"\\alpha"),
    rational_t _w_thick = 2.0f, rational_t _h_thick = 2.0f );
  virtual ~box_widget() {}

  virtual void resize( rational_t _w, rational_t _h, rational_t _w_thick = 2.0f, rational_t _h_thick = 2.0f );
  virtual rational_t    get_width() { return w; }
  virtual rational_t    get_height() { return h; }

protected:
  bitmap_widget *corner[4], *side[4];
  rational_t w, h, w_thick, h_thick;
};

//-------------------------------------------------------------------------------------

class bar_widget : public widget
{
public:
  bar_widget( const char *_widget_name, widget *_parent, short _x, short _y, widget_dir_e _dir );
  virtual ~bar_widget() {}

  widget_dir_e get_dir() { return (dir); }
  bool is_horiz() { return ( dir == WDIR_Left || dir == WDIR_Right ); }

  void set_val( rational_t _val ) { val = _val; }
  void set_full_val( rational_t _full_val ) { full_val = _full_val; }

protected:
  widget_dir_e dir;
  signed char x_fac, y_fac;     // for setting direction in calcs
  rational_t val, full_val;
};


//-------------------------------------------------------------------------------------


class fluid_bar : public bar_widget
{
public:
  fluid_bar( const char *_widget_name, widget *_parent, short _x, short _y, widget_dir_e _dir, rational_t _w, rational_t _h,
    const stringx& _name );
  virtual ~fluid_bar() {}

  virtual void frame_advance( time_value_t time_inc );

  void set_full_val( rational_t _full_val ) { full_val = _full_val; if(to_val > full_val) to_val = full_val; if(cur_val > full_val) cur_val = full_val; if ( full_val != old_full_val ) update = true; }
  void set_to_val( rational_t _to_val ) { to_val = _to_val; if(to_val < 0.0f) to_val = 0.0f; if(to_val > full_val) to_val = full_val; if ( to_val != old_to_val ) update = true; }
  void set_cur_val( rational_t _cur_val ) { cur_val = _cur_val; if(cur_val < 0.0f) cur_val = 0.0f; if(cur_val > full_val) cur_val = full_val; if ( cur_val != old_cur_val ) update = true; }
  // rates are in bar-lengths/sec
  void set_fill_rate( rational_t _fill_rate ) { fill_rate = _fill_rate; if ( fill_rate != old_fill_rate ) update = true; }
  void set_empty_rate( rational_t _empty_rate ) { empty_rate = _empty_rate; if ( empty_rate != old_empty_rate ) update = true; }
  // forces use of special fill rate till cur_val = to_val
  void use_special_rate( rational_t _special_rate ) { special_rate = _special_rate; using_special_rate = true; update = true; }

  void resize( rational_t _w, rational_t _h );
  virtual rational_t    get_width() { return w; }
  virtual rational_t    get_height() { return h; }

  virtual void render();

  bitmap_widget *get_bar_map() const { return bar_map; }

protected:
  bitmap_widget *bar_map;
  stringx name;
  rational_t w, h;
  rational_t full_val, to_val, cur_val, fill_rate, empty_rate, special_rate;
  rational_t old_full_val, old_to_val, old_cur_val, old_fill_rate, old_empty_rate;
  bool update, using_special_rate;
  color col[4];
};



//-------------------------------------------------------------------------------------

// widget used for positioning of other widgets
class layout_widget : public widget
{
public:
  layout_widget( const char *_widget_name, widget *_parent, short _x, short _y );
  virtual ~layout_widget() {}

  virtual void frame_advance( time_value_t time_inc );
protected:
  void update_text();
  void update_position( short dx, short dy );
  void update_rotation( short dax, short day, short daz );
  bitmap_widget *bg;
  text_widget *name, *pos, *abs_pos, *x_angle, *y_angle, *z_angle;
  widget *cur_widget;
};


#endif // WIDGET_H
