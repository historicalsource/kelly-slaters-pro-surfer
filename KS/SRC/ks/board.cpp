/*
Notes on the conversion of this module:

    I (PTA) have done the initial ocnversion.  Most of the changes are noted with PTA initials on or before 4/30/01.

    One of the major changes is that the IRC coordinate system has:
      XY horizontal with Z vertical, and positive X is forward.
    In Treyarch's:
      XZ horizontal with Y vertical, and positive Z is forward.
    Our system is right handed.  Hopefully IRC's is too.
    Changes to implement this are labeled "COORDSWAP".

*/

/********** include files **********/

#include "global.h"

#include "board.h"
#include "colmesh.h"	// For class cg_mesh
#include "commands.h"
#include "entity.h"
#include "inputmgr.h"
#if defined(TARGET_XBOX) || defined(TARGET_GC)
#include "floatobj.h"
#include "board.h"
#include "colmesh.h"
#include "conglom.h"
#include "beach.h"
#include "hwrasterize.h"
#include "wds.h"
#include "MusicMan.h"
#include "ks_camera.h"
#endif /* TARGET_XBOX JIV DEBUG */
#include "kellyslater_controller.h"
#include "wave.h"
#include "msgboard.h"
#include "beachdata.h"	// For per-beach parameters
#include "ksfx.h"       // for ks_fx_reset
#include "FrontEndManager.h"
#include "SFXEngine.h"
#include <stdio.h>
#include "VOEngine.h"
#include "water.h"
#include "wds.h"
#include "trail.h"
//#include "WipeTransition.h"
#include "timer.h"
#include "wavedata.h"
#include "wipeoutdata.h"

//////////////////////////////////////////////////////////////////////////////////////////////

vector3d node_offset(0.0f, 0.0f, 0.2f);

//extern ScoringManager scoreManager;

float world_gravity = 1.0f;
int use_debug_vals = 0;
float air_gravity = 20.0f;
float force_mod = 0.4f;
float water_friction = 0.1f;
float skag_friction = 4.0f;
float stick_factor = 2.5f;
#define CONTACT_EPSILON 0.05f
#define AIR_GRAV_MOD 0.72f
#define LAUNCH_VELOCITY_SCALE 0.59f
#define GRIND_DISMOUNT_VELOCITY_SCALE 0.80f
#define CHOP_HOP_VELOCITY_SCALE 0.75f
float YAW_ACC_ON_WAVE = 300.0f;
#define MAX_YAW_VEL_SCALE 21.0f
#define MAX_SURF_VELOCITY 7.0f
#define MIN_SURF_VELOCITY 0.25f
#define MAX_STALL_VELOCITY 10.0f
#define MAX_STALL_ACCELERATION 12.0f
#define TAILSLIDE_FRICTION_MOD 0.22f
#define WASH_FRICTION_MOD 0.22f
float DOWN_FACE_FRIC_MOD = 15.0f;

float DUCKDIVE_MAX_VEL = 4.0f;
float DUCKDIVE_ACC = 4.0f;

#define MAX_FLOAT_SPEED 7.0f
#define MIN_FLOAT_SPEED 3.3f
#define FLOAT_OLLIE_INCREMENT 1.5f
float FLOATER_DECELERATION = 0.6f;
float dec_percent = 0.5f;

// Rescaling values for tuning
float KS_TURN_SCALE = 2000.0;

extern float WAVE_CURRENT_MOD;

extern int	SET_RB_PO;
extern int	SET_OWNER_PO;
extern int	SET_BOARD_PO;


//\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /
//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
// \/  \/  \/  \/  \/  \/  \/  \/  \/  \/  \/  \/  \/  \/  \/  \/  \/  \/  \/  \/  \/  \/  \/


// PTA 4/29/01
//static float WaveBoxScale = 0.640;

const float SurfBoardObjectClass::CLOCKTIME_WIPEOUT = 10.0f;

const float SurfBoardObjectClass::LANDING_ANGLE_PERFECT	= 0.9781476f;	// 12 degrees
const float SurfBoardObjectClass::LANDING_ANGLE_REGULAR	= 0.866f;	// 30 degrees
const float SurfBoardObjectClass::LANDING_ANGLE_SLOPPY	= 0.5f;//0.707f;	// 45 degrees
const float SurfBoardObjectClass::LANDING_ANGLE_JUNK	= 0.5f;		// 60 degrees

const int SurfBoardObjectClass::LANDING_FLAG_PERFECT	= 0x01;
const int SurfBoardObjectClass::LANDING_FLAG_SLOPPY		= 0x02;
const int SurfBoardObjectClass::LANDING_FLAG_JUNK		= 0x04;

const int SurfBoardObjectClass::LANDING_FLAG_FAKEY		= 0x10;

SurfBoardObjectClass::SurfBoardObjectClass() // PTA 4/23/01 : MeshObjectClass()
{
		// These weren't being initialized. They'd get deep into the code as nan
	turnDegree=1.0f;
	grind_speed=1.0f;
	max_wipeout_time=1.0f;

	rb = NULL;
	state = BOARD_NONE;
	this->curLeanAngle = 0.0f;
    this->leanEaseIn = 0.0f;
	this->inAirFlag = true;

	front_hint_valid = false;
	center_hint_valid = false;
  tube_hint_valid = false;
	TurnType = 0;
	wiped_out=false;

	grind_entity = NULL;
	grind_object = NULL;

	last_f = 1.0f;
	init_grind = false;
	grind_ollie = false;
	grind_jump = false;
	roof_jump = false;

	float_speed = MAX_FLOAT_SPEED;
  air_timer = 0.0f;

  // create rigidbody
	rb = NEW PhysicsObjectClass;

	ground_turn = 0.0f;
	abs_ground_turn = 0.0f;

  wiped_out = false;
}

SurfBoardObjectClass::~SurfBoardObjectClass()
{
  delete rb;
}


int SurfBoardObjectClass::Init()
{
	if(!this->rb)
		return 0;

  props_forwardForce = 17.0f;

  rb->SetMass(1.0f);

  vector3d axis (0.0f, 0.0f, 1.0f); // look at
  forwardDir = axis;
  forward.Init(axis);

	this->turnProps[REGULAR_TURN].maxLeanAngle = 45.0f;
	this->turnProps[REGULAR_TURN].maxTurnAngle = 4.0f;
	this->turnProps[HARD_REGULAR_TURN].maxLeanAngle = 52.0f;
	this->turnProps[HARD_REGULAR_TURN].maxTurnAngle = 6.0f;
	this->turnProps[GRAB_TURN].maxLeanAngle = 50.0f;
	this->turnProps[GRAB_TURN].maxTurnAngle = 9.5f;
	this->turnProps[HARD_GRAB_TURN].maxLeanAngle = 60.0f;
	this->turnProps[HARD_GRAB_TURN].maxTurnAngle = 6.5f;
	this->turnProps[TAILSLIDE_TURN].maxLeanAngle = 3.0f;
	this->turnProps[TAILSLIDE_TURN].maxTurnAngle = 12.0f;
	this->turnProps[GRABSLIDE_TURN].maxLeanAngle = 3.0f;
	this->turnProps[GRABSLIDE_TURN].maxTurnAngle = 12.0f;
	this->turnProps[HARD_TAILSLIDE_TURN].maxLeanAngle = 3.0f;
	this->turnProps[HARD_TAILSLIDE_TURN].maxTurnAngle = 8.0f;
	this->turnProps[SNAP_TURN].maxLeanAngle = 0.0f;
	this->turnProps[SNAP_TURN].maxTurnAngle = 9.5f;
	this->turnProps[TAILCHUCK_TURN].maxLeanAngle = 0.0f;
	this->turnProps[TAILCHUCK_TURN].maxTurnAngle = 13.0f;
	this->turnProps[REBOUND_TURN].maxLeanAngle = 0.0f;
	this->turnProps[REBOUND_TURN].maxTurnAngle = 9.5f;
	this->turnProps[LAYBACK_SLIDE_TURN].maxLeanAngle = 0.0f;
	this->turnProps[LAYBACK_SLIDE_TURN].maxTurnAngle = 11.0f;
	this->turnProps[GOUGE_TURN].maxLeanAngle = 0.0f;
	this->turnProps[GOUGE_TURN].maxTurnAngle = 11.0f;
	this->turnProps[HARD_SNAP_TURN].maxLeanAngle = 65.0f;
	this->turnProps[HARD_SNAP_TURN].maxTurnAngle = 12.0f;
	this->turnProps[REVERT_CUTBACK_TURN].maxLeanAngle = 0.0f;
	this->turnProps[REVERT_CUTBACK_TURN].maxTurnAngle = 9.5f;
	this->turnProps[TRIM_TURN].maxLeanAngle = 8.0f;
	this->turnProps[TRIM_TURN].maxTurnAngle = 2.5f;
	this->turnProps[HARD_TRIM_TURN].maxLeanAngle = 8.0f;
	this->turnProps[HARD_TRIM_TURN].maxTurnAngle = 3.0f;
	this->turnProps[TRICK_SPIN].maxLeanAngle = 16.5f;
	this->turnProps[TRICK_SPIN].maxTurnAngle = 15.0f;
	this->turnProps[HARD_TRICK_SPIN].maxLeanAngle = 33.0f;
	this->turnProps[HARD_TRICK_SPIN].maxTurnAngle = 16.0f;
	this->turnProps[CARVE_TURN].maxLeanAngle = 59.0f;
	this->turnProps[CARVE_TURN].maxTurnAngle = 7.5f;
	this->turnProps[LIE_TURN].maxLeanAngle = 20.0f;
	this->turnProps[LIE_TURN].maxTurnAngle = 5.0f;

	this->maxJumpVelocity = 20.0f;
	this->maxLeanAngle = 45.0f;
	this->maxTurnAngle = 11.0f;

	return 1;
}

void SurfBoardObjectClass::InitConstants()
{
	// load constants
	extern float water_friction;
	extern float skag_friction;
	extern float stick_factor;
	extern float speed_factor;
	extern float carving_factor;

	int curbeach = g_game_ptr->get_beach_id ();
	water_friction = BeachDataArray[curbeach].water_friction;
	skag_friction = BeachDataArray[curbeach].skag_friction;
	stick_factor = BeachDataArray[curbeach].stick_force;
	speed_factor = BeachDataArray[curbeach].board_speed;
	carving_factor = BeachDataArray[curbeach].board_carving;

	// hack to allow artists to load a model and play animations
	stringx play_model = os_developer_options::inst()->get_string (os_developer_options::STRING_PLAY_MODEL);
	stringx play_anim = os_developer_options::inst()->get_string (os_developer_options::STRING_PLAY_ANIM);

	if ((play_model != "") && (play_anim != ""))
	{
		filespec spec(play_model);
		stringx mesh_path = spec.path;
		nglSetMeshPath( (char *)mesh_path.c_str() );
		stringx texture_path = spec.path;
		// trim "entities" off of path
		texture_path.to_lower();
		if( texture_path.find( "entities" ) != stringx::npos )
		{
			stringx::size_type last_slash        = texture_path.rfind( '\\' );
			stringx::size_type penultimate_slash = texture_path.rfind( '\\', last_slash-1 );
			texture_path = texture_path.substr( 0, penultimate_slash+1 );
		}
		stringx anim_path = texture_path;
		anim_path += "\\ANIMATIONS\\";
		texture_path += os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR);
		texture_path += "\\";
		nglSetTexturePath( (char *)texture_path.c_str() );

		entity *ent = g_entity_maker->create_entity_or_subclass (play_model, entity_id::make_unique_id (), po_identity_matrix, empty_string, entity::ACTIVE, NULL);
		ent->set_rel_position (vector3d (0, 5, -10));
		ent->set_visible (true);

		stringx anim = anim_path + play_anim;

		if (file_finder_exists (anim, entity_track_tree::extension ()))
		{
			anim += entity_track_tree::extension ();
			ent->load_anim (anim);
		}
		else
		{
			nglPrintf ("Missing animation: %s", anim.c_str());
			assert (false);
		}

		if (ent->has_physical_ifc ())
			ent->physical_ifc ()->disable ();

		ent->play_anim (ANIM_PRIMARY, play_anim, 0.3f, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
	}
}

void SurfBoardObjectClass::OnNewWave()
{
	lip_vec = *WAVE_GetMarker(WAVE_MarkerLipMark13) - *WAVE_GetMarker(WAVE_MarkerLipMark11);
	lip_vec -= YVEC*dot(YVEC, lip_vec);
	lip_vec.normalize();
	int curbeach = g_game_ptr->get_beach_id ();
	if (BeachDataArray[curbeach].bdir)
		lip_normal = cross(YVEC, lip_vec);
	else
		lip_normal = cross(lip_vec, YVEC);

	get_ks_controller()->ResetPhysics();
	//ResetPhysics();	// ??? Do we want this here? (dc 01/24/02)
}

int SurfBoardObjectClass::Terminate()
{
	// TODO
	return 1;
}

int SurfBoardObjectClass::Load(char *name)
{
	// calulate inertia tensor
	float min_x = 100000.0f, min_y = 100000.0f, min_z  = 100000.0f;
	float max_x = -100000.0f, max_y = -100000.0f, max_z = -100000.0f;

	x_extent = fabs(max_x - min_x);
	y_extent = fabs(max_y - min_y);
	z_extent = fabs(max_z - min_z);

	//this->rb->SetVolume(x_extent * 0.5f, y_extent  * 0.5f, z_extent  * 0.5f);

	return 1;
}

