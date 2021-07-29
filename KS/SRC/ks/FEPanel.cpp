// FEPanel.cpp

#include "global.h"
#include "hwrasterize.h"

#include "file_finder.h"

#include "FEPanel.h"
#include "osdevopts.h"
#include "kshooks.h"	// For KSWhatever calls
#include "FrontEndManager.h"
#include "geomgr.h"
#include "coords.h"

// ***************** StringList *************************

float	StringList::SPEED_FALL = 3.0f;
float	StringList::SPEED_DRIFT = 2.0f;
int		StringList::RANGE_D_SPEED_LO = 5;
int		StringList::RANGE_D_SPEED_HI = 10;
int		StringList::RANGE_F_SPEED_LO = 10;
int		StringList::RANGE_F_SPEED_HI = 20;
int		StringList::RANGE_D_MAX_LO = 20;
int		StringList::RANGE_D_MAX_HI = 60;
int		StringList::RANGE_BX_LO = 2;
int		StringList::RANGE_BX_HI = 7;
int		StringList::RANGE_BY_LO = 2;
int		StringList::RANGE_BY_HI = 7;

void StringList::Update(const time_value_t time_inc)
{
	float	driftSpeed = SPEED_DRIFT;
	float	fallSpeed = SPEED_FALL;

	adjustSizes(driftSpeed, fallSpeed);
	
	for (int i = 0; i < data.size(); i++)
	{
		delta_x[i] += drift_speed[i]*driftSpeed*time_inc;
		if (delta_x[i] < -drift_max[i] && drift_speed[i] < 0.0f)
		{
			drift_speed[i] = -drift_speed[i];
			delta_x[i] = -drift_max[i];
		}
		else if (delta_x[i] > drift_max[i] && drift_speed[i] > 0.0f)
		{
			drift_speed[i] = -drift_speed[i];
			delta_x[i] = drift_max[i];
		}
		
		delta_y[i] += fall_speed[i]*fallSpeed*time_inc;
	}
}

void StringList::MakeRand(void)
{
	int	driftLo = RANGE_D_SPEED_LO, driftHi = RANGE_D_SPEED_HI;
	int	fallLo = RANGE_F_SPEED_LO, fallHi = RANGE_F_SPEED_HI;
	int	driftMaxLo = RANGE_D_MAX_LO, driftMaxHi = RANGE_D_MAX_HI;
	int	y;

	adjustSizes(driftLo, fallLo);
	adjustSizes(driftHi, fallHi);
	adjustSizes(driftMaxLo, y);
	adjustSizes(driftMaxHi, y);
	
	if (data.size() > StringList::MAX_STRING_SIZE)
	{
		nglPrintf("String too long!  Max is %d and size is %d\n", StringList::MAX_STRING_SIZE, data.size());
		assert(0);
	}
	
	for (int i = 0; i < data.size(); i++)
	{
		drift_speed[i] = random(driftLo, driftHi);
		if (random(2) == 1) drift_speed[i] = -drift_speed[i];
		fall_speed[i] = -(random(fallLo, fallHi));
		drift_max[i] = random(driftMaxLo, driftMaxHi);
		delta_x[i] = 0;
		delta_y[i] = 0;
	}
}

void StringList::Break(void)
{
	int	xLo = RANGE_BX_LO, xHi = RANGE_BX_HI;
	int	yLo = RANGE_BY_LO, yHi = RANGE_BY_HI;
	int	amt;

	adjustSizes(xLo, yLo);
	adjustSizes(xHi, yHi);
	
	for (int i = 0; i < data.size(); i++)
	{
		amt = random(xLo, xHi);
		if (random(2) == 1) amt = -amt;
		delta_x[i] += amt;

		amt = random(yLo, yHi);
		if (random(2) == 1) amt = -amt;
		delta_y[i] += amt;
	}
}

StringList & StringList::operator=(const StringList & s)
{
	int	i;
	
	if (this != &s)
	{
		data = s.data;
		x = s.x;
		y = s.y;

		for (i = 0; i < MAX_STRING_SIZE; i++)
		{
			fall_speed[i] = s.fall_speed[i];
			drift_speed[i] = s.drift_speed[i];
			drift_max[i] = s.drift_max[i];
			delta_x[i] = s.delta_x[i];
			delta_y[i] = s.delta_y[i];
		}
	}
	
	return *this;
}

// ***************** TextString *************************

void TextString::cons(Font* f, stringx t, float x1, float y1, int z1, float s, const Font::HORIZJUST horizJust, const Font::VERTJUST vertJust, bool no_col, color32 col)
{
	font = f;
	text = t;
	x = x1;
	y = y1;
	adjustCoords(x, y);
	z = z1;
	scale = s;
	hJustify = horizJust;
	vJustify = vertJust;
	checkTime = false;
	time = 1.0f;
	even_number_spacing = true;
	no_color = no_col;
	color = col;
	if(color.get_alpha() == 0)
		color.set_alpha(255);
	fade = 0;
	button_scale = -1;
	override_alpha = false;
}

void TextString::setHJustify(const Font::HORIZJUST hjust)
{
	hJustify = hjust;
}

void TextString::setVJustify(const Font::VERTJUST vjust)
{
	vJustify = vjust;
}

float TextString::getX()
{
	float xout=x, yout=y;
	unadjustCoords(xout, yout);
	return xout;
}

float TextString::getY()
{
	float xout=x, yout=y;
	unadjustCoords(xout, yout);
	return yout;
}

int TextString::getZ(void)
{
	return z;
}

void TextString::Update(time_value_t time_inc)
{
	// Fade in.
	if (fade == 1)
	{
		fade_alpha += time_inc/fade_timer;
		if (fade_alpha >= 1.0f)
		{
			fade = 0;
			fade_alpha = 1.0f;
		}
	}
	// Fade out.
	else if(fade == -1)
	{
		fade_alpha -= time_inc/fade_timer;
		if (fade_alpha <= 0.0f)
		{
			fade = 0;
			fade_alpha = 0.0f;
		}
	}

	time -= time_inc;
	if (time < 0.0f) time = 0.0f;
}

void TextString::Draw()
{
	if(!checkTime || time > 0) Render();
}

void TextString::changeText(stringx t)
{
	MakeReplacements(t);
	text = t;
}

void TextString::changePos(float posx, float posy)
{
	x = posx;
	y = posy;
	adjustCoords(x, y);
}

void TextString::changeX(const float posX)
{
	float	posY;
	x = posX;
	adjustCoords(x, posY);
}

void TextString::changeY(const float posY)
{
	float	posX;
	y = posY;
	adjustCoords(posX, y);
}

// start (as opposed to stop) & fade_in (as opposed to fade out)
void TextString::ChangeFade(bool start, bool fade_in, float time)
{
	if(start)
	{
		fade_timer = time;
		if(fade_in)
		{
			if(fade != 1)  // don't restart fade if already started
			{
				fade = 1;
				fade_alpha = 0.0f;
			}
		}
		else
		{
			if(fade != -1)
			{
				fade = -1;
				fade_alpha = 1.0f;
			}
		}
	}
	else fade = 0;
}

void TextString::makeStringList(StringList *sl, stringx d, float x1, float y1)
{
	//StringList ret;// = StringList();
	sl->data = d;
	sl->x = x1;
	sl->y = y1;
//	ret.next = NULL;
	//return ret;
}

void TextString::Render(stringx* t, float x1, float y1)
{
	color32 tmp_color = color;
	if (!override_alpha)
	{
		if (fade != 0) tmp_color.set_alpha(FTOI(fade_alpha*255.0f));
		if (tmp_color.get_alpha() > color.get_alpha()) tmp_color = color;
	}
	else
	{
		tmp_color.set_alpha(fade_alpha*float(color.get_alpha()));
	}

	if(!font->unset)
		font->render(*t, tmp_color, no_color, override_alpha, x1, y1, z, hJustify, vJustify, even_number_spacing, scale, button_scale);
}

void TextString::Render(stringx* t, float x1, float y1, float * xs, float * ys, bool random_text_fade)
{
	color32 tmp_color = color;
	if (fade != 0) tmp_color.set_alpha(FTOI(fade_alpha*255.0f));
	if (tmp_color.get_alpha() > color.get_alpha()) tmp_color = color;

	if (!font->unset)
		font->render(*t, tmp_color, no_color, override_alpha, x1, y1, z, hJustify, vJustify, even_number_spacing, scale, button_scale, xs, ys, 0.0f, random_text_fade);
}

void TextString::MakeReplacements(stringx& text)
{
	for(int i=0; i<text.size(); i++)
	{
		for(int j=0; j<GT_REPLACE_LAST; j++)
		{
			if(text[i] == ksGlobalReplaceArray[j][0])
				text[i] = ksGlobalReplaceArray[j][1];
		}

		// make french & german replacements, if necessary
		// (they share a few letters)
		if(ksGlobalTextLanguage == LANGUAGE_FRENCH || ksGlobalTextLanguage == LANGUAGE_GERMAN)
		{
			for(int j=0; j<GT_REPLACE_FR_LAST; j++)
			{
				if(text[i] == ksGlobalReplaceFRArray[j][0])
					text[i] = ksGlobalReplaceFRArray[j][1];
			}

			for(int j=0; j<GT_REPLACE_GE_LAST; j++)
			{
				if(text[i] == ksGlobalReplaceGEArray[j][0])
					text[i] = ksGlobalReplaceGEArray[j][1];
			}
		}
	}
}

// ***************** MultiLineString *************************

void MultiLineString::cons(Font* f, stringx t, float x1, float y1, int z1, float s, const Font::HORIZJUST horizJust, const Font::VERTJUST vertJust, bool no_col, color32 col)
{
	TextString::cons(f, t, x1, y1, z1, s, horizJust, vertJust, no_col, col);
	setLineSpacing(-1);
	changeText(t);
	fonts[0] = f;
	for(int i=1; i<max_fonts; i++)
		fonts[i] = NULL;
}

void MultiLineString::addFont(int index, Font* f)
{
	assert(index < max_fonts);
	fonts[index] = f;
}

void MultiLineString::changeText(stringx t)
{
	TextString::changeText(t);

	line_num = 1;
	for(int i=0; i<text.size(); i++)
		if(text[i] == '\n') line_num++;
}

void MultiLineString::setLineSpacing(const int new_spacing)
{
	if(new_spacing == -1)
		vSpacing = font->getGlyph('A')->cell_height;
	else
		vSpacing = new_spacing;
}

void MultiLineString::Render(stringx* t, float x1, float y1)
{
	stringx token;
	int current_index = 0;
	int num_lines = 1;
	int line_shift = 0;   // the amount to shift to compensate for vertical justification in multi-line text
	int current_line = 0; // which line we're currently rendering
	color32 tmp_color = color;

	if(fade != 0) tmp_color.set_alpha(FTOI(fade_alpha*255.0f));
	if(tmp_color.get_alpha() > color.get_alpha()) tmp_color = color;

	if(!font->unset)
	{
		// First figure out how many different lines there are
		while((current_index = t->find(current_index, '\n')) != -1)
		{
			num_lines++;
			current_index++;
		}

		// Now figure out how much to shift the text vertically.
		switch(vJustify)
		{
		case Font::VERTJUST_TOP:
			// If vJustify is VERTJUST_TOP then don't shift.
			line_shift = 0; break;
		case Font::VERTJUST_CENTER:
			// If vJustify is VERTJUST_CENTER then shift up by half per extra line.
			line_shift = (vSpacing / 2) * (num_lines - 1); break;
		case Font::VERTJUST_BOTTOM:
			// If vJustify is VERTJUST_BOTTOM then shift up one per extra line.
			line_shift = vSpacing * (num_lines - 1); break;
		}

		// Now print out the lines one at a time
		current_index = 0;
		current_line = 0;
		do
		{
			token = t->read_token("\n", current_index);
			current_index += token.length() + 1;
			Font* f = font;
			if(fonts[current_line] != NULL && !fonts[current_line]->unset)
				f = fonts[current_line];
			f->render(token, tmp_color, no_color, override_alpha, x1, y1 - line_shift + current_line * vSpacing,
			             z, hJustify, vJustify, even_number_spacing, scale, button_scale);
			current_line++;
		} while(current_line < num_lines);
	}
}

// ***************** BouncingText *************************

