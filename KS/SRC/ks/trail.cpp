//
// Trails on the water
//
// Based on old Ike's code, now maintained by Leo
//

#include "global.h"
#include "conglom.h"
#include "kellyslater_controller.h"
#include "ksfx.h"
#include "ksreplay.h"
#include "timer.h"
#include "app.h"
#include "game.h"
#include "wds.h"
#include "profiler.h"
#include "osparticle.h"	// For nglParticleSystem (dc 05/22/02)
#include "ksngl.h"	// For nglMeshFastWrite**** (dc 05/22/02)

#include "trail.h"
#include "board.h"
#if defined(TARGET_PS2)
#include "ngl_ps2_internal.h"	// For nglMeshFastWriteVertex functions only!!! (dc 12/14/01)
#endif // defined(TARGET_PS2)
#if defined(TARGET_GC)
#include "ngl.h"
#include "ngl_gc_internal.h"	// For nglMeshFastWriteVertex functions only!!! (dc 12/14/01)
#endif

static trail *trail_p1 = NULL; // (true);
static trail *trail_p2 = NULL; //  (true);
static trail *trail_m1 = NULL; //  (false);
static trail *trail_m2 = NULL; //  (false);
trail* g_trails[MAX_TRAIL_GENERATORS]; // = { &trail_p1, &trail_p2, &trail_m1, &trail_m2 };

void TRAIL_StaticInit( void )
{
	trail_p1 = NEW trail(true);
	trail_p2 = NEW trail(true);
	trail_m1 = NEW trail(false);
	trail_m2 = NEW trail(false);
	int index=0;
	g_trails[index++]=trail_p1;
	g_trails[index++]=trail_p2;
	g_trails[index++]=trail_m1;
	g_trails[index++]=trail_m2;
}

// constants for tweaking on the debugger
static float floater_interval_time = 0.05f;
static float airdrops_time = 0.2f;
static float airdrops_interval_time = 0.05f;
static float hands_interval_time = 0.05f;
static float debug_trail_perc = 0.03f;
static float debug_trail_height = 0.1f;
static int debug_trail_mod = 20;
static float extra_splash_power_decay = 10.0f;
static float extra_splash_power_amount = 10.0f;
static float debug_zbias = 10000.0f; // zbias' the trail towards the camera, a higher value pulls it closer to the camera

float debug_ellipse_len = 0.7;
float debug_ellipse_wid = 0.3;

static struct
{
  float angle_scale;
  float vel_scale;
  float bonus_y;
  float tex_size;
  float tex_speed;
  float tex_scale;
  float grav;
  float curve;
} board_spray = { 0.025f, 0, 1, 0.1f, 0.5f, 2.5f, -30, 1 };

// ============================================================================

trail* ks_fx_trail_create (float sample_rate, float life, bool extra, kellyslater_controller *owner)
{
  // find an unused trail
  if (owner)
  {
    if (!trail_p1->is_valid ())
    {
      trail_p1->initialize (sample_rate, life, extra, owner);
      return trail_p1;
    }

    if (!trail_p2->is_valid ())
    {
      trail_p2->initialize (sample_rate, life, extra, owner);
      return trail_p2;
    }

    nglPrintf ("No trail for a player !\n");
    return NULL;
  }

  if (!trail_m1->is_valid ())
  {
    trail_m1->initialize (sample_rate, life, extra, owner);
    return trail_m1;
  }

  if (!trail_m2->is_valid ())
  {
    trail_m2->initialize (sample_rate, life, extra, owner);
    return trail_m2;
  }

  nglPrintf ("No trail for an object !\n");

  return NULL;
}

void ks_fx_trail_destroy (trail *dead)
{
	dead->destroy ();
}

void ks_fx_trail_draw(const int heroIdx)
{
  for (int j = 0; j < MAX_TRAIL_GENERATORS; j++)
  {
    // Splitscreen: only draw 1 trail per side.
	if (g_game_ptr->get_num_active_players() == 2)
	{
		assert(MAX_PLAYERS == 2);
		if (heroIdx == 0 && j == 1) continue;
		if (heroIdx == 1 && j == 0) continue;
	}

	 if (g_trails[j]->is_valid ())
      g_trails[j]->draw ();
  }
}

// ============================================================================

trail::trail (bool spray)
{
  if (spray)
    my_spray = (spray_params_t *) memalign (16, sizeof (spray_params_t));
  else
    my_spray = NULL;

  valid = false;
}

trail::~trail ()
{
  free (my_spray);
  destroy ();
}

void trail::initialize (float sample_rate, float life, bool extra, kellyslater_controller *owner)
{
  u_int i;

  valid = true;

  // NGL only supports strips of up to 36 vertexes
//  if (sample_rate * life > 17)
//    life = 17 / sample_rate;

  my_sample_rate = sample_rate;
  my_total_nodes = MAX_TRAIL_NODES;

  for (i = 0; i < my_total_nodes; i++)
  {
    my_trail_data[i].valid = 0;
    my_trail_data[i].pnt1.vvalid = 0;
    my_trail_data[i].pnt2.vvalid = 0;
    my_trail_data[i].pnt3.vvalid = 0;
  }

  my_extra = extra;
  my_life = life;
  my_last_sample = 0;
  my_index = 0;
  my_lastmag1 = 0;
  my_lastmag2 = 0;

  my_owner = owner;

  if (my_owner)
  {
    my_spray->last_apow = 0;
    my_spray->last_bpow = 0;
    my_spray->num_control_points_a = 0;
    my_spray->num_control_points_b = 0;
    my_spray->floater_interval = 0;
    my_spray->airdrops_interval = 0;
    my_spray->left_hand_interval = 0;
    my_spray->right_hand_interval = 0;
    my_spray->extra_splash_power[0] = 0;
    my_spray->extra_splash_power[1] = 0;

    for (i = 0; i < MAX_SPRAY_PTS; i++)
    {
      my_spray->control_points_a[i].seed = (g_random_ptr->rand()<<2)%0x7fffffff;
      my_spray->control_points_b[i].seed = (g_random_ptr->rand()<<2)%0x7fffffff;
      my_spray->control_points_a[i].valid = false;
      my_spray->control_points_b[i].valid = false;
    }
  }
}

void trail::destroy ()
{
  if (!valid)
    return;

  valid = false;
}

void trail::reset ()
{
  my_index = 0;
  my_last_sample = 0;
  my_lastmag1 = 0;
  my_lastmag2 = 0;

  for (u_int i = 0; i < my_total_nodes; i++)
    my_trail_data[i].valid = false;

  if (my_spray)
  {
    my_spray->last_apow = 0;
    my_spray->last_bpow = 0;
    my_spray->num_control_points_a = 0;
    my_spray->num_control_points_b = 0;
    my_spray->floater_interval = 0;
    my_spray->airdrops_interval = 0;
    my_spray->left_hand_interval = 0;
    my_spray->right_hand_interval = 0;
    my_spray->extra_splash_power[0] = 0;
    my_spray->extra_splash_power[1] = 0;

    for (int i = 0; i < MAX_SPRAY_PTS; i++)
    {
      my_spray->control_points_a[i].valid = false;
      my_spray->control_points_b[i].valid = false;
    }
  }
}

