// This file gets #included from wave.cpp .

#if defined(DEBUG) && !defined(TARGET_GC)
// the game cube compiler doesn't like the constant expression:
//   2 * WAVE_PerturbTest.num

static float WAVE_TestProfileInc = 0.05f;

#define WAVE_PERTURBTESTMAXENTRIES 10

// Perturb builder menu
class WaveMenuPerturb : public Menu
{
private:
	static char label[2 * WAVE_PERTURBTESTMAXENTRIES][64];
	
public:
	WaveMenuPerturb(Menu *p, int ents, pMenuEntry *ent) : Menu(p, ents, ent) {}
	virtual ~WaveMenuPerturb() {}
	
	virtual void OnOpen(Menu *cto, MenuSystem *c);
	virtual void OnClose(bool toparent) { Menu::OnClose(toparent); WaveDebug.BreakTest = 0; }
	virtual void OnButtonPress(int buttonid);
};

char WaveMenuPerturb::label[2 * WAVE_PERTURBTESTMAXENTRIES][64];

void WaveMenuPerturb::OnOpen(Menu *cto, MenuSystem *c)
{
	assert(WAVE_PERTURBTESTMAXENTRIES >= WAVE_PerturbTest.num);
	
	WaveDebug.BreakTest = 1;
	
	MenuEntryFloatEdit *pmefe;
	u_int mecount = 0;
	
	bool oldlock = mem_malloc_locked();
	mem_lock_malloc(false);
	
	ClearMenu();
	
	for (u_int i = 0; i < WAVE_PerturbTest.num; ++i)
	{
		sprintf(label[mecount], "X%d", i);
		pmefe = NEW MenuEntryFloatEdit(
			label[mecount],
			(float *) WAVE_PerturbTest.profilex + i,	// cast away const!!
			i * WAVE_TestProfileInc,
			WAVE_ControlPoints->numprofile - 1,
			WAVE_TestProfileInc
			);
		AddEntry(pmefe);
		++mecount;
		
		sprintf(label[mecount], "XX%d", i);
		pmefe = NEW MenuEntryFloatEdit(
			label[mecount],
			(float *) WAVE_PerturbTest.profilexx + i,	// cast away const!!
			0,
			WAVE_ControlPoints->numprofile - 1,
			WAVE_TestProfileInc
			);
		AddEntry(pmefe);
		++mecount;
	}
	
	mem_lock_malloc(oldlock);
	
	assert(2 * WAVE_PerturbTest.num == mecount);
	assert(NumEntries() == (int) mecount);
	c->Refresh();
	
	Menu::OnOpen(cto, c);
}

void WaveMenuPerturb::OnButtonPress(int buttonid)
{
	Menu::OnButtonPress(buttonid);
	
	int ae = GetActiveEntry();
	if (ae % 2)
	{
		return;
	}
	
	switch (buttonid)
	{
	case MENUCMD_RIGHT:
	case MENUCMD_R1:
		{
			// We have increased the value of an X; may need to change the value of higher X
			for (; ae + 2 < NumEntries(); ae += 2)
			{
				MenuEntryFloatEdit *curr = (MenuEntryFloatEdit *) GetEntry(ae);
				MenuEntryFloatEdit *next = (MenuEntryFloatEdit *) GetEntry(ae + 2);
				if (next->GetValue() <= curr->GetValue()) next->SetValue(curr->GetValue()+WAVE_TestProfileInc);
				else break;
			}
		}
		break;
		
	case MENUCMD_LEFT:
	case MENUCMD_L1:
		{
			// We have decreased the value of an X; may need to change the value of lower X
			for (; ae - 2 >= 0; ae -= 2)
			{
				MenuEntryFloatEdit *curr = (MenuEntryFloatEdit *) GetEntry(ae);
				MenuEntryFloatEdit *prev = (MenuEntryFloatEdit *) GetEntry(ae - 2);
				if (prev->GetValue() >= curr->GetValue()) prev->SetValue(curr->GetValue()-WAVE_TestProfileInc);
				else break;
			}
		}
		break;
		
	default:
		break;
	}
}

static WaveMenuPerturb *WAVE_MenuPerturb = NULL; //(NULL, 0, NULL);
Submenu *WAVE_SubmenuPerturb = NULL; //("Break Builder", &WAVE_MenuPerturb);
#endif	// #ifdef DEBUG

static MenuEntryListEdit *WAVE_EntryDebugPerturb=NULL;


// Perturb duration menu
class MenuPerturbDuration : public Menu
{
public:
	MenuPerturbDuration(Menu *p, int ents, pMenuEntry *ent) : Menu(p, ents, ent) {}
	virtual ~MenuPerturbDuration() {}
	
	virtual void OnOpen(Menu *cto, MenuSystem *c);
	virtual void OnClose(bool toparent);
};

