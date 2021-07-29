////////////////////////////////////////////////////////////////////////////////

// game.cpp
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.

////////////////////////////////////////////////////////////////////////////////
#include "global.h"

#include "anim_maker.h"
#include "billboard.h"
#include "colmesh.h"
#include "commands.h"
#include "debug_render.h"
#include "DemoMode.h"
#include "element.h"
#include "entity_maker.h"
#include "file_finder.h"
#include "fogmgr.h"
#include "game.h"
#include "game_info.h"
#include "game_process.h"
#include "gamefile.h"
#include "GameData.h"
#include "osGameSaver.h"
#include "geomgr.h"
#include "hwaudio.h"
#include "hwmovieplayer.h"
#include "interface.h"
#include "localize.h"
#include "lensflare.h"
#include "joystick.h"
#include "keyboard.h"
#include "mouse.h"
#include "MusicMan.h"
#include "msgboard.h"
#include "ksnsl.h"
//#include "WipeTransition.h"
#include "BoardManager.h"
#include "osdevopts.h"
#include "random.h"
#include "pmesh.h"
#include "beach.h"
#include "profiler.h"
#include "rumbleManager.h"
#ifdef GCCULL
#include "sound_group.h"
#endif
// BIGCULL #include "spiderman_controller.h"
#include "mustash.h"
#include "stashes.h"
#include "trigger.h"
#include "tricks.h"
#include "terrain.h"
#include "waveSound.h"
#include "wds.h"
#include "widget_script.h"
#include "VOEngine.h"
#include "script_lib_controller.h"
#include "script_lib_mfg.h"
#include "SFXEngine.h"
#include "file_manager.h"
#include "global.h"
#include "FrontEndManager.h"
#include "beachdata.h"		// For BEACHDATA_Load
#include "careerdata.h"		// For CAREERDATA_Load
#include "surferdata.h"		// For SURFERDATA_Load
#include "boarddata.h"
#include "camera_tool.h"
#include "ksfx.h"			// For ks_fx_destroy_all
#include "water.h"	// For WATER_Init
/*	Attempts at screen spray (dc 01/04/02)
#include "refract.h"	// For REFRACT_Init, REFRACT_Update
#include "displace.h"	// For DISPLACE_Init, DISPLACE_Draw
*/
#include "wavetex.h"
#include "kshooks.h"	// For KSWhatever calls
#include "blur.h"	// For BLUR_Init, BLUR_Draw
#include "wavedata.h"	// For WAVEDATA_Load()
#include "sounddata.h" // For SOUNDDATA_Load()
#include "soundscript.h"

#ifdef TARGET_PC
//#include "winapp.h"
#endif

#include "kellyslater_controller.h"
#include "ksdbmenu.h"
#include "ks_camera.h"
#include "sounddata.h"
#ifdef TARGET_PS2
#include "hwosps2/ps2_input.h"
//#include "libscf.h"
extern ps2_joypad_device *g_pad;
#include "ngl_ps2_internal.h"	// For NGL_VRAM_LOCKED_START only!!! (dc 12/17/01)
#endif

#if _CONSOLE_ENABLE
#include "console.h"
#endif

#if defined(TARGET_XBOX)
#include "region.h"
#include "terrain.h"

// JIV FIXME
//#define nglDestroyTexture(t) nglReleaseTexture(t)
#define NGL_VRAM_LOCKED_START 0 // let's be portable yay
#endif /* TARGET_XBOX JIV DEBUG */

extern profiler_counter profcounter_frame_delta;
extern profiler_counter profcounter_frame_avg_delta;
extern profiler_counter profcounter_frame_min_delta;
extern profiler_counter profcounter_frame_max_delta;

extern int global_frame_counter;  // CTT 04/09/00: this is a stupid kluge to save time at the last minute before E3
int g_wipeout_blur = 1;
bool g_screenshot = false;
const char *g_screenshot_filename = NULL;

Random *g_random_ptr;
Random *g_random_r_ptr;

//--------------------------------------------------------------
void clear_zbuffer()
{
#if defined(TARGET_PC) && 1
	//  For now, this was a hack anyway.
	//  if (g_game_ptr->get_total_delta() < 1.0f/20) // skip this minor visual improvement if we're getting slow framerate
	{
		// clear zbuffer to sort interface stuff in front of world!
		hw_rasta::inst()->set_zbuffering( false, true );
		render_quad_2d(vector2d(0.0f,0.0f),vector2d(1.0f,1.0f),0.999f,color32(0,0,0,0));
		hw_rasta::inst()->set_zbuffering( true, false );
	}
#endif
}


lensflare *sunFlare = NULL;
////////////////////////////////////////////////////////////////////////////////
//  globals
////////////////////////////////////////////////////////////////////////////////

cheat_info_t g_cheats;
debug_info_t g_debug;

game_info::game_info()
{
	reset();
}

bool game_info::set_str(const pstring &att, const stringx &val)
{
#define MAC(type, var, def, str)  { static pstring pstring_set(##str##); if(att == pstring_set) { ##var## = val; return(true); } }
#define GAME_INFO_STRS
#include "game_info_vars.h"
#undef GAME_INFO_STRS
#undef MAC

	return(false);
}

bool game_info::get_str(const pstring &att, stringx &val) const
{
#define MAC(type, var, def, str)  { static pstring pstring_get(##str##); if(att == pstring_get) { val = ##var##; return(true); } }
#define GAME_INFO_STRS
#include "game_info_vars.h"
#undef GAME_INFO_STRS
#undef MAC

	return(false);
}

bool game_info::set_num(const pstring &att, rational_t val)
{
#define MAC(type, var, def, str)  { static pstring pstring_set(##str##); if(att == pstring_set) { ##var## = (##type##)##val##; return(true); } }
#define GAME_INFO_NUMS
#include "game_info_vars.h"
#undef GAME_INFO_NUMS
#undef MAC

	return(false);
}

bool game_info::get_num(const pstring &att, rational_t &val) const
{
#define MAC(type, var, def, str)  { static pstring pstring_get(##str##); if(att == pstring_get) { val = (##type##)##var##; return(true); } }
#define GAME_INFO_NUMS
#include "game_info_vars.h"
#undef GAME_INFO_NUMS
#undef MAC

	return(false);
}

void game_info::reset()
{
#define MAC(type, var, def, str)    var = (##type##)##def##;
#define GAME_INFO_NUMS
#define GAME_INFO_STRS
#include "game_info_vars.h"
#undef GAME_INFO_NUMS
#undef GAME_INFO_STRS
#undef MAC
}


// <<<< DL -- get_hero_name() looks retarded but I had to do it because no easy was to make an array of hero_name_0, hero_name_1
// JIV Stroustrup 15.5 contains info on pointer to member function syntax
const stringx &game_info::get_hero_name(int hero_num)
{
	assert(hero_num < MAX_PLAYERS);
	switch(hero_num)
	{
	case 0: return get_hero_name_0();
	case 1: return get_hero_name_1();
	default: return get_hero_name_0();
	}

#if defined(TARGET_XBOX)
	STUB( "game_info::get_hero_name(int hero_num)" );
#endif /* TARGET_XBOX JIV DEBUG */
}


vector3d global_ZEROVEC = vector3d(0,0,0); // for being returned by reference
extern game *g_game_ptr;

#ifdef GCCULL
list<sound_stream *> g_stream_play_list;
#endif
file_finder *g_file_finder = NULL;
entity_maker *g_entity_maker = NULL;
anim_maker *g_anim_maker = NULL;

const chunk_flavor CHUNK_WEDGE      ("wedge");
const chunk_flavor CHUNK_NUM_WEDGES ("nowedges");
const chunk_flavor CHUNK_NORMAL     ("normal");
const chunk_flavor CHUNK_NO_WARNINGS  ("nowarn");

// for testing out multiple level loading
char g_scene_name[256];
char g_movie_name[256];
char g_console_command[256];   // for ps2 console (temp)


//For bluring stuff
//static float wipeBlurAlpha = .3;
//static float wipeDeltaY = 0;
//static float wipeDeltaX = 0;
//static float wipeColor[3] = {1, 1, 1};
//int wipeTransition = 0;
//static float wipeX = 512;
//static float wipeY = 512;
//static int wipeTex = 0;
//static float wipeYZero = 0;
//static float wipeXZero = 0;
#ifdef TARGET_PC
static int podump_idx = 0;
#define CAM_IN_MOTION_DELAY 2.0f
#endif

////////////////////////////////////////////////////////////////////////////////
//  game
////////////////////////////////////////////////////////////////////////////////

//bool KSReadFile( char* FileName, nglFileBuf* File, u_int Align );
//void KSReleaseFile( nglFileBuf* File );
//void* KSMemAllocNSL( u_int Size, u_int Align );
//void KSMemFree( void* Ptr );

const int game::SNAPSHOT_WIDTH = 128;
const int game::SNAPSHOT_HEIGHT = 128;

void nslNoPrint( const char * text ) {};
void nslNoErrors(const char * text ) {};

#ifdef TARGET_GC
// Forgive me, Father.
// Defined in gc_main.cpp to indicate necessity
// of displaying PAL60 query screen.
extern bool ksGCQueryPAL60;
#endif

/*** Constructor ***/
game::game()
{
	int i;
#ifdef KSCULL
	construct_script_controllers();
#endif

	// set up starting level
	strcpy(g_scene_name, os_developer_options::inst()->get_scene_name().c_str());

#ifdef USE_FILE_MANAGER
	file_manager::inst()->init_level_context("levels\\"+g_scene_name);
#endif

	// *** C++ style allocations ***
	the_file_finder = NEW file_finder;
	g_file_finder = the_file_finder;
	the_entity_maker = NEW entity_maker;
	g_entity_maker = the_entity_maker;
	the_anim_maker = NEW anim_maker;
	g_anim_maker = the_anim_maker;
	the_world = NEW world_dynamics_system;
	g_world_ptr = the_world;
	mb = NEW message_board();
	g_random_ptr   = &rstream;
	g_random_r_ptr = &rstream_r;

	// *** C style allocations ***
	gamefile = (gamefile_t*)malloc( sizeof(gamefile_t) );
	*gamefile = gamefile_t();

	//low_level_console_print("game allocations done"); llc_memory_log();

	// *** Member data initialization ***
	total_delta = 0.0f;
	flip_delta = 0.0f;
	limit_delta = 0.0f;
	min_delta = 1e10f;
	max_delta = 0.0f;
	avg_delta = 0.1f;

	cam_pos_in_use_count = 0;
	cam_in_motion_timer = 0.0f;
	field_of_view = 90.0f;


	// flags
	flag.disable_interface = false;
	flag.disable_start_menu = false;
	flag.hero_frozen = false;
	flag.i_quit = false;
	flag.i_restart = false;
	flag.i_win = false;
	flag.i_lose = false;
	flag.level_is_loaded = false;
	flag.load_new_level = false;
	flag.play_movie = false;
	flag.single_step = false;
	flag.wait_for_start_release = false;
	flag.game_paused = false;


	disconnect_status = false;

	set_render_state(GAME_RENDER_DRAW_FE);
	// We havent loaded a game yet
	currentGame.valid = GSErrorOther;
	// current cameras/mics
	//base_camera = NULL;
	current_view_camera = NULL;
	is_user_cam_on = false;
	was_user_cam_ever_on = false;
	for(i=0;i<MAX_PLAYERS;i++)
	{
		player_cam[i]=NULL;
		personality[i]=false;
		handicap[i] = 0;
	}
	current_mic = NULL;

	// game mode
//	set_game_mode(GAME_MODE_FREESURF_INFINITE);
	set_game_mode(GAME_MODE_CAREER);
	play_mode.push = NULL;
	play_mode.timeAttack = NULL;
	play_mode.meterAttack = NULL;
	play_mode.headToHead = NULL;

	// interface
	my_interface_widget = NULL;
	script_widget_holder = NULL;
	menu_screen = NULL;

	GenericGameSaver::create_inst();
	GenericGameSaver::inst()->init();


#if defined(TARGET_XBOX)
	extern bool KSReadFile( const char* FileName, nglFileBuf* File, u_int Align );
#endif /* TARGET_XBOX JIV DEBUG */

	// audio streams
	if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
	{
		nslSystemCallbackStruct callbacks;
		/*	Can't use these until sound files are included in stash (dc 03/06/02)
		callbacks.ReadFile = (bool (*)(const char *, nslFileBuf *, unsigned int))&KSReadFile;
		callbacks.ReleaseFile = (void (*)(nslFileBuf *))&KSReleaseFile;
		*/
		callbacks.ReadFile    = NULL;
		callbacks.ReleaseFile = NULL;
		callbacks.MemAlloc    = &KSMemAllocNSL;
		callbacks.MemFree     = &KSMemFree;
		callbacks.Error       = &KSCriticalError;

		if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO_WARNINGS))
			callbacks.Warning = &nslNoPrint;
		else
			callbacks.Warning = NULL;

		callbacks.CriticalError = &KSCriticalError;

		if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO_WARNINGS))
			callbacks.DebugPrint = &nslNoPrint;
		else
			callbacks.DebugPrint = NULL;


		nslSetSystemCallbacks(&callbacks);
#if defined(TARGET_XBOX)
		nslSetRootDir("D:\\");
#endif
#ifdef TARGET_PS2
		if (!nglUsingProView)
#endif
		if (!nslInit())
		{
#if defined(TARGET_XBOX) && defined(BUILD_FINAL)
			// Xbox requires a message on all disk read failures.  (dc 07/11/02)
			disk_read_error();
#endif
		}
#if defined(TARGET_PS2) || defined(TARGET_GC)
		nslSetRootDir("SOUNDS");
#elif defined(TARGET_XBOX)
		nslSetRootDir("D:\\SOUNDS");
#endif
#if defined(DEBUG) && defined(TARGET_PS2) && !defined(PROFILING_ON)
		nslSetHostStreamingPS2(true);
#endif

		VoiceOvers.init();
		//StoredConfigData::inst()->setGameConfig(&(g_career->cfg));
		MusicMan::create_inst();
		MusicMan::inst()->init();

	}
	else // NO AUDIO
	{
		nslSystemCallbackStruct callbacks;
		callbacks.ReadFile = NULL;
		callbacks.ReleaseFile = NULL;
		callbacks.MemAlloc = &KSMemAllocNSL;
		callbacks.MemFree = &KSMemFree;
		callbacks.CriticalError = &nslNoErrors;
		callbacks.DebugPrint = &nslNoPrint;
		nslSetSystemCallbacks(&callbacks);

	}



	StoredConfigData::create_inst();
	StoredConfigData::inst()->init();

#ifdef GCCULL
	music_track[0] = NULL;
	music_track[1] = NULL;
	music_state = MUSIC_STATE_QUIET;

	music_stream = NULL;
	music_applied_volume = 1.0f;
	music_fade_time_left = 0.0f;
	music_fade_to_volume = 0.0f;

	memset( &ambient_stream, 0, sizeof(ambient_stream) );
	memset( &next_ambient_stream, 0, sizeof(next_ambient_stream) );
#endif

	// *** Init functions ***
	geometry_manager::inst()->restore();
	init_random_ifl_frame_boost_table();
	load_locales();
	setup_input_registrations();
	setup_inputs();
	set_cursor( 0 ); // should be a member of the to-be written hw_graphics class

	// elements
	//  assert(element_manager::inst()->get_num_context() == 0);
	//  element_manager::inst()->push_context( "GAME" );
	//  element_manager::inst()->create_default_elements();
	//  element_manager::inst()->restore_default_elements();

	p_blank_material = NULL;
	/*
	p_blank_material = material_bank.new_instance( material(os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR)+"\\alpha") );
	hw_texture_mgr::inst()->set_white_texture(p_blank_material->get_texture(0));
	*/
	// Default the time to midnitght January 1, 2000 (just in case time is not supported
	systime.year = 2000;
	systime.month = 1;
	systime.weekday = 7;
	systime.day = 1;
	systime.hour = 0;
	systime.minute = 0;
	systime.second = 0;

	// default
	num_ai_players = 0;
	num_players = 1;
	num_active_players = 1;
	active_player = 0;
	heroname[0] = SurferDataArray[SURFER_KELLY_SLATER].name;
	heroname[1] = SurferDataArray[SURFER_KELLY_SLATER].name;
	surferIdx[0] = SURFER_KELLY_SLATER;
	surferIdx[1] = SURFER_KELLY_SLATER;
	boardIdx[0] = os_developer_options::inst()->get_int(os_developer_options::INT_BOARD_0);
	boardIdx[1] = os_developer_options::inst()->get_int(os_developer_options::INT_BOARD_1);

	for( i = 0; i < MAX_PLAYERS; i++ )
		player_devices[i] = i;

	// Initialize snapshots.
	snapshot = NULL;
	snapshotState = SNAPSTATE_NONE;
	destSnapshot = NULL;

	// for ps2 console support (until we get a keyboard on the ps2)
	g_console_command[0] = '\0';

	// If we find legal stash, all loads must be from stash.  This line allows us to run
	// a debug build from stash, without having to create a game.ini .  Ideally, we should
	// be able to run partly from stash and partly from data, but that's not working.  (dc 03/20/02)

	if (stash::open_stash("legal.st2", STASH_LEGAL))
	{
		os_developer_options::inst()->set_flag(os_developer_options::FLAG_STASH_ONLY, true);
	}
	else
	{
		os_developer_options::inst()->set_flag(os_developer_options::FLAG_STASH_ONLY, false);
		warning("could not open system stash file legal.st2\n");
	}

	// Get that legal screen up!
	frontendmanager.LoadFonts();

	for (int i=0; i < 4; i++)
	{
		nglListInit ();
		render_legal_screen();
		nglListSend(true);
	}
	float mytime = 0.0f;
	real_clock.reset();
	stash::open_stash("system.st2", STASH_SYSTEM);

#ifdef TARGET_XBOX
extern void nglSetGammaRamp(const char *FileName);
	nglSetGammaRamp("GammaRamp.raw");	// Must wait until system stash is loaded (dc 05/19/02)
#endif

	BEACHDATA_Load();
	CAREERDATA_Load();
	SURFERDATA_Load();
	TRICKDATA_Load();
	WAVEDATA_Load();
	GLOBALTEXT_Load();
	BOARDDATA_Load();
	globalCareerData.init();
	frontendmanager.LoadMap();

	LoadScoringSystem();

	#ifdef TARGET_GC
	// clear the screen so that movie skipping doesn't
	// show the old contents of the EFB for a frame
	nglListInit( );
	nglListBeginScene( );
	nglSetClearFlags( NGLCLEAR_COLOR | NGLCLEAR_Z );
	nglSetClearColor( 0, 0, 0, 0 );
	nglSetClearZ( 1.0f );
	nglListEndScene( );
	nglListSend( true );
	// extra, just to be safe
	nglFlip( );

	if( ksGCQueryPAL60 ) {
		render_pal60_screen( );
	}
	#endif

	// process setup
	push_process( startup_process );

	//real_clock.reset();

	most_recent_beach = BEACH_SEBASTIAN;
	most_recent_level = -1;
	LoadingStateInit();
#ifdef TARGET_PS2
	// Must come after system stash is opened!
	GenericGameSaver::inst()->calcSavedGameSize();
#endif
#ifdef TARGET_PS2
	while (mytime < 5.0f)
	{
		mytime = real_clock.elapsed();
	}
#elif defined (TARGET_GC)
	while (mytime < 1.0f)
	{
		mytime = real_clock.elapsed();
	}

#endif

	multiplayer_difficulty = 1000;

#if (defined DEBUG) && !(defined USER_MKV)
	nglPrintf("DONE WITH GAME CONSTRUCTOR\n");
#endif
}


/*** Destructor ***/
game::~game()
{
	// release the music streams
#ifdef ECULL
	if (music_track[0] != NULL)
		music_track[0]->release();
	music_track[0] = NULL;
	if (music_track[1] != NULL)
		music_track[1]->release();
	music_track[1] = NULL;
#endif

	// C style deallocations
	free(gamefile);

	// C++ style deallocations
	delete mb;

	if( the_world )
	{
		unload_current_level();
		delete g_world_ptr; // PEH TEST
		g_world_ptr = 0;
	}

	g_random_ptr    = NULL;
	g_random_r_ptr  = NULL;

	delete g_anim_maker;
	the_anim_maker = g_anim_maker = NULL;

	delete g_entity_maker;
	the_entity_maker = g_entity_maker = NULL;

	delete g_file_finder;
	the_file_finder = g_file_finder = NULL;

	hw_texture_mgr::inst()->set_white_texture(NULL);
	if (p_blank_material)
		delete p_blank_material;

	if (stash::is_stash_open())
		stash::close_stash();
	if (stash::using_stash())
		stash::free_stored();
	//#pragma todo("Find out why freeing the system stash crashes. (GT-2/2/01)")
	//  if (stash::is_system_stash_open())
	//    stash::free_stored(true);
#ifdef KSCULL
	destruct_script_controllers();
#endif

}


/*** construct_game_widgets ***/
void game::construct_game_widgets()
{
	assert(my_interface_widget == NULL);
	flag.disable_interface = true;	// temporarily disable overlays (dc 05/07/01)
	my_interface_widget = NEW interface_widget( "interface", NULL, 0, 0 );
	my_interface_widget->show();

	assert(script_widget_holder == NULL);
	script_widget_holder = NEW script_widget_holder_t( "script widg holder", NULL, 0, 0 );
	script_widget_holder->show();

	stringx file = stringx("interface\\")+os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + stringx("\\kellyslatermenu0");

	widget::set_rhw_2d_layer( widget::RHW0 );
	menu_screen = NEW background_widget( "menu bg", my_interface_widget, 0, 0, file );
	widget::restore_last_rhw_2d_layer();
	menu_screen->hide();
}

/*** destroy_game_widgets ***/
void game::destroy_game_widgets()
{
	if ( my_interface_widget )
		delete my_interface_widget;
	my_interface_widget = NULL;
	menu_screen = NULL;

	if ( script_widget_holder )
		delete script_widget_holder;
	script_widget_holder = NULL;
}

#if defined(TARGET_PS2)
#include <libcdvd.h>
extern bool bad_mc_stack_error;
#endif


/*** frame_advance ***/
void game::frame_advance()
{
	// update framerate counters
	time_value_t delta = total_delta;
	if (delta>0.25f) delta = 0.25f; // in case we stopped in the debugger
	avg_delta = (avg_delta*9.0f + delta)*(1.0f/10); // simple weighted average
	min_delta = min(delta,min_delta);
	max_delta = max(delta,max_delta);
	min_delta = (min_delta*99.0f + avg_delta)*(1.0f/100); // drift very slowly toward average
	max_delta = (max_delta*99.0f + avg_delta)*(1.0f/100);

	profcounter_frame_delta.set_count(FTOI(delta*1000.0f)); // to msecs
	profcounter_frame_avg_delta.set_count(FTOI(avg_delta*1000.0f)); // to msecs
	profcounter_frame_min_delta.set_count(FTOI(min_delta*1000.0f)); // to msecs
	profcounter_frame_max_delta.set_count(FTOI(max_delta*1000.0f)); // to msecs

#if defined(TARGET_PC)
	time_t longtime;
	time( &longtime );
	tm *system_time = localtime(&longtime);
	assert(system_time);

	systime.year = system_time->tm_year + 1900;       // full year (i.e. 2001)
	systime.month = system_time->tm_mon + 1;          // 1-12 ( 1 = January )
	systime.weekday = system_time->tm_wday + 1;       // 1-7  ( 1 = Sunday )
	systime.day = system_time->tm_mday;               // 1-31
	systime.hour = system_time->tm_hour;              // 0-23
	systime.minute = system_time->tm_min;             // 0-59
	systime.second = system_time->tm_sec;             // 0-59
#elif defined(TARGET_PS2)
													  /*
													  sceCdCLOCK system_time;
													  if(sceCdReadClock(&system_time))
													  {
													  systime.year = system_time.year + 1900;        // full year (i.e. 2001)
													  systime.month = system_time.month + 1;         // 1-12 ( 1 = January )
													  systime.weekday = system_time.pad + 1;        // 1-7  ( 1 = Sunday )
													  systime.day = system_time.day;                 // 1-31
													  systime.hour = system_time.hour;               // 0-23
													  systime.minute = system_time.minute;           // 0-59
													  systime.second = system_time.second;            // 0-59
													  }
	*/
#endif
#ifdef TARGET_PS2
	if (bad_mc_stack_error)
	{
		assert(0 && "THE MEMORY CARD HAS OVERRUN ITS STACK, PROBABLY CORRUPTING MEMORY!");
		bad_mc_stack_error=false;
	}
#endif
	frame_advance_level();
}


extern profiler_timer proftimer_advance;
//extern profiler_timer proftimer_events;
extern profiler_timer proftimer_special_fx;
//extern profiler_timer proftimer_dynamic_ents;

extern profiler_counter profcounter_total_blocks;
extern profiler_counter profcounter_alloced_mem;
extern profiler_counter profcounter_texture_mem;
extern profiler_counter profcounter_audio_mem;
extern profiler_counter profcounter_tri_rend;



bool g_controller_inserted = true;
bool g_controller_never_inserted = false;



extern profiler_timer proftimer_render_interface;
extern profiler_timer proftimer_debug_info;
extern profiler_timer proftimer_profiler;
extern profiler_timer proftimer_draw_prof_info;


/*** render ***/

/*
#include "hwosps2/ps2_m2vplayer.h"
static m2v_player_t* player;
// call m2v_destroy_player (player); when done
*/
/*
static char clock_char = '|';
static int timer = 0;
*/

void game::render_map_screen()
{
	if(start_drawing_map)
	{
		LoadingProgressUpdate();
		frontendmanager.DrawMap(loading_progress);
	}

}


void game::render_legal_screen()
{
	if(!frontendmanager.lfe)
	{
		frontendmanager.lfe = NEW LegalFrontEnd(&frontendmanager, "interface\\Legal\\", "Legal.PANEL");
		frontendmanager.lfe->Load();
	}
	nglListBeginScene();
	nglSetClearFlags(NGLCLEAR_Z | NGLCLEAR_COLOR | NGLCLEAR_STENCIL);
	nglSetClearColor(0, 0, 0, 0);
	frontendmanager.lfe->Draw();
	nglListEndScene();
}

#ifdef TARGET_GC
extern void system_idle( void );

