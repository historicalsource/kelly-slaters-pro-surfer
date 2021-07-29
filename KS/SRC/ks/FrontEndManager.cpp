// FrontEndManager.cpp

// With precompiled headers enabled, all text up to and including
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#include "FrontEndManager.h"
#include "pmesh.h"		// For vr_pmesh_bank
#include "colmesh.h"	// For cg_mesh_bank
#include "GlobalData.h"

/****************************** FEManager ******************************/

FEManager::FEManager()
{
#ifdef TARGET_GC
		// GameCube doesn't like functions with >32K of local data
	GCConstructorKludge1();
	GCConstructorKludge2();
#else
	trick_font.unset = true;
	numberFont.unset = true;
	//clockFont.unset = true;

	bio_scale = 0.7f;
	fe_init = false;
	fe_done = false;
	fe_done_loading = false;
	in_game_map_up = false;
	score_display = true;
	gms_skip_legal = false;
	start_on = false;
#endif

	map_loading_screen = false;
	default_pq = NULL;
	new_photo = false;
	tmp_game_mode = NUMBER_OF_GAME_MODES;	// set it to something invalid
	fontsLoaded = false;
	unsaved_career = false;

	font_hand.unset = true;
	font_thin.unset = true;
	font_bold.unset = true;
	font_info.unset = true;
	font_bold_old.unset = true;
	font_body.unset = true;

	// slowly weeding these out...
	yel_lt = color32(255, 255, 102, 255);
	yel_dk = color32(204, 170, 0, 255);
	green = color32(149, 251, 149, 255);
	green_br = color32(0, 255, 0, 255);
	blue = color32(0, 255, 255, 255);
	white = color32(255, 255, 255, 255);
	gray_dk = color32(20, 20, 20, 255);
	red_dk = color32(120, 50, 50, 255);
	blue_dk = color32(29, 53, 74, 255);

	// ... in favor of these
	//col_unselected = color32(153, 163, 171, 255);
	col_unselected = color32(255, 255, 255, 255);
	col_highlight = color32(255, 255, 102, 255);
	col_highlight2 = color32(204, 170, 0, 255);
	col_menu_header = color32(204, 170, 0, 255);
	col_help_bar = color32(153, 163, 171, 255);
	col_info_b = color32(0, 255, 255, 255);
	col_info_g = color32(149, 251, 149, 255);
	col_bio = color32(20, 20, 20, 255);
	col_bad = color32(235, 53, 8, 255);

	extras_movie          = false;
	extras_movie_sub      = -1;
	extras_movie_sub_sub  = -1;
}

void FEManager::GCConstructorKludge1( void )
{
	trick_font.unset = true;
}

void FEManager::GCConstructorKludge2( void )
{
	bio_scale = 0.7f;
	fe_init = false;
	fe_done = false;
	fe_done_loading = false;
	in_game_map_up = false;
	score_display = true;
	gms_skip_legal = false;
	start_on = false;
}


FEManager::~FEManager()
{
	delete IGO;
	delete pms;
	delete gms;
	delete paf;
	delete default_pq;
	delete map;
	delete helpbar;
	trick_font.unload();
	numberFont.unload();
	//clockFont.unload();

	font_hand.unload();
	font_thin.unload();
	font_bold.unload();
	font_info.unload();
	font_bold_old.unload();
	font_body.unload();
}

FloatingPQ* FEManager::GetDefaultPQ()
{
	if (!default_pq)
	{
		// Moved here to avoid calling nglGetScreenWidth from the constructor.
		default_pq = NEW FloatingPQ(0, 0, 100, 100, .5, 1, .5, 1, 0);
		default_pq->SetBehaviorNF(50, 50);
	}

	return default_pq;
}

void FEManager::LoadMap()
{
	LoadFonts();
	map = NEW BeachFrontEnd(NULL, this, "interface\\map\\", "FE_map.PANEL");
	map->Load();
	LoadScores();
	helpbar = NEW HelpbarFE(this, "levels\\frontend\\overlays\\", "helpbar.PANEL");
	helpbar->LoadPanel();
}

void FEManager::LoadFonts()
{
	if (fontsLoaded)
		return;

	if(trick_font.load("trick")) trick_font.unset = false;
	else trick_font.unset = true;
	if(numberFont.load("numbersitalic")) numberFont.unset = false;
	else numberFont.unset = true;

	if(font_hand.load("handwritten")) font_hand.unset = false;
	else font_hand.unset = true;
	if(font_thin.load("title_thin")) font_thin.unset = false;
	else font_thin.unset = true;
	if(font_bold.load("title_bold")) font_bold.unset = false;
	else font_bold.unset = true;
	if(font_info.load("info")) font_info.unset = false;
	else font_info.unset = true;
	if(font_bold_old.load("bold")) font_bold_old.unset = false;
	else font_bold_old.unset = true;
	if(font_body.load("body")) font_body.unset = false;
	else font_body.unset = true;

	fontsLoaded=true;
}

