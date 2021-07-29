////////////////////////////////////////////////////////////////////////////////

// app.cpp
// Copyright (C) 1999-2000 Treyarch, L.L.C.  ALL RIGHTS RESERVED

////////////////////////////////////////////////////////////////////////////////
#include "global.h"

#include "app.h"
#include "bp_tree.h"
#include "geomgr.h"
#include "hwrasterize.h"
#include "fogmgr.h"
#include "lightmgr.h"
#include "pmesh.h"
#include "element.h"
#include "trigger.h"
#include "hwaudio.h"
#include "dropshadow.h"
#include "hwmovieplayer.h"
#include "beam.h"
#include "iri.h" // for viri member
#include "maxiri.h" // for viri member
#include "SoundScript.h"

#ifdef TARGET_PC
//#include "winapp.h"
//#include "eventmgr.h"
//#include "di_input.h"
#elif defined(TARGET_MKS)
//#include "osinput.h"
#elif defined(TARGET_PS2)
#include "hwosps2/ps2_input.h"
bool g_master_clock_is_up = false;
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#include "ngl_ps2.h"
#elif defined(TARGET_XBOX)
bool g_master_clock_is_up = false;
#include "hwosxb/xb_input.h"
#elif defined(TARGET_GC)
bool g_master_clock_is_up = false;
#include "hwosgc/gc_input.h"
#else
#include "hwosnull/null_input.h"
#endif

#include "project.h"

#include "item.h"
#include "game.h"
#include "po.h"
#include "osdevopts.h"
#include "oserrmsg.h"
#include "profiler.h"
#include "pmesh.h"
#include "inputmgr.h"

#include "msgboard.h"
#include "osalloc.h"
// BIGCULL #include "scanner.h"

#include "terrain.h"
#include "vertwork.h"
#include "project.h"

// BIGCULL #include "switch_obj.h"

#include "script_lib_controller.h"
#include "script_lib_mfg.h"
#include "file_manager.h"

#include "particlecleaner.h" // for cleanup purposes
#include "trick_system.h"

#include "ksdbmenu.h"

DEFINE_SINGLETON(app)


/////////////////////////////////////////////////////////////////////////////
// Event signals
/////////////////////////////////////////////////////////////////////////////

// This static function must be implemented by every class which can generate
// signals, and is called once only by the application for each such class;
// the effect is to register the name and local id of each signal with the
// signal_manager.  This call must be performed before any signal objects are
// actually created for this class (via signaller::signal_ptr(); see signal.h).
void app::register_signals()
{
#define MAC(label,str)  signal_manager::inst()->insert( str, label );
#include "global_signals.h"
#undef MAC
}

static const char* signal_names[] =
{
#define MAC(label,str)  str,
#include "global_signals.h"
#undef MAC
};

unsigned short app::get_signal_id( const char *name )
{
	int idx;

	for( idx = 0; idx < (int)(sizeof(signal_names)/sizeof(char*)); ++idx )
	{
		int offset = strlen(signal_names[idx])-strlen(name);

		if( offset > (int)strlen( signal_names[idx] ) )
			continue;

		if( !strcmp(name,&signal_names[idx][offset]) )
			return( idx + PARENT_SYNC_DUMMY + 1 );
	}

	// not found
	return signaller::get_signal_id( name );
}

// This virtual function, used only for debugging purposes, returns the
// name of the given local signal
const char* app::get_signal_name( unsigned idx ) const
{
	assert( idx < N_SIGNALS );
	if ( idx <= (unsigned)PARENT_SYNC_DUMMY )
		return signaller::get_signal_name( idx );
	else
		return signal_names[idx-PARENT_SYNC_DUMMY-1];
}

//-----------------------------------------------------------------------------
// Declare the application globals
//-----------------------------------------------------------------------------

bp_tree<partition3,vector3d>::plane_list_t bp_tree<partition3,vector3d>::plane_list(100);
bp_tree<partition3,vector3d>::plane_list_t bp_tree<partition3,vector3d>::show_plane_list(100);
bp_tree<partition3,vector3d>::hit_list_t bp_tree<partition3,vector3d>::hit_list(100);
bp_tree<partition3,vector3d>::hit_dir_list_t bp_tree<partition3,vector3d>::hit_dir_list(100);

////////////////////////////////////////////////////////////////////////////////
// Hooks from WinMain
////////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
game *g_game_ptr = NULL;

