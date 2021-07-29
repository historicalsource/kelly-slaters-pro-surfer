#include "global.h"
#include "app.h"
#include "conglom.h"
#include "game.h"
#include "osdevopts.h"
#include "beachdata.h"
#include "profiler.h"
#include "camera.h"
#include "entity_maker.h"
#include "inputmgr.h"
#include "wds.h"
#include "region.h"
#include "terrain.h"
#include "sfxengine.h"
#include "wavedata.h"
#include "particle.h"
#include "SoundScript.h"
#include "wave.h"
#include "wavetex.h"
#include "ksfx.h"
#include "board.h"
#include "entity.h"
#include "kellyslater_controller.h"
#include "timer.h"
/*
#include "refract.h"	// For REFRACT_SprayEvent
*/
#include "trail.h"
#if defined (TARGET_PS2)
#include "ngl_ps2_internal.h"	// For nglMeshFastWriteVertex functions only!!! (dc 12/14/01)
#endif // defined (TARGET_XBOX)
#include "osparticle.h"	// For nglParticleSystem (dc 05/22/02)


#ifdef TARGET_PS2
//#define CRASH_PROF 1
#endif

#ifdef CRASH_PROF
#include <libpc.h>
#endif

extern float WAVETEX_HighlightAlphaOffset;
extern float WAVETEX_CoreHighlightAlphaOffset;


// Defines
#define MAX_CRASH_NODES 2000

#ifdef BUILD_BOOTABLE
#define FXD_PRECOMP
#endif

struct fx_def FXD;
struct fx_splash FX_ParticleD[FX_DEF_TOTAL];

#ifdef FXD_PRECOMP
nglParticleSystem FX_ParticleRD[FX_DEF_TOTAL];
#endif

nglTexture *fx_tex[FX_TEX_TOTAL];

int currentparticle=0;

typedef struct crashnode_t
{
	vector3d pos;
	float mag;
	float size;
	float age;
	u_int type;
	struct crashnode_t* Next;
} crashnode_s;

crashnode_s crashnodes[MAX_CRASH_NODES];
crashnode_s *CrashHead;
crashnode_s *CrashFreeHead;

float Crash_Seeds[WAVE_MESHSTEPMAXX+1];
static float last_spit_start;
static entity* tube_spit_entity;

// not in use atm.
struct loose_t
{
	nglParticleSystem part;
	u_int move : 1;
} LooseParticles[MAX_LOOSE_PARTICLES];

typedef enum
{
	LIP_SNAP_LEFT,
	LIP_SNAP_RIGHT,
	LIP_LAUNCH
} lip_splash_types;

typedef struct
{
	vector3d position;
	vector3d normal;
	WavePositionHint hint;
	float age;
	bool valid;
	int hero_index;
	nglTexture *texture;
	int type;
} lip_splash_t;

#define MAX_LIP_SPLASHES 2
lip_splash_t lip_splashes[MAX_LIP_SPLASHES];
float snap_grow = 2;
float snap_life = 3;
float snap_size = 5;
float snap_move = 0.4f;
float snap_collapse = 3;
float snap_stretch = 3;

// particle systems for the snow
static nglParticleSystem snow_particles[4];
static bool snow_enabled;
static float snow_speed;

// rain stuff
entity *rain_entity;        // Changed from static since I need ot access it from replay - rbroner
static bool rain_enabled;
static float rain_angle;

// underwater stuff
static nglParticleSystem algee_particles[4]; // (I think they look like bubbles but I've been told they are algee)
static entity *bubbles_entity;
static float bubbles_time;

// wipeout splash stuff
nglTexture* wipeout_splash_texture;

typedef struct
{
	bool valid;
	float start_time;
	int hero_index;
} wipeout_splash_t;

#define MAX_WIPEOUT_SPLASHES 2
wipeout_splash_t wipeout_splashes[MAX_WIPEOUT_SPLASHES];

void trails_update( void );
float clamp_inc(float, float, float);
void crash_draw_intervals(void);
void crash_update_intervals(void);

#define SPIT_DURATION 1.54f // set on the max file

#ifdef FXD_PRECOMP
void prepare_part_slow(u_int fxindex, nglParticleSystem *PartPtr, float interp_power, float interp_alpha, vector3d Spos1, vector3d Spos2, float Duration, float age);
#endif

void prepare_part(u_int fxindex, nglParticleSystem *PartPtr, float interp_power, float interp_alpha, vector3d Spos1, vector3d Spos2, float Duration, float age);
void spraypt_draw(struct spraycontrol *SprayControlPts, u_int fxindex, float vary, int scp_index, int web);

void param_translate(float h_angle, float v_angle, float mag, vector3d *output)
{
	output->x = mag * cosf(DEG_TO_RAD(180 + h_angle)) * sinf(DEG_TO_RAD(v_angle));	// fix trig (dc 08/16/01)
	output->z = mag * sinf(DEG_TO_RAD(180 + h_angle)) * sinf(DEG_TO_RAD(v_angle));	// fix trig (dc 08/16/01)
	output->y = mag * cosf(DEG_TO_RAD(v_angle));
}

