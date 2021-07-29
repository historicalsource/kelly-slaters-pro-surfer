#include "global.h"

//!#include "character.h"
#include "camera.h"
#include "geomgr.h"
#include "hwmath.h"
#include "wds.h"
#include "game.h"
//!#include "character.h"
#include "terrain.h"
#include "inputmgr.h"
#include "commands.h"
//!#include "attrib.h"
#include "collide.h"
#include "controller.h"
 // BIGCULL #include "gun.h"
#include "time_interface.h"
#include "ai_interface.h"
// BIGCULL #include "ai_senses.h"
#include "msgboard.h"
#include "console.h"
#include "inputmgr.h"
#include "wave.h"
#include "ks_camera.h"
#include "ksreplay.h"
//#include "WipeTransition.h"

#include "kellyslater_controller.h"
#include "beach.h"
#include "beachdata.h"
#include "ksfx.h"
#include "floatobj.h"
#include "random.h"
#include "kshooks.h"	// For KSWhatever calls
#include "blur.h"

extern float air_gravity;

#if defined(TARGET_XBOX)

// JIV FIXME  aaaarggggh
#define AIR_GRAV_MOD 0.72f

#endif /* TARGET_XBOX JIV DEBUG */

debug_camera::debug_camera( const entity_id& _id, entity* _target_entity )
  :   game_camera( _id, _target_entity )
{
  /* create_link_ifc() */  //  eventually this will have to be uncommented when bones do not have a default link interface
	yaw = 0.25*PI;
	pitch = -0.25*PI;
	magn = 2.5f;
}

void debug_camera::sync( camera& b )
{
}

void debug_camera::frame_advance( time_value_t t )
{
	input_mgr* inputmgr = input_mgr::inst();

	input_device *input_dev = inputmgr->get_device(JOYSTICK2_DEVICE);
	if (input_dev && input_dev->is_connected())  //  Use controller 2 if it is plugged in.
	{
		rational_t speed = 1.0f;
		if( inputmgr->get_control_state(JOYSTICK2_DEVICE, PAD2_USERCAM_FAST ) == AXIS_MAX )
			speed *= 5.0f;
		if( inputmgr->get_control_state(JOYSTICK2_DEVICE, PAD2_USERCAM_SLOW ) == AXIS_MAX )
			speed *= 0.2f;

		pitch += speed*0.05f*inputmgr->get_control_state(JOYSTICK2_DEVICE, PAD2_USERCAM_DOWN );  //  JOY_RY
		yaw += speed*0.05f*inputmgr->get_control_state(JOYSTICK2_DEVICE, PAD2_USERCAM_EQUALS_CHASECAM );  //   JOY_RX

		if (yaw > 2.0f*PI)
			yaw -= 2.0f*PI;
		else if (yaw < 0.0f)
			yaw += 2.0f*PI;

		if (pitch > 0.5f*PI)
			pitch = 0.5f*PI - 0.001f;
		else if (pitch < -0.5f*PI)
			pitch = -0.5f*PI + 0.001f;


		if( inputmgr->get_control_state(JOYSTICK2_DEVICE, PAD2_USERCAM_FORWARD ) == AXIS_MAX )
			magn -= 0.2f;
		if( inputmgr->get_control_state(JOYSTICK2_DEVICE, PAD2_USERCAM_BACKWARD ) == AXIS_MAX )
			magn += 0.2f;

		if (magn < 1.0f)
			magn = 1.0f;
	}
	else	//  Use controller 1 for both steering the surfer and the camera.
	{
		rational_t speed = 1.0f;
		if( inputmgr->get_control_state(JOYSTICK_DEVICE, USERCAM_FAST ) == AXIS_MAX )
			speed *= 5.0f;
		if( inputmgr->get_control_state(JOYSTICK_DEVICE, USERCAM_SLOW ) == AXIS_MAX )
			speed *= 0.2f;

		pitch += speed*0.05f*inputmgr->get_control_state(JOYSTICK_DEVICE, USERCAM_DOWN );  //  JOY_RY
		yaw += speed*0.05f*inputmgr->get_control_state(JOYSTICK_DEVICE, USERCAM_EQUALS_CHASECAM );  //   JOY_RX

		if (yaw > 2.0f*PI)
			yaw -= 2.0f*PI;
		else if (yaw < 0.0f)
			yaw += 2.0f*PI;

		if (pitch > 0.5f*PI)
			pitch = 0.5f*PI - 0.001f;
		else if (pitch < -0.5f*PI)
			pitch = -0.5f*PI + 0.001f;


		if( inputmgr->get_control_state(JOYSTICK_DEVICE, USERCAM_FORWARD ) == AXIS_MAX )
			magn -= 0.2f;
		if( inputmgr->get_control_state(JOYSTICK_DEVICE, USERCAM_BACKWARD ) == AXIS_MAX )
			magn += 0.2f;

		if (magn < 1.0f)
			magn = 1.0f;
	}

	vector3d zvec( 0.0f, 0.0f, 1.0f);// = get_target_entity()->get_abs_po().get_z_facing();
	vector3d yvec( 0.0f, 1.0f, 0.0f);
	zvec -= dot(zvec, yvec)*yvec;
	zvec.normalize();
	vector3d xvec = cross(yvec, zvec);
	vector3d pos_vec(0.0f, 0.0f, 0.0f);
	po orient(xvec, yvec, zvec, pos_vec);
	po yawpo, pitchpo;
	yawpo.set_rotate_y(yaw);
	pitchpo.set_rotate_z(pitch);
	vector3d delta_pos = magn*(pitchpo*yawpo*orient).slow_xform(vector3d(1.0f, 0.0f, 0.0f));
	vector3d ent_center = ((conglomerate*) get_target_entity ())->get_member("BIP01 PELVIS")->get_abs_position();
	po trans( po_identity_matrix );
	trans.set_position(ent_center + delta_pos);
	trans.set_facing(ent_center);
	this->set_rel_po(trans);
}

// ============================================================================
// Look_back_camera Camera

look_back_camera::look_back_camera (const entity_id& _id, entity* _target_entity)
  : game_camera (_id, _target_entity)
{
	init();
}

void look_back_camera::init()
{
	first_time = true;
}

void look_back_camera::sync (camera& b)
{
}

#define look_back_object_dist_limit 40000.0f

bool look_back_camera::FindNearestObject(void)
{
	vector3d lipnorm = ksctrl->get_board_controller().GetLipNormal();
	if (g_beach_ptr && g_beach_ptr->get_nearest_object_pos(get_target_entity()->get_abs_position(), object_pos, lipnorm))
	{
		if (dot(object_pos, object_pos) < look_back_object_dist_limit)
			return true;
		else
			return false;
	}
	else
		return false;
}

float offset_mult = 2.5f;
float offset_height = 0.9f;
float look_back_filter = 0.3f;
float towards_beach_thresh = -0.5f;
float offset_z_mult = 3.0f;

//  this camera always looks at the surfer, but it is angled at the closest, most interesting thing
//  sharing the wave with him.  That is usually the tube, but a close object on the wave is considered to be more interesting.
void look_back_camera::frame_advance (time_value_t t)
{
	po trans (po_identity_matrix);

	vector3d position;
	vector3d facing;
	float current_offset_height;
	float current_offset_mult = offset_mult;

	if (!FindNearestObject() || ksctrl->get_super_state() == SUPER_STATE_IN_TUBE)  //  no object to look at.
	{
		//  Put the camera a little in front of the surfer, looking east down the wave (the beach is south)
		position = get_target_entity ()->get_abs_position ();
		if (BeachDataArray[g_game_ptr->get_beach_id()].bdir)
			position.x -= current_offset_mult;
		else
			position.x += current_offset_mult;

		facing = get_target_entity ()->get_abs_position ();

	}
	else	//  we found something, point the camera towards it.
	{
		//  initially, just set a line from the surfer to the object.
		facing = get_target_entity ()->get_abs_position ();//object_pos;
		position = get_target_entity ()->get_abs_position () - object_pos;

		//  now move the position of the camera up and back along that line.
		vector3d offset = position - facing;
		offset.normalize();
		position += offset * current_offset_mult;
	}

	vector3d delta =  facing - position;

	delta.y = 0;
	delta.normalize();

	//  If the camera is in the direction of the beach, it might clip into the wave.  Move it closer.
	if (delta.z < towards_beach_thresh)
	{
		float offset_mod = towards_beach_thresh - delta.z;
		current_offset_mult -= offset_mod * offset_z_mult;
	}

	if (ksctrl->get_super_state() == SUPER_STATE_IN_TUBE)  //  Small tubes will clip into the ceiling sometimes.
		current_offset_height = 0;
	else
		current_offset_height = offset_height;

	if (first_time)
	{
		first_time = false;
		target_filter.Init_Filter(delta);
		offset_filter.Init_Filter(current_offset_mult);
		if (ksctrl->get_super_state() == SUPER_STATE_IN_TUBE)  //  Small tubes will clip into the ceiling sometimes.
			look_back_height_filter.Init_Filter(0);
		else
			look_back_height_filter.Init_Filter(offset_height);
	}
	else
	{
		target_filter.Filter_Vector(delta, t, look_back_filter);
		offset_filter.Filter_Float(current_offset_mult, t, look_back_filter);
		look_back_height_filter.Filter_Float(current_offset_height, t, look_back_filter);
	}

	delta.normalize();

	position = facing - (delta * current_offset_mult);

	position.y += current_offset_height;
	facing.y += current_offset_height;

	trans.set_position(position);
	trans.set_facing(facing);
	set_rel_po (trans);
}


// ============================================================================
// replay_camera Camera
//bool KSReadFile(char* FileName, nglFileBuf* File, u_int Align );
//void KSReleaseFile( nglFileBuf* File );

replay_camera::replay_camera (const entity_id& _id, entity* _target_entity)
  : game_camera (_id, _target_entity)
{
  rc                    = RC_NONE;
  rcr                   = RCR_NONE;

  regionChangeTime      = 0.0f;
  regionChangeFrame     = 0;

  ent                   = NULL;

  waveDir               = BeachDataArray[g_game_ptr->get_beach_id()].bdir;

  defaultCam            = vector3d(0, 1, -5);
  tubeCam               = waveDir ? vector3d(-5, 1, 0) : vector3d(5, 1, 0);
  yoyoCam               = vector3d(0.05f, 1.4f, -0.2f);
  birdCam               = vector3d(0, -0.2f, 1.2f);
  birdCamVel            = vector3d(10.0f, 0.0f, 0.0f);
  circleCamCloseDist    = 1.5f;

  takeoffCamPos         = vector3d(0, 1, 0);
  takeoffCamDir         = waveDir ? vector3d(-1, 0, 0) : vector3d(1, 0, 0);
  landingCamPos         = vector3d(0, 1, 0);
  landingCamDir         = waveDir ? vector3d(1, 0, 0) : vector3d(-1, 0, 0);

  holdCamTimer          = 0.0f;

  keepFaceThreshhold    = 0.8f;
  keepTubeThreshhold    = 1.5f;
}

void replay_camera::reset ()
{
  rc                    = RC_NONE;
  rcr                   = RCR_NONE;

  regionChangeTime      = 0.0f;
  regionChangeFrame     = 0;

  holdCamTimer          = 0.0f;
}

/*void replay_camera::toggle_player()
{
  if(ksreplay.NumSurfers() == 2)
    activePlayer = !activePlayer;
}*/

void replay_camera::sync (camera& b)
{
}

void replay_camera::pickCam()
{
  ReplayCam last_rc = rc;

  if(holdCamTimer > 0.0f)
    return;

  if(regionChange)
  {
    if(rcr == RCR_TUBE)
      rc = RC_TUBE_DEFAULT;
    else if(rcr == RCR_AIR)
    {
      switch(random(4))
      {
        case 0:
          rc = RC_AIR_YOYO;
          break;
        case 1:
          rc = RC_AIR_BIRD;
          velocity = birdCamVel;
          position = jumpMaxHeight + birdCam;
          position -= ((regionChangeTime*velocity)*0.5f);
          break;
        case 2:
          rc = RC_AIR_CIRCLE;
          circleCamStartDist  = sqrtf(defaultCam.x*defaultCam.x + defaultCam.z*defaultCam.z);
          break;
        case 3:
          rc = RC_AIR_LANDING;
          WAVE_GlobalCurrent(&velocity);
          velocity.y = velocity.z = 0.0f;
          position  = ksreplay.mainEntityPO[regionChangeFrame/ksreplay.MainPOFrames()].KSPos + landingCamPos;
          position -= velocity*regionChangeTime;
          target    = position + landingCamDir;
          break;
        case 4:
          rc = RC_AIR_TAKEOFF;
          WAVE_GlobalCurrent(&velocity);
          velocity.y = velocity.z = 0.0f;
          position  = ksreplay.mainEntityPO[ksreplay.mainPOFrame].KSPos + takeoffCamPos;
          target    = position + takeoffCamDir;
          break;
      }
    }
    else
      rc = RC_DEFAULT;
  }

  if(rc != last_rc)
    lastCamChange = 0.0f;
}


void replay_camera::updateCam(float dt)
{
	entity *body = ((conglomerate *)g_world_ptr->get_ks_controller(0)->get_owner())->get_member("BIP01 PELVIS");
  vector3d pos = body->get_abs_po().get_position();

/*  if(debugCam)
  {
	  input_mgr* inputmgr = input_mgr::inst();

	  rational_t speed = 1.0f;
	  if( inputmgr->get_control_state(JOYSTICK_DEVICE, REPLAYCAM_FAST ) == AXIS_MAX )
		  speed *= 5.0f;
	  if( inputmgr->get_control_state(JOYSTICK_DEVICE, REPLAYCAM_SLOW ) == AXIS_MAX )
		  speed *= 0.2f;

	  debug_pitch += speed*0.03f*inputmgr->get_control_state(JOYSTICK_DEVICE, REPLAYCAM_PITCH );  //  JOY_RY
	  debug_yaw += speed*0.03f*inputmgr->get_control_state(JOYSTICK_DEVICE, REPLAYCAM_YAW );  //   JOY_RX

	  if (debug_yaw > 2.0f*PI)
		  debug_yaw -= 2.0f*PI;
	  else if (debug_yaw < 0.0f)
		  debug_yaw += 2.0f*PI;

	  if (debug_pitch > 0.5f*PI)
		  debug_pitch = 0.5f*PI - 0.001f;
	  else if (debug_pitch < -0.5f*PI)
		  debug_pitch = -0.5f*PI + 0.001f;


    debug_dist += 0.03f*speed*inputmgr->get_control_state(JOYSTICK_DEVICE, REPLAYCAM_ZOOM );

	  if (debug_dist < 1.0f)
		  debug_dist = 1.0f;

	  po yawpo, pitchpo;
	  yawpo.set_rotate_y(debug_yaw);
	  pitchpo.set_rotate_z(debug_pitch);
	  target = pos;
	  position = target + debug_dist*(pitchpo*yawpo).slow_xform(vector3d(1.0f, 0.0f, 0.0f));
    return;
  }*/

  float interp;
  float omega;

  float zeroToOneToZero, oneToZeroToOne, zeroToOne;
  float d;

  switch(rc)
  {
    case RC_DEFAULT:
      position = defaultCam + pos;
      target   = pos;
      break;
    case RC_TUBE_DEFAULT:
      position = tubeCam + pos;
      target   = pos;
      break;
    case RC_AIR_YOYO:
      position      = pos + yoyoCam;
      position.y    = jumpMaxHeight.y + yoyoCam.y;
      target        = pos;
      break;
    case RC_AIR_BIRD:
      position     += dt*velocity;
      target        = pos;
      break;
    case RC_AIR_CIRCLE:
      interp = lastCamChange / (lastCamChange+regionChangeTime);
      if(interp < 0.0f)
        interp = 0.0f;
      if(interp > 1.0f)
        interp = 1.0f;
      
      omega = interp*2*PI;
      zeroToOneToZero = -0.5f*cosf(omega) + 0.5f;   // Smooth transition from 0 to 1 back to 0;
      oneToZeroToOne  = 1.0f - zeroToOneToZero;     // Smooth transition from 1 to 0 back to 1;
      zeroToOne       = -0.5f*cosf(0.5f*omega) + 0.5f;

      position.y = pos.y + defaultCam.y*oneToZeroToOne;

      d = zeroToOneToZero*circleCamCloseDist + oneToZeroToOne*circleCamStartDist;
      position.x = pos.x + d*sinf(zeroToOne*2*PI);
      position.z = pos.z - d*cosf(zeroToOne*2*PI);

      target = pos;

      break;
    case RC_AIR_TAKEOFF:
    case RC_AIR_LANDING:
      position     += dt*velocity;
      target       += dt*velocity;
      break;
    default:
      break;
  }


  lastCamChange += dt;
  holdCamTimer  -= dt;
}

