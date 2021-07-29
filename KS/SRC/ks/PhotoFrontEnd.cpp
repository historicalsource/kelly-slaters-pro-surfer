
// PhotoFrontEnd.cpp

#include "global.h"
#include "PhotoFrontEnd.h"

const float PhotoFrontEnd::TIME_ARROW_HIGHLIGHT = 0.05f;

//	PhotoFrontEnd()
// Constructor with initializers.
PhotoFrontEnd::PhotoFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pf_name)
{
	cons(s, man, p, pf_name);

	// Create submenus.
	selectMenu = NEW PhotoSelectMenu(s, man);
	saveMenu = NEW PhotoSaveMenu(s, man);
	develMenu = NEW PhotoDevelopMenu(s, man);
	// Add submenus.
	AddSubmenu(selectMenu);
	AddSubmenu(saveMenu);
	AddSubmenu(develMenu);

	selectMenu->next_sub = saveMenu;
	saveMenu->next_sub = develMenu;
	saveMenu->back = selectMenu;
	saveMenu->SetPrevMenuPhotoIdx(selectMenu->GetPhotoIdx());
	develMenu->SetPrevMenuPhotoIdx(selectMenu->GetPhotoIdx());
}

//	~PhotoFrontEnd()
// Destructor.
PhotoFrontEnd::~PhotoFrontEnd()
{
	// Unlike menu entries, submenus have to be manually deleted.
	delete selectMenu;
	delete saveMenu;
	delete develMenu;
}

//	Load()
// Loads the front end's panel quads.
void PhotoFrontEnd::Load(void)
{
	FEMultiMenu::Load();
	develMenu->Init();
	selectMenu->Init();
	saveMenu->Init();
}

//	Update()
// Must be called every frame.
void PhotoFrontEnd::Update(time_value_t time_inc)
{
	FEMultiMenu::Update(time_inc);
	
	// Update current menu.
	if (active)
		active->Update(time_inc);
}

//	Draw()
// Renders the selection menu.
void PhotoFrontEnd::Draw()
{
	// Draw current menu.
	if (active)
		active->Draw();
}

//	OnActivate()
// Responds when the selection front end pops up.
void PhotoFrontEnd::OnActivate(void)
{	
	FEMultiMenu::OnActivate();
	
	// Jump right to the save menu if we only took one photo.
	if (g_beach_ptr->get_challenges()->photo->GetNumTaken() == 1 &&
		g_career->PhotoExistsForLevel (g_game_ptr->get_level_id()))
	{
		saveMenu->next_sub = develMenu;
		MakeActive(saveMenu);
	// Otherwise, let the user choose from the photos in his run.
	}
	else if (g_beach_ptr->get_challenges()->photo->GetNumTaken() > 1)
	{
		if (g_career->PhotoExistsForLevel (g_game_ptr->get_level_id()))
			selectMenu->next_sub = saveMenu;
		else
			selectMenu->next_sub = develMenu;

		MakeActive(selectMenu);
	}
	else
	{
		MakeActive(develMenu);
	}
}

//	OnTriangle()
// Responds when the triangle button is pressed.
void PhotoFrontEnd::OnTriangle(int c)
{
	// Allow user to back up to selection menu.
	//if (active == saveMenu) MakeActive(selectMenu);
}

//	OnLeft()
// Responds when the left button is pressed.
void PhotoFrontEnd::OnLeft(int c)
{
	if (active)
		active->OnLeft(c);
}

//	OnRight()
// Responds when the right button is pressed.
void PhotoFrontEnd::OnRight(int c)
{
	if (active)
		active->OnRight(c);
}

//	OnUp()
// Responds when the up button is pressed.
void PhotoFrontEnd::OnUp(int c)
{
	if (active)
		active->OnUp(c);
}

//	OnDown()
// Responds when the down button is pressed.
void PhotoFrontEnd::OnDown(int c)
{
	if (active)
		active->OnDown(c);
}