void FEManager::LoadScores()
{
    nglFileBuf F;
    KSReadFile("interface\\map\\default_high_scores.txt", &F, 1);
	if(!F.Buf)
	{
		KSReleaseFile(&F);
		assert(0);
	}
    char *s = (char *) F.Buf;

	HighScoreData tmp;
	unsigned int count = 0;
	while(s[count] == '/' || s[count] == '\r')
	{
		if(s[count] == '/' && count+1 < F.Size && s[count+1] == '*')
		{
			count += 2;
			while(count < F.Size && !(s[count] == '*' && count+1 < F.Size && s[count+1] == '/'))
				count++;
			count += 2;
		}
		if(s[count] == '\r') count += 2;
	}
	s = &s[count];

	for(int i=0; i<BEACH_LAST*2; i++)
	{
		for(int j=0; j<HighScoreFrontEnd::NUM_ROWS; j++)
		{
			count = 0;
			if(s[count] == '\r') count += 2;
			if(s[count] == '/' && count+1 < F.Size && s[count+1] == '*')
			{
				count += 2;
				while(count < F.Size && !(s[count] == '*' && count+1 < F.Size && s[count+1] == '/'))
					count++;
				count += 2;
			}

			if(s[count] == '\r') count++;
			if(s[count] == '\n') count++;

			int character;
			if(i < BEACH_LAST)
				sscanf(&s[count], "%s %d %d\n", tmp.initials, &character, &tmp.score);
			else sscanf(&s[count], "%s %d %d %d\n", tmp.initials, &character, &tmp.score, &tmp.icons);
			strcpy(tmp.character, SurferDataArray[character].fullname);

			int bn = i;
			if(bn >= BEACH_LAST) bn -= BEACH_LAST;
			globalCareerData.setBeachHighScore(bn, j, i >= BEACH_LAST, tmp);
			s = strchr(&s[count], '\n');
			if(s[0] == '\n') s = &s[1];
		}
	}

	KSReleaseFile(&F);
}

int FEManager::AddHighScore(stringx name, int score, int icons, int character)
{
	// if icons is -1, this means that we aren't looking at the icon challenge high list
	bool icon_mode = icons != -1;
	int beach;
	beach = g_game_ptr->get_beach_id();
	int line = -1;


	HighScoreData temp;
	for(int i=HighScoreFrontEnd::NUM_ROWS-1; i>=0; i--)
	{
		int sc = globalCareerData.getBeachHighScore(beach,i, icon_mode).score;
		int ic = globalCareerData.getBeachHighScore(beach,i, icon_mode).icons;

		if((!icon_mode && score > sc) || (icon_mode && (icons > ic || icons == ic && score > sc)))
		{
			if(i == HighScoreFrontEnd::NUM_ROWS-1)
			{
				temp.initials[0] = name.data()[0];
				temp.initials[1] = name.data()[1];
				temp.initials[2] = name.data()[2];
				temp.initials[3] = '\0';
				strcpy( temp.character, SurferDataArray[character].fullname );
				temp.score = score;
				temp.icons = icons;
				globalCareerData.setBeachHighScore(beach, HighScoreFrontEnd::NUM_ROWS-1, icon_mode, temp);
			}
			else
			{
				HighScoreData hsd = globalCareerData.getBeachHighScore(beach, i, icon_mode);
				globalCareerData.setBeachHighScore(beach, i, icon_mode, globalCareerData.getBeachHighScore(beach,i+1, icon_mode));
				globalCareerData.setBeachHighScore(beach, i+1, icon_mode, hsd);
			}
			line = i;
		}
	}
	return line;
}

void FEManager::ReloadFontTextures()
{
	if(!trick_font.unset) trick_font.reload();
	if(!numberFont.unset) numberFont.reload();
	//if(!clockFont.unset) clockFont.reload();

	if(!font_hand.unset) font_hand.reload();
	if(!font_thin.unset) font_thin.reload();
	if(!font_bold.unset) font_bold.reload();
	if(!font_info.unset) font_info.reload();
	if(!font_bold_old.unset) font_bold_old.reload();
	if(!font_body.unset) font_body.reload();
}

void FEManager::ReloadTextures()
{
	ReloadFontTextures();
	map->ReloadPanel();
	map->ReloadMap();
	helpbar->ReloadPanel();
}

void FEManager::OnLevelLoaded()
{
	map->OnLevelLoaded();
}

