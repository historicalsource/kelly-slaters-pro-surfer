
#ifndef INCLUDED_IGO_WIDGET_SPLITMETER_H
#define INCLUDED_IGO_WIDGET_SPLITMETER_H

#include "global.h"
#include "igo_widget.h"
#include "fepanel.h"
#include "specialmeter.h"

// SplitMeterWidget - the onscreen trick meter displayed in splitscreen modes.
class SplitMeterWidget : public IGOWidget
{
private:
	static const color COLOR_NORMAL;
	static const color COLOR_SPECIAL;

	enum { NUM_FRAMES = 3 };

private:
	SpecialMeter *	meter;
	PanelQuad *		colorPQ;
	PanelQuad *		framePQs[NUM_FRAMES];

public:
	// Creators.
	SplitMeterWidget();
	virtual ~SplitMeterWidget();

	// Modifiers.
	virtual void SetDisplay(const bool d = true);
	virtual void Init(PanelFile & panel, SpecialMeter * specialMeter, const bool left);
	virtual void Update(const float dt);
	virtual void Draw(void);
	virtual void SetFillage(const float amt);

	// Accessors.
};

#endif INCLUDED_IGO_WIDGET_SPLITMETER_H