void replay_camera::updateCamRegion(float dt)
{
  unsigned f;

  ReplayCamRegion last_rcr = rcr;

  regionChangeTime -= dt;

  if((int)ksreplay.playframe < regionChangeFrame)
  {
    regionChange = false;
    return;
  }

  if(ksreplay.MainEntityRegion() == RCR_AIR)
  {
    if(rcr != RCR_AIR)
    {
      lastRegion = rcr;
      rcr = RCR_AIR;
      
      jumpStartPos = ksreplay.mainEntityPO[ksreplay.mainPOFrame].KSPos;
      jumpMaxHeight = jumpStartPos;

      for(f=ksreplay.playframe; f<ksreplay.numFrames; f++)
      {
        if(jumpMaxHeight.y < ksreplay.mainEntityPO[f/ksreplay.MainPOFrames()].KSPos.y)
          jumpMaxHeight = ksreplay.mainEntityPO[f/ksreplay.MainPOFrames()].KSPos;

        if(ksreplay.MainEntityRegion(f) != RCR_AIR)  
          break;
      }

      jumpEndPos = ksreplay.mainEntityPO[f/ksreplay.MainPOFrames()].KSPos;

      if(f == ksreplay.numFrames)
      {
        regionChangeFrame = f-1;
        regionChangeTime = ksreplay.frame[f-1].totalTime - ksreplay.frame[ksreplay.playframe].totalTime;
        nextRegion = RCR_NONE;
      }
      else
      {
        regionChangeFrame = f;
        regionChangeTime = ksreplay.frame[f].totalTime - ksreplay.frame[ksreplay.playframe].totalTime;
        nextRegion = ksreplay.MainEntityRegion(f);
      }
    }
  }
  else if(ksreplay.MainEntityRegion() == RCR_TUBE)
  {
    if(rcr != RCR_TUBE)
    {
      lastRegion = rcr;
      rcr = RCR_TUBE;
      for(f=ksreplay.playframe; f<ksreplay.numFrames; f++)
      {
        // If next region only lasts a short time then goes back to RCR_TUBE, treat the next region as RCR_TUBE as well
        if(ksreplay.MainEntityRegion(f) != RCR_TUBE)
        {
          int f2;
          for(f2=f+1; f2<(int)ksreplay.numFrames; f2++)
          {
            if(ksreplay.MainEntityRegion(f2) == RCR_TUBE)
              break;
          }

          // If end of replay or time until next RCR_TUBE is greater than threshhold, end of RCR_TUBE is found
          if(f2 == (int)ksreplay.numFrames || ((ksreplay.frame[f2].totalTime - ksreplay.frame[f].totalTime) > keepTubeThreshhold))
            break;

          // Otherwise, treat it all as RCR_TUBE
          f = f2;
        }
      }

      if(f == ksreplay.numFrames)
      {
        regionChangeFrame = f-1;
        regionChangeTime = ksreplay.frame[f-1].totalTime - ksreplay.frame[ksreplay.playframe].totalTime;
        nextRegion = RCR_NONE;
      }
      else
      {
        regionChangeFrame = f;
        regionChangeTime = ksreplay.frame[f].totalTime - ksreplay.frame[ksreplay.playframe].totalTime;
        nextRegion = ksreplay.MainEntityRegion(f);
      }
    }
  }
  else if(ksreplay.MainEntityRegion() == RCR_WIPEOUT)
  {
    if(rcr != RCR_WIPEOUT)
    {
      lastRegion = rcr;
      rcr = RCR_WIPEOUT;
      for(f=ksreplay.playframe; f<ksreplay.numFrames; f++)
      {
        if(ksreplay.MainEntityRegion(f) != RCR_WIPEOUT)
          break;
      }

      if(f == ksreplay.numFrames)
      {
        regionChangeFrame = f-1;
        regionChangeTime = ksreplay.frame[f-1].totalTime - ksreplay.frame[ksreplay.playframe].totalTime;
        nextRegion = RCR_NONE;
      }
      else
      {
        regionChangeFrame = f;
        regionChangeTime = ksreplay.frame[f].totalTime - ksreplay.frame[ksreplay.playframe].totalTime;
        nextRegion = ksreplay.MainEntityRegion(f);
      }
    }
  }
  else
  {
    if(rcr != RCR_FACE)
    {
      lastRegion = rcr;
      rcr = RCR_FACE;
      for(f=ksreplay.playframe; f<ksreplay.numFrames; f++)
      {
        // If next region only lasts a short time then goes back to RCR_FACE, treat the next region as RCR_FACE as well
        if(ksreplay.MainEntityRegion(f) != RCR_TUBE)
        {
          int f2;
          for(f2=f+1; f2<(int)ksreplay.numFrames; f2++)
          {
            if(ksreplay.MainEntityRegion(f2) == RCR_FACE)
              break;
          }

          // If end of replay or time until next RCR_TUBE is greater than threshhold, end of RCR_TUBE is found
          if(f2 == (int)ksreplay.numFrames || ((ksreplay.frame[f2].totalTime - ksreplay.frame[f].totalTime) > keepFaceThreshhold))
            break;

          // Otherwise, treat it all as RCR_FACE
          f = f2;
        }

        if(ksreplay.MainEntityRegion(f) != RCR_FACE)
          break;
      }

      if(f == ksreplay.numFrames)
      {
        regionChangeFrame = f-1;
        regionChangeTime = ksreplay.frame[f-1].totalTime - ksreplay.frame[ksreplay.playframe].totalTime;
        nextRegion = RCR_NONE;
      }
      else
      {
        regionChangeFrame = f;
        regionChangeTime = ksreplay.frame[f].totalTime - ksreplay.frame[ksreplay.playframe].totalTime;
        nextRegion = ksreplay.MainEntityRegion(f);
      }
    }
  }

  if(rcr != last_rcr)
    regionChange = true;
  else
    regionChange = false;
}

void replay_camera::frame_advance (time_value_t t)
{
	po trans (po_identity_matrix);
//	const int minRangeSq = 25;

  //ks_state = g_world_ptr->get_ks_controller(activePlayer)->get_current_state();
  //ks_super_state = g_world_ptr->get_ks_controller(activePlayer)->get_super_state();

  updateCamRegion(t);
  pickCam();
  updateCam(t);

  //ks_last_super_state = ks_super_state;
  //ks_last_state       = ks_state;
  trans.set_position (position);
	trans.set_facing (target);
	set_rel_po (trans);
}

// ============================================================================
// Old Shoulder Camera

extern float shoulder_cam_dy, shoulder_cam_dz, shoulder_cam_lag;
float shoulder_cam_delay = 5;
int shoulder_cam_roll = 0;

old_shoulder_camera::old_shoulder_camera (const entity_id& _id, entity* _target_entity)
  : game_camera (_id, _target_entity)
{
  turn_angle = 0;
  lip_jump = false;
  last_desired_pos = ZEROVEC;
  last_target = ZEROVEC;
}

old_shoulder_camera::~old_shoulder_camera ()
{
}

void old_shoulder_camera::frame_advance (time_value_t t)
{
  bool right = !BeachDataArray[g_game_ptr->get_beach_id ()].bdir;
  int current_state = ksctrl->get_current_state();
  po trans (po_identity_matrix);
  vector3d desired_pos;

  if ((current_state == STATE_LIEONBOARD) || (current_state == STATE_PADDLE))
  {
    desired_pos = get_target_entity ()->get_abs_po ().slow_xform (vector3d (-shoulder_cam_dz, shoulder_cam_dy, 0));

    if (right)
      desired_pos.x -= 2;
    else
      desired_pos.x += 2;
  }
  else if (current_state == STATE_LIETOSTAND)
  {
    desired_pos = get_target_entity ()->get_abs_po ().non_affine_slow_xform (vector3d (shoulder_cam_dz, shoulder_cam_dy, 0));

    if (ksctrl->get_last_state() != current_state)
      turn_angle = 180;

    po turn;

    if (right)
      turn.set_rotate_y (DEG_TO_RAD (turn_angle));
    else
      turn.set_rotate_y (DEG_TO_RAD (-turn_angle));

    desired_pos = turn.non_affine_slow_xform (desired_pos);
    desired_pos += get_target_entity ()->get_abs_position ();

    turn_angle -= 180 * t;
  }
  else
  {
    if (current_state == STATE_LAUNCH)
      lip_jump = true;
    else if (current_state == STATE_CHOP_HOP)
      lip_jump = false;


    if (ksctrl->get_board_controller().InAir() && get_target_entity()->get_abs_position().y > OVERHEAD_CAM_THRESHHOLD)
    {
      if ((current_state == STATE_LAUNCH) && (ksctrl->get_last_state() != current_state))
        turn_angle = 90;

      po turn;

      if (get_abs_position ().x < get_target_entity ()->get_abs_position ().x)
        turn.set_rotate_z (DEG_TO_RAD (-turn_angle));
      else
        turn.set_rotate_z (DEG_TO_RAD (turn_angle));

      desired_pos = turn.non_affine_slow_xform (YVEC) * shoulder_cam_dz + get_target_entity ()->get_abs_position ();

      if (turn_angle > 1)
        turn_angle -= 45 * t;
      else
        turn_angle = 0;

      desired_pos.z = get_target_entity ()->get_abs_position ().z + 3;
    }
    else if ((current_state == STATE_CHOP_HOP) || ((current_state == STATE_FREEFALL) && !lip_jump))
    {
      desired_pos = last_desired_pos;

      if (get_abs_position ().x < get_target_entity ()->get_abs_position ().x)
        desired_pos.x  = get_target_entity ()->get_abs_position ().x - 5;
      else
        desired_pos.x  = get_target_entity ()->get_abs_position ().x + 5;

      desired_pos.y = get_target_entity ()->get_abs_position ().y + shoulder_cam_dy;
    }
    else
    {
      desired_pos = get_target_entity ()->get_abs_po ().slow_xform (vector3d (shoulder_cam_dz, shoulder_cam_dy, 0));
//      desired_pos.z -= shoulder_cam_lag * __fabs (get_target_entity ()->get_abs_po ().get_x_facing ().x);

		float board_x = get_target_entity ()->get_abs_position ().x;
		float camera_x;

		//  figure out which way the board is turned.
		if (board_x > desired_pos.x)
			camera_x = board_x - 3;
		else
			camera_x = board_x + 3;

		//  Check to see if we need to change to the tube cam.  TODO:  check wave region, height, and floater for disallowing tube cam.
        if (camera_x > WAVE_SoundEmitter[WAVE_SE_TUBE].line.start.x - TUBE_TRANSITION_LENGTH &&
			camera_x < WAVE_SoundEmitter[WAVE_SE_TUBE].line.stop.x + TUBE_TRANSITION_LENGTH)
		{

			if (right)
			{
				if (camera_x < WAVE_SoundEmitter[WAVE_SE_TUBE].line.stop.x)  //  Camera is completely inside the tube.
				{
					float difference = WAVE_SoundEmitter[WAVE_SE_TUBE].line.stop.x - camera_x;
					difference /= WAVE_SoundEmitter[WAVE_SE_TUBE].line.stop.x - WAVE_SoundEmitter[WAVE_SE_TUBE].line.start.x;

					desired_pos.x = camera_x;
					desired_pos.y = (WAVE_SoundEmitter[WAVE_SE_TUBE].line.stop.y * (1-difference)) + (WAVE_SoundEmitter[WAVE_SE_TUBE].line.start.y * difference);
					desired_pos.z = (WAVE_SoundEmitter[WAVE_SE_TUBE].line.stop.z * (1-difference)) + (WAVE_SoundEmitter[WAVE_SE_TUBE].line.start.z * difference);
				}
				else
				{
					float difference = (WAVE_SoundEmitter[WAVE_SE_TUBE].line.start.x + TUBE_TRANSITION_LENGTH) - camera_x;
					difference /= TUBE_TRANSITION_LENGTH;

					desired_pos.x = (desired_pos.x * (1-difference)) + (camera_x * difference);
					desired_pos.y = (desired_pos.y * (1-difference)) + (WAVE_SoundEmitter[WAVE_SE_TUBE].line.start.y * difference);
					desired_pos.z = (desired_pos.z * (1-difference)) + (WAVE_SoundEmitter[WAVE_SE_TUBE].line.start.z * difference);
				}
			}
			else
			{
				if (camera_x > WAVE_SoundEmitter[WAVE_SE_TUBE].line.start.x)  //  Camera is completely inside the tube.
				{
					float difference = camera_x - WAVE_SoundEmitter[WAVE_SE_TUBE].line.start.x;
					difference /= WAVE_SoundEmitter[WAVE_SE_TUBE].line.stop.x - WAVE_SoundEmitter[WAVE_SE_TUBE].line.start.x;

					desired_pos.x = camera_x;
					desired_pos.y = (WAVE_SoundEmitter[WAVE_SE_TUBE].line.start.y * (1-difference)) + (WAVE_SoundEmitter[WAVE_SE_TUBE].line.stop.y * difference);
					desired_pos.z = (WAVE_SoundEmitter[WAVE_SE_TUBE].line.start.z * (1-difference)) + (WAVE_SoundEmitter[WAVE_SE_TUBE].line.stop.z * difference);
				}
				else
				{
					float difference = camera_x - (WAVE_SoundEmitter[WAVE_SE_TUBE].line.start.x - TUBE_TRANSITION_LENGTH);
					difference /= TUBE_TRANSITION_LENGTH;

					desired_pos.x = (desired_pos.x * (1-difference)) + (camera_x * difference);
					desired_pos.y = (desired_pos.y * (1-difference)) + (WAVE_SoundEmitter[WAVE_SE_TUBE].line.start.y * difference);
					desired_pos.z = (desired_pos.z * (1-difference)) + (WAVE_SoundEmitter[WAVE_SE_TUBE].line.start.z * difference);
				}
			}
		}  //  end of adjusting camera for the tube.

      float crz = get_target_entity ()->get_abs_position ().z - get_rel_position ().z;
      float dz = shoulder_cam_lag * __fabs (get_target_entity ()->get_abs_po ().get_x_facing ().x) - crz;

      desired_pos.z += crz - dz * 0.5 * t;

#if defined(TARGET_XBOX)
      assert(desired_pos.is_valid());
      assert(get_abs_position().is_valid());
#endif /* TARGET_XBOX JIV DEBUG */

      float len = (get_abs_position () - get_target_entity ()->get_abs_position ()).length2 ();
      len -= (shoulder_cam_dz + shoulder_cam_dy) * (shoulder_cam_dz + shoulder_cam_dy);
      if (len < 0)
      {
        if (len < -1)
          len = -1;

        desired_pos.y += -len * 0.5f;
      }

    }
  }

#if defined(TARGET_XBOX)
  assert(get_rel_position().is_valid());
  assert(desired_pos.is_valid());
#endif /* TARGET_XBOX JIV DEBUG */

  // add a small delay when the camera is moving
  vector3d delta_pos = desired_pos - get_rel_position ();
  delta_pos *= shoulder_cam_delay * t;

  last_desired_pos = desired_pos;
  desired_pos = get_rel_position () + delta_pos;

  trans.set_position (desired_pos);

  vector3d new_target = get_target_entity ()->get_abs_position ();
  new_target = last_target + ((new_target - last_target) * (7 * t));

#if defined(TARGET_XBOX)
  assert(new_target.is_valid());
#endif /* TARGET_XBOX JIV DEBUG */

  trans.set_facing (new_target);
  last_target = new_target;

  if (!ksctrl->get_board_controller().InAir() && shoulder_cam_roll)
  {
    vector3d y = 4 * get_rel_po ().get_y_facing () + ksctrl->get_board_controller().rb->my_po.get_y_facing ();
    y.normalize ();

    vector3d x = cross (trans.get_z_facing (), y);

    trans = po (x, y, trans.get_z_facing (), trans.get_position ());
  }

  set_rel_po (trans);
}



//  Linearly interpolate between the two vectors based on the difference between them.
float interpolate_float(float first_float, float second_float, float difference)
{
	assert (difference >= 0 && difference <= 1);

	return (first_float * (1-difference)) + (second_float * difference);
}

//  Interpolate between the two vectors based on the difference between them.  Use a bell curve, then normalize.
//  difference should range from 0 to 1
void interpolate_vector(vector3d *new_vector, vector3d first_vector, vector3d second_vector, float difference, int curve)
{
	assert(difference >= 0.0f && difference <= 1.0f);

	if (curve == 1)  // Bell curve
		difference = 3*difference*difference- 2*difference*difference*difference;
	else if (curve == 2)  //  weighted toward first vector
		difference *= difference;
	else if (curve == 3)  //  weighted toward second vector
		difference = (2*difference) - (difference*difference);

	*new_vector = (first_vector * (1-difference)) + (second_vector * difference);

	if (curve != 4)  //  Special case, don't normalize.
		new_vector->normalize();
}