void LIGHTMGR_StaticInitLightSet( void );
void CAMERA_TOOL_StaticInit( void );
void MENUDRAW_StaticInit( void );
void MENU_SCORING_SYSTEM_StaticInit( void );
void CAREER_StaticInit( void );
void KSDBMENU_StaticInit( void );
void WAVE_StaticInit( void );
#ifndef TARGET_GC
void WAVERENDER_StaticInit( void );
#endif
void WAVETEX_StaticInit( void );
void TRAIL_StaticInit( void );
void 	MENUSOUND_StaticInit( void );
void 	MENUCHEAT_StaticInit( void );
app::app()
{

	//stringx::init();

	// these are kludges to avoid calling NEW before main()
	LIGHTMGR_StaticInitLightSet();
	CAREER_StaticInit();
	TRAIL_StaticInit();

	CAMERA_TOOL_StaticInit();
	MENUDRAW_StaticInit();
	MENUSOUND_StaticInit();
	MENUCHEAT_StaticInit();
	MENU_SCORING_SYSTEM_StaticInit();
	WAVE_StaticInit();
	WAVETEX_StaticInit();
#ifndef TARGET_GC
	WAVERENDER_StaticInit();
#endif
	KSDBMENU_StaticInit();


	viri = NULL;
	g_particle_cleaner = NEW particle_cleaner;

	// instantiate singletons
#ifndef TARGET_GC  // gc instantiates this in main()
	error_context::create_inst();
#endif

	// set up the file system layer as early as possible
#ifdef USE_FILE_MANAGER
	file_manager::create_inst();
#ifdef TARGET_PS2
	file_manager::inst()->set_host_prefix(nglHostPrefix);
#else
	file_manager::inst()->set_root_dir(os_developer_options::inst()->get_string(os_developer_options::STRING_ROOT_DIR).c_str());
#endif
	file_manager::inst()->set_search_paths("sm_paths.txt");
#endif

	slc_manager::create_inst();
#ifdef KSCULL
	register_script_libs();
#endif

	//P  membudget_t::create_inst();

	geometry_manager::create_inst();
	hw_rasta::create_inst();

	fog_manager::create_inst();

	// not doing d3d specific stuff, like dinput_manager & hw_texture_manager
	initialize_lighting_matrices();

	initialize_mesh_stuff();
	set_viri(NEW instance_render_info[MAX_INSTANCES]);

	hw_texture_mgr::create_inst();

	signal_manager::create_inst();
	//anim_id_manager::create_inst();

	//  element_manager::create_inst();
	COLGEOM_stl_prealloc();
	ScoringManager::stl_prealloc();

	entity_manager::stl_prealloc();
	anim_id_manager::stl_prealloc();
	//entity_manager::create_inst();
  trigger_manager::create_inst();
	#ifdef USINGSTATICSTLALLOCATIONS
  entity_anim_tree::mem_init_func = &entity_anim_tree_stl_prealloc;
  entity_anim_tree::mem_free_func = &entity_anim_tree_stl_dealloc;
	#else
  entity_anim_tree_stl_prealloc();
  #endif

//P  storage_mgr::create_inst();

  #ifdef TARGET_PC
  dinput_mgr::create_inst();
  #elif defined(TARGET_MKS)
  sy_input_mgr::create_inst();
  #elif defined(TARGET_PS2)
  ps2_input_mgr::create_inst();
  #elif defined(TARGET_XBOX)
  xbox_input_mgr::create_inst();
  #elif defined(TARGET_GC)
  gc_input_mgr::create_inst();
  #else
  null_input_mgr::create_inst();
  #endif

  input_mgr::create_inst();
  #ifdef GCCULL
  sound_device::create_inst();
  #endif
//P  vr_dropshadow::create_inst();

  movieplayer::create_inst();
	SoundScriptManager::create_inst();
  //P   highlight::create_inst();

  // link script library class hierarchy
  slc_manager::inst()->link_hierarchy();

  // register event signals for all relevant classes
  entity::register_signals();
  beam::register_signals();
  trigger::register_signals();
  item::register_signals();
// BIGCULL  scanner::register_signals();
// BIGCULL  switch_obj::register_signals();
#ifdef KSCULL
	script_controller::register_signals();
	script_mfg::register_signals();
#endif

	// Set the global language here, before any text appears (dc 07/10/02)
	stringx str = os_developer_options::inst()->get_string(os_developer_options::STRING_LOCALE);
	if(str == "piglatin") ksGlobalTextLanguage = LANGUAGE_PIG_LATIN;
	else if(str == "french") ksGlobalTextLanguage = LANGUAGE_FRENCH;
	else if(str == "german") ksGlobalTextLanguage = LANGUAGE_GERMAN;
	else
	{
		// always default to english unless TV_PAL is defined
#ifndef TV_PAL
		ksGlobalTextLanguage = LANGUAGE_ENGLISH;

#else	// TV_PAL

#if defined(TARGET_PS2)
		int lang = sceScfGetLanguage();
		if(lang == SCE_FRENCH_LANGUAGE) ksGlobalTextLanguage = LANGUAGE_FRENCH;
		else if(lang == SCE_GERMAN_LANGUAGE) ksGlobalTextLanguage = LANGUAGE_GERMAN;
		else ksGlobalTextLanguage = LANGUAGE_ENGLISH;
#elif defined(TARGET_GC)
		int lang = OSGetLanguage( );

		switch( lang ) {		
		case OS_LANG_GERMAN:
			ksGlobalTextLanguage = LANGUAGE_GERMAN;
			break;
		case OS_LANG_FRENCH:
			ksGlobalTextLanguage = LANGUAGE_FRENCH;
			break;
		default:
			ksGlobalTextLanguage = LANGUAGE_ENGLISH;
			break;
		}

#else // TARGET_XBOX
		DWORD lang = XGetLanguage();
		switch(lang)
		{
		case XC_LANGUAGE_GERMAN:
			ksGlobalTextLanguage = LANGUAGE_GERMAN;
			break;
		case XC_LANGUAGE_FRENCH:
			ksGlobalTextLanguage = LANGUAGE_FRENCH;
			break;
		default:
			ksGlobalTextLanguage = LANGUAGE_ENGLISH;
			break;
		}
#endif
#endif	// TV_PAL
	}


	// had to move this after the language stuff, since the legal screen gets loaded 
	// somewhere in game()  -beth 7/14/02
	the_game = NEW game;
	g_game_ptr = the_game;
#ifdef TARGET_XBOX
	reboot_timer = 0.0f;
#endif

}

