#include "global.h"
#include "ksdbmenu.h"
#include "app.h"
#include "wave.h"
#include "wavetex.h"
#include "ksreplay.h"
//#if defined(TARGET_XBOX)
#include "conglom.h"
#include "inputmgr.h"
#include "kellyslater_controller.h"
#include "beachdata.h"
#include "camera.h"
#include "wds.h"
#include "game_info.h"
//#endif /* TARGET_XBOX JIV DEBUG */
#include "WaveSound.h"
#include "game.h"
#include "game_process.h"
#include "rumblemanager.h"
#include "profiler.h"
#include "menusound.h"

#ifndef countof
#define countof(x) ( sizeof x / sizeof *x )
#endif

bool superduperpausehack=false;
extern int g_wipeout_blur;
void KSMainMenu::OnOpen( Menu *cto, MenuSystem *c )
{
	superduperpausehack=false;
//	app::inst()->get_game()->pause();
	Menu::OnOpen(cto,c);
}

void KSMainMenu::OnClose( bool toparent )
{
	Menu::OnClose(toparent);
//	app::inst()->get_game()->unpause();
}

#if defined(TARGET_XBOX)
MenuRect menurect={ 40,100,340,400 };
#else
	#if defined(WIDE_MENUS)
		MenuRect menurect={ 10,100,400,400 };
	#else
		MenuRect menurect={ 10,100,210,400 };
	#endif

#endif /* TARGET_XBOX JIV DEBUG */


MenuSystem *menus=NULL; //MenuSystem(menurect);

MENUENTRYBUTTONFUNCTION(NullButton)
{
	return true;
}

MENUENTRYBUTTONFUNCTION(DebugButton)
{
	switch ( buttonid )
	{
		case MENUCMD_NONE    : debug_print("Button %s pressed\n","MENUCMD_NONE    "); break;
		case MENUCMD_SELECT  : debug_print("Button %s pressed\n","MENUCMD_SELECT  "); break;
		case MENUCMD_START   : debug_print("Button %s pressed\n","MENUCMD_START   "); break;
		case MENUCMD_UP      : debug_print("Button %s pressed\n","MENUCMD_UP      "); break;
		case MENUCMD_DOWN    : debug_print("Button %s pressed\n","MENUCMD_DOWN    "); break;
		case MENUCMD_LEFT    : debug_print("Button %s pressed\n","MENUCMD_LEFT    "); break;
		case MENUCMD_RIGHT   : debug_print("Button %s pressed\n","MENUCMD_RIGHT   "); break;
		case MENUCMD_CROSS   : debug_print("Button %s pressed\n","MENUCMD_CROSS   "); break;
		case MENUCMD_TRIANGLE: debug_print("Button %s pressed\n","MENUCMD_TRIANGLE"); break;
		case MENUCMD_SQUARE  : debug_print("Button %s pressed\n","MENUCMD_SQUARE  "); break;
		case MENUCMD_CIRCLE  : debug_print("Button %s pressed\n","MENUCMD_CIRCLE  "); break;
		case MENUCMD_L1      : debug_print("Button %s pressed\n","MENUCMD_L1      "); break;
		case MENUCMD_R1      : debug_print("Button %s pressed\n","MENUCMD_R1      "); break;
		case MENUCMD_L2      : debug_print("Button %s pressed\n","MENUCMD_L2      "); break;
		case MENUCMD_R2      : debug_print("Button %s pressed\n","MENUCMD_R2      "); break;
	}
	return true;
}

MENUENTRYBUTTONFUNCTION(BackButton)
{
	menus->CloseMenu();
	return true;
}

MENUENTRYBUTTONFUNCTION(FreezeButton)
{
	superduperpausehack=true;
	app::inst()->get_game()->pause();
	menus->CloseAllMenus();
	app::inst()->get_game()->pause();

	return true;
}

MENUENTRYBUTTONFUNCTION(FRButton)
{
	nglSetDebugFlag("ShowPerfInfo", !nglGetDebugFlag("ShowPerfInfo"));
	return true;
}

extern char g_dump_profile_info;
MENUENTRYBUTTONFUNCTION(ProfButton)
{
  g_dump_profile_info = 1;
  return true;
}

/////////////////   Paricles submenu code start   //////////////////////

extern int particles_info;
extern float snap_life;
extern float snap_size;
extern float snap_collapse;
extern int ks_fx_test_stage;

MENUENTRYBUTTONFUNCTION(particles_test)
{
#if defined(BUILD_DEBUG) && defined(TARGET_PS2)
  ks_fx_test_stage = 0;
#endif
	return true;
}

MenuEntryFunction *entry_particles_test = NULL; //MenuEntryFunction ("Test", particles_test);
MenuEntryIntEdit *entry_particles_info = NULL; //MenuEntryIntEdit ("Show Info", &particles_info, 0, 1);
MenuEntryFloatEdit *entry_snap_size = NULL; //MenuEntryFloatEdit ("Snap size", &snap_size, 1, 10, 1.0f);
MenuEntryFloatEdit *entry_snap_life = NULL; //MenuEntryFloatEdit ("Snap life", &snap_life, 0, 10, 1.0f);
MenuEntryFloatEdit *entry_snap_collapse = NULL; //MenuEntryFloatEdit ("Snap collapse", &snap_collapse, 0, 10, 1.0f);

pMenuEntry particles_entries[5]; // = { &entry_particles_test, &entry_particles_info, &entry_snap_size, &entry_snap_life, &entry_snap_collapse };
Menu *menu_inner_particles = NULL; //Menu (NULL, sizeof (particles_entries) / sizeof (particles_entries[0]), particles_entries);
Submenu *menu_particles = NULL; //Submenu ("Particles Menu", &menu_inner_particles);


///////////////    End Particles debug menu code   ////////////////////////////

/////////////////   Camera debug submenu code  start   //////////////////////

MENUENTRYBUTTONFUNCTION(ReplayCamButton)
{
	switch ( buttonid )
	{
		case MENUCMD_CROSS:
			g_game_ptr->turn_user_cam_on(false);   //  Don't allow other cameras to change user cam.
			g_world_ptr->get_ks_controller(0)->SetPlayerCamera((game_camera *)find_camera(entity_id("REPLAY_CAM")));
			menus->CloseMenu();
			break;
	}
	return true;
}
MENUENTRYBUTTONFUNCTION(UserCamButton)
{
	switch ( buttonid )
	{
		case MENUCMD_CROSS:
			g_game_ptr->turn_user_cam_on(true);   //  Don't allow other cameras to change user cam.
			menus->CloseMenu();
			break;
	}
	return true;
}

MENUENTRYBUTTONFUNCTION(KSDebugCamButton)
{
	switch ( buttonid )
	{
		case MENUCMD_CROSS:
			g_game_ptr->turn_user_cam_on(false);   //  Don't allow other cameras to change user cam.
			g_world_ptr->get_ks_controller(0)->SetPlayerCamera((game_camera *)find_camera(entity_id("KSDEBUG_CAM0")));
			menus->CloseMenu();
			break;
	}
	return true;
}

MENUENTRYBUTTONFUNCTION(BeachCamButton)
{
	switch ( buttonid )
	{
		case MENUCMD_CROSS:
			g_game_ptr->turn_user_cam_on(false);   //  Don't allow other cameras to change user cam.
			g_world_ptr->get_ks_controller(0)->SetPlayerCamera((game_camera *)find_camera(entity_id("BEACH_CAM0")));
			menus->CloseMenu();
			break;
	}
	return true;
}

MENUENTRYBUTTONFUNCTION(FpsCamButton)
{
	switch ( buttonid )
	{
		case MENUCMD_CROSS:
			g_game_ptr->turn_user_cam_on(false);   //  Don't allow other cameras to change user cam.
			g_world_ptr->get_ks_controller(0)->SetPlayerCamera((game_camera *)find_camera(entity_id("FPS_CAM0")));
			menus->CloseMenu();
			break;
	}
	return true;
}
MENUENTRYBUTTONFUNCTION(AutoCamButton)
{
	switch ( buttonid )
	{
		case MENUCMD_CROSS:
			g_game_ptr->turn_user_cam_on(false);   //  Don't allow other cameras to change user cam.
			g_world_ptr->get_ks_controller(0)->SetPlayerCamera((game_camera *)find_camera(entity_id("AUTO_CAM0")));
			menus->CloseMenu();
			break;
	}
	return true;
}

MENUENTRYBUTTONFUNCTION(BigWaveCamButton)
{
	switch ( buttonid )
	{
		case MENUCMD_CROSS:
			g_game_ptr->turn_user_cam_on(false);   //  Don't allow other cameras to change user cam.
			g_world_ptr->get_ks_controller(0)->SetPlayerCamera((game_camera *)find_camera(entity_id("BIG_WAVE_CAM0")));
			menus->CloseMenu();
			break;
	}
	return true;
}

MENUENTRYBUTTONFUNCTION(OldShoulderCamButton)
{
	switch ( buttonid )
	{
		case MENUCMD_CROSS:
			g_game_ptr->turn_user_cam_on(false);   //  Don't allow other cameras to change user cam.
			g_world_ptr->get_ks_controller(0)->SetPlayerCamera((game_camera *)find_camera(entity_id("OLD_SHOULDER_CAM0")));
			menus->CloseMenu();
			break;
	}
	return true;
}

MENUENTRYBUTTONFUNCTION(ShoulderCamButton)
{
	switch ( buttonid )
	{
		case MENUCMD_CROSS:
			g_game_ptr->turn_user_cam_on(false);   //  Don't allow other cameras to change user cam.
			g_world_ptr->get_ks_controller(0)->SetPlayerCamera((game_camera *)find_camera(entity_id("SHOULDER_CAM0")));
			menus->CloseMenu();
			break;
	}
	return true;
}

MENUENTRYBUTTONFUNCTION(FollowCamButton)
{
	switch ( buttonid )
	{
		case MENUCMD_CROSS:
			g_game_ptr->turn_user_cam_on(false);   //  Don't allow other cameras to change user cam.
			g_world_ptr->get_ks_controller(0)->SetPlayerCamera((game_camera *)find_camera(entity_id("FOLLOW_CAM0")));
			menus->CloseMenu();
			break;
	}
	return true;
}

MENUENTRYBUTTONFUNCTION(FollowCloseCamButton)
{
	switch ( buttonid )
	{
		case MENUCMD_CROSS:
			g_game_ptr->turn_user_cam_on(false);   //  Don't allow other cameras to change user cam.
			g_world_ptr->get_ks_controller(0)->SetPlayerCamera((game_camera *)find_camera(entity_id("FOLLOW_CLOSE_CAM0")));
			menus->CloseMenu();
			break;
	}
	return true;
}

MENUENTRYBUTTONFUNCTION(BuoyCamButton)
{
	switch ( buttonid )
	{
		case MENUCMD_CROSS:
			g_game_ptr->turn_user_cam_on(false);   //  Don't allow other cameras to change user cam.
			g_world_ptr->get_ks_controller(0)->SetPlayerCamera((game_camera *)find_camera(entity_id("BUOY_CAM0")));
			menus->CloseMenu();
			break;
	}
	return true;
}

MenuEntryFunction *entry_user_cam = NULL; //MenuEntryFunction ("User Cam", UserCamButton);
MenuEntryFunction *entry_replay_cam = NULL; //MenuEntryFunction ("Replay Cam", ReplayCamButton);
MenuEntryFunction *entry_ksdebug_cam = NULL; //MenuEntryFunction ("KSDebug Cam", KSDebugCamButton);
MenuEntryFunction *entry_beach_cam = NULL; //MenuEntryFunction ("Beach Cam", BeachCamButton);
MenuEntryFunction *entry_fps_cam = NULL; //MenuEntryFunction ("Beach Cam", BeachCamButton);
MenuEntryFunction *entry_auto_cam = NULL; //MenuEntryFunction ("Auto Cam", AutoCamButton);
MenuEntryFunction *entry_big_wave_cam = NULL; //MenuEntryFunction ("Big Wave Cam", BigWaveCamButton);
MenuEntryFunction *entry_old_shoulder_cam = NULL; //MenuEntryFunction ("Old Shoulder Cam", OldShoulderCamButton);
MenuEntryFunction *entry_shoulder_cam = NULL; //MenuEntryFunction ("Shoulder Cam", ShoulderCamButton);
MenuEntryFunction *entry_follow_cam = NULL; //MenuEntryFunction ("Follow Cam", FollowCamButton);
MenuEntryFunction *entry_follow_close_cam = NULL; //MenuEntryFunction ("Follow Cam", FollowCloseCamButton);
MenuEntryFunction *entry_buoy_cam = NULL; //MenuEntryFunction ("Buoy Cam", BuoyCamButton);

float field_of_view_fudge = 1.0f;
extern float distance_mult;
float shoulder_cam_dy = 0.5f, shoulder_cam_dz = 4.5f, shoulder_cam_lag = 4.5f;
MenuEntryFloatEdit *entry_field_of_view = NULL; //MenuEntryFloatEdit ("Field of View", &field_of_view_fudge, 0.2f, 4.0f, 0.05f);
MenuEntryFloatEdit *entry_shoulder_cam_lag = NULL; //MenuEntryFloatEdit ("Shoulder lag", &shoulder_cam_lag, 0, 10, 0.1f);
MenuEntryFloatEdit *entry_shoulder_cam_dy = NULL; //MenuEntryFloatEdit ("Shoulder dy", &shoulder_cam_dy, 0, 20, 0.1f);
MenuEntryFloatEdit *entry_shoulder_cam_dz = NULL; //MenuEntryFloatEdit ("Shoulder dz", &shoulder_cam_dz, 0, 20, 0.1f);
MenuEntryFloatEdit *entry_shoulder_distance = NULL; //MenuEntryFloatEdit ("Shoulder dist", &distance_mult, 1, 10, 0.1f);

extern float gBeachCamZoom;
MenuEntryFloatEdit *entry_beachcam_zoom = NULL; //MenuEntryFloatEdit("Beach Cam Zoom", &gBeachCamZoom, 0.0f, 1.0f, 1.0f);

extern float g_freq_scale;
MenuEntryFloatEdit *entry_beachcam_freq = NULL;

extern float g_beachcam_dist_scale;
MenuEntryFloatEdit *entry_beachcam_dist = NULL;

pMenuEntry camsettings_entries[4];

Menu *menu_inner_camsettings = NULL; //Menu(NULL, sizeof(camsettings_entries)/sizeof(camsettings_entries[0]), camsettings_entries);
Submenu *menu_camsettings = NULL; //Submenu ("Camera Settings", &menu_inner_camsettings);

