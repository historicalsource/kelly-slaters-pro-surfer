// FEPanel.h

#ifndef FEPANEL_H
#define FEPANEL_H

#include "global.h"
#include "stringx.h"
#include "entity.h"
#include "text_font.h"
#include "scoringmanager.h"

class StringList
{
public:
	enum { MAX_STRING_SIZE = 150 };
	static float SPEED_FALL;
	static float SPEED_DRIFT;
	static int RANGE_D_SPEED_LO;
	static int RANGE_D_SPEED_HI;
	static int RANGE_F_SPEED_LO;
	static int RANGE_F_SPEED_HI;
	static int RANGE_D_MAX_LO;
	static int RANGE_D_MAX_HI;
	static int RANGE_BX_LO;
	static int RANGE_BX_HI;
	static int RANGE_BY_LO;
	static int RANGE_BY_HI;

public:
	stringx		data;
	float		x;
	float		y;
	float		fall_speed[MAX_STRING_SIZE];
	float		drift_speed[MAX_STRING_SIZE];
	int			drift_max[MAX_STRING_SIZE];
	float		delta_x[MAX_STRING_SIZE];
	float		delta_y[MAX_STRING_SIZE];

public:
	void Update(time_value_t time_inc);
	void MakeRand(void);
	void Break(void);

	StringList & operator=(const StringList & s);
};

// Font text class
class TextString
{
protected:
	Font* font;
	stringx text;
	float x;
	float y;
	int z;
	Font::HORIZJUST hJustify;
	Font::VERTJUST	vJustify;
	bool even_number_spacing;
	int fade;			// 0 = no fading, 1 = fade in, -1 = fade out, 2 = constant alpha
	float fade_alpha;
	float fade_timer;
	float scale;
	float button_scale;
	bool override_alpha;

public:
	bool checkTime;
	float time;			// time remaining to draw
	bool no_color;
	color32 color;

	
	// *** all passed-in coordinates will be in 640x480 coords now, and 
	//	   converted to ps2 coordinates if necessary ***
	TextString(Font* f, stringx t, float x1, float y1, int z1 = 0, float s=1.0f, const Font::HORIZJUST horizJust = Font::HORIZJUST_CENTER, const Font::VERTJUST vertJust = Font::VERTJUST_CENTER) { cons(f, t, x1, y1, z1, s, horizJust, vertJust, true); }
	TextString(Font* f, stringx t, float x1, float y1, int z1, float s, const Font::HORIZJUST horizJust, const Font::VERTJUST vertJust, color32 col) { cons(f, t, x1, y1, z1, s, horizJust, vertJust, false, col); }
	TextString() {};	
	virtual ~TextString() {}
	virtual void Update(time_value_t time_inc);
	virtual void Draw();
	virtual void setHJustify(const Font::HORIZJUST hjust);
	virtual void setVJustify(const Font::VERTJUST vjust);
	virtual void setFont(Font * f) { font = f; }
	virtual void setButtonScale(float bs) { button_scale = bs; }
	virtual void numberSpacing(bool on) { even_number_spacing = on; }
	virtual void changeText(stringx t);
	virtual void changeScale(const float s) { scale = s; }
	virtual void changePos(float posx, float posy);
	virtual void changeX(const float posX);
	virtual void changeY(const float posY);
	virtual void changeZ(int _z) { z = _z; }
	virtual void ChangeFade(bool start, bool fade_in, float time=2.0f);
	virtual void SetFade(const float a) { fade = 2; fade_alpha = a; }

	virtual float getX();
	virtual float getY();
	virtual int getZ();
	virtual stringx getText() { return text; }
	virtual float GetScale(void) const { return scale; }
	virtual float GetButtonScale(void) const { return button_scale; }
	virtual bool GetNumberSpacing(void) const { return even_number_spacing; }
	virtual Font * GetFont(void) { return font; }
	virtual Font::HORIZJUST GetHJustify(void) const { return hJustify; }
	virtual Font::VERTJUST GetVJustify(void) const { return vJustify; }

