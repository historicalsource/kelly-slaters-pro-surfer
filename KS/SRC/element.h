#ifndef ELEMENT_H
#define ELEMENT_H

//#include "hwrasterize.h"
#include "rect.h"
#include "ostimer.h"
#include "color.h"

class font_def;
//class hw_texture;
#include "hwrasterize.h"

#define MAX_PERM_ITEMS 5

enum element_type_e
{
  ELEMENT_Container,          // element is simply a container for holding subelements
  ELEMENT_Text,                       // element is text
};

enum event_type_e
{
  EVENT_None,             // event does nothing
  EVENT_Color,            // event will do a linear interpolation to this color
  EVENT_Move,             // event will do a linear interpolation move to this point
  EVENT_Rotate,
  EVENT_Scale
};

class element;
class event;

typedef list<element *> element_list_t;
typedef list<event *> event_list_t;
//typedef map<int, element *> element_map_t;

class element_manager : public singleton
{
public:
  DECLARE_SINGLETON(element_manager)
  stringx interface_font_name;

  element_manager();
  ~element_manager();
  void purge();
  void create_default_elements();
  void restore_default_elements(); // damned singletons everywhere
  void enable() { enabled = true; }
  void disable() { enabled = false; }
  void push_context( const stringx& name );
  void pop_context();
  void push_back_element( element *e ) { context_stack.front()->push_back( e ); }
  void remove_element( element *e ) { context_stack.front()->remove( e ); }
  void frame_advance( time_value_t time_inc );
  void render();

  // because flat shading is different for some reason between Sega & nVidia:
  //hw_texture*   get_black_texture() { return black_texture; }
  //hw_texture*   get_white_texture() { return hw_texture_mgr::inst()->get_white_texture(); }
    // functions for scripting functionality
//    rational_t     bind_element(element *e);
//    element       *lookup_element(rational_t id);

  matrix4x4 projection_matrix;

  int get_num_context() const { return context_stack.size(); }

protected:
  class context : public element_list_t
  {
  public:
    context( const stringx& _name ) : name( _name ) {}
    const stringx& get_name() const { return name; }
  protected:
    stringx name;
  };
  // variables
  list<context *> context_stack;      // front context on stack is only one active
//  element_map_t element_map;        // so scripts can deal with handles instead of pointers
  int next_id;
  //refptr<hw_texture> black_texture;
  //refptr<hw_texture> white_texture;
  bool enabled;
};


class event
{
public:
  event( event_type_e t, element *e, time_value_t wt, time_value_t d )
  {
    type = t;
    owner = e;
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
  const float get_lerp( const time_value_t& t );
  void set_time_left( const time_value_t& t );
  const event_type_e& get_type() { return type; }
  void frame_advance( time_value_t time_inc );
  virtual void do_event( const float& ) = 0;
protected:
  event_type_e type;
  element     *owner;
  time_value_t wait_time;           // time to wait before activation (when passed by elapsed, activate)
  time_value_t duration;
  time_value_t elapsed;             // starts counting when added
};

class move_event: public event
{
public:
  move_event( element *e, time_value_t wt, time_value_t d, int _x, int _y ) :
      event( EVENT_Move, e, wt, d ) { x = float(_x); y = float(_y); }
  virtual void do_event( const float& );
protected:
  float x;
  float y;
};

class color_event: public event
{
public:
  color_event( element *e, time_value_t wt, time_value_t d, color _mycolor ) :
      event( EVENT_Color, e, wt, d ), mycolor( _mycolor ) {}
  virtual void do_event( const float& );
protected:
  color mycolor;
};

class rotate_event: public event
{
public:
  rotate_event( element *e, time_value_t wt, time_value_t d, float _angle ) :
      event( EVENT_Rotate, e, wt, d ), angle( _angle ) {}
  virtual void do_event( const float& );
protected:
  float angle;
};

class scale_event: public event
{
public:
  scale_event( element *e, time_value_t wt, time_value_t d, float _sx, float _sy ) :
      event( EVENT_Scale, e, wt, d ), sx( _sx ), sy( _sy ) {}
  virtual void do_event( const float& );
protected:
  float sx;
  float sy;
};

#define ELEMENTF_Hidden 0x00000001


class element
{
public:
  element(element *p);
  virtual ~element();
  void           move_to( int x, int y ) { T[0] = (float)x; T[1] = (float)y; }
  void           move_to( time_value_t wt, time_value_t d, int x, int y ) { move_event *e; e = NEW move_event( this, wt, d, x, y ); add_event( e ); }
  void           scale_to(float hs, float vs) { S[0] = hs; S[1] = vs; }
  void           scale_to(time_value_t wt, time_value_t d, float hs, float vs) { scale_event *e; e = NEW scale_event( this, wt, d, hs, vs ); add_event( e ); }
  void           rotate_to(float a)
                  {
                    float st, ct;
                    angle = a;

                    fast_sin_cos_approx( a, &st, &ct );

                    R[0][0] = ct;
                    R[0][1] = -st;
                    R[1][0] = st;
                    R[1][1] = ct;
                  }