static void fing_grind_cface (cg_mesh *colgeom, cface *hit_face)
{
  int half_idx = colgeom->get_num_verts () / 2 - 1;
  int i, low = 0, count = 0;

  // we assume that the bottom vertices (the grind path) of the collision geometry will come first on the mesh.
  // (bad assumption but anything better would required changing the exporter).
  //
  // 4  5  6  7
  // +--+--+--+
  // |\ |\ |\ |
  // | \| \| \|
  // +--+--+--+
  // 0  1  2  3
  //
  for (i = 0; i < 3; i++)
    if (hit_face->get_vert_ref (i) <= half_idx)
      count++;

  if (count == 2)
    return;

  if (count != 1)
  {
    // someone screwed up
    nglPrintf ("ERROR: Grinding geometry is wrong, ask for help on how to do it.\n");
    assert (false);
  }

  for (i = 0; i < 3; i++)
    if (hit_face->get_vert_ref (i) <= half_idx)
    {
      low = hit_face->get_vert_ref (i);
      break;
    }

  // find the "opposite face" of this triangle
  for (cface_vect::const_iterator it = colgeom->get_cfaces ().begin (); it != colgeom->get_cfaces ().end (); it++)
  {
    count = 0;

    for (i = 0; i < 3; i++)
      if ((*it).get_vert_ref (i) == low)
      {
        count++;
        break;
      }

    // another bad assumption on the vertex order...
    for (i = 0; i < 3; i++)
      if ((*it).get_vert_ref (i) == low - 1)
      {
        count++;
        break;
      }

    if (count == 2)
    {
      *hit_face = (*it);
      break;
    }
  }
}

bool SurfBoardObjectClass::check_entity_grind (entity *ent)
{
  entity *board = ((conglomerate *)my_board_model)->get_member("BOARD");
  cg_mesh *colgeom;
  vector3d p1, p2;
  cface hit_face;
  int vert_idx, next_idx, half_idx;

  colgeom = (cg_mesh *) (ent->get_colgeom());

  if (colgeom == NULL)
    return false;

  hit_list.clear ();

  if (!g_world_ptr->entity_entity_collision_check (board, ent, 0, &hit_face))
    return false;

  half_idx = colgeom->get_num_verts () / 2 - 1;

  // see this function for some bad hacks and excuses... I mean... explanations
  fing_grind_cface (colgeom, &hit_face);

  if (hit_face.get_vert_ref (0) <= half_idx)
  {
    vert_idx = hit_face.get_vert_ref(0);

    if (hit_face.get_vert_ref (1) <= half_idx)
      next_idx = hit_face.get_vert_ref (1);
    else
      next_idx = hit_face.get_vert_ref (2);
  }
  else
  {
    vert_idx = hit_face.get_vert_ref (1);
    next_idx = hit_face.get_vert_ref (2);
  }

  p1 = ent->get_abs_po ().slow_xform (colgeom->get_raw_vert_ptr (vert_idx)->get_point ());
  p2 = ent->get_abs_po ().slow_xform (colgeom->get_raw_vert_ptr (next_idx)->get_point ());

  grind_vector = p2 - p1;

  if (dot (grind_vector, rb->linMom - current) < 0)
  {
    vector3d swap = p1;
    int i = vert_idx;

    p1 = p2;
    p2 = swap;

    vert_idx = next_idx;
    next_idx = i;

    grind_vector = p2 - p1;
  }

  grind_vector.normalize ();

  grind_idx = vert_idx;
  if (next_idx > vert_idx)
    grind_forward = true;
  else
    grind_forward = false;

  grind_percent = 0;
  grind_speed = rb->linMom.length ();

  if (grind_speed < 0.1)
    grind_speed = 0.1;
  else if (grind_speed > 0.8)
    grind_speed = 0.8;

  return true;
}

// since there's no do_kellyslater_stuff () anymore...
bool SurfBoardObjectClass::do_grinding_stuff (float dt)
{
  cg_mesh *colgeom;
  vector3d p1, p2;
  int half_idx;

  if (ksctrl->CtrlEvent(PAD_GRIND) && (grind_entity == NULL))
  {
    check_for_grinding ();

	  // Begin grind.
    if (grind_entity != NULL)
      get_ks_controller()->StartGrind (grind_vector);
  }

  if (grind_entity == NULL)
    return false;

  colgeom = (cg_mesh *) (grind_entity->get_colgeom());
  half_idx = colgeom->get_num_verts () / 2 - 1;

  // do the grinding physics (if you could call it physics)
  p1 = grind_entity->get_abs_po ().slow_xform (colgeom->get_raw_vert_ptr (grind_idx)->get_point ());
  if (grind_forward)
    p2 = grind_entity->get_abs_po ().slow_xform (colgeom->get_raw_vert_ptr (grind_idx + 1)->get_point ());
  else
    p2 = grind_entity->get_abs_po ().slow_xform (colgeom->get_raw_vert_ptr (grind_idx - 1)->get_point ());

  grind_vector = p2 - p1;
  rb->linMom = ZEROVEC;
  rb->pos = p1 + grind_vector * grind_percent;
  my_board->set_rel_po (rb->my_po);
  my_board->set_rel_position (rb->pos);

  grind_percent += grind_speed / grind_vector.length ();

  bool end = false;

  CollideCallStruct collide (my_board->get_abs_position (), NULL, NULL, NULL, NULL);
  WAVE_CheckCollision (collide, false, false, false);

  if (collide.position.y > my_board->get_abs_position ().y)
  {
    end = true;
  }

  while (grind_percent > 1)
  {
    grind_percent -= 1;

    if (grind_forward)
      grind_idx++;
    else
      grind_idx--;

    if (((grind_idx == 0) && !grind_forward) || ((grind_idx == half_idx) && grind_forward))
    {
      end = true;
      break;
    }
  }

  if (end)
  {
    // fun ends here, go back to regular surfing.
    ksctrl->set_state (STATE_FREEFALL);
    ksctrl->set_super_state (SUPER_STATE_AIR);
    state = BOARD_NONE;
    grind_entity = NULL;
    grind_object = NULL;
    //get_ks_controller ()->float_meter.End ();
  }

  return true;
}

void SurfBoardObjectClass::check_for_grinding ()
{
	water_object *	currObj;
	entity *		ent;
	entity *		boardEntity = ((conglomerate *)my_board_model)->get_member("BOARD");
	nglMesh *		mesh;
	vector3d		c1(0.0f), c2(0.0f);
	float			r1, r2;

	// Compute board's sphere.
	r1 = boardEntity->get_radius();
	if (r1 < 0.01f)
	{
		if (boardEntity->get_colgeom())
			r1 = boardEntity->get_colgeom()->get_radius();

		if (r1 < 0.01f)
		{
			mesh = boardEntity->get_mesh ();

			if (mesh)
			{
				c1.x += mesh->SphereCenter[0];
				c1.y += mesh->SphereCenter[1];
				c1.z += mesh->SphereCenter[2];
				r1 = mesh->SphereRadius;
			}
		}
	}

	// Check each object in the beach.
	for (currObj = (water_object *) g_beach_ptr->get_object(0); currObj != NULL; currObj = (water_object *) currObj->next)
	{
		// Ignore non-physical objects.
		if (!currObj->is_physical())
			continue;

    if (!currObj->is_grindable ())
      continue;

    ent = currObj->get_entity();

    // Compute object's sphere.
    r2 = ent->get_radius();
    if (r2 < 0.01f)
    {
      if (ent->get_colgeom())
        r2 = ent->get_colgeom()->get_radius();

      if (r2 < 0.01f)
      {
        mesh = ent->get_mesh();

        if (mesh)
        {
          c2.x += mesh->SphereCenter[0];
          c2.y += mesh->SphereCenter[1];
          c2.z += mesh->SphereCenter[2];
          r2 = mesh->SphereRadius;
        }
      }
    }

    // Perform sphere-sphere collision check first.
    if ((c1 - c2).length2 () > (r1 + r2) * (r1+ r2))
      continue;

    // If this object's entity is a conglomerate, then we need to check all of its GRIND members.
		if (currObj->get_entity()->get_flavor() == ENTITY_CONGLOMERATE)
		{
			for (int i = 1; ; i++)
			{
				char member[32];
				sprintf (member, "GRIND%.2d", i);

				ent = ((conglomerate *)currObj->get_entity())->get_member(member);

				if (ent == NULL)
					break;

				// Perform mesh-mesh collision check.
				if (check_entity_grind (ent))
				{
					// Save object and entity that we are going to grind.
					grind_entity = ent;
					grind_object = currObj;

          return;
				}
			}
		}
	}

	grind_entity = NULL;
	grind_object = NULL;
}

float strafe_factor = 2.0f;

void SurfBoardObjectClass::CalculateAirTurn(float amnt)
{
	air_turn += amnt;
	if (air_turn > PI)
	{
		air_turn -= PI;
		ksctrl->get_my_scoreManager().UpdateLastSeries(ScoringManager::ATTR_NUM_SPINS_DELTA, 1);
	}
	else if (air_turn < -PI)
	{
		air_turn += PI;
		ksctrl->get_my_scoreManager().UpdateLastSeries(ScoringManager::ATTR_NUM_SPINS_DELTA, 1);
	}
}

bool SurfBoardObjectClass::DoingFaceTurn(void)
{
	return ((TurnType == SNAP_TURN) || (TurnType == REVERT_CUTBACK_TURN) || (TurnType == TAILCHUCK_TURN)
			|| (TurnType == REBOUND_TURN) || (TurnType == LAYBACK_SLIDE_TURN) || (TurnType == GOUGE_TURN)
			|| (TurnType == HARD_SNAP_TURN));
}


float TURN_MOD = 100.0f;
float ACC_LIMIT = 50.0f;
float SLOWDOWN_ANGLE = 8.2f;
void SurfBoardObjectClass::Turn(BoardStateEnum st, float degree, float dt, float leanEaseIn)
{
  if (!wiped_out)
  {
	//  The tube uses different physics, so roll instead of turn.
	if (ksctrl->get_super_state() == SUPER_STATE_IN_TUBE)
	{
		if (!BeachDataArray[g_game_ptr->get_beach_id ()].bdir)
			degree = -degree;

		vector3d vel = this->rb->linMom / this->rb->mass;
		if (st == BOARD_TURN_LEFT)
			vel.z -= degree*strafe_factor*(20.0f*dt);
		else
			vel.z += degree*strafe_factor*(20.0f*dt);

		rb->linMom = vel*this->rb->mass;  //  Now set this based on whatever changes we've made.

		this->turnDegree = 0.0;
		this->leanEaseIn = leanEaseIn;

		return;
	}

	this->turnDegree = degree;
    this->leanEaseIn = leanEaseIn;

	if (this->state != st)
	{
		ground_turn = 0.0f;
		abs_ground_turn = 0.0f;
	}

	if (degree == 0.0f)
	{
		this->state = BOARD_NONE;
		omega = 0.0f;
		return;
	}

	bool doing_face_turn = DoingFaceTurn();

	po turn;
//    vector3d up (rb->my_po.get_y_facing ());
    vector3d vel = this->rb->linMom / this->rb->mass;
    float scale = vel.length () * 0.3f;

    if (scale > 1.5f)
      scale = 1.5f;
	else
		scale = 1.0f;

	if (doing_face_turn)
		scale = 1.0f;

	if ((TurnType == GRABSLIDE_TURN) || (TurnType == TAILSLIDE_TURN))
		scale = 1.35f;


	float turn_amt;
	if (TurnType == TRICK_SPIN)
	{
		if (st == BOARD_TURN_RIGHT)
			degree = -1.0f*degree;

		float air_turn_mod = 30.0f*ksctrl->GetAttribs(ATTRIB_ROTATE)*this->maxTurnAngle;
		omega += 875.0f*degree*dt*ksctrl->GetAttribs(ATTRIB_ROTATE);
		if (omega > air_turn_mod)
			omega = air_turn_mod;
		if (omega < -air_turn_mod)
			omega = -air_turn_mod;

		turn_amt = omega*dt;
		turn.set_rot (YVEC, DEG_TO_RAD((turn_amt)));
		rb->my_po = (turn * rb->my_po);
		this->state = st;
		//air_turn += fabs(turn_amt);
		CalculateAirTurn(DEG_TO_RAD(turn_amt));
		return;
	}
	else if (!doing_face_turn && (fabs(omega) < ACC_LIMIT))
	{
		if (st == BOARD_TURN_RIGHT)
		{
			if (this->state == BOARD_TURN_LEFT)
				omega = 0.0f;

			degree = -1.0f*degree;
		}
		else if (this->state == BOARD_TURN_RIGHT)
			omega = 0.0f;

		if ((fabs(ground_turn) > SLOWDOWN_ANGLE) && (omega != 0.0f))
		{
			omega = (omega/fabs(omega))*TURN_MOD;
		}
		else
			omega += YAW_ACC_ON_WAVE*degree*dt;

		if (omega > MAX_YAW_VEL_SCALE*this->maxTurnAngle*scale)
			omega = MAX_YAW_VEL_SCALE*this->maxTurnAngle*scale;
		if (omega < -MAX_YAW_VEL_SCALE*this->maxTurnAngle*scale)
			omega = -MAX_YAW_VEL_SCALE*this->maxTurnAngle*scale;

		if ((fabs(ground_turn) < SLOWDOWN_ANGLE) && ((current_region == WAVE_REGIONLIP)
		|| (current_region == WAVE_REGIONCHIN) || (current_region == WAVE_REGIONLIP2)))
		{
			omega *=1.30f;
		}

		turn_amt = omega*dt;
		float t_amt = DEG_TO_RAD(turn_amt);
		turn.set_rot (YVEC, t_amt);
		rb->my_po = (turn * rb->my_po);
		this->state = st;
		abs_ground_turn += fabs(t_amt);
		ground_turn += t_amt;
		return;
	}

	omega = scale * carving_factor * this->maxTurnAngle * degree * KS_TURN_SCALE/60.0f;
	if ((fabs(ground_turn) > SLOWDOWN_ANGLE) && (TurnType != GRABSLIDE_TURN) && (TurnType != TAILSLIDE_TURN) && !doing_face_turn)
	{
		omega = TURN_MOD;
	}

	if (st == BOARD_TURN_RIGHT)
		omega *= -1.0f;

	turn_amt = omega*dt;

	if (!doing_face_turn && ((current_region == WAVE_REGIONLIP)
		|| (current_region == WAVE_REGIONCHIN) || (current_region == WAVE_REGIONLIP2)))
	{
		turn_amt *=1.30f;
	}


	float t_amt;
	vector3d turn_vec;// = my_board->get_abs_po().non_affine_inverse_xform(normal);
	turn_vec = YVEC;
	t_amt = DEG_TO_RAD(turn_amt);
	turn.set_rot (turn_vec, t_amt);

    rb->my_po = (turn * rb->my_po);

	abs_ground_turn += fabs(t_amt);
	ground_turn += t_amt;
    this->state = st;
  }
}




