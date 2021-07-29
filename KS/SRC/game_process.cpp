////////////////////////////////////////////////////////////////////////////////

// game_process.cpp
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.

////////////////////////////////////////////////////////////////////////////////
#include "global.h"

#include "commands.h"
#include "console.h"
#include "game.h"
#include "game_process.h"
#include "gamefile.h"
#include "hwmovieplayer.h"
#include "inputmgr.h"
#include "msgboard.h"
#include "osdevopts.h"
#include "wds.h"
#include "console.h"
#include "ksreplay.h"
#include "timer.h"
#include "ksfx.h"	// For ks_fx_update()
/*
#include "refract.h"	// For REFRACT_Update()
*/

#include "profiler.h"

#if defined(TARGET_XBOX)
#include "entity_maker.h"
#include "MusicMan.h"
#endif /* TARGET_XBOX JIV DEBUG */

bool played_movies = false;

////////////////////////////////////////////////////////////////
/// GAME PROCESS AND STATE FUNCTIONS ///////////////////////////
////////////////////////////////////////////////////////////////
bool g_noPlayerKilled=false;
// define the game flows
const game_state_e startup_flow[] =
{
  GSTATE_PLAY_INTRO_MOVIES,
  GSTATE_POP_PROCESS
};

// For PS2 and GC, beach movie must come before the level is loaded
#if (defined TARGET_PS2) || (defined TARGET_GC)
const game_state_e main_flow[] =
{
  GSTATE_FRONT_END,
  GSTATE_PLAY_MOVIE,
  GSTATE_LOAD_LEVEL,
  GSTATE_RUNNING,
  GSTATE_POP_PROCESS
};
#else
const game_state_e main_flow[] =
{
  GSTATE_FRONT_END,
  GSTATE_LOAD_LEVEL,
  GSTATE_PLAY_MOVIE,
  GSTATE_RUNNING,
  GSTATE_POP_PROCESS
};
#endif

const game_state_e play_movie_flow[] =
{
  GSTATE_PLAY_FE_MOVIE,
  GSTATE_POP_PROCESS
};

const game_state_e pause_flow[] =
{
  GSTATE_PAUSED
};

game_process startup_process( "startup", startup_flow, sizeof(startup_flow) / sizeof(game_state_e) );
game_process main_process( "main", main_flow, sizeof(main_flow) / sizeof(game_state_e) );
game_process play_movie_process( "play_movie", play_movie_flow, sizeof(play_movie_flow) / sizeof(game_state_e) );
game_process pause_process( "pause", pause_flow, sizeof(pause_flow) / sizeof(game_state_e) );



game_process::game_process()
{
  name = 0;
  flow = 0;
  index = 0;
  num_states = 0;
  timer = 0;
  allow_override = false;
}

game_process::game_process( const char *_name, const game_state_e *_flow, int _num_states )
  : name(_name), flow(_flow), num_states(_num_states)
{
  index = 0;
  timer = 0.0f;
  allow_override = false;
}

game_process::~game_process()
{
}

void game_process::go_next_state()
{
  ++index;
  assert( index < num_states );
}

void game_process::reset_index()
{
  index = 0;
}

void game_process::set_index(int i)
{
	index = i;
	assert(index < num_states);
}


////////////////////////////////////////////////////////////////

void game::push_process( game_process &process )
{
  process_stack.push_front( process );
  process_stack.front().reset_index();
  process_stack.front().set_timer( 0.0f );
}

void game::pop_process()
{
  assert( process_stack.size() != 0 );
  process_stack.pop_front();
}

int game::get_cur_state() const
{
  assert( process_stack.size() != 0 );
  return process_stack.front().get_cur_state();
}

void game::go_next_state()
{
  assert( process_stack.size() != 0 );
  process_stack.front().go_next_state();
}

void game::reset_index()
{
  assert( process_stack.size() != 0 );
  process_stack.front().reset_index();
}


////////////////////////////////////////////////////////////////
/// FRAME ADVANCES FOR VARIOUS GAME STATES /////////////////////
////////////////////////////////////////////////////////////////