void game::render_pal60_screen()
{
	input_mgr* input = input_mgr::inst( );
	bool pal60_done = false;
	bool selected = false;

	do {
		system_idle( );

		if( !frontendmanager.pfe ) {
			frontendmanager.pfe = NEW PAL60FrontEnd( NULL, &frontendmanager, "interface\\Legal\\", "Legal.PANEL" );
			frontendmanager.pfe->Init( );
			frontendmanager.pfe->OnActivate( );
		}

		input->poll_devices( );
		float value = input->get_control_state( ANY_LOCAL_JOYSTICK, PSX_LR );

		// the values passed to On* are ignored
		if( value < 0.0f ) {
			frontendmanager.pfe->OnLeft( 0 );
		} else if( value > 0.0f ) {
			frontendmanager.pfe->OnRight( 0 );
		}

		value = input->get_control_state( ANY_LOCAL_JOYSTICK, PSX_X );

		if( value > 0.0f && ( selected == false ) ) {
			selected = true;

			FEMenuEntry* highlighted = frontendmanager.pfe->highlighted;
			int which = ( highlighted == frontendmanager.pfe->first ) ? 0 : 1;
			frontendmanager.pfe->Select( which );
		}

		frontendmanager.pfe->Update( 0.0f );

		nglListInit( );
		nglListBeginScene( );
		nglSetClearFlags( NGLCLEAR_Z | NGLCLEAR_COLOR | NGLCLEAR_STENCIL );
		nglSetClearColor( 0, 0, 0, 0 );

		frontendmanager.pfe->Draw( );

		nglListEndScene( );
		nglListSend( true );

		pal60_done = frontendmanager.pfe->IsDone( );
	} while( pal60_done == false );

}
#endif

void game::render_title_screen()
{
	if(!frontendmanager.tfe)
	{
		frontendmanager.tfe = NEW TitleFrontEnd(&frontendmanager, "interface\\Legal\\", "Legal.PANEL");
		frontendmanager.tfe->Load();
	}
	nglListBeginScene();
	nglSetClearFlags(NGLCLEAR_Z | NGLCLEAR_COLOR | NGLCLEAR_STENCIL);
	nglSetClearColor(0, 0, 0, 0.0);
	frontendmanager.tfe->Draw();
	nglListEndScene();
}

void game::render_igo()
{

	// Render IGO
	nglListBeginScene();
	nglSetClearFlags(NGLCLEAR_Z | NGLCLEAR_STENCIL);
	ksnglSetPerspectiveMatrix(90.0f, nglGetScreenWidth()/2, nglGetScreenHeight()/2, 0.2f, 1200.0f);	 // this is called to set the farZ to something reasonable for the IGO
	START_PROF_TIMER( proftimer_kelly_slater_interface );
	IGODraw();
	STOP_PROF_TIMER( proftimer_kelly_slater_interface );

	if (rumbleMan.drawState)
		rumbleMan.drawCurrentState();

	nglListEndScene();

}

void game::render_mem_free_screen()
{
	if ( os_developer_options::inst()->is_flagged(os_developer_options::FLAG_MEM_FREE_SCREEN) || g_debug.mem_free_screen )
	{
		nglQuad q;

		nglInitQuad(&q);
		nglListBeginScene();
		nglSetQuadRect (&q, nglGetScreenWidth() - 100, nglGetScreenHeight() - 31, nglGetScreenWidth(), nglGetScreenHeight());
		nglSetQuadColor (&q, NGL_RGBA32 (0, 0, 0, 128));
		nglSetQuadZ (&q, 0.3f);
		nglListAddQuad (&q);

/*	Replaced by new API. (dc 05/30/02)
		KSNGL_SetFont (0);
		KSNGL_SetFontColor (0x80808080);
		KSNGL_SetFontZ (0.2f);
		KSNGL_SetFontScale (1, 1);
*/

		nglListAddString (nglSysFont, nglGetScreenWidth() - 95, nglGetScreenHeight() - 29, 0.2f,
			0x80808080, "RAM Free:");
		nglListAddString (nglSysFont, nglGetScreenWidth() - 95, nglGetScreenHeight() - 15, 0.2f,
			0x80808080, "%d", mem_get_total_avail() );

		nglListEndScene();
	}
}

void game::do_profiler_stuff()
{

	// Calculate and display frame rate
#ifdef PROFILING_ON
	STOP_PROF_TIMER( proftimer_cpu_net );
	STOP_PROF_TIMER( proftimer_render_cpu );
	proftimer_profiler.start();
	proftimer_draw_prof_info.start();

	nglListBeginScene();
	nglSetClearFlags(NGLCLEAR_Z | NGLCLEAR_STENCIL);
	profiler::inst()->render();
	nglListEndScene();

	proftimer_draw_prof_info.stop();
	proftimer_profiler.stop();
	START_PROF_TIMER( proftimer_render_cpu );
	START_PROF_TIMER( proftimer_cpu_net );
#endif // PROFILING_ON

}


void game::render_fe()
{
	FEDraw();
	render_mem_free_screen();
}



void game::do_autobuild_stuff()
{
#define AUTOBUILD_WAIT_FRAMES 1000	// frames to wait before doing mem dump
#define AUTOBUILD_SKIP_FRAMES 10	// print out success message every so often
#define AUTOBUILD_PRINT_FRAMES 100	// print out success message so many times

	extern char g_dump_profile_info;
	// register successful autobuild and compute metrics
	if (os_developer_options::inst()->is_flagged( os_developer_options::FLAG_AUTOBUILD ))
	{
		static int render_count = 0;
		++render_count;

#ifdef TARGET_PS2	// can't write to host on XBox (dc 03/01/02)
		if (render_count == AUTOBUILD_WAIT_FRAMES)
		{
			g_dump_profile_info = true;
			mem_dump_heap();
		}
#endif
		// Print success message every few frames, until we're pretty sure it got through
		if ((render_count % AUTOBUILD_SKIP_FRAMES == 0) &&
			(render_count / AUTOBUILD_SKIP_FRAMES < AUTOBUILD_PRINT_FRAMES) )
		{
			nglPrintf("AUTOBUILD:\tSuccessfully began game!\n");
		}
	}
}



void game::render_level_onescreen()
{

	nglListBeginScene();
	WAVETEX_SetPlayer(active_player);

#ifndef TARGET_GC
	// For the early frames of the snapshot, use a special camera.
	if (snapshotState == SNAPSTATE_TEXTURE_RENDER)
	{
		if (!is_user_cam_on &&
			current_view_camera != the_world->get_ks_controller(active_player)->get_wipeout_cam_ptr() &&
			current_view_camera != the_world->get_ks_controller(active_player)->get_wipeout_cam2_ptr())
		{
			// Save old camera.
			snapshotPrevCam = current_view_camera;

			// Use photo cam for taking pictures.
			the_world->get_ks_controller(active_player)->SetPlayerCamera(the_world->get_ks_controller(active_player)->get_photo_cam_ptr());
			player_cam[active_player]->frame_advance(0.1f);
			player_cam[active_player]->adjust_geometry_pipe();
		}
		else
			snapshotPrevCam = NULL;
	}
#endif

	set_current_camera(player_cam[active_player]);

	UNDERWATER_EntitiesTrackCamera();
	UNDERWATER_CameraChecks(active_player);

	entity *ent = g_world_ptr->get_ks_controller(active_player)->get_owner();
	ent->set_hero_id(active_player);
	ent->updatelighting(0.1,active_player);
	ent= ((conglomerate *)g_world_ptr->get_ks_controller(active_player)->GetBoardModel())->get_member("BOARD");
	ent->set_hero_id(active_player);
	ent->updatelighting(0.1,active_player);

	WAVETEX_CheckClearShadows();

	// Prepare render.
	nglListBeginScene();
		// This is so we can use child scenes without the screen clear happening
		// after they're drawn
	nglSetClearFlags(NGLCLEAR_COLOR | NGLCLEAR_Z | NGLCLEAR_STENCIL);

	// Render the level.
	geometry_manager::inst()->set_cop(hw_rasta::inst()->get_screen_width()*0.5f,hw_rasta::inst()->get_screen_height()*0.5f, WORLD_MIN_Z, WORLD_MAX_Z);
	geometry_manager::inst()->set_xform(geometry_manager::XFORM_LOCAL_TO_WORLD, identity_matrix);
	START_PROF_TIMER( proftimer_render_world );
	the_world->render(current_view_camera, active_player); // render the main 3d scene
	STOP_PROF_TIMER( proftimer_render_world );
	vr_billboard::flush();

	draw_debug_labels();
	frontendmanager.UpdateIGOScene();
	nglListEndScene();

	if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_SHOW_MOTIONBLUR))
	{
		BLUR_Draw();
	}
	nglListEndScene();
}

void game::render_level_splitscreen()
{
	int		centerX, centerY;
	float	share;

	if (the_world->get_ks_controller(0)->is_active())
	{


		// Calculate center of viewport.
		share = get_player_share(0);
		centerX = int((640.0f*share)/2.0f);
		centerY = int(480.0f/2.0f);
		adjustCoords(centerX, centerY);

		// Render this player's viewport.
		WAVETEX_SetPlayer(0);
		geometry_manager::inst()->set_cop(centerX, centerY, WORLD_MIN_Z, WORLD_MAX_Z);
		geometry_manager::inst()->set_xform(geometry_manager::XFORM_LOCAL_TO_WORLD, identity_matrix);
		set_current_camera(player_cam[0]);
		get_current_view_camera()->adjust_geometry_pipe();
		UNDERWATER_CameraChecks(0);
		entity *ent = g_world_ptr->get_ks_controller(0)->get_owner();
		ent->set_hero_id(0);
		//ent->updatelighting(0.1,0);
		ent= ((conglomerate *)g_world_ptr->get_ks_controller(0)->GetBoardModel())->get_member("BOARD");
		ent->set_hero_id(0);
		//ent->updatelighting(0.1,0);
		nglListBeginScene();
		ksnglSetViewport(player_viewports[0].get_left(), player_viewports[0].get_top(), player_viewports[0].get_right(), player_viewports[0].get_bottom());
		ksnglSetPerspectiveMatrix(90, centerX, centerY, 0.2f, 65536.0f);
		nglSetClearFlags(NGLCLEAR_COLOR | NGLCLEAR_Z | NGLCLEAR_STENCIL);		// maybe can move this to clear entire screen just 1x
		the_world->render(current_view_camera, 0); // render the main 3d scene
		vr_billboard::flush();  //what is this?
		nglListEndScene();
	}

	if (the_world->get_ks_controller(1)->is_active())
	{
		// Calculate center of viewport.
		share = get_player_share(1);
		centerX = int((640.0f*share)/2.0f + 640.0f*(1.0f-share));
		centerY = int(480.0f/2.0f);
		adjustCoords(centerX, centerY);

		// Render this player's viewport.
		WAVETEX_SetPlayer(1);
		geometry_manager::inst()->set_cop(centerX, centerY, WORLD_MIN_Z, WORLD_MAX_Z);
		geometry_manager::inst()->set_xform(geometry_manager::XFORM_LOCAL_TO_WORLD, identity_matrix);
		set_current_camera(player_cam[1]);
		get_current_view_camera()->adjust_geometry_pipe();
		UNDERWATER_CameraChecks(1);
		entity *ent = g_world_ptr->get_ks_controller(1)->get_owner();
		ent->set_hero_id(1);
		//ent->updatelighting(0.1,1);
		ent= ((conglomerate *)g_world_ptr->get_ks_controller(1)->GetBoardModel())->get_member("BOARD");
		ent->set_hero_id(1);
		//ent->updatelighting(0.1,1);
		nglListBeginScene();
		ksnglSetViewport(player_viewports[1].get_left(), player_viewports[1].get_top(), player_viewports[1].get_right(), player_viewports[1].get_bottom());
		ksnglSetPerspectiveMatrix(90, centerX, centerY, 0.2f, 65536.0f);
		nglSetClearFlags(NGLCLEAR_COLOR | NGLCLEAR_Z | NGLCLEAR_STENCIL);		// maybe can move this to clear entire screen just 1x
		the_world->render(current_view_camera, 1); // render the main 3d scene
		vr_billboard::flush();  //what is this?
		nglListEndScene();
	}
}
static color32 current, target, old;

void game::render_trippy_cheat()
{
#ifndef TARGET_GC
	if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_TRIPPY) ||
		g_session_cheats[CHEAT_TRIPPY].isOn())
	{

		static int y = 20;

#if defined(TARGET_PS2)
		float alpha = .7f * 128;
#else
		float alpha = .4f * 255;
#endif
		if (current == target)
		{
			target.c.r = random(255);
			target.c.g = random(255);
			target.c.b = random(255);
			while ((target.c.r <= 200 && target.c.g <= 200 && target.c.b <= 200) &&
			       ((target.c.r - current.c.r) + (target.c.g - current.c.g) + (target.c.b - current.c.b)) < 200)
			{
				target.c.r = random(255);
				target.c.g = random(255);
				target.c.b = random(255);
			}
		}
		if (current.c.r != target.c.r)
			current.c.r += FTOI((target.c.r - current.c.r) > 0?1:-1)*(fabsf(target.c.r - current.c.r)>3?3:1);
		if (current.c.g != target.c.g)
			current.c.g += FTOI((target.c.g - current.c.g) > 0?1:-1)*(fabsf(target.c.g - current.c.g)>3?3:1);
		if (current.c.b != target.c.b)
			current.c.b += FTOI((target.c.b - current.c.b) > 0?1:-1)*(fabsf(target.c.b - current.c.b)>3?3:1);

		nglListBeginScene();
		nglSetClearFlags(NGLCLEAR_Z | NGLCLEAR_STENCIL);
		nglTexture *Tex = nglGetFrontBufferTex();
		nglQuad q;

		nglInitQuad(&q);
		nglSetQuadMapFlags(&q, NGLMAP_CLAMP_U | NGLMAP_CLAMP_V | NGLMAP_BILINEAR_FILTER);
		nglSetQuadTex(&q, Tex);
		nglSetQuadColor(&q, NGL_RGBA32(current.c.r, current.c.g, current.c.b, 255));
#ifdef TARGET_PS2
		nglSetQuadRect(&q, -y , -y, RoundUpToPowerOf2(nglGetScreenWidth())+y, RoundUpToPowerOf2(nglGetScreenHeight())+y);
#else
		nglSetQuadRect(&q, -y, -y, nglGetScreenWidth()+y, nglGetScreenHeight()+y);
#endif
		nglSetQuadZ(&q, 1);
		nglSetQuadUV(&q, 0, 0, 1, 1);
		nglSetQuadBlend(&q, NGLBM_CONST_BLEND, FTOI(alpha));
		nglListAddQuad(&q);
		nglListEndScene();
	}
#endif
}

void game::render_shadows()
{

	#ifdef TARGET_GC
	// Render shadows.
	if (get_render_state() != GAME_RENDER_GAME_LEVEL ) return;

	if (snapshotState != SNAPSTATE_TEXTURE_RENDER && !is_splitscreen())
	{
		set_current_camera(player_cam[active_player]);
		current_view_camera->adjust_geometry_pipe();
		UNDERWATER_CameraChecks(active_player);
		void WAVETEX_RenderShadows( camera *cam );
		WAVETEX_RenderShadows(current_view_camera);
	}
	#endif

}





void game::render_level()
{

	#ifdef TARGET_GC
	#if 0
	// Render shadows.
	if (snapshotState != SNAPSTATE_TEXTURE_RENDER && !is_splitscreen())
	{
		set_current_camera(player_cam[0]);
		void WAVETEX_RenderShadows( camera *cam );
		WAVETEX_RenderShadows(current_view_camera);
	}
	#endif

	if (snapshotState != SNAPSTATE_NONE)
	{
		// For the early frames of the snapshot, use a special camera.
		if (snapshotState == SNAPSTATE_TEXTURE_RENDER)
		{
			if (!is_user_cam_on &&
				current_view_camera != the_world->get_ks_controller(active_player)->get_wipeout_cam_ptr() &&
				current_view_camera != the_world->get_ks_controller(active_player)->get_wipeout_cam2_ptr())
			{
				// Save old camera.
				snapshotPrevCam = current_view_camera;

				// Use photo cam for taking pictures.
				the_world->get_ks_controller(active_player)->SetPlayerCamera(the_world->get_ks_controller(active_player)->get_photo_cam_ptr());
				player_cam[active_player]->frame_advance(0.1f);
				player_cam[active_player]->adjust_geometry_pipe();
			}
			else
				snapshotPrevCam = NULL;
		}

		// Render to a snapshot for photo challenges.
		render_snapshot();

		// If we took a snapshot in this frame, move the camera back to it's original position.
		if (snapshotState == SNAPSTATE_TEXTURE_RENDER+1 && !is_user_cam_on) // && snapshotPrevCam)
		{
			if (snapshotPrevCam)
			{
				// Go back to old camera before photo cam was applied.
				the_world->get_ks_controller(active_player)->SetPlayerCamera((game_camera *) snapshotPrevCam);
				set_current_camera(player_cam[active_player]);
				current_view_camera->adjust_geometry_pipe();
			}
			else
			{
				snapshotPrevCam = NULL;
			}
		}
	}
	#endif


	if (!is_splitscreen())
	{
			render_level_onescreen();
	}
	else
	{
		// SPLITSCREEN RENDERING BEGINS HERE
		render_level_splitscreen();
	}

	// Render weirdness.
	render_trippy_cheat();

	// Render in game overlays.
	if (snapshotState != SNAPSTATE_TEXTURE_RENDER)
	{
		render_igo();
	}

	//We need to render the flash as the last scene
#ifdef TARGET_GC
	if (snapshotState >= SNAPSTATE_TEXTURE_RENDER && snapshotState <= SNAPSTATE_FLASH_END)
	{
		nglListBeginScene();
		nglSetClearFlags(NGLCLEAR_COLOR | NGLCLEAR_Z | NGLCLEAR_STENCIL);
		nglSetClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		ksnglSetViewport(0, 0, nglGetScreenWidth(), nglGetScreenHeight());
		ksnglSetOrthoMatrix(nglGetScreenWidth() / 2, nglGetScreenHeight() / 2, 0.2f, 65536.0f);
		//nglSetWorldToViewMatrix(identity);
		nglListEndScene();
	}
#endif
	
	#ifndef TARGET_GC
	// Render to a snapshot for photo challenges.
	render_snapshot();

	// If we took a snapshot in this frame, move the camera back to it's original position.
	if (snapshotState == SNAPSTATE_TEXTURE_RENDER+1 && !is_user_cam_on && snapshotPrevCam)
	{
		// Go back to old camera before photo cam was applied.
		the_world->get_ks_controller(active_player)->SetPlayerCamera((game_camera *) snapshotPrevCam);
		set_current_camera(player_cam[active_player]);
		current_view_camera->adjust_geometry_pipe();
	}
	#endif

	do_profiler_stuff();
	render_mem_free_screen();

	#ifndef TARGET_GC
	// Render shadows.
	if (snapshotState != SNAPSTATE_TEXTURE_RENDER+1 && !is_splitscreen())
	{
		void WAVETEX_RenderShadows( camera *cam );
		WAVETEX_RenderShadows(current_view_camera);
	}
	#endif
	do_autobuild_stuff();
}


void game::render()
{
	ksnglSetPerspectiveMatrix(get_field_of_view(), nglGetScreenWidth()/2, nglGetScreenHeight()/2, 0.2f, 1200.0f);	 // set a decent far and near plane
	if(get_cur_state() == GSTATE_PLAY_INTRO_MOVIES || get_cur_state() == GSTATE_PLAY_FE_MOVIE)
	{
		return;
	}

	switch (get_render_state())
	{
	case GAME_RENDER_LOADING_LEVEL:
		render_map_screen(); break;
	case GAME_RENDER_INITALIZE_FE:
		render_legal_screen(); break;
	case GAME_RENDER_DRAW_FE:
		render_fe(); break;
	case GAME_RENDER_GAME_LEVEL:
		render_level(); break;
	default:
		assert("Hitting render loop in bad state");
		break;
	}

}

extern void nglDownloadTexture(nglTexture *Dest, nglTexture *Source);
extern void REFRACT_WriteRenderTargetToFile(nglTexture *Texture);

void game::render_snapshot(void)
{
	nglMatrix	identity;
#if defined(TARGET_PS2)
	sceVu0UnitMatrix((float (*)[4])&identity);
#else
	nglIdentityMatrix(identity);
#endif /* TARGET_XBOX JIV DEBUG */

#ifdef TARGET_GC

	if( !destSnapshot ) {
		return;
	}

	if( snapshotState == SNAPSTATE_TEXTURE_RENDER ) {
		nglListBeginScene( );
		nglSetRenderTarget( destSnapshot, true );
		nglSetClearFlags( NGLCLEAR_COLOR | NGLCLEAR_Z );

		ksnglSetViewport( 0, 0, game::SNAPSHOT_WIDTH, game::SNAPSHOT_HEIGHT );
		ksnglSetPerspectiveMatrix( proj_field_of_view_in_degrees( ), game::SNAPSHOT_WIDTH / 2, game::SNAPSHOT_HEIGHT / 2, 0.2f, 1200.0f );
		the_world->render( current_view_camera, 0 );
		nglListEndScene( );
	  DCInvalidateRange( destSnapshot->ImageData, destSnapshot->Width * destSnapshot->Height * 4 );
  }

#else
	nglQuad		q;

#if defined(TARGET_XBOX)
	if (!destSnapshot)
		return;
#else
	if (!snapshot || !destSnapshot)
		return;
#endif

	nglIdentityMatrix(identity);

	// First frame: Take snapshot (render backbuffer to a texture).
	if (snapshotState == SNAPSTATE_TEXTURE_RENDER)
	{
		float minwidthheight = min(nglGetScreenWidth(), nglGetScreenHeight());
		float uvwidth = minwidthheight / nglGetScreenWidth(), uvheight = minwidthheight / nglGetScreenHeight();

		// Initialize source quad.
#ifdef TARGET_XBOX
		nglTexture *destTex = destSnapshot;
#else
		nglTexture *destTex = snapshot;
#endif
		nglInitQuad(&q);
		nglTexture *Tex = nglGetBackBufferTex();
		nglSetQuadMapFlags(&q, NGLMAP_CLAMP_U | NGLMAP_CLAMP_V | NGLMAP_BILINEAR_FILTER);
		nglSetQuadTex(&q, Tex);
		nglSetQuadColor(&q, NGL_RGBA32(255, 255, 255, 255));

		// Sets the source texture's UVs to be the largest square that can fit.
#if !defined(TARGET_PS2)
		nglSetQuadUV(&q, (1.0f-uvwidth)/2.0f, (1.0f-uvheight)/2.0f, (1.0f+uvwidth)/2.0f, (1.0f+uvheight)/2.0f);
#else
		float ps2Scale = float(nglGetScreenHeight())/float(RoundUpToPowerOf2(nglGetScreenHeight()));
		nglSetQuadUV(&q, (1.0f-uvwidth)/2.0f, ((1.0f-uvheight)/2.0f)*ps2Scale, (1.0f+uvwidth)/2.0f, ((1.0f+uvheight)/2.0f)*ps2Scale);
#endif

		nglSetQuadRect(&q, 0, 0, destTex->Width, destTex->Height);
		nglSetQuadZ(&q, 1.0f);
		nglSetQuadBlend(&q, NGLBM_OPAQUE, 0);

		// Render back buffer to our texture.
		nglListBeginScene();
		nglSetRenderTarget(destTex, true);
		nglSetClearFlags(NGLCLEAR_Z | NGLCLEAR_STENCIL);
		nglSetClearZ(1.0f);
		ksnglSetViewport(0, 0, destTex->Width-1, destTex->Height-1);	// inclusive
		nglSetOrthoMatrix(destTex->Width / 2, destTex->Height / 2, 0.2f, 65536.0f);
		nglSetWorldToViewMatrix(identity);
		nglListAddQuad(&q);
		nglListEndScene();
	}

	// Second frame: download snapshot texture to system memory.
	if (snapshotState == SNAPSTATE_TEXTURE_DOWNLOAD)
	{
#if !defined (TARGET_XBOX)
		nglTexture source;
		source.GsBaseTBP = snapshot->GsBaseTBP;
		nglDownloadTexture(destSnapshot, &source);	// got this method from REFRACT_WriteRenderTargetToFile()
#endif
	}

	if (snapshotState >= SNAPSTATE_TEXTURE_RENDER && snapshotState <= SNAPSTATE_FLASH_END)
	{
		nglListBeginScene();
		nglSetClearFlags(NGLCLEAR_COLOR | NGLCLEAR_Z | NGLCLEAR_STENCIL);
		nglSetClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		//nglSetClearZ(65536.0f);
		ksnglSetViewport(0, 0, nglGetScreenWidth(), nglGetScreenHeight());
		ksnglSetOrthoMatrix(nglGetScreenWidth() / 2, nglGetScreenHeight() / 2, 0.2f, 65536.0f);
		nglSetWorldToViewMatrix(identity);
		nglListEndScene();
	}
#endif // #ifdef TARGET_GC

	// Advance snapshot state.
	if (snapshotState != SNAPSTATE_NONE)
	{
		snapshotState = SNAPSTATE(int(snapshotState)+1);
		if (snapshotState >= SNAPSTATE_END)
			snapshotState = SNAPSTATE_NONE;
	}
}

void game::take_snapshot(nglTexture * dest)
{
/*
if (numSnapshots >= MAX_SNAPSHOTS)
return;
	*/

	destSnapshot = dest;

	if (snapshotState == SNAPSTATE_NONE)
	{
		SoundScriptManager::inst()->playEvent(SS_CAMERA_SNAPSHOT);
		//frontendmanager.IGO->HideSnapshot();
		snapshotState = SNAPSTATE_BEGIN;
	}
}

void game::render_menu_screen()
{
	if( menu_screen && !os_developer_options::inst()->is_flagged( os_developer_options::FLAG_NO_LOAD_SCREEN ) )
	{
		material::flush_last_context();
		hw_rasta::inst()->begin_scene();

		matrix4x4 projection_matrix = geometry_manager::inst()->xforms[geometry_manager::XFORM_VIEW_TO_PROJECTION];
		matrix4x4 world_matrix = geometry_manager::inst()->xforms[geometry_manager::XFORM_WORLD_TO_VIEW];

		geometry_manager::inst()->set_cop(hw_rasta::inst()->get_screen_width()*0.5f,hw_rasta::inst()->get_screen_height()*0.5f, INTERFACE_MIN_Z, INTERFACE_MAX_Z);
		geometry_manager::inst()->set_xform(geometry_manager::XFORM_LOCAL_TO_WORLD, identity_matrix);
		geometry_manager::inst()->set_xform(geometry_manager::XFORM_WORLD_TO_VIEW, identity_matrix);

		matrix4x4 ortho;
		ortho.make_scale(vector3d(1.0f,1.0f/PROJ_ASPECT,1.0f));
		geometry_manager::inst()->set_xform(geometry_manager::XFORM_VIEW_TO_PROJECTION, ortho);

		clear_zbuffer();
#ifdef NGL
		nglListInit();
		menus->Draw();


#endif


		menu_screen->show();
		menu_screen->render();
		menu_screen->hide();

		g_world_ptr->get_matvertbufs().flush();

		// restore the projection matrix
		geometry_manager::inst()->set_xform(geometry_manager::XFORM_VIEW_TO_PROJECTION, projection_matrix);
		geometry_manager::inst()->set_xform(geometry_manager::XFORM_WORLD_TO_VIEW, world_matrix);

		hw_rasta::inst()->end_scene();

		hw_rasta::inst()->flip();
#ifdef NGL
		nglListSend();
		nglFlip();
#endif
	}
}


#include "beachdata.h"

void game::set_level( int lid )
{
	levelid = lid;
	beachid = CareerDataArray[levelid].beach;
}

