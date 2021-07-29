
#ifndef INCLUDED_IGO_WIDGET_CAMERA_H
#define INCLUDED_IGO_WIDGET_CAMERA_H

#include "global.h"
#include "igo_widget.h"
#include "fepanel.h"

// CameraWidget - onscreen camera reticle image
class CameraWidget : public IGOWidget
{
private:
	enum { NUM_RETICLE_PQS = 28 };

private:
	PanelQuad *	reticlePQs[NUM_RETICLE_PQS];
	float		fade;
	float		showTimer;
	float		showTime;

private:
	void FadeReticle(void);
	
public:
	// Creators.
	CameraWidget();
	~CameraWidget();
	
	// Modifiers.
	void SetDisplay(const bool d = true);
	void Init(PanelFile & panel);
	void Reset(void);
	void Update(const float dt);
	void Draw(void);
	void Show(const float time);
	void Hide(void);
	
	// Accessors.
	bool IsShown(void) const { return reticlePQs[NUM_RETICLE_PQS-1] && reticlePQs[NUM_RETICLE_PQS-1]->IsOn(); }
	float GetFade(void) const { return fade; }
};

#endif INCLUDED_IGO_WIDGET_CAMERA_H