extern Submenu *menu_camtool;
pMenuEntry camera_entries[14]; // = {
/*	&entry_user_cam,
	&entry_beach_cam,
	&entry_auto_cam,
	&entry_big_wave_cam,
	&entry_ksdebug_cam,
	&entry_old_shoulder_cam,
	&entry_shoulder_cam,
	&entry_replay_cam,
  &entry_follow_cam,
	&menu_camsettings,
	&menu_camtool
};*/

Menu *menu_inner_cam = NULL; //Menu(NULL, sizeof(camera_entries)/sizeof(camera_entries[0]), camera_entries);
Submenu *menu_cam = NULL; //Submenu ("Camera Menu", &menu_inner_cam);







///////////////     End camera debug menu code   ////////////////////////////


/////////////////   Physics debug submenu code start   //////////////////////

extern float water_friction;
extern float skag_friction;
extern float world_gravity;
extern float stick_factor;

extern float g_max_dy;
extern float g_speed_dy;
extern float g_max_angle;
extern float g_speed_angle;
extern float g_go_over_back;
extern float g_special_meter_full;
extern float g_frame_by_frame;
extern float g_debug_mode_doik;
extern float g_intesity2_wip;

extern float SURFER_LIE_PULLBACK;
extern float SURFER_LIE_PADDLE_ACCELERATION;
extern int test_big_wave; 

int g_disable_wipeouts = 0;
int g_perfect_balance = 0;
int g_debug_balance = 0;
float stability_factor = 1.0f;
float speed_factor = 1.0f;
float carving_factor = 0.6f;
float trick_factor = 1.0f;

MENUENTRYBUTTONFUNCTION(physics_reset)
{
  // <<<< I don't totally understand this but I think this is correct:
  for(int i=0;i<g_game_ptr->get_num_players();i++)
    g_world_ptr->get_ks_controller(i)->ResetPhysics();

  return true;
}

MENUENTRYBUTTONFUNCTION(flyby_reset)
{
	g_debug.halt_on_asserts = false;  //  This is needed because it is trying to load files in the middle of a level.
    g_world_ptr->get_ks_controller(0)->StartFlyby();
	g_debug.halt_on_asserts = true;

	return true;
}

MenuEntryFunction *entry_physics_reset = NULL; //MenuEntryFunction ("Reset", physics_reset);
MenuEntryFunction *entry_flyby_reset = NULL; //MenuEntryFunction ("Flyby Reset", flyby_reset);

MenuEntryFloatEdit *entry_physics_lie_pullback = NULL; //MenuEntryFloatEdit ("Pullback While Lying", &SURFER_LIE_PULLBACK, 0, 10, 0.01f);
MenuEntryFloatEdit *entry_physics_lie_paddle_acc = NULL; //MenuEntryFloatEdit ("Paddle Acceleration", &SURFER_LIE_PADDLE_ACCELERATION, 0, 10, 0.01f);
MenuEntryIntEdit *entry_physics_test_big_wave = NULL; //MenuEntryIntEdit ("Test Big Wave", &SURFER_LIE_PADDLE_ACCELERATION, 0, 10, 0.01f);
MenuEntryFloatEdit *entry_physics_drag = NULL; //MenuEntryFloatEdit ("Friction", &water_friction, 0, 10, 0.01f);
MenuEntryFloatEdit *entry_physics_skag = NULL; //MenuEntryFloatEdit ("Skag", &skag_friction, 0, 50, 0.1f);
MenuEntryFloatEdit *entry_physics_gravity = NULL; //MenuEntryFloatEdit ("Gravity", &world_gravity, 0, 5.0f, 0.05f);
MenuEntryFloatEdit *entry_physics_stick = NULL; //MenuEntryFloatEdit ("Stick", &stick_factor, 0, 10, 0.1f);
MenuEntryFloatEdit *entry_physics_backofwave = NULL;

MenuEntryFloatEdit *entry_physics_stability = NULL; //MenuEntryFloatEdit ("Stability", &stability_factor, 0, 3, 0.01f);
MenuEntryFloatEdit *entry_physics_speed = NULL; //MenuEntryFloatEdit ("Speed", &speed_factor, 0, 5, 0.01f);
MenuEntryFloatEdit *entry_physics_carving = NULL; //MenuEntryFloatEdit ("Carving", &carving_factor, 0, 3, 0.01f);
MenuEntryFloatEdit *entry_physics_trick = NULL; //MenuEntryFloatEdit ("Trick", &trick_factor, 0, 3, 0.01f);
MenuEntryIntEdit *entry_physics_wipeout = NULL; //MenuEntryIntEdit ("No Wipeouts", &g_disable_wipeouts, 0, 1);
MenuEntryIntEdit *entry_physics_debug_balance = NULL; //MenuEntryIntEdit ("Debug Balance", &g_debug_balance, 0, 1);
MenuEntryIntEdit *entry_physics_balance = NULL; //MenuEntryIntEdit ("Perfect Balance", &g_perfect_balance, 0, 1);
MenuEntryFloatEdit *entry_special_meter_full = NULL;


MenuEntryFloatEdit *entry_floats_my = NULL; //MenuEntryFloatEdit ("Max dy", &g_max_dy, 0, 2, 0.01f);
MenuEntryFloatEdit *entry_floats_sy = NULL; //MenuEntryFloatEdit ("Speed dy", &g_speed_dy, 0, 5, 0.1f);
MenuEntryFloatEdit *entry_floats_ma = NULL; //MenuEntryFloatEdit ("Max Angle", &g_max_angle, 0, 45, 1.0f);
MenuEntryFloatEdit *entry_floats_sa = NULL; //MenuEntryFloatEdit ("Speed Angle", &g_speed_angle, 0, 5, 0.1f);

pMenuEntry floats_entries[4];
/*= { &entry_floats_my,
 &entry_floats_sy,
 &entry_floats_ma,
 &entry_floats_sa };*/

Menu *menu_inner_floats = NULL; //Menu(NULL, sizeof(floats_entries)/sizeof(floats_entries[0]), floats_entries);
Submenu *menu_floats = NULL; // Submenu ("Floating Objects", &menu_inner_floats);


pMenuEntry physics_entries[19];
/* = { &entry_physics_reset,
 &entry_physics_drag,
 &entry_physics_skag,
 &entry_physics_gravity,
 &entry_physics_stick,
 &entry_physics_backofwave,
 &entry_special_meter_full,
 &entry_physics_stability,
 &entry_physics_speed,
 &entry_physics_carving,
 &entry_physics_trick,
 &entry_physics_wipeout,
 &entry_physics_debug_balance,
 &entry_physics_balance,
 &menu_floats };*/
Menu *menu_inner_physics = NULL; //Menu(NULL, sizeof(physics_entries)/sizeof(physics_entries[0]), physics_entries);
Submenu *menu_physics = NULL; //Submenu ("Physics Menu", &menu_inner_physics);





///////////////    End Physics debug menu code   ////////////////////////////

/////////////////   RUMBLE debug submenu code  start   //////////////////////


//////////////////////// RUMBLE LEVELS ////////////////////////////////

  MenuEntryFloatEdit *rumble_level_landing     = NULL; //MenuEntryFloatEdit ("LANDING",
                                                      //          &rumbleMan.rumbleLevels[rumbleManager::LANDING], 0, 1, 0.05f);

  MenuEntryFloatEdit *rumble_level_in_air      = NULL; //MenuEntryFloatEdit ("IN_AIR",
                                                       //         &rumbleMan.rumbleLevels[rumbleManager::IN_AIR], 0, 1, 0.05f);
  MenuEntryFloatEdit *rumble_level_wiping_out  = NULL; //MenuEntryFloatEdit ("WIPING OUT",
                                                       //         &rumbleMan.rumbleLevels[rumbleManager::WIPING_OUT], 0, 1, 0.05f);
  MenuEntryFloatEdit *rumble_level_underwater  = NULL; //MenuEntryFloatEdit ("UNDERWATER",
                                                       //         &rumbleMan.rumbleLevels[rumbleManager::UNDERWATER], 0, 1, 0.05f);
  MenuEntryFloatEdit *rumble_level_grinding_object = NULL; //MenuEntryFloatEdit ("GRIDING OBJECTS",
                                                       //         &rumbleMan.rumbleLevels[rumbleManager::GRINDING_OBJECT], 0, 1, 0.05f);
  MenuEntryFloatEdit *rumble_level_floater     = NULL; //MenuEntryFloatEdit ("FLOATER",
                                                       //         &rumbleMan.rumbleLevels[rumbleManager::FLOATER], 0, 1, 0.05f);
  MenuEntryFloatEdit *rumble_level_in_wash     = NULL; //MenuEntryFloatEdit ("IN_WASH",
                                                       //         &rumbleMan.rumbleLevels[rumbleManager::IN_WASH], 0, 1, 0.05f);
  MenuEntryFloatEdit *rumble_level_pocket      = NULL; //MenuEntryFloatEdit ("LIE - IN THE POCKET",
                                                       //         &rumbleMan.rumbleLevels[rumbleManager::LIE_ON_BOARD_POCKET], 0, 1, 0.05f);
  MenuEntryFloatEdit *rumble_level_face        = NULL; //MenuEntryFloatEdit ("LIE - ON THE FACE",
                                                       //         &rumbleMan.rumbleLevels[rumbleManager::LIE_ON_BOARD_FACE], 0, 1, 0.05f);
  MenuEntryFloatEdit *rumble_level_chin        = NULL; //MenuEntryFloatEdit ("LIE - ON THE CHIN",
                                                       //         &rumbleMan.rumbleLevels[rumbleManager::LIE_ON_BOARD_CHIN], 0, 1, 0.05f);
  MenuEntryFloatEdit *rumble_level_tube        = NULL; //MenuEntryFloatEdit ("STANDING NEAR TUBE",
                                                       //         &rumbleMan.rumbleLevels[rumbleManager::STANDING_NEAR_TUBE], 0, 1, 0.05f);
  pMenuEntry rumble_level_entries[11];
	/* = { &rumble_level_landing,
	 &rumble_level_in_air,
	 &rumble_level_wiping_out,
	 &rumble_level_underwater,
	 &rumble_level_grinding_object,
	 &rumble_level_floater,
   &rumble_level_in_wash,
	 &rumble_level_pocket,
	 &rumble_level_face,
	 &rumble_level_chin,
	 &rumble_level_tube };*/

  Menu *menu_rumble_level = NULL; //Menu(NULL, sizeof(rumble_level_entries)/sizeof(rumble_level_entries[0]), rumble_level_entries);
  Submenu *sub_menu_rumble_level = NULL; //Submenu( "Rumble Intensities", &menu_rumble_level );

//////////////////////// RUMBLE FREQUENCY ////////////////////////////////


  MenuEntryFloatEdit *rumble_freq_landing     = NULL; //MenuEntryFloatEdit ("LANDING",
                                                      //          &rumbleMan.rumbleFreqs[rumbleManager::LANDING], 0, 2, 1);

  MenuEntryFloatEdit *rumble_freq_in_air      = NULL; //MenuEntryFloatEdit ("IN_AIR",
                                                      //          &rumbleMan.rumbleFreqs[rumbleManager::IN_AIR], 0, 2, 1);
  MenuEntryFloatEdit *rumble_freq_wiping_out  = NULL; //MenuEntryFloatEdit ("WIPING OUT",
                                                      //          &rumbleMan.rumbleFreqs[rumbleManager::WIPING_OUT], 0, 2, 1);
  MenuEntryFloatEdit *rumble_freq_underwater  = NULL; //MenuEntryFloatEdit ("UNDERWATER",
                                                      //          &rumbleMan.rumbleFreqs[rumbleManager::UNDERWATER], 0, 2, 1);
  MenuEntryFloatEdit *rumble_freq_grinding_object = NULL; //MenuEntryFloatEdit ("GRIDING OBJECTS",
                                                      //          &rumbleMan.rumbleFreqs[rumbleManager::GRINDING_OBJECT], 0, 2, 1);
  MenuEntryFloatEdit *rumble_freq_floater     = NULL; //MenuEntryFloatEdit ("FLOATER",
                                                      //          &rumbleMan.rumbleFreqs[rumbleManager::FLOATER], 0, 2, 1);
  MenuEntryFloatEdit *rumble_freq_in_wash     = NULL; //MenuEntryFloatEdit ("IN_WASH",
                                                      //          &rumbleMan.rumbleFreqs[rumbleManager::IN_WASH], 0, 2, 1);
  MenuEntryFloatEdit *rumble_freq_pocket      = NULL; //MenuEntryFloatEdit ("LIE - IN THE POCKET",
                                                      //          &rumbleMan.rumbleFreqs[rumbleManager::LIE_ON_BOARD_POCKET], 0, 2, 1);
  MenuEntryFloatEdit *rumble_freq_face        = NULL; //MenuEntryFloatEdit ("LIE - ON THE FACE",
                                                      //          &rumbleMan.rumbleFreqs[rumbleManager::LIE_ON_BOARD_FACE], 0, 2, 1);
  MenuEntryFloatEdit *rumble_freq_chin        = NULL; //MenuEntryFloatEdit ("LIE - ON THE CHIN",
                                                      //          &rumbleMan.rumbleFreqs[rumbleManager::LIE_ON_BOARD_CHIN], 0, 2, 1);
  MenuEntryFloatEdit *rumble_freq_tube        = NULL; //MenuEntryFloatEdit ("STANDING NEAR TUBE",
                                                      //          &rumbleMan.rumbleFreqs[rumbleManager::STANDING_NEAR_TUBE], 0, 2, 1);


  pMenuEntry rumble_freq_entries[11];
	/* = { &rumble_freq_landing,
	 &rumble_freq_in_air,
	 &rumble_freq_wiping_out,
	 &rumble_freq_underwater,
	 &rumble_freq_grinding_object,
	 &rumble_freq_floater,
   &rumble_freq_in_wash,
	 &rumble_freq_pocket,
	 &rumble_freq_face,
	 &rumble_freq_chin,
	 &rumble_freq_tube };*/

  Menu *menu_rumble_freq = NULL; //Menu(NULL, sizeof(rumble_freq_entries)/sizeof(rumble_freq_entries[0]), rumble_freq_entries);
  Submenu *sub_menu_rumble_freq = NULL; //Submenu( "Rumble Frequency (0 - low 1 - high 2 - both)", &menu_rumble_freq );