void game::set_beach( int bid )
{
	levelid = LEVEL_LAST - 1; // I'm setting it to this because that's the last level and there are no goals
	beachid = bid;
}

void game::set_level(stringx lname, bool career)
{
	stringx cmpto=lname;
	cmpto.to_upper();
	if(career)
	{ // in career mode, so load a level
		for ( int i=0; i<LEVEL_LAST; i++ )
		{
			char *bname=CareerDataArray[i].name;
			assert(strlen(bname));
			stringx cmpi=stringx(bname);
			cmpi.to_upper();
			if ( cmpto==cmpi )
			{
				set_level(i);
				return;
			}
		}
		set_level(0); // use the first level in the database by default
	}
	else // not in career mode, so load a beach
	{
		for ( int i=0; i<BEACH_LAST; i++ )
		{
			char *bname=BeachDataArray[i].folder;
			assert(strlen(bname));
			stringx cmpi=stringx(bname);
			cmpi.to_upper();
			if ( cmpto==cmpi )
			{
				set_beach(i);
				return;
			}
		}
		set_beach(BEACH_BELLS);
	}
}

void game::set_movie( stringx mname )
{
  movie_name = mname;
}


bool game::is_competition_level(void)
{
	if(CareerDataArray[levelid].goal[0] >= GOAL_COMPETITION_1 &&
		CareerDataArray[levelid].goal[0] <= GOAL_COMPETITION_3 &&
		SurferDataArray[GetSurferIdx(0)].competitor)
		return true;
	else
		return false;
}

stringx game::get_level_name( void )
{
	stringx lx = stringx(CareerDataArray[get_level_id()].name);
	return lx;
}

stringx game::get_beach_name( void )
{
	stringx lx = stringx(BeachDataArray[get_beach_id()].folder);
	return lx;
}

stringx game::get_beach_stash_name( void )
{
	stringx lx = stringx(BeachDataArray[get_beach_id()].stashfile);
	return lx;
}



//void nglLoadDebugMeshes();
extern int g_debug_num_loads;

/*** load_level_stash ***/
void game::load_level_stash()
{
	if(current_loading_state == LOADING_COMMON_STASH)
	{
		//strcpy(nglHostPrefix,"");
		strcpy(g_scene_name, get_beach_name().c_str());
		/*
		if (stash::open_stash("common.st2", STASH_COMMON) == false)
		{
		debug_print("could not open common stash file common.st2\n");
		warning("could not open common stash file common.st2\n");
		}
		*/

		int stash_size = 1;
		if(current_loading_sub_state == 0)
		{
			stash_size = stash::pre_open_stash_for_async("common.st2", STASH_COMMON);
			stash::set_async_read_size(STASH_COMMON);
			if(stash_size == 0) current_loading_sub_state = -1;
			SetStashSize(LOADING_COMMON_STASH, stash_size);
		}

		if(stash_size != 0)
		{
			bool done = stash::read_stash_async(STASH_COMMON);
			if(done) current_loading_sub_state = -1;
		}
	}
	else if(current_loading_state == LOADING_BEACH_STASH)
	{
		if(current_loading_sub_state == 0)
		{
#ifdef USE_FILE_MANAGER
			if (file_manager::inst()->level_context_is_set() == false)
				file_manager::inst()->init_level_context("levels\\"+g_scene_name);
#endif

			//stringx stash_filename (os_developer_options::inst()->get_string(os_developer_options::STRING_STASH_NAME));
			stringx stash_filename = get_beach_stash_name();

			stash_filename= "BEACHES\\"+stash_filename;
			//level_prefix = get_beach_stash_name();
			level_prefix = "levels\\"+get_beach_name();
			//(os_developer_options::inst()->get_string(os_developer_options::STRING_STASH_NAME));
			stash_filename += PLATFORM_STASH_EXT;
			// open up the stash for this level
			stash a_stash;
			if (stash::using_stash() == true)
			{
				if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_STASH_ONLY))
				{
					//					stash::open_stash((char *)stash_filename.c_str(),STASH_BEACH);
					int stash_size = stash::pre_open_stash_for_async((char *)stash_filename.c_str(), STASH_BEACH);
					stash::set_async_read_size(STASH_BEACH);
					if(stash_size == 0) current_loading_sub_state = -1;
					SetStashSize(LOADING_BEACH_STASH, stash_size);
				}
			}
			else current_loading_sub_state = -1;
		}

		if (stash::using_stash() == true)
		{
			bool done = stash::read_stash_async(STASH_BEACH);
			if(done) current_loading_sub_state = -1;
		}
	}
}


#if 0
#define load_log(s) { low_level_console_print(s); low_level_console_flush(); }
#else
#define load_log(s) ((void)0)
#endif

int test_big_wave = 0;

/*** load_this_level ***/
void game::load_this_level()
{
	static int bob = 0;
	bob++;

	STOP_PS2_PC;
	switch(current_loading_state)
	{

	case LOADING_WORLD_CREATE:
		{
			nglPrintf("load level start\n");
#ifdef TARGET_XBOX
			gDVDCache.StopCaching();
#endif

			low_level_console_init();
			if ( the_world==NULL )
			{
				the_world = NEW world_dynamics_system;
				g_world_ptr = the_world;
			}

#if defined(TARGET_PS2) && defined(PROFILING_ON)
			// stop the performance counter to avoid overflow.
			{
				int control;
				control  = SCE_PC0_CPU_CYCLE | (SCE_PC_U0|SCE_PC_S0|SCE_PC_K0|SCE_PC_EXL0);
				control |= SCE_PC1_DCACHE_MISS | (SCE_PC_U1);
				scePcStart( control, 0, 0 );
			}
#endif

			assert(!flag.level_is_loaded);
		}	// loading state LOADING_WORLD_CREATE
		break;
	case LOADING_RESET_SOUND:
		{

			//load_log("game begining load level"); llc_memory_log();
			//low_level_console_print("Loading level, run #%d", g_debug_num_loads);  llc_memory_log();
			//  malloc_stats();
			//  sound_device::inst()->partial_shutdown(); // does a reset on the ps2

			//  Sound stuff
			// sounds & stashes aren't working together at the moment
			nslReleaseAllSounds();
			//nslSourceId src = nslLoadSource("WAVE_FACE01");

			//  sound_device::inst()->partial_init();   // does nothing on the ps2
			// load the default sound instance, for now
			//#pragma todo("Get the sound stuff on the ps2 working without this kludge. (GT-2/12/01)")
			//  stringx sd_name("SILENT");
			//  sound_device::inst()->load_sound(sd_name);
			//  stringx sd_name("SPSTEP1");
			//  si->play();
			//nslSoundId snd = nslAddSound(src);
			//nslPlaySound(snd);


			//#define HANDLE_PRAGMA (message)
			//#pragma todo("In the real game the level should be set by the front end. (EO-6/30/01)")
			//#pragma message("In the real game the level should be set by the front end. (EO-6/30/01)")
			//  stringx override_filename (os_developer_options::inst()->get_string(os_developer_options::STRING_SCENE_NAME));
			//	set_level(override_filename);

			// Fix so that we aren't streaming any sounds during loading.
			if ( !os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
			{
#if defined(TARGET_XBOX)
				nslSetRootDir("D:\\");
#endif

				if (!nslInit())
				{
#if defined(TARGET_XBOX) && defined(BUILD_FINAL)
					// Xbox requires a message on all disk read failures.  (dc 07/11/02)
					disk_read_error();
#endif
				}

#if defined(TARGET_PS2) || defined(TARGET_GC)
				nslSetRootDir("SOUNDS");
#elif defined(TARGET_XBOX)
				nslSetRootDir("D:\\SOUNDS");
#endif

				if (g_career)
					StoredConfigData::inst()->setGameConfig(&(g_career->cfg));
				nslReleaseAllSounds();
				// If it fails

#ifdef TARGET_PS2
				if (!nglUsingProView)
#endif

					if (!nslReset(BeachDataArray[get_first_beach()].stashfile, NSL_LANGUAGE_ENGLISH))
					{
						nglPrintf("WARNING.. NO BEACH SOUND BANK FOUND.  USING BELLS");
						nslReset("BELLS",NSL_LANGUAGE_ENGLISH);
					}
#if defined(DEBUG) && defined(TARGET_PS2) && !defined(PROFILING_ON)
			nslSetHostStreamingPS2(true);
#endif
					SoundScriptManager::inst()->init();
					SOUNDDATA_Load();
			}

		}	// loading state LOADING_RESET_SOUND
		break;

	case LOADING_COMMON_STASH:
	case LOADING_BEACH_STASH:
		{
			load_level_stash();
		}
		break;

	case LOADING_INIT_DBMENU:
		{
			load_log("finished loading level stash");
			KSDBMENU_InitMainMenu();

			//load_log("game done loading level stash"); llc_memory_log();
			if ( os_file::is_system_locked() )
				os_file::system_unlock();

			if( !os_developer_options::inst()->get_flag(os_developer_options::FLAG_FORCE_PC_MOUSE_OPERATION) )
				set_cursor( 0 );

			flag.wait_for_start_release = false;
			global_frame_counter = 0;
#ifdef GCCULL
			music_stream = NULL;
			music_fade_time_left = 0.0f;
#endif

			fog_manager::inst()->set_fog_distance( 0, 160.0f );
			fog_manager::inst()->set_fog_color( color( 0.0f, 0.0f, 0.0f, 1.0f ) );
			fog_manager::inst()->update_fog(true);

			/*for(int i=0;i<MAX_PLAYERS;i++)
				gamefile->set_heroname(i, getHeroname(i).c_str());*/

			if (p_blank_material != NULL)
			{
				hw_texture_mgr::inst()->set_white_texture(NULL);
				material_bank.delete_instance( p_blank_material );
				p_blank_material = NULL;
			}



			/////////////////////////////////
			// preload all the textures and materials needed by ingame interface and PFE stuff
#ifdef TARGET_PC
			p_blank_material = material_bank.new_instance( material(os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR)+"\\alpha") );
			hw_texture_mgr::inst()->set_white_texture(p_blank_material->get_texture(0));
#endif

			//  element_manager::inst()->restore_default_elements();

#ifdef TARGET_PS2
			// Create snapshot texture.
			snapshot = nglCreateTexture(NGLTF_32BIT | NGLTF_VRAM_ONLY, SNAPSHOT_WIDTH, SNAPSHOT_HEIGHT);
			snapshot->GsBaseTBP = NGL_VRAM_LOCKED_START;
#endif

			load_log("created snapshot data");

			if ( !os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
			{
				// restart nsl
				//      char soundfile[256];
				if (!(g_game_ptr->get_game_mode() == GAME_MODE_CAREER && g_game_ptr->get_beach_id() == BEACH_INDOOR))
					MusicMan::inst()->loadSources();

				MusicMan::inst()->unpause();


				sfx.init();

				// Initialize the voice over engine
				VoiceOvers.init();
				// nslSetVolume(NSL_SOURCETYPE_VOICE, 1.3);
				//VoiceOvers.addTrickSound(nslLoadSource("SIRALT"), );
				// Initialize the wave sound engine
				wSound.init();
			}

			// x2 to fill both buffers on the PC,
			// and to flash past a texture streaming bug on the PS2
			render_menu_screen();
			render_menu_screen();

#if _CONSOLE_ENABLE
			// initialize the console (macro)
			init_console();
#endif

		} // loading state LOADING_INIT_DBMENU
		break;

	case LOADING_SCENE:
	case LOADING_HERO_1_STASH:
	case LOADING_HERO_1_AUX_STASH:
	case LOADING_HERO_1_REST:
	case LOADING_HERO_2_STASH:
	case LOADING_HERO_2_AUX_STASH:
	case LOADING_HERO_2_REST:
	case LOADING_SCENE_END:
		{
			// load level and hero
			the_world->load_scene( level_prefix, /*gamefile->get_heroname(0) */  SurferDataArray[g_game_ptr->GetSurferIdx(0)].name);
		}
		break;

	case LOADING_GAME_MODE:
		{
	    ksreplay.Clear(g_random_r_ptr->srand());   // Set the random number seed and save it in replay
      ksreplay.Record();

			load_log("loaded scene");

			// Load AI surfer.
			if (g_game_ptr->get_game_mode() == GAME_MODE_CAREER &&
				os_developer_options::inst()->is_flagged(os_developer_options::FLAG_AI_SURFERS))
			{
				for (int i=0; i < 5; i++)
				{
					if ((CareerDataArray[get_level_id()].goal[i] == GOAL_LOCAL) &&
						g_game_ptr->get_num_ai_players() == 0 &&
						CareerDataArray[get_level_id()].use_ai_surfer)
					{
						int randsurfer = random(SURFER_LAST);
						while (randsurfer == GetSurferIdx(0))
							randsurfer = random(SURFER_LAST);
						set_num_ai_players(1);
						SetSurferIdx(1, 10);
						if (BeachDataArray[g_game_ptr->get_beach_id()].use_wetsuit)
						{
							SetUsingPersonalitySuit(1, 1);
							g_world_ptr->load_ai_hero(AI_JOYSTICK, SURFER_MISC_SURFER,1);
						}
						else
						{
							SetUsingPersonalitySuit(1, 0);
							g_world_ptr->load_ai_hero(AI_JOYSTICK, SURFER_MISC_SURFER,0);
						}


						continue;
					}
				}
			}

			// Allocate and initialize play mode data. (must come after world loading)
			if (game_mode == GAME_MODE_PUSH)
			{
				play_mode.push = NEW PushMode;
				play_mode.push->Initialize(the_world->get_ks_controllers());
				play_mode.push->SetDifficulty(multiplayer_difficulty);
			}
			else if (game_mode == GAME_MODE_TIME_ATTACK)
			{
				play_mode.timeAttack = NEW TimeAttackMode;
				play_mode.timeAttack->Initialize(the_world->get_ks_controllers());
				play_mode.timeAttack->SetDifficulty(multiplayer_difficulty);
			}
			else if (game_mode == GAME_MODE_METER_ATTACK)
			{
				play_mode.meterAttack = NEW MeterAttackMode;
				play_mode.meterAttack->Initialize(the_world->get_ks_controllers());
			}
			else if (game_mode == GAME_MODE_HEAD_TO_HEAD)
			{
				play_mode.headToHead = NEW HeadToHeadMode;
				play_mode.headToHead->Initialize(the_world->get_ks_controllers());
			}

			//	Determine player viewports.
			if (num_players == 1)
			{
				player_viewports[0].set_left(0);
				player_viewports[0].set_top(0);
				player_viewports[0].set_right(nglGetScreenWidth() - 1);
				player_viewports[0].set_bottom(nglGetScreenHeight() - 1);
			}
			else if (num_players == 2)
			{
				if (num_active_players == 1)
				{
					player_viewports[0].set_left(0);
					player_viewports[0].set_top(0);
					player_viewports[0].set_right(nglGetScreenWidth() - 1);
					player_viewports[0].set_bottom(nglGetScreenHeight() - 1);

					player_viewports[1].set_left(0);
					player_viewports[1].set_top(0);
					player_viewports[1].set_right(nglGetScreenWidth() - 1);
					player_viewports[1].set_bottom(nglGetScreenHeight() - 1);
				}
				// Must come after play_mode.push->Initialize above (dc 07/09/02)
				else if (game_mode == GAME_MODE_PUSH)
				{
					for (int i = 0; i < MAX_PLAYERS; i++)
						player_viewports[i] = play_mode.push->GetPlayerViewport(i);
				}
				// Must come after play_mode.headToHead->Initialize above (dc 07/09/02)
				else if (game_mode == GAME_MODE_HEAD_TO_HEAD)
				{
					float left, right, top, bottom;
					left = 0, top = 0, right = 319, bottom = 479;
					adjustCoords(left, top);
					adjustCoords(right, bottom);
					player_viewports[0].set_left(left);
					player_viewports[0].set_top(top);
					player_viewports[0].set_right(right);
					player_viewports[0].set_bottom(bottom);

					left = 320, top = 0, right = 639, bottom = 479;
					adjustCoords(left, top);
					adjustCoords(right, bottom);
					player_viewports[1].set_left(left);
					player_viewports[1].set_top(top);
					player_viewports[1].set_right(right);
					player_viewports[1].set_bottom(bottom);
				}
				else
					assert(false);
			}
			else
				assert(false);

			// Reset timers.  (must come after game mode initialization)
			if (game_mode == GAME_MODE_TIME_ATTACK)
				TIMER_Init(play_mode.timeAttack->GetLevelDuration(active_player));
			else if (game_mode == GAME_MODE_METER_ATTACK)
				TIMER_Init(play_mode.meterAttack->GetLevelDuration(active_player));
			else if ((g_game_ptr->get_game_mode() == GAME_MODE_CAREER && g_game_ptr->get_beach_id() == BEACH_INDOOR)
				|| game_mode == GAME_MODE_FREESURF_INFINITE || game_mode == GAME_MODE_FREESURF_ICON)
				TIMER_Init(0);	// infinite time for freesurf, tutorial (dc 06/10/02)
			else
				TIMER_Init();

			//Open the level stash
			if ((g_game_ptr->get_game_mode() == GAME_MODE_CAREER)
				&& (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_STASH_ONLY)))
			{
				if (strlen(CareerDataArray[g_game_ptr->get_level_id()].stash_name))
				{
					char name[40];
					strcpy(name, (stringx("CHALLENG\\") + CareerDataArray[g_game_ptr->get_level_id()].stash_name).c_str());
					stash::open_stash(name, STASH_LEVEL);
				}
			}

			//Icon stash for freesurf icon
			if ((g_game_ptr->get_game_mode() == GAME_MODE_FREESURF_ICON)
				&& (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_STASH_ONLY)))
			{
				stash::open_stash("CHALLENG\\ICON.ST2", STASH_LEVEL);
			}

			// moved from kellyslater_controller
			g_beach_ptr = NEW beach;
			g_beach_ptr->load ();	// must come after TIMER_Init (dc 01/23/02)
			g_beach_ptr->reset();
			load_log("loaded beach");
			kellyslater_controller **ksctls = g_world_ptr->get_ks_controllers();

			if (g_game_ptr->get_num_players() + g_game_ptr->get_num_ai_players() == 2)
			{
				BOARD_ReloadTextures(ksctls[0]->get_board_controller().my_board_model,
					ksctls[1]->get_board_controller().my_board_model,
					g_game_ptr->GetSurferIdx(0), g_game_ptr->GetBoardIdx(0),
					g_game_ptr->GetSurferIdx(1),g_game_ptr->GetBoardIdx(1));
			}
			else
			{
				BOARD_ReloadTextures(ksctls[0]->get_board_controller().my_board_model,
					NULL,
					g_game_ptr->GetSurferIdx(0),g_game_ptr->GetBoardIdx(0));
			}

			// IGO must be loaded after the surfer.
			frontendmanager.InitIGO();
			load_log("loaded IGO");

			for (int i = 0; i < num_players; i++)
				g_world_ptr->get_ks_controller(i)->get_board_controller().InitConstants();

			WATER_Init();	// must come after TIMER_Init and g_beach_ptr->load (dc 01/23/02)
							/*	Attempts at screen spray (dc 01/04/02)
							REFRACT_Init();
							DISPLACE_Init();
			*/
			BLUR_Init();

			//load_level_stash();

			flag.goal_completed = false;

			kellyslater_controller *ksctrl = the_world->get_ks_controller(0);

			//  Certain beaches need specific cameras.
/*			if (BeachDataArray[get_beach_id()].is_big_wave)
			{
				ksctrl->SetPlayerCamera((game_camera *) find_camera(entity_id("BIG_WAVE_CAM0")));
				set_current_camera(find_camera(entity_id("BIG_WAVE_CAM0")));
			}
			else
*/			{
				ksctrl->SetPlayerCamera((game_camera *) find_camera(entity_id(StoredConfigData::inst()->getLastCamera(0))));
				set_current_camera(player_cam[0]);
			}

			if (the_world->get_hero_ptr(1) && the_world->get_ks_controller(1))
			{
				kellyslater_controller *ksctrl2 = the_world->get_ks_controller(1);
				ksctrl2->SetPlayerCamera((game_camera *) find_camera(entity_id("BEACH_CAM1")));		// player 2 default cam
				//player_cam[1] = find_camera(entity_id("BEACH_CAM1"));		// player 2 default cam
			}

			// Some challenges use a different camera.
			if (game_mode == GAME_MODE_CAREER && g_beach_ptr->get_challenges()->icon)
			{
				ksctrl->SetPlayerCamera((game_camera *) find_camera(entity_id("FOLLOW_CAM0")));
				set_current_camera(find_camera(entity_id("FOLLOW_CAM0")));
			}

			/// Start flyby.
			#if 1 // def TARGET_GC
				// the game cube fly-ins look like poo... doo dah doo dah -EO
			if ((game_mode == GAME_MODE_CAREER && g_game_ptr->get_num_players() == 1) ||
				os_developer_options::inst()->is_flagged (os_developer_options::FLAG_MAKE_MOVIE))	// I'm gonna kill Evan for breaking this, oh the doo dah day. -JMB
			#else
			if (g_game_ptr->get_num_players() == 1)
			#endif
			{
				set_active_player(0);

				//  Check to see if there is a flby for this beach.  If so, let the game structure know that the camera has changed.
				if (ksctrl->StartFlyby())
				{
					set_current_camera(find_camera(entity_id("FLYBY_CAM0")));
					// Pause a bunch of stuff until flyby is over
					if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
					{
						if (level_is_loaded())
						{
							wSound.pause();
							VoiceOvers.pause();
							sfx.pause();
						}
						/*if (nslGetSoundStatus(MusicMan::inst()->musicTrack.getSoundId()) == NSL_SOUNDSTATUS_PLAYING)
						nslPauseGuardSound(MusicMan::inst()->musicTrack.getSoundId());*/
						SoundScriptManager::inst()->pause();
						MusicMan::inst()->pause();
						nslPauseAllSounds();
					}

					if (level_is_loaded())
						rumbleMan.pause();

					WAVE_OnFlybyStart();	// Set the wave to its mature state.  (dc 07/08/02)
				}
			}

			// Toby debug: surf in photo cam.
			//ksctrl->SetPlayerCamera(ksctrl->get_photo_cam_ptr());

			// let the chase camera get into position before we run scripts
			current_view_camera->frame_advance( 0.1f );


    } // loading state LOADING_GAME_MODE
    break;

  case LOADING_LENS_FLARE:
	  {


		  // SETUP LENS FLARES
		  //===============================================================
		  if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_SHOW_LENSFLARE))
		  {
			  vector3d v;

			  v.x = BeachDataArray[get_beach_id()].lensflarex;
			  v.y = BeachDataArray[get_beach_id()].lensflarey;
			  v.z = BeachDataArray[get_beach_id()].lensflarez;

			  if (!((v.x == 0.0f) && (v.y == 0.0f) && (v.z == 0.0f)))
			  {
				  //      entity *sun;
				  //      po sunPo;
				  entity_id lf;
				  lf.set_entity_id("LENSFLARE");
				  //      entity *pos;
				  chunk_file f, g;

				  f.open("levels\\" + stringx(BeachDataArray[get_beach_id()].folder) + "\\ENTITIES\\lensflare.ent");
				  sunFlare = NEW lensflare(f, lf, to_entity_flavor_t("LENSFLARE"),EFLAG_GRAPHICS_VISIBLE | EFLAG_GRAPHICS);
				  g_world_ptr->add_lensflare(sunFlare);
				  g_world_ptr->get_the_terrain().get_region(0).add(sunFlare);
				  sunFlare->set_rel_position(v);

			  }
		  }

		  load_log("loaded lens flare");

	  }	// loading state LOADING_LENS_FLARE
	  break;

  case LOADING_SCRIPT:
	  {

		  //===============================================================

#ifndef PROJECT_KELLYSLATER

		  //load_log("game before script preload"); llc_memory_log();

		  // at this point, we must link and run the special "preload" script objects
		  script_manager* scriptman = the_world->get_script_manager();
		  script_object* common_preload = scriptman->find_object( "_common_preload" );
		  if ( common_preload )
		  {
			  common_preload->link( *scriptman );
			  common_preload->create_auto_instance();
			  while ( common_preload->has_threads() )
				  common_preload->run(USE_SUSPENDED);
		  }
		  script_object* local_preload = scriptman->find_object( "_local_preload" );
		  if ( local_preload )
		  {
			  local_preload->link( *scriptman );
			  local_preload->create_auto_instance();
			  while ( local_preload->has_threads() )
				  local_preload->run(USE_SUSPENDED);
		  }

		  // run all scripts for a single frame, so that constructors can load all the
		  // assets they need before we do the autosave and/or create a binary image;
		  // *** an error message will be generated if any sound or stream is started
		  // playing during this first "frame" of script execution
		  //sound_device::inst()->lock_playback_system();
		  scriptman->run(0.0f, IGNORE_SUSPENDED);
		  //sound_device::inst()->unlock_playback_system();

		  // we also need to run preload scripts for all loaded items
		  the_world->spawn_misc_preload_scripts();
#endif


		  // all doors have by now been attached to portals so we
		  // can recompute sectors properly
		  the_world->recompute_all_sectors();

		  os_file::system_lock();  // no file access allowed henceforth, unless specifically permitted by calling os_file::system_unlock()

#ifdef TARGET_PS2
		  if ( os_developer_options::inst()->get_int(os_developer_options::INT_RECORD_DEMO) > 0 )
		  {
			  int demo_number = os_developer_options::inst()->get_int(os_developer_options::INT_RECORD_DEMO);
			  stringx demo_filename(stringx::fmt, "%s.dm%d", g_scene_name, demo_number);
			  g_pad->record_demo_start(demo_filename);
		  }
		  else if ( os_developer_options::inst()->get_int(os_developer_options::INT_PLAYBACK_DEMO) > 0 )
		  {
			  int demo_number = os_developer_options::inst()->get_int(os_developer_options::INT_PLAYBACK_DEMO);
			  stringx demo_filename(stringx::fmt, "%s.dm%d", g_scene_name, demo_number);
			  g_pad->playback_demo_start(demo_filename);
		  }


#endif

	  } // loading state LOADING_SCRIPT
	  break;

  case LOADING_PERFORMANCE:
	  {


#if defined(TARGET_PS2) && defined(PROFILING_ON)
		  // restart the performance counters.
		  {
			  int control;
			  control  = SCE_PC0_CPU_CYCLE | (SCE_PC_U0|SCE_PC_S0|SCE_PC_K0|SCE_PC_EXL0);
			  control |= SCE_PC1_DCACHE_MISS | (SCE_PC_U1);
			  control |= SCE_PC_CTE;
			  scePcStart( control, 0, 0 );
		  }
#endif

		  // lock the file-system down and turn the disk over to streaming
		 	stash::close_stash(STASH_BEACH );
	    stash::close_stash(STASH_COMMON);
	    stash::close_stash(STASH_SURFER);
	    stash::close_stash(STASH_SURFER_AUX);
	    stash::close_stash(STASH_SURFER_BOARD);
	    stash::close_stash(STASH_SURFER_2);
	    stash::close_stash(STASH_SURFER_2_AUX);
	    stash::close_stash(STASH_SURFER_2_BOARD);
	    stash::close_stash(STASH_LEVEL);

#ifdef USE_FILE_MANAGER
		  file_manager::inst()->lock_file_system();
#endif

		  load_log("finished loading main data");


		  if ( !os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
		  {
			  MusicMan::inst()->playNext();

		  }
		  rumbleMan.init();
		  //      flag.level_is_loaded = true;

		  load_log("finished loading audio");

#ifdef USINGSTATICSTLALLOCATIONS
		  entity_anim::check_mem_init();
		  entity_anim_tree::check_mem_init();
		  po_anim::check_mem_init();
		  linear_anim<quaternion>::check_mem_init();
		  linear_anim<vector3d>::check_mem_init();
		  linear_anim<rational_t>::check_mem_init();
		  entity::movement_info::check_mem_init();
#endif


		  if(get_game_mode() == GAME_MODE_CAREER)
		  {
			  g_career->SetCurrentBeach(get_beach_id());
			  most_recent_level = get_level_id();
		  }
		  else most_recent_level = -1;
		  most_recent_beach = get_beach_id();

		  nglPrintf("load level end.\n");
		  // Print out info on the heap.
		  low_level_console_print("finished loading level");  llc_memory_log();
		  //  malloc_stats();

		  //  Reset the special meter timer.
		  kellyslater_controller *ksctrl = the_world->get_ks_controller(0);
		  ksctrl->get_special_meter()->SetUpSpecialTimer();
	  }	// loading state LOADING_PERFORMANCE
	  break;
  }
  START_PS2_PC;
}


