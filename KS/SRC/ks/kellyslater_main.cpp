#include "global.h"

#include "debug_render.h"
#include "game_info.h"
#include "wds.h"
#include "ksdbmenu.h"
#include "wave.h"
#include "ksfx.h"
#include "water.h"
#include "camera_tool.h"

#if defined(TARGET_PS2)
#include <sifdev.h>
#endif /* TARGET_PS2 JIV DEBUG */

#if defined(TARGET_XBOX)
#include "profiler.h"
#endif /* TARGET_XBOX JIV DEBUG */

#include "FrontEndManager.h"
#include "WaveSound.h"
extern camera_list *CameraList;

bool particle_enable = true;

#ifdef JWEBSTER
#ifdef DEBUG
extern vector3d g_wipeout_collide;
#endif
#endif

void world_dynamics_system::render_kelly_slater_stuff(const int heroIdx)
{
 // START_PROF_TIMER( proftimer_kelly_slater_interface );
 // IGODraw();
 // STOP_PROF_TIMER( proftimer_kelly_slater_interface );

/*#ifdef JWEBSTER
#ifdef DEBUG
	nglVector SphereColor = {1.0f, 0.0f, 0.0f, 0.7f};
	nglVector v = {g_wipeout_collide.x, g_wipeout_collide.y, g_wipeout_collide.z, 1.0f};

	extern nglMesh *nglSphereMesh;
	nglRenderParams Params;
	Params.Flags = NGLP_TINT | NGLP_SCALE;
	Params.TintColor = SphereColor;    // ---> color is the color of the sphere
	Params.Scale[0] = Params.Scale[1] = Params.Scale[2] = 3.0f * 10; // ---> scale is roughly the diameter of the sphere in meters
	nglMatrix Work;
	nglIdentityMatrix( Work );
	KSNGL_TranslateMatrix(Work, Work, v);    // ----> v is the location to draw the sphere
	nglListAddMesh(nglSphereMesh, Work, &Params);
#endif
#endif*/


  START_PROF_TIMER( proftimer_water );
  START_PROF_TIMER( proftimer_water_create );
	WATER_Create(heroIdx);
  STOP_PROF_TIMER( proftimer_water_create );
  START_PROF_TIMER( proftimer_water_submit );
  WATER_ListAdd();
  STOP_PROF_TIMER( proftimer_water_submit );
  STOP_PROF_TIMER( proftimer_water );

  START_PROF_TIMER( proftimer_kelly_slater_particle );
  if (particle_enable)
  {
	  ks_fx_draw(heroIdx);
  }
  STOP_PROF_TIMER( proftimer_kelly_slater_particle );

#ifdef DEBUG	// On XBox nglSphereMesh exists only in DEBUG
  extern WaveSound wSound;
  if (wSound.showSpheres)
    wSound.drawSoundPoints();
#endif
}

// This is a great function, i say we ship it :)
void world_dynamics_system::process_kelly_slater_stuff ()
{
}

extern bool superduperpausehack;

bool MENU_TurnMenuOn=false;

void world_dynamics_system::process_kelly_slater_debugmenus ()
{
	static bool bstate=TRUE;
	static bool lastMenuState = false;
	static bool lastPauseState = false;

	//if ( bstate && !MENU_GetButtonState(MENUCMD_SELECT) )
	if ( bstate && !g_game_ptr->was_select_pressed() )
	{
		bstate=FALSE;
	}
	else if ( MENU_TurnMenuOn )
	{
		MENU_TurnMenuOn=false;
		if ( !menus->IsActive() )
		{
			menus->OpenMenu(menu_main);
		}
	}
	//else if ( !bstate && MENU_GetButtonState(MENUCMD_SELECT) && !ksreplay.IsPlaying() && !frontendmanager.in_game_map_up)
	else if ( !bstate && g_game_ptr->was_select_pressed() && !frontendmanager.in_game_map_up && !dmm.inDemoMode() && !dmm.wasInDemo)
	{
		bstate=TRUE;
		if ( !menus->IsActive() )
		{
			menus->OpenMenu(menu_main);
		}
		else
		{
			menus->CloseMenu();
		}
	}
	menus->Tick(1);
	
	if ( menus->IsActive() && !lastMenuState)
	{
		if (!superduperpausehack)
		{
			lastPauseState = app::inst()->get_game()->is_paused();
			app::inst()->get_game()->enable_physics(false);
			IGODebug(true);
		}
	}
	else if (!menus->IsActive() && lastMenuState)
	{
		if (!superduperpausehack)
		{
			app::inst()->get_game()->enable_physics(!lastPauseState);
			IGODebug(false);
		}
	}
	lastMenuState = menus->IsActive();
	
	
	CameraList->Update();
}
