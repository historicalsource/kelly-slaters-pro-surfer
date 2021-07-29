#ifndef KSFX_H
#define KSFX_H

#include "ksngl.h"

#define MAX_LOOSE_PARTICLES 100

class trail;
class kellyslater_controller;
class SurfBoardObjectClass;

trail* ks_fx_trail_create (float sample_rate, float life, bool extra, kellyslater_controller *owner);
void ks_fx_trail_destroy (trail *dead);
void ks_fx_trail_draw(const int heroIdx);

struct fx_splash {
	float ModMax;
	float MinSize;
	float ModSize;
	float ModConst;
	float ModAlpha;
	float Ssize;
	float Rsize;
	float Esize;
	float Aspect;
	u_int Scol;
	u_int Rcol;
	u_int Ecol;
	float Life;
	float Rlife;
	float Num;
	float Svel_Hangle;
	float Svel_Vangle;
	float Svel_Mag;
	float Rvel1_Hangle;
	float Rvel1_Vangle;
	float Rvel1_Mag;
	float Rvel2_Hangle;
	float Rvel2_Vangle;
	float Rvel2_Mag;
	float Rvel3_Hangle;
	float Rvel3_Vangle;
	float Rvel3_Mag;
	float Force_Hangle;
	float Force_Vangle;
	float Force_Mag;
	u_int BlendMode;
	u_int MaterialFlags;
};

struct fx_data_render_def {
	int Part_Crash;
	int Geom_Trail;
  int Spray_Trail;
	int Geom_Spray;
	int Part_Loose;
  int Weather;
  int Underwater;
	int Splashes;
};

struct fx_def {
	struct fx_splash *Trail;
	struct fx_splash *Crash1;
	struct fx_splash *Lip1;
	struct fx_splash *Lip2;
	struct fx_splash *Bspray1;
	struct fx_splash *Bspray2;
	struct fx_splash *GenSplash;
	struct fx_splash *TubeSpit;
	struct fx_data_render_def Render;
	float Trail_Life;
	float Trail_Start_Width_Mod;
	float Trail_Max_Spread;
	float Spray_Max_Spread;
	float Spray_Min_Thresh;
	float Spray_Max_Thresh;
	float Spray_Scale1;
	float Spray_Scale2;
	float Spray_Vary1;
	float Spray_Vary2;
	float Wave_Inc;
	float Wave_Dec;
};

enum {
	FX_DEF_CRASH1,
	FX_DEF_LIP1,
	FX_DEF_LIP2,
	FX_DEF_WARN,
	FX_DEF_BSPRAY1,
	FX_DEF_BSPRAY2,
	FX_DEF_TRAIL,
	FX_DEF_GENSPLASH,
	FX_DEF_PADDLE,
	FX_DEF_TUBESPIT,
  FX_DEF_FLOATER,
	FX_DEF_AIRDROPS,
	FX_DEF_BUBBLES,
  FX_DEF_CHOPHOPSPLASH,
	FX_DEF_TOTAL,
};

enum {
	FX_TEX_CRASH,
	FX_TEX_CRASH2,
	FX_TEX_BSPRAY_P,
	FX_TEX_BSPRAY_G,
	FX_TEX_TRAIL,
  FX_TEX_PADDLE,
  FX_TEX_FLOATER,
  FX_TEX_WEATHER,
  FX_TEX_ALGEE,
  FX_TEX_SNAP_SPRAY,
	FX_TEX_LAUNCH,
	FX_TEX_TOTAL
};

extern bool particle_enable;
extern struct fx_def FXD;
extern nglTexture *fx_tex[FX_TEX_TOTAL];
extern struct fx_splash FX_ParticleD[FX_DEF_TOTAL];

#include "osparticle.h"	// For nglParticleSystem

#include "osparticle.h"	// For nglParticleSystem

// Interface Functions
float clamp_inc(float newone, float oldone, float max_inc);
void prepare_part(u_int fxindex, nglParticleSystem *PartPtr, float interp_power, float interp_alpha, vector3d Spos1, vector3d Spos2, float Duration, float age);

void ks_fx_init(void);
void ks_fx_update(void);
void ks_fx_draw(const int heroIdx);
void ks_fx_OnNewWave(void);	// named so that a search on "OnNewWave" will pick it up (dc 01/25/02) 
void ks_fx_reset (void);
void ks_fx_add_splash(u_int fxindex, const vector3d& pos, float power);
void ks_fx_create_big_splash(vector3d pos);
void ks_fx_create_paddle_splash(vector3d pos);
void ks_fx_create_snap_splash(const vector3d& pos, const vector3d& normal, WavePositionHint& hint, bool left, int hero_index);
void ks_fx_create_launch_splash(const vector3d& pos, const vector3d& normal, WavePositionHint& hint, int hero_index);
bool ks_fx_create_tube_spit (float dt, void** data);
bool ks_fx_create_lightning_l (float dt, void** data);
bool ks_fx_create_lightning_r (float dt, void** data);
void ks_fx_add_underwater_bubbles (vector3d pos);
void ks_fx_destroy_all( void );
bool ks_fx_spit_going_on ();
void ks_fx_init_wipeout_stuff ();
void ks_fx_start_wipeout_splash (int index);
void ks_fx_end_wipeout_splash (int index);

bool ks_fx_GetDrawCrash(void);
void ks_fx_SetDrawCrash(bool onoff);
bool ks_fx_GetDrawTrailGeom(void);
void ks_fx_SetDrawTrailGeom(bool onoff);
bool ks_fx_GetDrawSpray(void);
void ks_fx_SetDrawSpray(bool onoff);
bool ks_fx_GetDrawSprayGeom(void);
void ks_fx_SetDrawSprayGeom(bool onoff);
bool ks_fx_GetDrawLoose(void);
void ks_fx_SetDrawLoose(bool onoff);

bool ks_fx_GetDrawCrash(void);
void ks_fx_SetDrawCrash(bool onoff);
bool ks_fx_GetDrawTrailGeom(void);
void ks_fx_SetDrawTrailGeom(bool onoff);
bool ks_fx_GetDrawSpray(void);
void ks_fx_SetDrawSpray(bool onoff);
bool ks_fx_GetDrawSprayGeom(void);
void ks_fx_SetDrawSprayGeom(bool onoff);
bool ks_fx_GetDrawLoose(void);
void ks_fx_SetDrawLoose(bool onoff);
bool ks_fx_GetDrawWeather(void);
void ks_fx_SetDrawWeather(bool onoff);
bool ks_fx_GetDrawUnderwater(void);
void ks_fx_SetDrawUnderwater(bool onoff);
bool ks_fx_GetDrawSplashes(void);
void ks_fx_SetDrawSplashes(bool onoff);

#endif
