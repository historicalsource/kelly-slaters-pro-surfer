
#ifndef INCLUDED_IGO_WIDGET_ANALOGCLOCK_H
#define INCLUDED_IGO_WIDGET_ANALOGCLOCK_H

#include "global.h"
#include "igo_widget.h"
#include "fepanel.h"

// AnalogClockWidget - the onscreen analog game clock displayed in one-player modes.
class AnalogClockWidget : public IGOWidget
{
private:
	static const int		MAX_TIME_SEGMENTS = 60;
	static const float		SEG_TURN;
	static const float		CLOCK_CENTER_X;
	static const float		CLOCK_CENTER_Y;
	static const float		CLOCK_RADIUS;
	static const color		COLOR_CLOCK;
	static const color		COLOR_ELAPSED;
	static const color		COLOR_FLASH;
	static const color32	COLOR_SCORE;

private:
	int				clockMin;
	float			clockSec;

	float			elapsedInterval;

	PanelQuad *		clockHandPQ;
	PanelQuad *		clockFramePQ;
	PanelQuad *		clockFacePQ;
	PanelQuad *		num1PQ;
	PanelQuad *		num2PQ;
	PanelQuad *		num3PQ;
	PanelQuad *		num4PQ;
	PanelQuad *		num5PQ;
	PanelQuad *		infPQ;
	PanelQuad4 *	timeSegs[MAX_TIME_SEGMENTS];
	float			handCenterX, handCenterY;

	int				score;
	TextString *	scoreText;

private:
	stringx GetScoreText(const int sc) const;

public:
	// Creators.
	AnalogClockWidget();
	virtual ~AnalogClockWidget();

	// Modifiers.
	virtual void SetDisplay(const bool d = true);
	virtual void Init(PanelFile & panel, Font * numberFont);
	virtual void Update(const float dt);
	virtual void Draw(void);
	virtual void SetScore(const int sc);
	virtual void ShowElapsedTime(const float t);
	virtual void HideElapsedTime(void);

	// Accessors.
};

#endif INCLUDED_IGO_WIDGET_ANALOGCLOCK_H