void vector_filter::Init_Filter(vector3d start_vec)
{
	for (int n = 0; n < 2; n++)
	{
		xwPos[n] = start_vec.x;
		ywPos[n] = start_vec.y;
		zwPos[n] = start_vec.z;
		xerr[n] = 0.0f;
		yerr[n] = 0.0f;
		zerr[n] = 0.0f;
	}
}

#ifdef DEBUG
bool debug_filter = true;
#endif

//  Uses pattented Beach Cam (tm) technology!
void vector_filter::Filter_Vector (vector3d &filtered_vec, time_value_t time_step, float filter_speed_r, float filter_speed_i)
{
#ifdef DEBUG
	if (!debug_filter)  //  Just a debugging tool for checking what things look like unfiltered.
		return;
#endif

	if (time_step > 0.1f)
		time_step = 0.1f;
	else if (time_step < 0.014f)
		time_step = 0.014f;

	//  Default value, change to r.
	if (filter_speed_i == 1000000)
		filter_speed_i = filter_speed_r;

	float r = filter_speed_r*6.9282f;
	float i = filter_speed_i*4.0f;

	float eneg = 0.36787944f;
	float coeff = pow(eneg,r*time_step);
	float coss = fast_cos(i*time_step);	// fix trig (dc 08/16/01)
	float sins = fast_sin(i*time_step);
	float r1 = -coeff*coss;
	float i1 = coeff*sins;

	float a = r1*r1 + i1*i1;
	float g = 2.0f*r1 + a + 1.0f;

	float error = filtered_vec.x - xwPos[0];
    filtered_vec.x = (a+1.0f)*xwPos[0] - a*xwPos[1] + g*xerr[0];

    xwPos[1] = xwPos[0];
    xwPos[0] = filtered_vec.x;
    xerr[1] = xerr[0];
    xerr[0] = error;

	error = filtered_vec.y - ywPos[0];
    filtered_vec.y = (a+1.0f)*ywPos[0] - a*ywPos[1] + g*yerr[0];

    ywPos[1] = ywPos[0];
    ywPos[0] = filtered_vec.y;
    yerr[1] = yerr[0];
    yerr[0] = error;

	error = filtered_vec.z - zwPos[0];
    filtered_vec.z = (a+1.0f)*zwPos[0] - a*zwPos[1] + g*zerr[0];

    zwPos[1] = zwPos[0];
    zwPos[0] = filtered_vec.z;
    zerr[1] = zerr[0];
    zerr[0] = error;
}

void float_filter::Init_Filter(float start_float)
{
	for (int n = 0; n < 2; n++)
	{
		Pos[n] = start_float;
		err[n] = 0.0f;
	}
}


void float_filter::Filter_Float (float &filtered_float, time_value_t time_step, float filter_speed_r, float filter_speed_i)
{
#ifdef DEBUG
	if (!debug_filter)  //  Just a debugging tool for checking what things look like unfiltered.
		return;
#endif

	if (time_step > 0.1f)
		time_step = 0.1f;
	else if (time_step < 0.014f)
		time_step = 0.014f;

	//  Default value, change to r.
	if (filter_speed_i == 1000000)
		filter_speed_i = filter_speed_r;

	float r = filter_speed_r*6.9282f;
	float i = filter_speed_i*4.0f;

	float eneg = 0.36787944f;
	float coeff = pow(eneg,r*time_step);
	float coss = fast_cos(i*time_step);	// fix trig (dc 08/16/01)
	float sins = fast_sin(i*time_step);
	float r1 = -coeff*coss;
	float i1 = coeff*sins;

	float a = r1*r1 + i1*i1;
	float g = 2.0f*r1 + a + 1.0f;

	float error = filtered_float - Pos[0];
    filtered_float = (a+1.0f)*Pos[0] - a*Pos[1] + g*err[0];

    Pos[1] = Pos[0];
    Pos[0] = filtered_float;
    err[1] = err[0];
    err[0] = error;
}


// ============================================================================
// Shoulder Camera


shoulder_camera::shoulder_camera (const entity_id& _id, entity* _target_entity)
  : game_camera (_id, _target_entity)
{
	init();
}


shoulder_camera::~shoulder_camera ()
{
}

void shoulder_camera::init()
{
  lip_jump = false;
  last_desired_pos = ZEROVEC;
  last_target = ZEROVEC;
  first_time = true;
}

float distance_mult = 3.0f;  //  This controls how far back the camera is behid the player.  TODO:  Make this longer when boosting.
float min_dist_mult = 0.8f;
float lie_distance_mult = 9.0f;
float surfer_dir_thresh = 20.0f;
float shoulder_above_head = 0.5f;
float y_percentage = 0.25f;
float z_val = -.5f;
float along_wave_thresh = 0.35f;
float lie_cam_offset = 0.5f;
float shoulder_filter_val = 0.35f;
float shoulder_dist_filter_val = 0.9f;
float lie_filter_val = 0.3f;
float shoulder_towards_beach = 0.5f;
float original_y_bias = 0.8f;
float shoulder_extra_lie_y = 0.7f;
float air_time_mult = 0.5f;

void shoulder_camera::frame_advance (time_value_t time_step)
{
	int current_beach = g_game_ptr->get_beach_id ();
	bool right = !BeachDataArray[current_beach].bdir;
	int current_state = ksctrl->get_current_state();
	bool is_stalling = current_state == STATE_STALL || current_state == STATE_SUPER_STALL;
	bool is_lying = ksctrl->get_super_state() == SUPER_STATE_LIE_ON_BOARD;
	bool is_in_air = ksctrl->get_board_controller().InAir();
	po trans (po_identity_matrix);
	vector3d desired_pos;
	float adjusted_shoulder_above_head = shoulder_above_head;


	//  Figure out where the camera will go if it is following the surfer on the wave.
	vector3d board_pos = get_target_entity ()->get_abs_position ();
	vector3d last_surfer_cam;
	vector3d water_current_vel = ksctrl->get_board_controller().current;
	vector3d camera_pos;
	float adj_distance_mult = distance_mult;
	float surfer_velocity;
	float tube_difference = 0.0f;
	float adjusted_filter_val = shoulder_filter_val;

	if (first_time)
		last_tube_difference = 0.0f;

	//  Only update surfer_cam if we are using it this frame, otherwise we'll just interpolate from the last one.
	if (!(is_in_air && get_target_entity()->get_abs_position().y > OVERHEAD_CAM_THRESHHOLD))
	{
		//  A vector going behind the board.
		vector3d board_vec = -(get_target_entity ()->get_abs_po ().slow_xform (vector3d (1, 0, 0)) - board_pos);

		//  Check to see if we just landed from a vert trick, and replace the camera (for tweening them) if we did.
		if (use_delta_cam)
		{
			use_delta_cam = false;
			last_surfer_cam = delta_cam;
		}
		else
			last_surfer_cam = surfer_cam;  //  Otherwise just use what was there before.

		int board_state = ksctrl->get_board_controller().GetBoardState();
		if (!is_stalling && board_state != BOARD_GRIND && board_state != BOARD_FLOAT)
		{
			surfer_cam = ksctrl->get_board_controller().rb->linMom;

			surfer_cam.x -= water_current_vel.x;
			surfer_cam.y = 0;
			surfer_cam.z -= water_current_vel.z;

			surfer_velocity = surfer_cam.length();

			surfer_cam.normalize();


			//  If the surfer isn't going very fast, then start leaning towards the direction that the board is pointed.
			if (surfer_velocity < surfer_dir_thresh)
			{
				//  how much under the threshhold.
				float board_cam_dif = (surfer_dir_thresh - surfer_velocity)/surfer_dir_thresh;
				interpolate_vector(&surfer_cam,surfer_cam,board_vec,board_cam_dif);  //  Only do this if the board isn't completely turned around.
			}

			//  If the camera is close to being straight down the x-axis, then nudge it onto the x-axis.
			//  Also put it on the axis if the surfer is stalling.
			float camera_facing_along_wave = dot(surfer_cam, ZVEC);
			if (fabs(camera_facing_along_wave) < along_wave_thresh)
			{
				float difference;

				if (surfer_cam.z <= 0)
					difference = fabs(camera_facing_along_wave)/along_wave_thresh;
				else
					difference = 0;  //  Don't allow the camera to get closer to the wave than the surfer.

				if (surfer_cam.x < 0)
					interpolate_vector(&surfer_cam, surfer_cam, -XVEC, 1.0f - difference);
				else
					interpolate_vector(&surfer_cam, surfer_cam, XVEC, 1.0f - difference);
			}

			surfer_cam.z += shoulder_towards_beach;
		}
		else if (is_stalling) //  stalling, so just put the camera behind the surfer along the length of the wave.
		{
			if (board_vec.x < 0)
				surfer_cam = -XVEC;
			else
				surfer_cam = XVEC;
		}
		else	//  Some form of grinding or floater, put the camera directly behind him.
		{
			if (ksctrl->get_board_controller().rb->linMom.x < 0)
				surfer_cam = -XVEC;
			else
				surfer_cam = XVEC;
		}

		surfer_cam.y = -fabs(surfer_cam.length() * y_percentage);  //  figure out the velocity without the y, then make y half that.
		surfer_cam.normalize();

	}


	if (is_lying && current_state != STATE_LIETOSTAND)  //  Spin the camera around behind the player while he is standing up.
	{
		use_delta_cam = true;  //  This is a good place to initialize this.

		//  Set the camera up in front of the guy.
		delta_cam = ZVEC;

		if (right)
			delta_cam.x -= lie_cam_offset;
		else
			delta_cam.x += lie_cam_offset;

		delta_cam.y -= shoulder_extra_lie_y;

		delta_cam.normalize();

		camera_pos = delta_cam;

		//  Put the camera a little farther away.
		adj_distance_mult = lie_distance_mult;
	}
	else
	{
		static time_value_t jump_time_elapsed = 0.0f;

		if (current_state == STATE_LAUNCH)
			lip_jump = true;
		else if (current_state == STATE_CHOP_HOP)
			lip_jump = false;


		if (current_state == STATE_FREEFALL || current_state == STATE_LAUNCH ||
			current_state == STATE_AIR_TRICK)
		{
			float difference;
			float lean_difference;
			static vector3d in_air_cam;
			static vector3d lean_vector;
			float air_time = air_time_mult * ksctrl->get_board_controller().GetTotalAirTime();  //  How long to the top of the jump

			use_delta_cam =  true;  //  Next time we hit the wave, surfer_cam should change its value to delta_cam's.

			if (!jump_time_elapsed)
			{
				in_air_cam = ksctrl->get_board_controller().GetForwardDir();

				//  Line up the camera with the perfect landing vector.
				in_air_cam.z = z_val*in_air_cam.y;
				in_air_cam.y = -fabs(in_air_cam.y);
				in_air_cam.normalize();

				//  If the camera is in danger of going stright over the surfer's head, figure out an offset angle.
				camera_angle_change = dot(in_air_cam,surfer_cam);
				if (camera_angle_change < -.3)
				{
					lean_vector = (0.5f * in_air_cam) + (0.5f * surfer_cam);
					if (surfer_cam.x >= 0)
						lean_vector = (0.2f * lean_vector) + (0.8f * XVEC);
					else
						lean_vector = (0.2f * lean_vector) + (0.8f * -XVEC);
				}
			}

			jump_time_elapsed = min(air_time, jump_time_elapsed + time_step);

			//  Figure out how long we've been in the air.
			if (air_time)
				difference = jump_time_elapsed/air_time;
			else
				difference = 0;

			interpolate_vector(&delta_cam, surfer_cam, in_air_cam, difference, 3);

			//  Check to see if the angle between the old camera and the new one is large.  If so, then
			//  we need to push it over to the side a bit so that it won't spin around to quickly
			//  in an effort to keep the character right side up on the screen.
			if (camera_angle_change < -.3)
			{
				if (difference <= .5)
					lean_difference = difference * 2;
				else
					lean_difference = 2 - (difference * 2);

				lean_difference = 1 - ((1 - lean_difference) * (1 - lean_difference));  //  Weight the difference up.
				lean_difference *= -camera_angle_change;

				interpolate_vector(&delta_cam, delta_cam, lean_vector, lean_difference);
			}

			camera_pos = delta_cam;
		}  //  end if doing vert stuff.
		else  //  Just surfing flat on the wave.
		{
			//  Reset these, since we are not in the air right now.
			jump_time_elapsed = 0.0f;

			int current_region = ksctrl->get_board_controller().GetRegion();
			if (current_region != WAVE_REGIONWASH && current_region != WAVE_REGIONROOF &&
				!is_in_air)
			{
				//  Now check to see if the camera should be shifting to the tube_cam.
				vector3d tube_cam(0.0f, 0.0f, 0.0f);
				tube_difference = 1 - max(0.0f, ksctrl->Closest_Tube_Distance());  //  distance from nearest tube.
				tube_difference = max(0.0f, (tube_difference - 0.5f)) * 2;  //  Don't use the full tube dist.
				if (last_tube_difference < 1.0f && tube_difference >= 1.0f)
					tube_dist_filter.Init_Filter(tube_difference);
				last_tube_difference = tube_difference;

				if (tube_difference)
				{
					if (tube_difference >= 1)
						adj_distance_mult = WaveDataArray[current_beach].tube_cam_dist;  //  Shift the camera closer also.

					Get_Tube_Cam_Vector(ksctrl, &tube_cam, get_target_entity(), WaveDataArray[current_beach].tube_cam_dist);		//  Vector towards the tube line.
					tube_dist_filter.Filter_Float(tube_difference, time_step, shoulder_filter_val);
					tube_difference = min(1.0f, max(0.0f, tube_difference));
				}

				//  If we're getting close to the tube, then tube_difference will be > 0 and the camera will shift towards the tube.
				interpolate_vector(&camera_pos,surfer_cam,tube_cam, tube_difference);
			}

			//  On the rebound, keep the camera turned away from the water.
			if (current_state == STATE_RIGHT_REBOUND || current_state == STATE_LEFT_REBOUND ||
				current_state == STATE_RIGHTREVERTCUTBACK || current_state == STATE_LEFTREVERTCUTBACK ||
				current_state == STATE_RIGHT_TAIL_CHUCK || current_state == STATE_LEFT_TAIL_CHUCK)
				camera_pos.z = fabs(camera_pos.z);

			//  Depending on how much the surfer is facing the beach, move the camera closer so that it doesn't clip the wave.
			if (ksctrl->get_super_state() != SUPER_STATE_IN_TUBE)
			{
				vector3d orientation_vec = camera_pos;
				orientation_vec.y = 0;
				orientation_vec.normalize();
				float camera_towards_beach = dot(orientation_vec, -ZVEC);  //  how far the camera is pointed towards the beach.
				if (camera_towards_beach > 0)
				{
					float near_the_lip = max(1.0f, ksctrl->Lip_Distance());
					near_the_lip = 1 - (near_the_lip * near_the_lip);
					float clip_distance_mod = 1 - (camera_towards_beach * near_the_lip);
					adjusted_filter_val = min(0.9f, adjusted_filter_val + 1 - clip_distance_mod);
					adj_distance_mult = min_dist_mult + ((adj_distance_mult - min_dist_mult) * clip_distance_mod);
				}
			}
			else
				adjusted_shoulder_above_head = 0.0f;  //  While we're here (in tube controls), let's set the camera lower than usual for the tube.

		}  //  end if doing plain surfing on the wave
	}


	if (first_time)
	{
		first_time = false;
		shoulder_cam_filter.Init_Filter(camera_pos);
		dist_mult_filter.Init_Filter(adj_distance_mult);
	}

	if (is_lying)  //  Spin the camera around slower when the player is getting up off the board.
	{
		shoulder_cam_filter.Filter_Vector(camera_pos, time_step, lie_filter_val);
		dist_mult_filter.Filter_Float(adj_distance_mult, time_step, lie_filter_val);
	}
	else
	{
		shoulder_cam_filter.Filter_Vector(camera_pos, time_step, shoulder_filter_val);
		dist_mult_filter.Filter_Float(adj_distance_mult, time_step, shoulder_dist_filter_val);
	}
	camera_pos.normalize();


	desired_pos = board_pos - (adj_distance_mult*camera_pos);  //  Final position of camera.

	trans.set_position (desired_pos + (YVEC * adjusted_shoulder_above_head));

	vector3d new_target = get_target_entity ()->get_abs_position () + (YVEC * adjusted_shoulder_above_head);

	trans.set_facing (new_target);

	//  Make the camera roll around the y orientation of the guy just a little.
	vector3d original_y = trans.get_y_facing();
	vector3d y = 4.0f * get_rel_po ().get_y_facing () + ksctrl->get_board_controller().rb->my_po.get_y_facing ();
	y = ((1 - original_y_bias) * y) + (original_y_bias * original_y);
	y.normalize ();

	vector3d x = cross (trans.get_z_facing (), y);

	trans = po (x, y, trans.get_z_facing (), trans.get_position ());

	set_rel_po (trans);
}