/*** unload_current_level ***/
void game::unload_current_level()
{
	int i;
	mem_lock_malloc(false);
	set_render_state(GAME_RENDER_LOADING_LEVEL);
	debug_print("unload level start\n");

	assert(the_world);

	for(i=0;i<MAX_PLAYERS;i++ )
	{
		if(the_world->get_ks_controller(i))
			the_world->get_ks_controller(i)->get_my_scoreManager().Reset();
	}


#ifdef USE_FILE_MANAGER
	file_manager::inst()->unlock_file_system();
#endif
#ifdef TARGET_PS2
	//  malloc_stats();
	//  sound_device::inst()->stop_all_instances();
	//  sound_device::inst()->set_master_volume(0.0f);

	if ( os_developer_options::inst()->get_int(os_developer_options::INT_RECORD_DEMO) > 0 )
		g_pad->record_demo_stop();
	else if ( os_developer_options::inst()->get_int(os_developer_options::INT_PLAYBACK_DEMO) > 0 )
		g_pad->playback_demo_stop();
#endif

#ifndef TARGET_GC
	// Clear snapshot memory.
	snapshotState = SNAPSTATE_NONE;
	if (snapshot)
	{
		nglDestroyTexture(snapshot);
		snapshot = NULL;
	}
	/*
	for (int i = 0; i < MAX_SNAPSHOTS; i++)
	{
    if (snapshots[i])
    {
	nglDestroyTexture(snapshots[i]);
	//nglMemFree(snapshots[i]->Data);
	//delete snapshots[i];
	snapshots[i] = NULL;
	snapshotScores[i] = 0;
    }
	}
	numSnapshots = 0;
	*/
#endif
	for(i=0;i<MAX_PLAYERS;i++)
	{
		player_cam[i]=NULL;
	}
	current_view_camera = NULL;
	is_user_cam_on = false;
	was_user_cam_ever_on = false;

	// Release play mode data.
	delete play_mode.push;
	play_mode.push = NULL;
	delete play_mode.timeAttack;
	play_mode.timeAttack = NULL;
	delete play_mode.meterAttack;
	play_mode.meterAttack = NULL;
	delete play_mode.headToHead;
	play_mode.headToHead = NULL;

	IGORelease();
	/*	Attempts at screen spray (dc 01/04/02)
	REFRACT_Cleanup();
	*/

	ksreplay.Term();
	hit_list.clear ();
	normal_list1.clear ();
	normal_list2.clear ();

	fog_manager::inst()->set_fog_distance( 0, 160.0f );
	fog_manager::inst()->set_fog_color( color( 0.0f, 0.0f, 0.0f, 1.0f ) );
	fog_manager::inst()->update_fog(true);

	os_file::system_unlock();

	wSound.shutdown();
	if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
	{
		sfx.shutdown();
		SoundScriptManager::inst()->clearEvents();
		MusicMan::inst()->stop();
		nslReleaseAllSounds();

	}
	rumbleMan.shutdown();
	memcpy(&g_career->cfg, StoredConfigData::inst()->getGameConfig(), sizeof(ksConfigData));
	if ( !os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
		nslShutdown();
	flag.level_is_loaded = false;

	freeze_hero(false);

	// clear the global sound group list.
#ifdef GCCULL
	sound_groups.resize(0);
#endif

	g_entity_maker->purge_entity_cache();

	trigger_manager::inst()->purge();
#ifdef KSCULL
	script_pad[0].clear_callbacks();
	script_pad[1].clear_callbacks();
	m_script_mfg.clear_callbacks();
#endif

	anim_id_manager::inst()->stl_dealloc();
	WAVE_Unload();

	// moved from kellyslater_controller
	delete g_beach_ptr;
	g_beach_ptr = NULL;

	//
	// PUNY MORTALS! I will destroy your world! Muahahahahaha.
	//

	the_world->unload_scene();

	delete the_world;
	the_world = NULL;
	g_world_ptr = NULL;

	DestroyCameraList();
	ks_fx_destroy_all();

	g_file_finder->init_path_list();

#ifdef GCCULL
	clear_ambients();
#endif

	//base_camera = NULL;
#ifdef GCCULL
	music_stream = NULL;
#endif

#if defined(TARGET_PS2) && !defined(BUILD_FINAL)
	malloc_stats();
#endif

	////////////////////////////////////

#if _CONSOLE_ENABLE
	// destroy the console (macro)
	destroy_console();
#endif

	anim_id_manager::inst()->purge();
	entity_manager::inst()->purge();

	vr_pmesh_bank.purge();
	vr_pmesh_bank.debug_dump();

	hw_texture_mgr::inst()->unload_all_textures();

	slc_manager::inst()->purge();

	cg_mesh_bank.purge();

	//MJDFIXME
	//entity_track_bank.purge();

	// stuff with materials has to go before materials
	vr_billboard_bank.purge();
	vr_pmesh_bank.purge();
	material_bank.purge();

	// PEH TEST 3 lines
	//  sound_device::inst()->partial_shutdown();
	//  sound_device::inst()->partial_init();

	// restore default elements
	//#pragma todo("I think not re-instating the element manager causes the NY2 crash, but we need")
	//#pragma todo("to do some redesign work on it to make stash-only mode co-exist with this code. (GT-2/3/01)")

	//  assert(element_manager::inst()->get_num_context() == 0);
	//  element_manager::inst()->push_context( "GAME" );
	//  element_manager::inst()->restore_default_elements();

	// don't need to manually unload preloaded materials and textures
	// due to the above purging
	hw_texture_mgr::inst()->set_white_texture(NULL);
	if (p_blank_material)
	{
		p_blank_material = NULL;
	}

#ifdef GCCULL
	stop_music(false);
#endif
	flag.level_is_loaded = false;

#ifdef NGL
	low_level_console_release();
	/*#if defined(TARGET_GC)
	nglReleaseAllMeshes();
#else*/
	nglReleaseAllMeshFiles();
	//#endif // defined(TARGET_XBOX)
#if defined(TARGET_PS2)
#if !defined(BUILD_FINAL)
	malloc_stats();
#endif
	nglReleaseAllFonts();
#endif

	nglReleaseAllTextures();
#endif
	stash::free_stored(STASH_LEVEL);
	stash::free_stored(STASH_SURFER_2);
	stash::free_stored(STASH_SURFER_2_AUX);
	stash::free_stored(STASH_SURFER_2_BOARD);
	stash::free_stored(STASH_SURFER);
	stash::free_stored(STASH_SURFER_AUX);
	stash::free_stored(STASH_SURFER_BOARD);
	stash::free_stored(STASH_BEACH );
	stash::free_stored(STASH_COMMON);

	app::cleanup_stl_memory_dregs();

	// clear out the stored portion
	//stash::free_stored();

#ifdef USE_FILE_MANAGER
	file_manager::inst()->clear_level_context();
#endif
	debug_print("unload level end\n");
#ifdef TARGET_PS2
	//  sound_device::inst()->resize(0);
	//  malloc_stats();
#endif

	//  mem_check_leaks( "" );

	KSDBMENU_KillMainMenu();

}


float game_speed=1.0f;

void game::frame_advance_level()
{
	input_mgr* inputmgr=input_mgr::inst();

	// Spiderman
	// BIGCULL   update_pc_2_playstation_inputs();
#ifdef KSCULL
	script_pad[0].update();
	script_pad[1].update();
#endif

	// TO AVOID UNPAUSE-REPAUSE-UNPAUSE-REPAUSE MADNESS
	if( flag.wait_for_start_release )
	{
		if( inputmgr->get_control_state( JOYSTICK_DEVICE, GAME_PAUSE ) != AXIS_MAX )
			flag.wait_for_start_release = false;
	}

	time_value_t time_inc = real_clock.elapsed_and_reset();

	// WTB - enable this to force the game to update at a constant rate.  useful for debugging things that
	// only happen on a certain frame.
	int frame_lock = os_developer_options::inst()->get_int( os_developer_options::INT_FRAME_LOCK );
	if( frame_lock > 0)
		time_inc = 1.0f/(float)frame_lock;

	time_inc *= game_speed;

#if defined(TARGET_XBOX)
	assert(time_inc > 0.0f);
#endif /* TARGET_XBOX JIV DEBUG */

	assert(time_inc>=0 && time_inc<1e9f);
	// cap frame time delta, at most a tenth of a second passes,
	// no matter how much time went by

		/*	This duplicates what's already being done in blur.cpp (dc 04/05/02)
		wipeTrans.tick(time_inc);
	*/

#ifndef TARGET_GC
	// Check to see if the controller has been disconnected
	// GameCube doesn't display this because the wireless
	// controller can simulate disconnects when changing
	// "channels".
	static bool was_disconnected = false;  // so we only do it once per disconnect
	if( level_is_loaded() )
	{
		if( get_num_active_players() == 1 )
		{
			device_id_t controller = the_world->get_ks_controller( get_active_player() )->get_joystick_num();
			disconnect_status = !inputmgr->get_device( controller )->is_connected();
		}
		else if( get_num_active_players() == 2 )
		{
			device_id_t controller_1 = the_world->get_ks_controller( 0 )->get_joystick_num();
			device_id_t controller_2 = the_world->get_ks_controller( 1 )->get_joystick_num();
			bool connected_1 = inputmgr->get_device( controller_1 )->is_connected();
			bool connected_2 = inputmgr->get_device( controller_2 )->is_connected();

			if( !connected_1 || !connected_2 )
				disconnect_status = true;
			else
				disconnect_status = false;
		}
	}
	else
		disconnect_status = false;

	if( disconnect_status && !is_paused() && !was_disconnected && level_is_loaded() )
	{
		was_disconnected = frontendmanager.pms->SetDisconnect( true );
		if( was_disconnected )
		{
			frontendmanager.pms->startDraw();
#ifdef TARGET_XBOX
			frontendmanager.pms->MakeActive(PauseMenuSystem::LostControllerMenu);
#endif
		}
	}
	else
		was_disconnected = false;
#endif

	assert(time_inc>=0 && time_inc<1e9f);

	time_value_t max_time = 0.1F;

	//  Lock the framerate for making the flyby movie.
	if (os_developer_options::inst()->is_flagged (os_developer_options::FLAG_MAKE_MOVIE))
		max_time = 1.0f / 30.0f;

	if( time_inc > max_time )
	{
		time_inc = max_time;
	}

	assert(time_inc>=0 && time_inc<10.0f);

	if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_DEMO_MODE))
		dmm.tick(time_inc);


#if defined(TARGET_XBOX)
	assert(time_inc > 0.0f);
#endif /* TARGET_XBOX JIV DEBUG */

	if (time_inc)
	{
		if (!is_paused())
			game_clock::frame_advance(time_inc);
	}

	if (flag.i_restart)
	{
		os_file::system_unlock();
		gamefile->init_cur_data();

		{
			unload_current_level();
		}
	}

	assert(time_inc>=0 && time_inc<10.0f);

#if defined(TARGET_XBOX)
	assert(time_inc > 0.0f);
#endif /* TARGET_XBOX JIV DEBUG */

	//////////////////////// GAME STATE SWITCH /////////////////////////////////////////////
	switch ( get_cur_state() )
	{
	case GSTATE_PLAY_INTRO_MOVIES:
		advance_state_startup( time_inc );
		break;

	case GSTATE_FRONT_END:
		advance_state_front_end( time_inc );
		break;

	case GSTATE_LOAD_LEVEL:
		advance_state_load_level( time_inc );
		break;

	case GSTATE_PLAY_MOVIE:
		advance_state_play_movie( time_inc );
		break;

	case GSTATE_PLAY_FE_MOVIE:
		advance_state_play_fe_movie( time_inc );
		break;

	case GSTATE_PAUSED:
		advance_state_paused( time_inc );
		break;

	case GSTATE_RUNNING:
		advance_state_running( time_inc );
		break;

	case GSTATE_EMPTY_STATE:
		go_next_state();
		break;

	case GSTATE_POP_PROCESS:
		pop_process();
		break;

	default:
		assert(0);
		break;
	}
}



//--------------------------------------------------------------
extern profiler_counter profcounter_lod_tri_estimate;


typedef enum
{
	CHEAT_1,
		CHEAT_2,
		CHEAT_COUNT
} KS_CHEATS;

static control_id_t cheats[CHEAT_COUNT][5] =
{
	{ PSX_X, PSX_CIRCLE, PSX_TRIANGLE, PSX_SQUARE, INVALID_CONTROL_ID },
	{ PSX_SQUARE, PSX_TRIANGLE, PSX_CIRCLE, PSX_X, INVALID_CONTROL_ID },
};

static control_id_t *current_cheat = NULL;
static float cheat_timer;
static bool cheat_release;

static bool get_one_button_down (control_id_t& btn)
{
	input_mgr* inputmgr = input_mgr::inst();
	btn = INVALID_CONTROL_ID;

	for (int i = PSX_X; i <= PSX_SELECT; i++)
	{
		if (inputmgr->get_control_state(ANY_LOCAL_JOYSTICK, i) != 0.0f)
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

void game::do_the_cheat_codes(float time_inc)
{
	control_id_t btn;

	if (get_one_button_down (btn))
	{
		if (current_cheat)
		{
			if (*current_cheat == INVALID_CONTROL_ID)
			{
				mb->post (stringx ("CHEAT"), 2.0f);
				current_cheat = NULL;
			}
			else
			{
				if (btn == INVALID_CONTROL_ID)
				{
					if (cheat_release)
					{
						current_cheat++;
						cheat_timer = 0;
						cheat_release = false;
					}
					else
					{
						cheat_timer += time_inc;
						if (cheat_timer > CHEAT_SPEED)
							current_cheat = NULL;
					}
				}
				else if (btn == *current_cheat)
				{
					if (!cheat_release)
					{
						cheat_timer = 0;
						cheat_release = true;
					}
					else
					{
						cheat_timer += time_inc;
						if (cheat_timer > CHEAT_SPEED)
							current_cheat = NULL;
					}
				}
				else
				{
					current_cheat = NULL;
				}
			}
		}
		else if (btn != INVALID_CONTROL_ID)
		{
			for (int i = 0; i < CHEAT_COUNT; i++)
				if (cheats[i][0] == btn)
				{
					current_cheat = &cheats[i][0];
					cheat_timer = 0;
					cheat_release = true;
					break;
				}
		}
	}



	if ( input_mgr::inst()->get_control_delta( JOYSTICK_DEVICE, DO_MALOR_NEXT ) == AXIS_MAX )
	{
		the_world->malor_next();
	}
	else if(input_mgr::inst()->get_control_delta( JOYSTICK_DEVICE, DO_MALOR_PREV ) == AXIS_MAX)
	{
		the_world->malor_prev();
	}

	if ( input_mgr::inst()->get_control_delta( JOYSTICK_DEVICE, PLAYER_FULL_HEAL ) == AXIS_MAX ||
		(
		input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, PLAYER_STRAFE_LEFT ) > (2*AXIS_MAX)/3 &&
		input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, PLAYER_STRAFE_RIGHT ) > (2*AXIS_MAX)/3 &&
		input_mgr::inst()->get_control_delta( JOYSTICK_DEVICE, CHEAT_FULL_HEAL ) == AXIS_MIN &&
		input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, CHEAT_FULL_HEAL ) == AXIS_MIN &&
		input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, CHEAT_MALOR ) == 0 &&
		input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, CHEAT_I_WIN ) == 0 &&
		is_paused()
		)
		)
	{
		mb->post(stringx("Full Heal!"),2.0f);
	}

	if ( input_mgr::inst()->get_control_delta( JOYSTICK_DEVICE, PLAYER_RESET_POSITION ) == AXIS_MAX )
	{
	/*
	entity * hero = entity_manager::inst()->find_entity(entity_id("HERO"), IGNORE_FLAVOR);
	entity * start = entity_manager::inst()->find_entity(entity_id("HERO_START"), IGNORE_FLAVOR);
	hero->set_rel_position( start->get_abs_position() );
		*/
	}

#ifdef DEBUG
	if( input_mgr::inst()->get_control_delta( JOYSTICK_DEVICE, SINGLE_STEP ) == AXIS_MAX &&
		input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, PLANE_BOUNDS_MOD ) == AXIS_MAX )
	{
		g_debug.render_spheres = !g_debug.render_spheres;
	}
#endif

	if ( input_mgr::inst()->get_control_delta( JOYSTICK_DEVICE, TOGGLE_BSP_SPRAY_PAINT ) == AXIS_MAX )
	{
		os_developer_options::inst()->toggle_flag( os_developer_options::FLAG_BSP_SPRAY_PAINT );

		entity *space_filler = g_world_ptr->get_entity("SPACE_FILLER");

		if (space_filler)
		{
			bool enable=os_developer_options::inst()->is_flagged(os_developer_options::FLAG_BSP_SPRAY_PAINT);
			space_filler->set_active(enable);
			space_filler->set_visible(enable);
		}
	}
}

#ifdef GCCULL
sg_entry* game::get_sound_group_entry( const pstring& name )
{
	map<pstring,sound_group*>::iterator it = sound_group_map.find( name );
	if ( it == sound_group_map.end() )
		return 0;
	else
		return ( *it ).second->get_next();
}
#endif

//--------------------------------------------------------------
void game::pause()
{
	if (is_paused())
		return;


	flag.game_paused = true;
#ifndef PROJECT_KELLYSLATER
	script_widget_holder->freeze();
#endif

	if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
	{
		if (level_is_loaded())
		{
			wSound.pause();
			VoiceOvers.pause();
			sfx.pause();
		}
		/*if (nslGetSoundStatus(MusicMan::inst()->musicTrack.getSoundId()) == NSL_SOUNDSTATUS_PLAYING)
		nslPauseGuardSound(MusicMan::inst()->musicTrack.getSoundId());*/
		SoundScriptManager::inst()->pause();
		MusicMan::inst()->pause();
		nslPauseAllSounds();
	}

	if (level_is_loaded())
		rumbleMan.pause();

		/* old schoool

		  if(
		  get_cur_state() == GSTATE_RUNNING
		  &&
		  (!flag.wait_for_start_release)
		  &&
		  (!flag.disable_start_menu)
		  )
		  {
		  // UNTIL WE FIND A WAY TO GET SOME TEXTURE SPACE FOR IN-GAME PAUSE GRAPHICS
		  // THIS MIGHT NOT WORK ON DC
		  #if defined (TARGET_PC)
		  if( !os_developer_options::inst()->get_flag(os_developer_options::FLAG_FORCE_PC_MOUSE_OPERATION) )
		  SetCursor( LoadCursor(NULL, IDC_ARROW) );
		  #endif

			push_process( pause_process );
			sound_device::inst()->pause_all_streams();
			my_interface_widget->hide();
			script_widget_holder->hide();
			os_file::system_unlock();  // unlock filesystem for pause menu
			}
			// await modal response elsewhere
	*/

}

//--------------------------------------------------------------
void game::unpause()
{
	if( is_paused() )
	{
		if (level_is_loaded())
			rumbleMan.unpause();

		if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
		{
			nslUnpauseAllSounds();
			MusicMan::inst()->unpause();
			SoundScriptManager::inst()->unpause();
			if (level_is_loaded())
			{
				sfx.unpause();
				wSound.unpause();
				VoiceOvers.unpause();
			}
		}

		flag.game_paused = false;
		/* old school way
		#if defined (TARGET_PC)
		if( !os_developer_options::inst()->get_flag(os_developer_options::FLAG_FORCE_PC_MOUSE_OPERATION) )
		SetCursor( 0 );
		#endif

		  pop_process();    // leave pause game state
		  flag.wait_for_start_release = true;
		  my_interface_widget->show();
		  script_widget_holder->show();
		  os_file::system_lock();  // lock filesystem after pause menu
		  sound_device::inst()->resume_all_streams();
		  setup_inputs(); // PDA CAN AFFECT KEY MAPPINGS
		*/
	}
}


//--------------------------------------------------------------



bool game::is_paused() const
{
	return flag.game_paused; // old school   get_cur_state() == GSTATE_PAUSED;
}

void game::load_new_level( const stringx &new_level_name )
{
	strcpy(g_scene_name, new_level_name.c_str());
	flag.load_new_level = true;
}


// this just isn't right, but what can I do while current_view_camera's in game??
void game::enable_marky_cam( bool enable, bool sync, rational_t priority )
{
	assert( the_world->get_marky_cam_ptr() );
	if(FEDone()) assert( !enable || priority != the_world->get_marky_cam_ptr()->get_priority() );

	if(!FEDone() || (enable && priority >= the_world->get_marky_cam_ptr()->get_priority()) || (!enable && priority == the_world->get_marky_cam_ptr()->get_priority()))
	{
		if ( sync )
			the_world->get_marky_cam_ptr()->sync( *current_view_camera );

		the_world->enable_marky_cam( enable, priority );

		the_world->get_marky_cam_ptr()->camera_set_collide_with_world( false );
	}
}

//--------------------------------------------------------------


void game::render_interface()
{
	static float last_mem_used = 0.0f;
	START_PROF_TIMER( proftimer_render_interface );

#ifndef PROJECT_KELLYSLATER
	widget::prepare_to_render();
#endif

	matrix4x4 projection_matrix = geometry_manager::inst()->xforms[geometry_manager::XFORM_VIEW_TO_PROJECTION];
	matrix4x4 world_matrix = geometry_manager::inst()->xforms[geometry_manager::XFORM_WORLD_TO_VIEW];

	geometry_manager::inst()->set_cop(hw_rasta::inst()->get_screen_width()*0.5f,hw_rasta::inst()->get_screen_height()*0.5f, INTERFACE_MIN_Z, INTERFACE_MAX_Z);
	geometry_manager::inst()->set_xform(geometry_manager::XFORM_LOCAL_TO_WORLD, identity_matrix);
	geometry_manager::inst()->set_xform(geometry_manager::XFORM_WORLD_TO_VIEW, identity_matrix);

	matrix4x4 ortho;
	ortho.make_scale(vector3d(1.0f,1.0f/PROJ_ASPECT,1.0f));
	geometry_manager::inst()->set_xform(geometry_manager::XFORM_VIEW_TO_PROJECTION, ortho);

	if (flag.level_is_loaded)
	{
		// everything here only applies if level is loaded

		clear_zbuffer();

		//P    if (!pause_pda->is_shown())  // world and interface rendering
		if(!flag.disable_interface)
		{
			// skip rendering the interface elements if flagged to do so
			if ( !os_developer_options::inst()->is_flagged(os_developer_options::FLAG_INTERFACE_DISABLE)
				&& !is_letterbox_active()
				)
			{

				//        element_manager::inst()->render();

				g_world_ptr->get_matvertbufs().flush();
			}
		}

		// render script widgets over the letterboxing

#if _CONSOLE_ENABLE
		render_console();
#endif
		proftimer_render_interface.stop();

		proftimer_debug_info.start();

		// Display the scrolly/fadey text messages
		mb->render();

		// Display debug info
		if ( os_developer_options::inst()->is_flagged(os_developer_options::FLAG_SHOW_DEBUG_INFO) )
			show_debug_info();

		// Calculate and display frame rate
#ifdef PROFILING_ON
		STOP_PROF_TIMER( proftimer_cpu_net );
		STOP_PROF_TIMER( proftimer_render_cpu );
		proftimer_profiler.start();
		proftimer_draw_prof_info.start();

		profiler* theprofiler = profiler::inst();
		theprofiler->render();

		proftimer_draw_prof_info.stop();
		proftimer_profiler.stop();
		START_PROF_TIMER( proftimer_render_cpu );
		START_PROF_TIMER( proftimer_cpu_net );
#endif // PROFILING_ON

		// Setup the text buffer to write out
		if ( os_developer_options::inst()->is_flagged(os_developer_options::FLAG_SHOW_STATS) )
		{
#if defined(TARGET_PC)
			render_quad_2d(vector2d(0.0f,-0.835f),vector2d(0.25f,0.04f),0.1f,color32(0,0,0,128));
			float fFPS = get_instantaneous_fps();
			int poly_count = hw_rasta::inst()->get_poly_count(); //H +clockwise_clipped;
			//      debug_print(vector2di(250, 400), ftos(fFPS)+" fps (Polys: "+itos(poly_count)+")");
#endif
		}
		else if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_SHOW_FPS))
		{
			//      render_quad_2d(vector2d(0.66f,-0.875f),vector2d(0.25f,0.04f),0.1f,color32(0,0,0,128));
			// show real FPS and theoretical FPS
			float fps = get_console_fps();
			//float fps = theprofiler->get_instantaneous_fps();
			float internalfps = get_theoretical_fps();
			// choose a color
			color32 clr(128, 128, 128);  // white (>=60)
			if ( fps < 15 )
				clr = color32(160, 32, 32);  // red (<15)
			else if ( fps < 30 )
				clr = color32(160, 80, 16);  // orange (<30)
			else if ( fps < 60 )
				clr = color32(160, 160, 0);  // yellow (<60)
			// print to screen
			//      hw_rasta::inst()->print(stringx(stringx::fmt, "FPS %2.0f  (%3.1f)", fps, internalfps ),
			//        vector2di(460,23), clr); // 450, 60
			//      render_quad_2d(vector2d(-0.75f,0.96f),vector2d(0.25f,0.04f),0.1f,color32(0,0,0,160));
			render_quad_2d(vector2d(-0.75f,0.96f),vector2d(0.8f,0.12f),0.1f,color32(0,0,0,160));
#if 0 //defined(MEMTRACK)
			float mem_used = ((float)mem_get_total_alloced()) / 1048576.0f;
#elif defined(TARGET_PS2)
			struct mallinfo info = mallinfo();
			float mem_used = ((float)info.uordblks) / 1048576.0f;
#else // unknown
			float mem_used = 0.0f;
#endif
			hw_rasta::inst()->print(stringx(stringx::fmt, "FPS %2.0f  (%3.1f)", fps, internalfps ),
				vector2di(20,455), clr); // 450, 60

			// choose a color
			if ( last_mem_used < mem_used )
				clr = color32(32, 160, 32);  // green (freed memory last frame)
			else if ( last_mem_used > mem_used )
				clr = color32(160, 80, 16);  // red (more memory last frame)
			else
				clr = color32(128, 128, 128);  // same memory as last frame

			hw_rasta::inst()->print(stringx(stringx::fmt, "MEM(%2.2fMB)", mem_used ),
				vector2di(185,455), clr); // 450, 60
			last_mem_used = mem_used;
		}
