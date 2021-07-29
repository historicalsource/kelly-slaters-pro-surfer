// text_font.cpp
// Copyright (C) 2000 Treyarch L.L.C.  ALL RIGHTS RESERVED

// With precompiled headers enabled, all text up to and including
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#include "hwrasterize.h"
#include "text_font.h"
#include "vertwork.h"
#include "material.h"
#include "forceflags.h"
#include "projconst.h"
#include "chunkfile.h"
#include "geomgr.h"
#include "matfac.h"
#include "coords.h"
#include "globaltextenum.h"

extern stringx ksGlobalButtonArray[];

#if defined(TARGET_XBOX)
#include "osdevopts.h"
#endif /* TARGET_XBOX JIV DEBUG */

int g_debug_text_quads = 1;

//#pragma todo("Remove text_font and have the debugging output use the NEW one.")

text_font::text_font(const char* filename)
{
  chunk_file f;
  stringx fdname = stringx(filename)+".fd";
  f.open(fdname);
  if (f.at_eof())
    error("missing "+fdname);
  cwidth=0;
  cheight=0;
  cspacing=1;
  first=32;
  count=0;
  cperrow=0;
  while (!f.at_eof())
  {
    chunk_flavor flavor;
    serial_in(f,&flavor);
    static const chunk_flavor flavor_cwidth("cwidth");
    static const chunk_flavor flavor_cheight("cheight");
    static const chunk_flavor flavor_cspacing("cspacing");
    static const chunk_flavor flavor_first("first");
    static const chunk_flavor flavor_count("count");
    static const chunk_flavor flavor_cperrow("cperrow");
    if (flavor == flavor_cwidth)
    {
      serial_in(f,&cwidth);
    }
    else if (flavor == flavor_cheight)
    {
      serial_in(f,&cheight);
    }
    else if (flavor == flavor_cspacing)
    {
      serial_in(f,&cspacing);
    }
    else if (flavor == flavor_first)
    {
      serial_in(f,&first);
    }
    else if (flavor == flavor_count)
    {
      serial_in(f,&count);
    }
    else if (flavor == flavor_cperrow)
    {
      serial_in(f,&cperrow);
    }
    else if (flavor == CHUNK_END || flavor == CHUNK_EOF)
    {
      break;
    }
  }

  f.close();

  assert(cwidth>0 && cwidth<256);
  assert(cheight>0 && cheight<256);
  assert(cspacing>=0 && cspacing<16);
  assert(cperrow>0 && cperrow<1024);
  assert(first>=0 && first<=65536);
  assert(count>0 && count<65536);

  mat = NEW material(filename,
    MAT_FULL_SELF_ILLUM      |
    MAT_NO_FOG,
    //anim_texture::PUNCH_THROUGH
    TEX_CLAMP_U              |
    TEX_CLAMP_V);
  // should be filtered
#ifdef NGL
  filespec spec( filename );
  stringx path = spec.path;
  stringx name = spec.name;
  nglSetTexturePath( (char*)path.c_str() );
#if defined(TARGET_XBOX)
	nglTexture* Tex = nglLoadTextureA( (char*)name.c_str() );
#else
  nglTexture* Tex = nglLoadTexture( (char*)name.c_str() ); // Toby removed the lock part
#endif
  if ( Tex )
    itz=vector2d(1.0f/Tex->Width,1.0f/Tex->Height);
  else
    itz = vector2d( 1, 1 );
#else
  const hw_texture* tex = mat->get_texture(0);
  vector2di texsize(tex->get_original_width(),tex->get_original_height());
  itz=vector2d(1.0f/texsize.x,1.0f/texsize.y);
#endif
}

text_font::~text_font()
{
  delete mat;
}


void text_font::get_char_extent(char c, glyph_info& ginfo) const
{
  c -= first;
#ifdef TARGET_PC
  ldiv_t res = ldiv(c,cperrow); // do div and mod at same time
  uint32 x = res.rem;
  uint32 y = res.quot;
#else
  uint32 x = c % cperrow;
  uint32 y = c / cperrow;
#endif
  ginfo.texoffset = vector2di(x*(cwidth+cspacing), y*(cheight+cspacing));
  ginfo.cellsize = vector2di(cwidth,cheight);
}


vector2di text_font::render(const stringx& str, const vector2di& pos, color32 col, float depth, float size)
{
#ifdef NGL
  col.set_alpha( 255 );
/*	Replaced by new API. (dc 05/30/02)
  KSNGL_SetFontColor( col.to_ulong() );
*/
  nglListAddString( nglSysFont, pos.x, pos.y, 0, col.to_ulong(), (char *)str.c_str() );
  return pos;
#else

  if (!col.c.a) return pos;
  mat->send_context(0, MAP_DIFFUSE, FORCE_TRANSLUCENCY);

  vector2di curpos=pos;

  float rhw = 1.0f;
  float z = geometry_manager::inst()->fix_z_coord(depth);

  int strlen = str.length();

  int nchars=0;
  enum { BUFSIZE = 128 };
  assert(strlen<=BUFSIZE);
  surface_vert_ref text_font_indices[BUFSIZE][6];
  vert_workspace_xformed.lock(BUFSIZE*4);
  hw_rasta_vert_xformed * vert_it = vert_workspace_xformed.begin();
  int i4=0;

  for (int i=0; i<strlen; ++i)
  {
    char c=str[i];
    glyph_info ginfo;
    get_char_extent(c,ginfo);

    if (c!=' ')
    {
      vert_it->reset();
      vert_it->set_xyz_rhw(vector4d(curpos.x-0.5f,curpos.y-0.5f,z,rhw));
      vert_it->tc[0] = texture_coord(ginfo.texoffset.x*itz.x,ginfo.texoffset.y*itz.y);
      vert_it->diffuse = col;
      ++vert_it;
      vert_it->reset();
      vert_it->set_xyz_rhw(vector4d(curpos.x+ginfo.cellsize.x-0.5f,curpos.y-0.5f,z,rhw));
      vert_it->tc[0] = texture_coord((ginfo.texoffset.x+ginfo.cellsize.x)*itz.x,ginfo.texoffset.y*itz.y);
      vert_it->diffuse = col;
      ++vert_it;
      vert_it->reset();
      vert_it->set_xyz_rhw(vector4d(curpos.x-0.5f,curpos.y+ginfo.cellsize.y-0.5f,z,rhw));
      vert_it->tc[0] = texture_coord(ginfo.texoffset.x*itz.x,(ginfo.texoffset.y+ginfo.cellsize.y)*itz.y);
      vert_it->diffuse = col;
      ++vert_it;
      vert_it->reset();
      vert_it->set_xyz_rhw(vector4d(curpos.x+ginfo.cellsize.x-0.5f,curpos.y+ginfo.cellsize.y-0.5f,z,rhw));
      vert_it->tc[0] = texture_coord((ginfo.texoffset.x+ginfo.cellsize.x)*itz.x,(ginfo.texoffset.y+ginfo.cellsize.y)*itz.y);
      vert_it->diffuse = col;
      ++vert_it;

      surface_vert_ref* idxs = text_font_indices[nchars];
      idxs[0] = i4  ;
      idxs[1] = i4+1;
      idxs[2] = i4+2;
      idxs[3] = i4+2;
      idxs[4] = i4+1;
      idxs[5] = i4+3;
      i4+=4;
      ++nchars;
    }

    curpos.x+=ginfo.cellsize.x;
  }

  vert_workspace_xformed.unlock();
  // send batch to the card
  hw_rasta::inst()->send_indexed_vertex_list(vert_workspace_xformed, i4,
    text_font_indices[0], nchars*6,
    hw_rasta::SEND_VERT_SKIP_CLIP);

  return curpos;
#endif
}

