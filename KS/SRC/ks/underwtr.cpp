#include "global.h"		// included by all files
#include "ngl.h"	// For graphics engine
#include "kshooks.h"	// For KSWhatever calls
#include "wavetex.h"
#include "FrontEndManager.h"
#include "wds.h"

static float UNDERWATER_BottomMin=-100.0f;
static float UNDERWATER_BottomMax=100.0f;


entity *UNDERWATER_bottom_bottom=NULL;
entity *UNDERWATER_bottom_wall=NULL;
entity *UNDERWATER_bottom_ceiling=NULL;
entity *UNDERWATER_bottom_bubble=NULL;
entity *UNDERWATER_lightshafts=NULL;
entity *UNDERWATER_fish=NULL;

vector3d UNDERWATER_bottomspeed[MAX_PLAYERS];
vector3d UNDERWATER_bottompos[MAX_PLAYERS];
float WAVE_camx=0.0f;
float WAVE_camy=0.0f;
float WAVE_camz=0.0f;

float Wave_waterlevel = 0.2f;
float Wave_camerafudgefactory=0.0f;
float Wave_cameraoverwater[MAX_PLAYERS];
bool Wave_cameraunderwater[MAX_PLAYERS];
WavePositionHint underwaterhint[MAX_PLAYERS];


char *underwaterents[] = {
	"BOTTOM",
	"BOTTOM_WALL",
	"BOTTOM_CEILING",
	"CAMERA_CAPSULE",
	"LIGHTSHAFTS",
	"FISH",
	"ALGAE",
	"BUBBLES",
	"CAUSTICS",
	"EARTH",	// For Cosmos level (dc 07/13/02)
	"=S",
	""
};


bool check_youre_under_where=true;

void UNDERWATER_Init(int curbeach)
{
	int i;
	check_youre_under_where=true;

	for (i=0; i<MAX_PLAYERS; i++ )
	{
		Wave_cameraunderwater[i]=false;
		Wave_cameraoverwater[i]=10.0f; // not entirely accurate, but only affects first frame
	}

	UNDERWATER_bottom_wall = g_world_ptr->get_entity(BeachDataArray[curbeach].capsulename); //"BOTTOM_WALL") ;
	UNDERWATER_bottom_bottom = g_world_ptr->get_entity(BeachDataArray[curbeach].bottomname); //"BOTTOM") ;
	UNDERWATER_bottom_ceiling = g_world_ptr->get_entity("BOTTOM_CEILING");
	UNDERWATER_bottom_bubble = g_world_ptr->get_entity("CAMERA_CAPSULE");
	UNDERWATER_lightshafts = g_world_ptr->get_entity("LIGHTSHAFTS");
	UNDERWATER_fish = g_world_ptr->get_entity("FISH");

	if ( UNDERWATER_bottom_wall==NULL )
	{
		nglPrintf("Warning : Capsule entity not found (%s) transparency may look wrong\n",BeachDataArray[curbeach].capsulename);
	}
	if ( UNDERWATER_bottom_bottom==NULL )
	{
		nglPrintf("Warning : Bottom entity not found (%s) bottom scrolling not available\n",BeachDataArray[curbeach].bottomname);
	}
	if ( UNDERWATER_bottom_ceiling==NULL )
	{
		//nglPrintf("Warning : Capsule ceiling entity not found (%s)\n",BeachDataArray[curbeach].bottomname);
	}
	if ( UNDERWATER_bottom_bubble==NULL )
	{
		nglPrintf("Warning : Capsule bubble entity not found (%s)\n",BeachDataArray[curbeach].bottomname);
	}

	i=0;
	char *underent=underwaterents[i];
	while ( *underent )
	{
		entity *ent = g_world_ptr->get_entity(underent);
		if ( ent )
		{
			ent->put_in_underwater_scene(true);
		}
		underent=underwaterents[++i];
	}
	UNDERWATER_bottompos[0] = vector3d(0,0,0);
	UNDERWATER_bottompos[1] = vector3d(0,0,0);
	if ( UNDERWATER_bottom_bottom )
	{
		UNDERWATER_bottompos[0] = UNDERWATER_bottom_bottom->get_rel_position();
		UNDERWATER_bottompos[1] = UNDERWATER_bottom_bottom->get_rel_position();
	}
	UNDERWATER_bottomspeed[0] = vector3d(0,0,0);
	UNDERWATER_bottomspeed[1] = vector3d(0,0,0);

}