#if defined(TARGET_PC) && !defined(BUILD_BOOTABLE) && 0
		{
			char buf[128];
			extern unsigned __int64 g_lame_clock_count;
			_i64toa(g_lame_clock_count,buf,16);
			hw_rasta::inst()->print(buf,
				vector2di(400,83), color32_grey);
		}
#endif

#ifndef BUILD_BOOTABLE
		if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_SHOW_PLRCTRL))
		{
			char work_str[256];
			int y = 410;

			/*
			#ifdef _DEBUG
			sprintf( work_str, "Enum: %s (%d/%d)", g_spidey_anim_enums[g_spiderman_controller_ptr->curr_anim_enum], g_spiderman_controller_ptr->curr_anim_enum, (_SPIDEY_NUM_ANIMS - 1) );
			hw_rasta::inst()->print( work_str, vector2di(5,y), color32_grey );
			#endif
			*/

			y+= 14;
			entity_anim_tree *anim = g_world_ptr->get_hero_ptr(active_player)->get_anim_tree(ANIM_PRIMARY);
			sprintf( work_str, "Anim: %s", anim? anim->get_name().c_str() : "NULL" );
			hw_rasta::inst()->print( work_str, vector2di(5,y), color32_grey );
		}

		// Spiderman
#ifdef BUILD_DEBUG
		if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_SHOW_PSX_INFO))
		{
			//print_pc_2_playstation_debug_info();
		}
#endif

#endif // BUILD_BOOTABLE

#ifdef SHOW_MISC_ERRORS_ONSCREEN
		render_onscreen_errors();
#endif

#ifdef PROFILING_ON
		if (g_debug.turtle_watch)
		{
			const int LINE_SPACE = 13;
			if (total_delta > 1.0f/30.0f)
			{
				// frame time
				hw_rasta::inst()->print( "SLOW FRAME",
					vector2di(400,50+LINE_SPACE*4),  // move it down so its not cut out by movie marquee
					color32(216,64,0));
				char work_str[64];
				sprintf( work_str, "%6.2f", total_delta*1000.0f );
				hw_rasta::inst()->print( work_str,
					vector2di(550,50+LINE_SPACE*4), color32_grey );
				// flip
				hw_rasta::inst()->print( "SLOW FLIP",
					vector2di(400,50+LINE_SPACE*5),   // move it down so its not cut out by movie marquee
					color32(216,64,0));
				sprintf( work_str, "%6.2f", flip_delta*1000.0f );
				hw_rasta::inst()->print( work_str,
					vector2di(550,50+LINE_SPACE*5), color32_grey );
			}
		}
#endif // PROFILING_ON
		proftimer_debug_info.stop();

		proftimer_render_interface.start();
  }
  // restore the projection matrix
  geometry_manager::inst()->set_xform(geometry_manager::XFORM_VIEW_TO_PROJECTION, projection_matrix);
  geometry_manager::inst()->set_xform(geometry_manager::XFORM_WORLD_TO_VIEW, world_matrix);

  STOP_PROF_TIMER( proftimer_render_interface );
}

//---------------------------------------------------------------
bool game::is_letterbox_active() const
{
	return false; //P (p_letterbox && p_letterbox->active);
}

//--------------------------------------------------------------
void game::show_debug_info()
{
	entity * hero_ptr;
	hero_ptr = g_world_ptr->get_hero_ptr(active_player);
	region_graph::node* rgn = hero_ptr->get_region();
	stringx hero_sector_name("none");
	if ( rgn )
		hero_sector_name = rgn->get_data()->get_name();

	color32 col = rgn ? color32(192,192,128) : color32(192,128,0);
	hw_rasta::inst()->print("HERO @ "+v3tos(hero_ptr->get_abs_position())+" "+hero_sector_name,
		vector2di(14,52), col);
	//  int i;

	stringx camera_sector_name = current_view_camera->get_region()? current_view_camera->get_region()->get_data()->get_name() : stringx("none");
	sector* sec = g_world_ptr->get_the_terrain().find_sector( current_view_camera->terrain_position() );
	col = sec ? color32(192,192,128) : color32(192,128,0);
	hw_rasta::inst()->print("CAMERA @ "+v3tos(current_view_camera->get_abs_position())+" "+camera_sector_name,
		vector2di(14,28), col);

	camera * scene_analyzer_cam = find_camera(entity_id("SCENE_ANALYZER_CAM"));
	sec = g_world_ptr->get_the_terrain().find_sector( scene_analyzer_cam->terrain_position() );
	col = sec ? color32(192,192,128) : color32(192,128,0);
	hw_rasta::inst()->print("ANALYZER @ "+v3tos(scene_analyzer_cam->get_abs_position())+" "+camera_sector_name,
		vector2di(14,14), col);

#ifdef GCCULL
	char buffer[160];
	i = 0;
	list<sound_stream *>::iterator si;
	for (si = g_stream_play_list.begin(); si!=g_stream_play_list.end(); ++si)
	{
		if (*si)
		{
			int icolor = (i<MAX_STREAM_BUFFERS)?255:128;
			int isplaying = ((*si)->is_playing())?255:128;
			sprintf(buffer,"%s : %.2f", (*si)->get_file_name().c_str(), (*si)->get_volume());
			hw_rasta::inst()->print(buffer, vector2di(20,320+(i*14)), color32(isplaying,icolor,0) );
			++i;
		}
	}
#endif
}


//--------------------------------------------------------------
// MAY BE CALLED MULTIPLE TIMES NOW
//--------------------------------------------------------------
void game::setup_input_registrations()
{
	input_mgr* inputmgr = input_mgr::inst();
	// Register all the controls w/ the input manager.
	// Player control

	// Generic buttons used by script language
	inputmgr->register_control( game_control( BUTTON_A,  CT_BOOLEAN ) );
	inputmgr->register_control( game_control( BUTTON_B,  CT_BOOLEAN ) );
	inputmgr->register_control( game_control( BUTTON_X,  CT_BOOLEAN ) );
	inputmgr->register_control( game_control( BUTTON_Y,  CT_BOOLEAN ) );
	inputmgr->register_control( game_control( DIR_UPDOWN,    CT_BOOLEAN ) );
	inputmgr->register_control( game_control( DIR_LEFTRIGHT, CT_BOOLEAN ) );

	// Generic PS2 controller buttons
	inputmgr->register_control( game_control(PSX_X, CT_BOOLEAN) );
	inputmgr->register_control( game_control(PSX_CIRCLE, CT_BOOLEAN) );
	inputmgr->register_control( game_control(PSX_TRIANGLE, CT_BOOLEAN) );
	inputmgr->register_control( game_control(PSX_SQUARE, CT_BOOLEAN) );
	inputmgr->register_control( game_control(PSX_L1, CT_BOOLEAN) );
	inputmgr->register_control( game_control(PSX_L2, CT_BOOLEAN) );
	inputmgr->register_control( game_control(PSX_L3, CT_BOOLEAN) );
	inputmgr->register_control( game_control(PSX_R1, CT_BOOLEAN) );
	inputmgr->register_control( game_control(PSX_R2, CT_BOOLEAN) );
	inputmgr->register_control( game_control(PSX_R3, CT_BOOLEAN) );
	inputmgr->register_control( game_control(PSX_UD, CT_RATIONAL) );
	inputmgr->register_control( game_control(PSX_LR, CT_RATIONAL) );

#if GT_ENHANCED_LOOKAROUND_MODE// && defined (TARGET_PS2)
	inputmgr->register_control( game_control(PSX_RUD, CT_RATIONAL) );
	inputmgr->register_control( game_control(PSX_RLR, CT_RATIONAL) );
#endif
	inputmgr->register_control( game_control(PSX_START, CT_BOOLEAN) );
	inputmgr->register_control( game_control(PSX_SELECT, CT_BOOLEAN) );

	// FOR PC CONTROLS CONFIG
	inputmgr->register_control( game_control( CONFIG_INPUT_PRESS,   CT_BOOLEAN ) );
	inputmgr->register_control( game_control( CONFIG_INPUT_UP,      CT_BOOLEAN ) );
	inputmgr->register_control( game_control( CONFIG_INPUT_DOWN,    CT_BOOLEAN ) );
	inputmgr->register_control( game_control( CONFIG_INPUT_LEFT,    CT_BOOLEAN ) );
	inputmgr->register_control( game_control( CONFIG_INPUT_RIGHT,   CT_BOOLEAN ) );
	inputmgr->register_control( game_control( CONFIG_INPUT_ABUTTON, CT_BOOLEAN ) );
	inputmgr->register_control( game_control( CONFIG_INPUT_BBUTTON, CT_BOOLEAN ) );

	inputmgr->register_control( game_control( PLAYER_FORWARD_AXIS,   CT_RATIONAL ) );
	inputmgr->register_control( game_control( PLAYER_TURN_AXIS,      CT_RATIONAL ) );
	inputmgr->register_control( game_control( PLAYER_ALT_FORWARD_AXIS,   CT_RATIONAL ) );
	inputmgr->register_control( game_control( PLAYER_ALT_TURN_AXIS,      CT_RATIONAL ) );

	inputmgr->register_control( game_control( PLAYER_ATTACK,         CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_SHOOT,          CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_FORWARD,        CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_BACKWARD,       CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_TURN_LEFT,      CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_TURN_RIGHT,     CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_STRAFE_LEFT,    CT_RATIONAL ) );
	inputmgr->register_control( game_control( PLAYER_STRAFE_RIGHT,   CT_RATIONAL ) );
	inputmgr->register_control( game_control( PLAYER_BURST_LEFT,     CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_BURST_RIGHT,    CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_BURST_FORWARD,  CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_BURST_BACKWARD, CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_BLOCK_LOW,      CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_BLOCK_HIGH,     CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_JUMP,           CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_CROUCH,         CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_FRONT_CROUCH,   CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_REAR_CROUCH,    CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_RESET_POSITION, CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_FULL_HEAL,      CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_ALL_HEAL,       CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_MUKOR,          CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_NEXT_ITEM,      CT_RATIONAL ) );
	inputmgr->register_control( game_control( PLAYER_MOUSELOOK_MOD,  CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_USE_ITEM,       CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_ACTION,         CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_VSIM_HIGH,        CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_VSIM_LOW,      CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_VSIM_LEFT,      CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_VSIM_RIGHT,     CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_VSIM_HIGH_RIGHT,        CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_VSIM_LOW_RIGHT,      CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_VSIM_HIGH_LEFT,      CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_VSIM_LOW_LEFT,     CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_VSIM_EXTEND,     CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_VSIM_THRUST_MODE,     CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_VSIM_FREE_MODE,     CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_VSIM_SWORD_FLIP,     CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_WALK_MODE,     CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_HEAD_CONTROL_MODE,     CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_HEAD_LEVEL,     CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_HEAD_EXTEND,     CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_HEAD_THETA,     CT_RATIONAL ) );
	inputmgr->register_control( game_control( PLAYER_HEAD_PSI,     CT_RATIONAL ) );
	inputmgr->register_control( game_control( PLAYER_ARM_CYCLE,     CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_JAW_OPEN,     CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_TAIL_MOD,     CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_FLIP  ,     CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_FLY_TOGGLE, CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_SPIN_SWORD, CT_BOOLEAN ) );

	// Physics control
	inputmgr->register_control( game_control( GRAVITY_TOGGLE,        CT_BOOLEAN ) );
	inputmgr->register_control( game_control( KILL_VELOCITIES,       CT_BOOLEAN ) );
	inputmgr->register_control( game_control( STOP_PHYSICS,          CT_BOOLEAN ) );
	inputmgr->register_control( game_control( SINGLE_STEP,           CT_BOOLEAN ) );

	// Debug info control
	inputmgr->register_control( game_control( SHOW_DEBUG_INFO,       CT_BOOLEAN ) );
	inputmgr->register_control( game_control( SHOW_PROFILE_INFO,     CT_BOOLEAN ) );
	inputmgr->register_control( game_control( OPEN_PROFILE_ENTRY,    CT_BOOLEAN ) );
	inputmgr->register_control( game_control( SCROLL_PROFILE_INFO,   CT_RATIONAL ) );
	inputmgr->register_control( game_control( SHOW_MEMORY_BUDGET,    CT_BOOLEAN ) );

	// Fog control
	inputmgr->register_control( game_control( TOGGLE_FOG_COLOR_ADJUSTMENTS,  CT_BOOLEAN ) );
	inputmgr->register_control( game_control( SELECT_RED_ADJUSTMENTS,        CT_BOOLEAN ) );
	inputmgr->register_control( game_control( SELECT_GREEN_ADJUSTMENTS,      CT_BOOLEAN ) );
	inputmgr->register_control( game_control( SELECT_BLUE_ADJUSTMENTS,       CT_BOOLEAN ) );
	inputmgr->register_control( game_control( SELECT_DISTANCE_ADJUSTMENTS,   CT_BOOLEAN ) );
	inputmgr->register_control( game_control( INC_FOG_COLOR,        CT_BOOLEAN ) );
	inputmgr->register_control( game_control( DEC_FOG_COLOR,        CT_BOOLEAN ) );

	// User camera control
	inputmgr->register_control( game_control( TOGGLE_CAMERA_LOCK,    CT_BOOLEAN ) );
	inputmgr->register_control( game_control( USERCAM_FORWARD,       CT_BOOLEAN ) );
	inputmgr->register_control( game_control( USERCAM_BACKWARD,      CT_BOOLEAN ) );
	inputmgr->register_control( game_control( USERCAM_STRAFE_LEFT,   CT_BOOLEAN ) );
	inputmgr->register_control( game_control( USERCAM_STRAFE_RIGHT,  CT_BOOLEAN ) );
	inputmgr->register_control( game_control( USERCAM_UP,            CT_BOOLEAN ) );
	inputmgr->register_control( game_control( USERCAM_DOWN,          CT_BOOLEAN ) );

	inputmgr->register_control( game_control( USERCAM_PITCH,         CT_RATIONAL ) );
	inputmgr->register_control( game_control( USERCAM_YAW,           CT_RATIONAL ) );
	inputmgr->register_control( game_control( USERCAM_FAST,          CT_BOOLEAN ) );
	inputmgr->register_control( game_control( USERCAM_SLOW,          CT_BOOLEAN ) );
	inputmgr->register_control( game_control( USERCAM_SCREEN_SHOT,   CT_BOOLEAN ) );
	inputmgr->register_control( game_control( USERCAM_GENERATE_CUBE_MAP,   CT_BOOLEAN ) );
	inputmgr->register_control( game_control( USERCAM_EQUALS_CHASECAM, CT_BOOLEAN ) );
	inputmgr->register_control( game_control( USERCAM_SAVE1, CT_BOOLEAN ) );
	inputmgr->register_control( game_control( USERCAM_SAVE2, CT_BOOLEAN ) );
	inputmgr->register_control( game_control( USERCAM_SAVE3, CT_BOOLEAN ) );
	inputmgr->register_control( game_control( USERCAM_SAVE4, CT_BOOLEAN ) );
	inputmgr->register_control( game_control( USERCAM_SAVE5, CT_BOOLEAN ) );
	inputmgr->register_control( game_control( USERCAM_SAVE6, CT_BOOLEAN ) );
	inputmgr->register_control( game_control( USERCAM_SAVE7, CT_BOOLEAN ) );
	inputmgr->register_control( game_control( USERCAM_SAVE8, CT_BOOLEAN ) );
	inputmgr->register_control( game_control( USERCAM_SAVE9, CT_BOOLEAN ) );
	inputmgr->register_control( game_control( USERCAM_SAVE0, CT_BOOLEAN ) );
	inputmgr->register_control( game_control( USERCAM_SAVE_GO, CT_BOOLEAN ) );
	inputmgr->register_control( game_control( USERCAM_SAVE_CLEAR, CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PLAYER_STRAFE_MOD,  CT_BOOLEAN ) );


	inputmgr->register_control( game_control( PAD2_USERCAM_FORWARD,       CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PAD2_USERCAM_BACKWARD,      CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PAD2_USERCAM_STRAFE_LEFT,   CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PAD2_USERCAM_STRAFE_RIGHT,  CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PAD2_USERCAM_UP,            CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PAD2_USERCAM_DOWN,          CT_BOOLEAN ) );

	inputmgr->register_control( game_control( PAD2_USERCAM_PITCH,         CT_RATIONAL ) );
	inputmgr->register_control( game_control( PAD2_USERCAM_YAW,           CT_RATIONAL ) );
	inputmgr->register_control( game_control( PAD2_USERCAM_FAST,          CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PAD2_USERCAM_SLOW,          CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PAD2_USERCAM_EQUALS_CHASECAM,   CT_BOOLEAN ) );

	inputmgr->register_control( game_control( REPLAYCAM_ZOOM,     CT_BOOLEAN ) );
	inputmgr->register_control( game_control( REPLAYCAM_PITCH,    CT_BOOLEAN ) );
	inputmgr->register_control( game_control( REPLAYCAM_YAW,      CT_BOOLEAN ) );
	inputmgr->register_control( game_control( REPLAYCAM_SLOW,     CT_BOOLEAN ) );
	inputmgr->register_control( game_control( REPLAYCAM_FAST,     CT_BOOLEAN ) );

	// edit camera
	inputmgr->register_control( game_control( EDITCAM_FORWARD, CT_BOOLEAN ) );
	inputmgr->register_control( game_control( EDITCAM_BACKWARD, CT_BOOLEAN ) );
	inputmgr->register_control( game_control( EDITCAM_STRAFE_LEFT, CT_BOOLEAN ) );
	inputmgr->register_control( game_control( EDITCAM_STRAFE_RIGHT, CT_BOOLEAN ) );
	inputmgr->register_control( game_control( EDITCAM_UP, CT_BOOLEAN ) );
	inputmgr->register_control( game_control( EDITCAM_DOWN, CT_BOOLEAN ) );
	inputmgr->register_control( game_control( EDITCAM_PITCH_UP, CT_BOOLEAN ) );
	inputmgr->register_control( game_control( EDITCAM_PITCH_DOWN, CT_BOOLEAN ) );
	inputmgr->register_control( game_control( EDITCAM_YAW_LEFT, CT_BOOLEAN ) );
	inputmgr->register_control( game_control( EDITCAM_YAW_RIGHT, CT_BOOLEAN ) );
	inputmgr->register_control( game_control( EDITCAM_FAST, CT_BOOLEAN ) );
	inputmgr->register_control( game_control( EDITCAM_SLOW, CT_BOOLEAN ) );
	inputmgr->register_control( game_control( EDITCAM_USE_MOUSE, CT_BOOLEAN ) );


	// Scene analyzer
	inputmgr->register_control( game_control( TOGGLE_SCENE_ANALYZER,    CT_BOOLEAN ) );

	// planes viewer
	inputmgr->register_control( game_control( PLANE_BOUNDS_MOD,    CT_BOOLEAN ) );
	inputmgr->register_control( game_control( FIRST_PLANE_ADJUST,    CT_BOOLEAN ) );
	inputmgr->register_control( game_control( LAST_PLANE_ADJUST,    CT_BOOLEAN ) );

	// Misc.
	//inputmgr->register_control( game_control( DO_MALOR,              CT_BOOLEAN ) );
	inputmgr->register_control( game_control( DO_MALOR_NEXT,         CT_BOOLEAN ) );
	inputmgr->register_control( game_control( DO_MALOR_PREV,         CT_BOOLEAN ) );
	inputmgr->register_control( game_control( RELOAD_PARTICLE_GENERATORS, CT_BOOLEAN ) );
	inputmgr->register_control( game_control( RELOAD_CHARACTER_ATTRIBUTES, CT_BOOLEAN ) );
	inputmgr->register_control( game_control( DUMP_ENTITY_PO,         CT_BOOLEAN ) );
	inputmgr->register_control( game_control( TOGGLE_BSP_SPRAY_PAINT, CT_BOOLEAN ) );

#ifdef TARGET_PC
	inputmgr->register_control( game_control( TOGGLE_WIREFRAME, CT_BOOLEAN ) );
#endif

	// cheats
	inputmgr->register_control( game_control( CHEAT_MALOR,           CT_RATIONAL ) );
	inputmgr->register_control( game_control( CHEAT_FULL_HEAL,   CT_RATIONAL ) );
	inputmgr->register_control( game_control( CHEAT_GOD_MODE,    CT_RATIONAL ) );
	inputmgr->register_control( game_control( CHEAT_I_WIN,     CT_RATIONAL ) );

#ifdef SHOW_MISC_ERRORS_ONSCREEN
	inputmgr->register_control( game_control( CHEAT_SHOW_ERRORS,   CT_BOOLEAN ) );
#endif

#ifdef ENABLE_SCREEN_GRAB
	inputmgr->register_control( game_control( DC_SCREEN_GRAB, CT_BOOLEAN ) );
#endif

	inputmgr->register_control( game_control( QUIT_GAME,              CT_BOOLEAN ) );

	inputmgr->register_control( game_control( INTERFACE_BUTTON, CT_BOOLEAN ) );
	inputmgr->register_control( game_control( GAME_PAUSE, CT_BOOLEAN ) );

	// PFE CONTROL
	inputmgr->register_control( game_control( PFE_UPDOWN,    CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PFE_LEFTRIGHT, CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PFE_A,     CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PFE_B,     CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PFE_X,     CT_BOOLEAN ) );
	inputmgr->register_control( game_control( PFE_Y,     CT_BOOLEAN ) );
#ifdef TARGET_GC
	inputmgr->register_control( game_control( PFE_Z,     CT_BOOLEAN ) );
#endif
	inputmgr->register_control( game_control( PFE_START, CT_BOOLEAN ) );

	// ANNOYING NEW CRAP
	inputmgr->register_control( game_control( PLAYER_AUTOMAP_TRIGGER, CT_BOOLEAN ) );


	// MAXSTEEL specific
	inputmgr->register_control( game_control(SHOOT_PUNCH, CT_BOOLEAN) );
	inputmgr->register_control( game_control(KICK_ZOOMOUT, CT_BOOLEAN) );
	inputmgr->register_control( game_control(ACTION_ZOOMIN, CT_BOOLEAN) );
	inputmgr->register_control( game_control(JUMP_CLIMB, CT_BOOLEAN) );
	inputmgr->register_control( game_control(NANOTECH, CT_BOOLEAN) );
	inputmgr->register_control( game_control(INVENTORY, CT_BOOLEAN) );
	inputmgr->register_control( game_control(START_PDA, CT_BOOLEAN) );
	inputmgr->register_control( game_control(STICK_AXIS_V, CT_RATIONAL) );
	inputmgr->register_control( game_control(STICK_AXIS_H, CT_RATIONAL) );
	inputmgr->register_control( game_control(MOUSE_AXIS_V, CT_RATIONAL) );
	inputmgr->register_control( game_control(MOUSE_AXIS_H, CT_RATIONAL) );

	inputmgr->register_control( game_control(NEXT_AI_CAM, CT_BOOLEAN) );
	inputmgr->register_control( game_control(PREV_AI_CAM, CT_BOOLEAN) );
	inputmgr->register_control( game_control(TOGGLE_AI_CAM_MODE, CT_BOOLEAN) );

#if _VIS_ITEM_DEBUG_HELPER
	inputmgr->register_control( game_control(ITEM_SWITCH_AXIS_TYPE, CT_BOOLEAN) );
	inputmgr->register_control( game_control(ITEM_INC_AXIS, CT_BOOLEAN) );
	inputmgr->register_control( game_control(ITEM_DEC_AXIS, CT_BOOLEAN) );
	inputmgr->register_control( game_control(ITEM_ACC_AXIS, CT_BOOLEAN) );
	inputmgr->register_control( game_control(ITEM_SWITCH_DEBUG_ON, CT_BOOLEAN) );
#endif

#ifndef BUILD_BOOTABLE
	inputmgr->register_control( game_control(DUMP_FRAME_INFO, CT_BOOLEAN) );
#endif

	// BIGCULL   register_pc_2_playstation_inputs();
	register_kellyslater_inputs();
}

