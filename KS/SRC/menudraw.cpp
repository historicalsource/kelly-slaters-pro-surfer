#include "global.h"	// For all files
#include "menu.h"	// For menu structures
#include "entity.h"
#include "wave.h"
#include "wds.h"
#include "ksfx.h"	// For Get/Set particle draws
#include "water.h"	// For Get/Set water draws
#include "wavetex.h"	// For WavetexDebug_ShadowPass
#include "FrontEndManager.h"			// For Get/Set IGO draws

// Draw Entities Menu
#define MENUDRAW_MAXENTITIES 32
static int MENUDRAW_EntityFlags[MENUDRAW_MAXENTITIES];
static entity *MENUDRAW_Entities[MENUDRAW_MAXENTITIES];
#define MENUDRAW_MAXLABELCHAR 32
static char MENUDRAW_EntityLabels[MENUDRAW_MAXENTITIES][MENUDRAW_MAXLABELCHAR];

class MenuEntityDraw : public Menu
{
public:
	MenuEntityDraw(Menu *p, int ents, pMenuEntry *ent) : Menu(p, ents, ent) {}
	virtual ~MenuEntityDraw() {}

	virtual void OnOpen(Menu *cto, MenuSystem *c);
	virtual void OnClose(bool toparent);
	virtual void OnTick(float dtime);
};

static MenuEntityDraw *MENUDRAW_MenuEntities=NULL; //(NULL, 0, NULL);
static Submenu *MENUDRAW_SubmenuEntities = NULL; //Submenu("Entities", &MENUDRAW_MenuEntities);

// Draw Particles Menu
static MENUENTRYBUTTONFUNCTION(MENUDRAW_AllParticleOff);
static MENUENTRYBUTTONFUNCTION(MENUDRAW_AllParticleOn);

#undef USEINFO
#define USEINFO(type) MENUDRAW_ParticleType##type,
enum MenudrawParticleTypeEnum
{
#include "menudrawparticle.txt"
	MENUDRAW_ParticleTypeMax
};

static int MENUDRAW_ParticleFlags[MENUDRAW_ParticleTypeMax];

static MenuEntryFunction MENUDRAW_ParticleEntryAllOff("All Particle Off", MENUDRAW_AllParticleOff);
static MenuEntryFunction MENUDRAW_ParticleEntryAllOn("All Particle On", MENUDRAW_AllParticleOn);

#undef USEINFO
#define USEINFO(type)													\
static MenuEntryIntEdit *MENUDRAW_ParticleEntry##type	= NULL;
 //(#type, MENUDRAW_ParticleFlags + MENUDRAW_ParticleType##type, 0, 1);
#include "menudrawparticle.txt"





//#undef USEINFO
//#define USEINFO(type) MENUDRAW_ParticleEntry##type,
static pMenuEntry MENUDRAW_ParticleEntries[2+MENUDRAW_ParticleTypeMax];
//=
//{
//	&MENUDRAW_ParticleEntryAllOff,
//	&MENUDRAW_ParticleEntryAllOn,
//#include "menudrawparticle.txt"
//};







#undef USEINFO
#define USEINFO(type) ks_fx_GetDraw##type,
static bool (*MENUDRAW_ParticleGet[MENUDRAW_ParticleTypeMax])(void) =
{
#include "menudrawparticle.txt"
};

#undef USEINFO
#define USEINFO(type) ks_fx_SetDraw##type,
static void (*MENUDRAW_ParticleSet[MENUDRAW_ParticleTypeMax])(bool) =
{
#include "menudrawparticle.txt"
};

class MenuParticleDraw : public Menu
{
public:
	MenuParticleDraw(Menu *p, int ents, pMenuEntry *ent) : Menu(p, ents, ent) {}
	virtual ~MenuParticleDraw() {}

	virtual void OnOpen(Menu *cto, MenuSystem *c);
	virtual void OnTick(float dtime);
};

static MenuParticleDraw *MENUDRAW_MenuParticle = NULL; //(NULL, countof(MENUDRAW_ParticleEntries), MENUDRAW_ParticleEntries);
static Submenu *MENUDRAW_SubmenuParticle = NULL; //Submenu("Particle", &MENUDRAW_MenuParticle);

// Draw Water Menu
static MENUENTRYBUTTONFUNCTION(MENUDRAW_AllWaterOff);
static MENUENTRYBUTTONFUNCTION(MENUDRAW_AllWaterOn);

#undef USEINFO
#define USEINFO(type) MENUDRAW_WaterType##type,
enum MenudrawWaterTypeEnum
{
#include "menudrawwater.txt"
	MENUDRAW_WaterTypeMax
};

