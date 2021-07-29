
#ifndef INCLUDED_IGO_WIDGET_PHOTO_H
#define INCLUDED_IGO_WIDGET_PHOTO_H

#include "global.h"
#include "igo_widget.h"
#include "fepanel.h"

// PhotoWidget - snapshot camera reticle and onscreen mini-photo IGOs.
class PhotoWidget : public IGOWidget
{
private:
	static const float TIME_SHOWN;
	static const float TIME_FADE_IN;
	static const float TIME_FADE_OUT;

private:
	PanelQuad *		borderPQ;
	int				borderCenterX, borderCenterY, borderZ;
	nglTexture *	photoTexture;
	nglQuad			photoQuad, darkQuad;
	float			darkFade;
	float			fadeOutAlpha;

	float			shownTimer;
	bool			timed;

	int				photoNum;
	TextString *	pointText;
	int				score, * scorePtr;

private:
	virtual void SetPointText(void);
	
public:
	// Creators.
	PhotoWidget();
	~PhotoWidget();

	// Modifiers.
	void SetDisplay(const bool d = true);
	void Init(PanelQuad * pq, Font * font);
	void Reset(void);
	void Update(const float dt);
	void Draw(void);
	void Show(nglTexture * tex, int * sc, const int photoNum);
	void Show(nglTexture * tex, int * sc, const int photoNum, const float fade);
	void Hide(void);
	void SetPosition(const int x, const int y, const int z);

	// Accessors.
	bool IsShown(void) const { return photoTexture != NULL; }
};

#endif INCLUDED_IGO_WIDGET_PHOTO_H