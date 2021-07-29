
#ifndef INCLUDED_IGO_WIDGET_WAVEINDICATOR_H
#define INCLUDED_IGO_WIDGET_WAVEINDICATOR_H

#include "global.h"
#include "igo_widget.h"
#include "fepanel.h"

// WaveIndicatorWidget - the onscreen wave thingy that shows crash direction and warns of breaking waves.
class WaveIndicatorWidget : public IGOWidget
{
public:
	enum STATE
	{
		STATE_NONE,			// nothing drawn
		STATE_DIR,			// showing wave direction
		STATE_SURGE,		// showing a wave surge
		STATE_TONGUE,		// showing a wave tongue
		STATE_HILITE_NONE,	// showing wave, no part flashing
		STATE_HILITE_FACE,	// showing wave, flashing face section
		STATE_HILITE_LIP,	// showing wave, flashing lip section
		STATE_HILITE_TUBE,	// showing wave, flashing tube section
	};

private:
	enum { NUM_WAVES = 3 };
	enum { NUM_TONGUES = 6 };

	static const float TIME_FADE;
	static const float TIME_TUTORIAL_FADE;
	static const float TIME_ANIMATE;
	static const float SPEED_HILITE_FLASH;

private:
	PanelQuad *	wavePQ[2][NUM_WAVES];
	PanelQuad * arrowPQ[2];
	PanelQuad * tonguePQ[2][NUM_TONGUES];
	PanelQuad *	hilitePQ[2][3];
	PanelQuad *	heightPQ;
	
	TextString *	heightText;
	TextString *	unitText;
	TextString *	nextHeightText;
	TextString *	nextUnitText;

	int			dirIdx;
	float		waveIdx;
	float		tongueIdx;

	float		hiliteTime;		// amount of time highlight has been displayed
	float		fade;			// alpha of overlay
	int			fadeDir;		// 1 for fading in, -1 for fading out, 0 for no change
	STATE		state;			// what we are currently displaying

public:
	// Creators.
	WaveIndicatorWidget();
	~WaveIndicatorWidget();

	// Modifiers.
	void SetDisplay(const bool d = true);
	void Init(PanelFile & panel, const bool left, Font * font, const color32 & heightColor, const color32 & nextHeightColor);
	void Update(const float dt);
	void Draw(void);
	void ShowDirection(const bool fadeIn, const float waveHeight, const float nextWaveHeight);
	void ShowSurge(const bool fadeIn = true);
	void ShowTongue(const bool fadeIn = true);
	void ShowHighlight(const int section, const bool fadeIn = true);
	void Hide(const bool fadeOut = true);

	// Accessors.
	bool IsHiding(void) const { return state == STATE_NONE || fadeDir == -1; }
	STATE GetState(void) const { return state; }
};

#endif INCLUDED_IGO_WIDGET_WAVEINDICATOR_H