extern char g_scene_name[256];
extern char g_movie_name[256];
extern char g_console_command[256];

void game::advance_state_startup( time_value_t time_inc )
{
	stringx override_filename (os_developer_options::inst()->get_string(os_developer_options::STRING_SCENE_NAME));
	bool ignore_front_end=false;
	if (stricmp(override_filename.c_str(),"ASK") )
	{
		ignore_front_end=true;
	}
	STOP_PS2_PC;
	mem_create_heap(SURFER_HEAP, (int)(SURFER_STASH_MAX_SIZE_MEGS*1024*1024));
	soft_reset_process();

	mem_leak_prep();


	START_PS2_PC;
  if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_MOVIES))
  {
#if defined(TARGET_XBOX)
    g_game_ptr->play_movie("ACTIVISN",true);
    g_game_ptr->play_movie("TREYARCH",true);
    g_game_ptr->play_movie("KSINTRO", true);
    g_game_ptr->play_movie("BOAT",true);
		extern bool played_movies; // declared in game_process.cpp
    played_movies = true;
#elif defined(TARGET_GC)
		// FIXME: why is this here if it's also in gc_movieplayer.cpp?
		memcpy(&g_career->cfg, StoredConfigData::inst()->getGameConfig(), sizeof(ksConfigData));
		nslShutdown( );

		play_movie( "ACTIVISN", true );
    play_movie( "TREYARCH", true );
    play_movie( "KSINTRO", true );
    play_movie( "BOAT", true );
    played_movies = true;
#elif defined TARGET_PS2
    bool skip_all;

#ifdef TV_PAL
    sceGsResetGraph( 0, SCE_GS_INTERLACE, SCE_GS_PAL, SCE_GS_FRAME );

    skip_all = play_movie("ACTIVISN", true, 640, 512);
    if(!skip_all) skip_all = play_movie("TREYARCH", true, 640, 512);
    if(!skip_all) skip_all = play_movie("KSINTRO", true, 640, 512);
    if(!skip_all) skip_all = play_movie("BOAT", true, 512, 512);
    //play_movie("MAPROLL", true, 640, 480);
    played_movies = true;
#else
    sceGsResetGraph( 0, SCE_GS_INTERLACE, SCE_GS_NTSC, SCE_GS_FRAME );

    skip_all = play_movie("ACTIVISN", true, 640, 448);
    if(!skip_all) skip_all = play_movie("TREYARCH", true, 640, 480);
    if(!skip_all) skip_all = play_movie("KSINTRO", true, 640, 448);
    if(!skip_all) skip_all = play_movie("BOAT", true, 512, 448);
    //play_movie("MAPROLL", true, 640, 480);
    played_movies = true;
#endif
    KSNGL_ReInit();
    nglResetDisplay();
	if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
		if (!nslInit())
		{
#if defined(TARGET_XBOX) && defined(BUILD_FINAL)
			// Xbox requires a message on all disk read failures.  (dc 07/11/02)
			disk_read_error();
#endif
		}

#endif
  }

	g_random_ptr->srand();
	g_random_r_ptr->srand();
	anim_id_manager::inst()->purge();
	if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
	{
		// Start up audio
#ifdef TARGET_XBOX
		nslSetRootDir("D:\\");
#endif
		if (!nslInit())
		{
#if defined(TARGET_XBOX) && defined(BUILD_FINAL)
			// Xbox requires a message on all disk read failures.  (dc 07/11/02)
			disk_read_error();
#endif
		}
#ifdef TARGET_XBOX

		nslSetRootDir("D:\\SOUNDS");
		DWORD flags = XGetAudioFlags();
		switch (flags)
		{
			case XC_AUDIO_FLAGS_STEREO:   nslSetSpeakerMode(NSL_SPEAKER_STEREO); break;
			case XC_AUDIO_FLAGS_MONO:     nslSetSpeakerMode(NSL_SPEAKER_MONO);   break;
			case XC_AUDIO_FLAGS_SURROUND: nslSetSpeakerMode(NSL_SPEAKER_PROLOGIC);   break;
		}
#else
		nslSetRootDir("SOUNDS");
#endif

#if defined(DEBUG) && defined(TARGET_PS2) && !defined(PROFILING_ON)
		nslSetHostStreamingPS2(true);
#endif
		nslReset("FRONTEND", NSL_LANGUAGE_ENGLISH);
		SoundScriptManager::inst()->init();
		SOUNDDATA_Load();

	}
	if (!ignore_front_end)
	{

		for (int i=0; i < 4; i++)
		{
			nglListInit();
			render_title_screen();
			nglListSend(true);
		}
		nglSetClearColor(0, 0, 0, 0.0);

		STOP_PS2_PC;
		if (stash::open_stash("frontend.st2", STASH_COMMON) == false)
		{
			debug_print("could not open front end stash file frontend.st2\n");
		}
		START_PS2_PC;

		if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
		{
			VoiceOvers.init();
			MusicMan::inst()->loadSources();
			//MusicMan::inst()->playNext();

		}
		FEInit();
		dmm.Init();
		set_render_state(GAME_RENDER_DRAW_FE);
		frontendmanager.gms->MakeActive(GraphicalMenuSystem::TitleMenu);
	}
	else
	{
		nslShutdown();
		set_render_state(GAME_RENDER_LOADING_LEVEL);
	}

}

