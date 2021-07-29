/*-------------------------------------------------------------------------------------------------------

  XB_RASTERIZE.CPP - Kamui2 hw_rasta implementation

-------------------------------------------------------------------------------------------------------*/
#include "global.h"

#include "osalloc.h"

#include "xb_rasterize.h"
#include "xb_texturemgr.h"

#include "color.h"
#include <string>
#include <algorithm>

#include "stringx.h"
#include "txtcoord.h"
#include "algebra.h"
#include "singleton.h"
#include "refptr.h"

#include <stdio.h>






#define LOW_LEVEL_CONSOLE_HEIGHT   16
#define LOW_LEVEL_CONSOLE_WIDTH   40
#define LOW_LEVEL_CONSOLE_COLOR   0x80808080 // rgba, 0x80 is 1.0f
#define LOW_LEVEL_CONSOLE_Z_DEPTH 1.0f
#define LOW_LEVEL_CONSOLE_X_SCALE 2.0f
#define LOW_LEVEL_CONSOLE_Y_SCALE 2.0f


static char llc_lines[LOW_LEVEL_CONSOLE_HEIGHT][LOW_LEVEL_CONSOLE_WIDTH];

static char llc_start = -1;
static char llc_end = -1;
static char llc_count = 0;
static bool llc_available = false;

void low_level_console_init()
{
  llc_available = true;
}

void low_level_console_release()
{
  llc_available = false;
}

bool low_level_console_is_available()
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
#ifdef DEBUG	// Wouldn't want these to sneak in to final build.  (dc 03/27/02)
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
    nglListAddString( nglSysFont, 15, y, LOW_LEVEL_CONSOLE_Z_DEPTH, 
		NGL_RGBA32(
			(LOW_LEVEL_CONSOLE_COLOR && 0x00ff0000) >> 16, 
			(LOW_LEVEL_CONSOLE_COLOR && 0x0000ff00) >> 8, 
			(LOW_LEVEL_CONSOLE_COLOR && 0x000000ff) >> 0, 
			(LOW_LEVEL_CONSOLE_COLOR && 0xff000000) >> 24
		), 
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
  STUB( "llc_memory_log()");
}

DEFINE_SINGLETON(hw_rasta)

enum {
  HWOSXB_DEFAULT_SCREEN_WIDTH = 640,
  HWOSXB_DEFAULT_SCREEN_HEIGHT= 480,
  HWOSXB_DEFAULT_MAX_TEXTURES = 512,
};

void hw_rasta::init_xb_gfx()
{
#ifdef DEBUG
  static bool xb_gfx_are_initialized = false;
  assert (xb_gfx_are_initialized == false);
  xb_gfx_are_initialized = true;
#endif

  screen_width = HWOSXB_DEFAULT_SCREEN_WIDTH;
  screen_height = HWOSXB_DEFAULT_SCREEN_HEIGHT;
  
  pixel_depth = 32;

#ifdef PROJECT_KELLYSLATER
  nglSetPerspectiveMatrix( 90.0f, screen_width/2, screen_height/2, 0.2f, 65536.0f );

#else
  nglSetPerspectiveMatrix( 90.0f, nglGetScreenWidth()/2, nglGetScreenHeight()/2, PROJ_NEAR_PLANE_D, PROJ_FAR_PLANE_D );
#endif
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
  return nglGetScreenWidth(); //HWOSXB_DEFAULT_SCREEN_WIDTH;
}

int hw_rasta::get_screen_height() const
{
  return nglGetScreenHeight(); //HWOSXB_DEFAULT_SCREEN_HEIGHT;
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
  STUB( "hw_rasta::send_indexed_vertex_list" );
}

void hw_rasta::send_indexed_vertex_list( const vert_lit_buf& buf,
                                   int num_verts,
                                   const unsigned short* indices,
                                   int num_indices,
                                   unsigned flags )
{
  STUB( "hw_rasta::send_indexed_vertex_list" );
}


// Send a list of vertices to be interpreted as individual triangles.
void hw_rasta::send_indexed_vertex_list( const vert_buf_xformed& buf,
                                   int num_verts,
                                   const unsigned short* indices,
                                   int num_indices,
                                   unsigned flags )
{
  // one of these is the important one
  STUB( "hw_rasta::send_indexed_vertex_list" );
}

// Send a list of vertices to be interpreted as individual triangles.
void hw_rasta::send_indexed_vertex_strip( const vert_buf& buf,
                                   int num_verts,
                                   const unsigned short* indices,
                                   int num_indices,
                                   unsigned flags )
{
  STUB( "hw_rasta::send_indexed_vertex_strip" );
}