vector2di text_font::get_size(const stringx& str) const
{
  vector2di curpos(0,0);
  for (int i=0; i<str.length(); ++i)
  {
    char c=str[i];
    glyph_info ginfo;
    get_char_extent(c,ginfo);
    curpos.x += ginfo.cellsize.x;
  }
  return curpos;
}


//--------------------------------------------------------------

Font::Font()
{
  clear_glyph_info();
  m_texwidth = 0;
  m_texheight = 0;
  m_refcount = 0;
  max_width = 0;
}

Font::~Font()
{
}

void Font::unload()
{
  clear_glyph_info();
}


bool Font::is_loaded() const
{
  if (m_texname == "")
    return false;

  return true;
}


void Font::clear_glyph_info()
{
  memset(m_chars, 0, FONT_MAX_CHARS * sizeof (glyph_info));
}

#if !defined(TARGET_XBOX)
#include "kshooks.h"	// For KSWhatever calls
#endif /* TARGET_XBOX JIV DEBUG */

void KSReleaseFile( nglFileBuf* File );
bool Font::load(const stringx& fn)
{
	filename = fn;
	stringx file_fon = "interface\\font\\fon\\"+filename+".fon";
	nglFileBuf file;
	file.Buf = NULL;
	file.Size = 0;
	KSReadFile((char*)(file_fon.c_str()), &file, 1);
	int buf_size = file.Size;
	unsigned char* buffer = (unsigned char*) file.Buf;
//	int index = 0;

	if(buf_size == 0)
	{
		nglPrintf("Font load failed on %s\n", file_fon.data());
		return false;
	}

	// default scale
	m_scale = 1.0f;

	char targaName[256];
	char antiAliasBoolStr[256];
	int w = 0, h = 0;
	int foo;
	int borderPixels;

	// dimensions
	int retval = sscanf( (const char *) buffer,
		";; %s font description file version 1.1\n"
		";; width %d height %d\n"
		";; antialiased %s\n"
		";; ascent %f%% descent %f%%\n"
		";; numglyphs %d\n"
		";; border pixels %d\n"
		";; scale %f\n"
		";; xoffset %d yoffset %d\n",
		targaName,
		&w, &h,
		antiAliasBoolStr,
		&m_ascent, &m_descent,
		&numGlyphs,
		&borderPixels,
		&m_scale,
		&foo, &foo );

	assert(retval == 11);

	for( int i = 1; i <= numGlyphs; i++ )
	{
		char glyphDesc[256];
		int temp1, temp2;
		int retval;
		sprintf( glyphDesc, ";; glyphnum %4d", i );

		// lame way to get to next line
		const char *itr = strstr( (const char *) buffer, glyphDesc );
		assert(itr);

		retval = sscanf( itr,
			";; glyphnum %4u\n"
			"\tascii     %4u\n"
			"\txoffset   %4u\n"
			"\tyoffset   %4u\n"
			"\tcellwidth     %4u\n"
			"\tcellheight    %4u\n"
			"\tglyphwidth     %4u\n"
			"\tglyphheight    %4u\n"
			"\tglyphOriginx   %4u\n"
			"\tglyphOriginy   %4u\n",
			&temp1,
			&temp2,
			&m_chars[i - 1].cell_x,
			&m_chars[i - 1].cell_y,
			&m_chars[i - 1].cell_width,
			&m_chars[i - 1].cell_height,
			&m_chars[i - 1].glyph_width,
			&m_chars[i - 1].glyph_height,
			&m_chars[i - 1].glyph_x,
			&m_chars[i - 1].glyph_y );
		assert(retval == 10);
		m_chars[i - 1].ascii = (unsigned char) temp2;
		if( static_cast<char>(m_chars[i-1].ascii) == ' ' )
		{
			m_chars[i - 1].cell_height = 0;
			m_chars[i - 1].glyph_height = 0;
			m_chars[i - 1].glyph_y = 0;
		}
		assert(m_chars[i-1].ascii);
	}

	KSReleaseFile(&file);
	stringx path = "interface\\font\\"+os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR)+"\\";
	nglSetTexturePath((char*) path.c_str());
	texture = nglLoadTextureA(filename.data());
	if(!texture) return false;
	m_texwidth = texture->Width;
	m_texheight = texture->Height;

	// load art button textures
	art_button_tex[GT_PadU] = nglLoadTextureA("igo_icon_arrow");
	for(int i=1; i<GT_PadCircle; i++)
		art_button_tex[i] = art_button_tex[0];
	art_button_tex[GT_PadCircle] = nglLoadTextureA("igo_icon_circle");
	art_button_tex[GT_PadSquare] = nglLoadTextureA("igo_icon_square");
	art_button_tex[GT_PadCross] = nglLoadTextureA("igo_icon_x");
	art_button_tex[GT_PadTriangle] = nglLoadTextureA("igo_icon_triangle");
	art_button_tex[GT_PadL1] = nglLoadTextureA("igo_icon_left_1");
	art_button_tex[GT_PadL2] = nglLoadTextureA("igo_icon_left_1");
	art_button_tex[GT_PadR1] = nglLoadTextureA("igo_icon_right_1");
	art_button_tex[GT_PadR2] = nglLoadTextureA("igo_icon_right_1");
	art_button_tex[GT_PadStart] = nglLoadTextureA("igo_icon_start");

	for(int i=0; i<GT_ButtonSpacing; i++)
	{
		art_button_width[i] = 20;
		art_button_height[i] = 20;

		// these are the rectangular buttons, the triggers in PS2, and the start buttons for PS2 & XBOX
#ifdef TARGET_PS2
		if(i == GT_PadL1 || i == GT_PadL2 || i == GT_PadR1 || i == GT_PadR2 || i == GT_PadStart)
			art_button_width[i] = 40;
#elif TARGET_XBOX
		if(i == GT_PadStart)
			art_button_width[i] = 40;
#endif
	}

	return true;
}