// ============================================================================

void trail::add_point (vector3d *pointa, vector3d *center, vector3d *pointb, float vela, float velb, WavePositionHint *hint, bool dummy)
{
  float mag1 = 0.2f;
  float mag3 = 0.2f;

  if ((!my_sample_rate) || (!pointa) || (!pointb))
    return;

  if ((pointa->x > WAVE_MeshMaxX) || (pointa->x < WAVE_MeshMinX) || (pointa->z > WAVE_MeshMaxZ) || (pointa->z < WAVE_MeshMinZ))
    return;

  if (center != NULL)
    if ((center->x > WAVE_MeshMaxX) || (center->x < WAVE_MeshMinX) || (center->z > WAVE_MeshMaxZ) || (center->z < WAVE_MeshMinZ))
      return;

  if ((pointb->x > WAVE_MeshMaxX) || (pointb->x < WAVE_MeshMinX) || (pointb->z > WAVE_MeshMaxZ) || (pointb->z < WAVE_MeshMinZ))
    return;

  if (my_last_sample > (1.0f / my_sample_rate))
  {
    my_index++;
    if (my_index >= my_total_nodes)
      my_index = 0;

    my_last_sample -= (1.0f / my_sample_rate);
  }

  my_trail_data[my_index].age = 0;
  my_trail_data[my_index].valid = true;

	// we're only creating a dummy point while the surfer is in the air
	if (dummy)
		my_trail_data[my_index].valid = false;
	else
	{
	  WaveTolerance tolerance(1, 2);
	  if (my_extra)
		  create_surface_point (&my_trail_data[my_index].pnt2, center, hint, &tolerance);
	  tolerance.dthresh += debug_ellipse_len;	// side points are not as close to water
	  tolerance.zthresh += debug_ellipse_len;	// side points are not as close to hint
	  create_surface_point (&my_trail_data[my_index].pnt1, pointa, hint, &tolerance);
	  create_surface_point (&my_trail_data[my_index].pnt3, pointb, hint, &tolerance);
	}

  if (vela > 0)
    mag1 = min(0.2f , 1 / vela * debug_trail_perc);

  if (velb > 0)
    mag3 = min(0.2f , 1 / velb * debug_trail_perc);

  mag1 = clamp_inc (mag1, my_lastmag1, FXD.Trail_Max_Spread);
  mag3 = clamp_inc (mag3, my_lastmag2, FXD.Trail_Max_Spread);

  my_lastmag1 = mag1;
  my_lastmag2 = mag3;

  if (my_trail_data[my_index].valid)
  {
	  if (my_extra)
	  {
		WAVE_GetVHint (&my_trail_data[my_index].pnt2.hint, &my_trail_data[my_index].pnt1.hint, mag1, &my_trail_data[my_index].pnt1.vhint);
		WAVE_GetVHint (&my_trail_data[my_index].pnt2.hint, &my_trail_data[my_index].pnt3.hint, mag3, &my_trail_data[my_index].pnt3.vhint);
	  }
	  else
	  {
		WAVE_GetVHint (&my_trail_data[my_index].pnt3.hint, &my_trail_data[my_index].pnt1.hint, mag1, &my_trail_data[my_index].pnt1.vhint);
		WAVE_GetVHint (&my_trail_data[my_index].pnt1.hint, &my_trail_data[my_index].pnt3.hint, mag3, &my_trail_data[my_index].pnt3.vhint);
	  }

	  my_trail_data[my_index].pnt1.vvalid = 1;
	  my_trail_data[my_index].pnt3.vvalid = 1;
  }

  my_trail_data[my_index].uv = my_index % debug_trail_mod;

  float uv_factor = ((float)1.0f)/debug_trail_mod;
  my_trail_data[my_index].uv = uv_factor * my_trail_data[my_index].uv;
}

// This will create a point that is on the water that will be updated with the water's current.
void trail::create_surface_point (surface_point_t *newone, vector3d *pos, WavePositionHint *hint, 
								  const WaveTolerance *tolerance)
{
  WaveQueryFlags flags;

  flags = (WaveQueryFlags)(WAVE_YGIVEN | WAVE_NEARMATCH | WAVE_HINTSOUGHT | WAVE_NORMALSOUGHT);

  if (hint != NULL)
    flags = (WaveQueryFlags)(flags | WAVE_HINTGIVEN);

#if defined(TARGET_XBOX)
	WaveNearestArgs args(
      flags,
      *pos,
      &newone->pos,
	  &newone->norm,
      NULL,                       //  cs.current
	  &newone->region,
      *hint,
	  &newone->hint,
	  *tolerance );
#else
	WaveNearestArgs args = {
      flags,
      *pos,
      &newone->pos,
	  &newone->norm,
      NULL,                       //  cs.current
	  &newone->region,
      *hint,
	  &newone->hint,
	  *tolerance,
	};
#endif /* TARGET_XBOX JIV DEBUG */

	WAVE_FindNearest(args);
}

// ============================================================================

// This will iterate through all the trailpoints created and update them for the frame.
// it will also expire any points that journey into bad sections of the wave.
void trail::update (float dt)
{
  WaveRegionEnum region;

  // Toby hack: don't update trails of inactive players.
  if (my_owner && !my_owner->is_active())
	  return;

  my_last_sample += dt;

  for (u_int i = 0; i < my_total_nodes; i++)
  {
    trail_node_t *node = &my_trail_data[i];

    if (!node->valid)
      continue;

    node->age += dt;

    // Check to see if it has expired
    if (node->age > my_life)
    {
      node->valid = false;
      continue;
    }

    update_surface_point (&node->pnt1);
    region = node->pnt1.region;

    if ((region == WAVE_REGIONROOF) || (region == WAVE_REGIONCURL) ||
//        (region == WAVE_REGIONLIP)  || (region == WAVE_REGIONLIP2) ||
        (region == WAVE_REGIONSHOULDER) || (region == WAVE_REGIONSHOULDER2))
    {
//      node->valid = false;
      node->age *= 1.75f;
      continue;
    }

    if (my_extra)
    {
      update_surface_point (&node->pnt2);
      region = node->pnt2.region;

      if ((region == WAVE_REGIONROOF) || (region == WAVE_REGIONCURL) ||
//          (region == WAVE_REGIONLIP)  || (region == WAVE_REGIONLIP2) ||
          (region == WAVE_REGIONSHOULDER) || (region == WAVE_REGIONSHOULDER2))
      {
//        node->valid = false;
        node->age *= 1.75f;
        continue;
      }
    }

    update_surface_point (&node->pnt3);
    region = node->pnt3.region;

    if ((region == WAVE_REGIONROOF) || (region == WAVE_REGIONCURL) ||
//        (region == WAVE_REGIONLIP)  || (region == WAVE_REGIONLIP2) ||
        (region == WAVE_REGIONSHOULDER) || (region == WAVE_REGIONSHOULDER2))
    {
//      node->valid = false;
      node->age *= 1.75f;
      continue;
    }
  }

  if (my_owner)
  {
    SurfBoardObjectClass& board = my_owner->get_board_controller();

  	spraypt_update (my_spray->control_points_a, MAX_SPRAY_PTS);
	  spraypt_update (my_spray->control_points_b, MAX_SPRAY_PTS);

    if (board.InAir ())
      board_update_air (dt);
    else
      board_update (dt);
  }
}