void UNDERWATER_CameraReset( void )
{
	check_youre_under_where=true;
}

static bool underwater_camera_check_use_hints =0;


static bool UNDERWTR_CheckForTubeRideHack( const int playerIdx )
{
	// this is a routine that checks whether the underwater flag should be
	// overridden to false
	if ( !underwater_camera_check_use_hints  )
	{
		if ( g_world_ptr->get_ks_controller(playerIdx)->isDry() ) return true;
	}
	return false;
}



static void UNDERWATER_SetCameraUnderwater(const int playerIdx)
{
	#ifdef BUILD_BOOTABLE
	underwater_camera_check_use_hints= g_game_ptr->is_splitscreen();
	#else
	underwater_camera_check_use_hints=g_game_ptr->is_user_cam_on;
	#endif

	vector3d position_in;
	vector3d position_out;

	position_in.x = WAVE_camx;
	position_in.y = WAVE_camy+Wave_camerafudgefactory;
	position_in.z = WAVE_camz;

	if (WAVE_camx < WAVE_MeshMinX || WAVE_camx > WAVE_MeshMaxX ||
		WAVE_camz < WAVE_MeshMinZ || WAVE_camz > WAVE_MeshMaxZ)
	{
		check_youre_under_where=true;
		position_out.x = position_in.x;
		position_out.y = 0;
		position_out.z = position_in.z;
	}
	else
	{
		WaveQueryFlags flags = (WaveQueryFlags) 0; //WAVE_YGIVEN;
		if ( underwater_camera_check_use_hints  )
		{
			if ( check_youre_under_where )
			{
				check_youre_under_where=false;
				flags = (WaveQueryFlags) WAVE_HINTSOUGHT;
			}
			else
			{
				flags = (WaveQueryFlags) (WAVE_HINTGIVEN|WAVE_HINTSOUGHT);
			}
		}
		else
		{
			flags = (WaveQueryFlags) 0;
		}
		//vector3d *position_out = [will be set to some wave position with the same x and z as the camera position];
		vector3d normout;
		normout.x=0.0f;
		normout.y=0.0f;
		normout.z=0.0f;
		vector3d *normal = &normout;
		vector3d *current = NULL;
		WaveRegionEnum *region = NULL;
		const WavePositionHint &hintin = underwaterhint[playerIdx]; //NULLREF(WavePositionHint);
		WavePositionHint *hintout = &underwaterhint[playerIdx];
		const WaveTolerance &tolerance = WaveTolerance( 2.f, 7.f ); //NULLREF(WaveTolerance );

#if defined(TARGET_XBOX)
		WaveNearestArgs wna(flags,position_in,&position_out,normal,current,region,hintin,hintout,tolerance);
#else
		WaveNearestArgs wna={flags,position_in,&position_out,normal,current,region,hintin,hintout,tolerance};
#endif /* TARGET_XBOX JIV DEBUG */
 		bool ok=WAVE_FindNearest(wna);
		if ( !ok || normout.y < 0.0f && underwater_camera_check_use_hints )
		{
			check_youre_under_where=true;
			#if defined(TARGET_XBOX)
				WaveNearestArgs wna2(flags,position_in,&position_out,normal,current,region,hintin,hintout,tolerance);
			#else
				WaveNearestArgs wna2={flags,position_in,&position_out,normal,current,region,hintin,hintout,tolerance};
			#endif /* TARGET_XBOX JIV DEBUG */
			flags = (WaveQueryFlags) WAVE_HINTSOUGHT;
 			WAVE_FindNearest(wna2);
		}
	}

	//To test whether the camera is underwater, check whether:
	Wave_cameraoverwater[playerIdx] = position_in.y - position_out.y;
	Wave_cameraunderwater[playerIdx] = (Wave_cameraoverwater[playerIdx] < Wave_waterlevel ); //(position_in.y < position_out.y);

	if ( UNDERWTR_CheckForTubeRideHack(playerIdx) )
	{
		Wave_cameraunderwater[playerIdx] = false;
	}
}


