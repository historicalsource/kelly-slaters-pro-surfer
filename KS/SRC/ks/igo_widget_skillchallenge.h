#ifndef INCLUDED_IGO_WIDGET_SKILLCHALLENGE_H
#define INCLUDED_IGO_WIDGET_SKILLCHALLENGE_H

#include "global.h"
#include "igo_widget.h"
#include "fepanel.h"
#include "career.h"

class SkillChallengeWidget : public IGOWidget
{
private:

	static const float TIME_FADE;
	static const float TIME_ANIMATE;
	static const float SPEED_HILITE_FLASH;

	enum
	{
		STATE_NONE,
		STATE_FACE,
		STATE_FACE_SCORE,
		STATE_AIR,
		STATE_AIR_SCORE,
		STATE_TUBE,
		STATE_TUBE_SCORE,
		STATE_PHOTO1,
		STATE_PHOTO2,
		STATE_PHOTO3
	};

private:
	PanelQuad *	objectRoot;
	
	TextString *	skillText;
	TextString *	pointText;

	float		hiliteTime;		// amount of time highlight has been displayed
	float		fade;			// alpha of overlay
	int			fadeDir;		// 1 for fading in, -1 for fading out, 0 for no change

	int state;
	int num_photos_taken;
	float timer;
	int points;

public:
	// Creators.
	SkillChallengeWidget(int type);
	~SkillChallengeWidget();

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

#endif INCLUDED_IGO_WIDGET_SKILLCHALLENGE_H