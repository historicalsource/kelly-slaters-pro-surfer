#include "global.h"

#include "osalloc.h"

#include "gc_rasterize.h"
#include "gc_texturemgr.h"

#include "color.h"
#include <string>
#include <algorithm>

#include "stringx.h"
#include "txtcoord.h"
#include "algebra.h"
#include "singleton.h"
#include "refptr.h"
#include "projconst.h"

#include <stdio.h>

DEFINE_SINGLETON( hw_rasta )

enum {
  HWOSGC_DEFAULT_SCREEN_WIDTH = 640,
  HWOSGC_DEFAULT_SCREEN_HEIGHT= 448,
  HWOSGC_DEFAULT_MAX_TEXTURES = 512,
};

void hw_rasta::init_gc_gfx( void )
{
  screen_width = HWOSGC_DEFAULT_SCREEN_WIDTH;
  screen_height = HWOSGC_DEFAULT_SCREEN_HEIGHT;
  pixel_depth = 32;

#ifdef PROJECT_KELLYSLATER
  nglSetPerspectiveMatrix( proj_field_of_view_in_degrees(),
    0.2f,
    65536.0f );
#else
  nglSetPerspectiveMatrix( proj_field_of_view_in_degrees(),
    nglGetScreenWidth()/2,
    nglGetScreenHeight()/2,
    PROJ_NEAR_PLANE_D,
    PROJ_FAR_PLANE_D );
#endif
}

int LOW_LEVEL_CONSOLE_FONT_ID = 0;
#define LOW_LEVEL_CONSOLE_HEIGHT 16
#define LOW_LEVEL_CONSOLE_WIDTH 40
#define LOW_LEVEL_CONSOLE_COLOR 0x80808080 // rgba, 0x80 is 1.0f
#define LOW_LEVEL_CONSOLE_Z_DEPTH 1.0f
#define LOW_LEVEL_CONSOLE_X_SCALE 2.0f
#define LOW_LEVEL_CONSOLE_Y_SCALE 2.0f

static char llc_lines[LOW_LEVEL_CONSOLE_HEIGHT][LOW_LEVEL_CONSOLE_WIDTH];

static char llc_start = -1;
static char llc_end = -1;
static char llc_count = 0;
static bool llc_available = false;

void low_level_console_init( void )
{
/*
#pragma fixme("This texture is a memory leak, it will be fixed with FileFinder 2.0 (GT--4/20/01)")
  nglSetTexturePath( "" );
  nglLoadFontA( "font8x12", LOW_LEVEL_CONSOLE_FONT_ID );
  nglSetClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
*/
  llc_available = true;
/*
  nglFlip( );
*/
}

void low_level_console_release( void )
{
/*
  nglReleaseFont( LOW_LEVEL_CONSOLE_FONT_ID );
*/
  llc_available = false;
}

bool low_level_console_is_available( void )
{
  return llc_available;
}

void low_level_console_print(char *Text, ...)
{
#ifdef DEBUG	// Wouldn't want these to sneak in to final build.  (dc 03/27/02)
  char work_space[256];

  va_list argptr;
  va_start( argptr, Text );
  vsprintf( work_space, Text, argptr );
  va_end( argptr );

  char fixed_up_start = llc_start;
  if (fixed_up_start == 0)
    fixed_up_start = LOW_LEVEL_CONSOLE_HEIGHT;

  if (fixed_up_start == llc_end + 1)
  {
    assert(llc_count == LOW_LEVEL_CONSOLE_HEIGHT);

    // continuous scrolling case
    llc_start++;
    if (llc_start >= LOW_LEVEL_CONSOLE_HEIGHT)
      llc_start = 0;
    llc_end++;
    if (llc_end >= LOW_LEVEL_CONSOLE_HEIGHT)
      llc_end = 0;
  }
  else
  {
    if (llc_count == 0)
    {
      llc_start = 0;
      llc_end = 0;
      llc_count = 1;
    }
    else
    {
      // simple add case
      llc_end++;
      if (llc_end >= LOW_LEVEL_CONSOLE_HEIGHT)
        llc_end = 0;
      llc_count++;
    }
  }

  strncpy(llc_lines[llc_end], work_space, LOW_LEVEL_CONSOLE_WIDTH);
  llc_lines[llc_end][LOW_LEVEL_CONSOLE_WIDTH-1] = '\0';
#endif
}