void SurfBoardObjectClass::TrickSpin(float angle)
{
	if (!wiped_out)
	{
		po turn;
		turn.set_rot (YVEC, DEG_TO_RAD((angle)));
		rb->my_po = (turn * rb->my_po);
	}
}


const float JUMP_H_SCALE = .5f;
float FLOAT_MOD = 4.0f;
float test_scale_z = 1.0f;
float test_scale_y = 1.0f;
float roof_jump_z = -2.5f;
float roof_jump_y = 0.0f;
void SurfBoardObjectClass::Jump(float degree)
{
  exit_jump = false;
  float_jump = false;
  grind_ollie = false;
  lip_jump = false;
  launch_jump = false;
  bool in_cone = InLaunchCone();
  bool in_launch = InLaunchRegion();
  int state = get_ks_controller()->get_current_state();
  //float v_stick = get_ks_controller()->CtrlEvent(PAD_LSTICK_V);
  if ((state == STATE_CONTROLLED_JUMP) && !roof_jump)
  {
	  rb->linMom.z = -3.5f*test_scale_z;
	  rb->linMom.y = 3.5f*test_scale_y;
	  rb->linMom.x *= 0.5f;
  }
  else if ((state == STATE_LAUNCH) && (dot(normal, YVEC) > 0.93f) && InGrindRegion())
  {
	  vector3d v_temp;
	  rb->linMom.x -= WAVE_CURRENT_MOD*current.x; // add back in some x velocity so that jumps can be equal both directions

	  // change the linear momentum to land on the wave (a little bit in front of where we took off)
	  rb->linMom.z = 0.0f;
	  rb->linMom.y += 25.0f;

	  rb->linMom.y *= LAUNCH_VELOCITY_SCALE*ksctrl->GetAttribs(ATTRIB_AIR);
	  rb->linMom.x *= 0.85f;

	  float temp = rb->linMom.y;
	  rb->linMom.y = 0.0f;
	  float len = rb->linMom.length();
	  v_temp = rb->linMom - lip_normal*dot(rb->linMom, lip_normal);
	  v_temp.normalize();
	  rb->linMom = len*v_temp;
	  rb->linMom.y = temp;
	  rb->linMom.z = -4.5f;

	  if (this->TongueEffectOn())
	  {
		  rb->linMom.z = 0.0f;
	  }

	  take_off_dir = forward.dir;

	  // Add a splash at lip when surfer jumps
	  ksctrl->my_trail->create_chophop_splash(rb->pos);
	  lip_jump = true;

	  v_temp = float_pos - rb->pos;
	  float square = dot(v_temp, v_temp);
	  if (square < FLOAT_MOD)
		  float_jump = true;

  }
  else if ((state == STATE_LAUNCH) && (this->state != BOARD_GRIND) && (this->state != BOARD_FLOAT) && !roof_jump)
  {
	vector3d v_temp;
	rb->linMom.x -= WAVE_CURRENT_MOD*current.x; // add back in some x velocity so that jumps can be equal both directions

	// change the linear momentum to land on the wave (a little bit in front of where we took off)
	rb->linMom.z = 0.0f;
	rb->linMom.y += 20.0f;

	rb->linMom.y *= LAUNCH_VELOCITY_SCALE*ksctrl->GetAttribs(ATTRIB_AIR);
	rb->linMom.x *= 0.85f;

	float temp = rb->linMom.y;
	rb->linMom.y = 0.0f;
	float len = rb->linMom.length();
	v_temp = rb->linMom - lip_normal*dot(rb->linMom, lip_normal);
	v_temp.normalize();
	rb->linMom = len*v_temp;
	rb->linMom.y = temp;

	take_off_dir = forward.dir;

	// Add a splash at lip when surfer jumps
	ksctrl->my_trail->create_chophop_splash(rb->pos);
	lip_jump = true;
	launch_jump = true;

	v_temp = float_pos - rb->pos;
	float square = dot(v_temp, v_temp);
	if (square < FLOAT_MOD)
		float_jump = true;
  }
  else if ((this->state == BOARD_FLOAT) || (this->state == BOARD_GRIND) || grind_jump)
  {
	float add_y = 12.0f;
	float add_z = -4.5f;
	vector3d grind_vector = get_ks_controller()->GetGrindVector();
	rb->linMom.y += add_y;
	rb->linMom.z += add_z;
	this->state = BOARD_NONE;
	rb->linMom.y *= GRIND_DISMOUNT_VELOCITY_SCALE;

	take_off_dir = grind_vector;

	// Add a splash at lip when surfer jumps
    ksctrl->my_trail->create_chophop_splash(rb->pos);
  }
  else if (roof_jump)
  {
	 vector3d delta;

    delta.x = 0;
    delta.y = roof_jump_y; //this->maxJumpVelocity * degree;
    delta.z = roof_jump_z; //this->maxJumpVelocity * (1 - degree) * 0.5f*JUMP_H_SCALE;

    //delta = rb->my_po.non_affine_slow_xform (delta);

	rb->linMom.z *= 0.65f;
    rb->linMom += delta;
	if (rb->linMom.z > 0.0f)
		rb->linMom.z = 0.0f;

	if (rb->linMom.y < 0.9f)
		rb->linMom.y = 0.9f;
	else if (rb->linMom.y > 5.0f)
		rb->linMom.y = 5.0f;

	this->rb->linMom.x *= 0.6f;

  }
  else if ((in_launch && !in_cone) || ((current_region == WAVE_REGIONFACE)
	  || (current_region == WAVE_REGIONPOCKET) || (current_region == WAVE_REGIONSHOULDER2)))
  {
	vector3d delta;

	delta.x = 0;
	delta.y = 7.5f; //this->maxJumpVelocity * degree;
	delta.z = 0.0f; //this->maxJumpVelocity * (1 - degree) * 0.5f*JUMP_H_SCALE;

	delta = rb->my_po.non_affine_slow_xform (delta);

	this->rb->linMom += delta;
	if (this->rb->linMom.z > 0.0f)
		this->rb->linMom.z = 0.0f;

	rb->linMom.y *= CHOP_HOP_VELOCITY_SCALE;

  }
  else
  {
	vector3d delta;

	delta.x = rb->linMom.x;
	delta.y = 9.5f;//this->maxJumpVelocity * degree;
	delta.z = -1.4f; //this->maxJumpVelocity * (1 - degree) * 0.5f*JUMP_H_SCALE;

	this->rb->linMom = delta;
	if (this->rb->linMom.z > 0.0f)
		this->rb->linMom.z = 0.0f;

	rb->linMom.y *= CHOP_HOP_VELOCITY_SCALE;
  }

  air_timer = 0.0f;
  air_time = 2.0f*CalculatePathPeakTime();
  this->state = BOARD_NONE;
  air_turn = 0.0f;
}

extern int test_big_wave;

float g_takeoff_current_offset = 3.5f;
void SurfBoardObjectClass::ResetPhysics ()
{
	int			curbeach = g_game_ptr->get_beach_id();
	current_region = WAVE_REGIONFRONT;

	/*vector3d	take_offset;

	if (BeachDataArray[curbeach].bdir)
		current.x = WaveDataArray[curbeach].speedx;
	else
		current.x = -WaveDataArray[curbeach].speedx;

	current.y = 0.0f;
	current.z = -WaveDataArray[curbeach].speedz;

	// Pick spawn position for player.
	if (!g_game_ptr->is_splitscreen())
	{
		take_offset = - g_takeoff_current_offset*current;
	}
	// For splitscreen modes, we need to make sure player is starting in a safe spot.
	else
	{
		take_offset = - g_takeoff_current_offset*current;
	}

	rb->pos = *WAVE_GetMarker(WAVE_MarkerSpawnEnd) + take_offset;
	*/

	rb->pos = g_game_ptr->calc_beach_spawn_pos();

	rb->pos.x += (BeachDataArray[curbeach].bdir?-1.0f:1.0f)*5.0f;

	if (ksctrl->IsAIPlayer())
		rb->pos.x += (BeachDataArray[curbeach].bdir?-1.0f:1.0f)*20.0f;

	// Vanessa needs help to find the objects.
	//nglPrintf ("Spawning surfer at: %.0f %.0f %.0f\n", rb->pos.x, rb->pos.y, rb->pos.z);

	rb->linMom = ZEROVEC;
	if (ksctrl->get_last_state() != STATE_DUCKDIVE)
	{
		//rb->linMom = 0.8f*vector3d(4.0f, 0.0f, 12.0f);
		//rb->pos = vector3d(37.0f, 1.0f, -40.0f);
		po turn;
		vector3d board_z = -rb->linMom;
		board_z.normalize();
		float sign = -1.0f;
		if (BeachDataArray[curbeach].bdir)
			sign = 1.0f;

		//float angle = acos(dot(board_z, ZVEC));
		float angle = 3.14159f;
		turn.set_rot (YVEC, sign*angle);
		rb->my_po = po_identity_matrix;
		rb->my_po = (turn * rb->my_po);
		//  wave_front_hint.z = 0;
		//  wave_center_hint.z = 0;
	}

	ksctrl->reset_state ();
	ksctrl->tube_meter.End();

	my_board->set_rel_po(rb->my_po);
	my_board->set_rel_position (rb->pos);
	ksctrl->AlignSurferWithBoard(0.0f);

	//  ks_fx_reset();	// Should now be handled by ks_fx_OnNewWave.  (dc 01/26/02)

	grind_entity = NULL;
	grind_object = NULL;

	// Moved from SurfBoardObjectClass::Update.  Previously it was possible for wiped_out to be
	// set to false without reverting to the game camera.  (dc 01/26/02)
	if ((wiped_out))
	{
		//  Put the camera back the way it was before the wipeout.
		if (!g_game_ptr->user_cam_is_on() && !ksctrl->IsAIPlayer())
		{
			ksctrl->SetPlayerCamera(ksctrl->GetPlayerCam());
			g_game_ptr->set_current_camera(ksctrl->GetPlayerCam());
		}

		wiped_out = false;
	}

	ksctrl->SetPlayerCamera(ksctrl->GetPlayerCam());
	backwards_time = 0.0f;
	wipeout_time = 0.0f;
	standing_time = 0;
	last_f = 1.0f;

	ksctrl->SetShakeOff(false);
	in_water_wall = false;
	inAirFlag = false;
	front_hint_valid = false;
	center_hint_valid = false;
	tube_hint_valid = false;
	wave_info.onoff = false;
	exit_jump = false;

	this->turnDegree=0.0f;
	state = BOARD_NONE;
	this->curLeanAngle = 0.0f;
    this->leanEaseIn = 0.0f;

	// Let IGO know surfer was reset.
	if (!get_ks_controller()->IsAIPlayer())
		frontendmanager.IGO->OnSurferReset(ksctrl->get_player_num(), BeachDataArray[curbeach].bdir);
}

void SurfBoardObjectClass::ResetTimers()
{
	standing_time = 0;
	backwards_time = 0;
}

void SurfBoardObjectClass::SetWipeoutDone(void)
{
	max_wipeout_time = 0.0f;
	state = BOARD_NONE;
	ks_fx_end_wipeout_splash (get_ks_controller()->get_player_num());
}

void SurfBoardObjectClass::DoWipeOut (int wip_type, bool keep_cam)
{
	wipeout_type = wip_type;

	//  Competitions need to count the number of wipeouts.
	if (g_game_ptr->is_competition_level() &&
	    g_game_ptr->get_game_mode() == GAME_MODE_CAREER)
		g_beach_ptr->judges.IncPlayerWipeouts();

	wiped_out = true;
	max_wipeout_time = 50.0f;  // absurdly high number
	ksctrl->set_wave_hint(wave_center_hint);
	//	frontendmanager.IGO->TurnOnTubeTimer(ksctrl->get_player_num(), false);
	ksctrl->get_special_meter()->SetFillage(0.0f);
	//ksctrl->big_wave_meter.End();
	ksctrl->tube_meter.End();
	//ksctrl->float_meter.End();
	ksctrl->ResetJumpedEntities();
	ksctrl->ClearTricks();
	ksctrl->InitWipeout(wipeout_type);
	// Is this the ai player?
	if (g_game_ptr->get_num_ai_players())
	{
		if (ksctrl->get_player_num())
			return;
	}

	if (!(GWipeoutData[wipeout_type].input_vel & DISMOUNT_NO_AIR_FLAG))
		SetWipeoutPhysics();

	if(!keep_cam)
	{
	  camera *cur_cam = g_game_ptr->get_player_camera(ksctrl->get_player_num());
	  if (!g_game_ptr->user_cam_is_on() && (cur_cam != ksctrl->get_wipeout_cam2_ptr()) && (cur_cam != ksctrl->get_debug_cam_ptr()))
	  {
		  po temp = cur_cam->get_abs_po();
		  //ksctrl->SetPlayerCamera(ksctrl->get_wipeout_cam_ptr());
		  ksctrl->SetPlayerCamera(ksctrl->get_wipeout_cam2_ptr());

		  //  Set up the camera.
		  //(ksctrl->get_wipeout_cam_ptr())->init(temp, cur_cam);
		  (ksctrl->get_wipeout_cam2_ptr())->init(temp, cur_cam);
	  }
	}


	// Apply wipeout time penalty.
	if (g_game_ptr->get_num_active_players() == 1)
		TIMER_SetLevelSec(TIMER_GetLevelSec()+CLOCKTIME_WIPEOUT);

	// Let other objects know the surfer wiped out.
	g_eventManager.DispatchEvent(EVT_SURFER_WIPEOUT, ksctrl->get_player_num(), 0);
}


