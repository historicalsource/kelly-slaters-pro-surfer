
// TutorialFrontEnd.cpp

#include "global.h"
#include "TutorialFrontEnd.h"

const float TutorialFrontEnd::TIME_ARROW_HIGHLIGHT = 0.05f;

//	TutorialFrontEnd()
// Constructor with initializers.
TutorialFrontEnd::TutorialFrontEnd(FEMenuSystem* s, FEManager* man)
{
	cons(s, man, "", "");

	for (int i = 0; i < 9; i++)
		bgPQs[i] = NULL;

	help_text = NEW BoxText(&man->font_info, "", 135, 140, 800, 1.0f, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, color32(255, 255, 125, 255), 15);
	help_text->makeBox(390,400);

	// Needed to allow menu creation even if not in tutorial.  (dc 06/13/02)
	stringx raw_text = frontendmanager.IGO->GetTutorialManager() 
		? frontendmanager.IGO->GetTutorialManager()->button_text : "";
	pause_button_text = NEW TextString(&system->manager->font_bold, raw_text, 320, 365, 20, 0.6f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER);
	pause_button_text->setButtonScale(pause_button_text->GetScale()*1.4f);

}

//	~TutorialFrontEnd()
// Destructor.
TutorialFrontEnd::~TutorialFrontEnd()
{
	delete help_text;

	delete pause_button_text;
}

//	Load()
// Loads the front end's panel quads.
void TutorialFrontEnd::Load(void)
{
}

//	Update()
// Must be called every frame.
void TutorialFrontEnd::Update(time_value_t time_inc)
{
	FEMultiMenu::Update(time_inc);

	frontendmanager.IGO->GetTutorialManager()->Update(0.0f);
	help_text->changeText(frontendmanager.IGO->GetTutorialManager()->help_text);
	help_text->makeBox(390,400);
	
	if (frontendmanager.IGO->GetTutorialManager()->WaveIndicatorType())
		waveIndicator->Update(time_inc);

	// Update current menu.
	if (active)
		active->Update(time_inc);
}


//	Draw()
// Renders the selection menu.
void TutorialFrontEnd::Draw()
{
	// Draw current menu.
	if (active)
		active->Draw();

	// Draw help message
	help_text->Draw();
	pause_button_text->Draw();
}

//	OnActivate()
// Responds when the selection front end pops up.
void TutorialFrontEnd::OnActivate(void)
{	
	ignore_next_release = true;

	frontendmanager.IGO->SetDisplay(true);
	if (!frontendmanager.IGO->GetTutorialManager()->AlmostFinished())  //  it is the tutorial, but it is done.
		frontendmanager.IGO->GetTutorialManager()->PlayCurrentVO();
	frontendmanager.IGO->GetTutorialManager()->SetAdvancementText(false);

	waveIndicator = frontendmanager.IGO->GetWaveIndicatorWidget();

	int wave_ind_type = frontendmanager.IGO->GetTutorialManager()->WaveIndicatorType();
	if (wave_ind_type)
		waveIndicator->ShowHighlight(wave_ind_type - 1);

	FEMultiMenu::OnActivate();
}

//	OnUnactivate()
// Responds when the menu is dismissed.
void TutorialFrontEnd::OnUnactivate(FEMenu* m)
{	
	frontendmanager.IGO->GetTutorialManager()->StopCurrentVO();
	waveIndicator->Hide();
}


//  OnButtonRelease
void TutorialFrontEnd::OnButtonRelease(int c, int b)
{
	// Continue does nothing until button released
	if(b == FEMENUCMD_CROSS)
	{
		if (ignore_next_release)
			ignore_next_release = false;	//  The first release of the cross button is just left over from entering the menu.
		else
		{
			frontendmanager.IGO->GetTutorialManager()->StopCurrentVO();
			waveIndicator->Hide();
			frontendmanager.pms->endDraw();
			frontendmanager.pms->UpdateButtonDown();
		}
	}
}


//	OnTriangle()
// Responds when the triangle button is pressed.
void TutorialFrontEnd::OnTriangle(int c)
{
	SoundScriptManager::inst()->unpause();
	SoundScriptManager::inst()->playEvent(SS_FE_BACK);
	SoundScriptManager::inst()->pause();

	frontendmanager.IGO->SetDisplay(false);
	frontendmanager.pms->MakeActive(PauseMenuSystem::TutorialPauseMenu);
}

//	OnLeft()
// Responds when the left button is pressed.
void TutorialFrontEnd::OnLeft(int c)
{
	if (active)
		active->OnLeft(c);
}

//	OnRight()
// Responds when the right button is pressed.
void TutorialFrontEnd::OnRight(int c)
{
	if (active)
		active->OnRight(c);
}

//	OnUp()
// Responds when the up button is pressed.
void TutorialFrontEnd::OnUp(int c)
{
	if (active)
		active->OnUp(c);
}

//	OnDown()
// Responds when the down button is pressed.
void TutorialFrontEnd::OnDown(int c)
{
	if (active)
		active->OnDown(c);
}

