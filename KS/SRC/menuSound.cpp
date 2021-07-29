#include "global.h"	// For all files
#include "menu.h"	// For menu structures
#include "entity.h"
#include "wave.h"
#include "wds.h"

// Draw Entities Menu
#define MENUSOUND_MAXSOURCES 32
static float MENUSOUND_SourceVols[MENUSOUND_MAXSOURCES];
static nslSourceId MENUSOUND_Sources[MENUSOUND_MAXSOURCES];
#define MENUSOUND_MAXLABELCHAR 32
static char MENUDRAW_SourceLabels[MENUSOUND_MAXSOURCES][MENUSOUND_MAXLABELCHAR];
#define MENUSOUND_SoundTypeMax 32


class SoundMenuEntryFunctionFloatEdit : public MenuEntryFunctionFloatEdit
{
	float  *tfloat;
	protected:
	float  lo, hi, step;
	char *format;
	pMenuEntryButtonFunction fn;

	
	public:
	nslSourceId src;
	nslSoundId  snd;
	SoundMenuEntryFunctionFloatEdit( char *text, float *t, pMenuEntryButtonFunction fn, nslSourceId src, float l, float h, float s );
	SoundMenuEntryFunctionFloatEdit( char *text, float *t, pMenuEntryButtonFunction fn, nslSourceId src, float l, float h, float s, char *f );
	virtual ~SoundMenuEntryFunctionFloatEdit() {}

};



static SoundMenuEntryFunctionFloatEdit *MENUSOUND_SourceEntries[MENUSOUND_MAXSOURCES];
static MenuEntryFunction *DumpFunc;
class MenuSoundDraw : public Menu
{
public:
	MenuSoundDraw(Menu *p, int ents, pMenuEntry *ent) : Menu(p, ents, ent) {}
	virtual ~MenuSoundDraw() {}

	virtual void OnOpen(Menu *cto, MenuSystem *c);
	virtual void OnClose( bool toparent );
	virtual void OnTick(float dtime);
};

static MenuSoundDraw *MENUSOUND_MenuSources=NULL; //(NULL, 0, NULL);
static Submenu *MENUSOUND_SubmenuSources = NULL; //Submenu("Entities", &MENUDRAW_MenuEntities);



static pMenuEntry MENUSOUND_SoundEntries[MENUSOUND_SoundTypeMax];

class MenuSound : public Menu
{
public:
	MenuSound(Menu *p, int ents, pMenuEntry *ent) : Menu(p, ents, ent) {}
	virtual ~MenuSound() {}

	virtual void OnOpen(Menu *cto, MenuSystem *c);
	virtual void OnTick(float dtime);
};


// Miscellaneous Draw Menu
void MenuSound::OnOpen(Menu *cto, MenuSystem *c)
{
	// For "All On" and "All Off" to be recursive.
	// A little wasteful, since these will be called again
	// when the submenus are activated.  Also, this temporarily
	// sets the parent of these menus incorrectly. (dc 10/18/01)
	Menu::OnOpen(cto, c);
}
void MenuSound::OnTick(float dtime)
{
	// For "All On" and "All Off" to be recursive.
	MENUSOUND_MenuSources->OnTick(dtime);
}


static MenuSound *MENUSOUND_Menu = NULL; //(NULL, countof(MENUDRAW_Entries), MENUDRAW_Entries);
Submenu *menu_sound = NULL; //Submenu("Draw Menu", &MENUDRAW_Menu);




void MENUSOUND_StaticInit( void )
{
	
	MENUSOUND_MenuSources=NEW MenuSoundDraw(NULL, 0, NULL);
	MENUSOUND_SubmenuSources = NEW Submenu("Sounds", MENUSOUND_MenuSources);
	MENUSOUND_SoundEntries[0] = MENUSOUND_SubmenuSources;
	MENUSOUND_Menu = NEW MenuSound(NULL, countof(MENUSOUND_SoundEntries), MENUSOUND_SoundEntries);
	menu_sound = NEW Submenu("Sound Menu", MENUSOUND_Menu);
}






static stringx MENUSOUND_SourceLabel(nslSourceId src)
{
	stringx label;
	label = nslGetSourceName(src);
	label.to_lower();
	return label;
}





