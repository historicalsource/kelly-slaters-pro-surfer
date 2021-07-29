#ifndef GC_RASTERIZE_H
#define GC_RASTERIZE_H

#include "txtcoord.h"
#include "stringx.h"
#include "color.h"
#include "gc_texturemgr.h"
#include "osalloc.h"
#include "algebra.h"

enum 
{ 
  TEXTURE_COORD_DIFFUSE,   // Main texture
  TEXTURE_COORD_DECAL,     // decal or light map
//  TEXTURE_PASS_DYNAMIC,   // detail and environment
  MAX_TEXTURE_COORDS 
};

#include "maxskinbones.h"

class geometry_manager;

// Vertex formats.
// this one is just for our use for storing models, it will never be sent to D3D
class hw_rasta_vert
{
public:
  hw_rasta_vert() {}

  ////////////////////////////////////////////////////////////////////////////////
  // untransformed constructors
  hw_rasta_vert(const vector3d& _p, color32 _diffuse, float u, float v)
    : xyz(_p), diffuse(_diffuse)
  {
    bone_ids[0] = 0;
    tc[0].x = u;
    tc[0].y = v;
  }

  // compatibility version with different param order.
  hw_rasta_vert(const vector3d& _p, const texture_coord& _tc, color32 _diffuse )
    : xyz(_p), diffuse(_diffuse)
  {
    bone_ids[0] = 0;
    tc[0] = _tc;
  }

  hw_rasta_vert(const vector3d& _p, color32 _diffuse, uint8 bone_idx, float _u, float _v)
    : xyz(_p), diffuse(_diffuse)
  {
    bone_ids[0] = bone_idx;
    tc[0] = vector2d(_u,_v);
  }

  ////////////////////////////////////////////////////////////////////////////////
  // there are no transformed constructors yet

  // clear out stuff that isn't normally overwritten but needs cleared
  void reset() { /*bone_id = 0;*/ }

  vector3d xyz; // vertex coordinate

        vector3d& get_unxform()       { return xyz; }
  const vector3d& get_unxform() const { return xyz; }

  inline void set_xyz(const vector3d& p)
  {
    xyz=p; 
  }

  unsigned char clip_flags;
  signed char normalx,normaly,normalz;

  void set_normal(const vector3d& v)
  {
    normalx = (signed char)(v.x*127.0F);
    normaly = (signed char)(v.y*127.0F);
    normalz = (signed char)(v.z*127.0F);
  }
  vector3d get_normal() const
  {
    return vector3d(normalx*(1.0F/127.0F),
                    normaly*(1.0F/127.0F),
                    normalz*(1.0F/127.0F));
  }

  color32 diffuse; // color

  int num_bones;
  unsigned char bone_ids[MAX_SKIN_BONES]; // used to be kept in specular.c.a
  float bone_weights[MAX_SKIN_BONES];

        unsigned char& boneid()       { return bone_ids[0]; }
  const unsigned char& boneid() const { return bone_ids[0]; }

  vector2d tc[ MAX_TEXTURE_COORDS ]; // texture coordinate
};

class hw_rasta_vert_lit
{
public:
  hw_rasta_vert_lit() {}

  ////////////////////////////////////////////////////////////////////////////////
  // untransformed constructors
  hw_rasta_vert_lit(const vector3d& _p, color32 _diffuse, float u, float v) 
    : xyz(_p), diffuse(_diffuse)
  {
    tc[ 0 ] = vector2d( u, v );
  }

  // compatibility version with different param order.
  hw_rasta_vert_lit(const vector3d& _p, const texture_coord& _tc, color32 _diffuse ) 
    : xyz(_p), diffuse(_diffuse)
  {
    tc[ 0 ] = _tc;
  }

  // clear out stuff that isn't normally overwritten but needs cleared
  void reset() {}

  vector3d xyz; // vertex coordinate
  
  float reserved;

        vector3d& get_unxform()       { return xyz; }
  const vector3d& get_unxform() const { return xyz; }

  inline void set_xyz(const vector3d& p)
  {
    xyz=p; 
  }

  color32 diffuse; // color
  color32 specular; // compat with D3D_LVERTEX

  // this is the simplest way I thought of to make sure verts match.
  // which is required.
  unsigned char bone_ids[MAX_SKIN_BONES]; // used to be kept in specular.c.a
  float bone_weights[MAX_SKIN_BONES];

  vector2d tc[ MAX_TEXTURE_COORDS ]; // texture coordinate
};

class hw_rasta_vert_xformed
{
public:
  hw_rasta_vert_xformed() {}

  hw_rasta_vert_xformed(const vector3d& _p, rational_t _rhw, texture_coord _tc, color32 _diffuse/*,
                color32 _specular = color32(0,0,0,0)*/ ) 
     : xyz(_p)
     , rhw(_rhw) 
     , diffuse(_diffuse) /*specular(_specular), */
  {
    tc[ 0 ] = _tc;
  }