BouncingText::BouncingText()
{
	targetScale = 1.0f;
	speed = 1.0f;
	override_alpha = false;
}

BouncingText::BouncingText(Font* f, stringx t, float x1, float y1, int z1, float s, const Font::HORIZJUST horizJust, const Font::VERTJUST vertJust)
: TextString(f, t, x1, y1, z1, s, horizJust, vertJust)
{
	targetScale = scale;
	speed = 1.0f;
}

BouncingText::BouncingText(Font* f, stringx t, float x1, float y1, int z1, float s, const Font::HORIZJUST horizJust, const Font::VERTJUST vertJust, color32 col)
: TextString(f, t, x1, y1, z1, s, horizJust, vertJust, col)
{
	targetScale = scale;
	speed = 1.0f;
}

void BouncingText::Bounce(const float bounceScale, const float bounceTime)
{
	assert(bounceScale > 0.0f && bounceTime > 0.0f);

	targetScale = bounceScale;
	speed = targetScale/bounceTime;

	scale = 0.0f;
}

void BouncingText::Update(time_value_t time_inc)
{
	TextString::Update(time_inc);

	if (scale < targetScale)
	{
		scale += speed*time_inc;

		if (scale > targetScale)
			scale = targetScale;
	}
}

// ***************** RandomText *************************

void RandomText::Update(time_value_t time_inc)
{
	TextString::Update(time_inc);

	if (isRand)
		rand_string.Update(time_inc);
}

void RandomText::changeText(stringx t)
{
	TextString::changeText(t);
	rand_string.data = text;
}

void RandomText::Draw()
{
	if (!checkTime || time > 0)
	{
		if (isRand) Render(&text, x, y, rand_string.delta_x, rand_string.delta_y, !noFade);
		else Render(&text, x, y);
	}
}

void RandomText::makeRand()
{
	rand_string.MakeRand();
	time = DISPLAY_MAX;
	checkTime = true;
	isRand = true;
}

void RandomText::Break(void)
{
	if (!isRand)
		makeRand();

	rand_string.Break();
}

// ***************** BoxText ****************************

void BoxText::cons(Font* f, stringx t, float x1, float y1, int z1, float s, const Font::HORIZJUST horizJust, const Font::VERTJUST vertJust, color32 col, int mbs)
{
	TextString::cons(f, t, x1, y1, z1, s, horizJust, vertJust, col.get_alpha() == 0, col);
	non_floating_behavior = true;
	reverse = false;
	box_str_count = 0;
	adjusted_x = real_x = x1;
	adjusted_y = real_y = y1;
	adjustCoords(adjusted_x, adjusted_y);
	real_scale = s;
	fileBuf.Buf = NULL;
	fileBuf.Size = 0;
	max_box_strings = mbs;
	box_strings = NEW StringList[max_box_strings];
	scrollable = false;
}

BoxText::~BoxText()
{
	if (fileBuf.Buf)
		KSReleaseFile(&fileBuf);
	delete[] box_strings;
}

void BoxText::changePos(float posx, float posy)
{
	float dif_x, dif_y;
/*
	TextString::changePos(posx, posy); 
	adjustCoords(posx, posy);
	dif_x = posx - adjusted_x;
	dif_y = posy - adjusted_y;
	real_x = posx; 
	real_y = posy;
*/

	TextString::changePos(posx, posy);
	real_x = posx;
	real_y = posy;
	adjustCoords(posx, posy);
	dif_x = posx - adjusted_x;
	dif_y = posy - adjusted_y;
	adjusted_x = posx;
	adjusted_y = posy;

	for(int i = 0; i < box_str_count; i++)
	{
		box_strings[i].x += dif_x;
		box_strings[i].y += dif_y;
	}
}


void BoxText::Draw()
{
	if(!checkTime || time > 0)
	{
		int start = 0;
		int end = box_str_count;
		if(scrollable)
		{
			start = first_vis;
			end = first_vis + max_vis_lines;
			if(end > box_str_count) end = box_str_count;
		}
		for(int i=start; i<end; i++)
		{
			StringList* tmp = &box_strings[i];
			if(non_floating_behavior)
			{
				scale = real_scale;
				Render(&tmp->data, tmp->x, tmp->y);
			}
			else
			{
				float x = location_2d[0] - real_x;
				float y = location_2d[1] - real_y;
				if(scale > 0) Render(&tmp->data, tmp->x + x, tmp->y + y);
			}
		}
	}
}

bool BoxText::scroll(bool up, int lines)
{
	if(!scrollable) return false;
	if(up)
	{
		if(first_vis > 0)
		{
			first_vis -= lines;
			float height_diff = box_strings[1].y - box_strings[0].y;
			for(int i=0; i<box_str_count; i++)
				box_strings[i].y += height_diff;
			return true;
		}
		else return false;
	}
	else
	{
		if(first_vis + max_vis_lines < box_str_count)
		{
			first_vis += lines;
			float height_diff = box_strings[1].y - box_strings[0].y;
			for(int i=0; i<box_str_count; i++)
				box_strings[i].y -= height_diff;
			return true;
		}
		else return false;
	}
}

void BoxText::ReadFromFile(char* filename, bool remove_endlines)
{
	bool locked = mem_malloc_locked();
	mem_lock_malloc(false);

	fileBuf.Buf=NULL; fileBuf.Size=0;
	KSReadFile(filename, &fileBuf, 1);
	unsigned char* buffer = (unsigned char*) fileBuf.Buf;
	if(!buffer)
	{
		KSReleaseFile(&fileBuf);
		nglPrintf("File %s is missing\n", filename);
		return;
	}

	buffer[fileBuf.Size] = '\0';	// terminate string, using the extra byte allocated in wds_readfile (dc 05/19/02)
	text = stringx((char*) buffer);
	if(remove_endlines)
	{
		for(int i=0; i<text.length(); i++)
			if(text[i] == '\n') text[i] = ' ';
	}
	KSReleaseFile(&fileBuf);
	mem_lock_malloc(locked);
	MakeReplacements(text);
}

int BoxText::makeBox(int w, int h, bool r, float scale_override, int height_override)
{
	float scale_x = real_scale;
	float scale_y = real_scale;
	if(scale_override != -1)
	{
		scale_x = scale_override;
		scale_y = scale_override;
	}

	width = w;
	height = h;
	adjustSizes(width, height);
	adjustSizes(scale_x, scale_y);
	reverse = r;
	float cur_width = 0;
	int line = 0;
	Font::glyph_info* ci = font->getGlyph('A');
	int line_height = (int) (ci->cell_height*scale_y);
	if(height_override != -1) line_height = height_override;
	float tmp;
//	int end_line = 0;
	stringx current = "";
	char word[30];
	word[0] = '\0';
	int word_index = 0;
	float word_width = 0;
	box_str_count = 0;

	// this is to keep the color codes consistent
	char last_color[2];
	last_color[0] = ' ';
	last_color[1] = '\0';
	bool ignore_next = false;

	for(int i=0; i<text.length(); i++)
	{
		// ignore color codes, which won't appear in the text
		if(ignore_next)
			last_color[0] = (char) text[i];
		else if(text[i] == '@')
			ignore_next = true;

		// end the word on a space (or dash), and add the word since space will permit
		if(text[i] == ' ' || text[i] == '-')
		{
			if (this->hJustify != Font::HORIZJUST_RIGHT)
			{
				word[word_index] = text[i];
				word_index++;
			}
			else 
			{
				current += text[i];
			}
			word[word_index] = '\0';
			current = current + stringx(word);
			word[0] = '\0';
			word_index = 0;
			word_width = 0;
		}
		else if(text[i] == '\r') continue;
		else if(text[i] == '\n')
		{
			// forcibly end the line
			word[word_index] = '\0';
			current = current + stringx(word);
			word[0] = '\0';
			word_index = 0;
			word_width = 0;
			cur_width = 0;

			// if this assertion hits, you need to bump up the max_box_strings
			// passed into the constructor.  The default is 5.
			assert(box_str_count < max_box_strings);
			makeStringList(&box_strings[box_str_count], current, x, y + line*line_height);
			box_str_count++;

			if(last_color[0] != ' ') current = "@"+stringx(last_color);
			else current = "";
			line++;
			if(line*ci->cell_height*scale_y > height)
			{
				// height is currently ignored since no one really used it anyway
				// but if you want to know if it overran look here
			}
			continue;
		}
		else
		{
			word[word_index] = text[i];
			word_index++;
			if(word_index >= 30) assert(0);
			word[word_index] = '\0';
		}

		// store width of current character in tmp
		ci = font->getGlyph(text[i]);
		if(ignore_next || !ci)  // ignore width for @ & following character
		{
			tmp = 0;
			if(text[i] != '@')
				ignore_next = false;
		}
		else
		{
			tmp = ci->cell_width*scale_x;
			word_width += tmp;
		}

		// end the line if no more space
		if(cur_width + tmp > width)
		{
			cur_width = word_width;

			// if this assertion hits, you need to bump up the max_box_strings
			// passed into the constructor.  The default is 5.
			assert(box_str_count < max_box_strings);
			makeStringList(&box_strings[box_str_count], current, x, y + line*line_height);
			box_str_count++;

			if(last_color[0] != ' ') current = "@"+stringx(last_color);
			else current = "";
			line++;
			if(line*ci->cell_height*scale_y > height)
			{
#ifdef BETH
				nglPrintf("BETH: make box overran\n");
#endif
			}
		}
		else
			cur_width += tmp;
	}
	stringx d = current+stringx(word);
	if(d.length() > 0)
	{
		// if this assertion hits, you need to bump up the max_box_strings
		// passed into the constructor.  The default is 5.
		assert(box_str_count < max_box_strings);

		makeStringList(&box_strings[box_str_count], d, x, y + line*line_height);
		box_str_count++;
	}

	if(reverse)
	{
		int num_line = line;
		for(int i=0; i<box_str_count; i++)
		{
			StringList* tmp = &box_strings[i];
			assert(num_line >= 0);
			tmp->y = y - num_line*line_height;
			num_line--;
		}
	}
	return line+1;
}

void BoxText::UpdateInScene(bool ignore_scale)
{
	nglProjectPoint(location_2d, location_3d);

	// the returned value in the wrong coords, must convert to 640x480
	unadjustCoords(location_2d[0], location_2d[1]);

//	changePos(location_2d[0], location_2d[1]);

	// calculate the scale factor
	if(!ignore_scale)
	{
		nglVector temp;
		nglApplyMatrix(temp, *(nglMatrix*)&(geometry_manager::inst()->xforms[geometry_manager::XFORM_WORLD_TO_VIEW]), location_3d);
		float dist = location_2d[2] = temp[2];
		
			// PLEASE don't divide by zero
		if (dist == 0.0f) dist=1.0f;

		scale = real_scale / dist;
	}
}

void BoxText::SetLocation3D(vector3d loc)
{
	location_3d[0] = loc.x;
	location_3d[1] = loc.y;
	location_3d[2] = loc.z;
	location_3d[3] = 1;
}

// ***************** TrickBoxText ***********************

void TrickBoxText::Update(time_value_t time_inc)
{
	RandomText::Update(time_inc);

	if (isRand)
	{
		for(int i = 0; i < box_str_count; i++)
		{
			box_strings[i].Update(time_inc);
		}
	}
}

void TrickBoxText::Draw()
{
	if (!checkTime || time > 0)
	{
		for(int i = 0; i < box_str_count; i++)
		{
			StringList * tmp = &box_strings[i];

			if (override_alpha)
			{
				if (i == 0) color.set_alpha(255);
				else if (i == 1) color.set_alpha(127);
				else if (i == 2) color.set_alpha(63);
				else color.set_alpha(31);
			}

			if (isRand)
				Render(&tmp->data, tmp->x, tmp->y, tmp->delta_x, tmp->delta_y);
			else
				Render(&tmp->data, tmp->x, tmp->y);
		}
	}
}

void TrickBoxText::changePos(float posx, float posy)
{
	float	newx = posx;
	float	newy = posy;

	adjustCoords(newx, newy);

	for(int i=0; i<box_str_count; i++)
	{
		StringList* tmp = &box_strings[i];

		tmp->x += (newx-x);
		tmp->y += (newy-y);
	}

	RandomText::changePos(posx, posy);
}

