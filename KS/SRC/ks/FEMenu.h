// FEMenu.h

#ifndef FEMENU_H
#define FEMENU_H

#include "global.h"
#include "inputmgr.h"
#include "commands.h"
#include "ksnsl.h"
#include "ksreplay.h"
#include "text_font.h"
#include "app.h"
#include "game.h"
#include "camera.h"
#include "FEPanel.h"
#include "FEAnim.h"

#if defined(TARGET_PS2)
#include "hwosps2/ps2_m2vplayer.h"
#endif /* TARGET_PS2 JIV DEBUG */

#if defined(TARGET_PS2)
#define NUM_CONTROLLER_PORTS 2
#else
#define NUM_CONTROLLER_PORTS 4
#endif

#define MENU_HIGHLIGHT_MULTIPLIER 1000.0f // the divisor for the game.ini menu period entry
#define THROB_INTENSITY 0.5f // ranges from 0 to 1.  1 means it goes all the way from black to 100%.  0 means no throbbing.

enum FEMENUCMD
{
	FEMENUCMD_SELECT,
		FEMENUCMD_START,
		FEMENUCMD_UP,
		FEMENUCMD_DOWN,
		FEMENUCMD_LEFT,
		FEMENUCMD_RIGHT,
		FEMENUCMD_CROSS,
		FEMENUCMD_TRIANGLE,
		FEMENUCMD_SQUARE,
		FEMENUCMD_CIRCLE,
		FEMENUCMD_L1,
		FEMENUCMD_R1,
		FEMENUCMD_L2,
		FEMENUCMD_R2,
		FEMENUCMD_END,
};

class FEMenu;
class FEMenuSystem;
class FEManager;


/******************** FEMenuEntry ******************/
class FEMenuEntry
{
public:
	int entry_num;    // which number on the list is it
	FEMenuEntry* next;
	FEMenuEntry* previous;
	FEMenu* menu;
	
protected:
	bool highlight;
	bool disabled;
	entity* ent;
	color32 high_ent_color;
	color32 norm_ent_color;

	// this should be changed sparingly; preferably just to 
	// modify the actual text; not the color or position
	MultiLineString* text;
	
private:
	float highlight_intensity;  // ranges from -1 to +1 based on period and time
	float highlight_timer;
	int disabled_alpha;

	// special color is a different color from the menu color
	color32 special_color;
	color32 special_color_high;
	bool has_special_color;

	bool no_flash;

	float special_scale;
	float special_scale_high;
	bool has_special_scale;
	
public:
	// these are for use only with multi menu; else use next & prev
	FEMenuEntry* up;
	FEMenuEntry* down;
	FEMenuEntry* left;
	FEMenuEntry* right;
	
	FEMenuEntry() { ent = NULL; }
	FEMenuEntry(stringx l, FEMenu* m, bool floating=false, Font* ft=NULL) { cons(l, m, floating, ft); }
	virtual ~FEMenuEntry() { delete text; }
	virtual void Load() {}
	virtual void OnSelect() {}
	virtual void Highlight(bool h = true, bool anim=true);
	virtual void Disable(bool d = true);
	virtual bool GetDisable() { return disabled; }
	virtual void Draw();
	virtual void Update(time_value_t time_inc);
	// only for graphical
	virtual void TurnOn(bool on) {}
	virtual void SetSpecialColor(color32 c, color32 ch);
	virtual void SetSpecialScale(float s, float sh);
	virtual bool GetSpecialScale(float &s, float &sh);
	virtual void SetNoFlash(bool nf) { no_flash = nf; }
	virtual float GetHighlightIntensity() { return highlight_intensity; }