app::~app()
{
	delete the_game;
	the_game = g_game_ptr = NULL;

	// moved from a global into app for memory-leak removal purposes --GT 4/17/01
	if (viri != NULL)
		delete[] viri;

	cleanup();
}

void app::cleanup_stl_memory_dregs( void )
{
	//anim_id_manager::delete_inst();
	//entity_manager::delete_inst();
	//entity_manager::stl_prealloc();
	//anim_id_manager::stl_prealloc();
	anim_id_manager::inst()->stl_dealloc();
	//anim_id_manager::inst()->purge();
}



void app::cleanup() // this gets called before exit() when an error occurs
{
#if defined(TARGET_XBOX) || defined(TARGET_GC)
	movieplayer::delete_inst();
#endif

	//P  vr_dropshadow::delete_inst();
	//P   highlight::delete_inst();
#ifdef GCCULL
	if (sound_device::inst())
	{
		sound_device::inst()->clear(); // release all sounds
		sound_device::delete_inst();
	}
#endif
	input_mgr::delete_inst();

#ifdef TARGET_PC
	dinput_mgr::delete_inst();
#elif defined(TARGET_MKS)
	sy_input_mgr::delete_inst();
#elif defined (TARGET_PS2)
	ps2_input_mgr::delete_inst();
#elif defined (TARGET_XBOX)
	xbox_input_mgr::delete_inst();
#elif defined (TARGET_GC)
	gc_input_mgr::delete_inst();
#else
	null_input_mgr::delete_inst();
#endif

	//P  storage_mgr::delete_inst();

	trigger_manager::delete_inst();
	entity_manager::delete_inst();

	//  element_manager::delete_inst();

	anim_id_manager::delete_inst();
	signal_manager::delete_inst();

	hw_texture_mgr::delete_inst();
	fog_manager::delete_inst();
	hw_rasta::delete_inst();
	geometry_manager::delete_inst();
#ifdef PROFILING_ON
	// Profiler is created in w32_main.cpp / ps2main.cpp.
	profiler::delete_inst();
#endif

	slc_manager::delete_inst();
	//P  membudget_t::delete_inst();
#ifdef TARGET_PS2
	master_clock::delete_inst();
#endif

#ifdef USE_FILE_MANAGER
	file_manager::delete_inst();
#endif

	error_context::delete_inst();

	delete g_particle_cleaner;
}

void app::bomb()
{
	//app::cleanup();
	app::delete_inst();

#ifdef TARGET_PC
	windows_app::delete_inst();
#endif
	os_developer_options::delete_inst();
	exit(-1);
}