//	SetSize()
// Changes the width and height of the text box.
void TrickBoxText::SetSize(const int w, const int h)
{
	float	scale_x = scale, scale_y = scale;
	
	// Get hardware-dependent width and height.
	width = w;
	height = h;
	if (width < 0) width = 0;
	if (height < 0) height = 0;
	adjustSizes(width, height);

	// Get hardware-dependent font size.
	adjustSizes(scale_x, scale_y);
	lineHeight = int(font->getGlyph('A')->cell_height*scale_y);
	if (lineHeight == 0) lineHeight = 1;

	// Reposition onscreen text.
	for(int i = 0; i < numLines; i++)
	{
		StringList* tmp = &box_strings[i];

		// X is simple.
		tmp->x = x+(width/2);
		
		// Y is more complicated.
		if (vJustify == Font::VERTJUST_TOP)
			tmp->y = y+i*lineHeight;
		else
			tmp->y = (y+height-lineHeight) - i*lineHeight;
	}
}

//	MakeTrickBox()
// Overwrites the TextString's contents with text describing the specified trick chain.
// The new text is formatted to fit in the specified box.
void TrickBoxText::MakeTrickBox(const ScoringManager::Chain & chain, bool topJustify, bool fade)
{
	ScoringManager::SeriesList::const_reverse_iterator	srit;
	ScoringManager::TrickList::const_reverse_iterator	trit;
	box_str_count = 0;
	stringx				trickText;
	int					trickWidth;
	stringx				lineText;
	int					lineWidth = 0;
	stringx				separatorText(" @3+@n ");
	int					separatorWidth = int((font->getGlyph(' ')->cell_width + font->getGlyph('+')->cell_width + font->getGlyph(' ')->cell_width)*scale);
	char				fadeCode[3] = "@";
	int					rowIdx = 0;
	int					numRows;
	bool				firstTrick = true, done = false;
	int					i;
	u_int				n;
	float				scale_x = scale;
	float				scale_y = scale;

	adjustSizes(scale_x, scale_y);
	lineHeight = int(font->getGlyph('A')->cell_height*scale_y);
	if (lineHeight == 0) lineHeight = 1;

	hJustify = Font::HORIZJUST_CENTER;
	if (topJustify)
		vJustify = Font::VERTJUST_TOP;
	else
		vJustify = Font::VERTJUST_BOTTOM;

	numRows = int(height/lineHeight);

	// Process each trick in the chain.
	for (srit = chain.series.rbegin(); srit != chain.series.rend(); ++srit)
	{
		for (trit = (*srit).tricks.rbegin(), n = (*srit).tricks.size()-1; trit != (*srit).tricks.rend(); ++trit, n--)
		{
			if (!(*trit).IsInteresting())
				continue;

			trickText = "";

			// Distance from mouth of tube changes color of trick.
			if ((*trit).type == ScoringManager::Trick::TYPE_TRICK)
			{
				if ((*trit).mouthDist < 0.333333f)
					trickText += stringx("@1");
				else if ((*trit).mouthDist >= 0.333333f && (*trit).mouthDist <= 0.666666f)
					trickText += stringx("@2");
				else if ((*trit).mouthDist > 0.666666f)
					trickText += stringx("@3");
				else
					assert(false);
			}
			else
				trickText += stringx("@0");

			// Add spin text to trick name.
			if (n == 0)
			{
				if ((*srit).numSpins >= 8)
					trickText += "1440 ";
				else if ((*srit).numSpins >= 7)
					trickText += "1260 ";
				else if ((*srit).numSpins >= 6)
					trickText += "1080 ";
				else if ((*srit).numSpins >= 5)
					trickText += "900 ";
				else if ((*srit).numSpins >= 4)
					trickText += "720 ";
				else if ((*srit).numSpins >= 3)
					trickText += "540 ";
				else if ((*srit).numSpins >= 2)
					trickText += "360 ";
				else if ((*srit).numSpins >= 1)
					trickText += "180 ";
			}

			// Get primary trick name.
			trickText += (*trit).GetText();

			// Add from floater text at the end of trick name.
			if (n == (*srit).tricks.size()-1 && ((*srit).flags & (1 << ScoringManager::MOD_FROM_FLOATER)))
				trickText += " " + ksGlobalTextArray[GT_TRICK_FROM_FLOATER];

			// Add fakey text at the end of trick name.
			if (n == (*srit).tricks.size()-1 && ((*srit).flags & (1 << ScoringManager::MOD_TO_FAKEY)))
				trickText += " " + ksGlobalTextArray[GT_TRICK_FAKEY];

			// Calculate width in pixels of trick text.
			trickWidth = 0;
			for (i = 0; i < trickText.size(); i++)
			{
				// Ignore color codes.
				if (trickText[i] == '@' && i < trickText.size()-1)
				{
					if (trickText[i+1] != '@')
						i++;
					continue;
				}

				trickWidth += int(font->getGlyph(trickText[i])->cell_width*scale_x);
			}

			// First trick fits on this line.
			if (firstTrick && (lineWidth + trickWidth <= width))
			{
				lineText = trickText;
				lineWidth = trickWidth;
				firstTrick = false;
			}
			// Trick fits on this line.
			else if (lineWidth + separatorWidth + trickWidth <= width)
			{
				lineText = trickText + separatorText + lineText;
				lineWidth += separatorWidth + trickWidth;
			}
			// Trick does not fit on this line.
			else
			{
				// If we have room for one more row, add trick and separator to next line.
				if (rowIdx < numRows-1 && (trickWidth + separatorWidth <= width))
				{
					// Add fade code.
					if (fade) fadeCode[1] = char(rowIdx+1);
					else fadeCode[1] = char(1);
					lineText = fadeCode + lineText;
					
					if (topJustify)
						makeStringList(&box_strings[box_str_count], lineText, x+(width/2), y+rowIdx*lineHeight);
					else
						makeStringList(&box_strings[box_str_count], lineText, x+(width/2), (y+height-lineHeight) - rowIdx*lineHeight);
					box_str_count++;
					assert(box_str_count <= MAX_BOX_LINES);	
					
					// Advance to next row.
					rowIdx++;
					lineText = trickText + separatorText;
					lineWidth = trickWidth + separatorWidth;
					
				}
				// Final row: must truncate remaining tricks as an ellipses.
				else
				{
					// Add line text to string list.
					if (!firstTrick)
						lineText = stringx("@1... +@n ") + lineText;
					else
						lineText = stringx("@1...");

					// Add fade code.
					if (fade) fadeCode[1] = char(rowIdx+1);
					else fadeCode[1] = char(1);
					lineText = fadeCode + lineText;

					if(topJustify)
						makeStringList(&box_strings[box_str_count], lineText, x+(width/2), y+rowIdx*lineHeight);
					else
						makeStringList(&box_strings[box_str_count], lineText, x+(width/2), (y+height-lineHeight) - rowIdx*lineHeight);
					box_str_count++;
					assert(box_str_count <= MAX_BOX_LINES);

					// Done.
					lineText = "";
					lineWidth = 0;
					done = true;
					break;
				}
			}
		}
		if (done)
			break;
	}

	// Add last line if necessary.
	if (!lineText.empty())
	{
		// Add fade code.
		if (fade) fadeCode[1] = char(rowIdx+1);
		else fadeCode[1] = char(1);
		lineText = fadeCode + lineText;

		/*
		// Add line text to string list.
		if (topJustify)
			*currStringList = makeStringList(lineText, x+(width/2), y+rowIdx*lineHeight);
		else
			*currStringList = makeStringList(lineText, x+(width/2), (y+height-lineHeight) - rowIdx*lineHeight);
		currStringList = &((*currStringList)->next);
*/

		if(topJustify)
			makeStringList(&box_strings[box_str_count], lineText, x+(width/2), y+rowIdx*lineHeight);
		else
			makeStringList(&box_strings[box_str_count], lineText, x+(width/2), (y+height-lineHeight) - rowIdx*lineHeight);
		box_str_count++;
		assert(box_str_count <= MAX_BOX_LINES);

	}

	numLines = rowIdx+1;

	// Reverse order of strings for top justification.
	if (topJustify)
	{
		int num_line = rowIdx;
		/*
		StringList* tmp = head;
		while (tmp)
		{
			tmp->y = y + num_line*lineHeight;
			num_line--;
			tmp = tmp->next;
		}
		*/

		for(int i=0; i<box_str_count; i++)
		{
			StringList* tmp = &box_strings[i];
			tmp->y = y + num_line*lineHeight;
			num_line--;
		}
	}

	text = "";
}

void TrickBoxText::makeRand()
{
	for (int i = 0; i < box_str_count; i++)
		box_strings[i].MakeRand();

	time = DISPLAY_MAX;
	checkTime = true;
	isRand = true;
}


void TrickBoxText::Break(void)
{
	if (!isRand)
		makeRand();

	for (int i = 0; i < box_str_count; i++)
		box_strings[i].Break();
}

// ***************** BurstText ***********************

float BurstText::TARGET_SCALE = 2.0f;
float BurstText::TARGET_SCALE_TIME = 1.0f;
float BurstText::TARGET_ALPHA_TIME = 1.0f;

BurstText::BurstText()
{	
	scale = 1.0f;
	targetScale = 1.0f;
	scaleRate = 1.0f;
	override_alpha = true;
}

void BurstText::Update(time_value_t time_inc)
{
	//int	i;
	
	TextString::Update(time_inc);

	if (scale < targetScale)
	{
		scale += time_inc/scaleRate;
		if (scale >= targetScale)
			scale = targetScale;
	}
}

void BurstText::Burst(TextString * orig)
{
	text = orig->getText();
	time = orig->time;
	checkTime = orig->checkTime;
	x = orig->getX();
	y = orig->getY();
	adjustCoords(x, y);
	z = orig->getZ();
	button_scale = orig->GetButtonScale();
	no_color = orig->no_color;
	color = orig->color;
	font = orig->GetFont();
	even_number_spacing = orig->GetNumberSpacing();
	hJustify = orig->GetHJustify();
	vJustify = orig->GetVJustify();

	ChangeFade(true, false, TARGET_ALPHA_TIME);
	scale = orig->GetScale();
	targetScale = scale*TARGET_SCALE;
	scaleRate = TARGET_SCALE_TIME;
}

// ***************** BurstTrickText ***********************

float BurstTrickText::TARGET_SCALE = 2.0f;
float BurstTrickText::TARGET_SCALE_TIME = 1.0f;
float BurstTrickText::TARGET_ALPHA_TIME = 1.0f;

BurstTrickText::BurstTrickText()
{	
	scale = 1.0f;
	targetScale = 1.0f;
	scaleRate = 1.0f;
}

void BurstTrickText::Update(time_value_t time_inc)
{
	int	i;
	
	TrickBoxText::Update(time_inc);

	if (scale < targetScale)
	{
		scale += time_inc/scaleRate;
		if (scale >= targetScale)
			scale = targetScale;

		for (i = 0; i < numLines; i++)
		{
			box_strings[i].y = origY[i] + ((float(origLineHeight)*scale)/2.0f - float(origLineHeight)/2.0f)/2.0f;
		}
	}
}

void BurstTrickText::Burst(TrickBoxText * orig)
{
	int	i;

	numLines = box_str_count = orig->GetNumLines();
	for (i = 0; i < numLines; i++)
	{
		box_strings[i] = orig->GetLine(i);
		origY[i] = box_strings[i].y;
	}

	origLineHeight = orig->GetLineHeight();
	override_alpha = true;
	z = orig->getZ();
	time = orig->time;
	checkTime = orig->checkTime;
	no_color = true;
	color = color32(200, 200, 200, 255);
	font = orig->GetFont();
	button_scale = orig->GetButtonScale();
	even_number_spacing = orig->GetNumberSpacing();
	hJustify = orig->GetHJustify();
	vJustify = Font::VERTJUST_BOTTOM;
	ChangeFade(true, false, TARGET_ALPHA_TIME);

	scale = orig->GetScale();
	targetScale = scale*TARGET_SCALE;
	scaleRate = TARGET_SCALE_TIME;
}

// ***************** PreformatText **********************

void PreformatText::Draw()
{
	if((!checkTime || time > 0) && file_head)
		for(int i=0; i<num_vis_lines; i++)
			DrawLine(i+start_line, x, y+19*i);
}

void PreformatText::DrawLine(int line_num, float x1, float y1)
{
	Render(&file_head[line_num], x1, y1);
}

