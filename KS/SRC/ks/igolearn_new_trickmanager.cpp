
#include "global.h"
#include "igolearn_new_trickmanager.h"
#include "random.h"
#include "wds.h"
#include "globaltextenum.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	IGOLearnNewTrickManager class
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float counterX          = 175;
float counterY          = 40;
float counterWidth      = 100;
float counterHeight     = 100;

  //	IGOLearnNewTrickManager()
// Default constructor.
IGOLearnNewTrickManager::IGOLearnNewTrickManager()
{
	// Allocate icon resources.
	numIconResources = 13;
	iconResources = NEW IconResource[numIconResources];

	// Load icon resources.
	nglSetTexturePath("interface\\IGO\\textures\\");

	// Special tricks
	iconResources[0].Load(TRICK_WALK, "igo_icon_airwalk");
	iconResources[1].Load(TRICK_ALLEY_OOP, "igo_icon_alleyoop");
	iconResources[2].Load(TRICK_HELICOPTER_720, "igo_icon_heli");
	iconResources[3].Load(TRICK_SUPERMAN, "igo_icon_superman");
	iconResources[4].Load(TRICK_RODEO, "igo_icon_rodeo");
	iconResources[5].Load(TRICK_DARKSLIDE, "igo_icon_darkslide");
	iconResources[6].Load(TRICK_TWEAKER, "igo_icon_tweaker");
	iconResources[7].Load(TRICK_BACK_FLIP, "igo_icon_backflip");
	iconResources[8].Load(TRICK_JC_AIR, "igo_icon_changeman");
	iconResources[9].Load(TRICK_FACE_SHOVEIT, "igo_icon_180_shoveit");
	iconResources[10].Load(TRICK_HANGTEN, "igo_icon_hangten");
	iconResources[11].Load(TRICK_CHEATERS5, "igo_icon_cheaters5");
	iconResources[12].Load(TRICK_HEADSTAND, "igo_icon_180_headstand");


	// Load button textures
	buttonTexture[ICON_BUTTON_ARROW]     = nglLoadTexture("igo_icon_arrow");
	buttonTexture[ICON_BUTTON_CROSS]     = nglLoadTexture("igo_icon_x");
	buttonTexture[ICON_BUTTON_CIRCLE]    = nglLoadTexture("igo_icon_circle");
	buttonTexture[ICON_BUTTON_TRIANGLE]  = nglLoadTexture("igo_icon_triangle");
	buttonTexture[ICON_BUTTON_SQUARE]    = nglLoadTexture("igo_icon_square");
	buttonTexture[ICON_BUTTON_PLUS]      = nglLoadTexture("igo_icon_plus");
	buttonTexture[ICON_BUTTON_COMMA]     = nglLoadTexture("igo_icon_comma");
	buttonTexture[ICON_BUTTON_QUESTION_MARK]     = nglLoadTexture("igo_icon_question");

	/*counterTexture = nglLoadTexture("icon_score");

	//  Set up the little box that goes behind the counter.
	nglInitQuad(&counterQuad);
	nglSetQuadMapFlags(&counterQuad, NGLMAP_CLAMP_U | NGLMAP_CLAMP_V | NGLMAP_BILINEAR_FILTER);
	nglSetQuadTex(&counterQuad, counterTexture);
	nglSetQuadColor(&counterQuad, NGL_RGBA32(255, 255, 255, 255));
	nglSetQuadZ(&counterQuad, 800.0f);
	nglSetQuadRect(&counterQuad, counterX-(counterWidth * 0.5f), counterY-(counterHeight * 0.5f), counterX+counterWidth/2, counterY+counterHeight/2);*/

	// Allocate icons.
	icon_active = false;
	current_icon = NEW Icon;

	// Initialize icons.
	current_icon->SetShow(false);

	//counterText = NEW TextString(NULL, "", counterX + 42, counterY + 3, 800, 1.0f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, color32(255, 255, 255, 255));
	//timerText   = NEW TextString(NULL, "", 70, 160, 800, 1.0f, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, color32(255, 255, 255, 255));

	addIconDelay = 6.0f;
	addIconTimer = 4.0f;
	iconCounter = 0;
	prev_iconCounter = 0;
	trick_available_time = 0.0f;
	got_one_currently = false;
	already_got_one = false;
}

//	~IGOLearnNewTrickManager()
// Destructor.
IGOLearnNewTrickManager::~IGOLearnNewTrickManager()
{
	delete [] iconResources;
	delete current_icon;
	//delete counterText;
	//delete timerText;
}