static int MENUDRAW_WaterFlags[MENUDRAW_WaterTypeMax];

static MenuEntryFunction MENUDRAW_WaterEntryAllOff("All Water Off", MENUDRAW_AllWaterOff);
static MenuEntryFunction MENUDRAW_WaterEntryAllOn("All Water On", MENUDRAW_AllWaterOn);

#undef USEINFO
#define USEINFO(type)													\
static MenuEntryIntEdit *MENUDRAW_WaterEntry##type = NULL;
//(#type, MENUDRAW_WaterFlags + MENUDRAW_WaterType##type, 0, 1);
#include "menudrawwater.txt"

//#undef USEINFO
//#define USEINFO(type) &MENUDRAW_WaterEntry##type,
static pMenuEntry MENUDRAW_WaterEntries[MENUDRAW_WaterTypeMax+2];
//{
//	&MENUDRAW_WaterEntryAllOff,
//	&MENUDRAW_WaterEntryAllOn,
//#include "menudrawwater.txt"
//};

#undef USEINFO
#define USEINFO(type) WATER_GetDraw##type,
static bool (*MENUDRAW_WaterGet[MENUDRAW_WaterTypeMax])(void) =
{
#include "menudrawwater.txt"
};

#undef USEINFO
#define USEINFO(type) WATER_SetDraw##type,
static void (*MENUDRAW_WaterSet[MENUDRAW_WaterTypeMax])(bool) =
{
#include "menudrawwater.txt"
};

class MenuWaterDraw : public Menu
{
public:
	MenuWaterDraw(Menu *p, int ents, pMenuEntry *ent) : Menu(p, ents, ent) {}
	virtual ~MenuWaterDraw() {}

	virtual void OnOpen(Menu *cto, MenuSystem *c);
	virtual void OnTick(float dtime);
};

static MenuWaterDraw *MENUDRAW_MenuWater = NULL; //(NULL, countof(MENUDRAW_WaterEntries), MENUDRAW_WaterEntries);
static Submenu *MENUDRAW_SubmenuWater = NULL; //Submenu("Water", &MENUDRAW_MenuWater);

// Draw Menu plus Misc Items
static bool MENUDRAW_GetDrawIGO(void);
static void MENUDRAW_SetDrawIGO(bool onoff);
static bool MENUDRAW_GetDrawShadow(void);
static void MENUDRAW_SetDrawShadow(bool onoff);

static MENUENTRYBUTTONFUNCTION(MENUDRAW_AllOff);
static MENUENTRYBUTTONFUNCTION(MENUDRAW_AllOn);

#undef USEINFO
#define USEINFO(type) MENUDRAW_Type##type,
enum MenudrawTypeEnum
{
#include "menudrawmisc.txt"
	MENUDRAW_TypeMax
};

static int MENUDRAW_Flags[MENUDRAW_TypeMax];

static MenuEntryFunction *MENUDRAW_EntryAllOff = NULL; //("All Objects Off", MENUDRAW_AllOff);
static MenuEntryFunction *MENUDRAW_EntryAllOn = NULL; //("All Objects On", MENUDRAW_AllOn);

#undef USEINFO
#define USEINFO(type)												\
static MenuEntryIntEdit *MENUDRAW_Entry##type	=NULL;
//(#type, MENUDRAW_Flags + MENUDRAW_Type##type, 0, 1);
#include "menudrawmisc.txt"

extern int WavetexDebug_ShadowPass;
MenuEntryIntEdit   *MENUDRAW_Shadows= NULL; //MenuEntryIntEdit  ( "Player Shadows", &WavetexDebug_ShadowPass             ,        0,         1,    1 );

#undef USEINFO
#define USEINFO(type) &MENUDRAW_Entry##type,

static pMenuEntry MENUDRAW_Entries[MENUDRAW_TypeMax+6];
// =
//{
//	&MENUDRAW_EntryAllOff,
//	&MENUDRAW_EntryAllOn,
//	MENUDRAW_SubmenuEntities,
//	&MENUDRAW_SubmenuParticle,
//	&MENUDRAW_SubmenuWater,
//	&MENUDRAW_Shadows,
//#include "menudrawmisc.txt"
//};





#undef USEINFO
#define USEINFO(type) MENUDRAW_GetDraw##type,
static bool (*MENUDRAW_Get[MENUDRAW_TypeMax])(void) =
{
#include "menudrawmisc.txt"
};

#undef USEINFO
#define USEINFO(type) MENUDRAW_SetDraw##type,
static void (*MENUDRAW_Set[MENUDRAW_TypeMax])(bool) =
{
#include "menudrawmisc.txt"
};