SoundMenuEntryFunctionFloatEdit::SoundMenuEntryFunctionFloatEdit( char *text, float *t, pMenuEntryButtonFunction func, nslSourceId source, float l, float h, float s )
	: MenuEntryFunctionFloatEdit(text, t, func, l, h, s)
{
	src = source;
}
SoundMenuEntryFunctionFloatEdit::SoundMenuEntryFunctionFloatEdit( char *text, float *t, pMenuEntryButtonFunction func, nslSourceId source, float l, float h, float s, char *f  )
	: MenuEntryFunctionFloatEdit(text, t, func, l, h, s, f)
{
	src = source;
}
MENUENTRYBUTTONFUNCTION(Dump)
{
	os_file out;
	char line[80];
	int i;
	out.open((stringx(BeachDataArray[g_game_ptr->get_beach_id()].name) + stringx("SOUNDDUMP.TXT")).c_str(), os_file::FILE_WRITE);
	for (i = 0; i < MENUSOUND_MAXSOURCES; i++)
	{
		if (MENUSOUND_SourceEntries[i]->src != NSL_INVALID_ID)
		{
			sprintf(line, "%s\t%f\r\n", nslGetSourceName(MENUSOUND_SourceEntries[i]->src),
					nslGetSourceParam(MENUSOUND_SourceEntries[i]->src, NSL_SOUNDPARAM_VOLUME));
			out.write(line, strlen(line));
		}

	}

	out.close();

	return true;
}
MENUENTRYBUTTONFUNCTION(Play)
{
	int src = ((SoundMenuEntryFunctionFloatEdit*)entry)->src;
	
	if (nslGetSoundStatus(((SoundMenuEntryFunctionFloatEdit*)entry)->snd) != NSL_SOUNDSTATUS_INVALID)
	{
		nslStopSound(((SoundMenuEntryFunctionFloatEdit*)entry)->snd);
	}
	else
	{
		if (src != NSL_INVALID_ID)
			((SoundMenuEntryFunctionFloatEdit*)entry)->snd = nslAddSound(src);
		if (((SoundMenuEntryFunctionFloatEdit*)entry)->snd != NSL_INVALID_ID)
		{
			nslPlaySound(((SoundMenuEntryFunctionFloatEdit*)entry)->snd);
		}
}
  return true;
}
void MenuSoundDraw::OnClose( bool toparent )
{
	int i;
	for (i=0; i < MENUSOUND_MAXSOURCES; i++)
	{
		if (nslGetSoundStatus(MENUSOUND_SourceEntries[i]->snd) != NSL_SOUNDSTATUS_INVALID)
		{
			nslStopSound(MENUSOUND_SourceEntries[i]->snd);
		}
	}

	nslUnpauseAllSounds();
	mem_lock_malloc(false);

	for (i=0; i < NumEntries()-1; i++)
	{
		delete MENUSOUND_SourceEntries[i];
		MENUSOUND_SourceEntries[i] = NULL;
	}
	delete DumpFunc;
	DumpFunc = NULL;
	ClearMenu();

	mem_lock_malloc(true);
	Menu::OnClose(toparent);
}
void MenuSoundDraw::OnOpen(Menu *cto, MenuSystem *c)
{
	nslPauseAllSounds();
	//MenuEntryFunction *pmef;
	u_int mecount = 0;
#ifdef TARGET_PS2
	nslSourceId src = nslGetSourceByIndex(mecount);

	mem_lock_malloc(false);

	ClearMenu();

	
/*
	pmef = NEW MenuEntryFunction("All Entities Off", MENUDRAW_AllEntitiesOff);
	AddEntry(pmef);
	pmef = NEW MenuEntryFunction("All Entities On", MENUDRAW_AllEntitiesOn);
	AddEntry(pmef);

    entity::prepare_for_visiting();

	world_dynamics_system::entity_list::const_iterator ei = g_world_ptr->get_entities().begin();
	world_dynamics_system::entity_list::const_iterator ei_end = g_world_ptr->get_entities().end();
	for ( ; ei!=ei_end; ei++ )
	{
		entity* ent = *ei;
		if (
			ent &&
			!ent->already_visited() &&
			(
				ent->is_visible() ||	// exclude unspawned beach objects, but not ones we just turned off
				ent->is_ext_flagged(EFLAG_EXT_WAS_VISIBLE)
			) &&
			ent->is_flagged(EFLAG_GRAPHICS) &&
			(
				ent->get_vrep() ||
				ent->has_mesh() ||
				ent->get_flavor() == ENTITY_POLYTUBE ||
				ent->get_flavor() == ENTITY_LENSFLARE
			)
		)
		{
			ent->visit();
*/
	DumpFunc = NEW MenuEntryFunction("Dump Values", Dump);
	AddEntry(DumpFunc);
	bool stop = false;
	while(src != NSL_INVALID_ID && !stop)
	{
		MENUSOUND_Sources[mecount] = src;

		stringx label = MENUSOUND_SourceLabel(src);
		sprintf(MENUDRAW_SourceLabels[mecount], label.c_str());

		MENUSOUND_SourceEntries[mecount] = NEW SoundMenuEntryFunctionFloatEdit(MENUDRAW_SourceLabels[mecount], MENUSOUND_SourceVols + mecount, Play,src, 0.0f, 1.0f, .05f);
		MENUSOUND_SourceEntries[mecount]->SetValue(nslGetSourceParam(src, NSL_SOUNDPARAM_VOLUME));

		AddEntry(MENUSOUND_SourceEntries[mecount]);

		++mecount;
		if (mecount >= MENUSOUND_MAXSOURCES)
		{
			printf("MENUDRAW:\tToo many entities for draw menu.  Skipping the rest...\n");
			stop = true;
		}	
		src = nslGetSourceByIndex(mecount);
	}

	//assert(((u_int) NumEntries()) == mecount + 2);
		mem_lock_malloc(true);
#endif
	c->Refresh();

	Menu::OnOpen(cto, c);


}

void MenuSoundDraw::OnTick(float dtime)
{
	for (int i = 0; i < NumEntries() - 1; ++i)
	{
		nslSetSourceParam(MENUSOUND_Sources[i], NSL_SOUNDPARAM_VOLUME, MENUSOUND_SourceVols[i]);
	}
}