  // clear out stuff that isn't normally overwritten but needs cleared
  void reset() {}

  vector3d xyz; // vertex coordinate
  
  float rhw;    // inverse Z

  const vector3d& get_unxform() const { return xyz; }
  inline void set_xyz_rhw(const vector3d& p, rational_t _rhw)
  {
    xyz=p; rhw=_rhw;
  }
  inline void set_xyz_rhw(const vector4d& p)
  {
    xyz.x=p.x; xyz.y=p.y; xyz.z=p.z; rhw=p.w;
  }
  inline void homogenize_xyzw(const vector4d& p)
  {
    if (!p.w)
      xyz.x=xyz.y=xyz.z=rhw=0.0f;
    else
    {
      rhw = 1.0f / p.w;
      xyz.x=p.x*rhw; xyz.y=p.y*rhw; xyz.z=p.z*rhw;  
    }
  }

  color32 diffuse; // color
  color32 specular; // compat with D3D_TLVERTEX
  // this is the simplest way I thought of to make sure verts match.
  // which is required.
  unsigned char bone_ids[MAX_SKIN_BONES]; // used to be kept in specular.c.a
  float bone_weights[MAX_SKIN_BONES];

  vector2d tc[ MAX_TEXTURE_COORDS ]; // texture coordinate
};

inline bool operator==( const hw_rasta_vert & lhs, const hw_rasta_vert & rhs )
{
  //return memcmp( &lhs, &rhs, sizeof( hw_rasta_vert ) );
  return &lhs < &rhs;
}

inline bool operator==( const hw_rasta_vert_lit & lhs, const hw_rasta_vert_lit & rhs )
{
  //return memcmp( &lhs, &rhs, sizeof( hw_rasta_vert_lit ) );
  return &lhs < &rhs;
}

inline bool operator==( const hw_rasta_vert_xformed & lhs, const hw_rasta_vert_xformed & rhs )
{
  //return memcmp( &lhs, &rhs, sizeof( hw_rasta_vert_xformed ) );
  return &lhs < &rhs;
}


  
// index into a pool of surface verts;  can be used for dealing with vertex_buffers
typedef unsigned short surface_vert_ref;


enum lock_type_t
{
  LOCK_CLEAR,
  LOCK_NOOVERWRITE,
  LOCK_OVERWRITE
};

class vert_buf : public ref
{
  vert_buf( const vert_buf& vb );
public:
  vert_buf(); // to construct a global vertex workspace type buffer
  vert_buf(int _max_size); // to construct a buffer of a particular size
	~vert_buf();

  void deconstruct();
  
  void lock(int _max_size, lock_type_t type=LOCK_CLEAR); // pass in -1 to prevent resizing (which clears verts)
  void unlock();

  void optimize();

  int get_max_size() const { return max_size; }

  hw_rasta_vert * begin() const { assert(locked); return verts; }
  hw_rasta_vert * end()   const { assert(locked); return verts + max_size; }

private:
  hw_rasta_vert * verts;
  int max_size;
  bool locked;
  bool optimized;
  friend class hw_rasta;
};

class vert_buf_xformed : public ref
{
  vert_buf_xformed( const vert_buf_xformed& vb );
public:
  vert_buf_xformed();
  vert_buf_xformed(int _max_size); // to construct a buffer of a particular size
	~vert_buf_xformed();

  void deconstruct();

  void lock(int _max_size, lock_type_t type=LOCK_CLEAR); // pass in -1 to prevent clearing of existing verts
  void unlock();

  int get_max_size() const { return max_size; }

  hw_rasta_vert_xformed * begin() const { assert(locked); return verts; }
  hw_rasta_vert_xformed * end()   const { assert(locked); return verts + max_size; }

private:
  hw_rasta_vert_xformed* verts;
  int max_size;
  bool locked;
  friend class hw_rasta;
};


// Context for rendering a vertex strip.  Context includes things like texture map, texture flipping and clamping,
// modifier volume support, filtering mode, shading (flat/gouraud), and alpha type for translucent polygons.
class vertex_context
{
public:
  void set_texture( hw_texture* tex )
  {
    assert(tex->is_loaded());
    texture = tex;
    changed = true;
  }

  void set_diffuse(color32 c) {}
  void set_vertex_colored(bool b) {}

  enum filter_mode_t
  {
    POINT_SAMPLE,
    BILINEAR,
    TRILINEAR,
    ANISOTROPIC
  };
  void set_tex_filter_mode( filter_mode_t mode )
  {
    filter_mode = mode;
    changed = true;
  }

