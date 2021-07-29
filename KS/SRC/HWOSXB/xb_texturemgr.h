#ifndef XB_TEXTUREMGR_H
#define XB_TEXTUREMGR_H
/*-------------------------------------------------------------------------------------------------------

  XB implementation of the hw_texture manager.

-------------------------------------------------------------------------------------------------------*/
#include "singleton.h"
//#include "version.h"
#include "stringx.h"
#include "refptr.h"
#include "types.h"
#include "xb_graphics.h"

// JIV FIXME: fix malloc/free defs
//#undef free
//#undef malloc
#include "ngl.h"

class os_file;



// Texel bit organization.
enum texture_format_t
{
  TEX_ARGB1555 = 0,
  TEX_RGB565,
  TEX_ARGB4444,
  TEX_PAL8 = 6,
  TEX_RGB888,
  TEX_ARGB8888,
  INVALID_TEXTURE_FORMAT,
  NUM_TEXTURE_FORMATS = INVALID_TEXTURE_FORMAT   
};

class hw_texture : public ref
{
public:
  hw_texture() 
    : users( 0 ), width( 0 ), height( 0 ), format( 0 ) 
  {
    data = NULL;
    data_size = 0;
#ifndef PRE_WADE_GFX
    xb_texture = NULL;
#endif
  }
  virtual ~hw_texture()
  {
    if ((data != NULL) && (data_size > 0))
      free(data);
  }

  inline bool is_valid()    const 
#ifdef PRE_WADE_GFX
  { return (data != NULL) && (data_size > 0); }
#else
  { return (xb_texture); }
#endif
  inline bool is_loaded()   const { return is_valid(); }
  inline const stringx& get_name() const { assert( is_loaded() ); return name; }
  inline int get_width()    const { assert( is_loaded() ); return width; }
  inline int get_height()   const { assert( is_loaded() ); return height; }

	inline int get_original_width()  const { assert(is_loaded()); return original_width; }   // original height before any compatibility scaling
	inline int get_original_height() const { assert(is_loaded()); return original_height; }  // original width before any compatibility scaling
	
  inline int get_format() const { assert( is_loaded() ); return format; }
	inline bool is_translucent() const { return (format==TEX_ARGB8888)||(format==TEX_ARGB1555)||(format==TEX_ARGB4444); }

  void unload();

protected:
  stringx name;
  uint32 users;         // track how many people are using this texture
  uint32 width, height;
  uint32 original_width, original_height;
	uint32 width_bit, height_bit;	// 2 ^ n = Texture dimensions.
  int format;         
  uint32 sce_format;    // sony's texture format type

  uint32 addr;			    // VRAM address.

	uint32  data_size;		// Size of texture data.
	uint32* data;		      // RAM address.
#ifndef PRE_WADE_GFX
  nglTexture *xb_texture;
#endif

	// Load this texture from a file.
  bool load( const stringx & _name );
  bool load_raw_tga(os_file &the_file);
  
  friend class hw_texture_mgr;
  friend class hw_rasta;
  friend class vertex_context;
  friend void save_entire_state();
  friend void load_entire_state();
};

class hw_texture_mgr : public singleton
{
public:
  hw_texture_mgr();
  ~hw_texture_mgr();
  DECLARE_SINGLETON(hw_texture_mgr)

  // Automatically determines hw_texture format, file extensions.
  hw_texture* load_texture( const stringx & file_name );

  // checks for duplicates
  hw_texture* texture_loaded(const stringx & fname);

  // Release the surface memory
  void unload_all_textures();

  int get_memory_use() { return 0; }

  typedef vector<hw_texture *> hw_texture_list_t;
  typedef vector<stringx> hw_texture_name_list_t;

  const hw_texture_list_t& get_texture_list() const { return texture_list; }

  void set_white_texture(hw_texture* t) { white_texture = t; }
  hw_texture* get_white_texture() const { return white_texture; }
  
private:
  hw_texture_list_t texture_list;
  hw_texture_name_list_t name_list;  

  hw_texture* white_texture;
  
  friend class vertex_context;
  friend class hw_rasta;
  friend class hw_texture;
  friend void save_entire_state();
  friend void load_entire_state();
};

#endif
