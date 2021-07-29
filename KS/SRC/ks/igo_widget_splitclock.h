
#ifndef INCLUDED_IGO_WIDGET_SPLITCLOCK_H
#define INCLUDED_IGO_WIDGET_SPLITCLOCK_H

#include "global.h"
#include "igo_widget.h"
#include "fepanel.h"

// SplitClockWidget - the onscreen digital clock thingy displayed in splitscreen modes.
class SplitClockWidget : public IGOWidget
{
private:
	PanelQuad *		bgPQ;
	TextString *	timeText;

	float			clockSec;
	int				clockMin;

private:
	stringx GetTimeText(const int min, const float sec) const;

public:
	// Creators.
	SplitClockWidget();
	virtual ~SplitClockWidget();

	// Modifiers.
	virtual void SetDisplay(const bool d = true);
	virtual void Init(PanelFile & panel, Font * font);
	virtual void SetTime(const int newClockMin, const float newClockSec);
	virtual void Update(const float dt);
	virtual void Draw(void);

	// Accessors.
};

#endif INCLUDED_IGO_WIDGET_SPLITCLOCK_H