// Send a list of vertices to be interpreted as individual triangles.
void hw_rasta::send_indexed_vertex_strip( const vert_buf_xformed& buf,
                                   int num_verts,
                                   const unsigned short* indices,
                                   int num_indices,
                                   unsigned flags )
{
  STUB( "hw_rasta::send_indexed_vertex_strip" );
}



// Set the vertex context for future triangles.
void hw_rasta::send_context( vertex_context & , unsigned , color32 )
{
  STUB( "hw_rasta::send_context" );
}

int frame = 0, odev = 0;
bool use_half_offset = false;

void hw_rasta::send_texture( hw_texture* texture, int stage )
{
  STUB( "hw_rasta::send_texture" );
}

void hw_rasta::end_scene()
{
  STUB( "hw_rasta::end_scene" );
}

// polygon counting mechanism
void hw_rasta::reset_poly_count()
{
  STUB( "hw_rasta::reset_poly_count" );
}

int hw_rasta::get_poly_count()
{
//  STUB( "hw_rasta::get_poly_count" );	// this message got too annoying (dc 05/04/02)
  return 0;
}




void hw_rasta::flip()
{
  nglFlip();
}


void hw_rasta::begin_scene()
{
  STUB( "hw_rasta::begin_scene" );
}

//=====================================================================================
void hw_rasta::print(const stringx& str, const vector2di &xy, color32 rgba)
{
  nglListAddString( nglSysFont, xy.x, xy.y, 0, NGL_RGBA32(rgba.get_red(), rgba.get_green(), rgba.get_blue(), rgba.get_alpha()), (char*)str.c_str() );
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
    verts = NEW hw_rasta_vert[max_size];

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
        verts = NEW hw_rasta_vert[max_size];
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












vert_lit_buf::vert_lit_buf()
{
  verts = NULL;
  max_size = 0;
  locked = false;
  optimized = false;
}

vert_lit_buf::vert_lit_buf(int _max_size)
{
  max_size = _max_size;
  if (!max_size)
    verts = NULL;
  else
    verts = NEW hw_rasta_vert_lit[max_size];

  locked = false;
  optimized = false;
}

vert_lit_buf::~vert_lit_buf()
{
  deconstruct();
}

void vert_lit_buf::deconstruct()
{
  assert(!locked);
  if (verts)
  {
    delete[] verts;
    verts = 0;
  }
}

void vert_lit_buf::lock(int _max_size, lock_type_t type)
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
        verts = NEW hw_rasta_vert_lit[max_size];
    }
  }
  locked = true;
}

void vert_lit_buf::unlock()
{
  assert(locked);
  locked = false;
}

