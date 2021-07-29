// Beach Select Front End Screen

#ifndef BEACHFRONTEND_H
#define BEACHFRONTEND_H

#include "FEMenu.h"
#include "beachdata.h"
#include "Map.h"

class GraphicalMenuSystem;

struct BFE_Level
{
	bool locked;
	int beach;
	bool done;
};

#define MAX_BEACHES_PER_LOC 3

struct BFE_Location
{
	int levels[MAX_BEACHES_PER_LOC];
	int beaches[MAX_BEACHES_PER_LOC];
	bool all_levels_locked;
	bool all_beaches_locked;
	bool no_existing_levels;
};	

struct BFE_Beach
{
	bool locked;
	int location;
	// -1 if this is the main beach of the location; else the main beach's beach number
	int main_beach;
	// -1 unless there is a later beach at this location
	int secondary_beaches[MAX_BEACHES_PER_LOC-1];
	bool new_challenges;
	bool done;
};

class BeachFrontEnd : public FEMultiMenu
{
public:
  friend class DemoModeManager;

	FEMenuSystem* sys;

private:
	FEMenuEntry* first;
	FEMenuEntry* last;
	FEMenuEntry* current;

	BFE_Level levels[LEVEL_LAST];
	BFE_Beach beaches[BEACH_LAST];
	BFE_Location locations[MAP_LOC_LAST];

	// all -1 if on save/load or return to fe
	int current_beach_index;
	int current_level_index;
	int current_location_index;

	int cur_we_index;	// current west-east order index
	int home_beach;

	int goal_num[MAX_BEACHES_PER_LOC];		// number of goals for each challenge on this beach
	int on_challenge;						// index of currently highlighted challenge

	FEMenuEntry* chall_text[MAX_BEACHES_PER_LOC];
	TextString* goals_completed[MAX_BEACHES_PER_LOC];
	BoxText* goal_text[MAX_BEACHES_PER_LOC][MAX_GOALS_PER_LEVEL];	// goals for each challenge
	BoxText* description;
	BoxText* beach_bio;
	TextString* bio_title;

/*	Save/load no longer part of map screen.  (dc 07/01/02)
	FEMenuEntry* save_load;
*/
	FEMenuEntry* fe_return;
	FEMenuEntry* entry_list[BEACH_LAST];
	FEMenuEntry* yes_entry, *no_entry;
	BoxText* return_question;

	int voiceOverStage;
	nslSoundId voiceOverSound;
	SSEventId  voiceOverEvent;
	// blinking points
	float blink_counter;
	int blink_index;
	bool blink_on;
	static const int max_blink_index = 3;

	bool in_description_mode;
	bool in_bio_mode;		// for beach bios
	bool key_box_up;

	// text positions:
	static const int box_t_y = 40;
	static const int box_b_y = 440;
	static const int box_d_l_x = 60;	// for description box
	static const int box_d_r_x = 580;
	static const int box_d_t_y = 50;
	static const int box_d_b_y = 405;

	static const int box_offset = 22;
	static const int check_offset = 6;
	static const int goal_offset = 20;

	int box_z;
	float box_l_x[4], box_r_x[4], box_y[4];	// 4 lines to divide 3 columns and 3 rows
	float box_d_y[4];
	float box_k_x[4], box_k_y[4];			// key box
	int beach_l_l_x, beach_l_r_x, beach_r_l_x, beach_r_r_x;
	int check_l_x, check_r_x;
	int goal_l_x, goal_r_x;
	int line_l_x, line_r_x, line_y;
	static const int beach_y = 56;
	static const int chall_y = 75;
	static const int check_y = 93;

	static const int MAX_MAP_LOADING = 3;
	int map_wait_loading;

	// only in fe
	bool offset_set;
	vector2d map_points_offset;
	vector2d map_points_multiplier;

	PanelQuad* here[BEACH_LAST];
	PanelQuad* selected[BEACH_LAST];
	PanelQuad* open[BEACH_LAST];	// challenges not done
	PanelQuad* done[BEACH_LAST];	// challenges all done
	PanelQuad* select_circle[BEACH_LAST][max_blink_index];
	PanelQuad* new_open_circle[BEACH_LAST][max_blink_index];