void game::advance_state_play_movie( time_value_t time_inc )
{
  if(get_game_mode() == GAME_MODE_CAREER && !dmm.inDemoMode())
  {
	  if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_MOVIES))
	  {
		  if (!(os_developer_options::inst()->is_flagged(os_developer_options::FLAG_E3_BUILD)))
		  {
			  int location, surfer;

			  location = BeachDataArray[g_game_ptr->get_beach_id()].map_location;
			  surfer = g_game_ptr->GetSurferIdx(0);
        globalCareerData.unlockLocationMovie(BeachDataArray[g_game_ptr->get_beach_id()].map_location);
			  if(g_career->locations[location].CheckShowMovie())
			  {
				  debug_print("Playing movie for location #%d\n", location);
          stringx moviename = stringx("BEACHES\\") + g_game_ptr->get_beach_location_name().substr(0, 8);

#if defined(TARGET_PS2)
          #ifdef TV_PAL
          sceGsResetGraph( 0, SCE_GS_INTERLACE, SCE_GS_PAL, SCE_GS_FRAME );
          g_game_ptr->play_movie(moviename.c_str(), true, 640, 512);
          #else
          sceGsResetGraph( 0, SCE_GS_INTERLACE, SCE_GS_NTSC, SCE_GS_FRAME );
          g_game_ptr->play_movie(moviename.c_str(), true, 640, 448);
          #endif
          KSNGL_ReInit();
          nglResetDisplay();
#else
          g_game_ptr->play_movie( moviename.c_str( ), true );
#endif
			  }
		  }
	  }
  }

	go_next_state();
}

void game::advance_state_play_fe_movie( time_value_t time_inc )
{
#if !defined(TARGET_XBOX)
  frontendmanager.fe_init = false;
	//stash::close_stash(STASH_COMMON);
	//stash::free_stored(STASH_COMMON);

#if defined(TARGET_PS2)

#ifdef TV_PAL
  sceGsResetGraph( 0, SCE_GS_INTERLACE, SCE_GS_PAL, SCE_GS_FRAME );
  g_game_ptr->play_movie(movie_name.c_str(), true, 640, 512);
#else
  sceGsResetGraph( 0, SCE_GS_INTERLACE, SCE_GS_NTSC, SCE_GS_FRAME );
  g_game_ptr->play_movie(movie_name.c_str(), true, 640, 448);
#endif
  KSNGL_ReInit();
  nglResetDisplay();
  if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
	  if (!nslInit())
	  {
#if defined(TARGET_XBOX) && defined(BUILD_FINAL)
		  // Xbox requires a message on all disk read failures.  (dc 07/11/02)
		  disk_read_error();
#endif
	  }

#elif defined(TARGET_GC)
  g_game_ptr->play_movie( movie_name.c_str( ), true );
#endif
  start_drawing_map = true;

	frontendmanager.ReloadTextures();
	frontendmanager.map->HideAllDots();
	frontendmanager.map->PutUpLoadingScreen();
	for (int i=0; i < 4; i++)
	{
		nglListInit();
		render_map_screen();
		nglListSend(true);
	}
	start_drawing_map = false;
	nglReleaseAllTextures();
  soft_reset_process();
	set_render_state(GAME_RENDER_LOADING_LEVEL);
  start_drawing_map = true;
  frontendmanager.map->PickDemo(0);
	
  //frontendmanager.map->PickFE();
	//((BeachFrontEnd *)frontendmanager.gms->menus[GraphicalMenuSystem::BeachMenu])->Pick();
	//frontendmanager.OnLevelEnding();
	//frontendmanager.in_game_map_up = true;
	//frontendmanager.IGO->ShowMenuBackground(false);

#endif
}

