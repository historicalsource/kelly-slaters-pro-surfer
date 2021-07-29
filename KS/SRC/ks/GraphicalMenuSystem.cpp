
// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"


#include "GraphicalMenuSystem.h"
#include "profiler.h"

#if defined(TARGET_XBOX)
#include "refptr.h"
#include "conglom.h"
#include "wavesound.h"
#include "hwrasterize.h"
#endif /* TARGET_XBOX JIV DEBUG */

#include "FrontEndManager.h"
#include "kellyslater_controller.h"
#include "VOEngine.h"
#include "random.h"
#include "trick_system.h"
#include "rumbleManager.h"
#include "GameData.h"
#include "MCDetectFrontEnd.h"
#include "file_finder.h"
#include "DemoMode.h"
#if defined(TARGET_XBOX)
#include "ngl.h"
#include "wds.h"
#include "osdevopts.h"
#elif defined(TARGET_GC)
#include "ngl.h"
#else
#include "ngl_instbank.h"
#endif /* TARGET_XBOX JIV DEBUG */

#ifdef LEO
//#define ANOTHER_MOVIE
#endif

#define MOVIE_BUFFER_SIZE 0x100000

/************************ FEDebugMenu **********************************/

void FEDebugMenu::AddAll()
{
	Add(NEW FEMenuEntry("Surfer", this));
	Add(NEW FEMenuEntry("Board", this));
	Add(NEW FEMenuEntry("Room", this));
	Add(NEW FEMenuEntry("Overlays", this));
	Add(NEW FEMenuEntry("Language", this));
	Add(NEW FEMenuEntry("User Cam", this));
	Add(NEW FEMenuEntry("Normal Cam", this));
	Init();
}

void FEDebugMenu::Select(int entry)
{
	camera* cam;
	switch(entry)
	{
	case FEDB_Overlays:
		((GraphicalMenuSystem*) system)->fedb_draw_overlays = !((GraphicalMenuSystem*) system)->fedb_draw_overlays;
		break;
	case FEDB_Language: 
		if(ksGlobalTextLanguage == LANGUAGE_ENGLISH) ksGlobalTextLanguage = LANGUAGE_PIG_LATIN;
		else ksGlobalTextLanguage = LANGUAGE_ENGLISH;
		GLOBALTEXT_Load();
		break;
	case FEDB_UserCam:
		cam = find_camera(entity_id("USER_CAM"));
		app::inst()->get_game()->set_current_camera(cam);
		((GraphicalMenuSystem*) system)->usercam = true;
		man->em->active_camera = cam;
		break;
	case FEDB_NormCam:
		app::inst()->get_game()->set_current_camera(man->em->fe_camera);
		((GraphicalMenuSystem*) system)->usercam = false;
		man->em->active_camera = man->em->fe_camera;
		break;
	default: 
		man->em->FEDB_ToggleDraw(entry);
		break;
	}
}

/************************ LegalFrontEnd ********************************/

LegalFrontEnd::LegalFrontEnd(FEManager* man, stringx p, stringx pf_name)
{
	cons(NULL, man, p, pf_name);
	scale = scale_high = 0.666f*1.5f;
	color_high = color = manager->white;

	Add(NEW FEMenuEntry("", this, false, &frontendmanager.font_body));
	legal_babble = NEW BoxText(&frontendmanager.font_body, "", 320, 130, 0, scale, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, color, 15);

	Init();
	timer = 0;
}

LegalFrontEnd::~LegalFrontEnd()
{
	delete legal_babble;
}

void LegalFrontEnd::Update(time_value_t time_inc)
{
	timer += time_inc;
#ifdef BUILD_FINAL
	if(timer >= 10.0f) Select(0);
#else 
	if(timer >= 5.0f) Select(0);
#endif
	FEGraphicalMenu::Update(time_inc);
}

void LegalFrontEnd::Draw()
{
	legal_babble->Draw();
}

// for the language set
#ifdef TARGET_PS2
#include "libscf.h"
#endif