// draws the low_level_console on screen (call nglFlip to draw it if we aren't in a game state)
void low_level_console_flush()
{
#ifdef FOODAWG //DEBUG	// Wouldn't want these to sneak in to final build.  (dc 03/27/02)
  rational_t y;
  char curr_line;

  if (llc_count == 0)
    return;

  nglListInit();
  for (y=30.0f, curr_line=llc_start; y<418.0f; y+= 13.0f, curr_line++)
  {
    if (curr_line >= LOW_LEVEL_CONSOLE_HEIGHT)
      curr_line = 0;

    // print the line of text
/*	Replaced by new API. (dc 05/30/02)
    KSNGL_SetFont( LOW_LEVEL_CONSOLE_FONT_ID );
    KSNGL_SetFontColor( LOW_LEVEL_CONSOLE_COLOR );
    KSNGL_SetFontZ( LOW_LEVEL_CONSOLE_Z_DEPTH );
    KSNGL_SetFontScale( LOW_LEVEL_CONSOLE_X_SCALE, LOW_LEVEL_CONSOLE_Y_SCALE );
*/
    nglListAddString( nglSysFont, 15, y, LOW_LEVEL_CONSOLE_Z_DEPTH, LOW_LEVEL_CONSOLE_COLOR, 
		NGLFONT_TOKEN_SCALEXY "[%f, %f]" "%s", LOW_LEVEL_CONSOLE_X_SCALE, LOW_LEVEL_CONSOLE_Y_SCALE, 
		llc_lines[curr_line] );

    if (curr_line == llc_end)
      break;
  }
  nglListSend();
  nglFlip();
#endif
}

void low_level_console_clear()
{
  llc_start = -1;
  llc_end = -1;
  llc_count = 0;
}

void llc_memory_log()
{
	// this call fails because "We shouldn't be using this function before nglListInit (and after nglListSend)" 
  //nglSetClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

  low_level_console_print( "total %dK  used %dK  unused %dK", 0, 0, 0 );
  low_level_console_flush( );
}

void hw_rasta::set_pal_refresh( hw_rasta::pal_refresh_t mode )
{

}

void hw_rasta::end_of_render_callback(void* param)
{

}

void hw_rasta::invalidate_fog()
{

}

void hw_rasta::update_fog()
{

}

void hw_rasta::set_next_video_mode( video_mode_t mode )
{
  // code me

}

hw_rasta::video_mode_t hw_rasta::get_video_mode() const
{
  // code me
  return video_mode;
}

int hw_rasta::get_screen_width() const
{
  return HWOSGC_DEFAULT_SCREEN_WIDTH;
}
  
int hw_rasta::get_screen_height() const
{
  return HWOSGC_DEFAULT_SCREEN_HEIGHT;
}

void hw_rasta::set_next_bit_depth( bit_depth_t depth )
{
  // code me
  next_bit_depth = depth;
}

void hw_rasta::reset()
{
  // code me
}

/*-------------------------------------------------------------------------------------------------------

  Functions prefixed with send queue commands to the display.  Their operations are stored in a buffer
  and affect all send commands issued after them.

  WARNING: 
    send_start and send_end must be called in the order specified in poly_type_t excluding
    PT_NONE.  Any other order will break.  Sections can be skipped.

-------------------------------------------------------------------------------------------------------*/

// Notify beginnings and endings of sets of polygons
void hw_rasta::send_start( poly_type_t type )
{
  send_type = type;
}

void hw_rasta::send_end()
{
  send_type = PT_NONE;
}