void game::advance_state_load_level( time_value_t time_inc )
{
  if(current_loading_state == LOADING_INITIAL)
  {

#if defined(TARGET_XBOX)
	  assert( time_inc > 0.0f );
#endif /* TARGET_XBOX JIV DEBUG */

	  mem_lock_malloc(false);

	  g_entity_maker->purge_entity_cache();
	  if(!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
	  {
		MusicMan::inst()->shutdown();
	  }
		//stash::close_stash(STASH_COMMON);
		//stash::free_stored(STASH_COMMON);

#ifdef USINGSTATICSTLALLOCATIONS
		app::cleanup_stl_memory_dregs();
		entity_anim::mem_cleanup();
		entity_anim_tree::mem_cleanup();
		po_anim::mem_cleanup();
		linear_anim<quaternion>::mem_cleanup();
		linear_anim<vector3d>::mem_cleanup();
		linear_anim<rational_t>::mem_cleanup();
#endif

		// Ryan is going to remove all this code, so the assert will no longer happen. (dc 03/28/02)
		mem_leak_test(true);
		mem_leak_prep();

	  //entity_manager::stl_prealloc();
		anim_id_manager::inst()->purge();
	  // play the intro movies - this hack is a lost battle (but I'm winning the war against the
	  // game_process system).
	  if ( !played_movies && !os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_MOVIES) )
	  {
			/*
			// Nick doesn't want this anymore
		play_movie("ACTIVISN",true);
		play_movie("TREYARCH",true);
		//play_movie("SM_INTRO");
	  #ifdef TARGET_PS2
		nglFlip(); nglFlip(); nglRelockAllTextures();
	  #endif
		played_movies = true;
		*/
	  }
	  level_prefix = "levels\\"+stringx(g_scene_name);


  // reloads map & font textures, which should be in memory at all times
  frontendmanager.ReloadTextures();

 }


  //LoadingStatePrint();
  load_this_level();
  LoadingStateUpdate();

  if(current_loading_state >= LOADING_STATE_DONE)
  {
	  flag.game_paused = 0;

	  go_next_state();

#ifdef TARGET_PS2
	  // Lock frame rate to 30 to avoid jitter (dc 03/28/02)
	  if (os_developer_options::inst()->get_flag(os_developer_options::FLAG_E3_BUILD))
	  {
		  nglSetFrameLock(BeachDataArray[g_game_ptr->get_beach_id()].framelock);
	  }
#endif
  }
}

//--------------------------------------------------------------

void game::advance_state_paused( time_value_t time_inc )
{
#if defined(TARGET_XBOX)
  assert( time_inc > 0.0f );
#endif /* TARGET_XBOX JIV DEBUG */

	ksreplay.Tick(FALSE,time_inc);
	frame_advance_game_overlays( time_inc );
}

//--------------------------------------------------------------
void UNDERWATER_EntitiesTrackCamera( void );