void LegalFrontEnd::Load()
{
	stringx tmp = path+"legal_text_";
#ifdef TARGET_PS2
	tmp = tmp + "ps2";
#elif defined(TARGET_XBOX)
	tmp = tmp + "xbox";
#elif defined(TARGET_GC)
	tmp = tmp + "gc";
#else
#error Platform not declared!
#endif

	if(ksGlobalTextLanguage == LANGUAGE_FRENCH)
		tmp += "_fr";
	else if(ksGlobalTextLanguage == LANGUAGE_GERMAN)
		tmp += "_ge";
	tmp += ".txt";

	legal_babble->ReadFromFile((char*) (tmp.data()), false);
	legal_babble->makeBox(550, 300);
}

void LegalFrontEnd::Select(int n)
{
	if(timer > 5.0f && system)
	{
		system->MakeActive(GraphicalMenuSystem::TitleMenu);

	}
}

void LegalFrontEnd::SetSystem( FEMenuSystem* s )
{
	system = s;
}

/************************ TitleFrontEnd ********************************/

TitleFrontEnd::TitleFrontEnd(/*FEMenuSystem* s, */FEManager* man, stringx p, stringx pf_name)
{
	cons(NULL, man, p, pf_name);
	loading_draw_counter = 0;
	scale = scale_high = 1.0f;
	color = manager->yel_dk;
	color_high = manager->yel_lt;

	FEMenuEntry* tmp = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_PRESS_START], this, false, &man->font_bold);
	Add(tmp);
	tmp->SetPos(320, 320);

#ifdef TARGET_GC
	mc = NEW GCMCDetectFrontEnd(NULL, man, stringx("interface\\Legal\\"), stringx("Legal.PANEL"));
#else	
	mc = NEW MCDetectFrontEnd(NULL, man, stringx("interface\\Legal\\"), stringx("Legal.PANEL"));
#endif
	AddSubmenu(mc);

	loading = NEW TextString(&man->font_bold, ksGlobalTextArray[GT_FE_MENU_TITLE_LOADING], 320, 320, 0, scale, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, color);

	Init();
}

TitleFrontEnd::~TitleFrontEnd()
{
	delete loading; 
	delete mc;
}

void TitleFrontEnd::Update(time_value_t time_inc)
{
	if (active)
		active->Update(time_inc);
	else
	{
		if(loading_draw_counter > 4)
		{
			manager->em->ToOtherState();
			((GraphicalMenuSystem*) system)->LoadAll();
			loading_draw_counter = -1;
		}
		FEGraphicalMenu::Update(time_inc);
	}
}

void TitleFrontEnd::OnLeft(int c)
{
	if (active)
		active->OnLeft(c);
}

void TitleFrontEnd::OnRight(int c)
{
	if (active)
		active->OnRight(c);
}

#ifdef TARGET_XBOX
void TitleFrontEnd::OnTriangle(int c)
{
	if (active)
		active->OnTriangle(c);
}
#endif

void TitleFrontEnd::Draw()
{
	bkg->Draw(0);
	if (active)
	{
		if (active != mc || mc->drawMenu()) 
		{
			for(int i=0; i<9; i++)
				box[i]->Draw(0);
		}
		active->Draw();
	}
	else
	{
		if(loading_draw_counter >= 0)
			loading_draw_counter++;
		if(frontendmanager.fe_done_loading)
			highlighted->Draw();
		else loading->Draw();
	}
}

void TitleFrontEnd::Load()
{
	loading_draw_counter = 0;
	LoadPanel();
	bkg = GetPointer("title_screen_bkg");
	for(int i=0; i<9; i++)
		box[i] = GetPointer(("box_0"+stringx(i+1)).data());

	mc->Init();
}

void TitleFrontEnd::Select(int n)
{
	if (active)
	{
		active->Select(active->highlighted ? active->highlighted->entry_num : 0);
	}
	else
	{
		if (frontendmanager.fe_done_loading)
			MakeActive(mc);
	}
}