void SurfBoardObjectClass::SetWipeoutPhysics(void)
{
	if (GWipeoutData[wipeout_type].b_wip_anim_free != BOARD_ANIM_NULL)
	{
		vector3d delta_vel = ZEROVEC;
		int vel_type = GWipeoutData[wipeout_type].b_input_vel;
		if (vel_type & DIR_DATA_FLAG)
		{
			delta_vel = GWipeoutData[wipeout_type].b_xvel*XVEC + GWipeoutData[wipeout_type].b_yvel*YVEC + GWipeoutData[wipeout_type].b_zvel*ZVEC;
		}

		if (vel_type & BOARD_DIR_FLAG)
			delta_vel += GetForwardDir()*GWipeoutData[wipeout_type].b_dir_vel;
		else if (vel_type & BOARD_RIGHT_FLAG)
			delta_vel += this->GetRightDir()*GWipeoutData[wipeout_type].b_dir_vel;
		else if (vel_type & MOM_DIR_FLAG)
		{
			vector3d mom_dir = rb->linMom;
			mom_dir.normalize();
			delta_vel += mom_dir*GWipeoutData[wipeout_type].b_dir_vel;
		}

		if (vel_type & MOM_ADDITIVE_FLAG)
			rb->linMom += delta_vel*rb->mass;
		else
			rb->linMom = delta_vel*rb->mass;

		if (rb->linMom.y < 0.0f)
			rb->linMom.y =  delta_vel.y*rb->mass;

		float dot_board_vel = dot(rb->linMom, normal);
		if (dot_board_vel <= 0.3f*rb->mass)
		{
			rb->linMom -= normal*dot_board_vel;
			rb->linMom += 0.3f*normal*rb->mass;
		}

		// if b_wip_anim_free anim not NULL, then board flies inot air
		inAirFlag = true;

		SetBoardState(BOARD_IN_AIR);
	}
}


void SurfBoardObjectClass::KlugeWipeout(int tumble_anim, int swim_anim)
{

	ksctrl->end_secondary_cam();

	wiped_out = true;
	max_wipeout_time = 50.0f;  // absurdly high number
	ksctrl->set_wave_hint(wave_center_hint);
	//	frontendmanager.IGO->TurnOnTubeTimer(ksctrl->get_player_num(), false);
	ksctrl->get_special_meter()->SetFillage(0.0f);
	//ksctrl->big_wave_meter.End();
	ksctrl->tube_meter.End();
	//ksctrl->float_meter.End();
	ksctrl->ResetJumpedEntities();
	ksctrl->ClearTricks();
	ksctrl->InitKlugeWipeout(tumble_anim, swim_anim);
	// Is this the ai player?
	if (g_game_ptr->get_num_ai_players())
	{
		if (ksctrl->get_player_num())
			return;
	}

	camera *cur_cam = g_game_ptr->get_player_camera(ksctrl->get_player_num());
	if (!g_game_ptr->user_cam_is_on() && (cur_cam != ksctrl->get_wipeout_cam2_ptr()) && (cur_cam != ksctrl->get_debug_cam_ptr()))
	{
		po temp = cur_cam->get_abs_po();
		//ksctrl->SetPlayerCamera(ksctrl->get_wipeout_cam_ptr());
		ksctrl->SetPlayerCamera(ksctrl->get_wipeout_cam2_ptr());

		//  Set up the camera.
		//(ksctrl->get_wipeout_cam_ptr())->init(temp, cur_cam);
		(ksctrl->get_wipeout_cam2_ptr())->init(temp, cur_cam);
	}

	// Let other objects know the surfer wiped out.
	g_eventManager.DispatchEvent(EVT_SURFER_WIPEOUT, ksctrl->get_player_num(), 1);
}

#define MAX_STANDING_TIME 1.0f
#define MAX_BACKWARDS_TIME 1.3f
float WIPEOUT_RESET_TIME = 7.0f;

extern int g_disable_wipeouts;

bool SurfBoardObjectClass::CheckForWipeOut (CollideCallStruct& collide, float dt)
{
  if (ksctrl->get_super_state() == SUPER_STATE_FLYBY)  //  no wipeouts or anything else during flybys.
	  return false;

  if (g_disable_wipeouts)
    return false;

  if (!this->inAirFlag)
  {
    int current_state = ksctrl->get_current_state ();
	int super_state = ksctrl->get_super_state ();

    // check if the surfer is on the wrong place
    switch (current_region)
    {
    case WAVE_REGIONPOCKET:
    case WAVE_REGIONTUBE:
    case WAVE_REGIONFACE:
    case WAVE_REGIONLIP:
    case WAVE_REGIONLIP2:
    case WAVE_REGIONCHIN:
    case WAVE_REGIONSHOULDER:
    case WAVE_REGIONSHOULDER2:
    case WAVE_REGIONWASH:
	case WAVE_REGIONROOF:
	//case WAVE_REGIONCURL:
	//case WAVE_REGIONMAX:

      if ((current_state == STATE_WOBBLE) && (standing_time < 0))
      {
        ksctrl->set_state (STATE_STAND);
        ksctrl->set_super_state (SUPER_STATE_NORMAL_SURF);
      }

      standing_time -= dt;
      break;

    case WAVE_REGIONWOBBLE:
    case WAVE_REGIONFRONT:
      /*if ((current_state != STATE_LIEONBOARD) &&
          (current_state != STATE_PADDLE) &&
          (current_state != STATE_LIETOSTAND) &&
          (current_state != STATE_SWIMTOLIE))
      {
        if (standing_time < 0)
          standing_time = 0;

        standing_time += dt;

        if (standing_time > MAX_STANDING_TIME)
        {
          ksctrl->set_state (STATE_STANDTOLIE);
          ksctrl->set_super_state(SUPER_STATE_LIE_ON_BOARD);
        }
        else if (current_state != STATE_WOBBLE)
        {
          ksctrl->set_state (STATE_WOBBLE);
          ksctrl->set_super_state (SUPER_STATE_NORMAL_SURF);
        }
      }*/
      break;

    case WAVE_REGIONBACK:
	  if (current_state != STATE_ON_ROOF)
	  {
		  wipeout_type = WIP_TAKE_OFF_FLAT;
		  return true;
	  }
      break;

    case WAVE_REGIONMAX:
	  if (current_state != STATE_ON_ROOF)
	  {
		  wipeout_type = WIP_LOW_HIT_BY_LIP;
		  return true;
	  }
      break;

	case (WAVE_REGIONMAX + 1):
	  if (current_state != STATE_ON_ROOF)
	  {
		  wipeout_type = WIP_LOW_HIT_BY_LIP;
		  return true;
	  }
      break;

	case WAVE_REGIONCURL:
	  if (current_state != STATE_ON_ROOF)
	  {
		  wipeout_type = WIP_LOW_AIR_FOR;
		  return true;
	  }
      break;

	case WAVE_REGIONCEILING:
	  //if (current_state != STATE_ON_ROOF)
	  //{
		  CalculateCeilingWipeout();
		  return true;
	  //}
      break;

    default:
//    case WAVE_REGIONCEILING:
//    case WAVE_REGIONCURL:
//    case WAVE_REGIONROOF:
      wipeout_type = WIP_LOW_AIR_FOR;
      return true;
      break;
    }

    // check if the surfer is going backwards
    if (((super_state == SUPER_STATE_NORMAL_SURF) || (super_state == SUPER_STATE_PREPARE_JUMP)) &&
        (dot (rb->linMom, forward.dir) < -0.96f) &&
		!ksctrl->IsDoingSpecialTrick())
    {
      if (dot (*collide.current, forward.dir) > 0)
      {
        backwards_time += dt;

        if (backwards_time > (MAX_BACKWARDS_TIME * stability_factor))
        {
          wipeout_type = WIP_LOW_STUMBLE_BACK;
          return true;
        }
        else if (backwards_time > (MAX_BACKWARDS_TIME * stability_factor * 0.5f))
        {
          ksctrl->set_state (STATE_STUMBLE_BACK);
          ksctrl->set_super_state(SUPER_STATE_NORMAL_SURF);
		  if (ksctrl->CtrlEvent(PAD_SLIDE) && ksctrl->CtrlEvent(PAD_LSTICK_H))
		  {
				ksctrl->set_state(STATE_PICKTAILSLIDETURNCYCLE);
				ksctrl->set_super_state(SUPER_STATE_NORMAL_SURF);
				backwards_time = 0.0f;
		  }

        }
      }
	  else if (ksctrl->IsDoingSpecialTrick())
		  backwards_time = 0.0f;
      else
      {
        backwards_time -= dt;
        if (backwards_time < 0)
          backwards_time = 0;
        else if ((ksctrl->get_current_state () == STATE_STUMBLE_BACK) && (backwards_time < (MAX_BACKWARDS_TIME * stability_factor * 0.5f)))
        {
          ksctrl->set_state (STATE_STAND);
		  ksctrl->set_super_state (SUPER_STATE_NORMAL_SURF);
        }
      }
    }
    else
    {
      backwards_time -= dt;
      if (backwards_time < 0)
        backwards_time = 0;
      else if ((ksctrl->get_current_state () == STATE_STUMBLE_BACK) && (backwards_time < (MAX_BACKWARDS_TIME * stability_factor * 0.5f)))
      {
        ksctrl->set_state (STATE_STAND);
		ksctrl->set_super_state (SUPER_STATE_NORMAL_SURF);
      }
    }

	if ((super_state == SUPER_STATE_NORMAL_SURF) || (super_state == SUPER_STATE_PREPARE_JUMP))
	{
		vector3d x_point = rb->pos - *WAVE_GetMarker(WAVE_MarkerXCrash);
		if (dot(lip_vec, x_point) < 0.0f)
		{
			wipeout_type = WIP_TAKE_OFF_FLAT;	//  fixme wipeout
			return true;
		}
	}
  }

  // check for collision with other objects
  entity *hitme = g_beach_ptr->check_entity_collision(ksctrl, grind_object);

  //  Let the wipeout cam know what the object was that the surfer collided with, if any
  (ksctrl->get_wipeout_cam_ptr())->set_collision_obj(hitme);

	if (g_beach_ptr->get_smashed_entity())
	{
		sfx.collided(my_board_model, g_beach_ptr->get_smashed_entity());
	}

  if (hitme)
  {
		sfx.collided(my_board_model, hitme);
		wipeout_type = WIP_TAKE_OFF_FLAT;  //  fixme wipeout
    return true;
  }

  return false;
}

bool SurfBoardObjectClass::CheckForGrindLandingWipeOut (CollideCallStruct& collide, float dt)
{
	if (get_ks_controller()->IsDoingTrick())
	{
		get_ks_controller()->ResetJumpedEntities();
		if (!g_disable_wipeouts)
		{
			CalcLandingWipeout();
			return true;
		}
	}

	vector3d beach_dir = lip_vec;
	beach_dir.y = 0.0f;
	vector3d zvec = -ZVEC;
	zvec -= beach_dir*dot(beach_dir, zvec);
	zvec.normalize();

	take_off_dir = (beach_dir + zvec)*0.707106781f;

	vector3d board_dir = forward.dir;
	board_dir.y = 0.0f;
	board_dir.normalize();

	float angle = dot(board_dir, take_off_dir);
	if (angle > LANDING_ANGLE_PERFECT)
	{
		landingType = LANDING_FLAG_PERFECT;
	}
	else if (angle > LANDING_ANGLE_SLOPPY) // no landing sloppy, just regular or perfect
	{
		landingType = 0;
	}
	else if (angle < -LANDING_ANGLE_PERFECT)
	{
		landingType = LANDING_FLAG_PERFECT | LANDING_FLAG_FAKEY;
	}
	else if (angle < -LANDING_ANGLE_SLOPPY)
	{
		landingType = LANDING_FLAG_FAKEY;
	}
	else
	{
		get_ks_controller()->ResetJumpedEntities();
		if (g_disable_wipeouts)
			return false;

		CalcLandingWipeout();
		return true;
	}

	return false;
}

float WAVE_WIP_CURRENT_MOD = 0.3f;
void SurfBoardObjectClass::CalcLandingWipeout(void)
{
	//int intensity = ksctrl->GetWipeoutIntensity();
	int intensity = 0;
	vector3d bdir = GetForwardDir();
	bdir.y = 0.0f;
	bdir.normalize();
	vector3d mom = rb->linMom;


	// A little sound stuff
	if (ksctrl->GetWipeoutIntensity() != WIP_INT_LOW)
	{
		/*if (g_game_ptr->GetSurferIdx(get_ks_controller()->get_player_num()) != SURFER_LISA_ANDERSEN)
			SoundScriptManager::inst()->playEvent(SS_MALE_GRUNT);
		else
			SoundScriptManager::inst()->playEvent(SS_FEMALE_GRUNT); */
		if (g_game_ptr->GetSurferIdx(get_ks_controller()->get_player_num()) != SURFER_LISA_ANDERSEN)
			SoundScriptManager::inst()->playEvent(SS_WIPEOUT_FIRST_HIT_MALE);
		else
			SoundScriptManager::inst()->playEvent(SS_WIPEOUT_FIRST_HIT_FEMALE);

	}




	//  due to velocity of current, it appears like the surfer is always moving away from tube
	//  add some velocity away from tube to fudge apparent momentum so that wipeouts look realistoc
	mom.x -= WAVE_WIP_CURRENT_MOD*current.x;
	mom.y = 0.0f;
	mom.normalize();
	float cosine = dot(bdir, mom);
	vector3d temp = cross(bdir, mom);
	float cosine2 = dot(temp, YVEC);

	if (cosine >= 0.924f)
	{
		wipeout_type = WIP_LOW_AIR_FOR + intensity;
	}
	else if (cosine >= 0.383f)
	{
		if (cosine2 < 0.0f)
		{
			if (ksctrl->IsGoofy())
				wipeout_type = WIP_LOW_AIR_FOR_RIGHT + intensity;
			else
				wipeout_type = WIP_LOW_AIR_FOR_LEFT + intensity;
		}
		else
		{
			if (ksctrl->IsGoofy())
				wipeout_type = WIP_LOW_AIR_FOR_LEFT + intensity;
			else
				wipeout_type = WIP_LOW_AIR_FOR_RIGHT + intensity;
		}
	}
	else if (cosine >= -0.383f)
	{
		if (cosine2 < 0.0f)
		{
			if (ksctrl->IsGoofy())
				wipeout_type = WIP_LOW_AIR_RIGHT + intensity;
			else
				wipeout_type = WIP_LOW_AIR_LEFT + intensity;
		}
		else
		{
			if (ksctrl->IsGoofy())
				wipeout_type = WIP_LOW_AIR_LEFT + intensity;
			else
				wipeout_type = WIP_LOW_AIR_RIGHT + intensity;
		}
	}
	else if (cosine >= -0.924f)
	{
		if (cosine2 < 0.0f)
		{
			if (ksctrl->IsGoofy())
				wipeout_type = WIP_LOW_AIR_BOT_RIGHT + intensity;
			else
				wipeout_type = WIP_LOW_AIR_BOT_LEFT + intensity;
		}
		else
		{
			if (ksctrl->IsGoofy())
				wipeout_type = WIP_LOW_AIR_BOT_LEFT + intensity;
			else
				wipeout_type = WIP_LOW_AIR_BOT_RIGHT + intensity;
		}
	}
	else
		wipeout_type = WIP_LOW_AIR_BACK + intensity;
}

