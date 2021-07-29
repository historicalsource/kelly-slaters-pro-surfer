
#ifndef INCLUDED_IGO_WIDGET_BALANCE_H
#define INCLUDED_IGO_WIDGET_BALANCE_H

#include "global.h"
#include "igo_widget.h"
#include "fepanel.h"

// HorizBalanceWidget - the onscreen horizontal balance meter
class HorizBalanceWidget : public IGOWidget
{
private:
	PanelQuad * meterPQ;
	PanelQuad * arrowPQ;
	PanelQuad * leftKnobPQ;
	PanelQuad * rightKnobPQ;
	PanelQuad * colorLeftPQ;
	PanelQuad * colorRightPQ;

	float		meterCenterX;

	bool		flashing;

public:
	// Creators.
	HorizBalanceWidget();
	~HorizBalanceWidget();

	// Modifiers.
	void SetDisplay(const bool d = true);
	void Init(PanelFile & panel, const int playerIdx);
	void Update(const float dt);
	void Draw(void);
	void Show(const bool s = true);
	void SetArrow(float f);
	void SetFillage(const float f);
	void FitToViewport(const int x1, const int y1, const int x2, const int y2);

	// Accessors.
};

#endif INCLUDED_IGO_WIDGET_BALANCE_H