bool UNDERWATER_CameraUnderwater( const int playerIdx )
{
	if ( !FEDone() )
	{
		return false;
	}

	return Wave_cameraunderwater[playerIdx];
}

float UNDERWATER_CameraOverWaterDist( const int playerIdx )
{
	return Wave_cameraoverwater[playerIdx];
}


float UNDERWATER_overbottomrad=1.5f;
float UNDERWATER_underbottomrad=0.5f;

nglVector UNDERWATER_SortCenterHi(0, 10.0, 0, 0);
nglVector UNDERWATER_SortCenterLo(0, -10.0, 0, 0);

float UNDERWATER_forcesort_over=0.0f;
float UNDERWATER_forcesort_under=0.0f;

int UNDERWATER_czbias=-10;
int UNDERWATER_bzbias=-50000;

struct WallDebugStruct
{
	u_int WForce : 1;
	u_int CZForce : 1;
	u_int CZSort  : 1;
	u_int CShift  : 1;
	u_int CHide   : 1;
	u_int CHideAlways   : 1;
	u_int CZBias  : 1;
	u_int UnderZForce : 1;
	u_int UnderZSort  : 1;
	u_int UnderShift  : 1;
	u_int UnderHide   : 1;
	u_int BZBias  : 1;
	u_int BubHide   : 1;
	u_int BubHideAlways   : 1;
	u_int ShaftHide   : 1;
	u_int FishHide   : 1;       // but fish have scales, not hide!
	u_int ScrollBottomX : 1;
	u_int ScrollBottomZ : 1;
	u_int TweakBottomTex   : 1;

}
WallDebug =
{
	0, // WForce
	0, // CZForce
	0, // CZSort
	0, // CShift
	1, // CHide
	1, // CHideAlways
	0, // CZBias
	0, // UnderZForce
	0, // UnderZSort
	1, // UnderShift
	0, // UnderHide
	1, // BZBias
	1, // BubHide
	0, // BubHideAlways
	1, // ShaftHide
	1, // FishHide
	1, // ScrollBottomX
	1, // ScrollBottomZ
	0, // TweakBottomTex   : 1;
};

nglTexture *WAVETEX_GetShadowTex( void );

void UNDERWATER_CameraSelect(const int playerIdx)
{
	#if 1
	camera *cam=app::inst()->get_game()->get_player_camera(playerIdx);
	#else
	camera *cam=app::inst()->get_game()->get_current_view_camera()->get_rel_position()[0];
	#endif

	WAVE_camx=cam->get_rel_position()[0];
	WAVE_camy=cam->get_rel_position()[1];
	WAVE_camz=cam->get_rel_position()[2];
}