void TitleFrontEnd::SetSystem(FEMenuSystem *s)
{
	system = s;
	mc->SetSystem(s);
}

/************************ HelpbarFE ************************************/

HelpbarFE::HelpbarFE(FEManager* man, stringx p, stringx pf_name)
{
	cons(man, p, pf_name);

	float sc_sm = 0.8f; //0.45f;
//	color32 color = color32(153, 163, 171, 255);
	color32 color = manager->white;

	// add this to globaltext
	default_text[ARROW_H] = ksGlobalTextArray[GT_FE_MENU_MOVE];
	default_text[ARROW_V] = ksGlobalTextArray[GT_FE_MENU_MOVE];
	default_text[ARROW_BOTH] = ksGlobalTextArray[GT_FE_MENU_MOVE];
	default_text[CROSS] = ksGlobalTextArray[GT_FE_MENU_SELECT];
	default_text[CIRCLE] = ksGlobalTextArray[GT_FE_MENU_CIRCLE];
	default_text[TRIANGLE] = ksGlobalTextArray[GT_FE_MENU_BACK];
	default_text[SQUARE] = ksGlobalTextArray[GT_FE_MENU_SQUARE];

	for(int i=0; i<BTN_NUM; i++)
		help_text[i] = NEW TextString(&manager->font_hand, default_text[i], 0, 0, 0, sc_sm, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, color);
}

HelpbarFE::~HelpbarFE()
{
	for(int i=0; i<BTN_NUM; i++)
		delete help_text[i];
}

void HelpbarFE::LoadPanel(bool floating)
{
	FrontEnd::LoadPanel();
	SetPQIndices();
}

void HelpbarFE::SetPQIndices()
{
	buttons[ARROW_H] = GetPointer("hb_arrow_horizontal");
	buttons[ARROW_V] = GetPointer("hb_arrow_vertical");
	buttons[ARROW_BOTH] = GetPointer("hb_arrow_all");
	buttons[CROSS] = GetPointer("hb_btn_x");

	// swap circle & triangle buttons for xbox; only needs to be done once
#if defined( TARGET_XBOX )
	buttons[TRIANGLE] = GetPointer("hb_btn_circle");
	buttons[CIRCLE] = GetPointer("hb_btn_triangle");
	buttons[SQUARE] = GetPointer("hb_btn_square");
#elif defined( TARGET_GC )
	buttons[TRIANGLE] = GetPointer("hb_btn_square");
	buttons[CIRCLE] = GetPointer("hb_btn_circle");
	buttons[SQUARE] = GetPointer("hb_btn_triangle");
#else
	buttons[TRIANGLE] = GetPointer("hb_btn_triangle");
	buttons[CIRCLE] = GetPointer("hb_btn_circle");
	buttons[SQUARE] = GetPointer("hb_btn_square");
#endif
}

void HelpbarFE::Draw()
{
	if(disabled) return;
	FrontEnd::Draw();
	for(int i=0; i<BTN_NUM; i++)
		if(has_text[i]) help_text[i]->Draw();
}

void HelpbarFE::Reset()
{
	ChangeBtn(CROSS, true, default_text[CROSS]);
	ChangeBtn(TRIANGLE, true, default_text[TRIANGLE]);
	ChangeBtn(CIRCLE, false, "");
	ChangeBtn(SQUARE, false, "");
	ChangeBtn(ARROW_BOTH, true, default_text[ARROW_BOTH]);
	ChangeBtn(ARROW_H, false, "");
	ChangeBtn(ARROW_V, false, "");
	disabled = false;
}

void HelpbarFE::Reformat()
{
	int button_count = 0;
	for(int i=BTN_NUM-1; i>=0; i--)
	{
		if(has_text[i])
		{
			assert(button_count < 4);	// there shouldn't be more than 4 buttons
			help_text[i]->changePos(start_x+24-x_spacing*button_count, 419);
			buttons[i]->SetCenterPos(start_x-x_spacing*button_count, 420);
			button_count++;
		}
	}
}

