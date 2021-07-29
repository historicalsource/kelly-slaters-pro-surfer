//-------------------------------------------------------------------------------------------------------
// Profiler.cpp
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.
// code timing stuff
//-------------------------------------------------------------------------------------------------------
#include "global.h"

//#include "project.h"

#include "profiler.h"

#ifdef PROFILING_ON

#include "osdevopts.h"
#include "debug.h"
#include "hwrasterize.h"

#include <limits.h>

//P #include "membudget.h"

#include "debug_render.h"

//#pragma message( "Profiling is enabled" )

bool g_show_profile_info = false;

DEFINE_SINGLETON(profiler)

profiler_entry* profiler_entry::entries=NULL;
int profiler_entry::num_entries=0;

float profiler_timer::skip_thresh = 0.01f;

int entry_count = 0;

profiler_entry::profiler_entry(const char* aname, profiler_entry* parent, bool clear, bool isopen)
: clear_each_frame(clear), open(isopen), indent(0)
{
  assert( strlen(aname)<NAME_LEN );
  strcpy( name, aname );
  ++num_entries;
  next = NULL;
  if (parent)
  {
    parent->add_child(this);
    indent = parent->indent + 1;
  }
  else if (entries)
    entries->add_sibling(this);
  else
    entries = this;
}

void profiler_entry::create_text()
{
  // calculate the string buffer
  static const int max_line_width=32;
  static const char dots[] = " . . . . . . . . . . . . . . . . . . . ";
  static const char inds[] = "|||||||||||ò-";
  assert(indent<10);
  char work_str[128] = "";
  strncpy( work_str, inds + 10 - indent, indent );
  strcat( work_str, name );
  if ( strlen( work_str )  & 1 ) strcat( work_str, " " );
  stringx val = get_value_text();
  int ndots = max_line_width - strlen( work_str ) - val.length();
  if (ndots<1) ndots=1;
  strncat( work_str, dots, ndots );
  strcat( work_str, val.c_str() );

  text = work_str;
  open_pos = strchr( text.c_str(), 'ò' );
  value_pos = strchr( text.c_str(), 'û' );
}

void profiler_entry::update_text()
{
  if ( !text.length() )
    create_text();

  if ( open_pos )
  {
    if ( open )
      *open_pos = '-';
    else
      *open_pos = '+';
  }
}

void profiler_entry::print(int x,int y)
{
  update_text();
  hw_rasta::inst()->print(text, vector2di(x,y), get_color());
}

void profiler_group::reset()
{}

#ifdef TARGET_XBOX
static const color32 violet(255,128,255);
static const color32 white (255,255,255);
static const color32 red   (255,128,128);
static const color32 yellow(255,255,128);
static const color32 green (128,255,128);
#else
static const color32 violet(160,96,160);
static const color32 white (160,160,160);
static const color32 red   (255,96,96);
static const color32 yellow(160,160,96);
static const color32 green (96,160,128);
#endif

color32 profiler_group::get_color() const
{
  return violet;
}
stringx profiler_group::get_value_text() const
{
  static const stringx bars("======");
  return bars;
}
void profiler_group::update_text()
{
  profiler_entry::update_text();
}

color32 profiler_timer::get_color() const
{
  if (!limit/* || !os_developer_options::inst()->is_flagged(os_developer_options::FLAG_SHOW_LIMIT_INFO)*/)
    return white;
  if (last_time > limit)
    return red;
  if (last_time > limit*0.875f)
    return yellow;
  return green;
}

color32 profiler_value::get_color() const
{
  if (!limit/* || !os_developer_options::inst()->is_flagged(os_developer_options::FLAG_SHOW_LIMIT_INFO)*/)
    return white;
  if (last_value > limit)
    return red;
  if (last_value > limit*0.875f)
    return yellow;
  return green;
}

color32 profiler_counter::get_color() const
{
  if (!limit/* || !os_developer_options::inst()->is_flagged(os_developer_options::FLAG_SHOW_LIMIT_INFO)*/)
    return white;
  if (last_count > limit)
    return red;
  if (last_count > ((limit*7)>>3))
    return yellow;
  return green;
}

// what
stringx profiler_timer::get_value_text() const
{
	static stringx value_text( "û           " );
	//stringx value_text( "û           " );
  return value_text;
}