#ifndef BUILD_FINAL
static bool dump_heap=false;
static bool allocate_way_too_much_memory=false;
bool check_heap=false;
bool intentionalstackoverflow=false;

volatile int stackstomp=0;

void overflow_the_stack( int depth )
{
	int d=depth;
	if ( d>0 )
	{
		stackstomp=d;
		overflow_the_stack( d-1 );
	}
}


#endif

bool doshadowdrawing=true;

void app::tick()
{
	#ifndef BUILD_FINAL
	if ( dump_heap )
	{
		dump_heap=false;
		mem_dump_heap();
	}
	if ( check_heap )
	{
		//check_heap=false;
		mem_check_heap();
	}
	if ( allocate_way_too_much_memory )
	{
		allocate_way_too_much_memory=false;
		//void *memoryerror=
		malloc(256*1024*1024);
	}
	if ( intentionalstackoverflow )
	{
		intentionalstackoverflow=false;
		overflow_the_stack(2000);
	}
	mem_check_stack_collisions();
	#endif


	START_PROF_TIMER( proftimer_cpu_net );
	// total game frame timer
	hires_clock_t total_timer;
	total_timer.reset();

#if 0 //defined(MEMTRACK)
	mem_check_heap();
#endif

#if defined(TARGET_PC)
	if( !IsWindow(windows_app::inst()->get_hwnd()) )
		return;
#endif

	signal_manager::inst()->do_refresh();  // this must be done once per game frame
	input_mgr::inst()->poll_devices();

#ifdef GCCULL
	START_PROF_TIMER( proftimer_streams );
	sound_stream::advance_queue_requests();
	STOP_PROF_TIMER( proftimer_streams );
#endif

	// Frame Advance (animate) the scene
	START_PROF_TIMER( proftimer_advance );
	the_game->frame_advance();
	STOP_PROF_TIMER( proftimer_advance );

#ifndef BUILD_FINAL
	if ( check_heap )
	{
		//check_heap=false;
		mem_check_heap();
	}
#endif

	// Do not render the scene if replay is advancing to the end
	if(!ksreplay.IsPlaying() || !ksreplay.NoDraw())
	{

		START_PROF_TIMER( proftimer_render );
		START_PROF_TIMER( proftimer_render_cpu );



		// Render the level.
#ifdef NGL
		nglListInit();
			// Render shadows.
		if (doshadowdrawing)
			the_game->render_shadows();
		if (frontendmanager.fe_done)
			nglSetAnimTime (TIMER_GetTotalSec());
#else
		material::flush_last_context();
		fog_manager::inst()->update_fog();
		hw_rasta::inst()->begin_scene();
#endif
		menus->Draw();



		the_game->render();

		// The flip is now performed inside nglListSend, between building and sending the DMA chain. (dc 10/15/01)
		START_PROF_TIMER( proftimer_render_sendlist );
		// Don't flip the first time, because the back buffer is still uninitialized (dc 04/27/02)
		static bool first_time_ever = true;
#ifdef TARGET_PS2
		nglListSend(true);
#else
		nglListSend(!first_time_ever);
#endif
		first_time_ever = false;
		STOP_PROF_TIMER( proftimer_render_sendlist );

		STOP_PROF_TIMER( proftimer_render_cpu );
		STOP_PROF_TIMER( proftimer_render );
	}

	// total game frame time
	the_game->set_total_delta(total_timer.elapsed());
	the_game->set_limit_delta(0.0f);
	STOP_PROF_TIMER( proftimer_cpu_net );

#if defined(TARGET_XBOX) && !defined(BUILD_FINAL)
	if (
		input_mgr::inst()->get_control_state(ANY_LOCAL_JOYSTICK, PSX_SELECT) == AXIS_MAX &&
		input_mgr::inst()->get_control_state(ANY_LOCAL_JOYSTICK, PSX_START) == AXIS_MAX &&
		g_game_ptr != NULL
	   )
	{
		// Reboot to the dashboard.  Useful for people testing the game outside DevStudio.
		// Button combination is same as in XDK samples.  (dc 04/17/02)
		reboot_timer += g_game_ptr->get_total_delta();
		if(reboot_timer >= 3.0f)
			XLaunchNewImage(NULL, NULL);
	}
	else
		reboot_timer = 0.0f;
#endif
}

instance_render_info* app::get_viri()
{
	assert(viri != NULL);
	return viri;
}

void app::set_viri(instance_render_info* new_viri)
{
	assert (new_viri == NULL || (viri == NULL && new_viri != NULL));
	viri = new_viri;
}
