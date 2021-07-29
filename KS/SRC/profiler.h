#ifndef PROFILER_H
#define PROFILER_H
////////////////////////////////////////////////////////////////////////////////

// profiler.h
// Copyright (c) 1999-2000 Treyarch L.L.C.  ALL RIGHTS RESERVED.

// utility class for profiling the game.
// uses the OS timer for extremely high resolution monitoring,
// and spits results to profile_log.txt, which you can then read in excel
// and make cute graphs out of.

// to add a NEW counter or timer check profcounters.cpp or proftimers.cpp.

////////////////////////////////////////////////////////////////////////////////

//#define PROFILING_ON	// uncomment to profile, but beware --- messes up VU debugging (dc 06/08/01)

#include "ostimer.h"

#ifdef PROFILING_ON

#include "stringx.h"
#include "singleton.h"
#include "color.h"

extern bool g_show_profile_info;

class profiler;

class profiler_entry
{
protected:
  friend class profiler;
  static profiler_entry* entries;
  static int num_entries;
  profiler_entry* next;
  profiler_entry* children;

  // preallocated string buffer that holds the string containing the current value.
  stringx text;

  // character pointers into text to write '+' or '-' and timer data
  char* open_pos;
  char* value_pos;

  void create_text();
  virtual void update_text();
  virtual stringx get_value_text() const=0;
  virtual color32 get_color() const=0;

  enum {NAME_LEN=32};
  char name[NAME_LEN];
  bool clear_each_frame:1,
       open:1;
  uint8 indent;

  void print(int x, int y);
  virtual void reset()=0; // called once per frame to clear for further accumulation

  void add_sibling(profiler_entry* t)
  {
    profiler_entry* si = this;
    do
    {
      if (si == t) return;
      if (!si->next) break;
      si = si->next;
    } while (1);
    si->next = t;
  }

  void add_child(profiler_entry* t)
  {
    if (children)
      children->add_sibling(t);
    else
      children = t;
  }

public:
  profiler_entry(const char* aname, profiler_entry* parent=0, bool clear=true, bool isopen=true);
  void toggle_open() { open = !open; }
  bool is_open() { return open; }

  virtual ~profiler_entry() {}
  stringx get_name() const { return stringx(name); }
  stringx get_value() { update_text(); return stringx( value_pos ); }
  stringx get_text() { update_text(); return text; }

  virtual bool check_skip() const { return false; }
};

// this doesn't have any functionality except being able to contain other entries
class profiler_group : public profiler_entry
{
protected:
  friend class profiler;

  virtual void reset();
  virtual void update_text();
  virtual stringx get_value_text() const;
  virtual color32 get_color() const;

public:
  profiler_group(const char* aname, profiler_entry* parent=0, bool isopen=true)
    : profiler_entry(aname,parent,false,isopen)
  {
  }
};

class profiler_counter : public profiler_entry
{
protected:
  friend class profiler;
  unsigned limit;
  unsigned last_count;
  unsigned cur_accum_count;
  unsigned avg_count;

  virtual void reset();
  virtual void update_text();
  virtual stringx get_value_text() const;
  virtual color32 get_color() const;

public:
  profiler_counter(const char* aname, profiler_entry* parent=0, unsigned alimit=0, bool clear=true, bool isopen=true)
    : profiler_entry(aname,parent,clear,isopen), limit(alimit), last_count(0), cur_accum_count(0), avg_count(0)
  {
  }
  void add_count(unsigned n) { cur_accum_count += n; }
  void set_count(unsigned n) { cur_accum_count = n; }
  unsigned get_total() const { return last_count; }
  unsigned get_average() const { return avg_count; }
  unsigned get_running_total() const { return cur_accum_count; }
  unsigned get_limit() const { return limit; }
};

class profiler_value : public profiler_entry
{
protected:
  friend class profiler;
  float limit;
  float last_value;
  float cur_accum_value;
  float avg_value;

  virtual void reset();
  virtual void update_text();
  virtual stringx get_value_text() const;
  virtual color32 get_color() const;

public:
  profiler_value(const char* aname, profiler_entry* parent=0, float alimit=0, bool clear=true, bool isopen=true)
    : profiler_entry(aname,parent,clear,isopen), limit(alimit), last_value(0), cur_accum_value(0), avg_value(0)
  {
  }
  void add_value(float n) { cur_accum_value += n; }
  void set_value(float n) { cur_accum_value = n; }
  unsigned get_total() const { return (unsigned)last_value; }
  unsigned get_average() const { return (unsigned)avg_value; }
  unsigned get_running_total() const { return (unsigned)cur_accum_value; }
  unsigned get_limit() const { return (unsigned)limit; }
};

#ifdef TARGET_PS2
enum
{
  PROF_MILLISECOND,
  PROF_CYCLE,
  PROF_DCACHE_MISS,
  PROF_DCACHE_MISS_MS,
  PROF_HIT_COUNTER,
  PROF_PER_CALL,
  //PROF_MILLISECOND_MAX,
};
extern int g_prof_timer_mode;
#endif

////////////////////////////////////////////////////////////////////////////////

class profiler : public singleton
{
public:
  DECLARE_SINGLETON(profiler)

  profiler();
  ~profiler();

	// begin a NEW frame
  void start_frame();

  void render();

  // output to a host file
  void write_to_host_file(host_system_file_handle outfile);

  profiler_entry* get_sel_entry() { return selected_entry; }
  void scroll(int amount);

private:
  friend class profiler_entry;