bool g_prof_avg = true;
bool g_prof_max = false;
bool g_prof_max_reset = false;
void profiler_timer::update_text()
{
  profiler_entry::update_text();

  strcpy( value_pos, " . . . . . ." );
  float val;

  if (g_prof_max)
  {
	  val = max_time;
  }
  else if (g_prof_avg)
  {
	  val = avg_time;
  }
  else
  {
	  val = last_time;
  }

#ifdef TARGET_PS2
  switch ( g_prof_timer_mode )
  {
  case PROF_MILLISECOND:
  //case PROF_MILLISECOND_MAX:
  case PROF_DCACHE_MISS_MS:
  case PROF_HIT_COUNTER:
    val = min( val, 9999.99f );  // in ms
    sprintf( value_pos + 5, "%7.2f", val );
    break;
  case PROF_PER_CALL:
    val = min( val, 9999.99f );  // in ms
    sprintf( value_pos + 5, "%7.5f", val );
    break;
  case PROF_CYCLE:
  case PROF_DCACHE_MISS:
    val = min( val, 999999999.99f );  // in ms
    sprintf( value_pos, "%12.2f", val );
    break;
  }
#else
  val = min( val, 9999.99f );  // in ms
  sprintf( value_pos + 5, "%7.2f", val );
#endif
}

stringx profiler_value::get_value_text() const
{
  static stringx value_text( "û     " );
  return value_text;
}
void profiler_value::update_text()
{
  profiler_entry::update_text();

  float val = min( last_value, 999.99f );
  sprintf( value_pos, "%6.2f", val );
}

stringx profiler_counter::get_value_text() const
{
  static stringx value_text( "û       " );
  return value_text;
}
void profiler_counter::update_text()
{
  profiler_entry::update_text();

  int c = min( (int)last_count, (int)99999999 );
  sprintf( value_pos, "%8d", c );
}

/*
stringx profiler_timer::get_avg_value() const
{
  return stringx(stringx::fmt,"%6.2f",avg_time*1000.0f); // in ms
}

stringx profiler_value::get_avg_value() const
{
  return stringx(stringx::fmt,"%6.2f",avg_value);
}

stringx profiler_counter::get_avg_value() const
{
  return stringx(stringx::fmt,"%6d",avg_count);
}
*/

void profiler_counter::reset()
{
  last_count = cur_accum_count;
  //avg_count = (avg_count * 7 + last_count)>>3; // simple weighted average
  avg_count = (avg_count + last_count)>>1; // simpler weighted average
  if (clear_each_frame) cur_accum_count = 0;
}

void profiler_value::reset()
{
  last_value = cur_accum_value;
  avg_value = (avg_value * 7.0f + last_value)*(1.0f/8); // simple weighted average
  if (clear_each_frame) cur_accum_value = 0.0f;
}

void profiler_timer::reset()
{
	avg_time = history_total / PROFILER_HISTORYCOUNT;

#ifdef TARGET_PS2
  switch ( g_prof_timer_mode )
  {
  case PROF_MILLISECOND:
  //case PROF_MILLISECOND_MAX:
    last_time = (float)cur_accum_time / 294912.0f;
		if ( g_prof_max_reset )
		{
			max_time=0.0f;
		}
		else if ( max_time<last_time )
		{
			max_time=last_time;
		}
    break;
  case PROF_DCACHE_MISS_MS:
    // guesstimate on cache miss cycles.
    last_time = (float)cur_accum_time * 39.1f / 294912.0f;
    break;
  case PROF_CYCLE:
  case PROF_HIT_COUNTER:
  case PROF_DCACHE_MISS:
    last_time = (float) cur_accum_count;
    break;
  case PROF_PER_CALL:
	  last_time = cur_accum_count ? (float) cur_accum_time / (294912.0f * (float) cur_accum_count) : 0;
    break;
  }


#else
  last_time = cur_accum_time * 1000.0f;
#endif

	time_value_t last_history_time = history_ring[history_index];
	history_ring[history_index++] = last_time;
	history_index %= PROFILER_HISTORYCOUNT;
	history_total += last_time;
	history_total -= last_history_time;

// old way (dc 08/13/01)
//  avg_time = (avg_time * 7.0f + last_time)*(1.0f/8); // simple weighted average
  if (clear_each_frame)
  {
	  cur_accum_time = 0;
	  cur_accum_count = 0;
  }
}

#ifdef TARGET_PS2
  #define LINE_SPACE 14
  int lines_per_screen=25;
#else
  #define LINE_SPACE 13
  int lines_per_screen=27;
#endif

int skip_lines=0; //lines_per_screen*10;
int scroll_lines=10;



profiler::profiler()
{
  selected_line = 0;
}

