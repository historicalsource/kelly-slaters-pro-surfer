// Board Select Front End Screen

#ifndef BOARDFRONTEND_H
#define BOARDFRONTEND_H


#include "FEMenu.h"
#include "entity_maker.h"
#include "entityflags.h"

/*
	this class is going to be a bit different; it's not really 
	going to be a menu
*/

class GraphicalMenuSystem;
#define LOCATION_BOARD -1
#define PERSONALITY_BOARD 0
class BoardFrontEnd : public FEMultiMenu
{
public:
	// no enum needed; boards have no names, only numbers
	enum {	ROUND,
			SWALLOW,
			POINT,
			FLAT,
			TYPE_COUNT, };

	static const int num_3d_boards = 3;

private:
	bool unlocked;

	GraphicalMenuSystem* sys;
	int current_board_index;

	TextString* header;
	TextString* height;
	TextString* width;
	TextString* thick;
	TextString* tail;
	TextString* height_label;
	TextString* width_label;
	TextString* thick_label;
	TextString* tail_label;
	TextString* players[2];
	TextString* gauge_labels[4];
	TextString* top_gauge_labels[4];
	TextString* board_stats, *comb_stats;

	PanelQuad* arrows[2][2];	// left, right; on, off
	PanelQuad* top_gauges[4][4];
	PanelQuad* gauges[4][4];
	PanelQuad* red_gauges[4];	// additional gauges
	PanelQuad* line_a, *line_b;

	// to get the arrows to blink
	static const int arrow_timer = 3;
	int arrow_counter;
	int arrow_num;

	int surfer_index;
	bool wait_for_camera;

public:
	BoardFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pf_name);
	virtual ~BoardFrontEnd();
	virtual void Load();
	virtual void Update(time_value_t time_inc);
	virtual void Select();
	virtual void Select(int entry_index) { Select(); }
	virtual void Draw();
	virtual void OnActivate();
	virtual void OnUp(int c) {}
	virtual void OnDown(int c) {}
	virtual void OnLeft(int c);
	virtual void OnRight(int c);
	virtual void OnTriangle(int c);
	virtual void OnCross(int c);
	virtual void OnAnyButtonPress(int c, int b);
	bool IsLevelBoardUnlocked(int location);
	stringx GetLevelBoardName();

private:
	void SetPQIndices();
	void OnHighlightBoard();
	void SetDisplay(BoardData bd);
	void MaskGauge(int gauge, BoardData bd);
};

#endif