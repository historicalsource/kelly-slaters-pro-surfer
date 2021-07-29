
// Photo Front End Screens

#ifndef INCLUDED_PHOTOFRONTEND_H
#define INCLUDED_PHOTOFRONTEND_H

#include "global.h"
#include "FEMenu.h"

// PhotoFrontEnd - photo selection menus that popup at the end of a photo challenge run.
// The first menu selects between the run's three photos, and the second menu chooses between the selected
// photo and the currently saved one.
// Also contains panel quads for the IGO camera reticle and polaroid.
class PhotoFrontEnd : public FEMultiMenu
{
public:
	static const float TIME_ARROW_HIGHLIGHT;

private:
	class PhotoSelectMenu *	selectMenu;
	class PhotoSaveMenu *	saveMenu;
	class PhotoDevelopMenu * develMenu;

public:
	// Creators.
	PhotoFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pf_name);
	virtual ~PhotoFrontEnd();

	// Modifiers.
	virtual void Load(void);
	virtual void Update(time_value_t time_inc);
	virtual void Draw(void);
	virtual void OnActivate(void);
	virtual void OnLeft(int c);
	virtual void OnRight(int c);
	virtual void OnUp(int c);
	virtual void OnDown(int c);
	virtual void OnTriangle(int c);
	void OnEndRun(void);

	// Accessors.
	int GetSelectedPhotoIdx(void) const;
};

// PhotoSelectMenu - player chooses one of the three photos taken during his run.
class PhotoSelectMenu : public FEMultiMenu
{
private:
	PanelQuad *		arrowPQs[2][2];
	PanelQuad *		highlightPQs[3];
	PhotoWidget 	photoWidgets[3];
	TextString *	title;

	int				leftArrowIdx, rightArrowIdx;
	float			arrowHiTimer;
	int				highlightedIdx;

public:
	// Creators.
	PhotoSelectMenu(FEMenuSystem * s, FEManager * man);
	virtual ~PhotoSelectMenu();

	// Modifiers.
	virtual void Init(void);
	virtual void Update(time_value_t time_inc);
	virtual void Draw(void);
	virtual void Select(int entry_index);
	virtual void OnActivate(void);
	virtual void OnTriangle(int c);
	virtual void OnCross(int c);
	virtual void OnLeft(int c);
	virtual void OnRight(int c);
	virtual void OnStart(int c);
	virtual void setHigh(FEMenuEntry * menu, bool anim = true);

	// Accessors.
	int * GetPhotoIdx(void) { return &highlightedIdx; }
};

// PhotoSaveMenu - player chooses between his run's selected photo and the currently saved one.
class PhotoSaveMenu : public FEMultiMenu
{
private:
	PanelQuad *		arrowPQs[2][2];
	PanelQuad *		highlightPQs[2];
	PhotoWidget 	photoWidgets[2];
	BoxText *		title;

	int				leftArrowIdx, rightArrowIdx;
	float			arrowHiTimer;
	int * 			prevMenuPhotoIdx;	// selected index from the previous menu

public:
	// Creators.
	PhotoSaveMenu(FEMenuSystem * s, FEManager * man);
	virtual ~PhotoSaveMenu();

	// Modifiers.
	virtual void Init(void);
	virtual void Update(time_value_t time_inc);
	virtual void Draw(void);
	virtual void Select(int entry_index);
	virtual void OnActivate(void);
	virtual void OnTriangle(int c);
	virtual void OnCross(int c);
	virtual void OnLeft(int c);
	virtual void OnRight(int c);
	virtual void OnStart(int c);
	void SetPrevMenuPhotoIdx(int * idx) { prevMenuPhotoIdx = idx; }
};

// PhotoSaveMenu - player chooses between his run's selected photo and the currently saved one.
class PhotoDevelopMenu : public FEMultiMenu
{
private:
	TextString *	title;
	PhotoWidget 	photo;
	int *whichPhotoPtr;
	bool doneCompressing;
	int drawCounter;
public:
	// Creators.
	PhotoDevelopMenu(FEMenuSystem * s, FEManager * man);
	virtual ~PhotoDevelopMenu();
	void SetPrevMenuPhotoIdx(int * idx) { whichPhotoPtr = idx; }

	// Modifiers.
	virtual void Init(void);
	virtual void Update(time_value_t time_inc);
	virtual void Draw(void);
	virtual void OnActivate(void);
	virtual void OnTriangle(int c);
	virtual void OnCross(int c);
	virtual void OnStart(int c);
};

#endif INCLUDED_PHOTOFRONTEND_H