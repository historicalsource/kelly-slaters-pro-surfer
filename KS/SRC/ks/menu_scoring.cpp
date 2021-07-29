
#include "global.h"
#include "entity.h"
#include "path.h"
#include "entity_maker.h"
#include "entityflags.h"
#include "wds.h"
#include "profiler.h"
#include "marker.h"
#include "light.h"
#include "particle.h"
#include "item.h"
// BIGCULL #include "turret.h"
// BIGCULL #include "scanner.h"
#include "sky.h"

// BIGCULL #include "gun.h"
// BIGCULL #include "thrown_item.h"
// BIGCULL #include "melee_item.h"
#include "pmesh.h"
#include "file_finder.h"
#include "widget_entity.h"
#include "conglom.h"
#include "osdevopts.h"
// BIGCULL #include "manip_obj.h"
// BIGCULL#include "switch_obj.h"
#include "polytube.h"
#include "lensflare.h"
#include "FrontEndManager.h"
#include "menu_scoring.h"
#include "trick_system.h"

MENUENTRYBUTTONFUNCTION(SaveScoringButton)
{
	bool	filelocked = os_file::is_system_locked();

	if (filelocked)
		os_file::system_unlock();

	SaveScoringSystem();

	if (filelocked)
		os_file::system_lock();

	return true;
}

MENUENTRYBUTTONFUNCTION(LoadScoringButton)
{
	bool	memlocked = mem_malloc_locked();
	bool	filelocked = os_file::is_system_locked();

	mem_lock_malloc(false);
	if (filelocked)
		os_file::system_unlock();

	LoadScoringSystem();

	mem_lock_malloc(memlocked);
	if (filelocked)
		os_file::system_lock();
	return true;
}

Submenu * menu_scoring = NULL;

extern float air_base;
extern float speed_base;
extern float rotate_base;
extern float special_meter_bonus;