	// compatibility with FloatingText and MultiLineString
	virtual void setLineSpacing(const int new_spacing) {}
	virtual void resetLineSpacing() {}
	virtual void UpdateInScene(bool ignore_scale = false) {}
	virtual void SetLocation3D(vector3d loc) {}
	virtual vector3d GetLocation3D() { return vector3d(0, 0, 0); }
	virtual void SetBehaviorNF(float r_x, float r_y) {}
	virtual void SetBehavior(bool nfb) {}
	
	// makes replacements of bad characters, should 
	// always be called before render after text has been changed
	static void MakeReplacements(stringx& text);

protected:
	virtual void Render(void) { Render(&text, x, y); }
	virtual void Render(stringx* t, float x1, float y1);
	virtual void Render(stringx* t, float x1, float y1, float *xs, float * ys, bool random_text_fade = true);

	virtual void cons(Font* f, stringx t, float x1, float y1, int z1, float s, const Font::HORIZJUST horizJust, const Font::VERTJUST, bool no_col, color32 col=color32(0, 0, 0, 255));
	void makeStringList(StringList *sl, stringx d, float x1, float y1);
};

class MultiLineString : public TextString
{
protected:
	int vSpacing;
	int line_num;
	static const int max_fonts = 20;	// arbitrary
	Font* fonts[max_fonts];	// allows different lines to have different fonts

public:
	MultiLineString(Font* f, stringx t, float x1, float y1, int z1 = 0, float s=1.0f, const Font::HORIZJUST horizJust = Font::HORIZJUST_CENTER, const Font::VERTJUST vertJust = Font::VERTJUST_CENTER) { cons(f, t, x1, y1, z1, s, horizJust, vertJust, true); }
	MultiLineString(Font* f, stringx t, float x1, float y1, int z1, float s, const Font::HORIZJUST horizJust, const Font::VERTJUST vertJust, color32 col) { cons(f, t, x1, y1, z1, s, horizJust, vertJust, false, col); }
	MultiLineString() {};	

	virtual int getLineNum() { return line_num; }
	virtual void changeText(stringx t);
	virtual void setLineSpacing(const int new_spacing);
	virtual void resetLineSpacing() { setLineSpacing(-1); }
	virtual void addFont(int index, Font* f);
	virtual void setFont(Font* f) { fonts[0] = f; font = f; }
protected:
	virtual void Render(stringx* t, float x1, float y1);
	virtual void cons(Font* f, stringx t, float x1, float y1, int z1, float s, const Font::HORIZJUST horizJust, const Font::VERTJUST, bool no_col, color32 col=color32(0, 0, 0, 255));
};

class BouncingText : public TextString
{
protected:
	float	targetScale;
	float	speed;

public:
	BouncingText();
	BouncingText(Font* f, stringx t, float x1, float y1, int z1 = 0, float s=1.0f, const Font::HORIZJUST horizJust = Font::HORIZJUST_CENTER, const Font::VERTJUST vertJust = Font::VERTJUST_CENTER);
	BouncingText(Font* f, stringx t, float x1, float y1, int z1, float s, const Font::HORIZJUST horizJust, const Font::VERTJUST vertJust, color32 col);

	void Bounce(const float bounceScale, const float bounceTime);
	void Update(time_value_t time_inc);
};

class RandomText : public TextString
{
protected:
	static const int DISPLAY_MAX = 2;	// total time it will be displayed
	bool isRand;						// currently falling?
  bool noFade;            // Don't fade text based on y position

	StringList rand_string;

public:
	RandomText(Font* f, stringx t, float x1, float y1, int z1 = 0, float s=1.0f, const Font::HORIZJUST horizJust = Font::HORIZJUST_CENTER, const Font::VERTJUST vertJust = Font::VERTJUST_CENTER) : isRand(false) { cons(f, t, x1, y1, z1, s, horizJust, vertJust, true); noFade=false; }
	RandomText(Font* f, stringx t, float x1, float y1, int z1, float s, const Font::HORIZJUST horizJust, const Font::VERTJUST vertJust, color32 col) : isRand(false) { cons(f, t, x1, y1, z1, s, horizJust, vertJust, false, col); }
	RandomText() : isRand(false) {}