bool trail::hand_underwater (entity *hand, vector3d& pos) const
{
  WavePositionHint hint;
  WAVE_HintFromMarker(WAVE_MarkerHandDrag, &hint);

  vector3d normal;
  CollideCallStruct collide (hand->get_abs_position (), &normal, NULL, NULL, &hint);
  WAVE_CheckCollision (collide, true, true, false);
  pos = collide.position;

  // calculate the distance from the hand to the surface of the water
  vector3d dist = collide.position - hand->get_abs_position ();

  if (dot (dist, normal) > 0)
    return true;
  else
    return false;
}

void trail::board_update (float dt)
{
  vector3d pos, pointa, pointb, center_pt;
  spray_control_t SprayPt;
  u_int left;
  int hand;
	static SSEventId lHandEvent=-1, rHandEvent=-1;
  SurfBoardObjectClass& board = my_owner->get_board_controller();
  entity *board_ptr = my_owner->GetBoard();

  hand = my_owner->IsTubeHandInWater ();

  if ((hand == LEFT_HAND_IN_WATER) || (hand == BOTH_HANDS_IN_WATER))
  {
    entity *ent = ((conglomerate*) my_owner->get_owner ())->get_member("BIP01 L HAND");
    my_spray->left_hand_interval += dt;

    if ((my_spray->left_hand_interval > hands_interval_time) && hand_underwater (ent, pos))
    {
      ks_fx_add_splash (FX_DEF_FLOATER, pos, 1.0f);
      my_spray->left_hand_interval -= hands_interval_time;
			if (lHandEvent < 0)
				lHandEvent = SoundScriptManager::inst()->startEvent(SS_HANDDRAG, ent);
    }
		else if (lHandEvent >= 0)
		{
			lHandEvent = -1;
			SoundScriptManager::inst()->endEvent(lHandEvent);
		}
			
  }
  else
    my_spray->left_hand_interval = 0;

  if ((hand == RIGHT_HAND_IN_WATER) || (hand == BOTH_HANDS_IN_WATER))
  {
    entity *ent = ((conglomerate*) my_owner->get_owner ())->get_member("BIP01 R HAND");
    my_spray->right_hand_interval += dt;

    if ((my_spray->right_hand_interval > hands_interval_time) && hand_underwater (ent, pos))
    {
      ks_fx_add_splash (FX_DEF_FLOATER, pos, 1.0f);
      my_spray->right_hand_interval -= hands_interval_time;
			if (rHandEvent < 0)
				rHandEvent = SoundScriptManager::inst()->startEvent(SS_HANDDRAG, ent);
		}
		else if (rHandEvent >= 0)
		{
			SoundScriptManager::inst()->endEvent(rHandEvent);
			rHandEvent = -1;
		}
		
  }
  else
    my_spray->right_hand_interval = 0;

  pos = board_ptr->get_abs_po().slow_xform(vector3d (0, 0, 0.5));

  // compute the starting locations of the board trail in local space
  comp_trail_orig (&pointa, &center_pt, &pointb);
  center_pt = pos + board_ptr->get_abs_po().non_affine_slow_xform(center_pt);

  if (my_trail_data[my_index].valid)
  {
	  vector3d dist_vec;
	  float dist1, dist2;

	  dist_vec = pointa - my_spray->lastpointa;
	  dist1 = dist_vec.length2 ();

	  dist_vec = pointb - my_spray->lastpointa;
	  dist2 = dist_vec.length2 ();

	  if (dist1 > dist2)
	  {
		  dist_vec = pointb;
		  pointb = pointa;
		  pointa = dist_vec;
	  }
/*
    vector3d v1 (pos + my_spray->lastpointa - my_trail_data[my_index].pnt2.pos);
    vector3d v2 (pos + pointa - my_trail_data[my_index].pnt2.pos);
    vector3d v3 (pos + pointb - my_trail_data[my_index].pnt2.pos);

    v1.normalize ();
    v2.normalize ();
    v3.normalize ();

    if (dot (v1, v2) < dot (v1, v3))
    {
      v1 = pointa;
      pointa = pointb;
      pointb = v1;
    }
*/
  }

  my_spray->lastpointa = pointa;
  my_spray->lastpointb = pointb;

  if (pointa.x<0)
    left = 1;
  else
    left = 0;

  // Transform local 2 world
  pointa  = pos + board_ptr->get_abs_po().non_affine_slow_xform (pointa);
  pointb  = pos + board_ptr->get_abs_po().non_affine_slow_xform (pointb);

  assert( pointa.is_valid() );
  assert( pointb.is_valid() );

  // Add trail geometry pts.
  WavePositionHint *hintptr = NULL;

  if (board.center_hint_valid)
	  hintptr = &board.wave_center_hint;

  if (my_owner->get_super_state () != SUPER_STATE_LIE_ON_BOARD &&
		my_owner->get_super_state () != SUPER_STATE_FLYBY &&
		my_owner->get_current_state () != STATE_LIETOSTAND)
  {
    if (!board.WipedOut())
    	add_point (&pointa, &center_pt, &pointb, 0, 0, hintptr, false);

    // Calculate board spray
    vector3d avec, bvec;
    vector3d lvec, rvec;

    lvec.x = -1.0f;
    lvec.y = 0;
    lvec.z = 0;

    rvec.x = 1.0f;
    rvec.y = 0;
    rvec.z = 0;

    lvec = board.rb->my_po.non_affine_slow_xform(lvec);
    rvec = board.rb->my_po.non_affine_slow_xform(rvec);

    {
      vector3d v;

      v = cross(my_owner->get_board_controller().normal, lvec);
      lvec = cross(v, my_owner->get_board_controller().normal);

      v = cross(my_owner->get_board_controller().normal, rvec);
      rvec = cross(v, my_owner->get_board_controller().normal);
    }

    if (left)
    {
      avec = lvec;
      bvec = rvec;
    }
    else
    {
      avec = rvec;
      bvec = lvec;
    }

    float apow = comp_part_bspray (&avec, &pos, dt, FXD.Spray_Scale1);
    apow = clamp_inc(apow, my_spray->last_apow, FXD.Spray_Max_Spread);
    float bpow = comp_part_bspray (&bvec, &pos, dt, FXD.Spray_Scale1);
    bpow = clamp_inc(bpow, my_spray->last_bpow, FXD.Spray_Max_Spread);

    // we can't have spray coming from both sides at the same time
    if (apow > FXD.Spray_Min_Thresh && bpow > FXD.Spray_Min_Thresh)
    {
      if (apow > bpow)
        bpow = 0;
      else
        apow = 0;
    }

    my_spray->last_apow = apow;
    my_spray->last_bpow = bpow;

    apow += my_spray->extra_splash_power[0];
    bpow += my_spray->extra_splash_power[1];

    my_spray->extra_splash_power[0] -= extra_splash_power_decay * dt;
    my_spray->extra_splash_power[1] -= extra_splash_power_decay * dt;

    if (my_spray->extra_splash_power[0] < 0)
      my_spray->extra_splash_power[0] = 0;
    if (my_spray->extra_splash_power[1] < 0)
      my_spray->extra_splash_power[1] = 0;

    // Add Board Spray
    if (apow)
    {
      SprayPt.life = FX_ParticleD[FX_DEF_BSPRAY1].Life;
      SprayPt.trailpt = &(my_trail_data[my_index].pnt1.pos);
			SprayPt.trail_node = &my_trail_data[my_index];
      SprayPt.vel.x = avec.x * apow;
      SprayPt.vel.y = avec.y * apow + board_spray.bonus_y * apow;
      SprayPt.vel.z = avec.z * apow;

      spraypt_add(&SprayPt, my_spray->control_points_a, &my_spray->num_control_points_a);
    }

    if (bpow)
    {
      SprayPt.life = FX_ParticleD[FX_DEF_BSPRAY1].Life;
      SprayPt.trailpt = &(my_trail_data[my_index].pnt3.pos);
			SprayPt.trail_node = &my_trail_data[my_index];
      SprayPt.vel.x = bvec.x * bpow;
      SprayPt.vel.y = bvec.y * bpow + board_spray.bonus_y * bpow;
      SprayPt.vel.z = bvec.z * bpow;

      spraypt_add(&SprayPt, my_spray->control_points_b, &my_spray->num_control_points_b);
    }
  }
	else
	{
		my_spray->last_apow = 0;
		my_spray->last_bpow = 0;

    my_spray->extra_splash_power[0] = 0;
    my_spray->extra_splash_power[1] = 0;
	}

  // check if we're doing a floater
  if ((board.GetBoardState () == BOARD_FLOAT) && !board.IsGrindingObject ())
  {
    my_spray->floater_interval += dt;

    if (my_spray->floater_interval > floater_interval_time)
    {
      ks_fx_add_splash (FX_DEF_FLOATER, pos, 1.0f);
//      LooseParticles[currentparticle].part.Tex = fx_tex[FX_TEX_FLOATER];
      my_spray->floater_interval -= floater_interval_time;
    }
  }
  else
    my_spray->floater_interval = 0;

  my_spray->last_vec = board.rb->linMom - board.current;
}