void game::setup_inputs()
{
	input_mgr* inputmgr = input_mgr::inst();
	inputmgr->clear_mapping();

	// Set up the control mapping: REAL GAME CONTROLS

	// Generic controls used by scripts
	inputmgr->map_control( BUTTON_A,       JOYSTICK_DEVICE, JOY_BTNA );
	inputmgr->map_control( BUTTON_B,       JOYSTICK_DEVICE, JOY_BTNB );
	inputmgr->map_control( BUTTON_X,       JOYSTICK_DEVICE, JOY_BTNX );
	inputmgr->map_control( BUTTON_Y,       JOYSTICK_DEVICE, JOY_BTNY );
	inputmgr->map_control( DIR_UPDOWN,     JOYSTICK_DEVICE, JOY_Y );
	inputmgr->map_control( DIR_LEFTRIGHT,  JOYSTICK_DEVICE, JOY_X );
	inputmgr->map_control( DIR_UPDOWN,     JOYSTICK_DEVICE, JOY_RY );
	inputmgr->map_control( DIR_LEFTRIGHT,  JOYSTICK_DEVICE, JOY_RX );

	inputmgr->map_control( BUTTON_A,       KEYBOARD_DEVICE, KB_A ); // PEH TEST
	inputmgr->map_control( BUTTON_A,       KEYBOARD_DEVICE, KB_R );
	inputmgr->map_control( BUTTON_B,       KEYBOARD_DEVICE, KB_B );
	inputmgr->map_control( BUTTON_X,       KEYBOARD_DEVICE, KB_X ); // THIS USED TO BE 'y' FOR SOME REASON ??
	inputmgr->map_control( BUTTON_Y,       KEYBOARD_DEVICE, KB_Y ); // AND THIS 'G' hrmmm

	inputmgr->map_control( PFE_A,          JOYSTICK_DEVICE, JOY_BTNA );
	inputmgr->map_control( PFE_B,          JOYSTICK_DEVICE, JOY_BTNB );
	inputmgr->map_control( PFE_X,          JOYSTICK_DEVICE, JOY_BTNX );
	inputmgr->map_control( PFE_Y,          JOYSTICK_DEVICE, JOY_BTNY );

	//  Generic PS2 button mappings

	inputmgr->map_control( PSX_X,         JOYSTICK_DEVICE, JOY_PS2_BTNX );
	inputmgr->map_control( PSX_CIRCLE,    JOYSTICK_DEVICE, JOY_PS2_BTNO );
	inputmgr->map_control( PSX_TRIANGLE,  JOYSTICK_DEVICE, JOY_PS2_BTNTR );
	inputmgr->map_control( PSX_SQUARE,    JOYSTICK_DEVICE, JOY_PS2_BTNSQ );
	inputmgr->map_control( PSX_L1,        JOYSTICK_DEVICE, JOY_PS2_BTNL1 );
	inputmgr->map_control( PSX_L2,        JOYSTICK_DEVICE, JOY_PS2_BTNL2 );
	inputmgr->map_control( PSX_L3,        JOYSTICK_DEVICE, JOY_PS2_BTNL3 );
	inputmgr->map_control( PSX_R1,        JOYSTICK_DEVICE, JOY_PS2_BTNR1 );
	inputmgr->map_control( PSX_R2,        JOYSTICK_DEVICE, JOY_PS2_BTNR2 );
	inputmgr->map_control( PSX_R3,        JOYSTICK_DEVICE, JOY_PS2_BTNR3 );
	inputmgr->map_control( PSX_UD,        JOYSTICK_DEVICE, JOY_PS2_Y );
	inputmgr->map_control( PSX_LR,        JOYSTICK_DEVICE, JOY_PS2_X );
	inputmgr->map_control( PSX_START,     JOYSTICK_DEVICE, JOY_PS2_START );
	inputmgr->map_control( PSX_SELECT,    JOYSTICK_DEVICE, JOY_PS2_SELECT );

	//P  map_hero_control();

	{
		// Axis controls
		inputmgr->map_control( PLAYER_FORWARD_AXIS,   JOYSTICK_DEVICE, JOY_Y );
		inputmgr->map_control( PLAYER_TURN_AXIS,      JOYSTICK_DEVICE, JOY_X );

#if CHEATS_FOR_TESTING
		// cheats
		inputmgr->map_control( CHEAT_MALOR,      JOYSTICK_DEVICE, JOY_RY );
		inputmgr->map_control( CHEAT_I_WIN,      JOYSTICK_DEVICE, JOY_RY );
		inputmgr->map_control( CHEAT_FULL_HEAL,  JOYSTICK_DEVICE, JOY_RX );
		inputmgr->map_control( CHEAT_GOD_MODE,   JOYSTICK_DEVICE, JOY_RX );
#endif

#ifdef SHOW_MISC_ERRORS_ONSCREEN
		inputmgr->map_control( CHEAT_SHOW_ERRORS,       JOYSTICK_DEVICE, JOY_BTNY );
#endif

#if defined(TARGET_PC)
		inputmgr->map_control( PLAYER_NEXT_ITEM,      JOYSTICK_DEVICE, JOY_BTNA );
#endif

		// Button controls
		//  input_mgr::inst()->map_control( PLAYER_ATTACK,         KEYBOARD_DEVICE, KB_F6 );
		input_mgr::inst()->map_control( PLAYER_STRAFE_LEFT,    JOYSTICK_DEVICE, JOY_BTNL );
		input_mgr::inst()->map_control( PLAYER_STRAFE_RIGHT,   JOYSTICK_DEVICE, JOY_BTNR );

		input_mgr::inst()->map_control( PLAYER_RESET_POSITION, KEYBOARD_DEVICE, KB_HOME );
		input_mgr::inst()->map_control( PLAYER_MUKOR,          KEYBOARD_DEVICE, KB_END );

		inputmgr->map_control( PLAYER_NEXT_ITEM,      JOYSTICK_DEVICE, JOY_RX);

		// FOR PC CONTROLS CONFIGURATION USE
		// AND KEYBOARD EQUIVALENTS TO THE JOYSTICK INTERFACE
		input_mgr::inst()->map_control( CONFIG_INPUT_PRESS,   KEYBOARD_DEVICE, KB_SPACE ); // PEH WAS SPACE
		input_mgr::inst()->map_control( CONFIG_INPUT_UP,      KEYBOARD_DEVICE, KB_UP );
		input_mgr::inst()->map_control( CONFIG_INPUT_DOWN,    KEYBOARD_DEVICE, KB_DOWN );
		input_mgr::inst()->map_control( CONFIG_INPUT_LEFT,    KEYBOARD_DEVICE, KB_LEFT );
		input_mgr::inst()->map_control( CONFIG_INPUT_RIGHT,   KEYBOARD_DEVICE, KB_RIGHT );
		input_mgr::inst()->map_control( CONFIG_INPUT_ABUTTON, KEYBOARD_DEVICE, KB_RETURN ); // PEH WAS ESCAPE
		input_mgr::inst()->map_control( CONFIG_INPUT_BBUTTON, KEYBOARD_DEVICE, KB_BACKSPACE );


		// Physics control
		inputmgr->map_control( GRAVITY_TOGGLE,       KEYBOARD_DEVICE, KB_G );
		inputmgr->map_control( KILL_VELOCITIES,      KEYBOARD_DEVICE, KB_K );
		inputmgr->map_control( STOP_PHYSICS,         KEYBOARD_DEVICE, KB_F9 );
		inputmgr->map_control( SINGLE_STEP,          KEYBOARD_DEVICE, KB_F8 );

		// Debug info control
		inputmgr->map_control( SHOW_DEBUG_INFO,      KEYBOARD_DEVICE, KB_F12 );
		inputmgr->map_control( SHOW_MEMORY_BUDGET,   KEYBOARD_DEVICE, KB_F1 );

#ifdef TARGET_PS2
		inputmgr->map_control( SHOW_PROFILE_INFO,   JOYSTICK_DEVICE, JOY_PS2_SELECT );
		inputmgr->map_control( OPEN_PROFILE_ENTRY,  JOYSTICK_DEVICE, JOY_PS2_BTNR3 );
		inputmgr->map_control( SCROLL_PROFILE_INFO, JOYSTICK_DEVICE, JOY_PS2_RY );
#else
		inputmgr->map_control( SHOW_PROFILE_INFO,    KEYBOARD_DEVICE, KB_F2 );
		inputmgr->map_control( OPEN_PROFILE_ENTRY,   JOYSTICK_DEVICE, JOY_BTNR3 );
		inputmgr->map_control( SCROLL_PROFILE_INFO,  JOYSTICK_DEVICE, JOY_RY );
#endif

		// Fog control
		input_mgr::inst()->map_control( TOGGLE_FOG_COLOR_ADJUSTMENTS,        KEYBOARD_DEVICE, KB_F6);

		//User camera control
		inputmgr->map_control( TOGGLE_CAMERA_LOCK,   KEYBOARD_DEVICE, KB_V );
		inputmgr->map_control( USERCAM_FORWARD,      KEYBOARD_DEVICE, KB_I );
		inputmgr->map_control( USERCAM_BACKWARD,     KEYBOARD_DEVICE, KB_K );
		inputmgr->map_control( USERCAM_STRAFE_LEFT,  KEYBOARD_DEVICE, KB_J );
		inputmgr->map_control( USERCAM_STRAFE_RIGHT, KEYBOARD_DEVICE, KB_L );
		inputmgr->map_control( USERCAM_UP,           KEYBOARD_DEVICE, KB_O );
		inputmgr->map_control( USERCAM_DOWN,         KEYBOARD_DEVICE, KB_U );

		//  The user cam has now been implemented for Kelly Slater  (JBW)
		// keep this for PS2 conversion (JDB)
		inputmgr->map_control( TOGGLE_CAMERA_LOCK,   JOYSTICK_DEVICE, JOY_BTNL2 );
		inputmgr->map_control( USERCAM_PITCH,        JOYSTICK_DEVICE, JOY_Y );
		inputmgr->map_control( USERCAM_YAW,          JOYSTICK_DEVICE, JOY_X );
		//  inputmgr->map_control( USERCAM_PITCH,        JOYSTICK_DEVICE, JOY_RY );
		//  inputmgr->map_control( USERCAM_YAW,          JOYSTICK_DEVICE, JOY_RX );
		inputmgr->map_control( USERCAM_FORWARD,      JOYSTICK_DEVICE, JOY_BTNY );
		inputmgr->map_control( USERCAM_BACKWARD,     JOYSTICK_DEVICE, JOY_BTNA );
		inputmgr->map_control( USERCAM_STRAFE_LEFT,  JOYSTICK_DEVICE, JOY_BTNX );
		inputmgr->map_control( USERCAM_STRAFE_RIGHT, JOYSTICK_DEVICE, JOY_BTNB );
		inputmgr->map_control( USERCAM_DOWN,         JOYSTICK_DEVICE, JOY_RY );
		//  inputmgr->map_control( USERCAM_UP,         JOYSTICK_DEVICE, JOY_BTNR );
		inputmgr->map_control( USERCAM_FAST,         JOYSTICK_DEVICE, JOY_BTNR);
		inputmgr->map_control( USERCAM_SLOW,         JOYSTICK_DEVICE, JOY_BTNL);
		inputmgr->map_control( USERCAM_SCREEN_SHOT,         JOYSTICK_DEVICE, JOY_BTNR2);
		inputmgr->map_control( USERCAM_GENERATE_CUBE_MAP,   JOYSTICK_DEVICE, JOY_BTNR1);
		inputmgr->map_control( USERCAM_EQUALS_CHASECAM, JOYSTICK_DEVICE, JOY_RX);

		inputmgr->map_control( REPLAYCAM_ZOOM,         JOYSTICK_DEVICE, JOY_LY);
		inputmgr->map_control( REPLAYCAM_PITCH,        JOYSTICK_DEVICE, JOY_RY);
		inputmgr->map_control( REPLAYCAM_YAW,          JOYSTICK_DEVICE, JOY_RX);
		inputmgr->map_control( REPLAYCAM_SLOW,         JOYSTICK_DEVICE, JOY_BTNL);
		inputmgr->map_control( REPLAYCAM_FAST,         JOYSTICK_DEVICE, JOY_BTNR);

		inputmgr->map_control( PAD2_USERCAM_PITCH,        JOYSTICK2_DEVICE, JOY_Y );
		inputmgr->map_control( PAD2_USERCAM_YAW,          JOYSTICK2_DEVICE, JOY_X );
		//  inputmgr->map_control( PAD2_USERCAM_PITCH,        JOYSTICK2_DEVICE, JOY_RY );
		//  inputmgr->map_control( PAD2_USERCAM_YAW,          JOYSTICK2_DEVICE, JOY_RX );
		inputmgr->map_control( PAD2_USERCAM_FORWARD,      JOYSTICK2_DEVICE, JOY_BTNY );
		inputmgr->map_control( PAD2_USERCAM_BACKWARD,     JOYSTICK2_DEVICE, JOY_BTNA );
		inputmgr->map_control( PAD2_USERCAM_STRAFE_LEFT,  JOYSTICK2_DEVICE, JOY_BTNX );
		inputmgr->map_control( PAD2_USERCAM_STRAFE_RIGHT, JOYSTICK2_DEVICE, JOY_BTNB );
		inputmgr->map_control( PAD2_USERCAM_DOWN,         JOYSTICK2_DEVICE, JOY_RY );
		//  inputmgr->map_control( PAD2_USERCAM_UP,         JOYSTICK2_DEVICE, JOY_BTNR );
		inputmgr->map_control( PAD2_USERCAM_FAST,         JOYSTICK2_DEVICE, JOY_BTNR);
		inputmgr->map_control( PAD2_USERCAM_SLOW,         JOYSTICK2_DEVICE, JOY_BTNL);
		inputmgr->map_control( PAD2_USERCAM_EQUALS_CHASECAM, JOYSTICK2_DEVICE, JOY_RX);

		inputmgr->map_control( USERCAM_EQUALS_CHASECAM, KEYBOARD_DEVICE, KB_BACKSLASH);
		inputmgr->map_control( USERCAM_SAVE1, KEYBOARD_DEVICE, KB_1);
		inputmgr->map_control( USERCAM_SAVE2, KEYBOARD_DEVICE, KB_2);
		inputmgr->map_control( USERCAM_SAVE3, KEYBOARD_DEVICE, KB_3);
		inputmgr->map_control( USERCAM_SAVE4, KEYBOARD_DEVICE, KB_4);
		inputmgr->map_control( USERCAM_SAVE5, KEYBOARD_DEVICE, KB_5);
		inputmgr->map_control( USERCAM_SAVE6, KEYBOARD_DEVICE, KB_6);
		inputmgr->map_control( USERCAM_SAVE7, KEYBOARD_DEVICE, KB_7);
		inputmgr->map_control( USERCAM_SAVE8, KEYBOARD_DEVICE, KB_8);
		inputmgr->map_control( USERCAM_SAVE9, KEYBOARD_DEVICE, KB_9);
		inputmgr->map_control( USERCAM_SAVE0, KEYBOARD_DEVICE, KB_0);
		inputmgr->map_control( USERCAM_SAVE_GO, KEYBOARD_DEVICE, KB_M);
		inputmgr->map_control( USERCAM_SAVE_CLEAR, KEYBOARD_DEVICE, KB_P);
		inputmgr->map_control( PLAYER_STRAFE_MOD, KEYBOARD_DEVICE, KB_SLASH);

		// editor camera control
		inputmgr->map_control( EDITCAM_FORWARD,      KEYBOARD_DEVICE, KB_W );
		inputmgr->map_control( EDITCAM_BACKWARD,     KEYBOARD_DEVICE, KB_S );
		inputmgr->map_control( EDITCAM_STRAFE_LEFT,  KEYBOARD_DEVICE, KB_Q );
		inputmgr->map_control( EDITCAM_STRAFE_RIGHT, KEYBOARD_DEVICE, KB_E );
		inputmgr->map_control( EDITCAM_UP,           KEYBOARD_DEVICE, KB_C );
		inputmgr->map_control( EDITCAM_DOWN,         KEYBOARD_DEVICE, KB_Z );
		inputmgr->map_control( EDITCAM_PITCH_UP,     KEYBOARD_DEVICE, KB_R );
		inputmgr->map_control( EDITCAM_PITCH_DOWN,   KEYBOARD_DEVICE, KB_F );
		inputmgr->map_control( EDITCAM_YAW_LEFT,     KEYBOARD_DEVICE, KB_A );
		inputmgr->map_control( EDITCAM_YAW_RIGHT,    KEYBOARD_DEVICE, KB_D );
		inputmgr->map_control( EDITCAM_FAST,         KEYBOARD_DEVICE, KB_LSHIFT);
		inputmgr->map_control( EDITCAM_SLOW,         KEYBOARD_DEVICE, KB_LCONTROL);
		inputmgr->map_control( EDITCAM_SLOW,         KEYBOARD_DEVICE, KB_LALT);
		inputmgr->map_control( EDITCAM_SLOW,         KEYBOARD_DEVICE, KB_RALT);
		inputmgr->map_control( EDITCAM_SLOW,         KEYBOARD_DEVICE, KB_X);
		inputmgr->map_control( EDITCAM_USE_MOUSE, KEYBOARD_DEVICE, KB_SPACE);

		// Scene analyzer
		inputmgr->map_control( TOGGLE_SCENE_ANALYZER, KEYBOARD_DEVICE, KB_F10 );

		// planes viewer
		inputmgr->map_control( PLANE_BOUNDS_MOD, KEYBOARD_DEVICE, KB_RSHIFT );
		inputmgr->map_control( FIRST_PLANE_ADJUST, KEYBOARD_DEVICE, KB_LBRACKET );
		inputmgr->map_control( LAST_PLANE_ADJUST, KEYBOARD_DEVICE, KB_RBRACKET );

		//inputmgr->map_control( DO_MALOR,             KEYBOARD_DEVICE, KB_PAGEDOWN );
		inputmgr->map_control( DO_MALOR_NEXT,             KEYBOARD_DEVICE, KB_PAGEDOWN );
		inputmgr->map_control( DO_MALOR_PREV,             KEYBOARD_DEVICE, KB_PAGEUP );
		inputmgr->map_control( RELOAD_PARTICLE_GENERATORS, KEYBOARD_DEVICE, KB_F7 );
		inputmgr->map_control( RELOAD_CHARACTER_ATTRIBUTES, KEYBOARD_DEVICE, KB_F5 );
		inputmgr->map_control( DUMP_ENTITY_PO, KEYBOARD_DEVICE, KB_F4 );
		inputmgr->map_control( TOGGLE_BSP_SPRAY_PAINT, KEYBOARD_DEVICE, KB_F11 );

#ifdef TARGET_PC
		inputmgr->map_control( TOGGLE_WIREFRAME,     KEYBOARD_DEVICE, KB_F3 );
#endif

#ifdef ENABLE_SCREEN_GRAB
		inputmgr->map_control( DC_SCREEN_GRAB, JOYSTICK_DEVICE, JOY_BTNB );
#endif
		inputmgr->map_control( INTERFACE_BUTTON, JOYSTICK_DEVICE, JOY_BTNA );

		// PFE CONTROL
		input_mgr::inst()->map_control( PFE_UPDOWN,     JOYSTICK_DEVICE, JOY_Y );
		input_mgr::inst()->map_control( PFE_LEFTRIGHT,  JOYSTICK_DEVICE, JOY_X );
		input_mgr::inst()->map_control( PFE_UPDOWN,     JOYSTICK_DEVICE, JOY_RY );
		input_mgr::inst()->map_control( PFE_LEFTRIGHT,  JOYSTICK_DEVICE, JOY_RX );
#ifdef TARGET_GC
		inputmgr->map_control( PFE_Z,  JOYSTICK_DEVICE, JOY_BTNR2 );
		inputmgr->map_control( GAME_PAUSE, JOYSTICK_DEVICE, JOY_BTNSTART );
#endif
		//  input_mgr::inst()->map_control( PFE_START,      JOYSTICK_DEVICE, JOY_BTNSTART );

		// KEYBOARD SUPPORT FOR PC FRONT END
		input_mgr::inst()->map_control( PFE_START,      KEYBOARD_DEVICE, KB_RETURN );
		inputmgr->map_control( INTERFACE_BUTTON,        KEYBOARD_DEVICE, KB_SPACE );
		//  inputmgr->map_control( GAME_PAUSE,              KEYBOARD_DEVICE, KB_ESCAPE );

		inputmgr->map_control( PFE_A,          KEYBOARD_DEVICE, KB_A );
		inputmgr->map_control( PFE_B,          KEYBOARD_DEVICE, KB_B );
		inputmgr->map_control( PFE_X,          KEYBOARD_DEVICE, KB_X );
		inputmgr->map_control( PFE_Y,          KEYBOARD_DEVICE, KB_Y );

		inputmgr->map_control( NEXT_AI_CAM,          KEYBOARD_DEVICE, KB_EQUALS );
		inputmgr->map_control( PREV_AI_CAM,          KEYBOARD_DEVICE, KB_MINUS );
		inputmgr->map_control( TOGGLE_AI_CAM_MODE,   KEYBOARD_DEVICE, KB_BACKSPACE );

#if _VIS_ITEM_DEBUG_HELPER
		inputmgr->map_control( ITEM_SWITCH_AXIS_TYPE,      KEYBOARD_DEVICE, KB_SLASH );
		inputmgr->map_control( ITEM_INC_AXIS,              KEYBOARD_DEVICE, KB_PERIOD );
		inputmgr->map_control( ITEM_DEC_AXIS,              KEYBOARD_DEVICE, KB_COMMA );
		inputmgr->map_control( ITEM_ACC_AXIS,              KEYBOARD_DEVICE, KB_RALT );
		inputmgr->map_control( ITEM_SWITCH_DEBUG_ON,       KEYBOARD_DEVICE, KB_QUOTE );
#endif

#ifndef BUILD_BOOTABLE
		inputmgr->map_control( STOP_PHYSICS,         JOYSTICK_DEVICE, JOY_BTNSTART );
#endif
  }

  // BIGCULL   map_pc_2_playstation_inputs();
#if defined (PROJECT_KELLYSLATER)
  map_kellyslater_inputs();
#endif
}


//-----------------------------------------------------------------
bool game::was_start_pressed() const
{
#ifndef TARGET_GC
	return input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, PFE_START ) == AXIS_MAX;
#else
	bool start= input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, GAME_PAUSE ) == AXIS_MAX ;
  bool z= input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, PFE_Z ) == AXIS_MAX ;
  return start && !z;
#endif
}

bool game::was_select_pressed() const
{
	//  The press build will have a special button combo
#if BUILD_BOOTABLE
#ifdef BUILD_FINAL
	return false;	
//		return MENU_GetButtonState(MENUCMD_L1) && MENU_GetButtonState(MENUCMD_R1) &&
//			MENU_GetButtonState(MENUCMD_CROSS) && MENU_GetButtonState(MENUCMD_DOWN);
#endif
#endif

#ifdef TARGET_GC
	bool start= input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, GAME_PAUSE ) == AXIS_MAX ;
  bool z= input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, PFE_Z ) == AXIS_MAX ;
	#ifdef EVAN
	return z;
	#endif
  return start && z;
#endif
    // This really needs to be replaced with something reasonable.
    return MENU_GetButtonState(MENUCMD_SELECT);
}

bool game::was_A_pressed() const
{
	return input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, PFE_A ) == AXIS_MAX;
}

bool game::was_B_pressed() const
{
	return input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, PFE_B ) == AXIS_MAX;
}
//-----------------------------------------------------------------

void game::freeze_hero( bool freeze )
{
	for (int i = 0; i < num_active_players; i++)
	{
		if (the_world && the_world->get_hero_ptr(i))
			the_world->get_hero_ptr(i)->set_invulnerable(freeze);
	}

	flag.hero_frozen = freeze;
}

void game::turn_user_cam_on(bool user_cam_status)
{
	if (user_cam_status)
	{
		if (!was_user_cam_ever_on)
		{
			was_user_cam_ever_on = true;

			g_world_ptr->start_usercam();
		}

		g_world_ptr->get_ks_controller(0)->SetPlayerCamera((game_camera *)find_camera(entity_id("USER_CAM")));
	}

	is_user_cam_on = user_cam_status;	// must come after SetPlayerCamera
}

void game::set_player_camera(int n, camera *cam)
{
	if (is_user_cam_on)
		return;

	// mirrors set_current_camera
	assert(cam);
	player_cam[n]=cam;
	if(cam->is_a_game_camera())
	{
		((game_camera*)cam)->invalidate();
		((game_camera*)cam)->init();
	}

	//  <<<< what is all this? -DL
#if JDB_VERTIGO_EFFECT
	// This makes sure that any vertigo effect is lost
	PROJ_ZOOM = 0.8f;
	geometry_manager::inst()->rebuild_view_frame();
#endif

	PROJ_ZOOM = 0.8f;
	// Why include this call if the parameters are independent of the camera?  (dc 07/06/01)
	//  nglSetPerspectiveMatrix( 90.0f, nglGetScreenWidth()/2, nglGetScreenHeight()/2, 0.2f, 65536.0f );

}


void game::set_current_camera(camera *cam)
{
	if (cam==NULL) return;

	current_view_camera = cam;
	if ( cam->is_a_game_camera() )
	{
		// by invalidating the camera upon switching, we can avoid interpolating
		// from an irrelevant prior state
		((game_camera*)cam)->invalidate();
	}

#if JDB_VERTIGO_EFFECT
	// This makes sure that any vertigo effect is lost
	PROJ_ZOOM = 0.8f;
	geometry_manager::inst()->rebuild_view_frame();
#endif

	if(!frontendmanager.in_game_map_up)
	{
		if (os_developer_options::inst()->is_flagged (os_developer_options::FLAG_MAKE_MOVIE) ||   //  Special FOV for boat flyby
			g_game_ptr->get_beach_id () == BEACH_OPENSEA)
			PROJ_ZOOM = 0.62f;
		else
			PROJ_ZOOM = 0.8f;  //  regular in-game FOV
	}

	// Why include this call if the parameters are independent of the camera?  (dc 07/06/01)
	//  nglSetPerspectiveMatrix( 90.0f, nglGetScreenWidth()/2, nglGetScreenHeight()/2, 0.2f, 65536.0f );

}

//	move_snapshot_cam()
// Helper function - moves the current camera to a good position for taking a snapshot.
// The old po is saved in snapshotPrevCamPo so that it can be reset after snapshot is taken.
void game::move_snapshot_cam(void)
{
/*
po			newCamPo;
entity *	targetEntity;
vector3d	targ;
vector3d	cam;
vector3d	orient;

  // Save current camera po.
  snapshotPrevCamPo = player_cam[active_player]->get_rel_po();

	// Calculate camera target.
	targetEntity = ((game_camera *)player_cam[active_player])->get_target_entity();
	targ = ((conglomerate *) targetEntity)->get_member("BIP01 SPINE2")->get_abs_position();
	orient = the_world->get_ks_controller(active_player)->get_board_controller().GetForwardDir();
	orient.normalize();
	orient *= 0.4f;
	targ += orient;

	  // Calculate camera directional vector.
	  cam = ((game_camera *)player_cam[active_player])->get_rel_position();
	  orient = cam - targ;

		// Zoom camera.
		orient.normalize();
		if (the_world->get_ks_controller(active_player)->get_super_state() == SUPER_STATE_WIPEOUT)
		orient *= 2.0f;
		else
		orient *= 1.3f;

		  // Translate camera.
		  newCamPo.set_position(targ + orient);
		  newCamPo.set_facing(targ);

			// Use new camera po.
			player_cam[active_player]->set_rel_po(newCamPo);
	*/
}

//--------------------------------------------------------------

void game::end_level( void )
{


	unload_current_level();
	if(frontendmanager.in_game_map_up && !frontendmanager.return_to_fe)
	{
		reset_index();
		go_next_state();
	}
	else reset_index();

	frontendmanager.in_game_map_up = false;

#ifdef USINGSTATICSTLALLOCATIONS
	// Also add to void FEManager::ReleaseFE() in FrontendManager.cpp
	entity_anim::mem_cleanup();
	entity_anim_tree::mem_cleanup();
	po_anim::mem_cleanup();
	linear_anim<quaternion>::mem_cleanup();
	linear_anim<vector3d>::mem_cleanup();
	linear_anim<rational_t>::mem_cleanup();
	entity::movement_info::mem_cleanup();
#endif

#ifdef PLAY_ENDING_MOVIES

#if defined(TARGET_XBOX)
    g_game_ptr->play_movie("ACTIVISN",true);
#elif defined(TARGET_PS2)

#ifdef TV_PAL
    sceGsResetGraph( 0, SCE_GS_INTERLACE, SCE_GS_PAL, SCE_GS_FRAME );
		g_game_ptr->play_movie("KSINTRO", true, 640, 512);
#else
    sceGsResetGraph( 0, SCE_GS_INTERLACE, SCE_GS_NTSC, SCE_GS_FRAME );
		g_game_ptr->play_movie("KSINTRO", true, 640, 448);
#endif
#else
		nslShutdown( );

    play_movie( "TREYARCH", true );

#endif
	KSNGL_ReInit();
  nglResetDisplay();
	frontendmanager.ReloadTextures();
	frontendmanager.map->HideAllDots();
	for (int i=0; i < 4; i++)
	{
		nglListInit();
		render_map_screen();
		nglListSend(true);
	}
	nglReleaseAllTextures();
#endif

	start_drawing_map = false;

	mem_leak_test(true);
}

