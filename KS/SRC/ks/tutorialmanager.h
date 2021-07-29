
#ifndef INCLUDED_TUTORIALMANAGER_H
#define INCLUDED_TUTORIALMANAGER_H

#include "global.h"
#include "ngl.h"
#include "FEPanel.h"
#include "eventmanager.h"
#include "tutorialdata.h"

// IGOTutorialManager: manages display of onscreen help text and voice over
class IGOTutorialManager : public EventRecipient
{
private:
  enum
  {
    ICON_BUTTON_ARROW,
    ICON_BUTTON_CROSS,
    ICON_BUTTON_CIRCLE,
    ICON_BUTTON_TRIANGLE,
    ICON_BUTTON_SQUARE,
    ICON_BUTTON_PLUS,
    ICON_BUTTON_COMMA,
    ICON_NUM_BUTTONS
  };

	nglTexture * buttonTexture[ICON_NUM_BUTTONS];

	bool    finished;
	bool    first_step;
	int		current_step;
	float	current_step_time;
	int		last_step;			//  The last step for this part of the tutorial.

	bool	air_trick_in_chain;	//  These two variables let us know that a certain type of trick is in the current trick chain.
	bool	face_trick_in_chain;


	nslSoundId current_VO;

	int current_trick;
	int current_gap;
	bool needs_to_be_in_tube;
	bool is_perfect;
	bool show_advancement_text;
	bool show_hint_text;
	stringx ins_text;

public:
	// Creators.
	IGOTutorialManager();
	~IGOTutorialManager();

	// Modifiers.
	void Reset();
	void SetTutorialSection(int tutorial_level);
	void Update(const float dt);
	void Draw(void);
	void process_help_string(stringx &dest, const char *source);
	void SetCurrentTrick(int trick_num);
	void SetCurrentGap(int gap_num);
	void EndChain();
	nslSoundId play_sound(const char* name);
	virtual void OnEvent(const EVENT event, const int param1 = 0, const int param2 = 0);
	void SetAdvancementText(bool text_on) {show_advancement_text = text_on;}
	nslSoundId getCurrentSound() {return current_VO; }
	// Accessors.
	stringx help_text;
	stringx button_text;
	BoxText *instruction_text;
	bool Finished() { return finished; }
	bool AlmostFinished() { return current_step >= last_step - 1; }
	void PlayCurrentVO(bool intro = false);
	void StopCurrentVO();
	int WaveIndicatorType() {return GTutorialStep[current_step].wave_indicator;}
};

#endif INCLUDED_TUTORIALMANAGER_H