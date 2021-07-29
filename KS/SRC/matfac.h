#ifndef MATFAC_H
#define MATFAC_H
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    mat_fac

  a platform independent facade to the material class that lets you load ngl materials from text file
  chunks ArchEngine style
*/
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "global.h"
#include "color.h"
#include "map_e.h"

class material;
class color32;
class aggregate_vert_buf;
//class hw_texture;
#include "hwrasterize.h"

class mat_fac
{
public:
  mat_fac();
  mat_fac( const mat_fac& b);

  // why is this virtual?  this class isn't polymorphic.  4 extra bytes for nothin'  jdf 4/1/01
  virtual ~mat_fac();

  mat_fac& operator=( const mat_fac& b);

  int  get_anim_length() const;

  bool is_translucent() const;

  void send_context( int frame,
    map_e map = MAP_DIFFUSE,
    unsigned int force_flags = 0,
    color32 force_color = color32(0xffffffff));

  bool has_texture() const;

  void load_material(const stringx &file);

  unsigned get_blend_mode( int m ) const;
  void set_blend_mode( unsigned _mode, int m );

  int  get_original_width( int frame, map_e map = MAP_DIFFUSE ) const;
  int  get_original_height( int frame, map_e map = MAP_DIFFUSE ) const;
    
#ifdef NGL
  nglMaterial* get_ngl_material() { return &m_nglmat; }
#else
  // PC only function
  aggregate_vert_buf* find_mat_buf( int last_frame, unsigned force_flags );

  material* get_material()      { return( m_pcmat ); }

  // get ID of the given texture map
  hw_texture* get_texture( int frame, map_e map = MAP_DIFFUSE ) const;
#endif

  friend void serial_in( chunk_file& io,
    mat_fac* m,
    const stringx& texture_dir,
    unsigned additional_flags );

private:
#ifdef NGL
  nglMaterial m_nglmat;
#else
  material* m_pcmat;
#endif
};

#endif
