/*-------------------------------------------------------------------------------------------------------

  PS2_RASTERIZE.CPP - Kamui2 hw_rasta implementation

-------------------------------------------------------------------------------------------------------*/
#include "global.h"

#include "osalloc.h"

#include "ps2_rasterize.h"
#include "ps2_texturemgr.h"

#include "color.h"
#include <string>
#include <algorithm>

#include <libvu0.h>
#include <libvifpk.h>
#include <devfont.h>
#include "stringx.h"
#include "txtcoord.h"
#include "algebra.h"
#include "singleton.h"
#include "refptr.h"
#include "projconst.h"
#include "ksngl.h"	// For NGLFONT_TOKEN_SCALEXY (dc 05/30/02)

#include <stdio.h>






DEFINE_SINGLETON(hw_rasta)

enum {
//  HWOSPS2_DEFAULT_SCREEN_WIDTH = 128, //512,
//  HWOSPS2_DEFAULT_SCREEN_HEIGHT= 128, //448,
  HWOSPS2_DEFAULT_MAX_TEXTURES = 512,
};

static u_long128 PacketWorkspace[16384];

const __attribute__((aligned(32))) u_long giftag_tri[2] = { SCE_GIF_SET_TAG(0, 1, 0, 0, 0, 1), 0xEL };

__attribute__((aligned(32))) unsigned char PortScratchPad[1024];
#define SCRATCHPAD (uint32)( &PortScratchPad )


void hw_rasta::init_ps2_gfx()
{
#ifdef DEBUG
static bool ps2_gfx_are_initialized = false;
assert (ps2_gfx_are_initialized == false);
ps2_gfx_are_initialized = true;
#endif

  screen_width = nglGetScreenWidth(); //HWOSPS2_DEFAULT_SCREEN_WIDTH;
  screen_height = nglGetScreenHeight(); //HWOSPS2_DEFAULT_SCREEN_HEIGHT;
  pixel_depth = 32;

#ifdef PROJECT_KELLYSLATER
  nglSetPerspectiveMatrix( proj_field_of_view_in_degrees(), nglGetScreenWidth()/2, nglGetScreenHeight()/2, 0.2f, 65536.0f );
#else
  nglSetPerspectiveMatrix( 90.0f, nglGetScreenWidth()/2, nglGetScreenHeight()/2, PROJ_NEAR_PLANE_D, PROJ_FAR_PLANE_D );
#endif

  // get the GIF dma channel  (GIF is the GS interface)
  dmaGif = sceDmaGetChan(SCE_DMA_GIF);
  // enable Tag Transer
  dmaGif->chcr.TTE = 1;
	// get the VIF1 dma channel (VU1 interface)
	dmaVif = sceDmaGetChan(SCE_DMA_VIF1);
  dmaVif->chcr.TTE = 1;

  // use scratchpad for packet
  dma_data = (QWdata*)SCRATCHPAD;

//  low_level_console_init();

	// wait for odd field
  while(sceGsSyncV(0) == 0);
}

#define LOW_LEVEL_CONSOLE_FONT_ID  0
#define LOW_LEVEL_CONSOLE_HEIGHT   16
#define LOW_LEVEL_CONSOLE_WIDTH   40
#define LOW_LEVEL_CONSOLE_COLOR   0x80808080 // rgba, 0x80 is 1.0f
#define LOW_LEVEL_CONSOLE_Z_DEPTH 1.0f
#define LOW_LEVEL_CONSOLE_X_SCALE 1.0f
#define LOW_LEVEL_CONSOLE_Y_SCALE 1.0f


#ifdef DEBUG
static char llc_lines[LOW_LEVEL_CONSOLE_HEIGHT][LOW_LEVEL_CONSOLE_WIDTH];
#endif

static char llc_start = -1;
static char llc_end = -1;
static char llc_count = 0;
static bool llc_available = false;

void low_level_console_init()
{
/*	This font is now statically loaded with NGL (dc 12/20/01)
//#pragma fixme("This texture is a memory leak, it will be fixed with FileFinder 2.0 (GT--4/20/01)")
  nglSetTexturePath("");
  nglLoadFontA("Font8x12", LOW_LEVEL_CONSOLE_FONT_ID);
  nglSetClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
//  low_level_console_clear();
  nglFlip();
*/
  llc_available = true;
}