//	Reset()
// Clears the icons off the screen and resets timers.  Should be called when a new run begins.
void IGOLearnNewTrickManager::Reset(void)
{
	addIconDelay = 6.0f;
	addIconTimer = 4.0f;

	if (icon_active)
		PopFront();

	iconCounter = 0;
	prev_iconCounter = 0;
}

float max_trick_available_time = 5.0f;
float reset_trick_available_time = 10.0f;
float fade_in_time = 0.15f;  //  How long it takes the icon to fade in.

//	Draw()
// Draws all the icons onscreen.
void IGOLearnNewTrickManager::Draw()
{
	// Draw Counter
/*	nglListAddQuad(&counterQuad);
	stringx	text(iconCounter);
	text.append(" ");
	text.append(ksGlobalTextArray[GT_LEARN_TRICK_OF_3].c_str());
	counterText->changeText(text);
	counterText->Draw();*/

	float opacity;

	if (trick_available_time < fade_in_time)
		opacity= min(1.0f, trick_available_time/fade_in_time);  //  Make the icon fade in.
	else if (max_trick_available_time - trick_available_time < fade_in_time && 
		trick_available_time < max_trick_available_time)
		opacity= min(1.0f, max_trick_available_time - trick_available_time/fade_in_time);  //  Make the icon fade in.
	else
		opacity = 1.0f;

	if (!icon_active)
		return;

	// Draw Icons
	current_icon->Draw(opacity);


	//  Draw countdown timer.
	char text2[16];
	sprintf(text2, "%0.1f", max_trick_available_time - trick_available_time);
	//timerText->changeText(text2);
	//timerText->Draw();

	// Draw buttons
	int b[3];
	b[0] = GTrickList[current_icon->resource->trickIdx].button1;
	b[1] = GTrickList[current_icon->resource->trickIdx].button2;
	b[2] = GTrickList[current_icon->resource->trickIdx].button3;

	nglQuad quad;
	nglInitQuad(&quad);

	for(int i=0; i<3; i++)
	{
		if(b[i] == PAD_NONE)
			continue;

		int j = 26 + i*40;
		if(b[2] == PAD_NONE)
			j += 20;

		switch(b[i])
		{
		case PAD_U:
		case PAD_D:
		case PAD_L:
		case PAD_R:
		case PAD_UL:
		case PAD_UR:
		case PAD_DL:
		case PAD_DR:
			nglSetQuadTex(&quad, buttonTexture[ICON_BUTTON_ARROW]);
			break;

		case PAD_CIRCLE:    nglSetQuadTex(&quad, buttonTexture[ICON_BUTTON_CIRCLE]);    break;
		case PAD_CROSS:     nglSetQuadTex(&quad, buttonTexture[ICON_BUTTON_CROSS]);     break;
		case PAD_TRIANGLE:  nglSetQuadTex(&quad, buttonTexture[ICON_BUTTON_TRIANGLE]);  break;
		case PAD_SQUARE:    nglSetQuadTex(&quad, buttonTexture[ICON_BUTTON_SQUARE]);    break;
		}

		//  Change the middle button to a question mark if the surfer hasn't learned this trick yet.
		if (i == 1 && iconCounter < 3)
			nglSetQuadTex(&quad, buttonTexture[ICON_BUTTON_QUESTION_MARK]);

		int x_coord = j;
		int y_coord = 122;
		int x2_coord = x_coord + 20;
		int y2_coord = y_coord + 20;
		adjustCoords(x_coord, y_coord);
		adjustCoords(x2_coord, y2_coord);

		nglSetQuadRect(&quad, x_coord, y_coord, x2_coord, y2_coord);
		nglSetQuadColor(&quad, NGL_RGBA32(255, 255, 255, FTOI(255.0f * opacity)));
		nglSetQuadZ(&quad, 800.0f);

		x_coord = j + 20;
		y_coord = 122;
		x2_coord = x_coord + 40;
		y2_coord = y_coord + 20;
		adjustCoords(x_coord, y_coord);
		adjustCoords(x2_coord, y2_coord);

		float x_rotate = j + 10;
		float y_rotate = 122 + 10;
		adjustCoords(x_rotate, y_rotate);

		if (i != 1 || iconCounter >= 3)
		{
			switch(b[i])
			{
			case PAD_D:  nglRotateQuad(&quad, x_rotate, y_rotate, PI);       break;
			case PAD_L:  nglRotateQuad(&quad, x_rotate, y_rotate, 1.5f*PI);  break;
			case PAD_R:  nglRotateQuad(&quad, x_rotate, y_rotate, 0.5f*PI);  break;
			case PAD_UL: nglRotateQuad(&quad, x_rotate, y_rotate, 1.75f*PI); break;
			case PAD_UR: nglRotateQuad(&quad, x_rotate, y_rotate, 0.25f*PI); break;
			case PAD_DL: nglRotateQuad(&quad, x_rotate, y_rotate, 1.25f*PI); break;
			case PAD_DR: nglRotateQuad(&quad, x_rotate, y_rotate, 0.75f*PI); break;
			}
		}

		nglListAddQuad(&quad);

		if(i != 2 && b[i+1] != PAD_NONE)
		{
			if(b[2] != PAD_NONE)  // If it's a 3 button combo, only use comma's as separators
				nglSetQuadTex(&quad, buttonTexture[ICON_BUTTON_COMMA]);
			else if(i == 0)
			{
				if(b[1]>=PAD_U && b[1]<=PAD_DR)   // If second button in two button combo is a direction
					nglSetQuadTex(&quad, buttonTexture[ICON_BUTTON_COMMA]);
				else if(!(b[0]>=PAD_U && b[0]<=PAD_DR))   // If first button in two button combo is NOT a direction
					nglSetQuadTex(&quad, buttonTexture[ICON_BUTTON_COMMA]);
				else
					nglSetQuadTex(&quad, buttonTexture[ICON_BUTTON_PLUS]);
			}

			nglSetQuadRect(&quad, x_coord, y_coord, x2_coord, y2_coord);
			nglSetQuadColor(&quad, NGL_RGBA32(255, 255, 255, FTOI(255.0f * opacity)));
			nglSetQuadZ(&quad, 800.0f);
			nglListAddQuad(&quad);
		}
	}
}