void Font::reload()
{
	stringx path = "interface\\font\\"+os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR)+"\\";
	nglSetTexturePath((char*) path.c_str());
	texture = nglLoadTextureA(filename.data());

	// load art button textures
	art_button_tex[GT_PadU] = nglLoadTextureA("igo_icon_arrow");
	for(int i=1; i<GT_PadCircle; i++)
		art_button_tex[i] = art_button_tex[0];
	art_button_tex[GT_PadCircle] = nglLoadTextureA("igo_icon_circle");
	art_button_tex[GT_PadSquare] = nglLoadTextureA("igo_icon_square");
	art_button_tex[GT_PadCross] = nglLoadTextureA("igo_icon_x");
	art_button_tex[GT_PadTriangle] = nglLoadTextureA("igo_icon_triangle");
	art_button_tex[GT_PadL1] = nglLoadTextureA("igo_icon_left_1");
	art_button_tex[GT_PadL2] = nglLoadTextureA("igo_icon_left_1");
	art_button_tex[GT_PadR1] = nglLoadTextureA("igo_icon_right_1");
	art_button_tex[GT_PadR2] = nglLoadTextureA("igo_icon_right_1");
	art_button_tex[GT_PadStart] = nglLoadTextureA("igo_icon_start");
}

void Font::getColor(const char c, color32 & col)
{
	// only return alpha=0 for failed or no color cases, because render()
	// will assume that the function failed and draw with the original color

	// NOTE: These color codes are really only needed when you want
	// a single TextString object to have multiple colors.
	// If the TextString is going to be all one color, just set the color parameter in its constructor.
	switch (c)
	{
	case 'r' : col = color32(255, 0, 0, col.c.a); break;		// Red
	case 'g' : col = color32(149, 251, 149, col.c.a); break;		// Green
	case 'u' : col = color32(0, 0, 255, col.c.a); break;		// blUe
	case 'a' : col = color32(0, 0, 0, col.c.a); break;			// blAck
	case 'w' : col = color32(255, 255, 255, col.c.a); break;	// White
	case 'y' : col = color32(255, 255, 0, col.c.a); break;		// Yellow
	case 'o' : col = color32(255, 127, 0, col.c.a); break;		// Orange

	case '0' : col = color32(255, 150, 0, col.c.a); break;		// Gap tricks

	//case '1' : col = color32(255, 255, 127, col.c.a); break;	// High danger tricks
	case '1' : col = color32(255, 255, 102, col.c.a); break;	// High danger tricks
	//case '2' : col = color32(255, 255, 255, col.c.a); break;	// Med danger tricks
	case '2' : col = color32(255, 255, 255, col.c.a); break;	// Med danger tricks
	//case '3' : col = color32(140, 140, 140, col.c.a); break;	// Low danger tricks
	case '3' : col = color32(153, 163, 171, col.c.a); break;	// Low danger tricks

	case '4' : col = color32(2, 162, 255, col.c.a); break;		// Sick meter low tricks
	case '5' : col = color32(0, 216, 120, col.c.a); break;		// Sick meter med tricks
	case '6' : col = color32(236, 206, 74, col.c.a); break;		// Sick meter high tricks
	case '7' : col = color32(231, 101, 26, col.c.a); break;		// Sick meter extreme tricks

	case '8' : col = color32(255, 255, 102, col.c.a); break;	// Statistics screen: info color
	case '9' : col = color32(149, 251, 149, col.c.a); break;	// Statistics screen: standard color

	case 'B' : col = color32(10, 71, 102, col.c.a); break;		// Bio color regular (blue)
	case 'R' : col = color32(150, 0, 24, col.c.a); break;		// Bio color highlight (red)
	case 'G' : col = color32(149, 251, 149, col.c.a); break;		// Description title color (green)
	case 'W' : col = color32(255, 255, 255, col.c.a); break;	// Description color (white)

	case 'n' : col = color32(0, 0, 0, col.c.a); break;			// No color

	case 1   : col = color32(col.c.r, col.c.g, col.c.b, 255); break;	// full alpha
	case 2   : col = color32(col.c.r, col.c.g, col.c.b, 127); break;	// half alpha
	case 3   : col = color32(col.c.r, col.c.g, col.c.b, 63); break;		// 1/4th alpha
	case 4   : col = color32(col.c.r, col.c.g, col.c.b, 33); break;		// 1/8th alpha

	default : col = color32(255, 255, 255, 255);
	}
}

float Font::getScale(const char c)
{
	float scale;
	switch(c)
	{
	case '1': scale = 0.80f; break;
	case '2': scale = 0.85f; break;
	case '3': scale = 0.90f; break;
	case '4': scale = 0.95f; break;
	case '5': scale = 1.00f; break;
	case '6': scale = 1.05f; break;
	case '7': scale = 1.10f; break;
	case '8': scale = 1.15f; break;
	case '9': scale = 1.20f; break;
	case 'c': scale = 1.20f; break;		// credits scale
	case 'n': scale = 0.0f; break;
	default: scale = 0.0f; break;
	}
	return scale;
}