//	end_run()
// Call this function when the time runs out on the level.
// In singleplayer modes, the game simply pauses.
// In alternating multiplayer modes, the level restarts with the next player.
void game::end_run(void)
{
	// Single player.
	if (num_players == 1)
		pause();
	// Multiplayer.
	else
	{
		// Alternating.
		if (num_active_players == 1)
		{
			// Begin next player's turn.
			if (active_player+1 < num_players)
			{
				set_active_player(active_player+1);
				retry_level();
			}
			// All players have had their turn.
			else
			{
				pause();
			}

		}
		// Splitscreen.
		else
			pause();
	}
}

//	retry_mode()
// Plays the current game mode again.
void game::retry_mode(const bool fromMap)
{
	int	i;

	// Reset player scores.
	for (i = 0; i < MAX_PLAYERS; i++ )
	{
		if (the_world->get_ks_controller(i))
			the_world->get_ks_controller(i)->get_my_scoreManager().Reset();
	}

	// Reset play modes.
	if (play_mode.timeAttack)
		play_mode.timeAttack->Reset();
	if (play_mode.meterAttack)
		play_mode.meterAttack->Reset();
	if (play_mode.headToHead)
		play_mode.headToHead->Reset();

	// Restart the level.
	retry_level(fromMap);

	// Notify IGO.
	frontendmanager.IGO->OnModeReset();
}

//	retry_level()
// Plays the wave again, retains game mode state info.
void game::retry_level(const bool fromMap)
{
	int	i;
	kellyslater_controller *ksctrl = the_world->get_ks_controller(0);

	ksreplay.Clear(g_random_r_ptr->srand());   // Set the random number seed and save it in replay
	ksreplay.Record();

	if (ksctrl->get_super_state() == SUPER_STATE_FLYBY)
	{
		ksctrl->get_owner()->set_visible(true);
		ksctrl->get_my_board_model()->set_visible(true);
	}

	assert(the_world);

	// Set active player in singleplayer/alternating modes.
	if (num_active_players == 1)
	{
		if (play_mode.timeAttack)
			set_active_player(play_mode.timeAttack->GetActivePlayerIdx());
		else if (play_mode.meterAttack)
			set_active_player(play_mode.meterAttack->GetActivePlayerIdx());
	}
	// Set active player in splitscreen modes.
	else
	{
		for (i = 0; i < num_players; i++)
		{
			// Note: when changing these, change the code in set_active_player too.
			the_world->get_ks_controller(i)->set_active(true);
			the_world->get_hero_ptr(i)->set_visible(true);
			the_world->get_ks_controller(i)->GetBoardMember()->set_active(true);
			the_world->get_ks_controller(i)->GetBoard()->set_visible(true);
			the_world->get_ks_controller(i)->GetBoardModel()->set_visible(true);
		}
	}

	for (i = 0; i < MAX_PLAYERS; i++ )
	{
		if (the_world->get_ks_controller(i))
			the_world->get_ks_controller(i)->ResetPhysics();
	}

	if (game_mode == GAME_MODE_TIME_ATTACK)
		TIMER_Init(play_mode.timeAttack->GetLevelDuration(active_player));
	else if (game_mode == GAME_MODE_METER_ATTACK)
		TIMER_Init(play_mode.meterAttack->GetLevelDuration(active_player));
	else if ((g_game_ptr->get_game_mode() == GAME_MODE_CAREER && g_game_ptr->get_beach_id() == BEACH_INDOOR)
		|| game_mode == GAME_MODE_FREESURF_INFINITE || game_mode == GAME_MODE_FREESURF_ICON)
		TIMER_Init(0);	// infinite time for freesurf, tutorial (dc 06/10/02)
	else
		TIMER_Init();

	WAVE_ResetSchedule();

	//  Reset the special meter timer.
	ksctrl->get_special_meter()->SetUpSpecialTimer();

	if (g_beach_ptr)
		g_beach_ptr->reset();

	if (play_mode.push)
	{
		play_mode.push->Initialize(the_world->get_ks_controllers());
		for (i = 0; i < MAX_PLAYERS; i++)
			player_viewports[i] = play_mode.push->GetPlayerViewport(i);
	}
	frontendmanager.IGO->OnViewportChange();

	if (num_active_players == 1)
		the_world->get_ks_controller(active_player)->get_my_scoreManager().Reset();
	else
	{
		for (i = 0; i < MAX_PLAYERS; i++ )
		{
			if (the_world->get_ks_controller(i))
				the_world->get_ks_controller(i)->get_my_scoreManager().Reset();
		}
	}

	frontendmanager.IGO->ResetIconManager();
	frontendmanager.IGO->ResetLearnNewTrickManager();
	frontendmanager.IGO->ResetTutorialManager();
	frontendmanager.IGO->ResetHintManager();

	snapshotState = SNAPSTATE_NONE;

    if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
    {
		// Unpause the music track, paused because the pause menu has been up.  (dc 03/30/02)
		nslUnpauseAllSounds();
		MusicMan::inst()->unpause();
	}
	if (fromMap)
		flag.level_is_loaded = false;
}
nslSoundId feMusic=NSL_INVALID_ID;

void game::advance_state_front_end( time_value_t time_inc )
{
	static bool ignore_front_end=false;
	static bool in_fe=false;
	static bool override_checked=false;
	static bool first_time_through = true;

	//  mem_lock_malloc(false);

	if(!FEInitialized() && !ignore_front_end)
	{
		//	  frontendmanager.lfe = NEW LegalFrontEnd(&frontendmanager, "interface\\Legal\\", "Legal.PANEL");
		//	  frontendmanager.lfe->Load();
		if(frontendmanager.lfe) frontendmanager.lfe->Update(time_inc);
	}

	stringx override_filename (os_developer_options::inst()->get_string(os_developer_options::STRING_SCENE_NAME));
	if ( !override_checked && stricmp(override_filename.c_str(),"ASK") )
	{
		override_checked=true;
		ignore_front_end=true;
	}

	// this avoids loading the stash until the legal text has had an opportunity
	// to draw twice
	if(first_time_through)
	{
		first_time_through = false;
		return;
	}

	// uh - put the front end here I guess
	if(!FEInitialized() && !ignore_front_end)
	{
		//set_render_state(GAME_RENDER_INITALIZE_FE);
		mem_lock_malloc(false);
		WAVETEX_SetShadowScale(2.0f);
		anim_id_manager::inst()->purge();

		// Alrighty, folks, we're creating some heaps
		// The goal is to create two heaps for each surfer stash in the FE
		// Then, we delete whichever we don't use when we get to the time to go to
		// the game

		//#ifndef TARGET_GC
		//mem_create_heap_high(SURFER_HEAP2, (int)(SURFER_STASH_MAX_SIZE_MEGS*1024*1024));
		//#endif


		if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
		{

#ifdef TARGET_PS2
			if (!nglUsingProView)
#endif

			{

#if defined(TARGET_XBOX)
				nslSetRootDir("D:\\");
#endif
				if (!nslInit())
				{
#if defined(TARGET_XBOX) && defined(BUILD_FINAL)
					// Xbox requires a message on all disk read failures.  (dc 07/11/02)
					disk_read_error();
#endif
				}

#if defined(TARGET_PS2) || defined(TARGET_GC)
				nslSetRootDir("SOUNDS");
#elif defined(TARGET_XBOX)
				nslSetRootDir("D:\\SOUNDS");
#endif
				nslReset("FRONTEND",NSL_LANGUAGE_ENGLISH);
#if defined(DEBUG) && defined(TARGET_PS2) && !defined(PROFILING_ON)
				nslSetHostStreamingPS2(true);
#endif
				SoundScriptManager::inst()->init();
				SOUNDDATA_Load();

				VoiceOvers.init();
				MusicMan::inst()->loadSources();
				if (g_career)
					StoredConfigData::inst()->setGameConfig(&(g_career->cfg));
			}
		}


		{
			STOP_PS2_PC;
			if (!(stash::is_stash_open(STASH_COMMON) && stricmp(stash::get_stash_name(STASH_COMMON), "FRONTEND.ST2")==0))
			{
				if (stash::open_stash("frontend.st2", STASH_COMMON) == false)
				{
					debug_print("could not open front end stash file frontend.st2\n");
				}
				if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
				{
					MusicMan::inst()->loadSources();

				}
				FEInit();
				dmm.Init();
				set_render_state(GAME_RENDER_DRAW_FE);
			}
			START_PS2_PC;
			in_fe = true;
		}

		if (in_fe)
		{

		}


	}
		if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO) &&
		    frontendmanager.gms && frontendmanager.gms->active != GraphicalMenuSystem::TitleMenu &&
		nslGetSoundStatus(feMusic) == NSL_SOUNDSTATUS_INVALID)
		{
			nslSourceId s  = nslLoadSource( "FRONTEND" );
			if (s != NSL_INVALID_ID)
			{
				feMusic = nslAddSound( s );
				if (nslGetSoundStatus(feMusic) != NSL_SOUNDSTATUS_INVALID)
					nslPlaySound(feMusic);
			}

		}
		if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
			nslFrameAdvance(time_inc);
	// select a level
	if ( ignore_front_end )
	{
		set_render_state(GAME_RENDER_LOADING_LEVEL);
		stringx temp;
		// first set game mode so set_level() knows whether to choose from among beaches or levels.
		temp = os_developer_options::inst()->get_string(os_developer_options::STRING_GAME_MODE);

		if(stricmp(temp.c_str(),"Career")==0)
			set_game_mode(GAME_MODE_CAREER);
		else if(stricmp(temp.c_str(),"Freesurf Infinite")==0)
			set_game_mode(GAME_MODE_FREESURF_INFINITE);
		else if(stricmp(temp.c_str(),"Freesurf High Score")==0)
			set_game_mode(GAME_MODE_FREESURF_HIGHSCORE);
		else if(stricmp(temp.c_str(),"Freesurf Icon")==0)
			set_game_mode(GAME_MODE_FREESURF_ICON);
		else if(stricmp(temp.c_str(),"Practice")==0)
			set_game_mode(GAME_MODE_PRACTICE);
		else if (stricmp(temp.c_str(),"Time Attack")==0)
			set_game_mode(GAME_MODE_TIME_ATTACK);
		else if (stricmp(temp.c_str(), "Meter Attack") == 0)
			set_game_mode(GAME_MODE_METER_ATTACK);
		else if(stricmp(temp.c_str(),"SeaHorse")==0)
			set_game_mode(GAME_MODE_SEA_HORSE);
		else if(stricmp(temp.c_str(),"Head To Head")==0)
			set_game_mode(GAME_MODE_HEAD_TO_HEAD);
		else if(stricmp(temp.c_str(),"Push")==0)
			set_game_mode(GAME_MODE_PUSH);
		else if(stricmp(temp.c_str(),"ai") == 0)
			set_game_mode(GAME_MODE_AI);
		else
			set_game_mode(GAME_MODE_FREESURF_INFINITE);

		WAVETEX_SetShadowScale(1.0f);
		ignore_front_end=false;
		in_fe = true;
		set_level(override_filename, get_game_mode() == GAME_MODE_CAREER);

		if(get_game_mode() == GAME_MODE_CAREER)
		{
			g_career->init();
			g_career->StartNewCareer();
		}

		// Get hero name and index.
		for(int i=0;i<MAX_PLAYERS;i++)
		{
			temp=get_game_info().get_hero_name(i);
			setHeroname(i,temp);
			for (int j = 0; j < SURFER_LAST; ++j)
			{
				if (!stricmp(temp.c_str(), SurferDataArray[j].name) ||
					!stricmp(temp.c_str(), SurferDataArray[j].name_ps))
				{
					SetSurferIdx(i, j);
					if(get_game_mode() == GAME_MODE_CAREER)
					{
						g_career->SetMyId(j);
					}

					if (!stricmp(temp.c_str(), SurferDataArray[j].name_ps))
					{
						SetUsingPersonalitySuit(i, true);
					}
				}
			}
			/*	Old code:  delete soon if nothing breaks (dc 01/16/02)
			if (stricmp(temp.c_str(), "KellySlater") == 0 || stricmp(temp.c_str(), "PersonalityKS") == 0)
			SetSurferIdx(i, KELLY_SLATER);
			else if (stricmp(temp.c_str(), "RobMachado") == 0 || stricmp(temp.c_str(), "PersonalityRM") == 0)
			SetSurferIdx(i, ROB_MACHADO);
			else if (stricmp(temp.c_str(), "BruceIrons") == 0 || stricmp(temp.c_str(), "PersonalityBI") == 0)
			SetSurferIdx(i, BRUCE_IRONS);
			else if (stricmp(temp.c_str(), "KalaniRobb") == 0 || stricmp(temp.c_str(), "PersonalityKR") == 0)
			SetSurferIdx(i, KALANI_ROBB);
			else if (stricmp(temp.c_str(), "Frankenreiter") == 0 || stricmp(temp.c_str(), "PersonalityDF") == 0)
			SetSurferIdx(i, DONAVON_FRANKENREITER);
			else if (stricmp(temp.c_str(), "NathanFletcher") == 0 || stricmp(temp.c_str(), "PersonalityNF") == 0)
			SetSurferIdx(i, NATHAN_FLETCHER);
			else if (stricmp(temp.c_str(), "LisaAndersen") == 0 || stricmp(temp.c_str(), "PersonalityLA") == 0)
			SetSurferIdx(i, LISA_ANDERSEN);
			else if (stricmp(temp.c_str(), "TomCarrol") == 0 || stricmp(temp.c_str(), "PersonalityTA") == 0)
			SetSurferIdx(i, TOM_CARROLL);
			else if (stricmp(temp.c_str(), "TomCurren") == 0 || stricmp(temp.c_str(), "PersonalityTU") == 0)
			SetSurferIdx(i, TOM_CURREN);
			*/
		}

		//#ifndef TARGET_GC
		//mem_create_heap(SURFER_HEAP, (int)(SURFER_STASH_MAX_SIZE_MEGS*1024*1024));
		/*if (g_game_ptr->get_num_players() > 1)
		mem_create_heap_high(SURFER_HEAP2, (int)(SURFER_STASH_MAX_SIZE_MEGS*1024*1024));*/
		//#endif

		if(!FEDone()) FERelease();
		// continue to game
		go_next_state();
	}
	else
	{

			if(FEDoneLoading())
			{
				the_world->usercam_frame_advance(time_inc);
				the_world->scene_analyzer_frame_advance(time_inc);
			}
			FEUpdate(time_inc);
			if(FEDone()) { /*ignore_front_end = true;*/ in_fe = false; }


		/*
		if ( !menus.IsActive() )
		{
		low_level_console_init();
		//nglPrintf("Hello I'm game::advance_state_front_end() and I leak font refs\n");

		  // this should really be released, but the game has no text otherwise
		  nglLoadFontA( "Font8x12", 0 );

			menus.OpenMenu(&fakefrontend);
			}
			menus.Tick(time_inc);
			*/

	}

}


// do standard frame advances that almost everyone wants to
void game::frame_advance_game_overlays( time_value_t time_inc )
{

	if ( g_screenshot )
	{
		bool memlocked=mem_malloc_locked();
		mem_lock_malloc(false);
		nglScreenShot(g_screenshot_filename);
		mem_lock_malloc(memlocked);
		g_screenshot = false;
		g_screenshot_filename = NULL;
	}

	if ( flag.level_is_loaded )
	{
		IGOUpdate(time_inc);
#ifndef BUILD_FINAL
		g_world_ptr->process_kelly_slater_debugmenus ();
#endif
	}

}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

#ifdef GCCULL
void game::update_music_applied_volume( time_value_t time_inc, bool force_update )
{
	rational_t old_volume = music_applied_volume;

	rational_t user_volume = old_volume; //P optionsfile->get_option(GAME_OPT_MUSIC_VOLUME) / 10.0f;

	assert( user_volume >= 0.0f && user_volume <= 1.0f );
	assert( music_fade_to_volume >= 0.0f && music_fade_to_volume <= 1.0f );

	if ( force_update )
	{
		music_applied_volume = music_fade_to_volume = user_volume;
		music_fade_time_left = 0.0f;
	}
	else if ( music_fade_time_left > 0.0f )
	{
		if ( music_fade_time_left > time_inc )
		{
			music_applied_volume += ( music_fade_to_volume - music_applied_volume ) * ( time_inc / music_fade_time_left );
			music_fade_time_left -= time_inc;
		}
		else
		{
			music_fade_time_left = 0.0f;
			music_applied_volume = music_fade_to_volume;
		}

		if ( music_applied_volume > user_volume )
		{
			music_applied_volume = user_volume;
		}
	}

	if ( old_volume != music_applied_volume ||
		(old_volume == 0.0f && music_applied_volume == 0.0f) ||
		force_update )
	{
		if ( music_stream )
		{
			music_stream->set_volume( music_applied_volume );
		}
	}
}

#endif

//---------------------------------------------------------------

#ifdef GCCULL
void game::clear_ambients()
{
	ambient_stream.sp_region_node = 0;
	next_ambient_stream.sp_region_node = 0;

	if( ambient_stream.sp_stream )
	{
		ambient_stream.sp_stream->release();
		ambient_stream.sp_stream = 0;
	}
	if( next_ambient_stream.sp_stream )
	{
		next_ambient_stream.sp_stream->release();
		next_ambient_stream.sp_stream = 0;
	}
}

const time_value_t base_ambient_delay = 0.0f;

// NOT TIME-BASED, RATHER HERO-POSITION-BASED
void game::process_ambients( time_value_t delta_t )
{
	entity* hero = g_world_ptr->get_hero_ptr(active_player);	// game::process_ambients(): wtf is this? (multiplayer fixme?)
	if( hero )
	{
		region_graph::node *rnp = hero->get_region();
		if( rnp )
		{
			if( rnp != ambient_stream.sp_region_node )
			{
				// SWAP MAIN REGION WITH SECONDARY
				_ambient_stream ts = ambient_stream;
				ambient_stream = next_ambient_stream;
				next_ambient_stream = ts;
			}
			if( rnp != ambient_stream.sp_region_node )
			{
				stringx refstr = rnp->get_data()->get_region_ambient_sound_name();
				if( (!refstr.length()) || (ambient_stream.sp_stream && refstr.length() && (ambient_stream.sp_stream->get_file_name() != refstr)) )
				{
					// THROW AWAY OLD STREAM
					if( ambient_stream.sp_stream )
						ambient_stream.sp_stream->release();
					ambient_stream.sp_stream = 0;
					ambient_stream.sp_region_node = 0;
				}
				else if( ambient_stream.sp_stream && refstr.length() )
				{
					ambient_stream.sp_region_node = rnp;
				}
				if( !ambient_stream.sp_region_node )
				{
					ambient_stream.sp_region_node = rnp;
					if( refstr.length() )
					{
						//            ambient_stream.sp_stream = sound_device::inst()->create_stream( refstr );
						//            ambient_stream.s_base_volume = ambient_stream.sp_region_node->get_data()->get_region_ambient_sound_volume();
						//            ambient_stream.s_cur_volume = 1.0f;
						//            ambient_stream.sp_stream->request_queue( false, true );
					}
				}
			}

			// NOW SCAN LIST OF CONNECTED REGIONS
			vector3d hv = hero->get_abs_position();
			region_graph::node::iterator rit;
			for( rit = rnp->begin(); rit != rnp->end(); ++rit )
			{
				if( (*rit).get_data()->is_active() )
				{
					vector3d pv = (*rit).get_data()->get_effective_center();
					rational_t d = (pv-hv).length();
					rational_t r2 = (*rit).get_data()->get_effective_radius() *1.0f;
					if( r2 == 0.0f ) r2 = 0.00001f;

					// THIS ASSUMES THAT ONLY ONE PORTAL-SPHERE CAN BE INTERSECTED AT A SINGLE TIME
					if( d <= r2 )
					{
						if( !next_ambient_stream.sp_stream )
						{
							next_ambient_stream.sp_region_node = (*rit).get_data()->get_front();
							if( next_ambient_stream.sp_region_node == ambient_stream.sp_region_node )
								next_ambient_stream.sp_region_node = (*rit).get_data()->get_back();

							stringx refstr = next_ambient_stream.sp_region_node->get_data()->get_region_ambient_sound_name();
							if( refstr.length() )
							{
								//                next_ambient_stream.sp_stream = sound_device::inst()->create_stream( refstr );
								//                next_ambient_stream.s_base_volume = next_ambient_stream.sp_region_node->get_data()->get_region_ambient_sound_volume();
								//                next_ambient_stream.sp_stream->request_queue( false, true );
							}
						}
						ambient_stream.s_cur_volume = 0.5f + d/(r2*2.0f);
						if( ambient_stream.s_cur_volume > 1.0f )
							ambient_stream.s_cur_volume = 1.0f;
						if( ambient_stream.s_cur_volume < 0.0f )
							ambient_stream.s_cur_volume = 0.0f;
						next_ambient_stream.s_cur_volume = 0.5f - d/(r2*2.0f);
						if( next_ambient_stream.s_cur_volume > 1.0f )
							next_ambient_stream.s_cur_volume = 1.0f;
						if( next_ambient_stream.s_cur_volume < 0.0f )
							next_ambient_stream.s_cur_volume = 0.0f;
						break; // for() loop
					}
					else
					{
						ambient_stream.s_cur_volume = 1.0f;
						next_ambient_stream.s_cur_volume = 0.0f;
					}
				}
			}

			if( ambient_stream.sp_stream )
			{
				ambient_stream.sp_stream->set_volume( ambient_stream.s_base_volume * ambient_stream.s_cur_volume );
				if( ambient_stream.s_cur_volume <= 0.0f )
				{
					ambient_stream.sp_stream->release();
					ambient_stream.sp_stream = 0;
					ambient_stream.sp_region_node = 0;
				}
			}
			if( next_ambient_stream.sp_stream )
			{
				next_ambient_stream.sp_stream->set_volume( next_ambient_stream.s_base_volume * next_ambient_stream.s_cur_volume );
				if( next_ambient_stream.s_cur_volume <= 0.0f )
				{
					next_ambient_stream.sp_stream->release();
					next_ambient_stream.sp_stream = 0;
					next_ambient_stream.sp_region_node = 0;
				}
			}
    }
  }
}
#endif

// the actual user-perceived framerate
float game::get_instantaneous_fps() const
{
	assert(total_delta > 0);
	return 1.0F / total_delta;
}

float game::get_min_fps() const
{
	assert(min_delta > 0);
	return 1.0F / min_delta;
}

float game::get_max_fps() const
{
	assert(max_delta > 0);
	return 1.0F / max_delta;
}

float game::get_avg_fps() const  // for last MAX_FRAMES frames
{
	assert(avg_delta > 0);
	return 1.0F / avg_delta;
}

// theoretical max framerate
float game::get_theoretical_fps() const
{
	if (total_delta - flip_delta - limit_delta < 0.0001f) return 10000.0f;
	return 1.0f / (total_delta - flip_delta - limit_delta);
}

// this version is designed for console development
float game::get_console_fps() const
{
	if (total_delta - flip_delta < 0.0001f) return 60.0f;
	// compute what the FPS would be assuming NTSC frame lock
	return floor(60.0f / ceil(60.0f * total_delta));
}

// clears the screen to black
void game::clear_screen()
{
#ifdef TARGET_PC
	if (p_blank_material == NULL)
	{
		p_blank_material = material_bank.new_instance( material(os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR)+"\\alpha") );
		hw_texture_mgr::inst()->set_white_texture(p_blank_material->get_texture(0));
	}
	hw_rasta::inst()->begin_scene();
	clear_zbuffer();
	hw_rasta::inst()->end_scene();
	hw_rasta::inst()->flip();
	hw_rasta::inst()->begin_scene();
	clear_zbuffer();
	hw_rasta::inst()->end_scene();
#endif
}

extern void system_idle();

// This function has been changed to run it's own loop while the movie is playing, there's no reason
// to keep running through app::tick() when we're playing a damn movie (except we call system_idle()
// instead now, how lovely).

#ifdef TARGET_PS2
bool game::play_movie(const char *movie_name, bool canskip, int width, int height ) // PATH relative to data is now required to be sent in
#else
bool game::play_movie(const char *movie_name, bool canskip ) // PATH relative to data is now required to be sent in
#endif
{
	bool skip_all_movies=false;
	STOP_PS2_PC;

	//<NOTE><MIA> this really is not a good place for this...the logic above this, should determine
	// if this function is called or not..sticking it here is a failsafe
	if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_MOVIES))
		return false;

	flag.play_movie = true;

#if defined(TARGET_XBOX)
	//  FIXME: Why isn't this done in the Xbox movieplayer?
	nvlInit(nslGetDirectSoundObjXbox());
	strcpy(g_movie_name, "d:\\movies\\" );
	strcat(g_movie_name, movie_name);
	strcat(g_movie_name, ".bik");
#elif defined(TARGET_PS2)
	/*debug_print("file manager prior-%d",tmGetTreadPriority(file_manager::inst()->init_context_id));
	debug_print("Main prior-%d",tmGetMainTreadPriority());*/
  movieplayer::inst()->initHiRes(width, height);
  sprintf(g_movie_name, "cdrom0:\\MOVIES\\%s.PSS;1", movie_name);
#elif defined(TARGET_GC)
	// FIXME: Boo.
	memcpy(&g_career->cfg, StoredConfigData::inst()->getGameConfig(), sizeof(ksConfigData));

	strcpy( g_movie_name, "movies/" );
	strcat( g_movie_name, movie_name );
	strcat( g_movie_name, ".bik" );