void game::advance_state_running( time_value_t time_inc )
{
	bool first_frame = !level_is_loaded();	// query before value changes (dc 04/29/02)
	flag.level_is_loaded = true;

#if defined(TARGET_XBOX)
	assert( time_inc > 0.0f );
#endif /* TARGET_XBOX JIV DEBUG */

	if ( flag.i_lose
		|| ( g_world_ptr->get_hero_ptr(active_player)->is_dying()
		&& g_world_ptr->get_hero_ptr(active_player)->get_anim_tree(ANIM_PRIMARY)->is_finished()
		)
		)
	{
		// push_process( hero_dies_process );
		return;
	}

	if( flag.i_win || (os_developer_options::inst()->get_int(os_developer_options::INT_FORCE_WIN)) )
	{
		// push_process( complete_level_process );
		return;
	}

	if( flag.i_quit )
	{
		os_file::system_unlock();
		flag.i_quit = false;
		gamefile->init_cur_data();

		{
			unload_current_level();
			soft_reset_process();
		}

		return;
	}
	if ( flag.load_new_level )
	{
		os_file::system_unlock();
		flag.load_new_level = false;
		gamefile->init_cur_data();

		{
			unload_current_level();
			soft_reset_process();
		}

		return;
	}

	// ps2 console support (until we get a keyboard on the ps2
#if _CONSOLE_ENABLE
	if (g_console_command[0] != '\0')
	{
		g_console->processCommand(g_console_command, 1);
		g_console_command[0] = '\0';
	}
#endif

#if defined(TARGET_XBOX)
	assert( time_inc > 0.0f );
#endif /* TARGET_XBOX JIV DEBUG */

	// Note that game modes are updated even when the game is paused
	update_game_mode(time_inc);

	//BETH	if (!flag.stop_physics || flag.single_step)
	if((!flag.game_paused || flag.single_step) && !ksreplay.IsPlaying())
	{
		//if (TIMER_GetRemainingLevelSec() <= -5.0
		// This call is not in the right units (should be seconds),
		// and it is redundant with the one in app.cpp .  (dc 04/05/02)
		//		nglSetAnimTime(TIMER_GetTotalSec() / 60.f);
		START_PROF_TIMER( proftimer_adv_kelly_slater );
		ksreplay.Tick(TRUE,time_inc);

		// In single player modes, if player is wiping out then stop the visible clock.
		bool updateClock = true;
		if (num_active_players == 1 && the_world->get_ks_controller(active_player))
		{
			if (the_world->get_ks_controller(active_player)->get_super_state() == SUPER_STATE_WIPEOUT ||
				the_world->get_ks_controller(active_player)->get_current_state() == STATE_DUCKDIVE)
				updateClock = false;
		}
		
		TIMER_Tick(time_inc, updateClock);
		
		START_PROF_TIMER( proftimer_adv_wave );
		WAVE_Tick();
		STOP_PROF_TIMER( proftimer_adv_wave );
		START_PROF_TIMER( proftimer_adv_particles );
		if ( particle_enable )
		{
			ks_fx_update();
		}
		STOP_PROF_TIMER( proftimer_adv_particles );
		STOP_PROF_TIMER( proftimer_adv_kelly_slater );
		START_PROF_TIMER( proftimer_adv_world );

#if defined(TARGET_XBOX)
		assert( time_inc > 0.0f );
#endif /* TARGET_XBOX JIV DEBUG */
		the_world->frame_advance(time_inc);
		//UNDERWATER_EntitiesTrackCamera();

		STOP_PROF_TIMER( proftimer_adv_world );
		/*	Attempts at screen spray (dc 01/04/02)
		REFRACT_Update();
		*/
		flag.single_step = false;
	}
	else
	{
#if defined(TARGET_XBOX)
		assert( time_inc > 0.0f );
#endif /* TARGET_XBOX JIV DEBUG */

		TIMER_NoTick();
		ksreplay.Tick(FALSE,time_inc);
		vector<entity *>::const_iterator ei_end = the_world->get_entities().end();
		for ( vector<entity *>::const_iterator ei = the_world->get_entities().begin(); ei != ei_end; ++ei )
		{
			entity* e = *ei;
			if ( e )
			{
				e->update_abs_po();
				e->frame_done();
			}
		}

		the_world->usercam_frame_advance(time_inc);
		the_world->scene_analyzer_frame_advance(time_inc);
		//UNDERWATER_EntitiesTrackCamera();
	}

	mb->frame_advance(time_inc);
	current_mic->adjust_listener();
#ifdef GCCULL
	process_ambients( time_inc );
#endif
	START_PROF_TIMER( proftimer_adv_overlays );
	frame_advance_game_overlays( time_inc );
	STOP_PROF_TIMER( proftimer_adv_overlays );

	// Everything from here until a couple lines before the end of this function was moved
	// to here from game::frame_advance().  Should probably be done here instead,
	// since it was only done if is_level_loaded() anyhow. (dc 04/29/02)
	if (!level_is_loaded()) return;

	input_mgr* inputmgr = input_mgr::inst();
	int num_controllers = g_game_ptr->get_num_players() + g_game_ptr->get_num_ai_players();
	for (int n = 0; n < num_controllers; n++)
	{
		kellyslater_controller *ksctrl = g_world_ptr->get_ks_controller(n);

		//START_PROF_TIMER(proftimer_generic_0);
		if (!flag.game_paused || ksreplay.IsPlaying())
		{
			ksctrl->PerformIK();
		}
		//STOP_PROF_TIMER(proftimer_generic_0);

		ksctrl->UpdateHand();
	}

	START_PROF_TIMER( proftimer_adv_sound );
	if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
	{

		if (flag.level_is_loaded)
		{
			wSound.frameAdvance(time_inc);
			MusicMan::inst()->tick(true);
			SoundScriptManager::inst()->tick(time_inc);
		}
		nslFrameAdvance(time_inc);
	}

	STOP_PROF_TIMER( proftimer_adv_sound );


	{
		if ( inputmgr->get_control_delta( JOYSTICK_DEVICE, QUIT_GAME ) == AXIS_MAX )
		{
#if defined(TARGET_PC)
			// NOW WITH NEW QUIT-NO-MORE
			unload_current_level();
			return;
#endif //TARGET_PC
		}
		
#ifdef DEBUG
		if( inputmgr->get_control_delta( JOYSTICK_DEVICE, FIRST_PLANE_ADJUST ) == AXIS_MAX )
		{
			int dir = (inputmgr->get_control_state( JOYSTICK_DEVICE, PLANE_BOUNDS_MOD ) == AXIS_MAX)?-1:1;
			if ((dir==1 && FIRST_SHOW_PLANE<=LAST_SHOW_PLANE) ||
				(dir==-1 && FIRST_SHOW_PLANE>0) )
				FIRST_SHOW_PLANE += dir;
		}
		
		if( inputmgr->get_control_delta( JOYSTICK_DEVICE, LAST_PLANE_ADJUST ) == AXIS_MAX )
		{
			int dir = (inputmgr->get_control_state( JOYSTICK_DEVICE, PLANE_BOUNDS_MOD ) == AXIS_MAX)?1:-1;
			if ((dir==1 && LAST_SHOW_PLANE<GLOBAL_PMESH_COUNT-1) ||
				(dir==-1 && LAST_SHOW_PLANE>FIRST_SHOW_PLANE) )
				LAST_SHOW_PLANE += dir;
		}
#endif
		
		if( inputmgr->get_control_delta( JOYSTICK_DEVICE, SINGLE_STEP ) == AXIS_MAX &&
			inputmgr->get_control_state( JOYSTICK_DEVICE, PLANE_BOUNDS_MOD ) == 0 )
		{
			flag.single_step = true;
		}
		
		if( inputmgr->get_control_delta( JOYSTICK_DEVICE, SHOW_DEBUG_INFO ) == AXIS_MAX )
		{
			os_developer_options::inst()->toggle_flag(os_developer_options::FLAG_SHOW_DEBUG_INFO);
		}
		
#ifdef PROFILING_ON
		if ( g_show_profile_info )
		{
			if ( fabsf( inputmgr->get_control_state( JOYSTICK_DEVICE, SCROLL_PROFILE_INFO ) ) >= 0.5f )
				profiler::inst()->scroll( inputmgr->get_control_state( JOYSTICK_DEVICE, SCROLL_PROFILE_INFO ) * 2.0f );
			if ( inputmgr->get_control_delta( JOYSTICK_DEVICE, OPEN_PROFILE_ENTRY ) == AXIS_MAX )
				profiler::inst()->get_sel_entry()->toggle_open();
		}
		
		//    if ( inputmgr->get_control_delta( JOYSTICK_DEVICE, SHOW_PROFILE_INFO ) == AXIS_MAX )
		//    g_show_profile_info );
		
#ifndef BUILD_BOOTABLE
		if(inputmgr->get_control_delta( JOYSTICK_DEVICE,DUMP_FRAME_INFO) == AXIS_MAX)
			g_debug.dump_frame_info = true;
#endif
#endif
		
		//    #if CHEATS_FOR_TESTING
		do_the_cheat_codes(time_inc);
		//    #endif
	}
	
	START_PROF_TIMER( proftimer_camera );
	if(time_inc && (!flag.game_paused || flag.single_step))
	{
		// camera selection for scenes
		if ( the_world->is_marky_cam_enabled() )
		{
			set_current_camera(the_world->get_marky_cam_ptr());
			set_player_camera(0,the_world->get_marky_cam_ptr());
			current_view_camera->frame_advance( time_inc );
		}
		else
		{
			assert(time_inc>0 && time_inc<10.0f);
			if (is_splitscreen())
			{
				for(int i=0; i < num_active_players; i++)
				{
					if (first_frame) 
					{
						player_cam[i]->Reset();	// forget blends/filtering in progress (dc 04/29/02)
					}
					player_cam[i]->frame_advance(time_inc);
				}
			}
			else
			{
				if (first_frame) 
				{
					player_cam[active_player]->Reset();	// forget blends/filtering in progress (dc 04/29/02)
				}
				player_cam[active_player]->frame_advance(time_inc);
			}
		}
	}
	
	current_view_camera->adjust_geometry_pipe();
	
	if ( geometry_manager::inst()->is_scene_analyzer_enabled() )
	{
		camera * scene_analyzer_cam = find_camera(entity_id("SCENE_ANALYZER_CAM"));
		scene_analyzer_cam->adjust_geometry_pipe(true);
	}
	STOP_PROF_TIMER( proftimer_camera );
	
	if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
	{
		VoiceOvers.frameAdvance(time_inc);
		sfx.tick(time_inc);
	}
	rumbleMan.tick(time_inc);
	
#ifdef ENABLE_SCREEN_GRAB
	if ( inputmgr->get_control_state( JOYSTICK_DEVICE, DC_SCREEN_GRAB ) == AXIS_MAX )
	{
		static char* fname = "scrn0000.bmp";
		static int fnum = 0;
		sprintf( fname+4, "%04d", fnum++ );
		fname[8] = '.';
		SaveFrontBufferAsBMP( fname );
	}
#endif
	
	if (first_frame) 
	{
		// Needs to come at end, or animations won't frame advance.  Then the game will be paused with 
		// entities in a bind pose.  (dc 04/29/02)
		frontendmanager.OnLevelLoaded();

		if(num_players == 1 && !os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_REPLAY))
    {
      if(num_ai_players == 0)
			  ksreplay.Init(false);
      else
			  ksreplay.Init(true);
    }
		  
		mem_lock_malloc(true);
		LoadingStateReset();
		set_render_state(GAME_RENDER_GAME_LEVEL);
	}

	int end_level_count = 0;
	int num_player_controllers = 0;
	for (int n = 0; n < num_controllers; n++)
	{
		kellyslater_controller *ksctrl = g_world_ptr->get_ks_controller(n);
		if (!ksctrl->IsAIPlayer())
			num_player_controllers++;

		if (ksctrl->EndLevel())
			end_level_count++;
	}

	if (end_level_count == num_player_controllers) // a player did not press buttons for a while
	{
		if (!g_noPlayerKilled)
		{
			frontendmanager.pms->ActivateAndExit();
			
			//app::inst()->get_game()->set_render_state(GAME_RENDER_LOADING_LEVEL);
			g_noPlayerKilled = true;
		}
	}
}

void game::soft_reset_process()
{
  process_stack.resize(0);
  push_process( main_process );
}