//  This is the vector from the surfer to the tube.  It is global because More than one function might need it.
float Get_Tube_Cam_Vector(kellyslater_controller *ksctrl, vector3d *tube_vector, entity* board, float lag_distance)
{
	float camera_x;
	vector3d board_pos = board->get_abs_position();
	int current_beach = g_game_ptr->get_beach_id ();
	int current_wave = WAVE_GetIndex();

    assert(ksctrl);

	//  figure out which way the board is turned.
	vector3d board_vector = board->get_abs_po ().slow_xform (vector3d (1, 0, 0));
	if (board_pos.x > board_vector.x)
		camera_x = board_pos.x - lag_distance;
	else
		camera_x = board_pos.x + lag_distance;

	vector3d tube_start;
	vector3d tube_stop;

	tube_start.x = WAVE_MeshMinX;
	tube_start.y = WaveDataArray[current_wave].tubecenstart_y;
	tube_start.z = WaveDataArray[current_wave].tubecenstart_z;

	tube_stop.x = WAVE_MeshMaxX;
	tube_stop.y = WaveDataArray[current_wave].tubecenstop_y;
	tube_stop.z = WaveDataArray[current_wave].tubecenstop_z;


	if (!BeachDataArray[current_beach].bdir)
	{
		if (camera_x < tube_stop.x)  //  Camera is completely inside the tube.
		{
			float difference = tube_stop.x - camera_x;
			difference /= tube_stop.x - tube_start.x;
			difference = min(1.0f, difference);  //  just in case the surfer goes behind the tube in a wipeout.

			tube_vector->x = camera_x;
			tube_vector->y = interpolate_float(tube_stop.y, tube_start.y, difference);
			tube_vector->z = interpolate_float(tube_stop.z, tube_start.z, difference);
		}
		else
		{
			tube_vector->x =  tube_stop.x;
			tube_vector->y =  tube_stop.y;
			tube_vector->z =  tube_stop.z;
		}
	}
	else
	{
		if (camera_x > tube_start.x)  //  Camera is completely inside the tube.
		{
			float difference = camera_x - tube_start.x;
			difference /= tube_stop.x - tube_start.x;
			difference = min(1.0f, difference);  //  just in case the surfer goes behind the tube in a wipeout.

			tube_vector->x = camera_x;
			tube_vector->y = interpolate_float(tube_start.y, tube_stop.y, difference);
			tube_vector->z = interpolate_float(tube_start.z, tube_stop.z, difference);
		}
		else
		{
			tube_vector->x = tube_start.x;
			tube_vector->y = tube_start.y;
			tube_vector->z = tube_start.z;
		}
	}

	*tube_vector = board_pos - *tube_vector;
	tube_vector->normalize();

	return min(1.f, max(0.f, -ksctrl->Tube_Distance()));
}

// ============================================================================
// Stationary Camera

stationary_camera::stationary_camera (const entity_id& _id, entity* _target_entity)
  : game_camera (_id, _target_entity)
{
  current_point = camera_points.end ();
  change_timer = 0;
}

void stationary_camera::sync (camera& b)
{
}

void stationary_camera::frame_advance (time_value_t t)
{
  if (camera_points.size () == 0)
    return;

  vector3d target (get_target_entity ()->get_abs_position ());

  if (current_point == camera_points.end ())
    current_point = camera_points.begin ();

  if (change_timer < 0)
  {
    POINT_LIST::iterator point_i = camera_points.begin ();
    POINT_LIST::iterator point_i_end = camera_points.end ();
    float cur_len2 = ((*current_point) + g_beach_ptr->travel_distance - target).length2 ();

    // find closest camera to the surfer
    while (point_i != point_i_end)
    {
      vector3d v (*point_i);

      if (cur_len2 > (v + g_beach_ptr->travel_distance - target).length2 ())
      {
        current_point = point_i;
        change_timer = 3.0f;
      }

      point_i++;
    }
  }
  else
    change_timer -= t;

	po trans (po_identity_matrix);

  trans.set_position (*current_point + g_beach_ptr->travel_distance);
	trans.set_facing (target);

  set_rel_po (trans);
}

void stationary_camera::add_point (const vector3d & pt)
{
  camera_points.push_back (pt);
}

// ============================================================================
// Stationary Camera

fps_camera::fps_camera (const entity_id& _id, entity* _target_entity,  kellyslater_controller *_ksctrl)
  : game_camera (_id, _target_entity)
{
	set_ks_controller(_ksctrl);
}

void fps_camera::sync (camera& b)
{
}

void fps_camera::init()
{
}

float fps_height = 0.4;


void fps_camera::frame_advance (time_value_t t)
{
	vector3d offset_vec = YVEC*fps_height;
	entity *head = ((conglomerate *)ksctrl->get_owner())->get_member("BIP01 HEAD");
	po trans (head->get_abs_po());
	if (g_game_ptr->GetSurferIdx(ksctrl->get_player_num()) == SURFER_SURFREAK)
	{
		vector3d freak_facing = ksctrl->get_board_controller().GetForwardDir();
		freak_facing.y = 0;
		freak_facing.normalize();
		freak_facing *= fps_height;
		offset_vec += freak_facing;
	}
	if (head)
	{
		trans.set_position(trans.get_position() + offset_vec);
		trans.set_facing(get_ks_controller()->get_board_controller().GetForwardDir() + head->get_abs_position() + offset_vec);
	}
  set_rel_po (trans);
}

// ============================================================================
// Flyby Camera
/*
flyby_camera::flyby_camera (const entity_id& _id, entity* _target_entity)
  : game_camera (_id, _target_entity)
{
  current_point = camera_points.end ();
  change_timer = 0;
}

void flyby_camera::sync (camera& b)
{
}

void flyby_camera::frame_advance (time_value_t t)
{
  if (camera_points.size () == 0)
    return;

  vector3d target (get_target_entity ()->get_abs_position ());

  if (change_timer <= 0)
  {
    current_point++;
    change_timer = (*current_point).time;
  }
  else
    change_timer -= t;

  if (current_point == camera_points.end ())
  {
    current_point = camera_points.begin ();
    change_timer = (*current_point).time;
  }

  po trans (po_identity_matrix);

  float f = ((*current_point).time - change_timer) / (*current_point).time;
  POINT_ID next = current_point;
  next++;

  if (next == camera_points.end ())
    next = current_point;

  trans.set_position ((*current_point).pt + (((*next).pt - (*current_point).pt) * f) + g_beach_ptr->travel_distance);
	trans.set_facing (target);

  set_rel_po (trans);
}

void flyby_camera::add_point (const vector3d & pt, float time)
{
  PATH_POINT pp;
  pp.pt = pt;
  pp.time = time;

  camera_points.push_back (pp);
}
*/

float BEACH_CAM_Y_TILT_THRESHOLD = 4.0f;
float BEACH_CAM_Y = 4.0f;
float BEACH_CAM_Z = 50.0f;
float BEACH_CAM_FOV = 20;
float BEACH_VEL_X_SCALE = .1f;
float BEACH_CAM_PITCH = -.05;
float BEACH_CAM_TILT_SCALE = .2;
float BEACH_CAM_Z_DIFF_SCALE = .4;
static float BEACH_CAM_YAW = 0.496f;	// the previous ones don't seem to be used (dc 07/05/01)
static vector3d BEACH_CAM_DES(4.0f, 1.6f, -7.38f);
float magx = -4.0f;
float magy = 1.0f;
float magz = 5.38f;
float magyaw = 0.496f;
float MAX_ANGLE = 45.0f;
float DELTA_ANGLE = 10.0f;
float BREAK_MOD = -0.35f;
float ZOOM_MOD = 0.8f;
float BREAK_MOD2 = -0.64f;
float ZOOM_MOD2 = 1.5f;
float CLOSE_OUT_TIME = 0.5f;
#define DELTA_ZOOM 17.0f;

auto_camera::auto_camera( const entity_id& _id, entity* _target_entity, kellyslater_controller *_ksctrl )
  :   game_camera( _id, _target_entity )
{
  /* create_link_ifc() */  //  eventually this will have to be uncommented when bones do not have a default link interface
	delta_vec = vector3d(0,0,0);
  set_ks_controller(_ksctrl);
//  Reset();	// If possible, delay until wave has been loaded (dc 01/24/02)
}

void auto_camera::Reset( void )
{
	BEACH_CAM_DES.x = magx;
	BEACH_CAM_DES.y = magy;
	BEACH_CAM_DES.z = -magz;

	// Camera faces the other direction for left-breakers
	int curbeach = g_game_ptr->get_beach_id();
	if (BeachDataArray[curbeach].bdir)
	{
		BEACH_CAM_YAW = -magyaw;
		BEACH_CAM_DES.x = -magx;
	}
	else
	{
		BEACH_CAM_DES.x = magx;
		BEACH_CAM_YAW = magyaw;
	}

	break_cam_state = BREAK_CAM_OFF;
	init_filter = true;
	frame_advance(0.0f);
}

void auto_camera::sync( camera& b )
{
}

float g_object_dist_limit = 40000.0f;
void auto_camera::FindNearestObject(void)
{
	vector3d lipnorm = ksctrl->get_board_controller().GetLipNormal();
	if (g_beach_ptr && g_beach_ptr->get_nearest_object_pos(get_target_entity()->get_abs_position(), object_pos, lipnorm))
	{
		if (dot(object_pos, object_pos) < g_object_dist_limit)
		{
			object_near = true;
		}
		else
			object_near = false;
	}
	else
		object_near = false;
}

vector3d auto_camera::GetPosDifference(float t)
{
	float reverse = 0.0f;
	float delta = 25.0f;
	vector3d pos = get_target_entity()->get_abs_position();
	float zoom_factor = 1.0f;
	float break_factor = 1.0f;
	float break_point;
	float break_angle_factor = 1.0f;
	int curbeach = g_game_ptr->get_beach_id();
	vector3d lip_vec = XVEC;
	if (BeachDataArray[curbeach].bdir)
	{
		reverse = 1.0f;
		delta = -delta;
		lip_vec = -XVEC;
	}

	const vector3d *breakp = WAVE_GetMarker(WAVE_MarkerBreak);
	break_point = breakp->x;
	zoom_factor = 0.85f*BeachDataArray[curbeach].zoom_factor;

	float break_pos_x;
	FindNearestObject();
	if ((break_cam_state != BREAK_CAM_OFF) && !InLaunchState())
	{
		float sign = 1.0f;
		float new_delt = fabs(dot(lip_vec, *WAVE_GetMarker(WAVE_MarkerLipMark11p45) - *WAVE_GetMarker(WAVE_MarkerBreak)));
		if (reverse)
		{
			sign = -1.0f;
			new_delt = -new_delt;
		}

		if (wave_info.type == WAVE_PerturbTypeBigTongue)
		{
			break_angle_factor = 1.45f;
			if (sign*pos.x < sign*(break_point + new_delt))
			{
				delta = new_delt;
				zoom_factor *= ZOOM_MOD;
				break_factor = BREAK_MOD;
			}
			else
				delta = sign*(fabs(new_delt) + 20.0f);
		}
		break_pos_x = break_point + delta;
	}
	else if (object_near && (dot(lip_vec, object_pos) > -2.0f))
	{
		break_pos_x = pos.x + 20.0f*(reverse?1.0f:-1.0f);
		break_angle_factor = 1.4f;
	}
	else
		break_pos_x = break_point + delta;

	float angle = break_factor*5.2f*(pos.x - break_pos_x);
	if (angle > break_angle_factor*(MAX_ANGLE + reverse*DELTA_ANGLE))
		angle = break_angle_factor*(MAX_ANGLE + reverse*DELTA_ANGLE);
	else if (angle < break_angle_factor*(-MAX_ANGLE - !reverse*DELTA_ANGLE))
		angle = -break_angle_factor*(MAX_ANGLE + !reverse*DELTA_ANGLE);

	float c = fast_cos(DEG_TO_RAD(angle));
	float s = fast_sin(DEG_TO_RAD(angle));
	float x = -s*7.0f;
	float z = -c*7.0f;
	vector3d diff = vector3d(x, BEACH_CAM_DES.y, z);
	return zoom_factor*diff;
}

float g_break_cam_work = 0.0f;
void auto_camera::DoBreakCameraWork(void)
{
	if (g_break_cam_work || !wave_info.onoff)
	{
		break_cam_state = BREAK_CAM_OFF;
		return;
	}

	int stage = wave_info.stage;
	if ((wave_info.type == WAVE_PerturbTypeBigTongue) || (wave_info.type == WAVE_PerturbTypeTongue))
	{
		if (((stage == WAVE_PerturbStageDo) && (wave_info.stageprogress > 0.5f)) || (stage == WAVE_PerturbStageHold)
				|| ((stage == WAVE_PerturbStageCollapse) && (wave_info.stageprogress <= 0.5f)))
		{
				break_cam_state = BREAK_TONGUE_NORMAL;
		}
		else if (((stage == WAVE_PerturbStageCollapse) && (wave_info.stageprogress > 0.5f)) || (stage == WAVE_PerturbStageWait)
				|| ((stage == WAVE_PerturbStageUndo) && (wave_info.stageprogress < 0.2f)))
		{
				break_cam_state = BREAK_TONGUE_COLLAPSE;
		}
		else
			break_cam_state = BREAK_CAM_OFF;
	}
	else if ((wave_info.type == WAVE_PerturbTypeBigSurge) || (wave_info.type == WAVE_PerturbTypeSurge))
	{
		if (((stage == WAVE_PerturbStageDo) && (wave_info.stageprogress > 0.3f)) || (stage == WAVE_PerturbStageHold)
				|| (stage == WAVE_PerturbStageCollapse) || (stage == WAVE_PerturbStageHold)
				|| ((stage == WAVE_PerturbStageUndo) && (wave_info.stageprogress < 0.5f)))
				break_cam_state = BREAK_SURGE;
		else
			break_cam_state = BREAK_CAM_OFF;
	}
	else if ((wave_info.type == WAVE_PerturbTypeBigRush) || (wave_info.type == WAVE_PerturbTypeRush))
	{
		if (((stage == WAVE_PerturbStageDo) && (wave_info.stageprogress > 0.0f)) || (stage == WAVE_PerturbStageHold)
				|| (stage == WAVE_PerturbStageCollapse) || (stage == WAVE_PerturbStageHold)
				|| ((stage == WAVE_PerturbStageUndo) && (wave_info.stageprogress < 1.0f)))
				break_cam_state = BREAK_RUSH;
		else
			break_cam_state = BREAK_CAM_OFF;
	}
	else
		break_cam_state = BREAK_CAM_OFF;
}

bool auto_camera::InLaunchState(void)
{
//	int current_state = ksctrl->get_current_state();
	float air_time = ksctrl->get_board_controller().GetTotalAirTime();
	float air_timer = ksctrl->get_board_controller().GetAirTime();
	return (air_time && (air_timer/air_time < 0.98f)
				&& ksctrl->get_board_controller().lip_jump);
}

float tube_offest_val = 0.3f;
float stalling_offset_val = 0.2f;
float g_freq_scale = 1.0f;
float gBeachCamZoom = 0.0f;
float lie_extra_y = 2.0f;
float beach_above_head = 0.20f;
float g_lipvec_mod = 2.7f;
float g_zvec_mod = 3.3f;

#ifdef DEBUG
float g_auto_cam_speed_mod = 1.3f;
#else
float g_auto_cam_speed_mod = 1.0f;
#endif