void UNDERWATER_CameraChecks(const int playerIdx)
{
	#if 1
	camera *cam=app::inst()->get_game()->get_player_camera(playerIdx);
	#else
	camera *cam=app::inst()->get_game()->get_current_view_camera()->get_rel_position()[0];
	#endif

	WAVE_camx=cam->get_rel_position()[0];
	WAVE_camy=cam->get_rel_position()[1];
	WAVE_camz=cam->get_rel_position()[2];
	UNDERWATER_SetCameraUnderwater(playerIdx);

	WAVETEX_SetCameraPos( /*
WAVE_camx, WAVE_camy, WAVE_camz*/
 );

	if ( UNDERWATER_bottom_bottom )
	{
		#ifdef TARGET_GC
		if (!g_game_ptr->is_splitscreen())
		{
		#endif
		if ( WallDebug.TweakBottomTex )
		{
			g_igo_enabled=false;
	 		UNDERWATER_bottom_bottom->set_mesh_texture( WAVETEX_GetShadowTex() );
		}
		UNDERWATER_bottom_bottom->set_rel_position(UNDERWATER_bottompos[playerIdx]);
		#ifdef TARGET_GC
		}
		#endif
	}

	if ( UNDERWATER_lightshafts )
	{
		if ( WallDebug.ShaftHide )
		{
	 		UNDERWATER_lightshafts->set_visible(UNDERWATER_CameraUnderwater(playerIdx));
		}
	}
	if ( UNDERWATER_fish )
	{
		if ( WallDebug.FishHide )
		{
	 		UNDERWATER_fish->set_visible(UNDERWATER_CameraUnderwater(playerIdx));
		}
	}
	if ( UNDERWATER_bottom_bubble )
	{
		vector3d cpos=cam->get_rel_position();

		UNDERWATER_bottom_bubble->set_rel_position(cpos);

		if ( WallDebug.BubHideAlways )
	 		UNDERWATER_bottom_bubble->set_visible( false );
		else if ( WallDebug.BubHide )
	 		UNDERWATER_bottom_bubble->set_visible(UNDERWATER_CameraUnderwater(playerIdx));
		else
	 		UNDERWATER_bottom_bubble->set_visible( true );
	}

	if ( UNDERWATER_bottom_ceiling )
	{
		if ( WallDebug.CHideAlways )
		 	UNDERWATER_bottom_ceiling->set_visible(false);
		else if ( WallDebug.CHide )
		 	UNDERWATER_bottom_ceiling->set_visible(UNDERWATER_CameraUnderwater(playerIdx));
		else
		 	UNDERWATER_bottom_ceiling->set_visible(true);

		if (UNDERWATER_CameraUnderwater(playerIdx))
		{
//			if ( WallDebug.WForce )
//				WAVETEX_SetMatZSorted( false );
			#ifndef TARGET_GC
			if ( WallDebug.CZForce )
				UNDERWATER_bottom_ceiling->set_mesh_matflagbits( NGLMAT_FORCE_Z_WRITE );
			#endif
			if ( WallDebug.CZSort )
				UNDERWATER_bottom_ceiling->set_mesh_matflagbits( NGLMAT_ALPHA_SORT_FIRST );
			if ( WallDebug.CShift )
				UNDERWATER_bottom_ceiling->set_mesh_distance(UNDERWATER_SortCenterLo,WAVE_SortRadius*UNDERWATER_underbottomrad,UNDERWATER_forcesort_under);
			if ( WallDebug.CZBias )
				UNDERWATER_bottom_ceiling->set_zbias(UNDERWATER_czbias);
		}
		else
		{
//			if ( WallDebug.WForce )
//				WAVETEX_SetMatZSorted( true );
			#ifndef TARGET_GC
			if ( WallDebug.CZForce )
				UNDERWATER_bottom_ceiling->clear_mesh_matflagbits( ~NGLMAT_FORCE_Z_WRITE );
			#endif
			if ( WallDebug.CZSort )
				UNDERWATER_bottom_ceiling->clear_mesh_matflagbits( ~NGLMAT_ALPHA_SORT_FIRST );
			if ( WallDebug.CShift )
				UNDERWATER_bottom_ceiling->set_mesh_distance( UNDERWATER_SortCenterLo,WAVE_SortRadius*UNDERWATER_overbottomrad,UNDERWATER_forcesort_over);
			if ( WallDebug.CZBias )
				UNDERWATER_bottom_ceiling->set_zbias(0);
		}
	}

	if ( UNDERWATER_bottom_wall )
	{
		if ( WallDebug.UnderHide )
		{
			UNDERWATER_bottom_wall->set_visible(UNDERWATER_CameraUnderwater(playerIdx));
		}
		else
		{
			UNDERWATER_bottom_wall->set_visible(true);
		}
		if ( WallDebug.TweakBottomTex )
			UNDERWATER_bottom_wall->set_visible(false);
		if (UNDERWATER_CameraUnderwater(playerIdx))
		{
//			if ( WallDebug.WForce )
//				WAVETEX_SetMatZSorted( false );
			#ifndef TARGET_GC
			if ( WallDebug.UnderZForce )
				UNDERWATER_bottom_wall->set_mesh_matflagbits( NGLMAT_FORCE_Z_WRITE );
			#endif
			if ( WallDebug.UnderZSort )
				UNDERWATER_bottom_wall->set_mesh_matflagbits( NGLMAT_ALPHA_SORT_FIRST );
			if ( WallDebug.UnderShift )
				UNDERWATER_bottom_wall->set_mesh_distance(UNDERWATER_SortCenterLo,WAVE_SortRadius*UNDERWATER_underbottomrad,UNDERWATER_forcesort_under);
			if ( WallDebug.BZBias )
				UNDERWATER_bottom_wall->set_zbias(0);

		}
		else
		{
//			if ( WallDebug.WForce )
//				WAVETEX_SetMatZSorted( true );
			#ifndef TARGET_GC
			if ( WallDebug.UnderZForce )
				UNDERWATER_bottom_wall->clear_mesh_matflagbits( ~NGLMAT_FORCE_Z_WRITE );
			#endif
			if ( WallDebug.UnderZSort )
				UNDERWATER_bottom_wall->clear_mesh_matflagbits( ~NGLMAT_ALPHA_SORT_FIRST );
			if ( WallDebug.UnderShift )
				UNDERWATER_bottom_wall->set_mesh_distance( UNDERWATER_SortCenterLo,WAVE_SortRadius*UNDERWATER_overbottomrad,UNDERWATER_forcesort_over);
			if ( WallDebug.BZBias )
				UNDERWATER_bottom_wall->set_zbias(UNDERWATER_bzbias);
		}
	}
}