  int selected_line;
  profiler_entry* selected_entry;

  // recursive tree-walking functions
  static void reset_entry(profiler_entry*);
  static void print_entry(profiler_entry*, int& line);
  static void write_entry(profiler_entry*, host_system_file_handle outfile);
  static void write_entry_excel(profiler_entry*, host_system_file_handle outfile);
};

#define PROFILER_HISTORYCOUNT 30

class profiler_timer : public profiler_entry
{
protected:
  friend class profiler;
  time_value_t limit;
  time_value_t max_time;
  time_value_t last_time;
  time_value_t avg_time;
  bool started;

#if defined(TARGET_PC) || defined(TARGET_XBOX) || defined(TARGET_GC)
  hires_clock_t timer;
  time_value_t cur_accum_time;
#endif

#ifdef TARGET_PS2
  int start_time;
  int cur_accum_time;
#endif

  int cur_accum_count;

  time_value_t history_ring[PROFILER_HISTORYCOUNT];
  int history_index;
  time_value_t history_total;

  static float skip_thresh;

  virtual void reset();
  virtual void update_text();
  virtual stringx get_value_text() const;
  virtual color32 get_color() const;

public:
  profiler_timer(const char* aname, profiler_entry* parent=0, time_value_t alimit=0.0f, bool clear=true, bool isopen=true)
    : profiler_entry(aname,parent,clear,isopen), limit(alimit), max_time(0.0f), last_time(0.0f), avg_time(0.0f), started(false), 
	cur_accum_time(0), history_index(0), history_total(0)
  {
	  memset(history_ring, 0, sizeof(history_ring));
  }

#if defined(TARGET_PC) || defined(TARGET_XBOX) || defined(TARGET_GC)
  void start() { assert(!started); started=true; timer.reset(); }
  void stop() { /*assert(started);*/ started=false; cur_accum_time += timer.elapsed(); }
  time_value_t get_running_total() const { return cur_accum_time; }
#endif

#ifdef TARGET_PS2
  void start()
  {
	  assert(!started);
	  started=true;
	  start_time = scePcGetCounter1();
  }
  void stop()
  {
	/*assert(started);*/
	started=false;
	cur_accum_count++;
	cur_accum_time += scePcGetCounter1() - start_time;
  }
  time_value_t get_running_total();
#endif

  time_value_t get_total() const { return last_time; }
  time_value_t get_average() const { return avg_time; }
  time_value_t get_limit() const { return limit; }
  virtual bool check_skip() const;
};

// these are the bare minimum needed to compute frame rate
// in fact, I should have hw_rasta or game keep track of framerate, not profiler.
extern profiler_timer proftimer_frame_total;
//  extern profiler_timer proftimer_frame_render;
//  extern profiler_timer proftimer_frame_advance;
  extern profiler_timer proftimer_frame_flip;
  extern profiler_timer proftimer_frame_limiter;


#define START_PROF_TIMER(a) { extern profiler_timer a; a.start(); }
#define STOP_PROF_TIMER(a)  { extern profiler_timer a; a.stop(); }

#define SET_PROF_COUNT(a,b) { extern profiler_counter a; a.set_count(b); }
#define ADD_PROF_COUNT(a,b) { extern profiler_counter a; a.add_count(b); }

#else // !PROFILING_ON

// make some fake empty classes so we don't have to wrap lots of stuff in #ifdef PROFILING_ON

class profiler_group
{
public:
  profiler_group(const char* aname, void* parent=0, bool isopen=true) {}
};

class profiler_counter
{
public:
  profiler_counter(const char* aname, void* parent=0, unsigned alimit=0, bool clear=true, bool isopen=true) {}
  inline void add_count(unsigned n) {}
  inline void set_count(unsigned n) {}
  inline unsigned get_total() const { return 0; }
  inline unsigned get_average() const { return 0; }
  inline unsigned get_running_total() const { return 0; }
  inline unsigned get_limit() const { return 0; }
};

class profiler_value
{
public:
  profiler_value(const char* aname, void* parent=0, float alimit=0, bool clear=true, bool isopen=true) {}
  inline void add_value(float n) {}
  inline void set_value(float n) {}
  inline unsigned get_total() const { return 0; }
  inline unsigned get_average() const { return 0; }
  inline unsigned get_running_total() const { return 0; }
  inline unsigned get_limit() const { return 0; }
};

class profiler_timer
{
public:
  profiler_timer(const char* aname, void* parent=0, time_value_t alimit=0.0f, bool clear=true, bool isopen=true) {}
  inline void start() {}
  inline void stop() {}
  inline time_value_t get_total() const { return 0.0f; }
  inline time_value_t get_average() const { return 0.0f; }
  inline time_value_t get_running_total() const { return 0.0f; }
  inline time_value_t get_limit() const { return 0.0f; }
};

#define START_PROF_TIMER(a) {}
#define STOP_PROF_TIMER(a)  {}

#define SET_PROF_COUNT(a,b) {}
#define ADD_PROF_COUNT(a,b) {}




#endif // !PROFILING_ON

#if defined(TARGET_PS2) && defined(PROFILING_ON)

#undef STOP_PS2_PC
#undef START_PS2_PC
#define STOP_PS2_PC  stop_ps2_pc()
#define START_PS2_PC start_ps2_pc()

void stop_ps2_pc();
void start_ps2_pc();

#else
#define STOP_PS2_PC  
#define START_PS2_PC 

#endif
#endif // PROFILER_H
