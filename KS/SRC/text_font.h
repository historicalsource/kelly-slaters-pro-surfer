// text_font.h
// Copyright (C) 2000 Treyarch L.L.C.  ALL RIGHTS RESERVED

#ifndef TEXT_FONT_H
#define TEXT_FONT_H

#include <list>
#include "algebra.h"
#include "color.h"
#include "rect.h"
#include "singleton.h"
#include "globaltextenum.h"

#ifdef TARGET_PS2
	#include "HWOSPS2\ps2_texturemgr.h"
#endif

#if defined(TARGET_XBOX)
#include "refptr.h"
#include "hwrasterize.h"
#endif /* TARGET_XBOX JIV DEBUG */

class material;
class mat_fac;
class stringx;
//class hw_texture;
#include "hwrasterize.h"


// currently this only supports monospaced fonts
class text_font
{
  class glyph_info
  {
    friend class text_font;
    vector2di texoffset;
    //vector2di glyphoffset;
    //vector2di glyphsize;
    vector2di cellsize;
  };
public:
  text_font(const char* filename);
  ~text_font();

  // returns resulting cursor pos
  vector2di render(const stringx& str, const vector2di& pos, color32 col=color32_white, float depth=0.0f, float size=1.0f);

  vector2di get_size(const stringx& str) const;
protected:
  void get_char_extent(char c, glyph_info& ginfo) const;

  material* mat;
  vector2d itz;    // reciprocal of texture size (extent of one texel)
  uint32 cwidth,   // cell width
         cheight,  // cell height
         cspacing, // cell spacing (pixels between adjacent cells)
         cperrow,  // cells per row
         first,    // first glyph in font (usually 32)
         count;    // number of glyphs in font (usually 96)
};


#define FONT_MAX_CHARS  256

class Font
{
protected:
#if defined(TARGET_XBOX)
	friend class TextString;
	friend class BoxText;
#endif /* TARGET_XBOX JIV DEBUG */
	
	struct glyph_info
	{
		unsigned char ascii;
		unsigned int cell_x, cell_y;            // location of cell on texture
		unsigned int cell_width, cell_height;   // size of the character's box
		unsigned int glyph_x, glyph_y;
		unsigned int glyph_width, glyph_height; // size of the actual character
	};
	
public:
	enum HORIZJUST { HORIZJUST_LEFT, HORIZJUST_CENTER, HORIZJUST_RIGHT };
	enum VERTJUST  { VERTJUST_TOP, VERTJUST_CENTER, VERTJUST_BOTTOM };
	bool unset;
	int max_width;
	
	Font();
	~Font();
	
	bool load(const stringx& filename);
	// loads a font from a .FON/.TGA pair
	// name should not include an extension
	// does not increment the refcount
	void reload();
	void unload();
	
	bool is_loaded() const;
	
	void getColor(const char c, color32 & color);
	float getScale(const char c);
	void render(const stringx& text, color32 color, bool no_color, bool override_alpha, rational_t x,
		rational_t y, rational_t depth, const enum HORIZJUST horizJustification, const enum VERTJUST vertJustification,
		bool even_number_spacing, rational_t scale=1.0f, float art_button_scale=-1.0f, float * delta_x = NULL, 
		float * delta_y = NULL, rational_t angle=0.0f, bool random_string_fade=true);
	glyph_info* getGlyph(char c);
	int getIndex(char c);
	float text_width(const stringx &s) const;
	
	// added by Beth, was having problems with text_width()
	// not usually a good idea to default on the art button scale unless there are no
	// art buttons in the text.  It defaults to the normal scale, which isn't necessarily the same
	float getWidth(stringx s, float sc=1.0f, bool even_number_spacing=false, float art_sc_x=-1.0f);
	float text_height(const stringx &s) const;

	// this api is provided for use by the font manager
	// the font class itself doesn't care about the refcount
	inline void inc_ref() { m_refcount++; };
	inline void dec_ref() { m_refcount--; assert(m_refcount >= 0); };
	inline void set_ref(int val) { m_refcount = val; };
	inline int get_ref() const { return m_refcount; };
	