void MENU_SCORING_SYSTEM_StaticInit( void )
{
	// Attribute menu externs.

	// Attribute menu entries.
	MenuEntryTitle *	at_title = NEW MenuEntryTitle("Attribute Menu");
	MenuEntryIntEdit *	at_use = NEW MenuEntryIntEdit("Use These Attributes", &g_surfer_attribs.use_debug_values, 0, 1);
	MenuEntryIntEdit *	at_s_air = NEW MenuEntryIntEdit("Surfer Air", &g_surfer_attribs.player_air, 1, 20);
	MenuEntryIntEdit *	at_s_speed = NEW MenuEntryIntEdit("Surfer Speed", &g_surfer_attribs.player_speed, 1, 20);
	MenuEntryIntEdit *	at_s_rotate = NEW MenuEntryIntEdit("Surfer Spin", &g_surfer_attribs.player_rotate, 1, 20);
	MenuEntryIntEdit *	at_s_balance = NEW MenuEntryIntEdit("Surfer Balance", &g_surfer_attribs.player_balance, 1, 20);
	MenuEntryIntEdit *	at_b_air = NEW MenuEntryIntEdit("Board Air", &g_surfer_attribs.board_air, 1, 10);
	MenuEntryIntEdit *	at_b_speed = NEW MenuEntryIntEdit("Board Speed", &g_surfer_attribs.board_speed, 1, 10);
	MenuEntryIntEdit *	at_b_rotate = NEW MenuEntryIntEdit("Board Spin", &g_surfer_attribs.board_rotate, 1, 10);
	MenuEntryIntEdit *	at_b_balance = NEW MenuEntryIntEdit("Board Balance", &g_surfer_attribs.board_balance, 1, 10);
	MenuEntryFloatEdit *	at_base_rotate = NEW MenuEntryFloatEdit("Base Spin", &rotate_base, 0.0f, 1.0f, 0.01f);
	MenuEntryFloatEdit *	at_base_speed = NEW MenuEntryFloatEdit("Base Speed", &speed_base, 0.0f, 1.0f, 0.01f);
	MenuEntryFloatEdit *	at_base_air = NEW MenuEntryFloatEdit("Base Air", &air_base, 0.0f, 1.0f, 0.01f);
	MenuEntryFloatEdit *	at_special_meter_bonus = NEW MenuEntryFloatEdit("Special Meter", &special_meter_bonus, 0.0f, 1.0f, 0.05f);
	MenuEntry *	at_entries[14] =
	{
		at_title,
		at_use,
		at_s_rotate,
		at_s_speed,
		at_s_balance,
		at_s_air,
		at_b_rotate,
		at_b_speed,
		at_b_balance,
		at_b_air,
		at_base_rotate,
		at_base_speed,
		at_base_air,
		at_special_meter_bonus,
	};

	// Attribute menu definition.
	Menu * at_menu = NEW Menu(NULL, 14, at_entries);
	Submenu * menu_attr = NEW Submenu("Attribute Menu", at_menu);

	// Floater menu externs.
	extern float dismount_percent;
	extern float dec_percent;
	
	// Floater menu entry definitions.
	MenuEntryTitle *		fl_title = NEW MenuEntryTitle("Floater Menu");
	MenuEntryFloatEdit *	flt_value = NEW MenuEntryFloatEdit("Time Value", &dismount_percent, 0.0f, 1.0f, .025f);
	MenuEntryFloatEdit *	fld_value = NEW MenuEntryFloatEdit("Deceleration Value", &dec_percent, 0.0f, 1.0f, .025f);
	MenuEntry * fl_entries[3] =
	{
		fl_title,
		flt_value,
		fld_value,
	};

	// Floater menu definition.
	Menu * fl_menu = NEW Menu(NULL, 3, fl_entries);
	Submenu * menu_fl = NEW Submenu("Floater Menu", fl_menu);

	// Trick menu definition.
	TrickMenu *	tr_menu = NEW TrickMenu(NULL, 0, NULL);
	Submenu *	menu_tricks = NEW Submenu("Trick Menu", tr_menu);

	// Scoring menu externs.

	// Scoring menu entry definitions.
	MenuEntryTitle * sc_title			= NEW MenuEntryTitle("Scoring Menu");
	MenuEntryFloatEdit * sc_d_mouth0	= NEW MenuEntryFloatEdit("(Distance) Section: Main", &ScoringManager::MOUTH_DISTANCES[0], 0.0f, 200.0f, 1.0f);
	MenuEntryFloatEdit * sc_d_mouth1	= NEW MenuEntryFloatEdit("(Distance) Section: Edge  ", &ScoringManager::MOUTH_DISTANCES[1], 0.0f, 200.0f, 1.0f);
	MenuEntryFloatEdit * sc_b_mouth0	= NEW MenuEntryFloatEdit("(Bonus) In Section: Near Mouth", &ScoringManager::SCALE_MOUTH_DISTS[0], 0.1f, 9.0f, 0.1f);
	MenuEntryFloatEdit * sc_b_mouth1	= NEW MenuEntryFloatEdit("(Bonus) In Section: Main Area ", &ScoringManager::SCALE_MOUTH_DISTS[1], 0.1f, 9.0f, 0.1f);
	MenuEntryFloatEdit * sc_b_mouth2	= NEW MenuEntryFloatEdit("(Bonus) In Section: Far Edge  ", &ScoringManager::SCALE_MOUTH_DISTS[2], 0.1f, 9.0f, 0.1f);
	MenuEntryFloatEdit * sc_b_lip0		= NEW MenuEntryFloatEdit("(Bonus) Face Section: Lip   ", &ScoringManager::SCALE_LIP_DISTS[0], 0.1f, 9.0f, 0.1f);
	MenuEntryFloatEdit * sc_b_lip1		= NEW MenuEntryFloatEdit("(Bonus) Face Section: Face  ", &ScoringManager::SCALE_LIP_DISTS[1], 0.1f, 9.0f, 0.1f);
	MenuEntryFloatEdit * sc_b_lip2		= NEW MenuEntryFloatEdit("(Bonus) Face Section: Pocket", &ScoringManager::SCALE_LIP_DISTS[2], 0.1f, 9.0f, 0.1f);
	MenuEntryFloatEdit * sc_b_spin1		= NEW MenuEntryFloatEdit("(Bonus) Spin: 180 ", &ScoringManager::SCALE_SPINS[1], 0.1f, 9.0f, 0.1f);
	MenuEntryFloatEdit * sc_b_spin2		= NEW MenuEntryFloatEdit("(Bonus) Spin: 360 ", &ScoringManager::SCALE_SPINS[2], 0.1f, 9.0f, 0.1f);
	MenuEntryFloatEdit * sc_b_spin3		= NEW MenuEntryFloatEdit("(Bonus) Spin: 540 ", &ScoringManager::SCALE_SPINS[3], 0.1f, 9.0f, 0.1f);
	MenuEntryFloatEdit * sc_b_spin4		= NEW MenuEntryFloatEdit("(Bonus) Spin: 720 ", &ScoringManager::SCALE_SPINS[4], 0.1f, 9.0f, 0.1f);
	MenuEntryFloatEdit * sc_b_spin5		= NEW MenuEntryFloatEdit("(Bonus) Spin: 900 ", &ScoringManager::SCALE_SPINS[5], 0.1f, 9.0f, 0.1f);
	MenuEntryFloatEdit * sc_b_spin6		= NEW MenuEntryFloatEdit("(Bonus) Spin: 1080", &ScoringManager::SCALE_SPINS[6], 0.1f, 9.0f, 0.1f);
	MenuEntryFloatEdit * sc_b_spin7		= NEW MenuEntryFloatEdit("(Bonus) Spin: 1260", &ScoringManager::SCALE_SPINS[7], 0.1f, 9.0f, 0.1f);
	MenuEntryFloatEdit * sc_b_spin8		= NEW MenuEntryFloatEdit("(Bonus) Spin: 1440", &ScoringManager::SCALE_SPINS[8], 0.1f, 9.0f, 0.1f);
	MenuEntryFloatEdit * sc_b_perfect	= NEW MenuEntryFloatEdit("(Bonus) Perfect", &ScoringManager::SCALE_LANDINGS[ScoringManager::LAND_PERFECT], 0.1f, 9.0f, 0.1f);
	MenuEntryFloatEdit * sc_b_regular	= NEW MenuEntryFloatEdit("(Bonus) Regular", &ScoringManager::SCALE_LANDINGS[ScoringManager::LAND_REGULAR], 0.1f, 9.0f, 0.1f);
	MenuEntryFloatEdit * sc_b_sloppy	= NEW MenuEntryFloatEdit("(Bonus) Sloppy", &ScoringManager::SCALE_LANDINGS[ScoringManager::LAND_SLOPPY], 0.1f, 9.0f, 0.1f);
	MenuEntryFloatEdit * sc_b_junk		= NEW MenuEntryFloatEdit("(Bonus) Junk", &ScoringManager::SCALE_LANDINGS[ScoringManager::LAND_JUNK], 0.1f, 9.0f, 0.1f);
	MenuEntryFloatEdit * sc_b_rep1		= NEW MenuEntryFloatEdit("(Bonus) 1st Repetition", &ScoringManager::SCALE_REPEATS[1], 0.0f, 1.0f, 0.05f);
	MenuEntryFloatEdit * sc_b_rep2		= NEW MenuEntryFloatEdit("(Bonus) 2nd Repetition", &ScoringManager::SCALE_REPEATS[2], 0.0f, 1.0f, 0.05f);
	MenuEntryFloatEdit * sc_b_rep3		= NEW MenuEntryFloatEdit("(Bonus) 3rd Repetition", &ScoringManager::SCALE_REPEATS[3], 0.0f, 1.0f, 0.05f);
	MenuEntryFloatEdit * sc_b_repn		= NEW MenuEntryFloatEdit("(Bonus) nth Repetition", &ScoringManager::SCALE_REPEATS[4], 0.0f, 1.0f, 0.05f);
	MenuEntryFloatEdit * sc_b_waveA		= NEW MenuEntryFloatEdit("(Bonus) Wave A", &ScoringManager::SCALE_WAVE[0], 0.0f, 9.0f, 0.10f);
	MenuEntryFloatEdit * sc_b_waveB		= NEW MenuEntryFloatEdit("(Bonus) Wave B", &ScoringManager::SCALE_WAVE[1], 0.0f, 9.0f, 0.10f);
	MenuEntryFloatEdit * sc_b_waveC		= NEW MenuEntryFloatEdit("(Bonus) Wave C", &ScoringManager::SCALE_WAVE[2], 0.0f, 9.0f, 0.10f);
	MenuEntryFloatEdit * sc_b_tf		= NEW MenuEntryFloatEdit("(Bonus) To Fakey", &ScoringManager::SCALE_SERIES_MODS[ScoringManager::MOD_TO_FAKEY], 0.1f, 9.0f, 0.1f);
	MenuEntryFloatEdit * sc_b_ff		= NEW MenuEntryFloatEdit("(Bonus) From Floater", &ScoringManager::SCALE_SERIES_MODS[ScoringManager::MOD_FROM_FLOATER], 0.1f, 9.0f, 0.1f);
	MenuEntryIntEdit * sc_m_fr			= NEW MenuEntryIntEdit("(Meter) Fill Rate         ", &ScoringManager::METER_FILL_RATE, 1000, 100000);
	MenuEntryIntEdit * sc_m_frf			= NEW MenuEntryIntEdit("(Meter) Fill Rate Face    ", &ScoringManager::METER_FILL_RATE_FACE, 1000, 100000);
	MenuEntryFloatEdit * sc_m_dr		= NEW MenuEntryFloatEdit("(Meter) Drop Rate        ", &SpecialMeter::DROP_RATE, 0.01f, 1.0f, 0.01f);
	MenuEntryFloatEdit * sc_m_sdr		= NEW MenuEntryFloatEdit("(Meter) Special Drop Rate", &SpecialMeter::DROP_RATE_SPECIAL, 0.0001f, 0.009f, 0.0001f, " : %0.4f");
	MenuEntryFloatEdit * sc_m_sdrf		= NEW MenuEntryFloatEdit("(Meter) Special Face Trick", &SpecialMeter::DROP_RATE_SPECIAL_FACE, 0.01f, 1.0f, 0.01f);
	MenuEntryFloatEdit * sc_m_ps		= NEW MenuEntryFloatEdit("(Meter) Sloppy Penalty    ", &ScoringManager::METER_PENALTY_SLOPPY, 0.0f, 1.0f, 0.01);
	MenuEntryFloatEdit * sc_m_pj		= NEW MenuEntryFloatEdit("(Meter) Junk Penalty      ", &ScoringManager::METER_PENALTY_JUNK, 0.0f, 1.0f, 0.01);
	MenuEntryFloatEdit * sc_m_pb		= NEW MenuEntryFloatEdit("(Meter) Perfect Bonus     ", &SpecialMeter::PERFECT_BONUS, 0.0f, 50.0f, 0.5);
	MenuEntryIntEdit * sc_c_link		= NEW MenuEntryIntEdit("(Cap) Link ", &ScoringManager::CAP_LINK, 1, 100);
	MenuEntryIntEdit * sc_c_cool		= NEW MenuEntryIntEdit("(Cap) Cool ", &ScoringManager::CAP_COOL, 1, 100);
	MenuEntryFunction * sc_save			= NEW MenuEntryFunction("Save", SaveScoringButton);
	MenuEntryFunction * sc_load			= NEW MenuEntryFunction("Load", LoadScoringButton);
	MenuEntry * sc_entries[45] = 
	{
		sc_title,
		menu_attr,
		menu_fl,
		menu_tricks,
		sc_d_mouth0,
		sc_d_mouth1,
		sc_b_mouth0,
		sc_b_mouth1,
		sc_b_mouth2,
		sc_b_lip0,
		sc_b_lip1,
		sc_b_lip2,
		sc_b_spin1,
		sc_b_spin2,
		sc_b_spin3,
		sc_b_spin4,
		sc_b_spin5,
		sc_b_spin6,
		sc_b_spin7,
		sc_b_spin8,
		sc_b_perfect,
		sc_b_regular,
		sc_b_sloppy,
		sc_b_junk,
		sc_b_rep1,
		sc_b_rep2,
		sc_b_rep3,
		sc_b_repn,
		sc_b_waveA,
		sc_b_waveB,
		sc_b_waveC,
		sc_b_tf,
		sc_b_ff,
		sc_m_fr,
		sc_m_frf,
		sc_m_dr,
		sc_m_sdr,
		sc_m_sdrf,
		sc_m_ps,
		sc_m_pj,
		sc_m_pb,
		sc_c_link,
		sc_c_cool,
		sc_save,
		sc_load,
	};

	// Scoring menu definition.
	Menu * sc_menu = NEW Menu(NULL, 45, sc_entries);
	menu_scoring = NEW Submenu("Scoring Menu", sc_menu);
}

void TrickMenu::OnOpen(Menu *cto, MenuSystem *c)
{
	bool				memlocked = mem_malloc_locked();
	MenuEntryIntEdit *	pmeie;

	// Trick menu externs.
	extern struct SurferTrick GTrickList[];

	mem_lock_malloc(false);

	ClearMenu();

	AddEntry(NEW MenuEntryTitle("Trick Menu"));

	for (int i = 0; i < TRICK_NUM; i++)
	{
		pmeie = NEW MenuEntryIntEdit(ksGlobalTrickTextArray[i].c_str(), &GTrickList[i].Points, 0, 10000);
		AddEntry(pmeie);
	}
	c->Refresh();

	Menu::OnOpen(cto, c);

	mem_lock_malloc(memlocked);
}