void PreformatText::readText(char* filename, int ml, int vis_lines)
{
	max_lines = ml;
	num_vis_lines = vis_lines;
	start_line = 0;
	file_head = NEW stringx[max_lines];
	assert(max_lines <= absolute_limit);
	const int line_size = 300;
	char line[line_size];
	u_int cur_index = 0;
	int line_index = 0, file_index = 0;

	nglFileBuf file;
	file.Buf=NULL; file.Size=0;
	KSReadFile(filename, &file, 1);
	unsigned char* buffer = (unsigned char*) file.Buf;
	if(!buffer)
	{
		KSReleaseFile(&file);
		assert(0);
	}

	buffer[file.Size] = '\0';	// terminate string, using the extra byte allocated in wds_readfile (dc 05/19/02)

	// assume every lines ends with \r\n
	while(cur_index < file.Size && buffer[cur_index] != '\x04')
	{
		line_index = 0;
		while(buffer[cur_index] != '\n' && buffer[cur_index] != '\r' && buffer[cur_index] != '\x04')
			if(line_index < line_size)
				line[line_index++] = buffer[cur_index++];
			else { line_index++; cur_index++; }

		if(buffer[cur_index] == '\r' && buffer[cur_index+1] == '\n')
			cur_index++;	// skip over the \n as well
		cur_index++;

		if(line_index >= line_size)
		{
			line[line_size-1] = '\0';
#ifdef BETH
			nglPrintf("BETH: line index %d overrun on %s\n", line_index, line);
			assert(0);
#endif
		}
		else
			line[line_index] = '\0';
		if(file_index >= max_lines)
			file_index++;
		else file_head[file_index++] = stringx(line);
	}

	if(file_index >= max_lines)
	{
#ifdef BETH
		nglPrintf("BETH: lines %d has overrun max_lines %d\n", file_index, max_lines);
		assert(0);
#endif
	}

	KSReleaseFile(&file);

	actual_lines = file_index;

	// make necessary replacements
	for(int i=0; i<actual_lines; i++)
		MakeReplacements(file_head[i]);
}

bool PreformatText::scroll(bool up, int lines)
{
	if(up)
	{
		start_line = start_line - lines;
		if(start_line < 0) start_line = 0;
		return (start_line != 0);
	}
	else
	{
		start_line = start_line + lines;
		if(start_line + num_vis_lines - 1 >= actual_lines)
			start_line = actual_lines - num_vis_lines;
		return (start_line != actual_lines - num_vis_lines);
	}
}

// returns percentage down the page
float PreformatText::getPercentage()
{
	// actually total possible number of start lines
	int total_lines = actual_lines - num_vis_lines;
	return start_line/((float) total_lines);
}


// ***************** FloatingText ***********************

void FloatingText::cons(Font* f, stringx t, Font::HORIZJUST hj, Font::VERTJUST vj, color32 c)
{
	MultiLineString::cons(f, t, 0, 0, 0, 1, hj, vj, false, c);
	location_3d[0] = 0;
	location_3d[1] = 0;
	location_3d[2] = 0;
	location_3d[3] = 1;
	real_x = 0;
	real_y = 0;
	non_floating_behavior = false;
	real_scale = 1;
}

void FloatingText::SetLocation3D(vector3d loc)
{
	location_3d[0] = loc.x;
	location_3d[1] = loc.y;
	location_3d[2] = loc.z;
	location_3d[3] = 1;
}

void FloatingText::UpdateInScene(bool ignore_scale)
{
	nglProjectPoint(location_2d, location_3d);

	// the returned value in the wrong coords, must convert to 640x480
	unadjustCoords(location_2d[0], location_2d[1]);
	changePos(location_2d[0], location_2d[1]);

	// calculate the scale factor
	if(!ignore_scale)
	{
		nglVector temp;
		nglApplyMatrix(temp, *(nglMatrix*)&geometry_manager::inst()->xforms[geometry_manager::XFORM_WORLD_TO_VIEW], location_3d);
		float dist = location_2d[2] = temp[2];

			// PLEASE don't divide by zero
		if (dist == 0.0f) dist=1.0f;
 
		scale = real_scale / dist;
	}
}

void FloatingText::Draw()
{
	#ifdef TARGET_GC
		// the scale of text is not set right on the GC
	#pragma FIXME("The scale of text is not being calculated correctly on the gamecube")
	if ( scale < 0.75f ) scale=0.75f;
	#endif

	if(non_floating_behavior)
	{
		changePos(real_x, real_y);
		scale = real_scale;
	}
	if(scale > 0) MultiLineString::Draw();
}

void FloatingText::SetBehaviorNF(float r_x, float r_y)
{
	non_floating_behavior = true;
	real_x = r_x;
	real_y = r_y;
}

// ***************** Panel Quad *************************

void PanelQuad::cons(stringx n)
{
	name = n;
	isAnim = false;
	dont_draw = false;
	drawOn = true;
	fade = 0;
	layer = 0;
	mask = 1.0f;
	maskType = 0;
	next = NULL;
	on_menu = false;
	rotate = 0;
	width = 0;
	height = 0;
	clip = false;
	clipping.tl.x = 0;
	clipping.tl.y = 0;
	clipping.br.x = 639;
	clipping.br.y = 479;
	nglInitQuad(&quad);
#if defined(TARGET_XBOX)
	Xform.identity();
#endif /* TARGET_XBOX JIV DEBUG */
}

void PanelQuad::Init(float xa, float ya, float xb, float yb,
					 float r1, float g1, float b1, float a1,
					 float ua, float va, float ub, float vb,
					 float z1, matrix4x4 obj)
{
	r = r1; g = g1; b = b1; a = a1;
	u1 = ua; v1 = va; u2 = ub; v2 = vb;
	matrix = obj;
	vector3d vec1 = xform3d_1_homog(matrix, vector3d(xa, ya, z1));
	vector3d vec2 = xform3d_1_homog(matrix, vector3d(xb, yb, z1));
	x1 = vec1.x; x2 = vec2.x; y1 = vec1.y; y2 = vec2.y;
	z = vec1.z;
	width_a = x2-x1;
	height_a = y2-y1;
	adjustCoords(x1, y1);
	adjustCoords(x2, y2);
	width = x2-x1;
	height = y2-y1;

	nglSetQuadRect(&quad, x1, y1, x2, y2);
	nglSetQuadColor(&quad, NGL_RGBA32(FTOI(r*255.0f), FTOI(g*256.0f), FTOI(b*255.0f), FTOI(a*255.0f)));
	nglSetQuadUV(&quad, u1, v1, u2, v2);
	nglSetQuadZ(&quad, z);
	nglSetQuadMapFlags(&quad, NGLMAP_CLAMP_U | NGLMAP_CLAMP_V | NGLMAP_BILINEAR_FILTER);
}

//	SetFade()
// Sets the alpha value of the panel quad.
// Unlike ChangeFade(), this value does not change over time.
void PanelQuad::SetFade(const float amt)
{
	fade = 2;
	fade_alpha = amt;
}

// start (as opposed to stop) & fade_in (as opposed to fade out)
void PanelQuad::ChangeFade(bool start, bool fade_in, float time)
{
	if(start)
	{
		fade_timer = time;
		if(fade_in)
		{
			if(fade != 1 || drawOn == false)  // don't restart fade if already started
			{
				fade = 1;
				fade_alpha = 0.0f;
				drawOn = true;
			}
		}
		else
		{
			if(fade != -1)
			{
				fade = -1;
				fade_alpha = 1.0f;
			}
		}
	}
	else fade = 0;
}

void PanelQuad::Update(time_value_t time_inc)
{
	// Fade in.
	if(fade == 1)
	{
		fade_alpha += (time_inc/fade_timer)*a;
		if(fade_alpha >= a)
			fade = 0;
	}
	// Fade out.
	else if(fade == -1)
	{
		fade_alpha -= (time_inc/fade_timer)*a;
		if(fade_alpha <= 0.0f)
		{
			fade = 0;
			drawOn = false;
		}
	}
	// Constant fade.
	else if(fade == 2)
	{

	}

	if (maskType != 0)
	{
		if (mask > 1.0f)
			mask = 2.0f - mask;

		if (maskType == 1)
		{
			float tmp_x = (x2-x1)*mask + x1;
			float tmp_u = (u2-u1)*mask + u1;
			nglSetQuadRect(&quad, x1, y1, tmp_x, y2);
			nglSetQuadUV(&quad, u1, v1, tmp_u, v2);
		}
		else if (maskType == 2)
		{
			float tmp_x = x2 - (x2-x1)*mask;
			float tmp_u = u2 - (u2-u1)*mask;
			nglSetQuadRect(&quad, tmp_x, y1, x2, y2);
			nglSetQuadUV(&quad, tmp_u, v1, u2, v2);
		}
		else if (maskType == 3)
		{
			float tmp_y = (y1-y2)*mask + y2;
			float tmp_v = (v1-v2)*mask + v2;
			nglSetQuadRect(&quad, x1, tmp_y, x2, y2);
			nglSetQuadUV(&quad, u1, tmp_v, u2, v2);
		}

		if (rotate == 1)
			nglRotateQuad(&quad, (x2-x1)/2.0f+x1, (y2-y1)/2.0f+y1, rotation);
		if (rotate == 2)
			nglRotateQuad(&quad, rotate_x, rotate_y, rotation);
	}
	/*
	else
	{
		nglSetQuadRect(&quad, x1, y1, x2, y2);
		nglSetQuadUV(&quad, u1, v1, u2, v2);
	}
	*/
}

void PanelQuad::Draw(int current_layer, float alpha)
{
	if(drawOn &&
	   ((current_layer == -1 && on_menu) ||
	    (!dont_draw && current_layer == layer && !on_menu)))
	{
		if(fade != 0)
			nglSetQuadColor(&quad, NGL_RGBA32(FTOI(r*255.0f), FTOI(g*255.0f), FTOI(b*255.0f), FTOI(fade_alpha*255.0f)));
		else if(alpha != -1.0f)
			nglSetQuadColor(&quad, NGL_RGBA32(FTOI(r*255.0f), FTOI(g*255.0f), FTOI(b*255.0f), FTOI(alpha*255.0f)));
		else
			nglSetQuadColor(&quad, NGL_RGBA32(FTOI(r*255.0f), FTOI(g*255.0f), FTOI(b*255.0f), FTOI(a*255.0f)));

		if (isAnim)
			Animate();

		if (!clip)
  			nglListAddQuad(&quad);
		else
		{
			// Save old quad.
			float	x1 = quad.Verts[0].X;
			float	y1 = quad.Verts[0].Y;
			float	x2 = quad.Verts[3].X;
			float	y2 = quad.Verts[3].Y;
			float	u1 = quad.Verts[0].U;
			float	v1 = quad.Verts[0].V;
			float	u2 = quad.Verts[3].U;
			float	v2 = quad.Verts[3].V;
			float	uScale = (quad.Verts[3].U-quad.Verts[0].U)/float(x2-x1);
			float	vScale = (quad.Verts[3].V-quad.Verts[0].V)/float(y2-y1);

			// Clip coordinates.
			if (quad.Verts[0].X < clipping.tl.x)
			{
				quad.Verts[0].X = clipping.tl.x;
				quad.Verts[2].X = clipping.tl.x;
				quad.Verts[0].U += float(quad.Verts[0].X-x1)*uScale;
				quad.Verts[2].U += float(quad.Verts[0].X-x1)*uScale;
			}
			if (quad.Verts[3].X > clipping.br.x)
			{
				quad.Verts[1].X = clipping.br.x;
				quad.Verts[3].X = clipping.br.x;
				quad.Verts[1].U -= float(x2-quad.Verts[1].X)*uScale;
				quad.Verts[3].U -= float(x2-quad.Verts[1].X)*uScale;
			}
			if (quad.Verts[0].Y < clipping.tl.y)
			{
				quad.Verts[0].Y = clipping.tl.y;
				quad.Verts[1].Y = clipping.tl.y;
				quad.Verts[0].V += float(quad.Verts[0].Y-y1)*vScale;
				quad.Verts[1].V += float(quad.Verts[0].Y-y1)*vScale;
			}
			if (quad.Verts[3].Y > clipping.br.y)
			{
				quad.Verts[1].Y = clipping.br.y;
				quad.Verts[3].Y = clipping.br.y;
				quad.Verts[1].V -= float(y2-quad.Verts[3].Y)*vScale;
				quad.Verts[3].V -= float(y2-quad.Verts[3].Y)*vScale;
			}

			// Draw clipped quad.
			nglListAddQuad(&quad);

			// Reset old quad.
			nglSetQuadRect(&quad, x1, y1, x2, y2);
			nglSetQuadUV(&quad, u1, v1, u2, v2);
		}
	}
}

