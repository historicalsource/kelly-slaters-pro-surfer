
#ifndef INCLUDED_IGO_WIDGET_FANMETER_H
#define INCLUDED_IGO_WIDGET_FANMETER_H

#include "global.h"
#include "igo_widget.h"
#include "fepanel.h"

// FanMeterWidget - onscreen fan-like meter thingy
class FanMeterWidget : public IGOWidget
{
private:

private:
	int				numSections;
	PanelQuad4 **	sectionPQs;
	PanelQuad *		arrowPQ;

	float			size;
	int				centerX, centerY;
	float			angle;

private:
	//virtual void SelectSections(const int l, const int r, const int num);
	
public:
	// Creators.
	FanMeterWidget();
	virtual ~FanMeterWidget();

	// Modifiers.
	virtual void SetDisplay(const bool d = true);
	virtual void Init(PanelFile & panel);
	virtual void Update(const float dt);
	virtual void Draw(void);
	virtual void Show(const bool s);
	virtual void SetArrowPos(float f);
	virtual void SetSize(const float f);

	// Accessors.
};

#endif INCLUDED_IGO_WIDGET_FANMETER_H