	virtual void Update(time_value_t time_inc);
	virtual void Draw();
	virtual void changeText(stringx t);
	virtual void makeRand();
	virtual void unmakeRand() { isRand = false; }
	void Break(void);
  void NoFade(bool nf) { noFade = nf; }
	
	bool IsRand(void) const { return isRand; }
};


// NOT random
class BoxText : public TextString
{
protected:
	int width;
	int height;
	bool reverse;		// reverse the order of the lines
	int max_box_strings;
	StringList* box_strings;
	int box_str_count;
	nglFileBuf fileBuf;
	// Don't count on the floating behavior to work for BoxText; currently it's a hack for placement
	// all for floating
	bool non_floating_behavior;
	nglVector location_2d;
	nglVector location_3d;
	float real_scale;
	float real_x;
	float real_y;
	float adjusted_x, adjusted_y;

	// ability to be scrollable; won't scroll unless
	// box_str_count > max_vis_lines
	bool scrollable;
	int max_vis_lines;
	int first_vis;

public:
	BoxText(Font* f, stringx t, float x1, float y1, int z1 = 0, float s=1.0f, const Font::HORIZJUST horizJust = Font::HORIZJUST_CENTER, const Font::VERTJUST vertJust = Font::VERTJUST_CENTER) { cons(f, t, x1, y1, z1, s, horizJust, vertJust, color32(0, 0, 0, 0)); }
	BoxText(Font* f, stringx t, float x1, float y1, int z1, float s, const Font::HORIZJUST horizJust, const Font::VERTJUST vertJust, color32 col, int mbs=5) { cons(f, t, x1, y1, z1, s, horizJust, vertJust, col, mbs); }

	// don't use this normally!
	BoxText() {fileBuf.Buf = NULL;}
	~BoxText();
	virtual void Draw();
	virtual void UpdateInScene(bool ignore_scale=false);

	int getWidth()    {return width;};
	int getHeight()   {return height;};
	// scale_override is useful for cases (such as trick text) where the current scale may
	// not be the final scale
	// height override is useful when the default height (space between lines) of the font 
	// is not the desired height
	// returns number of lines
	int makeBox(int w, int h, bool r=false, float scale_override = -1, int height_override = -1);
	void SetLocation3D(vector3d loc);
	void SetScrollable(int vis) { scrollable = true; max_vis_lines = vis; first_vis = 0; }

	virtual void OnDown(int c) { scroll(false, 1); }
	virtual void OnUp(int c) { scroll(true, 1); }
	virtual bool scroll(bool up, int lines);

	// not to be confused with PreformatText, this file only contains a string that's not formatted
	// this function just reads the string from the file into the TextString
	void ReadFromFile(char* filename, bool remove_endlines);

	virtual void changeScale(const float s) { real_scale = s; }
	virtual void changePos(float posx, float posy);
	void SetBehavior(bool nfb) { non_floating_behavior = nfb; }
	int GetFirstVisable() { return first_vis; }

protected:
	void cons(Font* f, stringx t, float x1, float y1, int z1, float s, const Font::HORIZJUST horizJust, const Font::VERTJUST vertJust, color32 col, int mbs=5);
};

// can be random
class TrickBoxText : public RandomText
{
protected:
	int numLines;
	int lineHeight;
	int width;
	int height;
	bool reverse;		// reverse the order of the lines
	static const int MAX_BOX_LINES = 10;
	StringList box_strings[MAX_BOX_LINES];
	int box_str_count;

public:
	TrickBoxText(Font* f, stringx t, float x1, float y1, int z1 = 0, float s=1.0f, const Font::HORIZJUST horizJust = Font::HORIZJUST_CENTER, const Font::VERTJUST vertJust = Font::VERTJUST_CENTER) : numLines(0), lineHeight(0), reverse(false), box_str_count(0) { isRand=false; cons(f, t, x1, y1, z1, s, horizJust, vertJust, true); }
	TrickBoxText(Font* f, stringx t, float x1, float y1, int z1, float s, const Font::HORIZJUST horizJust, const Font::VERTJUST vertJust, color32 col) :numLines(0), lineHeight(0), reverse(false), box_str_count(0) { isRand=false; cons(f, t, x1, y1, z1, s, horizJust, vertJust, false, col); }
	TrickBoxText() {}

