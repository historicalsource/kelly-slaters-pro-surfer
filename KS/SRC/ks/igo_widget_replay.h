
#ifndef INCLUDED_IGO_WIDGET_REPLAY_H
#define INCLUDED_IGO_WIDGET_REPLAY_H

#include "global.h"
#include "igo_widget.h"
#include "fepanel.h"

// ReplayWidget - vcr-like control used during replays
class ReplayWidget : public IGOWidget
{
public:
  typedef enum { VCR_RESTART, VCR_PAUSE, VCR_PLAY, VCR_SLOW, VCR_FASTFORWARD } VCRButtonState;

private:

private:
  int vcrButton;
  int vcrButtonHL;
	PanelQuad *		vcrPQ;
	PanelQuad *		vcrHLPQ;
	PanelQuad *		vcrBGPQ;
	PanelQuad *		restartPQ;
	PanelQuad *		restartOffPQ;
	PanelQuad *		restartHLPQ;
	PanelQuad *		pausePQ;
	PanelQuad *		pauseOffPQ;
	PanelQuad *		pauseHLPQ;
	PanelQuad *		playPQ;
	PanelQuad *		playOffPQ;
	PanelQuad *		playHLPQ;
	PanelQuad *		slowPQ;
	PanelQuad *		slowOffPQ;
	PanelQuad *		slowHLPQ;
	PanelQuad *		fastforwardPQ;
	PanelQuad *		fastforwardOffPQ;
	PanelQuad *		fastforwardHLPQ;

  TextString *  pauseText;

  float         highlight_intensity;
  float         highlight_timer;
	
public:
	// Creators.
	ReplayWidget();
	virtual ~ReplayWidget();

	// Modifiers.
	virtual void SetDisplay(const bool d = true);
	virtual void Init(PanelFile & panel, Font * font);
	virtual void Update(const float dt);
	virtual void Draw(void);
	virtual void Select(int button);              // Activates the specified button
	virtual void SelectHighlight(int highlight);  // Highlights the specified button
	virtual void HighlightLeft();                 // Highlights the button to the left of the current button
	virtual void HighlightRight();                // Highlights the button to the right of the current button

	// Accessors.
  virtual int GetButton()  {return vcrButton;}  // Gets the currently active button
  virtual int GetHighlight()  {return vcrButtonHL;}  // Gets the currently highlighted button
};

#endif INCLUDED_IGO_WIDGET_REPLAY_H