	// text accessors/modifiers
	virtual void SetPos(float x, float y)				{ text->changePos(x, y); }
	virtual void SetZ(int z)							{ text->changeZ(z); }
	virtual void SetText(stringx str)					{ text->changeText(str); }
	virtual void SetLocation3D(vector3d loc)			{ text->SetLocation3D(loc); }
	virtual void SetHJustify(Font::HORIZJUST hjust)		{ text->setHJustify(hjust); }
	virtual void SetVJustify(Font::VERTJUST vjust)		{ text->setVJustify(vjust); }
	virtual void SetFade(bool s, bool f, float t=2.0f)	{ text->ChangeFade(s, f, t); }
	virtual void SetLineSpacing(const int s)			{ text->setLineSpacing(s); }
	virtual void SetFont(Font* f)						{ text->setFont(f); }
	virtual void SetBehaviorNF(float x, float y)		{ text->SetBehaviorNF(x, y); }
	virtual void SetBehavior(bool nfb)					{ text->SetBehavior(nfb); }
	virtual void SetColor(color32 col)					{ text->color = col; }
	virtual void SetScale(float s)						{ text->changeScale(s); }
	virtual void UpdateInScene()						{ text->UpdateInScene(); }
	virtual stringx GetText()							{ return text->getText(); }
	virtual float GetX()								{ return text->getX(); }
	virtual float GetY()								{ return text->getY(); }
	virtual color32 GetColor()							{ return text->color; }
	virtual int getLineNum()							{ return text->getLineNum(); }
	virtual float GetScale()							{ return text->GetScale(); }
	virtual void AddEntity(entity* e, color32 hc, color32 nc);
	virtual void AddFont(int index, Font* f)			{ text->addFont(index, f); }
	
protected:
	virtual void cons(stringx l, FEMenu* m, bool floating=false, Font* ft=NULL);
	virtual void OnHighlight(bool anim=true) {}
};

/********************* FEMenu **************************/

enum {
	FEMENU_SCROLLING			= 0x001,
	FEMENU_WRAP					= 0x002,
	FEMENU_HAS_COLOR			= 0x004,
	FEMENU_HAS_COLOR_HIGH		= 0x008,
	FEMENU_HAS_COLOR_HIGH_ALT	= 0x010,
	FEMENU_1_ENTRY_SHOWN		= 0x020,	// menus with only 1 visible entry
	FEMENU_DONT_SKIP_DISABLED	= 0x040,	// useful for 1 vis entry menus, to see an entry even if it's disabled
	FEMENU_USE_SCALE			= 0x080,
	FEMENU_NO_FLASHING			= 0x100,
	FEMENU_NORM_COLOR_NO_FLASH  = 0x200,	// when no flash, use normal color instead of high color
	FEMENU_DONT_SHOW_DISABLED	= 0x400,	// useful for normal menus where disabled entries are hidden
};

class FEMenu
{
protected:
	static const int HF_UP;
	static const int HF_DOWN;
	static const int HF_LEFT;
	static const int HF_RIGHT;
	static const int HF_SELECT;
	static const int HF_BACK;
	static const int HF_RESUME;
	static const int HF_CONTINUE;
	static const int HF_SWITCH;

public:
  friend class DemoModeManager;

	int center_x;      // center of the menu
	int center_y;
	int num_entries;
	int dy;            // distance between entries
	int half;          // half the menu height
	int menu_num;
	bool init;
	stringx font_filename;
	color32 color;
	color32 color_high;
	color32 color_high_alt;
	float scale;
	float scale_high;
	int max_vis_entries;
	int flags;
	
	FEMenuEntry* entries;
	FEMenuEntry* first_vis_entry;  // if scrolling, first visible entry
	FEMenuEntry* last_vis_entry;
	FEMenuEntry* highlighted;
	FEMenuSystem* system;
	FEMenu* back;
	int back_num;  // for going back to a submenu

protected:
	FEMenu *		submenus;
	FEMenu *		active;  // only if !submenu; = NULL if this is active
	FEMenu *		parent;
	TextString *	helpText;
	int				helpFlags;

public:
	FEMenu* next_sub;   // only if submenu
	
	FEMenu();
	FEMenu(FEMenuSystem* s, int x, int y, int mve=8) { cons(s, x, y, mve); }
	// WARNING: if there's only one color passed in, it's assumed to be the highlight color
	FEMenu(FEMenuSystem* s, int x, int y, color32 ch, color32 cha, int mve=8) { cons(s, x, y, ch, cha, mve); }
	FEMenu(FEMenuSystem* s, int x, int y, color32 c, color32 ch, color32 cha, float sc=1.0f, float sch=1.2f, int mve=8, int flg=0) { cons(s, x, y, c, ch, sc, sch, mve, flg); }
	virtual ~FEMenu();
	