void PanelQuad::Animate()
{
#if defined(TARGET_XBOX)
	assert(Xform.is_valid());
#endif

	// this code only transforms the 2 vertices used earlier
	vector3d wed[2];
	float diff_x = (x2-x1)/2.0f;
	float diff_y = (y2-y1)/2.0f;
	wed[0] = xform3d_1_homog(Xform, vector3d(-diff_x, -diff_y, z));
	wed[1] = xform3d_1_homog(Xform, vector3d(diff_x, diff_y, z));
	nglSetQuadRect(&quad, wed[0].x, wed[0].y, wed[1].x, wed[1].y);
}

void PanelQuad::Rotate(float x, float y, float r)
{
	rotate_x = x; rotate_y = y;
	adjustCoords(rotate_x, rotate_y);
	rotation = r;
	rotate = 2;

	if (rotate == 1)
		nglRotateQuad(&quad, (x2-x1)/2.0f+x1, (y2-y1)/2.0f+y1, rotation);
	if (rotate == 2)
		nglRotateQuad(&quad, rotate_x, rotate_y, rotation);
}

void PanelQuad::SetPos(float xa, float ya, float xb, float yb)
{
	x1=xa; y1=ya; x2=xb; y2=yb;
	adjustCoords(x1, y1);
	adjustCoords(x2, y2);

	width = x2-x1;
	height = y2-y1;
	nglSetQuadRect(&quad, x1, y1, x2, y2);

	/*
	if (rotate == 1)
		nglRotateQuad(&quad, (x2-x1)/2.0f+x1, (y2-y1)/2.0f+y1, rotation);
	if (rotate == 2)
		nglRotateQuad(&quad, rotate_x, rotate_y, rotation);
	*/
}

// ignore 2nd & 4th pairs
void PanelQuad::SetPos(float xa, float ya, float xb, float yb, float xc, float yc, float xd, float yd)
{
	SetPos(xa, ya, xc, yc);
}

// upper-left corner
void PanelQuad::SetPos(float xa, float ya)
{
	x1 = xa;
	y1 = ya;
	adjustCoords(x1, y1);

	x2 = x1+width-1;
	y2 = y1+height-1;
	nglSetQuadRect(&quad, x1, y1, x2, y2);
	
	/*
	if (rotate == 1)
		nglRotateQuad(&quad, (x2-x1)/2.0f+x1, (y2-y1)/2.0f+y1, rotation);
	if (rotate == 2)
		nglRotateQuad(&quad, rotate_x, rotate_y, rotation);
	*/
}

void PanelQuad::SetCenterX(float x)
{
	float y = 0;	// unused
	adjustCoords(x, y);
	float halfx = width/2.0f;
	x1 = x-halfx;
	x2 = x+halfx;
	nglSetQuadRect(&quad, x1, y1, x2, y2);

	/*
	if (rotate == 1)
		nglRotateQuad(&quad, (x2-x1)/2.0f+x1, (y2-y1)/2.0f+y1, rotation);
	if (rotate == 2)
		nglRotateQuad(&quad, rotate_x, rotate_y, rotation);
	*/
}

void PanelQuad::SetCenterY(float y)
{
	float x = 0;	// unused
	adjustCoords(x, y);
	float halfy = height/2.0f;
	y1 = y-halfy;
	y2 = y+halfy;
	nglSetQuadRect(&quad, x1, y1, x2, y2);

	/*
	if (rotate == 1)
		nglRotateQuad(&quad, (x2-x1)/2.0f+x1, (y2-y1)/2.0f+y1, rotation);
	if (rotate == 2)
		nglRotateQuad(&quad, rotate_x, rotate_y, rotation);
	*/
}

void PanelQuad::SetCenterPos(float cx, float cy)
{
	adjustCoords(cx, cy);

	x1 = cx-(width/2.0f);
	y1 = cy-(height/2.0f);
	x2 = cx+(width/2.0f);
	y2 = cy+(height/2.0f);
	nglSetQuadRect(&quad, x1, y1, x2, y2);

	/*
	if (rotate == 1)
		nglRotateQuad(&quad, (x2-x1)/2.0f+x1, (y2-y1)/2.0f+y1, rotation);
	if (rotate == 2)
		nglRotateQuad(&quad, rotate_x, rotate_y, rotation);
	*/
}

void PanelQuad::SetCenterPosQuadOnly(float cx, float cy)
{
	adjustCoords(cx, cy);
	float xa, ya, xb, yb;
	xa = cx-((x2-x1)/2.0f);
	ya = cy-((y2-y1)/2.0f);
	xb = cx+((x2-x1)/2.0f);
	yb = cy+((y2-y1)/2.0f);
	nglSetQuadRect(&quad, xa, ya, xb, yb);
}

// returns the 640x480 coords
void PanelQuad::GetPos(float &xa, float &ya, float &xb, float &yb)
{
	xa=x1; ya=y1; xb=x2; yb=y2;
	unadjustCoords(xa, ya);
	unadjustCoords(xb, yb);
}

void PanelQuad::GetCenterPos(float & cx, float & cy)
{
	cx = x1+(width/2.0f);
	cy = y1+(height/2.0f);

	unadjustCoords(cx, cy);
}

// Turns clipping on and clips to the specified bounds.
void PanelQuad::SetClip(const recti & bounds)
{
	clip = true;
	clipping = bounds;
	adjustCoords(clipping.tl.x, clipping.tl.y);
	adjustCoords(clipping.br.x, clipping.br.y);
}

// Turns clipping on.
void PanelQuad::SetClip(const bool on)
{
	clip = on;
}

// *********** PanelQuad4 ***************************

/*
PanelQuad4::PanelQuad4(stringx n)
{
	name = n;
	dont_draw = false;
	drawOn = true;
	layer = 0;
	next = NULL;
	rotate = 0;
	nglInitQuad(&quad);
}
*/
void PanelQuad4::Init(float xa, float xb, float xc, float xd,
		  float ya, float yb, float yc, float yd,
		  float r1, float g1, float b1, float a1, float z1)

{
	r = r1; g = g1; b = b1; a = a1;
	z = z1;
	x[0] = xa; x[1] = xb; x[2] = xc; x[3] = xd;
	y[0] = ya; y[1] = yb; y[2] = yc; y[3] = yd;
	for(int i=0; i<4; i++)
	{
		adjustCoords(x[i], y[i]);
		nglSetQuadVPos(&quad, i, x[i], y[i]);
	}
	nglSetQuadColor(&quad, NGL_RGBA32(FTOI(r*255.0f), FTOI(g*255.0f), FTOI(b*255.0f), FTOI(a*255.0f)));

	nglSetQuadZ(&quad, z);

	width = x[1]-x[0];
	height = y[2]-y[0];
}


void PanelQuad4::Init(float xa, float xb, float xc, float xd,
		  float ya, float yb, float yc, float yd,
		  float r1, float g1, float b1, float a1, float z1, matrix4x4 obj)

{
	r = r1; g = g1; b = b1; a = a1;
	matrix = obj;
	vector3d vec[4];
	vec[0] = xform3d_1_homog(matrix, vector3d(xa, ya, z1));
	vec[1] = xform3d_1_homog(matrix, vector3d(xb, yb, z1));
	vec[2] = xform3d_1_homog(matrix, vector3d(xc, yc, z1));
	vec[3] = xform3d_1_homog(matrix, vector3d(xd, yd, z1));
	z = vec[0].z;
	for(int i=0; i<4; i++)
	{
		x[i] = vec[i].x;
		y[i] = vec[i].y;
		adjustCoords(x[i], y[i]);
	}
	for(int i=0; i<4; i++)
		nglSetQuadVPos(&quad, i, x[i], y[i]);
 	nglSetQuadColor(&quad, NGL_RGBA32(FTOI(r*255.0f), FTOI(g*255.0f), FTOI(b*255.0f), FTOI(a*255.0f)));

	nglSetQuadZ(&quad, z);

	width = x[1]-x[0];
	height = y[2]-y[0];
}

void PanelQuad4::SetUV(float u1, float u2, float u3, float u4,
					   float v1, float v2, float v3, float v4)
{
	u[0] = u1; u[1] = u2; u[2] = u3; u[3] = u4;
	v[0] = v1; v[1] = v2; v[2] = v3; v[3] = v4;
	for(int i=0; i<4; i++)
		nglSetQuadVUV(&quad, i, u[i], v[i]);
}

// set the x, y coords permanently
void PanelQuad4::RotateOnce(float x1, float y1, float r)
{
	adjustCoords(x1, y1);
	nglRotateQuad(&quad, x1, y1, r);
	for(int i=0; i<4; i++)
	{
		nglQuadVertex* vert = &quad.Verts[i];
		x[i] = vert->X;
		y[i] = vert->Y;
	}
}

void PanelQuad4::SetPos(float xa, float ya, float xb, float yb, float xc, float yc, float xd, float yd)
{
	adjustCoords(xa, ya);
	adjustCoords(xb, yb);
	adjustCoords(xc, yc);
	adjustCoords(xd, yd);
	x[0] = xa;
	y[0] = ya;
	x[1] = xb;
	y[1] = yb;
	x[2] = xc;
	y[2] = yc;
	x[3] = xd;
	y[3] = yd;
	for(int i=0; i<4; i++)
		nglSetQuadVPos(&quad, i, x[i], y[i]);
}

void PanelQuad4::SetCenterPos(float cx, float cy)
{
	adjustCoords(cx, cy);

	x[0] = x[2] = cx-(width/2.0f);
	x[1] = x[3] = cx+(width/2.0f);
	y[0] = y[1] = cy-(height/2.0f);
	y[2] = y[3] = cy+(height/2.0f);

	for(int i=0; i<4; i++)
		nglSetQuadVPos(&quad, i, x[i], y[i]);
}

void PanelQuad4::GetCenterPos(float & cx, float & cy)
{
	cx = x[0]+(width/2.0f);
	cy = y[0]+(height/2.0f);

	unadjustCoords(cx, cy);
}

// Toby added this.
void PanelQuad4::Rotate(float rx, float ry, float r)
{
	PanelQuad::Rotate(rx, ry, r);

	for(int i=0; i<4; i++)
		nglSetQuadVPos(&quad, i, x[i], y[i]);
}

void PanelQuad4::Update(time_value_t time_inc)
{
	for(int i=0; i<4; i++)
	{
		nglSetQuadVPos(&quad, i, x[i], y[i]);
		nglSetQuadVUV(&quad, i, u[i], v[i]);
	}

	// Fade in.
	if(fade == 1)
	{
		fade_alpha += (time_inc/fade_timer)*a;
		if(fade_alpha >= a)
			fade = 0;
	}
	// Fade out.
	else if(fade == -1)
	{
		fade_alpha -= (time_inc/fade_timer)*a;
		if(fade_alpha <= 0.0f)
		{
			fade = 0;
			drawOn = false;
		}
	}
	// Constant fade.
	else if(fade == 2)
	{

	}

	if (mask != 1.0f)
	{
		if (mask > 1.0f)
			mask = 2.0f - mask;

		if (maskType == 1)
		{
			float tmp_x = (x2-x1)*mask + x1;
			float tmp_u = (u2-u1)*mask + u1;
			nglSetQuadRect(&quad, x1, y1, tmp_x, y2);
			nglSetQuadUV(&quad, u1, v1, tmp_u, v2);
		}
		else if (maskType == 2)
		{
			float tmp_x = x2 - (x2-x1)*mask;
			float tmp_u = u2 - (u2-u1)*mask;
			nglSetQuadRect(&quad, tmp_x, y1, x2, y2);
			nglSetQuadUV(&quad, tmp_u, v1, u2, v2);
		}
		else if (maskType == 3)
		{
			float tmp_y = (y1-y2)*mask + y2;
			float tmp_v = (v1-v2)*mask + v2;
			nglSetQuadRect(&quad, x1, tmp_y, x2, y2);
			nglSetQuadUV(&quad, u1, tmp_v, u2, v2);
		}

		// rotate == 1 is not supported here.
		if(rotate == 2)
			nglRotateQuad(&quad, rotate_x, rotate_y, rotation);
	}
}

