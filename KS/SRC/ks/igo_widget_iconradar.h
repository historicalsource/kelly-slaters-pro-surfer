
#ifndef INCLUDED_IGO_WIDGET_ICONRADAR_H
#define INCLUDED_IGO_WIDGET_ICONRADAR_H

#include "global.h"
#include "igo_widget.h"
#include "fepanel.h"
#include "challenge_icon.h"

// IconRadarWidget - the onscreen ellipse and spinning icons for the ghost icon challenge.
class IconRadarWidget : public IGOWidget
{
private:
	struct OnscreenIcon
	{
		nglVector		screenPos;
		nglTexture *	texture;
	};

private:
	PanelQuad *		ellipsePQ;
	IconChallenge *	challenge;

	int				numIconTextures;
	nglTexture **	iconTextures;

	int				numOnscreenIcons;
	OnscreenIcon *	onscreenIcons;

public:
	// Creators.
	IconRadarWidget();
	~IconRadarWidget();

	// Modifiers.
	void Init(PanelFile & panel, IconChallenge * chall);
	void Update(const float dt);
	void Draw(void);

	// Accessors.
};

#endif INCLUDED_IGO_WIDGET_ICONRADAR_H