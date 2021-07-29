
#ifndef INCLUDED_IGO_WIDGET_TIMEATTACK_H
#define INCLUDED_IGO_WIDGET_TIMEATTACK_H

#include "global.h"
#include "igo_widget.h"
#include "fepanel.h"

// TimeAttackWidget - the onscreen digital clock thingy displayed in Time Attack mode.
class TimeAttackWidget : public IGOWidget
{
private:
	static const color32 COLOR_TIME;
	static const color32 COLOR_TIME_ATTACK;
	static const color32 COLOR_SCORE;

private:
	PanelQuad *		bgPQ;

	float			clockSec;
	int				clockMin;
	TextString *	timeText;

	float			attackClockSec;
	int				attackClockMin;
	TextString *	attackText;

	int				score;
	TextString *	scoreText;

private:
	stringx GetTimeText(const int min, const float sec) const;
	stringx GetScoreText(const int sc) const;

public:
	// Creators.
	TimeAttackWidget();
	virtual ~TimeAttackWidget();

	// Modifiers.
	virtual void SetDisplay(const bool d = true);
	virtual void Init(PanelFile & panel, Font * numberFont, Font * clockFont);
	virtual void Update(const float dt);
	virtual void Draw(void);
	virtual void SetTime(const int newClockMin, const float newClockSec);
	virtual void SetAttackTime(const int newClockMin, const float newClockSec);
	virtual void SetScore(const int sc);

	// Accessors.
};

#endif INCLUDED_IGO_WIDGET_TIMEATTACK_H