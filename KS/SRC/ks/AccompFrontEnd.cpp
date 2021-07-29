// AccompFrontEnd.cpp

// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"
#include "CompressedPhoto.h"
#include "osGameSaver.h"
#if _XBOX
#include "xbglobals.h"
#endif /* TARGET_XBOX JIV DEBUG */

#include "AccompFrontEnd.h"

AccompFrontEnd::AccompFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pf_name)
{
	cons(s, man, p, pf_name);
	
	color = manager->yel_dk;
	color_high = manager->yel_lt;
#ifdef TARGET_XBOX
	tempTex[0] = nglCreateTexture(NGLTF_32BIT | NGLTF_LINEAR, CompressedPhoto::PHOTO_WIDTH,CompressedPhoto::PHOTO_HEIGHT);
	tempTex[1] = nglCreateTexture(NGLTF_32BIT | NGLTF_LINEAR, CompressedPhoto::PHOTO_WIDTH,CompressedPhoto::PHOTO_HEIGHT);
#else
	tempTex[0] = nglCreateTexture(NGLTF_32BIT, CompressedPhoto::PHOTO_WIDTH,CompressedPhoto::PHOTO_HEIGHT);
	tempTex[1] = nglCreateTexture(NGLTF_32BIT, CompressedPhoto::PHOTO_WIDTH,CompressedPhoto::PHOTO_HEIGHT);
#endif
	FEMenuEntry* feme = NEW FEMenuEntry("", this);
	Add(feme);

	float sc = 0.888f * 1.1875f;
	beach[0] = NEW TextString(&manager->font_bold_old, "", 191, 356, 0, sc, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, manager->gray_dk);
	beach[1] = NEW TextString(&manager->font_bold_old, "", 445, 356, 0, sc, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, manager->gray_dk);

	for(int i=0; i<MAX_PAGES; i++)
	{
		picture_exists[i] = false;
		beach_index[i] = -1;
		level_index[i] = -1;
	}
	for(int i=0; i<BEACH_LAST; i++)
		mag_cover_beach_index[i] = -1;
}

AccompFrontEnd::~AccompFrontEnd()
{
	nglDestroyTexture(tempTex[0]);
	nglDestroyTexture(tempTex[1]);
	tempTex[0] = NULL;
	tempTex[1] = NULL;
	delete beach[0];
	delete beach[1];
}

void AccompFrontEnd::Load()
{
	FEMultiMenu::Load();
	SetPQIndices();

	noImage = mag_photos[0]->GetTexture();
}

void AccompFrontEnd::Draw()
{
	if(!manager->em->OKtoDrawScrapbook()) return;

	FEMultiMenu::Draw();
	if(picture_exists[cur_page*2]) beach[0]->Draw();
	if(picture_exists[cur_page*2+1]) beach[1]->Draw();
}

void AccompFrontEnd::OnActivate()
{
	int previous = system->active;
	FEMultiMenu::OnActivate();

	if(previous == GraphicalMenuSystem::ExtrasMenu)
	{
		manager->em->ToMainScreen();
		back = system->menus[GraphicalMenuSystem::ExtrasMenu];
		back_num = 1;
	}
	else
	{
		// only for coming from ss, or direct from game
		manager->em->ToSurferSelect();
		back = system->menus[GraphicalMenuSystem::SurferMenu];
		back_num = SurferFrontEnd::ACT_SURFER;
	}

	num_photos = 0;
	int cover_index = 0;
	for(int i=0; i<LEVEL_LAST; i++)
	{	
		if (CareerDataArray[i].goal[0] == GOAL_PHOTO_1 ||
			CareerDataArray[i].goal[0] == GOAL_PHOTO_2 ||
			CareerDataArray[i].goal[0] == GOAL_PHOTO_3)
		{
			beach_index[num_photos] = CareerDataArray[i].beach;
			level_index[num_photos] = i;

			// Keep track of which beaches we are 
			// snapping pics for
			if (g_career->PhotoExistsForLevel (i))
			{
				mag_cover_beach_index[CareerDataArray[i].beach] = cover_index;
				cover_index++;
				num_photos++;
			}
			assert(num_photos < MAX_PAGES);
		}
	}

	num_pages = (num_photos/2)+(num_photos%2);
	cur_page = 0;

	for(int i=0; i<num_covers; i++)
	{
		mag_covers[i][0]->TurnOn(i == cur_page*2);
		mag_covers[i][1]->TurnOn(i == cur_page*2+1);
	}
	UpdatePhotos();

	manager->helpbar->Reset();
	manager->helpbar->AddArrowH(ksGlobalTextArray[GT_FE_MENU_SWITCH]);
	manager->helpbar->RemoveX();
	manager->helpbar->Reformat();
}