//////////////////////// RUMBLE VARIANCE ////////////////////////////////


  MenuEntryFloatEdit *rumble_amp_landing     = NULL; //MenuEntryFloatEdit ("LANDING",
                                                     //           &rumbleMan.rumbleVarAmplitudes[rumbleManager::LANDING], 0, 1, .05);

  MenuEntryFloatEdit *rumble_amp_in_air      = NULL; //MenuEntryFloatEdit ("IN_AIR",
                                                     //           &rumbleMan.rumbleVarAmplitudes[rumbleManager::IN_AIR], 0, 1, .05);
  MenuEntryFloatEdit *rumble_amp_wiping_out  = NULL; //MenuEntryFloatEdit ("WIPING OUT",
                                                     //           &rumbleMan.rumbleVarAmplitudes[rumbleManager::WIPING_OUT], 0, 1, .05);
  MenuEntryFloatEdit *rumble_amp_underwater  = NULL; //MenuEntryFloatEdit ("UNDERWATER",
                                                     //           &rumbleMan.rumbleVarAmplitudes[rumbleManager::UNDERWATER], 0, 1, .05);
  MenuEntryFloatEdit *rumble_amp_grinding_object = NULL; //MenuEntryFloatEdit ("GRIDING OBJECTS",
                                                     //           &rumbleMan.rumbleVarAmplitudes[rumbleManager::GRINDING_OBJECT], 0, 1, .05);
  MenuEntryFloatEdit *rumble_amp_floater     = NULL; //MenuEntryFloatEdit ("FLOATER",
                                                     //           &rumbleMan.rumbleVarAmplitudes[rumbleManager::FLOATER], 0, 1, .05);
  MenuEntryFloatEdit *rumble_amp_in_wash     = NULL; //MenuEntryFloatEdit ("IN_WASH",
                                                     //           &rumbleMan.rumbleVarAmplitudes[rumbleManager::IN_WASH], 0, 1, .05);
  MenuEntryFloatEdit *rumble_amp_pocket      = NULL; //MenuEntryFloatEdit ("LIE - IN THE POCKET",
                                                     //           &rumbleMan.rumbleVarAmplitudes[rumbleManager::LIE_ON_BOARD_POCKET], 0, 1, .05);
  MenuEntryFloatEdit *rumble_amp_face        = NULL; //MenuEntryFloatEdit ("LIE - ON THE FACE",
                                                     //           &rumbleMan.rumbleVarAmplitudes[rumbleManager::LIE_ON_BOARD_FACE], 0, 1, .05);
  MenuEntryFloatEdit *rumble_amp_chin        = NULL; //MenuEntryFloatEdit ("LIE - ON THE CHIN",
                                                     //           &rumbleMan.rumbleVarAmplitudes[rumbleManager::LIE_ON_BOARD_CHIN], 0, 1, .05);
  MenuEntryFloatEdit *rumble_amp_tube        = NULL; //MenuEntryFloatEdit ("STANDING NEAR TUBE",
                                                     //           &rumbleMan.rumbleVarAmplitudes[rumbleManager::STANDING_NEAR_TUBE], 0, 1, .05);


  pMenuEntry rumble_amp_entries[11];
	/* = { &rumble_amp_landing,
	 &rumble_amp_in_air,
	 &rumble_amp_wiping_out,
	 &rumble_amp_underwater,
	 &rumble_amp_grinding_object,
	 &rumble_amp_floater,
   &rumble_amp_in_wash,
	 &rumble_amp_pocket,
	 &rumble_amp_face,
	 &rumble_amp_chin,
	 &rumble_amp_tube };*/

  Menu *menu_rumble_amp = NULL; //Menu(NULL, sizeof(rumble_amp_entries)/sizeof(rumble_amp_entries[0]), rumble_amp_entries);
  Submenu *sub_menu_rumble_amp = NULL; //Submenu( "Rumble Variance", &menu_rumble_amp );


//////////////////////// RUMBLE PERIOD ////////////////////////////////

  MenuEntryFloatEdit *rumble_per_landing     = NULL; //MenuEntryFloatEdit ("LANDING",
                                                     //           &rumbleMan.rumbleVarPeriods[rumbleManager::LANDING], 0, 5, .1);

  MenuEntryFloatEdit *rumble_per_in_air      = NULL; //MenuEntryFloatEdit ("IN_AIR",
                                                     //           &rumbleMan.rumbleVarPeriods[rumbleManager::IN_AIR], 0, 5, .1);
  MenuEntryFloatEdit *rumble_per_wiping_out  = NULL; //MenuEntryFloatEdit ("WIPING OUT",
                                                     //           &rumbleMan.rumbleVarPeriods[rumbleManager::WIPING_OUT], 0, 5, .1);
  MenuEntryFloatEdit *rumble_per_underwater  = NULL; //MenuEntryFloatEdit ("UNDERWATER",
                                                     //           &rumbleMan.rumbleVarPeriods[rumbleManager::UNDERWATER], 0, 5, .1);
  MenuEntryFloatEdit *rumble_per_grinding_object = NULL; //MenuEntryFloatEdit ("GRIDING OBJECTS",
                                                     //           &rumbleMan.rumbleVarPeriods[rumbleManager::GRINDING_OBJECT], 0, 5, .1);
  MenuEntryFloatEdit *rumble_per_floater     = NULL; //MenuEntryFloatEdit ("FLOATER",
                                                     //           &rumbleMan.rumbleVarPeriods[rumbleManager::FLOATER], 0, 5, .1);
  MenuEntryFloatEdit *rumble_per_in_wash     = NULL; //MenuEntryFloatEdit ("IN_WASH",
                                                     //           &rumbleMan.rumbleVarPeriods[rumbleManager::IN_WASH], 0, 5, .1);
  MenuEntryFloatEdit *rumble_per_pocket      = NULL; //MenuEntryFloatEdit ("LIE - IN THE POCKET",
                                                     //           &rumbleMan.rumbleVarPeriods[rumbleManager::LIE_ON_BOARD_POCKET], 0, 5, .1);
  MenuEntryFloatEdit *rumble_per_face        = NULL; //MenuEntryFloatEdit ("LIE - ON THE FACE",
                                                     //           &rumbleMan.rumbleVarPeriods[rumbleManager::LIE_ON_BOARD_FACE], 0, 5, .1);
  MenuEntryFloatEdit *rumble_per_chin        = NULL; //MenuEntryFloatEdit ("LIE - ON THE CHIN",
                                                     //           &rumbleMan.rumbleVarPeriods[rumbleManager::LIE_ON_BOARD_CHIN], 0, 5, .1);
  MenuEntryFloatEdit *rumble_per_tube        = NULL; //MenuEntryFloatEdit ("STANDING NEAR TUBE",
                                                     //           &rumbleMan.rumbleVarPeriods[rumbleManager::STANDING_NEAR_TUBE], 0, 5, .1);


  pMenuEntry rumble_per_entries[11];
	/* = { &rumble_per_landing,
	 &rumble_per_in_air,
	 &rumble_per_wiping_out,
	 &rumble_per_underwater,
	 &rumble_per_grinding_object,
	 &rumble_per_floater,
   &rumble_per_in_wash,
	 &rumble_per_pocket,
	 &rumble_per_face,
	 &rumble_per_chin,
	 &rumble_per_tube }; */

  Menu *menu_rumble_per = NULL; //Menu(NULL, sizeof(rumble_per_entries)/sizeof(rumble_per_entries[0]), rumble_per_entries);
  Submenu *sub_menu_rumble_per = NULL; //Submenu( "Rumble Period", &menu_rumble_per );


//////////////////////// RUMBLE MAIN ////////////////////////////////
MENUENTRYBUTTONFUNCTION(WriteRumbleButton)
{
  if (buttonid==MENUCMD_CROSS)
    rumbleMan.writeLevels();
  return true;
}
MENUENTRYBUTTONFUNCTION(ToggleShowRumble)
{
  if (buttonid==MENUCMD_CROSS)
    rumbleMan.toggleDrawState();
  return true;
}
  MenuEntryFunction *write_rumble    = NULL; //MenuEntryFunction( "Save Rumble Settings     ",   WriteRumbleButton   );
  MenuEntryFunction *show_rumble    = NULL; //MenuEntryFunction( "Show Current Rumble State",   ToggleShowRumble   );
  pMenuEntry rumble_parent[6]; // = { &sub_menu_rumble_level, &sub_menu_rumble_freq, &sub_menu_rumble_amp, &sub_menu_rumble_per, &write_rumble, &show_rumble };
  Menu *menu_rumble_parent = NULL; //Menu(NULL, sizeof(rumble_parent)/sizeof(rumble_parent[0]), rumble_parent);
  Submenu *menu_rumble = NULL; //Submenu ("Rumble Menu", &menu_rumble_parent);


/////////////////   RUMBLE debug submenu code  end   //////////////////////


/////////////////   Replay debug submenu code  start   //////////////////////

MENUENTRYBUTTONFUNCTION(ReplayPlayButton)
{
	if ( buttonid==MENUCMD_CROSS )
		ksreplay.Play();
	return true;
}

MENUENTRYBUTTONFUNCTION(ReplayFwdButton)
{
	//if ( buttonid==MENUCMD_CROSS )
	//	ksreplay.StepAdvance();
	return true;
}

MENUENTRYBUTTONFUNCTION(ReplayRewButton)
{
	//if ( buttonid==MENUCMD_CROSS )
	//	ksreplay.StepRewind();
	return true;
}

MENUENTRYBUTTONFUNCTION(ReplaySlowButton)
{
	if ( buttonid==MENUCMD_CROSS )
		ksreplay.SpeedSlow();
	return true;
}

MENUENTRYBUTTONFUNCTION(ReplayResetButton)
{
	if ( buttonid==MENUCMD_CROSS )
	{
		ksreplay.Clear(g_random_r_ptr->srand());
		ksreplay.Pause(false);
	 	menus->CloseMenu();
	}
	return true;
}

MENUENTRYBUTTONFUNCTION(ReplayCancelButton)
{
	if ( buttonid==MENUCMD_CROSS )
	{
		ksreplay.Pause(false);
	 	menus->CloseMenu();
	}
	return true;
}

MENUENTRYBUTTONFUNCTION(ReplayRestartButton)
{
	if ( buttonid==MENUCMD_CROSS )
		ksreplay.Restart();
	return true;
}

MENUENTRYBUTTONFUNCTION(ReplaySaveButton)
{
	if ( buttonid==MENUCMD_CROSS )
	{
		ksreplay.SaveFile(NULL);
	 	menus->CloseMenu();
	}
	return true;
}

MENUENTRYBUTTONFUNCTION(ReplayLoadButton)
{
	if ( buttonid==MENUCMD_CROSS )
	{
		ksreplay.LoadFile("Replay0.rpl");
	 	menus->CloseMenu();
	}
	return true;
}

MenuEntryFunction *replay_play        = NULL; //MenuEntryFunction( "Play     ",   ReplayPlayButton   );
MenuEntryFunction *replay_slow        = NULL; //MenuEntryFunction( "Slow Mo  ",   ReplaySlowButton    );
MenuEntryFunction *replay_fwd         = NULL; //MenuEntryFunction( "Frame Fwd",   ReplayFwdButton    );
MenuEntryFunction *replay_rew         = NULL; //MenuEntryFunction( "Frame Rew",   ReplayRewButton    );
MenuEntryFunction *replay_restart     = NULL; //MenuEntryFunction( "Restart  ",   ReplayRestartButton    );
MenuEntryFunction *replay_reset       = NULL; //MenuEntryFunction( "Reset    ",   ReplayResetButton    );
MenuEntryFunction *replay_save        = NULL; //MenuEntryFunction( "Save    ",    ReplaySaveButton    );
MenuEntryFunction *replay_load        = NULL; //MenuEntryFunction( "Load    ",    ReplayLoadButton    );
MenuEntryFunction *replay_cancel      = NULL; //MenuEntryFunction( "Cancel   ",   ReplayCancelButton );

MenuEntryFunction *entry_ksdebug_cam_replay = NULL; //MenuEntryFunction *("KSDebug Cam", KSDebugCamButton);
MenuEntryFunction *entry_beach_cam_replay = NULL; //MenuEntryFunction ("Beach Cam", BeachCamButton);
MenuEntryFunction *entry_auto_cam_replay = NULL; //MenuEntryFunction ("Auto Cam", AutoCamButton);
MenuEntryFunction *entry_big_wave_cam_replay = NULL; //MenuEntryFunction ("Big Wave Cam", BigWaveCamButton);
MenuEntryFunction *entry_old_shoulder_cam_replay = NULL; //MenuEntryFunction ("Old Shoulder Cam", OldShoulderCamButton);
MenuEntryFunction *entry_follow_cam_replay = NULL; //MenuEntryFunction ("Follow Cam", FollowCamButton);
MenuEntryFunction *entry_follow_close_cam_replay = NULL; //MenuEntryFunction ("Follow Close Cam", FollowCloseCamButton);
MenuEntryFunction *entry_buoy_cam_replay = NULL; //MenuEntryFunction ("Buoy Cam", BuoyCamButton);

pMenuEntry camera_entries_replay[8];
/* = { &entry_beach_cam_replay,
 &entry_auto_cam_replay,
 &entry_big_wave_cam_replay,
 &entry_ksdebug_cam_replay,
 &entry_old_shoulder_cam_replay,
 &entry_follow_cam_replay };*/

Menu *menu_inner_cam_replay = NULL; //Menu(NULL, sizeof(camera_entries_replay)/sizeof(camera_entries_replay[0]), camera_entries_replay);
Submenu *menu_cam_replay = NULL; //Submenu ("Camera", &menu_inner_cam_replay);

pMenuEntry replay_entries[9]; // = { &replay_play, &replay_slow, &replay_fwd, &replay_restart, &replay_reset, &replay_cancel, &replay_save, &replay_load, &menu_cam_replay }; //, &replay_rew };
Menu *menu_inner_replay = NULL; //Menu(NULL, 8, replay_entries);
Submenu *menu_replay = NULL; //Submenu ("Replay Menu", &menu_inner_replay);


///////////////     End replay debug menu code   ////////////////////////////






//-------------------------------------
// Samples menu entries