void ks_fx_init(void)
{
	u_int i;
	int curbeach = g_game_ptr->get_beach_id();
	stringx texture_path = "levels\\" + g_game_ptr->get_beach_name() + "\\" + os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\";
	nglSetTexturePath( (char *)texture_path.c_str() );	// should fix this to be explicit (dc 06/21/00)

	fx_tex[FX_TEX_CRASH] = nglLoadTextureA("fx_crash1");
	fx_tex[FX_TEX_CRASH2] = nglLoadTextureA("fx_crash2");
	fx_tex[FX_TEX_BSPRAY_P] = nglLoadTextureA("fx_bspray_p");
	fx_tex[FX_TEX_BSPRAY_G] = nglLoadTextureA("fx_bspray_g");
	fx_tex[FX_TEX_TRAIL] = nglLoadTextureA("fx_trail");
	fx_tex[FX_TEX_PADDLE] = nglLoadTextureA("fx_bspray_p");
	fx_tex[FX_TEX_FLOATER] = nglLoadTextureA("fx_bspray_p");
	fx_tex[FX_TEX_ALGEE] = nglLoadTextureA("fx_algee");
	fx_tex[FX_TEX_SNAP_SPRAY] = nglLoadTextureA("fx_snapspray");
	fx_tex[FX_TEX_LAUNCH] = nglLoadTextureA("fx_launch");

  if (curbeach == BEACH_ANTARCTICA)
  {
    snow_speed = 0.3f;
    fx_tex[FX_TEX_WEATHER] = nglLoadTextureA ("fx_weather");
    snow_enabled = true;

    for (i = 0; i < 4; i++)
    {
      snow_particles[i].Life = 10;
      snow_particles[i].Rlife = 0;
#ifdef TARGET_XBOX
      snow_particles[i].Num = 250;
#else
      snow_particles[i].Num = 500;
#endif
      snow_particles[i].Dura = 1;
      snow_particles[i].Ctime = 1;
			snow_particles[i].Seed = (g_random_ptr->rand()<<2)%0x7fffffff;

      snow_particles[i].Scol = NGL_RGBA32 (0xff, 0xff, 0xff, 0xc0);
      snow_particles[i].Rcol = NGL_RGBA32 (0x00, 0x00, 0x00, 0x00);
      snow_particles[i].Ecol = NGL_RGBA32 (0xff, 0xff, 0xff, 0xc0);

      snow_particles[i].Ssize = 0.1;
      snow_particles[i].Rsize = 0;
      snow_particles[i].Esize = 0.1;

      snow_particles[i].Spos[0] = -25;
      snow_particles[i].Spos[1] = (i+1)*5;
      snow_particles[i].Spos[2] = -75;
      snow_particles[i].Spos[3] = 1;
      snow_particles[i].Svel[0] = 0;
      snow_particles[i].Svel[1] = 0;
      snow_particles[i].Svel[2] = 0;
      snow_particles[i].Svel[3] = 0;

      snow_particles[i].Rvel1[0] = 0;
      snow_particles[i].Rvel1[1] = -5;
      snow_particles[i].Rvel1[2] = 0;
      snow_particles[i].Rvel1[3] = 0;
      snow_particles[i].Rvel2[0] = 0;
      snow_particles[i].Rvel2[1] = 0;
      snow_particles[i].Rvel2[2] = 0;
      snow_particles[i].Rvel2[3] = 0;
      snow_particles[i].Rvel3[0] = 0;
      snow_particles[i].Rvel3[1] = 0;
      snow_particles[i].Rvel3[2] = 0;
      snow_particles[i].Rvel3[3] = 0;

      snow_particles[i].Force[0] = 0;
      snow_particles[i].Force[1] = 0;
      snow_particles[i].Force[2] = 0;
      snow_particles[i].Force[3] = 0;

      snow_particles[i].Rpos1[0] = 300;
      snow_particles[i].Rpos1[1] = 0;
      snow_particles[i].Rpos1[2] = 0;
      snow_particles[i].Rpos1[3] = 0;
      snow_particles[i].Rpos2[0] = 0;
      snow_particles[i].Rpos2[1] = 0;
      snow_particles[i].Rpos2[2] = 100;
      snow_particles[i].Rpos2[3] = 0;

      snow_particles[i].MaterialFlags = NGLMAT_BILINEAR_FILTER;
      snow_particles[i].BlendMode = NGLBM_BLEND;
      snow_particles[i].Tex = fx_tex[FX_TEX_WEATHER];
      snow_particles[i].MaxSize = 10000;
			snow_particles[i].Aspect = 1;
    }
  }
  else
    snow_enabled = false;

  for (i = 0; i < 4; i++)
  {
    algee_particles[i].Life = 10;
    algee_particles[i].Rlife = 0;
    algee_particles[i].Num = 150;
    algee_particles[i].Dura = 1;
    algee_particles[i].Ctime = 1;
		algee_particles[i].Seed = (g_random_ptr->rand()<<2)%0x7fffffff;

    algee_particles[i].Scol = NGL_RGBA32 (0xff, 0xff, 0xff, 0x80);
    algee_particles[i].Rcol = NGL_RGBA32 (0x00, 0x00, 0x00, 0x00);
    algee_particles[i].Ecol = NGL_RGBA32 (0xff, 0xff, 0xff, 0x80);

    algee_particles[i].Ssize = 0.7;
    algee_particles[i].Rsize = 0;
    algee_particles[i].Esize = 0.1;

    algee_particles[i].Spos[0] = WAVE_MeshMinX;
    algee_particles[i].Spos[1] = 0;
    algee_particles[i].Spos[2] = WAVE_MeshMinZ + i * WAVE_MESHWIDTH / 4;
    algee_particles[i].Spos[3] = 1;
    algee_particles[i].Svel[0] = 0;
    algee_particles[i].Svel[1] = 0;
    algee_particles[i].Svel[2] = 0;
    algee_particles[i].Svel[3] = 0;

    algee_particles[i].Rvel1[0] = 0;
    algee_particles[i].Rvel1[1] = -10;
    algee_particles[i].Rvel1[2] = 0;
    algee_particles[i].Rvel1[3] = 0;
    algee_particles[i].Rvel2[0] = 0;
    algee_particles[i].Rvel2[1] = 0;
    algee_particles[i].Rvel2[2] = 0;
    algee_particles[i].Rvel2[3] = 0;
    algee_particles[i].Rvel3[0] = 0;
    algee_particles[i].Rvel3[1] = 0;
    algee_particles[i].Rvel3[2] = 0;
    algee_particles[i].Rvel3[3] = 0;

    algee_particles[i].Force[0] = 0;
    algee_particles[i].Force[1] = 0;
    algee_particles[i].Force[2] = 0;
    algee_particles[i].Force[3] = 0;

    algee_particles[i].Rpos1[0] = WAVE_MESHWIDTH;
    algee_particles[i].Rpos1[1] = 0;
    algee_particles[i].Rpos1[2] = 0;
    algee_particles[i].Rpos1[3] = 0;
    algee_particles[i].Rpos2[0] = 0;
    algee_particles[i].Rpos2[1] = 0;
    algee_particles[i].Rpos2[2] = WAVE_MESHDEPTH;
    algee_particles[i].Rpos2[3] = 0;

    algee_particles[i].MaterialFlags = NGLMAT_BILINEAR_FILTER;
    algee_particles[i].Tex = fx_tex[FX_TEX_ALGEE];
    algee_particles[i].MaxSize = 10000;
		algee_particles[i].Aspect = 1;
    algee_particles[i].BlendMode = NGLBM_BLEND;
  }

	// Init Continuous Emitters
	for(i=0; i<WAVE_MESHSTEPMAXX; i++)
		Crash_Seeds[i] = (g_random_ptr->rand()<<2)%0x7fffffff;

	// Init Loose Emitters
	for(i=0;i<MAX_LOOSE_PARTICLES;i++)
	{
		LooseParticles[i].part.Seed = (g_random_ptr->rand()<<2)%0x7fffffff;
		LooseParticles[i].part.Tex = fx_tex[FX_TEX_BSPRAY_P];
		LooseParticles[i].move = 0;
	}

	for (i = 0; i < MAX_LIP_SPLASHES; i++)
		lip_splashes[i].valid = 0;

	for (i = 0; i < MAX_WIPEOUT_SPLASHES; i++)
		wipeout_splashes[i].valid = 0;

	texture_path = "levels\\" + g_game_ptr->get_beach_name() + "\\" + os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\";
	nglSetTexturePath ((char *)texture_path.c_str ());
	wipeout_splash_texture = nglLoadTexture ("splash");

	// load tube spit mesh
	nglSetMeshPath ("items\\tubespit\\entities\\");
	texture_path = "levels\\" + g_game_ptr->get_beach_name() + "\\" + os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\";
	nglSetTexturePath ((char *)texture_path.c_str ());

	tube_spit_entity = g_entity_maker->create_entity_or_subclass ("items\\tubespit\\entities\\tubespit_l", entity_id::make_unique_id (), po_identity_matrix, empty_string, entity::ACTIVE, NULL);

	if (tube_spit_entity)
	{
		if (BeachDataArray[curbeach].bdir)
			tube_spit_entity->set_rel_po (po (-1,0,0, 0,1,0, 0,0,1, 0,0,0));

		nglMesh *Mesh = tube_spit_entity->get_mesh ();

		for (u_int i = 0; i < Mesh->NSections; i++)
		{
			nglMeshSection* Section = &Mesh->Sections[i];
			nglMaterial* Material = Section->Material;

			Material->Flags &= ~(NGLMAT_BACKFACE_CULL | NGLMAT_BACKFACE_DEFAULT);

			// if we say something when we enable it, we should say something when we disable it too.
			nglPrintf (".. and disabling backface culling for the tube spit.\n");
		}

		tube_spit_entity->set_visible (false);
	}

	nglSetMeshPath ("items\\bubbles\\entities\\");
	texture_path = "levels\\" + g_game_ptr->get_beach_name() + "\\" + os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\";
	nglSetTexturePath ((char *)texture_path.c_str ());

	bubbles_entity = g_entity_maker->create_entity_or_subclass ("items\\bubbles\\entities\\bubbles", entity_id::make_unique_id (), po_identity_matrix, empty_string, entity::ACTIVE, NULL);

	if (bubbles_entity)
	{
		bubbles_entity->set_visible (false);
		bubbles_time = 0;
	}

	if (curbeach == BEACH_JEFFERSONBAY)
	{
		rain_enabled = true;
		rain_angle = 20;

		nglSetMeshPath ("items\\rain\\entities\\");
		texture_path = "items\\rain\\" + os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\";
		nglSetTexturePath ((char *)texture_path.c_str ());

		rain_entity = g_entity_maker->create_entity_or_subclass ("items\\rain\\entities\\rain_medium", entity_id::make_unique_id (), po_identity_matrix, empty_string, entity::ACTIVE, NULL);
	}
	else if (curbeach == BEACH_CORTESBANK)
	{
		rain_enabled = true;
		rain_angle = 30;

		nglSetMeshPath ("items\\rain\\entities\\");
		texture_path = "items\\rain\\" + os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\";
		nglSetTexturePath ((char *)texture_path.c_str ());

		rain_entity = g_entity_maker->create_entity_or_subclass ("items\\rain\\entities\\rain_heavy", entity_id::make_unique_id (), po_identity_matrix, empty_string, entity::ACTIVE, NULL);
	}
	else
		rain_enabled = false;

	// Legacy pointers
	FXD.Trail = &FX_ParticleD[FX_DEF_TRAIL];
	FXD.Crash1 = &FX_ParticleD[FX_DEF_CRASH1];
	FXD.Lip1 = &FX_ParticleD[FX_DEF_LIP1];
	FXD.Lip2 = &FX_ParticleD[FX_DEF_LIP2];
	FXD.Bspray1 = &FX_ParticleD[FX_DEF_BSPRAY1];
	FXD.Bspray2 = &FX_ParticleD[FX_DEF_BSPRAY2];
	FXD.GenSplash = &FX_ParticleD[FX_DEF_GENSPLASH];
	FXD.TubeSpit = &FX_ParticleD[FX_DEF_TUBESPIT];

	// Loose Board Particles
	FX_ParticleD[FX_DEF_BSPRAY1].ModMax =         0;
	FX_ParticleD[FX_DEF_BSPRAY1].ModAlpha =       0;
	FX_ParticleD[FX_DEF_BSPRAY1].MinSize  =       0;
	FX_ParticleD[FX_DEF_BSPRAY1].ModSize  =       0;
	FX_ParticleD[FX_DEF_BSPRAY1].ModConst  =      0;
	FX_ParticleD[FX_DEF_BSPRAY1].Ssize =          0.1f;
	FX_ParticleD[FX_DEF_BSPRAY1].Rsize =          0.05f;
	FX_ParticleD[FX_DEF_BSPRAY1].Esize =          0.2f;
	FX_ParticleD[FX_DEF_BSPRAY1].Aspect =         1;
	FX_ParticleD[FX_DEF_BSPRAY1].Scol =           0xffffffff;
	FX_ParticleD[FX_DEF_BSPRAY1].Rcol =           0x00000000;
	FX_ParticleD[FX_DEF_BSPRAY1].Ecol =           0x00ffffff;
	FX_ParticleD[FX_DEF_BSPRAY1].Life =           0.5f;
	FX_ParticleD[FX_DEF_BSPRAY1].Rlife =          0;
	FX_ParticleD[FX_DEF_BSPRAY1].Num  =           15;
	FX_ParticleD[FX_DEF_BSPRAY1].Svel_Hangle =    0;
	FX_ParticleD[FX_DEF_BSPRAY1].Svel_Vangle =    0;
	FX_ParticleD[FX_DEF_BSPRAY1].Svel_Mag =       0;
	FX_ParticleD[FX_DEF_BSPRAY1].Rvel1_Hangle =   0;
	FX_ParticleD[FX_DEF_BSPRAY1].Rvel1_Vangle =   0;
	FX_ParticleD[FX_DEF_BSPRAY1].Rvel1_Mag =      0;
	FX_ParticleD[FX_DEF_BSPRAY1].Rvel2_Hangle =   0;
	FX_ParticleD[FX_DEF_BSPRAY1].Rvel2_Vangle =   0;
	FX_ParticleD[FX_DEF_BSPRAY1].Rvel2_Mag =      0;
	FX_ParticleD[FX_DEF_BSPRAY1].Rvel3_Hangle =   0;
	FX_ParticleD[FX_DEF_BSPRAY1].Rvel3_Vangle =   0;
	FX_ParticleD[FX_DEF_BSPRAY1].Rvel3_Mag =      0;
	FX_ParticleD[FX_DEF_BSPRAY1].Force_Hangle =   0;
	FX_ParticleD[FX_DEF_BSPRAY1].Force_Vangle =   0;
	FX_ParticleD[FX_DEF_BSPRAY1].Force_Mag =      0;
	FX_ParticleD[FX_DEF_BSPRAY1].BlendMode =      NGLBM_BLEND;
	FX_ParticleD[FX_DEF_BSPRAY1].MaterialFlags =  NGLMAT_BILINEAR_FILTER;

	// Loose Board Particles
	FX_ParticleD[FX_DEF_BSPRAY2].ModMax =         0;
	FX_ParticleD[FX_DEF_BSPRAY2].ModAlpha =       0;
	FX_ParticleD[FX_DEF_BSPRAY2].MinSize  =       0;
	FX_ParticleD[FX_DEF_BSPRAY2].ModSize  =       0;
	FX_ParticleD[FX_DEF_BSPRAY2].ModConst  =      0;
	FX_ParticleD[FX_DEF_BSPRAY2].Ssize =          0;
	FX_ParticleD[FX_DEF_BSPRAY2].Rsize =          0.5f;
	FX_ParticleD[FX_DEF_BSPRAY2].Esize =          1.0f;
	FX_ParticleD[FX_DEF_BSPRAY2].Aspect =         1;
	FX_ParticleD[FX_DEF_BSPRAY2].Scol =           0xffffffff;
	FX_ParticleD[FX_DEF_BSPRAY2].Rcol =           0x00000000;
	FX_ParticleD[FX_DEF_BSPRAY2].Ecol =           0x409e9e9e;
	FX_ParticleD[FX_DEF_BSPRAY2].Life =           0.5f;
	FX_ParticleD[FX_DEF_BSPRAY2].Rlife =          0;
	FX_ParticleD[FX_DEF_BSPRAY2].Num  =           3;
	FX_ParticleD[FX_DEF_BSPRAY2].Svel_Hangle =    0;
	FX_ParticleD[FX_DEF_BSPRAY2].Svel_Vangle =    0;
	FX_ParticleD[FX_DEF_BSPRAY2].Svel_Mag =       0.0f;
	FX_ParticleD[FX_DEF_BSPRAY2].Rvel1_Hangle =   0;
	FX_ParticleD[FX_DEF_BSPRAY2].Rvel1_Vangle =   0;
	FX_ParticleD[FX_DEF_BSPRAY2].Rvel1_Mag =      0;
	FX_ParticleD[FX_DEF_BSPRAY2].Rvel2_Hangle =   0;
	FX_ParticleD[FX_DEF_BSPRAY2].Rvel2_Vangle =   0;
	FX_ParticleD[FX_DEF_BSPRAY2].Rvel2_Mag =      0;
	FX_ParticleD[FX_DEF_BSPRAY2].Rvel3_Hangle =   0;
	FX_ParticleD[FX_DEF_BSPRAY2].Rvel3_Vangle =   0;
	FX_ParticleD[FX_DEF_BSPRAY2].Rvel3_Mag =      0;
	FX_ParticleD[FX_DEF_BSPRAY2].Force_Hangle  =  0;
	FX_ParticleD[FX_DEF_BSPRAY2].Force_Vangle =   0;
	FX_ParticleD[FX_DEF_BSPRAY2].Force_Mag =      0;
	FX_ParticleD[FX_DEF_BSPRAY2].BlendMode =      NGLBM_BLEND;
	FX_ParticleD[FX_DEF_BSPRAY2].MaterialFlags =  NGLMAT_BILINEAR_FILTER;

	// Loose Trail Particles
	FX_ParticleD[FX_DEF_TRAIL].ModMax =         1;
	FX_ParticleD[FX_DEF_TRAIL].ModAlpha =       0;
	FX_ParticleD[FX_DEF_TRAIL].MinSize  =       0.1f;
	FX_ParticleD[FX_DEF_TRAIL].ModSize  =       0.1f;
	FX_ParticleD[FX_DEF_TRAIL].ModConst  =      0.5f;
	FX_ParticleD[FX_DEF_TRAIL].Ssize =          0.0f;
	FX_ParticleD[FX_DEF_TRAIL].Rsize =          0.1f;
	FX_ParticleD[FX_DEF_TRAIL].Esize =          0.5f;
	FX_ParticleD[FX_DEF_TRAIL].Aspect =         1;
	FX_ParticleD[FX_DEF_TRAIL].Scol =           0xffffffff;
	FX_ParticleD[FX_DEF_TRAIL].Rcol =           0x00000000;
	FX_ParticleD[FX_DEF_TRAIL].Ecol =           0xffffffff;
	FX_ParticleD[FX_DEF_TRAIL].Life =           0.4f;
	FX_ParticleD[FX_DEF_TRAIL].Rlife =          0;
	FX_ParticleD[FX_DEF_TRAIL].Num  =           30;
	FX_ParticleD[FX_DEF_TRAIL].Svel_Hangle =    0;
	FX_ParticleD[FX_DEF_TRAIL].Svel_Vangle =    0;
	FX_ParticleD[FX_DEF_TRAIL].Svel_Mag =       3.33f;
	FX_ParticleD[FX_DEF_TRAIL].Rvel1_Hangle =   0;
	FX_ParticleD[FX_DEF_TRAIL].Rvel1_Vangle =   0;
	FX_ParticleD[FX_DEF_TRAIL].Rvel1_Mag =      0;
	FX_ParticleD[FX_DEF_TRAIL].Rvel2_Hangle =   0;
	FX_ParticleD[FX_DEF_TRAIL].Rvel2_Vangle =   0;
	FX_ParticleD[FX_DEF_TRAIL].Rvel2_Mag =      1.66f;
	FX_ParticleD[FX_DEF_TRAIL].Rvel3_Hangle =   0;
	FX_ParticleD[FX_DEF_TRAIL].Rvel3_Vangle =   0;
	FX_ParticleD[FX_DEF_TRAIL].Rvel3_Mag =      0;
	FX_ParticleD[FX_DEF_TRAIL].Force_Hangle =   0;
	FX_ParticleD[FX_DEF_TRAIL].Force_Vangle =   180;
	FX_ParticleD[FX_DEF_TRAIL].Force_Mag =      0.16f;
	FX_ParticleD[FX_DEF_TRAIL].BlendMode =      NGLBM_BLEND;
	FX_ParticleD[FX_DEF_TRAIL].MaterialFlags =  NGLMAT_BILINEAR_FILTER;

	// Loose Trail Particles
	FX_ParticleD[FX_DEF_GENSPLASH].ModMax =         0;
	FX_ParticleD[FX_DEF_GENSPLASH].ModAlpha =       0;
	FX_ParticleD[FX_DEF_GENSPLASH].MinSize  =       0.001f;
	FX_ParticleD[FX_DEF_GENSPLASH].ModSize  =       0.001f;
	FX_ParticleD[FX_DEF_GENSPLASH].ModConst  =      0.0f;
	FX_ParticleD[FX_DEF_GENSPLASH].Ssize =          0.5f;
	FX_ParticleD[FX_DEF_GENSPLASH].Rsize =          0.25f;
	FX_ParticleD[FX_DEF_GENSPLASH].Esize =          2.0f;
	FX_ParticleD[FX_DEF_GENSPLASH].Aspect =         1;
	FX_ParticleD[FX_DEF_GENSPLASH].Scol =           0xffffffff;
	FX_ParticleD[FX_DEF_GENSPLASH].Rcol =           0x00000000;
	FX_ParticleD[FX_DEF_GENSPLASH].Ecol =           0x60ffffff;
	FX_ParticleD[FX_DEF_GENSPLASH].Life =           0.5f;
	FX_ParticleD[FX_DEF_GENSPLASH].Rlife =          0;
	FX_ParticleD[FX_DEF_GENSPLASH].Num  =           15;
	FX_ParticleD[FX_DEF_GENSPLASH].Svel_Hangle =    0;
	FX_ParticleD[FX_DEF_GENSPLASH].Svel_Vangle =    0;
	FX_ParticleD[FX_DEF_GENSPLASH].Svel_Mag =       1.66f;
	FX_ParticleD[FX_DEF_GENSPLASH].Rvel1_Hangle =   45;
	FX_ParticleD[FX_DEF_GENSPLASH].Rvel1_Vangle =   25;
	FX_ParticleD[FX_DEF_GENSPLASH].Rvel1_Mag =      2.0f;
	FX_ParticleD[FX_DEF_GENSPLASH].Rvel2_Hangle =   270;
	FX_ParticleD[FX_DEF_GENSPLASH].Rvel2_Vangle =   90;
	FX_ParticleD[FX_DEF_GENSPLASH].Rvel2_Mag =      10;
	FX_ParticleD[FX_DEF_GENSPLASH].Rvel3_Hangle =   180;
	FX_ParticleD[FX_DEF_GENSPLASH].Rvel3_Vangle =   90;
	FX_ParticleD[FX_DEF_GENSPLASH].Rvel3_Mag =      10;
	FX_ParticleD[FX_DEF_GENSPLASH].Force_Hangle =   0;
	FX_ParticleD[FX_DEF_GENSPLASH].Force_Vangle =   0;
	FX_ParticleD[FX_DEF_GENSPLASH].Force_Mag =      -1;
	FX_ParticleD[FX_DEF_GENSPLASH].BlendMode =      NGLBM_BLEND;
	FX_ParticleD[FX_DEF_GENSPLASH].MaterialFlags =  NGLMAT_BILINEAR_FILTER;

	FX_ParticleD[FX_DEF_CHOPHOPSPLASH].ModMax =         0;
	FX_ParticleD[FX_DEF_CHOPHOPSPLASH].ModAlpha =       0;
	FX_ParticleD[FX_DEF_CHOPHOPSPLASH].MinSize  =       0.001f;
	FX_ParticleD[FX_DEF_CHOPHOPSPLASH].ModSize  =       0.001f;
	FX_ParticleD[FX_DEF_CHOPHOPSPLASH].ModConst  =      0.0f;
	FX_ParticleD[FX_DEF_CHOPHOPSPLASH].Ssize =          0.5f;
	FX_ParticleD[FX_DEF_CHOPHOPSPLASH].Rsize =          0.25f;
	FX_ParticleD[FX_DEF_CHOPHOPSPLASH].Esize =          1.0f;
	FX_ParticleD[FX_DEF_CHOPHOPSPLASH].Aspect =         1;
	FX_ParticleD[FX_DEF_CHOPHOPSPLASH].Scol =           0xffffffff;
	FX_ParticleD[FX_DEF_CHOPHOPSPLASH].Rcol =           0x00000000;
	FX_ParticleD[FX_DEF_CHOPHOPSPLASH].Ecol =           0x60ffffff;
	FX_ParticleD[FX_DEF_CHOPHOPSPLASH].Life =           0.5f;
	FX_ParticleD[FX_DEF_CHOPHOPSPLASH].Rlife =          0;
	FX_ParticleD[FX_DEF_CHOPHOPSPLASH].Num  =           15;
	FX_ParticleD[FX_DEF_CHOPHOPSPLASH].Svel_Hangle =    0;
	FX_ParticleD[FX_DEF_CHOPHOPSPLASH].Svel_Vangle =    0;
	FX_ParticleD[FX_DEF_CHOPHOPSPLASH].Svel_Mag =       1.66f;
	FX_ParticleD[FX_DEF_CHOPHOPSPLASH].Rvel1_Hangle =   45;
	FX_ParticleD[FX_DEF_CHOPHOPSPLASH].Rvel1_Vangle =   25;
	FX_ParticleD[FX_DEF_CHOPHOPSPLASH].Rvel1_Mag =      2.0f;
	FX_ParticleD[FX_DEF_CHOPHOPSPLASH].Rvel2_Hangle =   270;
	FX_ParticleD[FX_DEF_CHOPHOPSPLASH].Rvel2_Vangle =   90;
	FX_ParticleD[FX_DEF_CHOPHOPSPLASH].Rvel2_Mag =      10;
	FX_ParticleD[FX_DEF_CHOPHOPSPLASH].Rvel3_Hangle =   180;
	FX_ParticleD[FX_DEF_CHOPHOPSPLASH].Rvel3_Vangle =   90;
	FX_ParticleD[FX_DEF_CHOPHOPSPLASH].Rvel3_Mag =      10;
	FX_ParticleD[FX_DEF_CHOPHOPSPLASH].Force_Hangle =   0;
	FX_ParticleD[FX_DEF_CHOPHOPSPLASH].Force_Vangle =   0;
	FX_ParticleD[FX_DEF_CHOPHOPSPLASH].Force_Mag =      -1;
	FX_ParticleD[FX_DEF_CHOPHOPSPLASH].BlendMode =      NGLBM_BLEND;
	FX_ParticleD[FX_DEF_CHOPHOPSPLASH].MaterialFlags =  NGLMAT_BILINEAR_FILTER;

	// Hands hit the water when the surfer is paddling
	FX_ParticleD[FX_DEF_PADDLE].ModMax =         0;
	FX_ParticleD[FX_DEF_PADDLE].ModAlpha =       0;
	FX_ParticleD[FX_DEF_PADDLE].MinSize  =       0.001f;
	FX_ParticleD[FX_DEF_PADDLE].ModSize  =       0.001f;
	FX_ParticleD[FX_DEF_PADDLE].ModConst  =      0.0f;
	FX_ParticleD[FX_DEF_PADDLE].Ssize =          0.3f;
	FX_ParticleD[FX_DEF_PADDLE].Rsize =          0.1f;
	FX_ParticleD[FX_DEF_PADDLE].Esize =          0.7f;
	FX_ParticleD[FX_DEF_PADDLE].Aspect =         1;
	FX_ParticleD[FX_DEF_PADDLE].Scol =           0xffffffff;
	FX_ParticleD[FX_DEF_PADDLE].Rcol =           0x00000000;
	FX_ParticleD[FX_DEF_PADDLE].Ecol =           0x00ffffff;
	FX_ParticleD[FX_DEF_PADDLE].Life =           0.5f;
	FX_ParticleD[FX_DEF_PADDLE].Rlife =          0;
	FX_ParticleD[FX_DEF_PADDLE].Num  =           10;
	FX_ParticleD[FX_DEF_PADDLE].Svel_Hangle =    0;
	FX_ParticleD[FX_DEF_PADDLE].Svel_Vangle =    0;
	FX_ParticleD[FX_DEF_PADDLE].Svel_Mag =       0;
	FX_ParticleD[FX_DEF_PADDLE].Rvel1_Hangle =   0;
	FX_ParticleD[FX_DEF_PADDLE].Rvel1_Vangle =   180;
	FX_ParticleD[FX_DEF_PADDLE].Rvel1_Mag =      2;
	FX_ParticleD[FX_DEF_PADDLE].Rvel2_Hangle =   0;
	FX_ParticleD[FX_DEF_PADDLE].Rvel2_Vangle =   90;
	FX_ParticleD[FX_DEF_PADDLE].Rvel2_Mag =      2;
	FX_ParticleD[FX_DEF_PADDLE].Rvel3_Hangle =   0;
	FX_ParticleD[FX_DEF_PADDLE].Rvel3_Vangle =   0;
	FX_ParticleD[FX_DEF_PADDLE].Rvel3_Mag =      0;
	FX_ParticleD[FX_DEF_PADDLE].Force_Hangle =   0;
	FX_ParticleD[FX_DEF_PADDLE].Force_Vangle =   0;
	FX_ParticleD[FX_DEF_PADDLE].Force_Mag =      0;
	FX_ParticleD[FX_DEF_PADDLE].BlendMode =      NGLBM_BLEND;
	FX_ParticleD[FX_DEF_PADDLE].MaterialFlags =  NGLMAT_BILINEAR_FILTER;

	// Tube Spit
	FX_ParticleD[FX_DEF_TUBESPIT].ModMax =         0;
	FX_ParticleD[FX_DEF_TUBESPIT].ModAlpha =       0;
	FX_ParticleD[FX_DEF_TUBESPIT].MinSize  =       0.001f;
	FX_ParticleD[FX_DEF_TUBESPIT].ModSize  =       0.001f;
	FX_ParticleD[FX_DEF_TUBESPIT].ModConst  =      0.0f;
	FX_ParticleD[FX_DEF_TUBESPIT].Ssize =          0.0f;
	FX_ParticleD[FX_DEF_TUBESPIT].Rsize =          2.5f;
	FX_ParticleD[FX_DEF_TUBESPIT].Esize =          6.0f;
	FX_ParticleD[FX_DEF_TUBESPIT].Aspect =         1;
	FX_ParticleD[FX_DEF_TUBESPIT].Scol =           0x80e0e0e0;
	FX_ParticleD[FX_DEF_TUBESPIT].Rcol =           0x00000000;
	FX_ParticleD[FX_DEF_TUBESPIT].Ecol =           0x00ffffff;
	FX_ParticleD[FX_DEF_TUBESPIT].Life =           1.0f;
	FX_ParticleD[FX_DEF_TUBESPIT].Rlife =          0;
	FX_ParticleD[FX_DEF_TUBESPIT].Num  =           50;
	FX_ParticleD[FX_DEF_TUBESPIT].Svel_Hangle =    0;
	FX_ParticleD[FX_DEF_TUBESPIT].Svel_Vangle =    0;
	FX_ParticleD[FX_DEF_TUBESPIT].Svel_Mag =       10;
	FX_ParticleD[FX_DEF_TUBESPIT].Rvel1_Hangle =   0;
	FX_ParticleD[FX_DEF_TUBESPIT].Rvel1_Vangle =   0;
	FX_ParticleD[FX_DEF_TUBESPIT].Rvel1_Mag =      33.32f;
	FX_ParticleD[FX_DEF_TUBESPIT].Rvel2_Hangle =   0;
	FX_ParticleD[FX_DEF_TUBESPIT].Rvel2_Vangle =   0;
	FX_ParticleD[FX_DEF_TUBESPIT].Rvel2_Mag =      8.33f;
	FX_ParticleD[FX_DEF_TUBESPIT].Rvel3_Hangle =   90.0f;
	FX_ParticleD[FX_DEF_TUBESPIT].Rvel3_Vangle =   90.0f;
	FX_ParticleD[FX_DEF_TUBESPIT].Rvel3_Mag =      3.33f;
	FX_ParticleD[FX_DEF_TUBESPIT].Force_Hangle =   0;
	FX_ParticleD[FX_DEF_TUBESPIT].Force_Vangle =   180;
	FX_ParticleD[FX_DEF_TUBESPIT].Force_Mag =      0;
	FX_ParticleD[FX_DEF_TUBESPIT].BlendMode =      NGLBM_BLEND;
	FX_ParticleD[FX_DEF_TUBESPIT].MaterialFlags =  NGLMAT_BILINEAR_FILTER;

	// Floater
	FX_ParticleD[FX_DEF_FLOATER].ModMax =         0;
	FX_ParticleD[FX_DEF_FLOATER].ModAlpha =       0;
	FX_ParticleD[FX_DEF_FLOATER].MinSize  =       0;
	FX_ParticleD[FX_DEF_FLOATER].ModSize  =       0;
	FX_ParticleD[FX_DEF_FLOATER].ModConst  =      0;
	FX_ParticleD[FX_DEF_FLOATER].Ssize =          0.3f;
	FX_ParticleD[FX_DEF_FLOATER].Rsize =          0.2f;
	FX_ParticleD[FX_DEF_FLOATER].Esize =          1.0f;
	FX_ParticleD[FX_DEF_FLOATER].Aspect =         1;
	FX_ParticleD[FX_DEF_FLOATER].Scol =           0xffffffff;
	FX_ParticleD[FX_DEF_FLOATER].Rcol =           0x00000000;
	FX_ParticleD[FX_DEF_FLOATER].Ecol =           0x40ffffff;
	FX_ParticleD[FX_DEF_FLOATER].Life =           0.2f;
	FX_ParticleD[FX_DEF_FLOATER].Rlife =          0;
	FX_ParticleD[FX_DEF_FLOATER].Num  =           3;
	FX_ParticleD[FX_DEF_FLOATER].Svel_Hangle =    0;
	FX_ParticleD[FX_DEF_FLOATER].Svel_Vangle =    0;
	FX_ParticleD[FX_DEF_FLOATER].Svel_Mag =       0;
	FX_ParticleD[FX_DEF_FLOATER].Rvel1_Hangle =   0;
	FX_ParticleD[FX_DEF_FLOATER].Rvel1_Vangle =   265;
	FX_ParticleD[FX_DEF_FLOATER].Rvel1_Mag =      10;
	FX_ParticleD[FX_DEF_FLOATER].Rvel2_Hangle =   0;
	FX_ParticleD[FX_DEF_FLOATER].Rvel2_Vangle =   85;
	FX_ParticleD[FX_DEF_FLOATER].Rvel2_Mag =      10;
	FX_ParticleD[FX_DEF_FLOATER].Rvel3_Hangle =   0;
	FX_ParticleD[FX_DEF_FLOATER].Rvel3_Vangle =   0;
	FX_ParticleD[FX_DEF_FLOATER].Rvel3_Mag =      0;
	FX_ParticleD[FX_DEF_FLOATER].Force_Hangle =   0;
	FX_ParticleD[FX_DEF_FLOATER].Force_Vangle =   0;
	FX_ParticleD[FX_DEF_FLOATER].Force_Mag =      0;
	FX_ParticleD[FX_DEF_FLOATER].BlendMode =      NGLBM_BLEND;
	FX_ParticleD[FX_DEF_FLOATER].MaterialFlags =  NGLMAT_BILINEAR_FILTER;

	FX_ParticleD[FX_DEF_AIRDROPS].ModMax =         0;
	FX_ParticleD[FX_DEF_AIRDROPS].ModAlpha =       0;
	FX_ParticleD[FX_DEF_AIRDROPS].MinSize  =       0;
	FX_ParticleD[FX_DEF_AIRDROPS].ModSize  =       0;
	FX_ParticleD[FX_DEF_AIRDROPS].ModConst  =      0;
	FX_ParticleD[FX_DEF_AIRDROPS].Ssize =          0;
	FX_ParticleD[FX_DEF_AIRDROPS].Rsize =          0.5f;
	FX_ParticleD[FX_DEF_AIRDROPS].Esize =          1.5;
	FX_ParticleD[FX_DEF_AIRDROPS].Aspect =         1;
	FX_ParticleD[FX_DEF_AIRDROPS].Scol =           0xffffffff;
	FX_ParticleD[FX_DEF_AIRDROPS].Rcol =           0x00000000;
	FX_ParticleD[FX_DEF_AIRDROPS].Ecol =           0x009e9e9e;
	FX_ParticleD[FX_DEF_AIRDROPS].Life =           0.4f;
	FX_ParticleD[FX_DEF_AIRDROPS].Rlife =          0.1f;
	FX_ParticleD[FX_DEF_AIRDROPS].Num  =           3;
	FX_ParticleD[FX_DEF_AIRDROPS].Svel_Hangle =    0;
	FX_ParticleD[FX_DEF_AIRDROPS].Svel_Vangle =    0;
	FX_ParticleD[FX_DEF_AIRDROPS].Svel_Mag =       0;
	FX_ParticleD[FX_DEF_AIRDROPS].Rvel1_Hangle =   0;
	FX_ParticleD[FX_DEF_AIRDROPS].Rvel1_Vangle =   0;
	FX_ParticleD[FX_DEF_AIRDROPS].Rvel1_Mag =      0;
	FX_ParticleD[FX_DEF_AIRDROPS].Rvel2_Hangle =   0;
	FX_ParticleD[FX_DEF_AIRDROPS].Rvel2_Vangle =   0;
	FX_ParticleD[FX_DEF_AIRDROPS].Rvel2_Mag =      0;
	FX_ParticleD[FX_DEF_AIRDROPS].Rvel3_Hangle =   0;
	FX_ParticleD[FX_DEF_AIRDROPS].Rvel3_Vangle =   0;
	FX_ParticleD[FX_DEF_AIRDROPS].Rvel3_Mag =      0;
	FX_ParticleD[FX_DEF_AIRDROPS].Force_Hangle  =  0;
	FX_ParticleD[FX_DEF_AIRDROPS].Force_Vangle =   0;
	FX_ParticleD[FX_DEF_AIRDROPS].Force_Mag =      0;
	FX_ParticleD[FX_DEF_AIRDROPS].BlendMode =      NGLBM_BLEND;
	FX_ParticleD[FX_DEF_AIRDROPS].MaterialFlags =  NGLMAT_BILINEAR_FILTER;

	FX_ParticleD[FX_DEF_BUBBLES].ModMax =         0;
	FX_ParticleD[FX_DEF_BUBBLES].ModAlpha =       0;
	FX_ParticleD[FX_DEF_BUBBLES].MinSize  =       0;
	FX_ParticleD[FX_DEF_BUBBLES].ModSize  =       0;
	FX_ParticleD[FX_DEF_BUBBLES].ModConst  =      0;
	FX_ParticleD[FX_DEF_BUBBLES].Ssize =          1;
	FX_ParticleD[FX_DEF_BUBBLES].Rsize =          0.5f;
	FX_ParticleD[FX_DEF_BUBBLES].Esize =          2.0f;
	FX_ParticleD[FX_DEF_BUBBLES].Aspect =         1;
	FX_ParticleD[FX_DEF_BUBBLES].Scol =           0xffffffff;
	FX_ParticleD[FX_DEF_BUBBLES].Rcol =           0x00000000;
	FX_ParticleD[FX_DEF_BUBBLES].Ecol =           0x009e9e9e;
	FX_ParticleD[FX_DEF_BUBBLES].Life =           0.8f;
	FX_ParticleD[FX_DEF_BUBBLES].Rlife =          0.1f;
	FX_ParticleD[FX_DEF_BUBBLES].Num  =           2;
	FX_ParticleD[FX_DEF_BUBBLES].Svel_Hangle =    0;
	FX_ParticleD[FX_DEF_BUBBLES].Svel_Vangle =    0;
	FX_ParticleD[FX_DEF_BUBBLES].Svel_Mag =       0;
	FX_ParticleD[FX_DEF_BUBBLES].Rvel1_Hangle =   0;
	FX_ParticleD[FX_DEF_BUBBLES].Rvel1_Vangle =   270;
	FX_ParticleD[FX_DEF_BUBBLES].Rvel1_Mag =      3;
	FX_ParticleD[FX_DEF_BUBBLES].Rvel2_Hangle =   90;
	FX_ParticleD[FX_DEF_BUBBLES].Rvel2_Vangle =   270;
	FX_ParticleD[FX_DEF_BUBBLES].Rvel2_Mag =      3;
	FX_ParticleD[FX_DEF_BUBBLES].Rvel3_Hangle =   0;
	FX_ParticleD[FX_DEF_BUBBLES].Rvel3_Vangle =   0;
	FX_ParticleD[FX_DEF_BUBBLES].Rvel3_Mag =      0;
	FX_ParticleD[FX_DEF_BUBBLES].Force_Hangle  =  0;
	FX_ParticleD[FX_DEF_BUBBLES].Force_Vangle =   0;
	FX_ParticleD[FX_DEF_BUBBLES].Force_Mag =      -2;
	FX_ParticleD[FX_DEF_BUBBLES].BlendMode =      NGLBM_BLEND;
	FX_ParticleD[FX_DEF_BUBBLES].MaterialFlags =  NGLMAT_BILINEAR_FILTER;

	FXD.Render.Part_Crash = 1;
	FXD.Render.Geom_Trail = 1;
	FXD.Render.Spray_Trail = 1;
	FXD.Render.Geom_Spray = 1;
	FXD.Render.Part_Loose = 1;
	FXD.Render.Splashes = 1;
	FXD.Render.Weather = 1;
	FXD.Render.Underwater = 1;
	FXD.Trail_Life = 2.0f;
	FXD.Trail_Start_Width_Mod = 0.7f;
	FXD.Trail_Max_Spread = 0.03f;
	FXD.Spray_Max_Spread = 2.0f;
	FXD.Spray_Min_Thresh = 2.0f;
	FXD.Spray_Max_Thresh = 20.0f;
	FXD.Spray_Scale1 = 200000.0f;
	FXD.Spray_Scale2 = 1.3f;
	FXD.Spray_Vary1  = 0.3f;
	FXD.Spray_Vary2  = 0.3f;

#ifdef TARGET_XBOX
  if (curbeach == BEACH_ANTARCTICA)
	{
		if (g_game_ptr->is_splitscreen())
			nglSetMaxParticles (4000);
		else
			nglSetMaxParticles (2000);
	}
	else
	{
		if (g_game_ptr->is_splitscreen())
			nglSetMaxParticles (3000);
		else
			nglSetMaxParticles (1500);
	}

#endif

#ifdef FXD_PRECOMP
	vector3d zero;

	zero.x = 0;
	zero.y = 0;
	zero.z = 0;

	for(i=0;i<FX_DEF_TOTAL;i++)
		prepare_part_slow(i, &FX_ParticleRD[i], 1, 1, zero, zero, 0, 0);
#endif

	for(i=0;i<WAVE_MESHSTEPMAXX;i++)
		WAVE_Emitter[i].type = WAVE_EM_NONE;

	//  ks_fx_reset();	// Should now be handled by ks_fx_OnNewWave.  (dc 01/26/02)

	ks_fx_OnNewWave();

	last_spit_start = 0;

#ifdef TARGET_XBOX
	PARTICLE_Init();
#endif
}