	void Update(time_value_t time_inc);
	void Draw();
	void changePos(float posx, float posy);
	void makeRand();
	void unmakeRand() { isRand = false; }
	void MakeTrickBox(const ScoringManager::Chain & chain, bool topJustify = true, bool fade = false);
	void SetSize(const int w, const int h);
	void Break(void);

	int getWidth(void ) const { return width; }
	int getHeight(void) const { return height; }
	int GetTextHeight(void) const { return numLines*lineHeight; }
	int GetLineHeight(void) const { return lineHeight; }
	int GetNumLines(void) const { return numLines; }
	const StringList & GetLine(const int i) { assert(i >= 0 && i < numLines); return box_strings[i]; }
};

class BurstText : public TextString
{
protected:
	static float TARGET_SCALE;
	static float TARGET_SCALE_TIME;
	static float TARGET_ALPHA_TIME;

protected:
	float	targetScale;
	float	scaleRate;

public:
	BurstText();

	void Update(time_value_t time_inc);
	void Burst(TextString * orig);
};

class BurstTrickText : public TrickBoxText
{
protected:
	static float TARGET_SCALE;
	static float TARGET_SCALE_TIME;
	static float TARGET_ALPHA_TIME;

protected:
	float	targetScale;
	float	scaleRate;
	float	origY[MAX_BOX_LINES];
	int		origLineHeight;

public:
	BurstTrickText();

	void Update(time_value_t time_inc);
	void Burst(TrickBoxText *	orig);
};

class PreformatText : public TextString
{
private:
	// file reading data (pre-formatted)
	stringx* file_head;
//	int fh_lines;
	int start_line;
	int num_vis_lines;
	int max_lines;		// maximum number of lines
	int actual_lines;	// number of actual lines
	static const int absolute_limit = 400;

public:
	PreformatText(Font* f, stringx t, float x1, float y1, int z1 = 0, float s=1.0f, const Font::HORIZJUST horizJust = Font::HORIZJUST_CENTER, const Font::VERTJUST vertJust = Font::VERTJUST_CENTER) : file_head(NULL) { cons(f, t, x1, y1, z1, s, horizJust, vertJust, true); }
	PreformatText(Font* f, stringx t, float x1, float y1, int z1, float s, const Font::HORIZJUST horizJust, const Font::VERTJUST vertJust, color32 col) : file_head(NULL) { cons(f, t, x1, y1, z1, s, horizJust, vertJust, false, col); }
	PreformatText() {}
	virtual ~PreformatText() { delete[] file_head; file_head = NULL; }

	virtual void Draw();

	// this function is to override the standard drawing behavior
	virtual void DrawLine(int line_num, float x1, float y1);
	int GetActualLines() { return actual_lines; }

	// pre-formatted data
	void readText(char* filename, int max_lines, int vis_lines);
	void unreadText() { delete[] file_head; file_head = NULL; }
	bool scroll(bool up, int lines);	// returns true if can scroll further in that direction
	float getPercentage();		// useful for scrollbars
};





class FloatingText : public MultiLineString
{
private:
	nglVector location_3d;
	nglVector location_2d;
	float real_scale;		// the constant scale
	float real_x;			// the constant location
	float real_y;
	bool non_floating_behavior;

public:
	FloatingText(Font* f, stringx text) { cons(f, text, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, color32(0, 0, 0, 0)); }
	FloatingText(Font* f, stringx text, color32 col) { cons(f, text, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, col); }
	FloatingText(Font* f, stringx text, const Font::HORIZJUST hj, const Font::VERTJUST vj, color32 col)
	{ cons(f, text, hj, vj, col); }
	// UpdateInScene must be called where there is a valid WorldToScreen matrix
	virtual void UpdateInScene(bool ignore_scale = false);
	virtual void Draw();