void trail::board_update_air (float dt)
{
  entity *board_ptr = my_owner->GetBoard();

  vector3d pt(ZEROVEC);
  add_point (&pt, &pt, &pt, 0, 0, NULL, true);

  if (my_owner->get_board_controller ().GetAirTime () < airdrops_time)
  {
    my_spray->airdrops_interval += dt;

    // Only add a splash at most every 1/60th of a sec (because of slo-mo replay) - rbroner (6/29/02)
    static float last_splash = 0.0f;
    last_splash += dt;
    if(last_splash < 1.0f/60.0f)
      return;

    last_splash = 0.0f;

    if (my_spray->airdrops_interval > airdrops_interval_time)
    {
      vector3d pos;

      pos = board_ptr->get_abs_po().slow_xform (vector3d (0, 0, -0.2));

      ks_fx_add_splash(FX_DEF_AIRDROPS, pos, 1.0f);
//      LooseParticles[currentparticle].part.Scol = (((u_int)((FX_ParticleD[FX_DEF_AIRDROPS].Scol>>24)*(1-my_owner->get_board_controller().GetAirTime ()/airdrops_time))<<24)+(FX_ParticleD[FX_DEF_AIRDROPS].Scol&0xffffff);
//      LooseParticles[currentparticle].move = 1;
//      LooseParticles[currentparticle].part.Tex = fx_tex[FX_DEF_AIRDROPS];
    }
  }
  else
    my_spray->airdrops_interval = 0;
}