// Do per-wave initialization
void ks_fx_init_wave(void)
{
	int curwave = WAVE_GetIndex();
	int i;

	if (tube_spit_entity)
	{
		if (WaveDataArray[curwave].tube_spit_scale != 1.0f)
			tube_spit_entity->set_render_scale (vector3d (WaveDataArray[curwave].tube_spit_scale, WaveDataArray[curwave].tube_spit_scale, WaveDataArray[curwave].tube_spit_scale));
		last_spit_start = 0;
	}

	FXD.Wave_Dec = WaveDataArray[curwave].wave_dec;
	FXD.Wave_Inc = WaveDataArray[curwave].wave_inc;

	// Continuous Crash Particles
	FX_ParticleD[FX_DEF_CRASH1].ModMax =       WaveDataArray[curwave].p_c1_mm;
	FX_ParticleD[FX_DEF_CRASH1].ModAlpha =     WaveDataArray[curwave].p_c1_ma;
	FX_ParticleD[FX_DEF_CRASH1].MinSize  =     WaveDataArray[curwave].p_c1_mis;
	FX_ParticleD[FX_DEF_CRASH1].ModSize  =     WaveDataArray[curwave].p_c1_ms;
	FX_ParticleD[FX_DEF_CRASH1].ModConst  =    WaveDataArray[curwave].p_c1_mc;
	FX_ParticleD[FX_DEF_CRASH1].Ssize =        WaveDataArray[curwave].p_c1_ss;
	FX_ParticleD[FX_DEF_CRASH1].Rsize =        WaveDataArray[curwave].p_c1_rs;
	FX_ParticleD[FX_DEF_CRASH1].Esize =        WaveDataArray[curwave].p_c1_es;
	FX_ParticleD[FX_DEF_CRASH1].Aspect =       WaveDataArray[curwave].p_c1_ar;
	FX_ParticleD[FX_DEF_CRASH1].Scol =         WaveDataArray[curwave].p_c1_sc;
	FX_ParticleD[FX_DEF_CRASH1].Rcol =         WaveDataArray[curwave].p_c1_rc;
	FX_ParticleD[FX_DEF_CRASH1].Ecol =         WaveDataArray[curwave].p_c1_ec;
	FX_ParticleD[FX_DEF_CRASH1].Life =         WaveDataArray[curwave].p_c1_l;
	FX_ParticleD[FX_DEF_CRASH1].Rlife =        WaveDataArray[curwave].p_c1_rl;
	FX_ParticleD[FX_DEF_CRASH1].Num  =         WaveDataArray[curwave].p_c1_n;
	FX_ParticleD[FX_DEF_CRASH1].Svel_Hangle =  WaveDataArray[curwave].p_c1_sh;
	FX_ParticleD[FX_DEF_CRASH1].Svel_Vangle =  WaveDataArray[curwave].p_c1_sv;
	FX_ParticleD[FX_DEF_CRASH1].Svel_Mag =     WaveDataArray[curwave].p_c1_sm;
	FX_ParticleD[FX_DEF_CRASH1].Rvel1_Hangle = WaveDataArray[curwave].p_c1_r1h;
	FX_ParticleD[FX_DEF_CRASH1].Rvel1_Vangle = WaveDataArray[curwave].p_c1_r1v;
	FX_ParticleD[FX_DEF_CRASH1].Rvel1_Mag =    WaveDataArray[curwave].p_c1_r1m;
	FX_ParticleD[FX_DEF_CRASH1].Rvel2_Hangle = WaveDataArray[curwave].p_c1_r2h;
	FX_ParticleD[FX_DEF_CRASH1].Rvel2_Vangle = WaveDataArray[curwave].p_c1_r2v;
	FX_ParticleD[FX_DEF_CRASH1].Rvel2_Mag =    WaveDataArray[curwave].p_c1_r2m;
	FX_ParticleD[FX_DEF_CRASH1].Rvel3_Hangle = WaveDataArray[curwave].p_c1_r3h;
	FX_ParticleD[FX_DEF_CRASH1].Rvel3_Vangle = WaveDataArray[curwave].p_c1_r3v;
	FX_ParticleD[FX_DEF_CRASH1].Rvel3_Mag =    WaveDataArray[curwave].p_c1_r3m;
	FX_ParticleD[FX_DEF_CRASH1].Force_Hangle = WaveDataArray[curwave].p_c1_fh;
	FX_ParticleD[FX_DEF_CRASH1].Force_Vangle = WaveDataArray[curwave].p_c1_fv;
	FX_ParticleD[FX_DEF_CRASH1].Force_Mag =    WaveDataArray[curwave].p_c1_fm;
	FX_ParticleD[FX_DEF_CRASH1].BlendMode =    NGLBM_BLEND;
	FX_ParticleD[FX_DEF_CRASH1].MaterialFlags =  0;//NGLMAT_BILINEAR_FILTER;

	FX_ParticleD[FX_DEF_LIP1].ModMax =         WaveDataArray[curwave].p_c3_mm;
	FX_ParticleD[FX_DEF_LIP1].ModAlpha =       WaveDataArray[curwave].p_c3_ma;
	FX_ParticleD[FX_DEF_LIP1].MinSize  =       WaveDataArray[curwave].p_c3_mis;
	FX_ParticleD[FX_DEF_LIP1].ModSize  =       WaveDataArray[curwave].p_c3_ms;
	FX_ParticleD[FX_DEF_LIP1].ModConst  =      WaveDataArray[curwave].p_c3_mc;
	FX_ParticleD[FX_DEF_LIP1].Ssize =          WaveDataArray[curwave].p_c3_ss;
	FX_ParticleD[FX_DEF_LIP1].Rsize =          WaveDataArray[curwave].p_c3_rs;
	FX_ParticleD[FX_DEF_LIP1].Esize =          WaveDataArray[curwave].p_c3_es;
	FX_ParticleD[FX_DEF_LIP1].Aspect =         WaveDataArray[curwave].p_c3_ar;
	FX_ParticleD[FX_DEF_LIP1].Scol =           WaveDataArray[curwave].p_c3_sc;
	FX_ParticleD[FX_DEF_LIP1].Rcol =           WaveDataArray[curwave].p_c3_rc;
	FX_ParticleD[FX_DEF_LIP1].Ecol =           WaveDataArray[curwave].p_c3_ec;
	FX_ParticleD[FX_DEF_LIP1].Life =           WaveDataArray[curwave].p_c3_l;
	FX_ParticleD[FX_DEF_LIP1].Rlife =          WaveDataArray[curwave].p_c3_rl;
	FX_ParticleD[FX_DEF_LIP1].Num  =           WaveDataArray[curwave].p_c3_n;
	FX_ParticleD[FX_DEF_LIP1].Svel_Hangle =    WaveDataArray[curwave].p_c3_sh;
	FX_ParticleD[FX_DEF_LIP1].Svel_Vangle =    WaveDataArray[curwave].p_c3_sv;
	FX_ParticleD[FX_DEF_LIP1].Svel_Mag =       WaveDataArray[curwave].p_c3_sm;
	FX_ParticleD[FX_DEF_LIP1].Rvel1_Hangle =   WaveDataArray[curwave].p_c3_r1h;
	FX_ParticleD[FX_DEF_LIP1].Rvel1_Vangle =   WaveDataArray[curwave].p_c3_r1v;
	FX_ParticleD[FX_DEF_LIP1].Rvel1_Mag =      WaveDataArray[curwave].p_c3_r1m;
	FX_ParticleD[FX_DEF_LIP1].Rvel2_Hangle =   WaveDataArray[curwave].p_c3_r2h;
	FX_ParticleD[FX_DEF_LIP1].Rvel2_Vangle =   WaveDataArray[curwave].p_c3_r2v;
	FX_ParticleD[FX_DEF_LIP1].Rvel2_Mag =      WaveDataArray[curwave].p_c3_r2m;
	FX_ParticleD[FX_DEF_LIP1].Rvel3_Hangle =   WaveDataArray[curwave].p_c3_r3h;
	FX_ParticleD[FX_DEF_LIP1].Rvel3_Vangle =   WaveDataArray[curwave].p_c3_r3v;
	FX_ParticleD[FX_DEF_LIP1].Rvel3_Mag =      WaveDataArray[curwave].p_c3_r3m;
	FX_ParticleD[FX_DEF_LIP1].Force_Hangle =   WaveDataArray[curwave].p_c3_fh;
	FX_ParticleD[FX_DEF_LIP1].Force_Vangle =   WaveDataArray[curwave].p_c3_fv;
	FX_ParticleD[FX_DEF_LIP1].Force_Mag =      WaveDataArray[curwave].p_c3_fm;
	FX_ParticleD[FX_DEF_LIP1].BlendMode =      NGLBM_BLEND;
	FX_ParticleD[FX_DEF_LIP1].MaterialFlags =  0;//NGLMAT_BILINEAR_FILTER;

	FX_ParticleD[FX_DEF_LIP2].ModMax =         WaveDataArray[curwave].p_c4_mm;
	FX_ParticleD[FX_DEF_LIP2].ModAlpha =       WaveDataArray[curwave].p_c4_ma;
	FX_ParticleD[FX_DEF_LIP2].MinSize  =       WaveDataArray[curwave].p_c4_mis;
	FX_ParticleD[FX_DEF_LIP2].ModSize  =       WaveDataArray[curwave].p_c4_ms;
	FX_ParticleD[FX_DEF_LIP2].ModConst  =      WaveDataArray[curwave].p_c4_mc;
	FX_ParticleD[FX_DEF_LIP2].Ssize =          WaveDataArray[curwave].p_c4_ss;
	FX_ParticleD[FX_DEF_LIP2].Rsize =          WaveDataArray[curwave].p_c4_rs;
	FX_ParticleD[FX_DEF_LIP2].Esize =          WaveDataArray[curwave].p_c4_es;
	FX_ParticleD[FX_DEF_LIP2].Aspect =         WaveDataArray[curwave].p_c4_ar;
	FX_ParticleD[FX_DEF_LIP2].Scol =           WaveDataArray[curwave].p_c4_sc;
	FX_ParticleD[FX_DEF_LIP2].Rcol =           WaveDataArray[curwave].p_c4_rc;
	FX_ParticleD[FX_DEF_LIP2].Ecol =           WaveDataArray[curwave].p_c4_ec;
	FX_ParticleD[FX_DEF_LIP2].Life =           WaveDataArray[curwave].p_c4_l;
	FX_ParticleD[FX_DEF_LIP2].Rlife =          WaveDataArray[curwave].p_c4_rl;
	FX_ParticleD[FX_DEF_LIP2].Num  =           WaveDataArray[curwave].p_c4_n;
	FX_ParticleD[FX_DEF_LIP2].Svel_Hangle =    WaveDataArray[curwave].p_c4_sh;
	FX_ParticleD[FX_DEF_LIP2].Svel_Vangle =    WaveDataArray[curwave].p_c4_sv;
	FX_ParticleD[FX_DEF_LIP2].Svel_Mag =       WaveDataArray[curwave].p_c4_sm;
	FX_ParticleD[FX_DEF_LIP2].Rvel1_Hangle =   WaveDataArray[curwave].p_c4_r1h;
	FX_ParticleD[FX_DEF_LIP2].Rvel1_Vangle =   WaveDataArray[curwave].p_c4_r1v;
	FX_ParticleD[FX_DEF_LIP2].Rvel1_Mag =      WaveDataArray[curwave].p_c4_r1m;
	FX_ParticleD[FX_DEF_LIP2].Rvel2_Hangle =   WaveDataArray[curwave].p_c4_r2h;
	FX_ParticleD[FX_DEF_LIP2].Rvel2_Vangle =   WaveDataArray[curwave].p_c4_r2v;
	FX_ParticleD[FX_DEF_LIP2].Rvel2_Mag =      WaveDataArray[curwave].p_c4_r2m;
	FX_ParticleD[FX_DEF_LIP2].Rvel3_Hangle =   WaveDataArray[curwave].p_c4_r3h;
	FX_ParticleD[FX_DEF_LIP2].Rvel3_Vangle =   WaveDataArray[curwave].p_c4_r3v;
	FX_ParticleD[FX_DEF_LIP2].Rvel3_Mag =      WaveDataArray[curwave].p_c4_r3m;
	FX_ParticleD[FX_DEF_LIP2].Force_Hangle =   WaveDataArray[curwave].p_c4_fh;
	FX_ParticleD[FX_DEF_LIP2].Force_Vangle =   WaveDataArray[curwave].p_c4_fv;
	FX_ParticleD[FX_DEF_LIP2].Force_Mag =      WaveDataArray[curwave].p_c4_fm;
	FX_ParticleD[FX_DEF_LIP2].BlendMode =      NGLBM_BLEND;
	FX_ParticleD[FX_DEF_LIP2].MaterialFlags =  0;//NGLMAT_BILINEAR_FILTER;

	FX_ParticleD[FX_DEF_WARN].ModMax =         WaveDataArray[curwave].p_c4_mm;
	FX_ParticleD[FX_DEF_WARN].ModAlpha =       WaveDataArray[curwave].p_c4_ma;
	FX_ParticleD[FX_DEF_WARN].MinSize  =       WaveDataArray[curwave].p_c4_mis;
	FX_ParticleD[FX_DEF_WARN].ModSize  =       WaveDataArray[curwave].p_c4_ms;
	FX_ParticleD[FX_DEF_WARN].ModConst  =      WaveDataArray[curwave].p_c4_mc;
	FX_ParticleD[FX_DEF_WARN].Ssize =          WaveDataArray[curwave].p_c4_ss;
	FX_ParticleD[FX_DEF_WARN].Rsize =          WaveDataArray[curwave].p_c4_rs;
	FX_ParticleD[FX_DEF_WARN].Esize =          WaveDataArray[curwave].p_c4_es;
	FX_ParticleD[FX_DEF_WARN].Aspect =         WaveDataArray[curwave].p_c4_ar;
	FX_ParticleD[FX_DEF_WARN].Scol =           WaveDataArray[curwave].p_c4_sc;
	FX_ParticleD[FX_DEF_WARN].Rcol =           WaveDataArray[curwave].p_c4_rc;
	FX_ParticleD[FX_DEF_WARN].Ecol =           WaveDataArray[curwave].p_c4_ec;
	FX_ParticleD[FX_DEF_WARN].Life =           WaveDataArray[curwave].p_c4_l;
	FX_ParticleD[FX_DEF_WARN].Rlife =          WaveDataArray[curwave].p_c4_rl;
	FX_ParticleD[FX_DEF_WARN].Num  =           WaveDataArray[curwave].p_c4_n;
	FX_ParticleD[FX_DEF_WARN].Svel_Hangle =    WaveDataArray[curwave].p_c4_sh;
	FX_ParticleD[FX_DEF_WARN].Svel_Vangle =    WaveDataArray[curwave].p_c4_sv;
	FX_ParticleD[FX_DEF_WARN].Svel_Mag =       WaveDataArray[curwave].p_c4_sm;
	FX_ParticleD[FX_DEF_WARN].Rvel1_Hangle =   WaveDataArray[curwave].p_c4_r1h;
	FX_ParticleD[FX_DEF_WARN].Rvel1_Vangle =   WaveDataArray[curwave].p_c4_r1v;
	FX_ParticleD[FX_DEF_WARN].Rvel1_Mag =      WaveDataArray[curwave].p_c4_r1m;
	FX_ParticleD[FX_DEF_WARN].Rvel2_Hangle =   WaveDataArray[curwave].p_c4_r2h;
	FX_ParticleD[FX_DEF_WARN].Rvel2_Vangle =   WaveDataArray[curwave].p_c4_r2v;
	FX_ParticleD[FX_DEF_WARN].Rvel2_Mag =      WaveDataArray[curwave].p_c4_r2m;
	FX_ParticleD[FX_DEF_WARN].Rvel3_Hangle =   WaveDataArray[curwave].p_c4_r3h;
	FX_ParticleD[FX_DEF_WARN].Rvel3_Vangle =   WaveDataArray[curwave].p_c4_r3v;
	FX_ParticleD[FX_DEF_WARN].Rvel3_Mag =      WaveDataArray[curwave].p_c4_r3m;
	FX_ParticleD[FX_DEF_WARN].Force_Hangle =   WaveDataArray[curwave].p_c4_fh;
	FX_ParticleD[FX_DEF_WARN].Force_Vangle =   WaveDataArray[curwave].p_c4_fv;
	FX_ParticleD[FX_DEF_WARN].Force_Mag =      WaveDataArray[curwave].p_c4_fm;
	FX_ParticleD[FX_DEF_WARN].BlendMode =      NGLBM_BLEND;
	FX_ParticleD[FX_DEF_WARN].MaterialFlags =  0;//NGLMAT_BILINEAR_FILTER;

#ifdef FXD_PRECOMP
	vector3d zero (ZEROVEC);

	prepare_part_slow (FX_DEF_CRASH1, &FX_ParticleRD[FX_DEF_CRASH1], 1, 1, zero, zero, 0, 0);
	prepare_part_slow (FX_DEF_LIP1, &FX_ParticleRD[FX_DEF_LIP1], 1, 1, zero, zero, 0, 0);
	prepare_part_slow (FX_DEF_LIP2, &FX_ParticleRD[FX_DEF_LIP2], 1, 1, zero, zero, 0, 0);
	prepare_part_slow (FX_DEF_WARN, &FX_ParticleRD[FX_DEF_WARN], 1, 1, zero, zero, 0, 0);
#endif

	// err... this goes here, I guess... has to be done after the wave is loaded
	for (i = 0; i < 4; i++)
	{
		algee_particles[i].Rpos1[0] = WAVE_MESHWIDTH;
		algee_particles[i].Rpos1[1] = 0;
		algee_particles[i].Rpos1[2] = 0;
		algee_particles[i].Rpos1[3] = 0;
		algee_particles[i].Rpos2[0] = 0;
		algee_particles[i].Rpos2[1] = 0;
		algee_particles[i].Rpos2[2] = WAVE_MESHDEPTH;
		algee_particles[i].Rpos2[3] = 0;
	}
}