//	Update()
// Call often with time delta.
void IGOLearnNewTrickManager::Update(const float dt)
{
	kellyslater_controller *ksctrl = g_world_ptr->get_ks_controller(g_game_ptr->get_active_player());
	bool special_meter_active = ksctrl->get_special_meter()->CanRegionLink();
	int super_state = g_world_ptr->get_ks_controller(g_game_ptr->get_active_player())->get_super_state();
	if (super_state != SUPER_STATE_NO_SUPERSTATE &&
		super_state != SUPER_STATE_FLYBY &&
		super_state != SUPER_STATE_WIPEOUT &&
		super_state != SUPER_STATE_LIE_ON_BOARD)
		addIconTimer -= dt;
	else
		addIconTimer -= dt*0.50f;

	//  Add to the amount of time since we started showing a trick.
	trick_available_time += dt;
	if (trick_available_time > reset_trick_available_time || !special_meter_active)  //  If they've completed the trick, give them another chance later.
	{
		trick_available_time = 0;
		already_got_one = false;
	}

	
	if (icon_active && (!special_meter_active || trick_available_time > max_trick_available_time))	//  Take off the trick icon?
		PopFront();
	else if (!icon_active && special_meter_active && trick_available_time < max_trick_available_time &&  //  Has enough time passed and is the meter special?
			!already_got_one && iconCounter < 3)  //  Have we already done all the icons?
	{
		bool add_trick = false;
		int which_trick_to_learn = CareerDataArray[g_game_ptr->get_level_id()].goal_param[0];
		current_trickIdx = SurferDataArray[g_game_ptr->GetSurferIdx(g_game_ptr->get_active_player())].learnedtrickBook[which_trick_to_learn];

		// Make sure the trick is in the current surfer's trickbook
		for (int trickBookIdx = 0; trickBookIdx < TRICKBOOK_SIZE; trickBookIdx++)
		{
			if(current_trickIdx == SurferDataArray[g_game_ptr->GetSurferIdx(0)].trickBook[trickBookIdx])
			{
				add_trick = true;
				break;
			}
		}

		// If the trick exists, add it, otherwise try again next frame
		if(add_trick)
		{
			PushBack(current_trickIdx);
			trick_available_time = 0.0f;	//  Set the timer for how long the player has a chance to do the trick.
			addIconTimer = addIconDelay;
			already_got_one = true;
		}
		else 
			assert(0);  //  There is a drastic problem if the trick is not in his trickbook but he is supposed to learn it (OK, maybe not drastic, but it *is* a problem).
	}
}

//	PushBack()
// Adds an icon with the specified trick to the screen display.
void IGOLearnNewTrickManager::PushBack(const int trickIdx)
{
	IconResource *	res = FindResource(trickIdx);

	if(!res)
		return;

	if (icon_active)
	{
		return;
	}

	current_icon->Initialize(*res);
	icon_active = true;

	int x_coord = 30, y_coord = 20, 
		x2_coord = x_coord + 96, y2_coord = y_coord + 96;
	adjustCoords(x_coord, y_coord);
	adjustCoords(x2_coord, y2_coord);
	nglSetQuadRect(&current_icon->quad, x_coord, y_coord, x2_coord, y2_coord);
}