	stringx get_filename() const;
	// returns the filename this font was loaded from,
	// including the path.
	
	// compares text to art button codes, returns true if is a button code
	// if true, then length is set to length of code string and button_index is
	//  set to button index in globaltext array
	bool CheckArtButtonCodes(const char* text, int& length, int& button_index);
	
	// returns rotation for arrow buttons based on globaltext index
	float FindArtButtonRotation(int index);
	

protected:
	
	void clear_glyph_info();
	
	void xform_coord(rational_t &x, rational_t &y, rational_t cx, rational_t cy, rational_t angle);
	// transforms a glyph vertex about a center coordinate
	
	
	glyph_info m_chars[FONT_MAX_CHARS];       // info for each character
	
	stringx m_texname;
	unsigned int m_texwidth, m_texheight;
	//  mat_fac *m_mat;
	nglTexture* texture;
	int numGlyphs;

	float m_ascent, m_descent;
	float m_scale;
	stringx filename;
	int m_refcount;
	
	nglTexture* art_button_tex[GT_ButtonSpacing];
	int art_button_width[GT_ButtonSpacing];
	int art_button_height[GT_ButtonSpacing];
};


class font_mgr:public singleton
{
public:
  DECLARE_SINGLETON(font_mgr)

  font_mgr();
  ~font_mgr();

  Font* acquire_font(const stringx& name);
  // fonts are now refcounted.
  // acquiring a font increments the count.
  // do not forget to release a font when
  // you're finished.

  void release_font(Font* f);
  // unrefs a font, and if there are no more
  // references, unloads it.

protected:
  list<Font*> m_fonts;
};


// this is the proportional spaced font stuff that used to reside in PFE_element

struct font_char_info
{
	int x0;
	int y0;
	int x1;
	int y1;
	int adv;
  int bitmap;
};

// changing this breaks the .FON files!
#define TYPEFACE_INFOS_MAX 256 // localizers will have to deal with this and signed/unsigned char issues!!

// NOW AN ON-DEMAND LOADED FONT
class font_def
{
public:
  font_def() { isopen=false; }
	bool open(const char *name);
  void unload();
	const font_char_info* get_char_info(char c) const;
	int text_width(const stringx& s) const;
  void render(const stringx& s, color32 clr, float& x,float y,float z,float rhw,float scale=1.0f) const;
  bool is_valid() { return isopen; }
protected:
	font_char_info char_infos[TYPEFACE_INFOS_MAX];
	refptr<hw_texture> frame;
	float        w;                    // width
	float        h;                    // height
	bool         isopen;
};

struct char_info
{
	int x0;
	int y0;
	int x1;
	int y1;
	int adv;
  //int bitmap;
};

class typeface_def
{
	char_info char_infos[TYPEFACE_INFOS_MAX];

  struct frame_info
  {
    material* mat;
    vector2d size;
  };
  frame_info frame;
  
  struct inter_kern
  {
    inter_kern() {}
    inter_kern(int l1, int l2, int k)
    {
      letter_pair = pair<int,int>(l1,l2);
      kern = k;
    }
    pair<int,int> letter_pair;
    int kern;
  };

  vector<inter_kern> interletter_kern_info;
  int interkern( int l1, int l2 ) const;

public:
  stringx m_name;
  int usercount;

  typeface_def() { usercount = 0; frame.mat = NULL; }
  ~typeface_def();
	void open(const stringx& name);
	const char_info *get_char_info(int c) const { assert((c >= 0) && (c < TYPEFACE_INFOS_MAX)); return &char_infos[c]; }
  int text_width(const stringx &s) const;
  int text_height(const stringx &s) const;
  void load();
  void unload();

  void render(const stringx& s, color32 clr, rational_t& x, rational_t y, rational_t z, rational_t rhw, rational_t scale=1.0f) const;
};

typeface_def *typeface_open( const stringx &fname );
typeface_def *typeface_already_exists( const stringx &fname );
void typeface_close( typeface_def *tdefptr );



#endif // TEXT_FONT_H