void auto_camera::frame_advance( time_value_t t )
{
//	int current_beach = g_game_ptr->get_beach_id ();

	if (!ksctrl)
		return;

	bool is_stalling = false;
	bool is_in_tube_controls = ksctrl->get_super_state() == SUPER_STATE_IN_TUBE;
	vector3d DesPos, pos = get_target_entity()->get_abs_position();
	WAVE_GetBreakInfo(&wave_info);	//  update wave info
	DoBreakCameraWork();
	int curbeach = g_game_ptr->get_beach_id();
	int super_state = ksctrl->get_super_state();
	int state = ksctrl->get_current_state();
	bool skip_filter = false;
	bool lock_orientation = false;
	float front_scale = 1.0f - 4.5f*t;
	float back_scale = 4.5*t;

	if (super_state == SUPER_STATE_LIE_ON_BOARD)
	{
		/*delta_vec = 1.8f*vector3d(4.0f, 0.0f, 12.0f);
		delta_vec.x = -delta_vec.x;
		delta_vec.y = 0.0f;
		delta_vec.z = -fabs(delta_vec.z);
		delta_vec.normalize();
		delta_vec = 4.5f*delta_vec;
		delta_vec.y = BEACH_CAM_DES.y + lie_extra_y;*/
		vector3d vec1 = ksctrl->get_board_controller().GetLipVec();
		delta_vec = -ZVEC*g_zvec_mod + vec1*g_lipvec_mod;
		delta_vec.y = BEACH_CAM_DES.y;
		Norm(delta_vec, delta_float);
		DesPos = pos + delta_float*delta_vec;
		cutoff_frequency = 0.8f;
	}
	else if ((state == STATE_STALL || state == STATE_SUPER_STALL) && ksctrl->IsUpDownThreshDone())
	{
		is_stalling = true;
		vector3d tube_cam;
		Get_Tube_Cam_Vector(ksctrl, &tube_cam, get_target_entity(), WaveDataArray[WAVE_GetIndex()].tube_cam_dist);
		tube_cam = -tube_cam;
		vector3d board_dir = BeachDataArray[curbeach].bdir?-XVEC:XVEC;
		bool behind = dot(board_dir, actual_vec*actual_float) < -0.5f;
		if (!behind)
			tube_cam.z = -fabs(tube_cam.z);

		if (!init_filter)
			delta_vec = (front_scale*delta_vec + back_scale*tube_cam)*WaveDataArray[WAVE_GetIndex()].tube_cam_dist;
		else
			delta_vec = tube_cam*WaveDataArray[WAVE_GetIndex()].tube_cam_dist;

		if (delta_vec.z > -0.25f)
			delta_vec.z = -0.25f;

		float length;
		Norm(delta_vec, length);

		if (!init_filter)
			delta_float = (front_scale*actual_float + back_scale*length);
		else
			delta_float = length;

		DesPos = pos + delta_float*delta_vec;
		cutoff_frequency = 0.8f;
		low_pass_filter.Init_Filter(DesPos);
		skip_filter = true;
	}
	else if (is_in_tube_controls)
	{
		//  When in the tube, have the camera look out from the tube at the surfer.
		vector3d tube_cam;
		Get_Tube_Cam_Vector(ksctrl, &tube_cam, get_target_entity(), WaveDataArray[WAVE_GetIndex()].tube_cam_dist);
		tube_cam = -tube_cam;
		vector3d board_dir = BeachDataArray[curbeach].bdir?-XVEC:XVEC;
		bool behind = dot(board_dir, actual_vec*actual_float) < -0.5f;
		if (!behind)
			tube_cam.z = -fabs(tube_cam.z);

		if (!init_filter)
			delta_vec = WaveDataArray[WAVE_GetIndex()].tube_cam_dist*(front_scale*delta_vec + back_scale*tube_cam);
		else
			delta_vec = tube_cam*WaveDataArray[WAVE_GetIndex()].tube_cam_dist;

		float length;
		Norm(delta_vec, length);

		if (!init_filter)
			delta_float = (front_scale*actual_float + back_scale*length);
		else
			delta_float = length;

		DesPos = pos + delta_float*delta_vec;

		low_pass_filter.Init_Filter(DesPos);
		skip_filter = true;

	}
	else if (InLaunchState())
	{
		cutoff_frequency = 0.88f;
		delta_vec = -ZVEC;
		if (ksctrl->get_board_controller().exit_jump)
			delta_float = 4.0f;
		else if (gBeachCamZoom)
		{
			delta_float = 3.0f;
		}
		else
			delta_float = 5.0f;

		DesPos = pos + delta_float*delta_vec;
	}
	else if (ksctrl->get_board_controller().GetBoardState() == BOARD_GRIND)
	{
		vector3d error = low_pass_filter.GetCurrentError();
		float error_squared = dot(error, error);
		if (error_squared < 0.30f)
		{
			lock_orientation = true;
		}

		vector3d grind_vec = ksctrl->GetGrindVector();
		delta_vec = -1.25f*grind_vec - 3.5f*ZVEC + 0.625f*YVEC;
		Norm(delta_vec, delta_float);
		DesPos = pos + delta_float*delta_vec;
		cutoff_frequency = 0.8f;
	}
	else if (ksctrl->get_board_controller().grind_jump)
	{
		delta_float = 5.8f;
		delta_vec = -ZVEC;
		DesPos = pos + delta_float*delta_vec;
		cutoff_frequency = 0.8f;
	}
	else
	{
		//  scale filter by gravity, in order to allow the cuttoff frequency to change appropriately
		//float scale = BeachDataArray[curbeach].board_speed*(0.76923f - (BeachDataArray[curbeach].board_speed - 1.3f)*0.1f);
		//float scale = WaveDataArray[curbeach].cutoff_freq*BeachDataArray[curbeach].gravity/65.0f;
		float scale = BeachDataArray[curbeach].cutoff_freq*0.667f;
		delta_vec = GetPosDifference(t);
		float length;
		Norm(delta_vec, length);
		if (!init_filter)
			delta_float = ((1.0f - 4.0f*t)*delta_float + 4.0f*t*length);
		else
			delta_float = length;

		DesPos = pos + delta_float*delta_vec;

		if (break_cam_state == BREAK_CAM_OFF)
			cutoff_frequency = 1.1f*scale*g_freq_scale;
		else
			cutoff_frequency = 1.0f;
	}

	if (!skip_filter && !init_filter)
		low_pass_filter.Filter_Vector(DesPos, t, cutoff_frequency*g_auto_cam_speed_mod);
	else if (init_filter)
		low_pass_filter.Init_Filter(DesPos);

	actual_vec = DesPos - pos;
	Norm(actual_vec, actual_float);

	//  This makes the camera look much more along the wave when the camera is being set to the tube vector.
	vector3d facing_offset(0,0,0);
	vector3d facing_pos = pos;
	if (is_in_tube_controls || is_stalling)
	{
		facing_offset.y = (DesPos.y - facing_pos.y) * tube_offest_val;
		facing_offset.z = (DesPos.z - facing_pos.z) * tube_offest_val;
	}

	if (init_filter)
	{
		facing_offset_filter.Init_Filter(facing_offset);
	}
	else
		facing_offset_filter.Filter_Vector(facing_offset, t, 0.7f);

	po mine;
	if (lock_orientation)
	{
		mine = this->get_rel_po();
		mine.set_position(DesPos);
	}
	else
	{
		mine.set_position(DesPos);
		mine.set_facing(facing_pos + facing_offset);
	}

	this->set_rel_po(mine);
	init_filter = false;
}

#if defined(TARGET_XBOX)
beach_camera::beach_camera( const entity_id& _id, entity* _target_entity, kellyslater_controller *_ksctrl )
#else
beach_camera::beach_camera( const entity_id& _id, entity* _target_entity, kellyslater_controller *_ksctrl )
#endif /* TARGET_XBOX JIV DEBUG */
  : auto_camera (_id, _target_entity, _ksctrl)
{
}

float break_cam_follow = 2.0f;
float break_cam_board_ang = 0.3f;
float closeout_dist1 = 5.0f;
float closeout_dist2 = 3.0f;
float angle_factor = 0.707f;
float g_object_angle = 55.0f;
float g_beachcam_dist_scale = 1.0f;
vector3d beach_camera::GetPosDifference(float t)
{
	float reverse = 0.0f;
	float delta = fabs(WAVE_GetMarker(WAVE_MarkerLipMark11p1)->x - WAVE_GetMarker(WAVE_MarkerLipMark8p5)->x);
	vector3d pos = get_target_entity()->get_abs_position();
	float zoom_factor = 1.2f;
	float break_factor = 1.0f;
	float break_point;
	float break_angle_factor = 1.0f;
	int curbeach = g_game_ptr->get_beach_id();
	vector3d lip_vec = ksctrl->get_board_controller().GetLipVec();
	float sign = 1.0f;
	if (BeachDataArray[curbeach].bdir)
	{
		reverse = 1.0f;
		delta = -delta;
		//lip_vec = -XVEC;
		sign = -1.0f;
	}


	float near_tube_x = (WAVE_GetMarker(WAVE_MarkerEmitterStartCrestX)->x + WAVE_GetMarker(WAVE_MarkerEmitterStartLipX)->x)*0.5f;
	const vector3d *breakp = WAVE_GetMarker(WAVE_MarkerBreak);
	break_point = breakp->x;
	zoom_factor = 0.97f*BeachDataArray[curbeach].zoom_factor;

	float angle = -sign*35.0f;
	float break_pos_x = break_point + delta;	// was used uninitialized previously (dc 01/29/02)
	FindNearestObject();
	int region = ksctrl->get_board_controller().GetWaveRegion();
	bool in_tube = (region == WAVE_REGIONTUBE) && ksctrl->Z_Within_Tube();
	bool near_tube = (reverse && (pos.x > near_tube_x)) || (!reverse && (pos.x < near_tube_x));
	if ((break_cam_state != BREAK_TONGUE_COLLAPSE) && (break_cam_state != BREAK_SURGE)
			&& ((in_tube && near_tube)
			|| ((break_cam_state == BREAK_RUSH) && in_tube)))
	{
		//  When in the tube, have the camera look in to the tube at the surfer.
		vector3d tube_cam = ksctrl->get_board_controller().GetLipVec();
		vector3d board_dir = BeachDataArray[curbeach].bdir?-XVEC:XVEC;
		bool behind = dot(board_dir, actual_vec*actual_float) < -0.5f;
		if (!behind)
			tube_cam.z = -fabs(tube_cam.z);

		if (!init_filter)
			delta_vec = 6.0f*((1.0f - 4.5f*t)*delta_vec + 4.5f*t*tube_cam);
		else
			delta_vec = 6.0f*tube_cam;

		if (delta_vec.z > -0.25f)
			delta_vec.z = -0.25f;

		return (delta_vec);
	}
	else if ((break_cam_state == BREAK_TONGUE_NORMAL) || (break_cam_state == BREAK_TONGUE_COLLAPSE)
												|| (break_cam_state == BREAK_SURGE))
	{
		float past_break;
		vector3d closeout_mid, closeout_end2;
		if (wave_info.type == WAVE_PerturbTypeSurge)
		{
			closeout_end2 = *WAVE_GetMarker(WAVE_MarkerSurgeEnd);
			closeout_mid = *WAVE_GetMarker(WAVE_MarkerLipMark11p1);
			past_break = dot(pos - closeout_end2, closeout_mid - closeout_end2);  // end is closer ot break than mid for Surge
		}
		else if (wave_info.type == WAVE_PerturbTypeBigTongue)
		{
			closeout_end2 = *WAVE_GetMarker(WAVE_MarkerLipMark11p8);
			closeout_mid = *WAVE_GetMarker(WAVE_MarkerLipMark11p45);
			past_break = dot(pos - closeout_end2, closeout_end2 - closeout_mid);
		}
		else
		{
			closeout_end2 = *WAVE_GetMarker(WAVE_MarkerLipMark11p45);
			closeout_mid = *WAVE_GetMarker(WAVE_MarkerLipMark11p1);
			past_break = dot(pos - closeout_end2, closeout_end2 - closeout_mid);
		}

		float cl_dist = fabs(closeout_end2.x - closeout_mid.x);
		if (dot((closeout_end2 - closeout_mid), (pos - closeout_mid)) < 0.0f)
			cl_dist += closeout_dist1;
		else
			cl_dist -= closeout_dist2;


		float new_delt = fabs(dot(lip_vec, closeout_mid - *WAVE_GetMarker(WAVE_MarkerBreak)));
		new_delt = sign*new_delt;

		float in_tongue = fabs(closeout_mid.x - pos.x) < cl_dist;
		if (in_tube && (((break_cam_state == BREAK_TONGUE_NORMAL) && in_tongue)
				|| ((break_cam_state == BREAK_TONGUE_COLLAPSE) && (past_break < 0.0f))
				|| ((break_cam_state == BREAK_SURGE) && (past_break < 0.0f))))
		{
			vector3d tube_cam;
			float dotp = dot(ksctrl->get_board_controller().GetForwardDir(), lip_vec);
			Get_Tube_Cam_Vector(ksctrl, &tube_cam, get_target_entity(), WaveDataArray[WAVE_GetIndex()].tube_cam_dist);
			if (dot(ksctrl->get_board_controller().GetForwardDir(), tube_cam) > 0.0f)
				tube_cam = -tube_cam;

			if (!init_filter)
			{
				delta_vec = ((1.0f - 8.0f*t)*delta_vec + 8.0*t*tube_cam);
				delta_float = ((1.0f - 5.0f*t)*delta_float + 5.0f*t*break_cam_follow);
				delta_vec *= delta_float;
			}
			else
				delta_vec = break_cam_follow*tube_cam;


			//delta_vec = tube_cam*break_cam_follow;
			if ((dot(delta_vec, actual_vec) < -0.5f) || (fabs(dotp) < 0.707f))
				delta_vec.z = -4.0f;

			return (delta_vec);
		}

		if (sign*pos.x < sign*break_pos_x)
		{
			delta = new_delt;
			zoom_factor *= BeachDataArray[curbeach].break_zoom;
			break_factor = BeachDataArray[curbeach].break_mod;
			//zoom_factor *= ZOOM_MOD2;
			//break_factor = BREAK_MOD2;
		}
		else
			delta = sign*(fabs(new_delt) + 20.0f);

		break_pos_x = break_point + delta;
		break_angle_factor = 1.45f;
		angle = break_factor*2.0f*(pos.x - break_pos_x);
	}
	else if (object_near && (dot(lip_vec, object_pos) > -2.0f))
	{
		break_pos_x = pos.x + 20.0f*(reverse?1.0f:-1.0f);
		break_angle_factor = 1.4f;
		zoom_factor *= 1.2f;
		angle = sign*g_object_angle;
	}

	if (angle > break_angle_factor*(MAX_ANGLE + reverse*DELTA_ANGLE))
		angle = break_angle_factor*(MAX_ANGLE + reverse*DELTA_ANGLE);
	else if (angle < break_angle_factor*(-MAX_ANGLE - !reverse*DELTA_ANGLE))
		angle = -break_angle_factor*(MAX_ANGLE + !reverse*DELTA_ANGLE);

	float c = fast_cos(DEG_TO_RAD(angle));
	float s = fast_sin(DEG_TO_RAD(angle));
	float x = -s;
	float z = -c;
	vector3d diff(x, 0.0f, z);
	vector3d dvec;
	if (!init_filter)
		dvec = ((1.0f - 4.0f*t)*delta_vec + 4.0f*t*diff)*5.8f;
	else
		dvec = diff*5.8f;

	diff = zoom_factor*g_beachcam_dist_scale*vector3d(dvec.x, BEACH_CAM_DES.y, dvec.z);
	return diff;
}


big_wave_camera::big_wave_camera( const entity_id& _id, entity* _target_entity, kellyslater_controller *_ksctrl )
  :   game_camera( _id, _target_entity )
{
  /* create_link_ifc() */  //  eventually this will have to be uncommented when bones do not have a default link interface

  set_ks_controller(_ksctrl);
  Reset();
}

float big_wave_diff_x = 2.0f;
float big_wave_diff_y = 2.0f;
float big_wave_diff_z = -18.0f;
void big_wave_camera::Reset( void )
{
	vector3d ent_center = get_target_entity()->get_abs_position();

	int curbeach = g_game_ptr->get_beach_id();
	float diff;
	if (BeachDataArray[curbeach].bdir)
		diff = -big_wave_diff_x;
	else
		diff = big_wave_diff_x;

	vector3d DesPos = ent_center + diff*XVEC + big_wave_diff_z*ZVEC + big_wave_diff_y*YVEC;
	low_pass_filter.Init_Filter(DesPos);

	cutoff_frequency = 0.95f;

	po mine;
	mine.set_position(ent_center + DesPos);
	mine.set_facing(ent_center);
	this->set_rel_po(mine);
}