void AccompFrontEnd::SetPQIndices()
{
	// arrows not needed
	GetPointer("button_off_01")->TurnOn(false);
	GetPointer("button_on_01")->TurnOn(false);
	GetPointer("button_off_02")->TurnOn(false);
	GetPointer("button_on_02")->TurnOn(false);

	for(int i=0; i<8; i++)
	{
		stringx tmp = stringx(i+1);
		mag_covers[i][0] = GetPointer(("sb_mag_0"+tmp+"a").data());
		mag_covers[i][1] = GetPointer(("sb_mag_0"+tmp+"b").data());
		mag_corners[i%4][i/4] = GetPointer(("sb_corner_0"+tmp).data());
		pol_corners[i%4][i/4] = GetPointer(("sb_corner_0"+tmp+"_polaroid").data());
	}

	mag_photos[0] = GetPointer("sb_image_blank01");
	mag_photos[1] = GetPointer("sb_image_blank02");
	mag_bright_cover[0] = GetPointer("sb_texture_bright_1");
	mag_bright_cover[1] = GetPointer("sb_texture_bright_2");
	mag_dark_cover[0] = GetPointer("sb_texture_dark_1");
	mag_dark_cover[1] = GetPointer("sb_texture_dark_2");
	mag_shadow[0] = GetPointer("sb_mag_shadow_left");
	mag_shadow[1] = GetPointer("sb_mag_shadow_right");
	pol_covers[0] = GetPointer("sb_polaroid_left");
	pol_covers[1] = GetPointer("sb_polaroid_right");
	pol_photos[0] = GetPointer("sb_polaroid_left_blank");
	pol_photos[1] = GetPointer("sb_polaroid_right_blank");
}

void AccompFrontEnd::UpdatePhotos()
{
	// Turn off all mag covers in preparation for updating pages
	for(int i=0; i<num_covers; i++)
	{
		mag_covers[i][0]->TurnOn(false);
		mag_covers[i][1]->TurnOn(false);
	}

	UpdatePage(true);
	UpdatePage(false);
}

void AccompFrontEnd::UpdatePage(bool right_page)
{
	int side = right_page? 1 : 0;
	int page = cur_page*2 + side;

	if(beach_index[page] != -1)
		picture_exists[page] = g_career->PhotoExistsForLevel (level_index[page]);

//	bool mag = false;
//	if(level_index[page] != -1)
//		mag = CareerDataArray[level_index[page]].goal[0] != GOAL_PHOTO_1;

	// always use magazine now
	bool mag = true;

	if(picture_exists[page])
	{
		g_career->GetPhotoForLevel (level_index[page])->CopyToTexture (tempTex[side]);

		// if mag cover, then have to change UVs to make it rectangular
		// right now, photo_1 challenges have polaroids, and others have mag covers
		if(mag)
		{
			float w = mag_covers[0][0]->GetWidthA();
			float h = mag_covers[0][0]->GetHeightA();
			// assuming that the picture will be scaled to a square the size of the height,
			// determine how much needs to be "trimmed" off the sides
			float excess = ((h-w)/2.0f)/h;
			mag_photos[side]->SetUV(excess, 0.0f, 1.0f - excess, 1.0f);
			mag_photos[side]->setTexture(tempTex[side]);
		}
		else
		{
			pol_photos[side]->SetUV(0.0f, 0.0f, 1.0f, 1.0f);
			pol_photos[side]->setTexture(tempTex[side]);
		}
	}

	if(picture_exists[page] && mag)
		mag_covers[mag_cover_beach_index[beach_index[page]]][side]->TurnOn(true);
	mag_bright_cover[side]->TurnOn(picture_exists[page] && mag);
	mag_dark_cover[side]->TurnOn(picture_exists[page] && mag);
	mag_shadow[side]->TurnOn(picture_exists[page] && mag);
	mag_photos[side]->TurnOn(picture_exists[page] && mag);

	pol_covers[side]->TurnOn(picture_exists[page] && !mag);
	pol_photos[side]->TurnOn(picture_exists[page] && !mag);

	for(int i=0; i<4; i++)
	{
		mag_corners[i][side]->TurnOn(picture_exists[page] && mag);
		pol_corners[i][side]->TurnOn(picture_exists[page] && !mag);
	}

	if(picture_exists[page])
		beach[side]->changeText(BeachDataArray[beach_index[page]].fe_name);
}

void AccompFrontEnd::SwitchPages(bool right)
{
	if(right) cur_page++;
	else cur_page--;
	UpdatePhotos();
}

void AccompFrontEnd::OnLeft(int c)
{
	if(cur_page > 0)
	{
		SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
		SwitchPages(false);
	}
	else
		SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
}

void AccompFrontEnd::OnRight(int c)
{
	if(cur_page < num_pages-1)
	{
		SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
		SwitchPages(true);
	}
	else
		SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
}

void AccompFrontEnd::OnTriangle(int c)
{
	SoundScriptManager::inst()->playEvent(SS_FE_BACK);
	if (this->back != frontendmanager.gms->menus[GraphicalMenuSystem::ExtrasMenu])
		manager->em->BioTBZoom(false);
	FEMultiMenu::OnTriangle(c);
}