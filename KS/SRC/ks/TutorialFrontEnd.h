
// Photo Front End Screens

#ifndef INCLUDED_TUTORIALFRONTEND_H
#define INCLUDED_TUTORIALFRONTEND_H

#include "global.h"
#include "FEMenu.h"

// TutorialFrontEnd - photo selection menus that popup at the end of a photo challenge run.
// The first menu selects between the run's three photos, and the second menu chooses between the selected
// photo and the currently saved one.
// Also contains panel quads for the IGO camera reticle and polaroid.
class TutorialFrontEnd : public FEMultiMenu
{
public:
	static const float TIME_ARROW_HIGHLIGHT;

private:
	PanelQuad *	bgPQs[9];

	BoxText *	help_text;
	TextString *	pause_button_text;
	WaveIndicatorWidget *waveIndicator;
	bool ignore_next_release;

public:
	// Creators.
	TutorialFrontEnd(FEMenuSystem* s, FEManager* man);
	virtual ~TutorialFrontEnd();

	// Modifiers.
	virtual void Load(void);
	virtual void Update(time_value_t time_inc);
	virtual void Draw(void);
	virtual void OnActivate(void);
	virtual void OnUnactivate(FEMenu* m);
	virtual void OnButtonRelease(int c, int b);
	virtual void OnLeft(int c);
	virtual void OnRight(int c);
	virtual void OnUp(int c);
	virtual void OnDown(int c);
	virtual void OnTriangle(int c);

};

#endif INCLUDED_TUTORIALFRONTEND_H