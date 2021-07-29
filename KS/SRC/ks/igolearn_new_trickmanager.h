
#ifndef INCLUDED_IGOLEARN_NEW_TRICKMANAGER_H
#define INCLUDED_IGOLEARN_NEW_TRICKMANAGER_H

#include "global.h"
#include "ngl.h"
#include "eventmanager.h"

// IGOLearnNewTrickManager: manages display of onscreen trick icons
class IGOLearnNewTrickManager : public EventRecipient
{
private:
	class IconResource
	{
	public:

		nglTexture *	texture;
		int				trickIdx;

	public:
		IconResource();
		~IconResource();

		void Load(const int idx, const stringx & texFilename);
	};

	class Icon
	{
	public:
		nglQuad			quad;
		IconResource*	resource;
		bool			show;

	public:
		Icon();
		~Icon();

		void Initialize(IconResource & res);
		void Draw(float opacity);
		void SetShow(const bool s = true);
	};

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
		ICON_BUTTON_QUESTION_MARK,
		ICON_NUM_BUTTONS
	};

	nglTexture		*buttonTexture[ICON_NUM_BUTTONS];

	int				numIconResources;
	IconResource	*iconResources;

	Icon			*current_icon;
	bool			icon_active;
	int				iconCounter;
	int				prev_iconCounter;
	int				current_trickIdx;

	float			addIconDelay;
	float			addIconTimer;
	float			subPointsTimer;
	int				specialCounter;

	float			trick_available_time;
	bool			already_got_one;
	bool			got_one_currently;

	//TextString		*counterText;
	//TextString		*timerText;

	nglTexture  *counterTexture;
	nglQuad     counterQuad;

private:
	void OnQueueChange(void);
	void SetButtonText(void);
	IconResource *FindResource(const int trickIdx);
	virtual void OnEvent(const EVENT event, const int param1 = 0, const int param2 = 0);

public:
	// Creators.
	IGOLearnNewTrickManager();
	~IGOLearnNewTrickManager();

	// Modifiers.
	void Reset(void);
	void Update(const float dt);
	void Draw();
	void PushBack(const int trickIdx);
	void PopFront(bool complete = false);
	void TrickChain(int trickIdx);
	void setFont(Font *font)  {;}/*counterText->setFont(font); timerText->setFont(font);}*/

	// Accessors.
	int IconsCleared() {return iconCounter;}
	bool CheckTrickPerformed();
};

#endif INCLUDED_IGOLEARN_NEW_TRICKMANAGER_H