void PanelQuad4::Draw(int current_layer, float alpha)
{
	if(drawOn &&
	   ((current_layer == -1 && on_menu) ||
	    (!dont_draw && current_layer == layer && !on_menu)))
	{
		if(fade != 0)
			nglSetQuadColor(&quad, NGL_RGBA32(FTOI(r*255.0f), FTOI(g*255.0f), FTOI(b*255.0f), FTOI(fade_alpha*255.0f)));
		else if(alpha != -1.0f)
			nglSetQuadColor(&quad, NGL_RGBA32(FTOI(r*255.0f), FTOI(g*255.0f), FTOI(b*255.0f), FTOI(alpha*255.0f)));
		else
			nglSetQuadColor(&quad, NGL_RGBA32(FTOI(r*255.0f), FTOI(g*255.0f), FTOI(b*255.0f), FTOI(a*255.0f)));

		nglListAddQuad(&quad);
	}
}

// Toby added this.   Booyah!
void PanelQuad4::SetVertices(const float nx[4], const float ny[4])
{
	for (int i = 0; i < 4; i++)
	{
		x[i] = nx[i];
		y[i] = ny[i];

		nglSetQuadVPos(&quad, i, x[i], y[i]);
	}
}

// *********** Floating PQ **************************

FloatingPQ::FloatingPQ(float _r, float _g, float _b, float _a, int _x1, int _y1, int _x2, int _y2, int _z, HJust _hj)
{
	cons("");
	r = _r; g = _g; b = _b; a = _a;
	z = _z;
	x1 = _x1; y1 = _y1; x2 = _x2; y2 = _y2;
	x1_const = x1;
	y1_const = y1;
	x2_const = x2;
	y2_const = y2;
	width = width_f = x2-x1;
	height = height_f = y2-y1;
	matrix = matrix4x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
	u1 = 0; v1 = 0; u2 = 0; v2 = 0;
	h_just = _hj;
	nglSetQuadRect(&quad, x1, y1, x2, y2);
	nglSetQuadColor(&quad, (u_int)FTOI((255.0f*r+1))>>1 | ((u_int)FTOI((255.0f*g+1))>>1) << 8 | ((u_int)FTOI((255.0f*b+1))>>1) << 16 | ((u_int)FTOI((255.0f*a+1))>>1)<<24);
	nglSetQuadZ(&quad, z);
	non_floating_behavior = false;
}

FloatingPQ::FloatingPQ(const PanelQuad* pq, HJust hj)
{
	isAnim = pq->isAnim;
	fade = pq->fade;
	fade_alpha = pq->fade_alpha;
	mask = pq->mask;
	maskType = pq->maskType;
	quad = pq->quad;
	drawOn = pq->drawOn;
	rotate = pq->rotate;
	rotation = pq->rotation;
	rotate_x = pq->rotate_x;
	rotate_y = pq->rotate_y;
	layer = pq->layer;
	r = pq->r;
	g = pq->g;
	b = pq->b;
	a = pq->a;
	x1 = pq->x1;
	y1 = pq->y1;
	x2 = pq->x2;
	y2 = pq->y2;
	x1_const = x1;
	y1_const = y1;
	x2_const = x2;
	y2_const = y2;
	width = width_f = x2-x1;
	height = height_f = y2-y1;
	matrix = pq->matrix;
	on_menu = pq->on_menu;
	Xform = pq->Xform;
	name = pq->name;
	dont_draw = pq->dont_draw;
	z = pq->z;
	u1 = pq->u1;
	v1 = pq->v1;
	u2 = pq->u2;
	v2 = pq->v2;
	h_just = hj;
	next = NULL;
	non_floating_behavior = false;
}

void FloatingPQ::Init(float xa, float ya, float xb, float yb, 
				  float r1, float g1, float b1, float a1, 
				  float ua, float va, float ub, float vb, 
				  float z1, matrix4x4 obj)
{
	PanelQuad::Init(xa, ya, xb, yb, r1, g1, b1, a1, ua, va, ub, vb, z1, obj);
	width_f = width_a;
	height_f = height_a;
	x1_const = x1;
	y1_const = y1;
	x2_const = x2;
	y2_const = y2;
	unadjustCoords(x1_const, y1_const);
	unadjustCoords(x2_const, y2_const);
	non_floating_behavior = false;
}

void FloatingPQ::UpdateInScene()
{
	nglProjectPoint(location_2d, location_3d);
	
  // the returned value in the wrong coords, must convert to 640x480
	unadjustCoords(location_2d[0], location_2d[1]);
	
	// calculate the scale factor
	nglVector temp;
	nglApplyMatrix(temp, *(nglMatrix*)&geometry_manager::inst()->xforms[geometry_manager::XFORM_WORLD_TO_VIEW], location_3d);
	float dist = location_2d[2] = temp[2];
	
	// PLEASE don't divide by zero
	if (dist == 0.0f) dist=1.0f;
	
	scale = 1.0f / dist;
	SetScale(scale);
}

void FloatingPQ::SetScale(float s)
{
	float tmpx = s*width_f/2.0f;
	float tmpy = s*height_f/2.0f;
	if(h_just == Center)
		PanelQuad::SetPos(location_2d[0] - tmpx, location_2d[1] - tmpy, location_2d[0] + tmpx, location_2d[1] + tmpy);
	else if(h_just == Left)
		PanelQuad::SetPos(location_2d[0], location_2d[1]-tmpy, location_2d[0]+2*tmpx, location_2d[1]+tmpy);
	else PanelQuad::SetPos(location_2d[0]-2*tmpx, location_2d[1]-tmpy, location_2d[0]+tmpx, location_2d[1]+tmpy);
}

void FloatingPQ::SetConstantScale(float s)
{
	width_f = width_f*s;
	height_f = height_f*s;
	SetScale(s);
}

void FloatingPQ::Draw(int layer, float alpha)
{
//	if(non_floating_behavior)
//		PanelQuad::SetPos(x1_const, y1_const, x2_const, y2_const);
	if(scale > 0 || non_floating_behavior) PanelQuad::Draw(layer, alpha);
}

void FloatingPQ::SetPos(float xa, float ya, float xb, float yb)
{
	x1_const = xa;
	x2_const = xb;
	y1_const = ya;
	y2_const = yb;
	width_f = xb - xa;
	height_f = yb - ya;
	PanelQuad::SetPos(xa, ya, xb, yb);
}

void FloatingPQ::SetBehaviorNF(float x, float y)
{
	float tmpx = width_f/2.0f;
	float tmpy = height_f/2.0f;
	x1_const = x - tmpx;
	y1_const = y - tmpy;
	x2_const = x + tmpx;
	y2_const = y + tmpy;
	PanelQuad::SetPos(x1_const, y1_const, x2_const, y2_const);
	non_floating_behavior = true;
}

void FloatingPQ::SetBehavior(bool nfb)
{
	non_floating_behavior = nfb;
	if(nfb) PanelQuad::SetPos(x1_const, y1_const, x2_const, y2_const);
}

void FloatingPQ::SetLocation3D(vector3d loc)
{
	location_3d[0] = loc.x;
	location_3d[1] = loc.y;
	location_3d[2] = loc.z;
	location_3d[3] = 1;
}

// *********** Panel Geom ***************************

PanelGeom* PanelGeom::LoadGeom(unsigned char* buffer, int& index)
{
	unsigned char type = ReadChar(buffer, index);
	switch (type)
	{
		case PanelGeomObject:
		{
			PanelObject* obj=NEW PanelObject;
			if (!obj->Load(buffer, index))
			{
				delete obj;
				return NULL;
			}
			return obj;
		}
		case PanelGeomText:
		{
			PanelText* inst=NEW PanelText;
			if (!inst->Load(buffer, index))
			{
				delete inst;
				return NULL;
			}
			return inst;
		}
		case PanelGeomSkater:
		{
			PanelSkaterModel* inst=NEW PanelSkaterModel;
			if (!inst->Load(buffer, index))
			{
				delete inst;
				return NULL;
			}
			return inst;
		}
		case PanelGeomObjectView:
		{
			PanelObjectModel* inst=NEW PanelObjectModel;
			if (!inst->Load(buffer, index))
			{
				delete inst;
				return NULL;
			}
			return inst;
		}
		case PanelGeomMovie:
		{
			PanelMovie* inst=NEW PanelMovie;
			if (!inst->Load(buffer, index))
			{
				delete inst;
				return NULL;
			}
			return inst;
		}
		default:
			return NULL;
	}
}

void PanelGeom::cons()
{
	children = NULL;
	next = NULL;
	parent = NULL;
}

PanelGeom::~PanelGeom()
{
	PanelGeom* tmp = children;
	PanelGeom* tmp2;
	while(tmp != NULL)
	{
		if(tmp->next != NULL)
			tmp2 = tmp->next;
		else
			tmp2 = NULL;
		delete tmp;
		tmp = tmp2;
	}
}

bool PanelGeom::Load(unsigned char* buffer, int& index)
{
	name = ReadString(buffer, index);
	stringx junkstringx = ReadString(buffer, index);
	ReadMatrix3x4(matrix, buffer, index);
	ReadVector3d(boundboxcenter, buffer, index);
	ReadVector3d(boundboxsize, buffer, index);
	nchildren = ReadLong(buffer, index);
	for(int i=nchildren; --i >=0; )
	{
		PanelGeom* geom = PanelGeom::LoadGeom(buffer, index);
		if(!geom) return false;

		geom->parent = this;
		geom->next = children;
		children = geom;
	}
	return true;
}

void PanelGeom::Reload()
{
	if(children) children->Reload();
}

//  This will find an object with a particular name and return a pointer to it.
//  If this returns NULL to the original caller, then there was no object with
//  that name in the tree.
PanelGeom *PanelGeom::FindObject(const stringx &search_name)
{
  if (name == search_name)
    return this;
  else
  {
    PanelGeom *temp;
    if (children)
    {
      temp = children->FindObject(search_name);
      if (temp && temp->name == search_name)
        return temp;
    }
  }

  return NULL;
}

void PanelGeom::Init(PanelQuad** pquads, bool floating)
{
	if(children) children->Init(pquads, matrix, floating);
}

void PanelGeom::Init(PanelQuad** pquads, matrix4x4 parent_matrix, bool floating)
{
	matrix4x4 this_matrix = parent_matrix*matrix;
	if(children) children->Init(pquads, this_matrix, floating);
}

void PanelGeom::Update(time_value_t time_inc)
{
	if(children) children->Update(time_inc);
}

void PanelGeom::Slide(float offset)
{
	if(children) children->Slide(offset);
}

/************** PanelBatch **************/

PanelBatch::~PanelBatch()
{
	delete[] wedges;
	delete[] colors;
	delete[] tex;
	delete[] didxs;
	delete pq;
}

bool PanelBatch::Load(PanelMaterial* mats, unsigned char* buffer, int& index)
{
	material = ReadShort(buffer, index);
	mat = mats[material];

	nwedges = ReadShort(buffer, index);

	wedges = NEW vector3d[nwedges];
	colors = NEW color32[nwedges];
	tex = NEW vector2d[nwedges];

	for(int i=0; i<nwedges; i++)
	{
		ReadVector3d(wedges[i], buffer, index);
		colors[i] = ReadLong(buffer, index);
		ReadVector2d(tex[i], buffer, index);
	}

	strip_count = ReadShort(buffer, index);
	index_count = ReadShort(buffer, index);

// i have no clue what this didxs is
	didxs = NEW uint16[index_count];
	for(int i=0; i<index_count; i++)
		didxs[i] = ReadShort(buffer, index);

	return true;
}

void PanelBatch::Init(PanelQuad** pquads, stringx name, bool floating)
{
	Init(pquads, name, identity_matrix, floating);
}