profiler::~profiler()
{
}

bool profiler_timer::check_skip() const
{
	return false;
	if ( g_prof_max )
		return max_time < skip_thresh;
	return avg_time < skip_thresh;
}

static int first_line,last_line;
#ifdef TARGET_XBOX
static int g_profiler_text_l = 50;
#else
static int g_profiler_text_l = 20;
#endif

void profiler::print_entry(profiler_entry* t, int& line)
{
	int xoff = g_profiler_text_l;

  if (!t) return;
  if (line>last_line) return;
  if (!t->check_skip())
  {
	  if ( line == profiler::inst()->selected_line )
	  {
		profiler::inst()->selected_entry = t;
		xoff -= 10;
	  }
	  if (line>=first_line)
	  {
#ifdef TARGET_PS2
		t->print(xoff,50+LINE_SPACE*(1+line-first_line));
#else
		if (line==profiler::inst()->selected_line)
		  hw_rasta::inst()->print("=>", vector2di(10,52+LINE_SPACE*(1+line-first_line)), color32_white);
		t->print(xoff,52+LINE_SPACE*(1+line-first_line));
#endif
	  }
	  ++line;
  }
  if (t->is_open())
    print_entry(t->children,line);
  print_entry(t->next,line);
}

void profiler::write_entry_excel(profiler_entry* t, host_system_file_handle outfile)
{
  if (!t) return;
  host_fprintf(outfile, "%s,%s\n", t->get_name().c_str(),t->get_value().c_str());
  write_entry_excel(t->children,outfile);
  write_entry_excel(t->next,outfile);
}

void profiler::write_entry(profiler_entry* t, host_system_file_handle outfile)
{
  if (!t) return;
  host_fprintf(outfile, "%s\n", t->get_text().c_str());
  write_entry(t->children,outfile);
  write_entry(t->next,outfile);
}

void profiler::reset_entry(profiler_entry* t)
{
  if (!t) return;
  t->reset();
  reset_entry(t->children);
  reset_entry(t->next);
}

char* prof_mode_names[] =
{
  "Milliseconds",
  "CPU Cycles",
  "Data Cache Misses",
  "Estimated Data Cache MS",
  "Hit Counter",
  "MS Per Call",
};

int g_prof_timer_mode = 0;
#ifndef countof
#define countof(x) (sizeof(x) / sizeof(*x))
#endif /* countof */

void profiler::start_frame()
{
/*	If we really need this, add a PROF_MAX instead of hard-coding 4.  (dc 08/15/01)
  if ( g_prof_timer_mode > 4 )
    g_prof_timer_mode = 0;
*/
  assert(countof(prof_mode_names) == 6);	// replace with PROF_MAX

  reset_entry(profiler_entry::entries);

#if defined(TARGET_PS2) && defined(PROFILING_ON)
  // reset the performance counter to monitor CPU cycles and cache misses.
  int control = 0;
  switch ( g_prof_timer_mode )
  {
  case PROF_MILLISECOND:
  case PROF_CYCLE:
  case PROF_HIT_COUNTER:
  case PROF_PER_CALL:
	control = SCE_PC0_NO_EVENT | SCE_PC1_CPU_CYCLE | (SCE_PC_U1|SCE_PC_S1|SCE_PC_K1|SCE_PC_EXL1) | SCE_PC_CTE;
	break;
  case PROF_DCACHE_MISS:
  case PROF_DCACHE_MISS_MS:
	control = SCE_PC0_NO_EVENT | SCE_PC1_DCACHE_MISS | (SCE_PC_U1) | SCE_PC_CTE;
	break;
  };
  scePcStart( control, 0, 0 );
  switch ( g_prof_timer_mode )
  {
  case PROF_MILLISECOND:
  case PROF_CYCLE:
  case PROF_DCACHE_MISS:
  case PROF_DCACHE_MISS_MS:
	profiler_timer::skip_thresh = 0.01f;
	break;
  case PROF_HIT_COUNTER:
	profiler_timer::skip_thresh = 0.5f;
	break;
  case PROF_PER_CALL:
	profiler_timer::skip_thresh = 0.0001f;
	break;
  };
#endif
}

static u_int g_profiler_background_colora = 0xc0;
static u_int g_profiler_background_colorr = 0;
static u_int g_profiler_background_colorg = 0;
static u_int g_profiler_background_colorb = 0;
static color32 g_profiler_background_color = color32(
	g_profiler_background_colorr,
	g_profiler_background_colorg,
	g_profiler_background_colorb,
	g_profiler_background_colora
);	// pretty dark, but still see-through
static float g_profiler_background_z = 0.3f;	// slightly farther than game near z plane
static float g_profiler_background_l = 0;
static float g_profiler_background_r = 400;
static float g_profiler_background_t = 10;
static float g_profiler_background_b = 440;