  enum clamp_mode_t
  {
    NOCLAMP,
    CLAMP_V,
    CLAMP_U,
    CLAMP_UV
  };
  void set_tex_clamp_mode( clamp_mode_t mode )
  {
    clamp_mode = mode;
    changed = true;
  }

  enum flip_mode_t
  {
    NOFLIP,
    FLIP_V,
    FLIP_U,
    FLIP_UV
  };
  void set_tex_flip_mode( flip_mode_t mode )
  {
    flip_mode = mode;
    changed = true;
  }

  enum alpha_mode_t
  {
    DESTALPHA,
    DESTCOLOR,
    INVDESTALPHA,
    INVDESTCOLOR,
    INVSRCALPHA,
    INVSRCCOLOR,
    SRCALPHA,
    SRCCOLOR,
    ONE,
    ZERO
  };
  void set_src_alpha_mode( alpha_mode_t mode )
  {
    src_alpha = mode;
    changed = true;
  }

  void set_dst_alpha_mode( alpha_mode_t mode )
  {
    dst_alpha = mode;
    changed = true;
  }

	void set_punchthrough( bool puf )
	{
		punchthrough = puf;
    changed = true;
	}

  enum shade_mode_t
  {
    FLAT,
    GOURAUD
  };
  void set_shade_mode( shade_mode_t mode )
  {
    shade_mode = mode;
    changed = true;
  }

  void set_fog(bool flag)
	{
    fog=flag;
    changed = true;
	}
		
	void set_translucent(bool) // ignored on PC.  I don't like this method, it isn't PC-friendly.
	{ // We should keep track of blending mode based on alpha modes and such
	}

  void set_specular(bool specf)
  {
  // specular = specf;
  }

  enum cull_mode_t
	{
	  CNONE,	
    CCW,	  
	  CW
	};
	void set_cull_mode( cull_mode_t mode )
	{
		cull_mode = mode;
    changed = true;
	}

  vertex_context();
  ~vertex_context();

private:
  hw_texture*   texture;
  filter_mode_t filter_mode;
  clamp_mode_t  clamp_mode;
  flip_mode_t   flip_mode;
  alpha_mode_t  src_alpha;
  alpha_mode_t  dst_alpha;
  shade_mode_t  shade_mode;
  cull_mode_t   cull_mode;
  bool          fog;
  bool          changed;
  bool          punchthrough;
  // bool specular;

  void process();  // it will be processed automagically if changed

  friend class hw_rasta;
};


class hw_rasta : public singleton
{
public:
  DECLARE_SINGLETON(hw_rasta)
public:

  /*-------------------------------------------------------------------------------------------------------
    Screen control

      Call the set_* functions, and then call reset to change the display for them to take effect.
  -------------------------------------------------------------------------------------------------------*/
  // In windowed mode, the renderer output is always the same size - it gets scaled to the size of the window.
  // Change the video mode in windowed mode and reset to change the resolution the renderer outputs.
  enum video_mode_t
  {
    VM_320x240,
    VM_512x384,
    VM_640x480,
    VM_800x600
  };
  void set_next_video_mode( video_mode_t mode );
  video_mode_t get_video_mode() const;
  int get_screen_width() const;
  int get_screen_height() const;

  enum pal_refresh_t
  {
    PAL_50HZ,
    PAL_60HZ
  };

  pal_refresh_t pal_refresh;

  pal_refresh_t get_pal_refresh() const { return pal_refresh; }
  void set_pal_refresh( pal_refresh_t mode );

  enum bit_depth_t
  {
    BD_16,
    BD_24
  };
  void set_next_bit_depth( bit_depth_t depth );
  bit_depth_t get_bit_depth() const { return bit_depth; }

  // Reset the renderer, making all set_* calls take effect.
  void reset();

  /*-------------------------------------------------------------------------------------------------------
    Global scene parameters
  -------------------------------------------------------------------------------------------------------*/
  void enable_fog( bool on );
  void set_fog_color( const color & c );
  void set_fog_dist( float start_dist, float end_dist );
  void set_fog_table_gamma( rational_t g );
  rational_t get_fog_table_gamma() const { return fog_table_gamma; }
  // center of projection
  void set_cop(float x, float y, float min_z=0.0f, float max_z=1.0f);

  // Force fog to be re-calculated
  void invalidate_fog();


  /*-------------------------------------------------------------------------------------------------------
    Rendering

      Functions prefixed with send queue commands to the display.  Their operations are stored in a buffer
    and affect all send commands issued after them.

    WARNING:
      send_start and send_end must be called in the order specified in poly_type_t excluding
      PT_NONE.  Any other order will break.  Sections can be skipped.  The sequence can only be called
      once per render cycle.
  -------------------------------------------------------------------------------------------------------*/
  enum poly_type_t
  {
    PT_NONE,
    PT_OPAQUE_POLYS,          // Send these first.
    PT_TRANS_POLYS,           // Send these second.
    PT_OPAQUE_MODIFIERS,      // Send these third.
    PT_TRANS_MODIFIERS        // Send these fourth.
  };

