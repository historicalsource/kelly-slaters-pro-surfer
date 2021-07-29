#ifndef MATERIAL_H
#define MATERIAL_H
////////////////////////////////////////////////////////////////////////////////
/*
  material.h
  the material a face is made of
  this is material in the standard 3d sense of the word, what 3d-studio
  means by material, what everything except DBTS I meant by material.

  (I used to be using the word 'surface' but that was confused by what
  directX meant by surface.)
*/
////////////////////////////////////////////////////////////////////////////////
#include "color.h"
#include "chunkfile.h"
#include "hwrasterize.h"
#include "instance.h"
#include "refptr.h"
#include "map_e.h"

////////////////////////////////////////////////////////////////////////////////
// material
////////////////////////////////////////////////////////////////////////////////

//class hw_texture;
#include "hwrasterize.h"


// FIXME:
// texture and material flags are mixed to maintain compatability with an exporter
// that was written back when there was only one texture to a material
enum
{
  TEX_ADDITIVE_ALPHA       = 0x01, // these must match the flags in the exporter!
  MAT_FULL_SELF_ILLUM      = 0x02,
  MAT_NO_FOG               = 0x04,
  MAT_ALLOW_ADDITIVE_LIGHT = 0x08,
  TEX_PUNCH_THROUGH        = 0x10, // alpha test > 50%
  TEX_CLAMP_U              = 0x20, // no repeat tiling horiz
  TEX_CLAMP_V              = 0x40, // no repeat tiling vert
  TEX_NO_VERTEX_COLOR      = 0x80, // don't use vertex color
  TEX_NO_MATERIAL_COLOR    = 0x100,// don't use material color
	MAT_BACKFACE_DEFAULT     = 0x200,
	MAT_NO_BACKFACE_CULL     = 0x400
};


// animated texture class
class anim_texture
{
	public:

		anim_texture() : tex_flags(0), diffuse_color( 255, 255, 255, 255 ) {}
    ~anim_texture();

/*
// these routines are redundant, and cause problems for poor
// fools who add a member to the class without changing them:
		anim_texture( const anim_texture & src )
		{
			filename        = src.filename;
			frame_list      = src.frame_list;
		}

		const anim_texture & operator=(const anim_texture & rhs)
		{
			filename        = rhs.filename;
			frame_list      = rhs.frame_list;
			return *this;
		}  */

		// load from a file.
		void load(const stringx & pathname);

		// get the texture at a certain time.
		hw_texture* get_texture(int frame) const;

    bool is_translucent() const;
    bool is_full_self_illum() const;

    unsigned get_flags() const { return tex_flags; }
    void set_flags(unsigned _flags) { tex_flags=_flags; }

    unsigned get_blend_mode() const { return blend_mode; }
    void set_blend_mode(unsigned _mode) { blend_mode=_mode; }

    float    get_blend_amount() const { return blend_amount; }
    void     set_blend_amount(float amt) { blend_amount=amt; }

    color32  get_diffuse_color() const { return diffuse_color; }
    void     set_diffuse_color(color32 c) { diffuse_color = c; }

    int get_anim_length() const;

    const stringx & get_filename() { return filename; }

    void process_vertex_contexts( unsigned mat_flags );

    void send_context(int frame,unsigned force_flags=0,color32 force_color=color32(0xffffffff));
    void send_texture(int frame,int texture_pass=0);

	private:
		stringx filename;
		vector<refptr<hw_texture> > frame_list;
    vector<vertex_context> vc_list;
    // keeping a vertex_context with each texture frame
    // is an important optimization because processing
    // the vertext context takes more time than setting,
    // and you can do the processing up front

    unsigned tex_flags;
    unsigned blend_mode;
    float    blend_amount;
    color32  diffuse_color;

    void process_ifl(const stringx &found_name);
};

//class color_and_mat_flag;

class material //: public ref // instance_bank is incompatible with ref/refptr
{
  public:

    // uninitialized material
        //material();
    material(const material &b );
    material( chunk_file& io, const stringx& texture_dir, unsigned additional_mat_flags, unsigned additional_tex_flags );
        //material(const stringx &filename, color src = color(0.5f, 0.5f, 0.5f, 0.0f), float srp = 1.0f, float sm = 1.0f, int f = 0);
	  material(const stringx &filename, unsigned additional_mat_flags=0, unsigned additional_tex_flags=0 );

    ~material();

    void set_defaults();

    int get_anim_length( int map = 0 ) const;