void FEManager::OnLevelEnding()
{
	map->OnLevelEnding();
}
extern 	bool g_noPlayerKilled;
void FEManager::InitFE()
{
	fe_done = false;
	g_noPlayerKilled=false;
	tmp_game_mode = g_game_ptr->get_game_mode();
	em = NEW FEEntityManager(this);
	gms = NEW GraphicalMenuSystem(this);

	map->SwitchState(gms, true);

#ifdef TARGET_XBOX
  // Start asynchronous DVD caching now that we're going into the frontend
  gDVDCache.StartCaching();
#endif // TARGET_XBOX

	// reloads map & font textures, which should be in memory at all times
	ReloadTextures();
	((BeachFrontEnd*)gms->menus[GraphicalMenuSystem::BeachMenu])->SkipSlide(true);
	((BeachFrontEnd*)gms->menus[GraphicalMenuSystem::BeachMenu])->ResetFirstTime();
	fe_init = true;
}

void FEManager::InitIGO()
{
	IGO = NEW IGOFrontEnd(this, "interface\\IGO\\", "IGO.PANEL");
	pms = NEW PauseMenuSystem(this, &font_info);

	map->SwitchState(pms, false);
	map->ResetFirstTime();

	IGO->Add(PanelAnimManager());
	IGO->Init();
	IGO->SetDisplay(score_display);
	pms->Load();	// called after IGO's panel is loaded
	fe_init = false;
}

#if defined(DEBUG) && defined(TARGET_XBOX)
// For setting wireframe mode
extern uint8 *nglGetDebugFlagPtr(const char *Flag);
#endif

void FEManager::DrawFE()
{
#if defined(TARGET_XBOX) && defined(DEBUG)
	// enable wireframe rendering if the flag is set
	if(os_developer_options::inst()->is_flagged( os_developer_options::FLAG_WIREFRAME ))
	{
		*nglGetDebugFlagPtr("DrawAsLines") = false;
	}
#endif
//	nglSetClearFlags(0);
	nglListBeginScene();
	nglSetClearFlags(NGLCLEAR_Z | NGLCLEAR_COLOR | NGLCLEAR_STENCIL);
	gms->DrawMovie();
	nglListEndScene();

	nglListBeginScene();
	nglSetClearFlags(NGLCLEAR_Z | NGLCLEAR_STENCIL);
	em->DrawBack();
	gms->UpdateInScene();
	nglListEndScene();

	nglListBeginScene();
	nglSetClearFlags(NGLCLEAR_Z | NGLCLEAR_STENCIL);
	gms->Draw();
	nglListEndScene();

	nglListBeginScene();
	nglSetClearFlags(NGLCLEAR_Z | NGLCLEAR_STENCIL);
	em->DrawFront();
	nglListEndScene();

	nglListBeginScene();
	nglSetClearFlags(NGLCLEAR_Z | NGLCLEAR_STENCIL);
	gms->DrawTop();
	nglListEndScene();

#if defined(TARGET_XBOX) && defined(DEBUG)
	// enable wireframe rendering if the flag is set
	if(os_developer_options::inst()->is_flagged( os_developer_options::FLAG_WIREFRAME ))
	{
		*nglGetDebugFlagPtr("DrawAsLines") = true;
	}
#endif
}

  // this is to make it easy to disable the IGOs from the debugger
bool g_igo_enabled=true;

void FEManager::DrawIGO()
{
#if defined(TARGET_XBOX) && defined(DEBUG)
	// enable wireframe rendering if the flag is set
	if(os_developer_options::inst()->is_flagged( os_developer_options::FLAG_WIREFRAME ))
	{
		*nglGetDebugFlagPtr("DrawAsLines") = false;
	}
#endif
	if (g_igo_enabled)
	{
		IGO->Draw();
		pms->Draw();
	}
#if defined(TARGET_XBOX) && defined(DEBUG)
	// enable wireframe rendering if the flag is set
	if(os_developer_options::inst()->is_flagged( os_developer_options::FLAG_WIREFRAME ))
	{
		*nglGetDebugFlagPtr("DrawAsLines") = true;
	}
#endif
}

void FEManager::DrawMap(float loading_progress)
{
	nglListBeginScene();
	ksnglSetPerspectiveMatrix(90.0f, nglGetScreenWidth()/2, nglGetScreenHeight()/2, 0.2f, 1200.0f); // Setup zfar to emulate IGO
	nglSetClearFlags(NGLCLEAR_Z | NGLCLEAR_COLOR | NGLCLEAR_STENCIL);
	nglSetClearColor(0, 0, 0, 0);
	map->DrawMap(loading_progress);
	nglListEndScene();
}