int SurfBoardObjectClass::CalculateCeilingWipeout(void)
{
	vector3d bdir = GetForwardDir();
	bdir.y = 0.0f;
	bdir.normalize();
	float cosine = dot(lip_vec, bdir);
	float cosine2 = dot(bdir, -ZVEC);
	int current_beach = g_game_ptr->get_beach_id ();
    int negative = BeachDataArray[current_beach].bdir;
	if (cosine >= 0.707f)
	{
		if (negative)
		{
			if (ksctrl->IsGoofy())
				wipeout_type = WIP_HIT_BY_CEILING_RIGHT;
			else
				wipeout_type = WIP_HIT_BY_CEILING_LEFT;
		}
		else
		{
			if (ksctrl->IsGoofy())
				wipeout_type = WIP_HIT_BY_CEILING_LEFT;
			else
				wipeout_type = WIP_HIT_BY_CEILING_RIGHT;
		}
	}
	else if (cosine < -0.707f)
	{
		if (negative)
		{
			if (ksctrl->IsGoofy())
				wipeout_type = WIP_HIT_BY_CEILING_LEFT;
			else
				wipeout_type = WIP_HIT_BY_CEILING_RIGHT;
		}
		else
		{
			if (ksctrl->IsGoofy())
				wipeout_type = WIP_HIT_BY_CEILING_RIGHT;
			else
				wipeout_type = WIP_HIT_BY_CEILING_LEFT;
		}
	}
	else if (cosine2 >= 0.707f)
		wipeout_type = WIP_HIT_BY_CEILING_FOR;
	else
		wipeout_type = WIP_HIT_BY_CEILING_BACK;

	return wipeout_type;
}

bool SurfBoardObjectClass::CheckForLandingWipeOut (CollideCallStruct& collide, float dt)
{

  vector3d up (rb->my_po.get_y_facing ());
  float d;
//  int points;	// number of points awarded


  if (ksctrl->get_current_state() == STATE_CONTROLLED_AIR)
  {
	  landingType = 0;
	  return false;
  }
  else if (get_ks_controller()->IsDoingTrick())// || !get_ks_controller()->AnimBlended())
  {
	  get_ks_controller()->ResetJumpedEntities();
	  if (!g_disable_wipeouts)
	  {
		  CalcLandingWipeout();
			if (!get_ks_controller()->IsAIPlayer() && !exit_jump)
				frontendmanager.IGO->GetHintManager()->SetHint(IGOHintManager::LandedDuringTrick);
		  return true;
	  }
  }
  else if (this->exit_jump)
	  return true;

  if (get_ks_controller()->IsFloaterLanding() || roof_jump)
  {
	  landingType = 0;
	  return false;
  }
  else if ((current_region == WAVE_REGIONROOF) || (current_region == WAVE_REGIONSHOULDER)
							|| (current_region == WAVE_REGIONBACK))
  {
	  landingType = 0;
	  return false;
  }

  // check if the board is facing up when it hits the water
  if (dot (*collide.normal, up) < 0.5f * (2 - stability_factor)) // 60 degrees
  {
//    return true;
  }

  d = dot (up, YVEC);

  // check if the board landed on flat water
  if (__fabs (d) > 0.9063f)
  {
    landingType = 0;
    return false;
  }

  int current_beach = g_game_ptr->get_beach_id ();
  int negative = BeachDataArray[current_beach].bdir;
  if (negative)
	take_off_dir.x = -fabs(take_off_dir.x);
  else
	take_off_dir.x = fabs(take_off_dir.x);

  vector3d v(forward.dir);
  if (fabs(take_off_dir.z) > fabs(take_off_dir.y))
  {
	  take_off_dir.z = -take_off_dir.z;
	  take_off_dir.y = 0;
	  take_off_dir.normalize ();

	  v.y = 0;
	  v.normalize ();
  }
  else
  {
	  take_off_dir.y = -take_off_dir.y;
	  take_off_dir.z = 0;
	  take_off_dir.normalize ();

	  v.z = 0;
	  v.normalize ();
  }

  d = dot (take_off_dir, v);

  if (d > LANDING_ANGLE_PERFECT)
  {
	  landingType = LANDING_FLAG_PERFECT;
  }
  else if (d > LANDING_ANGLE_REGULAR)
  {
	  landingType = 0;
  }
  else if (d > LANDING_ANGLE_SLOPPY)
  {
	  landingType = LANDING_FLAG_SLOPPY;
  }
  /*
  else if (d > LANDING_ANGLE_JUNK)
  {
	  landingType = LANDING_FLAG_JUNK;
  }
  */
  else if (d < -LANDING_ANGLE_PERFECT)
  {
	  landingType = LANDING_FLAG_PERFECT | LANDING_FLAG_FAKEY;
  }
  else if (d < -LANDING_ANGLE_REGULAR)
  {
    landingType = LANDING_FLAG_FAKEY;
  }
  else if (d < -LANDING_ANGLE_SLOPPY)
  {
    landingType = LANDING_FLAG_SLOPPY | LANDING_FLAG_FAKEY;
  }
  /*
  else if (d < -LANDING_ANGLE_JUNK)
  {
    landingType = LANDING_FLAG_JUNK | LANDING_FLAG_FAKEY;
  }
  */
  else
  {
    get_ks_controller()->ResetJumpedEntities();
    if (g_disable_wipeouts)
      return false;

		CalcLandingWipeout();
		if (!get_ks_controller()->IsAIPlayer())
			frontendmanager.IGO->GetHintManager()->SetHint(IGOHintManager::LandedSideways);
    return true;
  }

  return false;
}

float add_m = 0.15f;
float lip_mag = -0.2f;
float g_go_over_back = 0.0f;
bool SurfBoardObjectClass::GoneOverLip(float dt)
{
	//`return false;
	int state = ksctrl->get_current_state();
	int super_state = ksctrl->get_super_state();
	if (this->inAirFlag || (super_state == SUPER_STATE_LIE_ON_BOARD) || (super_state == SUPER_STATE_WIPEOUT)
		 || (super_state == SUPER_STATE_FLYBY) || (state == STATE_FLOAT) || (state == STATE_DUCKDIVE)
		 || (state == STATE_ON_ROOF) || (state == STATE_LIETOSTAND))
		return false;

	if ((current_region != WAVE_REGIONLIP) && (current_region != WAVE_REGIONLIP2) && (current_region != WAVE_REGIONBACK))
		return false;

	vector3d z_vec = ZVEC - dot(normal, ZVEC)*normal;
	z_vec.normalize();
	float infloat = dot(my_board->get_abs_position() - float_pos, z_vec);
	bool in_float_region = (infloat > lip_mag);
	bool in_back = (current_region == WAVE_REGIONBACK);
	if ((!g_go_over_back && (in_float_region || in_back)) ||
		((in_float_region || in_back) && ksctrl->IsDoingSpecialTrick()))
	{
		if (state != STATE_FLOAT)
		{
			vector3d pos = my_board->get_abs_position();
			vector3d error = float_pos - pos;
			if (dot(error, ZVEC) > 4.0f)
				return false;

			float mag = 1.0f;
			if (in_back)
				mag = 4.0f;

			if ((infloat > 0.0f) && in_back)
			{
				error *= add_m*mag;
				my_board->set_rel_position (pos + error);
				this->rb->pos = pos + error;
			}

			rb->linMom.z = -10.0f*fabs(dot(error, ZVEC));
		}

		if (current_region == WAVE_REGIONBACK)
			current_region = last_region;

		return true;
	}

	return false;
}

float g_grind_fudge = 1.0f;
bool SurfBoardObjectClass::InGrindRegion(void)
{
	//if ((current_region == WAVE_REGIONLIP)
		//|| (current_region == WAVE_REGIONLIP2)
		//|| (current_region == WAVE_REGIONROOF))
	//{
		vector3d diff_vec1 = *WAVE_GetMarker(WAVE_MarkerLipMark11) +
					g_grind_fudge*(*WAVE_GetMarker(WAVE_MarkerLipMark12) - *WAVE_GetMarker(WAVE_MarkerLipMark11));
		vector3d vec1 = my_board->get_abs_position() - diff_vec1;
		vector3d diff_vec2 = *WAVE_GetMarker(WAVE_MarkerLipMark8p5);
		vector3d vec2 = my_board->get_abs_position() - diff_vec2;
		if ((dot(vec1, lip_vec) < 0.0f) && (dot(vec2, lip_vec) > 0.0f))
			return true;
	//}

	return false;
}


bool SurfBoardObjectClass::CanBustThroughTube(vector3d position)
{
	//int current_beach = g_game_ptr->get_beach_id ();
	//bool left = BeachDataArray[current_beach].bdir;
	//float near_tube_x = (WAVE_GetMarker(WAVE_MarkerEmitterStartCrestX)->x + WAVE_GetMarker(WAVE_MarkerEmitterStartLipX)->x)*0.5f;
	//bool near_tube = (left && (position.x > near_tube_x)) || (!left && (position.x < near_tube_x));
	//vector3d crash_point = *WAVE_GetMarker(WAVE_MarkerLipCrash);
	if (!wave_info.onoff)
	{
		//if ((position.x > crash_point.x && !left) || (position.x < crash_point.x && left))
			return true;
	}

	int stage = wave_info.stage;
	if ((wave_info.type == WAVE_PerturbTypeBigTongue) || (wave_info.type == WAVE_PerturbTypeTongue))
	{
		if (((stage == WAVE_PerturbStageDo) && (wave_info.stageprogress > 0.5f)) || (stage == WAVE_PerturbStageHold)
				|| ((stage == WAVE_PerturbStageCollapse) && (wave_info.stageprogress <= 0.5f)))
		{
				//if (((position.x > crash_point.x && !left) || (position.x < crash_point.x && left)) && near_tube)
					return true;
		}
		else if (((stage == WAVE_PerturbStageCollapse) && (wave_info.stageprogress > 0.5f)) || (stage == WAVE_PerturbStageWait)
				|| ((stage == WAVE_PerturbStageUndo) && (wave_info.stageprogress < 0.2f)))
		{
				//if (((position.x > crash_point.x && !left) || (position.x < crash_point.x && left)) && near_tube)
					return true;
		}
		else
			return false;
	}
	else if ((wave_info.type == WAVE_PerturbTypeBigSurge) || (wave_info.type == WAVE_PerturbTypeSurge))
	{
		if (((stage == WAVE_PerturbStageDo) && (wave_info.stageprogress > 0.3f)) || (stage == WAVE_PerturbStageHold)
				|| (stage == WAVE_PerturbStageCollapse) || (stage == WAVE_PerturbStageHold)
				|| ((stage == WAVE_PerturbStageUndo) && (wave_info.stageprogress < 0.5f)))
		{
				//if (((position.x > crash_point.x && !left) || (position.x < crash_point.x && left)) && near_tube)
					return true;
		}
		else
			return false;
	}
	else if ((wave_info.type == WAVE_PerturbTypeBigRush) || (wave_info.type == WAVE_PerturbTypeRush))
	{
		return true;
	}

	return false;
}

/*	Should be out-of-date now. (dc 05/04/02)
static float BOARD_WipeoutToleranceD = 2.f;
static float BOARD_WipeoutToleranceZ = 7.f;
*/

float TOWARDS_TUBE_SPEED = 3.0f;
float TOWARDS_TUBE_SPEED_IN_TUBE = 6.0f;

// The proc for the replay.  Does some of the update
// tasks.  Its a wanna-be update proc
// -KS 10/12/01
int SurfBoardObjectClass::Downdate (float time_step)
{
  // Wipeout overrides physics.
  if (wiped_out)
  {
    wipeout_time += time_step;

    if (wipeout_time > max_wipeout_time)
    {
	  get_ks_controller()->ResetPhysics();
      return 1;
    }
  }

  return 0;
}

//extern int wipeTransition;


float TAKE_OFF_FRIC_MOD = 5.0f;
float TAKE_OFF_SKAG_MOD = 5.0f;
float WASH_CARVE_MOD = 4.5f;
float WASH_MOD = 3.0f;
float ROOF_MOD = 9.6f;
float ROOF_X_MOD = 66.0f;
float SPEED_MOD = 1.0f;
float TAKEOFF_CURRENT_MULT = 1.0f;
float TAKEOFF_CURRENT_X_MULT = 0.00f;
float toward_wave_force = 5.0f;
float PADDLE_MOD = 12.0f;
float max_stall_mod = 0.8f;
float grab_rail_mod = 0.2f;