void trail::draw ()
{
  nglMaterial Mat;
  float f_color;
  u_int i_color;
  u_int i;

  START_PROF_TIMER( proftimer_render_spray );

  if (my_owner)
  {
    // Draw Board Spray
#ifdef DEBUG
static int draw_spray_a = 1, draw_spray_b = 1;
    if (draw_spray_a)
      spraypt_draw (my_spray->control_points_a, FX_DEF_BSPRAY1, FXD.Spray_Vary1, my_spray->num_control_points_a, 1);
    if (draw_spray_b)
      spraypt_draw (my_spray->control_points_b, FX_DEF_BSPRAY1, FXD.Spray_Vary1, my_spray->num_control_points_b, 1);
#else
    spraypt_draw (my_spray->control_points_a, FX_DEF_BSPRAY1, FXD.Spray_Vary1, my_spray->num_control_points_a, 1);
    spraypt_draw (my_spray->control_points_b, FX_DEF_BSPRAY1, FXD.Spray_Vary1, my_spray->num_control_points_b, 1);
#endif
  }

  STOP_PROF_TIMER( proftimer_render_spray );

  START_PROF_TIMER( proftimer_render_trail );

  // Draw Trail Geometry
  memset( &Mat, 0, sizeof Mat );
  Mat.DetailMap = NULL;
  Mat.LightMap = NULL;
  Mat.Map = fx_tex[FX_TEX_TRAIL];
  if (g_game_ptr->get_beach_id () == BEACH_TEANIGHT)
    Mat.MapBlendMode = NGLBM_BLEND;
  else
    Mat.MapBlendMode = NGLBM_ADDITIVE;
  Mat.Flags = NGLMAT_TEXTURE_MAP | NGLMAT_ALPHA | NGLMAT_BILINEAR_FILTER;
  nglMatrix LocalToWorld;

  // cute
  nglIdentityMatrix (LocalToWorld);

  int index = my_index;
  int backindex = index;
  int back2index = index;
  int frontindex = index;
  int numnodes = 0, numnodesadded = 0;
  bool first = true; // hack to draw the trail when the player is jumping

  for (i = 0; i < my_total_nodes; i++)
  {
    backindex = index;
    index--;
    if (index<0)
      index = my_total_nodes - 1;

    if (index == (int) my_index)
      break;

    if (!my_trail_data[index].valid || !my_trail_data[backindex].valid)
    {
      if (first && !numnodes)
        continue;
      else
        break;
    }
    first = false;

    numnodes++;
  }

  if (numnodes >= 2)
  {
    KSNGL_CreateScratchMesh (2 * numnodes, &Mat, false);
    nglMeshWriteStrip (2 * numnodes);

    index = my_index;
    backindex = index;
    back2index = index;
    frontindex = index;
    first = true;
		float v = 1e10;

//u_int b_color = 0;
    // Create Trail Geometry
    for (i = 0; i < my_total_nodes; i++)
    {
      back2index = index + 1;
      backindex = index;
      index--;

      if (index < 0)
        index = my_total_nodes - 1;

      if (index == (int) my_index)
        break;

      if (!my_trail_data[index].valid || !my_trail_data[backindex].valid)
      {
        if (first && !numnodesadded)
          continue;
        else
          break;
      }
			if (first)
				v = my_trail_data[backindex].uv;
      first = false;

      frontindex = index - 1;
      if (frontindex<0)
        frontindex = my_total_nodes - 1;

      if (back2index>=(int) my_total_nodes)
        back2index = 0;

      f_color = 255.0f * (1.0f - min (my_trail_data[index].age / my_life, 1.0f));
      f_color = min (f_color, 255 * (1.0f - (float)numnodesadded/(float)numnodes));
      //        f_color = 128 * (1 - (float)i/numnodes);
      i_color = NGL_RGBA32 (0xff, 0xff, 0xff, FTOI(f_color));

//if (b_color == 0)
//b_color = 0xff0000;
//i_color = 0x80000000 + b_color;
//b_color >>= 8;

	  assert (v != 1e10);	// else uninitialized (dc 06/07/02)
      if (my_extra)
      {
        if(my_trail_data[back2index].valid && i != 0)
        {
          nglMeshFastWriteVertexPCUV( my_trail_data[backindex].pnt3.pos.x + my_trail_data[backindex].pnt3.norm.x * debug_trail_height,  my_trail_data[backindex].pnt3.pos.y + my_trail_data[backindex].pnt3.norm.y * debug_trail_height, my_trail_data[backindex].pnt3.pos.z + my_trail_data[backindex].pnt3.norm.z * debug_trail_height, i_color, 0.0f, v);
          nglMeshFastWriteVertexPCUV( my_trail_data[backindex].pnt2.pos.x + my_trail_data[backindex].pnt2.norm.x * debug_trail_height,  my_trail_data[backindex].pnt2.pos.y + my_trail_data[backindex].pnt2.norm.y * debug_trail_height, my_trail_data[backindex].pnt2.pos.z + my_trail_data[backindex].pnt2.norm.z * debug_trail_height, i_color, 0.5f, v);
        }
        else // vertex fade the first mesh piece to get ride of hard line
        {
          nglMeshFastWriteVertexPCUV( my_trail_data[backindex].pnt3.pos.x + my_trail_data[backindex].pnt3.norm.x * debug_trail_height,  my_trail_data[backindex].pnt3.pos.y + my_trail_data[backindex].pnt3.norm.y * debug_trail_height, my_trail_data[backindex].pnt3.pos.z + my_trail_data[backindex].pnt3.norm.z * debug_trail_height, NGL_RGBA32 (0xff, 0xff, 0xff, 0), 0.0f, v);
          nglMeshFastWriteVertexPCUV( my_trail_data[backindex].pnt2.pos.x + my_trail_data[backindex].pnt2.norm.x * debug_trail_height,  my_trail_data[backindex].pnt2.pos.y + my_trail_data[backindex].pnt2.norm.y * debug_trail_height, my_trail_data[backindex].pnt2.pos.z + my_trail_data[backindex].pnt2.norm.z * debug_trail_height, NGL_RGBA32 (0xff, 0xff, 0xff, 0), 0.5f, v);
        }
      }
      else
      {
        nglMeshFastWriteVertexPCUV( my_trail_data[backindex].pnt3.pos.x + my_trail_data[backindex].pnt3.norm.x * debug_trail_height,  my_trail_data[backindex].pnt3.pos.y + my_trail_data[backindex].pnt3.norm.y * debug_trail_height, my_trail_data[backindex].pnt3.pos.z + my_trail_data[backindex].pnt3.norm.z * debug_trail_height, i_color, 0.0f,my_trail_data[backindex].uv);
        nglMeshFastWriteVertexPCUV( my_trail_data[backindex].pnt1.pos.x + my_trail_data[backindex].pnt1.norm.x * debug_trail_height,  my_trail_data[backindex].pnt1.pos.y + my_trail_data[backindex].pnt1.norm.y * debug_trail_height, my_trail_data[backindex].pnt1.pos.z + my_trail_data[backindex].pnt1.norm.z * debug_trail_height, i_color, 1.0f, my_trail_data[backindex].uv);
      }
			v -= 1.0f / debug_trail_mod;
      numnodesadded++;
    }

	// Material must be specified at mesh creation time now.  (dc 06/03/02)
//    KSNGL_ScratchSetMaterial(&Mat);
#ifdef TARGET_XBOX
	nglMeshSetSphere(nglVector(0,0,0,0), 0);	// replace by nglMeshCalcSphere, when that's implemented (dc 06/11/02)
#else
    nglMeshCalcSphere();
#endif

    nglRenderParams nglparams;
    memset( &nglparams, 0, sizeof(nglparams) );

    nglparams.Flags |= NGLP_ZBIAS;
    nglparams.ZBias = debug_zbias;

	nglMesh *Mesh = nglCloseMesh();

    if(FXD.Render.Geom_Trail)
      nglListAddMesh( Mesh, LocalToWorld, &nglparams);

    // Create Trail Geometry for the right side
    if (my_extra)
    {
      KSNGL_CreateScratchMesh (2 * numnodes, &Mat, false);
      nglMeshWriteStrip (2 * numnodes);
      numnodesadded = 0;

      index = my_index;
      backindex = index;
      back2index = index;
      frontindex = index;
      first = true;

      for(i=0;i<my_total_nodes;i++)
      {
        back2index = index + 1;
        backindex = index;
        index--;
        if(index<0)
        {
          index = my_total_nodes - 1;
        }
        if(index == (int) my_index)
          break;

        if(!my_trail_data[index].valid || !my_trail_data[backindex].valid)
        {
          if (first && !numnodesadded)
            continue;
          else
  	  	    break;
        }
				if (first)
					v = my_trail_data[backindex].uv;
        first = false;

        frontindex = index - 1;
        if(frontindex<0)
        {
          frontindex = my_total_nodes - 1;
        }

        if(back2index>=(int) my_total_nodes)
          back2index = 0;

        f_color = 255.0f * (1.0f - min (my_trail_data[index].age / my_life, 1.0f));
        f_color = min (f_color, 255 * (1.0f - (float)numnodesadded/(float)numnodes));
        i_color = NGL_RGBA32 (0xff, 0xff, 0xff, FTOI(f_color));

        if(my_trail_data[back2index].valid && i != 0)
        {
          nglMeshFastWriteVertexPCUV( my_trail_data[backindex].pnt2.pos.x + my_trail_data[backindex].pnt2.norm.x * debug_trail_height,  my_trail_data[backindex].pnt2.pos.y + my_trail_data[backindex].pnt2.norm.y * debug_trail_height, my_trail_data[backindex].pnt2.pos.z + my_trail_data[backindex].pnt2.norm.z * debug_trail_height, i_color,  0.5f, v);
          nglMeshFastWriteVertexPCUV( my_trail_data[backindex].pnt1.pos.x + my_trail_data[backindex].pnt1.norm.x * debug_trail_height,  my_trail_data[backindex].pnt1.pos.y + my_trail_data[backindex].pnt1.norm.y * debug_trail_height, my_trail_data[backindex].pnt1.pos.z + my_trail_data[backindex].pnt1.norm.z * debug_trail_height, i_color,  1.0f, v);
        }
        else
        {
          nglMeshFastWriteVertexPCUV( my_trail_data[backindex].pnt2.pos.x + my_trail_data[backindex].pnt2.norm.x * debug_trail_height,  my_trail_data[backindex].pnt2.pos.y + my_trail_data[backindex].pnt2.norm.y * debug_trail_height, my_trail_data[backindex].pnt2.pos.z + my_trail_data[backindex].pnt2.norm.z * debug_trail_height, NGL_RGBA32 (0xff, 0xff, 0xff, 0),  0.5f, v);
          nglMeshFastWriteVertexPCUV( my_trail_data[backindex].pnt1.pos.x + my_trail_data[backindex].pnt1.norm.x * debug_trail_height,  my_trail_data[backindex].pnt1.pos.y + my_trail_data[backindex].pnt1.norm.y * debug_trail_height, my_trail_data[backindex].pnt1.pos.z + my_trail_data[backindex].pnt1.norm.z * debug_trail_height, NGL_RGBA32 (0xff, 0xff, 0xff, 0),  1.0f, v);
        }
				v -= 1.0f / debug_trail_mod;
        numnodesadded++;
      }

      assert (numnodes == numnodesadded);

	// Material must be specified at mesh creation time now.  (dc 06/03/02)
//      KSNGL_ScratchSetMaterial(&Mat);
#ifdef TARGET_XBOX
	  nglMeshSetSphere(nglVector(0,0,0,0), 0);	// replace by nglMeshCalcSphere, when that's implemented (dc 06/11/02)
#else
      nglMeshCalcSphere();
#endif

      nglRenderParams nglparams;
      memset( &nglparams, 0, sizeof(nglparams) );

      nglparams.Flags |= NGLP_ZBIAS;
      nglparams.ZBias = debug_zbias;

	  nglMesh *Mesh = nglCloseMesh();

      if(FXD.Render.Geom_Trail)
        nglListAddMesh( Mesh, LocalToWorld, &nglparams);
    }
  }

  STOP_PROF_TIMER( proftimer_render_trail );
}