  // Notify beginnings and endings of sets of polygons
  void send_start( poly_type_t type );
  void send_end();

  void set_zbuffering( bool enable_read, bool enable_write ) {}


  enum
  {
    SEND_VERT_NORMAL     = 0,
    SEND_VERT_FRONT_CLIP = 1, 
    SEND_VERT_SKIP_CLIP  = 2
    //SEND_VERT_DONT_TOUCH_PCW = 4  // this has meaning for MKS only
  };

  // Send a vertex buffer to be interpreted as a strip of triangles.
  void send_vertex_strip(const vert_buf& buf, int num_verts, unsigned flags=SEND_VERT_NORMAL);

  // Send a vertex buffer to be interpreted as a strip of triangles.
  void send_vertex_strip( const vert_buf_xformed& buf, int num_verts, 
                          unsigned flags=SEND_VERT_NORMAL );

  // Send a vertex buffer to be interpreted as individual triangles.
  void send_vertex_list( const vert_buf_xformed& buf, int num_verts, 
                         unsigned flags=SEND_VERT_NORMAL );

  // Send a vertex buffer to be interpreted as individual triangles.
  void send_vertex_list(const vert_buf& buf, int num_verts, unsigned flags=SEND_VERT_NORMAL);

  // Send a list of vertices and a list of indices
  void send_indexed_vertex_list( const vert_buf_xformed& buf, int num_verts, 
                                 const unsigned short* indices, int num_indices,
                                 unsigned flags=SEND_VERT_NORMAL );

  // Send a list of vertices and a list of indices
  void send_indexed_vertex_strip( const vert_buf_xformed& buf, int num_verts, 
                                  const unsigned short* indices, int num_indices,
                                  unsigned flags=SEND_VERT_NORMAL );

  // Send a list of vertices and a list of indices
  void send_indexed_vertex_list(const vert_buf& verts,
                                int num_verts,
                                const unsigned short* indices,
                                int num_indices,
                                unsigned flags=SEND_VERT_NORMAL);

  void send_indexed_vertex_strip(const vert_buf& verts,
                                 int num_verts,
                                 const unsigned short* indices,
                                 int num_indices,
                                 unsigned flags=SEND_VERT_NORMAL);

private:

  // Clip and send a list of vertices to be interpreted as individual triangles.
  void clip_send_vertex_list(hw_rasta_vert* verts, int num_verts);

  // Clip and send a list of vertices and a list of indices
  void clip_send_indexed_vertex_list(hw_rasta_vert* verts,
                                 int num_verts,
                                 const unsigned short* indices,
                                 int num_indices);

public:
  // Set the vertex context for future triangles.
  void send_context( vertex_context & ctx, unsigned force_flags=0, color32 c=color32(0xfffffffff) );

  // set the texture stages 
  void send_texture( hw_texture* texture, int stage=0 );

  // Call this before doing anything in a scene.
  void begin_scene();

  // Call this before calling render.
  void end_scene();

  // Begin asynchronously rendering all queued commands.
  //void render(); // moved into flip

  // Flip the screen.  This waits until rendering is finished and displays the rendered image.
  void flip();

  /*-------------------------------------------------------------------------------------------------------
    Profiling
  -------------------------------------------------------------------------------------------------------*/
  // polygon counting mechanism
  void reset_poly_count();
  int get_poly_count();

  // FIXME: TEMP
  void print(const stringx& str, const vector2di &xy, color32 clr=color32_white);

private:
  hw_rasta() { init_gc_gfx(); }

  void init_gc_gfx();
public:
  virtual ~hw_rasta() {}   // The destructor has to be public

private:
  const hw_rasta & operator=( const hw_rasta & );

  static void end_of_render_callback(void* param);
  void update_fog();

  // State variables
  poly_type_t   send_type;

  video_mode_t  video_mode;
  bit_depth_t   bit_depth;

  rational_t    fog_start_dist;
  rational_t    fog_end_dist;

  rational_t    old_fog_start_dist;
  rational_t    old_fog_end_dist;

  rational_t fog_table_gamma;
  rational_t old_fog_table_gamma;

  // Profiling vars
  int poly_count;

  // Queued states
  video_mode_t  next_video_mode;
  bit_depth_t   next_bit_depth;

  bool         in_scene;

  int          screen_width, screen_height;
  int          pixel_depth;

  friend class texture_mgr;
  friend class texture;
};

void low_level_console_init();
void low_level_console_release();
void low_level_console_print(char *Text, ...);
void low_level_console_flush();
void low_level_console_clear();
bool low_level_console_is_available();
void llc_memory_log();


#endif // GC_RASTERIZE_H