void low_level_console_release()
{
/*
  nglReleaseFont(LOW_LEVEL_CONSOLE_FONT_ID);
*/
  llc_available = false;
}

bool low_level_console_is_available()
{
  return llc_available;
}

void low_level_console_print(char *Text, ...)
{
#ifdef DEBUG	// Wouldn't want these to sneak in to final build.  (dc 03/27/02)
	//if (!llc_available) return;

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
  if (!llc_available) return;

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

int  mem_get_total_alloced(int heapid = SYSTEM_HEAP);
int mem_get_total_avail(int heapid = SYSTEM_HEAP);
int mem_get_largest_avail(int heapid = SYSTEM_HEAP);
int mem_get_total_mem(int heapindex = SYSTEM_HEAP);

void llc_memory_log()
{
	//int mem_get_total_alloced();
	//int mem_get_total_mem();
	//int mem_get_largest_avail();

  struct mallinfo info = mallinfo();
  nglSetClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

  //low_level_console_print("total %dK  used %dK  unused %dK", info.arena/1024, info.uordblks/1024, info.fordblks/1024);
  low_level_console_print("total %uK  used %uK  largest free %uK", mem_get_total_mem()/1024, mem_get_total_alloced()/1024, mem_get_largest_avail()/1024 );
  low_level_console_flush();
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
  return nglGetScreenWidth(); //HWOSPS2_DEFAULT_SCREEN_WIDTH;
}

int hw_rasta::get_screen_height() const
{
  return nglGetScreenHeight(); //HWOSPS2_DEFAULT_SCREEN_HEIGHT;
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

void reset_packet(sceVif1Packet &packet)
{
//    memset( PacketWorkspace, 0, sizeof(PacketWorkspace) );
  	sceVif1PkInit(&packet, (u_long128*)PacketWorkspace);

  	sceVif1PkReset(&packet);
  	sceVif1PkCnt(&packet, 0);
  	sceVif1PkOpenDirectCode(&packet, 0);

  	sceVif1PkOpenGifTag(&packet, *(u_long128*)&giftag_tri);

    // Dummy entry to prevent empty packets.
    sceVif1PkAddGsAD(&packet, SCE_GS_NOP, 0);
}

void send_packet(sceVif1Packet &packet)
{
  	sceVif1PkCloseGifTag(&packet);         // close GIF Tag

  	sceVif1PkCloseDirectCode(&packet);
  	sceVif1PkEnd(&packet, 0);
  	sceVif1PkTerminate(&packet);
  	FlushCache(0);
  	// DMA start
  	sceDmaChan *dmaVif = sceDmaGetChan(SCE_DMA_VIF1);
  	sceDmaSend(dmaVif,(u_int *)(((u_int)packet.pBase)));
  	sceGsSyncPath(0, 0);
}

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
#if 0       // -- internal compiler errors in the PS2 release build!! --
  hw_rasta_vert* verts = buf.verts;
//  assert( ((uint32)verts & 0x1f) == 0 );
  assert( (num_indices != 0) && (num_indices % 3 == 0) );

	sceGsSyncPath(0, 0);

// for now, let's not clip them
  if(( flags & SEND_VERT_FRONT_CLIP )&&( g_render_clipped ))
  {
      clip_send_indexed_vertex_list( verts, num_verts, indices, num_indices );
  }
  else
  {
    const unsigned short* indices_end = indices+num_indices;
    hw_rasta_vert* vert_ptr;

  	sceVif1Packet packet;
    reset_packet(packet);

    int x1, x2, x3;
    int y1, y2, y3;
    int z1, z2, z3;
    float u1, v1, q;
    int triangle_count = 0;

    while (indices!=indices_end)
    {
      x1 = (int)((verts + *(indices))->xyz.x * 16);
      y1 = (int)((verts + *(indices))->xyz.y * 16);
      z1 = (int)((((verts + *indices)->xyz.z) * z_scale_factor));

      x2 = (int)((verts + *(indices + 1))->xyz.x * 16);
      y2 = (int)((verts + *(indices + 1))->xyz.y * 16);
      z2 = (int)((((verts + *indices + 1)->xyz.z) * z_scale_factor));

      x3 = (int)((verts + *(indices + 2))->xyz.x * 16);
      y3 = (int)((verts + *(indices + 2))->xyz.y * 16);
      z3 = (int)((((verts + *indices + 2)->xyz.z) * z_scale_factor));

/*
      x1 = (int)(((verts + *(indices))->xyz.x - 320 + 2048) * 16);
	    y1 = (int)((((verts + *(indices))->xyz.y - 224) / 2 + 2048) * 16);
      z1 = (int)(((1.0f - (verts + *indices)->xyz.z) * z_scale_factor) * 16);

      x2 = (int)(((verts + *(indices + 1))->xyz.x - 320 + 2048) * 16);
      y2 = (int)((((verts + *(indices + 1))->xyz.y - 224) / 2 + 2048) * 16);
      z2 = (int)(((1.0f - (verts + *indices + 1)->xyz.z) * z_scale_factor) * 16);

      x3 = (int)(((verts + *(indices + 2))->xyz.x - 320 + 2048) * 16);
    	y3 = (int)((((verts + *(indices + 2))->xyz.y - 224) / 2 + 2048) * 16);
      z3 = (int)(((1.0f - (verts + *indices + 2)->xyz.z) * z_scale_factor) * 16);
*/
      if( g_front_clip )
         if ((z1 <= 0.0f || z2 <= 0.0f || z3 <= 0.0f) )
           {
             indices += 3;
             continue;
           }
      if( g_back_clip )
         if ((z1 >= z_scale_factor*16 ) || (z2 >= z_scale_factor*16) || (z3 >= z_scale_factor*16) )
           {
             indices += 3;
             continue;
           }
      if( g_left_clip )
         if( (x1 <= LEFT_BOUND) ||
           (x2 <= LEFT_BOUND) ||
           (x3 <= LEFT_BOUND) )
           {
             indices += 3;
             continue;
           }
      if( g_right_clip )
         if( (x1 >= RIGHT_BOUND) ||
             (x2 >= RIGHT_BOUND) ||
             (x3 >= RIGHT_BOUND) )
           {
             indices += 3;
             continue;
           }
      if( g_top_clip )
         if( (y1 <= TOP_BOUND) ||
             (y2 <= TOP_BOUND) ||
             (y3 <= TOP_BOUND) )
           {
             indices += 3;
             continue;
           }
      if( g_bottom_clip )
         if( (y1 >= BOTTOM_BOUND) ||
             (y2 >= BOTTOM_BOUND) ||
             (y3 >= BOTTOM_BOUND) )
         {
           indices += 3;
           continue;
         }

     	sceVif1PkAddGsAD(&packet, SCE_GS_PRIM, SCE_GS_SET_PRIM(0x04,1,g_tme,0,0,0,0,0,0));

      // try destructive after this...
      vert_ptr = verts + *indices++;

      u1 = vert_ptr->tc[0].x / vert_ptr->xyz.z;
      v1 = (1.0f - vert_ptr->tc[0].y) / vert_ptr->xyz.z;
      q = 1.0f/vert_ptr->xyz.z;
    	sceVif1PkAddGsAD(&packet, SCE_GS_ST, SCE_GS_SET_ST( *((int *)(&u1)), *((int *)(&v1)) ));
      if (g_no_lighting)
      {
    	  sceVif1PkAddGsAD(&packet, SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(255,
          255, 255, 255, *((int *)(&q)) ));
      }
      else
      {
    	  sceVif1PkAddGsAD(&packet, SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(vert_ptr->diffuse.get_red() / 2,
          vert_ptr->diffuse.get_green() / 2, vert_ptr->diffuse.get_blue() / 2, vert_ptr->diffuse.get_alpha(),
          *((int *)(&q)) ));
      }
    	sceVif1PkAddGsAD(&packet, SCE_GS_XYZ2, SCE_GS_SET_XYZ(x1, y1, z1));

      vert_ptr = verts + *indices++;
      u1 = vert_ptr->tc[0].x / vert_ptr->xyz.z;
      v1 = (1.0f - vert_ptr->tc[0].y) / vert_ptr->xyz.z;
      q = 1.0f/vert_ptr->xyz.z;
    	sceVif1PkAddGsAD(&packet, SCE_GS_ST, SCE_GS_SET_ST( *((int *)(&u1)), *((int *)(&v1)) ));
      if (g_no_lighting)
      {
    	  sceVif1PkAddGsAD(&packet, SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(255,
          255, 255, 255, *((int *)(&q)) ));
      }
      else
      {
    	  sceVif1PkAddGsAD(&packet, SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(vert_ptr->diffuse.get_red() / 2,
          vert_ptr->diffuse.get_green() / 2, vert_ptr->diffuse.get_blue() / 2, vert_ptr->diffuse.get_alpha(),
          *((int *)(&q)) ));
      }
  	  sceVif1PkAddGsAD(&packet, SCE_GS_XYZ2, SCE_GS_SET_XYZ(x2, y2, z2));

      vert_ptr = verts + *indices++;
      u1 = vert_ptr->tc[0].x / vert_ptr->xyz.z;
      v1 = (1.0f - vert_ptr->tc[0].y) / vert_ptr->xyz.z;
      q = 1.0f/vert_ptr->xyz.z;
    	sceVif1PkAddGsAD(&packet, SCE_GS_ST, SCE_GS_SET_ST( *((int *)(&u1)), *((int *)(&v1)) ));
      if (g_no_lighting)
      {
    	  sceVif1PkAddGsAD(&packet, SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(255,
          255, 255, 255, *((int *)(&q)) ));
      }
      else
      {
    	  sceVif1PkAddGsAD(&packet, SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(vert_ptr->diffuse.get_red() / 2,
          vert_ptr->diffuse.get_green() / 2, vert_ptr->diffuse.get_blue() / 2, vert_ptr->diffuse.get_alpha(),
          *((int *)(&q)) ));
      }
      sceVif1PkAddGsAD(&packet, SCE_GS_XYZ2, SCE_GS_SET_XYZ(x3, y3, z3));

      // we can only upload a certain number of triangles per packet.
      triangle_count++;
      if (triangle_count >= MAX_TRIANGLES_WE_CAN_UPLOAD)
      {
        triangle_count = 0;
        send_packet(packet);
        reset_packet(packet);
      }
    }
    if ( triangle_count )
      send_packet(packet);
  }
#endif
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

void hw_rasta::send_texture( hw_texture* texture, int stage = 0 )
{
#if 0
  if (texture && texture->ps2_texture)
  {
  	sceVif1Packet packet;
    reset_packet(packet);
    sceVif1PkAddGsAD(&packet, SCE_GS_TEX0_1, SCE_GS_SET_TEX0(texture->ps2_texture->Addr, texture->ps2_texture->Width/64,
      texture->ps2_texture->Format, texture->ps2_texture->WidthBit, texture->ps2_texture->HeightBit,1,0,0,0,0,0,0));

    sceVif1PkAddGsAD(&packet, SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0,0,1,1,0,0,0));
    sceVif1PkAddGsAD(&packet, SCE_GS_TEST_1, SCE_GS_SET_TEST(0,0,0,0,0,0,1,g_ztest_method));
    send_packet(packet);
  }
#endif
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
  nglFlip();
}


void hw_rasta::begin_scene()
{
}

//=====================================================================================
void hw_rasta::print(const stringx& str, const vector2di &xy, color32 rgba)
{
  unsigned int color = ((unsigned int)rgba.get_red() << 24) | ((unsigned int)rgba.get_green() << 16) | ((unsigned int)rgba.get_blue() << 8) | (unsigned int)rgba.get_alpha();

/*	Replaced by new API. (dc 05/30/02)
  KSNGL_SetFont( 0 );
  KSNGL_SetFontColor( color );
  KSNGL_SetFontZ( 0 );	// On top of everything
  KSNGL_SetFontScale( LOW_LEVEL_CONSOLE_X_SCALE, LOW_LEVEL_CONSOLE_Y_SCALE );
*/
  nglListAddString( nglSysFont, xy.x, xy.y, 
	  0, color, "%s", (char*)str.c_str() );
//  print_lines.push_back(print_node(str, xy, rgba));
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



#if 0

float  x1_f = 0.0f;
float  y1_f = 0.0f;
float  x2_f = 320.0f;
float  y2_f = 224.0f;

// texture test
if (g_do_texture_test)
{
  nglPrintf( "viewing texture %s at location %d, w=%d, h=%d, bw=%d, bh=%d.\n",
    texture->name.c_str(),
  	texture->addr,
		texture->width,
		texture->height,
		texture->width_bit,
		texture->height_bit );

	// Draw the test texture.
	sceDmaChan *dmaVif = sceDmaGetChan(SCE_DMA_VIF1);
    dmaVif->chcr.TTE = 1;

	sceVif1Packet packet;

  reset_packet(packet);

	sceVif1PkAddGsAD(&packet, SCE_GS_TEX0_1,
		SCE_GS_SET_TEX0(
			texture->addr, texture->width / 64, texture->sce_format,
			texture->width_bit, texture->height_bit, 1, 0, 0, 0, 0, 0, 0 ) );

	// Stretch nearest.
	sceVif1PkAddGsAD(&packet, SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0,0,0,0,0,0,0));

	// Z-always.
	sceVif1PkAddGsAD(&packet, SCE_GS_TEST_1, SCE_GS_SET_TEST(0,0,0,0,0,0,1,1));

//	sceVif1PkAddGsAD(&packet, SCE_GS_PRIM, SCE_GS_SET_PRIM(0x04,0,1,0,0,0,1,0,0));
  	sceVif1PkAddGsAD(&packet, SCE_GS_PRIM, SCE_GS_SET_PRIM(0x04,0,1,0,0,0,0,0,0));

  int x1, y1;
  int x2, y2;

  x1 = (x1_f - 320 + 2048) * 16;
  y1 = ((y1_f - 224) / 2 + 2048) * 16;
  x2 = (x2_f - 320 + 2048) * 16;
  y2 = ((y2_f - 224) / 2 + 2048) * 16;

/*	int x1 = (2048 - 280) * 16;
	int y1 = (2048 - 92) * 16;
	int x2 = (2048 - 280 + texture->width * 2) * 16;
	int y2 = (2048 - 92 + texture->height * 2 / 2) * 16;
*/
	int usize = texture->width * 16;
	int vsize = texture->height * 16;
float u1, v1, q;
int z = 1 << 4;

u1 = 0.0f;
v1 = 0.0f;
q = 1.0f;
 	sceVif1PkAddGsAD(&packet, SCE_GS_ST, SCE_GS_SET_ST( *((int *)(&u1)), *((int *)(&v1)) ));
//	sceVif1PkAddGsAD(&packet, SCE_GS_UV, SCE_GS_SET_UV(0, 0));
	sceVif1PkAddGsAD(&packet, SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, *((int *)(&q)) ));
	sceVif1PkAddGsAD(&packet, SCE_GS_XYZ2, SCE_GS_SET_XYZ(x1, y1, z));

u1 = 0.0f;
v1 = 1.0f;
 	sceVif1PkAddGsAD(&packet, SCE_GS_ST, SCE_GS_SET_ST( *((int *)(&u1)), *((int *)(&v1)) ));
//	sceVif1PkAddGsAD(&packet, SCE_GS_UV, SCE_GS_SET_UV(0, vsize));
	sceVif1PkAddGsAD(&packet, SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, *((int *)(&q)) ));
	sceVif1PkAddGsAD(&packet, SCE_GS_XYZ2, SCE_GS_SET_XYZ(x1, y2, z));