#endif

  bool key_down = false;
  int frame = 0;

  input_mgr* inputmgr = input_mgr::inst();

  // just play the damn thing...
  for ( ;; )
  {
    bool movie_finished = false;

    system_idle();

    if (flag.play_movie)
    {
      flag.play_movie = false;

      movieplayer::inst()->play(g_movie_name, true, true);

#ifdef TARGET_PS2
      if(!movieplayer::inst()->is_playing())
      {
        sprintf(g_movie_name, "host0:MOVIES\\%s.PSS", movie_name);
        movieplayer::inst()->play(g_movie_name, true, true);
      }
#endif

#ifdef TARGET_XBOX
		script_pad[0].update();
		input_mgr::inst()->scan_devices();
#endif
	  inputmgr->poll_devices();
		if ( inputmgr->get_control_state( ANY_LOCAL_JOYSTICK,PSX_X) == AXIS_MAX ||
				inputmgr->get_control_state( ANY_LOCAL_JOYSTICK,PSX_CIRCLE) == AXIS_MAX ||
				inputmgr->get_control_state( ANY_LOCAL_JOYSTICK,PSX_SQUARE) == AXIS_MAX ||
				inputmgr->get_control_state( ANY_LOCAL_JOYSTICK,PSX_TRIANGLE) == AXIS_MAX )
			{
				key_down = true;
			}

		}

		if (!movieplayer::inst()->is_playing())
		{
			//movieplayer::inst()->stop(); // but the movie isn't playing...
			movie_finished = true;
			clear_screen();
		}
		else
		{
			// check for end of movie

			// app.cpp does this, and will cancel out your input
			// scroll up a bit...you'll see that no app functions are called..the movie player is the only thing taking up cpu cycles at this time
			// if the app or the input_mgr ran in a seperate thread, then sure...but it's not
#ifdef TARGET_XBOX
			script_pad[0].update();
			input_mgr::inst()->scan_devices();
#endif
			inputmgr->poll_devices();

			if (key_down)
			{
				if (inputmgr->get_control_state( ANY_LOCAL_JOYSTICK,PSX_X) != AXIS_MAX &&
					inputmgr->get_control_state( ANY_LOCAL_JOYSTICK,PSX_CIRCLE) != AXIS_MAX &&
					inputmgr->get_control_state( ANY_LOCAL_JOYSTICK,PSX_SQUARE) != AXIS_MAX &&
					inputmgr->get_control_state( ANY_LOCAL_JOYSTICK,PSX_TRIANGLE) != AXIS_MAX)
				{
					key_down = false;
				}
			}
			else if ( canskip )
			{
				if (inputmgr->get_control_state( ANY_LOCAL_JOYSTICK,PSX_X) == AXIS_MAX ||
					inputmgr->get_control_state( ANY_LOCAL_JOYSTICK,PSX_CIRCLE) == AXIS_MAX ||
					inputmgr->get_control_state( ANY_LOCAL_JOYSTICK,PSX_SQUARE) == AXIS_MAX ||
					inputmgr->get_control_state( ANY_LOCAL_JOYSTICK,PSX_TRIANGLE) == AXIS_MAX ||
					inputmgr->get_control_state( ANY_LOCAL_JOYSTICK,PSX_START) == AXIS_MAX )
				{
					movieplayer::inst()->stop();
					movie_finished = true;
					clear_screen();

#ifdef TARGET_PS2
          // On PS2, pressing start skips all movies
          if(inputmgr->get_control_state( JOYSTICK_DEVICE,PSX_START) == AXIS_MAX)
					  skip_all_movies = true;
#endif
				}
			}
		}

		if (movie_finished)
			break;

		movieplayer::inst()->start_frame();
#if defined(TARGET_XBOX)
		// FIXME: Same here as above with nvl* in the XBOX section.
		nvlAdvance();
#endif
		movieplayer::inst()->end_frame();
		frame++;
  }

  // make sure the movie IS stopped so NVL doesn't crap out
  movieplayer::inst()->stop();
#if defined(TARGET_PS2)
  movieplayer::inst()->shutdown();
#elif defined(TARGET_XBOX)
	// FIXME: Once more, with feeling.
  nvlShutdown();
#endif

  START_PS2_PC;

  return( skip_all_movies );
}

//#define NO_MUSIC

#ifdef GCCULL
// *** NEW music manager ***
bool game::is_music_playing()
{
#ifdef NO_MUSIC
	return true;
#else
	return (music_track[0] != NULL);
#endif
}

bool game::is_music_queued()
{
#ifdef NO_MUSIC
	return true;
#else
	return (music_track[1] != NULL);
#endif
}

bool game::play_music( const pstring &music_name, bool force_it )
{
#ifdef GCCULL
#ifndef NO_MUSIC
	sound_instance *temp_inst = NULL;

	switch (music_state)
	{
	case MUSIC_STATE_QUIET:
		assert(music_track[0] == NULL && music_track[1] == NULL);

		//      music_track[0] = sound_device::inst()->create_sound(music_name);
		if (music_track[0] == NULL)
		{
			warning("Could not find music track %s.", music_name.c_str());
			return false;
		}
		music_track[0]->preload();

		// go to play pending
		music_state = MUSIC_STATE_PLAY_PENDING;
		break;

	case MUSIC_STATE_PLAY_PENDING:
	case MUSIC_STATE_PLAYING:
		// enqueue and preload
		assert(music_track[0] != NULL);

		//      temp_inst = sound_device::inst()->create_sound(music_name);

		if (temp_inst == NULL)
		{
			warning("Could not find music track %s.", music_name.c_str());
			return false;
		}

		if (music_track[1] != NULL)
		{
			music_track[1]->release();
		}
		music_track[1] = temp_inst;
		music_track[1]->preload();
		if (force_it == true)
		{
			music_state = MUSIC_STATE_FORCED_CHANGE_PENDING;
		}
		break;

	case MUSIC_STATE_FORCED_CHANGE_PENDING:
		warning("Scripters tried to force playback too soon after the last forced playback.  Bad scripters.");
		break;
	}
#endif
#endif
	return true;
}

void game::stop_music(bool signal_scripts)
{
#ifdef GCCULL
#ifndef NO_MUSIC
	if (music_track[0] != NULL)
	{
		music_track[0]->release();
		music_track[0] = NULL;
	}
	if (music_track[1] != NULL)
	{
		music_track[1]->release();
		music_track[1] = NULL;
	}
	music_state = MUSIC_STATE_QUIET;
	if (signal_scripts)
		get_script_mfg()->raise_signal(script_mfg::MUSIC_FINISHED);
#endif
#endif
}

void game::update_music( time_value_t time_inc )
{
#ifdef GCCULL
#ifndef NO_MUSIC
	switch (music_state)
	{
	case MUSIC_STATE_PLAY_PENDING:
		assert(music_track[0] != NULL);

		if (music_track[0]->is_ready())
		{
			// begin playback
			music_track[0]->play();
			music_state = MUSIC_STATE_PLAYING;
		}
		break;

	case MUSIC_STATE_PLAYING:
		assert(music_track[0] != NULL);

		// check if we are finished with track0
		if (music_track[0]->is_playing() == false)
		{
			music_track[0]->release();
			if (music_track[1] != NULL)
			{
				// if so, and we have a next track ready, begin its playback
				music_track[0] = music_track[1];
				music_track[1] = NULL;
				music_track[0]->play();
				m_script_mfg.raise_signal(script_mfg::MUSIC_TRACK_SWITCH);
			}
			else
			{
				// if not, go to quiet
				music_track[0] = NULL;
				music_state = MUSIC_STATE_QUIET;
				m_script_mfg.raise_signal(script_mfg::MUSIC_FINISHED);
			}
		}
		break;

	case MUSIC_STATE_FORCED_CHANGE_PENDING:
		assert(music_track[0] != NULL && music_track[1] != NULL);

		if (music_track[1]->is_ready())
		{
			music_track[1]->play();
			music_track[0]->release();
			music_track[0] = music_track[1];
			music_track[1] = NULL;
			music_state = MUSIC_STATE_PLAYING;
			m_script_mfg.raise_signal(script_mfg::MUSIC_TRACK_SWITCH);
			debug_print("Music track switch");
		}
		break;

	case MUSIC_STATE_QUIET:
		// do nothing
		break;
	}
#endif
#endif
}
#endif

void game::update_game_mode(const float time_inc)
{
	int	i;

	// Update push mode.
	if (!is_paused() && play_mode.push)
	{
		play_mode.push->Update(time_inc);

		// Notice viewport changes.
		assert(MAX_PLAYERS == 2);
		if (play_mode.push->GetPlayerViewport(0).get_right() != player_viewports[0].get_right())
		{
			// Update viewport in response to player pushing.
			for (i = 0; i < MAX_PLAYERS; i++)
				player_viewports[i] = play_mode.push->GetPlayerViewport(i);

			// Did this player destroy the other player?
			if (play_mode.push->GetPlayerShare(0) == 1.0f)
				set_active_player(0);
			else if (play_mode.push->GetPlayerShare(1) == 1.0f)
				set_active_player(1);

			// Tell IGO to notice viewport change.
			frontendmanager.IGO->OnViewportChange();
		}
	}

	// Update time attack mode.
	if (play_mode.timeAttack)
	{
		play_mode.timeAttack->Update(time_inc);
	}

	// Update meter attack mode.
	if (play_mode.meterAttack)
	{
		play_mode.meterAttack->Update(time_inc);
	}

	// Update head to head mode.
	if (!is_paused() && play_mode.headToHead)
	{
		play_mode.headToHead->Update(time_inc);
	}
}

void game::enable_physics(bool on)
{
	flag.game_paused = !on;
}

//	SetBoardIdx()
// Sets the texture to use for the surfer's board.
// Must be 0 thru 8.
void game::SetBoardIdx(int hero_num, const int idx)
{
	assert(hero_num >= 0 && hero_num < MAX_PLAYERS);
	boardIdx[hero_num] = idx;
}

void game::SetUsingPersonalitySuit(int hero_num, bool val)
{
	personality[hero_num] = val;
	if (GetUsingPersonalitySuit(hero_num))
		heroname[hero_num] = SurferDataArray[GetSurferIdx(hero_num)].name_ps;
	else
		heroname[hero_num] = SurferDataArray[GetSurferIdx(hero_num)].name;
}

//	SetSurferIdx()
// Sets the current surfer.
// This method makes setHeroname() obsolete.
void game::SetSurferIdx(int hero_num, int surfer)
{
	assert(hero_num >= 0 && hero_num < MAX_PLAYERS);
	assert(  surfer >= 0 &&   surfer < SURFER_LAST);

	surferIdx[hero_num] = surfer;
	if (GetUsingPersonalitySuit(hero_num))
		heroname[hero_num] = SurferDataArray[surfer].name_ps;
	else
		heroname[hero_num] = SurferDataArray[surfer].name;
}

//	set_game_mode()
// Sets the current gameplay mode.
// Must be called before loading the beach.
void game::set_game_mode(game_mode_t m)
{
	game_mode = m;

	switch (game_mode)
	{
	case GAME_MODE_CAREER :
		num_players = 1;
		num_active_players = 1;
		num_ai_players = 0;
		break;
	case GAME_MODE_FREESURF_INFINITE :
	case GAME_MODE_FREESURF_HIGHSCORE :
	case GAME_MODE_FREESURF_ICON :
		num_players = 1;
		num_active_players = 1;
		num_ai_players = 0;
		break;
	case GAME_MODE_PRACTICE :
		num_players = 1;
		num_active_players = 1;
		num_ai_players = 0;
		break;
	case GAME_MODE_TIME_ATTACK :
		num_players = 2;
		num_active_players = 1;
		num_ai_players = 0;
		break;
	case GAME_MODE_METER_ATTACK :
		num_players = 2;
		num_active_players = 1;
		num_ai_players = 0;
		break;
	case GAME_MODE_SEA_HORSE :
		num_players = 2;
		num_active_players = 1;
		num_ai_players = 0;
		break;
	case GAME_MODE_HEAD_TO_HEAD :
		num_players = 2;
		num_active_players = 2;
		num_ai_players = 0;
		break;
	case GAME_MODE_PUSH :
		num_players = 2;
		num_active_players = 2;
		num_ai_players = 0;
		break;
	case GAME_MODE_AI :
		num_players = 1;
		num_active_players = 1;
		num_ai_players = 1;
		break;
	default : assert(false);
	}
}

//	get_player_viewport()
// Retrieves the rect describing the specified player's render viewport.
const recti & game::get_player_viewport(const int playerIdx)
{
	assert(playerIdx >= 0 && playerIdx < num_players);

	return player_viewports[playerIdx];
}

//	set_active_player()
// Private helper function: sets the specified player to active, and
// makes all other players inactive.
void game::set_active_player(const int n)
{
	assert(n >= 0 && n < num_players);

	active_player = n;

	// Set this player to active, and all the rest to inactive.
	for (int i = 0; i < num_players; i++)
	{
		// Note: when changing these, change the code in retry_level too.
		the_world->get_ks_controller(i)->set_active(active_player == i);
		the_world->get_hero_ptr(i)->set_visible(active_player == i);
		the_world->get_ks_controller(i)->GetBoard()->set_visible(active_player == i);
		the_world->get_ks_controller(i)->GetBoardModel()->set_visible(active_player == i);
		the_world->get_ks_controller(i)->GetBoardMember()->set_visible(active_player == i);
	}
}

extern int show_anim_label;
void game::draw_debug_labels()
{
#ifdef DEBUG
	int num_players = this->get_num_players();
	for (int n = 0; n < num_players; n++)
	{
		kellyslater_controller *ksctrl = g_world_ptr->get_ks_controller(n);
		if (ksctrl && ksctrl->is_active())
			ksctrl->draw_debug_labels();
	}
	if (show_anim_label)
	{
		for (beach_object *fobj = g_beach_ptr->my_objects; fobj != NULL; fobj = fobj->next)
		{
			if (!fobj->is_active () || !fobj->spawned)
				continue;

			if (!fobj->is_physical ())
				continue;

			((surfing_object*)fobj)->draw_debug_labels();
		}
	}
#endif
}

void game::set_num_ai_players(int i) {

	assert (i < 2 && i >= 0);
	num_ai_players = i;


}

void game::LoadingStateInit()
{
	current_loading_state = 0;
	current_loading_sub_state = 0;
	loading_progress = 0;
	last_loading_progress = 0;

	// these are all estimates, which means that the first fe->game loading
	// progress won't be quite right.
	total_sub_states_common = 41;
	total_sub_states_beach = 38;
	total_sub_states_surfer1 = 6;
	total_sub_states_surfer1_aux = 10;
	total_sub_states_surfer2 = 6;
	total_sub_states_surfer2_aux = 10;

	start_drawing_map = false;
}

void game::LoadingStateReset()
{
	current_loading_state = 0;
	loading_progress = 0;
	last_loading_progress = 0;
}

void game::LoadingStateUpdate()
{
	switch(current_loading_state)
	{
	case LOADING_COMMON_STASH:
	case LOADING_BEACH_STASH:
	case LOADING_HERO_1_STASH:
	case LOADING_HERO_1_AUX_STASH:
	case LOADING_HERO_2_STASH:
	case LOADING_HERO_2_AUX_STASH:
		if(current_loading_sub_state < 0)	// done
		{
			current_loading_state++;
			current_loading_sub_state = 0;
		}
		else
			current_loading_sub_state++;
		break;
	default: current_loading_state++; break;
	}

	// skip second surfer loading if only 1 player
	if(num_players != 2 && (current_loading_state == LOADING_HERO_2_STASH ||
		current_loading_state == LOADING_HERO_2_AUX_STASH ||
		current_loading_state == LOADING_HERO_2_REST))
		current_loading_state = LOADING_SCENE_END;

	start_drawing_map = true;
}

void game::LoadingStatePrint()
{
	stringx tmp;
	switch(current_loading_state)
	{
	case LOADING_INITIAL:			tmp = "Initial Stuff"; break;
	case LOADING_WORLD_CREATE:		tmp = "New World"; break;
	case LOADING_RESET_SOUND:		tmp = "Sound Reset"; break;
	case LOADING_COMMON_STASH:		tmp = "Common Stash"; break;
	case LOADING_BEACH_STASH:		tmp = "Beach Stash"; break;
	case LOADING_INIT_DBMENU:		tmp = "Init Debug Menu"; break;
	case LOADING_SCENE:				tmp = "Scene"; break;
	case LOADING_HERO_1_STASH:		tmp = "Hero 1 Stash"; break;
	case LOADING_HERO_1_AUX_STASH:	tmp = "Hero 1 Aux Stash"; break;
	case LOADING_HERO_1_REST:		tmp = "Hero 1 Rest"; break;
	case LOADING_HERO_2_STASH:		tmp = "Hero 2 Stash"; break;
	case LOADING_HERO_2_AUX_STASH:	tmp = "Hero 2 Aux Stash"; break;
	case LOADING_HERO_2_REST:		tmp = "Hero 2 Rest"; break;
	case LOADING_SCENE_END:			tmp = "Scene End"; break;
	case LOADING_GAME_MODE:			tmp = "Game Mode"; break;
	case LOADING_LENS_FLARE:		tmp = "Lens Flare"; break;
	case LOADING_SCRIPT:			tmp = "Script"; break;
	case LOADING_PERFORMANCE:		tmp = "Performance"; break;
	case LOADING_STATE_DONE:		tmp = "Done"; break;
	default:						tmp = "Unknown"; break;
	}
	nglPrintf("Currently loading %s\n", tmp.data());
}

void game::LoadingStateSkipSurfer(bool surfer1)
{
	// put to end of aux stash loading so it will resume the next state on Update
	current_loading_sub_state = -1;
	if(surfer1)
		current_loading_state = LOADING_HERO_1_AUX_STASH;
	else current_loading_state = LOADING_HERO_2_AUX_STASH;
}

void game::LoadingProgressUpdate()
{
	// right now, there are 4 stash loads, and 9 in between states.
	// if i estimate that the 4 stash loads take 20% apiece, then the
	// in between states take up 2.22% apiece

	// this is all a gross generalization, of course
	/*
	loading_progress = 0.0222f * current_loading_state;

	  if(current_loading_state > LOADING_HERO_2)
	  loading_progress += 0.8f;
	  else if(current_loading_state == LOADING_HERO_2)
	  loading_progress += 0.6f + 0.2f*stash_loading_percent;
	  else if(current_loading_state == LOADING_HERO_1)
	  loading_progress += 0.4f + 0.2f*stash_loading_percent;
	  else if(current_loading_state > LOADING_BEACH_STASH)
	  loading_progress += 0.4f;
	  else if(current_loading_state == LOADING_BEACH_STASH)
	  loading_progress += 0.2f + 0.2f*stash_loading_percent;
	  else if(current_loading_state == LOADING_COMMON_STASH)
	  loading_progress += 0.2f*stash_loading_percent;
	*/

	if(num_players == 1)
	{
		total_sub_states_surfer2 = 0;
		total_sub_states_surfer2_aux = 0;
	}

	int total_states = LOADING_STATE_DONE + total_sub_states_common +
		total_sub_states_beach + total_sub_states_surfer1 +
		total_sub_states_surfer1_aux + total_sub_states_surfer2 +
		total_sub_states_surfer2_aux;

	int total = current_loading_state + current_loading_sub_state;

	if(current_loading_state > LOADING_COMMON_STASH) total += total_sub_states_common;
	if(current_loading_state > LOADING_BEACH_STASH) total += total_sub_states_beach;
	if(current_loading_state > LOADING_HERO_1_STASH) total += total_sub_states_surfer1;
	if(current_loading_state > LOADING_HERO_1_AUX_STASH) total += total_sub_states_surfer1_aux;
	if(current_loading_state > LOADING_HERO_2_STASH) total += total_sub_states_surfer2;
	if(current_loading_state > LOADING_HERO_2_AUX_STASH) total += total_sub_states_surfer2_aux;

	loading_progress = total/((float) total_states);
	if(loading_progress < last_loading_progress)
		loading_progress = last_loading_progress;
	last_loading_progress = loading_progress;
}

void game::SetStashSize(int loading_state, int size)
{
	int sub_states = (int)(size/(1024*128.0f));
	// add a bit to cover truncating errors
	sub_states += 1;

	switch(loading_state)
	{
	case LOADING_COMMON_STASH:		total_sub_states_common = sub_states; break;
	case LOADING_BEACH_STASH:		total_sub_states_beach = sub_states; break;
	case LOADING_HERO_1_STASH:		total_sub_states_surfer1 = sub_states; break;
	case LOADING_HERO_1_AUX_STASH:	total_sub_states_surfer1_aux = sub_states; break;
	case LOADING_HERO_2_STASH:		total_sub_states_surfer2 = sub_states; break;
	case LOADING_HERO_2_AUX_STASH:	total_sub_states_surfer2_aux = sub_states; break;
	default:
		nglPrintf("invalid state\n"); assert(0);
		break;
	}
}


extern float g_takeoff_current_offset;
vector3d game::calc_beach_spawn_pos()
{
	int			curbeach = g_game_ptr->get_beach_id();
	int			current_wave = WAVE_GetIndex();
	vector3d	take_offset,spawnPos, current;

	if (BeachDataArray[curbeach].bdir)
		current.x = WaveDataArray[current_wave].speedx;
	else
		current.x = -WaveDataArray[current_wave].speedx;

	current.y = 0.0f;
	current.z = -WaveDataArray[current_wave].speedz;

	take_offset = - g_takeoff_current_offset*current;

	// Pick spawn position for player.
	if (!g_game_ptr->is_splitscreen())
	{
		spawnPos = *WAVE_GetMarker(WAVE_MarkerSpawnEnd) + take_offset;
	}
	// For splitscreen modes, we need to make sure player is starting in a safe spot.
	else
	{

		spawnPos = *WAVE_GetMarker(WAVE_MarkerSplitscreenSpawnEnd) + take_offset;
	}

	// make sure the surfer is inside the wave mesh
	assert ((spawnPos.x < WAVE_MeshMaxX) && (spawnPos.x > WAVE_MeshMinX) && (spawnPos.z < WAVE_MeshMaxZ) && (spawnPos.z > WAVE_MeshMinZ));

	return spawnPos;
}

//	get_player_share()
// Returns the % of the screen that the specified player owns.
float game::get_player_share(const int playerIdx) const
{
	assert(playerIdx >= 0 && playerIdx < num_players);

	// Shares slide in push mode.
	if (play_mode.push)
		return play_mode.push->GetPlayerShare(playerIdx);
	// Each player controls 50% of the screen in head to head mode.
	else if (play_mode.headToHead)
		return 0.5f;
	// Change this function when new modes are added.
	else
		return 1.0f;
}
int game::get_first_beach()
{
	int i;
	int location = BeachDataArray[g_game_ptr->get_beach_id()].map_location;
	for (i=0; i < BEACH_LAST; i++)
	{
		if (BeachDataArray[i].map_location == location)
			return i;
	}
	assert(0 && "COULD NOT FIND THIS BEACH'S LOCATION IN THE BEACHDATAARRAY");
	return -1;
}
stringx game::get_beach_location_name()
{
	return BeachDataArray[get_first_beach()].name;
}
stringx game::get_beach_board_name(int location)
{
	int i;
	for (i=0; i < BEACH_LAST; i++)
	{
		if (BeachDataArray[i].map_location == location)
			return BeachDataArray[i].name;
	}
	assert(0 && "SEARCHING FOR AN INVALID BEACH NAME");
	return stringx("");
}


int game::get_last_surfer_index()
{
	int surferIdx = -1;
	int order = 1000;
	for (int i=0; i < SURFER_LAST; i++)
	{
		if (SurferDataArray[i].sort_order >= 0 && SurferDataArray[i].sort_order > order)
		{
			surferIdx = i;
			order = SurferDataArray[i].sort_order;
		}
	}

	return surferIdx;
}

int game::get_first_surfer_index()
{
	int surferIdx = -1;
	int order = 1000;
	for (int i=0; i < SURFER_LAST; i++)
	{
		if (SurferDataArray[i].sort_order >= 0 && SurferDataArray[i].sort_order < order)
		{
			surferIdx = i;
			order = SurferDataArray[i].sort_order;
		}
	}

	return surferIdx;
}

int game::get_next_surfer_index(int current_index)
{
	int nextSurferIndex = -1;
	int nextSurferSortOrder = 100;
	int adjSortOrder, nextAdjSortOrder;
	int currSurferOrder = SurferDataArray[current_index].sort_order;
	int i;
	for (i=0; i < SURFER_LAST; i++)
	{
		if (SurferDataArray[i].sort_order < 0 || i == current_index)
			continue;
		// Adjust this sort order so it is always higher
		if (SurferDataArray[i].sort_order < currSurferOrder)
			adjSortOrder = SurferDataArray[i].sort_order + SURFER_LAST;
		else
			adjSortOrder = SurferDataArray[i].sort_order;

		// Adjust the previous best sort order so it is always higher
		if (nextSurferSortOrder < currSurferOrder)
			nextAdjSortOrder = nextSurferSortOrder + SURFER_LAST;
		else
			nextAdjSortOrder = nextSurferSortOrder;

		// What is the difference between the two?
		if ((nextAdjSortOrder - currSurferOrder) > (adjSortOrder - currSurferOrder))
		{
			nextSurferSortOrder = SurferDataArray[i].sort_order;
			nextSurferIndex			= i;
		}
	}
	assert(nextSurferIndex >= 0 && nextSurferIndex < SURFER_LAST);
	return nextSurferIndex;
}

int game::get_prev_surfer_index(int current_index)
{
	int prevSurferIndex = -1;
	int prevSurferSortOrder = -100;
	int adjSortOrder, prevAdjSortOrder;
	int currSurferOrder = SurferDataArray[current_index].sort_order;
	int i;
	for (i=0; i < SURFER_LAST; i++)
	{
		if (SurferDataArray[i].sort_order < 0 || i == current_index)
			continue;
		// Adjust this sort order so it is always higher
		if (SurferDataArray[i].sort_order > currSurferOrder)
			adjSortOrder = SurferDataArray[i].sort_order - SURFER_LAST;
		else
			adjSortOrder = SurferDataArray[i].sort_order;

		// Adjust the previous best sort order so it is always higher
		if (prevSurferSortOrder > currSurferOrder)
			prevAdjSortOrder = prevSurferSortOrder - SURFER_LAST;
		else
			prevAdjSortOrder = prevSurferSortOrder;

		// What is the difference between the two?
		if ((currSurferOrder - prevAdjSortOrder) > (currSurferOrder - adjSortOrder))
		{
			prevSurferSortOrder = SurferDataArray[i].sort_order;
			prevSurferIndex			= i;
		}
	}
	assert(prevSurferIndex >= 0 && prevSurferIndex < SURFER_LAST);
	return prevSurferIndex;
}
void game::set_player_handicap(int hero, int new_handicap)
{
	if (new_handicap > globalCareerData.getMaxHandicap(g_game_ptr->GetSurferIdx(hero)))
		handicap[hero] = globalCareerData.getMaxHandicap(g_game_ptr->GetSurferIdx(hero));
	else
		handicap[hero] = new_handicap;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	GameEventRecipient class
//
//////////////////////////////////////////////////////////////////////////////////////////////////////

/*

  //	GameEventRecipient()
  // Default constructor.
  GameEventRecipient::GameEventRecipient()
  {
  int	i;

	numSnapshots = NULL;

	  for (i = 0; i < MAX_SNAPSHOTS; i++)
	  snapshotScores[i] = NULL;

		recordNextChain = false;
		}

		  //	~GameEventRecipient()
		  // Destructor.
		  GameEventRecipient::~GameEventRecipient()
		  {

			}

			  //	Init()
			  // Sets the pointers to the game's variables that this object modifies.
			  void GameEventRecipient::Init(int * numSnaps, int * snapScores[MAX_SNAPSHOTS])
			  {
			  int i = 0;

				numSnapshots = numSnaps;

				  for (i = 0; i < MAX_SNAPSHOTS; i++)
				  snapshotScores[i] = snapScores[i];
				  }

					//	OnEvent()
					// Responds to appropriate events.
					void GameEventRecipient::OnEvent(const EVENT event, const int param1, const int param2)
					{
					if (recordNextChain)
					{
					if (event == EVT_SCORING_CHAIN_END)
					{
					ScoringManager::CHAININFO *	lastChain;

					  lastChain = g_world_ptr->get_ks_controller(param1)->get_my_scoreManager().GetLastChainInfo();


						  //Press release - re-add this code later.
						  //if (param2)
						  //	*snapshotScores[(*numSnapshots)-1] = lastChain->points;
						  //else
						  //	*snapshotScores[(*numSnapshots)-1] = 0;
						  //

							recordNextChain = false;
							}
							}
							}

*/
