
#include "global.h"
//#pragma hdrstop

#if defined(TARGET_XBOX)
#include "conglom.h"
#include "inputmgr.h"
#include "profiler.h"
#endif /* TARGET_XBOX JIV DEBUG */

#include "kellyslater_controller.h"
#include "board.h"


#include "blendmodes.h"
#include "inputmgr.h"
#include "commands.h"
#include "mouse.h"
#include "joystick.h"
#include "keyboard.h"
#include "switch_obj.h"
// BIGCULL #include "manip_obj.h"
#include "physical_interface.h"
// BIGCULL #include "damage_interface.h"
#include "collide.h"
#include "osdevopts.h"
#include "file_finder.h"
#include "widget.h"
#include "entity_maker.h"
#include "entityflags.h"
#include "interface.h"
#include "fogmgr.h"
#include "app.h"
#include "game.h"
#include "wds.h"
#include "geomgr.h"
#include "debug_render.h"
#include "ai_interface.h"
#include "hwaudio.h"
#include "inputmgr.h"
#include "script_access.h"
#include "matfac.h"
#include "beach.h"
#include "timer.h"
#include "wavesound.h"
#include "FrontEndManager.h"
#include "trickdata.h"
#include "trick_system.h"
#include "GameData.h"
#include "SFXEngine.h"
#include "wavedata.h"
#include "BoardManager.h"
#include "wipeoutdata.h"
#include "game_info.h"
#include "ks_camera.h"
#include "sounddata.h"
#include "soundscript.h"
#include "blur.h"
#if defined(TARGET_XBOX)
#include "ksfx.h"
#include "floatobj.h"
#include "FEPanel.h"
#include "text_font.h"

extern int g_disable_wipeouts;

#endif /* TARGET_XBOX JIV DEBUG */

float TOTAL_DUCKDIVE_TIME = 1.0f;  //  How long it takes to dive under the wave before the surfer is reset.

extern SurferTrick GTrickList[TRICK_NUM];

extern int g_perfect_balance;
extern int g_debug_balance;

char g_dump_animation_info = 0;
bool g_IK_enabled =true;
bool g_no_tween = false;

stringx g_surfer_anims[_SURFER_NUM_ANIMS] =
{
#define MAC(a, b) b,
#include "kellyslater_shared_anims_mac.h"
#include "slater_ind_anims_mac.h"
#include "robb_ind_anims_mac.h"
#include "machado_ind_anims_mac.h"	
#include "irons_ind_anims_mac.h"
#include "frankenreiter_ind_anims_mac.h"
#include "fletcher_ind_anims_mac.h"
#include "curren_ind_anims_mac.h"
#include "carrol_ind_anims_mac.h"
#include "andersen_ind_anims_mac.h"
#undef MAC
};

stringx g_board_anims[_BOARD_NUM_ANIMS] =
{
#define MAC(a, b) b,
#include "board_anims_mac.h"
#undef MAC
};

#ifdef DEBUG
#define MAC(a) #a,
char *g_kscontroller_state[] =
{
#include "kellyslater_states_mac.h"
};
#undef MAC
const char *gp_ksstate_string[2];
const char *gp_ksanim_string[2];
#endif

stringx surfer_anims[_SURFER_NUM_ANIMS];
stringx board_anims[_BOARD_NUM_ANIMS];

#define SURFER_ANIM_FORWARDS  1
#define SURFER_ANIM_REVERSE  -1
#define SURFER_ACCELERATION 0.05f
float SURFER_CARVETURN_ACCELERATION = 0.50f;
float SURFER_BIG_WAVE_ACCELERATION = 0.5f;
float SURFER_CROUCH_ACCELERATION = 0.50f;
float SURFER_LIE_PULLBACK = 0.2f;
float SURFER_LIE_PADDLE_ACCELERATION = 0.25f;
float SURFER_DUCKDIVE_ACC = 1.0f;
#define MIN_OLLIE_CHARGE 0.2f
#define STALL_THRESHOLD 0.85f

#define MAX_BOARD_ATTRIB 10
#define MAX_PLAYER_ATTRIB 20

int debug_mode = 0;
int anim_num = 0;
int anim_num_last = 0;

float WAVE_CURRENT_MOD = 0.28f;
/* Shouldn't all these be member variables!?!?!?!

#define NUM_STAND_ANIMS 6
#define NUM_CARVE_ANIMS 18
#define NUM_TURN_ANIMS 18


int stand_anim_nums[NUM_STAND_ANIMS];
int carve_anim_nums[NUM_CARVE_ANIMS];
int turn_anim_nums[NUM_TURN_ANIMS];
*/

int	SET_RB_PO = 1;
int	SET_OWNER_PO = 2;
int	SET_BOARD_PO = 4;

#if defined(TARGET_XBOX)
bool spin_controller::is_valid( void ) const
{
	return _finite(scale_factor) && !_isnan(scale_factor);
}
#endif /* TARGET_XBOX JIV DEBUG */

spin_controller::spin_controller()
{
	my_board_controller = NULL;
	spin_time[SPIN_180] = 0.4f;
	spin_time[SPIN_360] = 0.96f;
	spin_time[SPIN_540] = 1.44f;
	spin_time[SPIN_720] = 1.92f;
	activated = false;
	num_spins = 0;
}

void spin_controller::frame_advance(float dt)
{
#if defined(TARGET_XBOX)
  assert( is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */

	if (activated)
	{
		float R1 = my_ks_controller->CtrlEvent(PSX_R1);
		float L1 = my_ks_controller->CtrlEvent(PSX_L1);
		if ((spin_type < SPIN_MANUAL) && !R1 && !L1 && (depress_timer > 0.2f))
		{
			Reset();
			return;
		}
		else if (!R1 && !L1)
			depress_timer = 0.0f;
		else if (R1 || L1)
			depress_timer += dt;

		float scale = scale_factor;
		float temp_time = spin_time[spin_type];
		if (spin_type < SPIN_MANUAL)
		{
			float attr = my_ks_controller->GetAttribs(ATTRIB_ROTATE);
			if (attr > 0.0f)
			{
				temp_time /= attr;
				scale *= attr;
			}
		}

		if (time > temp_time)
		{
			num_spins--;
			if ((num_spins == 0) && (spin_type < SPIN_MANUAL) && (R1 || L1))
			{
				num_spins++;
				time = 0.0f;
			}
			else if ((num_spins == 0) || (spin_type >= SPIN_MANUAL))
			{
				Reset();
				return;
			}
			else
			{
				time = 0.0f;
			}
		}

		time += dt;
		if ((temp_time < time) && ((spin_type < SPIN_MANUAL) && (num_spins > 1)))
			dt = temp_time - (time - dt);

#if defined(TARGET_XBOX)
    assert(is_valid());
#endif /* TARGET_XBOX JIV DEBUG */

		float angle = scale*dt*spin_dir;
		if (spin_type < SPIN_FLOATER)
			my_ks_controller->get_board_controller().CalculateAirTurn(DEG_TO_RAD(angle));

		my_board_controller->TrickSpin(angle);
	}
}


//  this function returns the spin axis because it can be used for many things
vector3d spin_controller::SetSpinType(int type, float t, vector3d final_vec, float time_mult)
{
#if defined(TARGET_XBOX)
    assert(final_vec.is_valid());
    assert(is_valid());
    assert(!_isnan(time_mult));
    assert(_finite(time_mult));
    assert(!_isnan(t));
    assert(_finite(t));
#endif /* TARGET_XBOX JIV DEBUG */

	spin_axis = ZEROVEC;
	if (type < SPIN_MANUAL)
	{
		if (t == 0.0f)
			return spin_axis;

		float mod = t/fabs(t);
			spin_dir = mod*1.0f;

		spin_type = type;
		activated = true;
		float amt = 180.0f + 180.0f*((float)spin_type);
		scale_factor = amt/spin_time[spin_type];
		time = 0.0f;
		spin_axis = spin_dir*my_board_controller->GetUpDir();
		num_spins++;

#if defined(TARGET_XBOX)
    assert(is_valid());
#endif /* TARGET_XBOX JIV DEBUG */

	}
	else if (type == SPIN_MANUAL)
	{
		vector3d forward_vec = my_board_controller->GetForwardDir();  // get board forward direction
#if defined(TARGET_XBOX)
    assert(forward_vec.is_valid());
    assert(spin_axis.is_valid());
#endif /* TARGET_XBOX JIV DEBUG */

		if (dot(forward_vec, ZVEC) < 0.0f)  //  don't allow when surfer is facing shore
			return spin_axis;

		vector3d up_vec = my_board_controller->GetUpDir();  //  get board up direction
		vector3d rev_forward_vec(forward_vec.x, -forward_vec.y, -forward_vec.z);
		float temp_dot = dot(forward_vec, rev_forward_vec);
		if (temp_dot > 1.0f) temp_dot = 1.0f;
		if (temp_dot < -1.0f) temp_dot = -1.0f;
		float amt = fast_acos(temp_dot); //  calculate the angle between vertical and outward beach vector

#if defined(TARGET_XBOX)
    assert(up_vec.is_valid());
    assert(rev_forward_vec.is_valid());
    assert(!_isnan(amt));
    assert(_finite(amt));
#endif /* TARGET_XBOX JIV DEBUG */

		if (dot(up_vec, cross(forward_vec, rev_forward_vec)) > 0.0f)
			spin_dir = -1.0f;
		else
			spin_dir = 1.0f;

		spin_time[SPIN_MANUAL] = t;
		scale_factor = RAD_TO_DEG(amt/spin_time[SPIN_MANUAL]);
		spin_type = SPIN_MANUAL;
		spin_axis = spin_dir*my_board_controller->GetUpDir();
		num_spins = 0;
#if defined(TARGET_XBOX)
    if(spin_time[SPIN_MANUAL] == 0.0f)
    {
      // guess what?  x/0 -> something bad
      scale_factor = 0.0f;
    }

    assert(is_valid());
#endif /* TARGET_XBOX JIV DEBUG */
	}
	else if (type == SPIN_FLOATER)
	{
		vector3d forward_vec = my_board_controller->GetForwardDir();  // get board forward direction
		forward_vec -= dot(YVEC, forward_vec)*YVEC;
		forward_vec.normalize();
		float dot_amt = dot(forward_vec, final_vec);
		if (dot_amt > 1.0f)
			dot_amt = 1.0f;
		else if (dot_amt < -1.0f)
			dot_amt = -1.0f;

		float time = 0.375f*0.318309886;//  0.374/3.14
		float amt = fast_acos(dot_amt);
		time *= fabs(amt);

		spin_axis = my_board_controller->GetUpDir();
		vector3d crs = cross(forward_vec, final_vec);
		if (dot(spin_axis, crs) > 0.0f)
			spin_dir = -1.0f;
		else
			spin_dir = 1.0f;

		spin_axis = spin_dir*spin_axis;
		spin_time[SPIN_FLOATER] = time * time_mult;
		if(amt == 0.0f)
			scale_factor = 0.0f;
		else
			scale_factor = RAD_TO_DEG(amt/spin_time[SPIN_FLOATER]);
		spin_type = SPIN_FLOATER;
		num_spins = 0;
#if defined(TARGET_XBOX)
	    assert(is_valid());
#endif /* TARGET_XBOX JIV DEBUG */
	}
	if (num_spins == 0)
	{
		time = 0.0f;
		activated = true;
		my_board_controller->TrickSpin(0.0f);
	}
	depress_timer = 0.0f;
	return spin_axis;
}

void kellyslater_controller::debug_mode_play_anim(void)
{
	if (anim_num != anim_num_last)
	{
		anim_num_last = anim_num;
		Anim(anim_num,BLEND_TIME,true);
		if ((anim_num >= SURFER_ANIM_TRK_AERIAL_180) && (anim_num <= SURFER_ANIM_TRK_AERIAL_HOP_UP))
		{
			int num = anim_num - SURFER_ANIM_TRK_AERIAL_180;
			BoardAnim(num,BLEND_TIME,true);
		}
		else if ((anim_num >= SURFER_DUMMY_ANIM1) && (anim_num <= SURFER_DUMMY_ANIM5))
		{
			int num = BOARD_DUMMY_ANIM1;
			BoardAnim(num,BLEND_TIME,true);
		}
		else
			BoardAnim(0,BLEND_TIME,true);
	}
}

void register_kellyslater_inputs()
{
  input_mgr* inputmgr = input_mgr::inst();
  inputmgr->register_control( game_control(PAD_SPRAY, CT_BOOLEAN) );
  inputmgr->register_control( game_control(PAD_GRAB, CT_BOOLEAN) );
  inputmgr->register_control( game_control(PAD_SNAP, CT_BOOLEAN) );
  inputmgr->register_control( game_control(PAD_CARVE, CT_BOOLEAN) );
  inputmgr->register_control( game_control(PAD_LSTICK_H, CT_RATIONAL) );
  inputmgr->register_control( game_control(PAD_LSTICK_V, CT_RATIONAL) );
  inputmgr->register_control( game_control(PAD_SLIDE, CT_BOOLEAN) );
  inputmgr->register_control( game_control(PAD_GRIND, CT_BOOLEAN) );
  inputmgr->register_control( game_control(PAD_SPEED_PUMP, CT_BOOLEAN) );
  inputmgr->register_control( game_control(PAD_EASY_TRICK, CT_BOOLEAN) );
  inputmgr->register_control( game_control(PAD_OLLIE, CT_BOOLEAN) );
  inputmgr->register_control( game_control(PAD_HARD_LEFT, CT_BOOLEAN) );
  inputmgr->register_control( game_control(PAD_HARD_RIGHT, CT_BOOLEAN) );
  inputmgr->register_control( game_control(PAD_LOOK_BACK, CT_BOOLEAN) );
  inputmgr->register_control( game_control(PAD_SWITCH_CAMERA, CT_BOOLEAN) );
}


void map_kellyslater_inputs()
{
  input_mgr* inputmgr = input_mgr::inst();
  inputmgr->map_control( PAD_SPRAY,        JOYSTICK_DEVICE, JOY_PS2_BTNO );
  inputmgr->map_control( PAD_GRAB,        JOYSTICK_DEVICE, JOY_PS2_BTNO );
  inputmgr->map_control( PAD_SNAP,        JOYSTICK_DEVICE, JOY_PS2_BTNO );
  inputmgr->map_control( PAD_CARVE,        JOYSTICK_DEVICE, JOY_PS2_BTNSQ );
  inputmgr->map_control( PAD_LSTICK_H,        JOYSTICK_DEVICE, JOY_PS2_X );
  inputmgr->map_control( PAD_LSTICK_V,        JOYSTICK_DEVICE, JOY_PS2_Y );
  inputmgr->map_control( PAD_SLIDE,        JOYSTICK_DEVICE, JOY_PS2_BTNTR );
  inputmgr->map_control( PAD_GRIND,        JOYSTICK_DEVICE, JOY_PS2_BTNTR );
  inputmgr->map_control( PAD_SPEED_PUMP,        JOYSTICK_DEVICE, JOY_PS2_BTNL3 );
  inputmgr->map_control( PAD_EASY_TRICK,        JOYSTICK_DEVICE, JOY_PS2_BTNO );
  inputmgr->map_control( PAD_OLLIE,        JOYSTICK_DEVICE, JOY_PS2_BTNX );
  inputmgr->map_control( PAD_HARD_LEFT,        JOYSTICK_DEVICE, JOY_PS2_BTNL1 );
  inputmgr->map_control( PAD_HARD_RIGHT,        JOYSTICK_DEVICE, JOY_PS2_BTNR1 );
  inputmgr->map_control( PAD_LOOK_BACK,        JOYSTICK_DEVICE, JOY_PS2_BTNR2 );
#ifndef TARGET_GC //Ain't got these buttons beeatch!
  inputmgr->map_control( PAD_SWITCH_CAMERA,        JOYSTICK_DEVICE, JOY_PS2_BTNL2 );
#endif
}


float kellyslater_controller::CtrlEvent(int control)
{
  input_mgr* inputmgr = input_mgr::inst();
  return inputmgr->get_control_state(joystick_num, control);
}

bool kellyslater_controller::CtrlDelta(int control)
{
  input_mgr* inputmgr = input_mgr::inst();
  return (inputmgr->get_control_delta(joystick_num, control));
}


float kellyslater_controller::GetStick(int control)
{
	input_mgr* inputmgr = input_mgr::inst();
	float result = inputmgr->get_control_state(joystick_num, control);
	if (result > 1.0f)
		result = 1.0f;
	else if (result < -1.0f)
		result = -1.0f;

	return result;
}

float air_base = 0.5f;	//  This is the base number of the stat, the starting point.
float speed_base = 0.85f;	//  This is the base number of the stat, the starting point.
float rotate_base = 0.39f;	//  This is the base number of the stat, the starting point.

float special_meter_bonus = 0.1f;

float jump_cheat_inc = 2 * (1/30);	//  Weird little math here.  there are 30 stat increments, we want 2 of them in this case.
float balance_cheat_inc = 2 * (1/30);
float super_jump_cheat_inc = 4 * (1/30);
float super_balance_cheat_inc = 4 * (1/30);

float kellyslater_controller::GetAttribs(int attr) 
{ 
	assert(attr >= 0 && attr < NUM_ATTRIBS); 
	float temp_attrib = attribs[attr];

	//  check for bonuses from cheats.
	if (attr == ATTRIB_BALANCE && g_session_cheats[CHEAT_SUPER_JUMP].isOn())
		temp_attrib += super_balance_cheat_inc * (1.0f - air_base);
	else if (attr == ATTRIB_AIR && g_session_cheats[CHEAT_INCREASE_JUMP].isOn())
		temp_attrib += jump_cheat_inc * (1.0f - air_base);

	if (attr == ATTRIB_BALANCE && g_session_cheats[CHEAT_SUPER_BALANCE].isOn())
		temp_attrib += super_jump_cheat_inc * (1.0f - air_base);
	else if (attr == ATTRIB_BALANCE && g_session_cheats[CHEAT_INCREASE_BALANCE].isOn())
		temp_attrib += balance_cheat_inc * (1.0f - air_base);

	//  Check to see if the player gets a bonus because the special meter is active.
	if (specialMeter.CanRegionLink())	//  Give a little extra when the special meter is active.
	{
		if (attr == ATTRIB_AIR)
			temp_attrib += special_meter_bonus * (1.0f - air_base); 
		else if (attr == ATTRIB_SPEED)
			temp_attrib += special_meter_bonus * (1.0f - speed_base); 
		else if (attr == ATTRIB_ROTATE)
			temp_attrib += special_meter_bonus * (1.0f - rotate_base); 
		else //  Balance
			temp_attrib += special_meter_bonus;

	}

	return temp_attrib; 
}


//---------------------------------------------------------
//
//
//        Class kellyslater_controller
//
//
//---------------------------------------------------------

const float kellyslater_controller::CLOCKTIME_DUCK_DIVE = 5.0f;

void kellyslater_controller::SetNewTrick(const int trick)
{
	if (trick != currentTrick)
	{
		newTrick = trick;
		trick_queued = true;
		manual = GTrickList[trick].flags & ManualFlag;
	}
}

void kellyslater_controller::SetCompletedTrick()
{

	if (completedTrick != currentTrick)
	{
		// Score this trick.
		my_scoreManager.AddTrick(currentTrick);

		trick_complete = true;
		completedTrick = currentTrick;
	}
}

void kellyslater_controller::SetCompletedTrick(const int trickIdx)
{

	if (trickIdx != currentTrick)
	{

		// Score this trick.
		my_scoreManager.AddTrick(trickIdx);
		trick_complete = true;
		completedTrick = trickIdx;
	}
}

void kellyslater_controller::SetCurrentTrick(void)
{
	currentTrick = newTrick;
}

//  This function resets current tricks and completed tricks so that when a transition
//  is made to a FREEFALL state, the same trick will not be recounted as complete
void kellyslater_controller::ResetTricks(void)
{
	completedTrick = -1;
	currentTrick = -1;
}

void kellyslater_controller::ClearTricks(void)
{
	bSpecialTrick = false;
	trick_queued = false;
	newTrick = -1;
	currentTrick = -1;
	completedTrick = -1;
}

float g_shake_offset_max = 0.1f;
void kellyslater_controller::AlignSurferWithBoard(float dt)
{
  if (!IK_enabled && g_IK_enabled  )
    last_offset = 0;

	po owner_po = my_parent_node->get_abs_po();
	po trans( po_identity_matrix );
	rational_t pos = get_floor_offset();// - last_offset;

  last_offset += owner_po.get_position ().y - last_position;
  last_position = owner_po.get_position ().y;

  if (__fabs (last_offset) < 0.01f)
    last_offset = 0.0f;
  else if (last_offset > 0)
    last_offset -= 0.5f * dt;
  else
    last_offset += 0.5f * dt;

  if (last_offset > g_shake_offset_max)
    last_offset = g_shake_offset_max;
  else if (last_offset < -g_shake_offset_max)
    last_offset = -g_shake_offset_max;

  trans.set_position(vector3d(0, pos, 0));
	trans = trans*owner_po;
	get_owner()->set_rel_po(trans);
}

kellyslater_controller::kellyslater_controller(int hero_num, entity *ent)
	: entity_controller(ent)
{
  char	idxStr[2] = "";		// board index in ascii
  my_player_num = hero_num;

  my_board = NULL;
  my_board_model = NULL;
  my_board_member = NULL;
  my_rotate_object = NULL;
  my_parent_node = NULL;
  ent->update_abs_po_reverse();

  photo_cam_ptr = NULL;
  SetupCameras();

  bAnim = SURFER_ANIM_IDLE_LAYING;
  mAnim = BOARD_ANIM_IDLE_LAYING;
  IK_enabled = true;
  end_level = false;
  num_wipeouts = 0;
  no_buttons_pressed = false;

  last_position = 0;
  last_offset = 0;

  lastTime = 0.0f;

  turn_index = 0;

  switch_cam_pressed_already = false;

  reset_state();

  state_stack.reserve(STATE_STACK_MAX_DEPTH);

  // BETH  stringx hero_name = g_game_ptr->get_game_info().get_hero_name();
  stringx hero_name = stringx(SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].name);

  hero_name.to_lower();
  stringx anim_dir = "characters\\COMMON\\ANIMATIONS\\";
  stringx ext = entity_track_tree::extension( );

  if (!stricmp (hero_name.c_str (), "Dog"))
    anim_dir = "characters\\Dog\\ANIMATIONS\\";

  ignore_tweening = false;

  //  load shared animations
  for(int i=0; i<_SURFER_NUM_SHARED_ANIMS; ++i)
  {
	  surfer_anims[i] = anim_dir+g_surfer_anims[i];
	  if(file_finder_exists(surfer_anims[i], ext))
	  {
		  surfer_anims[i] += ext;
		  get_owner()->load_anim(surfer_anims[i]);
	  }
	  else
	  {
#if defined(TARGET_PC) && defined(BUILD_DEBUG)
		  //warning("Missing spiderman animation: %s%s", surfer_anims[i].c_str(), ext.c_str());
#endif
		  g_surfer_anims[i] = g_surfer_anims[SURFER_ANIM_PLACEHOLDER];
	  }
  }

  InitializeSurfer(hero_name);  //  this sets stuff for individual surfers

  stringx name;
  if (g_game_ptr->GetUsingPersonalitySuit(hero_num))
  {
    name = stringx("PERSON") + SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].abbr;
  }
  else
  {
    name = SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].name;
  }

  SetWorldLimits();
  SetMyRatings();
  CalculateStats();

  stringx mesh_path = "characters\\"+name+"\\entities\\";
  nglSetMeshPath( (char *)mesh_path.c_str() );
  sprintf(idxStr, "%d", g_game_ptr->GetBoardIdx(hero_num));
//	stringx texture_path = "characters\\" + hero_name + "\\" + os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\BOARD" +idxStr+ "\\";
// BETH: the BOARD# directories are not used anymore, since board textures use ifl's
  stringx texture_path = "BOARDS\\" + hero_name + "\\" + os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\";
  nglSetTexturePath((char *)texture_path.c_str());

  my_board = NEW entity();
  my_rotate_object = NEW entity();
  if (!g_game_ptr->GetUsingPersonalitySuit(hero_num))
  {
    if (g_game_ptr->GetBoardIdx(hero_num) < MAX_BOARDS && stricmp(SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].board_ent_name, "BOARD") != 0)
    {
      stringx path = stringx("CHARACTERS\\") + stringx(SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].name) 
        + stringx("\\ENTITIES\\");
      nglSetMeshPath(path.c_str());
      my_board_model = g_entity_maker->create_entity_or_subclass(stringx(path) + stringx(SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].board_ent_name), entity_id::make_unique_id(), po_identity_matrix, empty_string, entity::ACTIVE, NULL);
    }
    else
    {
      nglSetMeshPath("ITEMS\\BOARD\\ENTITIES\\");
      my_board_model = g_entity_maker->create_entity_or_subclass("ITEMS\\BOARD\\ENTITIES\\BOARD", entity_id::make_unique_id(), po_identity_matrix, empty_string, entity::ACTIVE, NULL);
    }
  }
  else
  {
    if (g_game_ptr->GetBoardIdx(hero_num) < MAX_BOARDS && stricmp(SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].p_board_name, "BOARD") != 0)
    {
      nglSetMeshPath((stringx("CHARACTERS\\PERSONALITY") + SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].abbr + "\\ENTITIES\\").c_str());
      my_board_model = g_entity_maker->create_entity_or_subclass(stringx("CHARACTERS\\PERSONALITY") + SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].abbr +stringx("\\ENTITIES\\")
        + SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].p_board_name, entity_id::make_unique_id(), po_identity_matrix, empty_string, entity::ACTIVE, NULL);
    }
    else
    {
      nglSetMeshPath("ITEMS\\BOARD\\ENTITIES\\");
      my_board_model = g_entity_maker->create_entity_or_subclass("ITEMS\\BOARD\\ENTITIES\\BOARD", entity_id::make_unique_id(), po_identity_matrix, empty_string, entity::ACTIVE, NULL);
    }
  }

  // Since we have 1 ifl, we have to index correctly
	// Idon't know why this is done here, it's also in BOARD_LoadMesh() but I need to disable this for personality suits - leo
  if (!g_game_ptr->GetUsingPersonalitySuit(hero_num))
	  SetConglomTexture(my_board_model, g_game_ptr->GetBoardIdx(hero_num) + (hero_num*9));

  my_board_controller.set_board(my_board);
  my_board_controller.set_board_model(my_board_model);
  my_board_controller.set_ks_controller(this);
  my_board_controller.Init();

  my_board_member = ((conglomerate*)my_board_model)->get_member("BOARD");

  my_trickManager.set_ksctrl(this);
  my_scoreManager.SetKsctrl(this);
  my_carveManager.set_ksctrl(this);

  my_carveManager.FrameAdvance(0.0f, true);

  spin_ctrl.set_my_ks_controller(this);
  spin_ctrl.SetMyBoard(&my_board_controller);
	rideEvent    = -1;
	floaterEvent = -1;
	spinEvent    = -1;
	stallEvent   = -1;
	carveEvent   = -1;

#if 0
  // replace the board textures
  nglTexture *new_tex = nglLoadTextureA ("ksbrdtop2");
  nglTexture *old_tex = nglGetTextureA ("ksbrdtop");
  nglMesh *mesh = ((conglomerate*)my_board_model)->get_member ("BOARD")->get_mesh ();

  for (u_int i = 0; i < mesh->NMaterials; i++)
  {
	  nglMaterial* mat = &mesh->Materials[i];

	  if (mat->Map == old_tex)
	  {
		  nglReleaseTexture (mat->Map);
		  mat->Map = new_tex;
	  }
  }
#endif

  if (my_board_model && my_board_model->has_link_ifc())
  {
	  my_board_model->link_ifc()->set_parent(my_rotate_object);
	  my_board_model->set_rel_po(po_identity_matrix);
  }

  if (my_rotate_object->has_link_ifc())
  {
	  my_rotate_object->link_ifc()->set_parent(my_board);
	  my_rotate_object->set_rel_po(po_identity_matrix);
	  po trans( po_identity_matrix );
	  trans.set_rotate_y(-0.5f*PI);
	  my_rotate_object->set_rel_po(trans);
  }

  my_parent_node = my_rotate_object;
//  Reset();	// If possible, delay until wave has been loaded (dc 01/24/02)

  //rational_t pos = get_floor_offset();
  //get_owner()->set_rel_position(pos*YVEC);

  //   load surfboard animations
  anim_dir = "characters\\COMMON\\ANIMATIONS\\";
  for(int i=0; i<_BOARD_NUM_ANIMS; ++i)
  {
	  board_anims[i] = anim_dir+g_board_anims[i];
	  if(file_finder_exists(board_anims[i], ext))
	  {
		  board_anims[i] += ext;
		  my_board_model->load_anim(board_anims[i]);
	  }
	  else
	  {
#if defined(TARGET_PC) && defined(BUILD_DEBUG)
		  //warning("Missing spiderman animation: %s%s", surfer_anims[i].c_str(), ext.c_str());
#endif
		  board_anims[i] = board_anims[BOARD_DUMMY_ANIM1];
	  }
  }

  if (!this->IsAIPlayer())
  {
	  nglMesh *owner_mesh = ((conglomerate*) get_owner())->get_mesh();
	  if (owner_mesh)
	  {
		  owner_mesh->Flags &= (~NGLMESH_REJECT_SPHERE);
		  owner_mesh->Flags |= NGLMESH_REJECT_TRICLIP;
	  }

	  nglMesh *lores_owner_mesh = ((conglomerate*) get_owner())->get_lores_mesh();
	  if (lores_owner_mesh)
	  {
		  lores_owner_mesh->Flags &= (~NGLMESH_REJECT_SPHERE);
		  lores_owner_mesh->Flags |= NGLMESH_REJECT_TRICLIP;
	  }
  }

  my_ik_object = NEW ik_object(my_board_member, (conglomerate *) owner);

#ifdef DEBUG
  gp_ksstate_string[my_player_num] = g_kscontroller_state[state];
  gp_ksanim_string[my_player_num] = g_surfer_anims[state].c_str();

  stringx none = "NONE";
  color32 yel = color32(255, 255, 0, 255);
  anim_label = new FloatingText(&frontendmanager.font_info, none, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, yel);
  state_label = new FloatingText(&frontendmanager.font_info, none, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, yel);
#endif

  goofy_int = 0;

  trick_complete = false;
  manual = false;
  current_trick_type = false;
  trick_queued = false;

  bDoingTrick = false;
  bSpecialTrick = false;

  complete_time = 0.2f;
  rideTimer = 0.0f;

  perform_snap = false;
  shakeoff = false;

  float_quadrant = -1;
  grind_jump_timer = 0.0f;
  tube_trick = -1;

  stand_num = 0;

  completedTrick = -1;
  currentTrick = -1;
  newTrick = -1;
  ClearTricks();
  ResetJumpedEntities();

  my_trail = ks_fx_trail_create (30, 1.0f, 1, this);
  wave_norm = YVEC;
  wave_norm_lf = YVEC;
  player_cam = NULL;

  tube_meter.SetPlayerNum(my_player_num);

  if (goofy && !goofy_int)
	{
		get_owner()->set_handed_axis(2);
		my_board_model->set_handed_axis(2);
		goofy_int = 1;
	}

	get_on_board_anim = SURFER_ANIM_W_1_R_GOB;
	b_get_on_board_anim = BOARD_ANIM_W_1_R_GOB;
}


kellyslater_controller::~kellyslater_controller()
{
  if (IsAIPlayer())
    g_game_ptr->set_num_ai_players(0);


  delete my_board;
  //delete my_board_model;
  delete my_rotate_object;
  delete my_ik_object;

#ifdef DEBUG
  delete anim_label;
  delete state_label;
#endif

  ks_fx_trail_destroy (my_trail);
}

//	set_player_num()
// Sets this controller's player index into the world's controller array.
void kellyslater_controller::set_player_num(const int n)
{
	assert(n >= 0 && n < MAX_PLAYERS);

	my_player_num = n;
	specialMeter.Initialize(n);
}

void kellyslater_controller::SetupCameras(void)
{
	stringx	name;
	int		i;

	name = "BEACH_CAM" + stringx(my_player_num);
	beach_cam_ptr = NEW beach_camera(entity_id::make_entity_id(name.c_str()), g_world_ptr->get_hero_ptr(my_player_num), this);
	beach_cam_ptr->set_flag(EFLAG_MISC_NONSTATIC, true);
	g_world_ptr->add_camera(beach_cam_ptr);

	name = "AUTO_CAM" + stringx(my_player_num);
	auto_cam_ptr = NEW auto_camera(entity_id::make_entity_id(name.c_str()), g_world_ptr->get_hero_ptr(my_player_num), this);
	auto_cam_ptr->set_flag(EFLAG_MISC_NONSTATIC, true);
	g_world_ptr->add_camera(auto_cam_ptr);

	name = "BIG_WAVE_CAM" + stringx(my_player_num);
	big_wave_cam_ptr = NEW big_wave_camera(entity_id::make_entity_id(name.c_str()), g_world_ptr->get_hero_ptr(my_player_num), this);
	big_wave_cam_ptr->set_flag(EFLAG_MISC_NONSTATIC, true);
	g_world_ptr->add_camera(big_wave_cam_ptr);

	name = "KSDEBUG_CAM" + stringx(my_player_num);
	ksdebug_cam_ptr = NEW debug_camera(entity_id::make_entity_id(name.c_str()), g_world_ptr->get_hero_ptr(my_player_num));
    ksdebug_cam_ptr->set_ks_controller(this);
	ksdebug_cam_ptr->set_flag(EFLAG_MISC_NONSTATIC, true);
	g_world_ptr->add_camera(ksdebug_cam_ptr);

	name = "LOOK_BACK_CAM" + stringx(my_player_num);
	look_back_cam_ptr = NEW look_back_camera(entity_id::make_entity_id(name.c_str()), g_world_ptr->get_hero_ptr(my_player_num));
    look_back_cam_ptr->set_ks_controller(this);
	look_back_cam_ptr->set_flag(EFLAG_MISC_NONSTATIC, true);
	g_world_ptr->add_camera(look_back_cam_ptr);

	name = "OLD_SHOULDER_CAM" + stringx(my_player_num);
	old_shoulder_cam_ptr = NEW old_shoulder_camera(entity_id::make_entity_id(name.c_str()), g_world_ptr->get_hero_ptr(my_player_num));
    old_shoulder_cam_ptr->set_ks_controller(this);
	old_shoulder_cam_ptr->set_flag(EFLAG_MISC_NONSTATIC, true);
	g_world_ptr->add_camera(old_shoulder_cam_ptr);

	name = "SHOULDER_CAM" + stringx(my_player_num);
	shoulder_cam_ptr = NEW shoulder_camera(entity_id::make_entity_id(name.c_str()), g_world_ptr->get_hero_ptr(my_player_num));
    shoulder_cam_ptr->set_ks_controller(this);
	shoulder_cam_ptr->set_flag(EFLAG_MISC_NONSTATIC, true);
	g_world_ptr->add_camera(shoulder_cam_ptr);

	name = "FLYBY_CAM" + stringx(my_player_num);
	flyby_cam_ptr = NEW flyby_camera(entity_id::make_entity_id(name.c_str()), g_world_ptr->get_hero_ptr(my_player_num));
    flyby_cam_ptr->set_ks_controller(this);
	flyby_cam_ptr->set_flag(EFLAG_MISC_NONSTATIC, true);
	g_world_ptr->add_camera(flyby_cam_ptr);

	name = "WIPEOUT_CAM" + stringx(my_player_num);
	wipeout_cam_ptr = NEW wipeout_camera(entity_id::make_entity_id(name.c_str()), g_world_ptr->get_hero_ptr(my_player_num));
    wipeout_cam_ptr->set_ks_controller(this);
	wipeout_cam_ptr->set_flag(EFLAG_MISC_NONSTATIC, true);
	g_world_ptr->add_camera(wipeout_cam_ptr);

	name = "WIPEOUT_CAM_2" + stringx(my_player_num);
	wipeout_cam2_ptr = NEW wipeout_camera_2(entity_id::make_entity_id(name.c_str()), g_world_ptr->get_hero_ptr(my_player_num));
    wipeout_cam2_ptr->set_ks_controller(this);
	wipeout_cam2_ptr->set_flag(EFLAG_MISC_NONSTATIC, true);
	g_world_ptr->add_camera(wipeout_cam2_ptr);

	name = "FOLLOW_CAM" + stringx(my_player_num);
	follow_cam_ptr = NEW follow_camera(entity_id::make_entity_id(name.c_str()), g_world_ptr->get_hero_ptr(my_player_num));
    follow_cam_ptr->set_ks_controller(this);
	follow_cam_ptr->set_flag(EFLAG_MISC_NONSTATIC, true);
	g_world_ptr->add_camera(follow_cam_ptr);

	name = "FOLLOW_CLOSE_CAM" + stringx(my_player_num);
	follow_close_cam_ptr = NEW follow_close_camera(entity_id::make_entity_id(name.c_str()), g_world_ptr->get_hero_ptr(my_player_num));
    follow_close_cam_ptr->set_ks_controller(this);
	follow_close_cam_ptr->set_flag(EFLAG_MISC_NONSTATIC, true);
	g_world_ptr->add_camera(follow_close_cam_ptr);

	name = "BUOY_CAM" + stringx(my_player_num);
	buoy_cam_ptr = NEW buoy_camera(entity_id::make_entity_id(name.c_str()), g_world_ptr->get_hero_ptr(my_player_num));
    buoy_cam_ptr->set_ks_controller(this);
	buoy_cam_ptr->set_flag(EFLAG_MISC_NONSTATIC, true);
	g_world_ptr->add_camera(buoy_cam_ptr);

	name = "DUCKDIVE_CAM" + stringx(my_player_num);
	duckdive_cam_ptr = NEW duckdive_camera(entity_id::make_entity_id(name.c_str()), g_world_ptr->get_hero_ptr(my_player_num), this);
	duckdive_cam_ptr->set_flag(EFLAG_MISC_NONSTATIC, true);
	g_world_ptr->add_camera(duckdive_cam_ptr);

	name = "FPS_CAM" + stringx(my_player_num);
	fps_cam_ptr = NEW fps_camera(entity_id::make_entity_id(name.c_str()), g_world_ptr->get_hero_ptr(my_player_num), this);
	fps_cam_ptr->set_flag(EFLAG_MISC_NONSTATIC, true);
	g_world_ptr->add_camera(fps_cam_ptr);

	// Create a photo cam, but only if we need it.
	if (g_game_ptr->get_game_mode() == GAME_MODE_CAREER)
	{
		for (i = 0; i < MAX_GOALS_PER_LEVEL; i++)
		{
			if (CareerDataArray[g_game_ptr->get_level_id()].goal[i] == GOAL_PHOTO_1 ||
				CareerDataArray[g_game_ptr->get_level_id()].goal[i] == GOAL_PHOTO_2 ||
				CareerDataArray[g_game_ptr->get_level_id()].goal[i] == GOAL_PHOTO_3)
			{
				name = "PHOTO_CAM" + stringx(my_player_num);
				photo_cam_ptr = NEW photo_camera(entity_id::make_entity_id(name.c_str()), g_world_ptr->get_hero_ptr(my_player_num), this);
				photo_cam_ptr->set_flag(EFLAG_MISC_NONSTATIC, true);
				g_world_ptr->add_camera(photo_cam_ptr);
				break;
			}
		}
	}
}

//  Sets all attributes to a 0 to 1 scale.
void kellyslater_controller::SetSurferAttribs(void)
{
	int surfer_air, surfer_speed, surfer_spin, surfer_balance;
	int board_air, board_speed, board_spin, board_balance;

	if (g_surfer_attribs.use_debug_values)  //  Don't use the regular stats, use debug numbers
	{
		surfer_air = g_surfer_attribs.player_air;
		surfer_speed = g_surfer_attribs.player_speed;
		surfer_spin = g_surfer_attribs.player_rotate;
		surfer_balance = g_surfer_attribs.player_balance;

		board_air = g_surfer_attribs.board_air;
		board_speed = g_surfer_attribs.board_speed;
		board_spin = g_surfer_attribs.board_rotate;
		board_balance = g_surfer_attribs.board_balance;
	}
	else if (g_game_ptr->get_game_mode() == GAME_MODE_CAREER && g_session_cheats[CHEAT_UNLOCK_ALL_SKILL_POINTS].isOn())	//  they're in career mode and using the max stats cheat
	{
		surfer_air = SurferDataArray[g_game_ptr->GetSurferIdx(my_player_num)].attr_air + 6;
		surfer_speed = SurferDataArray[g_game_ptr->GetSurferIdx(my_player_num)].attr_speed + 6;
		surfer_spin = SurferDataArray[g_game_ptr->GetSurferIdx(my_player_num)].attr_spin + 6;
		surfer_balance = SurferDataArray[g_game_ptr->GetSurferIdx(my_player_num)].attr_balance + 6;
	
		int board_num = g_game_ptr->GetBoardIdx(my_player_num);
		if (board_num == -1)  //  beach board
		{
			int current_beach = g_game_ptr->get_beach_id ();
			board_air = LocationBoardArray[BeachDataArray[current_beach].map_location].air;
			board_speed = LocationBoardArray[BeachDataArray[current_beach].map_location].speed;
			board_spin = LocationBoardArray[BeachDataArray[current_beach].map_location].spin;
			board_balance = LocationBoardArray[BeachDataArray[current_beach].map_location].balance;
		}
		else
		{
			board_air = SurferBoardArray[g_game_ptr->GetSurferIdx(my_player_num)][board_num].air;
			board_speed = SurferBoardArray[g_game_ptr->GetSurferIdx(my_player_num)][board_num].speed;
			board_spin = SurferBoardArray[g_game_ptr->GetSurferIdx(my_player_num)][board_num].spin;
			board_balance = SurferBoardArray[g_game_ptr->GetSurferIdx(my_player_num)][board_num].balance;
		}
	}
	else if (g_game_ptr->get_game_mode() == GAME_MODE_CAREER)	//  they're in career mode, so use the stats the player has built up.
	{
		surfer_air = g_career->GetJump();
		surfer_speed = g_career->GetSpeed();
		surfer_spin = g_career->GetSpin();
		surfer_balance = g_career->GetBalance();
	
		int board_num = g_game_ptr->GetBoardIdx(my_player_num);
		if (board_num == -1)  //  beach board
		{
			int current_beach = g_game_ptr->get_beach_id ();
			board_air = LocationBoardArray[BeachDataArray[current_beach].map_location].air;
			board_speed = LocationBoardArray[BeachDataArray[current_beach].map_location].speed;
			board_spin = LocationBoardArray[BeachDataArray[current_beach].map_location].spin;
			board_balance = LocationBoardArray[BeachDataArray[current_beach].map_location].balance;
		}
		else
		{
			board_air = SurferBoardArray[g_game_ptr->GetSurferIdx(my_player_num)][board_num].air;
			board_speed = SurferBoardArray[g_game_ptr->GetSurferIdx(my_player_num)][board_num].speed;
			board_spin = SurferBoardArray[g_game_ptr->GetSurferIdx(my_player_num)][board_num].spin;
			board_balance = SurferBoardArray[g_game_ptr->GetSurferIdx(my_player_num)][board_num].balance;
		}
	}
	else	//  Everywhere else, just use the base stats, puls the current handicap
	{
		int current_handicap = g_game_ptr->get_player_handicap(my_player_num);

		surfer_air = SurferDataArray[g_game_ptr->GetSurferIdx(my_player_num)].attr_air + current_handicap;
		surfer_speed = SurferDataArray[g_game_ptr->GetSurferIdx(my_player_num)].attr_speed + current_handicap;
		surfer_spin = SurferDataArray[g_game_ptr->GetSurferIdx(my_player_num)].attr_spin + current_handicap;
		surfer_balance = SurferDataArray[g_game_ptr->GetSurferIdx(my_player_num)].attr_balance + current_handicap;
	
		int board_num = g_game_ptr->GetBoardIdx(my_player_num);
		if (board_num == -1)  //  beach board
		{
			int current_beach = g_game_ptr->get_beach_id ();
			board_air = LocationBoardArray[BeachDataArray[current_beach].map_location].air;
			board_speed = LocationBoardArray[BeachDataArray[current_beach].map_location].speed;
			board_spin = LocationBoardArray[BeachDataArray[current_beach].map_location].spin;
			board_balance = LocationBoardArray[BeachDataArray[current_beach].map_location].balance;
		}
		else
		{
			board_air = SurferBoardArray[g_game_ptr->GetSurferIdx(my_player_num)][board_num].air;
			board_speed = SurferBoardArray[g_game_ptr->GetSurferIdx(my_player_num)][board_num].speed;
			board_spin = SurferBoardArray[g_game_ptr->GetSurferIdx(my_player_num)][board_num].spin;
			board_balance = SurferBoardArray[g_game_ptr->GetSurferIdx(my_player_num)][board_num].balance;
		}
	}

	float inv_max_attrib = 1.0f/(MAX_BOARD_ATTRIB + MAX_PLAYER_ATTRIB);

	attribs[ATTRIB_AIR] = ((surfer_air + board_air) * inv_max_attrib * (1.0 - air_base)) + 
							air_base;
	attribs[ATTRIB_SPEED] = ((surfer_speed + board_speed) * inv_max_attrib * (1.0 - speed_base)) + 
							speed_base;
	attribs[ATTRIB_ROTATE] = ((surfer_spin + board_spin) * inv_max_attrib * (1.0 - rotate_base)) + 
							rotate_base;
	attribs[ATTRIB_BALANCE] = (surfer_balance + board_balance) * inv_max_attrib;
}


void kellyslater_controller::InitializeSurfer(stringx hero_name)
{
	int start_stand = 0, start_turn = 0, start_carve = 0, start_surfer_anims = 0, end_surfer_anims = 0;

	surfer_num = -1;
	for (int i = 0; i < SURFER_LAST; ++i)
	{
		if (!stricmp(hero_name.c_str(), SurferDataArray[i].name)
			|| !stricmp(hero_name.c_str(), SurferDataArray[i].name_ps))
		{
			goofy = SurferDataArray[i].goofy;
			start_stand = SurferDataArray[i].start_stand;
			start_turn = SurferDataArray[i].start_turn;
			start_carve = SurferDataArray[i].start_carve;
			start_surfer_anims = SurferDataArray[i].start_anims;
			end_surfer_anims = SurferDataArray[i].end_anims;
			surfer_num = i;
		}
	}
	#ifndef TARGET_GC
	assertmsg (surfer_num != -1, "Who's this guy you're trying to load ?\n");
	#endif

	int n;
	for (n = 0; n < NUM_STAND_ANIMS; n++)
		stand_anim_nums[n] = start_stand + n;

	for (n = 0; n < NUM_TURN_ANIMS; n++)
		turn_anim_nums[n] = start_turn + n;

	for (n = 0; n < NUM_CARVE_ANIMS; n++)
		carve_anim_nums[n] = start_carve + n;

	stringx anim_dir = "characters\\COMMON\\ANIMATIONS\\";
	stringx ext = entity_track_tree::extension( );

	//   load unique kelly slater anims
	anim_dir = "characters\\"+hero_name+"\\ANIMATIONS\\";
	for(int i=start_surfer_anims; i<end_surfer_anims; ++i)
	{
		surfer_anims[i] = anim_dir+g_surfer_anims[i];
		if(file_finder_exists(surfer_anims[i], ext))
		{
			surfer_anims[i] += ext;
			get_owner()->load_anim(surfer_anims[i]);
		}
		else
		{
			g_surfer_anims[i] = g_surfer_anims[SURFER_ANIM_PLACEHOLDER];
		}
	}

	/*for (n = _SURFER_NUM_START_SLATER_ANIMS; n < _SURFER_NUM_ANIMS; n++)
	{
		if ((n < start_surfer_anims) || (n >= end_surfer_anims))
			g_surfer_anims[n] = g_surfer_anims[SURFER_ANIM_PLACEHOLDER];
	}*/

	//   set surfer attributes
	SetSurferAttribs();

	did_celebration = false;
	done_scoring = false;
}


void kellyslater_controller::CalcTransform(vector3d &rot, vector3d &pos)
{
/*
	// compute a transform from Euler rotation, translation
	mat44	unit;
	mat44	tmp;
	mat44	maxFix;

	ircVu0RotMatrixZ ( &tmp, &unit, rot.z );
	ircVu0RotMatrixX ( &tmp, &tmp, rot.x );
	ircVu0RotMatrixY ( &tmp, &tmp, rot.y );

	ircVu0RotMatrixZ ( &maxFix, &unit, 0 );
	ircVu0RotMatrixX ( &maxFix, &maxFix,  M_PI_div2 );
	ircVu0RotMatrixY ( &maxFix, &maxFix, 0 );

	ircVu0MulMatrix (&tmp, &tmp, &maxFix);

	tmp.Scale(1.0f/12.0f);

	ircVu0TransMatrix(&tm, &tmp, &pos);
  */

  po r1,r2,r3,result;

  r1.set_rotate_x(rot.x);
  r2.set_rotate_y(-rot.y);
  r3.set_rotate_z(rot.z);

  result = r2*r1*r3;

  vector3d pos3;
  pos3.x = pos.x;
  pos3.y = pos.y;
  pos3.z = pos.z;
  result.set_position(pos3/12);

  get_owner()->set_rel_po(result);
}

void kellyslater_controller::	reset_state ()
{
	//  Check to see if this has already been reset.
	if (state == STATE_SWIMTOLIE || state == STATE_LIEONBOARD)
		return;

	last_super_state = SUPER_STATE_NO_SUPERSTATE;
	super_state = SUPER_STATE_LIE_ON_BOARD;

	if (my_board_controller.wiped_out)
		state = STATE_SWIMTOLIE;
	else
		state = STATE_LIEONBOARD;

	last_state = STATE_NOSTATE;
}


void kellyslater_controller::Reset(void)
{
	pos.x			= 0.8600f;
	pos.y			= 4.0f;
	pos.z			= 0.350f; //5.0f;

//	script.Reset();
	rot = vector3d (0,0,0);			// current orientation angles
	rotVel = vector3d (0,0,0);		// current orientation angle velocities
	rotAccel = vector3d (0,0,0);	// current orientation angle accelerations
	rotDest = vector3d (0,0,0);		// destination orientation angle
	rotDestVel = vector3d (0,0,0);	// destination orientation angle velocities
    velocity = ZEROVEC;
	CalcTransform(rot, pos);

	wave_norm = YVEC;
	wave_norm_lf = YVEC;

	dry = true;
	left_hand_dry = true;
	right_hand_dry = true;

	if (get_on_board_anim == 0)
		get_on_board_anim = SURFER_ANIM_W_1_R_GOB;

	if (b_get_on_board_anim == 0)
		b_get_on_board_anim = BOARD_ANIM_W_1_R_GOB;

	wip_phys_state = WIP_PHYS_DISMOUNT;

	speed = 0.001f;
	rideTimer = 0.0f;
	lastAnim = 0;
	bAnim = BOARD_ANIM_W_1_R_GOB;
	mAnim = SURFER_ANIM_W_1_R_GOB;

	specialMeter.Reset();
	//float_meter.End();
	did_celebration = false;
	done_scoring = false;
	shakeoff = false;
	ClearTricks();

	//  make sure surfer is on board and on the wave
	frame_advance(0.0f);
	if (get_owner())
	{
		entity_anim_tree *surfer_tr = get_owner()->get_anim_tree(ANIM_PRIMARY);
		if (surfer_tr)
			surfer_tr->frame_advance(0.0f);
	}

	if (my_board_model)
	{
		entity_anim_tree *board_tr = my_board_model->get_anim_tree(ANIM_PRIMARY);
		if (board_tr)
			board_tr->frame_advance(0.0f);
	}

	AlignSurferWithBoard(0);
	if (beach_cam_ptr) beach_cam_ptr->Reset();
	if (auto_cam_ptr) auto_cam_ptr->Reset();
	if (big_wave_cam_ptr) big_wave_cam_ptr->Reset();
}

void kellyslater_controller::check_celebration(void)
{
	bool	endMe = false;
	
	// Already celebrating, so do nothing.
	if (super_state == SUPER_STATE_CPU_CONTROLLED)
	{
		return;
	}


	// Only begin celebrations from a normal state.
	if ((g_game_ptr->get_game_mode() == GAME_MODE_PUSH) || (g_game_ptr->get_game_mode() == GAME_MODE_HEAD_TO_HEAD))
	{
		if (!TIMER_IsInfiniteDuration() && TIMER_GetRemainingLevelSec() <= 0.0f)
		{
			/*
			if (!my_board_controller.InAir() || (get_super_state() == SUPER_STATE_WIPEOUT))
			{
				if (get_super_state() == SUPER_STATE_AIR)
				{
					state = STATE_DONTCELEBRATE;
					super_state = SUPER_STATE_CPU_CONTROLLED;
					endMe = true;
				}
			}
			else if (get_super_state() == SUPER_STATE_LIE_ON_BOARD)
				endMe = true;
			else if (!done_scoring)
			{
				if (!my_board_controller.InAir())
				{
					state = STATE_DONTCELEBRATE;
					super_state = SUPER_STATE_CPU_CONTROLLED;
					endMe = true;
				}
			}
			*/

			if (get_super_state() == SUPER_STATE_NORMAL_SURF)
			{
				state = STATE_DONTCELEBRATE;
				super_state = SUPER_STATE_CPU_CONTROLLED;
				endMe = true;
			}
			else if (get_super_state() == SUPER_STATE_WIPEOUT)
			{
				endMe = true;
			}
			else if (get_super_state() == SUPER_STATE_LIE_ON_BOARD)
			{
				endMe = true;
			}
			else if (get_super_state() == SUPER_STATE_PREPARE_JUMP)
			{
				state = STATE_DONTCELEBRATE;
				super_state = SUPER_STATE_CPU_CONTROLLED;
				endMe = true;
			}
			else if (get_super_state() == SUPER_STATE_AIR)
			{
				if (state == STATE_LANDING)
				{
					state = STATE_DONTCELEBRATE;
					super_state = SUPER_STATE_CPU_CONTROLLED;
					endMe = true;
				}
			}
			else if (get_super_state() == SUPER_STATE_IN_TUBE)
			{
				int temp_state = state;
				EndTube();
				if (temp_state == STATE_TUBE_RAILGRAB)
					state = STATE_CELEBRATERAILGRAB;
				else
					state = STATE_DONTCELEBRATE;

				super_state = SUPER_STATE_CPU_CONTROLLED;
				endMe = true;
			}
		}

		if (endMe)
		{
			my_scoreManager.FinishChain(true);
			done_scoring = true;
		}
	}
	else
	{
		if (get_board_controller().InAir() ||
			get_special_meter()->CanRegionLink() ||
			IsDoingSpecialTrick() ||
			get_super_state() == SUPER_STATE_WIPEOUT
			|| get_super_state() == SUPER_STATE_LIE_ON_BOARD)
		{
			return;
		}

		if (!TIMER_IsInfiniteDuration() && TIMER_GetRemainingLevelSec() <= 0.0f)
		{
			if (g_game_ptr->get_game_mode() == GAME_MODE_CAREER)
			{
				if ((get_super_state() == SUPER_STATE_IN_TUBE) && (state == STATE_TUBE_RAILGRAB))
				{
					EndTube();
					state = STATE_CELEBRATERAILGRAB;
					super_state = SUPER_STATE_CPU_CONTROLLED;
				}
				else if (g_career->WasNewGoalPassed() || os_developer_options::inst()->is_flagged(os_developer_options::FLAG_ALWAYS_CELEBRATE))
				{
					if (get_super_state() == SUPER_STATE_IN_TUBE)
					{
						int temp_state = state;
						EndTube();
						if (temp_state == STATE_TUBE_RAILGRAB)
							state = STATE_CELEBRATERAILGRAB;
						else
							StartCelebration();

						super_state = SUPER_STATE_CPU_CONTROLLED;
						endMe = true;
					}
					else
						StartCelebration();
				}
				else if (g_career->WasAnyGoalPassed())
				{
					state = STATE_DONTCELEBRATE;
					super_state = SUPER_STATE_CPU_CONTROLLED;
				}
				else
				{
					StartDisappointment();
				}
			}
			else if ((get_super_state() == SUPER_STATE_IN_TUBE) && (state == STATE_TUBE_RAILGRAB))
			{
				EndTube();
				state = STATE_CELEBRATERAILGRAB;
				super_state = SUPER_STATE_CPU_CONTROLLED;
			}
			else
			{
				state = STATE_DONTCELEBRATE;
				super_state = SUPER_STATE_CPU_CONTROLLED;
			}

			my_scoreManager.FinishChain(true);
			done_scoring = true;
		}
	}
}

int g_max_wipeouts = 3;
float g_test_float = 0.003f;
void kellyslater_controller::frame_advance(time_value_t t)
{
	get_owner()->physical_ifc()->disable();

	if (debug_mode)
	{
		debug_mode_play_anim();
#ifdef DEBUG
		if ((state >= STATE_NOSTATE) && (state < STATE_NUMSTATES))
		{
			gp_ksstate_string[my_player_num] = g_kscontroller_state[state];
		}

		if ((mAnim >= 0) && (mAnim < _SURFER_NUM_ANIMS))
		{
			gp_ksanim_string[my_player_num] = g_surfer_anims[mAnim].c_str();
		}
#endif
		return;
	}

#if !defined(EVAN)
	if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_E3_BUILD) && !IsAIPlayer())
	{
		if (num_wipeouts > (g_max_wipeouts - 1))
		{
			end_level = true;
		}
	}
#endif

	if ((super_state != SUPER_STATE_WIPEOUT) && (super_state != SUPER_STATE_CPU_CONTROLLED)
		&& (super_state != SUPER_STATE_LIE_ON_BOARD) && (super_state != SUPER_STATE_FLYBY))
	{
		my_trickManager.FrameAdvance(t);
	}

	process_controls(t);
	spin_ctrl.frame_advance(t);
	
	if (super_state != SUPER_STATE_FLYBY)
		my_board_controller.rb->Update(t);

	update(t);

	if (g_dump_animation_info)
	{
		get_owner()->get_anim_tree(ANIM_PRIMARY)->debug_print_PRS_to_file();
		g_dump_animation_info = 0;
	}

	// Update special meter.
	specialMeter.Update(t, GetTrickRegion(), GetCurrentTrick(), my_scoreManager.GetChain().GetTrickCount(GetCurrentTrick()));
	
	// Update scoring manager.
	my_scoreManager.Update(t);
}


void kellyslater_controller::SetJumpedEntity (entity *ent)
{
	bool exists = false;

	if (!ent || (num_jumped__entities > 0 && ent == jumped_entities[num_jumped__entities-1]))
		return;

	g_beach_ptr->find_object (ent)->jumped_over (owner);

	if (num_jumped__entities < MAX_JUMP_ENT)
	{
		// Toby change: only add this entity if it has not been added yet.
		for (int i = 0; i < num_jumped__entities; i++)
		{
			if (jumped_entities[i] == ent)
				exists = true;
		}
		
		if (!exists)
		{
			jumped_entities[num_jumped__entities++] = ent;
			if (ent->get_parsed_name().find ("sebastian_pier") != stringx::npos )
			{
				SoundScriptManager::inst()->playEvent(SS_PIER_CHEER, ent);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
//    constructor Section
//
////////////////////////////////////////////////////////////////////////////////////////



void kellyslater_controller::SetTurnStat(int table,int heading, float turnVel, float bankAccel, float bankVel, float bank)
{
	switch (table)
	{
		case TURN_RATINGS:
			turnRatings[heading][TURN_SUBHEADING_BANK_ANGLE]	= bank;
			turnRatings[heading][TURN_SUBHEADING_BANK_VEL]		= bankVel;
			turnRatings[heading][TURN_SUBHEADING_BANK_ACCEL]	= bankAccel;
			turnRatings[heading][TURN_SUBHEADING_TURN_VEL]		= turnVel;
			break;
		case WORST_TURNS:
			worstTurns[heading][TURN_SUBHEADING_BANK_ANGLE]	= bank;
			worstTurns[heading][TURN_SUBHEADING_BANK_VEL]	= bankVel;
			worstTurns[heading][TURN_SUBHEADING_BANK_ACCEL]	= bankAccel;
			worstTurns[heading][TURN_SUBHEADING_TURN_VEL]	= turnVel;
			break;
		case BEST_TURNS:
			bestTurns[heading][TURN_SUBHEADING_BANK_ANGLE]	= bank;
			bestTurns[heading][TURN_SUBHEADING_BANK_VEL]	= bankVel;
			bestTurns[heading][TURN_SUBHEADING_BANK_ACCEL]	= bankAccel;
			bestTurns[heading][TURN_SUBHEADING_TURN_VEL]	= turnVel;
			break;
	}
}


void kellyslater_controller::CalcTurnStats(turn_data * td, int heading)
{
	float worst, best, rating;

	worst	= worstTurns[heading][TURN_SUBHEADING_BANK_ANGLE];
	best	= bestTurns[heading][TURN_SUBHEADING_BANK_ANGLE];
	rating	= turnRatings[heading][TURN_SUBHEADING_BANK_ANGLE];
	td->Bank = worst + (rating * (best - worst));

	worst	= worstTurns[heading][TURN_SUBHEADING_BANK_VEL];
	best	= bestTurns[heading][TURN_SUBHEADING_BANK_VEL];
	rating	= turnRatings[heading][TURN_SUBHEADING_BANK_VEL];
	td->BankVel = worst + (rating * (best - worst));

	worst	= worstTurns[heading][TURN_SUBHEADING_BANK_ACCEL];
	best	= bestTurns[heading][TURN_SUBHEADING_BANK_ACCEL];
	rating	= turnRatings[heading][TURN_SUBHEADING_BANK_ACCEL];
	td->BankAccel = worst + (rating * (best - worst));

	worst	= worstTurns[heading][TURN_SUBHEADING_TURN_VEL];
	best	= bestTurns[heading][TURN_SUBHEADING_TURN_VEL];
	rating	= turnRatings[heading][TURN_SUBHEADING_TURN_VEL];
	td->TurnVel = worst + (rating * (best - worst));
}


void kellyslater_controller::SetWorldLimits()
{
	SetTurnStat(WORST_TURNS, TURN_HEADING_CARVE, 2.5, 0.10, 0.50, 10.00);
	SetTurnStat(BEST_TURNS,  TURN_HEADING_CARVE, 8.0, 1.00, 2.00, 20.00);

	SetTurnStat(WORST_TURNS, TURN_HEADING_HARD_CARVE,  5.00, 0.20, 1.00, 20.00);
	SetTurnStat(BEST_TURNS,  TURN_HEADING_HARD_CARVE, 16.00, 2.00, 4.00, 40.00);

	SetTurnStat(WORST_TURNS, TURN_HEADING_GRAB,  5.00, 0.20, 1.00, 10.00);
	SetTurnStat(BEST_TURNS,  TURN_HEADING_GRAB, 10.00, 2.00, 4.00, 20.00);

	SetTurnStat(WORST_TURNS, TURN_HEADING_HARD_GRAB, 10.00, 0.40, 2.00, 20.00);
	SetTurnStat(BEST_TURNS,  TURN_HEADING_HARD_GRAB, 20.00, 4.00, 8.00, 40.00);

	SetTurnStat(WORST_TURNS, TURN_HEADING_SLIDE,  5.00, 0.02, 0.25, 2.50);
	SetTurnStat(BEST_TURNS,  TURN_HEADING_SLIDE, 10.00, 0.05, 0.50, 5.00);

	SetTurnStat(WORST_TURNS, TURN_HEADING_HARD_SLIDE, 10.00, 0.05, 0.50,  5.00);
	SetTurnStat(BEST_TURNS,  TURN_HEADING_HARD_SLIDE, 20.00, 0.10, 1.00, 10.00);
}


void kellyslater_controller::SetMyRatings()
{
	SetTurnStat(TURN_RATINGS, TURN_HEADING_CARVE,      1.0, 1.0, 1.0, 1.0);
	SetTurnStat(TURN_RATINGS, TURN_HEADING_HARD_CARVE, 1.0, 1.0, 1.0, 1.0);
	SetTurnStat(TURN_RATINGS, TURN_HEADING_GRAB,       1.0, 1.0, 1.0, 1.0);
	SetTurnStat(TURN_RATINGS, TURN_HEADING_HARD_GRAB,  1.0, 1.0, 1.0, 1.0);
	SetTurnStat(TURN_RATINGS, TURN_HEADING_SLIDE,      1.0, 1.0, 1.0, 1.0);
	SetTurnStat(TURN_RATINGS, TURN_HEADING_HARD_SLIDE, 1.0, 1.0, 1.0, 1.0);
}

void kellyslater_controller::CalculateStats()
{
	CalcTurnStats(&carve, TURN_HEADING_CARVE);
	CalcTurnStats(&hardCarve, TURN_HEADING_HARD_CARVE);
	CalcTurnStats(&grab, TURN_HEADING_GRAB);
	CalcTurnStats(&hardGrab, TURN_HEADING_HARD_GRAB);
	CalcTurnStats(&slide, TURN_HEADING_SLIDE);
	CalcTurnStats(&hardSlide, TURN_HEADING_HARD_SLIDE);
}


////////////////////////////////////////////////////////////////////////////////////////
//
//    process_controls Section
//
////////////////////////////////////////////////////////////////////////////////////////

#if defined(TARGET_XBOX)
// default arguments cannot be redefined within same scope
void kellyslater_controller::SetAnimAndFrame( int a, int b, float time, float blenda, float blendb, bool KSLoop, bool BLoop, bool KSAnimCall, bool BAnimCall)
#else
void kellyslater_controller::SetAnimAndFrame( int a, int b, float time, float blenda, float blendb, bool KSLoop, bool BLoop, bool KSAnimCall, bool BAnimCall )
#endif /* TARGET_XBOX JIV DEBUG */
{
#ifdef BRONER
  stringx tmp;
  if ((mAnim!=a) || KSAnimCall)
  {
    tmp = stringx("Anim:  ") + stringx(a) + "\t"
                                    + stringx(time) + "\t"
                                    + stringx(blenda) + "\t"
                                    + stringx(KSLoop) + "\t"
                                    + stringx(KSAnimCall) + "\n";
    nglPrintf(tmp.c_str());
  }
  if ((bAnim != b) || BAnimCall)
  {
    tmp = stringx("BAnim: ") + stringx(b) + "\t"
                                    + stringx(time) + "\t"
                                    + stringx(blendb) + "\t"
                                    + stringx(BLoop) + "\t"
                                    + stringx(BAnimCall) + "\n";
    nglPrintf(tmp.c_str());
  }
#endif

  if ((bAnim != b) || BAnimCall)
  {
    BoardAnim(b, blendb, BLoop, 0);
  }
  else
  {
    if (time > lastTime)
      my_board_model->get_anim_tree(ANIM_PRIMARY)->frame_advance(time-lastTime);
    else
      my_board_model->get_anim_tree(ANIM_PRIMARY)->frame_advance(time);
  }

	if (( mAnim!=a) || KSAnimCall )
	{
		Anim(a,blenda,KSLoop,0);
	}
	else
	{
		//Anim(a,0,true,time);
		//Anim(a,0,false,time);
    if (time > lastTime)
      get_owner()->get_anim_tree(ANIM_PRIMARY)->frame_advance(time-lastTime);
    else
      get_owner()->get_anim_tree(ANIM_PRIMARY)->frame_advance(time);

		// this doesn't work - the frame doesn't update
		//mFrame=f;
    //rational_t start_time = (mFrame / 30.0f);
    //get_owner()->play_anim(ANIM_PRIMARY, g_surfer_anims[a], start_time, ANIM_NONCOSMETIC | (mAnimDir == SURFER_ANIM_REVERSE ? ANIM_REVERSE : 0));
	}
  lastTime = time;
}

void kellyslater_controller::SetPlayerCamera(game_camera *cam)
{
	if (!cam)
		return;

	UNDERWATER_CameraReset();
	if ((cam == beach_cam_ptr) || (cam == follow_close_cam_ptr) || (cam == follow_cam_ptr) || (cam == shoulder_cam_ptr) || (cam == ksdebug_cam_ptr) || (cam == fps_cam_ptr))
	{
		player_cam = cam;
		player_cam->init();
	}

	g_game_ptr->set_player_camera(this->my_player_num, cam);
}


extern profiler_timer proftimer_ksc_anim;

void kellyslater_controller::Anim(int animid, float blend_time, bool loop, float time, bool reverse )
{
  START_PROF_TIMER( proftimer_ksc_anim );

//  bool same_anim = (mAnim==animid);
  mAnim=animid;
  mAnimDir = reverse ? SURFER_ANIM_REVERSE : SURFER_ANIM_FORWARDS;
  float time_dilation = 1.0f;

  entity_anim_tree* anim_tree = get_owner()->get_anim_tree(ANIM_PRIMARY);
  bool no_tween;

  no_tween = g_no_tween || !anim_tree || (blend_time <= 0.01f);///*|| anim_tree->is_finished()*/ || ((anim_tree->get_timescale_factor() == 0.0f || same_anim) && anim_tree->get_time_remaining() < 0.15f);
  ignore_tweening = no_tween;

  //no_tween = false;  // test by JBW

  ksreplay.SetKSAnimInfo(this, blend_time, loop, time);

  if(time > 0.0f)
  {
    //rational_t start_time = (mFrame / 30.0f);
    get_owner()->play_anim(ANIM_PRIMARY, g_surfer_anims[animid], time, (loop ? ANIM_LOOPING : 0) | (no_tween ? 0 : ANIM_TWEEN) | ANIM_NONCOSMETIC | (mAnimDir == SURFER_ANIM_REVERSE ? ANIM_REVERSE : 0));
    get_owner()->get_anim_tree(ANIM_PRIMARY)->frame_advance(0.0f);
  }
  else
  {
	  get_owner()->play_anim(ANIM_PRIMARY, g_surfer_anims[animid], 0.0f, (loop ? ANIM_LOOPING : 0) | (no_tween ? 0 : ANIM_TWEEN) | ANIM_NONCOSMETIC | (mAnimDir == SURFER_ANIM_REVERSE ? ANIM_REVERSE : 0));
	  if (!no_tween)
		  get_owner()->get_anim_tree(ANIM_PRIMARY)->set_tween_duration(blend_time);
  }

#ifdef _DEBUG
  curr_anim_enum = animid;
#endif

  anim_tree = get_owner()->get_anim_tree(ANIM_PRIMARY);
	if ( anim_tree==NULL )
	{
		warning("Bad anim_tree");
		return;
	}
  anim_tree->set_timescale_factor(time_dilation);

  if(time_dilation == 0.0f)
  {
    anim_tree->set_time(0.0f);
    anim_tree->frame_advance(0.0f);
  }
  STOP_PROF_TIMER( proftimer_ksc_anim );
}

void kellyslater_controller::BoardAnim(int animid, float blend_time, bool loop, float time )
{
#if 1

//  bool same_anim = (bAnim==animid);
  bAnim=animid;
  short bAnimDir = SURFER_ANIM_FORWARDS;
  float time_dilation = 1.0f;

  if (animid == BOARD_ANIM_ZERO)
	  my_board_model->set_rel_po(po_identity_matrix);

  entity_anim_tree* anim_tree = my_board_model->get_anim_tree(ANIM_PRIMARY);
  bool no_tween = false;	// was uninitialized (dc 01/29/02)

  if(!ignore_tweening)
	  no_tween = !anim_tree || (blend_time == 0.0f);
	//no_tween = !anim_tree /*|| anim_tree->is_finished()*/ || ((anim_tree->get_timescale_factor() == 0.0f || same_anim) && anim_tree->get_time_remaining() < 0.15f);

  //no_tween = false;  // test by JBW
  ksreplay.SetKSBAnimInfo(this, blend_time, loop, time);
  if(time > 0.0f)
  {
    //rational_t start_time = (mFrame / 30.0f);
    my_board_model->play_anim(ANIM_PRIMARY, g_board_anims[animid], time, (loop ? ANIM_LOOPING : 0) | (no_tween ? 0 : ANIM_TWEEN) | ANIM_NONCOSMETIC | (mAnimDir == SURFER_ANIM_REVERSE ? ANIM_REVERSE : 0));
    my_board_model->get_anim_tree(ANIM_PRIMARY)->frame_advance(0.0f);
  }
  else
  {
	  my_board_model->play_anim(ANIM_PRIMARY, g_board_anims[animid], 0.0f, (loop ? ANIM_LOOPING : 0) | (no_tween ? 0 : ANIM_TWEEN) | ANIM_NONCOSMETIC | (bAnimDir == SURFER_ANIM_REVERSE ? ANIM_REVERSE : 0));
	  if (!no_tween)
		  my_board_model->get_anim_tree(ANIM_PRIMARY)->set_tween_duration(blend_time);
  }

  anim_tree = my_board_model->get_anim_tree(ANIM_PRIMARY);
	if ( anim_tree==NULL )
	{
		warning("Bad anim_tree");
		return;
	}
  anim_tree->set_timescale_factor(time_dilation);

  if(time_dilation == 0.0f)
  {
    anim_tree->set_time(0.0f);
    anim_tree->frame_advance(0.0f);
  }
#endif
}

float kellyslater_controller::GetAnimPercentage()
{
  entity_anim_tree * anim_tree = get_owner()->get_anim_tree(ANIM_PRIMARY);
	if ( anim_tree==NULL )
	{
		assert(0);
		return 1.0f;
	}

  float d = anim_tree->get_duration();
  float result;
  if (d)
    result = (anim_tree->get_time()/anim_tree->get_duration());
  else
	{
		assert(0);
    result = 1.0f;
	}

  return min(result, 1.0f);
}


float kellyslater_controller::GetAnimDuration()
{
	entity_anim_tree * anim_tree = get_owner()->get_anim_tree(ANIM_PRIMARY);
	if ( anim_tree==NULL )
	{
		assert(0);
		return 1.0f;
	}

	return anim_tree->get_duration();
}

float kellyslater_controller::GetBoardAnimDuration()
{
	entity_anim_tree * anim_tree = my_board_model->get_anim_tree(ANIM_PRIMARY);
	if ( anim_tree==NULL )
	{
		assert(0);
		return 1.0f;
	}

	return anim_tree->get_duration();
}

float kellyslater_controller::GetAnimTime()
{
  entity_anim_tree * anim_tree = get_owner()->get_anim_tree(ANIM_PRIMARY);
	//assert(anim_tree);
  return (anim_tree?anim_tree->get_time():0.0f);
}


float kellyslater_controller::GetBoardAnimTime()
{
  entity_anim_tree * anim_tree = my_board_model->get_anim_tree(ANIM_PRIMARY);
	//assert(anim_tree);
  return (anim_tree?anim_tree->get_time():0.0f);
}


bool kellyslater_controller::AnimComplete()
{
  entity_anim_tree * anim_tree = get_owner()->get_anim_tree(ANIM_PRIMARY);
	//assert(anim_tree);
	if ( anim_tree==NULL ) return true;
  if (anim_tree->is_finished())
  {
	  if (anim_tree->is_done_tween())
		  return true;
  }
  return false;
  //return (anim_tree->is_finished() && anim_tree->is_done_tween());
}

bool kellyslater_controller::BoardAnimComplete()
{
  entity_anim_tree * anim_tree = my_board_model->get_anim_tree(ANIM_PRIMARY);
	//assert(anim_tree);
	if ( anim_tree==NULL ) return true;
  if (anim_tree->is_finished())
  {
	  if (anim_tree->is_done_tween())
		  return true;
  }
  return false;
  //return (anim_tree->is_finished() && anim_tree->is_done_tween());
}

bool kellyslater_controller::AnimLooped()
{
  entity_anim_tree * anim_tree = get_owner()->get_anim_tree(ANIM_PRIMARY);
	//assert(anim_tree);
  return anim_tree->was_looped();  //  this may return true only on the actual frame of looping
}


bool kellyslater_controller::AnimBlended()
{
  entity_anim_tree * anim_tree = get_owner()->get_anim_tree(ANIM_PRIMARY);
	//assert(anim_tree);
  return anim_tree->was_blended();
}


void kellyslater_controller::RotationVel(int angleId, float angle)
{

	float	*gopRotDestVel;
	float	*gopRotVel;
	float	*gopRotAccel;

	switch (angleId)
	{
		case TURN:
			gopRotVel	  	= &(rotVel.y);
			gopRotDestVel	= &(rotDestVel.y);
			gopRotAccel		= &(rotAccel.y);
			break;
		case BANK:
			gopRotVel	   	= &(rotVel.z);
			gopRotDestVel 	= &(rotDestVel.z);
			gopRotAccel		= &(rotAccel.z);
			break;
		case PITCH:
		default:
			gopRotVel	   	= &(rotVel.x);
			gopRotDestVel 	= &(rotDestVel.x);
			gopRotAccel		= &(rotAccel.x);
			break;
	}

	*gopRotDestVel = *gopRotVel = DEG_TO_RAD(angle);
	*gopRotAccel = 0;
}


void kellyslater_controller::RotationTo(int angleId, float angle, float vel, float accel)
{

	float	*gopRotDest;
	float	*gopRotDestVel;
	float	*gopRot;
	float	*gopRotAccel;

	switch (angleId)
	{
		case TURN:
			gopRotDest		= &(rotDest.y);
			gopRotDestVel	= &(rotDestVel.y);
			gopRot			= &(rot.y);
			gopRotAccel		= &(rotAccel.y);
			break;
		case BANK:
			gopRotDest		= &(rotDest.z);
			gopRotDestVel	= &(rotDestVel.z);
			gopRot			= &(rot.z);
			gopRotAccel		= &(rotAccel.z);
			break;
		case PITCH:
		default:
			gopRotDest		= &(rotDest.x);
			gopRotDestVel	= &(rotDestVel.x);
			gopRot			= &(rot.x);
			gopRotAccel		= &(rotAccel.x);
			break;
	}

	*gopRotDest = DEG_TO_RAD(angle);
	vel = DEG_TO_RAD(__fabs(vel));
	accel = DEG_TO_RAD(__fabs(accel));
	if (*gopRotDest >= *gopRot)
	{
		*gopRotAccel = accel;
		*gopRotDestVel = vel;
	}
 	else
	{
		*gopRotAccel = -accel;
		*gopRotDestVel = -vel;
	}
}



float kellyslater_controller::GetRotation(int angleId)
{
  float outval;
	switch (angleId)
	{
		case TURN:
			outval = rot.y;
			break;
		case BANK:
			outval = rot.z;
			break;
		case PITCH:
		default:
			outval = rot.x;
			break;
	}

	return outval;
}


/*
void kellyslater_controller::SetAccel(float a)
{
	speed += a;
}
*/


int kellyslater_controller::GetTurnCycle(float _degree)
{
  int result;
	if (_degree > 0.0f)
	{
		if (_degree < CUTOFF0)
			result = PART_RIGHT0;
		else if (_degree < CUTOFF1)
			result = PART_RIGHT1;
		else
			result = PART_RIGHT2;
	}
	else if (_degree < 0.0f)
	{
		if (_degree > -CUTOFF0)
			result = PART_LEFT0;
		else if (_degree > -CUTOFF1)
			result = PART_LEFT1;
		else
			result = PART_LEFT2;
	}
	else
	{
		result = PART_CENTER;
	}

	return result;
}

int kellyslater_controller::GetAnalogTurn(void)
{
	//  this function scales the analog stick output between -9 to 9, 0 exclusive

	float f_result =  9.0f*my_board_controller.GetLeanPercentage();
	if (f_result <= 0.0f)
		f_result -= 1.0f;
	else
		f_result += 1.0f;

	int result = (int) f_result;

	if (result > 9)
		result = 9;
	else if (result < -9)
		result = -9;

	return result;
}

void kellyslater_controller::DetermineTakeoffType(bool weak)
{
	bool left_wave = BeachDataArray[g_game_ptr->get_beach_id ()].bdir;
	vector3d board_vec = my_board_controller.GetForwardDir();

	//  Which direction is the board facing.
	if ((board_vec.x > 0 && left_wave) || (board_vec.x < 0 && !left_wave))  //  Faced towards tube.
	{
		if (board_vec.z > 0)  //  backwards
			my_scoreManager.AddTrick(TRICK_FAKIEFADETAKEOFF);
		else
			my_scoreManager.AddTrick(TRICK_FADETAKEOFF);
	}
	else
	{
		if (board_vec.z > 0)  //  backwards
			my_scoreManager.AddTrick(TRICK_FAKIETAKEOFF);
		else
			my_scoreManager.AddTrick(TRICK_TAKEOFF);
	}

	my_scoreManager.UpdateLastTrick(ScoringManager::ATTR_DIST_LIP, weak?100.0f:Lip_Distance());
}

/*
WaveRegionEnum kellyslater_controller::WaveRegion()
{
  return WAVE_GetWaveRegion(owner->get_abs_position());
}
*/

bool kellyslater_controller::IsInsideWave()
{
/*	This code is not doing anything, since WAVE_CheckCollision does not set the NO_HITS flag.
	Is the intent to check whether the position is in the water, or to check whether it is underneath
	the lip of the wave?  (dc 05/29/01)
CollideCallStruct collide;

vector3d v = owner->get_abs_position();
collide.position.x = v.x;
collide.position.y = v.y;
collide.position.z = v.z;
WAVE_CheckCollision(collide, NULL);
return (collide.flags!=NO_HITS);
*/
	return false;
}

void kellyslater_controller::SetFloatPO(float angle)
{
	vector3d xvec, yvec, zvec;
	if (IsPlayingFloatAnim() && !spin_ctrl.IsActivated())
	{
		if (float_trick == TRICK_FLOAT_RIGHT)
			zvec = grind_right;
		else if (float_trick == TRICK_FLOAT_LEFT)
			zvec = -grind_right;
		else if (float_trick == TRICK_FLOAT_BACK)
			zvec = -grind_vector;
		else// if (float_trick == TRICK_FLOAT_FORWARD)
			zvec = grind_vector;
	}
	else
		zvec = my_board_controller.rb->my_po.get_z_facing();

	yvec = cross(grind_vector, grind_right);
	zvec -= yvec*dot(zvec, yvec);
	zvec.normalize();
	xvec = cross(yvec, zvec);
	po board_po = po(xvec, yvec, zvec, vector3d(0,0,0));

	po turn = po_identity_matrix;
	turn.set_rot (board_po.non_affine_inverse_xform(grind_vector), angle);
	board_po = turn * board_po;
	my_board_controller.rb->my_po = board_po;
}

bool kellyslater_controller::CheckGrindPathEnd(void)
{
	vector3d vec1 = *WAVE_GetMarker(WAVE_MarkerGrindStart1) - *WAVE_GetMarker(WAVE_MarkerGrindStart2);
	vector3d vec2 = my_board_controller.rb->pos - *WAVE_GetMarker(WAVE_MarkerGrindStart2);
	if (dot(vec1,vec2) > 0.0f)
		return true;

	return false;
}

float float_time_max = 2.0f;
float dismount_percent = 0.5f;

void kellyslater_controller::StartFloat(const vector3d direction)
{

	vector3d vel = my_board_controller.GetVelocity();
	current = my_board_controller.GetWaveCurrent();
	vel -= WAVE_CURRENT_MOD*current;
	if (dot(XVEC, vel) >= 0.0f)
		grind_vector = XVEC;
	else
		grind_vector = -XVEC;

	vector3d forward_vec = my_board_controller.GetForwardDir();
	forward_vec.y = 0.0f;
	forward_vec.normalize();
	grind_right = cross(YVEC, grind_vector);

	SetCompletedTrick(TRICK_FLOATER);
	currentTrick = TRICK_FLOATER;
	if (float_trick == -1)
	{
		float d = dot(forward_vec, grind_vector);
		float r = dot(forward_vec, grind_right);
		if (d > 0.707f)
		{
			float_trick = TRICK_FLOAT_FORWARD;
			float_anim = SURFER_ANIM_TRK_FLOAT_F_C_1;
		}
		else if (d < -0.707f)
		{
			float_trick = TRICK_FLOAT_BACK;
			float_anim = SURFER_ANIM_TRK_FLOAT_B_C_1;
		}
		else if (r > 0.707f)
		{
			float_trick = TRICK_FLOAT_RIGHT;
			if (!goofy)
				float_anim = SURFER_ANIM_TRK_FLOAT_R_C_1;
			else
				float_anim = SURFER_ANIM_TRK_FLOAT_L_C_1;
		}
		else
		{
			float_trick = TRICK_FLOAT_LEFT;
			if (!goofy)
				float_anim = SURFER_ANIM_TRK_FLOAT_L_C_1;
			else
				float_anim = SURFER_ANIM_TRK_FLOAT_R_C_1;
		}
	}

	float_time = vel.length()*(0.2f*float_time_max*dismount_percent) + 0.5f;
	if (float_time > float_time_max)
		float_time = float_time_max;

	state = STATE_FLOAT;
	super_state = SUPER_STATE_NORMAL_SURF;
	my_board_controller.SetBoardState(BOARD_FLOAT);
	Anim(SURFER_ANIM_TRK_INTO_FLOAT_1, BLEND_TIME, false);
	BoardAnim(BOARD_ANIM_TRK_INTO_FLOAT_1, BLEND_TIME, false);
	slide_dismount_timer = 0.0f;
	tween_timer = 0.0f;
	tween_time = 1.0f;
	IK_enabled = true;

	floaterEvent = SoundScriptManager::inst()->startEvent(SS_FLOATER, my_board_model);
}


void kellyslater_controller::StartGrind(const vector3d direction)
{
/*	grind_vector = direction;
	grind_vector.normalize();

	vector3d yvec = YVEC - grind_vector*dot(YVEC, grind_vector);
	vector3d forward_vec = my_board_controller.GetForwardDir();
	forward_vec -= yvec*dot(yvec, forward_vec);
	forward_vec.normalize();
	vector3d right = cross(yvec, forward_vec);
	grind_right = cross(yvec, grind_vector);

	vector3d pos = my_board->get_abs_position();
	po new_po = po(right, yvec,forward_vec, pos);
	my_board_controller.rb->my_po = new_po;
	my_board->set_rel_po(new_po);

	if (float_trick == -1)
	{
		float d = dot(forward_vec, grind_vector);
		float r = dot(forward_vec, grind_right);
		if (d > 0.707f)
			float_trick = TRICK_FLOAT_FORWARD;
		else if (d < -0.707f)
			float_trick = TRICK_FLOAT_BACK;
		else if (r > 0.707f)
			float_trick = TRICK_FLOAT_RIGHT;
		else
			float_trick = TRICK_FLOAT_LEFT;
	}

	vector3d grnd = grind_vector;
	float_quadrant = 0;
	float_board_anim = BOARD_ANIM_ZERO;
	if (float_trick == TRICK_FLOAT_RIGHT)
	{
		grnd = grind_right;
		float_quadrant = 1;
		if (!goofy)
			float_anim = SURFER_ANIM_TRK_FLOAT_R_C_1;
		else
			float_anim = SURFER_ANIM_TRK_FLOAT_L_C_1;

		//float_board_anim = BOARD_ANIM_TRK_FLOAT_R_1;
	}
	else if (float_trick == TRICK_FLOAT_LEFT)
	{
		grnd = -grind_right;
		float_quadrant = 3;
		if (!goofy)
			float_anim = SURFER_ANIM_TRK_FLOAT_L_C_1;
		else
			float_anim = SURFER_ANIM_TRK_FLOAT_R_C_1;

		float_board_anim = BOARD_ANIM_TRK_FLOAT_L_1;
	}
	else if (float_trick == TRICK_FLOAT_BACK)
	{
		grnd = -grind_vector;
		float_quadrant = 2;
		float_anim = SURFER_ANIM_TRK_FLOAT_B_C_1;
	}
	else// if (float_trick == TRICK_FLOAT_FORWARD)
	{
		float_trick = TRICK_FLOAT_FORWARD;
		grnd = grind_vector;
		float_quadrant = 0;
		float_anim = SURFER_ANIM_TRK_FLOAT_F_C_1;
	}

	float dot_amt = dot(forward_vec, grnd);
	if (dot_amt > 1.0f)
		dot_amt = 1.0f;
	else if (dot_amt < -1.0f)
		dot_amt = -1.0f;

//	float amt = fast_acos(dot_amt);

	int f_anim;
	vector3d crs = cross(forward_vec, grnd);
	if (dot(yvec, crs) > 0.0f)
		f_anim = SURFER_ANIM_TRK_FLOAT_TRR_1;
	else
		f_anim = SURFER_ANIM_TRK_FLOAT_TLL_1;

	spin_ctrl.SetSpinType(SPIN_FLOATER, 0.0f, grnd, 2.0f);
	if (state == STATE_LANDING)
		my_board_controller.BoardFloatWave(0.0f);

	state = STATE_GRIND;
	super_state = SUPER_STATE_NORMAL_SURF;
	my_board_controller.SetBoardState(BOARD_GRIND);
	Anim(f_anim, 0.2f, false);
	float_meter.Init(my_player_num, false, 5.0, attribs[ATTRIB_BALANCE]);//, true);
	slide_dismount_timer = 0.0f;*/
}
// -------------------------------
//
//      Pseudo States - were states in the old IRC system, now are C-functions
//
// -------------------------------
void kellyslater_controller::TurnDegree()
{
	stick = GetStick(PAD_LSTICK_H);
	degree = fabs(1.0f*stick);
}


void kellyslater_controller::TurnType()
{
	lastTurnType = turnType;
	last_region = current_region;
	current_region = my_board_controller.GetWaveRegion();
	//turnType = (int) GetStick(PAD_LSTICK_H);

	if (CtrlEvent(PAD_GRAB) && (CtrlDelta(PAD_SLIDE) || (CtrlEvent(PAD_LSTICK_H) && CtrlEvent(PAD_SLIDE))))
		turnType = GRABSLIDE_TURN;
	else if (CtrlEvent(PAD_GRAB))
		turnType = GRAB_TURN;
	else if (CtrlDelta(PAD_OLLIE) || (CtrlEvent(PAD_LSTICK_H) && CtrlEvent(PAD_OLLIE)))
		turnType = CROUCH_TURN;
	//else if (my_board_controller.InSnapRegion() && perform_snap && CtrlEvent(PAD_SNAP))
	else if (perform_snap)
		turnType = SNAP_TURN;
	else if (CtrlDelta(PAD_CARVE) || (CtrlEvent(PAD_LSTICK_H) && CtrlEvent(PAD_CARVE)))
		turnType = CARVE_TURN;
	else if (CtrlDelta(PAD_SLIDE) || (CtrlEvent(PAD_LSTICK_H) && CtrlEvent(PAD_SLIDE)))
		turnType = TAILSLIDE_TURN;
	else if (CtrlEvent(PAD_LSTICK_H))
	{
		//if (CtrlEvent(PAD_HARD))
			//turnType = HARD_REGULAR_TURN;
		turnType = HARD_REGULAR_TURN;
		//else
			//turnType = REGULAR_TURN;
	}

	perform_snap = false;

	if (turnType != lastTurnType ||
		((last_region == WAVE_REGIONWOBBLE && current_region != WAVE_REGIONWOBBLE) && !my_board_controller.DoingFaceTurn()
			&& (turnType != TAILSLIDE_TURN) && (turnType != GRABSLIDE_TURN)))
	{
		my_board_controller.SetTurnType(REGULAR_TURN);
		changeTurnType = TRUE;
	}
	else
		changeTurnType = FALSE;

	return;
}


void kellyslater_controller::Spin(float t)
{
	if (CtrlEvent(PAD_LSTICK_H))
	{
		my_board_controller.SetTurnType(TRICK_SPIN);

		TurnDegree();
		if (stick > 0.0f)
			my_board_controller.Turn(BOARD_TURN_RIGHT, degree, t);
		else
			my_board_controller.Turn(BOARD_TURN_LEFT, degree, t);
	}
	else
	{
		my_board_controller.SetTurnType(TRICK_SPIN);
		my_board_controller.Turn(BOARD_TURN_RIGHT, 0.0f, t);
	}
}

float mav_fudge = 2.0f;

//  How far the surfer is from the lip.  For use with the big wave to see where the surfer is on the wave.
//  Range: 0 (on lip) and up, with 1.0 being at about the bottom of the wave.
float kellyslater_controller::Lip_Distance()
{
	vector3d float_pos = my_board_controller.GetFloatPos();
	vector3d max_pos = *WAVE_GetMarker(WAVE_MarkerLipMark6);
	float total_depth = float_pos.z - max_pos.z;

	if (g_game_ptr->get_beach_id() == BEACH_MAVERICKS)  // Mav's weird shape causes it to need a little extra space.
		total_depth *= mav_fudge;

	float distance = max(0.0f, float_pos.z - my_board->get_abs_position ().z);
	distance = distance/total_depth;

	return distance;
}


bool kellyslater_controller::CheckForRoof(void)
{
	int region = my_board_controller.GetWaveRegion();
	bool break_map = my_board_controller.TongueEffectOn();
	if (!my_board_controller.inAirFlag && ((region == WAVE_REGIONROOF) || (region == (WAVE_REGIONMAX + 1))
		|| (region == WAVE_REGIONSHOULDER) || (region == WAVE_REGIONCURL) || (region == WAVE_REGIONMAX)))
	{
		if (!break_map || (break_map && (state != STATE_FLOAT)))
		{
			state = STATE_ON_ROOF;
			bSpecialTrick = false;
			super_state = SUPER_STATE_NORMAL_SURF;
			return true;
		}
	}

	return false;
}

float duck_turning_speed = 5.0f;
float sink_val = 1.0f;
float start_wip_time = 0.0f;

/*
// variable for turns
int		leftTurnAnim;
int		rightTurnAnim;
int		turnAnim;
float	turnVel;
float	bankAccel;
float	bankVel;
float	bank;
*/

extern int test_big_wave;
float big_wave_extra_speed = 0;
float bg_acc_mod = 1.0f;
int takeoff_wobble_thresh = 5;
float g_lie_too_vert = 0.34f;

void kellyslater_controller::SuperStateLieOnBoard(float dt)
{
//	int current_beach = g_game_ptr->get_beach_id ();

	if (super_state != last_super_state)
	{
		if (!IsAIPlayer())
			frontendmanager.IGO->GetHintManager()->SetHint(IGOHintManager::StandUp);
		no_buttons_pressed = true;
		spin_ctrl.Reset();
		BoardAnim(BOARD_ANIM_ZERO, 0.0f, false);
		get_owner()->SetCull(true);

		//  Reset the level timer so that the big wave will take the whole time.
//		if (BeachDataArray[current_beach].is_big_wave || test_big_wave)
//			TIMER_Init();  
	}

	last_super_state = super_state;

	my_board_controller.grind_ollie = false;
	my_board_controller.roof_jump = false;
	bool too_vert = (dot(my_board_controller.GetUpDir(), YVEC) < g_lie_too_vert);
	my_board_controller.SetTurnType(LIE_TURN);
	rideTimer = 0.0f;

	if (my_trickManager.ButtonPressed())
	{
		no_buttons_pressed = false;
		num_wipeouts = 0;
	}

	switch (state)
	{
	case STATE_LIEONBOARD:
		{
			if (state != last_state)
			{
				IK_enabled = false;
				Anim(SURFER_ANIM_IDLE_LAYING, 0.0f);
				BoardAnim(BOARD_ANIM_IDLE_LAYING, 0.0f);
				//big_wave_meter.Init(my_player_num, 30, 5, true);
			}

			last_state = state;

			//my_board_controller.rb->linMom.z += SURFER_LIE_PULLBACK * (30 * dt);

			vector3d forw = my_board_controller.GetForwardDir();
			forw.y = 0.0f;
			forw.normalize();

			float face_z = WAVE_GetMarker(WAVE_MarkerFaceTrickZ)->z;
			bool duck_dive = (face_z < my_board_controller.rb->pos.z);

			//big_wave_meter.Resize(Lip_Distance());

			if (CtrlEvent(PSX_TRIANGLE))
			{
				state = STATE_LIETOSTAND;
				super_state = SUPER_STATE_NORMAL_SURF;
				float lip_dist = Lip_Distance();
				take_off_type = (int)(lip_dist * 3);
				if (take_off_type >= takeoff_wobble_thresh)
					takeoff_wobble = true;
				else
				{
					takeoff_wobble = false;
					DetermineTakeoffType();
				}
				//big_wave_meter.End();
			}
			else if (duck_dive && CtrlEvent(PAD_GRAB) && (dot(ZVEC, forw) > 0.707f) && !g_game_ptr->is_splitscreen())
			{
				state = STATE_DUCKDIVE;
				//big_wave_meter.End();
			}
			else
			{
				if (CtrlEvent(PAD_LSTICK_H))	// allow move to turn
				{
					TurnDegree();
					my_board_controller.SetTurnType(LIE_TURN);
					if (stick > 0.0f)
					{
						if (mAnim != SURFER_ANIM_LIE_PADDEL_R_1)
						{
							if (!goofy_int)
							{
								Anim(SURFER_ANIM_LIE_PADDEL_R_1, BLEND_TIME);
								BoardAnim(BOARD_ANIM_LIE_PADDEL_R_1, BLEND_TIME);
							}
							else	//  Reverse the animations for the goofy guy, so he'll look back in the right direction.
							{
								Anim(SURFER_ANIM_LIE_PADDEL_L_1, BLEND_TIME);
								BoardAnim(BOARD_ANIM_LIE_PADDEL_L_1, BLEND_TIME);
							}
						}

						my_board_controller.Turn(BOARD_TURN_RIGHT, degree, dt);
					}
					else
					{
						if (mAnim != SURFER_ANIM_LIE_PADDEL_L_1)
						{
							if (!goofy_int)
							{
								Anim(SURFER_ANIM_LIE_PADDEL_L_1, BLEND_TIME);
								BoardAnim(BOARD_ANIM_LIE_PADDEL_L_1, BLEND_TIME);
							}
							else	//  Reverse the animations for the goofy guy, so he'll look back in the right direction.
							{
								Anim(SURFER_ANIM_LIE_PADDEL_R_1, BLEND_TIME);
								BoardAnim(BOARD_ANIM_LIE_PADDEL_R_1, BLEND_TIME);
							}
						}

						my_board_controller.Turn(BOARD_TURN_LEFT, degree, dt);
					}
				}
				else
				{
					if (mAnim != SURFER_ANIM_IDLE_LAYING)
						Anim(SURFER_ANIM_IDLE_LAYING, BLEND_TIME);

					my_board_controller.Turn(BOARD_TURN_RIGHT, 0.0, dt);
				}

				if (CtrlEvent(PAD_LSTICK_V))
					state = STATE_PADDLE;
			}

			if (too_vert)
			{
				my_board_controller.DoWipeOut(WIP_LOW_LIE_FOR);
				if (!IsAIPlayer())
					frontendmanager.IGO->GetHintManager()->SetHint(IGOHintManager::DidNotStand);
			}
			else if ((my_board_controller.GetWaveRegion() == WAVE_REGIONLIP) || (my_board_controller.GetWaveRegion() == WAVE_REGIONLIP2))
			{
				my_board_controller.DoWipeOut(WIP_LOW_LIE_FLAT);
				if (!IsAIPlayer())
					frontendmanager.IGO->GetHintManager()->SetHint(IGOHintManager::DidNotStand);
			}

			exit_state = true;
			break;
		}

	case STATE_PADDLE:
		{
			if (state != last_state)
			{
				if (GetStick(PAD_LSTICK_V) < 0.0f)	//  Forward
				{
					Anim(SURFER_ANIM_LIE_PADDLE, BLEND_TIME);
					BoardAnim(BOARD_ANIM_LIE_PADDLE, BLEND_TIME);
				}
				else		//  backward
				{
					Anim(SURFER_ANIM_LIE_PADDLE_BK, BLEND_TIME);
					BoardAnim(BOARD_ANIM_LIE_PADDLE_BK, BLEND_TIME);
				}
			}

			last_state = state;

			//my_board_controller.rb->linMom.z += SURFER_LIE_PULLBACK * (30 * dt);

			vector3d forw = my_board_controller.GetForwardDir();
			forw.y = 0.0f;
			forw.normalize();

			float face_z = WAVE_GetMarker(WAVE_MarkerFaceTrickZ)->z;
			bool duck_dive = (face_z < my_board_controller.rb->pos.z);
			//big_wave_meter.Resize(Lip_Distance());

			if (CtrlEvent(PSX_TRIANGLE))
			{
				state = STATE_LIETOSTAND;
				super_state = SUPER_STATE_NORMAL_SURF;
				float lip_dist = Lip_Distance();
				take_off_type = (int)(lip_dist * 3);
				if (take_off_type >= takeoff_wobble_thresh)
					takeoff_wobble = true;
				else
				{
					takeoff_wobble = false;
					DetermineTakeoffType();
				}
				my_board_controller.MoveForward(0.00f);
				//big_wave_meter.End();
			}
			else if (duck_dive && CtrlEvent(PAD_GRAB) && (dot(ZVEC, forw) > 0.707f) && !g_game_ptr->is_splitscreen())
			{
				state = STATE_DUCKDIVE;
				//big_wave_meter.End();
			}
			else
			{
				if (CtrlEvent(PAD_LSTICK_H))	// allow move to turn
				{
					TurnDegree();
					my_board_controller.SetTurnType(LIE_TURN);
					if (stick > 0.0f)
						my_board_controller.Turn(BOARD_TURN_RIGHT, degree, dt);
					else
						my_board_controller.Turn(BOARD_TURN_LEFT, degree, dt);
				}
				else
				{
					my_board_controller.Turn(BOARD_TURN_RIGHT, 0.0, dt);
				}

				if (CtrlEvent(PAD_LSTICK_V))	// allow move forward or back
				{
					stick = GetStick(PAD_LSTICK_V);
					if (stick >= 0.0f)
						my_board_controller.MoveForward(-SURFER_LIE_PADDLE_ACCELERATION);
					else if (stick < 0.0f)
						my_board_controller.MoveForward(SURFER_LIE_PADDLE_ACCELERATION);
				}
				else
				{
					my_board_controller.MoveForward(-SURFER_LIE_PADDLE_ACCELERATION);
					Anim(SURFER_ANIM_IDLE_LAYING, BLEND_TIME);
					BoardAnim(BOARD_ANIM_IDLE_LAYING, BLEND_TIME);
					state = STATE_LIEONBOARD;
					last_state = state;
					IK_enabled = false;
				}
			}

			if (!g_disable_wipeouts)
			{
				if (too_vert)
				{
					my_board_controller.DoWipeOut(WIP_LOW_LIE_FOR);
					if (!IsAIPlayer())
						frontendmanager.IGO->GetHintManager()->SetHint(IGOHintManager::DidNotStand);
				}
				else if ((my_board_controller.GetWaveRegion() == WAVE_REGIONLIP) || (my_board_controller.GetWaveRegion() == WAVE_REGIONLIP2))
				{
					my_board_controller.DoWipeOut(WIP_LOW_LIE_FLAT);
					if (!IsAIPlayer())
						frontendmanager.IGO->GetHintManager()->SetHint(IGOHintManager::DidNotStand);
				}
			}

			exit_state = true;
			break;
		}

	case STATE_DUCKDIVE:
		if (state != last_state)
		{
			duckdive_timer = 0;
			Anim(SURFER_ANIM_BAS_DUCKDIVE_DOWN_1, BLEND_TIME, false);
			BoardAnim(BOARD_ANIM_BAS_DUCKDIVE_DOWN_1, BLEND_TIME, false);
			duckdive_cam_ptr->Reset();
			SetPlayerCamera(duckdive_cam_ptr);
			dry = false;
			tween_time = 0.5f;
			tween_timer = 0.0f;

			// Make clock stop while duck diving.
			if (g_game_ptr->get_num_active_players() == 1)
				TIMER_SetLevelSec(TIMER_GetLevelSec()+CLOCKTIME_DUCK_DIVE);

			g_eventManager.DispatchEvent(EVT_SURFER_DUCK_DIVE, my_player_num);
		}

		bSpecialTrick = false;  //  just in case a special trick gets cued up, this allows level to end
		last_state = state;

		exit_state = true;
		if (AnimComplete())
		{
			if (mAnim == SURFER_ANIM_BAS_DUCKDIVE_DOWN_1)
			{
				Anim(SURFER_ANIM_BAS_DUCKDIVE_IDLE_1, BLEND_TIME);
				BoardAnim(BOARD_ANIM_BAS_DUCKDIVE_IDLE_1, BLEND_TIME);
			}
			else if (mAnim == SURFER_ANIM_BAS_DUCKDIVE_UP_1)
			{
				duckdive_cam_ptr->SetExitTransition();
				state = STATE_LIEONBOARD;
				last_state = state;
				Anim(SURFER_ANIM_IDLE_LAYING, BLEND_TIME, false);
				BoardAnim(BOARD_ANIM_IDLE_LAYING, BLEND_TIME, false);
				exit_state = false;
			}
		}
		else if ((duckdive_timer >= TOTAL_DUCKDIVE_TIME) && (mAnim == SURFER_ANIM_BAS_DUCKDIVE_IDLE_1))
		{
			Anim(SURFER_ANIM_BAS_DUCKDIVE_UP_1, BLEND_TIME, false);
			BoardAnim(BOARD_ANIM_BAS_DUCKDIVE_UP_1, BLEND_TIME, false);
		}
		else if (mAnim == SURFER_ANIM_BAS_DUCKDIVE_IDLE_1)
		{
			duckdive_timer += dt;
		}
		else if (mAnim == SURFER_ANIM_BAS_DUCKDIVE_UP_1)
		{
			float perc = GetAnimPercentage();
			if (!duckdive_cam_ptr->do_reset && (perc > 0.6f) && !this->IsAIPlayer())
				duckdive_cam_ptr->SetReset();
		}

		OrientToWave(true, dt, SET_RB_PO);
		break;

	case STATE_SWIMTOLIE:

		//WAVE_GlobalCurrent (&my_board_controller.rb->linMom);
		if (state != last_state)
		{
			if (!IsAIPlayer())
			{
				SoundScriptManager::inst()->playEvent(SS_EVENT_RESPAWN);
			}
			Anim(get_on_board_anim, 0.0f, false);
			BoardAnim(b_get_on_board_anim, 0.2f, false);
//      IGOStandUp();
			IK_enabled = false;
		}

		last_state = state;

		if (AnimComplete())//AnimEvnt(ANIM_END_EVENT))
			state = STATE_LIEONBOARD;
		else
			exit_state = true;
		break;

	case STATE_STANDTOLIE:

		if (state != last_state)
		{
			Anim(SURFER_ANIM_TRANS_STAND_LIE_1, BLEND_TIME, false);
//      IGOStandUp();
			IK_enabled = false;
		}

		last_state = state;

		if (AnimComplete())//AnimEvnt(ANIM_END_EVENT))
			state = STATE_LIEONBOARD;

		exit_state = true;
		break;

/*	case STATE_STANDTRANS:

		if (state != last_state)
			Anim(SURFER_ANIM_LIETOSTAND, BLEND_TIME, false);

		last_state = state;

		if (AnimComplete())//AnimEvnt(ANIM_END_EVENT))
		{
			state = STATE_STAND;
		}

		exit_state = true;
		break;
*/
	}  //  end switch
}

float float_dismount_vel = 2.0f;
void kellyslater_controller::SuperStatePrepareJump(float dt)
{
	int cycle;
	float charge;


	//float grind_delta = 1.0f;

	if (grind_jump_timer >= 0.0f)
		grind_jump_timer += dt;

	if (last_super_state != SUPER_STATE_PREPARE_JUMP)
	{
		if ((my_board_controller.GetBoardState() == BOARD_GRIND) || (my_board_controller.GetBoardState() == BOARD_FLOAT))
			grind_jump_timer = 0.0f;
		else
			grind_jump_timer = -1.0f;

		my_board_controller.grind_jump = false;
		my_board_controller.grind_ollie = false;
		my_board_controller.roof_jump = false;
	}

	//my_board_controller.GoneOverLip(dt);

	// this resets float_trick and float_quadrant so that float tricks inputted while in air will be
	// executed upon landing
	float_trick = -1;
	float_quadrant = -1;

	DetermineTubeGaps();
	last_super_state = super_state;

	if (CheckForRoof())
		return;

	switch (state)
	{
	case STATE_CROUCH:
		{
			float acc;
			if (my_board_controller.GetWaveRegion() == WAVE_REGIONWASH)
				acc = SURFER_CROUCH_ACCELERATION;
			else
				acc = SURFER_CROUCH_ACCELERATION*dot(my_board_controller.GetForwardDir(), ZVEC) + SURFER_ACCELERATION;

			my_board_controller.MoveForward(acc);
			IK_enabled = true;
			my_board_controller.SetTurnType(HARD_REGULAR_TURN);
			if (last_state != state)
			{
				BoardAnim(BOARD_ANIM_ZERO, 0.2f, false);
				Anim(SURFER_ANIM_BAS_CROUCH, 0.2f);
			}

			last_state = state;

			bool pad_ollie = CtrlEvent(PAD_OLLIE);
			float vert = CtrlEvent(PAD_LSTICK_V);
			int next_air_state = -1;
			if (!pad_ollie)
			{
				// Launch up wave.
				if ((my_board_controller.GetBoardState() != BOARD_GRIND) && (my_board_controller.GetBoardState() != BOARD_FLOAT)
					&& my_board_controller.InLaunchRegion() && my_board_controller.InLaunchCone())
					next_air_state = STATE_LAUNCH;
				// Normal jump.
				else
					next_air_state = STATE_CHOP_HOP;
			}

			if (pad_ollie)
			{
				//charge = GetAnimPercentage(); // PTA 4/26/01  GetBlendPercentage();

				if ((my_board_controller.GetBoardState() != BOARD_GRIND) && (my_board_controller.GetBoardState() != BOARD_FLOAT)
									&& CtrlEvent(PAD_LSTICK_H))	// allow move to turn
				{
					state = STATE_PICKCROUCHTURNCYCLE;
				}
				else
				{
					if ((my_board_controller.GetBoardState() == BOARD_GRIND) || (my_board_controller.GetBoardState() == BOARD_FLOAT))
					{
						if (float_time < -0.5f)
						{
							state = STATE_EXIT_FLOAT;
							super_state = SUPER_STATE_NORMAL_SURF;
							my_board_controller.AddVelocity(-float_dismount_vel*ZVEC);
							my_board_controller.SetBoardState(BOARD_NONE);
							exit_state = true;
							break;
						}

						bool break_map = my_board_controller.TongueEffectOn();
						if (!break_map)
							float_time -= dt;

						if (CheckGrindPathEnd())
						{
							state = STATE_EXIT_FLOAT;
							super_state = SUPER_STATE_NORMAL_SURF;
							my_board_controller.AddVelocity(-float_dismount_vel*ZVEC);
							my_board_controller.SetBoardState(BOARD_NONE);
							exit_state = true;
							break;
						}
					}
					else //  if grinding or floating, don't want to reset
						my_board_controller.Turn(BOARD_TURN_LEFT, 0.0f, dt);

					exit_state = true;
				}
			}
			else if ((next_air_state == STATE_LAUNCH) || ((next_air_state == STATE_CHOP_HOP) && (vert > -0.95)))
			{
				//region = WaveRegion();
				charge = GetAnimPercentage(); // PTA 4/26/01  GetBlendPercentage();
				//if (charge < MIN_OLLIE_CHARGE || IsInsideWave() == FALSE || (region != REGION_LIP && region != REGION_SHOULDER && region != REGION_FACE))
					//charge = MIN_OLLIE_CHARGE;

				my_board_controller.MoveForward(0.0f);
				//float_meter.End();

				if ((my_board_controller.GetBoardState() == BOARD_GRIND) || (my_board_controller.GetBoardState() == BOARD_FLOAT))
				{
					my_board_controller.grind_jump = true;
					my_board_controller.grind_ollie = true;
				}

				state = next_air_state;

				super_state = SUPER_STATE_AIR;
				exit_state = false;
			}
			else
			{
				state = STATE_STAND;
				super_state = SUPER_STATE_NORMAL_SURF;
				exit_state = true;
			}
		}
		break;

	case STATE_PICKCROUCHTURNCYCLE:
		last_state = state;
	    if (!CtrlEvent(PAD_LSTICK_H))
		{
			state = STATE_CROUCH;
		}
		else
		{
			/*my_board_controller.SetTurnType(TRIM_TURN);
			stick = GetStick(PAD_LSTICK_H);
			cycle = GetTurnCycle(stick);
			if (cycle == PART_LEFT2 || cycle == PART_LEFT1 || cycle == PART_LEFT0)
			{
				Anim(SURFER_ANIM_BAS_L_CROUCH + goofy_int, BLEND_TIME);
				state = STATE_CROUCH_LEFTTURN;
			}
			else
			{
				Anim(SURFER_ANIM_BAS_R_CROUCH - goofy_int, BLEND_TIME);
				state = STATE_CROUCH_RIGHTTURN;
			}*/

			my_board_controller.SetTurnType(HARD_REGULAR_TURN);
			stick = GetStick(PAD_LSTICK_H);

			cycle = GetTurnCycle(stick);
			turn_index = GetAnalogTurn();
			int anim_n;
			if (my_board_controller.GetWaveRegion() == WAVE_REGIONWOBBLE)  //  Special bottom turns for the wake area.
			{
				if (turn_index < 0)
					anim_n = turn_anim_nums[0] + 18 - (turn_index + 1) + goofy_int*9;
				else
					anim_n = turn_anim_nums[9] + 18 + (turn_index - 1) - goofy_int*9;
			}
			else
			{
				if (turn_index < 0)
					anim_n = turn_anim_nums[0] - (turn_index + 1) + goofy_int*9;
				else
					anim_n = turn_anim_nums[9] + (turn_index - 1) - goofy_int*9;
			}

			if (cycle == PART_LEFT2 || cycle == PART_LEFT1 || cycle == PART_LEFT0)
			{
				Anim(anim_n, 0.175f);
				BoardAnim(BOARD_ANIM_ZERO, 0.175f, false);
				state = STATE_CROUCH_LEFTTURN;
			}
			else
			{
				Anim(anim_n, 0.175f);
				BoardAnim(BOARD_ANIM_ZERO, 0.175f, false);
				state = STATE_CROUCH_RIGHTTURN;
			}
		}
		break;

	case STATE_CROUCH_LEFTTURN:
		{
			last_state = state;
			exit_state = true;
			my_board_controller.MoveForward(SURFER_CROUCH_ACCELERATION);

			bool pad_ollie = CtrlEvent(PAD_OLLIE);
			float vert = CtrlEvent(PAD_LSTICK_V);
			int next_air_state = -1;
			if (!pad_ollie)
			{
				// Launch up wave.
				if (my_board_controller.InLaunchRegion() && my_board_controller.InLaunchCone())
					next_air_state = STATE_LAUNCH;
				// Normal jump.
				else
					next_air_state = STATE_CHOP_HOP;
			}

			if (!pad_ollie && ((next_air_state == STATE_LAUNCH) || ((next_air_state == STATE_CHOP_HOP) && (vert > -0.95))))
			{
				state = next_air_state;
				super_state = SUPER_STATE_AIR;

				charge = GetAnimPercentage(); // PTA 4/26/01  GetBlendPercentage();
				my_board_controller.MoveForward(0.0f);
				exit_state = false;
			}
			else if (!pad_ollie)
			{
				state = STATE_PICKTURN;
				super_state = SUPER_STATE_NORMAL_SURF;
			}
			else if ((GetTurnCycle(stick) >= PART_CENTER) || (turn_index != GetAnalogTurn()))
			{
				state = STATE_PICKCROUCHTURNCYCLE;
				BoardAnim(BOARD_ANIM_ZERO, 0.2f, false);
			}
			else
			{
				TurnType();
				if (changeTurnType)
				{
					state = STATE_PICKCROUCHTURNCYCLE;
					BoardAnim(BOARD_ANIM_ZERO, 0.2f, false);
				}
			}

			TurnDegree();
			my_board_controller.Turn(BOARD_TURN_LEFT, degree, dt);
			break;
		}

	case STATE_CROUCH_RIGHTTURN:
		{
			last_state = state;
			exit_state = true;
			my_board_controller.MoveForward(SURFER_CROUCH_ACCELERATION);

			bool pad_ollie = CtrlEvent(PAD_OLLIE);
			float vert = CtrlEvent(PAD_LSTICK_V);
			int next_air_state = -1;
			if (!pad_ollie)
			{
				// Launch up wave.
				if (my_board_controller.InLaunchRegion() && my_board_controller.InLaunchCone())
					next_air_state = STATE_LAUNCH;
				// Normal jump.
				else
					next_air_state = STATE_CHOP_HOP;
			}

			if (!pad_ollie && ((next_air_state == STATE_LAUNCH) || ((next_air_state == STATE_CHOP_HOP) && (vert > -0.95))))
			{
				state = next_air_state;
				super_state = SUPER_STATE_AIR;

				charge = GetAnimPercentage(); // PTA 4/26/01  GetBlendPercentage();
				my_board_controller.MoveForward(0.0f);
				exit_state = false;
			}
			else if (!pad_ollie)
			{
				state = STATE_PICKTURN;
				super_state = SUPER_STATE_NORMAL_SURF;
			}
			else if ((GetTurnCycle(stick) <= PART_CENTER) || (turn_index != GetAnalogTurn()))
			{
				state = STATE_PICKCROUCHTURNCYCLE;
				BoardAnim(BOARD_ANIM_ZERO, 0.2f, false);
			}
			else
			{
				TurnType();
				if (changeTurnType)
				{
					state = STATE_PICKTURN;
					BoardAnim(BOARD_ANIM_ZERO, 0.2f, false);
					super_state = SUPER_STATE_NORMAL_SURF;
				}
			}

			TurnDegree();
			my_board_controller.Turn(BOARD_TURN_RIGHT, degree, dt);
			break;
		}
	}
}


bool kellyslater_controller::DoFaceTrick()
{
	float face_z = WAVE_GetMarker(WAVE_MarkerFaceTrickZ)->z;
	int cur_region = my_board_controller.GetWaveRegion();
	bool trick_region = (face_z < my_board_controller.rb->pos.z) &&
						((cur_region == WAVE_REGIONFACE) || (cur_region == WAVE_REGIONLIP) ||
						(cur_region == WAVE_REGIONLIP2) || (cur_region == WAVE_REGIONCHIN) ||
						(cur_region == WAVE_REGIONPOCKET) || (cur_region == WAVE_REGIONTUBE) ||
						(cur_region == WAVE_REGIONWOBBLE) || (cur_region == WAVE_REGIONWASH) ||
						((cur_region == WAVE_REGIONTUBE) && Z_Within_Tube()));

	if ((state == STATE_FLAT_TAKEOFF_WOBBLE) || (state == STATE_LIETOSTAND))
		trick_region = false;

	if (trick_queued && (newTrick == TRICK_SNAP) && (state != STATE_LIETOSTAND))
	{
		trick_queued = false;
		if (trick_region)
		{
			perform_snap = true;
			state = STATE_PICKSNAPTURNCYCLE;
			return true;
		}
		else
			bSpecialTrick = false;
	}
	else if (trick_queued && (newTrick == TRICK_REVERT_CUTBACK) && (state != STATE_LIETOSTAND))
	{
		trick_queued = false;
		if (trick_region)
		{
			perform_snap = true;
			state = STATE_PICKSNAPTURNCYCLE;
			return true;
		}
		else
			bSpecialTrick = false;
	}
	else if (trick_queued && (newTrick == TRICK_TAIL_CHUCK) && (state != STATE_LIETOSTAND))
	{
		trick_queued = false;
		if (trick_region)
		{
			perform_snap = true;
			state = STATE_PICKSNAPTURNCYCLE;
			return true;
		}
		else
			bSpecialTrick = false;
	}
	else if (trick_queued && (newTrick == TRICK_GOUGE) && (state != STATE_LIETOSTAND))
	{
		trick_queued = false;
		if (trick_region)
		{
			perform_snap = true;
			state = STATE_PICKSNAPTURNCYCLE;
			return true;
		}
		else
			bSpecialTrick = false;
	}
	else if (trick_queued && (newTrick == TRICK_LAYBACK_SLIDE) && (state != STATE_LIETOSTAND))
	{
		trick_queued = false;
		if (trick_region)
		{
			perform_snap = true;
			state = STATE_PICKSNAPTURNCYCLE;
			return true;
		}
		else
			bSpecialTrick = false;
	}
	else if (trick_queued && ((newTrick == TRICK_FACE_SHOVEIT) || (newTrick == TRICK_HANGTEN)
				|| (newTrick == TRICK_DARKSLIDE) || (newTrick == TRICK_CHEATERS5) || (newTrick == TRICK_SITTER)
				|| (newTrick == TRICK_MANUAL) || (newTrick == TRICK_HEADSTAND) || (newTrick == TRICK_CRUZER)))
	{
		trick_queued = false;
		state = STATE_FACE_SPECIAL_TRICK;
		return true;
	}

	return false;
}

bool player_relative = true;
float BIG_TURN_THRESH = 0.4f;
float big_turn_amount = 0.02f;
float SMALL_TURN_THRESH = 0.15f;
float small_turn_amount = 0.01f;
float float_turn_mag = 420.0f;
float start_tube_threshhold = 1.0f;
float float_start = 2.5f;
float float_start_vel = -1.1f;
float back_force = 0.2f;
float bg_meter_mult = 0.375f;
float bg_meter_mod = 0.05f;
float min_bg_meter = 0.3f;
float g_time_rem = 2.5f;
float out_of_tube_fudge = 5.0f;

void kellyslater_controller::SuperStateNormalSurf(float dt)
{
	SSEventId gruntEvent = -1;
//	input_mgr*	inputmgr = input_mgr::inst();
	int			cycle;
//	float		charge;
	int current_beach = g_game_ptr->get_beach_id ();
//	int current_region = my_board_controller.GetWaveRegion();

	SetTrickRegion(TREGION_FACE);

	if (last_super_state != super_state)
	{
		my_carveManager.SetInternalVars();
		if (last_state != STATE_LANDING)
		{
			my_trickManager.ClearTrickManager();
			bSpecialTrick = false;
			trick_queued = false;
		}

		out_of_tube = true;
		last_was_main_tube = 1;
		last_tube_dist = 1.0f;  //  Show that the surfer was not just inside a tube.

		press_up_time = 10.0f;	//  Just some large number to show that pressing down is allowed.

		if ((state != STATE_FLOAT) && (state != STATE_GRIND))
			my_board_controller.SetBoardState(BOARD_NONE);

		num_wipeouts = 0;
		no_buttons_pressed = false;
		end_level = false;
/*		if (BeachDataArray[current_beach].is_big_wave && !test_big_wave)
		{
			if (state != STATE_BIGWAVESTAND)
			{
				big_wave_meter.End();
				//  Temporarilly commented out while this is used for the special meter timer.  frontendmanager.IGO->TurnOnTubeTimer(my_player_num, false);
				my_board_controller.DoWipeOut(WIP_LOW_LIE_FOR); //  hit back
				ks_fx_reset();  //  reset the wave particles.

				//  Get rid of all the objects that have been spawned
				// This is a duplication of code in beach.cpp and should be combined (dc 02/13/02)
				beach_object *one = g_beach_ptr->my_objects;
				while (one)
				{
					one->despawn();
					one = one->next;
				}
			}
		}*/
	}

	my_carveManager.FrameAdvance(dt, false);

	vector3d float_pos = my_board_controller.GetFloatPos();
	vector3d lip_norm = my_board_controller.GetLipNormal();
	//my_board_controller.GoneOverLip(dt);
	IK_enabled = true;
	my_board_controller.grind_ollie = false;
	my_board_controller.grind_jump = false;
	my_board_controller.roof_jump = false;
	last_super_state = super_state;
	bool in_float_region = false;
	bool pad_oll = CtrlEvent(PAD_OLLIE);
	bool pad_slide = CtrlEvent(PAD_SLIDE);
	bool slide_delta = CtrlDelta(PAD_SLIDE);
	bool pad_grab = CtrlEvent(PAD_GRAB);
	bool in_grind_region = my_board_controller.InGrindRegion();
	//bool is_grinding_object = my_board_controller.IsGrindingObject();
	bool vel_up_wave = (dot(my_board_controller.GetVelocity(), lip_norm) > float_start_vel);
	bool doing_snap = ((state == STATE_RIGHTSNAP180) || (state == STATE_LEFTSNAP180));

	if (slide_delta && !pad_slide)
	{
		int wave_region = my_board_controller.GetWaveRegion();
		vector3d z_vec = lip_norm - dot(my_board_controller.GetUpDir(), lip_norm)*my_board_controller.GetUpDir();
		z_vec.normalize();
//		float test = dot(my_board->get_abs_position() - float_pos, z_vec);
		in_float_region = (dot(my_board->get_abs_position() - float_pos, z_vec) > -float_start) &&
			 ((wave_region == WAVE_REGIONLIP) || (wave_region == WAVE_REGIONLIP2) || (wave_region == WAVE_REGIONCHIN)
			 || (wave_region == WAVE_REGIONFACE) || (wave_region == WAVE_REGIONBACK));
	}

	//if (((state == STATE_GRIND) || (state == STATE_FLOAT)) && !in_grind_region && !is_grinding_object)
	if ((state == STATE_FLOAT) && !in_grind_region)
	{
		if (state == STATE_FLOAT)
		{
			state = STATE_EXIT_FLOAT;
			my_board_controller.AddVelocity(-float_dismount_vel*ZVEC);
		}
		else
			state = STATE_STAND;

		my_board_controller.SetBoardState(BOARD_NONE);
		float_trick = -1;
	}
	/*else if (((float_region_timer > 0.3f) && (fabs(CtrlEvent(PAD_LSTICK_H)) < 0.6f) && (fabs(CtrlEvent(PAD_LSTICK_V)) < 0.6f)
				&& in_float_region && in_grind_region && !pad_slide && !pad_grab && !pad_oll && !doing_snap) ||
		is_grinding_object)*/
	else if (vel_up_wave && in_float_region && in_grind_region && !pad_slide && !pad_grab && !pad_oll && !bSpecialTrick && slide_delta
		&& (state != STATE_LIETOSTAND) && (state != STATE_ON_ROOF))
	{
		state = STATE_FLOAT;
		if (my_board_controller.GetBoardState() != BOARD_FLOAT)
		{
			float_trick = -1;
			StartFloat(XVEC);
			my_trickManager.ClearTrickManager();
			my_board_controller.ResetFloatSpeed(false);
			float_balance = 0.0f;
		}

		my_board_controller.SetBoardState(BOARD_FLOAT);
	}
	else if (!doing_snap && in_float_region && in_grind_region && !pad_slide && !pad_grab && !pad_oll)
		float_region_timer += dt;
	else
		float_region_timer = 0.0f;

	this->DoFaceTrick();

	if (!bSpecialTrick && trick_queued && (newTrick == TRICK_SUPER_STALL))
	{
		if (state == STATE_STALL)  //  otherwise the state is super_stall and this shouldn't be done again.
		{
			SetCompletedTrick(TRICK_SUPER_STALL);
			trick_queued = false;
			state = STATE_SUPER_STALL;
		}
	}

	//  Detect whether or not the surfer just finished passing through a little tube.
	vector3d current_tube_pos;
	int main_tube;
	float tube_dist = Closest_Tube_Distance(&main_tube, &current_tube_pos);
	vector3d current_pos = my_board->get_abs_position ();
	vector3d lip_pos = *WAVE_GetMarker(WAVE_MarkerLipCrash);
	if (tube_dist > 0)	//  not inside the tube.
	{
		if (last_tube_dist <= 0 && last_was_main_tube != 1)  //  Did we just come out of the tube?
		{

			if (((current_tube_pos.x + 1 > current_pos.x && last_tube_pos.x + 1 < current_pos.x) ||  //  Is the surfer on the other side of the tube now?
				(current_tube_pos.x - 1 < current_pos.x && last_tube_pos.x - 1 > current_pos.x)) &&
				!out_of_tube)
			{
				if (floater_gap)
					my_scoreManager.AddGap(GAP_TUBE_FLOATER_ANTARCTICA + current_beach);  //  Floatered over the tube.
				else
				{
					SoundScriptManager::inst()->playEvent(SS_SHOOT_BREAK);
					my_scoreManager.AddGap(GAP_LITTLE_TUBE_ANTARCTICA + current_beach);		//  Surfed through the tube.
				}
			}
		}

		if (current_pos.z < lip_pos.z - out_of_tube_fudge)
			out_of_tube = true;
		else
			out_of_tube = false;

		last_was_main_tube = main_tube;
		last_tube_pos = current_tube_pos;
		floater_gap = false;
	}
	else  //  While in the tube area, make sure that the surfer did not bust through the wall to or from the outside.
	{
		//  Check to see if at some point while inside the tube, this became the little tube.
		if (!main_tube)
			last_was_main_tube = main_tube;

		if (current_pos.z < lip_pos.z - out_of_tube_fudge)	//  Let them go a tiny bit outside the tube, but not much
			out_of_tube = true;

		//  Check to see if the surfer is in a floater while this is happening.
		if (state == STATE_FLOAT)
			floater_gap = true;
	}

	last_tube_dist = tube_dist;

	//  see if roof shoulf throw surfer off
	CheckForRoof();

	int region = my_board_controller.GetWaveRegion();
	if ((my_board_controller.GetUpDir().y < 0.0f) && (my_board_controller.GetWaveNormal().y < 0.0f))
	{
		if ((region == WAVE_REGIONCEILING) || (region == WAVE_REGIONMAX) || (region == WAVE_REGIONTUBE) || (region == (WAVE_REGIONMAX + 1)))
		{
			my_board_controller.DoWipeOut(my_board_controller.CalculateCeilingWipeout());
		}
		else if ((state != STATE_FLOAT) && (state != STATE_ON_ROOF) && (state != STATE_LIETOSTAND)
			&& !doing_snap)
		{
			bSpecialTrick = false;
			state = STATE_CONTROLLED_JUMP;
			super_state = SUPER_STATE_AIR;
		}
	}

	if (shakeoff)
	{
		bSpecialTrick = false;
		state = STATE_SHAKEOFF;
		super_state = SUPER_STATE_NORMAL_SURF;
	}

	DetermineTubeGaps();

	float time_rem = WAVE_GetScheduleRemainingSec();
	if (time_rem < g_time_rem)
	{
		my_board_controller.DoWipeOut(WIP_TAKE_OFF_FLAT);
	}

	float vert = CtrlEvent(PAD_LSTICK_V);

	//  anytime the player has been holding up, make sure the camera won't immeditate swing around.
	if (vert < -STALL_THRESHOLD)
		press_up_time = 0.0f;
	else
		press_up_time += dt;

	//CheckSnap(dt);
	switch (state)
	{

//--------------------------------------------------
//               *****  STANDS  *******
//--------------------------------------------------

	case STATE_LIETOSTAND:

		//WAVE_GlobalCurrent (&my_board_controller.rb->linMom);
		if (state != last_state)
		{
			//  When the surfer starts standing up, check to see if he is too far from the wave.
/*			float face_z = WAVE_GetMarker(WAVE_MarkerFaceTrickZ)->z;
			if (face_z > my_board_controller.rb->pos.z)
				takeoff_wobble = true;
			else
				takeoff_wobble = false;
*/
//			Anim(SURFER_ANIM_TRANS_LIE2STAND, BLEND_TIME, false);
			SoundScriptManager::inst()->playEvent(SS_STAND_UP, my_board_model);
			if (take_off_type >= 3)
			{
				Anim(SURFER_ANIM_TAKE_OFF_WEAK_1, BLEND_TIME, false);
				BoardAnim(BOARD_ANIM_TAKE_OFF_WEAK_1, BLEND_TIME, false);
			}
			else if (take_off_type == 2)
			{
				Anim(SURFER_ANIM_TAKE_OFF_REG_1, BLEND_TIME, false);
				BoardAnim(BOARD_ANIM_TAKE_OFF_REG_1, BLEND_TIME, false);
			}
			else
			{
				if (my_board_controller.GetForwardDir().z < 0)
				{
					Anim(SURFER_ANIM_TAKE_OFF_LATE_1, BLEND_TIME, false);
					BoardAnim(BOARD_ANIM_TAKE_OFF_LATE_1, BLEND_TIME, false);
				}
				else
				{
					Anim(SURFER_ANIM_TAKE_OFF_LATE_FK_1, BLEND_TIME, false);
					BoardAnim(BOARD_ANIM_TAKE_OFF_LATE_FK_1, BLEND_TIME, false);
				}
			}
			if (!IsAIPlayer())
				IGOStandUp();
		}

		last_state = state;
		my_board_controller.Turn(BOARD_TURN_RIGHT, 0.0, dt);

		my_board_controller.MoveForward(SURFER_ACCELERATION);

		exit_state = true;
		if (AnimComplete())//AnimEvnt(ANIM_END_EVENT))
		{
			if (takeoff_wobble && ((int)(Lip_Distance() * 3)) >= takeoff_wobble_thresh)
			{
				state = STATE_FLAT_TAKEOFF_WOBBLE;
				exit_state = false;
			}
			else
			{
				if (takeoff_wobble)	//  takeoff type has not been determined yet
				{
					DetermineTakeoffType(true);
				}

				my_trickManager.ClearTrickManager();
//				if (BeachDataArray[current_beach].is_big_wave && !test_big_wave)
//					state = STATE_BIGWAVESTAND;
//				else
					state = STATE_STAND;

				super_state = SUPER_STATE_NORMAL_SURF;
				IK_enabled = true;
				Anim(StandAnim(), BLEND_TIME);
				BoardAnim(BOARD_ANIM_ZERO, 0.0f, false);
			}
		}

		break;

	case STATE_FLAT_TAKEOFF_WOBBLE:

		if (last_state != state)
		{
			Anim (SURFER_ANIM_W_1_TAK_FLAT_WOBBLE, BLEND_TIME, false);
			BoardAnim (BOARD_ANIM_W_1_TAK_FLAT_WOBBLE, BLEND_TIME, false);
			last_state = state;
		}

		exit_state = true;
		if (AnimComplete())
		{
			my_board_controller.DoWipeOut(WIP_TAKE_OFF_FLAT);
			if (!IsAIPlayer())
				frontendmanager.IGO->GetHintManager()->SetHint(IGOHintManager::StoodTooFarOut);
			exit_state = false;
		}

		break;

    case STATE_STAND:
		{
			my_board_controller.SetTurnType(HARD_REGULAR_TURN);
			IK_enabled = true;
			if ((mAnim != SURFER_ANIM_CROUCH_SPD_1) && (vert < -STALL_THRESHOLD))
			{
				Anim(SURFER_ANIM_CROUCH_SPD_1, BLEND_TIME);
				BoardAnim(BOARD_ANIM_CROUCH_SPD_1, BLEND_TIME);
			}
			else if (((mAnim == SURFER_ANIM_CROUCH_SPD_1) && (vert >= -STALL_THRESHOLD)) || (state != last_state)
							|| ((mAnim != SURFER_ANIM_CROUCH_SPD_1) &&
							(my_board_controller.GetLastWaveRegion() != my_board_controller.GetWaveRegion())))
			{
				Anim(StandAnim(), BLEND_TIME);
				BoardAnim(BOARD_ANIM_ZERO, BLEND_TIME, false);
			}
			else if (my_board_controller.InTubeRegion() && Z_Within_Tube())
			{
				if (BeachDataArray[g_game_ptr->get_beach_id()].is_small_wave)
				{
					//  Always grab the outside rail.
					bool left_wave = BeachDataArray[g_game_ptr->get_beach_id()].bdir;
					if ((left_wave && !goofy) || (!left_wave && goofy))
					{
						if (mAnim != SURFER_ANIM_TRN_GRAB_L_1)
						{
							Anim(SURFER_ANIM_TRK_TUBE_GRAB_L_1, BLEND_TIME);
							BoardAnim(BOARD_ANIM_ZERO, BLEND_TIME, false);
						}
					}
					else
					{
						if (mAnim != SURFER_ANIM_TRK_TUBE_GRAB_R_1)
						{
							Anim(SURFER_ANIM_TRN_GRAB_R_1, BLEND_TIME);
							BoardAnim(BOARD_ANIM_ZERO, BLEND_TIME, false);
						}
					}
				}
				else if (mAnim != stand_anim_nums[5])
				{
					Anim(stand_anim_nums[5], BLEND_TIME);
					BoardAnim(BOARD_ANIM_ZERO, BLEND_TIME, false);
				}
			}

			last_state = state;

			if (CtrlEvent(PAD_CARVE))
				my_board_controller.MoveForward(SURFER_ACCELERATION);

			//  Only allow stalling if the surfer is faced away from the tube.
			bool facing_left = my_board_controller.GetForwardDir().x < 0;  
			if (((BeachDataArray[current_beach].bdir && facing_left) ||
				(!BeachDataArray[current_beach].bdir && !facing_left)) &&
				vert > STALL_THRESHOLD)
			{
				state = STATE_STALL;
			}
			else if (CtrlEvent(PAD_OLLIE))
			{
				state = STATE_CROUCH;
				super_state = SUPER_STATE_PREPARE_JUMP;
			}
			else if (perform_snap || CtrlEvent(PAD_LSTICK_H))
			{
				BoardAnim(BOARD_ANIM_ZERO, 0.175f, false);
				state = STATE_PICKTURN;
			}
			
			else
			{
//				if (!BeachDataArray[current_beach].is_big_wave || test_big_wave)
				{
/*					if (test_big_wave)
					{
						big_wave_extra_speed = max(0.0f, big_wave_extra_speed - (dt * bg_acc_mod));
						my_board_controller.MoveForward(big_wave_extra_speed);
					}
*/					my_board_controller.Turn(BOARD_TURN_RIGHT, 0.0, dt);
				}

				exit_state = true;
			}

		}
		break;
    case STATE_BIGWAVESTAND:
		{
//			float big_wave_meter_result;
//			float meter_depth = max(min_bg_meter, Lip_Distance());

			IK_enabled = true;
			if (state != last_state)
			{
				Anim(StandAnim(), BLEND_TIME);
				BoardAnim(BOARD_ANIM_ZERO, 0.0f, false);
				//big_wave_meter.Init(my_player_num, 30, 5, true);
			}

			last_state = state;

			//big_wave_meter.Resize(meter_depth);

			int direction = 0;
			stick = GetStick(PAD_LSTICK_H); //LEFT_H);

			if (stick > 0.5f)
				direction = 1;
			else if (stick < -0.5f)
				direction = -1;

			//big_wave_meter_result = big_wave_meter.Update(direction,dt);

/*
			if (fabs(big_wave_meter_result) > meter_depth)
			{
				//big_wave_meter.End();
				//  Temporarilly commented out while this is used for the special meter timer.  frontendmanager.IGO->TurnOnTubeTimer(my_player_num, false);
				my_board_controller.DoWipeOut(WIP_TAKE_OFF_FLAT);  //  hit back
				ks_fx_reset();  //  reset the wave particles.

				//  Get rid of all the objects that have been spawned
				// This is a duplication of code in beach.cpp and should be combined (dc 02/13/02)
				beach_object *one = g_beach_ptr->my_objects;
				while (one)
				{
					one->despawn();
					one = one->next;
				}
			}


			if (player_relative)
				big_wave_meter_result = -big_wave_meter_result;
*/

			//  This commented out stuff is a way of doing the meter in discrete levels of turning.
			//  Below is the analog method.
/*			if (big_wave_meter_result > BIG_TURN_THRESH)
				big_wave_meter_result = big_turn_amount;
			else if (big_wave_meter_result > SMALL_TURN_THRESH)
				big_wave_meter_result = small_turn_amount;
			else if (big_wave_meter_result < -BIG_TURN_THRESH)
				big_wave_meter_result = -big_turn_amount;
			else if (big_wave_meter_result < -SMALL_TURN_THRESH)
				big_wave_meter_result = -small_turn_amount;*/

/*
			my_board_controller.MoveForward(-back_force);
			if (big_wave_meter_result > SMALL_TURN_THRESH ||
				big_wave_meter_result < -SMALL_TURN_THRESH)
			{
				my_board_controller.MoveForward(fabs(big_wave_meter_result * bg_meter_mult));
				big_wave_meter_result *= bg_meter_mod;
			}
			else
				big_wave_meter_result = 0;

			my_board_controller.Turn(BOARD_TURN_RIGHT, big_wave_meter_result, dt);
			exit_state = true;
*/

		}
		break;
    case STATE_SHAKEOFF:
		if (last_state != state)
		{
			shakeoff = false;
			Anim(SURFER_ANIM_TRK_TUBE_BUST_1, BLEND_TIME, false);
			BoardAnim(BOARD_ANIM_ZERO, 0.0f, false);
		}

		last_state = state;

		TurnDegree();
		my_board_controller.SetTurnType(LIE_TURN);
		if (stick > 0.0f)
		{
			my_board_controller.Turn(BOARD_TURN_RIGHT, degree, dt);
		}
		else
		{
			my_board_controller.Turn(BOARD_TURN_LEFT, degree, dt);
		}

		if (AnimComplete())
		{
			state = STATE_STAND;
			last_state = state;
			super_state = SUPER_STATE_NORMAL_SURF;
			IK_enabled = true;
			Anim(StandAnim(), BLEND_TIME);
			BoardAnim(BOARD_ANIM_ZERO, 0.0f, false);
		}

		exit_state = true;
		break;
//--------------------------------------------------
//               *****  GRINDS  &  FLOATS  *******
//--------------------------------------------------

	case STATE_FLOAT:
		{
			last_state = state;
//			bool update_score = true;

			bool lstickhor_pressed = CtrlEvent(PAD_LSTICK_H);
			bool slide_held = !CtrlDelta(PAD_SLIDE) && CtrlEvent(PAD_SLIDE);
			//bool grab_held = !CtrlDelta(PAD_GRAB) && CtrlEvent(PAD_GRAB);
			bool grab_held = false;

			my_scoreManager.UpdateLastTrick(ScoringManager::ATTR_TIME_DELTA, dt);

			if (AnimComplete() && (mAnim == SURFER_ANIM_TRK_INTO_FLOAT_1))
			{
				Anim(float_anim, BLEND_TIME);
				BoardAnim(BOARD_ANIM_ZERO, BLEND_TIME, false);
			}

			// Ollie out of grind.
			if (CtrlEvent(PAD_OLLIE))
			{
				//my_board_controller.SetBoardState(BOARD_NONE);
				ResetTricks();
				state = STATE_CROUCH;
				super_state = SUPER_STATE_PREPARE_JUMP;
				break;
			}
			else if ((slide_dismount_timer > 0.15f) && (slide_held && lstickhor_pressed))
			{
				ResetTricks();
				turnType = TAILSLIDE_TURN;
				state = STATE_PICKTAILSLIDETURNCYCLE;
				my_board_controller.AddVelocity(-4.0f*ZVEC);
				break;
			}
			else if ((slide_dismount_timer > 0.15f) && (grab_held && lstickhor_pressed))
			{
				ResetTricks();
				turnType = GRAB_TURN;
				state = STATE_PICKTURN;
				my_board_controller.AddVelocity(-4.0f*ZVEC);
				break;
			}
			else if ((slide_dismount_timer > 0.15f) && lstickhor_pressed)
			{
				ResetTricks();
				state = STATE_PICKTURN;
				my_board_controller.AddVelocity(-4.0f*ZVEC);
			}
			else if (/*(slide_held || grab_held) && */lstickhor_pressed && !CtrlEvent(PAD_OLLIE))
				slide_dismount_timer += dt;
			else if (!lstickhor_pressed || !slide_held || !grab_held)
				slide_dismount_timer = 0.0f;


			bool towards_roof = (dot(my_board_controller.GetVelocity(), my_board_controller.GetLipVec()) < 0.0f);
			bool near_roof = false;
			vector3d roof_dist = *WAVE_GetMarker(WAVE_MarkerLipMark9);
			roof_dist -= my_board_controller.rb->pos;
			if (dot(roof_dist, roof_dist) < 25.0f)
				near_roof = true;

			float min_time = 0.0f;
			bool break_map = my_board_controller.TongueEffectOn();
			if (break_map)
				min_time = -5.5f;

			bool fall_from_roof = (near_roof && towards_roof);
			if (!in_grind_region || ((float_time < min_time) && !fall_from_roof))
			{
				state = STATE_EXIT_FLOAT;
				my_board_controller.AddVelocity(-float_dismount_vel*ZVEC);
				my_board_controller.SetBoardState(BOARD_NONE);
				exit_state = true;
				break;
			}

			float_time -= dt;

			if (CheckGrindPathEnd())
			{
				state = STATE_EXIT_FLOAT;
				my_board_controller.AddVelocity(-float_dismount_vel*ZVEC);
				my_board_controller.SetBoardState(BOARD_NONE);
				exit_state = true;
				break;
			}

			exit_state = true;

			//  Reorient the surfer's root to the wave surface
			//OrientToWave(true, dt, SET_RB_PO);

			break;
		}

	case STATE_EXIT_FLOAT:
		{
			if (last_state != state)
			{
				Anim(SURFER_ANIM_TRK_OUT_FLOAT_1, BLEND_TIME, false);
				vector3d spin_vec, for_dir = my_board_controller.GetForwardDir();
				if (dot(for_dir, ZVEC) > -0.5f)
				{
					if (dot(for_dir, XVEC) > 0.0f)
						spin_vec = XVEC - ZVEC;
					else
						spin_vec = -XVEC - ZVEC;

					spin_vec.normalize();
					spin_ctrl.SetSpinType(SPIN_FLOATER, 0.8f, spin_vec);
				}
			}

			last_state = state;

			if (AnimComplete())
			{
				ResetTricks();
				state = STATE_STAND;
				break;
			}

			exit_state = true;
			break;
		}

	case STATE_GRIND:
		{
			state = STATE_STAND;
			/*
			last_state = state;
			bool update_score = true;
			vector3d grind_vec;
			int new_quadrant = float_quadrant;
			if (float_trick == TRICK_FLOAT_RIGHT)
			{
				grind_vec = grind_right;
				new_quadrant = 1;
			}
			else if (float_trick == TRICK_FLOAT_LEFT)
			{
				grind_vec = -grind_right;
				new_quadrant = 3;
			}
			else if (float_trick == TRICK_FLOAT_BACK)
			{
				grind_vec = -grind_vector;
				new_quadrant = 2;
			}
			else if (float_trick == TRICK_FLOAT_FORWARD)
			{
				grind_vec = grind_vector;
				new_quadrant = 0;
			}

			if (new_quadrant != float_quadrant)
			{
				int anim;
				vector3d spin_axis = spin_ctrl.SetSpinType(SPIN_FLOATER, 0.0f, grind_vec);
				spin_ctrl.Pause();
				if (dot(spin_axis, YVEC) > 0.0f)
					anim = SURFER_ANIM_TRK_FLOAT_TL_1;
				else
					anim = SURFER_ANIM_TRK_FLOAT_TR_1;

				Anim(anim, 0.2f, false);
				BoardAnim(BOARD_ANIM_ZERO, 0.2f, false);
				float_quadrant = new_quadrant;
			}

			if (float_balance > 1.0f)
				float_balance = 1.0f;
			else if (float_balance < -1.0f)
				float_balance = -1.0f;

			SetFloatPO(DEG_TO_RAD(37.0f)*float_balance);
			if (fabs(float_balance) >= 1.0f)  //  Too far, knock the surfer over.
			{
				float val = dot(grind_right, -ZVEC);
				if (((val > 0.0f) && (float_balance > 0.0f))
					|| ((val < 0.0f) && (float_balance < 0.0f)))
				{
					float_meter.End();
					turnType = CARVE_TURN;
					state = STATE_PICKCARVETURNCYCLE;
					my_board_controller.AddVelocity(-4.0f*ZVEC);
					exit_state = true;
					break;
				}


				float_meter.End();
				my_board_controller.SetBoardState(BOARD_NONE);
				my_board_controller.DoWipeOut(WIP_LOW_STUMBLE_BACK);
				break;
			}


			if ((mAnim == SURFER_ANIM_TRK_FLOAT_TL_1) || (mAnim == SURFER_ANIM_TRK_FLOAT_TR_1) ||
				(mAnim == SURFER_ANIM_TRK_FLOAT_TLL_1) || (mAnim == SURFER_ANIM_TRK_FLOAT_TRR_1))
			{
				if (AnimComplete())
				{
					if (mAnim == (SURFER_ANIM_TRK_FLOAT_TL_1))
					{
						spin_ctrl.Continue();
						Anim(SURFER_ANIM_TRK_FLOAT_TLL_1, 0.2f, false);
						BoardAnim(BOARD_ANIM_ZERO, 0.2f, false);
					}
					else if (mAnim == (SURFER_ANIM_TRK_FLOAT_TR_1))
					{
						Anim(SURFER_ANIM_TRK_FLOAT_TRR_1, 0.2f, false);
						BoardAnim(BOARD_ANIM_ZERO, 0.2f, false);
						spin_ctrl.Continue();
					}
					else if ((mAnim == SURFER_ANIM_TRK_FLOAT_TLL_1) || (mAnim == SURFER_ANIM_TRK_FLOAT_TRR_1))
					{
						if ((float_trick == TRICK_FLOAT_LEFT) && goofy)
							float_anim = SURFER_ANIM_TRK_FLOAT_R_C_1;
						else if ((float_trick == TRICK_FLOAT_RIGHT) && goofy)
							float_anim = SURFER_ANIM_TRK_FLOAT_L_C_1;

						update_score = false;
						SetCompletedTrick(float_trick);
						Anim(float_anim, BLEND_TIME);
						//BoardAnim(float_board_anim, BLEND_TIME);
						//float_trick = -1;
					}
				}
			}
			else
			{
				if ((float_trick == TRICK_FLOAT_LEFT) && goofy)
					float_anim = SURFER_ANIM_TRK_FLOAT_R_C_1;
				else if ((float_trick == TRICK_FLOAT_RIGHT) && goofy)
					float_anim = SURFER_ANIM_TRK_FLOAT_L_C_1;

				int offset = (int) (37.0f*float_balance/(74.0f/11.0f));
				if (offset > 5)
					offset = 5;
				else if (offset < -5)
					offset = -5;

				offset += float_anim;
				if (offset != mAnim)
					Anim(offset, BLEND_TIME);
			}

			if ((mAnim > (float_anim - 5)) && (mAnim < (float_anim + 5)))// && update_score)
				my_scoreManager.UpdateLastTrick(ScoringManager::ATTR_TIME_DELTA, dt);

			// Ollie out of grind.
			if (CtrlEvent(PAD_OLLIE))
			{
				//my_board_controller.SetBoardState(BOARD_NONE);
				state = STATE_CROUCH;
				super_state = SUPER_STATE_PREPARE_JUMP;
				break;
			}
			else if ((slide_dismount_timer > 0.15f) && (!CtrlDelta(PAD_SLIDE) && CtrlEvent(PAD_LSTICK_H) && CtrlEvent(PAD_SLIDE)))
			{
				turnType = TAILSLIDE_TURN;
				state = STATE_PICKTAILSLIDETURNCYCLE;
				my_board_controller.AddVelocity(-4.0f*ZVEC);
				float_meter.End();
				break;
			}
			else if (!CtrlDelta(PAD_SLIDE) && CtrlEvent(PAD_LSTICK_H) && CtrlEvent(PAD_SLIDE))
				slide_dismount_timer += dt;
			else if (!CtrlEvent(PAD_LSTICK_H))
				slide_dismount_timer = 0.0f;

			if (CheckGrindPathEnd())
			{
				float_meter.End();
				state = STATE_STAND;
				break;
			}

			exit_state = true; */
			break;
		}

//--------------------------------------------------
//               *****  ON ROOF  *******
//--------------------------------------------------

	case STATE_ON_ROOF:
		{
			int current_beach = g_game_ptr->get_beach_id ();
			if (state != last_state)
			{
				my_scoreManager.AddGap(GAP_ROOF_LAND_ANTARCTICA + current_beach);
				Anim(SURFER_ANIM_FREEFALL, BLEND_TIME);
				BoardAnim(BOARD_ANIM_BAS_FREEFALL, BLEND_TIME);
				my_board_controller.Turn(BOARD_TURN_RIGHT, 0.0f, dt);
				my_board_controller.SetBoardState(BOARD_NONE);
				spin_ctrl.SetSpinType(SPIN_FLOATER, 1.0f, -ZVEC, 1.5f);
				tween_timer = 0.0f;
				tween_time = 1.0f;
			}
			last_state = state;

			vector3d normal = my_board_controller.GetWaveNormal();
			int region = my_board_controller.GetWaveRegion();
			if ((dot(normal, -ZVEC) > 0.65f) || (region == WAVE_REGIONCURL) || (region == WAVE_REGIONMAX)
				|| (region == (WAVE_REGIONMAX + 1)))
			{
				state = STATE_LAUNCH;
				super_state = SUPER_STATE_AIR;

				float sc = 1.0f;
				if (BeachDataArray[current_beach].bdir)
					sc = -1.0f;

				my_board_controller.roof_jump = true;
				my_board_controller.Jump();
			}


			exit_state = true;

			//  Reorient the surfer's root to the wave surface
			OrientToWave(true, dt, SET_RB_PO | SET_BOARD_PO);
			my_board->set_rel_position (my_board->get_rel_position());
		}
		break;


//--------------------------------------------------
//               *****  GRABS  *******
//--------------------------------------------------

/*
		case STATE_GRABRAIL:
			if (CtrlEvent(PAD_GRAB)!=AXIS_MAX) state = STATE_GRABRAILTOSTAND;
			else if (CtrlEvent(PAD_LSTICK_H)!=AXIS_MID)	state = STATE_TURN;
			{
				if (waveBank >= 0.0)
				{
					Anim(SURFER_ANIM_GRABRIGHTRAIL);
				}
				else
				{
					Anim(SURFER_ANIM_GRABRIGHTRAIL);
				}
				exit_state = true;
			}
			break;

		case STATE_GRABRAILTOSTAND:
			if (CtrlEvent(PAD_LSTICK_H)!=AXIS_MID)	state = STATE_TURN;
			else
			{
				if (waveBank >= 0.0)
					Anim(SURFER_ANIM_GRABRIGHTRAILTOSTAND);
				else
					Anim(SURFER_ANIM_GRABLEFTRAILTOSTAND);
				state = STATE_STAND;
			}
			break;

		case STATE_GRABRAILINTUBE:
			if (CtrlEvent(PAD_GRAB)!=AXIS_MAX) state = STATE_STAND;
			else if (CtrlEvent(PAD_SPEED_PUMP)==AXIS_MAX) state = STATE_SPEEDPUMP;
			else if (waveRegion!=WAVE_REGION_TUBE) state = STATE_STANDTOGRABRAIL;	//(WaveRegion() != TUBE)
			else
			{
				if (waveBank >= 0.0)
					Anim(SURFER_ANIM_SPEEDPUMP);	// right rail is out of water
				else
					Anim(SURFER_ANIM_SPEEDPUMP);	// left rail is out of water
				exit_state = true;
			}
			break;
*/
//--------------------------------------------------
//               *****  STALLS *******
//--------------------------------------------------

	case STATE_SUPER_STALL:
		if (last_state != state)
		{
			Anim(SURFER_ANIM_SUPER_STALL_1, BLEND_TIME);
			BoardAnim(BOARD_ANIM_SUPER_STALL_1, BLEND_TIME);
			if (stallEvent == -1)
				stallEvent = SoundScriptManager::inst()->startEvent(SS_STALL, my_board_model);
		}
		my_scoreManager.UpdateLastTrick(ScoringManager::ATTR_TIME_DELTA, dt);  //  superstall is the same as stall except for this.

	case STATE_STALL:
		//  If in the tube and facing forwards (not looking into the tube) start tube controls.
		if (region == WAVE_REGIONTUBE &&
			my_board->get_abs_position ().z > lip_pos.z &&
			((my_board_controller.GetForwardDir().x >= 0 && !BeachDataArray[g_game_ptr->get_beach_id()].bdir) ||
			(my_board_controller.GetForwardDir().x <= 0 && BeachDataArray[g_game_ptr->get_beach_id()].bdir)))
			StartTube();

		my_board_controller.SetTurnType(TRIM_TURN);

		if (last_state != state && state == STATE_STALL)  //  Don't change anims for super stall.
		{
			Anim(SURFER_ANIM_STALL, BLEND_TIME);
			BoardAnim(BOARD_ANIM_STALL, BLEND_TIME);
			if (stallEvent == -1)
				stallEvent = SoundScriptManager::inst()->startEvent(SS_STALL, my_board_model);
		}

		last_state = state;

		if (!(CtrlEvent(PAD_LSTICK_V) > STALL_THRESHOLD))
			state = STATE_STAND;
		else if (CtrlEvent(PAD_LSTICK_H))	// allow move to turn
		{
			TurnDegree();
			my_board_controller.SetTurnType(REGULAR_TURN);
			if (stick > 0.0)
				my_board_controller.Turn(BOARD_TURN_RIGHT, degree, dt);
			else
				my_board_controller.Turn(BOARD_TURN_LEFT, degree, dt);
		}

		exit_state = true;
		break;
/*
	    if (CtrlEvent(PAD_LSTICK_H)!=AXIS_MID) state = STATE_TURN;
      else
      {

  	    SetAccel(-0.5);

	      Anim(SURFER_ANIM_SPEEDPUMP);
	      state = STATE_STAND;
				exit_state = true;
      }
			break;
*/

//--------------------------------------------------
//               *****  TURNS  *******
//  Picking them.
//--------------------------------------------------

	case STATE_PICKTURN:

		TurnType();

		last_state = state;
	    if (turnType == GRAB_TURN)
		    state = STATE_PICKGRABTURNCYCLE;
		else if (turnType == GRABSLIDE_TURN)
			state = STATE_PICKGRABSLIDETURNCYCLE;
	    else if (turnType == TAILSLIDE_TURN)
		    state = STATE_PICKTAILSLIDETURNCYCLE;
		else if (turnType == CROUCH_TURN)
		{
		    state = STATE_PICKCROUCHTURNCYCLE;
			super_state = SUPER_STATE_PREPARE_JUMP;
		}
	    else if (turnType == REGULAR_TURN)
		    state = STATE_PICKTURNCYCLE;
	    else if (turnType == HARD_REGULAR_TURN)
		    state = STATE_PICKHARDTURNCYCLE;
		else if (turnType == CARVE_TURN)
		    state = STATE_PICKCARVETURNCYCLE;
	    else
		    state = STATE_PICKSNAPTURNCYCLE;

		break;


	case STATE_PICKGRABTURNCYCLE:
		last_state = state;
		if (!CtrlEvent(PAD_LSTICK_H))	state = STATE_STAND;
		else
		{
			my_board_controller.SetTurnType(GRAB_TURN);
			stick = GetStick(PAD_LSTICK_H);

			cycle = GetTurnCycle(stick);
			stick = GetStick(PAD_LSTICK_H);

			cycle = GetTurnCycle(stick);
			turn_index = GetAnalogTurn();
			int anim_n;
			if (turn_index < 0)
				anim_n = SURFER_ANIM_TRN_GRAB_L_1 - (turn_index + 1) + goofy_int*9;
			else
				anim_n = SURFER_ANIM_TRN_GRAB_R_1 + (turn_index - 1) - goofy_int*9;

			if (cycle == PART_LEFT2 || cycle == PART_LEFT1 || cycle == PART_LEFT0)
			{
				Anim(anim_n, 0.175f);
				BoardAnim(BOARD_ANIM_ZERO, 0.175f, false);
				state = STATE_LEFTTURN;
			}
			else
			{
				Anim(anim_n, 0.175f);
				BoardAnim(BOARD_ANIM_ZERO, 0.175f, false);
				state = STATE_RIGHTTURN;
			}
		}
		break;

	case STATE_PICKGRABSLIDETURNCYCLE:
		last_state = state;
		if (!CtrlEvent(PAD_LSTICK_H))	state = STATE_STAND;
		else
		{
			my_board_controller.SetTurnType(GRABSLIDE_TURN);
			stick = GetStick(PAD_LSTICK_H);

			cycle = GetTurnCycle(stick);
			if (cycle == PART_LEFT2 || cycle == PART_LEFT1 || cycle == PART_LEFT0)
			{
				Anim(SURFER_ANIM_LEFTGRABTURN - goofy_int, 0.175f);
				BoardAnim(BOARD_ANIM_ZERO, 0.175f, false);
				state = STATE_GRABSLIDE_LEFTTURN;
			}
			else
			{
				Anim(SURFER_ANIM_RIGHTGRABTURN + goofy_int, 0.175f);
				BoardAnim(BOARD_ANIM_ZERO, 0.175f, false);
				state = STATE_GRABSLIDE_RIGHTTURN;
			}
		}
		break;

	case STATE_PICKTAILSLIDETURNCYCLE:

		last_state = state;
		if (!CtrlEvent(PAD_LSTICK_H))	state = STATE_STAND;
		else
		{
			my_board_controller.SetTurnType(TAILSLIDE_TURN);
			stick = GetStick(PAD_LSTICK_H);

			cycle = GetTurnCycle(stick);
			bool small_wave = (BeachDataArray[g_game_ptr->get_beach_id()].is_small_wave
				&& my_board_controller.InTubeRegion() && Z_Within_Tube());
			if (cycle == PART_LEFT2 || cycle == PART_LEFT1 || cycle == PART_LEFT0)
			{
				Anim(small_wave?STATE_GRABSLIDE_LEFTTURN:SURFER_ANIM_TRN_L_TAILSLIDE_1 + goofy_int, 0.175f, false);
				if (!small_wave) BoardAnim(BOARD_ANIM_TRN_L_TAILSLIDE_1 + goofy_int, 0.175f, false);
				state = STATE_TAILSLIDE_LEFTTURN;
			}
			else
			{
				Anim(small_wave?STATE_GRABSLIDE_RIGHTTURN:SURFER_ANIM_TRN_R_TAILSLIDE_1 - goofy_int, 0.175f, false);
				if (!small_wave) BoardAnim(BOARD_ANIM_TRN_R_TAILSLIDE_1 - goofy_int, 0.175f, false);
				state = STATE_TAILSLIDE_RIGHTTURN;
			}
		}
		break;

	case STATE_TAILSLIDE_RIGHTTURN:

		{
			if (last_state != state)
			{
				spinEvent = SoundScriptManager::inst()->startEvent(SS_SPIN, my_board_model, .5);
		
				trick_added = false;
				my_board_controller.ResetTurnAmounts();
			}

			last_state = state;
			float tamount = my_board_controller.GetAbsTurnAmount();
			if (CtrlEvent(PAD_OLLIE))
			{
				state = STATE_CROUCH_RIGHTTURN;
				super_state = SUPER_STATE_PREPARE_JUMP;
			}
			else if (tamount > 4.6f*PI)
			{
				my_board_controller.DoWipeOut(goofy?WIP_SLIDE_LOSE_BALANCE_LEFT:WIP_SLIDE_LOSE_BALANCE_RIGHT); //  hit right
				if (!IsAIPlayer())
					frontendmanager.IGO->GetHintManager()->SetHint(IGOHintManager::SpunTooMuch);
				exit_state = false;
				break;
			}
			else if (GetTurnCycle(stick) <= PART_CENTER)
				state = STATE_PICKTURN;
			else
			{
				TurnType();
				if (changeTurnType)
					state = STATE_PICKTURN;
			}

			if (AnimComplete())
			{
				if (mAnim == (SURFER_ANIM_TRN_R_TAILSLIDE_1 - goofy_int))
					Anim(SURFER_ANIM_TRN_R_TAILSLIDE_2 - goofy_int, BLEND_TIME);
			}

			if (tamount > 0.33333f*PI)
			{
				if (!trick_added)
				{
					trick_added = true;
					SetCompletedTrick(TRICK_TAILSLIDE);
				}
				else if (trick_added)
				{
					int num_turns = (int) (tamount/PI);
					my_scoreManager.UpdateLastTrick(ScoringManager::ATTR_NUM_SPINS_TOTAL, num_turns);
				}
			}

			TurnDegree();
			my_board_controller.Turn(BOARD_TURN_RIGHT, degree, dt);
			exit_state = true;
			break;
		}

	case STATE_TAILSLIDE_LEFTTURN:
		{
			if (last_state != state)
			{
				spinEvent = SoundScriptManager::inst()->startEvent(SS_SPIN, my_board_model, .5);
				trick_added = false;
				my_board_controller.ResetTurnAmounts();
			}

			last_state = state;
			float tamount = my_board_controller.GetAbsTurnAmount();
			if (CtrlEvent(PAD_OLLIE))
			{
				state = STATE_CROUCH_LEFTTURN;
				super_state = SUPER_STATE_PREPARE_JUMP;
			}
			else if (tamount > 4.6f*PI)
			{
				my_board_controller.DoWipeOut(goofy?WIP_SLIDE_LOSE_BALANCE_RIGHT:WIP_SLIDE_LOSE_BALANCE_LEFT);  //  hit left
				if (!IsAIPlayer())
					frontendmanager.IGO->GetHintManager()->SetHint(IGOHintManager::SpunTooMuch);
				exit_state = false;
				break;
			}
			else if (GetTurnCycle(stick) >= PART_CENTER)
				state = STATE_PICKTURN;
			else
			{
				TurnType();
				if (changeTurnType)
					state = STATE_PICKTURN;
			}

			if (AnimComplete())
			{
				if (mAnim == (SURFER_ANIM_TRN_L_TAILSLIDE_1 + goofy_int))
					Anim(SURFER_ANIM_TRN_L_TAILSLIDE_2 + goofy_int, BLEND_TIME);
			}

			if (tamount > 0.33333f*PI)
			{
				if (!trick_added)
				{
					trick_added = true;
					SetCompletedTrick(TRICK_TAILSLIDE);
				}
				else if (trick_added)
				{
					int num_turns = (int) (tamount/PI);
					my_scoreManager.UpdateLastTrick(ScoringManager::ATTR_NUM_SPINS_TOTAL, num_turns);
				}
			}

			TurnDegree();
			my_board_controller.Turn(BOARD_TURN_LEFT, degree, dt);
			exit_state = true;
			break;
		}

	case STATE_GRABSLIDE_RIGHTTURN:
		{
			if (last_state != state)
			{
				trick_added = false;
				my_board_controller.ResetTurnAmounts();
			}

			last_state = state;
			float tamount = my_board_controller.GetAbsTurnAmount();
			if (CtrlEvent(PAD_OLLIE))
			{
				state = STATE_CROUCH_RIGHTTURN;
				super_state = SUPER_STATE_PREPARE_JUMP;
			}
			else if (my_board_controller.GetAbsTurnAmount() > 4.6f*PI)
			{
				my_board_controller.DoWipeOut(goofy?WIP_SLIDE_LOSE_BALANCE_LEFT:WIP_SLIDE_LOSE_BALANCE_RIGHT);  //  hit right
				if (!IsAIPlayer())
					frontendmanager.IGO->GetHintManager()->SetHint(IGOHintManager::SpunTooMuch);
				exit_state = false;
				break;
			}
			else if (GetTurnCycle(stick) <= PART_CENTER)
				state = STATE_PICKTURN;
			else
			{
				TurnType();
				if (changeTurnType)	state = STATE_PICKTURN;
			}

			/*if (AnimComplete())
			{
				if (mAnim == (SURFER_ANIM_TRN_R_TAILSLIDE_1 - goofy_int))
					Anim(SURFER_ANIM_TRN_R_TAILSLIDE_2 - goofy_int, BLEND_TIME);
			}*/

			if (tamount > 0.33333f*PI)
			{
				if (!trick_added)
				{
					trick_added = true;
					SetCompletedTrick(TRICK_POWERSLIDE);
				}
				else if (trick_added)
				{
					int num_turns = (int) (tamount/PI);
					my_scoreManager.UpdateLastTrick(ScoringManager::ATTR_NUM_SPINS_TOTAL, num_turns);
				}
			}

			TurnDegree();
			my_board_controller.Turn(BOARD_TURN_RIGHT, degree, dt);
			exit_state = true;
			break;
		}

	case STATE_GRABSLIDE_LEFTTURN:
		{
			if (last_state != state)
			{
				trick_added = false;
				my_board_controller.ResetTurnAmounts();
			}

			last_state = state;
			float tamount = my_board_controller.GetAbsTurnAmount();
			if (CtrlEvent(PAD_OLLIE))
			{
				state = STATE_CROUCH_LEFTTURN;
				super_state = SUPER_STATE_PREPARE_JUMP;
			}
			else if (my_board_controller.GetAbsTurnAmount() > 4.6f*PI)
			{
				my_board_controller.DoWipeOut(goofy?WIP_SLIDE_LOSE_BALANCE_RIGHT:WIP_SLIDE_LOSE_BALANCE_LEFT);  //  hit left
				if (!IsAIPlayer())
					frontendmanager.IGO->GetHintManager()->SetHint(IGOHintManager::SpunTooMuch);
				exit_state = false;
				break;
			}
			else if (GetTurnCycle(stick) >= PART_CENTER)
				state = STATE_PICKTURN;
			else
			{
				TurnType();
				if (changeTurnType)	state = STATE_PICKTURN;
			}

			/*if (AnimComplete())
			{
				if (mAnim == (SURFER_ANIM_TRN_L_TAILSLIDE_1 + goofy_int))
					Anim(SURFER_ANIM_TRN_L_TAILSLIDE_2 + goofy_int, BLEND_TIME);
			}*/

			if (tamount > 0.33333f*PI)
			{
				if (!trick_added)
				{
					trick_added = true;
					SetCompletedTrick(TRICK_POWERSLIDE);
				}
				else if (trick_added)
				{
					int num_turns = (int) (tamount/PI);
					my_scoreManager.UpdateLastTrick(ScoringManager::ATTR_NUM_SPINS_TOTAL, num_turns);
				}
			}

			TurnDegree();
			my_board_controller.Turn(BOARD_TURN_LEFT, degree, dt);
			exit_state = true;
			break;
		}

	case STATE_PICKHARDTAILSLIDETURNCYCLE:
		last_state = state;
	    if (!CtrlEvent(PAD_LSTICK_H))	state = STATE_STAND;

	    my_board_controller.SetTurnType(HARD_TAILSLIDE_TURN);

	    stick = GetStick(PAD_LSTICK_H); //LEFT_H);

	    cycle = GetTurnCycle(stick);
	    if (cycle == PART_LEFT2)
	    {
		    //Anim(SURFER_ANIM_LEFTTAILSLIDETURN + goofy_int, BLEND_TIME);
		    state = STATE_LEFTTURNPART2;
	    }
	    else if (cycle == PART_LEFT1)
	    {
		    //Anim(SURFER_ANIM_LEFTTAILSLIDETURN + goofy_int, BLEND_TIME);
		    state = STATE_LEFTTURNPART1;
	    }
	    else if (cycle == PART_LEFT0)
	    {
		    //Anim(SURFER_ANIM_LEFTTAILSLIDETURN + goofy_int, BLEND_TIME);
		    state = STATE_LEFTTURNPART0;
	    }
	    else if (cycle == PART_RIGHT0)
	    {
		    //Anim(SURFER_ANIM_RIGHTTAILSLIDETURN - goofy_int, BLEND_TIME);
		    state = STATE_RIGHTTURNPART0;
	    }
	    else if (cycle == PART_RIGHT1)
	    {
		    //Anim(SURFER_ANIM_RIGHTTAILSLIDETURN - goofy_int, BLEND_TIME);
		    state = STATE_RIGHTTURNPART1;
	    }
	    else
	    {
		    //Anim(SURFER_ANIM_RIGHTTAILSLIDETURN - goofy_int, BLEND_TIME);
		    state = STATE_RIGHTTURNPART2;
	    }
		break;

	case STATE_PICKTURNCYCLE: //  unused state
		last_state = state;
		if (!CtrlEvent(PAD_LSTICK_H))	state = STATE_STAND;
		else
		{

			my_board_controller.SetTurnType(REGULAR_TURN);

			stick = GetStick(PAD_LSTICK_H);

			cycle = GetTurnCycle(stick);
			if (cycle == PART_LEFT2 || cycle == PART_LEFT1 || cycle == PART_LEFT0)
			{
				//Anim(SURFER_ANIM_LEFTTURN + goofy_int, BLEND_TIME);
				state = STATE_LEFTTURN;
			}
			else
			{
				//Anim(SURFER_ANIM_RIGHTTURN - goofy_int, BLEND_TIME);
				state = STATE_RIGHTTURN;
			}
		}
		break;

	case STATE_PICKHARDTURNCYCLE:
		last_state = state;
		if (!CtrlEvent(PAD_LSTICK_H))	state = STATE_STAND;
		else
		{

			my_board_controller.SetTurnType(HARD_REGULAR_TURN);
			stick = GetStick(PAD_LSTICK_H);

			cycle = GetTurnCycle(stick);
			turn_index = GetAnalogTurn();
			int anim_n;
			if (my_board_controller.GetWaveRegion() == WAVE_REGIONWOBBLE)  //  Special bottom turns for the wake area.
			{
				if (turn_index < 0)
					anim_n = turn_anim_nums[0] + 18 - (turn_index + 1) + goofy_int*9;
				else
					anim_n = turn_anim_nums[9] + 18 + (turn_index - 1) - goofy_int*9;
			}
			else if (BeachDataArray[g_game_ptr->get_beach_id()].is_small_wave
						&& my_board_controller.InTubeRegion() && Z_Within_Tube())
			{
				if (turn_index < 0)
					anim_n = SURFER_ANIM_TRN_GRAB_L_1 - (turn_index + 1) + goofy_int*9;
				else
					anim_n = SURFER_ANIM_TRN_GRAB_R_1 + (turn_index - 1) - goofy_int*9;
			}
			else
			{
				if (turn_index < 0)
					anim_n = turn_anim_nums[0] - (turn_index + 1) + goofy_int*9;
				else
					anim_n = turn_anim_nums[9] + (turn_index - 1) - goofy_int*9;
			}

			if (cycle == PART_LEFT2 || cycle == PART_LEFT1 || cycle == PART_LEFT0)
			{
				Anim(anim_n, 0.175f);
				BoardAnim(BOARD_ANIM_ZERO, 0.175f, false);
				state = STATE_LEFTTURN;
			}
			else
			{
				Anim(anim_n, 0.175f);
				BoardAnim(BOARD_ANIM_ZERO, 0.175f, false);
				state = STATE_RIGHTTURN;
			}
		}
		break;

	case STATE_PICKCARVETURNCYCLE:
		last_state = state;
		if (!CtrlEvent(PAD_LSTICK_H))	state = STATE_STAND;
		else
		{

			/*if (my_board_controller.GetTurnType() != CARVE_TURN)
			{
				SetCompletedTrick(TRICK_CARVE);
			}*/

			my_board_controller.SetTurnType(CARVE_TURN);
			stick = GetStick(PAD_LSTICK_H);

			cycle = GetTurnCycle(stick);
			stick = GetStick(PAD_LSTICK_H);

			cycle = GetTurnCycle(stick);
			turn_index = GetAnalogTurn();
			int anim_n;
			if (BeachDataArray[g_game_ptr->get_beach_id()].is_small_wave
				 && Z_Within_Tube() && my_board_controller.InTubeRegion())
			{
				if (turn_index < 0)
					anim_n = SURFER_ANIM_TRN_GRAB_L_1 - (turn_index + 1) + goofy_int*9;
				else
					anim_n = SURFER_ANIM_TRN_GRAB_R_1 + (turn_index - 1) - goofy_int*9;
			}
			else
			{
				if (turn_index < 0)
					anim_n = carve_anim_nums[0] - (turn_index + 1) + goofy_int*9;
				else
					anim_n = carve_anim_nums[9] + (turn_index - 1) - goofy_int*9;
			}

			if (cycle == PART_LEFT2 || cycle == PART_LEFT1 || cycle == PART_LEFT0)
			{
				Anim(anim_n, 0.175f);
				BoardAnim(BOARD_ANIM_ZERO, 0.175f, false);
				state = STATE_LEFTTURN;
			}
			else
			{
				Anim(anim_n, 0.175f);
				BoardAnim(BOARD_ANIM_ZERO, 0.175f, false);
				state = STATE_RIGHTTURN;
			}
		}
		break;

	case STATE_PICKSNAPTURNCYCLE:
		last_state = state;

	    //if (!CtrlEvent(PAD_LSTICK_H))	state = STATE_STAND;

	    stick = GetStick(PAD_LSTICK_H); //LEFT_H);

		my_board_controller.InitSnap();
	    if (stick > 0.0f)
	    {
		    if (newTrick == TRICK_REVERT_CUTBACK)
				state = STATE_RIGHTREVERTCUTBACK;
			else if (newTrick == TRICK_TAIL_CHUCK)
				state = STATE_RIGHT_TAIL_CHUCK;
			else if (newTrick == TRICK_GOUGE)
				state = STATE_RIGHT_GOUGE;
			else if (newTrick == TRICK_LAYBACK_SLIDE)
				state = STATE_RIGHT_LAYBACK_SLIDE;
			else
			{
				if (my_board_controller.InSnapRegion() && my_board_controller.InSnapCone())
					state = STATE_RIGHTSNAP180;
				else
					state = STATE_RIGHT_REBOUND;
			}
	    }
	    else if (stick < 0.0f)
	    {
			if (newTrick == TRICK_REVERT_CUTBACK)
				state = STATE_LEFTREVERTCUTBACK;
			else if (newTrick == TRICK_TAIL_CHUCK)
				state = STATE_LEFT_TAIL_CHUCK;
			else if (newTrick == TRICK_GOUGE)
				state = STATE_LEFT_GOUGE;
			else if (newTrick == TRICK_LAYBACK_SLIDE)
				state = STATE_LEFT_LAYBACK_SLIDE;
			else
			{
				if (my_board_controller.InSnapRegion() && my_board_controller.InSnapCone())
					state = STATE_LEFTSNAP180;
				else
					state = STATE_LEFT_REBOUND;
			}
	    }
		else
		{
			vector3d rdir = my_board_controller.GetRightDir();
			vector3d away_from_beach = my_board_controller.GetLipNormal();
			if (dot(rdir, away_from_beach) > 0.0f)
			{
				if (newTrick == TRICK_REVERT_CUTBACK)
					state = STATE_LEFTREVERTCUTBACK;
				else if (newTrick == TRICK_TAIL_CHUCK)
					state = STATE_LEFT_TAIL_CHUCK;
				else if (newTrick == TRICK_GOUGE)
					state = STATE_LEFT_GOUGE;
				else if (newTrick == TRICK_LAYBACK_SLIDE)
					state = STATE_LEFT_LAYBACK_SLIDE;
				else
				{
					if (my_board_controller.InSnapRegion() && my_board_controller.InSnapCone())
						state = STATE_LEFTSNAP180;
					else
						state = STATE_LEFT_REBOUND;
				}
			}
			else
			{
				if (newTrick == TRICK_REVERT_CUTBACK)
					state = STATE_RIGHTREVERTCUTBACK;
				else if (newTrick == TRICK_TAIL_CHUCK)
					state = STATE_RIGHT_TAIL_CHUCK;
				else if (newTrick == TRICK_GOUGE)
					state = STATE_RIGHT_GOUGE;
				else if (newTrick == TRICK_LAYBACK_SLIDE)
					state = STATE_RIGHT_LAYBACK_SLIDE;
				else
				{
					if (my_board_controller.InSnapRegion() && my_board_controller.InSnapCone())
						state = STATE_RIGHTSNAP180;
					else
						state = STATE_RIGHT_REBOUND;
				}
			}

		}

    if ((state == STATE_LEFTSNAP180) || (state == STATE_RIGHTSNAP180))
      ks_fx_create_snap_splash (((conglomerate*) owner)->get_member("BIP01 R FOOT")->get_abs_position(), GetBoard()->get_abs_po().get_z_facing(), get_board_controller().wave_center_hint, state == STATE_LEFTSNAP180, my_player_num);
    else
    {
      // add extra_splash_power to the trail
    }

		break;


//--------------------------------------------------
//               *****  TURNS  *******
//  Actually performing them.
//--------------------------------------------------


	case STATE_RIGHTREVERTCUTBACK:

		exit_state = false;
		bDoingTrick = true;
		if (last_state != state)
		{
			SoundScriptManager::inst()->playEvent(SS_FAST_TURN, my_board_model);
			if (random(100) < GRUNT_PROB * 100)
				if (g_game_ptr->GetSurferIdx(my_player_num) != SURFER_LISA_ANDERSEN)
					gruntEvent = SoundScriptManager::inst()->playEvent(SS_MALE_GRUNT); 
				else
					gruntEvent = SoundScriptManager::inst()->playEvent(SS_FEMALE_GRUNT); 
			my_board_controller.SetTurnType(REVERT_CUTBACK_TURN);
			Anim(SURFER_ANIM_TRK_CUTBACK_R_2 - goofy_int, 0.1f, false);
			BoardAnim(BOARD_ANIM_TRK_CUTBACK_R_2 - goofy_int, 0.1f, false);
			force_dir = my_board_controller.GetForwardDir();
			my_trail->create_face_trick_splash (false);
			initial_face_trick_region = my_board_controller.GetWaveRegion();
			SetCompletedTrick(TRICK_REVERT_CUTBACK);
			SetCurrentTrick();
		}

		last_state = state;

	    if (!AnimComplete())  //while (AnimEvnt(~ANIM_END_EVENT))
	    {
			if (dot(force_dir, my_board_controller.GetLipVec()) > 0.866f)
				my_board_controller.SetControllerForce(26.0f*force_dir);

		    my_board_controller.Turn(BOARD_TURN_RIGHT, 1.0, dt);
			exit_state = true;
	    }
		else
		{
			my_board_controller.ResetTurnAmounts();
			bSpecialTrick = false;
			ResetTricks();
			if (initial_face_trick_region != WAVE_REGIONTUBE &&
				my_board_controller.GetWaveRegion() == WAVE_REGIONTUBE)
				my_scoreManager.AddGap(GAP_INSANE_TUBE_ENTRY);
			if (dot(my_board_controller.GetVelocity(), my_board_controller.GetForwardDir()) < 0.0f)
				state = STATE_CUTBACK_END;
			else
				state = STATE_STAND;
		}
		break;

	case STATE_RIGHT_REBOUND:

		exit_state = false;
		bDoingTrick = true;
		if (last_state != state)
		{
			SoundScriptManager::inst()->playEvent(SS_FAST_TURN, my_board_model);
			if (random(100) < GRUNT_PROB * 100)
				if (g_game_ptr->GetSurferIdx(my_player_num) != SURFER_LISA_ANDERSEN)
					gruntEvent= SoundScriptManager::inst()->playEvent(SS_MALE_GRUNT); 
				else
					gruntEvent= SoundScriptManager::inst()->playEvent(SS_FEMALE_GRUNT); 
			my_board_controller.SetTurnType(REBOUND_TURN);
			Anim(SURFER_ANIM_TRK_CUTBACK_R_1 - goofy_int, 0.1f, false);
			BoardAnim(BOARD_ANIM_TRK_CUTBACK_R_1 - goofy_int, 0.1f, false);
			force_dir = my_board_controller.GetForwardDir();
			my_trail->create_face_trick_splash (false);
			initial_face_trick_region = my_board_controller.GetWaveRegion();
			SetCompletedTrick(TRICK_REBOUND);
			SetCurrentTrick();
		}

		last_state = state;

	    if (!AnimComplete())  //while (AnimEvnt(~ANIM_END_EVENT))
	    {
			if (dot(force_dir, my_board_controller.GetLipVec()) > 0.866f)
			{
				my_board_controller.SetControllerForce(-15.0f*force_dir);
				my_board_controller.AddControllerForce(5.0f*my_board_controller.GetForwardDir());
			}
		    my_board_controller.Turn(BOARD_TURN_RIGHT, 1.0, dt);
			exit_state = true;
	    }
		else
		{
			my_board_controller.ResetTurnAmounts();
			bSpecialTrick = false;
			ResetTricks();
			if (initial_face_trick_region != WAVE_REGIONTUBE &&
				my_board_controller.GetWaveRegion() == WAVE_REGIONTUBE)
				my_scoreManager.AddGap(GAP_INSANE_TUBE_ENTRY);
			state = STATE_REBOUND_END;
		}
		break;

	case STATE_RIGHT_TAIL_CHUCK:

		exit_state = false;
		bDoingTrick = true;
		if (last_state != state)
		{
			SoundScriptManager::inst()->playEvent(SS_FAST_TURN, my_board_model);
			if (random(100) < GRUNT_PROB * 100)
				if (g_game_ptr->GetSurferIdx(my_player_num) != SURFER_LISA_ANDERSEN)
					gruntEvent= SoundScriptManager::inst()->playEvent(SS_MALE_GRUNT); 
				else
					gruntEvent= SoundScriptManager::inst()->playEvent(SS_FEMALE_GRUNT); 
			my_board_controller.SetTurnType(TAILCHUCK_TURN);
			Anim(SURFER_ANIM_TRK_CUTBACK_R_3 - goofy_int, 0.1f, false);
			BoardAnim(BOARD_ANIM_TRK_CUTBACK_R_3 - goofy_int, 0.1f, false);
			force_dir = my_board_controller.GetForwardDir();
			my_trail->create_face_trick_splash (false);
			initial_face_trick_region = my_board_controller.GetWaveRegion();
			SetCompletedTrick(TRICK_TAIL_CHUCK);
			SetCurrentTrick();
		}

		last_state = state;

	    if (!AnimComplete())  //while (AnimEvnt(~ANIM_END_EVENT))
	    {
			my_board_controller.rb->linMom = ZEROVEC;
		    my_board_controller.Turn(BOARD_TURN_RIGHT, 1.0, dt);
			exit_state = true;
	    }
		else
		{
			my_board_controller.ResetTurnAmounts();
			bSpecialTrick = false;
			ResetTricks();
			if (initial_face_trick_region != WAVE_REGIONTUBE &&
				my_board_controller.GetWaveRegion() == WAVE_REGIONTUBE)
				my_scoreManager.AddGap(GAP_INSANE_TUBE_ENTRY);
			state = STATE_TAIL_CHUCK_END;
		}
		break;

	case STATE_RIGHT_GOUGE:

		exit_state = false;
		bDoingTrick = true;
		if (last_state != state)
		{
			SoundScriptManager::inst()->playEvent(SS_FAST_DOUBLE_TURN, my_board_model);
			if (random(100) < GRUNT_PROB * 100)
				if (g_game_ptr->GetSurferIdx(my_player_num) != SURFER_LISA_ANDERSEN)
					gruntEvent= SoundScriptManager::inst()->playEvent(SS_MALE_GRUNT); 
				else
					gruntEvent= SoundScriptManager::inst()->playEvent(SS_FEMALE_GRUNT); 

			dummy_state = last_state;
			my_board_controller.SetTurnType(GOUGE_TURN);
			Anim(SURFER_ANIM_TRK_CUTBACK_R_4 - goofy_int, 0.1f, false);
			BoardAnim(BOARD_ANIM_TRK_CUTBACK_R_4 - goofy_int, 0.1f, false);
			force_dir = my_board_controller.GetForwardDir();
			my_trail->create_face_trick_splash (false);
			initial_face_trick_region = my_board_controller.GetWaveRegion();
			SetCompletedTrick(TRICK_GOUGE);
			SetCurrentTrick();
		}

		last_state = state;

	    if (!AnimComplete())  //while (AnimEvnt(~ANIM_END_EVENT))
	    {
			if (mAnim == (SURFER_ANIM_TRK_CUTBACK_R_4 - goofy_int))
			{
				my_board_controller.MoveForward(1.5f*SURFER_CARVETURN_ACCELERATION);
				my_board_controller.Turn(BOARD_TURN_RIGHT, 1.0, dt);
			}
			else
			{
				my_board_controller.MoveForward(4.0f*SURFER_CARVETURN_ACCELERATION);
				my_board_controller.Turn(BOARD_TURN_LEFT, 1.0, dt);
			}

			exit_state = true;
	    }
		else
		{
			if (mAnim == (SURFER_ANIM_TRK_CUTBACK_R_4 - goofy_int))
			{
				Anim(SURFER_ANIM_TRK_CUTBACK_R_41 - goofy_int, 0.1f, false);
				BoardAnim(BOARD_ANIM_TRK_CUTBACK_R_41 - goofy_int, 0.1f, false);
				exit_state = true;
			}
			else
			{
				my_board_controller.ResetTurnAmounts();
				ResetTricks();
				if (initial_face_trick_region != WAVE_REGIONTUBE &&
					my_board_controller.GetWaveRegion() == WAVE_REGIONTUBE)
					my_scoreManager.AddGap(GAP_INSANE_TUBE_ENTRY);
				bSpecialTrick = false;
				state = STATE_STAND;
			}
		}
		break;

	case STATE_RIGHT_LAYBACK_SLIDE:

		exit_state = false;
		bDoingTrick = true;
		if (last_state != state)
		{
			SoundScriptManager::inst()->playEvent(SS_FAST_TURN, my_board_model);
			if (random(100) < GRUNT_PROB * 100)
				if (g_game_ptr->GetSurferIdx(my_player_num) != SURFER_LISA_ANDERSEN)
					gruntEvent= SoundScriptManager::inst()->playEvent(SS_MALE_GRUNT); 
				else
					gruntEvent= SoundScriptManager::inst()->playEvent(SS_FEMALE_GRUNT); 
			my_board_controller.SetTurnType(LAYBACK_SLIDE_TURN);
			Anim(SURFER_ANIM_TRK_CUTBACK_R_5 - goofy_int, 0.1f, false);
			BoardAnim(BOARD_ANIM_TRK_CUTBACK_R_5 - goofy_int, 0.1f, false);
			force_dir = my_board_controller.GetForwardDir();
			my_trail->create_face_trick_splash (false);
			initial_face_trick_region = my_board_controller.GetWaveRegion();
			SetCompletedTrick(TRICK_LAYBACK_SLIDE);
			SetCurrentTrick();
		}

		last_state = state;

	    if (!AnimComplete())  //while (AnimEvnt(~ANIM_END_EVENT))
	    {
			my_board_controller.rb->linMom = -6.0f*ZVEC;
		    my_board_controller.Turn(BOARD_TURN_RIGHT, 1.0, dt);
			exit_state = true;
	    }
		else
		{
			my_board_controller.ResetTurnAmounts();
			my_board_controller.rb->linMom = -10.0f*ZVEC;
			bSpecialTrick = false;
			ResetTricks();
			if (initial_face_trick_region != WAVE_REGIONTUBE &&
				my_board_controller.GetWaveRegion() == WAVE_REGIONTUBE)
				my_scoreManager.AddGap(GAP_INSANE_TUBE_ENTRY);
			state = STATE_LAYBACK_SLIDE_END;
		}
		break;

    case STATE_RIGHTSNAP180:

		exit_state = false;
		if (last_state != state)
		{
			SoundScriptManager::inst()->playEvent(SS_FAST_TURN, my_board_model);
			if (random(100) < GRUNT_PROB * 100)
				if (g_game_ptr->GetSurferIdx(my_player_num) != SURFER_LISA_ANDERSEN)
					gruntEvent= SoundScriptManager::inst()->playEvent(SS_MALE_GRUNT); 
				else
					gruntEvent= SoundScriptManager::inst()->playEvent(SS_FEMALE_GRUNT); 
			Anim(SURFER_ANIM_TRN_SNAP_R_1 - goofy_int, 0.2f, false);
			BoardAnim(BOARD_ANIM_TRN_SNAP_R_1 - goofy_int, 0.2f, false);
			my_board_controller.SetTurnType(SNAP_TURN);
			initial_face_trick_region = my_board_controller.GetWaveRegion();
			SetCompletedTrick(TRICK_SNAP);
			SetCurrentTrick();
		}

		last_state = state;

	    if (AnimComplete())//AnimEvnt(ANIM_END_EVENT))
		{
			my_board_controller.ResetTurnAmounts();
			state = STATE_STAND;
			bSpecialTrick = false;
			ResetTricks();
			if (initial_face_trick_region != WAVE_REGIONTUBE &&
				my_board_controller.GetWaveRegion() == WAVE_REGIONTUBE)
				my_scoreManager.AddGap(GAP_INSANE_TUBE_ENTRY);
			my_board_controller.rb->linMom += 5.0f*my_board_controller.GetForwardDir();
			my_board_controller.SetTurnType(HARD_REGULAR_TURN);
		}
		else
	    {
		    /*if (!CtrlEvent(PAD_SNAP))
		    {
			    if (GetAnimPercentage() < 0.25)
				    state = STATE_RIGHTSNAP45;
			    else if (GetAnimPercentage() < 0.50)
				    state = STATE_RIGHTSNAP90;
		    }*/
		    my_board_controller.Turn(BOARD_TURN_RIGHT, 1.0, dt);
			exit_state = true;
	    }
		break;

    case STATE_RIGHTSNAP45:
			if (last_state != state)
			{
				SoundScriptManager::inst()->playEvent(SS_FAST_TURN, my_board_model);
				if (random(100) < GRUNT_PROB * 100)
					if (g_game_ptr->GetSurferIdx(my_player_num) != SURFER_LISA_ANDERSEN)
						gruntEvent= SoundScriptManager::inst()->playEvent(SS_MALE_GRUNT); 
					else
						gruntEvent= SoundScriptManager::inst()->playEvent(SS_FEMALE_GRUNT); 
			}
		last_state = state;
	    if (GetAnimPercentage() < 0.25)
	    {
		    my_board_controller.Turn(BOARD_TURN_RIGHT, 1.0, dt);
	    }
		else
		{
			state = STATE_STAND;
		}
		exit_state = true;
		break;

    case STATE_RIGHTSNAP90:
			if (last_state != state)
			{
				SoundScriptManager::inst()->playEvent(SS_FAST_TURN, my_board_model);
				if (random(100) < GRUNT_PROB * 100)
					if (g_game_ptr->GetSurferIdx(my_player_num) != SURFER_LISA_ANDERSEN)
						gruntEvent= SoundScriptManager::inst()->playEvent(SS_MALE_GRUNT); 
					else
						gruntEvent= SoundScriptManager::inst()->playEvent(SS_FEMALE_GRUNT); 
			}
		last_state = state;
	    if (GetAnimPercentage() < 0.50)
	    {
		    my_board_controller.Turn(BOARD_TURN_RIGHT, 1.0, dt);
	    }
		else
		{
			state = STATE_STAND;
		}
		exit_state = true;
		break;

	case STATE_LEFTREVERTCUTBACK:


		exit_state = false;
		bDoingTrick = true;
		if (last_state != state)
		{
			SoundScriptManager::inst()->playEvent(SS_FAST_TURN, my_board_model);
			if (random(100) < GRUNT_PROB * 100)
				if (g_game_ptr->GetSurferIdx(my_player_num) != SURFER_LISA_ANDERSEN)
					gruntEvent= SoundScriptManager::inst()->playEvent(SS_MALE_GRUNT); 
				else
					gruntEvent= SoundScriptManager::inst()->playEvent(SS_FEMALE_GRUNT); 
			my_board_controller.SetTurnType(REVERT_CUTBACK_TURN);
			Anim(SURFER_ANIM_TRK_CUTBACK_L_2 + goofy_int, 0.1f, false);
			BoardAnim(BOARD_ANIM_TRK_CUTBACK_L_2 + goofy_int, 0.1f, false);
			force_dir = my_board_controller.GetForwardDir();
			my_trail->create_face_trick_splash (true);
			initial_face_trick_region = my_board_controller.GetWaveRegion();
			SetCompletedTrick(TRICK_REVERT_CUTBACK);
			SetCurrentTrick();
		}

		last_state = state;

	    if (!AnimComplete())  //while (AnimEvnt(~ANIM_END_EVENT))
	    {
			if (dot(force_dir, my_board_controller.GetLipVec()) > 0.866f)
				my_board_controller.SetControllerForce(26.0f*force_dir);

		    my_board_controller.Turn(BOARD_TURN_LEFT, 1.0, dt);
			exit_state = true;
	    }
		else
		{
			my_board_controller.ResetTurnAmounts();
			bSpecialTrick = false;
			ResetTricks();
			if (initial_face_trick_region != WAVE_REGIONTUBE &&
				my_board_controller.GetWaveRegion() == WAVE_REGIONTUBE)
				my_scoreManager.AddGap(GAP_INSANE_TUBE_ENTRY);
			if (dot(my_board_controller.GetVelocity(), my_board_controller.GetForwardDir()) < 0.0f)
				state = STATE_CUTBACK_END;
			else
				state = STATE_STAND;
		}
		break;

	case STATE_LEFT_REBOUND:

		exit_state = false;
		bDoingTrick = true;
		if (last_state != state)
		{
			SoundScriptManager::inst()->playEvent(SS_FAST_TURN, my_board_model);
			if (random(100) < GRUNT_PROB * 100)
				if (g_game_ptr->GetSurferIdx(my_player_num) != SURFER_LISA_ANDERSEN)
					gruntEvent= SoundScriptManager::inst()->playEvent(SS_MALE_GRUNT); 
				else
					gruntEvent= SoundScriptManager::inst()->playEvent(SS_FEMALE_GRUNT); 
			my_board_controller.SetTurnType(REBOUND_TURN);
			Anim(SURFER_ANIM_TRK_CUTBACK_L_1 + goofy_int, 0.1f, false);
			BoardAnim(BOARD_ANIM_TRK_CUTBACK_L_1 + goofy_int, 0.1f, false);
			force_dir = my_board_controller.GetForwardDir();
			my_trail->create_face_trick_splash (true);
			initial_face_trick_region = my_board_controller.GetWaveRegion();
			SetCompletedTrick(TRICK_REBOUND);
			SetCurrentTrick();
		}

		last_state = state;

	    if (!AnimComplete())  //while (AnimEvnt(~ANIM_END_EVENT))
	    {
			if (dot(force_dir, my_board_controller.GetLipVec()) > 0.866f)
			{
				my_board_controller.SetControllerForce(-15.0f*force_dir);
				my_board_controller.AddControllerForce(5.0f*my_board_controller.GetForwardDir());
			}
		    my_board_controller.Turn(BOARD_TURN_LEFT, 1.0, dt);
			exit_state = true;
	    }
		else
		{
			my_board_controller.ResetTurnAmounts();
			bSpecialTrick = false;
			ResetTricks();
			if (initial_face_trick_region != WAVE_REGIONTUBE &&
				my_board_controller.GetWaveRegion() == WAVE_REGIONTUBE)
				my_scoreManager.AddGap(GAP_INSANE_TUBE_ENTRY);
			state = STATE_REBOUND_END;
		}
		break;

	case STATE_LEFT_TAIL_CHUCK:

		exit_state = false;
		bDoingTrick = true;
		if (last_state != state)
		{
			SoundScriptManager::inst()->playEvent(SS_FAST_TURN, my_board_model);
			if (random(100) < GRUNT_PROB * 100)
				if (g_game_ptr->GetSurferIdx(my_player_num) != SURFER_LISA_ANDERSEN)
					gruntEvent= SoundScriptManager::inst()->playEvent(SS_MALE_GRUNT); 
				else
					gruntEvent= SoundScriptManager::inst()->playEvent(SS_FEMALE_GRUNT); 
			my_board_controller.SetTurnType(TAILCHUCK_TURN);
			Anim(SURFER_ANIM_TRK_CUTBACK_L_3 + goofy_int, 0.1f, false);
			BoardAnim(BOARD_ANIM_TRK_CUTBACK_L_3 + goofy_int, 0.1f, false);
			force_dir = my_board_controller.GetForwardDir();
			my_trail->create_face_trick_splash (true);
			initial_face_trick_region = my_board_controller.GetWaveRegion();
			SetCompletedTrick(TRICK_TAIL_CHUCK);
			SetCurrentTrick();
		}

		last_state = state;

	    if (!AnimComplete())  //while (AnimEvnt(~ANIM_END_EVENT))
	    {
			my_board_controller.rb->linMom = ZEROVEC;
		    my_board_controller.Turn(BOARD_TURN_LEFT, 1.0, dt);
			exit_state = true;
	    }
		else
		{
			my_board_controller.ResetTurnAmounts();
			bSpecialTrick = false;
			ResetTricks();
			if (initial_face_trick_region != WAVE_REGIONTUBE &&
				my_board_controller.GetWaveRegion() == WAVE_REGIONTUBE)
				my_scoreManager.AddGap(GAP_INSANE_TUBE_ENTRY);
			state = STATE_TAIL_CHUCK_END;
		}
		break;

	case STATE_LEFT_GOUGE:

		exit_state = false;
		bDoingTrick = true;
		if (last_state != state)
		{
			SoundScriptManager::inst()->playEvent(SS_FAST_DOUBLE_TURN, my_board_model);
			if (random(100) < GRUNT_PROB * 100)
				if (g_game_ptr->GetSurferIdx(my_player_num) != SURFER_LISA_ANDERSEN)
					gruntEvent= SoundScriptManager::inst()->playEvent(SS_MALE_GRUNT); 
				else
					gruntEvent= SoundScriptManager::inst()->playEvent(SS_FEMALE_GRUNT); 
			dummy_state = last_state;
			my_board_controller.SetTurnType(GOUGE_TURN);
			Anim(SURFER_ANIM_TRK_CUTBACK_L_4 + goofy_int, 0.1f, false);
			BoardAnim(BOARD_ANIM_TRK_CUTBACK_L_4 + goofy_int, 0.1f, false);
			force_dir = my_board_controller.GetForwardDir();
			my_trail->create_face_trick_splash (true);
			initial_face_trick_region = my_board_controller.GetWaveRegion();
			SetCompletedTrick(TRICK_GOUGE);
			SetCurrentTrick();
		}

		last_state = state;

	    if (!AnimComplete())  //while (AnimEvnt(~ANIM_END_EVENT))
	    {
			my_board_controller.MoveForward(1.5f*SURFER_CARVETURN_ACCELERATION);
			if (mAnim == (SURFER_ANIM_TRK_CUTBACK_L_4 + goofy_int))
				my_board_controller.Turn(BOARD_TURN_LEFT, 1.0, dt);
			else
				my_board_controller.Turn(BOARD_TURN_RIGHT, 1.0, dt);

			exit_state = true;
	    }
		else
		{
			if (mAnim == (SURFER_ANIM_TRK_CUTBACK_L_4 + goofy_int))
			{
				Anim(SURFER_ANIM_TRK_CUTBACK_L_41 + goofy_int, 0.1f, false);
				BoardAnim(BOARD_ANIM_TRK_CUTBACK_L_41 + goofy_int, 0.1f, false);
				exit_state = true;
			}
			else
			{
				my_board_controller.ResetTurnAmounts();
				ResetTricks();
				if (initial_face_trick_region != WAVE_REGIONTUBE &&
					my_board_controller.GetWaveRegion() == WAVE_REGIONTUBE)
					my_scoreManager.AddGap(GAP_INSANE_TUBE_ENTRY);
				bSpecialTrick = false;
				state = STATE_STAND;
			}
		}
		break;

	case STATE_LEFT_LAYBACK_SLIDE:

		exit_state = false;
		bDoingTrick = true;
		if (last_state != state)
		{
			SoundScriptManager::inst()->playEvent(SS_FAST_TURN, my_board_model);
			if (random(100) < GRUNT_PROB * 100)
				if (g_game_ptr->GetSurferIdx(my_player_num) != SURFER_LISA_ANDERSEN)
					gruntEvent= SoundScriptManager::inst()->playEvent(SS_MALE_GRUNT); 
				else
					gruntEvent= SoundScriptManager::inst()->playEvent(SS_FEMALE_GRUNT); 
			my_board_controller.SetTurnType(LAYBACK_SLIDE_TURN);
			Anim(SURFER_ANIM_TRK_CUTBACK_L_5 + goofy_int, 0.1f, false);
			BoardAnim(BOARD_ANIM_TRK_CUTBACK_L_5 + goofy_int, 0.1f, false);
			force_dir = my_board_controller.GetForwardDir();
			my_trail->create_face_trick_splash (true);
			initial_face_trick_region = my_board_controller.GetWaveRegion();
			SetCompletedTrick(TRICK_LAYBACK_SLIDE);
			SetCurrentTrick();
		}

		last_state = state;

	    if (!AnimComplete())  //while (AnimEvnt(~ANIM_END_EVENT))
	    {
			my_board_controller.rb->linMom = -6.0f*ZVEC;
		    my_board_controller.Turn(BOARD_TURN_LEFT, 1.0, dt);
			exit_state = true;
	    }
		else
		{
			my_board_controller.ResetTurnAmounts();
			bSpecialTrick = false;
			my_board_controller.rb->linMom = -10.0f*ZVEC;
			ResetTricks();
			if (initial_face_trick_region != WAVE_REGIONTUBE &&
				my_board_controller.GetWaveRegion() == WAVE_REGIONTUBE)
				my_scoreManager.AddGap(GAP_INSANE_TUBE_ENTRY);
			state = STATE_LAYBACK_SLIDE_END;
		}
		break;

    case STATE_LEFTSNAP180:

		exit_state = false;
		if (last_state != state)
		{
			SoundScriptManager::inst()->playEvent(SS_FAST_TURN, my_board_model);
			if (random(100) < GRUNT_PROB * 100)
				if (g_game_ptr->GetSurferIdx(my_player_num) != SURFER_LISA_ANDERSEN)
					gruntEvent= SoundScriptManager::inst()->playEvent(SS_MALE_GRUNT); 
				else
					gruntEvent= SoundScriptManager::inst()->playEvent(SS_FEMALE_GRUNT); 
			my_board_controller.SetTurnType(SNAP_TURN);
			Anim(SURFER_ANIM_TRN_SNAP_L_1 + goofy_int, 0.2f, false);
			BoardAnim(BOARD_ANIM_TRN_SNAP_L_1 + goofy_int, 0.2f, false);
			initial_face_trick_region = my_board_controller.GetWaveRegion();
			SetCompletedTrick(TRICK_SNAP);
			SetCurrentTrick();
		}

		last_state = state;

	    if (!AnimComplete())  //while (AnimEvnt(~ANIM_END_EVENT))
	    {
		    /*if (!CtrlEvent(PAD_SNAP))
		    {
			    if (GetAnimPercentage() < 0.25)
				    state = STATE_LEFTSNAP45;
			    else if (GetAnimPercentage() < 0.50)
				    state = STATE_LEFTSNAP90;
		    }*/
		    my_board_controller.Turn(BOARD_TURN_LEFT, 1.0, dt);
			exit_state = true;
	    }
		else
		{
			my_board_controller.ResetTurnAmounts();
			bSpecialTrick = false;
			ResetTricks();
			if (initial_face_trick_region != WAVE_REGIONTUBE &&
				my_board_controller.GetWaveRegion() == WAVE_REGIONTUBE)
				my_scoreManager.AddGap(GAP_INSANE_TUBE_ENTRY);
			my_board_controller.rb->linMom += 5.0f*my_board_controller.GetForwardDir();
			state = STATE_STAND;
			my_board_controller.SetTurnType(HARD_REGULAR_TURN);
		}
		break;

    case STATE_LEFTSNAP45:
			if (last_state != state)
			{
				SoundScriptManager::inst()->playEvent(SS_FAST_TURN, my_board_model);
				if (random(100) < GRUNT_PROB * 100)
					if (g_game_ptr->GetSurferIdx(my_player_num) != SURFER_LISA_ANDERSEN)
						gruntEvent= SoundScriptManager::inst()->playEvent(SS_MALE_GRUNT); 
					else
						gruntEvent= SoundScriptManager::inst()->playEvent(SS_FEMALE_GRUNT); 
			}
		last_state = state;
	    if(GetAnimPercentage() < 0.25)
	    {
		    my_board_controller.Turn(BOARD_TURN_LEFT, 1.0, dt);
	    }
		else
		{
			state = STATE_STAND;
		}
		exit_state = true;
		break;

    case STATE_LEFTSNAP90:
			if (last_state != state)
			{
				SoundScriptManager::inst()->playEvent(SS_FAST_TURN, my_board_model);
				if (random(100) < GRUNT_PROB * 100)
					if (g_game_ptr->GetSurferIdx(my_player_num) != SURFER_LISA_ANDERSEN)
						gruntEvent= SoundScriptManager::inst()->playEvent(SS_MALE_GRUNT); 
					else
						gruntEvent= SoundScriptManager::inst()->playEvent(SS_FEMALE_GRUNT); 
			}
		last_state = state;
	    if (GetAnimPercentage() < 0.50)
	    {
		    my_board_controller.Turn(BOARD_TURN_LEFT, 1.0, dt);
	    }
		else
		{
			state = STATE_STAND;
		}
		exit_state = true;
		break;


    case STATE_RIGHTTURN:
		last_state = state;
		if (CtrlEvent(PAD_OLLIE))
		{
			state = STATE_CROUCH_RIGHTTURN;
			super_state = SUPER_STATE_PREPARE_JUMP;
		}
		else if ((turnType == GRAB_TURN) && !CtrlEvent(PAD_GRAB))
		{
			turnType = REGULAR_TURN;
			state = STATE_PICKTURN;
		}
	    else if ((GetTurnCycle(stick) <= PART_CENTER) || (turn_index != GetAnalogTurn()))
			state = STATE_PICKTURN;
		else
		{
			TurnType();
			if (test_big_wave)
			{
				big_wave_extra_speed = SURFER_BIG_WAVE_ACCELERATION;
				my_board_controller.MoveForward(SURFER_BIG_WAVE_ACCELERATION);
			}
			else if (turnType == CARVE_TURN)
				my_board_controller.MoveForward(SURFER_CARVETURN_ACCELERATION);
			else if (turnType == HARD_REGULAR_TURN)
				my_board_controller.MoveForward(SURFER_CROUCH_ACCELERATION);

			if (changeTurnType)	state = STATE_PICKTURN;
		}

	    TurnDegree();
//		if (!BeachDataArray[current_beach].is_big_wave || test_big_wave)
		    my_board_controller.Turn(BOARD_TURN_RIGHT, degree, dt);

		exit_state = true;
		break;

    case STATE_LEFTTURN:
		last_state = state;
		if (CtrlEvent(PAD_OLLIE))
		{
			state = STATE_CROUCH_LEFTTURN;
			super_state = SUPER_STATE_PREPARE_JUMP;
		}
		else if ((turnType == GRAB_TURN) && !CtrlEvent(PAD_GRAB))
		{
			turnType = REGULAR_TURN;
			state = STATE_PICKTURN;
		}
	    else if ((GetTurnCycle(stick) >= PART_CENTER) || (turn_index != GetAnalogTurn()))
			state = STATE_PICKTURN;
		else
		{
			TurnType();
			if (test_big_wave)
			{
				big_wave_extra_speed = SURFER_BIG_WAVE_ACCELERATION;
				my_board_controller.MoveForward(SURFER_BIG_WAVE_ACCELERATION);
			}
			else if (turnType == CARVE_TURN)
				my_board_controller.MoveForward(SURFER_CARVETURN_ACCELERATION);
			else if (turnType == HARD_REGULAR_TURN)
				my_board_controller.MoveForward(SURFER_CROUCH_ACCELERATION);

			if (changeTurnType)	state = STATE_PICKTURN;
		}

	    TurnDegree();
//		if (!BeachDataArray[current_beach].is_big_wave || test_big_wave)
		    my_board_controller.Turn(BOARD_TURN_LEFT, degree, dt);
		exit_state = true;
		break;

    case STATE_RIGHTTURNPART0:
		last_state = state;
	    if ((GetTurnCycle(stick) <= PART_CENTER) || (turn_index != GetAnalogTurn()))
			state = STATE_PICKTURN;

	    TurnType();
		if (turnType == CARVE_TURN)
			my_board_controller.MoveForward(SURFER_ACCELERATION);

	    if (changeTurnType)	state = STATE_PICKTURN;

	    TurnDegree();
	    my_board_controller.Turn(BOARD_TURN_RIGHT, degree, dt);

		exit_state = true;
		break;

    case STATE_RIGHTTURNPART1:
		last_state = state;
	    if ((GetTurnCycle(stick) <= PART_CENTER) || (turn_index != GetAnalogTurn()))
			state = STATE_PICKTURN;

	    TurnType();
		if (turnType == CARVE_TURN)
			my_board_controller.MoveForward(SURFER_ACCELERATION);

	    if (changeTurnType)	state = STATE_PICKTURN;

	    TurnDegree();
	    my_board_controller.Turn(BOARD_TURN_RIGHT, degree, dt);
		exit_state = true;
		break;

    case STATE_RIGHTTURNPART2:
		last_state = state;
	    if ((GetTurnCycle(stick) <= PART_CENTER) || (turn_index != GetAnalogTurn()))
			state = STATE_PICKTURN;

	    TurnType();
		if (turnType == CARVE_TURN)
			my_board_controller.MoveForward(SURFER_ACCELERATION);

	    if (changeTurnType)	state = STATE_PICKTURN;

	    TurnDegree();
	    my_board_controller.Turn(BOARD_TURN_RIGHT, degree, dt);
		exit_state = true;
		break;

    case STATE_LEFTTURNPART0:
		last_state = state;
	    if ((GetTurnCycle(stick) >= PART_CENTER) || (turn_index != GetAnalogTurn()))
			state = STATE_PICKTURN;

	    TurnType();
		if (turnType == CARVE_TURN)
			my_board_controller.MoveForward(SURFER_ACCELERATION);

	    if (changeTurnType)	state = STATE_PICKTURN;

	    TurnDegree();
		my_board_controller.Turn(BOARD_TURN_LEFT, degree, dt);
		exit_state = true;
		break;

    case STATE_LEFTTURNPART1:
		last_state = state;
	    if ((GetTurnCycle(stick) >= PART_CENTER) || (turn_index != GetAnalogTurn()))
			state = STATE_PICKTURN;

	    TurnType();
		if (turnType == CARVE_TURN)
			my_board_controller.MoveForward(SURFER_ACCELERATION);

	    if (changeTurnType)	state = STATE_PICKTURN;

	    TurnDegree();
		my_board_controller.Turn(BOARD_TURN_LEFT, degree, dt);
		exit_state = true;
		break;

    case STATE_LEFTTURNPART2:
		last_state = state;
	    if ((GetTurnCycle(stick) >= PART_CENTER) || (turn_index != GetAnalogTurn()))
			state = STATE_PICKTURN;

	    TurnType();
		if (turnType == CARVE_TURN)
			my_board_controller.MoveForward(SURFER_ACCELERATION);

	    if (changeTurnType)	state = STATE_PICKTURN;

	    TurnDegree();
		my_board_controller.Turn(BOARD_TURN_LEFT, degree, dt);
		exit_state = true;
		break;

    case STATE_WOBBLE:

      if (CtrlEvent(PAD_LSTICK_H))	// allow move to turn
      {
        TurnDegree();
        my_board_controller.SetTurnType(REGULAR_TURN);
        if (stick > 0.0)
          my_board_controller.Turn(BOARD_TURN_RIGHT, degree, dt);
        else
          my_board_controller.Turn(BOARD_TURN_LEFT, degree, dt);
      }

      if (last_state != state)
        Anim (SURFER_ANIM_BAS_WOBBLE_1, BLEND_TIME, true);

      //IK_enabled = false;
      last_state = state;
      exit_state = true;
      break;

	case STATE_CUTBACK_END:
	{
		if (last_state != state)
		{
			if (last_state == STATE_LEFTREVERTCUTBACK)
			{
				Anim(SURFER_ANIM_TRK_CUTBACK_L_21 + goofy_int, BLEND_TIME, false);
				BoardAnim(BOARD_ANIM_TRK_CUTBACK_L_21 + goofy_int, BLEND_TIME, false);
			}
			else
			{
				Anim(SURFER_ANIM_TRK_CUTBACK_R_21 - goofy_int, BLEND_TIME, false);
				BoardAnim(BOARD_ANIM_TRK_CUTBACK_R_21 - goofy_int, BLEND_TIME, false);
			}
		}

		last_state = state;

		float vert = CtrlEvent(PAD_LSTICK_V);
		//  Only allow stalling if the surfer is faced away from the tube.
		bool facing_left = my_board_controller.GetForwardDir().x < 0;  
		if (((BeachDataArray[current_beach].bdir && facing_left) ||
			(!BeachDataArray[current_beach].bdir && !facing_left)) &&
			vert > STALL_THRESHOLD)
			state = STATE_STALL;
		else if (CtrlEvent(PAD_LSTICK_H))	// allow move to turn
		{
			state = STATE_PICKTURN;
		}
		else if (AnimComplete())
			state = STATE_STAND;
		else
			exit_state = true;

		break;
	}
	case STATE_REBOUND_END:
	{
		if (last_state != state)
		{
			if (last_state == STATE_LEFT_REBOUND)
			{
				Anim(SURFER_ANIM_TRK_CUTBACK_L_11 + goofy_int, BLEND_TIME, false);
				BoardAnim(BOARD_ANIM_TRK_CUTBACK_L_11 + goofy_int, BLEND_TIME, false);
			}
			else
			{
				Anim(SURFER_ANIM_TRK_CUTBACK_R_11 - goofy_int, BLEND_TIME, false);
				BoardAnim(BOARD_ANIM_TRK_CUTBACK_R_11 - goofy_int, BLEND_TIME, false);
			}
		}

		last_state = state;

		float vert = CtrlEvent(PAD_LSTICK_V);
		//  Only allow stalling if the surfer is faced away from the tube.
		bool facing_left = my_board_controller.GetForwardDir().x < 0;  
		if (((BeachDataArray[current_beach].bdir && facing_left) ||
			(!BeachDataArray[current_beach].bdir && !facing_left)) &&
			vert > STALL_THRESHOLD)
			state = STATE_STALL;
		else if (CtrlEvent(PAD_LSTICK_H))	// allow move to turn
		{
			state = STATE_PICKTURN;
		}
		else if (AnimComplete())
			state = STATE_STAND;
		else
			exit_state = true;

		break;
	}

	case STATE_TAIL_CHUCK_END:
	{
		if (last_state != state)
		{
			if (last_state == STATE_LEFT_TAIL_CHUCK)
			{
				Anim(SURFER_ANIM_TRK_CUTBACK_L_31 + goofy_int, BLEND_TIME, false);
				BoardAnim(BOARD_ANIM_TRK_CUTBACK_L_31 + goofy_int, BLEND_TIME, false);
			}
			else
			{
				Anim(SURFER_ANIM_TRK_CUTBACK_R_31 - goofy_int, BLEND_TIME, false);
				BoardAnim(BOARD_ANIM_TRK_CUTBACK_R_31 - goofy_int, BLEND_TIME, false);
			}
		}

		last_state = state;

		float vert = CtrlEvent(PAD_LSTICK_V);
		//  Only allow stalling if the surfer is faced away from the tube.
		bool facing_left = my_board_controller.GetForwardDir().x < 0;  
		if (((BeachDataArray[current_beach].bdir && facing_left) ||
			(!BeachDataArray[current_beach].bdir && !facing_left)) &&
			vert > STALL_THRESHOLD)
			state = STATE_STALL;
		else if (CtrlEvent(PAD_LSTICK_H))	// allow move to turn
		{
			state = STATE_PICKTURN;
		}
		else if (AnimComplete())
			state = STATE_STAND;
		else
			exit_state = true;

		break;
	}

	case STATE_GOUGE_END:
	{
		if (last_state != state)
		{
			if (last_state == STATE_LEFT_GOUGE)
			{
				Anim(SURFER_ANIM_TRK_CUTBACK_L_41 + goofy_int, BLEND_TIME, false);
				BoardAnim(BOARD_ANIM_TRK_CUTBACK_L_41 + goofy_int, BLEND_TIME, false);
			}
			else
			{
				Anim(SURFER_ANIM_TRK_CUTBACK_R_41 - goofy_int, BLEND_TIME, false);
				BoardAnim(BOARD_ANIM_TRK_CUTBACK_R_41 - goofy_int, BLEND_TIME, false);
			}
		}

		last_state = state;

		float vert = CtrlEvent(PAD_LSTICK_V);
		//  Only allow stalling if the surfer is faced away from the tube.
		bool facing_left = my_board_controller.GetForwardDir().x < 0;  
		if (((BeachDataArray[current_beach].bdir && facing_left) ||
			(!BeachDataArray[current_beach].bdir && !facing_left)) &&
			vert > STALL_THRESHOLD)
			state = STATE_STALL;
		else if (CtrlEvent(PAD_LSTICK_H))	// allow move to turn
		{
			state = STATE_PICKTURN;
		}
		else if (AnimComplete())
			state = STATE_STAND;
		else
			exit_state = true;

		break;
	}

	case STATE_LAYBACK_SLIDE_END:
	{
		if (last_state != state)
		{
			if (last_state == STATE_LEFT_LAYBACK_SLIDE)
			{
				Anim(SURFER_ANIM_TRK_CUTBACK_L_51 + goofy_int, BLEND_TIME, false);
				BoardAnim(BOARD_ANIM_TRK_CUTBACK_L_51 + goofy_int, BLEND_TIME, false);
			}
			else
			{
				Anim(SURFER_ANIM_TRK_CUTBACK_R_51 - goofy_int, BLEND_TIME, false);
				BoardAnim(BOARD_ANIM_TRK_CUTBACK_R_51 - goofy_int, BLEND_TIME, false);
			}
		}

		last_state = state;

		float vert = CtrlEvent(PAD_LSTICK_V);
		//  Only allow stalling if the surfer is faced away from the tube.
		bool facing_left = my_board_controller.GetForwardDir().x < 0;  
		if (((BeachDataArray[current_beach].bdir && facing_left) ||
			(!BeachDataArray[current_beach].bdir && !facing_left)) &&
			vert > STALL_THRESHOLD)
			state = STATE_STALL;
		else if (CtrlEvent(PAD_LSTICK_H))	// allow move to turn
		{
			state = STATE_PICKTURN;
		}
		else if (AnimComplete())
			state = STATE_STAND;
		else
			exit_state = true;

		break;
	}

	case STATE_FACE_SPECIAL_TRICK:

		if (last_state != state)
		{
			Anim (GTrickList[newTrick].anim_id, BLEND_TIME, false);
			if (GTrickList[newTrick].board_anim_id != BOARD_ANIM_NULL)
				BoardAnim (GTrickList[newTrick].board_anim_id, BLEND_TIME, false);
			else
				BoardAnim(BOARD_ANIM_ZERO, BLEND_TIME, false);

			initial_face_trick_region = my_board_controller.GetWaveRegion();

			SetCompletedTrick(newTrick);
			SetCurrentTrick();
			last_state = state;
		}

		bDoingTrick = true;
		exit_state = true;
		if (AnimComplete())
		{
			state = STATE_STAND;
			exit_state = false;
			bSpecialTrick = false;
			ResetTricks();
			my_board_controller.SetBoardState(BOARD_NONE);
			if (initial_face_trick_region != WAVE_REGIONTUBE &&
				my_board_controller.GetWaveRegion() == WAVE_REGIONTUBE)
				my_scoreManager.AddGap(GAP_INSANE_TUBE_ENTRY);
		}

		my_board_controller.Turn(BOARD_TURN_RIGHT, 0.0, dt);

		break;

	case STATE_STUMBLE_BACK:
		if (last_state != state)
			Anim (SURFER_ANIM_STUMBLE_BACK, BLEND_TIME);

		last_state = state;

		if (AnimComplete())
		{
			my_board_controller.DoWipeOut(WIP_LOW_STUMBLE_BACK);
		}
		else
			exit_state = true;

		/*if (CtrlEvent(PAD_SLIDE) && CtrlEvent(PAD_LSTICK_H))
		{
			int region = my_board_controller.GetWaveRegion();
			if ((region == WAVE_REGIONLIP) || (region == WAVE_REGIONLIP2) || (region == WAVE_REGIONCHIN)
				|| (region == WAVE_REGIONFACE) || (region == WAVE_REGIONPOCKET))
			{
				state = STATE_PICKTAILSLIDETURNCYCLE;
				super_state = SUPER_STATE_NORMAL_SURF;
				exit_state = false;
			}
		}*/
		break;

  }
	if (gruntEvent >= 0)
	{
		nslSoundId snd = SoundScriptManager::inst()->getSoundId(gruntEvent);
		if (nslGetSoundStatus(snd) != NSL_SOUNDSTATUS_INVALID)
		{	
			nslSetSoundParam(snd, NSL_SOUNDPARAM_VOLUME, random(20) + 30);
		}
	}
}

float g_orient_board = 1.0f;
float exit_move_time_delta = 0.4f;
void kellyslater_controller::SuperStateAir(float dt)
{
	last_super_state = super_state;

	SetTrickRegion(TREGION_AIR);

	IK_enabled = true;
	possible_exit_jump = false;
	//if (CtrlEvent(PAD_OLLIE) && CtrlDelta(PAD_OLLIE) && (my_board_controller.GetAirTime() < exit_move_time_delta)
		//&& !my_board_controller.roof_jump && my_board_controller.lip_jump && !my_board_controller.grind_jump)
		//possible_exit_jump = true;

	if (trick_queued && (newTrick == TRICK_EXIT))
	{
		if ((my_board_controller.GetAirTime() < exit_move_time_delta)
			&& !my_board_controller.roof_jump && my_board_controller.launch_jump && !my_board_controller.grind_jump)
		{
			possible_exit_jump = true;
		}

		trick_queued = false;
	}

	switch (state)
	{
		//--------------------------------------------------
//               *****  OLLIES  *******
//--------------------------------------------------

	case STATE_CONTROLLED_JUMP:

		if (state == last_state)
		{
			my_scoreManager.AddGap(GAP_POP_OFF);
			if (my_board_controller.roof_jump)
			{
				Anim(SURFER_ANIM_IDLE_ON_AIR, 0.1f, false);
				BoardAnim(BOARD_ANIM_IDLE_ON_AIR, 0.1f, false);
			}
			else
			{
				Anim(SURFER_ANIM_TRN_INVERT_1, BLEND_TIME);
				BoardAnim(BOARD_ANIM_TRN_INVERT_1, BLEND_TIME);
			}
			state = STATE_CONTROLLED_AIR;
			tween_timer = 0.0f;
			tween_time = 1.0f;
		}
		else
			my_board_controller.Jump(0.0f);

		last_state = state;
		exit_state = true;
		break;

	case STATE_CONTROLLED_AIR:
		{
			tween_timer += dt;
			if (tween_timer > tween_time)
				tween_timer = tween_time;

			last_state = state;
			exit_state = true;

			po new_po = my_board_controller.rb->my_po;

			int current_beach = g_game_ptr->get_beach_id ();

			vector3d y_axis = YVEC;
			vector3d z_axis = -ZVEC;
			if (!my_board_controller.roof_jump)
			{
				if (BeachDataArray[current_beach].bdir)
					z_axis = -ZVEC - XVEC;
				else
					z_axis = -ZVEC + XVEC;
			}

			z_axis.normalize();

			vector3d x_axis = cross(y_axis, z_axis);

			po target(x_axis, y_axis, z_axis, vector3d(0.0f, 0.0f, 0.0f));
			quaternion up = target.get_quaternion();
			quaternion po_quat = new_po.get_quaternion();
			quaternion quat = slerp(po_quat, up, tween_timer/tween_time);
			quat.normalize();

			matrix4x4 mat;
			quat.to_matrix(&mat);

			po end_po(mat);
			my_board_controller.rb->my_po = end_po;

			if (!my_board_controller.InAir())
			{
				state = STATE_LANDING;
				bSpecialTrick = false;
				exit_state = false;
			}
		}

		break;

    case STATE_LAUNCH:
		//IK_enabled = false;
		tween_timer = 0.0f;
		tween_time = 1.0f;
		my_board_controller.SetTurnType(TRICK_SPIN);

		if (possible_exit_jump)
			my_board_controller.exit_jump = true;

		if (last_state != state)
		{
			Anim(SURFER_ANIM_IDLE_ON_AIR, 0.1f, false);
			BoardAnim(BOARD_ANIM_IDLE_ON_AIR, 0.1f, false);
			my_board_controller.Jump(0.0f);
			my_trickManager.ClearTrickManager();
			hold_time = my_board_controller.GetTotalAirTime();
			hold_timer = 0.0f;
			t_spin_hold = 0.0f;
			if (!my_board_controller.grind_jump && !my_board_controller.grind_ollie)
				spin_ctrl.SetSpinType(SPIN_MANUAL, hold_time);
			
			SoundScriptManager::inst()->playEvent(SS_TAKEOFF, my_board_model);
			ClearTricks();
//			ks_fx_create_big_splash (my_board->get_abs_position ());
			ks_fx_create_launch_splash (((conglomerate*) owner)->get_member("BIP01 R FOOT")->get_abs_position(), GetBoard()->get_abs_po().get_z_facing(), get_board_controller().wave_center_hint, my_player_num);
		}

		last_state = state;

		if (AnimBlended())
		{
			state = STATE_FREEFALL;
			exit_state = false;
		}
		else
		{
			// adjust the surfer orientation and avoit a pop
			vector3d y (my_board_controller.rb->my_po.get_y_facing ());

			// the beach is at -Z
			if (my_board_controller.InLaunchRegion() && (dot (y, ZVEC) > -0.996f))
			{
			  po rot_po;

			  rot_po.set_rot (cross (y, ZVEC), DEG_TO_RAD (180 * dt));
			  my_board_controller.rb->my_po = po (rot_po * my_board_controller.rb->my_po);
			}

			if (CtrlEvent(PAD_LSTICK_H))
			{
				t_spin_hold += dt;
				if (t_spin_hold > 0.25f)
					spin_ctrl.Reset();
			}

			if (!spin_ctrl.IsActivated())
				Spin(dt);

			exit_state = true;
		}
		break;

	case STATE_CHOP_HOP:
		//IK_enabled = false;
		tween_timer = 0.0f;
		tween_time = 0.5f;
		my_board_controller.SetTurnType(TRICK_SPIN);
		if (last_state != state)
		{
			ClearTricks();
			my_trickManager.ClearTrickManager();
			Anim(SURFER_ANIM_TRK_AERIAL_HOP_UP, 0.1, false);
			BoardAnim(BOARD_ANIM_TRK_AERIAL_HOP_UP, 0.1, false);
			my_board_controller.Jump(0.0f);
			t_spin_hold = 0.0f;
			SoundScriptManager::inst()->playEvent(SS_CHOPHOP, my_board_model);

		}
		last_state = state;

		if (AnimComplete())
		{
			my_scoreManager.UpdateLastSeries(ScoringManager::ATTR_HOP, true);
			//SetCompletedTrick(TRICK_HOP);
			hold_time = 2.0f*my_board_controller.CalculatePathPeakTime();
			hold_timer = 0.0f;
			//if (!my_board_controller.grind_jump && !my_board_controller.grind_ollie)
				//spin_ctrl.SetSpinType(SPIN_MANUAL, hold_time);

			state = STATE_FREEFALL_CHOP;
		}
		else
		{
			exit_state = true;
		}

		if (CtrlEvent(PAD_LSTICK_H))
		{
			t_spin_hold += dt;
			if (t_spin_hold > 0.25f)
				spin_ctrl.Reset();
		}

		if (!spin_ctrl.IsActivated())
			Spin(dt);

		break;

    case STATE_FREEFALL:
	    Freefall(dt);
		break;

	case STATE_FREEFALL_CHOP:
		Freefall(dt);
		break;

	case STATE_AIR_TRICK:
		StateAirTrick(dt);
		break;


    case STATE_LANDING:
	    my_board_controller.Turn(BOARD_TURN_RIGHT, 0.0, dt);
		IK_enabled = true;
		if (state != last_state)
		{
			if (my_board_controller.GetLanding() & SurfBoardObjectClass::LANDING_FLAG_JUNK)
				Anim(SURFER_ANIM_AIR_JUNKLAND_1, 0.1, false);       // Blend time changed from 0 to 0.1 since no blending looks bad in slo-mo replay - rbroner (6/25/02)
			else
				Anim(SURFER_ANIM_TRK_AERIAL_HOP_DOWN, 0.1, false);  // Blend time changed from 0 to 0.1 since no blending looks bad in slo-mo replay - rbroner (6/25/02)

			BoardAnim(BOARD_ANIM_ZERO, 0.1, false);       // Blending also necessary here...changed from 0 to 0.1 - rbroner (6/25/02)
			my_board_controller.SetBoardState(BOARD_NONE);
			my_board_controller.SetVelocity(ZEROVEC);
			if (my_board_controller.grind_jump)
				my_scoreManager.UpdateLastSeries(ScoringManager::ATTR_FROM_FLOATER, true);
			g_eventManager.DispatchEvent(EVT_SURFER_LAND, my_player_num, my_board_controller.GetLanding());
			if (last_state != STATE_FREEFALL_CHOP)
				SoundScriptManager::inst()->playEvent(SS_LANDING, my_board_model);
			tween_time = 0.4f;
			tween_timer = 0.0f;
		}

		last_state = state;

		if (trick_queued && DoFaceTrick())
		{
			trick_queued = true;	// guarentees that character will do face trick
			state = STATE_STAND;
			super_state = SUPER_STATE_NORMAL_SURF;
			ResetJumpedEntities();
		}
		else if (!AnimComplete())// && !AnimLooped())//AnimEvnt(~ANIM_END_EVENT))
		{
			exit_state = true;

			bool can_float = my_board_controller.TongueEffectOn();
			bool tongue_on = can_float;

			if (can_float && ((CtrlDelta(PAD_GRIND) && !CtrlEvent(PAD_GRIND))) && my_board_controller.InGrindRegion() &&
						(my_board_controller.grind_ollie || my_board_controller.CollideWithLip()))
			{
				float_trick = -1;
				my_trickManager.ClearTrickManager();
				ClearTricks();  //  this won't clear float tricks
				last_state = state;
				my_board_controller.grind_ollie = false;
				StartFloat(grind_vector);
				my_board_controller.ResetFloatSpeed(false);
				my_board_controller.IncrementFloatSpeed();
			}
			else if (((tongue_on && !my_board_controller.float_jump) || !tongue_on)
				&& ((my_board_controller.GetWaveRegion() == WAVE_REGIONROOF) || (my_board_controller.GetWaveRegion() == WAVE_REGIONBACK)))
			{
				state = STATE_ON_ROOF;
				super_state = SUPER_STATE_NORMAL_SURF;
				exit_state = false;
			}

			if (my_board_controller.GetWaveRegion() == WAVE_REGIONROOF)
				OrientToWave(true, dt, SET_RB_PO);
		}
		else
		{
			// this resets float_trick and float_quadrant so that float tricks inputted while in air will be
			// executed upon landing
			float_trick = -1;
			float_quadrant = -1;

			my_board_controller.grind_ollie = false;
			Anim(StandAnim(), BLEND_TIME);
			BoardAnim(BOARD_ANIM_ZERO, 0.0f, false);
			state = STATE_STAND;
			super_state = SUPER_STATE_NORMAL_SURF;
			//my_trickManager.ClearTrickManager();
			//ClearTricks();  //  this won't clear float tricks
			last_state = state;
			spin_ctrl.Reset();
			ResetJumpedEntities();
			my_board_controller.AddVelocity(-4.5f*ZVEC);
		}
		break;

	case STATE_JUNK_LANDING:

	    my_board_controller.Turn(BOARD_TURN_RIGHT, 0.0, dt);
		IK_enabled = true;
		if (state != last_state)
		{
			junk_combo_pressed = false;
			junk_time = 0.8f;
			junk_timer = 0.0f;
			Anim(SURFER_ANIM_AIR_JUNKLAND_1, 0.0, false);
			//BoardAnim(BOARD_ANIM_TRK_AERIAL_HOP_DOWN, 0.1, false);
			my_board_controller.SetVelocity(ZEROVEC);
			if (my_board_controller.grind_jump)
				my_scoreManager.UpdateLastSeries(ScoringManager::ATTR_FROM_FLOATER, true);
			g_eventManager.DispatchEvent(EVT_SURFER_LAND, my_player_num, my_board_controller.GetLanding());
		}

		last_state = state;

		if (AnimComplete())// && junk_combo_pressed)
		{
			// this resets float_trick and float_quadrant so that float tricks inputted while in air will be
			// executed upon landing
			float_trick = -1;
			float_quadrant = -1;

			my_board_controller.grind_ollie = false;
			Anim(StandAnim(), BLEND_TIME);
			BoardAnim(BOARD_ANIM_ZERO, 0.0f, false);
			state = STATE_STAND;
			super_state = SUPER_STATE_NORMAL_SURF;
			my_trickManager.ClearTrickManager();
			ClearTricks();  //  this won't clear float tricks
			last_state = state;
			spin_ctrl.Reset();
			ResetJumpedEntities();
			my_board_controller.AddVelocity(-4.5f*ZVEC);
		}
		else if (AnimComplete() && !junk_combo_pressed)
		{
			my_board_controller.DoWipeOut(WIP_TAKE_OFF_FLAT);
			float_trick = -1;
			float_quadrant = -1;
			my_board_controller.grind_ollie = false;
			Anim(StandAnim(), BLEND_TIME);
			BoardAnim(BOARD_ANIM_ZERO, 0.0f, false);
			my_trickManager.ClearTrickManager();
			ClearTricks();  //  this won't clear float tricks
			spin_ctrl.Reset();
			ResetJumpedEntities();
			my_board_controller.AddVelocity(-4.5f*ZVEC);
		}
		else
			exit_state = true;


		if ((junk_timer < junk_time) && CtrlEvent(PAD_CARVE))
			junk_combo_pressed = true;

		junk_timer += dt;
		break;
	}
}

int g_iter = 1000;
void kellyslater_controller::OrientToWave(bool upVec, float dt, int flag)
{
	tween_timer += dt;
	if (tween_timer > tween_time)
		tween_timer = tween_time;

	vector3d x_axis, y_axis, z_axis;
	if (upVec)
		y_axis = YVEC;
	else
		y_axis = my_board_controller.GetWaveNormal();

	po new_po;
	if (flag & SET_RB_PO)
	{
		new_po = my_board_controller.rb->my_po;
		z_axis = my_board_controller.rb->my_po.get_z_facing();
	}
	else if (flag & SET_OWNER_PO)
	{
		new_po = get_owner()->get_rel_po();
		z_axis = get_owner()->get_abs_po().get_z_facing();
	}

	z_axis -= y_axis*dot(z_axis, y_axis);
	z_axis.normalize();
	x_axis = cross(y_axis, z_axis);

	if (!upVec)
	{
		float dot_p = dot(y_axis, wave_norm_lf);
		if (dot_p > 1.0f)
			dot_p = 1.0f;
		else if (dot_p < -1.0f)
			dot_p = -1.0f;

		if (dot_p < 0.991445f)  //  angle greater than 7.5 deg
			tween_timer = dt;
	}

	po target(x_axis, y_axis, z_axis, vector3d(0.0f, 0.0f, 0.0f));
	quaternion up = target.get_quaternion();
	quaternion po_quat = new_po.get_quaternion();
	quaternion quat = slerp(po_quat, up, tween_timer/tween_time);
	quat.normalize();

	matrix4x4 mat;
	quat.to_matrix(&mat);

	po end_po(mat);
	if (flag & SET_RB_PO)
		my_board_controller.rb->my_po = end_po;

	if (flag & SET_OWNER_PO)
		get_owner()->set_rel_po (end_po);

	if (flag & SET_BOARD_PO)
		my_board->set_rel_po (my_board_controller.rb->my_po);
}

//	SetTrickRegion()
// Private helper function that changes the surfer's trick region for this frame.
void kellyslater_controller::SetTrickRegion(const TRICKREGION r)
{
	prevTrickRegion = trickRegion;
	trickRegion = r;

	if (prevTrickRegion != trickRegion)
		g_eventManager.DispatchEvent(EVT_TRICK_REGION_CHANGE, my_player_num);
}

void kellyslater_controller::StartCelebration(void) 
{
	super_state = SUPER_STATE_CPU_CONTROLLED;
	state = STATE_CELEBRATE;
}

void kellyslater_controller::StartDisappointment(void) 
{
	super_state = SUPER_STATE_CPU_CONTROLLED;
	state = STATE_DISAPPOINTED;
}


int kellyslater_controller::GetCurrentTrick(void) 
{ 
	if (super_state == SUPER_STATE_IN_TUBE)
	{
		if (state == STATE_TUBE_RAILGRAB)
			return TRICK_TUBE_RAIL_GRAB;
		else
		{
			if (tube_trick != -1)
				return tube_trick;
			else
				return last_tube_trick;
		}
	}
	else
		return currentTrick; 
}


void kellyslater_controller::SuperStateCPUControlled(float dt) 
{
	if (last_state != state)
	{
		bSpecialTrick = false;
		rideTimer = 0.0f;
		int current_beach = g_game_ptr->get_beach_id ();
		if ((current_beach == BEACH_COSMOS) && (state != STATE_CELEBRATERAILGRAB))
			state = STATE_DONTCELEBRATE;

		// Straighten him up.  (dc 03/27/02)
		my_board_controller.Turn(BOARD_TURN_RIGHT, 0.0f, dt);

		switch (state)
		{
		case STATE_CELEBRATE:
				Anim(SURFER_ANIM_BAS_CEL_01, BLEND_TIME, false);
				BoardAnim(BOARD_ANIM_ZERO, 0.0f, false);
			break;

		case STATE_DONTCELEBRATE:
				Anim(SURFER_ANIM_BAS_CELNOT_01, BLEND_TIME, false);
				BoardAnim(BOARD_ANIM_ZERO, 0.0f, false);
			break;

		case STATE_DISAPPOINTED:
				Anim(SURFER_ANIM_BAS_DIS_01, BLEND_TIME, false);
				BoardAnim(BOARD_ANIM_ZERO, 0.0f, false);
			break;
		case STATE_CELEBRATERAILGRAB:
			{
				bool left_wave = BeachDataArray[g_game_ptr->get_beach_id ()].bdir;

				//  Always grab the outside rail.
				if ((left_wave && !goofy) || (!left_wave && goofy))  
					Anim(SURFER_ANIM_TRK_TUBE_GRAB_L_1, BLEND_TIME, false);
				else
					Anim(SURFER_ANIM_TRK_TUBE_GRAB_R_1, BLEND_TIME, false);
				BoardAnim(BOARD_ANIM_ZERO, BLEND_TIME, false);
			}
			break;

		default:
			break;
		}
	}
		
	last_state = state;
	
	if (AnimComplete() || (state == STATE_CELEBRATERAILGRAB))
	{
		done_scoring = true;
		did_celebration = true;
		IK_enabled = true;
		Anim(StandAnim(), BLEND_TIME);
		BoardAnim(BOARD_ANIM_ZERO, 0.0f, false);
	}
	
	exit_state = true;
}

void kellyslater_controller::Freefall(float dt)
{
//	my_board_controller.SetTurnType(TRICK_SPIN);
	exit_state = true;
	IK_enabled = true;

	if (possible_exit_jump)
		my_board_controller.exit_jump = true;

	if (last_state != state)
	{
		if (my_board_controller.GetVelocity().y < 0.0f)
		{
			Anim(SURFER_ANIM_FREEFALL, BLEND_TIME);
			BoardAnim(BOARD_ANIM_BAS_FREEFALL, BLEND_TIME);
		}
		else
		{
      // This anim was getting called twice in a row at the beginning of most jumps (first in SuperStateAir(), then here),
      // causing a sudden change in the animation, expecially noticeable in slo-mo replay. So only call it if it's not
      // already the current anim - rbroner (6/25/02)
      if(mAnim != SURFER_ANIM_IDLE_ON_AIR)
      {
			  Anim(SURFER_ANIM_IDLE_ON_AIR, BLEND_TIME);
			  BoardAnim(BOARD_ANIM_IDLE_ON_AIR, BLEND_TIME);
      }
		}
	}
	else if ((mAnim == SURFER_ANIM_IDLE_ON_AIR) && (my_board_controller.GetVelocity().y < 0.0f))
	{
		Anim(SURFER_ANIM_FREEFALL, BLEND_TIME);
		BoardAnim(BOARD_ANIM_BAS_FREEFALL, BLEND_TIME);
	}

	last_state = state;

	if (my_board_controller.InAir())
	{
		if (CtrlEvent(PAD_LSTICK_H) && (spin_ctrl.GetSpinType() == SPIN_MANUAL))
			spin_ctrl.Reset();

		if (CtrlEvent(PSX_R1) && CtrlDelta(PSX_R1))
			spin_ctrl.SetSpinType(SPIN_180, -1.0f);
		if (CtrlEvent(PSX_L1) && CtrlDelta(PSX_L1))
			spin_ctrl.SetSpinType(SPIN_180, 1.0f);
		else if (!spin_ctrl.IsActivated())
			Spin(dt);

		if (trick_queued && !my_board_controller.roof_jump)// && (state == STATE_FREEFALL))
		{
			exit_state = false;
			current_trick_time = 0.0f;
			state = STATE_AIR_TRICK;
			first_trick = true;
		}
	}
    else
	{
		//if (my_board_controller.GetLanding() & SurfBoardObjectClass::LANDING_FLAG_JUNK)
			//state = STATE_JUNK_LANDING;
		//else
			state = STATE_LANDING;

		bSpecialTrick = false;
		exit_state = false;
	}

	if (g_orient_board && !my_board_controller.float_jump)
		OrientToWave(false, dt, SET_RB_PO);
}

float min_air_trick_time = 0.5f;
float min_tube_trick_time = 0.9f;
float trick_time_fudge = 0.07f;

void kellyslater_controller::StateAirTrick(float dt)
{
	exit_state = true;
	last_state = state;
	bDoingTrick = true;

	current_trick_time += dt;

	if (possible_exit_jump)
		my_board_controller.exit_jump = true;

	if (trick_queued && (first_trick || (trick_timer > min_air_trick_time) || (trick_timer < trick_time_fudge)))
	{
		first_trick = false;
		int trick_ident = GTrickList[newTrick].anim_id;
		int trick_board_ident = GTrickList[newTrick].board_anim_id;

		//  A couple of animations need to be reversed for goofy surfers.
		if (goofy)
		{
			if (trick_ident == SURFER_ANIM_ALL_AIR_FLIP_L_1)
				trick_ident = SURFER_ANIM_ALL_AIR_FLIP_R_1;
			else if (trick_ident == SURFER_ANIM_ALL_AIR_FLIP_R_1)
				trick_ident = SURFER_ANIM_ALL_AIR_FLIP_L_1;

			if (trick_board_ident == BOARD_ANIM_AIR_SHOVETHIS)
				trick_board_ident = BOARD_ANIM_AIR_SHOVEIT_1;
			else if (trick_board_ident == BOARD_ANIM_AIR_SHOVEIT_1)
				trick_board_ident = BOARD_ANIM_AIR_SHOVETHIS;
		}

		Anim(trick_ident, 0.20f, manual);
		BoardAnim(trick_board_ident, 0.20f, manual);
		current_trick_type = manual;
		airIKtrick = newTrick;
		SetCurrentTrick();
		trick_queued = false;
		trick_timer = 0.0f;
	}

	if (current_trick_type && my_trickManager.ActionButtonNonZero() && !my_trickManager.ActionButtonHeld()
			&& AnimBlended() && current_trick_time >= min_air_trick_time)
	{
		state = STATE_FREEFALL;
		ResetTricks();
		exit_state = false;
	}

	if (my_board_controller.InAir())
	{
		bool exit_move = GTrickList[currentTrick].trick_type == TRICKTYPE_EXIT;
		if (exit_move || (CtrlEvent(PAD_LSTICK_H) && (spin_ctrl.GetSpinType() == SPIN_MANUAL)))
			spin_ctrl.Reset();

		if (!exit_move)
		{
			if (CtrlEvent(PSX_R1) && CtrlDelta(PSX_R1))
				spin_ctrl.SetSpinType(SPIN_180, -1.0f);
			if (CtrlEvent(PSX_L1) && CtrlDelta(PSX_L1))
				spin_ctrl.SetSpinType(SPIN_180, 1.0f);
			else if (!spin_ctrl.IsActivated())
				Spin(dt);
		}

		if (!current_trick_type && AnimComplete())
		{
			if (!exit_move)
			{
				state = STATE_FREEFALL;
				ResetTricks();
				exit_state = false;
				bSpecialTrick = false;
			}
			else
			{
				Anim(GTrickList[currentTrick].anim_id + 1, BLEND_TIME);
				BoardAnim(GTrickList[currentTrick].board_anim_id + 1, BLEND_TIME);
			}
		}
		else if (!current_trick_type && (GetAnimPercentage() > 0.1f))
		{
			SetCompletedTrick();  //  SetCompletedTrick  must be called before SetCurrentTrick
			SetCurrentTrick();
		}
		else if (current_trick_type && (trick_timer > trick_time_fudge))
		{
			SetCompletedTrick();  //  SetCompletedTrick  must be called before SetCurrentTrick
			if ((currentTrick == completedTrick))
				my_scoreManager.UpdateLastTrick(ScoringManager::ATTR_TIME_DELTA, dt);
		}
	}
	else
	{
		exit_state = false;
		//if (my_board_controller.GetLanding() & SurfBoardObjectClass::LANDING_FLAG_JUNK)
			//state = STATE_JUNK_LANDING;
		//else
			state = STATE_LANDING;

		bSpecialTrick = false;
	}
	hold_timer += dt;
	trick_timer += dt;
	if (g_orient_board && !my_board_controller.float_jump)
		OrientToWave(false, dt, SET_RB_PO);
}

void kellyslater_controller::SuperStateWipeout(float dt)
{
	// Wipeout ends ride.
	rideTimer = 0.0f;
	IK_enabled = false;

	if (super_state != last_super_state)
	{
		spin_ctrl.Reset();
	}

	if (state != STATE_WIPEOUT_GENERAL)
		state = STATE_WIPEOUT_GENERAL;

	last_super_state = super_state;
	switch (state)
	{
		case STATE_WIPEOUT_GENERAL:

			IK_enabled = false;
			exit_state = true;
			if (last_state != state)
			{
				if (wip_phys_state == WIP_PHYS_TUMBLE)
					Anim(wip_anim_trans, BLEND_TIME, false);
				else
					Anim(wip_anim_dis, BLEND_TIME, false);

				if (b_wip_anim_dis == BOARD_ANIM_NULL)
					BoardAnim(BOARD_ANIM_ZERO, BLEND_TIME, false);
				else
					BoardAnim(b_wip_anim_dis, BLEND_TIME, false);
			}

			last_state = state;

			if (AnimComplete())
			{
				if (mAnim == wip_anim_dis)
				{
					if (wip_phys_state == WIP_PHYS_DISMOUNT2)
					{
						water_region = (WaveRegionEnum) my_board_controller.GetWaveRegion();
						water_current = my_board_controller.GetWaveCurrent();
						water_normal = my_board_controller.GetWaveNormal();
						wip_position = ((conglomerate *)get_owner())->get_member("BIP01 PELVIS")->get_abs_position();
						my_board_controller.SetWipeoutPhysics();
					}

					Anim(wip_anim_free, BLEND_TIME);
					wip_phys_state = WIP_PHYS_FREEFALL;
				}
				else if (mAnim == wip_anim_imp)
				{
					low_wip_y_height = wip_position.y;
					if (wip_anim_imp2 != SURFER_ANIM_NULL)
					{
						ks_fx_create_big_splash(wip_position);
						Anim(wip_anim_imp2, BLEND_TIME, false);
					}
					else
					{
						Anim(wip_anim_trans, BLEND_TIME, false);
						wip_phys_state = WIP_PHYS_TUMBLE;
						if (!IsAIPlayer())
							get_owner()->SetCull(false);
					}
				}
				else if (mAnim == wip_anim_imp2)
				{
					low_wip_y_height = wip_position.y;
					Anim(wip_anim_trans, BLEND_TIME, false);
					wip_phys_state = WIP_PHYS_TUMBLE;
					if (!IsAIPlayer())
						get_owner()->SetCull(false);
				}
				else if (mAnim == wip_anim_trans)
				{
					Anim(wip_anim_swim, BLEND_TIME, false);
					wip_phys_state = WIP_PHYS_SWIM;
				}
				else if (mAnim == wip_anim_swim)
				{
					if (no_buttons_pressed)
						num_wipeouts++;

					get_owner()->SetCull(true);
					my_board_controller.SetWipeoutDone();
					int eos_anim;
					if (wip_anim_swim == SURFER_ANIM_W_1_R_SWIM)
					{
						eos_anim = SURFER_ANIM_W_1_R_EOS;
						get_on_board_anim = SURFER_ANIM_W_1_R_GOB;
						b_get_on_board_anim = BOARD_ANIM_W_1_R_GOB;
					}
					else
					{
						eos_anim = SURFER_ANIM_W_1_L_EOS;
						get_on_board_anim = SURFER_ANIM_W_1_L_GOB;
						b_get_on_board_anim = BOARD_ANIM_W_1_L_GOB;
					}

					Anim(eos_anim, BLEND_TIME, true);
				}
			}

			if (BoardAnimComplete())
			{
				if (bAnim == b_wip_anim_dis)
				{
					if (b_wip_anim_free != BOARD_ANIM_NULL)
						BoardAnim(b_wip_anim_free, 0.0f);
					else
						BoardAnim(BOARD_ANIM_ZERO, 0.0f);
				}
				else if (bAnim == b_wip_anim_imp)
					BoardAnim(BOARD_ANIM_ZERO, BLEND_TIME, false);
			}

			if (b_wip_anim_imp && !my_board_controller.inAirFlag && (wip_phys_state != WIP_PHYS_DISMOUNT2)
					&& ((bAnim == b_wip_anim_dis) || (bAnim == b_wip_anim_free)))
			{
				if (b_wip_anim_imp != -1)
					BoardAnim(b_wip_anim_imp, 0.0f, false);
			}

			break;
	}
}


void kellyslater_controller::start_secondary_cam(camera *cur_cam)
{
	if (state == STATE_DUCKDIVE || super_state == SUPER_STATE_AIR || super_state == SUPER_STATE_WIPEOUT || 
		super_state == SUPER_STATE_CPU_CONTROLLED || super_state == SUPER_STATE_FLYBY ||
		cur_cam == photo_cam_ptr)
		return;

	camera *secondary_cam = look_back_cam_ptr;  //  Most beaches have the look back cam as a secondary camera.

/*	if (g_game_ptr->get_beach_id () == BEACH_test_buoy)  //  Some have the buoy cam, because it is easier to see towards the beach.
		secondary_cam = buoy_cam_ptr;
*/
	if (cur_cam != secondary_cam)
	{
		SetPlayerCamera((game_camera *)secondary_cam);
	}
}


void kellyslater_controller::end_secondary_cam()
{
	camera *secondary_cam = look_back_cam_ptr;  //  Most beaches have the look back cam as a secondary camera.

/*	if (g_game_ptr->get_beach_id () == BEACH_test_buoy)  //  Some have the buoy cam, because it is easier to see towards the beach.
		secondary_cam = buoy_cam_ptr;
*/
	if (g_game_ptr->get_player_camera(my_player_num) == secondary_cam)			//  Ending Look back.
	{
		SetPlayerCamera(player_cam);
	}
}

float g_special_meter_full = 0.0f;
void kellyslater_controller::process_controls(float dt)
{
//	int cycle;
//	float charge;
	exit_state = false;
	trick_complete = false;
	bDoingTrick = false; //  this must be set every frame, and if trick is in process, code will set true
	perform_snap = false;
	rideTimer += dt;

	if (g_surfer_attribs.use_debug_values)
		SetSurferAttribs();

	if (g_special_meter_full)
		specialMeter.SetFillage(1.0f);

	//  If the user is pressing the look back button, use the look back camera.	Otherwise, normal cam.
	if (!app::inst()->get_game()->is_paused())  //  only change the camera when not paused.
	{
		camera *cur_cam = app::inst()->get_game()->get_player_camera(my_player_num);

		if (CtrlEvent(PAD_LOOK_BACK))
			start_secondary_cam(cur_cam);
		else if (!CtrlEvent(PAD_LOOK_BACK))  //  We're done looking back, so change to shoulder cam.
			end_secondary_cam();

		//  Switch Cameras, don't allow this while the secondary camera is active.
		if (CtrlEvent(PAD_SWITCH_CAMERA) && super_state != SUPER_STATE_AIR && super_state != SUPER_STATE_WIPEOUT &&
			super_state != SUPER_STATE_FLYBY && super_state != SUPER_STATE_CPU_CONTROLLED
			&& state != STATE_DUCKDIVE)
		{
			if (!switch_cam_pressed_already)  //  only do this once per press.
			{
				switch_cam_pressed_already = true;

				// Woo, photos!  (Psst, don't tell anyone this is here!)
				#ifdef TOBY
				if (g_beach_ptr->get_challenges()->photo) g_beach_ptr->get_challenges()->photo->Debug_TakePhoto();
				#endif
				
				if (cur_cam == follow_close_cam_ptr)
				{
					StoredConfigData::inst()->setLastCamera(get_player_num(), (stringx("BEACH_CAM") + stringx(get_player_num())).c_str());
					SetPlayerCamera(beach_cam_ptr);
				}
				else if ((cur_cam == beach_cam_ptr) && (g_session_cheats[CHEAT_FPS_CAM].isOn()))
				{
					StoredConfigData::inst()->setLastCamera(get_player_num(), (stringx("FPS_CAM") + stringx(get_player_num())).c_str());
					SetPlayerCamera(fps_cam_ptr);
				}
				else if (cur_cam == beach_cam_ptr)
				{
					StoredConfigData::inst()->setLastCamera(get_player_num(), (stringx("FOLLOW_CLOSE_CAM") + stringx(get_player_num())).c_str());
					SetPlayerCamera(follow_close_cam_ptr);
				}
				else if (cur_cam == fps_cam_ptr)
				{
					StoredConfigData::inst()->setLastCamera(get_player_num(), (stringx("FOLLOW_CLOSE_CAM") + stringx(get_player_num())).c_str());
					SetPlayerCamera(follow_close_cam_ptr);
				}
			}
		}
		else
			switch_cam_pressed_already = false;

	}  //  end if not paused.

	assert(state_stack.size()<STATE_STACK_MAX_DEPTH);
	while (!exit_state)
	{
		my_board_controller.MoveForward(0.0f);  //  resets board forward force at start of frame
		my_board_controller.SetControllerForce(ZEROVEC);
		switch (super_state)
		{
		case SUPER_STATE_FLYBY:
			SuperStateFlyby(dt);
			break;
		case SUPER_STATE_NORMAL_SURF:
			SuperStateNormalSurf(dt);
			break;
		case SUPER_STATE_LIE_ON_BOARD:
			SuperStateLieOnBoard(dt);
			break;
		case SUPER_STATE_PREPARE_JUMP:
			SuperStatePrepareJump(dt);
			break;
		case SUPER_STATE_AIR:
			SuperStateAir(dt);
			break;
		case SUPER_STATE_WIPEOUT:
			SuperStateWipeout(dt);
			break;
		case SUPER_STATE_IN_TUBE:
			SuperStateInTube(dt);
			break;
		case SUPER_STATE_CPU_CONTROLLED:
			SuperStateCPUControlled(dt);
			break;

		default:
			debug_print("Surfer entered unimplemented state.");
			state = STATE_STAND;
			super_state = SUPER_STATE_NORMAL_SURF;
			exit_state = true;
		}
	}

	/*int bturn = my_board_controller.GetTurnType();
	if ((bturn == CARVE_TURN) || (bturn == GRAB_TURN) || (bturn == HARD_GRAB_TURN)
							|| my_board_controller.DoingFaceTurn())
		bDoingTrick = true;

	int bstate = my_board_controller.GetBoardState();
	if ((bstate == BOARD_GRIND) || (bstate == BOARD_FLOAT))
		bDoingTrick = true;*/


#ifdef DEBUG
	if ((state >= STATE_NOSTATE) && (state < STATE_NUMSTATES))
	{
		gp_ksstate_string[my_player_num] = g_kscontroller_state[state];
	}

	if ((mAnim >= 0) && (mAnim < _SURFER_NUM_ANIMS))
	{
		gp_ksanim_string[my_player_num] = g_surfer_anims[mAnim].c_str();
	}
#endif

}


/*
void kellyslater_controller::SetupTurn()
{
  input_mgr* inputmgr = input_mgr::inst();
	if (CtrlEvent(PAD_GRAB)!=AXIS_MID
		SetupGrabTurn();
	if (CtrlEvent(PAD_SLIDE_TURN)!=AXIS_MID)
		SetupTailslideTurn();
	if (CtrlEvent(PAD_SNAP_TURN)!=AXIS_MID)
		SetupSnapTurn();
	else
		SetupRegularTurn();
}

void kellyslater_controller::SetupRegularTurn()
{
  input_mgr* inputmgr = input_mgr::inst();
	if (CtrlEvent(PAD_HARD_TURN)!=AXIS_MID)
	{
		turnVel			= hardCarve.TurnVel;
		bankAccel		= hardCarve.BankAccel;
		bankVel			= hardCarve.BankVel;
		bank	  		= hardCarve.Bank;
		leftTurnAnim	= SURFER_ANIM_LEFTTURN;
		rightTurnAnim	= SURFER_ANIM_RIGHTTURN;
	}
	else
	{
		turnVel			= carve.TurnVel;
		bankAccel		= carve.BankAccel;
		bankVel			= carve.BankVel;
		bank		  	= carve.Bank;
		leftTurnAnim	= SURFER_ANIM_LEFTTURN;
		rightTurnAnim	= SURFER_ANIM_RIGHTTURN;
	}

}
void kellyslater_controller::SetupGrabTurn()
{
  input_mgr* inputmgr = input_mgr::inst();
	if (CtrlEvent(PAD_HARD_TURN)!=AXIS_MID)
	{
		turnVel			= hardGrab.TurnVel;
		bankAccel		= hardGrab.BankAccel;
		bankVel			= hardGrab.BankVel;
		bank			  = hardGrab.Bank;
		leftTurnAnim	= SURFER_ANIM_LEFTGRABTURN;
		rightTurnAnim	= SURFER_ANIM_RIGHTGRABTURN;
	}
	else
	{
		turnVel			= grab.TurnVel;
		bankAccel		= grab.BankAccel;
		bankVel			= grab.BankVel;
		bank		  	= grab.Bank;
		leftTurnAnim	= SURFER_ANIM_LEFTGRABTURN;
		rightTurnAnim	= SURFER_ANIM_RIGHTGRABTURN;
	}

}


void kellyslater_controller::SetupTailslideTurn()
{
  input_mgr* inputmgr = input_mgr::inst();
	if (CtrlEvent(PAD_HARD_TURN)!=AXIS_MID)
	{
		turnVel			= hardSlide.TurnVel;
		bankAccel		= hardSlide.BankAccel;
		bankVel			= hardSlide.BankVel;
		bank		  	= hardSlide.Bank;
		leftTurnAnim	= SURFER_ANIM_LEFTSLIDETURN;
		rightTurnAnim	= SURFER_ANIM_RIGHTSLIDETURN;
	}
	else
	{
		turnVel			= slide.TurnVel;
		bankAccel		= slide.BankAccel;
		bankVel			= slide.BankVel;
		bank	  		= slide.Bank;
		leftTurnAnim	= SURFER_ANIM_LEFTSLIDETURN;
		rightTurnAnim	= SURFER_ANIM_RIGHTSLIDETURN;
	}
}


void kellyslater_controller::SetupSnapTurn()
{
	turnVel			= SNAP_TURN_VEL;
	bankAccel		= SNAP_BANK_ACCEL;
	bankVel			= SNAP_BANK_VEL;
	bank			= SNAP_BANK;
	leftTurnAnim	= SURFER_ANIM_LEFTSNAP;
	rightTurnAnim	= SURFER_ANIM_RIGHTSNAP;
}
*/

////////////////////////////////////////////////////////////////////////////////////////
//
//    Update Section
//
////////////////////////////////////////////////////////////////////////////////////////




/*
static void LimitVel(float destVel, float *vel, float *accel)
{
	if (*accel != 0)
	{
		if (*vel > 0)
		{
			if (*vel >= destVel)
			{
				*vel = destVel;
				*accel = 0;
			}
		}
		else
		{
			if (*vel <= destVel)
			{
				*vel = destVel;
				*accel = 0;
			}
		}
	}
}

static void LimitRot(float destRot, float *destVel, float *rot, float *vel, float *accel)
{
	if (*vel != 0)
	{
		if (*vel > 0)
		{
			if (*rot >= destRot)
			{
				*rot = destRot;
				*vel = 0;
				*accel = 0;
				*destVel = 0;
			}
		}
		else
		{
			if (*rot <= destRot)
			{
				*rot = destRot;
				*vel = 0;
				*accel = 0;
				*destVel = 0;
			}
		}
	}
}
*/




/*
bool get_wave_collision_info(vector3d & pos, vector3d * surfloc, vector3d * surfangles)
{
  *surfloc = pos;

  surfloc->y = 16;

  *surfangles = vector3d (0,0,0);

  rx = acos (nx / vector3d (nx, ny, 0).length ());
  rz = acos (nz / vector3d (0, ny, nz).length ());

  *surfangles = vector3d (rx,0,rz);

  return (pos.y<surfloc->y);
}
*/

#define TRICK_SCORE_X	320
#define TRICK_SCORE_Y	24

// return desired distance from character's root to floor
rational_t kellyslater_controller::get_floor_offset(void)
{
  // for now at least, this is entirely determined by the root animation
  entity_anim_tree* a = NULL;
  rational_t last_floor_offset = 0.0f;

  for(int i=(MAX_ANIM_SLOTS-1); i>=0 && (a == NULL); --i)
//  for(int i=0; i<MAX_ANIM_SLOTS && (a == NULL); ++i)
  {
    a = get_owner()->get_anim_tree( i );
    if(a && (a->is_finished() || a->get_floor_offset() <= 0.0f || !a->is_root(get_owner())))
      a = NULL;
  }

  if ( a!=NULL && a->is_relative_to_start() )
  {
    vector3d relp = ZEROVEC;
    a->get_current_root_relpos( &relp );
    last_floor_offset = a->get_floor_offset();
    last_floor_offset += relp.y;
  }
  return last_floor_offset;
}

float under_water_height = -5;
float no_swim = 0.5f;
float modifier_rate = 2.0f;
float wip_skag_fric = 2.5f;

void kellyslater_controller::downdate(float dt)
{
	//  how far the surfer is into his wipeout determines how much of a programatic affect we want to have on him.
	float wipeout_timing = my_board_controller.get_wipeout_time();
	if (wipeout_timing <= start_wip_time)
		wipeout_timing = 0;
	else
	{
		wipeout_timing -= start_wip_time;
		wipeout_timing /= (WIPEOUT_RESET_TIME - start_wip_time);
	}

  if (my_board_controller.WipedOut() && (last_super_state == SUPER_STATE_WIPEOUT))
  {
	  DoWipeoutPhysics(dt);
  }
  else
  {

    AlignSurferWithBoard(dt);

    my_board_controller.GetVelocity (velocity);

    WAVE_GlobalCurrent (&current);
    velocity -= current;

    if (state == STATE_LIEONBOARD)
    {
      left_hand_dry = false;
      right_hand_dry = false;
    }
    else if (state == STATE_PADDLE)
    {
      vector3d pos, n (my_board->get_abs_po ().get_y_facing ());

			const float paddle_offset = -0.75f;

      pos = ((conglomerate*) owner)->get_member("BIP01 L HAND")->get_handed_abs_position ();
      if (dot (n, pos - my_board->get_abs_position ()) > 0)
        left_hand_dry = true;
      else if (left_hand_dry)
      {
        vector3d handpos = pos + paddle_offset * owner->get_abs_po().get_x_facing();
        sfx.paddle(false, handpos.x, handpos.y, handpos.z, .5);
        ks_fx_create_paddle_splash (pos);

        left_hand_dry = false;
      }

      pos = ((conglomerate*) owner)->get_member("BIP01 R HAND")->get_handed_abs_position ();
      if (dot (n, pos - my_board->get_abs_position ()) > 0)
        right_hand_dry = true;
      else if (right_hand_dry)
      {
        vector3d handpos = pos + paddle_offset * owner->get_abs_po().get_x_facing();
        sfx.paddle(true, handpos.x, handpos.y, handpos.z, .5);
        ks_fx_create_paddle_splash (pos);
        right_hand_dry = false;
      }
    }
  }
  
/*
  // Add water effects if the board is on the water
  // This is being called in the player update over the board update
  // because this is the only function that works in replay
  if (!my_board_controller.inAirFlag)
    ks_fx_board_update(this, dt);
  else
    ks_fx_board_update_air(this, dt);
    */
}

// Lifted from SurferScriptObjectClass::Update(float dt)  PTA 4/25/01
void kellyslater_controller::update(float dt)
{
	float	dist = -1;	// distance between surfer and the tube opening
	bool	founddist = false;
	float	px = my_board_controller.rb->pos.x;	// player x position

	// Calculate how close player is to a wave break.  (used for scoring purposes)
	for (u_int i = 0; i < WAVE_SoundEmitter[WAVE_SE_TUBE].numsegment; i++)
	{
		// Player is inside a break.
		if (px >= WAVE_SoundEmitter[WAVE_SE_TUBE].segment[i].start.x &&
			px <= WAVE_SoundEmitter[WAVE_SE_TUBE].segment[i].stop.x)
		{
			dist = 0.0f;
			founddist = true;
			break;
		}
		// Player is between two breaks.
		else if (px >= WAVE_SoundEmitter[WAVE_SE_TUBE].segment[i].stop.x &&
			i < WAVE_SoundEmitter[WAVE_SE_TUBE].numsegment-1 &&
			px <= WAVE_SoundEmitter[WAVE_SE_TUBE].segment[i+1].start.x)
		{
			// Dist = closest of the two breaks.
			float	d = __fabs(px - WAVE_SoundEmitter[WAVE_SE_TUBE].segment[i].stop.x);
			dist = __fabs(px - WAVE_SoundEmitter[WAVE_SE_TUBE].segment[i+1].start.x);
			if (d < dist)
				dist = d;
			founddist = true;
			break;
		}
		// Player is to the right of this break.
		else if (px > WAVE_SoundEmitter[WAVE_SE_TUBE].segment[i].stop.x &&
			i == WAVE_SoundEmitter[WAVE_SE_TUBE].numsegment-1)
		{
			dist = __fabs(px - WAVE_SoundEmitter[WAVE_SE_TUBE].segment[i].stop.x);
			founddist = true;
			break;
		}
		// Player is to the left of this break.
		else if (px < WAVE_SoundEmitter[WAVE_SE_TUBE].segment[i].start.x &&
			i == 0)
		{
			dist = __fabs(px - WAVE_SoundEmitter[WAVE_SE_TUBE].segment[i].start.x);
			founddist = true;
			break;
		}
	}

	// If we couldn't calculte the distance using tube emitters, use the face emitter.
	if (!founddist)
	{
		dist = __fabs(px - WAVE_SoundEmitter[WAVE_SE_FACE].segment[0].stop.x);
		founddist = true;
	}

	// Update danger/coolness of trick based on surfer's position on the wave.
	if (dist <= ScoringManager::MOUTH_DISTANCES[0])
	{
		my_scoreManager.SetMouthDist(0.0f);
#if defined(DEBUG) && defined(TOBY)
		//frontendmanager.IGO->SetDebugText(stringx(dist)+stringx("CRITICAL"));
#endif
	}
	else if (dist <= ScoringManager::MOUTH_DISTANCES[1])
	{
		my_scoreManager.SetMouthDist(0.5f);
#if defined(DEBUG) && defined(TOBY)
		//frontendmanager.IGO->SetDebugText(stringx(dist)+stringx("MAIN"));
#endif
	}
	else
	{
		my_scoreManager.SetMouthDist(1.0f);
#if defined(DEBUG) && defined(TOBY)
		//frontendmanager.IGO->SetDebugText(stringx(dist)+stringx("EDGE"));
#endif
	}

	if (my_board_controller.state != BOARD_ANIM_CONTROL)
	{	
		// update board & set surfer to board's position
		my_board_controller.Update(dt);

		downdate(dt);
	}
	
	if ((get_super_state() == SUPER_STATE_NORMAL_SURF && rideEvent ==-1))
	{
		rideEvent = SoundScriptManager::inst()->startEvent(SS_BOARD_RUNNING, my_board_model, .5);
	}
	if (rideEvent !=-1 && (get_super_state() != SUPER_STATE_NORMAL_SURF || 
	    nslGetSoundStatus(SoundScriptManager::inst()->getSoundId(rideEvent)) == NSL_SOUNDSTATUS_INVALID))
	{
		SoundScriptManager::inst()->endEvent(rideEvent, .3);
		rideEvent = -1;
	}

	if (floaterEvent >= 0 && (state !=STATE_FLOAT || 
	    nslGetSoundStatus(SoundScriptManager::inst()->getSoundId(floaterEvent)) == NSL_SOUNDSTATUS_INVALID))
	{
		SoundScriptManager::inst()->endEvent(floaterEvent);
		floaterEvent = -1;
	}

	if (spinEvent >= 0 && (state !=STATE_TAILSLIDE_RIGHTTURN && state != STATE_TAILSLIDE_LEFTTURN || 
	    nslGetSoundStatus(SoundScriptManager::inst()->getSoundId(spinEvent)) == NSL_SOUNDSTATUS_INVALID))
	{
		SoundScriptManager::inst()->endEvent(spinEvent, .3);
		spinEvent = -1;
	}	
	if (stallEvent >= 0 &&  ((state !=STATE_STALL && state != STATE_SUPER_STALL) || 
	    nslGetSoundStatus(SoundScriptManager::inst()->getSoundId(stallEvent)) == NSL_SOUNDSTATUS_INVALID))
	{
		SoundScriptManager::inst()->endEvent(stallEvent);
		stallEvent = -1;
	}

	if (carveEvent >= 0 && (get_board_controller().InAir() ||
		  !(input_mgr::inst()->get_control_state(JOYSTICK_TO_DEVICE_ID(this->get_joystick_num()), PSX_LR)) || 
	    nslGetSoundStatus(SoundScriptManager::inst()->getSoundId(carveEvent)) == NSL_SOUNDSTATUS_INVALID))
	{
		SoundScriptManager::inst()->endEvent(carveEvent, .3);
		carveEvent = -1;
	}

	if (!get_board_controller().InAir() && carveEvent < 0 &&  get_super_state() == SUPER_STATE_NORMAL_SURF &&
			(input_mgr::inst()->get_control_state(JOYSTICK_TO_DEVICE_ID(this->get_joystick_num()), PSX_LR)))
	{
		carveEvent = SoundScriptManager::inst()->startEvent(SS_CARVE, my_board_model, .3);
	}
}

void kellyslater_controller::DoWipImpactSoundStuff(void)
{
	vector3d toAIPlayer, forw;
	if (IsAIPlayer())
	{
		toAIPlayer = get_owner_po().get_position() - g_game_ptr->get_player_camera(0)->get_abs_position();
		toAIPlayer.normalize();
	}
	forw = g_game_ptr->get_player_camera(0)->get_abs_po().get_facing();
	SSEventId s;
	nslSoundId snd;
	float dotVal = dot(forw, toAIPlayer);
	
	// Play sounds for the main player or for AI players that are in front
	if (!IsAIPlayer() || (IsAIPlayer() && 
		(dotVal > .8) &&
		!UNDERWATER_CameraUnderwater(0) && 
		get_super_state() != SUPER_STATE_FLYBY))
	{
		if ((wipeout_type == WIP_HIT_BY_CEILING_LEFT) || 
		(wipeout_type == WIP_HIT_BY_CEILING_RIGHT) ||
		(wipeout_type == WIP_HIT_BY_CEILING_FOR) ||
		(wipeout_type == WIP_HIT_BY_CEILING_BACK) ||
		(wipeout_type == WIP_TUBE_LOSE_BALANCE_LEFT) ||
		(wipeout_type == WIP_TUBE_LOSE_BALANCE_RIGHT))
		{
		 if (random(100) < 20)
		 {
	
			 
				if (IsAIPlayer())
					s = SoundScriptManager::inst()->playEvent(	SS_WIPEOUT_HITWATER_TUBE, my_board_model); 
				else
					s = SoundScriptManager::inst()->playEvent(	SS_WIPEOUT_HITWATER_TUBE  ); 
				snd = SoundScriptManager::inst()->getSoundId(s);
				if (nslGetSoundStatus(snd) != NSL_SOUNDSTATUS_INVALID && !IsAIPlayer())
					nslDampenGuardSound(snd);

		 }
		 else
		 {
				if (IsAIPlayer())
					s = SoundScriptManager::inst()->playEvent(SS_WIPEOUT_HITWATER, my_board_model); 
				else
					s = SoundScriptManager::inst()->playEvent(SS_WIPEOUT_HITWATER); 

				snd = SoundScriptManager::inst()->getSoundId(s);

				if (nslGetSoundStatus(snd) != NSL_SOUNDSTATUS_INVALID && !IsAIPlayer())
					nslDampenGuardSound(snd);

		 }

		}
		else
		{
			if (IsAIPlayer())
				s = SoundScriptManager::inst()->playEvent(SS_WIPEOUT_HITWATER, my_board_model); 
			else
				s = SoundScriptManager::inst()->playEvent(SS_WIPEOUT_HITWATER); 

			snd = SoundScriptManager::inst()->getSoundId(s);
			if (nslGetSoundStatus(snd) != NSL_SOUNDSTATUS_INVALID && !IsAIPlayer())
				nslDampenGuardSound(snd);
		}


		if (IsAIPlayer())
		{
			if (g_game_ptr->GetSurferIdx(my_player_num) != SURFER_LISA_ANDERSEN)
				s= SoundScriptManager::inst()->playEvent(SS_MALE_WIPEOUT, my_board_model); 
			else
				s = SoundScriptManager::inst()->playEvent(SS_FEMALE_WIPEOUT, my_board_model); 
		}
		else
		{
			if (g_game_ptr->GetSurferIdx(my_player_num) != SURFER_LISA_ANDERSEN)
				s= SoundScriptManager::inst()->playEvent(SS_MALE_WIPEOUT); 
			else
				s = SoundScriptManager::inst()->playEvent(SS_FEMALE_WIPEOUT); 
		}

		snd = SoundScriptManager::inst()->getSoundId(s);

		if (nslGetSoundStatus(snd) != NSL_SOUNDSTATUS_INVALID && !IsAIPlayer())
			nslDampenGuardSound(snd);
		
		// Immediately start shutting down the voice
		SoundScriptManager::inst()->endEvent(s, 2);
	}
}

#ifdef JWEBSTER
#ifdef DEBUG
vector3d g_wipeout_collide;
#endif
#endif

float g_impact_grav_mult = 2.0f;
void kellyslater_controller::DoWipeoutPhysics(float dt)
{
	// move the surfer
	vector3d skag_force;
	float grav = 30.0f;

	bool in_air = (wip_phys_state == WIP_PHYS_DISMOUNT) || (wip_phys_state == WIP_PHYS_FREEFALL);			
	switch (wip_phys_state)
	{
	case WIP_PHYS_IMPACT:
		{
			grav *= g_impact_grav_mult;
			vector3d grav_vec = -grav*(YVEC - dot(YVEC, water_normal)*water_normal);
			skag_force = (water_current - surfer_velocity)*wip_skag_fric;
			surfer_velocity += (grav_vec + skag_force)*dt;
			surfer_velocity -= dot(water_normal, surfer_velocity)*water_normal;
			wip_position += surfer_velocity*dt;
			break;
		}
	case WIP_PHYS_TUMBLE:
	case WIP_PHYS_SWIM:
		{
			water_current.y = 0.0f;
			skag_force = (water_current - surfer_velocity)*wip_skag_fric;
			surfer_velocity += 3.0f*skag_force*dt;
			wip_position += surfer_velocity*dt;
			break;
		}
	case WIP_PHYS_DISMOUNT:
	case WIP_PHYS_FREEFALL:
		{
			surfer_velocity -= grav*dt*YVEC;
			wip_position += surfer_velocity*dt;
			break;
		}
	case WIP_PHYS_DISMOUNT2:
		AlignSurferWithBoard(dt);
		return;
		break;
	}

	wave_norm_lf = water_normal;  //  save last frames water normal
	vector3d pelvis_position(((conglomerate*) get_owner ())->get_member("BIP01 PELVIS")->get_abs_position());
	CollideCallStruct collide (wip_position, &water_normal, NULL, &water_region, &wave_hint);
	static vector3d prev_surfer_pos;
	if (wave_hint_valid)
	{
		float deltadist = (wip_position - prev_surfer_pos).length();
		collide.tolerance.dthresh = collide.tolerance.zthresh = deltadist + 1;
	}
	else
	{
		collide.tolerance.dthresh = 1; 
		collide.tolerance.zthresh = 0;	// will trigger error if hint used (dc 05/04/02)
	}

	WaveMarkerEnum front = WAVE_MarkerInvalid;
	WaveMarkerEnum back = WAVE_MarkerInvalid;
	bool limit_z = false;
	if (((water_region == WAVE_REGIONTUBE) || (water_region == WAVE_REGIONCEILING)) && in_air)
	{
		limit_z = true;
		front = WAVE_MarkerTubeMinZ;
		back = WAVE_MarkerTubeMaxZ;
	}

	WAVE_CheckCollision (collide, wave_hint_valid, false, !in_air, limit_z, front, back);
	prev_surfer_pos = collide.position;
	WAVE_GlobalCurrent (&water_current);
	wave_hint_valid = true;

	if (!in_air)
		water_region = *collide.region;

	float height = -(collide.position.y - pelvis_position.y);
	vector3d tube_bottom = *WAVE_GetMarker(WAVE_MarkerTubeBottom);
	float height2 = pelvis_position.y - tube_bottom.y;

	bool ignore_roof = (wipeout_type == WIP_HIT_BY_CEILING_LEFT);
	if ((water_region == WAVE_REGIONCEILING) || (water_region == WAVE_REGIONSHOULDER)
			|| (ignore_roof && (water_region == WAVE_REGIONROOF)))
	{
		height = 2.0f;  // set greater than zero, ignore collision
		wave_hint_valid = false;
	}

#ifdef JWEBSTER
#ifdef DEBUG
	g_wipeout_collide = collide.position;
#endif
#endif

	// Is he entering the water for the first time?
	// A: No, he had to paddle to get to the wave.
	if (dry && (surfer_velocity.y < 0.0f) && (((height < 0) && (water_normal.y > 0.0f) && (dot(surfer_velocity, water_normal) < 0.0f)
			&& (water_region != WAVE_REGIONCEILING)) || (height2 < 0.0f)))
	{
		if ((height2 > 0.0f) && (water_region == WAVE_REGIONROOF)
				&& (dot(surfer_velocity, surfer_velocity) > 0.5f))
		{
			vector3d vel = water_normal*dot(water_normal, surfer_velocity);
			surfer_velocity -= 1.5f*vel;
			wip_position = collide.position;
		}
		else
		{
			if (wip_anim_imp == SURFER_ANIM_NULL)
			{
				low_wip_y_height = wip_position.y;
				Anim(wip_anim_trans, BLEND_TIME, false);
				wip_phys_state = WIP_PHYS_TUMBLE;
				if (!IsAIPlayer())
					get_owner()->SetCull(false);
			}
			else
			{
				Anim(wip_anim_imp, BLEND_TIME, false);
				wip_phys_state = WIP_PHYS_IMPACT;
			}

			ks_fx_create_big_splash(collide.position);
			dry = false;
			DoWipImpactSoundStuff();

			if (height > 0.0f)
				collide.position.y = tube_bottom.y;
		}

		ks_fx_start_wipeout_splash (my_player_num);
	}

	bool tumble_or_swim = (wip_phys_state == WIP_PHYS_TUMBLE) || (wip_phys_state == WIP_PHYS_SWIM);
	if ((wip_phys_state == WIP_PHYS_IMPACT) || tumble_or_swim)
	{
		if (tumble_or_swim)
			wip_position.y = collide.position.y;
		else
			wip_position = collide.position;

		wip_position += YVEC*get_floor_offset();
	}

	//  Reorient the surfer's root to the wave surface, or if tumbling or swimming to the up vector
	bool up_dir = (tumble_or_swim || my_board_controller.exit_jump);
	OrientToWave(up_dir, dt, SET_OWNER_PO);

	if (tumble_or_swim)
	{
		if (wip_position.y > low_wip_y_height)
			wip_position.y = low_wip_y_height;
		else if (wip_position.y < low_wip_y_height)
			low_wip_y_height = wip_position.y;

		if (CtrlEvent(PAD_OLLIE) && CtrlDelta(PAD_OLLIE))
		{
			get_owner()->SetCull(true);
			my_board_controller.SetWipeoutDone();
			get_on_board_anim = SURFER_ANIM_W_1_R_GOB;
			b_get_on_board_anim = BOARD_ANIM_W_1_R_GOB;
		}
	}

	// reset the po based on possible maximum height limit
	get_owner()->set_rel_position(wip_position);
}

float g_intensity_height = 5.8f;
float g_intesity2_wip = 0.0f;
int kellyslater_controller::GetWipeoutIntensity(void)
{
	if (g_intesity2_wip)
		return WIP_INT_MED;

	float wave_height = WAVE_GetMarker(WAVE_MarkerEmitterStartCrestX)->y - WAVE_GetMarker(WAVE_MarkerSurferSpawn)->y;
	float wave_x = WAVE_GetMarker(WAVE_MarkerLipMark11p45)->x;
	int current_beach = g_game_ptr->get_beach_id ();
	bool in_boring_region = false;
	bool right = BeachDataArray[current_beach].bdir;
	float tube_dist = Closest_Tube_Distance();
	float x_pos = get_owner()->get_abs_position().x;

	if ((right && (x_pos < wave_x)) || (!right && (x_pos > wave_x)))
		in_boring_region = true;

	if (in_boring_region && (tube_dist > 0.9f))
		return WIP_INT_LOW;
	else if (wave_height > g_intensity_height)
	{
		if (tube_dist < 0.3f)
			return WIP_INT_EXT;

		return WIP_INT_MED;
	}
	else
	{
		if (tube_dist < 0.3f)
			return WIP_INT_MED;

		return WIP_INT_LOW;
	}
}

void kellyslater_controller::InitKlugeWipeout(int tumble_anim, int swim_anim)
{
	my_scoreManager.FinishChain(true);
	surfer_velocity = ZEROVEC;
	wip_position = ((conglomerate *)get_owner())->get_member("BIP01 PELVIS")->get_abs_position();
	low_wip_y_height = wip_position.y - 1.5f;

	state = STATE_WIPEOUT_GENERAL;
	super_state = SUPER_STATE_WIPEOUT;

	wip_anim_trans = tumble_anim;
	wip_anim_swim = swim_anim;
	b_wip_anim_dis = BOARD_ANIM_NULL;

	tween_timer = 0.0f;
	tween_time = 0.3f;

	water_normal = my_board_controller.GetWaveNormal();

	wip_phys_state = WIP_PHYS_TUMBLE;
	dry = false;
	wave_hint_valid = false;
	water_region = (WaveRegionEnum) my_board_controller.GetWaveRegion();
	water_current = my_board_controller.GetWaveCurrent();

	if (!IsAIPlayer())
		get_owner()->SetCull(false);
}

void kellyslater_controller::InitWipeout(int wip_type)
{
	wipeout_type = wip_type;
	vector3d delta_vel = ZEROVEC;
	surfer_velocity = ZEROVEC;
	water_normal = my_board_controller.GetWaveNormal();
	wip_position = ((conglomerate *)get_owner())->get_member("BIP01 PELVIS")->get_abs_position();
	int vel_type = GWipeoutData[wip_type].input_vel;
	bool last_state_air = (last_super_state == SUPER_STATE_AIR);

	if (vel_type & DIR_DATA_FLAG)
	{
		delta_vel = GWipeoutData[wip_type].xvel*XVEC + GWipeoutData[wip_type].yvel*YVEC + GWipeoutData[wip_type].zvel*ZVEC;
		if (last_state_air && !my_board_controller.lip_jump)
		{
			delta_vel.z = 0.0f;
			my_board_controller.rb->linMom *= 0.4f;
		}
	}

	if ((vel_type & BOARD_DIR_FLAG) && (vel_type & BOARD_RIGHT_FLAG))
	{
		vector3d dir_vec = my_board_controller.GetForwardDir() + (goofy?-1.0:1.0f)*my_board_controller.GetRightDir();
		dir_vec.normalize();
		delta_vel += dir_vec*GWipeoutData[wip_type].dir_vel;
	}
	else if ((vel_type & BOARD_NEG_DIR_FLAG) && (vel_type & BOARD_RIGHT_FLAG))
	{
		vector3d dir_vec = -my_board_controller.GetForwardDir() + (goofy?-1.0:1.0f)*my_board_controller.GetRightDir();
		dir_vec.normalize();
		delta_vel += dir_vec*GWipeoutData[wip_type].dir_vel;
	}
	else if (vel_type & BOARD_DIR_FLAG)
		delta_vel += my_board_controller.GetForwardDir()*GWipeoutData[wip_type].dir_vel;
	else if (vel_type & BOARD_RIGHT_FLAG)
		delta_vel += (goofy?-1.0:1.0f)*my_board_controller.GetRightDir()*GWipeoutData[wip_type].dir_vel;

	if (vel_type & MOM_DIR_FLAG)
	{
		vector3d mom_dir = my_board_controller.rb->linMom;
		mom_dir.normalize();
		delta_vel += mom_dir*GWipeoutData[wip_type].dir_vel;
	}

	if (vel_type & MOM_ADDITIVE_FLAG)
	{
		surfer_velocity = my_board_controller.rb->linMom;
		surfer_velocity += delta_vel;
	}
	else
		surfer_velocity = delta_vel;

	surfer_velocity.y =  delta_vel.y;

	float dot_surf_vel = dot(surfer_velocity, water_normal);
	if (dot_surf_vel <= 0.3f)
	{
		surfer_velocity -= water_normal*dot_surf_vel;
		surfer_velocity += 0.3f*water_normal;
	}

	state = STATE_WIPEOUT_GENERAL;
	super_state = SUPER_STATE_WIPEOUT;
	
	wip_anim_dis = GWipeoutData[wip_type].wip_anim_dis;
	wip_anim_free = GWipeoutData[wip_type].wip_anim_free;
	wip_anim_imp = GWipeoutData[wip_type].wip_anim_imp;
	wip_anim_imp2 = GWipeoutData[wip_type].wip_anim_imp2;
	wip_anim_trans = GWipeoutData[wip_type].wip_anim_trans;
	wip_anim_swim = GWipeoutData[wip_type].wip_anim_swim;
	b_wip_anim_dis = GWipeoutData[wip_type].b_wip_anim_dis;
	b_wip_anim_free = GWipeoutData[wip_type].b_wip_anim_free;
	b_wip_anim_imp = GWipeoutData[wip_type].b_wip_anim_imp;


	tween_timer = 0.0f;
	tween_time = 0.3f;

	if (GWipeoutData[wipeout_type].input_vel & DISMOUNT_NO_AIR_FLAG)
		wip_phys_state = WIP_PHYS_DISMOUNT2;
	else
		wip_phys_state = WIP_PHYS_DISMOUNT;
	dry = true;

	wave_hint_valid = false;
	water_region = (WaveRegionEnum) my_board_controller.GetWaveRegion();
	water_current = my_board_controller.GetWaveCurrent();
}

extern float g_frame_by_frame;
float g_debug_mode_doik = 0.0f;
void kellyslater_controller::PerformIK(void)
{
	bool ik_enable = IK_enabled;
	my_ik_object->SetFloorObj(my_board_member);
	if ((state == STATE_AIR_TRICK) || (state == STATE_TUBE_TRICK) || this->IsDoingTrick())
	{
		int flags = GTrickList[currentTrick].IK_flags;

		/*if (state == STATE_AIR_TRICK)
			flags = GTrickList[currentTrick].IK_flags;
		else if (state == STATE_TUBE_TRICK)
				flags = GTrickList[last_tube_trick].IK_flags;
		else
			flags = GTrickList[currentTrick].IK_flags;*/

		if (flags & DoIkFlag)
		{
			if (flags & IkBlendFlag)
			{
				if (AnimBlended())
					ik_enable = false;
				else
					ik_enable = true;
			}
			else
				ik_enable = true;

			if (flags & BoardNodeFlag)
				my_ik_object->SetFloorObj(my_parent_node);
		}
		else
			ik_enable = false;
	}
	else if (debug_mode)
	{
		if (g_debug_mode_doik)
		{
			if (g_frame_by_frame && (CtrlEvent(PSX_CIRCLE) || CtrlEvent(PSX_SQUARE)))
				ik_enable = true;
			else
				ik_enable = false;
		}
		else
			ik_enable = false;
	}

	if (ik_enable)
		my_ik_object->PerformIK();
}

float g_tube_gap1 = 0.15f;
float g_tube_gap2 = 0.35f;
void kellyslater_controller::DetermineTubeGaps(void)
{
	int region = my_board_controller.GetWaveRegion();
	if ((region != WAVE_REGIONTUBE) || !this->Z_Within_Tube())
	{
		passed_first_tube_thresh = false;
		passed_second_tube_thresh = false;
		return;
	}

	int current_beach = g_game_ptr->get_beach_id ();

	vector3d tube_start = *WAVE_GetMarker(WAVE_MarkerTubeGapEnd) - *WAVE_GetMarker(WAVE_MarkerTubeGapStart);
	float length = tube_start.length();
	if (length > 0.0f)
		tube_start /= length;

	vector3d tube_pos = get_owner()->get_abs_position() - *WAVE_GetMarker(WAVE_MarkerTubeGapStart);
	float perc = dot(tube_pos, tube_start);

	//  check position in the tube to see if the surfer just passed a tube threshhold.
	if ((perc > g_tube_gap2*length) && !passed_second_tube_thresh && passed_first_tube_thresh)
	{
		my_scoreManager.AddGap(GAP_TUBE_MID_ANTARCTICA + (current_beach * 2) + 1);
		passed_second_tube_thresh = true;
		SoundScriptManager::inst()->playEvent(SS_TUBE_DEEP);
	}

	if ((perc > g_tube_gap1*length) && !passed_first_tube_thresh)
	{
		my_scoreManager.AddGap(GAP_TUBE_MID_ANTARCTICA + (current_beach * 2));
		passed_first_tube_thresh = true;
		SoundScriptManager::inst()->playEvent(SS_TUBE_MID);
	}
}

//  Positive numbers mean tell how far you are from the tube threshhold.  Negative numbers tell what percentage
//  (0.0 to 1.0) distance you are between the tube threshhold and the actual start of the tube.
//  0.0 means that the surfer is completely in the tube.
float kellyslater_controller::Tube_Distance()
{
	//  get the distance of the threshold that is considered "close" to the tube.
	vector3d tube_start = *WAVE_GetMarker(WAVE_MarkerBreak);
	vector3d tube_tip = *WAVE_GetMarker(WAVE_MarkerLipMark10);
	float tube_offset = 1.5f * fabs(tube_start.x - tube_tip.x);
	int EmitterType = WAVE_SE_TUBE;
	if (WAVE_SoundEmitter[WAVE_SE_TUBE].numsegment == 0)
	{
		EmitterType = WAVE_SE_SPLASH;
	}
	

	float board_x = my_board->get_abs_position ().x;
	// Different directions mean different points to measure the distance from.
	if (BeachDataArray[g_game_ptr->get_beach_id()].bdir)
	{
		if (board_x > WAVE_SoundEmitter[EmitterType].line.start.x - tube_offset)
		{
			if (board_x > WAVE_SoundEmitter[EmitterType].line.start.x)  //  completely in the tube.
			{
				float in_tube_dist = board_x - WAVE_SoundEmitter[EmitterType].line.start.x;
				float in_tube_dist_div = (WAVE_SoundEmitter[EmitterType].line.stop.x - WAVE_SoundEmitter[EmitterType].line.start.x);
				if ( in_tube_dist_div==0 )
					in_tube_dist = 1.0f;
				else
					in_tube_dist = in_tube_dist/in_tube_dist_div;
				return max(-2.0f, -(in_tube_dist + 1));  //  in the tube ranges from -1 to -2.
			}
			else
			{
				float total_dist = WAVE_SoundEmitter[EmitterType].line.start.x - board_x;
				total_dist = (total_dist/tube_offset) - 1;
				return total_dist;
			}
		}
		else  //  Within the tube offset but not in the tube yet.
		{
			float total_dist = (WAVE_SoundEmitter[EmitterType].line.start.x - tube_offset) - board_x;
			total_dist /= WAVE_SoundEmitter[EmitterType].line.start.x - WAVE_MeshMinX;
			return min(1.0f, total_dist);
		}
	}
	else
	{
		if (board_x < WAVE_SoundEmitter[EmitterType].line.stop.x + tube_offset)
		{
			if (board_x < WAVE_SoundEmitter[EmitterType].line.stop.x)  //  completely in the tube.
			{
				float in_tube_dist = WAVE_SoundEmitter[EmitterType].line.stop.x - board_x;
				in_tube_dist = in_tube_dist/(WAVE_SoundEmitter[EmitterType].line.stop.x - WAVE_SoundEmitter[WAVE_SE_TUBE].line.start.x);
				return max(-2.0f, -(in_tube_dist + 1));  //  in the tube ranges from -1 to -2.
			}
			else
			{
				float total_dist = board_x - WAVE_SoundEmitter[EmitterType].line.stop.x;
				total_dist = (total_dist/tube_offset) - 1;
				return total_dist;
			}
		}
		else  //  Within the tube offset but not in the tube yet.
		{
			float total_dist = board_x - (WAVE_SoundEmitter[EmitterType].line.stop.x + tube_offset);
			total_dist /= WAVE_MeshMaxX - WAVE_SoundEmitter[EmitterType].line.stop.x;
			return min(1.0f, total_dist);
		}
	}
}

float transition_length_mod = 2.1f;

//  Distance to the closest tube.
//  0 means completely within a tube, 1 means far away from tube.  Inbetween ranges give percentage within threshhold of tube.
//  0 to -1 is depth inside the tube.
//  The main_tube flag tells whether this is a litle tube or the main tube.
float kellyslater_controller::Closest_Tube_Distance(int *main_tube, vector3d *closest_point, float x_pos)
{
	//  get the distance of the threshold that is considered "close" to the tube.
	bool left = BeachDataArray[g_game_ptr->get_beach_id()].bdir;
	vector3d tube_start = *WAVE_GetMarker(WAVE_MarkerBreak);
	vector3d tube_tip = *WAVE_GetMarker(WAVE_MarkerLipMark10);
	float tube_offset = transition_length_mod * fabs(tube_start.x - tube_tip.x);

	float closest_so_far = 1.0f;  //  Check each tube and keep track of which one is closest.
	float which_tube = 0;

	//  default to the sufer's position.
	if (x_pos == 1000000)
		x_pos = my_board->get_abs_position ().x;

	//  Check each tube.
	for (u_int current_tube = 0; current_tube < WAVE_SoundEmitter[WAVE_SE_TUBE].numsegment; current_tube++)
	{
		//  first check to see if the position is completely inside the tube.
		if (x_pos >= WAVE_SoundEmitter[WAVE_SE_TUBE].segment[current_tube].start.x &&
			x_pos <= WAVE_SoundEmitter[WAVE_SE_TUBE].segment[current_tube].stop.x)
		{
			if (fabs(WAVE_SoundEmitter[WAVE_SE_TUBE].segment[current_tube].start.x - x_pos) >
				fabs(WAVE_SoundEmitter[WAVE_SE_TUBE].segment[current_tube].stop.x - x_pos))
			{
				if (closest_point)
					*closest_point = WAVE_SoundEmitter[WAVE_SE_TUBE].segment[current_tube].stop;
			}
			else
			{
				if (closest_point)
					*closest_point = WAVE_SoundEmitter[WAVE_SE_TUBE].segment[current_tube].start;
			}

			if (main_tube != NULL)
			{
				if (WAVE_SoundEmitter[WAVE_SE_TUBE].numsegment == 1 ||
					(left && current_tube == 1) ||
					(!left && current_tube == 0))
					*main_tube = true;
				else
					*main_tube = false;
			}

			float depth = (x_pos - WAVE_SoundEmitter[WAVE_SE_TUBE].segment[current_tube].start.x)/
								(WAVE_SoundEmitter[WAVE_SE_TUBE].segment[current_tube].stop.x - WAVE_SoundEmitter[WAVE_SE_TUBE].segment[current_tube].start.x);

			if (left)
				return -depth;
			else
				return depth - 1;
		}

		//  Now look at the points and see which is closest.
		float dist;

		dist = fabs((WAVE_SoundEmitter[WAVE_SE_TUBE].segment[current_tube].start.x - x_pos)/tube_offset);
		if (dist < closest_so_far)
		{
			closest_so_far = dist;
			if (closest_point)
			{
				*closest_point = WAVE_SoundEmitter[WAVE_SE_TUBE].segment[current_tube].start;
				which_tube = current_tube;
			}
		}

		dist = fabs((x_pos - WAVE_SoundEmitter[WAVE_SE_TUBE].segment[current_tube].stop.x)/tube_offset);
		if (dist < closest_so_far)
		{
			closest_so_far = dist;
			if (closest_point)
			{
				*closest_point = WAVE_SoundEmitter[WAVE_SE_TUBE].segment[current_tube].stop;
				which_tube = current_tube;
			}
		}
	}

	if (main_tube != NULL)
	{
		if (WAVE_SoundEmitter[WAVE_SE_TUBE].numsegment == 1 ||
			(left && which_tube == 1) ||
			(!left && which_tube == 0))
			*main_tube = true;
		else
			*main_tube = false;
	}

	return closest_so_far;  //  This is the closest that the position got to any of the tubes.
}

//  Set up everything for a flyby of the beach.
bool kellyslater_controller::StartFlyby()
{
	//  Check to make sure that the user is allowing flybys.
	if (os_developer_options::inst()->is_flagged (os_developer_options::FLAG_NOFLYBY))
		return false;

	if (!flyby_cam_ptr->load())  //   Don't do anything if the file isn't there.
		return false;

	flyby_cam_ptr->start();
	SetPlayerCamera(flyby_cam_ptr);

	if (os_developer_options::inst()->is_flagged (os_developer_options::FLAG_MAKE_MOVIE) || 
		g_game_ptr->get_beach_id () == BEACH_OPENSEA)
	{
		g_debug.halt_on_asserts = 0;
		frontendmanager.IGO->SetDisplay(false);
	}

	exit_state = true;

	super_state = SUPER_STATE_FLYBY;
	state = STATE_FLYBY;

	get_owner()->set_visible(false);
	my_board_model->set_visible(false);
	flyby_first = true;

	return true;
}

void kellyslater_controller::SuperStateFlyby(float dt)
{
	input_mgr* inputmgr=input_mgr::inst();

	// if we're in career mode, display the level goals....
	if (g_game_ptr->get_game_mode() == GAME_MODE_CAREER && !dmm.inDemoMode() &&
		frontendmanager.pms->active != PauseMenuSystem::GoalsMenu &&
		frontendmanager.map->IsSlideOutDone())
	{
		frontendmanager.pms->startDraw(PauseMenuSystem::GoalsMenu, false);
		if (frontendmanager.IGO->GetTutorialManager())
			frontendmanager.IGO->GetTutorialManager()->PlayCurrentVO(true);
	}

	// make sure that if you skip the beach movie on the XB you don't skip the flyby.
	if (flyby_first)
	{
		if ( inputmgr->get_control_state( JOYSTICK_DEVICE,PSX_X) == AXIS_MAX ||
			inputmgr->get_control_state( JOYSTICK_DEVICE,PSX_CIRCLE) == AXIS_MAX ||
			inputmgr->get_control_state( JOYSTICK_DEVICE,PSX_SQUARE) == AXIS_MAX ||
			inputmgr->get_control_state( JOYSTICK_DEVICE,PSX_TRIANGLE) == AXIS_MAX ||
			inputmgr->get_control_state( JOYSTICK_DEVICE,PSX_START) == AXIS_MAX )
		{
			flyby_key_down = true;
		}

		flyby_first = false;
	}

	if (flyby_key_down)
	{
		if (inputmgr->get_control_state( JOYSTICK_DEVICE,PSX_X) != AXIS_MAX &&
			inputmgr->get_control_state( JOYSTICK_DEVICE,PSX_CIRCLE) != AXIS_MAX &&
			inputmgr->get_control_state( JOYSTICK_DEVICE,PSX_SQUARE) != AXIS_MAX &&
			inputmgr->get_control_state( JOYSTICK_DEVICE,PSX_TRIANGLE) != AXIS_MAX &&
			inputmgr->get_control_state( JOYSTICK_DEVICE,PSX_START) != AXIS_MAX )
		{
			flyby_key_down = false;
		}
	}

	//  Check to see if the user wants to exit this state, if if it is over.
	bool hitStart = CtrlEvent(PSX_START);
	if (!flyby_key_down && (flyby_cam_ptr->is_finished() || CtrlEvent(PAD_OLLIE) || hitStart))
	{
		// Unpause a bunch of stuff
		rumbleMan.unpause();

		if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
		{
			nslUnpauseAllSounds();
			MusicMan::inst()->unpause();
			SoundScriptManager::inst()->unpause();
			sfx.unpause();
			wSound.unpause();
			VoiceOvers.unpause();
		}

		my_board_controller.ResetPhysics();   //  This will reset the wave and change the super state to lie on the board.
		// ks::SuperStateFlyby(): flyby only works for 1 player (multiplayer fixme?)
		SetPlayerCamera((game_camera *) find_camera(StoredConfigData::inst()->getLastCamera(get_player_num())));
		TIMER_Reset();  //  Reset the level timer back so that the wave will start breaking again.
		ks_fx_reset();  //  reset the wave particles.

		//  Get rid of all the objects that have been spawned
		g_beach_ptr->reset ();

		WAVE_EndWave();  //  Reset the wave.

    // Reset the random numbers and the wave schedule. This is needed to correctly recreate the wave in replay.
	  ksreplay.Clear(g_random_r_ptr->srand());
    ksreplay.Record();
    WAVE_ResetSchedule();

		// turn off the level goals....
		frontendmanager.IGO->SetDisplay(frontendmanager.score_display);
		if (g_game_ptr->get_game_mode() == GAME_MODE_CAREER && !dmm.inDemoMode())
		{
			frontendmanager.pms->endDraw();
			if (hitStart)
			{
//#if !defined(TARGET_XBOX)
				frontendmanager.pms->pause_controller = DEVICE_ID_TO_JOYSTICK_INDEX( get_joystick_num() );
				if (frontendmanager.IGO->GetTutorialManager())
					frontendmanager.IGO->GetTutorialManager()->Update(dt);				
				else
					frontendmanager.pms->startDraw();
//#endif
				frontendmanager.pms->UpdateButtonDown();
			}
		}

		get_owner()->set_visible(true);
		my_board_model->set_visible(true);

		//  Start off with the right animations.
		Anim(SURFER_ANIM_IDLE_LAYING, 0.0f);		
		BoardAnim(BOARD_ANIM_IDLE_LAYING, 0.0f);

		if (g_game_ptr->get_game_mode() != GAME_MODE_CAREER || g_game_ptr->get_beach_id() != BEACH_INDOOR)
			BLUR_TurnOn();
	}

	if (os_developer_options::inst()->is_flagged (os_developer_options::FLAG_MAKE_MOVIE))
	{

		g_screenshot = true;
	}

	IK_enabled = false;
	exit_state = true;
	rideTimer = 0.0f;
}


float tube_hardness = 3.0f;
float tube_tilt_towards_beach = -0.2f;  //  tilt the board just a little bit towards the beach.

//  Turn the board towards the end of the tube, but with the front still slanted slightly towards the beach.
void kellyslater_controller::Tube_Align(float z_offset, float spin_rate)
{
	vector3d alignment_vec;

	z_offset += tube_tilt_towards_beach;  //  The default offset from the Z-axis.

	if ( my_board_controller.GetForwardDir().x > 0)
		alignment_vec = XVEC + (z_offset * ZVEC);
	else
		alignment_vec = -XVEC + (z_offset * ZVEC);

	alignment_vec.normalize();

	spin_ctrl.SetSpinType(SPIN_FLOATER, 0.0f, alignment_vec, spin_rate);
}

void kellyslater_controller::StartTube()
{
	int current_beach = g_game_ptr->get_beach_id ();

	last_need_extra_balance = false;
	last_tube_trick = -1;
	super_state = SUPER_STATE_IN_TUBE;
	depth_meter_filter.Init_Filter(0);

	//  Small beaches must grab the board all the time.
	if (BeachDataArray[current_beach].is_small_wave)
		state = STATE_TUBE_RAILGRAB;
	else
		state = STATE_TUBE_STAND;

	Tube_Align();

	tube_meter.Init(my_player_num, false, tube_hardness, GetAttribs(ATTRIB_BALANCE), 
					!my_scoreManager.GetChain().GetTrickCount(TRICK_TUBE_RIDE));
	tube_balance = 0.0f;
	//  Temporarilly commented out while this is used for the special meter timer.  frontendmanager.IGO->TurnOnTubeTimer(my_player_num, true);
	frontendmanager.IGO->TurnOnTubeIndicator(my_player_num, false);
	SetTrickRegion(TREGION_TUBE);
	my_scoreManager.AddTrick(TRICK_TUBE_RIDE);
	trick_complete = true;

	//  Show that the surfer hasn't moved back in the tube yet.
	passed_first_tube_thresh = false;
	passed_second_tube_thresh = false;
	for (int index = 0; index < NUM_TUBE_DIFFICULTY_LEVELS; index++)
		passed_tube_difficulty_level[index] = false;
	tube_ride_time = 0.0f;
	bDoingTrick = true;
	last_direction = -1;
	IK_enabled = true;

	just_started = 0;
	out_of_tube_time = 0.0f;
}

#define TUBE_ACCELERATION 0.09f
#define SMALL_TUBE_ACCELERATION 0.05f  //  This actually makes the surfer go backwards.
#define MAX_OUT_OF_TUBE_TIME 0.5f		//  A little buffer for being out of the tube.  This helps with the wave stepping.

void kellyslater_controller::EndTube()
{
	my_board_controller.state = BOARD_NONE;
	//  Temporarilly commented out while this is used for the special meter timer.  frontendmanager.IGO->TurnOnTubeTimer(my_player_num, false);
	set_state (STATE_STAND);
	set_super_state (SUPER_STATE_NORMAL_SURF);
	tube_meter.End();
	if (ks_fx_spit_going_on())  //  Make the guy move faster is tube isspitting when he leaves)
		my_board_controller.MoveForward(TUBE_ACCELERATION * 2);
	bDoingTrick = false;
}


void kellyslater_controller::SetTubeTrick(int trick, int anim, int banim)
{
	tube_trick = trick;
	tube_anim = anim;
	tube_board_anim = banim;
	last_tube_trick = -1;
	left_stick_pressed = false;
	current_trick_time = 0.0f;
}

bool kellyslater_controller::Z_Within_Tube()
{
	vector3d tub_wall_point = *WAVE_GetMarker(WAVE_MarkerLipCrash);
	return my_board->get_abs_position ().z >= tub_wall_point.z;
}


float osc_rate = 60.0f;
float osc_thresh = 0.1f;
bool debug_osc = true;
float tube_depth_factor = 0.3f;  //  0.5 seems like a good value, maybe use less for the press demo.
float blend_fudge = 1.4f;
float small_tube_stall_mod = 0.8f;
float tube_board_normal_thresh_small = 0.25f;
float tube_board_normal_thresh_medium = 0.20f;
float tube_board_normal_thresh_large = 0.15f;
float balance_mult = 4.0f;
float tube_meter_filter_val = 0.8f;
float depth_fudge = 0.2f;  //  This makes the surfer grab his rail just a little before getting into the final region of the wave.
float high_wall_quickcut_degree = 1.4f;

void kellyslater_controller::SuperStateInTube(float dt)
{
	int current_balance_anim;
	bool currently_need_extra_balance = false;

	if (!just_started)
	{
		just_started = 1;
		last_state = STATE_NOSTATE;
	}

	//  Check to see if the surfer is exiting the tube.
	//  Give a little buffer so that the stepping of the wave won't shove the surfer in and out of the tube.
	int region = my_board_controller.GetWaveRegion();
	if (region != WAVE_REGIONTUBE || !Z_Within_Tube())
	{
		out_of_tube_time += dt;
		if (out_of_tube_time > MAX_OUT_OF_TUBE_TIME)
		{
			my_board_controller.ResetTimers();
			EndTube();
			return;
		}
	}
	else
		out_of_tube_time = 0.0f;

	float vert_stick = GetStick(PAD_LSTICK_V);
	int current_beach = g_game_ptr->get_beach_id ();
	int current_wave = WAVE_GetIndex ();
	bool left_wave = BeachDataArray[current_beach].bdir;
	float current_depth;  //  0 to 1 of how far in the tube the surfer is.  0 if not main tube.
	float current_total_depth;  //  0 to 1 of the total possible depth in the tube.
	bDoingTrick = true;

	//  Check to see how far into the tube the surfer is.  If it is not the main tube, then pretend he's at the start.
	int main_tube;
	current_depth = -Closest_Tube_Distance(&main_tube);
	if (!main_tube)
		current_depth = 0;

	//  Certain tricks may need to play a different animation based on position and handedness.
	float handedness_offset = 0;
	if (last_tube_trick == TRICK_ONE_HAND_DRAG || tube_trick == TRICK_ONE_HAND_DRAG ||
			last_tube_trick == TRICK_GRAB_DRAG_TRICK || tube_trick == TRICK_GRAB_DRAG_TRICK ||
			last_tube_trick == TRICK_FOOT_DRAG || tube_trick == TRICK_FOOT_DRAG ||
			last_tube_trick == TRICK_BEHIND_BACK_DRAG || tube_trick == TRICK_BEHIND_BACK_DRAG)
	{
		float swap_hands = false;

		//  Check to see if we are playing a trick that needs to have an animation for the oposite side.
		if (left_wave)
			swap_hands = !swap_hands;

		//  Check to see if the surfer is goofy footed and the animation needs to be on the opposite side.
		if (goofy)
			swap_hands = !swap_hands;

		if (swap_hands)
			handedness_offset += 7;
	}
	else if (goofy && (last_tube_trick == TRICK_QUICK_CUT_LEFT || tube_trick == TRICK_QUICK_CUT_LEFT))
		handedness_offset = 1;
	else if (goofy && (last_tube_trick == TRICK_QUICK_CUT_RIGHT || tube_trick == TRICK_QUICK_CUT_RIGHT))
		handedness_offset = -1;


	stick = GetStick(PAD_LSTICK_H);

	last_super_state = super_state;

	tube_ride_time += dt;
	//frontendmanager.IGO->SetTubeTimer(my_player_num, tube_ride_time);

	SetTrickRegion(TREGION_TUBE);

	current_balance_anim = (int)(balance_mult * tube_balance);  //  figure out which animation based on the balance.

	if (goofy)
		current_balance_anim = -current_balance_anim;

	//  check position in the tube to see if the surfer just passed a tube threshhold.
	if (current_depth > WaveDataArray[current_wave].firsttubethresh && !passed_first_tube_thresh && !BeachDataArray[current_beach].is_small_wave)
	{
		my_scoreManager.AddGap(GAP_TUBE_MID_ANTARCTICA + (current_beach * 2));
		passed_first_tube_thresh = true;
	}

	if (current_depth > WaveDataArray[current_wave].secondtubethresh && !passed_second_tube_thresh && !BeachDataArray[current_beach].is_small_wave)
	{
		my_scoreManager.AddGap(GAP_TUBE_MID_ANTARCTICA + (current_beach * 2) + 1);
		passed_second_tube_thresh = true;
	}

	//  Figure out how close to the last thresh the surfer is.
	current_total_depth = current_depth / WaveDataArray[current_wave].knockdowntubethresh;

	//  Show that the user has let go of the pad after doing a trick.
	if (!stick && !vert_stick)
		left_stick_pressed = false;

	if ((CtrlEvent(PAD_GRIND) && left_stick_pressed) || (stick >= -0.1f && stick <= 0.1f))
		tube_balance = tube_meter.Update(0, dt);
	else if (stick > 0.1f)
		tube_balance = tube_meter.Update(1, dt);    //  Right direction was pressed, move meter over right.
	else if (stick < -0.1f)
		tube_balance = tube_meter.Update(-1, dt);   //  Left direction was pressed, move meter over left.

	//  Too far, knock the surfer over.
	float wave_normal = my_board_controller.GetWaveNormal().y;
	if (tube_balance > 1.0f)
	{
		tube_meter.End();
		//  Temporarilly commented out while this is used for the special meter timer.  frontendmanager.IGO->TurnOnTubeTimer(my_player_num, false);
		//  this should be falling left off of the board
		my_board_controller.DoWipeOut(goofy?WIP_TUBE_LOSE_BALANCE_LEFT:WIP_TUBE_LOSE_BALANCE_RIGHT);  //  hit right
		if (!IsAIPlayer())
			frontendmanager.IGO->GetHintManager()->SetHint(IGOHintManager::LostBalance);
	}
	else if (tube_balance < -1.0f)
	{
		tube_meter.End();
		//  Temporarilly commented out while this is used for the special meter timer.  frontendmanager.IGO->TurnOnTubeTimer(my_player_num, false);
		//  this should be falling right off of the board
		my_board_controller.DoWipeOut(goofy?WIP_TUBE_LOSE_BALANCE_RIGHT:WIP_TUBE_LOSE_BALANCE_LEFT);  // hit left
		if (!IsAIPlayer())
			frontendmanager.IGO->GetHintManager()->SetHint(IGOHintManager::LostBalance);
	}
	else if (state != STATE_TUBE_RAILGRAB &&
			((BeachDataArray[current_beach].is_small_wave && wave_normal < tube_board_normal_thresh_small) ||
			(BeachDataArray[current_beach].is_big_wave && wave_normal < tube_board_normal_thresh_large) ||
			(!BeachDataArray[current_beach].is_small_wave && !BeachDataArray[current_beach].is_big_wave && wave_normal < tube_board_normal_thresh_medium)))
	{
		//  The problem here is that the surfer got too high on the tube wall, either by quickcutting or entering the tube that way.

		tube_meter.End();
		//  this should be falling down the face of the wave, which may be to the left or to the right, 
		//  depending on whether the wave is a left or a right.
		my_board_controller.DoWipeOut(my_board_controller.CalculateCeilingWipeout());  
		if (!IsAIPlayer())
			frontendmanager.IGO->GetHintManager()->SetHint(IGOHintManager::TooHighOnTubeWall);
	}

	//  Check forward and back movement, but not when a trick has just been started.
	int direction;  //  Record which way he was going for later.
	if (((CtrlEvent(PAD_OLLIE) || vert_stick < -.25f ) && !(CtrlEvent(PAD_GRIND) && left_stick_pressed)) ||
		tube_trick == TRICK_SPIT_EJECT || last_tube_trick == TRICK_SPIT_EJECT)
	{
		my_board_controller.MoveForwardOnX(TUBE_ACCELERATION);		//  Move forwards.
		if (!BeachDataArray[current_beach].is_small_wave)
			currently_need_extra_balance = true;  //  More balance when trying to leave the tube.
		direction = 1;
	}
	else if (current_depth > WaveDataArray[current_wave].knockdowntubethresh)  
	{
		my_board_controller.MoveForwardOnX(2 * TUBE_ACCELERATION);  //  If the surfer is too far back in the tube, push him forward quickly.
		direction = -1;
	}
	else if (current_depth > WaveDataArray[current_wave].secondtubethresh)  
	{
		my_board_controller.StopOnWave();  //  If the surfer is pretty far back in the tube, then stop him.
		direction = 0;
	}
	else if (vert_stick > 0.25f && !(CtrlEvent(PAD_GRIND) && left_stick_pressed))
	{
		my_board_controller.MoveForwardOnX(-TUBE_ACCELERATION);		//  Move backwards.
		direction = -1;
	}
	else
	{
		my_board_controller.StopOnWave();
		direction = 0;
	}

	switch (state)
	{
	case STATE_TUBE_STAND:

		currentTrick = -1;
		if (last_state != state)
		{
			if (direction == 0)  //  standing
				Anim(SURFER_ANIM_TRK_TUBE_FS_1 + current_balance_anim, BLEND_TIME);
			else if (direction == -1)  //  moving backwards
				Anim(SURFER_ANIM_TRK_TUBE_FS_1 + current_balance_anim, BLEND_TIME);
			else  //  moving forwards
				Anim(SURFER_ANIM_TRK_TUBE_SPEED_1 + current_balance_anim, BLEND_TIME);

			BoardAnim(BOARD_ANIM_ZERO, BLEND_TIME, false);
			last_state = state;
		}
		else
		{
			//  check to see if the balance has changed enough to change the animation.
			if (direction == 0)  //  standing
			{
				if (last_balance_anim != current_balance_anim || direction != last_direction)
					Anim(SURFER_ANIM_TRK_TUBE_FS_1 + current_balance_anim, BLEND_TIME * blend_fudge);
			}
			else if (direction == -1)  //  moving backwards
			{
				if (last_balance_anim != current_balance_anim || direction != last_direction)
					Anim(SURFER_ANIM_TRK_TUBE_FS_1 + current_balance_anim, BLEND_TIME * blend_fudge);
			}
			else  //  moving forwards
			{
				if (last_balance_anim != current_balance_anim || direction != last_direction)
					Anim(SURFER_ANIM_TRK_TUBE_SPEED_1 + current_balance_anim, BLEND_TIME * blend_fudge);
			}
		}

		//  make the board wiggle back and forth a bit, just to look nice.
		if (!spin_ctrl.IsActivated() && debug_osc)
		{
			if (oscilate_up)
			{
				Tube_Align(osc_thresh, osc_rate);
				oscilate_up = false;
			}
			else
			{
				Tube_Align(-osc_thresh, osc_rate);
				oscilate_up = true;
			}
		}

		my_scoreManager.UpdateLastTrick(ScoringManager::ATTR_TIME_DELTA, dt, NormalRideFlag);

		if (CtrlEvent(PAD_GRAB) || current_depth > WaveDataArray[current_wave].secondtubethresh) //  surfer needs to grab his rail, possibly because he's too far back in the tube.
			state = STATE_TUBE_RAILGRAB;
		else if (CtrlEvent(PAD_HARD_LEFT) && tube_trick == -1 && last_tube_trick == -1)
			my_trickManager.ManuallyQueueTrick(&GTrickList[TRICK_QUICK_CUT_LEFT]);
		else if (CtrlEvent(PAD_HARD_RIGHT) && tube_trick == -1 && last_tube_trick == -1)
			my_trickManager.ManuallyQueueTrick(&GTrickList[TRICK_QUICK_CUT_RIGHT]);
		else if (tube_trick >= 0)  //  Check to see if a trick just started.
		{
			left_stick_pressed = true;
			if (GTrickList[tube_trick].flags & BalanceBlendFlag)
				Anim(tube_anim + current_balance_anim + handedness_offset,
					 BLEND_TIME, GTrickList[tube_trick].flags & ManualFlag);
			else
				Anim(tube_anim + handedness_offset,
					 BLEND_TIME, GTrickList[tube_trick].flags & ManualFlag);

			if (last_tube_trick == TRICK_QUICK_CUT_LEFT || tube_trick == TRICK_QUICK_CUT_LEFT ||
				last_tube_trick == TRICK_QUICK_CUT_RIGHT || tube_trick == TRICK_QUICK_CUT_RIGHT)
				BoardAnim(tube_board_anim + handedness_offset, BLEND_TIME, GTrickList[tube_trick].flags & ManualFlag);
			else
				BoardAnim(tube_board_anim, BLEND_TIME, GTrickList[tube_trick].flags & ManualFlag);

			//  Check to see if this is the ejection trick.
			void *data = NULL;
			if (tube_trick == TRICK_SPIT_EJECT)
				ks_fx_create_tube_spit(0,&data);

			last_tube_trick = tube_trick;
			tube_trick = -1;
			state = STATE_TUBE_TRICK;
			tube_trick_time_so_far = 0.0f;
			SetCompletedTrick(last_tube_trick);
			currentTrick = last_tube_trick;
		}

		my_board_controller.Turn(BOARD_TURN_RIGHT, 0, dt);  //  Left and right (up and down the side of the tube).

		exit_state = true;

		break;

	case STATE_TUBE_RAILGRAB:

		currentTrick = -1;
		if (last_state != state)
		{
			//  More stability during rail grabs.
			last_state = state;

			if (!BeachDataArray[current_beach].is_small_wave)
				SetCompletedTrick(TRICK_TUBE_RAIL_GRAB);
		}

		if (!BeachDataArray[current_beach].is_small_wave)
			currently_need_extra_balance = true;

		//  Always grab the outside rail.
		if ((left_wave && !goofy) || (!left_wave && goofy))  
			Anim(SURFER_ANIM_TRK_TUBE_GRAB_L_1 + current_balance_anim, BLEND_TIME);
		else
			Anim(SURFER_ANIM_TRK_TUBE_GRAB_R_1 + current_balance_anim, BLEND_TIME);
		BoardAnim(BOARD_ANIM_ZERO, BLEND_TIME, false);

		if (!BeachDataArray[current_beach].is_small_wave)
			my_scoreManager.UpdateLastTrick(ScoringManager::ATTR_TIME_DELTA, dt, ModRideFlag);
		else
			my_scoreManager.UpdateLastTrick(ScoringManager::ATTR_TIME_DELTA, dt, NormalRideFlag);

		if (!(current_depth > WaveDataArray[current_wave].secondtubethresh - depth_fudge) &&  //  Stop grabbing the rail?
			!CtrlEvent(PAD_GRAB) && !BeachDataArray[current_beach].is_small_wave)
			state = STATE_TUBE_STAND;

		//  Don't give extra balance if the surfer is only grabbing his rail because he is deep in a regular tube.
		if (current_depth > WaveDataArray[current_wave].secondtubethresh - depth_fudge && 
			!BeachDataArray[current_beach].is_small_wave && !CtrlEvent(PAD_GRAB))
			currently_need_extra_balance = false;

		if (tube_trick >= 0)  //  don't accept any tricks while holding the rail.
			tube_trick = -1;

		exit_state = true;

		break;

	case STATE_TUBE_TRICK:
		{
			last_state = state;
			current_trick_time += dt;

			//  Check to see if the amount of leaning over has changed enough to be worth changing the animation.
			if (last_balance_anim != current_balance_anim && GTrickList[last_tube_trick].flags & BalanceBlendFlag)
				Anim(tube_anim + current_balance_anim + handedness_offset, BLEND_TIME * 1.5, GTrickList[last_tube_trick].flags & ManualFlag);

			float turn_degree = 1.0f;
			if (last_tube_trick == TRICK_QUICK_CUT_LEFT)
			{
				//  We need to move farther down the wall on this quickcut to counter the physics.
				if (left_wave)
					turn_degree = high_wall_quickcut_degree;

				my_board_controller.MoveForwardOnX(-TUBE_ACCELERATION);  //  Just a little stalling.
				my_board_controller.Turn(BOARD_TURN_LEFT, turn_degree, dt);  //  Left (up and down the side of the tube).
			}
			else if (last_tube_trick == TRICK_QUICK_CUT_RIGHT)
			{
				//  We need to move farther down the wall on this quickcut to counter the physics.
				if (!left_wave)
					turn_degree = high_wall_quickcut_degree;

				my_board_controller.MoveForwardOnX(-TUBE_ACCELERATION);  //  Just a little stalling.
				my_board_controller.Turn(BOARD_TURN_RIGHT, turn_degree, dt);  //  Right (up and down the side of the tube).
			}

			tube_trick_time_so_far += dt;
			my_scoreManager.UpdateLastTrick(ScoringManager::ATTR_TIME_DELTA, dt);
			my_scoreManager.UpdateLastTrick(ScoringManager::ATTR_TIME_DELTA, dt, NormalRideFlag);

			//  Check to see if the trick is done, but only if it is not the ejection trick.
			if (tube_trick != TRICK_SPIT_EJECT && last_tube_trick != TRICK_SPIT_EJECT)
			{
				SurferTrick *last_trick = &GTrickList[last_tube_trick];

				bool no_interrupt_trick = last_trick->flags & NoInterruptFlag;
				if ((!no_interrupt_trick && !my_trickManager.ActionButtonHeld() && current_trick_time >= min_tube_trick_time) ||
					(no_interrupt_trick && AnimComplete()))
				{
					//  This needs to be here so that there is no pop between the current animation and the next one.
					Anim(SURFER_ANIM_TRK_TUBE_FS_1 + current_balance_anim, BLEND_TIME);

					state = STATE_TUBE_STAND;
					bSpecialTrick = false;		//  Set this in case it was a noninteruptable trick or a special trick.
					last_tube_trick = -1;
					exit_state = false;
				}
			}
			exit_state = true;
			break;
		}
	default:
		if (state != STATE_WIPEOUT_GENERAL)  //  something got buggered, so just get out of the tube and pray this works.
		{
			EndTube();
			return;
		}
	}  //  end switch

	//  If the player is trying to move forward or grab his rail, make balancing easier.
	if (last_need_extra_balance != currently_need_extra_balance)
	{
		if (currently_need_extra_balance)
			tube_meter.AdjustStability(2);	//  more balance
		else
			tube_meter.AdjustStability(.5);  //  Go back to normal balance difficulty.

		last_need_extra_balance = currently_need_extra_balance;
	}

	last_balance_anim = current_balance_anim;  //  Update what the current balance level.
	last_direction = direction;  //  Update the current direction that the surfer is moving.

	depth_meter_filter.Filter_Float(current_total_depth,dt, tube_meter_filter_val);
	frontendmanager.IGO->SetTubeDepthMeter(my_player_num, current_total_depth);
}

int kellyslater_controller::StandAnim(void)
{
	stand_num++;
	if (stand_num > 2)
		stand_num = 0;

	if (my_board_controller.InTubeRegion() && this->Z_Within_Tube())
	{
		if (BeachDataArray[g_game_ptr->get_beach_id()].is_small_wave)
		{
			//  Always grab the outside rail.
			bool left_wave = BeachDataArray[g_game_ptr->get_beach_id()].bdir;
			if ((left_wave && !goofy) || (!left_wave && goofy))  
				return SURFER_ANIM_TRK_TUBE_GRAB_L_1;
			else
				return SURFER_ANIM_TRK_TUBE_GRAB_R_1;
		}

		return (stand_anim_nums[5]);
	}
	else if (my_board_controller.InLaunchRegion())
		return (stand_anim_nums[stand_num + 3]);

	return (stand_anim_nums[stand_num]);

}

bool kellyslater_controller::IsFloaterLanding(void)
{
	return (((float_trick != -1) || CtrlEvent(PAD_GRIND)) &&
					(my_board_controller.grind_ollie || my_board_controller.CollideWithLip()));
}

// added by beth to set texture ifl for all entities of a conglom (the board, actually)
void kellyslater_controller::SetConglomTexture(entity* c, int b)
{
	if(!c) return;
	if(c->has_link_ifc())
	{
		link_interface* li = c->link_ifc();
		entity* c1 = (entity*) li->get_first_child();
		while(c1)
		{
			SetConglomTexture(c1, b);
			c1 = (entity*) c1->link_ifc()->get_next_sibling();
		}
		c->SetTextureFrame(b);
	}
}

bool kellyslater_controller::IsAIPlayer()
{ 
  return (g_game_ptr->get_num_ai_players() && (my_player_num==1)); 
}


//  Check to see if a trick is being performed that puts the surfer's hand in the water.
int kellyslater_controller::IsTubeHandInWater()
{
	if (super_state !=  SUPER_STATE_IN_TUBE)  //  Make sure that the surfer is actually in the tube.
		return 0;

	//  figure out which trick is currently being done, if any.
	int current_trick = last_tube_trick;
	if (last_tube_trick == -1)
		current_trick = tube_trick;

	if (current_trick == TRICK_ONE_HAND_ROOF_DRAG)
	{
		if (goofy)
			return LEFT_HAND_IN_WATER;
		else
			return RIGHT_HAND_IN_WATER;
	}

	if (current_trick == TRICK_ONE_HAND_DRAG)
	{
		if (BeachDataArray[g_game_ptr->get_beach_id ()].bdir)
			return RIGHT_HAND_IN_WATER;
		else
			return LEFT_HAND_IN_WATER;
	}

	if (current_trick == TRICK_TWO_HAND_DRAG ||
		current_trick == TRICK_TWO_HAND_ROOF_DRAG)
		return BOTH_HANDS_IN_WATER;

	//  No trick was found that puts the surfer's hand in the water.
	return NO_HANDS_IN_WATER;
}

// Returns true if player is in the air, doing a trick, etc.
bool kellyslater_controller::IsDoingSomething(void)
{
	if ((g_game_ptr->get_game_mode() == GAME_MODE_PUSH) || (g_game_ptr->get_game_mode() == GAME_MODE_HEAD_TO_HEAD))
	{
		return (my_board_controller.InAir() || super_state == SUPER_STATE_WIPEOUT ||
			(super_state == SUPER_STATE_CPU_CONTROLLED && !did_celebration));
	}
	else
	{
		return my_board_controller.InAir() ||
			get_special_meter()->CanRegionLink() ||
			IsDoingSpecialTrick() ||
			super_state == SUPER_STATE_WIPEOUT ||
			(super_state == SUPER_STATE_CPU_CONTROLLED && !did_celebration);
	}
}

#ifdef DEBUG

extern int show_state_label;
extern int show_anim_label;

/*static void calc_screen_pos (entity *ent, float& left, float& top)
{
  nglMatrix nglWorldToScreen;
  nglGetMatrix (nglWorldToScreen, NGLMTX_WORLD_TO_SCREEN);
  vector3d pos = ent->get_abs_position();
  nglVector pt0 = {pos.x, pos.y, pos.z, 1};
  nglVector pt = {0.0f, 0.0f, 0.0f, 0.0f};

  nglApplyMatrix(pt, nglWorldToScreen, pt0);	// Sony prototype doesn't use const
  left = pt[0];
  top = pt[1];
  float w0 = pt[3];
  left /= w0;
  top /= w0;
  left -= 2048 - nglGetScreenWidth() / 2;
  top -= 2048 - nglGetScreenHeight() / 2;
}*/


void kellyslater_controller::draw_debug_labels()
{
	if (show_state_label || show_anim_label)
	{
		//float left, top;

   // calc_screen_pos(the_world->get_board_ptr(0), left, top);
   /* KSNGL_SetFont( 0 );
		unsigned int f = (unsigned int) 128;
		unsigned int s = (unsigned int) 255;
		unsigned int color = s + (s<<16) + (f<<24);
		KSNGL_SetFontColor( color );
		KSNGL_SetFontScale( 1.5f, 1.5f );
		KSNGL_SetFontZ( 0.2f );*/

		if (show_state_label)
		{
			vector3d abs_pos = my_board->get_abs_position();
			state_label->changeText(gp_ksstate_string[my_player_num]);
			state_label->SetLocation3D(abs_pos);
			state_label->UpdateInScene(true);
			state_label->Draw();
		}

		if (show_anim_label)
		{
			vector3d abs_pos = get_owner()->get_abs_position();
			anim_label->changeText(gp_ksanim_string[my_player_num]);
			anim_label->SetLocation3D(abs_pos);
			anim_label->UpdateInScene(true);
			anim_label->Draw();

/*      for (beach_object *fobj = g_beach_ptr->my_objects; fobj != NULL; fobj = fobj->next)
      {
        if (!fobj->is_active ())
          continue;

        if (!fobj->is_physical ())
          continue;

        entity *ent = ((water_object*)fobj)->get_entity ();
        if (ent->get_anim_tree (ANIM_PRIMARY))
        {
          calc_screen_pos (ent, left, top);
    			nglListAddString (left, top - 40, "%s", ent->get_anim_tree (ANIM_PRIMARY)->get_name ().c_str ());
        }
      }*/
    }
	}
}

#endif


float USER_BALANCE_INC = 1.0f;					//  How much the acc should change per unit of time.
float TUTORIAL_USER_BALANCE_INC = 0.2f;					//  How much the acc should change per unit of time.
float TIME_BALANCE_INC = 0.005f;  //  How much the acc should change per unit of time.
float TUTORIAL_TIME_BALANCE_INC = 0.003f;  //  How much the acc should change per unit of time.
#define BALANCE_SCALE 10.0f						//  The closer the meter is to the edge of the scale, the faster it accelerates.

BalanceMeter::BalanceMeter()
{
	player_num = -1;
}

//  Start the balance meter going.
//  set reset to false if you want the meter to continue from where it was before.
//  Stat bonus should range from 0 to 1.
void BalanceMeter::Init(int player, bool vertical_meter, float base_time, float stat_bonus, bool reset)
{
	assert(player >= 0 && player < MAX_PLAYERS);
	
	player_num = player;
	
	time_to_full_acc = base_time + (base_time * stat_bonus);  //  The better the stat, the slower the acceleration grows.
	if (reset)
	{
		current_balance = 0;
		balance_acc = 0;
		total_balance_time = time_to_full_acc * 0.5f;
	}
	vert_meter = vertical_meter;
	frontendmanager.IGO->TurnBalanceMeterOn(player_num, vert_meter, true);
	frontendmanager.IGO->SetBalanceMeter(player_num, vert_meter, current_balance);
}

#define delta_offset 0.7f

//  Direction should range from -1 to 1, with 0 being no change.
//  The farther along the balance is already, the bigger differnce that calling this function will have.
float BalanceMeter::Update(int direction, const float time_step)
{
	//  Figure out how fast the meter should be moving based on how long the player has been balancing.
	float delta_to_full_speed;
	total_balance_time += time_step;
	delta_to_full_speed = total_balance_time/time_to_full_acc;
	float user_bal_inc = USER_BALANCE_INC;
	float time_bal_inc = TIME_BALANCE_INC;

	if (g_game_ptr->get_game_mode() == GAME_MODE_CAREER && g_game_ptr->get_beach_id() == BEACH_INDOOR)
	{
		user_bal_inc = TUTORIAL_USER_BALANCE_INC;
		time_bal_inc = TUTORIAL_TIME_BALANCE_INC;
	}

	if (g_perfect_balance || g_session_cheats[CHEAT_PERFECT_BALANCE].isOn())   //  perfect balance cheat.
		return current_balance;

	if (current_balance == 0)  //  Make sure that the meter doesn't get stuck in the middle.
	{
		if (random(-1.0F,1.0F) > 0)
			current_balance = user_bal_inc * time_step * (delta_to_full_speed * delta_offset);
		else
			current_balance = -(user_bal_inc * time_step * (delta_to_full_speed * delta_offset));
	}

	if (!g_debug_balance)
	{
		//  This changes the acceleration based on user input, normalized for time. (.05 is just an arbitray time normalization.)
		balance_acc += (direction * user_bal_inc * (time_step *.5f)) * (delta_to_full_speed * min(1.0f, delta_offset));

		//  Now add some even if the user isn't adding anything, based on how long the surfer has been balancing.
		//  The last bit is a little something to make the meter move faster when it is near the edges.
		if (balance_acc > 0)
			balance_acc += time_bal_inc * time_step * (fabs(current_balance) * BALANCE_SCALE);
		else
			balance_acc -= time_bal_inc * time_step * (fabs(current_balance) * BALANCE_SCALE);
	}
	else
		balance_acc += (direction * user_bal_inc * (time_step *.5f)) * (min(1.0f, delta_offset));

	//  This adds whatever input we have so far into the total balance.
	current_balance += balance_acc;

	//  Update the meter and get out of here.
	frontendmanager.IGO->SetBalanceMeter(player_num, vert_meter, current_balance);
	return current_balance;
}


void BalanceMeter::End()
{
	frontendmanager.IGO->TurnBalanceMeterOn(player_num, vert_meter, false);
}

//  Start the balance meter going.
//  set reset to false if you want the meter to continue from where it was before.
void BigWaveMeter::Init(int player, float base_time, int stat_bonus, bool reset)
{
	player_num = player;
	
	if (reset)
	{
		current_balance = 0;
		balance_acc = 0;
	}
	frontendmanager.IGO->ShowBigWaveMeter(true);
	frontendmanager.IGO->SetBigWaveMeterPos(current_balance);
}

#define big_wave_delta_offset 0.3f
float BG_USER_BALANCE_INC = 0.7f;

//  Direction should range from -1 to 1, with 0 being no change.
//  The farther along the balance is already, the bigger differnce that calling this function will have.
float BigWaveMeter::Update(int direction, const float time_step)
{
	//  This changes the acceleration based on user input, normalized for time. (.05 is just an arbitray time normalization.)
	balance_acc += (direction * USER_BALANCE_INC * (time_step *.5f)) * big_wave_delta_offset;

	//  This adds whatever input we have so far into the total balance.
	current_balance += balance_acc;

	//  Update the meter and get out of here.
	frontendmanager.IGO->SetBigWaveMeterPos(current_balance);
	return current_balance;
}


void BigWaveMeter::Resize(float new_size)
{
	frontendmanager.IGO->SetBigWaveMeterSize(new_size);
}


void BigWaveMeter::End()
{
	frontendmanager.IGO->ShowBigWaveMeter(false);
}