void Font::render(const stringx& text, color32 specifiedColor, bool no_color, bool override_alpha,
				  rational_t x, rational_t y, rational_t depth,
				  const enum HORIZJUST horizJustification, const enum VERTJUST vertJustification,
				  bool even_number_spacing, rational_t scale, float art_button_scale, 
				  float * delta_x, float * delta_y, rational_t angle, bool random_string_fade)
{
	color32	color = specifiedColor;
	bool	useColorCodes = no_color;	// color codes only work for non-colored text
	
#ifdef TARGET_GC
	if ( scale<0.5f ) scale=0.5f;
#endif
	
	float scale_x = scale;
	float scale_y = scale;
	if(art_button_scale == -1) art_button_scale = scale;
	float art_button_scale_x = art_button_scale;
	float art_button_scale_y = art_button_scale;
	
	adjustSizes(scale_x, scale_y);
	adjustSizes(art_button_scale_x, art_button_scale_y);
	
	// Invisible?
	if (!color.c.a)
		return;

	float r = color.c.r/256.0f;
	float g = color.c.g/256.0f;
	float b = color.c.b/256.0f;
	float a = color.c.a/256.0f;
	
	float invw,invh;
	int ssize = text.size();
	
	invw = 1.0f / m_texwidth;
	invh = 1.0f / m_texheight;
	
	float base_dx, base_dy;
	fast_sin_cos_approx( angle, &base_dy, &base_dx );
	base_dy = -base_dy;
	
	int zero_index = getIndex('0');
	int nine_index = getIndex('9');
	
	if(max_width == 0 && even_number_spacing)
	{
		const Font::glyph_info *ci = &m_chars[zero_index];
		max_width = ci->cell_width;
		for(int i=1; i<10; i++)
		{
			ci = &m_chars[zero_index+i];
			if(ci->cell_width > (u_int) max_width) max_width = ci->cell_width;
			
		}
	}
	
	// Move text based on horizontal justification.
	if (horizJustification == HORIZJUST_CENTER || horizJustification == HORIZJUST_RIGHT)
	{
		float length = getWidth(text, scale, even_number_spacing, art_button_scale);

		// Adjust placement.
		if (horizJustification == HORIZJUST_CENTER)
			x = x - length/2.0f;
		else if (horizJustification == HORIZJUST_RIGHT)
			x = x - length;
	}
	
	float art_button_y=0.0f;

	// Move text based on vertical justification.
	// y is bottom edge of text after these adjustments
	if (vertJustification == VERTJUST_CENTER)
	{
		const Font::glyph_info *ci = &m_chars[getIndex('A')];
		y = y + (ci->glyph_height*scale_y/2.0f);
		float text_height = ci->glyph_height*scale_y;
		float button_height = art_button_height[GT_PadCross]*art_button_scale_y;
		art_button_y = y - text_height/2.0f + button_height/2.0f;
	}
	else if (vertJustification == VERTJUST_BOTTOM)
	{
		//Use y value as is
		art_button_y = y;
	}
	else if (vertJustification == VERTJUST_TOP)
	{
		const Font::glyph_info *ci = &m_chars[getIndex('A')];
		y = y + (ci->glyph_height*scale_y);
		float text_height = ci->glyph_height*scale_y;
		float button_height = art_button_height[GT_PadCross]*art_button_scale_y;
		art_button_y = y - text_height + button_height;
	}
	
	bool ignore_next = false;
	bool scale_code = false;
	int art_button_count = 0;	// has this many characters left in art button code
	int art_button_index = 0;	// globaltext array index of art button
	float art_button_start_x = 0;

	for (int i=0; i<ssize; ++i)
	{
		if(ignore_next)
		{
			ignore_next = false;
			
			if(scale_code)
			{
				float s = getScale(text[i]);
				if(s <= 0)	// no scale
				{
					scale_x = scale;
					scale_y = scale;
					art_button_scale_x = art_button_scale;
					art_button_scale_y = art_button_scale;
				}
				else
				{
					scale_x = scale*s;
					scale_y = scale*s;
					art_button_scale_x = art_button_scale*s;
					art_button_scale_y = art_button_scale*s;
				}

				adjustSizes(scale_x, scale_y);
				adjustSizes(art_button_scale_x, art_button_scale_y);
			}
			else if (useColorCodes)
			{
				no_color = false;
				
				getColor(text[i], color);
				
				// getColor failed in this case
				if (color.c.a == 0) no_color = true;
				
				r = color.c.r/256.0f;
				g = color.c.g/256.0f;
				b = color.c.b/256.0f;
				if (override_alpha)
					a = specifiedColor.c.a/256.0f;
				else
					a = color.c.a/256.0f;
			}
			continue;
		}
		if(text[i] == '@')
		{
			// Interpret @ at the end of string as @.
			if (i == ssize-1)
			{
				// do nothing
			}
			else if (text[i+1] == '@')
			{
				// Interpret @@ as @.
				i++;
			}
			else
			{
				// Interpret @x as a color code.
				ignore_next = true;
				scale_code = false;
				continue;
			}
		}
		if(text[i] == '^')
		{
			// Interpret ^ at the end of string as ^.
			if (i == ssize-1)
			{
				// do nothing
			}
			else if (text[i+1] == '^')
			{
				// Interpret ^^ as ^.
				i++;
			}
			else
			{
				// Interpret ^x as a scale code.
				ignore_next = true;
				scale_code = true;
				continue;
			}
		}

		// possible art button code
		if(art_button_count <= 0 && text[i] == ksGlobalButtonArray[GT_ButtonSpacing].c_str()[0])
		{
			int length, button_index;
			bool is_art_button = CheckArtButtonCodes(&(text.c_str()[i]), length, button_index);
			if(is_art_button)
			{
				art_button_count = length;
				art_button_index = button_index;
				art_button_start_x = x;
			}
		}
		
		int tmp = getIndex(text[i]);

#ifdef BETH
		if(text[i] == '*' || text[i] == '\\' || text[i] == '>')
			assert(0);
#endif


		if (tmp == -1)
		{
			// bad characters replaced with '#', which is the square symbol in the font
			tmp = getIndex('#');
#ifdef BETH
			nglPrintf("Bad character: %c\n", text[i]);
#endif
		}
		assert(tmp != -1);
		const Font::glyph_info *ci = &m_chars[tmp];
		float x0, y0, x1, y1, tx0, ty0, tx1, ty1;
		float v[4][2];
		
		int width = ci->cell_width;
		if(even_number_spacing && tmp >= zero_index && tmp <= nine_index)
			width = max_width;
		
		if (width != 0 && art_button_count <= 1)
		{
			// all coords and texture coords are different for art buttons
			if(art_button_count == 1)
			{
				x0 = art_button_start_x;
				x1 = x + (art_button_width[art_button_index]+1.0f)*art_button_scale_x;
//				y0 = y - art_button_height[art_button_index]*art_button_scale_y;
//				y0 = art_button_y;
//				y1 = y0 + (art_button_height[art_button_index]+1.0f)*art_button_scale_y;
				y1 = art_button_y;
				y0 = y1 - (art_button_height[art_button_index]+1.0f)*art_button_scale_y;
				tx0 = 0; tx1 = 1;
				ty0 = 0; ty1 = 1;
			}
			else
			{
				float tmp_x = x;
				if (delta_x) tmp_x = x + delta_x[i];
//				x0 = tmp_x - ci->cell_width*scale_x + ci->cell_width*m_ascent*scale_x;
				x0 = tmp_x;
				x1 = x0 + width * scale_x;
				
				float tmp_y = y;
				if (delta_y) tmp_y = y + delta_y[i];
				y0 = tmp_y - ci->cell_height * scale_y + ci->cell_height * m_descent * scale_y;
				y1 = y0 + ci->cell_height * scale_y;

				tx0 = ((float)ci->cell_x) * invw;
				ty0 = ((float)ci->cell_y) * invh;
				tx1 = tx0 + (width * invw);
				ty1 = ty0 + ((ci->cell_height) * invh);
			}
			
			v[0][0] = x0;
			v[0][1] = y0;
			v[1][0] = x1;
			v[1][1] = y0;
			v[2][0] = x0;
			v[2][1] = y1;
			v[3][0] = x1;
			v[3][1] = y1;

			// For random text: fade out as y changes. (hack)
			if (delta_y && random_string_fade)
			{
				float	fadeLeft = 0;
				float	fadeRight = 639;
				float	fadeTop = 240;
				float	fadeBottom = 400;
				
				adjustCoords(fadeLeft, fadeTop);
				adjustCoords(fadeRight, fadeBottom);

				if (y0 <= fadeTop)
					a = 0.0f;
				else if (y0 >= fadeBottom)
					a = 1.0f;
				else
					a = (y0-fadeTop)/(fadeBottom-fadeTop);
			}
		
			nglQuad q;
			float nglz = depth;
			nglInitQuad( &q );
			if(!no_color && art_button_count != 1)
				nglSetQuadColor(&q, NGL_RGBA32(FTOI(r*255.0f), FTOI(g*255.0f), FTOI(b*255.0f), FTOI(a*255.0f)));

			// art button color shouldn't be set except for alpha
			if(!no_color && art_button_count == 1)
				nglSetQuadColor(&q, NGL_RGBA32(255, 255, 255, FTOI(a*255.0f)));
			nglSetQuadMapFlags(&q, NGLMAP_CLAMP_U | NGLMAP_CLAMP_V | NGLMAP_BILINEAR_FILTER);
			if(art_button_count == 1) nglSetQuadTex(&q, art_button_tex[art_button_index]);
			else nglSetQuadTex( &q, texture);
#if defined(TARGET_XBOX)
			if(depth > 1.0f)
				nglz = depth = 1.0f;
#endif /* TARGET_XBOX JIV DEBUG */
			
			nglSetQuadRect( &q, v[0][0], v[0][1], v[3][0], v[3][1] );
			nglSetQuadUV( &q, tx0, ty0, tx1, ty1 );
			nglSetQuadZ( &q, nglz );
			if(art_button_count == 1 && art_button_index < GT_PadCircle)
				nglRotateQuad(&q, (x1-x0)/2.0f+x0, (y1-y0)/2.0f+y0, FindArtButtonRotation(art_button_index));
			nglListAddQuad( &q );

			if(art_button_count == 1)
			{
				x = art_button_start_x + art_button_width[art_button_index] * art_button_scale_x;
				y += base_dy * width * art_button_scale_y;
			}
			else
			{
				x += base_dx * width * scale_x;
				y += base_dy * width * scale_y;
			}
		}
		if(art_button_count > 0) art_button_count--;
	}
}