MenuEntryFunction *menu_null = NULL; //MenuEntryFunction( "No Effect",   NullButton  );
MenuEntryFunction *menu_show = NULL; //MenuEntryFunction( "Show Button", DebugButton );
MenuEntryFunction *menu_back = NULL; //MenuEntryFunction( "Prev Menu",   BackButton  );
MenuEntryFunction *menu_freeze = NULL; //MenuEntryFunction( "Menus off",   FreezeButton  );
MenuEntryFunction *menu_fr   = NULL; //MenuEntryFunction( "Show FPS",   FRButton  );
MenuEntryFunction *menu_prof = NULL; //MenuEntryFunction( "Dump Profile Info", ProfButton  );


//MenuEntryFunction *menu_camera = NULL; //MenuEntryFunction( "Camera",   CameraButton  );

pMenuEntry menu_inner_entries[4]; //={&menu_null, &menu_show, &menu_back, &menu_freeze };

Menu *menu_inner = NULL; //Menu(NULL,4,menu_inner_entries);

Submenu *menu_sub = NULL; //Submenu( "Inner Menu", &menu_inner );
int edited=12;
MenuEntryIntEdit *menu_int = NULL; //MenuEntryIntEdit( "Int Value", &edited, 5, 20 );
int xedited=5;
MenuEntryIntEdit *menu_hex = NULL; //MenuEntryIntEdit( "Hex Value", &xedited, 0, 255, " : 0x%X" );
float fedited=0.5f;
MenuEntryFloatEdit *menu_float = NULL; //MenuEntryFloatEdit( "Float Value", &fedited, -10, 20, 0.1f );

char *testlist[4]=
{
	"Item 1",
	"Item 2",
	"Item 3",
	"Item 4"
};
int ledited=2;
MenuEntryListEdit *menu_testlist = NULL; //MenuEntryListEdit( "List", &ledited, 4, testlist);

char *testlisttext[4]=
{
	"Item 1 Val 5",
	"Item 2 Val 1",
	"Item 3 Val 0",
	"Item 4 Val 9",
};
int testlistvals[4]=
{
	5,1,0,9
};
int eedited=5;
MenuEntryEnumEdit *menu_enum = NULL; //MenuEntryEnumEdit( "Enum", &eedited, 4, testlisttext, testlistvals );


/////////////////////    End Debug Labels menu code  ///////////////////////////////////////////

//-------------------------------------
// Samples menu

MenuEntryTitle *sample_title=NULL; //MenuEntryTitle("Sample menu");

pMenuEntry menu_sample_entries[11]; //={&sample_title, &menu_null, &menu_sub, &menu_show, &menu_int, &menu_hex, &menu_float, &menu_testlist, &menu_enum, &menu_back, &menu_freeze };

Menu *menu_sample = NULL; //Menu(NULL,11,menu_sample_entries);

Submenu *menu_samples = NULL; //Submenu( "Sample menus", &menu_sample );


//////////////////////////////   Start Animation Menu   //////////////////////////////////

char *anim_mode_list[2]=
{
	"Debug Mode Off",
	"Debug Mode On"
};

extern int debug_mode;
MenuEntryListEdit *menu_animmode_list = NULL; //MenuEntryListEdit( "Mode", &debug_mode, 2, anim_mode_list);

extern stringx g_surfer_anims[_SURFER_NUM_ANIMS];

//////////////////////   Start Debug Labels menu code  ////////////////////////////////////

MenuEntryTitle *label_title=NULL; //MenuEntryTitle("Labels menu");

int show_state_label=0;
int show_anim_label=0;
MenuEntryIntEdit *state_menu = NULL; //MenuEntryIntEdit( "State Label", &show_state_label, 0, 1 );
MenuEntryIntEdit *anim_menu = NULL; //MenuEntryIntEdit( "Anim Label", &show_anim_label, 0, 1 );
MenuEntryIntEdit *wipeout_blur = NULL; //MenuEntryIntEdit ("Wipeout Blur", &g_wipeout_blur, 0, 1);

pMenuEntry menu_labels_entries[4]; //={&label_title, &state_menu, &anim_menu, &wipeout_blur};

Menu *menu_labels = NULL; //Menu(NULL, 3, menu_labels_entries);

Submenu *menu_label = NULL; //Submenu("Labels Menu", &menu_labels);

extern int anim_num;
MenuEntryStringxListEdit *menu_animnum_list = NULL; //MenuEntryListEdit( "Anim", &anim_num, (int) _SURFER_NUM_ANIMS, debug_surfer_anims);

MenuEntryFloatEdit *menu_frame_by_frame_list = NULL;
MenuEntryTitle *anim_title = NULL; //MenuEntryTitle("Animation Menu");
MenuEntryFloatEdit *menu_debug_do_ik_list = NULL;
MenuEntryFloatEdit *wip_intensity_level = NULL;

pMenuEntry menu_anim_entries[7]; //={&anim_title, &menu_animmode_list, &menu_animnum_list, &menu_label };

Menu *anim_menu2 = NULL; //Menu(NULL,4,menu_anim_entries);

Submenu *menu_anim = NULL; //Submenu( "Animation Menu", &anim_menu2 );


/////////////////////////////   End Animation Menu     ///////////////////////////////////////////

///////////////////////////////Sound Menu///////////////////////////////////

nslSoundId menuSources[15];
nslSoundId menuSounds[15];
MENUENTRYBUTTONFUNCTION(Face1)
{
  if (nslGetSoundStatus(menuSounds[0]) != NSL_SOUNDSTATUS_INVALID)
  {
    nslStopSound(menuSounds[0]);
  }
  menuSources[0] = nslGetSource("WAVE_FACE01", false);

	menuSounds[0] = nslAddSound(menuSources[0] );
  nslPlaySound(menuSounds[0]);
  nslSetSoundParam(menuSounds[0], NSL_SOUNDPARAM_VOLUME, .5);
  nslFrameAdvance(.1);
	return true;
}

MENUENTRYBUTTONFUNCTION(Face2)
{
  if (nslGetSoundStatus(menuSounds[1]) != NSL_SOUNDSTATUS_INVALID)
  {
    nslStopSound(menuSounds[1]);
  }
  menuSources[1] = nslGetSource("WAVE_FACE01", false);

	menuSounds[1] = nslAddSound(menuSources[1] );
  nslPlaySound(menuSounds[1]);
  nslSetSoundParam(menuSounds[1], NSL_SOUNDPARAM_PITCH, .8);
  nslSetSoundParam(menuSounds[1], NSL_SOUNDPARAM_VOLUME, .5);
  nslFrameAdvance(.1);
	return true;
}

MENUENTRYBUTTONFUNCTION(Crash)
{
  if (nslGetSoundStatus(menuSounds[2]) != NSL_SOUNDSTATUS_INVALID)
  {
    nslStopSound(menuSounds[2]);
  }
  menuSources[2]   = nslGetSource("WAVE_WAVECRASH01", false);

	menuSounds[2] = nslAddSound(menuSources[2] );
  nslPlaySound(menuSounds[2]);
  nslFrameAdvance(.1);
	return true;
}
MENUENTRYBUTTONFUNCTION(Foam)
{
  if (nslGetSoundStatus(menuSounds[3]) != NSL_SOUNDSTATUS_INVALID)
  {
    nslStopSound(menuSounds[3]);
  }
  menuSources[3]  = nslGetSource("WAVE_EDGE01", false);

	menuSounds[3] = nslAddSound(menuSources[3] );
  nslPlaySound(menuSounds[3]);
  nslFrameAdvance(.1);
	return true;
}


MENUENTRYBUTTONFUNCTION(Tube1)
{
  if (nslGetSoundStatus(menuSounds[4]) != NSL_SOUNDSTATUS_INVALID)
  {
    nslStopSound(menuSounds[4]);
  }
  menuSources[4]  = nslGetSource("WAVE_TUBE02", false);

	menuSounds[4] = nslAddSound(menuSources[4] );
  nslPlaySound(menuSounds[4]);
  nslFrameAdvance(.1);
	return true;
}


MENUENTRYBUTTONFUNCTION(Tube2)
{
  if (nslGetSoundStatus(menuSounds[5]) != NSL_SOUNDSTATUS_INVALID)
  {
    nslStopSound(menuSounds[5]);
  }
  menuSources[5]  = nslGetSource("WAVE_TUBE02", false);

  menuSounds[5] = nslAddSound(menuSources[5] );
  nslPlaySound(menuSounds[5]);
  nslSetSoundParam(menuSounds[5], NSL_SOUNDPARAM_PITCH, .8);
  nslFrameAdvance(.1);
	return true;
}



MENUENTRYBUTTONFUNCTION(Splash)
{
  if (nslGetSoundStatus(menuSounds[6]) != NSL_SOUNDSTATUS_INVALID)
  {
    nslStopSound(menuSounds[6]);
  }
  menuSources[6] = nslGetSource("BAIL_UNDERWATER01A", false);

	menuSounds[6] = nslAddSound(menuSources[6] );
  nslPlaySound(menuSounds[6]);
  nslFrameAdvance(.1);
	return true;
}

MENUENTRYBUTTONFUNCTION(Under)
{
  if (nslGetSoundStatus(menuSounds[7]) != NSL_SOUNDSTATUS_INVALID)
  {
    nslStopSound(menuSounds[7]);
  }
  menuSources[7]  = nslGetSource("WAVE_UNDERWATER01", false);

	menuSounds[7] = nslAddSound(menuSources[7] );
  nslPlaySound(menuSounds[7]);
  nslFrameAdvance(.1);
	return true;
}


MENUENTRYBUTTONFUNCTION(Gull1)
{
  if (nslGetSoundStatus(menuSounds[8]) != NSL_SOUNDSTATUS_INVALID)
  {
    nslStopSound(menuSounds[8]);
  }
  menuSources[8]  = nslGetSource("SEAGULL1", false);

	menuSounds[8] = nslAddSound(menuSources[8] );
  nslPlaySound(menuSounds[8]);
  nslFrameAdvance(.1);
	return true;
}
MENUENTRYBUTTONFUNCTION(Gull2)
{
  if (nslGetSoundStatus(menuSounds[9]) != NSL_SOUNDSTATUS_INVALID)
  {
    nslStopSound(menuSounds[9]);
  }
  menuSources[9] = nslGetSource("SEAGULL2", false);

	menuSounds[9] = nslAddSound(menuSources[9] );
  nslPlaySound(menuSounds[9]);
  nslFrameAdvance(.1);
	return true;
}
MENUENTRYBUTTONFUNCTION(Gull3)
{
  if (nslGetSoundStatus(menuSounds[10]) != NSL_SOUNDSTATUS_INVALID)
  {
    nslStopSound(menuSounds[10]);
  }
  menuSources[10]  = nslGetSource("SEAGULL3", false);

	menuSounds[10] = nslAddSound(menuSources[10] );
  nslPlaySound(menuSounds[10]);
  nslFrameAdvance(.1);
	return true;
}
MENUENTRYBUTTONFUNCTION(Buoy1)
{
  if (nslGetSoundStatus(menuSounds[11]) != NSL_SOUNDSTATUS_INVALID)
  {
    nslStopSound(menuSounds[11]);
  }
  menuSources[11] = nslGetSource("BUOY1", false);

	menuSounds[11] = nslAddSound(menuSources[11] );
  nslPlaySound(menuSounds[11]);
  nslFrameAdvance(.1);
	return true;
}
MENUENTRYBUTTONFUNCTION(Buoy2)
{
  if (nslGetSoundStatus(menuSounds[12]) != NSL_SOUNDSTATUS_INVALID)
  {
    nslStopSound(menuSounds[12]);
  }
  menuSources[12]   = nslGetSource("BUOY2", false);

	menuSounds[12] = nslAddSound(menuSources[12] );
  nslPlaySound(menuSounds[12]);
  nslFrameAdvance(.1);
	return true;
}


MENUENTRYBUTTONFUNCTION(Lion1)
{
  if (nslGetSoundStatus(menuSounds[13]) != NSL_SOUNDSTATUS_INVALID)
  {
    nslStopSound(menuSounds[13]);
  }
  menuSources[13]  = nslGetSource("SEA_LION", false);

	menuSounds[13] = nslAddSound(menuSources[13] );
  nslPlaySound(menuSounds[13]);
  nslFrameAdvance(.1);
	return true;
}

MENUENTRYBUTTONFUNCTION(Lion2)
{
  if (nslGetSoundStatus(menuSounds[14]) != NSL_SOUNDSTATUS_INVALID)
  {
    nslStopSound(menuSounds[14]);
  }
  menuSources[14]  = nslGetSource("SEA_LION2", false);

	menuSounds[14] = nslAddSound(menuSources[14] );
  nslPlaySound(menuSounds[14]);
  nslFrameAdvance(.1);
	return true;
}


MENUENTRYBUTTONFUNCTION(Stop)
{
  for (int i=0; i < 15; i++)
    if (nslGetSoundStatus(menuSounds[i]) != NSL_SOUNDSTATUS_INVALID)
      nslStopSound(menuSounds[i]);

  return true;
}



MenuEntryFunction *soundStop     = NULL; //MenuEntryFunction("Stop All Sounds",           Stop   );
MenuEntryFunction *soundGull1    = NULL; //MenuEntryFunction("Play Sea Gull 1",           Gull1  );
MenuEntryFunction *soundGull2    = NULL; //MenuEntryFunction("Play Sea Gull 2",           Gull2  );
MenuEntryFunction *soundGull3    = NULL; //MenuEntryFunction("Play Sea Gull 3",           Gull3  );

MenuEntryFunction *soundBuoy1    = NULL; //MenuEntryFunction("Play Buoy 1",               Buoy1  );
MenuEntryFunction *soundBuoy2    = NULL; //MenuEntryFunction("Play Buoy 2",               Buoy2  );

MenuEntryFunction *soundLion1    = NULL; //MenuEntryFunction("Play Sea Lion 1",           Lion1  );
MenuEntryFunction *soundLion2    = NULL; //MenuEntryFunction("Play Sea Lion 2",           Lion2  );


MenuEntryFunction *soundFace1    = NULL; //MenuEntryFunction("Play Face Sound 1",         Face1  );
MenuEntryFunction *soundFace2    = NULL; //MenuEntryFunction("Play Face Sound 2",         Face2  );
MenuEntryFunction *soundTube1    = NULL; //MenuEntryFunction("Play Tube Sound 1",         Tube1  );
MenuEntryFunction *soundTube2    = NULL; //MenuEntryFunction("Play Tube Sound 2",         Tube2  );
MenuEntryFunction *soundSplash   = NULL; //MenuEntryFunction("Play Splash Sound",         Splash );
MenuEntryFunction *soundUnder    = NULL; //MenuEntryFunction("Play Underwater Sound",     Under  );
MenuEntryFunction *soundFoam     = NULL; //MenuEntryFunction("Play Foam Sound",           Foam   );
MenuEntryFunction *soundCrash    = NULL; //MenuEntryFunction("Play Crash Sound",          Crash   );


