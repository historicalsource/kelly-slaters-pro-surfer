// Multi Select Front End Screen


// not currently being used
#if 0



#ifndef MULTIFRONTEND_H
#define MULTIFRONTEND_H

#include "FEMenu.h"


class MultiFrontEnd : public FEMultiMenu
{
public:
	enum
	{
		MultiPushEntry,
		MultiHeadToHeadEntry,
		MultiTimeAttackEntry,
		MultiEndEntry
	};

private:
	static const int num_ops = MultiEndEntry;
	int cur_index;

	PanelQuad* pq[num_ops];
	PanelQuad* pqhi[num_ops];
	PanelQuad* box[9];
	TextString* pushTitle;
	TextString* headToHeadTitle;
	TextString* timeAttackTitle;
	//TextString* meterAttackTitle;
	BoxText* pushDesc;
	BoxText* headToHeadDesc;
	BoxText* timeAttackDesc;
	//BoxText* meterAttackDesc;

public:
	MultiFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pf_name);
	~MultiFrontEnd();
	virtual void Draw();
	virtual void Load();
	virtual void Select(int entry_index);
	virtual void OnActivate();
	virtual void OnUp(int c);
	virtual void OnDown(int c);
	virtual void OnTriangle(int c);
private:
	virtual void SetPQIndices();
};


#endif



#endif // 0