int particles_info;

#if defined(BUILD_DEBUG) && defined(TARGET_PS2)
#include "ngl_ps2_internal.h"

int ks_fx_test_stage = -1;
float ks_fx_test_time[4];

// do some profiling
static void ks_fx_check_test ()
{
#ifdef DEBUG
	if (ks_fx_test_stage > 1)
	{
		// TIMER_GetFrameSec is not updated while the game is paused, hack back to NGL internals and fix this later
		//    ks_fx_test_time[ks_fx_test_stage - 2] = TIMER_GetFrameSec () * 1000;
		ks_fx_test_time[ks_fx_test_stage - 2] = nglPerfInfo.RenderMS;

		if (ks_fx_test_stage > 4)
			ks_fx_test_stage = -1;
	}
#endif

	switch (ks_fx_test_stage)
	{
	case -1:
		break;

	case 0:
		FXD.Render.Spray_Trail = 0;
		FXD.Render.Part_Crash = 0;
		FXD.Render.Geom_Spray = 0;
		ks_fx_test_stage++;
		break;

	case 1:
		FXD.Render.Spray_Trail = 0;
		FXD.Render.Part_Crash = 1;
		FXD.Render.Geom_Spray = 0;
		ks_fx_test_stage++;
		break;

	case 2:
		FXD.Render.Spray_Trail = 1;
		FXD.Render.Part_Crash = 0;
		FXD.Render.Geom_Spray = 0;
		ks_fx_test_stage++;
		break;

	case 3:
		FXD.Render.Spray_Trail = 0;
		FXD.Render.Part_Crash = 0;
		FXD.Render.Geom_Spray = 1;
		ks_fx_test_stage++;
		break;

	case 4:
		FXD.Render.Spray_Trail = 1;
		FXD.Render.Part_Crash = 1;
		FXD.Render.Geom_Spray = 1;
		ks_fx_test_stage++;
		break;

	case 5: // dummy because rendering is async now
		ks_fx_test_stage++;
		break;
	}

	if (particles_info)
	{
		nglQuad q;
		nglInitQuad (&q);
		nglSetQuadRect (&q, 350, 200, 630, 280);
		nglSetQuadColor (&q, NGL_RGBA32 (0, 0, 0, 192));
		nglSetQuadZ (&q, 0.3f);
		nglListAddQuad (&q);

/*	Replaced by new API. (dc 05/30/02)
		KSNGL_SetFont (0);
		KSNGL_SetFontColor (NGL_RGBA32 (0xFF, 0xFF, 0xFF, 0xFF));
		KSNGL_SetFontScale (1.0, 1.0);
		KSNGL_SetFontZ (0.2f);
*/
		nglListAddString (nglSysFont, 360, 210, 0.2f, NGL_RGBA32 (0xFF, 0xFF, 0xFF, 0xFF),
			"None: %.2f ms\nAll: %.2f ms\nCrash: %.2f ms\nSprayP: %.2f ms\nSprayG: %.2f ms\n",
			ks_fx_test_time[0], ks_fx_test_time[3], ks_fx_test_time[1] - ks_fx_test_time[0], ks_fx_test_time[2] - ks_fx_test_time[0], ks_fx_test_time[3] - ks_fx_test_time[0]);
	}
}
#endif

