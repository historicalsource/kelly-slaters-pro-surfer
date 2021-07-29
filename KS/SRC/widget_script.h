#ifndef WIDGET_SCRIPT_H
#define WIDGET_SCRIPT_H

class widget;

//-------------------------------------------------------------------------------------

#define NUM_COLOR_CLUES   3

enum clue_col_e
{
  CLUE_Default,
  CLUE_Red,
  CLUE_Green,
  CLUE_Blue,
  CLUE_Total,
};

class clue_widget : public widget
{
public:
  clue_widget( const char *_widget_name, widget *_parent, short _x, short _y );
  virtual ~clue_widget() {}

  void set_clue_col( int panel, clue_col_e cc );
  void clear_clues();
  const clue_col_e& get_clue_col( int panel ) { return ( clue_col[panel] ); }

  void set_show_while_letterboxed( bool show_during ) { show_while_letterboxed = show_during; }
  bool get_show_while_letterboxed() const { return show_while_letterboxed; }

protected:
  bool show_while_letterboxed;
  bitmap_widget *clue_bar, *clue[NUM_COLOR_CLUES];
  clue_col_e clue_col[NUM_COLOR_CLUES];
};

//-------------------------------------------------------------------------------------

class timer_widget : public widget
{
public:
  timer_widget( const char *_widget_name, widget *_parent, short _x, short _y );
  virtual ~timer_widget();

  virtual void show();
  virtual void frame_advance( time_value_t time_inc );

  void set_digit_color(color col);
  void set_bg_color(color col);

  virtual void update_scale();
  
  void set_time_left( rational_t _time_left );
  rational_t get_time_left() const { return time_left; }
  void freeze() { running = false; }
  void run() { running = true; }
  void inc_time_left( rational_t time_delta );

  virtual void render();

//  void use_red_nums();
//  void use_green_nums();

  void add_script_function(const stringx &func, time_value_t time);
  void remove_script_function(time_value_t start = -1.0f, time_value_t end = FLT_MAX);

  virtual void set_layer( rhw_layer_e rhw_layer );

protected:
  class timer_func
  {
  public:
    time_value_t time;
    stringx function;

    timer_func()
    {
      time = -1.0f;
      function = empty_string;
    }

    timer_func(const stringx &f, time_value_t t)
    {
      time = t;
      function = f;
    }

    void copy(const timer_func &b)
    {
      time = b.time;
      function = b.function;
    }

    timer_func(const timer_func &b)
    {
		  copy( b );
    }

    ~timer_func()
    {
    }

    timer_func& operator=(const timer_func &b) 
    {
		  copy( b );
      return *this;
    }
  };

  void resize_timer();
//  void set_digit_vrep( int index, int num );
//  vrep_widget *digits[4];
  bitmap_widget *digits[10];
  bitmap_widget *colon1, *colon2, *point, *bg;

  int minutes;
  int seconds1;
  int seconds2;
  int tenths;

#ifdef NGL
//  nglMesh* digit_mesh[10];
#else
//  visual_rep *digit_vreps[10];//, *red_vreps[10];
#endif

  rational_t time_left;
  bool running, updated;//, using_red_nums;

  vector<timer_func> script_calls_left;
  vector<timer_func> script_calls_made;
};


//-------------------------------------------------------------------------------------

class script_widget_holder_t : public widget
{
public:
  script_widget_holder_t( const char *_widget_name, widget *_parent, short _x, short _y );
  virtual ~script_widget_holder_t() {}
  void frame_advance( time_value_t time_inc );
  void freeze() { running = false; }
  void run() { running = true; }

//  clue_widget *get_clue_widget() const { return my_clue_widget; }
  timer_widget *get_timer_widget() const { return my_timer_widget; }

protected:
//  clue_widget *my_clue_widget;
  timer_widget *my_timer_widget;
  bool running;
};




#endif // WIDGET_SCRIPT_H