//	OnEndRun()
// Must be called when the run ends in career mode.
void PhotoFrontEnd::OnEndRun(void)
{
	if (selectMenu) selectMenu->setHigh(selectMenu->entries);
	if (saveMenu) saveMenu->setHigh(saveMenu->entries);
}

//	GetSelectedPhotoIdx()
// Returns which photo the user selected to save.
// -1 means don't save.
int PhotoFrontEnd::GetSelectedPhotoIdx(void) const
{
	assert(saveMenu);
	
	if (g_game_ptr->get_game_mode() == GAME_MODE_CAREER && g_beach_ptr->get_challenges()->photo &&
		saveMenu->highlighted && saveMenu->highlighted->entry_num == 0)
	{
		if (selectMenu->highlighted->entry_num < g_beach_ptr->get_challenges()->photo->GetNumTaken())
			return selectMenu->highlighted->entry_num;
		else
			return -1;
	}
	else
		return -1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	PhotoSelectMenu class definition
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//	PhotoSelectMenu()
// Constructor with initializers.
PhotoSelectMenu::PhotoSelectMenu(FEMenuSystem* s, FEManager* man)
{
	Font *			font;
	FEMenuEntry *	photo1, *photo2, *photo3;
	
	cons(s, man, "", "");
	//flags |= FEMENU_1_ENTRY_SHOWN | FEMENU_DONT_SKIP_DISABLED;
	font = &manager->font_info;

	arrowPQs[0][0] = NULL;
	arrowPQs[0][1] = NULL;
	arrowPQs[1][0] = NULL;
	arrowPQs[1][1] = NULL;
	highlightPQs[0] = NULL;
	highlightPQs[1] = NULL;
	highlightPQs[2] = NULL;
	title = NULL;

	// Create first photo entry.
	photo1 = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_PHOTO_PICK], this, false, font);
	photo1->SetPos(170, 320);
	Add(photo1);

	// Create second photo entry.
	photo2 = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_PHOTO_PICK], this, false, font);
	photo2->SetPos(320, 320);
	Add(photo2);

	// Create third photo entry.
	photo3 = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_PHOTO_PICK], this, false, font);
	photo3->SetPos(470, 320);
	Add(photo3);

	photo1->left = photo3;
	photo1->right = photo2;
	photo1->up = photo1;
	photo1->down = photo1;
	photo2->left = photo1;
	photo2->right = photo3;
	photo2->up = photo2;
	photo2->down = photo2;
	photo3->left = photo2;
	photo3->right = photo1;
	photo3->up = photo3;
	photo3->down = photo3;

	// Create title text.
	title = NEW TextString(&manager->font_bold, ksGlobalTextArray[GT_FE_MENU_PHOTO_TITLE_SELECT], 320, 75, 100, 1.0f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, man->col_highlight2);

	SetHelpText(HF_LEFT | HF_RIGHT | HF_SELECT);
	helpText->changePos(320, 424);

	arrowHiTimer = 0.0f;
	leftArrowIdx = 0;
	rightArrowIdx = 0;
	highlightedIdx = 0;
}

//	~PhotoSelectMenu()
// Destructor.
PhotoSelectMenu::~PhotoSelectMenu()
{
	delete title;
}

//	Init()
// One-time initialization of the menu.
void PhotoSelectMenu::Init(void)
{
	arrowPQs[0][0] = GetPointer("pp_btn_3_left_off");
	arrowPQs[0][0]->TurnOn(true);
	arrowPQs[0][1] = GetPointer("pp_btn_3_left_on");
	arrowPQs[0][1]->TurnOn(true);
	arrowPQs[1][0] = GetPointer("pp_btn_3_right_off");
	arrowPQs[1][0]->TurnOn(true);
	arrowPQs[1][1] = GetPointer("pp_btn_3_right_on");
	arrowPQs[1][1]->TurnOn(true);

	photoWidgets[0].Init(GetPointer("pp_polaroid_01"), &manager->font_info);
	photoWidgets[1].Init(GetPointer("pp_polaroid_02"), &manager->font_info);
	photoWidgets[2].Init(GetPointer("pp_polaroid_03"), &manager->font_info);

	highlightPQs[0] = GetPointer("pp_hilite_01");
	highlightPQs[0]->TurnOn(true);
	highlightPQs[1] = GetPointer("pp_hilite_02");
	highlightPQs[1]->TurnOn(true);
	highlightPQs[2] = GetPointer("pp_hilite_03");
	highlightPQs[2]->TurnOn(true);

	arrowHiTimer = 0.0f;
	leftArrowIdx = 0;
	rightArrowIdx = 0;

	setHigh(entries);
}