u1 = 1.0f;
v1 = 0.0f;
 	sceVif1PkAddGsAD(&packet, SCE_GS_ST, SCE_GS_SET_ST( *((int *)(&u1)), *((int *)(&v1)) ));
//	sceVif1PkAddGsAD(&packet, SCE_GS_UV, SCE_GS_SET_UV(usize, 0));
	sceVif1PkAddGsAD(&packet, SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, *((int *)(&q)) ));
	sceVif1PkAddGsAD(&packet, SCE_GS_XYZ2, SCE_GS_SET_XYZ(x2, y1, z));

u1 = 1.0f;
v1 = 1.0f;
 	sceVif1PkAddGsAD(&packet, SCE_GS_ST, SCE_GS_SET_ST( *((int *)(&u1)), *((int *)(&v1)) ));
//	sceVif1PkAddGsAD(&packet, SCE_GS_UV, SCE_GS_SET_UV(usize, vsize));
	sceVif1PkAddGsAD(&packet, SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, *((int *)(&q)) ));
	sceVif1PkAddGsAD(&packet, SCE_GS_XYZ2, SCE_GS_SET_XYZ(x2, y2, z));

  send_packet(packet);

	odev = !sceGsSyncV(0);

	/* Add half pixel to offset address for interlace */
	sceGsSetHalfOffset((odev&1)?(&double_buf.draw1):(&double_buf.draw0), 2048, 2048, use_half_offset?odev:0);

	// change display buffer
	FlushCache(0);
	sceGsSyncPath(0, 0);
	sceGsSwapDBuff(&double_buf, frame);
	frame++;
}
#endif