int SurfBoardObjectClass::Update (float time_step)
{
//  vector3d	unitdir, waveGrindVec;
  vector3d	offset = rb->my_po.non_affine_slow_xform(node_offset);
  vector3d	position = rb->pos;
  float current_y_val = position.y;  // to be used for duckdiving
  vector3d	boardThickness(0, 0.1f, 0);
  int current_beach = g_game_ptr->get_beach_id ();
  int current_wave = WAVE_GetIndex ();
  bool in_back = false;
  last_region = current_region;
  vector3d board_norm = my_board->get_abs_po().non_affine_slow_xform (YVEC);

  WAVE_GetBreakInfo(&wave_info);	//  update wave info
  vector3d forward_vec = GetForwardDir();

  // Grinding overrides physics.
  if (do_grinding_stuff (time_step))
    return 1;

  // Wipeout overrides physics.
  if (wiped_out)
  {
//    frontendmanager.IGO->TurnOnTubeTimer(ksctrl->get_player_num(), false);
    ksctrl->tube_meter.End();
    //ksctrl->float_meter.End();
    wipeout_time += time_step;

    if ((wipeout_time > max_wipeout_time) && (ksctrl->get_super_state() == SUPER_STATE_WIPEOUT))
    {
	  ksctrl->ResetPhysics();
	  bool is_split_screen = g_game_ptr->is_splitscreen();
	  bool is_ai_player = get_ks_controller()->IsAIPlayer();
	  // We don't want to end the wave in splitscreen (Toby 01/29/02)
	  if (is_split_screen || is_ai_player)
	  {
	  }
	  else
	  {
		  WAVE_EndWave();	// Restart wave (dc 01/25/02)
	  }
    }
  }

  if (this->state == BOARD_ANIM_CONTROL)
  {
    debug_print("Board under animation control.  Untested.\n");
  }
  // Floater physics.
  else if (!inAirFlag && this->state == BOARD_FLOAT)
  {
	  BoardFloatWave(time_step);
  }
  else if (this->state == BOARD_IN_AIR)  //  this will only be set during some wipeouts and some exit moves
	  BoardInAir(time_step);
  // Normal physics.
  else
  {
    vector3d forces (0,0,0);
    //CollideCallStruct collide_center (rb->pos, &normal, &current, &current_region, &wave_center_hint);
	CollideCallStruct collide_center (rb->pos + offset, &normal, &current, &current_region, &wave_center_hint);
	if (center_hint_valid)
	{
		float deltadist = (rb->pos - prev_center_pos).length();
		collide_center.tolerance.dthresh = collide_center.tolerance.zthresh = deltadist + 1;
	}
	else
	{
		collide_center.tolerance.dthresh = 1;
		collide_center.tolerance.zthresh = 0;	// will trigger error if hint used (dc 05/04/02)
	}

	#ifdef BUILD_DEBUG
		// This assert is on my bug list. Let me know if it happens -EO
  assert(collide_center.position.is_valid());
	#endif

		// danger Will Robinson. This is panic code in case the above assertion
		//   never gets fixed. It may do something abnormal, but it won't crash
	  //   the game -EO
  if (!collide_center.position.is_valid())
	{
		collide_center.position=vector3d(0,0,0);
	}

/*
    if (wiped_out)
    {
      collide_center.tolerance = WaveTolerance(BOARD_WipeoutToleranceD, BOARD_WipeoutToleranceZ);
    }
*/
    float height;

    float grav = world_gravity*BeachDataArray[current_beach].gravity;

	if (this->inAirFlag)
	{
		grav = air_gravity*AIR_GRAV_MOD;
	}
	else if (ksctrl->get_current_state() == STATE_DUCKDIVE)
		grav = 0.0f;
	else if (ksctrl->get_super_state() == SUPER_STATE_LIE_ON_BOARD)
		grav = 5.0f;
	/*else // adjust normal so that surfer never gets upside down
	{
		if (normal.y < 0.0f)
		{
			normal.y = 0.0f;
			normal.normalize();
		}
	}*/

    // add weight
    vector3d weight (0, -rb->mass * grav, 0);
    forces = weight;

	if (exit_jump && this->inAirFlag)
	{
		float time_scale = 1.0f;
		if (air_time > 0.0f)
			time_scale = air_timer/(air_time*air_time);

		if (time_scale > 1.0f) // so that forces are always approx. the same for all exit jumps
			time_scale = 0.0f;

		forces *= 0.8f;
		forces += 15.0f*time_scale*ZVEC;
	}

	bool in_wash_region = (current_region == WAVE_REGIONWASH) || ((current_region == WAVE_REGIONTUBE) && !ksctrl->Z_Within_Tube());

    // set the forward force direction
    this->forward.dir = this->forwardDir;
    forward.dir = rb->my_po.non_affine_slow_xform (forward.dir);

	float wash_scale = 1.0f;
	if ((ksctrl->get_super_state() != SUPER_STATE_LIE_ON_BOARD) && in_wash_region)
	{
		if (TurnType == CARVE_TURN)
			wash_scale = WASH_CARVE_MOD;
		else
			wash_scale = WASH_MOD;
	}

    forces += forward.dir * forward.s * wash_scale;

    if ((ksctrl->get_current_state () == STATE_CONTROLLED_JUMP) ||
		(ksctrl->get_current_state () == STATE_LAUNCH) || (ksctrl->get_current_state () == STATE_CHOP_HOP))
    {
      // create a fake invisible wall to make the player jump straight up
      normal = vector3d (0, 0, -1);
      height = 1 + CONTACT_EPSILON;
    }
    else
    {
#if defined(TARGET_XBOX) || defined(TARGET_GC)
      assert(collide_center.position.is_valid());
#endif /* TARGET_XBOX JIV DEBUG */

      if ((position.x < WAVE_MeshMaxX) && (position.x > WAVE_MeshMinX) &&
        (position.z < WAVE_MeshMaxZ) && (position.z > WAVE_MeshMinZ))
      {
		  int current_beach = g_game_ptr->get_beach_id ();

		  //  Do some extra work to allow the surfer (and only the surfer) to go through the tube wall.
		  WaveRegionEnum old_region = *collide_center.region;	// otherwise possibly uninitialized (dc 01/29/02)
		  bool can_be_in_water_wall = false;
		  int super_state = ksctrl->get_super_state();
		  if (!BeachDataArray[current_beach].is_big_wave && (ksctrl->get_current_state() != STATE_ON_ROOF) &&
			(super_state == SUPER_STATE_NORMAL_SURF || super_state == SUPER_STATE_PREPARE_JUMP))
		  {
			  can_be_in_water_wall = true;
		  }

		bool duckdiving = (ksctrl->get_current_state() == STATE_DUCKDIVE);
        center_hint_valid = WAVE_CheckCollision (collide_center, center_hint_valid, !inAirFlag && !duckdiving, true);
		prev_center_pos = collide_center.position;
		if (can_be_in_water_wall &&
			(*collide_center.region == (WAVE_REGIONMAX + 1)) &&
			CanBustThroughTube(position))			//  Last check, don't allow wall riding past this point.
		{
			*collide_center.region = old_region;
			in_water_wall = true;
		}
		else if (in_water_wall)
		{
			ksctrl->SetShakeOff(true);
			ksctrl->get_my_scoreManager().AddGap(GAP_BUST_WALL_ANTARCTICA + current_beach);
			in_water_wall = false;
		}

		WAVE_GetGrindPosition(collide_center.hint, &wave_dummy_hint, collide_center.position, &float_pos);
		//wave_front_hint = wave_center_hint;
		if (inAirFlag)
			CorrectLipError(position, time_step);
      }
      else
      {
        collide_center.position.y = WATER_Altitude (collide_center.position.x, collide_center.position.z);
        WATER_Normal (collide_center.position.x, collide_center.position.z, collide_center.normal->x, collide_center.normal->y, collide_center.normal->z);
        center_hint_valid = false;
        *collide_center.region = WAVE_REGIONCURL; // not really, just to make the surfer wipe out
      }

	  ksctrl->SetWaveNorm(*(collide_center.normal));

#if defined(TARGET_XBOX) || defined(TARGET_GC)
      assert(collide_center.position.is_valid());
#endif /* TARGET_XBOX JIV DEBUG */

/*    This method runs into problems when the point returned by WAVE_CheckCollision is on the
      inside ceiling of the wave.  Since the normal points down, we get a negative value for
	  height, and the surfer wipes out.

	  A return value on the ceiling is possible, because we only attempt to match the surfer's
	  x- and z-position, not his y-position.  So we don't claim to have found the nearest wave
	  point to the surfer's location.

	  With either the old or the new version it is possible for the surfer to fall through the
	  top layers of the wave without wiping out.  (dc 10/25/01)

      // calculate the distance from the board to the surface of the water
      vector3d dist = collide_center.position - (position + offset);

      if (dot (dist, normal) > 0)
        height = -dist.length ();
      else
        height = dist.length ();
*/
			height = (position + offset).y - collide_center.position.y;

			// Contact with objects: check for wipeout
			if (!wiped_out && (ksctrl->get_current_state() != STATE_DUCKDIVE))
			{
				in_back = GoneOverLip(time_step);

				if (!in_back && CheckForWipeOut(collide_center, time_step))
				{
					DoWipeOut (wipeout_type);

					if (!get_ks_controller()->IsAIPlayer())
					{
						if (wipeout_type == WIP_LOW_STUMBLE_BACK)
							frontendmanager.IGO->GetHintManager()->SetHint(IGOHintManager::SurfingBackwards);
						else if (wipeout_type != WIP_TAKE_OFF_FLAT)
							frontendmanager.IGO->GetHintManager()->SetHint(IGOHintManager::HitTubeWall);
					}

					if (this->GetBoardState() == BOARD_IN_AIR)
					{
						lip_jump = false;
						launch_jump = false;
						return 1;
					}
				}
			}
		}

		if (!wiped_out && this->inAirFlag)
		{
//			get_ks_controller()->SetJumpedEntity(g_beach_ptr->surfer_over_entity(collide_center.position, 0.15f));
			get_ks_controller()->SetJumpedEntity(g_beach_ptr->surfer_over_entity(my_board->get_abs_position(), 1.0f));
		}

    collide_center.region = NULL;

    if (height < CONTACT_EPSILON)
    {
      float scale;
      // projection of the weight over the normal vector
      //scale = dot (normal, weight);
	  scale = dot (board_norm, weight);
      if (scale < 0)
        scale = -scale;

      //forces += (normal * scale);

	  // only add normal force if on the surface
	  if (dot(normal, YVEC) > 0.0f)
		forces += (board_norm * scale);
	// could be hitting wave on the way up (dc 10/25/01)
	// also only want to coolide with water that the surfer is falling towards
    if ((rb->velo.y < 0)/* && (dot(rb->linMom, normal) < 0.0f)*/)
	  {
      if (this->inAirFlag)
        last_f = 1.0f;

      // check if the surfer landed correctly after a jump
      if ((ksctrl->get_current_state () == STATE_FREEFALL_CHOP) ||
          (ksctrl->get_current_state () == STATE_FREEFALL) ||
          (ksctrl->get_current_state () == STATE_CONTROLLED_AIR) ||
          (ksctrl->get_current_state () == STATE_AIR_TRICK))
      {
#if defined(TARGET_XBOX) || defined(TARGET_GC)
        assert(collide_center.position.is_valid());
#endif /* TARGET_XBOX JIV DEBUG */

        // Add splash when surfer lands
        if (lip_jump)
          ksctrl->my_trail->create_big_landing_splash ();
        else
          ksctrl->my_trail->create_chophop_splash(collide_center.position);

#if defined(TARGET_XBOX) || defined(TARGET_GC)
        assert(collide_center.position.is_valid());
#endif /* TARGET_XBOX JIV DEBUG */

        omega = 0.0f;
        if (grind_jump && !grind_ollie && CheckForGrindLandingWipeOut(collide_center, time_step))
        {
          DoWipeOut (wipeout_type);
        }
		else if (!grind_jump && CheckForLandingWipeOut (collide_center, time_step))
        {
		  if (this->exit_jump)
			{
			  KlugeWipeout(SURFER_ANIM_W_1_TUM_EXIT, SURFER_ANIM_W_1_SWIM);

				// put it here I guess
				ks_fx_start_wipeout_splash (get_ks_controller()->get_player_num());
			}
		  else
			DoWipeOut (wipeout_type);

		  if (this->GetBoardState() == BOARD_IN_AIR)
		  {
			  lip_jump = false;
			  launch_jump = false;
			  return 1;
		  }
        }
        else
          rb->linMom.y *= 0.25f; // slow down to compensate the speed boost when the surfer jumped
      }

	  inAirFlag = false;
      lip_jump = false;
	  launch_jump = false;
    }
	}
    else
    {
      this->inAirFlag = true;
    }

    if (height < CONTACT_EPSILON)
    {
      // stay above the water
      position = collide_center.position - offset;
#if defined(TARGET_XBOX) || defined(TARGET_GC)
      assert(position.is_valid());
#endif /* TARGET_XBOX JIV DEBUG */
    }
    else if ((ksctrl->get_current_state () != STATE_CONTROLLED_JUMP) && (ksctrl->get_current_state () != STATE_CONTROLLED_AIR) &&
		(ksctrl->get_current_state () != STATE_AIR_TRICK) && (ksctrl->get_current_state () != STATE_LAUNCH) &&
		(ksctrl->get_current_state () != STATE_CHOP_HOP) && (ksctrl->get_current_state () != STATE_FREEFALL) &&
		(ksctrl->get_current_state () != STATE_FREEFALL_CHOP))
    {
      // stick the surfer to the water and avoid the small jumps
      position = collide_center.position - offset;
      this->inAirFlag = false;
	  lip_jump = false;
	  launch_jump = false;
#if defined(TARGET_XBOX) || defined(TARGET_GC)
      assert(position.is_valid());
#endif /* TARGET_XBOX JIV DEBUG */
    }

    if (!this->inAirFlag)
    {
		//  push surfer towards Wave mesh
		vector3d limit_forces = ZEROVEC;
		KeepSurferOnWave(position, limit_forces);
		forces += limit_forces;

	  float current_mod = 1.0f;
	  if (ksctrl->get_super_state() == SUPER_STATE_LIE_ON_BOARD)
		  current_mod = TAKEOFF_CURRENT_MULT;

      vector3d relative_velocity = (this->rb->linMom / this->rb->mass) - current_mod*current;
	  int super_state = ksctrl->get_super_state();
      float stick = ksctrl->CtrlEvent(PAD_LSTICK_V);

      if (wiped_out || (dot (forward.dir, YVEC) > 0) || (super_state == SUPER_STATE_LIE_ON_BOARD))
        stick = 0;
      else if (stick < 0)
        stick = -1;
      else if (stick > 0)
        stick = 1;

      if (ksctrl->get_current_state () != STATE_EXIT_FLOAT)
      {
		if (ksctrl->get_super_state() == SUPER_STATE_LIE_ON_BOARD)
			relative_velocity.x += current.x * TAKEOFF_CURRENT_X_MULT;
		else
			relative_velocity.x += current.x * 0.5f;

		// calculate the skag force perpendicular to the board
		vector3d skag = (relative_velocity - forward.dir * dot (relative_velocity, forward.dir)) * -skag_friction;

		float skag_force_mod = 1.0f;
		float fric_mod = 1.0f;
		if (ksctrl->get_current_state() == STATE_DUCKDIVE)
		{
			skag_force_mod = 0.0f;
			fric_mod = 0.0f;
			rb->linMom += forwardDir*DUCKDIVE_ACC*time_step;
			if (dot(rb->linMom, forwardDir) > DUCKDIVE_MAX_VEL)
			{
				rb->linMom -= forwardDir*dot(forwardDir, rb->linMom);
				rb->linMom += forwardDir*DUCKDIVE_MAX_VEL;
			}
		}
		else if (ksctrl->get_super_state() == SUPER_STATE_LIE_ON_BOARD)
		{
			/*float pad_mod = PADDLE_MOD;
			float current_mag = current.length();
			if (pad_mod > (current_mag - 0.5f))
				pad_mod = current_mag - 0.5f;

			relative_velocity = (this->rb->linMom / this->rb->mass) - (current - pad_mod*ksctrl->CtrlEvent(PAD_LSTICK_V)*forward.dir);
			skag = (relative_velocity - forward.dir * dot (relative_velocity, forward.dir)) * -skag_friction;*/
			float lie_wash_scale = 1.0f;
			if (in_wash_region)
				lie_wash_scale = 2.0f;

			fric_mod = TAKE_OFF_FRIC_MOD*lie_wash_scale;
			skag_force_mod = TAKE_OFF_SKAG_MOD*lie_wash_scale;
		}
		else if ((ksctrl->get_current_state() == STATE_ON_ROOF) || (current_region == WAVE_REGIONROOF))
		{
			if (ksctrl->get_current_state() == STATE_LANDING)
			{
				relative_velocity = current*0.6f;

				// recalc skag
				skag = (relative_velocity - forward.dir * dot (relative_velocity, forward.dir)) * -skag_friction;
				skag = lip_vec*dot(lip_vec, skag);
				relative_velocity = lip_vec*dot(lip_vec, relative_velocity);
				vector3d grav_force = weight - board_norm * dot (board_norm, weight);
				if (dot(grav_force, ZVEC) > 0.0f)
					forces -= 1.0f*grav_force;

				rb->linMom = lip_vec*dot(lip_vec, rb->linMom);
			}
			else
			{
				relative_velocity = current*0.6f;

				// recalc skag
				skag = (relative_velocity - forward.dir * dot (relative_velocity, forward.dir)) * -skag_friction;
				vector3d grav_force = weight - board_norm * dot (board_norm, weight);
				if (dot(grav_force, ZVEC) > 0.0f)
					forces -= 2.0f*grav_force;

				if (rb->linMom.z > 0.0f)
					rb->linMom.z = 0.0f;
			}
		}
		else if ((ksctrl->get_super_state() != SUPER_STATE_LIE_ON_BOARD) &&
			((current_region == WAVE_REGIONWASH) || ((current_region == WAVE_REGIONTUBE) && !ksctrl->Z_Within_Tube())))
		{
			skag_force_mod = WASH_FRICTION_MOD;
			fric_mod = WASH_FRICTION_MOD;
		}
		else if ((this->TurnType == TAILSLIDE_TURN) || (this->TurnType == GRABSLIDE_TURN))
			skag_force_mod = TAILSLIDE_FRICTION_MOD;
		else if (!TongueEffectOn()) //  the increased friction makes it hard to get out of the tongue
		{
			vector3d lip_pos2 = *WAVE_GetMarker(WAVE_MarkerLipMark13);
			vector3d lip_pos1 = *WAVE_GetMarker(WAVE_MarkerLipMark11p8);
			float length = dot(lip_pos2 - lip_pos1, lip_vec);
			vector3d surfer_lip = rb->pos - lip_pos1;
			float surfer_length = dot(surfer_lip, lip_vec);
			if (surfer_length > 0.0f)
			{
				fric_mod += DOWN_FACE_FRIC_MOD*surfer_length/length;
			}
		}

		//  limit friction so that at low frame rates there is not unstabiltity
		float limit_fric_skag = (time_step > 0.0f)?(1.0f/(time_step*skag_friction)):1.0f;
		float limit_fric_fric = (time_step > 0.0f)?(1.0f/(time_step*water_friction)):1.0f;
		limit_fric_fric *= 0.95f;
		limit_fric_skag *= 0.95f;
		if (fric_mod > limit_fric_fric)
			fric_mod = limit_fric_fric;

		if (skag_force_mod > limit_fric_skag)
			skag_force_mod = limit_fric_skag;

		forces += skag_force_mod*skag;
		forces += fric_mod*relative_velocity * -water_friction;
        forces += forward.dir * (-stick * stick_factor);

        // don't let the surfer get away from the wave
		if (((current_region == WAVE_REGIONFRONT) || (current_region == WAVE_REGIONWOBBLE)) &&
					((super_state == SUPER_STATE_NORMAL_SURF) || (super_state == SUPER_STATE_PREPARE_JUMP)))
		  forces += toward_wave_force*ZVEC;
        else if ((dot (rb->my_po.get_y_facing (), YVEC) > 0.966f) && (super_state != SUPER_STATE_LIE_ON_BOARD))
          forces += ZVEC;

		if (TurnType == SNAP_TURN)
		{
			//forces += 5.0f*snap_vec;
			forces.x = 0.0f;
			if (rb->linMom.z > 0.0f)
				rb->linMom.z = 0.0f;

			if (forces.z > 0.0f)
				forces.z = 0.0f;
		}
      }

	  forces += controller_force;
	  if (in_back && (forces.z > 0.0f))
		  forces.z = 0.0f;

      forces *= speed_factor*ksctrl->GetAttribs(ATTRIB_SPEED)*SPEED_MOD;
	  //forces *= SPEED_MOD;
    }

	int current_state = ksctrl->get_current_state ();
	if (!this->inAirFlag &&
		(current_state == STATE_STALL || current_state == STATE_SUPER_STALL ||
		current_state == STATE_TUBE_RAILGRAB))	//  Small waves should pull you towards the middle in the tube
	{
		vector3d vel = this->rb->linMom / this->rb->mass;
		float acc = MAX_STALL_ACCELERATION*time_step;
		float val = dot(forward_vec, XVEC);
		if (val > 0.25f)
			vel.x -= acc;
		else if (val < -0.25f)
			vel.x += acc;

		if (current_state == STATE_STALL)
		{
			if (vel.x > MAX_STALL_VELOCITY)
				vel.x = MAX_STALL_VELOCITY;
			else if (vel.x < -MAX_STALL_VELOCITY)
				vel.x = -MAX_STALL_VELOCITY;
		}
		else  //  Super stall is *really* fast.
		{
			if (vel.x > 3 * MAX_STALL_VELOCITY)
				vel.x = 3 * MAX_STALL_VELOCITY;
			else if (vel.x < -(3 * MAX_STALL_VELOCITY))
				vel.x = -(3 * MAX_STALL_VELOCITY);
		}

		//  Distance from the tube affects the velocity of the board while stalling.
		float tube_distance = max( 0.0f, min(max_stall_mod, ksctrl->Closest_Tube_Distance()));
		vector3d main_tube_start = *WAVE_GetMarker(WAVE_MarkerLipMark6);
		vector3d board_pos = my_board->get_abs_position();
		bool left_wave = BeachDataArray[g_game_ptr->get_beach_id ()].bdir;
		float along_wave = (position.x - WAVE_MeshMinX)/(WAVE_MeshMaxX - WAVE_MeshMinX);
		assert(along_wave >= 0 && along_wave <= 1);

		float tube_middle_z = (WaveDataArray[current_wave].tubecenstart_z * (1 - along_wave)) +
			(WaveDataArray[current_wave].tubecenstop_z * along_wave);


		float delta_from_tube_center = tube_middle_z - position.z;
		if ((board_pos.x < main_tube_start.x && left_wave) ||
			(board_pos.x > main_tube_start.x && !left_wave))
			vel.z = (1 - tube_distance) * (delta_from_tube_center * TOWARDS_TUBE_SPEED);
		else  //  move towards the center slowly when in the tube.
			vel.z = delta_from_tube_center * TOWARDS_TUBE_SPEED_IN_TUBE;

		if (current_state == STATE_TUBE_RAILGRAB)  //  not quite so fast when in the tube grabbing your rail.
			vel.z *= grab_rail_mod;

		rb->linMom = vel*this->rb->mass;
	}
	else if (ksctrl->get_super_state() == SUPER_STATE_IN_TUBE)  //  special physics for being in the tube.
	{
		//  Set the surfer at normal speed based on his direction.
		vector3d vel = this->rb->linMom / this->rb->mass;
		float acc = MAX_STALL_ACCELERATION*time_step;
		float val = dot(forward_vec, XVEC);
		if (val > 0.25f)
			vel.x -= acc;
		else if (val < -0.25f)
			vel.x += acc;

		if (vel.x > MAX_STALL_VELOCITY)
			vel.x = MAX_STALL_VELOCITY;
		else if (vel.x < -MAX_STALL_VELOCITY)
			vel.x = -MAX_STALL_VELOCITY;

		//  No default left/right or up/down.
		vel.z = 0;
		vel.y = 0;

		rb->linMom = vel*this->rb->mass;
		rb->linMom += forward.dir * forward.s;
	}
	// Calc normal linear momentum.
	else if (ksctrl->get_current_state() != STATE_DUCKDIVE)
		this->rb->linMom += (forces * time_step);

    // adjust the momentum to the wave surface
    if (!this->inAirFlag && (ksctrl->get_current_state() != STATE_DUCKDIVE))// && (dot(normal,YVEC) > 0.0f))
    {
		if (ksctrl->get_current_state() != STATE_DUCKDIVE)
		{
		  vector3d vel;
		  float scale;

		  vel = this->rb->linMom / this->rb->mass;
		  scale = vel.length ();

		  vel -= normal * dot (normal, vel);
		  vel.normalize ();

		  vel *= scale;
		  rb->linMom = vel;
		}

    // keep the surfer on the wave
    #define LIMIT(min, a, max) ( (a) =										\
		(a) < (min) ? (center_hint_valid = front_hint_valid = false, min) :	\
		(a) > (max) ? (center_hint_valid = front_hint_valid = false, max) :	\
		(a)																	\
	)

    vector3d boundsMin, boundsMax;

    WAVE_GetCollisionBox(boundsMin, boundsMax, 5.0f, 5.0f);

    LIMIT(boundsMin.x, position.x, boundsMax.x);
    LIMIT(boundsMin.z, position.z, boundsMax.z);

    // align the surfboard with the wave surface
      vector3d x, y, z, front (0,0,0.5);
      front = rb->my_po.non_affine_slow_xform (front);
      front += position;

	  float f = DEG_TO_RAD (maxLeanAngle * turnDegree);
	  float sc = 1.0f;
	  if (time_step != 0.0f)
		  sc = 0.4f/time_step;

	  f = (sc*last_f + f)/sc;
	  last_f = f;

      if (f != 0)
        f = __fabs (curLeanAngle) / f;

	  /*	Board normal can vary between wave normal and vertical.  Never let it get
			too far from the wave normal.  This fixes some orientation problems near
			vertical parts of the wave. (dc 10/25/01)
	  */
      if (f < .5f) f = .5f;
      y = ((1 - f) * YVEC) + (f * normal);

	  CollideCallStruct collide_front(front, NULL, NULL, NULL, &wave_front_hint);
	  static vector3d prev_front_pos;
	  if (front_hint_valid)
	  {
		  float deltadist = (rb->pos - prev_front_pos).length();
		  collide_front.tolerance.dthresh = collide_front.tolerance.zthresh = deltadist + 1;
	  }
	  else
	  {
		  collide_front.tolerance.dthresh = 1;
		  collide_front.tolerance.zthresh = 0;	// will trigger error if hint used (dc 05/04/02)
	  }
	  /*	We expect a near match whether or not the hint is valid, since we are on the
			wave surface (not in the air).  Otherwise, we may sometimes get a bad return
			position.  In such a case, we can get essentially random values for "front".
			(dc 10/31/01)
	  */
      front_hint_valid = WAVE_CheckCollision (collide_front, front_hint_valid, true, true);
//      WAVE_CheckCollision (collide_front, front_hint_valid, true, front_hint_valid);
	prev_front_pos = collide_front.position;

 	  /*	We want to use the entire position, not just the y-value.  Otherwise, "front"
			will not be on the wave surface, and we can get the board to stick out of the
			wave.  (dc 10/31/01)
	  */
     front = collide_front.position;
//      front.y = collide_front.position.y;

      z = front - position;
	  //z -= normal*dot(normal, z);
      z.normalize();

	  /*	If this happens, the board will stick straight out of the water, and we'll
			be unable to fix its orientation in future passes. (dc 10/31/01)
	  */

      if (fabsf(dot(z, normal)) > .99f)
      {
        // Use the previous position and hope we get it right next frame
        front = position + rb->my_po.non_affine_slow_xform (ZVEC);
        z = front - position;
        z.normalize();
        front_hint_valid = false;
      }

#if 0
#if defined(TARGET_XBOX) || defined(TARGET_GC)
      float err = fabsf(dot(z, normal));

      assert( err < .99f );
#else
#if !defined(EVAN) && !defined(TOBY) && !defined(LEO) && !defined(JWEBSTER) && !defined(JASON) && !defined(KEVIN)
      assert(fabsf(dot(z, normal)) < .99f);
#endif
#endif /* TARGET_XBOX JIV DEBUG */
#endif // 0

      x = cross (y, z);
      x.normalize();
      y = cross (z, x);

	  if ((ksctrl->get_current_state() != STATE_ON_ROOF) && (ksctrl->get_current_state() != STATE_DUCKDIVE)
						&& (current_region != WAVE_REGIONROOF))
		rb->my_po = po (x, y, z, vector3d(0,0,0));
    }
	else
	{
		// Failure to turn off this flag causes bad return values from WAVE_CheckCollision.
		// We could keep this flag on if we kept calling WAVE_CheckCollision while the guy
		// was in the air. (dc 10/26/01)
		front_hint_valid = false;
	}

	// ensure that duck diving is actually a dive
	if (ksctrl->get_current_state() == STATE_DUCKDIVE)
	{
		if (position.y > current_y_val)
			position.y = current_y_val;
	}

#if defined(TARGET_XBOX) || defined(TARGET_GC)
    assert(rb->my_po.get_position().is_valid());
    assert(position.is_valid());
#endif /* TARGET_XBOX JIV DEBUG */

    // set the board entity position considering the board thickness
    boardThickness = rb->my_po.non_affine_slow_xform (boardThickness);

	offset = rb->my_po.non_affine_slow_xform(node_offset);
    my_board->set_rel_po (rb->my_po);
    my_board->set_rel_position (position + offset + boardThickness);

/*  Nice try.

#if defined(DEBUG) && !defined(EVAN) && !defined(JWEBSTER)
	assert ((rb->pos - position).length2() < 25);	// no sudden pops in position
#endif

*/

    this->rb->pos = position;

    // make the surfer lean when he's turning

	po lean;
	float desiredLeanAngle = 0, torque_turn = 0;
	float deltaLeanAngle;

	if (this->state == BOARD_TURN_LEFT)
	{
		desiredLeanAngle = DEG_TO_RAD(-this->maxLeanAngle * (torque_turn + this->turnDegree));
	}
	else if(this->state == BOARD_TURN_RIGHT)
	{
		desiredLeanAngle = DEG_TO_RAD(this->maxLeanAngle * (torque_turn + this->turnDegree));
	}

	deltaLeanAngle = desiredLeanAngle - this->curLeanAngle;
	this->curLeanAngle += deltaLeanAngle * this->leanEaseIn * time_step;
	lean.set_position (vector3d (0, 0, 0));
	lean.set_rot (this->forwardDir, this->curLeanAngle);
	my_board->set_rel_po (lean * my_board->get_rel_po());

	if (this->inAirFlag && !wiped_out)
		air_timer += time_step;
  }

  return 1;
}

