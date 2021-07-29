// Stats Select Front End Screen


// not currently being used
#if 0


#ifndef STATSFRONTEND_H
#define STATSFRONTEND_H

#include "SurferFrontEnd.h"
#include "FEMenu.h"

class GraphicalMenuSystem;

class StatsFrontEnd : public FEMultiMenu
{
private:
	GraphicalMenuSystem* sys;
	int cur_surfer;
	int cur_beach;

//	static const int num_beaches = BeachFrontEnd::num_beaches+1;
	static const int BeachOverallEntry = BEACH_LAST;
	static const int num_beaches = BeachOverallEntry+1;

	// pq pointers
	PanelQuad* pq_name[SURFER_LAST];
	PanelQuad* pq_beach[num_beaches];

	TextString* ts_tube;
	TextString* ts_float;
	TextString* ts_air;
	TextString* ts_score;
	TextString* ts_trick;

	int num_extras;		// extras are people without art
	TextString* extras;
	int* extra_indices;
	int current_extra_index;

	int num_extras_b;		// extras are people without art
	TextString* extras_b;
	int* extra_indices_b;
	int current_extra_index_b;


public:
	StatsFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pf_name);
	virtual ~StatsFrontEnd();
	virtual void Load();
	virtual void Draw();
	virtual void OnActivate();
	virtual void OnLeft(int c);
	virtual void OnRight(int c);
	virtual void OnTriangle(int c);
private:
	void OnSwitchBeach();
	void SetPQIndices();
};

#endif


#endif // 0