// Send a vertex buffer to be interpreted as individual triangles.
void hw_rasta::send_vertex_strip(const vert_buf& buf, int num_verts, unsigned flags )
{
}

// Send a vertex buffer to be interpreted as individual triangles.
void hw_rasta::send_vertex_strip( const vert_buf_xformed & buf, int num_verts, unsigned flags )
{
}


// Send a vertex buffer to be interpreted as individual triangles.
void hw_rasta::send_vertex_list( const vert_buf_xformed & buf, int num_verts, unsigned flags )
{
  // legacy?
}

// Send a vertex buffer to be interpreted as individual triangles.
void hw_rasta::send_vertex_list( const vert_buf & buf, int num_verts, unsigned flags )
{
  // legacy?
}
  

void hw_rasta::clip_send_vertex_list( hw_rasta_vert * verts, int num_verts )
{
  // legacy for sure.
}  // end function
  

const int MAX_TRIANGLES_WE_CAN_UPLOAD = 1600; // actual max is something like 1630-ish

int g_tme = 1;

int z_scale_factor = 1;

int MAX_MESHES = 10;
int g_front_clip = false;
int g_back_clip = false;
int g_left_clip = true;
int g_right_clip = true;
int g_top_clip = true;
int g_bottom_clip = true;
int g_render_clipped = false;
int g_no_lighting = false;
int g_ztest_method = 3;

int TOP_BOUND = 0x5ff0;
int LEFT_BOUND = 0x5ff0;
int RIGHT_BOUND = 0x9ff0;
int BOTTOM_BOUND = 0x9ff0;

// Send a list of vertices to be interpreted as individual triangles.
void hw_rasta::send_indexed_vertex_list( const vert_buf& buf,
                                   int num_verts, 
                                   const unsigned short* indices, 
                                   int num_indices,
                                   unsigned flags )
{

}


// Send a list of vertices to be interpreted as individual triangles.
void hw_rasta::send_indexed_vertex_list( const vert_buf_xformed& buf,
                                   int num_verts, 
                                   const unsigned short* indices, 
                                   int num_indices,
                                   unsigned flags )
{
  // one of these is the important one
}
  
// Send a list of vertices to be interpreted as individual triangles.
void hw_rasta::send_indexed_vertex_strip( const vert_buf& buf,
                                   int num_verts, 
                                   const unsigned short* indices, 
                                   int num_indices,
                                   unsigned flags )
{
}
    
// Send a list of vertices to be interpreted as individual triangles.
void hw_rasta::send_indexed_vertex_strip( const vert_buf_xformed& buf,
                                   int num_verts, 
                                   const unsigned short* indices, 
                                   int num_indices,
                                   unsigned flags )
{
}
  


// Set the vertex context for future triangles.
void hw_rasta::send_context( vertex_context & , unsigned , color32 )
{
}

int frame = 0, odev = 0;
bool use_half_offset = false;

void hw_rasta::send_texture( hw_texture* texture, int stage )
{

}

void hw_rasta::end_scene()
{
}

// polygon counting mechanism
void hw_rasta::reset_poly_count() 
{ 
}
  
int hw_rasta::get_poly_count()    
{ 
  return 0;
}




void hw_rasta::flip()
{
	// empty
}


void hw_rasta::begin_scene()
{
}

//=====================================================================================
void hw_rasta::print(const stringx& str, const vector2di &xy, color32 rgba)
{
  unsigned int color = ((unsigned int)rgba.get_red() << 24) | ((unsigned int)rgba.get_green() << 16) | ((unsigned int)rgba.get_blue() << 8) | (unsigned int)rgba.get_alpha();
  
/*	Replaced by new API. (dc 05/30/02)
  KSNGL_SetFont( LOW_LEVEL_CONSOLE_FONT_ID );
  KSNGL_SetFontColor( color );
  KSNGL_SetFontZ( 0 );
  KSNGL_SetFontScale( 1.0f, 2.0f );
*/
  nglListAddString( nglSysFont, xy.x, (float)xy.y * ((float)nglGetScreenHeight()/480.0f), 
	  0, color, "\3" "[1, 2]" "%s", (char*)str.c_str() );
} 