//	PopFront()
// Removes the top-most icon from the screen.
void IGOLearnNewTrickManager::PopFront(bool complete)
{
	if (!icon_active)
		return;

	if (complete)
		iconCounter++;

	icon_active = false;

	current_icon->SetShow(false);

	addIconDelay *= 0.97f;
}

//	TrickChain()
// Receives each trick that is completed, and acts upon it
void IGOLearnNewTrickManager::TrickChain(int trickIdx)
{
	if (current_icon->resource && trickIdx == current_icon->resource->trickIdx)
	{
		if (GTrickList[trickIdx].trick_type == TRICKTYPE_AERIAL)  //  With the air tricks, wait until the surfer lands.
			got_one_currently = true;
		else
			PopFront(true);
	}
}

//	FindResource()
// Returns the icon resource with the specified trick index.
IGOLearnNewTrickManager::IconResource* IGOLearnNewTrickManager::FindResource(const int trickIdx)
{
	int i;

	for (i = 0; i < numIconResources; i++)
	{
		if (iconResources[i].trickIdx == trickIdx)
			return &iconResources[i];
	}

	return NULL;
}


void IGOLearnNewTrickManager::OnEvent(const EVENT event, const int param1, const int param2)
{
	//  is there a trick currently waiting to see if it is landed?
	if (got_one_currently && param1 == g_game_ptr->get_active_player())  
	{
		if (event == EVT_SURFER_LAND)
		{
			PopFront(true);
			got_one_currently = false;
		}
		else if (event == EVT_SURFER_WIPEOUT)
			got_one_currently = false;
	}
}


bool IGOLearnNewTrickManager::CheckTrickPerformed()
{
	char buf[64];

	//  Check to see if the trick has been performed since last we checked.
	if (prev_iconCounter < iconCounter)
	{
		prev_iconCounter = iconCounter;

		if (iconCounter <= 3)
		{
			if (iconCounter == 1)
				sprintf (buf, ksGlobalTextArray[GT_GOAL_PERFORM_NEW_TRICK].c_str(), iconCounter, ksGlobalTrickTextArray[current_trickIdx].c_str());
			else
				sprintf (buf, ksGlobalTextArray[GT_GOAL_PERFORM_NEW_TRICK_PLURAL].c_str(), iconCounter, ksGlobalTrickTextArray[current_trickIdx].c_str());
			frontendmanager.IGO->Print(buf);

		}
	}

	if (iconCounter == 3)
		return true;
	else
		return false;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	IGOLearnNewTrickManager::IconResource class
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//	IconResource()
// Default constructor.
IGOLearnNewTrickManager::IconResource::IconResource()
{
	texture = NULL;
	trickIdx = -1;
}

//	~IconResource()
// Destructor.
IGOLearnNewTrickManager::IconResource::~IconResource()
{

}

//	Load()
// Initializes the resource and laods the specified texture.
void IGOLearnNewTrickManager::IconResource::Load(const int idx, const stringx & texFilename)
{
	trickIdx = idx;
	texture = nglLoadTexture(texFilename.c_str());
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	IGOLearnNewTrickManager::Icon class
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//	Icon()
// Default constructor.
IGOLearnNewTrickManager::Icon::Icon()
{
	show = true;
	resource = NULL;
}

//	~Icon()
// Destructor.
IGOLearnNewTrickManager::Icon::~Icon()
{

}

//	Initialize()
// Initializes this icon with the specified resource.
void IGOLearnNewTrickManager::Icon::Initialize(IconResource & res)
{
	show = true;
	resource = &res;

	nglInitQuad(&quad);
	nglSetQuadTex(&quad, res.texture);
	nglSetQuadColor(&quad, NGL_RGBA32(255, 255, 255, 0));
	nglSetQuadZ(&quad, 800.0f);

}

//	Draw()
// Adds this icon to the current render list.
void IGOLearnNewTrickManager::Icon::Draw(float opacity)
{
	if (show)
		nglListAddQuad(&quad);

	nglSetQuadColor(&quad, NGL_RGBA32(255, 255, 255, FTOI(255.0f * opacity)));

}

//	SetShow()
// Toggles the display of this icon.
void IGOLearnNewTrickManager::Icon::SetShow(const bool s)
{
	show = s;
}