class MenuDraw : public Menu
{
public:
	MenuDraw(Menu *p, int ents, pMenuEntry *ent) : Menu(p, ents, ent) {}
	virtual ~MenuDraw() {}

	virtual void OnOpen(Menu *cto, MenuSystem *c);
	virtual void OnTick(float dtime);
};

static MenuDraw *MENUDRAW_Menu = NULL; //(NULL, countof(MENUDRAW_Entries), MENUDRAW_Entries);
Submenu *menu_draw = NULL; //Submenu("Draw Menu", &MENUDRAW_Menu);

void MENUDRAW_StaticInit( void )
{
	MENUDRAW_MenuEntities=NEW MenuEntityDraw(NULL, 0, NULL);
	MENUDRAW_SubmenuEntities = NEW Submenu("Entities", MENUDRAW_MenuEntities);

	#undef USEINFO
	#define USEINFO(type)													\
	MENUDRAW_ParticleEntry##type = NEW MenuEntryIntEdit 						\
	(#type, MENUDRAW_ParticleFlags + MENUDRAW_ParticleType##type, 0, 1);
	#include "menudrawparticle.txt"

	int index=0;
	MENUDRAW_ParticleEntries[index++] = &MENUDRAW_ParticleEntryAllOff;
	MENUDRAW_ParticleEntries[index++] = &MENUDRAW_ParticleEntryAllOn;
	#undef USEINFO
	#define USEINFO(type) MENUDRAW_ParticleEntries[index++] = MENUDRAW_ParticleEntry##type;
	#include "menudrawparticle.txt"

	MENUDRAW_MenuParticle = NEW MenuParticleDraw(NULL, countof(MENUDRAW_ParticleEntries), MENUDRAW_ParticleEntries);
	MENUDRAW_SubmenuParticle = NEW Submenu("Particle", MENUDRAW_MenuParticle);

	#undef USEINFO
	#define USEINFO(type)													\
	MENUDRAW_WaterEntry##type	=					\
	NEW MenuEntryIntEdit(#type, MENUDRAW_WaterFlags + MENUDRAW_WaterType##type, 0, 1);
	#include "menudrawwater.txt"

	index=0;
	MENUDRAW_WaterEntries[index++]=&MENUDRAW_WaterEntryAllOff;
	MENUDRAW_WaterEntries[index++]=&MENUDRAW_WaterEntryAllOn;
	#undef USEINFO
	#define USEINFO(type) MENUDRAW_WaterEntries[index++]=MENUDRAW_WaterEntry##type;
	#include "menudrawwater.txt"

	MENUDRAW_MenuWater = NEW MenuWaterDraw(NULL, countof(MENUDRAW_WaterEntries), MENUDRAW_WaterEntries);
	MENUDRAW_SubmenuWater = NEW Submenu("Water", MENUDRAW_MenuWater);

	MENUDRAW_EntryAllOff = NEW MenuEntryFunction("All Objects Off", MENUDRAW_AllOff);
	MENUDRAW_EntryAllOn  = NEW MenuEntryFunction("All Objects On", MENUDRAW_AllOn);

	#undef USEINFO
	#define USEINFO(type)												\
	MENUDRAW_Entry##type	=	NEW MenuEntryIntEdit(#type, MENUDRAW_Flags + MENUDRAW_Type##type, 0, 1);
	#include "menudrawmisc.txt"

	MENUDRAW_Shadows = NEW MenuEntryIntEdit("Player Shadows", &WavetexDebug_ShadowPass, 0, 1, 1);

	index=0;
	MENUDRAW_Entries[index++]=MENUDRAW_EntryAllOff    ;
	MENUDRAW_Entries[index++]=MENUDRAW_EntryAllOn     ;
	MENUDRAW_Entries[index++]=MENUDRAW_SubmenuEntities;
	MENUDRAW_Entries[index++]=MENUDRAW_SubmenuParticle;
	MENUDRAW_Entries[index++]=MENUDRAW_SubmenuWater   ;

	#undef USEINFO
	#define USEINFO(type) MENUDRAW_Entries[index++]=	MENUDRAW_Entry##type;
	#include "menudrawmisc.txt"

	MENUDRAW_Menu = NEW MenuDraw(NULL, countof(MENUDRAW_Entries), MENUDRAW_Entries);
	menu_draw = NEW Submenu("Draw Menu", MENUDRAW_Menu);
}







// Entity Draw Menu
static MENUENTRYBUTTONFUNCTION(MENUDRAW_AllEntitiesOff);
static MENUENTRYBUTTONFUNCTION(MENUDRAW_AllEntitiesOn);

static stringx MENUDRAW_EntityLabel(entity *ent)
{
	stringx label;
	if ((label = ent->get_parsed_name()) != "")
	{
	}
	else if ((label = ent->get_name()) != "")
	{
	}
	else
	{
		label = "Unknown";
	}
	if (label.find("_ENTID_") == 0)	// get rid of "_ENTID_27_" type prefixes
	{
		label = label.substr(7);
		int realstart = label.find("_");
		if (realstart >= 0)
		{
			label = label.substr(realstart + 1);
		}
	}
	label.to_lower();
	return label;
}

void MenuEntityDraw::OnOpen(Menu *cto, MenuSystem *c)
{
	if (NumEntries()) // Already opened by parent (hack).  (dc 06/18/02)
	{
		Menu::OnOpen(cto, c);
		return;
	}

	MenuEntryIntEdit *pmeie;
	MenuEntryFunction *pmef;
	u_int mecount = 0;

	bool old_lock_malloc = mem_malloc_locked();
	mem_lock_malloc(false);

//	ClearMenu();

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

			MENUDRAW_Entities[mecount] = ent;

			stringx label = MENUDRAW_EntityLabel(ent);
			sprintf(MENUDRAW_EntityLabels[mecount], label.c_str());

			pmeie = NEW MenuEntryIntEdit(MENUDRAW_EntityLabels[mecount], MENUDRAW_EntityFlags + mecount, 0, 1);

			pmeie->SetValue(ent->is_visible());

			AddEntry(pmeie);

			++mecount;
			if (mecount >= MENUDRAW_MAXENTITIES)
			{
				debug_print("MENUDRAW:\tToo many entities for draw menu.  Skipping the rest...\n");
			}
		}
	}

	assert(((u_int) NumEntries()) == mecount + 2);
	c->Refresh();

	mem_lock_malloc(old_lock_malloc);

	Menu::OnOpen(cto, c);
}