bool PanelBatch::check(int x2, int y2)
{
	int x1 = 0;  // always
	int y1 = 0;
	int x3 = (x2 == 1) ? ((x2 == 2) ? 3 : 2) : 1;
	int x4 = (x2 == 2 || x3 == 2) ? 3 : 2;
	int y3 = (y2 == 1) ? ((y2 == 2) ? 3 : 2) : 1;
	int y4 = (y2 == 2 || y3 == 2) ? 3 : 2;
	return (eq_to_tolerance(wedges[x1].x, wedges[x2].x, .01f) &&
			eq_to_tolerance(wedges[x3].x, wedges[x4].x, .01f) &&
			eq_to_tolerance(wedges[y1].y, wedges[y2].y, .01f) &&
			eq_to_tolerance(wedges[y3].y, wedges[y4].y, .01f) &&
			eq_to_tolerance(tex[x1].x, tex[x2].x, .01f) &&
			eq_to_tolerance(tex[x3].x, tex[x4].x, .01f) &&
			eq_to_tolerance(tex[y1].y, tex[y2].y, .01f) &&
			eq_to_tolerance(tex[y3].y, tex[y4].y, .01f));
}

void PanelBatch::Init(PanelQuad** pquads, stringx name, matrix4x4 parent_matrix, bool floating)
{
	// don't know how to draw with less than 4 vertices
	assert(nwedges >= 4);

	if(nwedges == 4)
	{
//		if((eq_to_tolerance(wedges[0].x, wedges[1].x, .01f)
//				&& !eq_to_tolerance(tex[0].x, tex[1].x, .01f)) ||
//		   (eq_to_tolerance(wedges[0].x, wedges[2].x, .01f)
//				&& !eq_to_tolerance(tex[0].x, tex[2].x, .01f)))

		// yes, it's ugly, but it's fairly accurate
		if(!(check(1, 2) || check(1, 3) || check(2, 1) || check(2, 3) || check(3, 1) || check(3, 2)))
		{
			pq = NEW PanelQuad4(name);
			u_int mapflags = 0;

			if(mat.texture == NULL)
				pq->dont_draw = true;
			else
			{
				pq->dont_draw = false;
				if(mat.hasmap) 
				{
					pq->setTexture(mat.texture);
				}
			}

			if (mat.bilinearfilter) 
			{
				mapflags |= NGLMAP_BILINEAR_FILTER;
			}

			if (!mat.wrapu)
			{
				mapflags |= NGLMAP_CLAMP_U;
			}
				
			if (!mat.wrapv)
			{
				mapflags |= NGLMAP_CLAMP_V;
			}
			
      if( mapflags )
			  pq->setMaterialFlags(mapflags);

			float r, b, g, a;

			r = colors[0].get_red()/256.0;
			g = colors[0].get_green()/256.0;
			b = colors[0].get_blue()/256.0;
			a = colors[0].get_alpha()/256.0;
			pq->Init(wedges[0].x, wedges[1].x, wedges[2].x, wedges[3].x,
				wedges[0].y, wedges[1].y, wedges[2].y, wedges[3].y, r, g, b, a, wedges[0].z, parent_matrix);
			pq->SetUV(tex[0].x, tex[1].x, tex[2].x, tex[3].x, tex[0].y, tex[1].y, tex[2].y, tex[3].y);
		}
		else
		{
			if(floating) pq = NEW FloatingPQ(name);
			else pq = NEW PanelQuad(name);
			u_int mapflags = 0;
			if(mat.texture == NULL)
				pq->dont_draw = true;
			else
			{
				pq->dont_draw = false;
				if(mat.hasmap) 
				{
					pq->setTexture(mat.texture);
				}
			}
			if (mat.bilinearfilter) 
			{
				mapflags |= NGLMAP_BILINEAR_FILTER;
			}

			if (!mat.wrapu)
			{
				mapflags |= NGLMAP_CLAMP_U;
			}
				
			if (!mat.wrapv)
			{
				mapflags |= NGLMAP_CLAMP_V;
			}
			if( mapflags )	
			  pq->setMaterialFlags(mapflags);

			float x1, x2, y1, y2, z, u1, u2, v1, v2, r, b, g, a;
			r = colors[0].get_red()/256.0;
			g = colors[0].get_green()/256.0;
			b = colors[0].get_blue()/256.0;
			a = colors[0].get_alpha()/256.0;
			x1 = wedges[0].x;
			y1 = wedges[0].y;
			u1 = tex[0].x;
			v1 = tex[0].y;
			int index = 0;
			for(int i=3; i>=0; i--)
				if(!eq_to_tolerance(x1, wedges[i].x, .01f) &&
				   !eq_to_tolerance(y1, wedges[i].y, .01f))
				   index = i;
			x2 = wedges[index].x;
			y2 = wedges[index].y;
			u2 = tex[index].x;
			v2 = tex[index].y;
			z = wedges[0].z;
			pq->Init(x1, y1, x2, y2, r, g, b, a, u1, v1, u2, v2, z, parent_matrix);
		}
	}
	else
	{
/*
#ifdef BETH
		nglPrintf("BETH: %s nwedges = %d\n", name.data(), nwedges);
		for(int i=0; i<nwedges; i++)
			nglPrintf("BETH: %f %f\n", wedges[i].x, wedges[i].y);
#endif
*/
		pq = NEW PanelQuad(name);
		u_int mapflags = 0;
		if(mat.texture == NULL)
			pq->dont_draw = true;
		else
		{
			pq->dont_draw = false;
			if(mat.hasmap) 
			{
				pq->setTexture(mat.texture);
			}
		}

		if (mat.bilinearfilter) 
		{
			mapflags |= NGLMAP_BILINEAR_FILTER;
		}

		if (!mat.wrapu)
		{
			mapflags |= NGLMAP_CLAMP_U;
		}
			
		if (!mat.wrapv)
		{
			mapflags |= NGLMAP_CLAMP_V;
		}
		
    if( mapflags )
		  pq->setMaterialFlags(mapflags);

		float x1, x2, y1, y2, z, u1, u2, v1, v2, r, b, g, a;
		r = colors[0].get_red()/256.0;
		g = colors[0].get_green()/256.0;
		b = colors[0].get_blue()/256.0;
		a = colors[0].get_alpha()/256.0;

		// this looks for the min & max of x & y.  hopefully, this is what's intended
		int x_lo = 0;
		int x_hi = 0;
		int y_lo = 0;
		int y_hi = 0;
		int u_lo = 0;
		int u_hi = 0;
		int v_lo = 0;
		int v_hi = 0;
		x1 = wedges[x_lo].x;
		x2 = wedges[x_hi].x;
		y1 = wedges[y_lo].y;
		y2 = wedges[y_hi].y;
		u1 = tex[u_lo].x;
		u2 = tex[u_hi].x;
		v1 = tex[v_lo].y;
		v2 = tex[v_hi].y;
		for(int i=1; i<nwedges; i++)
		{
			if(x1 > wedges[i].x) { x_lo = i; x1 = wedges[i].x; }
			if(x2 < wedges[i].x) { x_hi = i; x2 = wedges[i].x; }
			if(y1 > wedges[i].y) { y_lo = i; y1 = wedges[i].y; }
			if(y2 < wedges[i].y) { y_hi = i; y2 = wedges[i].y; }
			if(u1 > tex[i].x) { u_lo = i; u1 = tex[i].x; }
			if(u2 < tex[i].x) { u_hi = i; u2 = tex[i].x; }
			if(v1 > tex[i].y) { v_lo = i; v1 = tex[i].y; }
			if(v2 < tex[i].y) { v_hi = i; v2 = tex[i].y; }
		}
		z = wedges[0].z;

		pq->Init(x1, y1, x2, y2, r, g, b, a, u1, v1, u2, v2, z, parent_matrix);

	}

	if(*pquads)
	{
		if(pq->z > (*pquads)->z)
		{
			pq->next = *pquads;
			*pquads = pq;
		}
		else
		{
			PanelQuad* previous = *pquads;
			PanelQuad* current = previous->next;
			bool end = false;
			while(current && !end)
			{
				if(pq->z > current->z)
				{
					end = true;
					previous->next = pq;
					pq->next = current;
				}
				else
				{
					previous = current;
					current = current->next;
				}
			}
			if(!end)
				previous->next = pq;
		}
	}
	else
		*pquads = pq;
}

void PanelBatch::Reload(PanelMaterial* mats)
{
	mat = mats[material];
	u_int mapflags = 0;
	if(mat.hasmap) 
	{
		pq->setTexture(mat.texture);
	}

	if (mat.bilinearfilter) 
	{
		mapflags |= NGLMAP_BILINEAR_FILTER;
	}

	if (!mat.wrapu)
	{
		mapflags |= NGLMAP_CLAMP_U;
	}
		
	if (!mat.wrapv)
	{
		mapflags |= NGLMAP_CLAMP_V;
	}
	if( mapflags )	
	  pq->setMaterialFlags(mapflags);
}

void PanelBatch::Update(time_value_t time_inc)
{
	pq->Update(time_inc);
}

void PanelBatch::Slide(float offset)
{
	float cx, cy;
	pq->GetCenterPos(cx, cy);
	pq->SetCenterPosQuadOnly(cx+offset, cy);
}

bool PanelBatch::eq_to_tolerance(float a, float b, float tol)
{
	if(a >= b)
		return ((a-b) < tol);
	else return ((b-a) < tol);
}

/************** PanelObject *************/

PanelObject::~PanelObject()
{
	delete[] batches;
	delete[] materials;
}

bool PanelObject::Load(unsigned char* buffer, int& index)
{
	if(!PanelGeom::Load(buffer, index)) return false;

	size = ReadShort(buffer, index);

	materials = NEW PanelMaterial[size];
	for(int i=0; i<size; i++)
	{
		materials[i].texture = NULL;
		PanelFile::ReadPanelMaterial(materials[i], buffer, index);
	}

	nbatches = ReadShort(buffer, index);
	batches = NEW PanelBatch[nbatches];
	for(int i=0; i<nbatches; i++)
		if(!batches[i].Load(materials, buffer, index)) return false;

	return true;
}

void PanelObject::Reload()
{
	// reload textures
	for(int i=0; i<size; i++)
	{
		PanelMaterial &mat = materials[i];
		if(mat.hasmap)
			mat.texture = nglLoadTextureA(mat.filename.data());
		else mat.texture = NULL;
	}
	for(int i=0; i<nbatches; i++)
		batches[i].Reload(materials);
	PanelGeom::Reload();
}

PanelQuad* PanelObject::GetQuad()
{
	if(size > 0)
		return batches[0].pq;
	else return NULL;
}

void PanelObject::Init(PanelQuad** pquads, bool floating)
{
	for(int i=0; i<nbatches; i++)
		batches[i].Init(pquads, name, matrix, floating);
	if(children)
		children->Init(pquads, matrix, floating);
}

void PanelObject::Init(PanelQuad** pquads, matrix4x4 parent_matrix, bool floating)
{
	matrix4x4 this_matrix = parent_matrix*matrix;
	for(int i=0; i<nbatches; i++)
		batches[i].Init(pquads, name, this_matrix, floating);
	if(children)
		children->Init(pquads, this_matrix, floating);
}

void PanelObject::Update(time_value_t time_inc)
{
	for(int i=0; i<nbatches; i++)
		batches[i].Update(time_inc);
	if(children) children->Update(time_inc);
}

void PanelObject::Slide(float offset)
{
	for(int i=0; i<nbatches; i++)
		batches[i].Slide(offset);
	if(children) children->Slide(offset);
}

/************** PanelText ***************/

bool PanelText::Load(unsigned char* buffer, int& index)
{
	if(!PanelGeom::Load(buffer, index)) return false;
	fontname = ReadString(buffer, index);
	color = ReadLong(buffer, index);
	justification = ReadChar(buffer, index);
	linespacing = ReadFloat(buffer, index);
	numtextlines = ReadLong(buffer, index);
	text = ReadString(buffer, index);
	return true;
}

void PanelText::Init(PanelQuad** pquads)
{
	Init(pquads, identity_matrix);
}

void PanelText::Init(PanelQuad** pquads, matrix4x4 parent_matrix)
{
	matrix4x4 this_matrix = parent_matrix*matrix;

	// text needs to be drawn here, with the appropriate matrix
	// right now, an incorrect quad will be drawn
	nglInitQuad(&quad);
	vector3d bbc = boundboxcenter;
	vector3d bbs = boundboxsize;
	nglSetQuadRect(&quad, bbc.x - bbs.x, bbc.y - bbs.y, bbc.x + bbs.x, bbc.y + bbs.y);
	nglSetQuadColor(&quad, NGL_RGBA32(0, 0x64, 0x64, 0));
	nglSetQuadZ(&quad, 1.0f);
//	if(next) next->Init(pquads, parent_matrix);
	if(children) children->Init(pquads, this_matrix);
}