	// setBack sets the default behavior, used in the default OnTriangle()
	virtual void setBack(FEMenu* b, int bn=1) { back = b; back_num = bn; }
	virtual void setHigh(FEMenuEntry* menu, bool anim=true);
	virtual void setVis(FEMenuEntry* first);
	virtual void Add(FEMenuEntry* e);		// add to the end
	virtual void Init();
	virtual void Load(bool floating) {}
	virtual void Load() { Load(false); }
	virtual void Draw();
	virtual void DrawTop() {}
	virtual void UpdateInScene() {}
	virtual void Update(time_value_t time_inc);
	virtual void HighlightDefault(void);
	virtual void OnActivate();
	virtual void OnActivate(int sub_menu) { OnActivate(); }
	virtual void OnUnactivate(FEMenu* m) {}
	virtual void OnSelect(int c);
	virtual void OnStart(int c);
	virtual void OnUp(int c) { if(active) active->OnUp(c); else Previous(); }
	virtual void OnDown(int c) { if(active) active->OnDown(c); else Next(); }
	virtual void OnLeft(int c) {}
	virtual void OnRight(int c) {}
	virtual void OnCross(int c);
	virtual void OnTriangle(int c);
	virtual void OnSquare(int c) {}
	virtual void OnCircle(int c) {}
	virtual void OnL1(int c) {}
	virtual void OnR1(int c) {}
	virtual void OnL2(int c) {}
	virtual void OnR2(int c) {}
	virtual void OnAnyButtonPress(int c, int b) {}  // *not* used to process normal button presses
	virtual void OnButtonRelease(int c, int b) {}	// not commonly needed
	virtual void SetAllScale(float s);
	virtual void MakeActive(FEMenu* a);
	virtual void SetHelpText(const int hFlags);
	
	// this function is only over-ridden (and used) with graphical menus
	virtual void Select(int entry_num) {}
	
	// lots of sounds associated with menu movements... this is just a 
	// wraparound to make the code cleaner
	static void play_sound(const char* name);
	
protected:
	virtual void cons(FEMenuSystem* s, int x, int y, int mve=8) { cons(s, x, y, color32(0, 0, 0, 0), color32(0, 0, 0, 0), mve); }
	virtual void cons(FEMenuSystem* s, int x, int y, color32 ch, color32 cha, int mve=8) { cons(s, x, y, color32(0, 0, 0, 0), ch, cha, mve); }
	virtual void cons(FEMenuSystem* s, int x, int y, color32 c, color32 ch, color32 cha, float sc=1.0, float sch=1.2, int mve=8, int flg=0);// { cons(s, x, y, c, ch, sc, sch, mve, FEMENU_HAS_COLOR | FEMENU_HAS_COLOR_HIGH); }
	virtual void Next();
	virtual void Previous();
	virtual void Select();
	virtual void AddSubmenu(FEMenu* sub);

private:
	//	virtual void cons(FEMenuSystem* s, int x, int y, color32 c, color32 ch, float sc=1.0, float sch=1.2, int mve=8, int flg=0);
};

/********************** FrontEnd **************************/
class FrontEnd
{
public:
	PanelAnimManager pam;
	PanelFile panel;
	FrontEnd* next;  // for FEManager
	stringx path;
	FEManager* manager;
	
	FrontEnd() {}
	FrontEnd(FEManager* man, stringx p, stringx pf_name) { cons(man, p, pf_name); }
	virtual ~FrontEnd() {}
	
	virtual void Add(PanelAnimManager a) { pam = a; }
	virtual void Update(time_value_t time_inc);
	virtual void Draw();
	//	virtual void DrawPanel(int layer) { panel.Draw(layer); }
	virtual void LoadPanel(bool floating=false) { panel.Load(floating); }
	
	// call DrawPanel or LoadPanel instead of GetPanel()->Draw (or ->Load)
	virtual PanelFile* GetPanel() { return &panel; }	// warning: might be NULL
	virtual void SetPanel(stringx p) { panel.SetFilename(p); }
protected:
	void cons(FEManager* man, stringx p, stringx pf_name);
	void TurnOn(PanelQuad* pq, bool on) { if(pq) pq->TurnOn(on); }
	void ToggleOn(PanelQuad* pq) { if(pq) pq->ToggleOn(); }
	void ChangeFade(PanelQuad* pq, bool start, bool in, float t) { if(pq) pq->ChangeFade(start, in, t); }
	void Mask(PanelQuad* pq, float f) { if(pq) pq->Mask(f); }
	void SetLayer(PanelQuad* pq, int l) { if(pq) pq->SetLayer(l); }
	virtual void SetPQIndices() {}
	void ReloadPanel() { panel.Reload(); }
	PanelQuad* GetPointer(const char* s) { return panel.GetPointer(s); }
};