//pMenuEntry pmenu_sound_sounds[16]; //={&soundStop, &soundGull1, &soundGull2, &soundGull3, &soundBuoy1, &soundBuoy2, &soundLion1,&soundLion2, &soundFace1, &soundFace2, &soundTube1, &soundTube2, &soundSplash, &soundUnder,&soundFoam, &soundCrash};
/*
Menu *sound_sounds_menu          = NULL; //Menu(NULL,16, pmenu_sound_sounds);
Submenu *menu_sound_sounds       = NULL; //Submenu( "Sound Samples Menu", &sound_sounds_menu );
*/



MENUENTRYBUTTONFUNCTION(MuteSound)
{
	if ( buttonid==MENUCMD_CROSS )
		nslSetMasterVolume(0.0);
	return true;
}

MENUENTRYBUTTONFUNCTION(UnmuteSound)
{
	if ( buttonid==MENUCMD_CROSS )
		nslSetMasterVolume(1.0);
	return true;
}

MENUENTRYBUTTONFUNCTION(CamPerspective)
{
	if ( buttonid==MENUCMD_CROSS )
  {
      wSound.perspective = wSound.CAMERA_PERSPECTIVE;
  }
	return true;
}

MENUENTRYBUTTONFUNCTION(SurferPerspective)
{
	if ( buttonid==MENUCMD_CROSS )
  {
      wSound.perspective = wSound.SURFER_PERSPECTIVE;
  }
	return true;
}

MENUENTRYBUTTONFUNCTION(Spheres)
{
	if ( buttonid==MENUCMD_CROSS )
  {
    if (wSound.showSpheres == true)
    {
      wSound.showSpheres = false;
    }
    else
    {
      wSound.showSpheres = true;
    }

  }
	return true;
}

extern WaveSound wSound;
/*
MenuEntryFunction *soundMute     = NULL; //MenuEntryFunction("Mute Sound",      MuteSound  );
MenuEntryFunction *soundUnmute   = NULL; //MenuEntryFunction( "Unmute Sound",   UnmuteSound  );
MenuEntryFunction *toggleSpheres = NULL; //MenuEntryFunction( "Toggle Sound Extents", Spheres  );
MenuEntryFunction *camPersp      = NULL; //MenuEntryFunction( "Set Camera Persp.", CamPerspective  );
MenuEntryFunction *surferPersp   = NULL; //MenuEntryFunction( "Set Surfer Persp.", SurferPerspective  );

MenuEntryFloatEdit *fcVol   = NULL; //MenuEntryFloatEdit( "Face Raw Volume",         &wSound.faceVolume,   0.0f, 1.0f, .05f );
MenuEntryFloatEdit *tbVol   = NULL; //MenuEntryFloatEdit( "Tube Raw Volume",         &wSound.tubeVolume,   0.0f, 1.0f, .05f );
MenuEntryFloatEdit *foVol   = NULL; //MenuEntryFloatEdit( "Foam Raw Volume",         &wSound.foamVolume,   0.0f, 1.0f, .05f );

MenuEntryFloatEdit *ffmin   = NULL; //MenuEntryFloatEdit( "Foam Min",   &wSound.foamMin,  0.0f, 100.0f, .5);
MenuEntryFloatEdit *fbmin   = NULL; //MenuEntryFloatEdit( "Foam Min",   &wSound.foamMax,  0.0f, 100.0f, .5);

MenuEntryFloatEdit *ffmax   = NULL; //MenuEntryFloatEdit( "Face Max",   &wSound.faceMin,  0.0f, 100.0f, .5);
MenuEntryFloatEdit *fbmax   = NULL; //MenuEntryFloatEdit( "Face Max",   &wSound.faceMax,  0.0f, 100.0f, .5);

MenuEntryFloatEdit *tmin    = NULL; //MenuEntryFloatEdit( "Tube Min",     &wSound.tubeMax,      0.0f, 100.0f, .5);
MenuEntryFloatEdit *tmax    = NULL; //MenuEntryFloatEdit( "Tube Max",     &wSound.tubeMax,      0.0f, 100.0f, .5);


pMenuEntry menu_sound_entries[15]; //={&surferPersp, &camPersp,  &soundMute, &soundUnmute, &fcVol, &tbVol, &foVol, &ffmin, &fbmin, &ffmax, &fbmax, &tmin, &tmax, &toggleSpheres, &menu_sound_sounds};

Menu *sound_menu = NULL; //Menu(NULL,15,menu_sound_entries);

Submenu *menu_sound = NULL; //Submenu( "Sound Menu", &sound_menu );

*/
//////////////////////////////End Sound Menu//////////////////////////////////

extern Submenu *menu_wave;
extern Submenu *menu_wavetex;
extern Submenu *menu_waverender;
extern Submenu *menu_draw;
extern Submenu *menu_sound;
extern Submenu *menu_cheat;
extern Submenu *menu_scoring;

#if 0
int liter0,liteg0,liteb0,liter1,liteg1,liteb1;
int darkr0,darkg0,darkb0,darkr1,darkg1,darkb1;

MENUENTRYBUTTONFUNCTION(LiteButton)
{
	switch ( buttonid )
	{
		case MENUCMD_LEFT    :
		case MENUCMD_RIGHT   :
			WAVETEX_TweakPalette(0,liter0,liteg0,liteb0,liter1,liteg1,liteb1)
	}
	return true;
}

MENUENTRYBUTTONFUNCTION(DarkButton)
{
	switch ( buttonid )
	{
		case MENUCMD_LEFT    :
		case MENUCMD_RIGHT   :
			WAVETEX_TweakPalette(1,darkr0,darkg0,darkb0,darkr1,darkg1,darkb1)
	}
	return true;
}

//MenuEntryIntCallbackEdit editlr0=MenuEntryIntCallbackEdit("Light");

#endif

// Memory menu ------------------------------------------------------

MENUENTRYBUTTONFUNCTION(MemoryDump)
{
	switch ( buttonid )
	{
		case MENUCMD_CROSS:
			mem_dump_heap();
			menus->CloseMenu();
			break;
	}
	return true;
}

MENUENTRYBUTTONFUNCTION(MemoryLeakDump)
{
	switch ( buttonid )
	{
		case MENUCMD_CROSS:
			mem_leak_test(false);
			menus->CloseMenu();
			break;
	}
	return true;
}


MENUENTRYBUTTONFUNCTION(MemoryScreen)
{
	switch ( buttonid )
	{
		case MENUCMD_CROSS:
      if (g_debug.mem_free_screen)
        g_debug.mem_free_screen = 0;
      else
        g_debug.mem_free_screen = 1;
			break;
	}
	return true;
}
MenuEntryFunction *memfreescreen= NULL; //MenuEntryFunction("Toggle Mem Free Display",      MemoryScreen );
MenuEntryFunction *memdump     = NULL; //MenuEntryFunction("Dump heap",      MemoryDump );
MenuEntryFunction *memdumpleaks= NULL; //MenuEntryFunction("Dump heap",      MemoryDump );
pMenuEntry menu_memory_entries[3]; //={ &memdump, &memfreescreen};

Menu *memory_menu = NULL; //Menu(NULL,2,menu_memory_entries);

Submenu *menu_memory = NULL; //Submenu( "Memory Menu", &memory_menu );


//-------------------------------------
// Exit level

MENUENTRYBUTTONFUNCTION(ExitLevel)
{
	if ( buttonid==MENUCMD_CROSS )
	{
	 	menus->CloseAllMenus();
		app::inst()->get_game()->end_level();
	}
	return true;
}

#ifdef PROFILING_ON
static MenuEntryFunction *menu_show_profile_info = NULL;

static MENUENTRYBUTTONFUNCTION(ToggleProfileInfo)
{
	switch ( buttonid )
	{
	case MENUCMD_CROSS:
		g_show_profile_info = !g_show_profile_info;
		break;
	}
	return true;
}
#endif

MenuEntryFunction *exit_level    = NULL; //MenuEntryFunction( "Exit Level    ",   ExitLevel  );

//-------------------------------------
// Main menu

MenuEntryTitle *title = NULL; //MenuEntryTitle("Debug menu");

// Initialized here to get the array size right, then again in StaticInit.  (dc 05/28/01)
pMenuEntry menu_main_entries[]={
	(pMenuEntry) &menu_cam,
	(pMenuEntry) &menu_scoring,
	(pMenuEntry) &menu_replay,
	(pMenuEntry) &menu_physics,
	(pMenuEntry) &menu_particles,
	(pMenuEntry) &menu_rumble,
	(pMenuEntry) &menu_wavetex,
#ifndef TARGET_GC
	(pMenuEntry) &menu_waverender,
#endif
	(pMenuEntry) &menu_wave,
	(pMenuEntry) &menu_anim,
	(pMenuEntry) &menu_memory,
	(pMenuEntry) &menu_sound,
	(pMenuEntry) &menu_draw,
	(pMenuEntry) &menu_fr,
	(pMenuEntry) &menu_cheat,
#ifdef PROFILING_ON
	(pMenuEntry) &menu_show_profile_info,
	(pMenuEntry) &menu_prof,
#endif
	(pMenuEntry) &exit_level,
	(pMenuEntry) &menu_freeze ,
};

//Menu *menu_main = NULL; //Menu(NULL,sizeof(menu_main_entries)/sizeof(menu_main_entries[0]),menu_main_entries);
KSMainMenu *menu_main = NULL; //KSMainMenu(NULL,sizeof(menu_main_entries)/sizeof(menu_main_entries[0]),menu_main_entries);


extern pMenuEntry camtool_entries[2]; // = { &camtool_title, &entry_add_cam };
extern Menu *menu_inner_camtool; // = NULL; //Menu(NULL, 2, camtool_entries);


void KSDBMENU_InitMainMenu( void )
{
	menu_main->ClearMenu();
	menu_main->AddEntries(sizeof(menu_main_entries)/sizeof(menu_main_entries[0]),menu_main_entries);
	menu_inner_cam->ClearMenu();
	menu_inner_cam->AddEntries(sizeof(camera_entries)/sizeof(camera_entries[0]), camera_entries);
	menu_inner_camtool->ClearMenu();
	menu_inner_camtool->AddEntries(2, camtool_entries);
}

void KSDBMENU_KillMainMenu( void )
{
	menu_main->ClearMenu();
	menu_inner_cam->ClearMenu();
	menu_inner_camtool->ClearMenu();
	
}








#ifdef USEFAKEFRONTEND


//-------------------------------------
// Fake front end


MENUENTRYBUTTONFUNCTION(GoANTARCTICA)
{
	if ( buttonid==MENUCMD_CROSS )
	{
		app::inst()->get_game()->unpause();
		app::inst()->get_game()->set_level(BEACH_ANTARCTICA);
		app::inst()->get_game()->go_next_state();
	 	menus->CloseAllMenus();
	}
	return true;
}

MENUENTRYBUTTONFUNCTION(GoBELLS)
{
	if ( buttonid==MENUCMD_CROSS )
	{
		app::inst()->get_game()->unpause();
		app::inst()->get_game()->set_level(BEACH_BELLS);
		app::inst()->get_game()->go_next_state();
	 	menus->CloseAllMenus();
	}
	return true;
}

MENUENTRYBUTTONFUNCTION(GoCORTESBANKS)
{
	if ( buttonid==MENUCMD_CROSS )
	{
		app::inst()->get_game()->unpause();
		app::inst()->get_game()->set_level(BEACH_CORTESBANK);
		app::inst()->get_game()->go_next_state();
	 	menus->CloseAllMenus();
	}
	return true;
}

MENUENTRYBUTTONFUNCTION(GoGLAND)
{
	if ( buttonid==MENUCMD_CROSS )
	{
		app::inst()->get_game()->unpause();
		app::inst()->get_game()->set_level(BEACH_GLAND);
		app::inst()->get_game()->go_next_state();
	 	menus->CloseAllMenus();
	}
	return true;
}


MENUENTRYBUTTONFUNCTION(GoJAWS)
{
	if ( buttonid==MENUCMD_CROSS )
	{
		app::inst()->get_game()->unpause();
		app::inst()->get_game()->set_level(BEACH_JAWS);
		app::inst()->get_game()->go_next_state();
	 	menus->CloseAllMenus();
	}
	return true;
}

MENUENTRYBUTTONFUNCTION(GoJEFFERSONBAY)
{
	if ( buttonid==MENUCMD_CROSS )
	{
		app::inst()->get_game()->unpause();
		app::inst()->get_game()->set_level(BEACH_JEFFERSONBAY);
		app::inst()->get_game()->go_next_state();
	 	menus->CloseAllMenus();
	}
	return true;
}

MENUENTRYBUTTONFUNCTION(GoMAVERICKS)
{
	if ( buttonid==MENUCMD_CROSS )
	{
		app::inst()->get_game()->unpause();
		app::inst()->get_game()->set_level(BEACH_MAVERICKS);
		app::inst()->get_game()->go_next_state();
	 	menus->CloseAllMenus();
	}
	return true;
}

MENUENTRYBUTTONFUNCTION(GoMUNDAKA)
{
	if ( buttonid==MENUCMD_CROSS )
	{
		app::inst()->get_game()->unpause();
		app::inst()->get_game()->set_level(BEACH_MUNDAKA);
		app::inst()->get_game()->go_next_state();
	 	menus->CloseAllMenus();
	}
	return true;
}

MENUENTRYBUTTONFUNCTION(GoPIPELINE)
{
	if ( buttonid==MENUCMD_CROSS )
	{
		app::inst()->get_game()->unpause();
		app::inst()->get_game()->set_level(BEACH_PIPELINE);
		app::inst()->get_game()->go_next_state();
	 	menus->CloseAllMenus();
	}
	return true;
}

MENUENTRYBUTTONFUNCTION(GoSEBASTIAN)
{
	if ( buttonid==MENUCMD_CROSS )
	{
		app::inst()->get_game()->unpause();
		app::inst()->get_game()->set_level(BEACH_SEBASTIAN);
		app::inst()->get_game()->go_next_state();
	 	menus->CloseAllMenus();
	}
	return true;
}

MENUENTRYBUTTONFUNCTION(GoTEAHUPOO)
{
	if ( buttonid==MENUCMD_CROSS )
	{
		app::inst()->get_game()->unpause();
		app::inst()->get_game()->set_level(BEACH_TEAHUPOO);
		app::inst()->get_game()->go_next_state();
	 	menus->CloseAllMenus();
	}
	return true;
}