//	Update()
// Called every frame.
void PhotoSelectMenu::Update(time_value_t time_inc)
{
	FEMultiMenu::Update(time_inc);

	photoWidgets[0].Update(time_inc);
	photoWidgets[1].Update(time_inc);
	photoWidgets[2].Update(time_inc);
	
	// Update arrow highlight timer.
	if (arrowHiTimer > 0.0f)
	{
		arrowHiTimer -= time_inc;
		if (arrowHiTimer <= 0.0f)
		{
			arrowHiTimer = 0.0f;
			leftArrowIdx = 0;
			rightArrowIdx = 0;
		}
	}
}

//	Draw()
// Renders the menu.
void PhotoSelectMenu::Draw()
{
	// Draw entries.
	highlighted->Draw();

	// Draw the rest.
	//arrowPQs[0][leftArrowIdx]->Draw(0);
	//arrowPQs[1][rightArrowIdx]->Draw(0);
	photoWidgets[0].Draw();
	photoWidgets[1].Draw();
	photoWidgets[2].Draw();
	highlightPQs[highlighted->entry_num]->Draw(0);
	title->Draw();
	helpText->Draw();
}

//	Select()
// Called when the user chooses the specified entry.
void PhotoSelectMenu::Select(int entry_index)
{
	// If there is already a photo saved for this beach, then popup a second selection menu.
	if (g_career->PhotoExistsForLevel (g_game_ptr->get_level_id()) &&
		highlighted->entry_num < g_beach_ptr->get_challenges()->photo->GetNumTaken())
	{
		highlightedIdx = highlighted->entry_num;
	}
	SoundScriptManager::inst()->unpause();
	SoundScriptManager::inst()->playEvent(SS_FE_ONX);
	SoundScriptManager::inst()->pause();
	parent->MakeActive(next_sub);

}

//	OnActivate()
// Called when the menu pops up.
void PhotoSelectMenu::OnActivate(void)
{
	PhotoChallenge *	photoChallenge = g_beach_ptr->get_challenges()->photo;
	float				fade;
	int					hiIdx = 0;
	int					i;
	
	FEMultiMenu::OnActivate();

	// Display photo 1.
	if (photoChallenge->GetNumTaken() > 0)
	{
		fade = 0.0f;
		entries->Disable(false);
	}
	else
	{
		fade = 1.0f;
		entries->Disable(true);
	}
	photoWidgets[0].Show(photoChallenge->GetPhotoTexture(0), photoChallenge->GetPhotoScore(0), 0, fade);

	// Display photo 2.
	if (photoChallenge->GetNumTaken() > 1)
	{
		fade = 0.0f;
		entries->next->Disable(false);
	}
	else
	{
		fade = 1.0f;
		entries->next->Disable(true);
	}
	photoWidgets[1].Show(photoChallenge->GetPhotoTexture(1), photoChallenge->GetPhotoScore(1), 1, fade);

	// Display photo 3.
	if (photoChallenge->GetNumTaken() > 2)
	{
		fade = 0.0f;
		entries->next->next->Disable(false);
	}
	else
	{
		fade = 1.0f;
		entries->next->next->Disable(true);
	}
	photoWidgets[2].Show(photoChallenge->GetPhotoTexture(2), fade == 1.0f ? NULL : photoChallenge->GetPhotoScore(2), 2, fade);

	// Highlight highest scored photo by default.
	for (i = 1; i < 3; i++)
	{
		if (*photoChallenge->GetPhotoScore(i) > *photoChallenge->GetPhotoScore(hiIdx))
			hiIdx = i;
	}
	setHigh(entries);
	for (i = 0; i < hiIdx; i++)
		FEMultiMenu::Right();
}