float limit_gain = 25.0f;
void SurfBoardObjectClass::KeepSurferOnWave(vector3d &position, vector3d &forces)
{
	forces = ZEROVEC;
	int super_state = ksctrl->get_super_state();
	if (super_state != SUPER_STATE_LIE_ON_BOARD)
	{
		vector3d BotLeft = *WAVE_GetMarker(WAVE_MarkerBottomLeft);
		vector3d TopRight = *WAVE_GetMarker(WAVE_MarkerTopRight);
		vector3d boundsMin, boundsMax;
		float bound_min_x, bound_max_x, bound_min_z, bound_max_z;
		if (TopRight.x > BotLeft.x)
		{
			bound_max_x = TopRight.x;
			bound_min_x = BotLeft.x;
		}
		else
		{
			bound_max_x = BotLeft.x;
			bound_min_x = TopRight.x;
		}

		if (TopRight.z > BotLeft.z)
		{
			bound_max_z = TopRight.z;
			bound_min_z = BotLeft.z;
		}
		else
		{
			bound_max_z = BotLeft.z;
			bound_min_z = TopRight.z;
		}

		WAVE_GetCollisionBox(boundsMin, boundsMax, 1.0f, 1.0f);
		if (fabs(position.x - boundsMin.x) < fabs(bound_min_x - boundsMin.x))
		{
			forces += (limit_gain - fabs(bound_min_x - boundsMin.x))*XVEC;
		}
		else if (fabs(position.x - boundsMax.x) < fabs(bound_max_x - boundsMax.x))
		{
			forces += -(limit_gain - fabs(bound_max_x - boundsMax.x))*XVEC;
		}

		if (fabs(position.z - boundsMin.z) < fabs(bound_min_z - boundsMin.z))
		{
			forces += (limit_gain - fabs(bound_min_z - boundsMin.z))*ZVEC;
		}
		else if (fabs(position.z - boundsMax.z) < fabs(bound_max_z - boundsMax.z))
		{
			forces += -(limit_gain - fabs(bound_max_z - boundsMax.z))*ZVEC;
		}
	}
}