void trail::create_big_landing_splash ()
{
  if (my_spray)
  {
    my_spray->extra_splash_power[0] = my_spray->extra_splash_power[1] = extra_splash_power_amount;
  }
}

void trail::create_face_trick_splash (bool left)
{
	if (left)
		my_spray->extra_splash_power[0] = extra_splash_power_amount;
	else
		my_spray->extra_splash_power[1] = extra_splash_power_amount;
}

void trail::create_chophop_splash (vector3d& pos)
{
	ks_fx_add_splash(FX_DEF_CHOPHOPSPLASH, pos, 1.0f);
}

// ============================================================================
// helper functions

void trail::update_surface_point (surface_point_t *sp)
{
  WaveQueryFlags flags = (WaveQueryFlags)(WAVE_HINTSOUGHT | WAVE_NORMALSOUGHT| WAVE_HINTGIVEN | WAVE_REGIONSOUGHT);

  if (sp->vvalid)
    flags = (WaveQueryFlags)(flags | WAVE_VELOCITYGIVEN);

#if defined(TARGET_XBOX)
  WaveFloaterArgs args (flags, &sp->hint, sp->vhint, &sp->pos, &sp->norm, &sp->region);
#else
  WaveFloaterArgs args = { flags, &sp->hint, sp->vhint, &sp->pos, &sp->norm, &sp->region };
#endif /* TARGET_XBOX JIV DEBUG */

  WAVE_TrackFloater(args);
}

void trail::spraypt_update (spray_control_t *SprayControlPts, u_int max)
{
	u_int i;
//	vector3d pos;

	// for camera spray calculation
//	vector3d campos3d = app::inst()->get_game()->get_current_view_camera()->get_rel_position();
//	nglVector campos = {campos3d.x, campos3d.y, campos3d.z, 1};
//	nglVector c, v, proj, perp;
//	float normv;
//	float projlen;
//	float dist_sq;
//	static float spray_on_lens_cutoff_sq = 4;

	for (i = 0; i < max; i++)
	{
		SprayControlPts[i].age += (float)TIMER_GetFrameSec();

		// Check if we should draw
		if((SprayControlPts[i].age > SprayControlPts[i].life) || !SprayControlPts[i].valid || !SprayControlPts[i].trail_node->valid)
		{
			SprayControlPts[i].valid = 0;
			continue;
		}
		// Update Particle Position
		spraypt_pos(&SprayControlPts[i]);

/*
		// Project camera position onto spray line.  Did spray hit camera?
		sceVu0SubVector(c, campos, (nglVector &) SprayControlPts[i].start);
		sceVu0SubVector(v, (nglVector &) SprayControlPts[i].current, (nglVector &) SprayControlPts[i].start);
		normv = sqrtf(sceVu0InnerProduct(v, v));
		sceVu0DivVectorXYZ(v, v, normv);
		projlen = sceVu0InnerProduct(c, v);
		if (projlen < 0 || projlen > normv) continue;
		sceVu0ScaleVectorXYZ(proj, v, projlen);
		sceVu0SubVector(perp, c, proj);
		dist_sq = sceVu0InnerProduct(perp, perp);
		if (dist_sq < spray_on_lens_cutoff_sq)
		{
			REFRACT_SprayEvent(SprayControlPts[i].vel);
		}
*/

/*
vector3d c = campos - SprayControlPts[i].start;
vector3d v = SprayControlPts[i].current - SprayControlPts[i].start;
float normv = sqrtf(dot(v, v));
v /= normv;
float projlen = dot(c, v);
if (projlen < 0 || projlen > normv) continue;
vector3d proj = projlen * v;
vector3d perp = c - proj;
float dist = sqrtf(dot(perp, perp));

static float spray_on_lens_cutoff = 2;
if (dist > spray_on_lens_cutoff) continue;
REFRACT_SprayEvent(SprayControlPts[i].vel);
*/
	}
}

// This function tries to treat the board as an ellipse to get some more
// realistic trail patterns in the water.
void trail::comp_trail_orig (vector3d *pointa, vector3d *cent, vector3d *pointb)
{
  float theta;

  vector3d unit;

  entity *board_ptr = my_owner->GetBoard();

  unit = my_owner->get_board_controller().current - my_owner->get_board_controller().rb->linMom;
  unit = board_ptr->get_abs_po().non_affine_inverse_xform(unit);
  unit.y = 0;
  unit.normalize();

#if !defined(TARGET_PS2)
  assert( unit.is_valid() );

  if( unit.x == 0.0f )
  {
    // avoid divide by 0
    unit.x = 0.0000001f;
  }
#endif /* TARGET_XBOX JIV DEBUG */

  theta = atanf(debug_ellipse_wid/debug_ellipse_len * unit.z / unit.x);

  pointa->x = - debug_ellipse_wid * sinf(theta);
  pointa->y = 0;
  pointa->z = debug_ellipse_len * cosf(theta);

  pointb->x = - debug_ellipse_wid * sinf(theta + PI);
  pointb->y =  0;
  pointb->z = debug_ellipse_len * cosf(theta + PI);

  *cent = (*pointa - *pointb)/2 + *pointb;

  assert( pointa->is_valid() );
  assert( pointb->is_valid() );
  assert( cent->is_valid() );
}