float UNDERWATER_oldspeedmult = 0.00f;
float UNDERWATER_newspeedmult = 1.0f - UNDERWATER_oldspeedmult;

static void UNDERWATER_ScrollPlayerBottom( const int playerId )
{
	#ifdef TARGET_GC
	if (g_game_ptr->is_splitscreen()) return;
	#endif
	if ( UNDERWATER_bottom_bottom )
	{
		//vector3d botpos = UNDERWATER_bottom_bottom->get_rel_position();
		vector3d newvel;
	  if ( g_world_ptr->get_ks_controller(playerId)->get_super_state() != SUPER_STATE_WIPEOUT )
		{
			vector3d water_current;
			WAVE_GlobalCurrent(&water_current);

			newvel.x=water_current.x*6.0f;
			newvel.z=water_current.z*6.0f;

		}
		else
		{
			g_world_ptr->get_ks_controller(playerId)->get_owner()->get_velocity(&newvel);
		}
		newvel.y=0.0f;

		vector3d oldvel = UNDERWATER_bottomspeed[playerId];

		UNDERWATER_bottomspeed[playerId] = ( UNDERWATER_oldspeedmult * oldvel ) +
		                   ( UNDERWATER_newspeedmult * newvel );

		if ( WallDebug.ScrollBottomX )
			UNDERWATER_bottompos[playerId].x += TIMER_GetFrameSec() * UNDERWATER_bottomspeed[playerId].x;
		if ( WallDebug.ScrollBottomZ )
			UNDERWATER_bottompos[playerId].z += TIMER_GetFrameSec() * UNDERWATER_bottomspeed[playerId].z;
		if ( UNDERWATER_bottompos[playerId].x>UNDERWATER_BottomMax )
			UNDERWATER_bottompos[playerId].x=UNDERWATER_BottomMin;
		else if ( UNDERWATER_bottompos[playerId].x<UNDERWATER_BottomMin )
			UNDERWATER_bottompos[playerId].x=UNDERWATER_BottomMax;
		if ( UNDERWATER_bottompos[playerId].z>UNDERWATER_BottomMax )
			UNDERWATER_bottompos[playerId].z=UNDERWATER_BottomMin;
		else if ( UNDERWATER_bottompos[playerId].z<UNDERWATER_BottomMin )
			UNDERWATER_bottompos[playerId].z=UNDERWATER_BottomMax;
		//UNDERWATER_bottom_bottom->set_rel_position(UNDERWATER_bottompos[playerId]);
	}
}

void UNDERWATER_ScrollBottom( void )
{
	if ( g_game_ptr->get_num_players() > 1 )
		UNDERWATER_ScrollPlayerBottom(1);
	UNDERWATER_ScrollPlayerBottom(0);
}


void UNDERWATER_EntitiesTrackCamera(void)
{
	//UNDERWATER_CameraChecks(g_game_ptr->get_active_player());
	WAVE_camx=app::inst()->get_game()->get_current_view_camera()->get_rel_position()[0];
	WAVE_camy=app::inst()->get_game()->get_current_view_camera()->get_rel_position()[1];
	WAVE_camz=app::inst()->get_game()->get_current_view_camera()->get_rel_position()[2];

	if ( UNDERWATER_bottom_bubble )
	{
		vector3d cpos=app::inst()->get_game()->get_current_view_camera()->get_rel_position();
		UNDERWATER_bottom_bubble->set_rel_position(cpos);
	}
}



