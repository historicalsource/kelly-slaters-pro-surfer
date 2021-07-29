#ifndef INCLUDED_IGO_WIDGET_METERCHALLENGE_H
#define INCLUDED_IGO_WIDGET_METERCHALLENGE_H

#include "global.h"
#include "igo_widget.h"
#include "fepanel.h"


class MeterChallengeWidget : public IGOWidget
{
private:

	static const float TIME_FADE;
	static const float TIME_ANIMATE;
	static const float SPEED_HILITE_FLASH;

	enum
	{
		STATE_NONE,
		STATE_ICON,
		STATE_360_SPIN,
		STATE_540_SPIN
	};

private:
	PanelQuad *	objectRoot;
	
	TextString *	Text;

	float		hiliteTime;		// amount of time highlight has been displayed
	float		fade;			// alpha of overlay
	int			fadeDir;		// 1 for fading in, -1 for fading out, 0 for no change


public:
	// Creators.
	MeterChallengeWidget();
	~MeterChallengeWidget();

	// Modifiers.
	void SetDisplay(const bool d = true);
	void Init(PanelFile & panel, Font * numberfont, Font * textfont, const color32 & textColor1,  const color32 & textColor2);
	void Update(const float dt);
	void Draw(void);
	void Hide(const bool fadeOut = true);
	void Show(const bool fadeIn = true);

	// Accessors.
	bool IsHiding(void) const { return (fadeDir == -1); }
};

#endif INCLUDED_IGO_WIDGET_METERCHALLENGE_H