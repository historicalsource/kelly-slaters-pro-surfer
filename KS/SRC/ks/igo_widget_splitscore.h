
#ifndef INCLUDED_IGO_WIDGET_SPLITSCORE_H
#define INCLUDED_IGO_WIDGET_SPLITSCORE_H

#include "global.h"
#include "igo_widget.h"
#include "fepanel.h"

// SplitScoreWidget - the onscreen score background thingy displayed in splitscreen modes.
class SplitScoreWidget : public IGOWidget
{
private:
	SpecialMeter *	meter;
	PanelQuad *		bgPQ;
	TextString *	scoreText;

public:
	// Creators.
	SplitScoreWidget();
	virtual ~SplitScoreWidget();

	// Modifiers.
	virtual void SetDisplay(const bool d = true);
	virtual void Init(PanelFile & panel, Font * font, const bool left);
	virtual void Update(const float dt);
	virtual void Draw(void);
	virtual void SetScore(const int score);

	// Accessors.
};

#endif INCLUDED_IGO_WIDGET_SPLITSCORE_H