	// code-generated boxes/lines
	PanelQuad* box[9][2];
	PanelQuad* line_across, *line_down;
	PanelQuad* completed_lights[MAX_BEACHES_PER_LOC][MAX_GOALS_PER_LEVEL][2];	// challenge, goal, on/off
	PanelQuad* locked_light_template;
	PanelQuad* locked_first_light_template;
	PanelQuad* unlocked_light_template;
	PanelQuad* path_pq;
	PanelQuad* bkg;
	PanelQuad* cell_phone;
	PanelQuad* chall_div_line;
	PanelQuad* bio_book, *bio_circle;
	PanelQuad* bio_scr[3], *bio_scr_marker;

	static const int bio_count_max = 3;
	int bio_counter;

	bool bio_up_pressed, bio_down_pressed;
	float bio_scroll_x, bio_scroll_y_t, bio_scroll_y_b;
	int bio_total_lines, bio_total_visible;

public:
	entity* map_ent;

private:
	MapData map;
	bool no_progress_path;		// beaches too close or exactly the same

	bool in_frontend;
	bool in_career_mode;
	bool in_demo_mode;
	bool boxes_on;				// the frames and background behind the beach and challenge text (dc 04/27/02)
	bool sliding_in;			// or out
	bool ignore_controller;		// because still sliding
	bool first_time;

	// stuff saved when going to FEreturn
	FEMenuEntry* former_high;
	FEMenuEntry* former_secondary;
	int former_we_index;
	int former_beach_index;
	int former_level_index;

public:
	BeachFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pf_name);
	virtual ~BeachFrontEnd();
	virtual void Load();
	virtual void ReloadPanel();
	virtual void ReloadMap() { map.Reload(path_pq); }
	virtual void Update(time_value_t time_inc);
	virtual void UpdateInScene();
	virtual void Draw();
	virtual void DrawMap(float loading_progress);
	virtual void OnLevelLoaded();
	virtual void OnLevelEnding();
	virtual void Select(int entry_index);
	virtual void Pick();
	virtual void PickDemo(int beach);
	void HideAllDots();
	virtual void OnActivate();
	virtual void OnLeft(int c);
	virtual void OnRight(int c);
	virtual void OnUp(int c);
	virtual void OnDown(int c);
	virtual void OnTriangle(int c);
	virtual void OnCross(int c);
	virtual void OnCircle(int c);
	virtual void OnAnyButtonPress(int c, int b);
	virtual void OnButtonRelease(int c, int b);
	void SwitchState(FEMenuSystem* s, bool in_fe);
	void To2dFEMap();

	int GetCurrentBeach() { return current_beach_index; }
	int GetCurrentLevel() { return current_level_index; }
	int GetCurrentLocation() { return current_location_index; }
	
	void PutUpLoadingScreen();
	void PrepareForLoading();
	void SetForPractice();
	void SkipSlide(bool in);
	bool IsSliding(void) const { return panel.IsSliding(); }
	bool IsSlideOutDone(void) const { return panel.IsSlideOutDone(); }
	bool exitingWithoutSelect;
	void ResetFirstTime() { first_time = true; }
private:
	void SwitchBeach(bool right);
	void SetOnChallenge(int chall);
	void UpdateBeach(bool reset_challenge);
	int UpdateChallenges(int chall_num, int offset, bool right);
	int UpdateGoals(int chall_num, int offset, bool right);
	void TurnBox(int b_num, bool all_off, float offset);
	virtual void SetPQIndices();
	void SetBeachPoints();

	// most of these functions are to break up the ::OnActivate function
	// and eliminate duplicate code in ::OnActivate and ::PickDemo
	void SetBeachData();
	void FindFirstAvailable(int level);
	void StartFrontendFade();
	void SetProgressPath();

	bool Realistic(bool press_build_only);
	void SetOffset();
	void ReadjustBeachPoints();
	vector2d ApplyOffset(vector2d);
	void SetDescription(bool on, bool update_beach=true);
	void SetKeyBox(bool on);
	void SetBeachBio(bool on);
	void UpdateBeachBioScrollbar();
	bool BeachBioExists(stringx& tmp);
	void ResetHelpbar();	// reset to non-bio, non-desc helpbar
	void ToFEReturn();
	void FromFEReturn();
	void ReturnToFE();
};


#endif