	virtual void changeScale(const float s) { real_scale = s; }

	virtual void SetLocation3D(vector3d loc);
	virtual vector3d GetLocation3D() { return vector3d(location_3d[0], location_3d[1], location_3d[2]); }
	virtual void SetBehaviorNF(float r_x, float r_y);
	virtual void SetBehavior(bool nfb) { non_floating_behavior = nfb; }

protected:
	void cons(Font* f, stringx t, const Font::HORIZJUST hj, const Font::VERTJUST vj, color32 c);
};




enum PanelGeomKind
{
	PanelGeomObject		=0x91,
	PanelGeomInstance	=0x92,
	PanelGeomGroup		=0x93,
	PanelGeomText		=0x94,
	PanelGeomSkater		=0x95,
	PanelGeomObjectView	=0x96,
	PanelGeomMovie		=0x97,
};

struct PanelMaterial
{
	stringx name;
	uint32 color;
	float emissive;
	float shininess;
	float shinestr;
	bool additive;
	bool hasmap;
	nglTexture* texture;
	bool bilinearfilter;
	bool wrapu;
	bool wrapv;
	stringx filename;
};

class PanelAnim;
class PanelQuad
{
protected:
	bool isAnim;
	int fade;			// 0 = no fading, 1 = fade in, -1 = fade out, 2 = constant alpha
	float fade_alpha;
	float fade_timer;
	float mask;
	int maskType;		// 0 = none, 1 = left to right, 2 = bottom to top, 3 = closing church doors
	PanelAnim* anim;
	nglQuad quad;
	bool drawOn;
	int rotate;    // 0=no rotate, 1 = rotate about center, 2 = rotate about rotate_x & _y
	float rotation;
	float rotate_x;
	float rotate_y;
	int layer;
	float r, g, b, a;
	float x1, y1, x2, y2;  // always use SetPos
	float width, height;		// PS2 coords (if applicable)
	float width_a, height_a;	// 640x480 coords
	matrix4x4 matrix;  // obj to world
	bool on_menu;
	bool clip;
	recti clipping;
public:
	matrix4x4 Xform;  // set by PanelAnim
	stringx name;
	bool dont_draw;
	float z;

	// beware! these coords already have the matrix applied to them
	// you must change them using SetPos, otherwise the quad's coords will not be set
	float u1, v1, u2, v2;
	PanelQuad* next;
public:
	PanelQuad() {}
	PanelQuad(stringx n) { cons(n); }
	virtual ~PanelQuad() {}
	void MakeAnim(PanelAnim* a) { anim = a; }
	void StartAnim(bool on) { assert(anim); isAnim = on; }
	virtual void Init(float xa, float ya, float xb, float yb, 
				  float r1, float g1, float b1, float a1, 
				  float ua, float va, float ub, float vb, 
				  float z1, matrix4x4 obj);
	void AddedToMenu() { on_menu = true; }
	void OffMenu() { on_menu = false; }   // only used in *very* special cases
	virtual void TurnOn(bool on) { drawOn = on; }
	void ToggleOn() { drawOn = !drawOn; }
	void SetFade(const float amt);
	void ChangeFade(bool start, bool fade_in, float time=2.0f);
	void Mask(const float m, const int type = 1) { mask = m; maskType = type; } 
	virtual void SetLayer(int l) { layer = l; }
	virtual void Rotate(float r) { rotate = 1; rotation = r; }
	virtual void Rotate(float x, float y, float r);
	virtual void Update(time_value_t time_inc);
	virtual void Draw(int layer = 0, float alpha = -1.0f);
	void setTexture(nglTexture* tex) { nglSetQuadTex(&quad, tex); }
	void setMaterialFlags(u_int mapflags) { nglSetQuadMapFlags(&quad, mapflags); }
	virtual void SetColor(const float _r, const float _g, const float _b, const float _a) { r = _r; g = _g; b = _b; a = _a; }
	virtual void SetColor(const color clr) { r = clr.r; g = clr.g; b = clr.b; a = clr.a; }
	virtual void SetZ(const float _z) { z = _z; nglSetQuadZ(&quad, z); }
	virtual void SetUV(float ua, float va, float ub, float vb) { u1=ua; u2=ub; v1=va; v2=vb; nglSetQuadUV(&quad, u1, v1, u2, v2); }
	virtual bool IsOn(void) const { return drawOn; }
	// this function does not apply the matrix again, but it does adjust PS2 coords
	virtual void SetPos(float xa, float ya, float xb, float yb);
	virtual void SetPos(float xa, float ya);
	// ignore 2nd & 4th coord pairs
	virtual void SetPos(float xa, float ya, float xb, float yb, float xc, float yc, float xd, float yd);
	virtual void GetPos(float &xa, float &ya, float &xb, float &yb);
	virtual void SetCenterX(float x);	// instead of setting all the coords, only change the x center coord
	virtual void SetCenterY(float y);	// instead of setting all the coords, only change the y center coord
	virtual void SetCenterPos(float cx, float cy);
	virtual void GetCenterPos(float & cx, float & cy);
	virtual void SetCenterPosQuadOnly(float cx, float cy);	// sets only the quad's position (not x1, y1...) used for slides
	virtual float GetWidth() { return width; }
	virtual float GetHeight() { return height; }
	virtual float GetWidthA() { return width_a; }
	virtual float GetHeightA() { return height_a; }
	virtual void SetClip(const bool on);
	virtual void SetClip(const recti & bounds);