/******************* FEGraphicalMenuEntry *****************/
class FEGraphicalMenuEntry : public FEMenuEntry
{
private:
	PanelQuad* pq;
	PanelQuad* pq_high;
	PanelAnimFile* highlight_paf;
	PanelAnimManager* pam;
	bool already_playing;
public:
	FEGraphicalMenuEntry() {}
	FEGraphicalMenuEntry(FEMenu* m);
	FEGraphicalMenuEntry(FEMenu* m, PanelQuad* quad_normal, PanelQuad* quad_high=NULL);
	FEGraphicalMenuEntry(FEMenu* m, PanelQuad* quad, PanelAnimFile* pf, PanelAnimManager* pm, PanelQuad* quad_high=NULL);
	
	// these functions are to add the pq's later, when loaded
	virtual void Load(PanelQuad* quad_normal, PanelQuad* quad_high=NULL);
	virtual void Load(PanelQuad* quad_normal, PanelAnimFile* pf, PanelAnimManager* pm, PanelQuad* quad_high = NULL);
	virtual void Draw();
	virtual void OnHighlight(bool anim=true);
	virtual void TurnOn(bool on);
	
	// warning... use of these functions will cause them to be removed from the drawing list
	virtual void SetPQ(PanelQuad* p) { pq = p; pq->AddedToMenu(); }
	virtual void SetPQHigh(PanelQuad* p) { pq_high = p; pq_high->AddedToMenu(); }
};

/****************** FEGraphicalMenu **********************/
class FEGraphicalMenu : public FEMenu, public FrontEnd
{
public:
	FEGraphicalMenu() {}
	FEGraphicalMenu(FEMenuSystem* s, FEManager* man, stringx path, stringx pf_name) { cons(s, man, path, pf_name); }
	virtual void Init();
	virtual void Load(bool floating) { if(!parent) LoadPanel(floating); }
	virtual void Load() { Load(false); }
	virtual void Draw();
	virtual void Select(int entry_num);
	virtual void Update(time_value_t time_inc) { FrontEnd::Update(time_inc); FEMenu::Update(time_inc); }
	virtual void OnActivate();
	virtual void Add(FEMenuEntry* e) { FEMenu::Add(e); }
	
	// remapped FrontEnd stuff
	virtual void TurnOn(PanelQuad* pq, bool on) { if(parent) ((FEGraphicalMenu*)parent)->TurnOn(pq, on); else FrontEnd::TurnOn(pq, on); }
	virtual void ToggleOn(PanelQuad* pq)		{ if(parent) ((FEGraphicalMenu*)parent)->ToggleOn(pq); else FrontEnd::ToggleOn(pq); }
	virtual void ChangeFade(PanelQuad* p, bool s, bool i, float t) { if(parent) ((FEGraphicalMenu*)parent)->ChangeFade(p, s, i, t); else FrontEnd::ChangeFade(p, s, i, t); }
	virtual void Mask(PanelQuad* pq, float f)	{ if(parent) ((FEGraphicalMenu*)parent)->Mask(pq, f); else FrontEnd::Mask(pq, f);}
	virtual void SetLayer(PanelQuad* pq, int l) { if(parent) ((FEGraphicalMenu*)parent)->SetLayer(pq, l); else FrontEnd::SetLayer(pq, l); }
	virtual void LoadPanel(bool floating=false)	{ if(parent) ((FEGraphicalMenu*)parent)->LoadPanel(floating); else FrontEnd::LoadPanel(floating); }
	virtual void ReloadPanel()					{ if(parent) ((FEGraphicalMenu*)parent)->ReloadPanel(); else FrontEnd::ReloadPanel(); }
	virtual PanelQuad* GetPointer(const char* s) { if(parent) return ((FEGraphicalMenu*)parent)->GetPointer(s); else return FrontEnd::GetPointer(s); }
	virtual void SetPanel(stringx p)			{ if(parent) ((FEGraphicalMenu*)parent)->SetPanel(p); else FrontEnd::SetPanel(p); }
private:
	void Next();
	void Previous();
protected:
	virtual void cons(FEMenuSystem* s, FEManager* man, stringx path, stringx pf_name);//, FEGraphicalMenu* par = NULL);
};