float max_toward_wave = 0.8f;
float start_modifying_offset = 0.5f;  //  Lower value means less area near tube that's affected.
void big_wave_camera::frame_advance( time_value_t t )
{
	vector3d ent_center = get_target_entity()->get_abs_position();
	int curbeach = g_game_ptr->get_beach_id();
	float diff;
	if (BeachDataArray[curbeach].bdir)
		diff = -big_wave_diff_x;
	else
		diff = big_wave_diff_x;

	vector3d DesPos = ent_center + diff*XVEC + big_wave_diff_z*ZVEC + big_wave_diff_y*YVEC;

	//  Move the camera closer to the wave as it gets farther from the tube.  This allows us to see the tube better.
	float tube_dist_mod = ksctrl->Tube_Distance();
	tube_dist_mod = min( 1.0f, max(0.0f, tube_dist_mod + start_modifying_offset)) * max_toward_wave;  //  The range was -1 to 1.  Change it to 0 to 1.
	tube_dist_mod = 1 - tube_dist_mod;
	float change = (DesPos.z - ent_center.z) * (1 - tube_dist_mod);
	DesPos.z -= change * 0.5f;
	if (BeachDataArray[curbeach].bdir)
		DesPos.x += change;
	else
		DesPos.x -= change;

	low_pass_filter.Filter_Vector(DesPos, t, cutoff_frequency);

	po mine;
	mine.set_position(DesPos);
	mine.set_facing(ent_center);
	this->set_rel_po(mine);
}

wipeout_camera_2::wipeout_camera_2 (const entity_id& _id, entity* _target_entity)
  : game_camera (_id, _target_entity)
{
  /* create_link_ifc() */  //  eventually this will have to be uncommented when bones do not have a default link interface
  previous_camera = NULL;
  wave_hint_valid = false;
}

wipeout_camera_2::~wipeout_camera_2 ()
{
}

void wipeout_camera_2::init(po initial_po, camera *prev_cam)
{
	previous_camera = prev_cam;
	start_point = prev_cam->get_abs_position();
	lowest_y = start_point.y;
	this->set_rel_po(prev_cam->get_abs_po());
	rel_vec = start_point - get_ks_controller()->get_owner()->get_abs_position();
	camera_filter.Init_Filter(start_point);
	rel_vec.y = 0.0f;
	rel_vec.normalize();
	wave_hint_valid = false;
	water_region = (WaveRegionEnum) ksctrl->get_board_controller().GetWaveRegion();
}


float wip_mag = 3.5f;
float wip_filter_speed = 1.2f;
float dist_below = 1.5f;
float dist_below_surf = 0.8f;
float g_wipcam_dist_away = 3.0f;
void wipeout_camera_2::frame_advance (time_value_t time_step)
{
	float filter_speed = wip_filter_speed;
	vector3d new_target, target_vec = rel_vec;
	int wipeout_state = ksctrl->GetWipeoutPhysState();
	bool is_swimming = wipeout_state == WIP_PHYS_TUMBLE || wipeout_state == WIP_PHYS_SWIM;
	if (ksctrl->IsGoofy())
		new_target = ((conglomerate*) get_target_entity ())->get_member("BIP01 PELVIS")->get_handed_abs_position();
	else
		new_target = ((conglomerate*) get_target_entity ())->get_member("BIP01 PELVIS")->get_abs_position();

	WaveMarkerEnum front = WAVE_MarkerInvalid;
	WaveMarkerEnum back = WAVE_MarkerInvalid;
	bool limit_z = false;
	if (((water_region == WAVE_REGIONTUBE) || (water_region == WAVE_REGIONCEILING)) && !is_swimming)
	{
		limit_z = true;
		front = WAVE_MarkerTubeMinZ;
		back = WAVE_MarkerTubeMaxZ;
	}

	CollideCallStruct collide (this->get_abs_position(), &water_normal, NULL, &water_region, &wave_hint);
	static vector3d prev_cam_pos;
	if (wave_hint_valid)
	{
		float deltadist = (this->get_abs_position() - prev_cam_pos).length();
		collide.tolerance.dthresh = collide.tolerance.zthresh = deltadist + 1;
	}
	else
	{
		collide.tolerance.dthresh = 1; 
		collide.tolerance.zthresh = 0;	// will trigger error if hint used (dc 05/04/02)
	}
	WAVE_CheckCollision (collide, wave_hint_valid, false, false, limit_z, front, back);
	prev_cam_pos = collide.position;
	wave_hint_valid = true;
	water_region = *collide.region;

	if (!is_swimming)
	{
		target_vec = start_point;
		target_vec.y = collide.position.y + 1.0f;

		rel_vec = this->get_abs_position() - new_target;
		rel_vec.y = 0.0f;
		float length = rel_vec.length();
		if (length > 0.0f)
		{
			rel_vec /= length;
			if (length < g_wipcam_dist_away)
			{
				float ypos = target_vec.y;
				target_vec = new_target + rel_vec*g_wipcam_dist_away;
				target_vec.y = ypos;
			}
		}
	}
	else
	{
		filter_speed *= 1.5f;
		target_vec *= wip_mag;
		//target_vec += -dist_below*YVEC;
		target_vec.y = collide.position.y - new_target.y - dist_below_surf;
		target_vec = new_target + target_vec;
		wave_hint_valid = false;
	}

	if (target_vec.y < lowest_y)
		lowest_y = target_vec.y;
	else
		target_vec.y = lowest_y;

	camera_filter.Filter_Vector(target_vec, time_step, filter_speed);

	po mine;
	mine.set_position(target_vec);
	mine.set_facing(new_target);
	this->set_rel_po(mine);
}


wipeout_camera::wipeout_camera (const entity_id& _id, entity* _target_entity)
  : game_camera (_id, _target_entity)
{
	collision_object = NULL;
  previous_camera = NULL;
}

wipeout_camera::~wipeout_camera ()
{
}


void wipeout_camera::init(po initial_po, camera *prev_cam)
{
	just_started = true;
	was_swimming = false;
	starter_po = initial_po;
	lowest_z = 10000000;

	previous_camera = prev_cam;

	current_rotation = random(0.0f, (PI * 0.35f));
}


float behind_dist = 6.0f;
float below_dist = 4.0f;
float slightly_above = 2.5f;

float obj_below_dist = 3.0f;
float obj_behind_dist = 3.0f;


float slow_filter = 0.5f;
float fast_filter = 1.0f;
float obj_rotation_inc = 2.2f;
float general_rotation_inc = 0.4f;
float wipeout_cam_dist1 = 6.0f;

void wipeout_camera::frame_advance (time_value_t time_step)
{
	vector3d camera_pos;
	vector3d new_target;
	po trans(po_identity_matrix);
	int wipeout_state = ksctrl->GetWipeoutPhysState();
	bool is_swimming = wipeout_state == WIP_PHYS_TUMBLE || wipeout_state == WIP_PHYS_SWIM;
	bool reset_camera = false;

	if (!collision_object)  //  Just a general fall.
	{
		//  Push the camera lower down, but not much lower than the surfer and move the camera close to the surfer.
		if (is_swimming)
		{
			new_target = get_target_entity()->get_abs_position() - 3.7*YVEC;
			wipeout_cam_dist1 = 6.0f;
		}
		else
			new_target = ((conglomerate*) get_target_entity ())->get_member("BIP01 PELVIS")->get_abs_position();

		camera_pos.y = new_target.y;
		camera_pos.z = new_target.z;
		camera_pos.x = new_target.x - behind_dist;

		if (!is_swimming)	//  Above the water, look down.
		{
			camera_pos.y += slightly_above;
		}
		else	//  Below the water, look up.
		{
			//  Now that we have the base positions, rotate the camera around the object.
			current_rotation += general_rotation_inc * time_step;
			if (current_rotation > PI)
				current_rotation -= 1.5 * PI;

			rotate_cam(&camera_pos, new_target, current_rotation);

			camera_pos.y -= below_dist;

			if (is_swimming != was_swimming)
				reset_camera = true;
		}

		was_swimming = is_swimming;

		vector3d cam_dif;

		float camera_dist;
		if (just_started)
		{
			just_started = false;
			cam_dif = starter_po.get_position() - new_target;
			camera_filter.Init_Filter(cam_dif);
			camera_dist = cam_dif.length();
			distance_filter.Init_Filter(camera_dist);
		}
		else
		{
			cam_dif = camera_pos - new_target;

			camera_dist = wipeout_cam_dist1;

			if (reset_camera)
			{
				camera_filter.Init_Filter(cam_dif);
				distance_filter.Init_Filter(camera_dist);
			}
			else
			{
				camera_filter.Filter_Vector(cam_dif, time_step, slow_filter);
				distance_filter.Filter_Float(camera_dist, time_step, slow_filter);
			}
		}

		cam_dif.normalize();
		camera_pos = new_target + (cam_dif * camera_dist);

		if (!is_swimming)
		{
			if (camera_pos.z < lowest_z)
				lowest_z = camera_pos.z;
			else
				camera_pos.z = lowest_z;
		}
	}
	else  //  the sufer hit something, so circle that thing.
	{
		//  Bigger objects should be farther away.  This tells us how much farther.
		float distance_mult = collision_object->get_colgeom()->get_radius() * 0.3f;

		//  Figure out what we are rotating around.
		vector3d object_pos = collision_object->get_abs_position ();

		//  Push the camera above the object that the surfer ran into.
		camera_pos.y = object_pos.y;
		camera_pos.z = object_pos.z;
		if (!BeachDataArray[g_game_ptr->get_beach_id ()].bdir)
			camera_pos.x = object_pos.x - (obj_behind_dist * distance_mult);
		else
			camera_pos.x = object_pos.x + (obj_behind_dist * distance_mult);

		//  Now that we have the base positions, rotate the camera around the object.
		current_rotation += obj_rotation_inc * time_step;
		if (current_rotation > PI)
			current_rotation -= 2 * PI;

		rotate_cam(&camera_pos, object_pos, current_rotation);

		camera_pos.y = object_pos.y + (obj_below_dist * distance_mult);

		new_target = object_pos;

		if (just_started)
		{
			just_started = false;
			camera_filter.Init_Filter(starter_po.get_position());
			target_filter.Init_Filter(get_target_entity ()->get_abs_position ());  //  The surfer is the initial target.
		}
		else
		{
			camera_filter.Filter_Vector(camera_pos, time_step, fast_filter);
			target_filter.Filter_Vector(new_target, time_step, fast_filter);
		}
	}


	trans.set_position (camera_pos);
	trans.set_facing (new_target);

	set_rel_po (trans);
}

//  Rotate the camera position around the center position by the rotation value.
void wipeout_camera::rotate_cam(vector3d *camera_pos, vector3d center_pos, float rotation_val)
{
	vector3d camera_dif = *camera_pos - center_pos;  //  The difference between the target and the camera.
	vector3d right_angle(0,0,0);
	right_angle.z = camera_dif.x;
	camera_dif = (cosf(rotation_val) * camera_dif) + (sinf(rotation_val) * (right_angle));
	*camera_pos = center_pos + camera_dif;
}


//  Use an ANIM file to fly around the beaches before the actual gameplay starts.
flyby_camera::flyby_camera (const entity_id& _id, entity* _target_entity)
  : game_camera (_id, _target_entity)
{
	this->compute_sector(g_world_ptr->get_the_terrain( ));
	animation = new PanelAnimFile;
}


flyby_camera::~flyby_camera ()
{
	delete animation;
}


void flyby_camera::frame_advance (time_value_t time_step)
{
	po trans(po_identity_matrix);

	if (is_playing && !g_game_ptr->is_paused())
		current_anim_time += time_step;

	if (current_anim_time >= animation->totalseconds)  //  Check to see if we're at the end of the animation.
	{
		current_anim_time = animation->totalseconds;
		is_playing = false;
	}

	matrix4x4 camera_matrix = camera->GetXFormMatrix(current_anim_time);

	vector3d camera_position;
	camera_position.x = -camera_matrix.w.x;
	camera_position.y = camera_matrix.w.z;  //  MAX thinks that z is up, so swap the z and y.
	camera_position.z = -camera_matrix.w.y;

	matrix4x4 target_matrix;

	vector3d camera_target;
	if (target)
	{
		target_matrix = target->GetXFormMatrix(current_anim_time);

		camera_target.x = -target_matrix.w.x;
		camera_target.y = target_matrix.w.z;  //  MAX thinks that z is up, so swap the z and y.
		camera_target.z = -target_matrix.w.y;
	}
	else
		camera_target = (const vector3d&)camera_matrix[2];


	trans.set_position (camera_position);
	trans.set_facing (camera_target);

	set_rel_po (trans);

	//  Check for a note to do some action.
	stringx event;
	if (camera->GetEvent(current_anim_time - time_step, current_anim_time, event))
	{
		if (event == "fade")
			BLUR_TurnOn();
	}
}


bool flyby_camera::load()
{

	//  Later, get the name of the file for the current level.
	char filename[100]; // let's make the buffer large enough for the file name

	// These need to go in the beach database! (dc 01/29/02)
	switch( g_game_ptr->get_beach_id () )
	{
	case BEACH_ANTARCTICA:
		strcpy(filename,"levels\\Antarctica\\Animations\\Antarctica_flyby.ANIM");
		break;
	case BEACH_BELLS:
		strcpy(filename,"levels\\bells\\Animations\\Bells_flyby.ANIM");
		break;
	case BEACH_CORTESBANK:
		strcpy(filename,"levels\\Cortes Bank\\Animations\\Cortes Bank_flyby.ANIM");
		break;
	case BEACH_GLAND:
		strcpy(filename,"levels\\G-Land\\Animations\\G-Land_flyby.ANIM");
		break;
	case BEACH_JAWS:
		strcpy(filename,"levels\\Jaws\\Animations\\Jaws_flyby.ANIM");
		break;
	case BEACH_JEFFERSONBAY:
		strcpy(filename,"levels\\Jefferson Bay\\Animations\\Jefferson Bay_flyby.ANIM");
		break;
	case BEACH_MAVERICKS:
		strcpy(filename,"levels\\Mavericks\\Animations\\Mavericks_flyby.ANIM");
		break;
	case BEACH_MUNDAKA:
		strcpy(filename,"levels\\Mundaka\\Animations\\Mundaka_flyby.ANIM");
		break;
	case BEACH_PIPELINE:
		strcpy(filename,"levels\\Pipeline\\Animations\\Pipeline_flyby.ANIM");
		break;
	case BEACH_SEBASTIAN:
		strcpy(filename,"levels\\Sebastian\\Animations\\Sebastian_flyby.ANIM");
		break;
	case BEACH_TEAHUPOO:
		strcpy(filename,"levels\\Teahupoo\\Animations\\Teahupoo_flyby.ANIM");
		break;
	case BEACH_TRESTLES:
		strcpy(filename,"levels\\Trestles\\Animations\\Trestles_flyby.ANIM");
		break;
	case BEACH_INDOOR:
		strcpy(filename,"levels\\Indoor\\Animations\\Indoor_flyby.ANIM");
		break;
	case BEACH_COSMOS:
		strcpy(filename,"levels\\Cosmos\\Animations\\Cosmos_flyby.ANIM");
		break;
	case BEACH_CURRENSPOINT:
		strcpy(filename,"levels\\Currens Point\\Animations\\Currens Point_flyby.ANIM");
		break;
	case BEACH_BELLSNIGHT:
		strcpy(filename,"levels\\Bells Night\\Animations\\Bells Night_flyby.ANIM");
		break;
	case BEACH_GLAND2:
		strcpy(filename,"levels\\G-Land2\\Animations\\G-Land2_flyby.ANIM");
		break;
	case BEACH_JAWSBIG:
		strcpy(filename,"levels\\Jaws Big\\Animations\\Jaws Big_flyby.ANIM");
		break;
	case BEACH_KIRRA:
		strcpy(filename,"levels\\Kirra\\Animations\\Kirra_flyby.ANIM");
		break;
	case BEACH_KIRRA2:
		strcpy(filename,"levels\\Kirra2\\Animations\\Kirra2_flyby.ANIM");
		break;
	case BEACH_MAVBIG:
		strcpy(filename,"levels\\Mav Big\\Animations\\Mav Big_flyby.ANIM");
		break;
	case BEACH_MUNDAKA2:
		strcpy(filename,"levels\\Mundaka2\\Animations\\Mundaka2_flyby.ANIM");
		break;
	case BEACH_TEANIGHT:
		strcpy(filename,"levels\\Tea Night\\Animations\\Tea Night_flyby.ANIM");
		break;
	case BEACH_TEAEVE:
		strcpy(filename,"levels\\Tea Eve\\Animations\\Tea Eve_flyby.ANIM");
		break;
	case BEACH_OPENSEA:
		strcpy(filename,"levels\\Open Sea\\Animations\\Open Sea_flyby.ANIM");
		break;
	default:
		return false;	//  No flythrough for this beach.
		break;
	}

	if (!(animation->Load(filename,NULL)))  //  Flag that the file wasn't available.
		return false;

	is_playing = false;
	current_anim_time = 0.0f;
	camera = animation->FindObject("camera");

	//  This will allow the old file format to work until we replace all the old animations.
	if (!camera)
		camera = animation->FindObject("camera_dummy");

	target = animation->FindObject("target");

	return true;
}


void flyby_camera::start()
{
	is_playing = true;
}


bool flyby_camera::is_finished()
{
	return current_anim_time >= animation->totalseconds;
}