bool Font::CheckArtButtonCodes(const char* text, int& length, int& button_index)
{
	// GT_ButtonSpacing is after last real button
	for(int i=0; i<GT_ButtonSpacing; i++)
	{
		stringx tmp = ksGlobalButtonArray[i];
		if(strncmp(text, tmp.c_str(), tmp.length()) == 0)
		{
			length = tmp.length();
			button_index = i;
			return true;
		}
	}
	return false;
}

float Font::FindArtButtonRotation(int index)
{
	float pi = 3.14f;
	switch(index)
	{
	case GT_PadU:	return 0.0f;
	case GT_PadUR:	return 0.25f*pi;
	case GT_PadR:	return 0.5f*pi;
	case GT_PadDR:	return 0.75f*pi;
	case GT_PadD:	return pi;
	case GT_PadDL:	return 1.25f*pi;
	case GT_PadL:	return 1.5f*pi;
	case GT_PadUL:	return 1.75f*pi;
	default: assert(0);
	}
	return 0.0f;
}

Font::glyph_info* Font::getGlyph(char c)
{
  int which = getIndex(c);
  if (which >=0)
	  return &m_chars[which];
  else
    return NULL;
}

int Font::getIndex(char c)
{
	unsigned char c1 = (unsigned char) c;
	for(int i=0; i<numGlyphs; i++)
		if(m_chars[i].ascii == c1)
			return i;
	return -1;
}

stringx Font::get_filename() const
{
  return m_texname;
}


float Font::text_width(const stringx &s) const
{
  float w = 0;
  int len = s.length();

  for (int i = 0; i < len; i++)
  {
    w += m_chars[s[i]].cell_width;
  }
  return w;
}

float Font::getWidth(stringx s, float sc, bool even_number_spacing, float art_button_sc)
{
	float scale_x = sc;
	float scale_y = sc;
	adjustSizes(scale_x, scale_y);

	if(art_button_sc == -1.0f)
		art_button_sc = sc;
	float art_sc_x = art_button_sc;
	float art_sc_y = art_button_sc;
	adjustSizes(art_sc_x, art_sc_y);

	float w = 0;
	int len = s.length();
	bool ignore_next = false;
	int art_button_count = 0;
	
	for (int i = 0; i < len; i++)
	{
		if(art_button_count > 0) art_button_count--;
		if(art_button_count > 0) continue;
		if(ignore_next)
		{
			ignore_next = false;
			continue;
		}
		if(s[i] == '@')
		{
			// Interpret @ at the end of string as @.
			if (i == len-1)
			{
				// do nothing
			}
			else if (s[i+1] == '@')
			{
				// Interpret @@ as @.
				i++;
			}
			else
			{
				// Interpret @x as a color code.
				ignore_next = true;
				continue;
			}
		}
		// possible art button code
		if(art_button_count <= 0 && s[i] == ksGlobalButtonArray[GT_ButtonSpacing].c_str()[0])
		{
			int length, button_index;
			bool is_art_button = CheckArtButtonCodes(&(s.c_str()[i]), length, button_index);
			if(is_art_button)
			{
				w += (art_button_width[button_index])*art_sc_x;
				art_button_count = length;
				continue;
			}
		}

		int tmp = getIndex(s[i]);
		if (tmp == -1) tmp = getIndex('X'); // assert(tmp != -1);
		assert(tmp != -1);
		const Font::glyph_info *ci = &m_chars[tmp];
		if(even_number_spacing && tmp >= getIndex('0') && tmp <= getIndex('9'))
			w += (max_width)*scale_x;
		else
			w += (ci->cell_width)*scale_x;
	}
	return w;
}


float Font::text_height(const stringx &s) const
{
  float h = 0;

  int len = s.length();

  for (int i = 0; i < len; i++)
  {
    if (m_chars[s[i]].cell_height > h)
      h = m_chars[s[i]].cell_height;
  }
  return h;
}


void Font::xform_coord(rational_t &x, rational_t &y, rational_t cx, rational_t cy, rational_t angle)
{
  float s, c, nx, ny;

  s = sin(angle);	// fix trig (dc 08/16/01)
  c = cos(angle);

  x -= cx;
  y -= cy;
  nx = c * x + s * y;
  ny = -s * x + c * y;
  x = nx + cx;
  y = ny + cy;
}




DEFINE_SINGLETON(font_mgr)


font_mgr::font_mgr()
{

}

font_mgr::~font_mgr()
{
  list<Font*>::iterator it = m_fonts.begin();
  for (; it != m_fonts.end(); it++)
  {
    delete *it;
  }
}

Font *font_mgr::acquire_font(const stringx& name)
{
  list<Font*>::iterator it = m_fonts.begin();
  for (; it != m_fonts.end(); it++)
  {
    Font *f = (*it);
    if (f->get_filename() == name)
    {
      f->inc_ref();
      return f;
    }
  }
  debug_print("Loading NEW font %s", name.c_str());
  Font *n = NEW Font;
  n->load(name);
  if (!n->is_loaded())
    error("Unable to load font %s", name.c_str());
  n->inc_ref();
  m_fonts.push_back(n);
  return n;
}