float GRIND_MAG = 3.0f;
void SurfBoardObjectClass::BoardFloatWave(const float time_step)
{
	vector3d	unitdir;
	this->forward.dir = this->forwardDir;
	forward.dir = rb->my_po.non_affine_slow_xform (forward.dir);
	vector3d normal, position (rb->pos);
	CollideCallStruct collide (rb->pos, &normal, &current, &current_region, &wave_center_hint);
	if (center_hint_valid)
	{
		float deltadist = (rb->pos - prev_center_pos).length();
		collide.tolerance.dthresh = collide.tolerance.zthresh = deltadist + 1;
	}
	else
	{
		collide.tolerance.dthresh = 1;
		collide.tolerance.zthresh = 0;	// will trigger error if hint used (dc 05/04/02)
	}

	center_hint_valid = WAVE_CheckCollision (collide, !inAirFlag && center_hint_valid, !inAirFlag, !inAirFlag && center_hint_valid);
	position = prev_center_pos = collide.position;
	WAVE_GetGrindPosition(collide.hint, &wave_center_hint, collide.position, &float_pos);
	//position = float_pos;
	wave_front_hint = wave_center_hint;
	front_hint_valid = center_hint_valid;

	unitdir = get_ks_controller()->GetGrindVector();
	vector3d velo;
	/*float mag, dir = dot(XVEC, forward_vec);
	if (dir > 0.0f)
	  mag = 0.0f;
	else
	  mag = -0.0f;*/

	float_speed -= dec_percent*FLOATER_DECELERATION*time_step;
	if (float_speed > MAX_FLOAT_SPEED)
		float_speed = MAX_FLOAT_SPEED;
	//else if (float_speed < MIN_FLOAT_SPEED)
		//float_speed = MIN_FLOAT_SPEED;

	velo = unitdir*float_speed;
	//velo = ZEROVEC;
	this->rb->linMom = velo*this->rb->mass;

	vector3d error = float_pos - position;
	error -= unitdir*dot(error, unitdir);
	float error_mag = GRIND_MAG;
	vector3d add_mom = ZEROVEC;
	add_mom += 2.0f*error_mag*error*time_step*30.0f;
	add_mom -= normal*dot(add_mom, normal);

	this->rb->linMom += add_mom;

	// set the board entity position considering the board thickness
	vector3d board_thick (0, 0.1f, 0);
	board_thick = rb->my_po.non_affine_slow_xform (board_thick);
	board_thick = ZEROVEC;

	//rb->my_po = po (XVEC,YVEC,ZVEC, vector3d (0,0,0));

	//  Reorient the surfer's root to the wave surface
	ksctrl->OrientToWave(true, time_step, SET_RB_PO);
	my_board->set_rel_po (rb->my_po);
	my_board->set_rel_position (position +  board_thick);

	this->rb->pos = position;

	po lean;
	float desiredLeanAngle = 0;
	float deltaLeanAngle;

	deltaLeanAngle = desiredLeanAngle - this->curLeanAngle;
	this->curLeanAngle += deltaLeanAngle * this->leanEaseIn * time_step;
	lean.set_position (vector3d (0, 0, 0));
	lean.set_rot (this->forwardDir, this->curLeanAngle);
	my_board->set_rel_po (lean * my_board->get_rel_po());
	omega = 0.0f;

	current_region = WAVE_REGIONLIP;

    vector3d boundsMin, boundsMax;

    WAVE_GetCollisionBox(boundsMin, boundsMax, 1.0f, 1.0f);

    LIMIT(boundsMin.x, position.x, boundsMax.x);
    LIMIT(boundsMin.z, position.z, boundsMax.z);

	// check for collision with other objects
	entity *hitme = g_beach_ptr->check_entity_collision(ksctrl, grind_object);

	//  Let the wipeout cam know what the object was that the surfer collided with, if any
	(ksctrl->get_wipeout_cam_ptr())->set_collision_obj(hitme);

	if (g_beach_ptr->get_smashed_entity())
	{
		sfx.collided(my_board_model, g_beach_ptr->get_smashed_entity());
	}

	if (hitme)
	{
		sfx.collided(my_board_model, hitme);
		DoWipeOut(WIP_TAKE_OFF_FLAT);
	}
}

void SurfBoardObjectClass::BoardInAir(const float time_step)
{
	vector3d normal, position (rb->pos);
	CollideCallStruct collide (rb->pos, &normal, &current, &current_region, &wave_center_hint);
	center_hint_valid = WAVE_CheckCollision (collide, !inAirFlag && center_hint_valid, !inAirFlag, !inAirFlag && center_hint_valid);
	front_hint_valid = false;

	float height = position.y - collide.position.y;

	float grav = 15.0f;
	rb->linMom -= grav*time_step*YVEC*rb->mass;

	vector3d tube_bottom = *WAVE_GetMarker(WAVE_MarkerTubeBottom);
	float height2 = position.y - tube_bottom.y;

	if ((current_region == WAVE_REGIONCEILING) || (current_region == WAVE_REGIONSHOULDER))
	{
		center_hint_valid = false;
		height = 2.0f;  // set greater than zero, ignore collision
	}

	if ((rb->linMom.y <= 0.0f) && (((height < 0.0f) && (normal.y > 0.0f) && (dot(rb->linMom, normal) < 0.0f)
			&& (current_region != WAVE_REGIONCEILING)) || (height2 < 0.0f)))
	{
		if ((height2 > 0.0f) && (current_region == WAVE_REGIONROOF)
				&& (dot(rb->linMom, rb->linMom) > 0.5f))
		{
			vector3d vel = normal*dot(normal, rb->linMom);
			rb->linMom -= 1.5f*vel;
		}
		else
		{
			this->SetBoardState(BOARD_NONE);
			this->inAirFlag = false;
			//wave_front_hint = wave_center_hint;
			//front_hint_valid = true;

			if (height > 0.0f)
				collide.position.y = tube_bottom.y;
		}

		position = collide.position;
	}

	my_board->set_rel_po (rb->my_po);
	my_board->set_rel_position (position);

	this->rb->pos = position;

	po lean;
	float desiredLeanAngle = 0;
	float deltaLeanAngle;

	deltaLeanAngle = desiredLeanAngle - this->curLeanAngle;
	this->curLeanAngle += deltaLeanAngle * this->leanEaseIn * time_step;
	lean.set_position (vector3d (0, 0, 0));
	lean.set_rot (this->forwardDir, this->curLeanAngle);
	my_board->set_rel_po (lean * my_board->get_rel_po());

	ksctrl->OrientToWave(false, time_step, SET_RB_PO);
}


void SurfBoardObjectClass::CorrectLipError(vector3d &pos, float dt)
{
	if (lip_jump && float_jump && !exit_jump)
	{
		vector3d delta = -0.5f*ZVEC;
		if (TongueEffectOn())
		{
			delta = ZEROVEC;
		}

		vector3d vec1 = rb->pos - (float_pos + delta);
		vec1.y = 0.0f;
		//vec1.normalize();
		rb->pos -= vec1*dt*4.0f;
		pos = rb->pos;
	}

}

float SurfBoardObjectClass::GetLeanPercentage(void)
{
	float percent = this->curLeanAngle/DEG_TO_RAD(this->maxLeanAngle);
	return percent;
}


float SurfBoardObjectClass::CalculatePathPeakTime(void)
{
	float vel = rb->linMom.y/rb->mass;
	float grav = -air_gravity*AIR_GRAV_MOD/rb->mass;
	if (grav == 0.0f)
		return 0.0f;

	return (-vel/grav);
}

void SurfBoardObjectClass::ResetFloatSpeed(bool max)
{
	if (max)
		float_speed = MAX_FLOAT_SPEED;
	else
	{
		vector3d speed = rb->linMom - WAVE_CURRENT_MOD*current;
		speed.y = 0.0f;
		float_speed = 2.5f*speed.length();
	}
}


void SurfBoardObjectClass::IncrementFloatSpeed(void)
{
	float_speed += FLOAT_OLLIE_INCREMENT;
}

bool SurfBoardObjectClass::CollideWithLip(void)
{
	//  Collide surf board with cylinder of radius 1.5f meters along lip
	/*vector3d diff_vec1 = *WAVE_GetMarker(WAVE_MarkerLipMark11) +
					1.0f*(*WAVE_GetMarker(WAVE_MarkerLipMark12) - *WAVE_GetMarker(WAVE_MarkerLipMark11));
	vector3d diff_vec2 = *WAVE_GetMarker(WAVE_MarkerLipMark10) +
					0.75f*(*WAVE_GetMarker(WAVE_MarkerGrindStart) - *WAVE_GetMarker(WAVE_MarkerLipMark10));
	vector3d vec1 = my_board->get_abs_position() - diff_vec1;
	vector3d vec2 = my_board->get_abs_position() - diff_vec2;
	vec1.y = 0.0f;
	vec2.y = 0.0f;

	//  Take out component along lip
	vector3d vec3 = vec1 - lip_vec*dot(vec1, lip_vec);
	vector3d vec4 = vec2 - lip_vec*dot(vec2, lip_vec);

	float sq_length1 = dot(vec3, vec3);
	float sq_length2 = dot(vec4, vec4);
	if ((sq_length1 < 1.0f) || (sq_length2 < 1.0f))
	{
		//  now see if surfboard is within the ends of the cylinder
		if (dot(vec1, vec2) < 0.0f)
			return true;
	}*/

	vector3d vec1 = my_board->get_abs_position() - float_pos;
	if (float_jump)
		vec1.y = 0.0f;

	if (dot(vec1, vec1) < 0.4f)
		return true;


	return false;
}

//  useful for tubes, this changes player velocity, but only along the X-axis.
void SurfBoardObjectClass::MoveForwardOnX(float degree)
{
//	int current_beach = g_game_ptr->get_beach_id ();

	forward.SetForceScalar(props_forwardForce * degree);

	//  Set the surfer at normal speed based on his direction.
	vector3d vel = this->rb->linMom / this->rb->mass;

	//  No left/right or up/down.
	vel.x *= fabs(lip_vec.x);
	vel.y *= fabs(lip_vec.y);
	vel.z *= fabs(lip_vec.z);

	rb->linMom = vel*this->rb->mass;
	rb->linMom += forward.dir * forward.s;
}