//	OnTriangle()
// Responds when the triangle button is pressed.
void PhotoSelectMenu::OnTriangle(int c)
{

}

//	OnCross()
// Responds when the X button is pressed.
void PhotoSelectMenu::OnCross(int c)
{
	Select(highlighted->entry_num);
}

//	OnLeft()
// Responds when the left button is pressed.
void PhotoSelectMenu::OnLeft(int c)
{
	FEMultiMenu::OnLeft(c);
	SoundScriptManager::inst()->unpause();
	SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
	SoundScriptManager::inst()->pause();
	// Flash left arrow highlight.
	leftArrowIdx = 1;
	arrowHiTimer = PhotoFrontEnd::TIME_ARROW_HIGHLIGHT;
}

//	OnRight()
// Responds when the right button is pressed.
void PhotoSelectMenu::OnRight(int c)
{
	FEMultiMenu::OnRight(c);
	SoundScriptManager::inst()->unpause();
	SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
	SoundScriptManager::inst()->pause();
	// Flash right arrow highlight.
	rightArrowIdx = 1;
	arrowHiTimer = PhotoFrontEnd::TIME_ARROW_HIGHLIGHT;
}

// Added this so that Start will button-through end run screens.
void PhotoSelectMenu::OnStart(int c)
{
	Select(highlighted->entry_num);
}

//	setHigh()
// Changes the highlighted entry.
void PhotoSelectMenu::setHigh(FEMenuEntry * menu, bool anim)
{
	FEMultiMenu::setHigh(menu, anim);

	if (highlighted)
		highlightedIdx = highlighted->entry_num;
	else
		highlightedIdx = -1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	PhotoSaveMenu class definition
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//	PhotoSaveMenu()
// Constructor with initializers.
PhotoSaveMenu::PhotoSaveMenu(FEMenuSystem* s, FEManager* man)
{
	Font *			font;
	FEMenuEntry *	photo1, *photo2;
	stringx			str;
	
	cons(s, man, "", "");
	flags |= FEMENU_1_ENTRY_SHOWN | FEMENU_DONT_SKIP_DISABLED;
	font = &manager->font_info;

	arrowPQs[0][0] = NULL;
	arrowPQs[0][1] = NULL;
	arrowPQs[1][0] = NULL;
	arrowPQs[1][1] = NULL;
	highlightPQs[0] = NULL;
	highlightPQs[1] = NULL;
	title = NULL;

	// Create first photo entry.
	photo1 = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_PHOTO_SAVE_NEW], this, false, font);
	photo1->SetPos(245, 320);
	Add(photo1);

	// Create second photo entry.
	photo2 = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_PHOTO_KEEP_OLD], this, false, font);
	photo2->SetPos(395, 320);
	Add(photo2);

	photo1->left = photo2;
	photo1->right = photo2;
	photo1->up = photo1;
	photo1->down = photo1;
	photo2->left = photo1;
	photo2->right = photo1;
	photo2->up = photo2;
	photo2->down = photo2;

	// Create title text.
	str.printf(ksGlobalTextArray[GT_FE_MENU_PHOTO_TITLE_SAVE].c_str(), BeachDataArray[g_game_ptr->get_beach_id()].fe_name);
	title = NEW BoxText(&manager->font_bold, str, 320, 75, 100, 0.8f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, man->col_highlight2);
	title->makeBox(500, 200);

	// Initialize help text at bottom of screen.
	SetHelpText(HF_LEFT | HF_RIGHT | HF_SELECT);
	helpText->changePos(320, 424);

	arrowHiTimer = 0.0f;
	leftArrowIdx = 0;
	rightArrowIdx = 0;
	prevMenuPhotoIdx = NULL;
}

//	~PhotoSaveMenu()
// Destructor.
PhotoSaveMenu::~PhotoSaveMenu()
{
	delete title;
}