MENUENTRYBUTTONFUNCTION(GoTRESTLES)
{
	if ( buttonid==MENUCMD_CROSS )
	{
		app::inst()->get_game()->unpause();
		app::inst()->get_game()->set_level(BEACH_TRESTLES);
		app::inst()->get_game()->go_next_state();
	 	menus->CloseAllMenus();
	}
	return true;
}

MenuEntryFunction *startANTARCTICA     = NULL; //MenuEntryFunction( "Antarctica   ",   GoANTARCTICA    );
MenuEntryFunction *startBELLS          = NULL; //MenuEntryFunction( "Bells        ",   GoBELLS         );
MenuEntryFunction *startCORTESBANKS    = NULL; //MenuEntryFunction( "Cortes Banks ",   GoCORTESBANKS   );
MenuEntryFunction *startGLAND          = NULL; //MenuEntryFunction( "G-Land       ",   GoGLAND         );
MenuEntryFunction *startJAWS           = NULL; //MenuEntryFunction( "Jaws         ",   GoJAWS          );
MenuEntryFunction *startJEFFERSONBAY   = NULL; //MenuEntryFunction( "Jefferies Bay",   GoJEFFERSONBAY  );
MenuEntryFunction *startMAVERICKS      = NULL; //MenuEntryFunction( "Mavericks    ",   GoMAVERICKS     );
MenuEntryFunction *startMUNDAKA        = NULL; //MenuEntryFunction( "Mundaka      ",   GoMUNDAKA       );
MenuEntryFunction *startPIPELINE       = NULL; //MenuEntryFunction( "Pipeline     ",   GoPIPELINE      );
MenuEntryFunction *startSEBASTIAN      = NULL; //MenuEntryFunction( "Sebastian    ",   GoSEBASTIAN     );
MenuEntryFunction *startTEAHUPOO       = NULL; //MenuEntryFunction( "Teahupoo     ",   GoTEAHUPOO      );
MenuEntryFunction *startTRESTLES       = NULL; //MenuEntryFunction( "Trestles     ",   GoTRESTLES      );

MenuEntryTitle *ffetitle = NULL; //MenuEntryTitle("Fake Front End");

pMenuEntry ffe_entries[14]; /*={&ffetitle,
	&startBELLS          ,
	&startGLAND          ,
	&startTRESTLES       ,
	&startJEFFERSONBAY   ,
	&startHUNTINGTON     ,
	&startJAWS           ,
	&startMAVERICKS      ,
	&startMUNDAKA        ,
	&startPIPELINE       ,
	&startSEBASTIAN      ,
	&startTEAHUPOO       ,
	&startANTARCTICA     ,
	&startCORTESBANKS
};*/

Menu *fakefrontend = NULL; //Menu(NULL,sizeof(ffe_entries)/sizeof(ffe_entries[0]),ffe_entries);

#endif