void ks_fx_init_wipeout_stuff ()
{
	((particle_generator*)bubbles_entity)->always_render = true;
	bubbles_time = 2;
}

void ks_fx_update (void)
{
	vector3d water_current;
	float ms;
	u_int i;

	START_PROF_TIMER( proftimer_adv_crash );

	static float crash_update_time = 0;
	crash_update_time += TIMER_GetFrameSec ();

	if (crash_update_time > 1.0f/30.0f)
	{
		crash_update_intervals();

    // When the framerate is less than 30fps, crash_update_time would get bigger and bigger. So if later on the framerate
    //   speeds up (which effectively happens in slo-mo in replay), crash_update_intervals was still getting called every
    //   frame and was crashing in replay in slo-mo. That's why the line below was changed - rbroner (6/23/02)

		//crash_update_time -= 1.0f/30.0f;
		crash_update_time = 0.0f;
	}

	STOP_PROF_TIMER( proftimer_adv_crash );

	START_PROF_TIMER( proftimer_adv_trail );
	trails_update();
	STOP_PROF_TIMER( proftimer_adv_trail );

	START_PROF_TIMER( proftimer_adv_misc );

	float dt = (float)TIMER_GetFrameSec();

	for (i = 0; i < MAX_LIP_SPLASHES; i++)
	{
		if (lip_splashes[i].valid)
		{
			lip_splashes[i].age += dt;

			if (lip_splashes[i].age > snap_life)
			{
				lip_splashes[i].valid = false;
				continue;
			}

			vector3d current;
			CollideCallStruct collide(lip_splashes[i].position, NULL, &current, NULL, &lip_splashes[i].hint);
			WAVE_CheckCollision(collide, true, true, true);
			lip_splashes[i].position = collide.position + current * dt * snap_move; // if we move at full speed it looks bad, I prefer to have the code look ugly
		}
	}

  int heroNum = g_game_ptr->get_active_player();
	if (rain_enabled && (rain_entity != NULL))
	{
		camera *cam = app::inst()->get_game()->get_current_view_camera();

		po rot;
		rot.set_rotate_x (DEG_TO_RAD (rain_angle));
		rain_entity->set_rel_po (rot);

		rain_entity->set_rel_position (cam->get_rel_position() + cam->get_rel_po().non_affine_slow_xform (vector3d (0,1.5,5.5)));

		if (UNDERWATER_CameraUnderwater (heroNum) || !FXD.Render.Weather)
			rain_entity->set_visible (false);
		else
			rain_entity->set_visible (true);
	}

	if (bubbles_entity)
	{
		//vector3d bubbles_position (0, 0, 1);

		if (UNDERWATER_CameraUnderwater (heroNum) && FXD.Render.Underwater && !g_game_ptr->is_splitscreen() /*&& (bubbles_time > 0)*/)
		{
			//camera *cam = app::inst()->get_game()->get_current_view_camera();
			bubbles_entity->set_visible (true);
			bubbles_entity->set_rel_position (((conglomerate*)g_world_ptr->get_hero_ptr(heroNum))->get_member("BIP01 SPINE")->get_abs_position ());
			//bubbles_entity->set_rel_position (cam->get_rel_position() + cam->get_rel_po().non_affine_slow_xform (bubbles_position));

			bubbles_time -= TIMER_GetFrameSec ();
		}
		else
		{
			bubbles_entity->set_visible (false);
			//      ((particle_generator*)bubbles_entity)->always_render = false;
			//      bubbles_time = 0;
		}
	}

	WAVE_GlobalCurrent(&water_current);

	ms = TIMER_GetFrameSec ();

	if (snow_enabled)
	{
		for (i = 0; i < 4; i++)
		{
			snow_particles[i].Spos[1] -= ms / snow_speed;
			if (snow_particles[i].Spos[1] < 0)
			{
				snow_particles[i].Spos[0] = -125;
				snow_particles[i].Spos[1] += 20;
				snow_particles[i].Spos[2] = -75;
			}
			else
			{
				snow_particles[i].Spos[0] += water_current.x * ms;
				snow_particles[i].Spos[2] += water_current.z * ms;
			}
			static float max_snow_size = 2500;
			snow_particles[i].MaxSize = max_snow_size;
		}
	}

	for (i = 0; i < 4; i++)
	{
		if (algee_particles[i].Spos[2] > WAVE_MeshMaxZ)
		{
			algee_particles[i].Spos[0] = WAVE_MeshMinX;
			algee_particles[i].Spos[1] = -5;
			algee_particles[i].Spos[2] = WAVE_MeshMinZ;
		}
		else
		{
			algee_particles[i].Spos[0] += water_current.x * ms;
			algee_particles[i].Spos[1] += ms;
			algee_particles[i].Spos[2] += water_current.z * ms;
		}
	}

	if (!ks_fx_spit_going_on () && tube_spit_entity)
			tube_spit_entity->set_visible (false);

	int curbeach = g_game_ptr->get_beach_id ();

	if (curbeach == BEACH_ANTARCTICA)
	{
		static float aurora_time_on = 25;
		static float aurora_time_off = 35;
		static float aurora_time = aurora_time_on;
		static int aurora_state = 1;

		world_dynamics_system::entity_list::const_iterator ei = g_world_ptr->get_entities().begin();
		world_dynamics_system::entity_list::const_iterator ei_end = g_world_ptr->get_entities().end();
		entity::prepare_for_visiting();
		entity* ent = NULL;

		for ( ; ei!=ei_end; ei++ )
		{
			entity* e2 = *ei;

			if (e2 && !e2->already_visited())
			{
				e2->visit();

				if (!stricmp (e2->get_name ().c_str (), "northernlights"))
				{
					ent = e2;
					break;
				}
			}
		}

		if (ent)
		{
			if (aurora_state == 0)
			{
				// fade in
				if (aurora_time < 0)
				{
					aurora_state = 1;
					aurora_time = aurora_time_on;
					ent->set_render_color (color32_white);
				}
				else
				{
					color32 col = color32_white;
					col.set_alpha (col.get_alpha () * (1 - aurora_time));
					ent->set_render_color (col);
				}
			}
			else if (aurora_state == 1)
			{
				// visible
				if (aurora_time < 0)
				{
					aurora_state = 2;
					aurora_time = 1;
				}
			}
			else if (aurora_state == 2)
			{
				// fade out
				if (aurora_time < 0)
				{
					aurora_state = 3;
					aurora_time = aurora_time_off;
					ent->set_visible (false);
					color32 col = color32_white;
					col.set_alpha (0);
					ent->set_render_color (col);
				}
				else
				{
					color32 col = color32_white;
					col.set_alpha (col.get_alpha () * aurora_time);
					ent->set_render_color (col);
				}
			}
			else if (aurora_state == 3)
			{
				// hidden
				if (aurora_time < 0)
				{
					aurora_state = 0;
					aurora_time = 1;
					ent->set_visible (true);
				}
			}
		}

		aurora_time -= dt;
	}


	STOP_PROF_TIMER( proftimer_adv_misc );
}

#ifdef FXD_PRECOMP
void prepare_part(u_int fxindex, nglParticleSystem *PartPtr, float interp_power, float interp_alpha, vector3d Spos1, vector3d Spos2, float Duration, float age)
{
	PartPtr->Life = FX_ParticleD[fxindex].Life;
	PartPtr->Rlife = FX_ParticleD[fxindex].Rlife;
	PartPtr->Num = (int)FX_ParticleD[fxindex].Num;
	PartPtr->Dura = Duration;
	PartPtr->Ctime = age;
	PartPtr->Scol = NGL_RGBA32 ((FX_ParticleD[fxindex].Scol&0x00ff0000)>>16, (FX_ParticleD[fxindex].Scol&0x0000ff00)>>8, FX_ParticleD[fxindex].Scol&0x000000ff, (u_int)FTOI((FX_ParticleD[fxindex].Scol>>24)*interp_alpha));
	PartPtr->Rcol = NGL_RGBA32 ((FX_ParticleD[fxindex].Rcol&0x00ff0000)>>16, (FX_ParticleD[fxindex].Rcol&0x0000ff00)>>8, FX_ParticleD[fxindex].Rcol&0x000000ff, (u_int)FTOI((FX_ParticleD[fxindex].Rcol>>24)*interp_alpha));
	PartPtr->Ecol = NGL_RGBA32 ((FX_ParticleD[fxindex].Ecol&0x00ff0000)>>16, (FX_ParticleD[fxindex].Ecol&0x0000ff00)>>8, FX_ParticleD[fxindex].Ecol&0x000000ff, (u_int)FTOI((FX_ParticleD[fxindex].Ecol>>24)*interp_alpha));

	PartPtr->Ssize = FX_ParticleD[fxindex].Ssize * interp_power;
	if(PartPtr->Ssize < FX_ParticleD[fxindex].MinSize)
		PartPtr->Ssize = FX_ParticleD[fxindex].MinSize;

	PartPtr->Rsize = FX_ParticleD[fxindex].Rsize * interp_power;

	PartPtr->Esize = FX_ParticleD[fxindex].Esize * interp_power;
	if(PartPtr->Esize < FX_ParticleD[fxindex].MinSize)
		PartPtr->Esize = FX_ParticleD[fxindex].MinSize;

	PartPtr->Spos[0] = Spos1.x;
	PartPtr->Spos[1] = Spos1.y;
	PartPtr->Spos[2] = Spos1.z;
	PartPtr->Spos[3] = 1;

	PartPtr->Svel[0] = FX_ParticleRD[fxindex].Svel[0] * interp_power;
	PartPtr->Svel[1] = FX_ParticleRD[fxindex].Svel[1] * interp_power;
	PartPtr->Svel[2] = FX_ParticleRD[fxindex].Svel[2] * interp_power;
	PartPtr->Svel[3] = 0;

	PartPtr->Rvel1[0] = FX_ParticleRD[fxindex].Rvel1[0] * interp_power;
	PartPtr->Rvel1[1] = FX_ParticleRD[fxindex].Rvel1[1] * interp_power;
	PartPtr->Rvel1[2] = FX_ParticleRD[fxindex].Rvel1[2] * interp_power;
	PartPtr->Rvel1[3] = 0;

	PartPtr->Rvel2[0] = FX_ParticleRD[fxindex].Rvel2[0] * interp_power;
	PartPtr->Rvel2[1] = FX_ParticleRD[fxindex].Rvel2[1] * interp_power;
	PartPtr->Rvel2[2] = FX_ParticleRD[fxindex].Rvel2[2] * interp_power;
	PartPtr->Rvel2[3] = 0;

	PartPtr->Rvel3[0] = FX_ParticleRD[fxindex].Rvel3[0] * interp_power;
	PartPtr->Rvel3[1] = FX_ParticleRD[fxindex].Rvel3[1] * interp_power;
	PartPtr->Rvel3[2] = FX_ParticleRD[fxindex].Rvel3[2] * interp_power;
	PartPtr->Rvel3[3] = 0;

	PartPtr->Force[0] = FX_ParticleRD[fxindex].Force[0];
	PartPtr->Force[1] = FX_ParticleRD[fxindex].Force[1];
	PartPtr->Force[2] = FX_ParticleRD[fxindex].Force[2];
	PartPtr->Force[3] = 0;

	PartPtr->Rpos1[0] = Spos2.x - Spos1.x;
	PartPtr->Rpos1[1] = Spos2.y - Spos1.y;
	PartPtr->Rpos1[2] = Spos2.z - Spos1.z;
	PartPtr->Rpos1[3] = 0;
	PartPtr->Rpos2[0] = 0;
	PartPtr->Rpos2[1] = 0;
	PartPtr->Rpos2[2] = 0;
	PartPtr->Rpos2[3] = 0;

	PartPtr->Aspect = FX_ParticleRD[fxindex].Aspect;
	PartPtr->MaterialFlags = FX_ParticleRD[fxindex].MaterialFlags;
	PartPtr->BlendMode = FX_ParticleRD[fxindex].BlendMode;
}
#endif