//	Init()
// One-time initialization of the menu.
void PhotoSaveMenu::Init(void)
{
	arrowPQs[0][0] = GetPointer("pp_btn_2_left_off");
	arrowPQs[0][0]->TurnOn(true);
	arrowPQs[0][1] = GetPointer("pp_btn_2_left_on");
	arrowPQs[0][1]->TurnOn(true);
	arrowPQs[1][0] = GetPointer("pp_btn_2_right_off");
	arrowPQs[1][0]->TurnOn(true);
	arrowPQs[1][1] = GetPointer("pp_btn_2_right_on");
	arrowPQs[1][1]->TurnOn(true);

	photoWidgets[0].Init(GetPointer("pp_polaroid_04"), &manager->font_info);
	photoWidgets[1].Init(GetPointer("pp_polaroid_05"), &manager->font_info);

	highlightPQs[0] = GetPointer("pp_hilite_04");
	highlightPQs[0]->TurnOn(true);
	highlightPQs[1] = GetPointer("pp_hilite_05");
	highlightPQs[1]->TurnOn(true);

	arrowHiTimer = 0.0f;
	leftArrowIdx = 0;
	rightArrowIdx = 0;

	setHigh(entries);
}

//	Update()
// Called every frame.
void PhotoSaveMenu::Update(time_value_t time_inc)
{
	FEMultiMenu::Update(time_inc);

	photoWidgets[0].Update(time_inc);
	photoWidgets[1].Update(time_inc);
	
	// Update arrow highlight timer.
	if (arrowHiTimer > 0.0f)
	{
		arrowHiTimer -= time_inc;
		if (arrowHiTimer <= 0.0f)
		{
			arrowHiTimer = 0.0f;
			leftArrowIdx = 0;
			rightArrowIdx = 0;
		}
	}
}

//	Draw()
// Renders the menu.
void PhotoSaveMenu::Draw()
{
	// Draw entries.
	highlighted->Draw();

	// Draw the rest.
	//arrowPQs[0][leftArrowIdx]->Draw(0);
	//arrowPQs[1][rightArrowIdx]->Draw(0);
	photoWidgets[0].Draw();
	photoWidgets[1].Draw();
	highlightPQs[highlighted->entry_num]->Draw(0);
	title->Draw();
	helpText->Draw();
}

//	Select()
// Called when the user chooses the specified entry.
void PhotoSaveMenu::Select(int entry_index)
{
	if (entry_index == 0)
		parent->MakeActive(next_sub);
	else
		system->endDraw(false);
}

//	OnActivate()
// Called when the menu pops up.
void PhotoSaveMenu::OnActivate(void)
{
	PhotoChallenge *	photoChallenge = g_beach_ptr->get_challenges()->photo;
	float				fade;
	int					otherIdx = 0;
	bool				loaded = false;
	//int					loadedScore = 1234;
	
	FEMultiMenu::OnActivate();

	assert(prevMenuPhotoIdx && *prevMenuPhotoIdx >= 0 && *prevMenuPhotoIdx < 3);

	// Display photo 1.
	if (photoChallenge->GetNumTaken() > *prevMenuPhotoIdx) fade = 0.0f;
	else fade = 1.0f;
	photoWidgets[0].Show(photoChallenge->GetPhotoTexture(*prevMenuPhotoIdx), NULL, *prevMenuPhotoIdx, fade);

	// Pick a temporary photo index to overwrite with the one loaded from the memory card.
	while (otherIdx == *prevMenuPhotoIdx) otherIdx++;

	// Load photo from memory card.
	if (g_career->PhotoExistsForLevel (g_game_ptr->get_level_id()))
	{
		CompressedPhoto	*photo = g_career->GetPhotoForLevel (g_game_ptr->get_level_id());
		photo->CopyToTexture(photoChallenge->GetPhotoTexture(otherIdx));
		loaded = true;
	}

	// Display photo 2.
	if (loaded) fade = 0.0f;
	else fade = 1.0f;
	photoWidgets[1].Show(photoChallenge->GetPhotoTexture(otherIdx), NULL, otherIdx, fade);

	setHigh(entries->next);
}

//	OnTriangle()
// Responds when the triangle button is pressed.
void PhotoSaveMenu::OnTriangle(int c)
{
	//FEMultiMenu::OnTriangle(c);
}

