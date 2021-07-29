// FrontEndManager.h

#ifndef FEMANAGER_H
#define FEMANAGER_H

#include "FEEntityManager.h"

#ifdef TARGET_GC
#include "GCMCDetectFrontEnd.h"
#endif

// the main class that holds all menu systems or overlays
class FEManager
{
public:
	IGOFrontEnd* IGO;
	PauseMenuSystem* pms;
	GraphicalMenuSystem* gms;
	FEEntityManager* em;
	LegalFrontEnd* lfe;
#ifdef TARGET_GC
	PAL60FrontEnd* pfe;
#endif
	TitleFrontEnd* tfe;
	// all beaches, plus an High Icons list
	HighScoreData high_score[BEACH_LAST+1][HighScoreFrontEnd::NUM_ROWS];	// saves 14 high scores per beach

	bool gms_skip_legal;

	// slowly weeding these out....
	Font trick_font;
	Font numberFont;
	//Font clockFont;

	// .... in favor of these
	Font font_hand;
	Font font_thin;
	Font font_bold;
	Font font_info;
	Font font_body;	// old body
	Font font_bold_old;	// old bold

	// slowly weeding these out....
	color32 yel_lt;		// menu entry highlights
	color32 yel_dk;		// unselected menu entries
	color32 green;		// menu entry highlights
	color32 green_br;	// save/load screen
	color32 blue;		// other text; unselected menu entries
	color32 white;		// other text
	color32 gray_dk;	// other text on light background
	color32 red_dk;		// highlight for gray text
	color32 blue_dk;	// text in name entry menu

	// ...in favor of these
	color32 col_unselected;		// white
	color32 col_highlight;		// light yellow, alternating with below
	color32 col_highlight2;		// dark yellow
	color32 col_menu_header;	// dark yellow
	color32 col_help_bar;		// gray
	color32 col_info_b;			// blue
	color32 col_info_g;			// green
	color32 col_bio;			// dark gray
	color32 col_bad;			// red

	float bio_scale;
	PanelAnimFile* paf;  // delete this ???
	bool IGO_active;     // it's either the IGO or gms
	bool score_display;
	bool start_on;
	int current_surfer;
	game_mode_t tmp_game_mode;

	bool fe_init;
	bool fe_done;
	bool fe_done_loading;
	bool in_game_map_up;
	bool return_to_fe;		// only used for in-game map
	bool map_loading_screen;
	bool new_photo;			// photo challenge completed in a recent run; Scrapbook should be first FE screen
	bool unsaved_career;
	bool extras_movie;		// so FE will return to Extras Menu after being deleted
	int  extras_movie_sub;      // If returning to extras menus, return to this sub menu
	int  extras_movie_sub_sub;  // If returning to extras menus, return to this sub-sub menu

	BeachFrontEnd* map;
	HelpbarFE* helpbar;

public:
	FEManager();
	virtual ~FEManager();

	void GCConstructorKludge1( void );
	void GCConstructorKludge2( void );
	void LoadMap();
	void ReloadTextures();
	void InitFE();
	void InitIGO();
	void UpdateFE(time_value_t time_inc);
	void UpdateIGO(time_value_t time_inc);
	void UpdateIGOScene();
	void DrawFE();
	void DrawIGO();
	void DrawMap(float loading_progress);
	void OnLevelLoaded();
	void OnLevelEnding();
	void enableScoreDisplay(bool on) { score_display = on; }
	bool IsMenuActivated(void) const { return pms->draw; }
	void ReleaseIGO();
	void ReleaseFE();
	int AddHighScore(stringx name, int score, int icons, int character);
	void LoadFonts();
	FloatingPQ* GetDefaultPQ();

private:
	bool fontsLoaded;
	void LoadScores();
	void ReloadFontTextures();

	FloatingPQ* default_pq;
};

extern FEManager frontendmanager;	// global variable, defined in FrontEndManager.cpp
extern bool g_igo_enabled;

bool FEInitialized();
void FEInit();
void FEUpdate(time_value_t time_inc);
void FEDraw();
bool FEDone();
bool FEDoneLoading();
void FERelease();
void IGOUpdate(time_value_t time_inc);
void IGODraw();
void IGOStandUp();              // when surfer stands up
void IGODebug(bool on);
void IGOPrint(stringx text);
bool IGOIsPaused();
void IGORelease();

#endif