void hw_rasta::set_cop(float x, float y, float min_z, float max_z)
{
}

void hw_rasta::enable_fog( bool on )
{
}
  
void hw_rasta::set_fog_color( const color & c )
{
}
  
void hw_rasta::set_fog_dist( float dist, float dist2 )
{
}

void hw_rasta::set_fog_table_gamma( rational_t g )
{
}

////////////////////////////////////////////////////////////////////////////////
//  vertex_context
#include "osdevopts.h"
vertex_context::vertex_context()
{
  texture  = NULL;
  if (os_developer_options::inst()->is_flagged( os_developer_options::FLAG_POINT_SAMPLE ) )
    filter_mode = POINT_SAMPLE;
  else
    filter_mode = BILINEAR;
  clamp_mode  = NOCLAMP;
  flip_mode   = NOFLIP;
  src_alpha   = SRCALPHA;
  dst_alpha   = INVSRCALPHA;
  shade_mode  = GOURAUD;
  cull_mode   = CCW;

  fog     = false;
  changed = true;
  punchthrough = false;
}

vertex_context::~vertex_context() 
{
}

void vertex_context::process()
{
  // code me
} 


vert_buf::vert_buf()
{
  verts = NULL;
  max_size = 0;
  locked = false;
  optimized = false;
}

vert_buf::vert_buf(int _max_size)
{
  max_size = _max_size;
  if (!max_size)
    verts = NULL;
  else
    verts = new hw_rasta_vert[max_size];

  locked = false;
  optimized = false;
}

vert_buf::~vert_buf()
{
  deconstruct();
}

void vert_buf::deconstruct()
{
  assert(!locked);
  if (verts)
  {
    delete[] verts;
    verts = 0;
  }
}

void vert_buf::lock(int _max_size, lock_type_t type)   
{ 
  assert(!locked);

  if (_max_size>=0) // pass in -1 to prevent clearing of existing verts
  {
    if (_max_size > max_size)
    {
      if (verts)
        delete[] verts;
      max_size = _max_size;
      if (!max_size)
        verts = NULL;
      else
        verts = new hw_rasta_vert[max_size];
    }
  }
  locked = true; 
}

void vert_buf::unlock() 
{
  assert(locked); 
  locked = false; 
}

void vert_buf::optimize()
{
  assert(!locked);
  assert(!optimized);
  optimized=true;
}


vert_buf_xformed::vert_buf_xformed()
{
  verts = NULL;
  max_size = 0;
  locked = false;
}

vert_buf_xformed::vert_buf_xformed(int _max_size)
{
  max_size = _max_size;
  if (!max_size)
    verts = NULL;
  else
    verts = new hw_rasta_vert_xformed[max_size];
  locked = false;
}

vert_buf_xformed::~vert_buf_xformed()
{
  deconstruct();
}

void vert_buf_xformed::deconstruct()
{
  assert(!locked);
  if (verts)
  {
    delete[] verts;
    verts = 0;
  }
}

void vert_buf_xformed::lock(int _max_size, lock_type_t type)   
{ 
  assert(!locked);
  if (_max_size>=0) // pass in -1 to prevent clearing of existing verts
  {
    if (_max_size > max_size)
    {
      if (verts)
        delete[] verts;
      max_size = _max_size;
      if (!max_size)
        verts = NULL;
      else
        verts = new hw_rasta_vert_xformed[max_size];
    }
  }
  locked = true; 
}

void vert_buf_xformed::unlock() 
{
  assert(locked); 
  locked = false; 
}

void hw_rasta::clip_send_indexed_vertex_list( hw_rasta_vert* verts, 
                                               int num_verts, 
                                               const unsigned short* indices, 
                                               int num_indices )
{

}                    