//	OnCross()
// Responds when the X button is pressed.
void PhotoSaveMenu::OnCross(int c)
{
	Select(highlighted->entry_num);
	SoundScriptManager::inst()->unpause();
	SoundScriptManager::inst()->playEvent(SS_FE_ONX);
	SoundScriptManager::inst()->pause();
}

//	OnLeft()
// Responds when the left button is pressed.
void PhotoSaveMenu::OnLeft(int c)
{
	FEMultiMenu::OnLeft(c);
	SoundScriptManager::inst()->unpause();
	SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
	SoundScriptManager::inst()->pause();
	// Flash left arrow highlight.
	leftArrowIdx = 1;
	arrowHiTimer = PhotoFrontEnd::TIME_ARROW_HIGHLIGHT;
}

//	OnRight()
// Responds when the right button is pressed.
void PhotoSaveMenu::OnRight(int c)
{
	FEMultiMenu::OnRight(c);
	SoundScriptManager::inst()->unpause();
	SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
	SoundScriptManager::inst()->pause();
	// Flash right arrow highlight.
	rightArrowIdx = 1;
	arrowHiTimer = PhotoFrontEnd::TIME_ARROW_HIGHLIGHT;
}

// Added this so that Start will button-through end run screens on Xbox.
void PhotoSaveMenu::OnStart(int c)
{
	Select(highlighted->entry_num);
	SoundScriptManager::inst()->unpause();
	SoundScriptManager::inst()->playEvent(SS_FE_ONX);
	SoundScriptManager::inst()->pause();
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	PhotoDevelopMenu class definition
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//	PhotoDevelopMenu()
// Constructor with initializers.
PhotoDevelopMenu::PhotoDevelopMenu(FEMenuSystem* s, FEManager* man)
{
	Font *			font;
	stringx			str;
	
	cons(s, man, "", "");
	flags |= FEMENU_1_ENTRY_SHOWN | FEMENU_DONT_SKIP_DISABLED;
	font = &man->font_info;

	title = NULL;

	// Create title text.
	title = NEW TextString(&manager->font_bold, ksGlobalTextArray[GT_PHOTO_COMPRESS], 320, 75, 100, 1.0f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, man->col_highlight2);
	// Initialize help text at bottom of screen.
	//SetHelpText(HF_LEFT | HF_RIGHT | HF_SELECT);
	SetHelpText(0);
	helpText->changePos(320, 424);
	doneCompressing = false;
	drawCounter = 0;
}

//	~PhotoSaveMenu()
// Destructor.
PhotoDevelopMenu::~PhotoDevelopMenu()
{
	delete title;
}

//	Init()
// One-time initialization of the menu.
void PhotoDevelopMenu::Init(void)
{

	photo.Init(GetPointer("pp_polaroid_02"), &manager->font_info);
	
}

void PhotoDevelopMenu::Draw()
{
	drawCounter++;
	photo.Draw();
	title->Draw();
	helpText->Draw();
}

void PhotoDevelopMenu::Update(time_value_t time_inc)
{
	if (!doneCompressing && drawCounter >= 4)
	{
		g_career->GetPhotoForLevel (g_game_ptr->get_level_id())->CopyFromTexture(g_beach_ptr->get_challenges()->photo->GetPhotoTexture(*whichPhotoPtr));
		doneCompressing = true;
		SetHelpText( HF_CONTINUE );
		system->endDraw(false);
	}
}

void PhotoDevelopMenu::OnActivate()
{
	photo.Show(g_beach_ptr->get_challenges()->photo->GetPhotoTexture(*whichPhotoPtr), NULL, *whichPhotoPtr, 0.0f);
	doneCompressing = false;
	drawCounter = 0;
	SetHelpText(0);
}


void PhotoDevelopMenu::OnTriangle(int c)
{
}
void PhotoDevelopMenu::OnCross(int c)
{
 // Advance if done processing
}
void PhotoDevelopMenu::OnStart(int c)
{
// Advance if done processing
}