void HelpbarFE::ChangeBtn(int index, bool add, stringx text)
{
	buttons[index]->TurnOn(add);
	if(add)
	{
		if(text == "")
			help_text[index]->changeText(default_text[index]);
		else help_text[index]->changeText(text);
	}
	else help_text[index]->changeText("");
	has_text[index] = add;
}

/************************ GraphicalMenuSystem **************************/

GraphicalMenuSystem::GraphicalMenuSystem(FEManager* man)
{
	back_movie_tex = NULL;
	back_movie_source = NULL;
	back_movie = NULL;
	back_movie_buffer = NULL;
	another_movie_tex = NULL;
	another_movie_source = NULL;
	another_movie = NULL;
	usercam = false;

	static bool first_time_through = true;
	if (!man->lfe) first_time_through = false;	// we skipped the front end the first time?  (dc 04/26/02)
	if (!man->tfe) 
	{
		frontendmanager.tfe = NEW TitleFrontEnd(&frontendmanager, "interface\\Legal\\", "Legal.PANEL");
		frontendmanager.tfe->Load();

	}
	if(first_time_through)
		cons(OptionsMenu, man, &man->font_bold);
	else cons(OptionsMenu-1, man, &man->font_bold);

	fedbm_up = false;
	fedbm = NEW FEDebugMenu(this, man);
	fedbm->AddAll();

	restartMovie();

	frontendmanager.map_loading_screen = false;
	Add(NEW MainFrontEnd(this, man, "levels\\frontend\\overlays\\", "main_menu.PANEL"));
	Add(NEW ExtrasFrontEnd(this, man, "levels\\frontend\\overlays\\", "extras.PANEL"));
	Add(NEW CreditsFrontEnd(this, man, "levels\\frontend\\overlays\\", "credit_screen.PANEL"));
	Add(NEW HighScoreFrontEnd(this, man, "levels\\frontend\\overlays\\", "high_score.PANEL", false));
	Add(NEW SurferFrontEnd(this, man, "levels\\frontend\\overlays\\", "surfer_select.PANEL"));
	Add(man->map);
	Add(NEW SaveLoadFrontEnd(this, man, "levels\\frontend\\overlays\\", "save_load.PANEL"));
	Add(NEW TrickBookFrontEnd(this, man, "levels\\frontend\\overlays\\", "trickbook.PANEL"));
	Add(NEW BoardFrontEnd(this, man, "levels\\frontend\\overlays\\", "board_select.PANEL"));
	Add(NEW AccompFrontEnd(this, man, "levels\\frontend\\overlays\\", "scrapbook.PANEL"));
	Add(NEW LogbookFrontEnd(this, man, "levels\\frontend\\overlays\\", ""));
	Add(man->tfe);

	((LegalFrontEnd*) menus[TitleMenu])->SetSystem(this);
	if(first_time_through)
	{
		Add(man->lfe);
	}
	LoadedAll = false;
	for(int i=0; i<OptionsMenu+1; i++)
		is_loaded[i] = false;
	soundStarted = false;
	InitAll();

	skip_flyin = false;
	first_screen_sub = -1;

	if(first_time_through)
	{
		((LegalFrontEnd*) menus[Legal])->SetSystem(this);
		
		MakeActive(TitleMenu);
		first_time_through = false;
		first_screen = MainMenu;
		prepare_to_make_active = false;
	}
	else
	{
		// determine first screen to go to after returning to front end
		if(manager->extras_movie)
		{
			first_screen = ExtrasMenu;
			manager->extras_movie = false;
			skip_flyin = true;
		}
		else if(!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_E3_BUILD) && manager->new_photo)
		{
			manager->new_photo = false;
			first_screen = AccompMenu;
			skip_flyin = true;
		}
		else if(dmm.ReturnFromDemoToTitle())
			first_screen = TitleMenu;
		else if(dmm.ReturnFromDemoToMain())
		{
			first_screen = MainMenu;
			MusicMan::inst()->pause();
		}
		else if(g_game_ptr->get_game_mode() == GAME_MODE_CAREER)
		{
			first_screen = SurferMenu;
			first_screen_sub = SurferFrontEnd::ACT_SURFER;
			skip_flyin = true;
		}
		else
		{
			first_screen = MainMenu;
			skip_flyin = true;
		}

		prepare_to_make_active = true;
	}

	beach = 0;
	cheat_index = 0;
	cheat_release = false;
	exiting = false;
	play_fe_movie_on_exit = false;
	dmm.wasInDemo = false;
	multiplayer = false;
	multi_1 = true;
	// this is to set the viewports correctly, if we just exited multiplayer
	g_game_ptr->set_game_mode(GAME_MODE_CAREER);
	
	fedb_draw_overlays = true;
}