	// these two functions are used to set a PQ's params to be exactly what another's are
	virtual void GetFade(int& f, float& alpha, float& timer) { f = fade; alpha = fade_alpha, timer = fade_timer; }
	virtual void SetFade(int f, float alpha, float timer) { fade = f; fade_alpha = alpha; fade_timer = timer; }

	virtual nglTexture* GetTexture() { return quad.Tex; }
	// only for PanelQuad4
	virtual void SetUV(float, float, float, float, float, float, float, float) {}
	virtual nglQuad* getQuad() { return &quad; }	// don't use except for debugging!
	virtual float GetRotation(void) const { return rotation; }

private:
	void Animate();
protected:
	void cons(stringx n);

	friend class FloatingPQ;
};


class PanelQuad4 : public PanelQuad
{
private:
	float x[4];
	float y[4];
	float u[4];
	float v[4];

public:
	PanelQuad4() { cons(""); }
	PanelQuad4(stringx n) { cons(n); }
	virtual void Init(float xa, float xb, float xc, float xd,
		  float ya, float yb, float yc, float yd,
		  float r1, float g1, float b1, float a1,  
		  float z1);
	virtual void Init(float xa, float xb, float xc, float xd,
		  float ya, float yb, float yc, float yd,
		  float r1, float g1, float b1, float a1,  
		  float z1, matrix4x4 obj);
//	virtual void Rotate(float r) { rotate = 1; rotation = r; }
	virtual void Rotate(float x, float y, float r);
	virtual void RotateOnce(float x, float y, float r);
	virtual void Update(time_value_t time_inc);
	virtual void Draw(int layer, float alpha = -1.0f);
	virtual void SetUV(float u1, float u2, float u3, float u4, float v1, float v2, float v3, float v4);
	virtual void SetPos(float xa, float ya, float xb, float yb, float xc, float yc, float xd, float yd);
	virtual void SetCenterPos(float cx, float cy);
	virtual void GetCenterPos(float & cx, float & cy);
	virtual void SetVertices(const float nx[4], const float ny[4]);
};