void FEManager::UpdateFE(time_value_t time_inc)
{
	em->Update(time_inc);
	gms->Update(time_inc);
}

void FEManager::UpdateIGO(time_value_t time_inc)
{
	IGO->Update(time_inc);
	pms->Update(time_inc);

/*
	// start just released
	if (start_on && !getButtonState(FEMENUCMD_START))
		start_on = false;

	// If START was pressed and we are in a competition level, pop up heat mid menu.
	if(!start_on && getButtonState(FEMENUCMD_START))
	{
		start_on = true;

		if (g_game_ptr->is_competition_level() &&
			g_game_ptr->get_game_mode() == GAME_MODE_CAREER &&
			IGO->GetRunState() == IGOFrontEnd::RUNSTATE_NORMAL &&
			!compEndMenu->draw)
		{
			if (!heatMidMenu->draw)
				heatMidMenu->startDraw();
			else
				heatMidMenu->endDraw();
		}
	}
	*/
}

void FEManager::UpdateIGOScene()
{
	if(pms->draw) pms->UpdateInScene();
}
extern nslSoundId feMusic;
void FEManager::ReleaseFE()
{
	mem_lock_malloc(false);
#ifdef TARGET_PS2
  GenericGameSaver::inst()->releaseIconData();
#endif
	// exit the menu system
  g_game_ptr->start_drawing_map = false;
  g_entity_maker->purge_entity_cache();
  g_world_ptr->unload_scene();
	feMusic = NSL_INVALID_ID;
	delete gms;
	gms = NULL;
	lfe = NULL;	// deleted already as part of "delete gms" (dc 04/26/02)
	tfe = NULL;
	delete em;
	em = NULL;

    #ifdef USINGSTATICSTLALLOCATIONS
  // Also add to void void game::end_level( void ) in game.cpp
	entity_anim::mem_cleanup();
	entity_anim_tree::mem_cleanup();
	po_anim::mem_cleanup();
	linear_anim<quaternion>::mem_cleanup();
	linear_anim<vector3d>::mem_cleanup();
	linear_anim<rational_t>::mem_cleanup();
  entity::movement_info::mem_cleanup();
#endif

	app::cleanup_stl_memory_dregs();
//  anim_id_manager::inst()->purge();
  entity_manager::inst()->purge();

#ifdef TARGET_PS2
  vr_pmesh_bank.purge();
  vr_pmesh_bank.debug_dump();
#endif

  hw_texture_mgr::inst()->unload_all_textures();

  slc_manager::inst()->purge();

#ifdef TARGET_PS2
  cg_mesh_bank.purge();
#endif

  //MJDFIXME
  //entity_track_bank.purge();

  // stuff with materials has to go before materials
  vr_billboard_bank.purge();
#ifdef TARGET_PS2
  	vr_pmesh_bank.purge();
#endif
  material_bank.purge();
#ifdef USE_FILE_MANAGER
  file_manager::inst()->clear_level_context();
#endif

	// release all meshes now in em destructor
	nglReleaseAllTextures();
	//ReloadTextures();

	fe_done = true;
	fe_done_loading = false;
	/*
	bold.unload();
	body.unload();
	*/
}

void FEManager::ReleaseIGO()
{
	delete IGO;
	IGO = NULL;
	delete pms;
	
	/*
	trick_font.unload();
	numberFont.unload();
	clockFont.unload();
	*/
}

/////////////////////////////////////////////////////////
// global variables & functions
/////////////////////////////////////////////////////////

FEManager frontendmanager;// = FEManager();

bool FEInitialized()
{
	return frontendmanager.fe_init;
}

void FEInit()
{
	frontendmanager.InitFE();
}

void FEUpdate(time_value_t time_inc)
{
	frontendmanager.UpdateFE(time_inc);
}

void FEDraw()
{
	frontendmanager.DrawFE();
}

bool FEDone()
{
	return frontendmanager.fe_done;
}

bool FEDoneLoading()
{
	return frontendmanager.fe_done_loading;
}

void FERelease()
{
	frontendmanager.ReleaseFE();
}

void IGOUpdate(time_value_t time_inc)
{
	frontendmanager.UpdateIGO(time_inc);
}

void IGODraw()
{
	frontendmanager.DrawIGO();
}

void IGOStandUp()
{
	frontendmanager.IGO->OnSurferStandUp();
}

void IGODebug(bool on)
{
	frontendmanager.IGO->DebugMenu(on);
}

void IGOPrint(stringx text)
{
	frontendmanager.IGO->Print(text);
}

bool IGOIsPaused()
{
	return frontendmanager.pms->draw;
}

void IGORelease()
{
	frontendmanager.ReleaseIGO();
}