void vert_lit_buf::optimize()
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
    verts = NEW hw_rasta_vert_xformed[max_size];
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
        verts = NEW hw_rasta_vert_xformed[max_size];
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
#if 0

  // allocating statically to ensure 32-byte alignment:  maybe an align directive would be happier
  static hw_rasta_vert* target_v = (hw_rasta_vert*)os_malloc32x(sizeof(hw_rasta_vert)*2);
  static hw_rasta_vert* target_v2 = target_v+1;

  // the various prefetches in the following code won't generate exceptions if they
  // go past the end of the array.

  const unsigned short* indices_end = indices+num_indices;
  hw_rasta_vert* temp_vert[3];
  hw_rasta_vert* next_vert[3];
  for( int i = 0; i<3; ++i)
  {
    // for each vert, if rhw is less than zero, we need to clip
    next_vert[i] =  verts + indices[i];
  }

  for ( ; indices!=indices_end; )
  {
    int num_front_clips = 0;
    int not_clipped=-1;
    int clipped=-1;
    for( int i = 0; i<3; ++i)
    {
      // for each vert, if rhw is less than zero, we need to clip
      hw_rasta_vert* vert_ptr = next_vert[i];
      temp_vert[i] = next_vert[i];

      not_clipped=i;
    }
    if(num_front_clips==3)  // this shouldn't happen but...
    {
    }
    else
    {
      if(num_front_clips==0)
      {
        // prefetch for next iteration:
        next_vert[0] = verts + indices[3];

        // this backface "optimization" only got us a third of a millisecond on firelake...
        float xw0, xw1, xw2, yw0, yw1, yw2;
        xw0 = temp_vert[0]->xyz.x;
        yw0 = temp_vert[0]->xyz.y;
        xw1 = temp_vert[1]->xyz.x;

        // prefetch for next iteration:
        next_vert[1] = verts + indices[4];

        yw1 = temp_vert[1]->xyz.y;
        xw2 = temp_vert[2]->xyz.x;
        yw2 = temp_vert[2]->xyz.y;
        // backface cull:  if anybody knows how to do this for tris that cross the front clip plane,
        // let me know.

        // prefetch for next iteration:
        next_vert[2] = verts + indices[5];

        rational_t v1x = xw1 - xw0;
        rational_t v1y = yw1 - yw0;
        rational_t v2x = xw2 - xw1;
        rational_t v2y = yw2 - yw1;
        if (v1x*v2y-v2x*v1y>0)
        {
          hw_rasta_vert* vert_ptr = temp_vert[0];
          vert_ptr->PCW=KM_VERTEXPARAM_NORMAL;
          new_set_vertex(  vert_ptr );

          vert_ptr = temp_vert[1];
          vert_ptr->PCW=KM_VERTEXPARAM_NORMAL;
          new_set_vertex(  vert_ptr );

          vert_ptr = temp_vert[2];
          vert_ptr->PCW=KM_VERTEXPARAM_ENDOFSTRIP;
          new_set_vertex(  vert_ptr );
        }
        }
      else if(num_front_clips==2)
      {
        for( int i=0;i<3;++i )
        {

          hw_rasta_vert* this_vert;
          if( i==not_clipped )
          {
            this_vert = temp_vert[i];
          }
          else
          {
            hw_rasta_vert* source_v = temp_vert[not_clipped];
            hw_rasta_vert* clip_to_v = temp_vert[i];

            CLIP_VERT( (*target_v), source_v, clip_to_v );

            this_vert = target_v;
          }
          this_vert->PCW = (i==2)?KM_VERTEXPARAM_ENDOFSTRIP:KM_VERTEXPARAM_NORMAL;
          new_set_vertex(  this_vert );
        }
      }
      else // 1 clips, send a strip, very exciting.
      {
        hw_rasta_vert* unclipped_v[2];
        int unclipped_v_idx = 0;

        // prefetches for next iteration:

        for( int i=0;i<3;++i )
        {
          hw_rasta_vert* this_vert;
          if( i != clipped )
          {
            unclipped_v[ unclipped_v_idx ] = temp_vert[i];
            ++unclipped_v_idx;
          }
          else
          {
            // send two verts:  we can optimize by unrolling CLIP_VERT and only evaluating clip_to_v w once
            hw_rasta_vert* source_v = temp_vert[(clipped+2)%3];  // previous clippage
            hw_rasta_vert* clip_to_v = temp_vert[(clipped)];

            CLIP_VERT( (*target_v), source_v, clip_to_v );
            this_vert = target_v;

            source_v = temp_vert[(clipped+1)%3];
            CLIP_VERT( (*target_v2), source_v, clip_to_v );     // subsequent clippage
          }
        }

        // prefetches for next iteration:

        switch( clipped )
        {
          case 0:
            target_v2->PCW = KM_VERTEXPARAM_NORMAL;
            new_set_vertex(  target_v2 );

            unclipped_v[0]->PCW = KM_VERTEXPARAM_NORMAL;
            new_set_vertex(  unclipped_v[0] );

            // prefetches for next iteration:

            target_v->PCW = KM_VERTEXPARAM_NORMAL;
            new_set_vertex(  target_v );

            unclipped_v[1]->PCW = KM_VERTEXPARAM_ENDOFSTRIP;
            new_set_vertex(  unclipped_v[1] );
            break;
          case 1:
            target_v->PCW = KM_VERTEXPARAM_NORMAL;
            new_set_vertex(  target_v );

            target_v2->PCW = KM_VERTEXPARAM_NORMAL;
            new_set_vertex(  target_v2 );

            // prefetches for next iteration:

            unclipped_v[0]->PCW = KM_VERTEXPARAM_NORMAL;
            new_set_vertex(  unclipped_v[0] );

            unclipped_v[1]->PCW = KM_VERTEXPARAM_ENDOFSTRIP;
            new_set_vertex(  unclipped_v[1] );
            break;
          case 2:
            unclipped_v[0]->PCW = KM_VERTEXPARAM_NORMAL;
            new_set_vertex(  unclipped_v[0] );

            unclipped_v[1]->PCW = KM_VERTEXPARAM_NORMAL;
            new_set_vertex(  unclipped_v[1] );

              // prefetches for next iteration:

            target_v2->PCW = KM_VERTEXPARAM_NORMAL;
            new_set_vertex(  target_v2 ) ;

            target_v->PCW = KM_VERTEXPARAM_ENDOFSTRIP;
            new_set_vertex(  target_v ) ;

            break;
        }
      }
    }
    // waste to try to prefetch
    next_vert[0] = verts + indices[3];
    next_vert[1] = verts + indices[4];
    next_vert[2] = verts + indices[5];
    indices += 3;
  }
#endif
}