  virtual void   rotate_to(time_value_t wt, time_value_t d, float a) { rotate_event *e; e = NEW rotate_event( this, wt, d, a ); add_event( e ); }
  virtual void   set_subrect(int x0, int y0, int x1, int y1) { subrect = rectf(x0, y0, x1, y1); }
  void           set_origin(int ox, int oy) { O[0] = (float)ox; O[1] = (float)oy; }
  void           set_color(color c) { mycolor[0] = c; mycolor[1] = c; mycolor[2] = c; mycolor[3] = c; }
  void           set_color(time_value_t wt, time_value_t d, color c) { color_event *e; e = NEW color_event( this, wt, d, c ); add_event( e ); }
  void           set_color( color col0, color col1, color col2, color col3 )  // to set colors per vert
  {
    mycolor[0] = col0;
    mycolor[1] = col1;
    mycolor[2] = col2;
    mycolor[3] = col3;
  }
  void           set_scale(float hs);
  void           add_event(event *e);
  void           event_flush();
  void           offset(float v[2]);
  virtual void   transform(float v[2], color &c, int index);
  void           ndc(float v[2]);
  void           hide() { flags |= ELEMENTF_Hidden; }
  void           show() { flags &= ~ELEMENTF_Hidden; }
  bool           is_visible() { return !(flags & ELEMENTF_Hidden); }
  bool           event_run_list_empty() { return ( event_run_list.empty() ); }
  virtual void   render() = 0;
  void           frame_advance( time_value_t time_inc );
  void           set_linear_animation( bool _linear_animation ) { linear_animation = _linear_animation; }
  friend class   event;
  friend class   move_event;
  friend class   color_event;
  friend class   rotate_event;
  friend class   scale_event;
protected:
  element       *parent;        // parent, if any, of this element
  element_type_e type;        // type of this element
  unsigned int   flags;
  element_list_t subelements;     // list of elements of which this is the parent
  rectf          subrect;             // subrect of element which is actually displayed
  float          R[2][2];       // 2D rotation into element space
  float          T[2];        // 2D translation into element space
  float          S[2];                // 2D scale factor
  float          S_Override[2];       // 2D scale factor Override
  float          O[2];                // 2D origin offset
  float          angle;               // 2D angle of rotation
  color          mycolor[4];          // vertex color info
  event_list_t   event_run_list;      // list of currently running events (ordered by time of activation)
  bool           linear_animation;    // Is animation linear or time active / time left?
};


class text_element: public element
{
public:
  text_element(element *p, font_def *f, stringx t = empty_string)
    : element(p)
  { type = ELEMENT_Text; font = f; text = t; }
  virtual ~text_element();
  void      render();
  void      set_text(const char *fmt, ...);
  int       get_text_width();    // in pixels
protected:
  font_def *font;
  stringx   text;
};


extern font_def       *font;
extern text_element   *test_text;

#endif // ELEMENT_H