#ifdef FXD_PRECOMP
void prepare_part_slow(u_int fxindex, nglParticleSystem *PartPtr, float interp_power, float interp_alpha, vector3d Spos1, vector3d Spos2, float Duration, float age)
#else
void prepare_part(u_int fxindex, nglParticleSystem *PartPtr, float interp_power, float interp_alpha, vector3d Spos1, vector3d Spos2, float Duration, float age)
#endif
{
	vector3d tmp;

	PartPtr->Life = FX_ParticleD[fxindex].Life;
	PartPtr->Rlife = FX_ParticleD[fxindex].Rlife;
	PartPtr->Num = (int)FX_ParticleD[fxindex].Num;
	PartPtr->Dura = Duration;
	PartPtr->Ctime = age;
	PartPtr->Scol = NGL_RGBA32 ((FX_ParticleD[fxindex].Scol&0x00ff0000)>>16, (FX_ParticleD[fxindex].Scol&0x0000ff00)>>8, FX_ParticleD[fxindex].Scol&0x000000ff, (u_int)FTOI((FX_ParticleD[fxindex].Scol>>24)*interp_alpha));
	PartPtr->Rcol = NGL_RGBA32 ((FX_ParticleD[fxindex].Rcol&0x00ff0000)>>16, (FX_ParticleD[fxindex].Rcol&0x0000ff00)>>8, FX_ParticleD[fxindex].Rcol&0x000000ff, (u_int)FTOI((FX_ParticleD[fxindex].Rcol>>24)*interp_alpha));
	PartPtr->Ecol = NGL_RGBA32 ((FX_ParticleD[fxindex].Ecol&0x00ff0000)>>16, (FX_ParticleD[fxindex].Ecol&0x0000ff00)>>8, FX_ParticleD[fxindex].Ecol&0x000000ff, (u_int)FTOI((FX_ParticleD[fxindex].Ecol>>24)*interp_alpha));

	PartPtr->Ssize = FX_ParticleD[fxindex].Ssize * interp_power;
	if(PartPtr->Ssize < FX_ParticleD[fxindex].MinSize)
		PartPtr->Ssize = FX_ParticleD[fxindex].MinSize;

	PartPtr->Rsize = FX_ParticleD[fxindex].Rsize * interp_power;

	PartPtr->Esize = FX_ParticleD[fxindex].Esize * interp_power;
	if (PartPtr->Esize < FX_ParticleD[fxindex].MinSize)
		PartPtr->Esize = FX_ParticleD[fxindex].MinSize;

	PartPtr->Spos[0] = Spos1.x;
	PartPtr->Spos[1] = Spos1.y;
	PartPtr->Spos[2] = Spos1.z;
	PartPtr->Spos[3] = 1;

	param_translate(FX_ParticleD[fxindex].Svel_Hangle, FX_ParticleD[fxindex].Svel_Vangle, (interp_power * FX_ParticleD[fxindex].Svel_Mag), &tmp);
	PartPtr->Svel[0] = tmp.x;
	PartPtr->Svel[1] = tmp.y;
	PartPtr->Svel[2] = tmp.z;
	PartPtr->Svel[3] = 0;

	param_translate(FX_ParticleD[fxindex].Rvel1_Hangle, FX_ParticleD[fxindex].Rvel1_Vangle, (interp_power * FX_ParticleD[fxindex].Rvel1_Mag), &tmp);
	PartPtr->Rvel1[0] = tmp.x;
	PartPtr->Rvel1[1] = tmp.y;
	PartPtr->Rvel1[2] = tmp.z;
	PartPtr->Rvel1[3] = 0;

	param_translate(FX_ParticleD[fxindex].Rvel2_Hangle, FX_ParticleD[fxindex].Rvel2_Vangle, (interp_power * FX_ParticleD[fxindex].Rvel2_Mag), &tmp);
	PartPtr->Rvel2[0] = tmp.x;
	PartPtr->Rvel2[1] = tmp.y;
	PartPtr->Rvel2[2] = tmp.z;
	PartPtr->Rvel2[3] = 0;

	param_translate(FX_ParticleD[fxindex].Rvel3_Hangle, FX_ParticleD[fxindex].Rvel3_Vangle, (interp_power * FX_ParticleD[fxindex].Rvel3_Mag), &tmp);
	PartPtr->Rvel3[0] = tmp.x;
	PartPtr->Rvel3[1] = tmp.y;
	PartPtr->Rvel3[2] = tmp.z;
	PartPtr->Rvel3[3] = 0;

	param_translate(FX_ParticleD[fxindex].Force_Hangle, FX_ParticleD[fxindex].Force_Vangle, FX_ParticleD[fxindex].Force_Mag, &tmp);
	PartPtr->Force[0] = tmp.x;
	PartPtr->Force[1] = tmp.y;
	PartPtr->Force[2] = tmp.z;
	PartPtr->Force[3] = 0;

	PartPtr->Rpos1[0] = Spos2.x - Spos1.x;
	PartPtr->Rpos1[1] = Spos2.y - Spos1.y;
	PartPtr->Rpos1[2] = Spos2.z - Spos1.z;
	PartPtr->Rpos1[3] = 0;
	PartPtr->Rpos2[0] = 0;
	PartPtr->Rpos2[1] = 0;
	PartPtr->Rpos2[2] = 0;
	PartPtr->Rpos2[3] = 0;

	PartPtr->Aspect = FX_ParticleD[fxindex].Aspect;
	PartPtr->MaterialFlags = FX_ParticleD[fxindex].MaterialFlags;
	PartPtr->BlendMode = FX_ParticleD[fxindex].BlendMode;
}

void ks_fx_draw(const int heroIdx)
{
	u_int i;

#ifdef TARGET_XBOX
	PARTICLE_LockVertexBuffer();	// Must occur before any calls to PARTICLE_ListAdd.  (dc 05/30/02)
#endif

#if defined(BUILD_DEBUG) && defined(TARGET_PS2)
	ks_fx_check_test();
#endif

  if (UNDERWATER_CameraUnderwater(heroIdx))
  {
    if (FXD.Render.Underwater)
      for (i = 0; i < 4; i++)
        nglListAddParticle(&algee_particles[i]);
  }
  else
  {
    if (snow_enabled && FXD.Render.Weather)
    {
			// Don't display the snow on the flyby since it's only around the wave
			if (g_world_ptr->get_ks_controller(g_game_ptr->get_active_player())->get_super_state() != SUPER_STATE_FLYBY)
			{
	      for (i = 0; i < 4; i++)
		      nglListAddParticle(&snow_particles[i]);
			}
    }
  }

  START_PROF_TIMER( proftimer_render_crash );
  crash_draw_intervals(); 	// Draw Crash Particles
  STOP_PROF_TIMER( proftimer_render_crash );

  START_PROF_TIMER( proftimer_render_loose );
  // Draw Loose Particles
  for(i=0;i<MAX_LOOSE_PARTICLES;i++)
  {
    LooseParticles[i].part.Ctime += (float)TIMER_GetFrameSec();

    float maxtime = (float)(LooseParticles[i].part.Dura + LooseParticles[i].part.Life + LooseParticles[i].part.Rlife);

    if (FXD.Render.Part_Loose)
    {
      // Check if we should draw
      if(LooseParticles[i].part.Ctime < maxtime)
      {
        if (LooseParticles[i].move)
        {
          vector3d water_current;

          WAVE_GlobalCurrent(&water_current);
          LooseParticles[i].part.Spos[0] += water_current.x * (float)TIMER_GetFrameSec();
          LooseParticles[i].part.Spos[1] += water_current.y * (float)TIMER_GetFrameSec();
          LooseParticles[i].part.Spos[2] += water_current.z * (float)TIMER_GetFrameSec();
        }

				// shouldn't be here
        LooseParticles[i].part.MaxSize = 75000;

        nglListAddParticle( &LooseParticles[i].part );
      }
    }
  }
  STOP_PROF_TIMER( proftimer_render_loose );

  ks_fx_trail_draw(heroIdx);

	if (FXD.Render.Splashes)
	{
		for (i = 0; i < MAX_LIP_SPLASHES; i++)
		{
			if (lip_splashes[i].valid)
			{
				nglMaterial material;
				u_int i_color;
				float radius;

				// only render the current player on split screen mode
				if (lip_splashes[i].hero_index != heroIdx)
					continue;

				// QA would love to send me a bug about a black rectangle on the screen
				if (lip_splashes[i].texture == NULL)
					continue;

				memset (&material, 0, sizeof (material));
				material.Map = lip_splashes[i].texture;
				material.MapBlendMode = NGLBM_BLEND;
				material.Flags = NGLMAT_TEXTURE_MAP | NGLMAT_ALPHA | NGLMAT_BILINEAR_FILTER;
				nglMatrix local_to_world;
				nglIdentityMatrix(local_to_world);

				KSNGL_CreateScratchMesh (4, &material, false);
				nglMeshWriteStrip (4);

				vector3d center = lip_splashes[i].position;

				float alpha = FTOI(255.0f * (1.0f - 0.5f * min (lip_splashes[i].age, snap_grow) / snap_grow));

				if (lip_splashes[i].age > snap_grow)
					alpha *= 1 - (lip_splashes[i].age - snap_grow) / (snap_life - snap_grow);

				i_color = NGL_RGBA32 (0xff, 0xff, 0xff, alpha);
				radius = lip_splashes[i].age * snap_size;

				po rot;
				vector3d p2;
				vector3d v = cross (ZVEC, lip_splashes[i].normal);
				v.normalize ();
				static float snap_angle = 0.5f;
				rot.set_rot (v, snap_angle * acos (dot (ZVEC, lip_splashes[i].normal)));
				rot.set_position (center);

				static float snap_start_x = 1, snap_end_x = 8;
				static float snap_start_y = 1, snap_end_y = 8;
				float dx = snap_start_x + (snap_end_x - snap_start_x) * lip_splashes[i].age / snap_grow;
				float dy = snap_start_y + (snap_end_y - snap_start_y) * min (lip_splashes[i].age, snap_grow) / snap_grow;

				if (lip_splashes[i].age > snap_grow)
					dy *= 1 - (lip_splashes[i].age - snap_grow) / (snap_life - snap_grow);

				static float skew = 1;
				static float under = -.25;
				static float f = .5;

				nglRenderParams Params;
				memset (&Params, 0, sizeof (Params));
				Params.Flags |= NGLP_ZBIAS;
				Params.ZBias = 10000.0f;

				switch (lip_splashes[i].type)
				{
				case LIP_SNAP_LEFT:
					p2 = rot.slow_xform (vector3d (-dx * f, under, 0));
					nglMeshFastWriteVertexPCUV (p2.x,  p2.y,  p2.z, i_color, 0, 0);
					p2 = rot.slow_xform (vector3d (dx * (1 - f) + snap_start_x, under, 0));
					nglMeshFastWriteVertexPCUV (p2.x,  p2.y,  p2.z, i_color, 1, 0);
					p2 = rot.slow_xform (vector3d (-dx * f - skew, dy, 0));
					nglMeshFastWriteVertexPCUV (p2.x,  p2.y,  p2.z, i_color, 0, -1);
					p2 = rot.slow_xform (vector3d (dx * (1 - f) + snap_start_x - skew, dy, 0));
					nglMeshFastWriteVertexPCUV (p2.x,  p2.y,  p2.z, i_color, 1, -1);
					break;

				case LIP_SNAP_RIGHT:
					p2 = rot.slow_xform (vector3d (dx * (1 - f) + snap_start_x, under, 0));
					nglMeshFastWriteVertexPCUV (p2.x,  p2.y,  p2.z, i_color, 0, 0);
					p2 = rot.slow_xform (vector3d (-dx * f, under, 0));
					nglMeshFastWriteVertexPCUV (p2.x,  p2.y,  p2.z, i_color, 1, 0);
					p2 = rot.slow_xform (vector3d (dx * (1 - f) + snap_start_x + skew, dy, 0));
					nglMeshFastWriteVertexPCUV (p2.x,  p2.y,  p2.z, i_color, 0, -1);
					p2 = rot.slow_xform (vector3d (-dx *  f + skew, dy, 0));
					nglMeshFastWriteVertexPCUV (p2.x,  p2.y,  p2.z, i_color, 1, -1);
					break;

				case LIP_LAUNCH:
					p2 = rot.slow_xform (vector3d (-dx * f, under, 0));
					nglMeshFastWriteVertexPCUV (p2.x,  p2.y,  p2.z, i_color, 0, 0);
					p2 = rot.slow_xform (vector3d (dx * (1 - f) + snap_start_x, under, 0));
					nglMeshFastWriteVertexPCUV (p2.x,  p2.y,  p2.z, i_color, 1, 0);
					p2 = rot.slow_xform (vector3d (-dx * f, dy, 0));
					nglMeshFastWriteVertexPCUV (p2.x,  p2.y,  p2.z, i_color, 0, -1);
					p2 = rot.slow_xform (vector3d (dx * (1 - f) + snap_start_x, dy, 0));
					nglMeshFastWriteVertexPCUV (p2.x,  p2.y,  p2.z, i_color, 1, -1);
					break;

				default:
					warning ("Unknown lip splash type\n");
					break;
				}

	// Material must be specified at mesh creation time now.  (dc 06/03/02)
//				KSNGL_ScratchSetMaterial (&material);
#ifdef TARGET_XBOX
				nglMeshSetSphere(nglVector(0,0,0,0), 0);	// replace by nglMeshCalcSphere, when that's implemented (dc 06/11/02)
#else
				nglMeshCalcSphere ();
#endif
				nglListAddMesh( nglCloseMesh(), local_to_world, &Params);
			}
		}

		for (i = 0; i < MAX_WIPEOUT_SPLASHES; i++)
		{
			if (wipeout_splashes[i].valid && heroIdx == wipeout_splashes[i].hero_index)
			{
				u_int frame = (int)((TIMER_GetTotalSec() - wipeout_splashes[i].start_time) * 30.0f);

				if ((wipeout_splash_texture == NULL) || (wipeout_splash_texture->NFrames <= frame))
					wipeout_splashes[i].valid = false;
				else
				{
					nglMaterial Mat;
					memset (&Mat, 0, sizeof Mat);
					Mat.Map = wipeout_splash_texture;
					Mat.DetailMap = NULL;
					Mat.LightMap = NULL;
					Mat.MapBlendMode = NGLBM_BLEND;
					Mat.Flags = NGLMAT_TEXTURE_MAP | NGLMAT_ALPHA | NGLMAT_BILINEAR_FILTER;

					nglMatrix LocalToWorld;
					nglIdentityMatrix (LocalToWorld);

					nglRenderParams Params;
					memset (&Params, 0, sizeof (Params));
					Params.Flags |= NGLP_TEXTURE_FRAME;
					Params.TextureFrame = frame;
					Params.Flags |= NGLP_ZBIAS;
					Params.ZBias = 10000.0f;

					KSNGL_CreateScratchMesh (4, &Mat, false);
					nglMeshWriteStrip (4);

					u_int color = NGL_RGBA32 (0xff, 0xff, 0xff, 0xff);

					static float splash_size = 3;
					static float splash_dy = -1;
					vector3d points[4] = { vector3d (-splash_size, 0, 0), vector3d (splash_size, 0, 0), vector3d (-splash_size, splash_size, 0), vector3d (splash_size, splash_size, 0) };

					po cam = app::inst()->get_game()->get_current_view_camera()->get_rel_po ();
					cam.set_position (ZEROVEC);
					vector3d wipeout_pos = g_world_ptr->get_hero_ptr(heroIdx)->get_abs_position ();

					for (int j = 0; j < 4; j++)
						points[j] = wipeout_pos + cam.non_affine_slow_xform (points[j] + vector3d (0, splash_dy, 0));

					nglMeshFastWriteVertexPCUV (points[0].x,  points[0].y, points[0].z, color, 0, 0);
					nglMeshFastWriteVertexPCUV (points[1].x,  points[1].y, points[1].z, color, 1, 0);
					nglMeshFastWriteVertexPCUV (points[2].x,  points[2].y, points[2].z, color, 0, -1);
					nglMeshFastWriteVertexPCUV (points[3].x,  points[3].y, points[3].z, color, 1, -1);

					// Material must be specified at mesh creation time now.  (dc 06/03/02)
//						KSNGL_ScratchSetMaterial (&Mat);
#ifdef TARGET_XBOX
					nglMeshSetSphere(nglVector(0,0,0,0), 0);	// replace by nglMeshCalcSphere, when that's implemented (dc 06/11/02)
#else
					nglMeshCalcSphere ();
#endif
					nglListAddMesh( nglCloseMesh(), LocalToWorld, &Params);
				}
			}
		}
	}
}