void PanelText::Draw(float alpha)
{
}

/************** PanelFile ***************/

PanelFile::PanelFile(const stringx f, const stringx p)
{
	obs = NULL;
	next = NULL;
	filename = f;
	path = p;
	pquads = NULL;
	slide_state = PF_SLIDE_NONE;
	slide_offset = 0;
	slide_timer = 0;
	slide_max_time = 1.0f;
}

PanelFile::~PanelFile()
{
	PanelGeom* tmp = obs;
	PanelGeom* tmp2;
	while(tmp)
	{
		tmp2 = tmp->next;
		delete tmp;
		tmp = tmp2;
	}
}

void PanelFile::SetFilename(stringx p)
{
	filename = p;
	// delete all objects in anticipation that new objects from
	// the new panel file will be loaded
	PanelGeom* tmp = obs;
	PanelGeom* tmp2;
	while(tmp)
	{
		tmp2 = tmp->next;
		delete tmp;
		tmp = tmp2;
	}
	obs = NULL;
	pquads = NULL;
}

PanelGeom *PanelFile::FindObject(const stringx& search_name)
{
  PanelGeom *temp = NULL;

  PanelGeom* tmp = obs;
  while(tmp)
  {
	  temp = tmp->FindObject(search_name);
	  tmp = tmp->next;
  }
//  if (obs != NULL)
//   temp = obs->FindObject(search_name);

  if(temp == NULL)
  {
	  debug_print("BETH: FindObject failed on %s\n", search_name.data());
#ifdef BETH
	  assert(0);
#endif
  }

  return temp;
}


PanelGeom *PanelFile::FindObject(const char *search_name)
{
  stringx temp(search_name);
  return FindObject(temp);
}

PanelQuad* PanelFile::FindQuad(const stringx& search_name)
{
	PanelGeom* obj = FindObject(search_name);
	if(!obj) return NULL;
	return obj->GetQuad();
}

stringx PanelFile::texture_path = "";
stringx PanelFile::common_path = "";

bool PanelFile::Load(bool floating)
{
	nglFileBuf file;
	file.Buf=NULL; file.Size=0;
	KSReadFile((char *)((path+filename).c_str()), &file, 1);
	unsigned char* buffer = (unsigned char*) file.Buf;
	int index = 0;

	texture_path = os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR)+"\\";
	common_path = "interface\\FE\\common\\"+texture_path;
	texture_path = path+texture_path;
	nglSetTexturePath(texture_path.data());

	if(!buffer)
	{
		KSReleaseFile(&file);
		pquads = NULL;
		return false;
	}
	if(!ReadHeader(buffer, index))
	{
		KSReleaseFile(&file);
		assert(0);
		return false;
	}
//	uint32 version =
		ReadLong(buffer, index);
	uint32 nobjects = ReadLong(buffer, index);
	for(u_int i=0; i<nobjects; i++)
	{
		PanelGeom* geom = PanelGeom::LoadGeom(buffer, index);
		if(!geom)
		{
			KSReleaseFile(&file);
			return false;
		}
		// add geom to the front of the list
		geom->next = obs;
		obs = geom;
	}

	KSReleaseFile(&file);
	Init(floating);
	return true;
}

void PanelFile::Reload()
{
	texture_path = os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR)+"\\";
	common_path = "interface\\FE\\common\\"+texture_path;
	nglSetTexturePath((path+texture_path).data());

	PanelGeom* tmp = obs;
	while(tmp)
	{
		tmp->Reload();
		tmp = tmp->next;
	}
}

// just confirms that the Pnl\0 header leads the file
bool PanelFile::ReadHeader(unsigned char* buffer, int& index)
{

	if(ReadChar(buffer, index) == 'P' &&
	   ReadChar(buffer, index) == 'n' &&
	   ReadChar(buffer, index) == 'l' &&
	   ReadChar(buffer, index) == '\0')
	   return true;
	else return false;
}

void PanelFile::ReadPanelMaterial(PanelMaterial& mat, unsigned char* buffer, int& index)
{
	mat.name = ReadString(buffer, index);
	if (mat.name.size() > 2 && mat.name[0] == 'w' && mat.name[1] == 'i')
	{
		int i;
		i = 2;
	}
	mat.color = ReadLong(buffer, index);
	mat.emissive = ReadFloat(buffer, index);
	mat.shininess = ReadFloat(buffer, index);
	mat.shinestr = ReadFloat(buffer, index);
	mat.additive = ReadChar(buffer, index);
	mat.hasmap = ReadChar(buffer, index);

#if defined(TARGET_XBOX)
#define TEXMAP_EXT      ".dds"
#define TEXMAP_EXT_CAPS ".DDS"
#elif defined(TARGET_GC)
#define TEXMAP_EXT      ".gct"
#define TEXMAP_EXT_CAPS ".GCT"
#elif defined(TARGET_PS2)
#define TEXMAP_EXT      ".tm2"
#define TEXMAP_EXT_CAPS ".TM2"
#endif

	if(mat.hasmap)
	{
    mat.filename = ReadString(buffer, index);
		if(file_finder_exists(texture_path + mat.filename, TEXMAP_EXT) ||
		   file_finder_exists(texture_path + mat.filename, TEXMAP_EXT_CAPS))
    {
			mat.texture = nglLoadTextureA(mat.filename.data());
    }
		else
			if(file_finder_exists(common_path + mat.filename, TEXMAP_EXT) ||
			   file_finder_exists(common_path + mat.filename, TEXMAP_EXT_CAPS))
      {
				nglSetTexturePath(common_path.data());
				mat.texture = nglLoadTextureA(mat.filename.data());
				nglSetTexturePath(texture_path.data());
			}
			else
			{
				nglPrintf("BETH: file %s not found in normal or common paths\n", mat.filename.data());
#ifdef BETH
				nglPrintf("BETH: texture path = %s, common path = %s\n", texture_path.data(), common_path.data());
#endif
			}

		mat.bilinearfilter = ReadChar(buffer, index);
		mat.wrapu = ReadChar(buffer, index);
		mat.wrapv = ReadChar(buffer, index);
  }
}

PanelQuad* PanelFile::GetPointer(const char* search_name)
{
	PanelQuad* tmp = pquads;
	while(tmp)
	{
		if(tmp->name == search_name)
			return tmp;
		tmp = tmp->next;
	}

#ifndef USER_MKV
	nglPrintf("Object %s not found in PANEL file\n", search_name);
#endif
	return frontendmanager.GetDefaultPQ();
}

void PanelFile::Init(bool floating)
{
	PanelGeom* tmp = obs;
	while(tmp)
	{
		tmp->Init(&pquads, floating);
		tmp = tmp->next;
	}
}

void PanelFile::Draw(int layer)
{
	// all that needs to be drawn are the batches, in order
	PanelQuad* tmp = pquads;
	while(tmp != NULL)
	{
		tmp->Draw(layer);
		tmp = tmp->next;
	}
}
void PanelFile::ForceDoneSlide(bool in)
{
	if (in)
		slide_state = PF_SLIDE_IN_DONE;
	else
		slide_state = PF_SLIDE_OUT_DONE;
	slide_offset = 640;
	slide_timer = 0;
	if (in)
		slide_offset = 640-slide_offset;
	
	PanelGeom* tmp = obs;
	while (tmp)
	{
		tmp->Update(0);
		tmp->Slide(slide_offset);
		tmp = tmp->next;
	}
}

void PanelFile::Update(time_value_t time_inc)
{
	if (IsSliding())
	{
		slide_timer += time_inc;
		if (slide_timer > slide_max_time) 
		{
			slide_timer = 0;
			if (slide_state == PF_SLIDE_IN) 
				slide_state = PF_SLIDE_IN_DONE;
			else
				slide_state = PF_SLIDE_OUT_DONE;
		}
		else
		{
			slide_offset = 320*(1+sinf(1.57f*(2*slide_timer/slide_max_time - 1)));
			if (slide_state == PF_SLIDE_IN)
				slide_offset = 640-slide_offset;
		}
	}

	PanelGeom* tmp = obs;
	while (tmp)
	{
		tmp->Update(time_inc);
		if (IsSliding())
			tmp->Slide(slide_offset);
		tmp = tmp->next;
	}
}

void PanelFile::StartSlide(bool in, float max_time)
{
	if(IsSliding()) return;
	slide_max_time = max_time;
	slide_state = in ? PF_SLIDE_IN : PF_SLIDE_OUT;
	slide_timer = 0.0f;
	slide_offset = in ? 640 : 0;

	PanelGeom* tmp = obs;
	while(tmp)
	{
		tmp->Slide(slide_offset);
		tmp = tmp->next;
	}
}

/************* Other functions ***********/

uint32 ReadLong(unsigned char* buffer, int& index)
{
	uint32 ret;
	ret = buffer[index] | (buffer[index+1]<<8) | (buffer[index+2]<<16) |
		(buffer[index+3]<<24);
	index += 4;
	return ret;
}

unsigned char ReadChar(unsigned char* buffer, int& index)
{
	unsigned char ret;
	ret = buffer[index];
	index++;
	return ret;
}


float ReadFloat(unsigned char* buffer, int& index)
{
	float ret;

	uint32 tmp = buffer[index] | (buffer[index+1]<<8) | (buffer[index+2]<<16) |
		(buffer[index+3]<<24);
	ret = *(float*) &tmp;
	index += 4;
	return ret;
}

short ReadShort(unsigned char* buffer, int& index)
{
	short ret;
	ret = buffer[index] | (buffer[index+1]<<8);
	index += 2;
	return ret;
}

stringx ReadString(unsigned char* buffer, int& index)
{
	short size = ReadShort(buffer, index);
	char* tmp = NEW char[size+1];
	for(int i=0; i<size; i++)
		tmp[i] = ReadChar(buffer, index);
	tmp[size] = '\0';
	stringx ret = stringx(tmp);

	delete[] tmp;
	return ret;
}

void ReadMatrix3x4(matrix4x4& matrix, unsigned char* buffer, int& index)
{
	matrix = identity_matrix;
	ReadVector3d(matrix.xrow(), buffer, index);
	ReadVector3d(matrix.yrow(), buffer, index);
	ReadVector3d(matrix.zrow(), buffer, index);
	ReadVector3d(matrix.wrow(), buffer, index);
}

void ReadVector3d(vector3d& v, unsigned char* buffer, int& index)
{
	v.x = ReadFloat(buffer, index);
	v.y = ReadFloat(buffer, index);
	v.z = ReadFloat(buffer, index);
}

void ReadVector2d(vector2d& v, unsigned char* buffer, int& index)
{
	v.x = ReadFloat(buffer, index);
	v.y = ReadFloat(buffer, index);
}

void make_euler(matrix4x4& m, const vector3d& angles, bool is_fe_cam)
{
	if(is_fe_cam)
	{
		matrix4x4 rotx, roty, rotz;//, trans;
		static matrix4x4 fudge(
			-1, 0, 0, 0,
			0, 0, -1, 0,
			0, -1, 0, 0,
			0, 0, 0, 1 );
		rotx.make_rotate(vector3d(1, 0, 0), angles.x);
		roty.make_rotate(vector3d(0, 1, 0), angles.z);
		rotz.make_rotate(vector3d(0, 0, 1), angles.y);
		m = fudge * rotx * roty * rotz;
	}
	else
	{
		float s0,c0,s1,c1,s2,c2;
		fast_sin_cos_approx(angles.x, &s0, &c0);
		fast_sin_cos_approx(angles.y, &s1, &c1);
		fast_sin_cos_approx(angles.z, &s2, &c2);

		m.x = vector4d(s1*s0*s2+c1*c2, c0*s2, c1*s0*s2-s1*c2, 0.0F);
		m.y = vector4d(s1*s0*c2-c1*s2, c0*c2, c1*s0*c2+s1*s2, 0.0F);
		m.z = vector4d(    s1*c0, -s0, c1*c0, 0.0F);
		m.w = vector4d(    0.0F,0.0F,0.0F,1.0F);
	}
}