/********************** FETextMultiMenu ********************/
class FETextMultiMenu : public FEMenu
{
public:
	// these are only for convience; must be set by user
	FEMenuEntry* first;
	FEMenuEntry* last;
public:
	FETextMultiMenu() {}
	FETextMultiMenu(FEMenuSystem* s) { cons(s); }
	// WARNING: if there's only one color, it's assumed to be the highlight
	FETextMultiMenu(FEMenuSystem* s, color32 ch) { cons(s, ch); }
	FETextMultiMenu(FEMenuSystem* s, color32 c, color32 ch, float sc=1.0f, float sch=1.2f, int flg=0) { cons(s, c, ch, sc, sch, flg); }
	// setVis makes no sense in a multi menu
	virtual void setVis(FEMenuEntry* first) {}
	// Init can only be defined in an overloaded class
	// must define x&y coords for all entries
	virtual void Init() {}
	virtual void OnUp(int c) { Up(); }
	virtual void OnDown(int c) { Down(); }
	virtual void OnLeft(int c) { Left(); }
	virtual void OnRight(int c) { Right(); }
	
protected:
	virtual void cons(FEMenuSystem* s) { cons(s, color32(0, 0, 0, 0), color32(0, 0, 0, 0)); }
	virtual void cons(FEMenuSystem* s, color32 ch) { cons(s, color32(0, 0, 0, 0), ch, FEMENU_HAS_COLOR_HIGH); }
	virtual void cons(FEMenuSystem* s, color32 c, color32 ch, float sc=1.0f, float sch=1.2f, int flg=0);// { cons(s, c, ch, sc, sch, FEMENU_HAS_COLOR | FEMENU_HAS_COLOR_HIGH); }
	virtual void Up();
	virtual void Down();
	virtual void Left();
	virtual void Right();
private:
	//	void cons(FEMenuSystem* s, color32 c, color32 ch, float sc=1.0f, float sch=1.2f);
};


/********************** FEMultiMenu ********************/
class FEMultiMenu : public FEGraphicalMenu
{
public:
	// these are only for convience; must be set by user
	FEMenuEntry* first;
	FEMenuEntry* last;

	// secondary_cursor is similar to a highlight, but is in addition to
	// the menu entry thinks it's highlighted as normal, but there can be two highlights on screen
	// useful for going up-down and left-right with different "cursors"
	FEMenuEntry* secondary_cursor;
public:
	FEMultiMenu() {}
	FEMultiMenu(FEMenuSystem* s, FEManager* man, stringx path, stringx pf_name) { cons(s, man, path, pf_name);}
	virtual void OnUp(int c) { Up(); }
	virtual void OnDown(int c) { Down(); }
	virtual void OnLeft(int c) { Left(); }
	virtual void OnRight(int c) { Right(); }
	virtual void SetSecondaryCursor(FEMenuEntry* e, bool anim=true);
	
protected:
	virtual void cons(FEMenuSystem* s, FEManager* man, stringx p, stringx pf) { FEGraphicalMenu::cons(s, man, p, pf); secondary_cursor = NULL; }
	virtual void Up();
	virtual void Down();
	virtual void Left();
	virtual void Right();
};

/********************** FEMenuSystem********************/
class FEMenuSystem
{
protected:
	bool button_down[FEMENUCMD_END][NUM_CONTROLLER_PORTS];
	int device_flags;

public:
	FEMenu** menus;
	FEManager* manager;
	int active;
	int size;     // size allocated
	int count;    // number currently added
	Font* font;
	FEMenuSystem() {}
	FEMenuSystem(int s, FEManager* man, Font* f) { cons(s, man, f); InitAll(); }
	virtual ~FEMenuSystem() { delete[] menus; }

	enum { ALL_DEVICE_FLAGS = 0xffffffff };
	int GetDeviceFlags(){ return device_flags; };
	void SetDeviceFlags(int _df){ device_flags = _df; };
	
	virtual void InitAll();  // to initialize all menus after adds but before Tick
	virtual void Add(FEMenu* m);
	virtual void MakeActive(int index, int sub_menu=1);
	virtual void Update(time_value_t time_inc);
	virtual void UpdateButtonDown();
	virtual void Draw() { assert(menus[active]); menus[active]->Draw(); }
	virtual void Select(int menu_index, int entry_index) = 0;
	virtual void Exit() {}

	virtual void startDraw(int menu_num = -1, const bool pauseGame = true) { }
	virtual void endDraw(const bool unpause = true) { }

	virtual FEMenu * GetActiveMenu(void) { assert(menus[active]); return menus[active]; }
	
protected:
	virtual void cons(int s, FEManager* man, Font* f);
	virtual void OnButtonPress(int button, int controller);	
};

int getButtonPressed(int button);
int getButtonPressed(int button, int controller);
int getAnalogState(int button);
int getAnalogState(int button, int controller);
int getButtonState(int button);
int getButtonState(int button, int controller);

#endif