GraphicalMenuSystem::~GraphicalMenuSystem()
{
	killMovie();
	delete fedbm;

	// delete all menus except for the map menu
	for(int i=0; i<size; i++)
		if(i != BeachMenu) 
		{
			delete menus[i];
			menus[i] = NULL;
		}
}

void GraphicalMenuSystem::LoadAll()
{
	// This is taking long enough that we turn off our 
	// performance counters
	// This macro will compile on all platforms
	if (!LoadedAll)
	{
		STOP_PS2_PC;

		manager->em->LoadAll();

		for(int i=0; i<OptionsMenu; i++)
			if(i != Legal && i != TitleMenu && i != BeachMenu)
				menus[i]->Load();

		frontendmanager.fe_done_loading = true;

		// Turn on our performance counters
		// This macro will compile on all platforms
		START_PS2_PC;
		LoadedAll = true;
	}
}

#ifdef KS_NVL

static void *movie_output_func (struct nvlMovie* m, size_t* s, void* data)
{
  nglTexture* tex = (nglTexture*)data;

  assert (tex);

  *s = tex->Width * tex->Height << 2;

  return tex->Data;
}

#endif // KS_NVL

void GraphicalMenuSystem::restartMovie()
{
// DAJ debug
#if defined(TARGET_PS2)

#ifdef KS_NVL
  nvlStreamSystemInit( true );
  nvlInit();

  back_movie_buffer = malloc (MOVIE_BUFFER_SIZE);

  back_movie_source = nvlLoadMovieSource ("cdrom0:\\MOVIES\\SLAVA.IPU;1", MOVIE_BUFFER_SIZE, back_movie_buffer);
  if (!back_movie_source)
    back_movie_source = nvlLoadMovieSource ("host0:movies\\slava.ipu", MOVIE_BUFFER_SIZE, back_movie_buffer);

  if (back_movie_source)
  {
    back_movie = nvlAddMovie (back_movie_source);
    back_movie_tex = nglCreateTexture (NGLTF_32BIT, nvlMovieGetWidth (back_movie), nvlMovieGetHeight (back_movie));
    memset (back_movie_tex->Data, 0xff, back_movie_tex->Width * back_movie_tex->Height * sizeof(int));
    nvlPlayMovie (back_movie, movie_output_func, back_movie_tex);
  }

#ifdef ANOTHER_MOVIE
  another_movie_source = nvlLoadMovieSource ("cdrom0:\\MOVIES\\SLAVA.IPU;1");
  if (!another_movie_source)
    another_movie_source = nvlLoadMovieSource ("host0:movies\\slava.ipu");

  if (another_movie_source)
  {
    another_movie = nvlAddMovie (another_movie_source);
    another_movie_tex = nglCreateTexture (NGLTF_32BIT, nvlMovieGetWidth (another_movie), nvlMovieGetHeight (another_movie));
    memset (another_movie_tex->Data, 0xff, another_movie_tex->Width * another_movie_tex->Height * sizeof(int));
    nvlPlayMovie (another_movie, movie_output_func, another_movie_tex);
  }
#endif

#endif // KS_NVL

#endif // TARGET_XBOX
}