void profiler::render()
{
  first_line = skip_lines;
  last_line = first_line + lines_per_screen;
  if ( g_show_profile_info )
  {
    int line = 0;

    // draw a great big dark quad behind the output, so we can see it clearly.
	nglQuad quad;
	nglInitQuad(&quad);
	nglSetQuadZ(&quad, g_profiler_background_z);
	nglSetQuadRect(&quad, g_profiler_background_l, g_profiler_background_t, g_profiler_background_r, 
		g_profiler_background_b);
	nglSetQuadColor(&quad, NGL_RGBA32(g_profiler_background_colorr, g_profiler_background_colorg, 
		g_profiler_background_colorb, g_profiler_background_colora));
	nglListAddQuad(&quad);

    hw_rasta::inst()->print( prof_mode_names[g_prof_timer_mode], vector2di(g_profiler_text_l,40), color32_cyan );

    entry_count = 0;
    print_entry(profiler_entry::entries,line);
  }
  else
  {
/*P
    if ( os_developer_options::inst()->is_flagged(os_developer_options::FLAG_SHOW_MEMORY_BUDGET) )
    {
      stringx color_str;
      int i;
      for ( i=0; i<membudget_t::N_CATEGORIES; ++i )
      {
        // determine color based on current usage
        int used = membudget()->get_usage( (membudget_t::category_t)i );
        int budgeted = membudget()->get_cap( (membudget_t::category_t)i );
        color32 clr;
        if ( used < budgeted*0.9f )
          clr = color32(128,128,128);  // white (<90%)
        else if ( used <= budgeted )
          clr = color32(160,160,0);  // yellow (<=100%)
        else
          clr = color32(160,32,32);  // red (>100%)
        int vs = 14;
        const char* name = get_membudget_category_name( (membudget_t::category_t)i );
        hw_rasta::inst()->print( name,                   vector2di( 50,60+vs*i), clr );
        hw_rasta::inst()->print( itos(used),             vector2di(190,60+vs*i), clr );
        hw_rasta::inst()->print( "/  " + itos(budgeted), vector2di(264,60+vs*i), clr );
      }
    }
P*/
  }
}

void profiler::scroll(int dir)
{
  if ( !g_show_profile_info )
    return;

  selected_line += dir;
  if ( selected_line < 0 )
    selected_line = 0;
  if ( selected_line >= profiler_entry::num_entries )
    selected_line = profiler_entry::num_entries - 1;

  if ( selected_line <= skip_lines )
    skip_lines = selected_line - 1;
  if ( selected_line >= skip_lines + lines_per_screen )
    skip_lines = selected_line - lines_per_screen + 1;

  if ( skip_lines < 0 )
    skip_lines = 0;
  if ( skip_lines >= profiler_entry::num_entries - lines_per_screen )
    skip_lines = profiler_entry::num_entries - lines_per_screen - 1;
}

void profiler::write_to_host_file(host_system_file_handle outfile)
{
  host_fprintf(outfile, "//////////////////HUMANS/////////////////////\n");
  write_entry(profiler_entry::entries,outfile);
  host_fprintf(outfile, "//////////////////EXCEL/////////////////////\n");
  write_entry_excel(profiler_entry::entries,outfile);
}


#ifdef TARGET_PS2
static int pc_count = 0;
void stop_ps2_pc()
{
    int control;
    pc_count--;
    control = SCE_PC0_CPU_CYCLE | (SCE_PC_U0|SCE_PC_S0|SCE_PC_K0|SCE_PC_EXL0);
    control |= SCE_PC1_DCACHE_MISS | (SCE_PC_U1);
    scePcStart( control, 0, 0 );
}

void start_ps2_pc()
{
    int control;
    pc_count++;
    assert(pc_count <= 0);
    if (pc_count == 0)
    {
      control = SCE_PC0_CPU_CYCLE | (SCE_PC_U0|SCE_PC_S0|SCE_PC_K0|SCE_PC_EXL0);
      control |= SCE_PC1_DCACHE_MISS | (SCE_PC_U1);
      control |= SCE_PC_CTE;
      scePcStart( control, 0, 0 );
    }
}

#endif


#endif // PROFILING_ON