// ============================================================================
// Follow Camera


float follow_dist_behind = 8.0f;
float air_dist_mult = 4.5f;
float follow_cam_height = 0.2f;
float fast_filter_speed = 0.7f;
float default_filter_speed = 0.5f;
float medium_filter_speed = 0.4f;
float slow_filter_speed = 0.3f;
float slowest_filter_speed = 0.15f;
float min_turn_offset = 0.17f;  //  Don't let the camera line up completely perpendicular to the wave.
float min_tube_turn_offset = 1.9f;
float follow_z_val = -0.5f;
float stall_z = 0.5f;
float lie_height_mod = 3.5f;

follow_camera::follow_camera (const entity_id& _id, entity* _target_entity)
  : game_camera (_id, _target_entity)
{
	init();
}


void follow_camera::init()
{
	first_time = true;
	jump_time_elapsed = 0;
}

void follow_camera::frame_advance (time_value_t time_step)
{
	int current_beach = g_game_ptr->get_beach_id ();
	int current_wave = WAVE_GetIndex();
	float tube_dist_behind = WaveDataArray[current_wave].tube_cam_dist;
	int current_state = ksctrl->get_current_state();
	int super_state = ksctrl->get_super_state();

	bool in_launch = current_state == STATE_FREEFALL || current_state == STATE_LAUNCH || current_state == STATE_AIR_TRICK;
	bool is_stalling = current_state == STATE_STALL || current_state == STATE_SUPER_STALL;
	bool is_in_tube_controls = super_state == SUPER_STATE_IN_TUBE;

	float distance_mult;
	vector3d closest_tube_point;
	int which_tube_closest;
	float tube_dist = max(0.0f, ksctrl->Closest_Tube_Distance(&which_tube_closest, &closest_tube_point));
	vector3d board_pos = get_target_entity ()->get_abs_position ();
	float filter_speed;

	//  Which way is the surfer going.
	vector3d body_dir;
	if ((ksctrl->get_board_controller().GetBoardState() == BOARD_GRIND)
		|| (ksctrl->get_board_controller().GetBoardState() == BOARD_FLOAT))  //  the direction of the board isn't useful in these cases.
		body_dir = ksctrl->get_board_controller().rb->linMom;
	else
		body_dir = ksctrl->get_board_controller().GetForwardDir();
	body_dir.normalize();

	//  This will be added to the position of the surfer, to get the camera_position.
	vector3d behind_surfer;
	behind_surfer.y = 0;


	if (in_launch)
	{
		float difference;
		float air_time = 0.55f * ksctrl->get_board_controller().GetTotalAirTime();  //  How long to the top of the jump

		if (!jump_time_elapsed)
		{
			//  Line up the camera with the perfect landing vector.
			in_air_cam.x = body_dir.x;
			in_air_cam.y = -fabs(body_dir.y);
			in_air_cam.z = follow_z_val*in_air_cam.y;
			in_air_cam.normalize();
		}

		jump_time_elapsed = min(air_time, jump_time_elapsed + time_step);

		//  Figure out how long we've been in the air.
		if (air_time && jump_time_elapsed >= 0)
			difference = jump_time_elapsed/air_time;
		else
			difference = 0;

		interpolate_vector(&behind_surfer, last_cam_vec, in_air_cam, difference, 3);

		distance_mult = air_dist_mult;
		filter_speed = default_filter_speed;

	}  //  end if doing vert stuff
	else
	{
		jump_time_elapsed = 0;

		//  Check to see if the surfer is getting close to a tube or stalling.
		if (is_in_tube_controls)
		{
			Get_Tube_Cam_Vector(ksctrl, &behind_surfer, get_target_entity(), tube_dist_behind);
			distance_mult = tube_dist_behind;  //  move the camera closer in the tube.
			filter_speed = fast_filter_speed;
		}
		else if (is_stalling)
		{
			Get_Tube_Cam_Vector(ksctrl, &behind_surfer, get_target_entity(), tube_dist_behind);

			behind_surfer.z = max(0.0f, behind_surfer.z);

			behind_surfer.normalize();

			distance_mult = tube_dist_behind;  //  move the camera closer in the tube.
			filter_speed = default_filter_speed;
		}
		else  //  Normal surfing.
		{
			//  Check to see if the surfer is closer to the beach than the tube.
			vector3d tube_start = *WAVE_GetMarker(WAVE_MarkerLipMark6);
			bool beside_tube = board_pos.z < tube_start.z;

			//  Figure out how much the surfer is moving along the wave.
			float cam_direction = dot(body_dir, ksctrl->get_board_controller().GetLipVec());

			//  Don't let the camera get too close to the wave, especially near a tube.
			float adjusted_tube_dist = 0;
			if (!beside_tube &&
				((body_dir.x < 0 && board_pos.x < closest_tube_point.x) ||  //  camera near a tube.
				(body_dir.x > 0 && board_pos.x > closest_tube_point.x) ||
				tube_dist == 0))  //  completely inside the tube.
			{
				//  Figure out how much the tube should push the camera over.
				adjusted_tube_dist = 1 - tube_dist;
				for (int index = 0; index < WaveDataArray[current_wave].look_in_tube_adj; index++)
					adjusted_tube_dist *= adjusted_tube_dist;

				cam_direction *= 1.0f - (2 * min_turn_offset) - (min_tube_turn_offset * adjusted_tube_dist);  //  Scale the value down.
			}
			else
				cam_direction *= 1.0f - (2 * min_turn_offset);  //  Scale the value down.

			//  Move the camera out a little bit when lying on the board.
			if (super_state == SUPER_STATE_LIE_ON_BOARD)
			{
				cam_direction *= 0.5f;
			}

			//  position the camera behind the surfer based on the direction that the surfer is heading along the wave.
			if (BeachDataArray[current_beach].bdir)
				behind_surfer.x = -cam_direction;
			else
				behind_surfer.x = cam_direction;

			if (super_state == SUPER_STATE_LIE_ON_BOARD)
				behind_surfer.y = lie_height_mod * (-follow_cam_height * (1 - adjusted_tube_dist));
			else if (beside_tube)
				behind_surfer.y = -(follow_cam_height * 0.5f); //  Lower camera angle if close to tube.
			else
				behind_surfer.y = -follow_cam_height * (1 - adjusted_tube_dist);

			behind_surfer.z = 1 - fabs(cam_direction);

			behind_surfer.normalize();

			if (body_dir.z < 0)
				filter_speed = default_filter_speed;
			else
				filter_speed = slow_filter_speed;

			distance_mult = follow_dist_behind;
		}
	}

	//  Make sure that the distance that the camera is behind the surfer doesn't change too rapidly.
	if (first_time)
		distance_filter.Init_Filter(distance_mult);
	distance_filter.Filter_Float(distance_mult, time_step, filter_speed);

	//  Now set the final camera up.
	po trans (po_identity_matrix);

	if (first_time)
	{
		first_time = false;
		cam_pos_filter.Init_Filter(behind_surfer);
	}

	cam_pos_filter.Filter_Vector(behind_surfer, time_step, filter_speed);
	behind_surfer.normalize();

	//  The launch will need to interpolate between whatever came before the launch.
	if (!in_launch)
		last_cam_vec = behind_surfer;

	trans.set_position (board_pos - (behind_surfer * distance_mult));
	trans.set_facing (board_pos);

	set_rel_po (trans);
}


// ============================================================================
// Follow Close Camera


float follow_close_dist_behind = 4.6f;
float close_air_dist_mult = 3.5f;
float follow_close_lie_dist = 6.0f;
float follow_close_above_head = 0.7f;
float follow_close_min_turn_offset = 0.03f;
float follow_close_default_filter_speed = 0.3f;
float follow_close_cam_height = 0.1f;
float cam_dir_mod = 1.0f;


follow_close_camera::follow_close_camera (const entity_id& _id, entity* _target_entity)
  : game_camera (_id, _target_entity)
{
	init();
}


void follow_close_camera::init()
{
	first_time = true;
	jump_time_elapsed = 0;
	if (ksctrl)
		frame_advance(0.0f);
}

void follow_close_camera::frame_advance (time_value_t time_step)
{
	int current_beach = g_game_ptr->get_beach_id ();
	int current_wave = WAVE_GetIndex();
	float tube_dist_behind = WaveDataArray[current_wave].tube_cam_dist;
	int current_state = ksctrl->get_current_state();
	int super_state = ksctrl->get_super_state();

	bool in_launch = current_state == STATE_FREEFALL || current_state == STATE_LAUNCH || current_state == STATE_AIR_TRICK;
	bool is_stalling = current_state == STATE_STALL || current_state == STATE_SUPER_STALL;
	bool is_in_tube_controls = super_state == SUPER_STATE_IN_TUBE;
	bool is_lying_on_board = super_state == SUPER_STATE_LIE_ON_BOARD;

	float distance_mult;
	vector3d closest_tube_point;
	int which_tube_closest;
	float tube_dist = max(0.0f, ksctrl->Closest_Tube_Distance(&which_tube_closest, &closest_tube_point));
	vector3d board_pos = get_target_entity ()->get_abs_position ();
	float filter_speed;
	float adjusted_follow_close_above_head = follow_close_above_head;
	if (is_in_tube_controls)
		adjusted_follow_close_above_head = 0;  //  Don't push the camera up when the surfer is in the tube.

	//  Which way is the surfer going.
	vector3d body_dir;
	if ((ksctrl->get_board_controller().GetBoardState() == BOARD_GRIND)
		|| (ksctrl->get_board_controller().GetBoardState() == BOARD_FLOAT))  //  the direction of the board isn't useful in these cases.
		body_dir = ksctrl->get_board_controller().rb->linMom;
	else
		body_dir = ksctrl->get_board_controller().GetForwardDir();
	body_dir.normalize();

	//  This will be added to the position of the surfer, to get the camera_position.
	vector3d behind_surfer;
	behind_surfer.y = 0;


	if (in_launch)
	{
		float difference;
		float air_time = 0.55f * ksctrl->get_board_controller().GetTotalAirTime();  //  How long to the top of the jump

		if (!jump_time_elapsed)
		{
			//  Line up the camera with the perfect landing vector.
			if (BeachDataArray[current_beach].bdir)
				in_air_cam.x = -fabs(body_dir.x);
			else
				in_air_cam.x = fabs(body_dir.x);
			in_air_cam.y = -fabs(body_dir.y);
			in_air_cam.z = follow_z_val*in_air_cam.y;
			in_air_cam.normalize();
		}

		jump_time_elapsed = min(air_time, jump_time_elapsed + time_step);

		//  Figure out how long we've been in the air.
		if (air_time && jump_time_elapsed >= 0)
			difference = jump_time_elapsed/air_time;
		else
			difference = 0;

		interpolate_vector(&behind_surfer, last_cam_vec, in_air_cam, difference, 3);

		distance_mult = close_air_dist_mult;
		filter_speed = follow_close_default_filter_speed;

	}  //  end if doing vert stuff
	else if (is_lying_on_board)
	{
		behind_surfer = board_pos + YVEC;  //  Start with where the camera is pointing.
		vector3d lip_pos = *WAVE_GetMarker(WAVE_MarkerLipMark11p8);
		behind_surfer.x -= lip_pos.x;	//  get the direction towards the lip.
		behind_surfer.z = lip_pos.z - behind_surfer.z;
		behind_surfer.y = 1.0f;
		behind_surfer.normalize();
		distance_mult = follow_close_lie_dist;
		filter_speed = follow_close_default_filter_speed;
	}
	else
	{
		jump_time_elapsed = 0;

		//  Check to see if the surfer is getting close to a tube or stalling.
		if (is_in_tube_controls)
		{
			Get_Tube_Cam_Vector(ksctrl, &behind_surfer, get_target_entity(), tube_dist_behind);
			distance_mult = tube_dist_behind;  //  move the camera closer in the tube.
			filter_speed = fast_filter_speed;
		}
		else if (is_stalling && ksctrl->IsUpDownThreshDone())
		{
			Get_Tube_Cam_Vector(ksctrl, &behind_surfer, get_target_entity(), tube_dist_behind);

			behind_surfer.z = max(0.0f, behind_surfer.z);

			behind_surfer.normalize();

			distance_mult = tube_dist_behind;  //  move the camera closer in the tube.
			filter_speed = follow_close_default_filter_speed;
		}
		else  //  Normal surfing.
		{
			//  Check to see if the surfer is closer to the beach than the tube.
			vector3d tube_start = *WAVE_GetMarker(WAVE_MarkerLipMark6);
			bool beside_tube = board_pos.z < tube_start.z;

			//  Figure out how much the surfer is moving along the wave.
			float cam_direction = dot(body_dir, ksctrl->get_board_controller().GetLipVec());

			//  Don't let the camera get too close to the wave, especially near a tube.
			float adjusted_tube_dist = 0;
			if (!beside_tube &&
				((body_dir.x < 0 && board_pos.x < closest_tube_point.x) ||  //  camera near a tube.
				(body_dir.x > 0 && board_pos.x > closest_tube_point.x) ||
				tube_dist == 0))  //  completely inside the tube.
			{
				//  Figure out how much the tube should push the camera over.
				adjusted_tube_dist = 1 - tube_dist;
				for (int index = 0; index < WaveDataArray[current_wave].look_in_tube_adj; index++)
					adjusted_tube_dist *= adjusted_tube_dist;

				cam_direction *= 1.0f - (2 * follow_close_min_turn_offset) - (min_tube_turn_offset * adjusted_tube_dist);  //  Scale the value down.

				adjusted_follow_close_above_head = 0;  //  Don't push the camera up when the surfer is in the tube.
			}
			else
				cam_direction *= 1.0f - (2 * follow_close_min_turn_offset);  //  Scale the value down.

			//  Don't make the camera point directly away from the beach.  Shift more towards being along the wave.
			if (cam_direction < 0)
				cam_direction = (cam_dir_mod * (1 + cam_direction)) - 1;
			else
				cam_direction = 1 - (cam_dir_mod * (1 - cam_direction));

			//  Move the camera out a little bit when lying on the board.
//			if (is_lying_on_board)
//			{
//				cam_direction *= 0.5f;
//			}

			//  position the camera behind the surfer based on the direction that the surfer is heading along the wave.
			if (BeachDataArray[current_beach].bdir)
				behind_surfer.x = -cam_direction;
			else
				behind_surfer.x = cam_direction;

//			if (is_lying_on_board)
//				behind_surfer.y = lie_height_mod * (-follow_close_cam_height * (1 - adjusted_tube_dist));
//			else
			if (beside_tube)
				behind_surfer.y = -(follow_close_cam_height * 0.5f); //  Lower camera angle if close to tube.
			else
				behind_surfer.y = -follow_close_cam_height * (1 - adjusted_tube_dist);

			behind_surfer.z = 1 - fabs(cam_direction);

			behind_surfer.normalize();

			if (body_dir.z < 0)
				filter_speed = follow_close_default_filter_speed;
			else
				filter_speed = slow_filter_speed;

//			if (is_lying_on_board)
//				distance_mult = follow_close_lie_dist;
//			else
				distance_mult = follow_close_dist_behind;
		}
	}

	//  Special case for face tricks, very slow filter so the camera isn't swinging all over the place.
	if (current_state == STATE_LEFTREVERTCUTBACK || current_state == STATE_RIGHTREVERTCUTBACK ||
		current_state == STATE_LEFT_REBOUND || current_state == STATE_RIGHT_REBOUND ||
		current_state == STATE_LEFT_TAIL_CHUCK || current_state == STATE_RIGHT_TAIL_CHUCK ||
		current_state == STATE_LEFT_LAYBACK_SLIDE || current_state == STATE_RIGHT_LAYBACK_SLIDE)
		filter_speed = medium_filter_speed;
	else if (current_state == STATE_LEFT_GOUGE || current_state == STATE_RIGHT_GOUGE)
		filter_speed = slowest_filter_speed;

	//  Make sure that the distance that the camera is behind the surfer doesn't change too rapidly.
	if (first_time)
		distance_filter.Init_Filter(distance_mult);
	else
		distance_filter.Filter_Float(distance_mult, time_step, filter_speed);

	//  Now set the final camera up.
	po trans (po_identity_matrix);

	if (first_time)
	{
		cam_pos_filter.Init_Filter(behind_surfer);
		above_head_filter.Init_Filter(adjusted_follow_close_above_head);
	}
	else
	{
		cam_pos_filter.Filter_Vector(behind_surfer, time_step, filter_speed);
		above_head_filter.Filter_Float(adjusted_follow_close_above_head, time_step, filter_speed);
	}

	behind_surfer.normalize();

	//  The launch will need to interpolate between whatever came before the launch.
	if (!in_launch)
		last_cam_vec = behind_surfer;

	//  This makes the camera look much more along the wave when the camera is being set to the tube vector.
	vector3d facing_offset(0,0,0);
	vector3d position_pos = board_pos - (behind_surfer * distance_mult) + (YVEC * adjusted_follow_close_above_head);
	vector3d facing_pos = board_pos + (YVEC * adjusted_follow_close_above_head);
	if (is_in_tube_controls)
	{
		facing_offset.y = (position_pos.y - facing_pos.y) * tube_offest_val;
		facing_offset.z = (position_pos.z - facing_pos.z) * tube_offest_val;
	}
	else if (is_stalling && ksctrl->IsUpDownThreshDone())
	{
		facing_offset.y = (position_pos.y - facing_pos.y) * stalling_offset_val;
		facing_offset.z = (position_pos.z - facing_pos.z) * stalling_offset_val;
	}

	if (first_time)
	{
		first_time = false;
		facing_offset_filter.Init_Filter(facing_offset);
	}
	else
		facing_offset_filter.Filter_Vector(facing_offset, time_step, filter_speed);

	trans.set_position (position_pos);
	trans.set_facing (facing_pos + facing_offset);

	set_rel_po (trans);
}