void GraphicalMenuSystem::killMovie()
{
// DAJ debug
#if defined(TARGET_PS2)

#ifdef KS_NVL
  free (back_movie_buffer);
  back_movie_buffer = NULL;

  if (back_movie_tex)
  {
    nglDestroyTexture (back_movie_tex);
    back_movie_tex = NULL;
  }

  if (back_movie)
  {
    nvlStopMovie (back_movie);
    back_movie = NULL;
  }

  if (back_movie_source)
  {
    nvlReleaseMovieSource (back_movie_source);
    back_movie_source = NULL;
  }

#ifdef ANOTHER_MOVIE
  if (another_movie_tex)
  {
    nglDestroyTexture (another_movie_tex);
    another_movie_tex = NULL;
  }

  if (another_movie)
  {
    nvlStopMovie (another_movie);
    another_movie = NULL;
  }

  if (another_movie_source)
  {
    nvlReleaseMovieSource (another_movie_source);
    another_movie_source = NULL;
  }
#endif

  nvlShutdown();
  nvlStreamSystemShutdown();
#endif

#endif // TARGET_XBOX
}

void GraphicalMenuSystem::UpdateInScene()
{
#if defined(DEBUG) && !defined(TARGET_GC)
	if (getButtonState(FEMENUCMD_R2))
		nglScreenShot();
#endif
	menus[active]->UpdateInScene();
	if(active != BeachMenu && active != Legal)
		menus[BeachMenu]->UpdateInScene();
}