// This will add a point of spray to a list so that it can be updated, as well as
// the appropriate particles and geometry at the time of drawing.
bool trail::spraypt_add (spray_control_t *SC, spray_control_t *ListPtr, int *ListCount)
{
  if (SC->trailpt == ListPtr[*ListCount].trailpt)
    return false;

	*ListCount = 1 + *ListCount;
	if(*ListCount>=MAX_SPRAY_PTS)
		*ListCount=0;

	spraypt_pos(SC);

	ListPtr[*ListCount].age = 0;
	ListPtr[*ListCount].life = SC->life;
	ListPtr[*ListCount].trailpt = SC->trailpt;
	ListPtr[*ListCount].vel = SC->vel;
	ListPtr[*ListCount].start = SC->start;
	ListPtr[*ListCount].current = SC->current;
	ListPtr[*ListCount].valid = 1;
	ListPtr[*ListCount].trail_node = SC->trail_node;

  return true;
}

void trail::spraypt_pos (spray_control_t *SC)
{
	vector3d Grav (0, board_spray.grav, 0);

	SC->start = (*(SC->trailpt) - SC->trail_node->pnt2.pos) * FXD.Trail_Start_Width_Mod + SC->trail_node->pnt2.pos;
	SC->current = SC->start + (SC->vel * SC->age) + (Grav * SC->age * SC->age * 0.5f);
}

static float spray_min_power = 12;
static float spray_age = 0.65f;

bool trail::spray_object (entity *ent) const
{
	if (spray_object_internal (my_spray->control_points_a, my_spray->num_control_points_a, ent))
		return true;

	return spray_object_internal (my_spray->control_points_b, my_spray->num_control_points_b, ent);
}

bool trail::spray_object_internal (spray_control_t *SprayControlPts, int scp_index, entity *ent) const
{
	int index;
	int backindex;
  bool first;
	vector3d p2;
	int i;
  u_int nodes_added = 0;
  u_int nodes = 0;
  vector3d c1 (ent->get_abs_position ());
  float r1;

  r1 = ent->get_radius ();

  if (r1 < 0.01f)
  {
    if (ent->get_colgeom ())
      r1 = ent->get_colgeom ()->get_radius ();

    if (r1 < 0.01f)
    {
		  nglMesh *mesh = ent->get_mesh ();

      if (mesh)
      {
        c1.x += mesh->SphereCenter[0];
        c1.y += mesh->SphereCenter[1];
        c1.z += mesh->SphereCenter[2];
        r1 = mesh->SphereRadius;
      }
    }
  }
static float spray_extra_radius = 2;
r1 += spray_extra_radius;

	for (first = true, index = scp_index, i = 0; i < MAX_SPRAY_PTS; i++)
	{
		backindex = index;
		index--;

		if (index < 0)
			index = MAX_SPRAY_PTS - 1;

		if (index == scp_index)
			break;

		if (!SprayControlPts[index].valid || !SprayControlPts[backindex].valid)
			break;

    if (!SprayControlPts[index].trail_node->valid || !SprayControlPts[backindex].trail_node->valid)
    {
      if (first && !nodes)
        continue;
      else
			  break;
    }
    first = false;

    nodes++;
	}

  for (first = true, index = scp_index, i = 0; i < MAX_SPRAY_PTS; i++)
  {
		backindex = index;
		index--;

		if (index < 0)
			index = MAX_SPRAY_PTS - 1;

		if (index == scp_index)
			break;

		if (!SprayControlPts[index].valid || !SprayControlPts[backindex].valid)
			break;

		if (!SprayControlPts[index].trail_node->valid || !SprayControlPts[backindex].trail_node->valid)
		{
			if (first && !nodes_added)
				continue;
			else
				break;
		}
		first = false;

    if (SprayControlPts[backindex].vel.length2 () > spray_min_power)
    {
			vector3d Grav (0, board_spray.grav, 0);
			vector3d vel (SprayControlPts[backindex].vel);

			vel *= 1 - max (SprayControlPts[backindex].age / spray_age, (float)i/nodes);
			Grav *= 1 - max (SprayControlPts[backindex].age / spray_age, (float)i/nodes);

			float age = SprayControlPts[backindex].age;
			p2 = SprayControlPts[backindex].start + (vel * age) + (Grav * (age * age * board_spray.curve));
      nodes_added++;
		}
		else
		{
			p2 = SprayControlPts[backindex].start;
		}

		c1.y = p2.y = 0;

    if ((c1 - p2).length2 () <= (r1 * r1))
			return true;
	}

	return false;
}