class FloatingPQ : public PanelQuad
{
public:
	enum HJust { Center, Left, Right };

private:
	nglVector location_3d;
	nglVector location_2d;
	float scale;
	int h_just;
	float width_f;	// these are consistent; don't change when you change the points
	float height_f;
	float x1_const;			// based on the original coords; used for non_floating_behavior
	float x2_const;
	float y1_const;
	float y2_const;
	bool non_floating_behavior;		// pretend to be a normal quad
public:
	FloatingPQ(stringx n) { cons(n); h_just = Center; }
	FloatingPQ(float r, float g, float b, float a, int x1, int y1, int x2, int y2, int z, HJust hj=Center);
	FloatingPQ(const PanelQuad* pq, HJust hj=Center);
	virtual void Init(float xa, float ya, float xb, float yb, 
				  float r1, float g1, float b1, float a1, 
				  float ua, float va, float ub, float vb, 
				  float z1, matrix4x4 obj);
	virtual void UpdateInScene();
	virtual void Draw(int layer, float alpha = -1.0f);
	virtual void SetLocation3D(vector3d l);
	virtual void SetWidth(float w) { width = w; }
	virtual void SetHeight(float h) { height = h; }

	// both of these are about the justification
	virtual void SetConstantScale(float s);		// adjust width_f, height_f relative to s
	virtual void SetScale(float s);				// only adjusts the points

	virtual void SetPos(float xa, float ya, float xb, float yb);
	virtual void GetPos(float &xa, float &ya, float &xb, float &yb) { xa = x1_const; xb = x2_const; ya = y1_const; yb = y2_const; }
	virtual vector3d GetLocation3D() { return vector3d(location_3d[0], location_3d[1], location_3d[2]); }
	virtual void SetBehaviorNF(float x, float y);
	virtual void SetBehavior(bool nfb);
};



class PanelBatch;
class PanelGeom
{
public:
	stringx name;
	stringx properties;
	matrix4x4 matrix; // obj to parent
	vector3d boundboxcenter;
	vector3d boundboxsize;
	uint32 nchildren;

	PanelGeom* children;
	PanelGeom* next;
	PanelGeom* parent;
	PanelGeom() { cons(); }
	virtual ~PanelGeom();
	virtual PanelGeomKind Kind() const=0;
	virtual bool Load(unsigned char* buffer, int& index);
	virtual void Reload();
	static PanelGeom* LoadGeom(unsigned char* buffer, int& index);
	virtual PanelGeom *FindObject(const stringx &search_name);
	virtual PanelQuad* GetQuad() { return NULL; }
	virtual void Init(PanelQuad** pquads, bool floating=false);
	virtual void Init(PanelQuad** pquads, matrix4x4 matrix, bool floating=false);
	virtual void Update(time_value_t time_inc);
	virtual void Slide(float offset);
protected:
	void cons();
};

class PanelBatch
{
public:
	uint32 material;
	PanelMaterial mat;
	
	uint32 verttype; // fvf code
	uint32 color;
	uint16 nwedges;

	PanelQuad* pq;

	// these 3 are arrays of size nwedges
	vector3d* wedges;
	color32* colors;
	vector2d* tex;    // the uv coords

	uint16 strip_count;
	uint16 index_count;

	// this is an array of size index_count
	uint16* didxs;

private:
	bool eq_to_tolerance(float a, float b, float tol);

public:
	PanelBatch() { pq = NULL; wedges = NULL; colors = NULL; tex = NULL; didxs = NULL; }
	virtual ~PanelBatch();
	bool Load(PanelMaterial* mats, unsigned char* buffer, int& index);
	void Reload(PanelMaterial* mats);
	// called once to set up the quad

	void Init(PanelQuad** pquads, stringx name, bool floating=false);
	void Init(PanelQuad** pquads, stringx name, matrix4x4 matrix, bool floating=false);
	void Update(time_value_t time_inc);
	bool check(int x2, int y2);
	void Slide(float offset);
};

class PanelObject : public PanelGeom
{
public:
	PanelMaterial* materials;
	uint16 size;  // # of materials
	uint16 nbatches;
	PanelBatch* batches;
	PanelObject() { batches = NULL; materials = NULL; cons(); }
	virtual ~PanelObject();
	virtual PanelGeomKind Kind() const { return PanelGeomObject; }
	virtual bool Load(unsigned char* buffer, int& index);
	virtual void Reload();
	virtual PanelQuad* GetQuad();
	virtual void Init(PanelQuad** pquads, bool floating=false);
	virtual void Init(PanelQuad** pquads, matrix4x4 matrix, bool floating=false);
	virtual void Update(time_value_t time_inc);
	virtual void Slide(float offset);
};