void MenuEntityDraw::OnClose(bool toparent)
{
	bool old_lock_malloc = mem_malloc_locked();
	mem_lock_malloc(false);

	while (pMenuEntry e = GetEntry(0))
	{
		DelEntry(e);
		delete e;
	}
	Menu::OnClose(toparent);

	mem_lock_malloc(old_lock_malloc);
}

void MenuEntityDraw::OnTick(float dtime)
{
	for (int i = 0; i < NumEntries() - 2; ++i)
	{
		bool wasvisible = MENUDRAW_Entities[i]->is_visible();
		if (wasvisible != (bool) MENUDRAW_EntityFlags[i])
		{
			// record that it was turned off by us, not someone else
			MENUDRAW_Entities[i]->set_ext_flag(EFLAG_EXT_WAS_VISIBLE, wasvisible);
			MENUDRAW_Entities[i]->set_visible(MENUDRAW_EntityFlags[i]);
		}
	}
}

static void MENUDRAW_SetAllEntities(bool onoff)
{
	for (int i = 0; i < MENUDRAW_MenuEntities->NumEntries() - 2; ++i)
	{
		MENUDRAW_EntityFlags[i] = onoff;
	}
}

static MENUENTRYBUTTONFUNCTION(MENUDRAW_AllEntitiesOff)
{
	switch ( buttonid )
	{
	case MENUCMD_CROSS:
		MENUDRAW_SetAllEntities(false);
		break;
	}
	return true;
}

static MENUENTRYBUTTONFUNCTION(MENUDRAW_AllEntitiesOn)
{
	switch ( buttonid )
	{
	case MENUCMD_CROSS:
		MENUDRAW_SetAllEntities(true);
		break;
	}
	return true;
}

// Water Draw Menu
void MenuWaterDraw::OnOpen(Menu *cto, MenuSystem *c)
{
	for (u_int i = 0; i < MENUDRAW_WaterTypeMax; ++i)
	{
		MENUDRAW_WaterFlags[i] = MENUDRAW_WaterGet[i]();
	}

	Menu::OnOpen(cto, c);
}

void MenuWaterDraw::OnTick(float dtime)
{
	for (u_int i = 0; i < MENUDRAW_WaterTypeMax; ++i)
	{
		MENUDRAW_WaterSet[i](MENUDRAW_WaterFlags[i]);
	}
}

static void MENUDRAW_SetAllWater(bool onoff)
{
	for (u_int i = 0; i < MENUDRAW_WaterTypeMax; ++i)
	{
		MENUDRAW_WaterFlags[i] = onoff;
	}
}