void KSDBMENU_StaticInit( void )
{
	// menu system

	menus = NEW MenuSystem(menurect);

	// Particle menu

  int index=0;
	entry_particles_test = NEW MenuEntryFunction("Test", particles_test);
	entry_particles_info = NEW MenuEntryIntEdit ("Show Info", &particles_info, 0, 1);
	entry_snap_size = NEW MenuEntryFloatEdit ("Snap size", &snap_size, 1, 10, 1.0f);
	entry_snap_life = NEW MenuEntryFloatEdit ("Snap life", &snap_life, 0, 10, 1.0f);
	entry_snap_collapse = NEW MenuEntryFloatEdit ("Snap collapse", &snap_collapse, 0, 10, 1.0f);

	particles_entries[index++] = entry_particles_test;
	particles_entries[index++] = entry_particles_info;
	particles_entries[index++] = entry_snap_size;
	particles_entries[index++] = entry_snap_life;
	particles_entries[index++] = entry_snap_collapse;
	assert(index==countof(particles_entries));
	menu_inner_particles = NEW Menu(NULL, index, particles_entries);
	menu_particles = NEW Submenu("Particles Menu", menu_inner_particles);

	// Camera menu

	entry_user_cam = NEW MenuEntryFunction("User Cam", UserCamButton);
	entry_replay_cam = NEW MenuEntryFunction("Replay Cam", ReplayCamButton);
	entry_ksdebug_cam = NEW MenuEntryFunction("KSDebug Cam", KSDebugCamButton);
	entry_beach_cam = NEW MenuEntryFunction("Beach Cam", BeachCamButton);
	entry_fps_cam = NEW MenuEntryFunction("FPS Cam", FpsCamButton);
	entry_auto_cam = NEW MenuEntryFunction("Auto Cam", AutoCamButton);
	entry_big_wave_cam = NEW MenuEntryFunction("Big Wave Cam", BigWaveCamButton);
	entry_old_shoulder_cam = NEW MenuEntryFunction("Old Shoulder Cam", OldShoulderCamButton);
	entry_shoulder_cam = NEW MenuEntryFunction("Shoulder Cam", ShoulderCamButton);
	entry_follow_cam = NEW MenuEntryFunction("Follow Cam", FollowCamButton);
	entry_follow_close_cam = NEW MenuEntryFunction("Follow Close Cam", FollowCloseCamButton);
	entry_buoy_cam = NEW MenuEntryFunction("Buoy Cam", BuoyCamButton);

	entry_field_of_view = NEW MenuEntryFloatEdit ("Field of View", &field_of_view_fudge, 0.2f, 4.0f, 0.01f);
	entry_shoulder_cam_lag = NEW MenuEntryFloatEdit ("Shoulder lag", &shoulder_cam_lag, 0, 10, 0.1f);
	entry_shoulder_cam_dy = NEW MenuEntryFloatEdit ("Shoulder dy", &shoulder_cam_dy, 0, 20, 0.1f);
	entry_shoulder_cam_dz = NEW MenuEntryFloatEdit ("Shoulder dz", &shoulder_cam_dz, 0, 20, 0.1f);
	entry_shoulder_distance = NEW MenuEntryFloatEdit ("Shoulder dist", &distance_mult, 1, 10, 0.1f);

	entry_beachcam_zoom = NEW MenuEntryFloatEdit("Beach Cam Zoom", &gBeachCamZoom, 0.0f, 1.0f, 1.0f);
	entry_beachcam_freq = NEW MenuEntryFloatEdit("Beach Cam Speed", &g_freq_scale, 0.0f, 4.0f, 0.05f);
	entry_beachcam_dist = NEW MenuEntryFloatEdit("Beach Cam Dist", &g_beachcam_dist_scale, 0.0f, 4.0f, 0.05f);

	camsettings_entries[0]=entry_field_of_view;
	camsettings_entries[1]=entry_beachcam_zoom;
	camsettings_entries[2]=entry_beachcam_freq;
	camsettings_entries[3]=entry_beachcam_dist;

	menu_inner_camsettings = NEW Menu(NULL, sizeof(camsettings_entries)/sizeof(camsettings_entries[0]), camsettings_entries);
	menu_camsettings = NEW Submenu ("Camera Settings", menu_inner_camsettings);

	index=0;
	camera_entries[ index++ ] =entry_user_cam;
	camera_entries[ index++ ] =entry_beach_cam;
	camera_entries[ index++ ] =entry_fps_cam;
	camera_entries[ index++ ] =entry_auto_cam;
	camera_entries[ index++ ] =entry_big_wave_cam;
	camera_entries[ index++ ] =entry_ksdebug_cam;
	camera_entries[ index++ ] =entry_old_shoulder_cam;
	camera_entries[ index++ ] =entry_shoulder_cam;
	camera_entries[ index++ ] =entry_replay_cam;
	camera_entries[ index++ ] =entry_follow_cam;
	camera_entries[ index++ ] =entry_follow_close_cam;
	camera_entries[ index++ ] =entry_buoy_cam;
	camera_entries[ index++ ] =menu_camsettings;
	camera_entries[ index++ ] =menu_camtool;
	assert(index==countof(camera_entries));

	menu_inner_cam = NEW Menu(NULL, index, camera_entries);
	menu_cam = NEW Submenu ("Camera Menu", menu_inner_cam);

	///////////////    Physics debug menu code   ////////////////////////////

	entry_physics_reset = NEW MenuEntryFunction("Reset", physics_reset);
	entry_flyby_reset = NEW MenuEntryFunction("Flyby Reset", flyby_reset);

	entry_physics_lie_pullback = NEW MenuEntryFloatEdit ("Lie Pullback", &SURFER_LIE_PULLBACK, 0, 10, 0.01f);
	entry_physics_lie_paddle_acc = NEW MenuEntryFloatEdit ("Paddle Acc", &SURFER_LIE_PADDLE_ACCELERATION, 0, 10, 0.01f);
	entry_physics_test_big_wave = NEW MenuEntryIntEdit ("Test Big Wave", &test_big_wave, 0, 1);
	entry_physics_drag = NEW MenuEntryFloatEdit ("Friction", &water_friction, 0, 10, 0.01f);
	entry_physics_skag = NEW MenuEntryFloatEdit ("Skag", &skag_friction, 0, 50, 0.1f);
	entry_physics_gravity = NEW MenuEntryFloatEdit ("Gravity", &world_gravity, 0, 5.0f, 0.05f);
	entry_physics_stick = NEW MenuEntryFloatEdit ("Stick", &stick_factor, 0, 10, 0.1f);
	entry_physics_backofwave = NEW MenuEntryFloatEdit("Go Over Back?", &g_go_over_back, 0.0f, 1.0f, 1.0f);
	entry_special_meter_full = NEW MenuEntryFloatEdit("Always Link?", &g_special_meter_full, 0.0f, 1.0f, 1.0f);

	entry_physics_stability = NEW MenuEntryFloatEdit ("Stability", &stability_factor, 0, 3, 0.01f);
	entry_physics_speed = NEW MenuEntryFloatEdit ("Speed", &speed_factor, 0, 5, 0.01f);
	entry_physics_carving = NEW MenuEntryFloatEdit ("Carving", &carving_factor, 0, 3, 0.01f);
	entry_physics_trick = NEW MenuEntryFloatEdit ("Trick", &trick_factor, 0, 3, 0.01f);
	entry_physics_wipeout = NEW MenuEntryIntEdit ("No Wipeouts", &g_disable_wipeouts, 0, 1);
	entry_physics_debug_balance = NEW MenuEntryIntEdit ("Debug Balance", &g_debug_balance, 0, 1);
	entry_physics_balance = NEW MenuEntryIntEdit ("Perfect Balance", &g_perfect_balance, 0, 1);

	entry_floats_my = NEW MenuEntryFloatEdit ("Max dy", &g_max_dy, 0, 2, 0.01f);
	entry_floats_sy = NEW MenuEntryFloatEdit ("Speed dy", &g_speed_dy, 0, 5, 0.1f);
	entry_floats_ma = NEW MenuEntryFloatEdit ("Max Angle", &g_max_angle, 0, 45, 1.0f);
	entry_floats_sa = NEW MenuEntryFloatEdit ("Speed Angle", &g_speed_angle, 0, 5, 0.1f);

	floats_entries[0] = entry_floats_my;
	floats_entries[1] = entry_floats_sy;
	floats_entries[2] = entry_floats_ma;
	floats_entries[3] = entry_floats_sa;

	menu_inner_floats = NEW Menu(NULL, sizeof(floats_entries)/sizeof(floats_entries[0]), floats_entries);
	menu_floats = NEW Submenu ("Floating Objects", menu_inner_floats);

	index=0;
	physics_entries[index++] =  entry_physics_reset;
	physics_entries[index++] =  entry_flyby_reset;
	physics_entries[index++] =  entry_physics_lie_pullback;
	physics_entries[index++] =  entry_physics_lie_paddle_acc;
	physics_entries[index++] =  entry_physics_test_big_wave;
	physics_entries[index++] =  entry_physics_drag;
	physics_entries[index++] =  entry_physics_skag;
	physics_entries[index++] =  entry_physics_gravity;
    physics_entries[index++] =  entry_physics_stick;
	physics_entries[index++] =  entry_physics_backofwave;
	physics_entries[index++] =  entry_special_meter_full;
	physics_entries[index++] =  entry_physics_stability;
	physics_entries[index++] =  entry_physics_speed;
	physics_entries[index++] =  entry_physics_carving;
	physics_entries[index++] =  entry_physics_trick;
	physics_entries[index++] =  entry_physics_wipeout;
	physics_entries[index++] =  entry_physics_debug_balance;
	physics_entries[index++] =  entry_physics_balance;
	physics_entries[index++] =  menu_floats;

	assert(index==countof(physics_entries));
	menu_inner_physics = NEW Menu(NULL, index, physics_entries);
	menu_physics = NEW Submenu ("Physics Menu", menu_inner_physics);


//////////////////////// RUMBLE LEVELS ////////////////////////////////

  rumble_level_landing     = NEW MenuEntryFloatEdit ("LANDING",
                                                                &rumbleMan.rumbleLevels[rumbleManager::LANDING], 0, 1, 0.05f);

  rumble_level_in_air      = NEW MenuEntryFloatEdit ("IN_AIR",
                                                                &rumbleMan.rumbleLevels[rumbleManager::IN_AIR], 0, 1, 0.05f);
  rumble_level_wiping_out  = NEW MenuEntryFloatEdit ("WIPING OUT",
                                                                &rumbleMan.rumbleLevels[rumbleManager::WIPING_OUT], 0, 1, 0.05f);
  rumble_level_underwater  = NEW MenuEntryFloatEdit ("UNDERWATER",
                                                                &rumbleMan.rumbleLevels[rumbleManager::UNDERWATER], 0, 1, 0.05f);
  rumble_level_grinding_object = NEW MenuEntryFloatEdit ("GRIDING OBJECTS",
                                                                &rumbleMan.rumbleLevels[rumbleManager::GRINDING_OBJECT], 0, 1, 0.05f);
  rumble_level_floater     = NEW MenuEntryFloatEdit ("FLOATER",
                                                                &rumbleMan.rumbleLevels[rumbleManager::FLOATER], 0, 1, 0.05f);
  rumble_level_in_wash     = NEW MenuEntryFloatEdit ("IN_WASH",
                                                                &rumbleMan.rumbleLevels[rumbleManager::IN_WASH], 0, 1, 0.05f);
  rumble_level_pocket      = NEW MenuEntryFloatEdit ("LIE - IN THE POCKET",
                                                                &rumbleMan.rumbleLevels[rumbleManager::LIE_ON_BOARD_POCKET], 0, 1, 0.05f);
  rumble_level_face        = NEW MenuEntryFloatEdit ("LIE - ON THE FACE",
                                                                &rumbleMan.rumbleLevels[rumbleManager::LIE_ON_BOARD_FACE], 0, 1, 0.05f);
  rumble_level_chin        = NEW MenuEntryFloatEdit ("LIE - ON THE CHIN",
                                                                &rumbleMan.rumbleLevels[rumbleManager::LIE_ON_BOARD_CHIN], 0, 1, 0.05f);
  rumble_level_tube        = NEW MenuEntryFloatEdit ("STANDING NEAR TUBE",
                                                                &rumbleMan.rumbleLevels[rumbleManager::STANDING_NEAR_TUBE], 0, 1, 0.05f);
	index=0;
	rumble_level_entries[index++] = rumble_level_landing;
	rumble_level_entries[index++] = rumble_level_in_air;
	rumble_level_entries[index++] = rumble_level_wiping_out;
	rumble_level_entries[index++] = rumble_level_underwater;
	rumble_level_entries[index++] = rumble_level_grinding_object;
	rumble_level_entries[index++] = rumble_level_floater;
  rumble_level_entries[index++] = rumble_level_in_wash;
	rumble_level_entries[index++] = rumble_level_pocket;
	rumble_level_entries[index++] = rumble_level_face;
	rumble_level_entries[index++] = rumble_level_chin;
	rumble_level_entries[index++] = rumble_level_tube;
	assert(index==countof(rumble_level_entries));
  menu_rumble_level = NEW Menu(NULL, index, rumble_level_entries);
  sub_menu_rumble_level = NEW Submenu( "Rumble Intensities", menu_rumble_level );

	//////////////////////// RUMBLE FREQUENCY ////////////////////////////////


  rumble_freq_landing     = NEW MenuEntryFloatEdit ("LANDING",
                                                                &rumbleMan.rumbleFreqs[rumbleManager::LANDING], 0, 2, 1);

  rumble_freq_in_air      = NEW MenuEntryFloatEdit ("IN_AIR",
                                                                &rumbleMan.rumbleFreqs[rumbleManager::IN_AIR], 0, 2, 1);
  rumble_freq_wiping_out  = NEW MenuEntryFloatEdit ("WIPING OUT",
                                                                &rumbleMan.rumbleFreqs[rumbleManager::WIPING_OUT], 0, 2, 1);
  rumble_freq_underwater  = NEW MenuEntryFloatEdit ("UNDERWATER",
                                                                &rumbleMan.rumbleFreqs[rumbleManager::UNDERWATER], 0, 2, 1);
  rumble_freq_grinding_object = NEW MenuEntryFloatEdit ("GRIDING OBJECTS",
                                                                &rumbleMan.rumbleFreqs[rumbleManager::GRINDING_OBJECT], 0, 2, 1);
  rumble_freq_floater     = NEW MenuEntryFloatEdit ("FLOATER",
                                                                &rumbleMan.rumbleFreqs[rumbleManager::FLOATER], 0, 2, 1);
  rumble_freq_in_wash     = NEW MenuEntryFloatEdit ("IN_WASH",
                                                                &rumbleMan.rumbleFreqs[rumbleManager::IN_WASH], 0, 2, 1);
  rumble_freq_pocket      = NEW MenuEntryFloatEdit ("LIE - IN THE POCKET",
                                                                &rumbleMan.rumbleFreqs[rumbleManager::LIE_ON_BOARD_POCKET], 0, 2, 1);
  rumble_freq_face        = NEW MenuEntryFloatEdit ("LIE - ON THE FACE",
                                                                &rumbleMan.rumbleFreqs[rumbleManager::LIE_ON_BOARD_FACE], 0, 2, 1);
  rumble_freq_chin        = NEW MenuEntryFloatEdit ("LIE - ON THE CHIN",
                                                                &rumbleMan.rumbleFreqs[rumbleManager::LIE_ON_BOARD_CHIN], 0, 2, 1);
  rumble_freq_tube        = NEW MenuEntryFloatEdit ("STANDING NEAR TUBE",
                                                                &rumbleMan.rumbleFreqs[rumbleManager::STANDING_NEAR_TUBE], 0, 2, 1);

	index=0;
	rumble_freq_entries[index++] =  rumble_freq_landing;
	rumble_freq_entries[index++] =  rumble_freq_in_air;
	rumble_freq_entries[index++] =  rumble_freq_wiping_out;
	rumble_freq_entries[index++] =  rumble_freq_underwater;
	rumble_freq_entries[index++] =  rumble_freq_grinding_object;
	rumble_freq_entries[index++] =  rumble_freq_floater;
  rumble_freq_entries[index++] =  rumble_freq_in_wash;
	rumble_freq_entries[index++] =  rumble_freq_pocket;
	rumble_freq_entries[index++] =  rumble_freq_face;
	rumble_freq_entries[index++] =  rumble_freq_chin;
	rumble_freq_entries[index++] =  rumble_freq_tube;
	assert(index==countof(rumble_freq_entries));
  menu_rumble_freq = NEW Menu(NULL, index, rumble_freq_entries);
  sub_menu_rumble_freq = NEW Submenu( "Rumble Frequency (0 - low 1 - high 2 - both)", menu_rumble_freq );

//////////////////////// RUMBLE VARIANCE ////////////////////////////////


  rumble_amp_landing     = NEW MenuEntryFloatEdit ("LANDING",
                                                                &rumbleMan.rumbleVarAmplitudes[rumbleManager::LANDING], 0, 1, .05);

  rumble_amp_in_air      = NEW MenuEntryFloatEdit ("IN_AIR",
                                                                &rumbleMan.rumbleVarAmplitudes[rumbleManager::IN_AIR], 0, 1, .05);
  rumble_amp_wiping_out  = NEW MenuEntryFloatEdit ("WIPING OUT",
                                                                &rumbleMan.rumbleVarAmplitudes[rumbleManager::WIPING_OUT], 0, 1, .05);
  rumble_amp_underwater  = NEW MenuEntryFloatEdit ("UNDERWATER",
                                                                &rumbleMan.rumbleVarAmplitudes[rumbleManager::UNDERWATER], 0, 1, .05);
  rumble_amp_grinding_object = NEW MenuEntryFloatEdit ("GRIDING OBJECTS",
                                                                &rumbleMan.rumbleVarAmplitudes[rumbleManager::GRINDING_OBJECT], 0, 1, .05);
  rumble_amp_floater     = NEW MenuEntryFloatEdit ("FLOATER",
                                                                &rumbleMan.rumbleVarAmplitudes[rumbleManager::FLOATER], 0, 1, .05);
  rumble_amp_in_wash     = NEW MenuEntryFloatEdit ("IN_WASH",
                                                                &rumbleMan.rumbleVarAmplitudes[rumbleManager::IN_WASH], 0, 1, .05);
  rumble_amp_pocket      = NEW MenuEntryFloatEdit ("LIE - IN THE POCKET",
                                                                &rumbleMan.rumbleVarAmplitudes[rumbleManager::LIE_ON_BOARD_POCKET], 0, 1, .05);
  rumble_amp_face        = NEW MenuEntryFloatEdit ("LIE - ON THE FACE",
                                                                &rumbleMan.rumbleVarAmplitudes[rumbleManager::LIE_ON_BOARD_FACE], 0, 1, .05);
  rumble_amp_chin        = NEW MenuEntryFloatEdit ("LIE - ON THE CHIN",
                                                                &rumbleMan.rumbleVarAmplitudes[rumbleManager::LIE_ON_BOARD_CHIN], 0, 1, .05);
  rumble_amp_tube        = NEW MenuEntryFloatEdit ("STANDING NEAR TUBE",
                                                                &rumbleMan.rumbleVarAmplitudes[rumbleManager::STANDING_NEAR_TUBE], 0, 1, .05);

	index=0;
	rumble_amp_entries[index++] = rumble_amp_landing;
	rumble_amp_entries[index++] = rumble_amp_in_air;
	rumble_amp_entries[index++] = rumble_amp_wiping_out;
	rumble_amp_entries[index++] = rumble_amp_underwater;
	rumble_amp_entries[index++] = rumble_amp_grinding_object;
	rumble_amp_entries[index++] = rumble_amp_floater;
  rumble_amp_entries[index++] = rumble_amp_in_wash;
	rumble_amp_entries[index++] = rumble_amp_pocket;
	rumble_amp_entries[index++] = rumble_amp_face;
	rumble_amp_entries[index++] = rumble_amp_chin;
	rumble_amp_entries[index++] = rumble_amp_tube;
	assert(index==countof(rumble_amp_entries));
  menu_rumble_amp = NEW Menu(NULL, index, rumble_amp_entries);
  sub_menu_rumble_amp = NEW Submenu( "Rumble Variance", menu_rumble_amp );


//////////////////////// RUMBLE PERIOD ////////////////////////////////

  rumble_per_landing     = NEW MenuEntryFloatEdit ("LANDING",
                                                                &rumbleMan.rumbleVarPeriods[rumbleManager::LANDING], 0, 5, .1);

  rumble_per_in_air      = NEW MenuEntryFloatEdit ("IN_AIR",
                                                                &rumbleMan.rumbleVarPeriods[rumbleManager::IN_AIR], 0, 5, .1);
  rumble_per_wiping_out  = NEW MenuEntryFloatEdit ("WIPING OUT",
                                                                &rumbleMan.rumbleVarPeriods[rumbleManager::WIPING_OUT], 0, 5, .1);
  rumble_per_underwater  = NEW MenuEntryFloatEdit ("UNDERWATER",
                                                                &rumbleMan.rumbleVarPeriods[rumbleManager::UNDERWATER], 0, 5, .1);
  rumble_per_grinding_object = NEW MenuEntryFloatEdit ("GRIDING OBJECTS",
                                                                &rumbleMan.rumbleVarPeriods[rumbleManager::GRINDING_OBJECT], 0, 5, .1);
  rumble_per_floater     = NEW MenuEntryFloatEdit ("FLOATER",
                                                                &rumbleMan.rumbleVarPeriods[rumbleManager::FLOATER], 0, 5, .1);
  rumble_per_in_wash     = NEW MenuEntryFloatEdit ("IN_WASH",
                                                                &rumbleMan.rumbleVarPeriods[rumbleManager::IN_WASH], 0, 5, .1);
  rumble_per_pocket      = NEW MenuEntryFloatEdit ("LIE - IN THE POCKET",
                                                                &rumbleMan.rumbleVarPeriods[rumbleManager::LIE_ON_BOARD_POCKET], 0, 5, .1);
  rumble_per_face        = NEW MenuEntryFloatEdit ("LIE - ON THE FACE",
                                                                &rumbleMan.rumbleVarPeriods[rumbleManager::LIE_ON_BOARD_FACE], 0, 5, .1);
  rumble_per_chin        = NEW MenuEntryFloatEdit ("LIE - ON THE CHIN",
                                                                &rumbleMan.rumbleVarPeriods[rumbleManager::LIE_ON_BOARD_CHIN], 0, 5, .1);
  rumble_per_tube        = NEW MenuEntryFloatEdit ("STANDING NEAR TUBE",
                                                                &rumbleMan.rumbleVarPeriods[rumbleManager::STANDING_NEAR_TUBE], 0, 5, .1);

	index=0;

	rumble_per_entries[index++] = rumble_per_landing;
	rumble_per_entries[index++] = rumble_per_in_air;
	rumble_per_entries[index++] = rumble_per_wiping_out;
	rumble_per_entries[index++] = rumble_per_underwater;
	rumble_per_entries[index++] = rumble_per_grinding_object;
	rumble_per_entries[index++] = rumble_per_floater;
  rumble_per_entries[index++] = rumble_per_in_wash;
	rumble_per_entries[index++] = rumble_per_pocket;
	rumble_per_entries[index++] = rumble_per_face;
	rumble_per_entries[index++] = rumble_per_chin;
	rumble_per_entries[index++] = rumble_per_tube;
	assert(index==countof(rumble_per_entries));
  menu_rumble_per = NEW Menu(NULL, index, rumble_per_entries);
  sub_menu_rumble_per = NEW Submenu( "Rumble Period", menu_rumble_per );


	//////////////////////// RUMBLE MAIN ////////////////////////////////
  write_rumble    = NEW MenuEntryFunction( "Save Rumble Settings     ",   WriteRumbleButton   );
  show_rumble    = NEW MenuEntryFunction( "Show Current Rumble State",   ToggleShowRumble   );
	index=0;
	rumble_parent[index++] = sub_menu_rumble_level;
	rumble_parent[index++] = sub_menu_rumble_freq;
	rumble_parent[index++] = sub_menu_rumble_amp;
	rumble_parent[index++] = sub_menu_rumble_per;
	rumble_parent[index++] = write_rumble;
	rumble_parent[index++] = show_rumble;
	assert(index==countof(rumble_parent));
  menu_rumble_parent = NEW Menu(NULL, index, rumble_parent);
  menu_rumble = NEW Submenu ("Rumble Menu", menu_rumble_parent);



/////////////////   Replay debug submenu code  start   //////////////////////


	replay_play       = NEW MenuEntryFunction( "Play     ",   ReplayPlayButton   );
	replay_slow       = NEW MenuEntryFunction( "Slow Mo  ",   ReplaySlowButton    );
	replay_fwd        = NEW MenuEntryFunction( "Frame Fwd",   ReplayFwdButton    );
	replay_rew        = NEW MenuEntryFunction( "Frame Rew",   ReplayRewButton    );
	replay_restart    = NEW MenuEntryFunction( "Restart  ",   ReplayRestartButton    );
	replay_reset      = NEW MenuEntryFunction( "Reset    ",   ReplayResetButton    );
	replay_save       = NEW MenuEntryFunction( "Save    ",    ReplaySaveButton    );
	replay_load       = NEW MenuEntryFunction( "Load    ",    ReplayLoadButton    );
	replay_cancel     = NEW MenuEntryFunction( "Cancel   ",   ReplayCancelButton );

	entry_ksdebug_cam_replay = NEW MenuEntryFunction("KSDebug Cam", KSDebugCamButton);
	entry_beach_cam_replay = NEW MenuEntryFunction("Beach Cam", BeachCamButton);
	entry_auto_cam_replay = NEW MenuEntryFunction("Auto Cam", AutoCamButton);
	entry_big_wave_cam_replay = NEW MenuEntryFunction("Big Wave Cam", BigWaveCamButton);
	entry_old_shoulder_cam_replay = NEW MenuEntryFunction("Old Shoulder Cam", OldShoulderCamButton);
	entry_follow_cam_replay = NEW MenuEntryFunction("Follow Cam", FollowCamButton);
	entry_follow_cam_replay = NEW MenuEntryFunction("Follow Close Cam", FollowCamButton);
	entry_buoy_cam_replay = NEW MenuEntryFunction("Buoy Cam", BuoyCamButton);

	index=0;
	camera_entries_replay[index++] = entry_beach_cam_replay;
	camera_entries_replay[index++] = entry_auto_cam_replay;
	camera_entries_replay[index++] = entry_big_wave_cam_replay;
	camera_entries_replay[index++] = entry_ksdebug_cam_replay;
	camera_entries_replay[index++] = entry_old_shoulder_cam_replay;
	camera_entries_replay[index++] = entry_follow_cam_replay;
	camera_entries_replay[index++] = entry_follow_close_cam_replay;
	camera_entries_replay[index++] = entry_buoy_cam_replay;
	assert(index==countof(camera_entries_replay));
	menu_inner_cam_replay = NEW Menu(NULL, index, camera_entries_replay);
	menu_cam_replay = NEW Submenu ("Camera", menu_inner_cam_replay);

	index=0;
	replay_entries[index++] =  replay_play;
	replay_entries[index++] =  replay_slow;
	replay_entries[index++] =  replay_fwd;
	replay_entries[index++] =  replay_restart;
	replay_entries[index++] =  replay_reset;
	replay_entries[index++] =  replay_cancel;
	replay_entries[index++] =  replay_save;
	replay_entries[index++] =  replay_load;
	replay_entries[index++] =  menu_cam_replay;
	assert(index==countof(replay_entries));
	menu_inner_replay = NEW Menu(NULL, index, replay_entries);
	menu_replay = NEW Submenu ("Replay Menu", menu_inner_replay);

	menu_freeze = NEW MenuEntryFunction( "Freeze action",   FreezeButton  );
	menu_fr =   NEW MenuEntryFunction( "Show FPS",   FRButton  );
	menu_prof = NEW MenuEntryFunction( "Dump Profile Info",   ProfButton  );

	#if 0
	//-------------------------------------
	// Samples menu entries

	menu_null = NEW MenuEntryFunction( "No Effect",   NullButton  );
	menu_show = NEW MenuEntryFunction( "Show Button", DebugButton );
	menu_back = NEW MenuEntryFunction( "Prev Menu",   BackButton  );

	pMenuEntry menu_inner_entries[4]={&menu_null, &menu_show, &menu_back, &menu_freeze };

	menu_inner = NEW Menu(NULL,4,menu_inner_entries);

	menu_sub = NEW Submenu( "Inner Menu", &menu_inner );
	menu_int = NEW MenuEntryIntEdit( "Int Value", &edited, 5, 20 );
	menu_hex = NEW MenuEntryIntEdit( "Hex Value", &xedited, 0, 255, " : 0x%X" );
	menu_float = NEW MenuEntryFloatEdit( "Float Value", &fedited, -10, 20, 0.1f );

	menu_testlist = NEW MenuEntryListEdit( "List", &ledited, 4, testlist);

	menu_enum = NEW MenuEntryEnumEdit( "Enum", &eedited, 4, testlisttext, testlistvals );


	sample_title= NEW MenuEntryTitle("Sample menu");

	pMenuEntry menu_sample_entries[11]={&sample_title, &menu_null, &menu_sub, &menu_show, &menu_int, &menu_hex, &menu_float, &menu_testlist, &menu_enum, &menu_back, &menu_freeze };

	menu_sample = NEW Menu(NULL,11,menu_sample_entries);

	menu_samples = NEW Submenu( "Sample menus", &menu_sample );
	#endif


	//////////////////////////////   Start Animation Menu   //////////////////////////////////

	menu_animmode_list = NEW MenuEntryListEdit( "Mode", &debug_mode, 2, anim_mode_list);


	//////////////////////   Start Debug Labels menu code  ////////////////////////////////////

	label_title= NEW MenuEntryTitle("Labels menu");

	state_menu = NEW MenuEntryIntEdit( "State Label", &show_state_label, 0, 1 );
	anim_menu = NEW MenuEntryIntEdit( "Anim Label", &show_anim_label, 0, 1 );
	wipeout_blur = NEW MenuEntryIntEdit ("Wipeout Blur", &g_wipeout_blur, 0, 1);

	index=0;
	menu_labels_entries[index++]=label_title;
	menu_labels_entries[index++]=state_menu;
	menu_labels_entries[index++]=anim_menu;
	menu_labels_entries[index++]=wipeout_blur;
	assert(index==countof(menu_labels_entries));
	menu_labels = NEW Menu(NULL, 3, menu_labels_entries);
	menu_label = NEW Submenu("Labels Menu", menu_labels);

	menu_animnum_list = NEW MenuEntryStringxListEdit( "Anim", &anim_num, (int) _SURFER_NUM_ANIMS, g_surfer_anims);

	anim_title= NEW MenuEntryTitle("Animation Menu");
	menu_frame_by_frame_list = NEW MenuEntryFloatEdit("Frame by frame?", &g_frame_by_frame, 0.0f, 1.0f, 1.0f);
	menu_debug_do_ik_list = NEW MenuEntryFloatEdit("Do IK?", &g_debug_mode_doik, 0.0f, 1.0f, 1.0f);
	wip_intensity_level = NEW MenuEntryFloatEdit("Set Intensity 2 Wip?", &g_intesity2_wip, 0.0f, 1.0f, 1.0f);

	index=0;
	menu_anim_entries[index++]= anim_title;
	menu_anim_entries[index++]= menu_animmode_list;
	menu_anim_entries[index++]= menu_frame_by_frame_list;
	menu_anim_entries[index++]= menu_debug_do_ik_list;
	menu_anim_entries[index++]= wip_intensity_level;
	menu_anim_entries[index++]= menu_animnum_list;
	menu_anim_entries[index++]= menu_label;
	assert(index==countof(menu_anim_entries));
	anim_menu2 = NEW Menu(NULL,index,menu_anim_entries);

	menu_anim = NEW Submenu( "Animation Menu", anim_menu2 );


	/////////////////////////////   End Animation Menu     ///////////////////////////////////////////

	///////////////////////////////Sound Menu///////////////////////////////////
/*
	soundStop     = NEW MenuEntryFunction("Stop All Sounds",           Stop   );
	soundGull1    = NEW MenuEntryFunction("Play Sea Gull 1",           Gull1  );
	soundGull2    = NEW MenuEntryFunction("Play Sea Gull 2",           Gull2  );
	soundGull3    = NEW MenuEntryFunction("Play Sea Gull 3",           Gull3  );

	soundBuoy1    = NEW MenuEntryFunction("Play Buoy 1",               Buoy1  );
	soundBuoy2    = NEW MenuEntryFunction("Play Buoy 2",               Buoy2  );

	soundLion1    = NEW MenuEntryFunction("Play Sea Lion 1",           Lion1  );
	soundLion2    = NEW MenuEntryFunction("Play Sea Lion 2",           Lion2  );


	soundFace1    = NEW MenuEntryFunction("Play Face Sound 1",         Face1  );
	soundFace2    = NEW MenuEntryFunction("Play Face Sound 2",         Face2  );
	soundTube1    = NEW MenuEntryFunction("Play Tube Sound 1",         Tube1  );
	soundTube2    = NEW MenuEntryFunction("Play Tube Sound 2",         Tube2  );
	soundSplash   = NEW MenuEntryFunction("Play Splash Sound",         Splash );
	soundUnder    = NEW MenuEntryFunction("Play Underwater Sound",     Under  );
	soundFoam     = NEW MenuEntryFunction("Play Foam Sound",           Foam   );
	soundCrash    = NEW MenuEntryFunction("Play Crash Sound",          Crash   );


	index=0;

	pmenu_sound_sounds[index++]= soundStop;
	pmenu_sound_sounds[index++]= soundGull1;
	pmenu_sound_sounds[index++]= soundGull2;
	pmenu_sound_sounds[index++]= soundGull3;
	pmenu_sound_sounds[index++]= soundBuoy1;
	pmenu_sound_sounds[index++]= soundBuoy2;
	pmenu_sound_sounds[index++]= soundLion1;
	pmenu_sound_sounds[index++]= soundLion2;
	pmenu_sound_sounds[index++]= soundFace1;
	pmenu_sound_sounds[index++]= soundFace2;
	pmenu_sound_sounds[index++]= soundTube1;
	pmenu_sound_sounds[index++]= soundTube2;
	pmenu_sound_sounds[index++]= soundSplash;
	pmenu_sound_sounds[index++]= soundUnder;
	pmenu_sound_sounds[index++]= soundFoam;
	pmenu_sound_sounds[index++]= soundCrash;
	assert(index==countof(pmenu_sound_sounds));
	sound_sounds_menu  = NEW Menu(NULL,index, pmenu_sound_sounds);
	menu_sound_sounds  = NEW Submenu( "Sound Samples Menu", sound_sounds_menu );



	soundMute     = NEW MenuEntryFunction("Mute Sound",      MuteSound  );
	soundUnmute   = NEW MenuEntryFunction( "Unmute Sound",   UnmuteSound  );
	toggleSpheres = NEW MenuEntryFunction( "Toggle Sound Extents", Spheres  );
	camPersp      = NEW MenuEntryFunction( "Set Camera Persp.", CamPerspective  );
	surferPersp   = NEW MenuEntryFunction( "Set Surfer Persp.", SurferPerspective  );

	fcVol  = NEW MenuEntryFloatEdit( "Face Raw Volume",         &wSound.faceVolume,   0.0f, 1.0f, .05f );
	tbVol  = NEW MenuEntryFloatEdit( "Tube Raw Volume",         &wSound.tubeVolume,   0.0f, 1.0f, .05f );
	foVol  = NEW MenuEntryFloatEdit( "Foam Raw Volume",         &wSound.foamVolume,   0.0f, 1.0f, .05f );

	ffmin  = NEW MenuEntryFloatEdit( "Foam Min",   &wSound.foamMin,  0.0f, 100.0f, .5);
	fbmin  = NEW MenuEntryFloatEdit( "Foam Min",   &wSound.foamMax,  0.0f, 100.0f, .5);

	ffmax  = NEW MenuEntryFloatEdit( "Face Max",   &wSound.faceMin,  0.0f, 100.0f, .5);
	fbmax  = NEW MenuEntryFloatEdit( "Face Max",   &wSound.faceMax,  0.0f, 100.0f, .5);

	tmin   = NEW MenuEntryFloatEdit( "Tube Min",     &wSound.tubeMax,      0.0f, 100.0f, .5);
	tmax   = NEW MenuEntryFloatEdit( "Tube Max",     &wSound.tubeMax,      0.0f, 100.0f, .5);


	index=0;

	menu_sound_entries[index++]= surferPersp;
	menu_sound_entries[index++]= camPersp;
	menu_sound_entries[index++]= soundMute;
	menu_sound_entries[index++]= soundUnmute;
	menu_sound_entries[index++]= fcVol;
	menu_sound_entries[index++]= tbVol;
	menu_sound_entries[index++]= foVol;
	menu_sound_entries[index++]= ffmin;
	menu_sound_entries[index++]= fbmin;
	menu_sound_entries[index++]= ffmax;
	menu_sound_entries[index++]= fbmax;
	menu_sound_entries[index++]= tmin;
	menu_sound_entries[index++]= tmax;
	menu_sound_entries[index++]= toggleSpheres;
	menu_sound_entries[index++]= menu_sound_sounds;
	assert(index==countof(menu_sound_entries));

	sound_menu = NEW Menu(NULL,index,menu_sound_entries);

	menu_sound = NEW Submenu( "Sound Menu", sound_menu );

*/
	//////////////////////////////End Sound Menu//////////////////////////////////

	// Memory menu ------------------------------------------------------

	memfreescreen= NEW MenuEntryFunction("Toggle Mem Free Display",      MemoryScreen );
	memdump     = NEW MenuEntryFunction("Dump heap",      MemoryDump );
	memdumpleaks= NEW MenuEntryFunction("Dump leaks",      MemoryLeakDump );
	menu_memory_entries[0]=memdump;
	menu_memory_entries[1]=memdumpleaks;
	menu_memory_entries[2]=memfreescreen;
	memory_menu = NEW Menu(NULL,3,menu_memory_entries);
	menu_memory = NEW Submenu( "Memory Menu", memory_menu );

#ifdef PROFILING_ON
	// Profiling --------------------------------------------------------

	menu_show_profile_info = NEW MenuEntryFunction("Show Profiling", ToggleProfileInfo);
#endif

	//-------------------------------------
	// Exit level

	exit_level    = NEW MenuEntryFunction( "Exit Level    ",   ExitLevel  );

	//-------------------------------------
	// Main menu

	title= NEW MenuEntryTitle("Debug menu");

	index=0;
	//menu_main_entries[index++]=
	//	&title,
	menu_main_entries[index++]=	menu_cam;
	menu_main_entries[index++]=	menu_scoring;
	menu_main_entries[index++]=	menu_replay;
	menu_main_entries[index++]=	menu_physics;
	menu_main_entries[index++]=	menu_particles;
	menu_main_entries[index++]= menu_rumble;
	menu_main_entries[index++]=	menu_wavetex;
#ifndef TARGET_GC
	menu_main_entries[index++]=	menu_waverender;
#endif
	menu_main_entries[index++]=	menu_wave;
	menu_main_entries[index++]=	menu_anim;
	menu_main_entries[index++]=	menu_memory;
	menu_main_entries[index++]=	menu_sound;
	menu_main_entries[index++]=	menu_draw;
	menu_main_entries[index++]=	menu_fr;
	menu_main_entries[index++]=	menu_cheat;
#ifdef PROFILING_ON
	menu_main_entries[index++]=	menu_show_profile_info;
	menu_main_entries[index++]=	menu_prof;
#endif
	menu_main_entries[index++]=	exit_level;
	menu_main_entries[index++]=	menu_freeze ;

	assert(((u_int) index)==countof(menu_main_entries));

	menu_main = NEW KSMainMenu(NULL,index,menu_main_entries);
	//fakefrontend = NEW Menu(NULL,sizeof(ffe_entries)/sizeof(ffe_entries[0]),ffe_entries);
}