void ks_fx_end_wipeout_splash (int index)
{
	for (int i = 0; i < MAX_WIPEOUT_SPLASHES; i++)
	{
		if (wipeout_splashes[i].hero_index == index)
			wipeout_splashes[i].valid = false;
	}
}

void ks_fx_start_wipeout_splash (int index)
{
	if (index >= g_game_ptr->get_num_players())
		return;	// no splash for AI surfers, because ks_fx_draw isn't called for them (dc 03/30/02)

	for (int i = 0; i < MAX_WIPEOUT_SPLASHES; i++)
	{
		if (!wipeout_splashes[i].valid)
		{
      if(!ksreplay.IsPlaying())
          ksreplay.SetWipeoutSplash(index);

			wipeout_splashes[i].valid = true;
			wipeout_splashes[i].hero_index = index;
			wipeout_splashes[i].start_time = TIMER_GetTotalSec ();
			break;
		}
	}
}

void ks_fx_create_big_splash(vector3d pos)
{
	ks_fx_add_splash(FX_DEF_GENSPLASH, pos, 1.0f);
}

void ks_fx_create_paddle_splash(vector3d pos)
{
	ks_fx_add_splash(FX_DEF_PADDLE, pos, 1.0f);
	LooseParticles[currentparticle].part.Tex = fx_tex[FX_TEX_PADDLE];
	LooseParticles[currentparticle].move = 1;
}

void ks_fx_create_snap_splash(const vector3d& pos, const vector3d& normal, WavePositionHint& hint, bool left, int hero_index)
{
	if (hero_index >= g_game_ptr->get_num_players())
	{
		return;	// no splash for AI surfers, because ks_fx_draw isn't called for them (dc 03/30/02)
	}

	lip_splash_t *splash = NULL;

	for (int i = 0; i < MAX_LIP_SPLASHES; i++)
	{
		if (!lip_splashes[i].valid)
		{
			splash = &lip_splashes[i];
			break;
		}
	}

	if (splash != NULL)
	{
		splash->age = 0;
		splash->valid = true;
		splash->position = pos;
		splash->hint = hint;
		splash->normal = normal;
		splash->type = left ? LIP_SNAP_LEFT : LIP_SNAP_RIGHT;
		splash->hero_index = hero_index;
		splash->texture = fx_tex[FX_TEX_SNAP_SPRAY];
	}
}

void ks_fx_create_launch_splash (const vector3d& pos, const vector3d& normal, WavePositionHint& hint, int hero_index)
{
	if (hero_index >= g_game_ptr->get_num_players())
		return;	// no splash for AI surfers, because ks_fx_draw isn't called for them.

	lip_splash_t *splash = NULL;

	for (int i = 0; i < MAX_LIP_SPLASHES; i++)
	{
		if (!lip_splashes[i].valid)
		{
			splash = &lip_splashes[i];
			break;
		}
	}

	if (splash != NULL)
	{
		splash->age = 0;
		splash->valid = true;
		splash->position = pos;
		splash->hint = hint;
		splash->normal = normal;
		splash->type = LIP_LAUNCH;
		splash->hero_index = hero_index;
		splash->texture = fx_tex[FX_TEX_LAUNCH];
	}
}

float debug_dura= 1.0f;

bool ks_fx_spit_going_on ()
{
	if (last_spit_start == 0)
		return false;

	if (last_spit_start + SPIT_DURATION < TIMER_GetTotalSec ())
//	if (last_spit_start + FX_ParticleD[FX_DEF_TUBESPIT].Life < TIMER_GetTotalSec ())
		return false;

	return true;
}

bool ks_fx_create_tube_spit (float dt, void** data)
{
	vector3d pos;
	u_int angle;

	if (dt < 0)
	{
		if (tube_spit_entity)
			tube_spit_entity->set_visible(false);

		return false;
	}

	// keep updating until we're done
	if (ks_fx_spit_going_on ())
		return true;

	if (*data == NULL)
	{
		last_spit_start = TIMER_GetTotalSec ();

		if (BeachDataArray[g_game_ptr->get_beach_id()].bdir)
		{
			pos = WAVE_SoundEmitter[WAVE_SE_FACE].segment[WAVE_SoundEmitter[WAVE_SE_FACE].numsegment - 1].stop;
			angle = 0;
		}
		else
		{
			pos = WAVE_SoundEmitter[WAVE_SE_FACE].segment[0].start;
			angle = 180;
		}

		if (tube_spit_entity)
		{
			float TextureScrollFrame = TIMER_GetTotalSec() / 60.f;
			tube_spit_entity->set_rel_position (pos);
			tube_spit_entity->set_visible (true);
			tube_spit_entity->set_texture_scroll (-TextureScrollFrame * -0.65f + 6.5f, -TextureScrollFrame * 0);
		}

		FX_ParticleD[FX_DEF_TUBESPIT].Rvel1_Vangle = 90;
		FX_ParticleD[FX_DEF_TUBESPIT].Rvel1_Hangle = angle;
		FX_ParticleD[FX_DEF_TUBESPIT].Svel_Vangle = 90;
		FX_ParticleD[FX_DEF_TUBESPIT].Svel_Hangle = angle;

		ks_fx_add_splash(FX_DEF_TUBESPIT, pos, 1.0f);

		LooseParticles[currentparticle].part.Dura = debug_dura;
		LooseParticles[currentparticle].part.Tex = fx_tex[FX_TEX_CRASH];

		*data = &last_spit_start;

		SoundScriptManager::inst()->playEvent(SS_TUBE_SPIT, tube_spit_entity);

		return true;
	}

	return false;
}

typedef struct
{
	float time;
	color ambient;
	float highlight;
	float core;
} lightning_params_t;

static lightning_params_t static_lightning_params;

bool ks_fx_create_lightning (float dt, void** data, const char *name, float start, float total)
{
	region* reg = g_world_ptr->get_the_terrain ().find_region ("terrain")->get_data ();
	lightning_params_t *params = (lightning_params_t*) *data;
	vector<light_source*>::iterator lit;
	vector<light_source*> lights = g_world_ptr->get_lights ();
	entity_manager::iterator it;
	static float lightning_spec = 500;

	if (params == NULL)
	{
		*data = &static_lightning_params; //malloc (sizeof (lightning_params_t));
		params = (lightning_params_t*) *data;

		// only 1 at a time
		if (params->time != 0)
			return false;

		params->time = 0;
		params->ambient = reg->get_ambient ();
		params->highlight = WAVETEX_HighlightAlphaOffset;
		params->core = WAVETEX_CoreHighlightAlphaOffset;
	}

	params->time += dt;

	if (params->time > total || dt < 0)
	{
		int curbeach = g_game_ptr->get_beach_id ();

		WAVETEX_HighlightAlphaOffset = BeachDataArray[curbeach].highoff;
		WAVETEX_CoreHighlightAlphaOffset = BeachDataArray[curbeach].coreoff;
		reg->set_ambient (params->ambient);

		for (it = entity_manager::inst ()->begin (); it != entity_manager::inst ()->end (); it++)
		{
			if (!stricmp ((*it).second->get_parsed_name ().c_str (), name))
			{
				(*it).second->set_visible (false);
				(*it).second->SetTextureFrame (0);
			}
		}

		for (lit = lights.begin (); lit != lights.end (); lit++)
		{
			(*lit)->set_additive_color (color (0, 0, 0, 1));
		}

		params->time = 0;

		//    free (*data);
		*data = NULL;

		return false;
	}

	float stage = (params->time - start) / (total - start);

	if (stage > 0.5f)
	{
		WAVETEX_HighlightAlphaOffset = params->highlight + lightning_spec * (1 - stage);
		WAVETEX_CoreHighlightAlphaOffset = params->core + lightning_spec * (1 - stage);

		color col = params->ambient * stage;
		col.a = 1;
		reg->set_ambient (col);
	}
	else if (stage > 0)
	{
		WAVETEX_HighlightAlphaOffset = params->highlight + lightning_spec * stage;
		WAVETEX_CoreHighlightAlphaOffset = params->core + lightning_spec * stage;

		color col = params->ambient * (1 - stage);
		col.a = 1;
		reg->set_ambient (col);
	}

	if (stage > 0)
	{
		for (lit = lights.begin (); lit != lights.end (); lit++)
		{
			(*lit)->set_additive_color (color (stage, stage, stage, 1));
		}
	}

	for (it = entity_manager::inst ()->begin (); it != entity_manager::inst ()->end (); it++)
	{
		if (!stricmp ((*it).second->get_parsed_name ().c_str (), name))
		{
			(*it).second->set_visible (true);
			(*it).second->SetTextureFrame ((int)(params->time * 30));
		}
	}

	return true;
}

bool ks_fx_create_lightning_l (float dt, void** data)
{
	static float la1 = 1.3, la2 = 1.7;
	sfx.thunder();
	return ks_fx_create_lightning (dt, data, "lightning_l", la1, la2);
}

bool ks_fx_create_lightning_r (float dt, void** data)
{
	static float lb1 = 0.9, lb2 = 1.3;
	sfx.thunder();
	return ks_fx_create_lightning (dt, data, "lightning_r", lb1, lb2);
}

void ks_fx_add_splash(u_int fxindex, const vector3d& pos, float power)
{
	currentparticle++;
	if(currentparticle>=MAX_LOOSE_PARTICLES)
		currentparticle=0;

#ifdef FXD_PRECOMP
	prepare_part_slow(fxindex, &LooseParticles[currentparticle].part, power, 1.0f, pos, pos, 0.0001, 0);
#else
	prepare_part(fxindex, &LooseParticles[currentparticle].part, power, 1.0f, pos, pos, 0.0001, 0);
#endif

	LooseParticles[currentparticle].part.Tex = fx_tex[FX_TEX_BSPRAY_P];
	LooseParticles[currentparticle].move = 0;
}

void ks_fx_add_underwater_bubbles (vector3d pos)
{
	ks_fx_add_splash(FX_DEF_BUBBLES, pos, 1.0f);
	//  LooseParticles[currentparticle].part.Tex = fx_tex[FX_TEX_PADDLE];
	LooseParticles[currentparticle].move = 1;
}

void ks_fx_OnNewWave(void)	// named so that a search on "OnNewWave" will pick it up (dc 01/25/02)
{
	ks_fx_reset();
	ks_fx_init_wave();
}

void ks_fx_reset (void)
{
	u_int i,j;

	currentparticle = 0;

	// Reset Loose Particles
	for(i=0;i<MAX_LOOSE_PARTICLES;i++)
	{
		LooseParticles[i].part.Ctime = 1;
		LooseParticles[i].part.Dura = LooseParticles[i].part.Life = LooseParticles[i].part.Rlife = 0;
	}

	// Reset Trail Geometry
	for (j = 0; j < MAX_TRAIL_GENERATORS; j++)
		if (g_trails[j]->is_valid ())
			g_trails[j]->reset ();

	for (i = 0; i < MAX_LIP_SPLASHES; i++)
		lip_splashes[i].valid = false;

	for (u_int i = 0; i < MAX_CRASH_NODES; i++)
		crashnodes[i].Next = &crashnodes[i + 1];
	crashnodes[ MAX_CRASH_NODES - 1 ].Next = NULL;

	CrashFreeHead = &crashnodes[0];
	CrashHead=NULL;

	if (tube_spit_entity)
		tube_spit_entity->set_visible(false);
}

void ks_fx_destroy_all( void )
{
	//	ks_fx_trail_destroy(Surfer_Trail);
#ifdef TARGET_XBOX
	PARTICLE_Cleanup();
#endif
}

void trails_update( void )
{
	float dt = (float)TIMER_GetFrameSec();

	for (int i = 0; i < MAX_TRAIL_GENERATORS; i++)
	{
		if (g_trails[i]->is_valid ())
			g_trails[i]->update (dt);
	}
}

float clamp_inc(float newone, float oldone, float max_inc)
{
	float delta;

	delta = newone - oldone;

	if(fabs(delta) > max_inc)
	{
		if(delta<0)
			newone = oldone - max_inc;
		else
			newone = oldone + max_inc;
	}

	return(newone);
}

void set_size(crashnode_s *Ptr)
{
	if(Ptr->age > FXD.Wave_Inc)
		Ptr->size = Ptr->mag * (1 - ( Ptr->age - FXD.Wave_Inc ) / FXD.Wave_Dec);
	else
		Ptr->size = Ptr->mag * Ptr->age /
		FXD.Wave_Inc;
}