void GraphicalMenuSystem::Update(time_value_t time_inc)
{
	if(prepare_to_make_active)
	{
		if(first_screen_sub != -1)
			MakeActive(first_screen, first_screen_sub);
		else MakeActive(first_screen);
		if(skip_flyin)
		{
			if(first_screen == MainMenu)
				manager->em->JumpTo(FEEntityManager::CAM_POS_WALL_1);
      else if(first_screen == ExtrasMenu)
				manager->em->JumpTo(FEEntityManager::CAM_POS_WALL_1);
			else if(first_screen == SurferMenu)
				manager->em->JumpTo(FEEntityManager::CAM_POS_WALL_2_IN);
			else if(first_screen == AccompMenu)
				manager->em->JumpTo(FEEntityManager::CAM_POS_WALL_2_OUT);
		}
		prepare_to_make_active = false;
	}

// DAJ debug
#if defined(TARGET_PS2)

  if(active != Legal)
  {
#ifdef KS_NVL

    if (back_movie && (nvlMovieGetFrame (back_movie) == nvlMovieGetFrameNum (back_movie) - 1))
    {
      nvlStopMovie (back_movie);
      nvlReleaseMovieSource (back_movie_source);

      back_movie_source = nvlLoadMovieSource ("cdrom0:\\MOVIES\\SLAVA.IPU;1", MOVIE_BUFFER_SIZE, back_movie_buffer);
      if (!back_movie_source)
        back_movie_source = nvlLoadMovieSource ("host0:movies\\slava.ipu", MOVIE_BUFFER_SIZE, back_movie_buffer);

      if (back_movie_source)
      {
        back_movie = nvlAddMovie (back_movie_source);
        nvlPlayMovie (back_movie, movie_output_func, back_movie_tex);
      }
    }

#ifdef ANOTHER_MOVIE
    if (another_movie && (nvlMovieGetFrame (another_movie) == nvlMovieGetFrameNum (another_movie) - 1))
    {
      nvlStopMovie (another_movie);
      nvlReleaseMovieSource (another_movie_source);

      another_movie_source = nvlLoadMovieSource ("cdrom0:\\MOVIES\\SLAVA.IPU;1");
      if (!another_movie_source)
        another_movie_source = nvlLoadMovieSource ("host0:movies\\slava.ipu");

      if (another_movie_source)
      {
        another_movie = nvlAddMovie (another_movie_source);
        nvlPlayMovie (another_movie, movie_output_func, another_movie_tex);
      }
    }
#endif

    nvlAdvance();
#endif // KS_NVL
  }
#endif // TARGET_XBOX

	FEMenuSystem::Update(time_inc);

//#if !defined(LEAVE_BEHIND)
//  if((active == MainMenu) && (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_DEMO_MODE)))
//    dmm.tick(time_inc);
//#endif

	if (!soundStarted && active != TitleMenu)
	{

		if(!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
		{
				SSEventId s = SoundScriptManager::inst()->playEvent(SS_FE_WATERLAP);
				nslSoundId snd = SoundScriptManager::inst()->getSoundId(s);
				if (nslGetSoundStatus(snd) != NSL_SOUNDSTATUS_INVALID)
					nslSetSoundEmitter(frontendmanager.em->behindTheCamera, snd);	
				SoundScriptManager::inst()->playEvent(SS_BOATCUT);
				MusicMan::inst()->playNext();
				soundStarted = true;
		}
	}
	check_for_cheats (time_inc);

	if(exiting)
	{
		manager->map->PrepareForLoading();
		app::inst()->get_game()->unpause();

    if(!play_fe_movie_on_exit)
    {
		  app::inst()->get_game()->go_next_state();
		  Exit();
		  g_game_ptr->set_render_state(game::GAME_RENDER_LOADING_LEVEL);
    }
    else
    {
		  app::inst()->get_game()->pop_process();
		  app::inst()->get_game()->push_process(play_movie_process);
		  Exit();
    }
		stash::close_stash(STASH_COMMON);
		stash::free_stored(STASH_COMMON);
	}
}

void GraphicalMenuSystem::DrawMovie()
{
//#ifndef BETH_CAMERA_ANIM
//	nglListAddQuad(&background_quad);
//#endif
}

// stuff to draw on top of 3D stuff
void GraphicalMenuSystem::DrawTop()
{
	if(fedb_draw_overlays)
	{
		if(active) menus[active]->DrawTop();

#ifdef ANOTHER_MOVIE
		nglInitQuad(&another_quad);
		nglSetQuadRect(&another_quad, 0, 0, 100, 100);

		if (another_movie_tex)
		{
			nglSetQuadMapFlags(&another_quad, NGLMAP_CLAMP_U | NGLMAP_CLAMP_V | NGLMAP_BILINEAR_FILTER);
			nglSetQuadTex(&another_quad, another_movie_tex);
  			nglListAddQuad(&another_quad);
		}
#endif
	}
}

void GraphicalMenuSystem::Draw()
{
/*	static int count = 0;
	count++;
	if(count == 50)
	{
		mem_dump_heap();
	}
*/
	if(fedbm_up) fedbm->Draw();
	if(fedb_draw_overlays && menus[active])
		FEMenuSystem::Draw();
	if(manager->em->OKtoDrawHelpbar())
		manager->helpbar->Draw();
}

void GraphicalMenuSystem::Exit()
{
#ifdef TARGET_PS2	
	if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
	{
		extern float ps2MovieVolume;
		ps2MovieVolume = nslGetMasterVolume() * nslGetVolume(NSL_SOURCETYPE_MOVIE);
	}
#endif
  //Turn off the performance counter
  STOP_PS2_PC;
	g_game_ptr->set_game_mode(manager->tmp_game_mode);
	manager->ReleaseFE();
  //Turn on the performance counter
  START_PS2_PC;

}


bool GraphicalMenuSystem::get_one_button_down (control_id_t& btn) const
{
  input_mgr* inputmgr = input_mgr::inst();
  btn = INVALID_CONTROL_ID;

  for (int i = PSX_X; i <= PSX_SELECT; i++)
  {
    if (inputmgr->get_control_state( ANY_LOCAL_JOYSTICK, i) != 0.0f)
    {
      if (btn != INVALID_CONTROL_ID)
        return false;
      else
        btn = i;
    }
  }

  return true;
}

#define CHEAT_SPEED 1

void GraphicalMenuSystem::check_for_cheats (float time_inc)
{
  control_id_t btn;
  int i, j;

  struct
  {
    int size;
    control_id_t buttons[MAX_CHEAT_SIZE];
  }
  cheats[] =
  {
    { 4, { PSX_X, PSX_CIRCLE, PSX_TRIANGLE, PSX_SQUARE } },
    { 4, { PSX_SQUARE, PSX_TRIANGLE, PSX_CIRCLE, PSX_X } }
  };
  const int num_cheats = sizeof (cheats) / sizeof (cheats[0]);

  if (get_one_button_down (btn))
  {
    if (btn == INVALID_CONTROL_ID)
    {
      if (cheat_release)
      {
        cheat_timer = 0;
        cheat_release = false;
      }
      else
      {
        cheat_timer += time_inc;

        if (cheat_timer > CHEAT_SPEED)
          cheat_index = 0; // user is too slow, reset
      }
    }
    else
    {
      if (!cheat_release)
      {
        cheat_timer = 0;
        cheat_release = true;

        cheat_buffer[cheat_index] = btn;
        cheat_index++;
      }
      else
      {
        cheat_timer += time_inc;

        if (cheat_timer > CHEAT_SPEED)
          cheat_index = 0; // user is too slow, reset
      }
    }
  }
  else
  {
    cheat_timer += time_inc;

    if (cheat_timer > CHEAT_SPEED)
      cheat_index = 0; // user is too slow, reset
  }

  for (;;)
  {
    bool valid = false;

    for (i = 0; i < num_cheats; i++)
    {
      int count = min (cheats[i].size, cheat_index);

      for (j = 0; j < count; j++)
      {
        if (cheats[i].buttons[j] == cheat_buffer[j])
        {
          if (j == cheat_index - 1)
            valid = true;
        }
        else
          break;
      }

      if (j == cheats[i].size)
      {
        nglPrintf ("You suck and need to cheat ? Because of that the game is now much harder !\n");
        cheat_index = 0;
        break;
      }
    }

    if (valid || cheat_index == 0)
      break;

	if (cheat_index < 0 || cheat_index >= MAX_CHEAT_SIZE)
	{
		cheat_index = 0;
		break;
	}

    // remove first button and try again
    for (i = 0; i < MAX_CHEAT_SIZE - 1; i++)
      cheat_buffer[i] = cheat_buffer[i+1];
    cheat_index--;
  }
}

void GraphicalMenuSystem::OnButtonPress(int button, int controller)
{
	if(os_developer_options::inst()->is_flagged(os_developer_options::FLAG_DEBUG_MENUS) && button == FEMENUCMD_SQUARE)
		fedbm_up = !fedbm_up;
	else if(!fedbm_up)
	{
		if(manager->em->InFlyin())
		{
			int jump_to = -1;
			if(active == MainMenu) jump_to = FEEntityManager::CAM_POS_WALL_1;
			if(active == BeachMenu) jump_to = FEEntityManager::CAM_POS_WALL_3_MAP;
			if(active == AccompMenu) jump_to = FEEntityManager::CAM_POS_WALL_1;
			if(jump_to != -1) manager->em->JumpTo(jump_to);
		}
		if(!usercam) FEMenuSystem::OnButtonPress(button, controller);
	}
	else
	{
		switch(button)
		{
		case FEMENUCMD_UP      : fedbm->OnUp(controller); break;
		case FEMENUCMD_DOWN    : fedbm->OnDown(controller); break;
		case FEMENUCMD_LEFT    : fedbm->OnLeft(controller); break;
		case FEMENUCMD_RIGHT   : fedbm->OnRight(controller); break;
		case FEMENUCMD_CROSS   : fedbm->OnCross(controller); break;
		case FEMENUCMD_TRIANGLE: fedbm->OnTriangle(controller); break;
		case FEMENUCMD_SQUARE  : fedbm->OnSquare(controller); break;
		case FEMENUCMD_CIRCLE  : fedbm->OnCircle(controller); break;
		}
	}
}






