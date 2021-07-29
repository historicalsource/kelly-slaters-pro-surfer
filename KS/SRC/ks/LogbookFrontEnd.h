// Logbook Front End Screen

#ifndef LOGBOOKFRONTEND_H
#define LOGBOOKFRONTEND_H


#include "FEMenu.h"
class GraphicalMenuSystem;

class LogbookFrontEnd : public FEMultiMenu
{
private:
	GraphicalMenuSystem* sys;
	// have no more than 1 entry per level
	static const int max_notes = LEVEL_LAST;
	static const int max_line_size = 400;
	BoxText* notes[max_notes];		// indexed by order completed
	TextString* dates[max_notes];	// indexed by order completed
	stringx note_body[max_notes];	// indexed by level
	int note_page_num[max_notes];	// indexed by order completed
	PanelQuad* book;
	int cur_spread;
	int max_spread;
/*
	static const int date_l_x = 104;
	static const int note_l_x = 115;
	static const int edge_l_x = 295;
	static const int date_r_x = 345;
	static const int note_r_x = 356;
	static const int edge_r_x = 564;
*/
	static const int date_l_x = 95;
	static const int edge_l_x = 290;
	static const int date_r_x = 350;
	static const int edge_r_x = 573;
	
	static const int start_y = 96;
	static const int end_y = 359;
	static const int y_diff = 20;
	static const int y_extra_spacing = 10;

public:
	LogbookFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pf_name);
	virtual ~LogbookFrontEnd();
	virtual void Load();
	virtual void Select(int entry_index) {}
	virtual void Draw();
	virtual void OnActivate();
	virtual void OnLeft(int c) { if(cur_spread > 0) cur_spread--; }
	virtual void OnRight(int c) { if(cur_spread < max_spread) cur_spread++; }
	virtual void OnTriangle(int c);
	
private:
	void ReadNotesFromFile();
	void UpdateUnlockedNotes();
};

#endif