// This function is pretty fragile, but necessary, basically when a new crashpoint
// is found, this function inserts it into the huge list.
float debug_reject_dist = 0.5f * 0.5f;
crashnode_s *crash_insert(crashnode_s *Hint, vector3d pos, float mag, u_int type)
{
	crashnode_s *Ptr;
	crashnode_s *BackPtr;
	//	u_int done = 0;
	float tmp;

	//	currentcrash++;
	//	if(currentcrash>=MAX_CRASH_NODES)
	//		currentcrash = 0;
	//  assert (!crashnodes[currentcrash].used);

	// Don't crash the game if it can't insert the crash node - rbroner (6/24/02)
	if(CrashFreeHead == NULL)
	{
		return NULL;
	}

	assert( CrashFreeHead );
	crashnode_s *node = CrashFreeHead;

	node->age = (float)TIMER_GetFrameSec();
	node->mag = mag;
	node->size= 0;
	node->pos = pos;
	node->type = type;


	set_size( node );

	if((CrashHead==NULL)||(CrashHead->pos.x >= node->pos.x))
	{
		CrashFreeHead = node->Next;
		node->Next = CrashHead;
		CrashHead = node;
		return(CrashHead);
	}

	if(Hint!=NULL)
	{
		BackPtr = Hint;
		Ptr = Hint;
	}
	else
	{
		BackPtr = CrashHead;
		Ptr = CrashHead;
	}

#if defined(TARGET_XBOX)
	int running_count = 0;

	while((Ptr!=NULL) && (Ptr->pos.x < node->pos.x) && (running_count < MAX_CRASH_NODES))
	{
		tmp = node->pos.x - Ptr->pos.x;
		if((tmp * tmp) < (debug_reject_dist * debug_reject_dist))
			return(Ptr);

		running_count++;

		BackPtr=Ptr;
		Ptr=Ptr->Next;
	}

	if(running_count == MAX_CRASH_NODES)
	{
		debug_print( "crash_insert circular list\n" );
	}
#else
	while((Ptr!=NULL) && (Ptr->pos.x < node->pos.x))
	{
		tmp = node->pos.x - Ptr->pos.x;
		if((tmp * tmp) < (debug_reject_dist * debug_reject_dist))
			return(Ptr);

		BackPtr=Ptr;
		Ptr=Ptr->Next;
	}
#endif /* TARGET_XBOX JIV DEBUG */

	if(BackPtr->Next!=NULL)
	{
		tmp = node->pos.x - BackPtr->pos.x;
		if((tmp * tmp) < (debug_reject_dist * debug_reject_dist))
			return(BackPtr);
	}

	CrashFreeHead = node->Next;
	node->Next = BackPtr->Next;
	BackPtr->Next=node;

	return( node );
}


void crash_draw_intervals(void)
{
	u_int i, data = (u_int) -1, tex = (u_int) -1, type;
	crashnode_s *Ptr;
	float size, alpha = -1, age;
	nglParticleSystem PS;
	vector3d water_current;
	float interp = -1;
	int curbeach = g_game_ptr->get_beach_id ();
	float crash_fudge = WaveDataArray[WAVE_GetIndex()].crash_displacement;

	if (!BeachDataArray[curbeach].bdir)
		crash_fudge = -crash_fudge;

	memset (&PS, 0, sizeof (PS));
	WAVE_GlobalCurrent(&water_current);

	Ptr=CrashHead;
	if(CrashHead==NULL)
		return;

#if defined(TARGET_XBOX)
	int running_count = 0;

	while((Ptr!=NULL) && (Ptr->pos.x <= WAVE_Emitter[0].x) && (running_count < MAX_CRASH_NODES))
	{
		running_count++;
		Ptr=Ptr->Next;
	}

	if(running_count == MAX_CRASH_NODES)
	{
		debug_print( "crash_draw_intervals circular list\n" );
	}
#else
	while((Ptr!=NULL) && (Ptr->pos.x <= WAVE_Emitter[0].x))
		Ptr=Ptr->Next;

#endif /* TARGET_XBOX JIV DEBUG */

	if(Ptr==NULL)
		return;

	for(i=1;i<WAVE_MESHSTEPMAXX;i++)
	{
		size = 0;
		type = WAVE_EM_NONE;
		age = 0;

#if defined(TARGET_XBOX)
		running_count = 0;

		while((Ptr->pos.x > WAVE_Emitter[i-1].x) && (Ptr->pos.x <= WAVE_Emitter[i].x) && (running_count < MAX_CRASH_NODES))
#else
			while((Ptr->pos.x > WAVE_Emitter[i-1].x) && (Ptr->pos.x <= WAVE_Emitter[i].x))
#endif /* TARGET_XBOX JIV DEBUG */
			{

#if defined(TARGET_XBOX)
				running_count++;
#endif /* TARGET_XBOX JIV DEBUG */

				// Maybe we should also check WAVE_Emitter[i-1].type == WAVE_EM_NONE too since the data can be uninitialized.
				if((WAVE_Emitter[i].type == WAVE_EM_NONE) || (WAVE_Emitter[i].type == (WaveEmitterEnum) Ptr->type))
				{
					if(Ptr->size > size)
					{
						size = Ptr->size;
						age = Ptr->age;
						type = Ptr->type;
					}
				}
				else
				{
					// Auto Expire, it's drifted out of friendly waters
					Ptr->age = FXD.Wave_Dec + FXD.Wave_Inc + 1;
				}

				if(Ptr->Next == NULL)
					break;
				Ptr=Ptr->Next;
			}

#if defined(TARGET_XBOX)
			if(running_count == MAX_CRASH_NODES)
			{
				debug_print( "crash_draw_intervals circular list\n" );
			}
#endif /* TARGET_XBOX JIV DEBUG */

			// Something to Draw?
			if(size)
			{
				vector3d emit1, emit2;
				vector3d tmp;

				emit1.x = WAVE_Emitter[i-1].x;
				emit1.y = WAVE_Emitter[i-1].y;
				emit1.z = WAVE_Emitter[i-1].z;

				emit2.x = WAVE_Emitter[i].x;
				emit2.y = WAVE_Emitter[i].y;
				emit2.z = WAVE_Emitter[i].z;

				tmp = emit2 - emit1;

				switch(type)
				{
				case WAVE_EM_TRICKLE:
					alpha = 1.0f;

					if(FX_ParticleD[FX_DEF_LIP1].ModConst>0)
						interp = age / FX_ParticleD[FX_DEF_LIP1].ModConst;
					else
						interp = 1.0f;

					if(interp > 1.0f)
						interp = 1.0f;
					size = FX_ParticleD[FX_DEF_LIP1].ModSize;
					data = FX_DEF_LIP1;
					tex = FX_TEX_CRASH;
					break;

				case WAVE_EM_CREST:
					if(FX_ParticleD[FX_DEF_LIP2].ModAlpha>0)
						alpha = age / FX_ParticleD[FX_DEF_LIP2].ModAlpha;
					else
						alpha = 1.0f;

					if(alpha > 1.0f)
						alpha = 1.0f;
					size = FX_ParticleD[FX_DEF_LIP2].ModSize;
					data = FX_DEF_LIP2;
					tex = FX_TEX_CRASH;
					break;

				case WAVE_EM_WARN:
					if(FX_ParticleD[FX_DEF_WARN].ModAlpha>0)
						alpha = age / FX_ParticleD[FX_DEF_WARN].ModAlpha;
					else
						alpha = 1.0f;

					if(alpha > 1.0f)
						alpha = 1.0f;
					size = FX_ParticleD[FX_DEF_WARN].ModSize;
					data = FX_DEF_WARN;
					tex = FX_TEX_CRASH;
					break;

				case WAVE_EM_SPLASH:
					alpha = 1.0f;
				data = FX_DEF_CRASH1;
				tex = FX_TEX_CRASH2;

				emit1.x += crash_fudge;
				emit2.x += crash_fudge;
				break;

      default:
				assert(false);
				break;
			}

			assert (data != (u_int) -1);	// else uninitialized (dc 01/29/02)
			assert (tex != (u_int) -1);	// else uninitialized (dc 01/29/02)
			assert (alpha != -1);	// else uninitialized (dc 01/29/02)

			// Submit Crash1
			prepare_part(data, &PS, size, alpha,emit1,emit2, -1.0f, (float)TIMER_GetTotalSec());
			PS.Seed = (int)Crash_Seeds[i];
			PS.Tex = fx_tex[tex];

			PS.Num = (u_int)FTOI(tmp.length() * PS.Num);

			switch(type)
			{
			case WAVE_EM_TRICKLE:
				tmp.normalize();
				tmp *= water_current.x;

				assert (interp != -1);	// else uninitialized (dc 01/29/02)
				PS.Force[0] *= interp;
				PS.Force[1] *= interp;
				PS.Force[2] *= interp;
				PS.Force[3] *= interp;

				break;
			default:
				tmp.y = 0;
				tmp.x = water_current.x;
				tmp.z = 0;
				break;
			}

			PS.Svel[0] += tmp.x;
			PS.Svel[1] += tmp.y;
			PS.Svel[2] += tmp.z;
      PS.MaxSize = 75000;


		  int heroNum = WAVETEX_GetPlayer(); //g_game_ptr->get_active_player();
			if (UNDERWATER_CameraUnderwater (heroNum))
			{
				PS.Scol = NGL_RGBA32 (40 + ((BeachDataArray[curbeach].underwaterambient & 0x00ff0000) >> 16), 40 + ((BeachDataArray[curbeach].underwaterambient & 0x0000ff00) >> 8), 40 + (BeachDataArray[curbeach].underwaterambient & 0x000000ff), (PS.Scol & 0xff000000) >> 24);
				PS.Ecol = NGL_RGBA32 (40 + ((BeachDataArray[curbeach].underwaterambient & 0x00ff0000) >> 16), 40 + ((BeachDataArray[curbeach].underwaterambient & 0x0000ff00) >> 8), 40 + (BeachDataArray[curbeach].underwaterambient & 0x000000ff), (PS.Ecol & 0xff000000) >> 24);
			}

			if(FXD.Render.Part_Crash)
				nglListAddParticle(&PS);
		}

		if(Ptr==NULL)
			return;
	}
}

float perc_big = 0;
float size_big = 1;

void crash_update_intervals( void )
{
	u_int i=0;
	crashnode_s *Ptr;
	crashnode_s *BackPtr;
	crashnode_s *Hint;
	vector3d water_current;

#ifdef CRASH_PROF
	static u_int insert_cycles = 0;
	static u_int insert_misses = 0;
	static u_int total_cycles = 0;
	static u_int total_misses = 0;
	int control;

	static int count = 0;
	control = ( SCE_PC0_CPU_CYCLE | SCE_PC_U0 );
	control |= ( SCE_PC1_DCACHE_MISS | SCE_PC_U1 );
	control |= SCE_PC_CTE;
	scePcStart( control, 0, 0 );
#endif



	WAVE_GlobalCurrent(&water_current);

	Hint=NULL;
	Ptr = CrashHead;
	BackPtr = CrashHead;

	while(Ptr!=NULL && i<MAX_CRASH_NODES)
	{
		Ptr->pos.x += water_current.x * (float)TIMER_GetFrameSec();
		Ptr->age += (float)TIMER_GetFrameSec();

		// is it time to delete
		if(Ptr->age > (FXD.Wave_Inc + FXD.Wave_Dec))
		{
			// are we at front of list
			if(BackPtr == Ptr)
			{
				CrashHead = Ptr->Next;
				BackPtr = Ptr->Next;
			}
			else
			{
				BackPtr->Next = Ptr->Next;
			}

			crashnode_s *node = Ptr;
			Ptr=Ptr->Next;
			node->Next = CrashFreeHead;
			CrashFreeHead = node;
			i++;
			continue;
		}

		set_size(Ptr);

		BackPtr = Ptr;
		Ptr = Ptr->Next;
		i++;
	}

#ifdef BRONER
  static int curcrashnodes = 0;
  static int maxcrashnodes = 0;
  curcrashnodes = i;
  if((int)i > maxcrashnodes)
    maxcrashnodes = i;
#endif

	//assert (i < MAX_CRASH_NODES);   // Don't assert if the number of crash nodes as reached the max - rbroner (6/24/02)

	// Add any new spray
	for(i = 0; i<WAVE_MESHSTEPMAXX; i++)
	{
		if(WAVE_Emitter[i].type > WAVE_EM_NONE)
		{
			vector3d tmp;
			float mag = 0;

			tmp.x = WAVE_Emitter[i].x;
			tmp.y = WAVE_Emitter[i].y;
			tmp.z = WAVE_Emitter[i].z;

			switch(WAVE_Emitter[i].type)
			{
			case WAVE_EM_TRICKLE:
				mag = FX_ParticleD[FX_DEF_LIP1].ModSize;
				break;
			case WAVE_EM_CREST:
				mag = FX_ParticleD[FX_DEF_LIP2].ModSize;
				break;
			case WAVE_EM_SPLASH:
				if(WAVE_Emitter[i].cresty > 0)
				{
					mag = FX_ParticleD[FX_DEF_CRASH1].ModConst + WAVE_Emitter[i].cresty * FX_ParticleD[FX_DEF_CRASH1].ModSize;
					if((WAVE_Emitter[i-1].type == WAVE_EM_TRICKLE) || ((i<(WAVE_MESHSTEPMAXX-1))&&(WAVE_Emitter[i+1].type == WAVE_EM_TRICKLE)))
					{
						if(random(100) < perc_big)
							mag += size_big;
					}
				}
				break;
			case WAVE_EM_WARN:
				mag = FX_ParticleD[FX_DEF_CRASH1].ModConst + WAVE_Emitter[i].y * FX_ParticleD[FX_DEF_CRASH1].ModSize;
				break;
			default:
				assert(false);
				break;
			}
#ifdef CRASH_PROF
			int temp_cycles = scePcGetCounter0();
			int temp_misses = scePcGetCounter1();
#endif
			// this is just a kludge because it was crashing when rendering the xbox boat movie
			if(!os_developer_options::inst()->is_flagged (os_developer_options::FLAG_MAKE_MOVIE))
				Hint = crash_insert(Hint,tmp,mag,WAVE_Emitter[i].type);
#ifdef CRASH_PROF
			insert_cycles += scePcGetCounter0() - temp_cycles;
			insert_misses += scePcGetCounter1() - temp_misses;
#endif
		}
	}

#ifdef CRASH_PROF
	total_cycles += scePcGetCounter0();
	total_misses += scePcGetCounter1();
	if( count == 30 )
	{
		total_cycles /= 30;
		total_misses /= 30;
		insert_cycles /= 30;
		insert_misses /= 30;

		nglPrintf( "TC: %u   TM: %u        IC: %u   IM: %u\n", total_cycles, total_misses, insert_cycles, insert_misses );

		total_cycles = total_misses = insert_cycles = insert_misses = 0;
		count = 0;
	}
	count++;
	scePcStop();
#endif
	return;
}

// For MENUDRAW
bool ks_fx_GetDrawCrash(void)
{
	return FXD.Render.Part_Crash;
}

void ks_fx_SetDrawCrash(bool onoff)
{
	FXD.Render.Part_Crash = onoff;
}

bool ks_fx_GetDrawTrailGeom(void)
{
	return FXD.Render.Geom_Trail;
}

void ks_fx_SetDrawTrailGeom(bool onoff)
{
	FXD.Render.Geom_Trail = onoff;
}

bool ks_fx_GetDrawSpray(void)
{
	return FXD.Render.Spray_Trail;
}

void ks_fx_SetDrawSpray(bool onoff)
{
	FXD.Render.Spray_Trail = onoff;
}

bool ks_fx_GetDrawSprayGeom(void)
{
	return FXD.Render.Geom_Spray;
}

void ks_fx_SetDrawSprayGeom(bool onoff)
{
	FXD.Render.Geom_Spray = onoff;
}

bool ks_fx_GetDrawLoose(void)
{
	return FXD.Render.Part_Loose;
}

void ks_fx_SetDrawLoose(bool onoff)
{
	FXD.Render.Part_Loose = onoff;
}

bool ks_fx_GetDrawWeather(void)
{
	return FXD.Render.Weather;
}

void ks_fx_SetDrawWeather(bool onoff)
{
	FXD.Render.Weather = onoff;
}

bool ks_fx_GetDrawUnderwater(void)
{
	return FXD.Render.Underwater;
}

void ks_fx_SetDrawUnderwater(bool onoff)
{
	FXD.Render.Underwater = onoff;
}

bool ks_fx_GetDrawSplashes(void)
{
	return FXD.Render.Splashes;
}

void ks_fx_SetDrawSplashes(bool onoff)
{
	FXD.Render.Splashes = onoff;
}