// ============================================================================
// Buoy Camera,  basicly a simplified version of the follow cam that stays behind the wave.


float buoy_cam_height = 0.6f;
float buoy_fast_filter_speed = 0.8f;
float buoy_default_filter_speed = 0.5f;
float buoy_min_turn_offset = 0.17f;  //  Don't let the camera line up completely perpendicular to the wave.
float buoy_min_secondary_tube_turn_offset = 2.0f - (2 * buoy_min_turn_offset);
float buoy_min_main_tube_turn_offset = 1.6f;
float buoy_z_val = -0.5f;
float wave_height_mod = 2.1f;

buoy_camera::buoy_camera (const entity_id& _id, entity* _target_entity)
  : game_camera (_id, _target_entity)
{
	init();
}


void buoy_camera::init()
{
	first_time = true;
	jump_time_elapsed = 0;
}

void buoy_camera::frame_advance (time_value_t time_step)
{
	int current_beach = g_game_ptr->get_beach_id ();
	int current_wave = WAVE_GetIndex();
	float tube_dist_behind = WaveDataArray[current_wave].tube_cam_dist;
	int current_state = ksctrl->get_current_state();
	int super_state = ksctrl->get_super_state();

	bool in_launch = current_state == STATE_FREEFALL || current_state == STATE_LAUNCH || current_state == STATE_AIR_TRICK;
	bool is_stalling = current_state == STATE_STALL || current_state == STATE_SUPER_STALL;
	bool is_in_tube_controls = super_state == SUPER_STATE_IN_TUBE;

	float distance_mult;
	vector3d board_pos = get_target_entity ()->get_abs_position ();
	float filter_speed;

	if (first_time)	//  This will be set to false later in the fundtion.
	{
		//  Figure out the height of the wave.
		vector3d top = *WAVE_GetMarker(WAVE_MarkerEmitterStartCrestX);
		vector3d bottom = *WAVE_GetMarker(WAVE_MarkerSurferSpawn);
		default_distance_mult = (top.y - bottom.y) * wave_height_mod;

		distance_filter.Init_Filter(default_distance_mult);
	}

	//  Which way is the surfer going.
	vector3d body_dir;
	if ((ksctrl->get_board_controller().GetBoardState() == BOARD_GRIND)
		|| (ksctrl->get_board_controller().GetBoardState() == BOARD_FLOAT))  //  the direction of the board isn't useful in these cases.
		body_dir = ksctrl->get_board_controller().rb->linMom;
	else
		body_dir = ksctrl->get_board_controller().GetForwardDir();
	body_dir.normalize();

	//  This will be added to the position of the surfer, to get the camera_position.
	vector3d behind_surfer;
	behind_surfer.y = 0;


	if (in_launch)
	{
		float difference;
		float air_time = 0.55f * ksctrl->get_board_controller().GetTotalAirTime();  //  How long to the top of the jump

		if (!jump_time_elapsed)
		{
			//  Line up the camera with the perfect landing vector.
			in_air_cam.x = body_dir.x;
			in_air_cam.y = -fabs(body_dir.y);
			in_air_cam.z = -buoy_z_val*in_air_cam.y;
			in_air_cam.normalize();
		}

		jump_time_elapsed = min(air_time, jump_time_elapsed + time_step);

		//  Figure out how long we've been in the air.
		if (air_time && jump_time_elapsed >= 0)
			difference = jump_time_elapsed/air_time;
		else
			difference = 0;

		interpolate_vector(&behind_surfer, last_cam_vec, in_air_cam, difference, 3);

		distance_mult = default_distance_mult;
		filter_speed = buoy_default_filter_speed;

	}  //  end if doing vert stuff
	else
	{
		jump_time_elapsed = 0;

		//  Check to see if the surfer is getting close to a tube or stalling.
		if (is_in_tube_controls)
		{
			Get_Tube_Cam_Vector(ksctrl, &behind_surfer, get_target_entity(), tube_dist_behind);
			distance_mult = tube_dist_behind;  //  move the camera closer in the tube.
			filter_speed = buoy_fast_filter_speed;
		}
		else if (is_stalling)
		{
			Get_Tube_Cam_Vector(ksctrl, &behind_surfer, get_target_entity(), tube_dist_behind);

			behind_surfer.z = max(0.0f, behind_surfer.z);

			behind_surfer.normalize();

			distance_mult = tube_dist_behind;  //  move the camera closer in the tube.
			filter_speed = buoy_default_filter_speed;
		}
		else  //  Normal surfing.
		{
			//  Figure out how much the surfer is moving along the wave.
			float cam_direction = dot(body_dir, ksctrl->get_board_controller().GetLipVec());

			//  Don't let the camera get too close to the wave.
			cam_direction *= 1.0f - (2 * buoy_min_turn_offset);  //  Scale the value down.

			//  Move the camera out a little bit when lying on the board.
			if (super_state == SUPER_STATE_LIE_ON_BOARD)
			{
				cam_direction *= 0.5f;
			}

			//  position the camera behind the surfer based on the direction that the surfer is heading along the wave.
			if (BeachDataArray[current_beach].bdir)
				behind_surfer.x = -cam_direction;
			else
				behind_surfer.x = cam_direction;

			if (super_state == SUPER_STATE_LIE_ON_BOARD)
				behind_surfer.y = 2 * (-buoy_cam_height);
			else
				behind_surfer.y = -buoy_cam_height;

			behind_surfer.z = -(1 - fabs(cam_direction));

			behind_surfer.normalize();

			if (body_dir.z < 0)
				filter_speed = buoy_default_filter_speed;
			else
				filter_speed = buoy_fast_filter_speed;

			distance_mult = default_distance_mult;
		}
	}

	//  Make sure that the distance that the camera is behind the surfer doesn't change too rapidly.
	distance_filter.Filter_Float(distance_mult, time_step, filter_speed);

	//  Now set the final camera up.
	po trans (po_identity_matrix);

	if (first_time)
	{
		first_time = false;
		cam_pos_filter.Init_Filter(behind_surfer);
	}

	cam_pos_filter.Filter_Vector(behind_surfer, time_step, filter_speed);
	behind_surfer.normalize();

	//  The launch will need to interpolate between whatever came before the launch.
	if (!in_launch)
		last_cam_vec = behind_surfer;

	trans.set_position (board_pos - (behind_surfer * distance_mult));
	trans.set_facing (board_pos);

	set_rel_po (trans);
}


/////////////////////////////////////  Duck Diving Camera functions   ///////////////////////////////////


duckdive_camera::duckdive_camera( const entity_id& _id, entity* _target_entity, kellyslater_controller *_ksctrl )
  :   game_camera( _id, _target_entity )
{
  /* create_link_ifc() */  //  eventually this will have to be uncommented when bones do not have a default link interface

  set_ks_controller(_ksctrl);
  //Reset();
}


void duckdive_camera::Reset( void )
{
	orig_cam = (game_camera *) g_game_ptr->get_player_camera(get_ks_controller()->get_player_num());
	vector3d ent_center = get_target_entity()->get_abs_position();

	vector3d abs_po = orig_cam->get_abs_position();
	delta_vec = abs_po - ent_center;
	low_pass_filter.Init_Filter(abs_po);

	po mine;
	mine.set_position(abs_po);
	mine.set_facing(ent_center);

	this->set_rel_po(mine);

	Norm(delta_vec, delta_float);
	actual_float = delta_float;
	actual_vec = delta_vec;

	trans_2_norm_cam = false;
	do_reset = false;
	start_trans = false;

	cutoff_frequency = 1.0f;
}

float duck_z = -3.0f;
float duck_y = -4.0f;
float filt_float = 0.025f;
float filt_vec = 0.025f;
float freq_scale = 8.0f;
void duckdive_camera::frame_advance( time_value_t t )
{
	vector3d ent_center = get_target_entity()->get_abs_position();

	if (!trans_2_norm_cam)
		delta_vec = duck_y*YVEC + duck_z*ZVEC;
	else
		delta_vec = orig_cam->GetStartPosition();

	delta_vec = actual_float*actual_vec + (delta_vec - actual_float*actual_vec)*(filt_vec*60.0f*t);
	//delta_vec = delta_vec + (actual_vec*actual_float - delta_vec)*(1.0f - filt_vec*60.0f*t);
	Norm(delta_vec, delta_float);

	//delta_float = delta_float + (actual_float - delta_float)*(1.0f - filt_float*60.0f*t);
	delta_float = actual_float + (delta_float - actual_float)*(filt_float*60.0f*t);
	vector3d DesPos = ent_center + delta_float*delta_vec;
	low_pass_filter.Filter_Vector(DesPos, t, cutoff_frequency*freq_scale);

	actual_vec = low_pass_filter.GetCurrentState() - ent_center;
	Norm(actual_vec, actual_float);

	po mine;
	mine.set_position(DesPos);

	conglomerate *surfer = (conglomerate*) get_ks_controller()->get_owner();
	entity* pelvis = surfer->get_member("BIP01 PELVIS");
	mine.set_facing(pelvis->get_abs_position());

	this->set_rel_po(mine);

	if (start_trans)
	{
		get_ks_controller()->SetPlayerCamera(orig_cam);
		orig_cam->init();
	}

	if (do_reset)
		start_trans = true;



}

void duckdive_camera::SetReset(void)
{
	do_reset = true;
/*	This duplicates what's already being done in blur.cpp (dc 04/05/02)
	wipeTrans.start();
*/
	WAVE_EndWave();
}

/////////////////////////////////////  Photo Camera functions   ///////////////////////////////////

float photo_camera::DIST_TUBE = 2.0f;
float photo_camera::DIST_NORMAL = 1.7f;
float photo_camera::DIST_AIR = 2.0f;

//	photo_camera()
// Constructor with initializers.
photo_camera::photo_camera(const entity_id & _id, entity * _target_entity, kellyslater_controller * _ksctrl)
  : game_camera (_id, _target_entity)
{
	set_ks_controller(_ksctrl);
	init();
}

//	init()
// Must be called after constructing.
void photo_camera::init()
{
	first_time = true;
	jump_time_elapsed = 0;
}

//	frame_advance()
// Must be called every frame with the elapsed time,
void photo_camera::frame_advance(time_value_t dt)
{
	int current_beach = g_game_ptr->get_beach_id ();
	int current_wave = WAVE_GetIndex();
	//float tube_dist_behind = WaveDataArray[current_wave].tube_cam_dist;
	int current_state = ksctrl->get_current_state();
	int super_state = ksctrl->get_super_state();

	bool in_launch = current_state == STATE_FREEFALL || current_state == STATE_LAUNCH || current_state == STATE_AIR_TRICK;
	//bool is_stalling = current_state == STATE_STALL || current_state == STATE_SUPER_STALL;
	bool is_in_tube_controls = super_state == SUPER_STATE_IN_TUBE;

	float distance_mult;
	//vector3d closest_tube_point;
	//int which_tube_closest;
	//float tube_dist = max(0.0f, ksctrl->Closest_Tube_Distance(&which_tube_closest, &closest_tube_point));
	vector3d target_pos;

	//  Which way is the surfer going.
	vector3d body_dir;
	if ((ksctrl->get_board_controller().GetBoardState() == BOARD_GRIND)
		|| (ksctrl->get_board_controller().GetBoardState() == BOARD_FLOAT))  //  the direction of the board isn't useful in these cases.
		body_dir = ksctrl->get_board_controller().rb->linMom;
	else
		body_dir = ksctrl->get_board_controller().GetForwardDir();
	body_dir.normalize();

	// Calculate target position.
	target_pos = ((conglomerate *) get_target_entity())->get_member("BIP01 SPINE2")->get_abs_position();
	target_pos += body_dir*0.4f;

	//  This will be added to the position of the surfer, to get the camera_position.
	vector3d behind_surfer;
	behind_surfer.y = 0;

	// Surfer in the air.
	if (in_launch)
	{
		behind_surfer.x = 0.0f;
		behind_surfer.y = 0.3f;
		behind_surfer.z = 1.0f;

		behind_surfer.normalize();

		distance_mult = DIST_AIR;
	}
	// Surfer in the tube.
	else if (is_in_tube_controls)
	{
		vector3d tubeVec = vector3d(WAVE_MeshMinX, WaveDataArray[current_wave].tubecenstart_y, WaveDataArray[current_wave].tubecenstart_z)-vector3d(WAVE_MeshMaxX, WaveDataArray[current_wave].tubecenstop_y, WaveDataArray[current_wave].tubecenstop_z);
		behind_surfer = tubeVec;
		behind_surfer.normalize();
		distance_mult = DIST_TUBE;
	}
	// Surfer on the face.
	else
	{
		//  Check to see if the surfer is closer to the beach than the tube.
		vector3d tube_start = *WAVE_GetMarker(WAVE_MarkerLipMark6);
		bool beside_tube = target_pos.z < tube_start.z;

		//  Figure out how much the surfer is moving along the wave.
		float cam_direction = dot(body_dir, ksctrl->get_board_controller().GetLipVec());

		//  Don't let the camera get too close to the wave, especially near a tube.
		float adjusted_tube_dist = 0;
		/*
		if (!beside_tube &&
		((body_dir.x < 0 && target_pos.x < closest_tube_point.x) ||  //  camera near a tube.
		(body_dir.x > 0 && target_pos.x > closest_tube_point.x) ||
		tube_dist == 0))  //  completely inside the tube.
		{
		//  Figure out how much the tube should push the camera over.
		adjusted_tube_dist = 1 - tube_dist;
		for (int index = 0; index < WaveDataArray[current_wave].look_in_tube_adj; index++)
		adjusted_tube_dist *= adjusted_tube_dist;

		  if (which_tube_closest)  //  check for main tube.
		  cam_direction *= 1.0f - (2 * min_turn_offset) - (min_main_tube_turn_offset * adjusted_tube_dist);  //  Scale the value down.
		  else
		  cam_direction *= 1.0f - (2 * min_turn_offset) - (min_secondary_tube_turn_offset * adjusted_tube_dist);  //  Scale the value down.
		  }
		  else
		*/
		//	cam_direction *= 1.0f - (2 * min_turn_offset);  //  Scale the value down.

		//  Move the camera out a little bit when lying on the board.
		if (super_state == SUPER_STATE_LIE_ON_BOARD)
		{
			cam_direction *= 0.5f;
		}

		//  position the camera behind the surfer based on the direction that the surfer is heading along the wave.
		if (BeachDataArray[current_beach].bdir)
			behind_surfer.x = -cam_direction;
		else
			behind_surfer.x = cam_direction;

		if (super_state == SUPER_STATE_LIE_ON_BOARD)
			behind_surfer.y = lie_height_mod * (follow_cam_height * (1 - adjusted_tube_dist));
		else if (beside_tube)
			behind_surfer.y = (follow_cam_height * 0.5f); //  Lower camera angle if close to tube.
		else
			behind_surfer.y = follow_cam_height * (1 - adjusted_tube_dist);

		behind_surfer.z = -(1 - fabs(cam_direction));

		behind_surfer.normalize();

		if (in_launch)
			distance_mult = DIST_AIR;
		else
			distance_mult = DIST_NORMAL;
	}

	//  Now set the final camera up.
	po			trans(po_identity_matrix);
	vector3d	pos;
	float		airMinY = WAVE_GetMarker(WAVE_MarkerLipMark7)->y + 1.0f;

	behind_surfer.normalize();

	pos = target_pos+(behind_surfer*distance_mult);
	if (in_launch && pos.y < airMinY)
		pos.y = airMinY;
	trans.set_position(pos);

	trans.set_facing(target_pos);

	set_rel_po(trans);
}