    bool is_translucent() const;
    bool is_full_self_illum() const { return mat_flags & MAT_FULL_SELF_ILLUM; }
    bool has_environment_map() const { return (texture_filename[MAP_ENVIRONMENT].length() > 0 ); }
	  bool has_light_map() const { return (texture_filename[MAP_DIFFUSE2].length() > 0); }
	  bool has_diffuse_map(int m) const { assert(m < MAPS_PER_MATERIAL); return (texture_filename[m].length() > 0); }
    const stringx& get_texture_filename( int tex=0 ) const { return texture_filename[ tex ]; }

    // send the context for this material to the renderer.
    // sets up all required states.
    // frame is a frame number for animated materials - the modulo is done internally.
		void send_context(int frame,
      map_e map = MAP_DIFFUSE,
      unsigned int force_flags = 0,
      color32 force_color = color32(0xffffffff));

    // get ID of the given texture map
    hw_texture* get_texture( int frame, map_e map = MAP_DIFFUSE ) const;

		rational_t get_specular_power() const { return 0.0f; }
		rational_t get_blend_amount(int m) const { assert(m < MAPS_PER_MATERIAL && has_diffuse_map(m)); return diffuse_map[m].get_blend_amount(); }
		rational_t get_environment_blend() const { return get_blend_amount(MAP_ENVIRONMENT); }

    color32 get_diffuse_color( int m = 0 ) const {
      assert( m < MAPS_PER_MATERIAL && m >= 0 );
      return diffuse_map[m].get_diffuse_color( );
    }

    void set_diffuse_color( color32 c, int m = 0 );

    unsigned get_blend_mode( int m = 0 ) const { return diffuse_map[ m ].get_blend_mode(); }
    void set_blend_mode( unsigned _mode, int m = 0 ) { diffuse_map[ m ].set_blend_mode( _mode ); }
    //color_and_mat_flag get_color_and_mat_flag( int m = 0 );
    //void set_color_and_mat_flag( const color_and_mat_flag& camatf, int m = 0 );

    const stringx& get_material_name() { return material_name; }

    float get_det_u_scale() const { return det_u_scale; }
    float get_det_v_scale() const { return det_v_scale; }
    float get_det_range() const { return det_range; }
    float get_det_alpha_clamp() const { return det_alpha_clamp; }

    // The instance_bank expects this function for getting the string by which this
    // instance is indexed.
    const stringx& get_instance_name() const { return material_name; }

		material &operator=(const material &b);    // operator= is invalid operation:  declaring but not defining
	  bool operator==(const material & src) const { return (material_name==src.material_name); }
	  bool operator!=(const material & src) const { return !(material_name==src.material_name); }

    static void flush_last_context();  // if you decide to do an end run to hw_rasta, you better remember to call this.

    void set_flags( unsigned _flags );
    unsigned get_flags() const;

    void process_vertex_contexts();

  private:
    void build_hash_name();
  public:
    stringx        material_name;
    stringx        texture_filename[MAPS_PER_MATERIAL]; // name of single texture or IFL file if animating.
		anim_texture     diffuse_map[MAPS_PER_MATERIAL];  // FIXME:  diffuse_map is a poor word choice
    float            u_anim;
    float            v_anim;

    float            det_u_scale;
    float            det_v_scale;
    float            det_range;
    float            det_alpha_clamp;

    unsigned mat_flags;

    // the last material that sent it's context and its time for duplicate material send optimization.
		static material*   last_context_material; 
		static hw_texture* last_context_texture;
		static map_e       last_context_map;
		static unsigned    last_context_ff;
    static color32     last_context_color;

		friend void serial_in( chunk_file& io, material* m, const stringx& texture_dir, unsigned additional_flags );
};


/*
// because pair<> is ugly:
class color_and_mat_flag
{
public:
  color_and_mat_flag( const color32& _c, bool _mat_flag ) : c(_c), mat_flag(_mat_flag) {}
  color_and_mat_flag() {}
  color32 c;
  bool  mat_flag;  
};
*/

void serial_in( chunk_file& io, material* m, const stringx& texture_dir);

void skip_material_chunk( chunk_file& io );

extern const chunk_flavor CHUNK_MATERIAL;
extern const chunk_flavor CHUNK_TEXTURE;
extern const chunk_flavor CHUNK_ENVIRONMENT;
extern const chunk_flavor CHUNK_DIFFUSE_COLOR;
extern const chunk_flavor CHUNK_UVANIM;
extern const chunk_flavor CHUNK_FLAGS;
extern const chunk_flavor CHUNK_NAME;
extern const chunk_flavor CHUNK_TEXFLAGS;
extern const chunk_flavor CHUNK_TEXINFO;
extern const chunk_flavor CHUNK_DETINFO;


extern instance_bank<material> material_bank;

#endif // MATERIAL_H