void MenuPerturbDuration::OnOpen(Menu *cto, MenuSystem *c)
{
	bool memlocked=mem_malloc_locked();
	mem_lock_malloc(false);
	
	ClearMenu();
	
	for (u_int mecount = 0; mecount < WAVE_PerturbStageMax; ++mecount)
	{
		MenuEntryFloatEdit *pmefe = NEW MenuEntryFloatEdit(
			WAVE_PerturbStageName[mecount + 1], // + 1 accounts for type NONE
			WAVE_PerturbArray[WAVE_DebugPerturbType]->duration + mecount + 1, // + 1 accounts for type NONE
			0.f, 20.f, 0.1f
			);
		AddEntry(pmefe);
	}
	
	c->Refresh();
	
	Menu::OnOpen(cto, c);
	
	mem_lock_malloc(memlocked);
}

void MenuPerturbDuration::OnClose(bool toparent)
{
	bool memlocked=mem_malloc_locked();
	mem_lock_malloc(false);
	
	ClearMenu();
	
	Menu::OnClose(toparent);
	
	mem_lock_malloc(memlocked);
}

static MenuPerturbDuration *WAVE_MenuDuration= NULL; //(NULL, 0, NULL);
Submenu *WAVE_SubmenuDuration= NULL; //("Break Duration", &WAVE_MenuDuration);


#undef USEINFO
#undef USEINFONO
#undef USEINFOYES
#undef USEINFOINT
#undef USEINFOFLOAT
#define USEINFO(item, type, lo, hi, step, init, menu) USEINFO##menu(item, type, lo, hi, step, init)
#define USEINFONO(item, type, lo, hi, step, init)
#define USEINFOYES(item, type, lo, hi, step, init) USEINFO##type(item)
#define USEINFOINT(item) 			\
static MenuEntryIntEdit * WAVE_MenuEntry##item = NULL;
#define USEINFOFLOAT(item) 			\
static MenuEntryFloatEdit * WAVE_MenuEntry##item = NULL;
#include "wavedebug.txt"


#undef USEINFOYES
#define USEINFOYES(item, type, lo, hi, step, init) WAVE_MenuEntry##item,
static pMenuEntry WAVE_MenuEntries[] =
{
	WAVE_EntryDebugPerturb,
#if defined(DEBUG) && !defined(TARGET_GC)
		WAVE_SubmenuPerturb,
#endif
		WAVE_SubmenuDuration,
#include "wavedebug.txt"
};


static Menu *WAVE_Menu = NULL; //(NULL, countof(WAVE_MenuEntries), WAVE_MenuEntries);
Submenu *menu_wave = NULL; //("Wave Geometry Menu", &WAVE_Menu);

void WAVEMENU_StaticInit( void )
{
#if defined(DEBUG) && !defined(TARGET_GC)
	WAVE_MenuPerturb = NEW WaveMenuPerturb(NULL, 0, NULL);
	WAVE_SubmenuPerturb = NEW Submenu("Break Builder", WAVE_MenuPerturb);
	WAVE_EntryDebugPerturb = NEW MenuEntryListEdit (
		"Break",
		& (int &) WAVE_DebugPerturbType,
		WAVE_PerturbTypeMax,
		WAVE_PerturbName
		);
#endif
	WAVE_MenuDuration= NEW MenuPerturbDuration(NULL, 0, NULL);
	WAVE_SubmenuDuration = NEW Submenu("Break Duration", WAVE_MenuDuration);

#undef USEINFOYES
#undef USEINFOINT
#undef USEINFOFLOAT
#define USEINFOYES(item, type, lo, hi, step, init) USEINFO##type(item, lo, hi, step)
#define USEINFOINT(item, lo, hi, step)							\
	WAVE_MenuEntry##item =									\
	NEW MenuEntryIntEdit(#item, &WaveDebug.##item, lo, hi);
#define USEINFOFLOAT(item, lo, hi, step)						\
	WAVE_MenuEntry##item =									\
	NEW MenuEntryFloatEdit(#item, &WaveDebug.##item, lo, hi, step);
#include "wavedebug.txt"
	
	int index=0;
	WAVE_MenuEntries[index++] = WAVE_EntryDebugPerturb;
#if defined(DEBUG) && !defined(TARGET_GC)
	WAVE_MenuEntries[index++] = WAVE_SubmenuPerturb;
#endif
	WAVE_MenuEntries[index++] = WAVE_SubmenuDuration;
#undef USEINFOYES
#define USEINFOYES(item, type, lo, hi, step, init) WAVE_MenuEntries[index++] = WAVE_MenuEntry##item;
#include "wavedebug.txt"
	
	WAVE_Menu = NEW Menu(NULL, countof(WAVE_MenuEntries), WAVE_MenuEntries);
	menu_wave = NEW Submenu("Wave Geometry Menu", WAVE_Menu);
	
}