void font_mgr::release_font(Font *f)
{
  f->dec_ref();
  debug_print("Releasing font %s; %i references remain", f->get_filename().c_str(), f->get_ref());
  if (f->get_ref() == 0)
  {
    debug_print("No more references to this font; it's toast.");
    m_fonts.remove(f);
    delete f;
  }
}

////////////////////////////////////////////////////////////////////


const font_char_info def_char_infos[TYPEFACE_INFOS_MAX] =
{
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 9 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  { 167,  62, 183,  78, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   1,   1,  12,  28, 0 },
  {  15,   1,  19,  28, 0 },
  {  23,   1,  34,  28, 0 },
  {  36,   1,  47,  28, 0 },
  {  49,   1,  61,  28, 0 },
  {   1,  30,  12,  57, 0 },
  {  13,  30,  24,  57, 0 },
  {  25,  30,  37,  57, 0 },
  {  38,  30,  49,  57, 0 },
  {  50,  30,  62,  57, 0 },
  {  55,  86,  59, 100, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,  14,  15, 0 },
  {  16,   0,  29,  15, 0 },
  {  30,   0,  45,  15, 0 },
  {  47,   0,  61,  15, 0 },
  {  63,   0,  76,  15, 0 },
  {  78,   0,  90,  15, 0 },
  {  90,   0, 105,  15, 0 },
  { 108,   0, 121,  15, 0 },
  { 124,   0, 127,  15, 0 },
  { 129,   0, 139,  15, 0 },
  { 142,   0, 155,  15, 0 },
  { 157,   0, 169,  15, 0 },
  { 170,   0, 186,  15, 0 },
  { 189,   0, 202,  15, 0 },
  { 204,   0, 220,  15, 0 },
  { 221,   0, 234,  15, 0 },
  { 235,   0, 250,  15, 0 },
  {   0,  21,  14,  36, 0 },
  {  15,  21,  27,  36, 0 },
  {  28,  21,  41,  36, 0 },
  {  42,  21,  55,  36, 0 },
  {  56,  21,  71,  36, 0 },
  {  71,  21,  93,  36, 0 },
  {  93,  21, 107,  36, 0 },
  { 107,  21, 121,  36, 0 },
  { 121,  21, 133,  36, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  { 134,  23, 147,  36, 0 },
  { 148,  23, 159,  36, 0 },
  { 160,  23, 172,  36, 0 },
  { 174,  23, 186,  36, 0 },
  { 187,  23, 198,  36, 0 },
  { 200,  23, 210,  36, 0 },
  { 211,  23, 223,  36, 0 },
  { 225,  23, 236,  36, 0 },
  { 239,  23, 242,  36, 0 },
  { 243,  23, 251,  36, 0 },
  {   0,  44,  12,  57, 0 },
  {  13,  44,  23,  57, 0 },
  {  24,  44,  38,  57, 0 },
  {  40,  44,  52,  57, 0 },
  {  53,  44,  66,  57, 0 },
  {  67,  44,  78,  57, 0 },
  {  79,  44,  92,  57, 0 },
  {  94,  44, 105,  57, 0 },
  { 106,  44, 117,  57, 0 },
  { 117,  44, 128,  57, 0 },
  { 129,  44, 140,  57, 0 },
  { 141,  44, 153,  57, 0 },
  { 153,  44, 171,  57, 0 },
  { 172,  44, 183,  57, 0 },
  { 184,  44, 195,  57, 0 },
  { 195,  44, 206,  57, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 },
  {   0,   0,   0,   0, 0 }
};
//-----------------------------------------------------------------
bool font_def::open(const char *name)
{
#ifdef TARGET_PS2
  filespec foospec( name );
  stringx path = foospec.path.c_str();
  stringx _name = foospec.name.c_str();
  nglSetTexturePath( (char*)( path.c_str() ) );
  nglTexture* Tex = nglLoadTexture( (char*)( _name.c_str() ) );
//  nglTexture* Tex = nglLoadTextureLock( (char*)( _name.c_str() ) );
  if ( Tex )
  {
    w = Tex->Width;
    h = Tex->Height;
  }
  else
    w = h = 0;
#else
  frame = hw_texture_mgr::inst()->load_texture(name);
  w = (float)frame->get_width();
  h = (float)frame->get_height();
#endif
  memcpy(char_infos, def_char_infos, sizeof(char_info) * TYPEFACE_INFOS_MAX);

  isopen=true;
  return true;
}

void font_def::unload()
{
  frame->unload();
  frame = NULL;
  isopen = false;
}

//------------------------------------------------------------------
const font_char_info *font_def::get_char_info(char c) const
{
#ifndef __GNUC__
  assert(c >= 0 && c < TYPEFACE_INFOS_MAX);
#endif
  return &char_infos[c];
}
//------------------------------------------------------------------
int font_def::text_width(const stringx& s) const
{
  int w = 0;
  const font_char_info *ci;
  for(int i=0; i<s.size(); ++i)
  {
    ci = get_char_info(s[i]);
    if(ci)
    {
      w += ci->x1 - ci->x0 + 1 + ci->adv;
    }
  }
  return w;
}

void font_def::render(const stringx& s, color32 clr, float& x, float y, float z, float rhw, float scale) const
{
  vertex_context vc;
  vc.set_translucent(true);
  vc.set_src_alpha_mode(vertex_context::SRCALPHA);
  vc.set_dst_alpha_mode(vertex_context::INVSRCALPHA);
  hw_rasta::inst()->send_context(vc);
  hw_rasta::inst()->send_texture(frame);
  material::flush_last_context();

#ifdef TARGET_PC
  z = geometry_manager::inst()->fix_z_coord(z);
#endif

  x += 0.5f;
  y += 0.5f;

  float invw = 1.0f / w, invh = 1.0f / h;
  //  color32 spec(0,0,0,0);  // unused -- remove me?

#ifdef TARGET_PS2
  nglQuad q;
  nglInitQuad( &q );
  filespec spec( frame->get_name() );
  nglFixedString p( spec.name.c_str() );
  nglSetQuadMapFlags(&q, NGLMAP_CLAMP_U | NGLMAP_CLAMP_V | NGLMAP_BILINEAR_FILTER);
  nglSetQuadTex( &q, nglGetTexture( p ) );
#endif

  for(int i = 0; i < s.size(); ++i)
  {
    const font_char_info* ci = get_char_info(s[i]);
    if(ci)
    {
      // start out with the local coordinates
      float x0 = x +  (float)ci->x0      * scale;
      float x1 = x + ((float)ci->x1 - 1) * scale;
      float y0 = y +  (float)ci->y0      * scale;
      float y1 = y + ((float)ci->y1 - 1) * scale;

#ifdef TARGET_PS2
      nglSetQuadRect( &q, x0, y0, x1, y1 );
      nglSetQuadUV( &q, ci->x0 * invw, ci->y0 * invh, ci->x1 * invw, ci->y1 * invh );
      nglSetQuadZ( &q, z );
      nglListAddQuad( &q );
#endif

#ifdef TARGET_PC
      vert_workspace_xformed_quad.lock(4);
      hw_rasta_vert_xformed* vert_it = vert_workspace_xformed_quad.begin();
      vert_it[0] = hw_rasta_vert_xformed(vector3d(x0, y0, z), rhw, texture_coord(ci->x0 * invw, ci->y0 * invh), clr/*, spec*/);
      vert_it[1] = hw_rasta_vert_xformed(vector3d(x1, y0, z), rhw, texture_coord(ci->x1 * invw, ci->y0 * invh), clr/*, spec*/);
      vert_it[2] = hw_rasta_vert_xformed(vector3d(x0, y1, z), rhw, texture_coord(ci->x0 * invw, ci->y1 * invh), clr/*, spec*/);
      vert_it[3] = hw_rasta_vert_xformed(vector3d(x1, y1, z), rhw, texture_coord(ci->x1 * invw, ci->y1 * invh), clr/*, spec*/);
      vert_workspace_xformed_quad.unlock();
      // should use a big triangle list instead
      hw_rasta::inst()->send_vertex_strip(vert_workspace_xformed_quad, 4);
#endif

      x += float(ci->x1 - ci->x0 + 1 + ci->adv) * scale;
    }
  }

  x -= 0.5f;
}


/////////////////////////////////////////////////////////////


typeface_def::~typeface_def()
{
  unload();
}

//--------------------------------------------------------------
void typeface_def::open( const stringx& name )
{
  m_name = name;
}

//--------------------------------------------------------------
void typeface_def::load()
{
  if( !usercount )
  {
    // SYNTHESIZE FILENAME FOR char_infos BASED ON m_name AND READ DATA FIRST
    stringx s( m_name );
    s += ".fon";
    text_file fp;
    fp.open( s, os_file::FILE_READ );
    if(! fp.is_open() )
    {
      error( stringx("Typeface has no FON file") );
    }
    else
    {
      int i;
      int number_of_bitmaps;
      fp.read( &number_of_bitmaps );
      assert(number_of_bitmaps == 1);
      for( i = 0; i < number_of_bitmaps; ++i )
      {
        stringx texname;
        fp.read( &texname );
        assert(!frame.mat);
        // load the texture
        frame_info aframe;
        aframe.mat = NEW material(texname,
          MAT_FULL_SELF_ILLUM      |
          MAT_NO_FOG,
          //anim_texture::PUNCH_THROUGH
          TEX_CLAMP_U              |
          TEX_CLAMP_V);
        // should be filtered
#ifdef TARGET_PS2
        filespec foospec( texname );
        stringx path = foospec.path.c_str();
        stringx name = foospec.name.c_str();
        nglSetTexturePath( (char*)( path.c_str() ) );
        nglTexture* Tex = nglLoadTexture( (char*)( name.c_str() ) );
//        nglTexture* Tex = nglLoadTextureLock( (char*)( name.c_str() ) );
        if ( Tex )
          aframe.size = vector2d( Tex->Width, Tex->Height );
        else
          aframe.size = vector2d( 0, 0 );
#else
        const hw_texture* texture = aframe.mat->get_texture(0);
        aframe.size = vector2d(texture->get_original_width(),texture->get_original_height());
#endif
        frame = aframe; //frame.push_back(aframe);
      }

      for( i = 0; i < TYPEFACE_INFOS_MAX; ++i )
      {
        fp.read( &char_infos[ i ].x0 );
        fp.read( &char_infos[ i ].y0 );
        fp.read( &char_infos[ i ].x1 );
        fp.read( &char_infos[ i ].y1 );
        fp.read( &char_infos[ i ].adv );
        // NEEDED FOR MULTI-BITMAP TYPEFACES AT A LATER DATE
        //fp.read( &char_infos[ i ].bitmap );
        //char_infos[ i ].bitmap = 0;
      }
      while( 1 )
      {
        char c1, c2;
        int i1;
        fp.read( &c1 );
        if( c1 != '[' )
        {
          fp.read( &c2 );
          fp.read( &i1 );
          interletter_kern_info.push_back( inter_kern( c1, c2, i1 ) );
          continue;
        }
        break;
      }

      fp.close();
    }
  }

  // MAINTAIN COUNT OF USERS SO I DON'T RELEASE RESOURCES TOO EARLY DURING AN unload
  ++usercount;
}

//--------------------------------------------------------------
void typeface_def::unload()
{
  if( usercount )
  {
    --usercount;

    if( !usercount )
    {
      // SINCE THE LAST KNOWN USER JUST TOLD US TO UNLOAD
      // DUMP THE TEXTURE NOW
      //for( int i = 0; i < frame.size(); ++i )
      {
        delete frame.mat;
        frame.mat = NULL;
      }
      //frame.resize(0);
      interletter_kern_info.resize(0);
    }
  }
}

//--------------------------------------------------------------
int typeface_def::text_width( const stringx &s ) const
{
  int w = 0;
  const char_info *ci;
  int ssize = s.size();

  for (int i=0 ; i<ssize; ++i)
  {
    if( s[i] == '\\' )
    {
      ++i;
      if (i>=ssize-1) break;
      if (s[i] == 'n') break;
      if (s[i] == 'c') { i += 9; } // skip 3x3 color
      // skip any other escapes with extra chars here
      // ignores all other escapes
      continue;
    }
    ci = get_char_info((unsigned char)(s[i]));
    if(ci)
    {
      w += ci->x1 - ci->x0 + 0 + ci->adv;
      if( i < ssize-1 )
      {
        w += interkern( s[i], s[i+1] );
      }
    }
  }
  return w;
}


//--------------------------------------------------------------
int typeface_def::text_height( const stringx &s ) const
{
  int largest_h = 0;
  const char_info *ci;
  int ssize = s.size();

  for (int i = 0; i<ssize; ++i)
  {
    if( s[i] == '\\' )
    {
      ++i;
      if (i>=ssize-1) break;
      if( s[i] == 'n' ) break;
      if( s[i] == 'c' ) { i += 9; } // skip 3x3 color
      // skip any other escapes with extra chars here
      // ignores all other escapes
      continue;
    }
    ci = get_char_info(s[i]);
    if(ci)
    {
      int h = ci->y1 - ci->y0 + 1;
      if ( h > largest_h )
      {
        largest_h = h;
      }
    }
  }
  return largest_h;
}

//--------------------------------------------------------------
int typeface_def::interkern( int l1, int l2 ) const
{
  vector<inter_kern>::const_iterator viki;
  for( viki = interletter_kern_info.begin(); viki != interletter_kern_info.end(); ++viki )
  {
    if( (*viki).letter_pair == pair<int,int>(l1,l2) )
    {
      return (*viki).kern;
    }
  }
  return 0;
}



void typeface_def::render(const stringx& s, color32 clr, rational_t& x, rational_t y, rational_t z, rational_t rhw, rational_t scale) const
{
  if (!clr.c.a) return;
#ifdef TARGET_PC
  rhw = 1.0f;
  z = geometry_manager::inst()->fix_z_coord(z);
#endif

  const char_info* ci;
  const frame_info* fi=0;
  float invw,invh;
  int ssize=s.size();

  fi = &frame;
  invw = 1.0f / fi->size.x;
  invh = 1.0f / fi->size.y;
  frame.mat->send_context(0, MAP_DIFFUSE, FORCE_TRANSLUCENCY);

#ifdef TARGET_PC
  float v[4][2];
  enum { BUFSIZE = 128 };
  assert(ssize<=BUFSIZE);
  surface_vert_ref text_font_indices[BUFSIZE][6];
  vert_workspace_xformed.lock(BUFSIZE*4);
  hw_rasta_vert_xformed * vert_it = vert_workspace_xformed.begin();
  int i4=0, nc=0;
#endif

#ifdef TARGET_PS2
  filespec spec( frame.mat->texture_filename[0] );
  nglFixedString p( spec.name.c_str() );
  nglTexture* Tex = nglGetTexture( p );
  nglQuad q;
  nglInitQuad( &q );
  nglSetQuadMapFlags(&q, NGLMAP_CLAMP_U | NGLMAP_CLAMP_V | NGLMAP_BILINEAR_FILTER);
  nglSetQuadTex( &q, Tex );
#endif

  for (int i=0; i<ssize; ++i)
  {
    ci = get_char_info( s[i] );

    if(ci && ( (ci->x0||ci->x1||ci->y0||ci->y1) || (ci->adv) ) )
    {
#ifdef TARGET_PS2
      nglSetQuadRect( &q,
        x, y,
        x + ((float)(ci->x1 - ci->x0) - 1.0f) * scale,
        y + ((float)(ci->y1 - ci->y0) - 1.0f) * scale );
/*
      nglSetQuadUV( &q,
        ( ci->x0 + 0.5f ) * invw,
        ( ci->y0 + 0.5f ) * invh,
        ( ci->x1 + 0.5f ) * invw,
        ( ci->y1 + 0.5f ) * invh );
*/
      nglSetQuadUV( &q, 0, 0, 1, 1 );
      nglSetQuadZ( &q, z );
      nglListAddQuad( &q );
#endif

#ifdef TARGET_PC
      // ALL IN SCREEN COORDINATES
      v[0][0] = x;
      v[0][1] = y;
      v[1][0] = x + ((float)(ci->x1-ci->x0) - 1.0f) * scale;
      v[1][1] = y;
      v[2][0] = x + ((float)(ci->x1-ci->x0) - 1.0f) * scale;
      v[2][1] = y + ((float)(ci->y1-ci->y0) - 1.0f) * scale;
      v[3][0] = x;
      v[3][1] = y + ((float)(ci->y1-ci->y0) - 1.0f) * scale;

      //color32 spec = color32(0,0,0,0);

      *vert_it++ = hw_rasta_vert_xformed(vector3d(v[0][0], v[0][1], z), rhw, texture_coord(((float)ci->x0 + 0.5f) * invw, ((float)ci->y0 + 0.5f) * invh), clr/*, spec*/);
      *vert_it++ = hw_rasta_vert_xformed(vector3d(v[1][0], v[1][1], z), rhw, texture_coord(((float)ci->x1 + 0.5f) * invw, ((float)ci->y0 + 0.5f) * invh), clr/*, spec*/);
      *vert_it++ = hw_rasta_vert_xformed(vector3d(v[3][0], v[3][1], z), rhw, texture_coord(((float)ci->x0 + 0.5f) * invw, ((float)ci->y1 + 0.5f) * invh), clr/*, spec*/);
      *vert_it++ = hw_rasta_vert_xformed(vector3d(v[2][0], v[2][1], z), rhw, texture_coord(((float)ci->x1 + 0.5f) * invw, ((float)ci->y1 + 0.5f) * invh), clr/*, spec*/);

      surface_vert_ref* idxs = text_font_indices[nc++];
      idxs[0] = i4  ;
      idxs[1] = i4+1;
      idxs[2] = i4+2;
      idxs[3] = i4+2;
      idxs[4] = i4+1;
      idxs[5] = i4+3;

      i4 += 4;
#endif

      x += (float)(ci->x1 - ci->x0 + 0 + ci->adv) * scale;
      if( i < ssize-1 )
        x += interkern( s[i], s[i+1] ) * scale;
    }
  }

#if 0
  *vert_it++ = hw_rasta_vert_xformed(vector3d(  0,   0, z), rhw, texture_coord(0.0f, 0.0f), clr/*, spec*/);
  *vert_it++ = hw_rasta_vert_xformed(vector3d(nglGetScreenHeight(),   0, z), rhw, texture_coord(1.0f, 0.0f), clr/*, spec*/);
  *vert_it++ = hw_rasta_vert_xformed(vector3d(  0, nglGetScreenHeight(), z), rhw, texture_coord(0.0f, 1.0f), clr/*, spec*/);
  *vert_it++ = hw_rasta_vert_xformed(vector3d(nglGetScreenHeight(), nglGetScreenHeight(), z), rhw, texture_coord(1.0f, 1.0f), clr/*, spec*/);

  surface_vert_ref* idxs = text_font_indices[nc++];
  idxs[0] = i4  ;
  idxs[1] = i4+1;
  idxs[2] = i4+2;
  idxs[3] = i4+2;
  idxs[4] = i4+1;
  idxs[5] = i4+3;

  i4 += 4;
#endif

#ifdef TARGET_PC
  vert_workspace_xformed.unlock();
  // send batch of characters
  hw_rasta::inst()->send_indexed_vertex_list(vert_workspace_xformed, i4,
    text_font_indices[0],nc*6,
    hw_rasta::SEND_VERT_SKIP_CLIP);
#endif
}


//--------------------------------------------------------------
static list<typeface_def*> typeface_list;

typeface_def *typeface_open( const stringx &fname )
{
  typeface_def *res = 0;

  // DETERMINE IF FACE SETUP ALREADY
  if( (res = typeface_already_exists( fname ) ) != 0 )
  {
    return( res );
  }

  res = NEW typeface_def;
  if( res )
  {
    // FIRST WE MUST READ/SETUP A CHAR_INFO ARRAY
    res->open( fname );
    typeface_list.push_back( res );
  }

  // OTHERWISE ALLOCATE A NEW FACE
  return( res );
}

//--------------------------------------------------------------
typeface_def *typeface_already_exists( const stringx &fname )
{
  list<typeface_def*>::const_iterator tdi;

  for( tdi = typeface_list.begin(); tdi != typeface_list.end(); ++tdi )
  {
    if( fname == (*tdi)->m_name )
    {
      return *tdi;
    }
  }

  return 0;
}

//--------------------------------------------------------------
void typeface_close( typeface_def *tdefptr )
{
  if( !tdefptr->usercount )
  {
    typeface_list.remove( tdefptr );
    delete tdefptr;
  }
}