class PanelText : public PanelGeom
{
public:
	stringx fontname;
	color32 color;
	enum JustKind 
	{
		Left=0, HCenter=1, HStretch=2, Right=3, HMask =  0x3,
		Top=0, VCenter=4, VStretch=8, Bottom=12, VMask = 0xC,
	};
	uint8 justification;
	float linespacing;
	uint32 numtextlines;
	stringx text;

	nglQuad quad;  // temporary placeholder

	PanelText() {}
	virtual PanelGeomKind Kind() const { return PanelGeomText; }
	virtual bool Load(unsigned char* buffer, int& index);

	virtual void Init(PanelQuad** pquads);
	virtual void Init(PanelQuad** pquads, matrix4x4 matrix);
	virtual void Draw(float alpha=1);
};

class PanelFile
{
private:
	enum { PF_SLIDE_NONE, PF_SLIDE_IN, PF_SLIDE_OUT, PF_SLIDE_IN_DONE, PF_SLIDE_OUT_DONE };
	int slide_state;
	float slide_offset;
	float slide_timer;
	float slide_max_time;

	stringx filename;
public:
	PanelGeom* obs;
	stringx path;
	static stringx texture_path;
	static stringx common_path;
	PanelFile* next;  // for use with FrontEnd
	PanelQuad* pquads;

	PanelFile() { obs = NULL; }
	PanelFile(const stringx f, const stringx p);
	virtual ~PanelFile();
	static bool ReadHeader(unsigned char* buffer, int& index);
	static void ReadPanelMaterial(PanelMaterial& mat, unsigned char* buffer, int& index);
	virtual PanelGeom* FindObject(const stringx& search_name);
	virtual PanelGeom* FindObject(const char* search_name);
	virtual PanelQuad* FindQuad(const stringx& search_name);
	PanelQuad* GetPointer (const char* search_name);
	virtual void SetFilename(stringx p);

	void Draw(int layer);
	void Update(time_value_t time_inc);
	bool Load(bool floating=false);
	void Reload();
	
	void ForceDoneSlide(bool in);

	void StartSlide(bool in, float max_time=1.0f);
	bool IsSliding(void) const { return slide_state == PF_SLIDE_IN || slide_state == PF_SLIDE_OUT; }
	bool IsSlideOutDone(void) const { return slide_state == PF_SLIDE_OUT_DONE; }

private:
	void Init(bool floating=false);  // called by Load()
};

// these classes seem useless, but not sure how to deal with these kinds 
// of PanelGeom
class PanelSkaterModel : public PanelGeom
{
public:
	virtual bool Load(unsigned char* buffer, int& index) { return PanelGeom::Load(buffer, index);}
	virtual PanelGeomKind Kind() const { return PanelGeomSkater; }
};

class PanelObjectModel : public PanelGeom
{
public:
	virtual bool Load(unsigned char* buffer, int& index) { return PanelGeom::Load(buffer, index); }
	virtual PanelGeomKind Kind() const { return PanelGeomObjectView; }
};

class PanelMovie : public PanelGeom
{
public:
	virtual bool Load(unsigned char* buffer, int& index) { return PanelGeom::Load(buffer, index); }
	virtual PanelGeomKind Kind() const { return PanelGeomMovie; }
};

/************* Other Functions *****************/

uint32 ReadLong(unsigned char* buffer, int& index);
unsigned char ReadChar(unsigned char* buffer, int& index);
float ReadFloat(unsigned char* buffer, int& index);
short ReadShort(unsigned char* buffer, int& index);
stringx ReadString(unsigned char* buffer, int& index);

void ReadMatrix3x4(matrix4x4& m, unsigned char* buffer, int& index);
void ReadVector3d(vector3d& v, unsigned char* buffer, int& index);
void ReadVector2d(vector2d& v, unsigned char* buffer, int& index);
void make_euler(matrix4x4& m, const vector3d& angles, bool is_fe_cam=false);


#endif