static MENUENTRYBUTTONFUNCTION(MENUDRAW_AllWaterOff)
{
	switch ( buttonid )
	{
	case MENUCMD_CROSS:
		MENUDRAW_SetAllWater(false);
		break;
	}
	return true;
}

static MENUENTRYBUTTONFUNCTION(MENUDRAW_AllWaterOn)
{
	switch ( buttonid )
	{
	case MENUCMD_CROSS:
		MENUDRAW_SetAllWater(true);
		break;
	}
	return true;
}

// Particle Draw Menu
void MenuParticleDraw::OnOpen(Menu *cto, MenuSystem *c)
{
	for (u_int i = 0; i < MENUDRAW_ParticleTypeMax; ++i)
	{
		MENUDRAW_ParticleFlags[i] = MENUDRAW_ParticleGet[i]();
	}

	Menu::OnOpen(cto, c);
}

void MenuParticleDraw::OnTick(float dtime)
{
	for (u_int i = 0; i < MENUDRAW_ParticleTypeMax; ++i)
	{
		MENUDRAW_ParticleSet[i](MENUDRAW_ParticleFlags[i]);
	}
}

static void MENUDRAW_SetAllParticle(bool onoff)
{
	for (u_int i = 0; i < MENUDRAW_ParticleTypeMax; ++i)
	{
		MENUDRAW_ParticleFlags[i] = onoff;
	}
}

static MENUENTRYBUTTONFUNCTION(MENUDRAW_AllParticleOff)
{
	switch ( buttonid )
	{
	case MENUCMD_CROSS:
		MENUDRAW_SetAllParticle(false);
		break;
	}
	return true;
}

static MENUENTRYBUTTONFUNCTION(MENUDRAW_AllParticleOn)
{
	switch ( buttonid )
	{
	case MENUCMD_CROSS:
		MENUDRAW_SetAllParticle(true);
		break;
	}
	return true;
}

// Miscellaneous Draw Menu
void MenuDraw::OnOpen(Menu *cto, MenuSystem *c)
{
	for (u_int i = 0; i < MENUDRAW_TypeMax; ++i)
	{
		MENUDRAW_Flags[i] = MENUDRAW_Get[i]();
	}
	// For "All On" and "All Off" to be recursive.
	// A little wasteful, since these will be called again
	// when the submenus are activated.  Also, this temporarily
	// sets the parent of these menus incorrectly. (dc 10/18/01)
	MENUDRAW_MenuEntities->OnOpen(cto, c);
	MENUDRAW_MenuParticle->OnOpen(cto, c);
	MENUDRAW_MenuWater->OnOpen(cto, c);

	Menu::OnOpen(cto, c);
}

void MenuDraw::OnTick(float dtime)
{
	for (u_int i = 0; i < MENUDRAW_TypeMax; ++i)
	{
		MENUDRAW_Set[i](MENUDRAW_Flags[i]);
	}
	// For "All On" and "All Off" to be recursive.
	MENUDRAW_MenuEntities->OnTick(dtime);
	MENUDRAW_MenuParticle->OnTick(dtime);
	MENUDRAW_MenuWater->OnTick(dtime);
}

static void MENUDRAW_SetAll(bool onoff)
{
	for (u_int i = 0; i < MENUDRAW_TypeMax; ++i)
	{
		MENUDRAW_Flags[i] = onoff;
	}
}

static MENUENTRYBUTTONFUNCTION(MENUDRAW_AllOff)
{
	switch ( buttonid )
	{
	case MENUCMD_CROSS:
		MENUDRAW_SetAll(false);
		MENUDRAW_SetAllEntities(false);
		MENUDRAW_SetAllParticle(false);
		MENUDRAW_SetAllWater(false);
		break;
	}
	return true;
}

static MENUENTRYBUTTONFUNCTION(MENUDRAW_AllOn)
{
	switch ( buttonid )
	{
	case MENUCMD_CROSS:
		MENUDRAW_SetAll(true);
		MENUDRAW_SetAllEntities(true);
		MENUDRAW_SetAllParticle(true);
		MENUDRAW_SetAllWater(true);
		break;
	}
	return true;
}

static bool MENUDRAW_GetDrawIGO(void)
{
	return g_igo_enabled;
}

static void MENUDRAW_SetDrawIGO(bool onoff)
{
	g_igo_enabled = onoff;
}

static bool MENUDRAW_GetDrawShadow(void)
{
	return WavetexDebug_ShadowPass;
}

static void MENUDRAW_SetDrawShadow(bool onoff)
{
	WavetexDebug_ShadowPass = onoff;
}