void trail::spraypt_draw(spray_control_t *SprayControlPts, u_int fxindex, float vary, int scp_index, int web)
{
	int index;
	int backindex;
	float f_color;
	u_int i_color;
  u_int nodes = 0;
  int i, j;
  bool first;

	for (first = true, index = scp_index, i = 0; i < MAX_SPRAY_PTS; i++)
	{
		backindex = index;
		index--;

		if (index < 0)
			index = MAX_SPRAY_PTS - 1;

		if (index == scp_index)
			break;

		if (!SprayControlPts[index].valid || !SprayControlPts[backindex].valid)
			break;

    if (!SprayControlPts[index].trail_node->valid || !SprayControlPts[backindex].trail_node->valid)
    {
      if (first && !nodes)
        continue;
      else
			  break;
    }
    first = false;

    nodes++;

    if (web)
    {
      if (SprayControlPts[index].current.y < -2)
        continue;

      nglParticleSystem PS;
      memset (&PS, 0, sizeof (PS));

      PS.Tex = fx_tex[FX_TEX_BSPRAY_P];
      PS.Rvel1[0]= - SprayControlPts[index].vel.x*vary;
      PS.Rvel1[1]= - SprayControlPts[index].vel.y*vary;
      PS.Rvel1[2]= - SprayControlPts[index].vel.z*vary;

      vector3d pos1 = SprayControlPts[index].start;
      vector3d pos2 = SprayControlPts[backindex].start;

      prepare_part(fxindex, &PS, 1.0f, 1.0f, pos1, pos2, 0.001f, SprayControlPts[index].age);

      pos1 = SprayControlPts[index].start;
      pos2 = SprayControlPts[index].current;

      // Draw Particles for the spray
      PS.Rpos2[0] = (pos2.x - pos1.x) * 0.5f;
      PS.Rpos2[1] = (pos2.y - pos1.y) * 0.5f;
      PS.Rpos2[2] = (pos2.z - pos1.z) * 0.5f;
      PS.Seed = (int)SprayControlPts[index].seed;
      PS.MaxSize = 75000;

      if (FXD.Render.Spray_Trail)
        nglListAddParticle (&PS);
    }
  }

  // Draw Web for spray
  if (web && (nodes > 1))
  {
    nglMaterial Mat;
    memset (&Mat, 0, sizeof Mat);
    u_int nodes_added;

#define SPRAY_STRIPS 3

    Mat.Map = fx_tex[FX_TEX_BSPRAY_G];
    Mat.DetailMap = NULL;
    Mat.LightMap = NULL;
    Mat.MapBlendMode = NGLBM_BLEND;
    Mat.Flags = NGLMAT_TEXTURE_MAP | NGLMAT_ALPHA | NGLMAT_BILINEAR_FILTER;
    nglMatrix LocalToWorld;
    nglIdentityMatrix (LocalToWorld);

    // NGL is limited to 36 verts per strip and I don't feel like breaking up the strip
#define MAX_NGL_STRIP 64
    if (nodes > MAX_NGL_STRIP)
      nodes = MAX_NGL_STRIP;

    KSNGL_CreateScratchMesh (2 * SPRAY_STRIPS * nodes, &Mat, false);

    for (j = 0; j < SPRAY_STRIPS; j++)
    {
      nglMeshWriteStrip (2 * nodes);
      float mycount = 0;
      nodes_added = 0;

      for (first = true, index = scp_index, i = 0; i < MAX_SPRAY_PTS; i++)
      {
        backindex = index;
        index--;

        if (index < 0)
          index = MAX_SPRAY_PTS - 1;

        if (index == scp_index)
          break;

        if (!SprayControlPts[index].valid || !SprayControlPts[backindex].valid)
          break;

        if (!SprayControlPts[index].trail_node->valid || !SprayControlPts[backindex].trail_node->valid)
        {
          if (first && !nodes_added)
            continue;
          else
            break;
        }
        first = false;

        if (i == MAX_NGL_STRIP)
          break;

				if (first)
					mycount = SprayControlPts[backindex].trail_node->uv;

static u_int spray_col[6][3] = { { 0xde, 0xde, 0xde }, { 0xde, 0xde, 0xde }, { 0xde, 0xde, 0xde }, { 0xde, 0xde, 0xde }, { 0xde, 0xde, 0xde }, { 0xde, 0xde, 0xde } };
static u_int spray_alpha[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

        vector3d mag = SprayControlPts[index].current - SprayControlPts[backindex].current;

        float myu1 = mycount;
        if (i != 0)
          mycount += mag.length() * board_spray.tex_size;

        vector3d p1, p2;
        if (SprayControlPts[backindex].vel.length2 () > spray_min_power)
        {
          vector3d Grav (0, board_spray.grav, 0);
          vector3d vel (SprayControlPts[backindex].vel);

          vel *= 1 - max (SprayControlPts[backindex].age / spray_age, (float)i/nodes);
          Grav *= 1 - max (SprayControlPts[backindex].age / spray_age, (float)i/nodes);

          float age = SprayControlPts[backindex].age * j / SPRAY_STRIPS;

          p1 = SprayControlPts[backindex].start + (vel * age) + (Grav * (age * age * board_spray.curve));
          age = SprayControlPts[backindex].age * (j + 1) / SPRAY_STRIPS;
          p2 = SprayControlPts[backindex].start + (vel * age) + (Grav * (age * age * board_spray.curve));
        }
        else
        {
          p1 = SprayControlPts[backindex].start;
          p2 = SprayControlPts[backindex].start;
        }

        float v1, v2;
        v1 = (float) j / SPRAY_STRIPS;
        v2 = (float) (j + 1) / SPRAY_STRIPS;

        f_color = 1 - 0.25f * min (SprayControlPts[index].age/spray_age, 1.f);// - ((float)j / SPRAY_STRIPS) * 0.5f;
        i_color = NGL_RGBA32 (spray_col[j][0], spray_col[j][1], spray_col[j][2], FTOI(spray_alpha[j] * f_color));
        nglMeshFastWriteVertexPCUV (p1.x,  p1.y,  p1.z, i_color, myu1, v1);

        f_color = 1 - 0.25f * min (SprayControlPts[index].age/spray_age, 1.f);// - ((float)(j + 1) / SPRAY_STRIPS) * 0.5f;
        i_color = NGL_RGBA32 (spray_col[j+1][0], spray_col[j+1][1], spray_col[j+1][2],  FTOI(spray_alpha[j+1] * f_color));
        nglMeshFastWriteVertexPCUV (p2.x,  p2.y,  p2.z, i_color, myu1, v2);
        nodes_added++;
      }
    }

    assert (nodes == nodes_added);

	// Material must be specified at mesh creation time now.  (dc 06/03/02)
//    KSNGL_ScratchSetMaterial (&Mat);
#ifdef TARGET_XBOX
	nglMeshSetSphere(nglVector(0,0,0,0), 0);	// replace by nglMeshCalcSphere, when that's implemented (dc 06/11/02)
#else
    nglMeshCalcSphere ();
#endif
	nglMesh *Mesh = nglCloseMesh();

    if (FXD.Render.Geom_Spray)
      nglListAddMesh( Mesh, LocalToWorld, NULL);
  }
}

// This function returns the power that the board should return and is called once
// for the left side of the board and once for the right.
float trail::comp_part_bspray (vector3d *Vec, vector3d *Pos, float dt, float scale)
{
  SurfBoardObjectClass& board = my_owner->get_board_controller();
  vector3d edge_u;
  vector3d vel;
  vector3d lastvel_u;
  float dmag;
  float dangle;
  vector3d vel_u;
  float power;

  if (board.WipedOut ())
    return 0;

  edge_u =  *Vec;

  lastvel_u = my_spray->last_vec;

  vel = board.rb->linMom - board.current;

  vel_u = vel;
  vel_u.normalize();

  lastvel_u.normalize();

  dmag = (vel.length() - my_spray->last_vec.length());
  if (dmag > 0)
    dmag = 0;
  else
    dmag *= -1.0f;

//	dangle = dot(lastvel_u, vel_u);

  {
    vector3d v1 = lastvel_u;
    v1.y = 0;
    vel_u.y = 0;
    vel_u.normalize();
    v1.normalize();

  	dangle = dot(v1, vel_u);
  }

  if (dangle > 0)
	  dangle = 1 - dangle;
  else
	  dangle= 0;

	dangle /= dt;
	dmag /= dt;

  power = 0;
  if (dot (edge_u, lastvel_u) > 0)
  {
    power = dangle * board_spray.angle_scale + dmag * board_spray.vel_scale * dangle;
    power *= scale;

    if ((board.GetTurnType () == CARVE_TURN) && (my_owner->CtrlEvent(PAD_CARVE)))
    {
      static float carve_scale = 5;
      power *= carve_scale;
    }

    if (((board.GetTurnType () == TAILSLIDE_TURN) || (board.GetTurnType () == GRABSLIDE_TURN)) && (my_owner->CtrlEvent(PAD_SLIDE)))
    {
      static float slide_scale = 5;
      power *= slide_scale;
    }

    if ((board.GetTurnType () == SNAP_TURN) && (my_owner->CtrlEvent(PAD_SNAP)))
    {
static float snap_scale = 25;
      power *= snap_scale;
    }
  }

//  power *= dt;

  if (power > FXD.Spray_Max_Thresh)
	  power = FXD.Spray_Max_Thresh;

  if (power < FXD.Spray_Min_Thresh)
	  power = FXD.Spray_Min_Thresh;

  return(power);
}

