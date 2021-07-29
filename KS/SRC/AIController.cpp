#include "AIController.h"
#include "inputmgr.h"
#include "joystick.h"
#include "kellyslater_controller.h"
#include "board.h"
#include "wds.h"
#include "timer.h"
#include "random.h"
#include "trickdata.h"
#include "wipeoutdata.h"


int AISurferController::get_axis_count() const
{
  return JOY_PS2_NUM_AXES;
}

AISurferController::AISurferController()
{
  myState = STATE_STANDING;
  mySubstate = NO_SUBSTATE;
  curTrick = 0;
  offdh = diffAngle = paddleAngle =0;
  toTargetDistance = lastToTargetDistance = 999999;  // Arbitrarily high
  toTubeDist = 0;
  lastRelativeVelocityToTarget = relativeVelocityToTarget = 0;
  heading = vector3d(-1,0,0);

  X = Y = 0;
  ButtonX = ButtonO = ButtonSq = ButtonTr = false;
  ButtonL1 = ButtonL2 = ButtonL3 = false;
  ButtonR1 = ButtonR2 = ButtonR3 = false;
  ButtonStart = ButtonSelect = 0;

  oldX = oldY = 0;
  oldButtonX = oldButtonO = oldButtonSq = oldButtonTr = false;
  oldButtonL1 = oldButtonL2 = oldButtonL3 = false;
  oldButtonR1 = oldButtonR2 = oldButtonR3   = false;
  oldButtonStart = oldButtonSelect = 0;
  
  acel = oldAcel =0;
  last_toward_tube_velocity = this_toward_tube_velocity = -99999;
  ksctl = NULL;
}
axis_id_t AISurferController::get_axis_id(int axis) const 
{
  return axis;
}

rational_t AISurferController::get_axis_delta(axis_id_t axis, int control_axis ) const
{
  switch (axis) 
  {
    case JOY_PS2_X: // left analog horiz
      return X - oldX;

    case JOY_PS2_Y: // left analog vert
      return Y - oldY;

    case JOY_PS2_DX: // just Dpad
      if (oldX < 0)
      {
        if (X < 0)
          return 0;
        else if (X > 0)
          return 2;
        else 
          return 1;

      }
      else if (oldX > 0)
      {
        if (X < 0)
          return 0;
        else if (X > 0)
          return -2;
        else 
          return -1;
          
      }
      else
      {
        if (X == 0)
          return 0;
        else if (X < 0)
          return -1;
        else 
          return 1;
      }
      
    case JOY_PS2_DY: // just Dpad -1 = up
      if (oldY < 0)
      {
        if (Y < 0)
          return 0;
        else if (Y > 0)
          return 2;
        else 
          return 1;

      }
      else if (oldY > 0)
      {
        if (Y < 0)
          return 0;
        else if (Y > 0)
          return -2;
        else 
          return -1;
          
      }
      else
      {
        if (Y == 0)
          return 0;
        else if (Y < 0)
          return -1;
        else 
          return 1;
      }
      
 
    case JOY_PS2_LX: //  - 1 = left
      return X - oldX;
    case JOY_PS2_LY: // -1 = up
      return Y - oldY;
    case JOY_PS2_RX: 
      return XRight - oldXRight;
    case JOY_PS2_RY: 
      return YRight - oldYRight;

    case JOY_PS2_BTNL3: // L3
      return ButtonL3 - oldButtonL3;
    case JOY_PS2_BTNR3: 
      return ButtonR3 - oldButtonR3;
    case JOY_PS2_BTNX:  // A (ps2 x)
      return ButtonX - oldButtonX;
    case JOY_PS2_BTNO:  // B (ps2 o)
        return ButtonO - oldButtonO;
    case JOY_PS2_BTNSQ: // X (ps2 sq)
        return ButtonSq - oldButtonSq;
    case JOY_PS2_BTNTR: // Y (ps2 tr)
        return ButtonTr - oldButtonTr;

    case JOY_PS2_BTNR1: // C (ps2 R1)
        return ButtonR1 - oldButtonR1;
    case JOY_PS2_BTNR2: // R (ps2 R2)
        return ButtonR2 - oldButtonR2;

    case JOY_PS2_BTNL1: // Z (ps2 L1)
        return ButtonL1 - oldButtonL1;
    case JOY_PS2_BTNL2: // L (ps2 L2)
        return ButtonL2 - oldButtonL2;


    case JOY_PS2_START: // Start
      return ButtonStart - oldButtonStart;
    case JOY_PS2_SELECT:
      return ButtonSelect - oldButtonSelect;

    default:
      assert("ps2_joypad_device::get_axis_state illegal axis queried." == 0);
      return 0.0f;
  
  }

}
void AISurferController::clearButtons()
{
  X = Y = 0;
  ButtonX = ButtonO = ButtonSq = ButtonTr = false;
  ButtonL1 = ButtonL2 = ButtonL3 = false;
  ButtonR1 = ButtonR2 = ButtonR3 = false;
  ButtonStart = ButtonSelect = 0;

}
rational_t AISurferController::get_axis_old_state(axis_id_t axis, int control_axis ) const
{
   switch (axis) 
  {
    case JOY_PS2_X: // left analog horiz
      return oldX;

    case JOY_PS2_Y: // left analog vert
      return oldY;

    case JOY_PS2_DX: // just Dpad
      if (oldX < 0)
      {
          return -1.0f;
      }
      else if (oldX > 0)
      {
          return 1.0f;
      }
      return 0.0f;
      
    case JOY_PS2_DY: // just Dpad -1 = up
      if (oldY < 0)
      {
          return -1.0f;  //up
      }
      else if (oldY > 0)
      {
          return 1.0f;
      }
      return 0.0f;
 
    case JOY_PS2_LX: //  - 1 = left
      return oldX;
    case JOY_PS2_LY: // -1 = up
      return oldY;
    case JOY_PS2_RX: 
      return oldXRight;
    case JOY_PS2_RY: 
      return oldYRight;

    case JOY_PS2_BTNL3: // L3
      return oldButtonL3;
    case JOY_PS2_BTNR3: 
      return oldButtonR3;
    case JOY_PS2_BTNX:  // A (ps2 x)
      return oldButtonX;
    case JOY_PS2_BTNO:  // B (ps2 o)
        return oldButtonO;
    case JOY_PS2_BTNSQ: // X (ps2 sq)
        return oldButtonSq;
    case JOY_PS2_BTNTR: // Y (ps2 tr)
        return oldButtonTr;

    case JOY_PS2_BTNR1: // C (ps2 R1)
        return oldButtonR1;
    case JOY_PS2_BTNR2: // R (ps2 R2)
        return oldButtonR2;

    case JOY_PS2_BTNL1: // Z (ps2 L1)
        return oldButtonL1;
    case JOY_PS2_BTNL2: // L (ps2 L2)
        return oldButtonL2;


    case JOY_PS2_START: // Start
      return oldButtonStart;
    case JOY_PS2_SELECT:
      return oldButtonSelect;

    default:
      assert("ps2_joypad_device::get_axis_state illegal axis queried." == 0);
      return 0.0f;
  
  }

}

rational_t AISurferController::get_axis_state(axis_id_t axis, int control_axis ) const
{
  switch (axis) 
  {
    case JOY_PS2_X: // left analog horiz
      return X;

    case JOY_PS2_Y: // left analog vert
      return Y;

    case JOY_PS2_DX: // just Dpad
      if (X < 0)
      {
          return -1.0f;
      }
      else if (X > 0)
      {
          return 1.0f;
      }
      return 0.0f;
      
    case JOY_PS2_DY: // just Dpad -1 = up
      if (Y < 0)
      {
          return -1.0f;  //up
      }
      else if (Y > 0)
      {
          return 1.0f;
      }
      return 0.0f;
 
    case JOY_PS2_LX: //  - 1 = left
      return X;
    case JOY_PS2_LY: // -1 = up
      return Y;
    case JOY_PS2_RX: 
      return XRight;
    case JOY_PS2_RY: 
      return YRight;

    case JOY_PS2_BTNL3: // L3
      return ButtonL3;
    case JOY_PS2_BTNR3: 
      return ButtonR3;
    case JOY_PS2_BTNX:  // A (ps2 x)
      return ButtonX;
    case JOY_PS2_BTNO:  // B (ps2 o)
        return ButtonO;
    case JOY_PS2_BTNSQ: // X (ps2 sq)
        return ButtonSq;
    case JOY_PS2_BTNTR: // Y (ps2 tr)
        return ButtonTr;

    case JOY_PS2_BTNR1: // C (ps2 R1)
        return ButtonR1;
    case JOY_PS2_BTNR2: // R (ps2 R2)
        return ButtonR2;

    case JOY_PS2_BTNL1: // Z (ps2 L1)
        return ButtonL1;
    case JOY_PS2_BTNL2: // L (ps2 L2)
        return ButtonL2;


    case JOY_PS2_START: // Start
      return ButtonStart;
    case JOY_PS2_SELECT:
      return ButtonSelect;

    default:
      assert("ps2_joypad_device::get_axis_state illegal axis queried." == 0);
      return 0.0f;
  
  }
}
void AISurferController::setupStateVars()
{
  oldAcel = acel;
  

  oldX = X;
  oldY = Y;
  oldXRight   = XRight;
  oldYRight   = YRight;
  oldButtonX  = ButtonX;
  oldButtonO  = ButtonO;
  oldButtonSq = ButtonSq;
  oldButtonTr = ButtonTr;

  oldButtonL1 = ButtonL1; 
  oldButtonL2 = ButtonL2;
  oldButtonL3 = ButtonL3;
  oldButtonR1 = ButtonR1;
  oldButtonR2 = ButtonR2;
  oldButtonR3 = ButtonR3;

  oldButtonSelect = ButtonSelect;
  oldButtonStart  = ButtonStart;
  
  ksctl = NULL;
  
  if (g_game_ptr && g_game_ptr->get_num_ai_players())
  {
    kellyslater_controller *ks = g_world_ptr->get_ai_controller();
    if (ks && (ks->get_joystick_num() == AI_JOYSTICK))
    {
      ksctl = ks;
    }
  }

  if (!ksctl)
    return;

  velocity = ksctl->get_board_controller().GetVelocity();
  dir = ksctl->get_board_controller().GetForwardDir();
  right = ksctl->get_board_controller().GetRightDir();


  if (BeachDataArray[g_game_ptr->get_beach_id()].bdir)
  {
    toTube = WAVE_SoundEmitter[WAVE_SE_TUBE].line.stop - WAVE_SoundEmitter[WAVE_SE_TUBE].line.start;
    toTubeDist = WAVE_SoundEmitter[WAVE_SE_TUBE].line.start.x - ksctl->get_owner_po().get_position().x;
    paddleDirVec = vector3d(-.3, 0, -.8);
    
    
  }
  else
  {
    toTube = WAVE_SoundEmitter[WAVE_SE_TUBE].line.start - WAVE_SoundEmitter[WAVE_SE_TUBE].line.stop;
    toTubeDist = ksctl->get_owner_po().get_position().x - WAVE_SoundEmitter[WAVE_SE_TUBE].line.stop.x;
    paddleDirVec = vector3d(.3, 0, -.8);
   
  }

  lastToTargetDistance = toTargetDistance;
  toTargetDistance = (target - ksctl->get_owner_po().get_position()).length();
  lastRelativeVelocityToTarget = relativeVelocityToTarget;

	float tfs=TIMER_GetFrameSec();
	if (tfs==0.0f)
		tfs=0.01f;
	// PLEASE Don't divide by zero
  relativeVelocityToTarget = (lastToTargetDistance - toTargetDistance)/tfs;

  

  paddleDirVec.normalize();
  dir.normalize();  
  toTube.normalize();
  heading = target - ksctl->get_owner_po().get_position();
  heading.normalize();
  if (toTargetDistance > 8)
    heading.z -= .2;

  heading.normalize();
  // The angle dbetween our current direction and the desired
  // paddling dir
  paddleAngle = angle_between(dir, paddleDirVec)*180.0f/(float)PI;

  // Calc the heading for once we're standing
  last_toward_tube_velocity = this_toward_tube_velocity;
  this_toward_tube_velocity = dot(velocity, heading);

  // Calc how far off we are off the desired heading
  offdh = dot(heading, dir);
  
  // calc the angle
  diffAngle = angle_between(dir, heading)*180.0f/(float)PI;

  
  velocity.normalize();
  
  if (last_toward_tube_velocity > -99999)
  {
   acel = this_toward_tube_velocity - last_toward_tube_velocity;
  }
  else
    acel = 0;


}

bool AISurferController::doStandUp()
{

  pressDir(PAD_L, 0);

  static float startPaddleTime = 999999;
  if(ksctl->get_super_state() != SUPER_STATE_LIE_ON_BOARD)
    return true;
  if (buttonStatus(PAD_TRIANGLE) && ksctl->get_current_state() == STATE_LIEONBOARD)
  {
    releaseButton(PAD_TRIANGLE);
    return true;
  }
  if (paddleAngle > 10) 
  {

    if (dot(paddleDirVec - dir, right) > 0)
      pressDir(PAD_R, (paddleAngle>90?1:paddleAngle/90.0f));
    else
      pressDir(PAD_L, (paddleAngle>90?1:paddleAngle/90.0f));
   
  }
  int myRegion = ksctl->get_board_controller().GetRegion();
  if ((myRegion  == WAVE_REGIONPOCKET) || (hitRegionPocket) || (myRegion  == WAVE_REGIONFACE))
  {
    hitRegionPocket = true;
    startPaddleTime = 99999;
    pressButton(PAD_TRIANGLE);
    pressDir(PAD_U, 1.0);
    return false;
    }
    else 
  {
    releaseButton(PAD_TRIANGLE);
  }

  if (myRegion == WAVE_REGIONWOBBLE)
  {    
    if (startPaddleTime == 99999)
    {
      startPaddleTime = TIMER_GetTotalSec();
    }
   
      //pressDir(PAD_U, 1);
    
  }
  else
  {
    pressDir(PAD_U, 0);
  }

  return false;
}


void AISurferController::moveToTarget()
{
  pressDir(PAD_U, 0);
  releaseButton(PAD_TRIANGLE);
  // If we're heading towards the tube and close to it
  // turn away
  if ((dot(toTube, dir) > 0) && (toTubeDist < 15))
  {

    if (dot((-1*toTube) - dir, right) > 0)
      pressButton(PAD_R);
    else
      pressButton(PAD_L);
    return;
  }
  // We're close, but no longer heading towards it
  else if (toTubeDist < 15)
  {
    // Still keep turning
    vector3d desiredHeading;
    desiredHeading = -1*toTube;
    // We don't want to head straight along 
    // the x axis
    // so add some z
    desiredHeading.z -= .2;
    desiredHeading.normalize();

    if (dot(desiredHeading, dir) < .8)
    {
      if (dot(desiredHeading - dir, right) > 0)
        pressDir(PAD_R, .5);
      else
        pressDir(PAD_L, .5);
    }
    else
    {
      releaseButton(PAD_L);
      pressButton(PAD_U);
    }
    return;
  }
  if (offdh < 0) // We're pointed in the wrong direction!
  {
    if (toTargetDistance < 4)
    {
      // Try to slow down
      pressDir(PAD_D, 1);
    }
    else
    {
      // No slowdown
      pressDir(PAD_D, 0);
      // If we're really off
      if (offdh < -.1)
      {
        // turn hard
        if (dot(heading - dir, right) > 0)
          pressDir(PAD_R, 1.0);
        else
          pressDir(PAD_L, 1.0);
        
      }
      else
      {
        // keep doing as before (prevents some flipping issues when offdh = 0
        X = oldX;
      }
    }
  }
  else if (offdh < .8) // Still have lots to turn
  { 
    // Keep turning 
    if (dot(heading - dir, right) > 0)
      pressDir(PAD_R, diffAngle>90.0?1.0f:diffAngle/90.0f);
    else
      pressDir(PAD_L, diffAngle>90.0?1.0f:diffAngle/90.0f);
 
  }
  else // we're pointed in the right general direction
  {

    
    // We're close, haven't turned much
    if ((oldX > .01) || (oldX < -.01))
    {
      // Slowly narrow in on the right heading
      if (acel > oldAcel)
        X  = oldX;   
      else
        X = -oldX * .85;
    }
    else
    {
      // keep on turning 
      if (dot(heading - dir, right) > 0)
        pressDir(PAD_R, diffAngle>90.0f?1.0f:diffAngle/90.0f);
      else
        pressDir(PAD_L, diffAngle>90.0f?1.0f:diffAngle/90.0f);
    
    }
    // Do we need a speed up?
    if (this_toward_tube_velocity < 0)
      pressDir(PAD_U, 1.0);

    else
    {
      if (toTargetDistance < 10)
      {
        if (relativeVelocityToTarget > 3) // if we are going 1 m/s faster than the target
          pressDir(PAD_D, 1.0);
        else
          pressDir(PAD_D, 0);

      }
      else if (toTargetDistance > 20)
        pressDir(PAD_U, 1.0);
      else
        pressDir(PAD_U, 0.0);
    }
  }
  
  int myRegion = ksctl->get_board_controller().GetRegion();
  // If we're still a little ways away, and on the lip, 
  // we leave it, since it can pull us back to the tube
  if ((myRegion == WAVE_REGIONLIP) && (toTargetDistance > 10))
  {
    if (dot(dir, vector3d(-1,0,0)) > 0)
      pressDir(PAD_L, 1.0);
    else
      pressDir(PAD_R, 1.0);
  }

  // Smooth out the turn values a bit
  X = (.75*X + .25 * oldX);

  if (X > 1) X = 1;
  if (X < -1) X = -1;
}

static float collideTimer = 99999;
#define NO_COLLIDE_TIME .1f
void AISurferController::checkCollisions()
{
  bool collided = false;
  static bool last_collided  = false;
  kellyslater_controller *thePlayer = g_world_ptr->get_ks_controller(0);
 	float r1, r2;
  entity *ent = ksctl->GetBoardMember();
  vector3d c1 (ent->get_abs_position ());
	nglMesh *mesh;
  bool sphereCollide = false;
	collideTimer += TIMER_GetFrameSec();
	if (collideTimer < NO_COLLIDE_TIME)
		return;
	
	entity *e2 = thePlayer->GetBoardMember();
	vector3d c2 (e2->get_abs_position ());
  r1 = ent->get_radius ();

  // Setup the centers of the spheres
  if (r1 < 0.01f)
  {
    if (ent->get_colgeom ())
      r1 = ent->get_colgeom ()->get_radius ();

    if (r1 < 0.01f)
    {
      mesh = ent->get_mesh ();

      if (mesh)
      {
        c1.x += mesh->SphereCenter[0];
        c1.y += mesh->SphereCenter[1];
        c1.z += mesh->SphereCenter[2];
        r1 = mesh->SphereRadius;
      }
    }
  }

  // Setup the centers of the spheres
	if (e2->is_visible ())
  {
		r2 = e2->get_radius ();

		if (r2 < 0.01f)
		{
		  if (e2->get_colgeom ())
			  r2 = e2->get_colgeom ()->get_radius ();

		  if (r2 < 0.01f)
		  {
			  mesh = e2->get_mesh ();

			  if (mesh)
			  {
			    c2.x += mesh->SphereCenter[0];
			    c2.y += mesh->SphereCenter[1];
			    c2.z += mesh->SphereCenter[2];
			    r2 = mesh->SphereRadius;
			  }
		  }
		}

		  // sphere-sphere collision
		if ((c1 - c2).length2 () <= (r1 + r2) * (r1+ r2))
			sphereCollide = true;
  }

  

  if (sphereCollide)
  {
    vector3d myPos  = ksctl->get_owner_po().get_position();
    vector3d hisPos = thePlayer->get_owner_po().get_position();
    myPos.y = 0;
    hisPos.y = 0;

    // Do an actual collision check between the boards
   /* if (e2->get_colgeom ())
    {
     

      if ((ksctl->get_current_state() != STATE_LIEONBOARD) && (thePlayer->get_current_state() != STATE_LIEONBOARD))
      {

        if ((myPos - hisPos).length2() < .4f)
        {
          collided = true;
        }
        else 
          collided = g_world_ptr->entity_entity_collision_check (ent, e2, 0);
      }
      else 
        collided = g_world_ptr->entity_entity_collision_check (ent, e2, 0);

      
      if ( collided )
      {*/
				collideTimer = 0.0f;
          vector3d v (c2 - c1);
          v.normalize ();

          // How did we hit?
          float fd = -dot (ent->get_abs_po ().get_x_facing (), v);

          if (fd > 0.7071067f)
          {
            ksctl->get_board_controller().DoWipeOut(WIP_LOW_AIR_FOR);  //  hit forward
          }
          else if (fd < -0.7071067f)
          {
            ksctl->get_board_controller().DoWipeOut(WIP_LOW_AIR_BACK);  //  hit back
          }
          else
          {
            if (dot (ent->get_abs_po ().get_z_facing (), v) > 0)
              ksctl->get_board_controller().DoWipeOut(WIP_LOW_AIR_RIGHT);  //  hit right
            else
              ksctl->get_board_controller().DoWipeOut(WIP_LOW_AIR_LEFT);  //  hit left
          }

          vector3d momentumP, momentumMe, axis;
          float axisMomentumP, axisMomentumMe;
          momentumP  = thePlayer->get_board_controller().rb->linMom;
          momentumMe = ksctl->get_board_controller().rb->linMom;
          // c1 = center for me
          // c2 = center for
          axis = c1 - c2;
		
					SoundScriptManager::inst()->playEvent(SS_HIT_GENERIC, ksctl->get_my_board_model());
	
          axis.normalize();
          axisMomentumP = dot(momentumP, axis);
          axisMomentumMe = dot(momentumMe, axis);
          axis = axis * (axisMomentumMe - axisMomentumP);
          ksctl->set_super_state(SUPER_STATE_WIPEOUT);
          ksctl->get_board_controller ().rb->linMom *= -1;
          ksctl->get_board_controller ().DoWipeOut(WIP_TAKE_OFF_FLAT);
          axis*=-1;
          thePlayer->get_board_controller().rb->linMom += .01f*axis;

     /* }
    }
*/


  }
  last_collided = collided;
}
int bounceDir = 0;
static bool doTricks = true;
void AISurferController::poll()
{
  
  setupStateVars();
  kellyslater_controller *otherPlayer = NULL;
  if (!ksctl)
    return;
  if (ksctl->get_super_state() == SUPER_STATE_FLYBY)
		return;
  checkCollisions(); 

  // See whether our heading has changed

  // Are we lying down?
  if ((ksctl->get_super_state() == SUPER_STATE_LIE_ON_BOARD))
  {
		if ((ksctl->get_last_super_state() != SUPER_STATE_LIE_ON_BOARD))
		{
			hitRegionPocket = false;
			clearButtons();
		}
    myState = STATE_STANDING;
    mySubstate = NO_SUBSTATE;
  }
   
  switch (myState)
  {
    case STATE_STANDING:  
    if (doStandUp())
    {
      myState = STATE_BOUNCE_AROUND_WAVE;
    }
    break;
    case STATE_CHASE:
      target = g_world_ptr->get_ks_controller(0)->get_owner_po().get_position();

      moveToTarget();
      if (doTricks && (random() < .01) && (toTubeDist  > 40))
      {
        
        curTrick = random(TRICK_NUM);
        // We dont want special tricks, and we only want air
        while (!(GTrickList[curTrick].flags & AirFlag) ||
              (GTrickList[curTrick].flags & SpecialFlag))
          curTrick = random(TRICK_NUM);
  
        myState = STATE_DO_TRICK;
      }
      break;
    case STATE_DO_TRICK:
      if (doTrick())
        myState = STATE_BOUNCE_AROUND_WAVE;
      break;
    case STATE_AVOID_SURFER:
      target = g_world_ptr->get_ks_controller(0)->get_owner_po().get_position();
      if (BeachDataArray[g_game_ptr->get_beach_id()].bdir)  
      {
        if (fabsf(target.x - (WAVE_SoundEmitter[WAVE_SE_FACE].line.stop.x-20)) > fabsf(target.x - WAVE_SoundEmitter[WAVE_SE_FACE].line.start.x))
        { 
         target = WAVE_SoundEmitter[WAVE_SE_FACE].line.stop;
         target.x -= 20;
        }
        else 
        { 
         target = WAVE_SoundEmitter[WAVE_SE_FACE].line.start;
        }
          
      }
      else
      {
        if (fabsf(target.x - (WAVE_SoundEmitter[WAVE_SE_FACE].line.start.x-20)) > fabsf(target.x - WAVE_SoundEmitter[WAVE_SE_FACE].line.stop.x))
        { 
         target = WAVE_SoundEmitter[WAVE_SE_FACE].line.start;
         target.x -= 20;
        }
        else 
        { 
         target = WAVE_SoundEmitter[WAVE_SE_FACE].line.stop;
        }
          
      }
      if (doTricks && (random() < .05) && (toTubeDist  > 40))
      {
        
        curTrick = random(TRICK_NUM);
        // We dont want special tricks, and we only want air
        while (!(GTrickList[curTrick].flags & AirFlag) ||
              (GTrickList[curTrick].flags & SpecialFlag))
          curTrick = random(TRICK_NUM);
  
        myState = STATE_DO_TRICK;
      }

      moveToTarget();
      break;
    case STATE_BOUNCE_AROUND_WAVE:
      if (BeachDataArray[g_game_ptr->get_beach_id()].bdir)  
      {
        if (bounceDir)
        { 
         target = WAVE_SoundEmitter[WAVE_SE_FACE].line.stop;
         target.x -= 20;
        }
        else 
        { 
         target = WAVE_SoundEmitter[WAVE_SE_FACE].line.start;
        }
        if ((ksctl->get_owner_po().get_position() - target).length2() < 25)
        {
          bounceDir = bounceDir?0:1;
        }

          
      }
      else
      {
        if (bounceDir)
        { 
         target = WAVE_SoundEmitter[WAVE_SE_FACE].line.start;
         target.x -= 20;
        }
        else 
        { 
         target = WAVE_SoundEmitter[WAVE_SE_FACE].line.stop;
        }
        if ((ksctl->get_owner_po().get_position() - target).length2() < 25)
        {
          bounceDir = bounceDir?0:1;
        }
          
      }
      if (doTricks && (random() < .05) && (toTubeDist  > 40))
      {
        
        curTrick = random(TRICK_NUM);
        // We dont want special tricks, and we only want air
        while (!(GTrickList[curTrick].flags & AirFlag) ||
              (GTrickList[curTrick].flags & SpecialFlag))
          curTrick = random(TRICK_NUM);
  
        myState = STATE_DO_TRICK;
      }
			otherPlayer = g_world_ptr->get_ks_controller(0);
			// if we're going in opposite directions
			if (dot(otherPlayer->get_board_controller().GetForwardDir(), ksctl->get_board_controller().GetForwardDir()) < -.5)
			{
				float badDist = -1;

				badDist = (otherPlayer->get_board_controller().rb->linMom + ksctl->get_board_controller().rb->linMom).length()*10;
				// And we're getting close
				if ((otherPlayer->get_owner_po().get_position() - ksctl->get_owner_po().get_position()).length() < badDist)
				{
					// Jump!
					myState = STATE_DO_TRICK;
				}
			}
			
      moveToTarget();
			
      break;
    default:
      assert(0);
      break;
  }

}



bool AISurferController::doTrick()
{
  float myAngle, dotVal;
  static float timer;
  static vector3d takeoff, land;
  static float launchHeight, lastHeight;
  vector3d dir2, right2;
  bool stopTurn = false;
  static float turnSign = 0;
  static float numButtons = -1;
  static bool  doneTrick = 0;
  static int   trickButtonProg;
  switch (mySubstate)
  {
    case NO_SUBSTATE:
      if (turnSign != 0)
        turnSign = 0;
      
      numButtons = -1;
      mySubstate = TRICK_APPROACH;
    case TRICK_APPROACH: // build up;

      if (numButtons == -1)
      {
        if (GTrickList[curTrick].button3 == PAD_NONE)
          numButtons = 2;
        else
          numButtons = 3;
      }
      trickButtonProg = -1;
      // Get moving!
      stopTurn = false;
      if (turnSign != 0)
        turnSign = 0;
      pressButton(PAD_CROSS);
      // Are we ready to launch?
      if (dot(dir, vector3d(0,0,1)) > .7) 
      {
        int region = ksctl->get_board_controller().GetRegion();
        if ((region == WAVE_REGIONLIP) || (region == WAVE_REGIONLIP2) || (region == WAVE_REGIONCHIN) )
          mySubstate = TRICK_LAUNCH;

        pressDir(PAD_L, 0);
      }
      else
      {
        if (dot(dir, vector3d(-1, 0, 0)) > 0)
          pressDir(PAD_R, 1.0);
        else
          pressDir(PAD_L, 1.0);
        
      }
      return false;
      break;
    case TRICK_LAUNCH:
      lastHeight = launchHeight = ksctl->get_owner_po().get_position().y;
      // LAUNCH!
      releaseButton(PAD_CROSS);
			pressDir(PAD_U, 0);
      mySubstate = TRICK_DOING_TRICK;
      timer = TIMER_GetTotalSec();
      return false;
      break;
    case TRICK_DOING_TRICK:
      takeoff = ksctl->get_board_controller().GetTakeoffDir();
      ButtonO = true;
      land = takeoff;
      
      land.y = 0;
      land.z *= -1;
      land.normalize();
      dir2 = dir;
      dir2.y = 0;
      dir2.normalize();
      right2 = right;
      right2.y = 0;
      right2.normalize();
      Y = -1;
      
      if (turnSign == 0)
      {

        if (numButtons == 2)  // Two button seqeunce.
        {
          if ((GTrickList[curTrick].button1 == PAD_L) || 
             (GTrickList[curTrick].button1 == PAD_DL) || 
             (GTrickList[curTrick].button1 == PAD_UL))
            turnSign = -1;
          else if ((GTrickList[curTrick].button1 == PAD_R) || 
                   (GTrickList[curTrick].button1 == PAD_DR) || 
                   (GTrickList[curTrick].button1 == PAD_UR))
            turnSign = 1;
          else
          {
            dotVal = dot(land - dir2, right2);
      
            if (dotVal > 0)
              turnSign = 1;
            else
              turnSign = -1;
          }
        }
        else
        {
          dotVal = dot(land - dir2, right2);
      
          if (dotVal > 0)
            turnSign = 1;
          else
            turnSign = -1;
        }
      }

      if ((numButtons == 2) || ((numButtons == 3) && (trickButtonProg == 3)))
      {
        myAngle = 180.0f*angle_between(dir2, land)/PI;
        // Rotate til we're ready to land

        if (myAngle > 90)
          if (turnSign > 0)
            pressDir(PAD_R, 1);
          else
            pressDir(PAD_L, 1);
        else if (myAngle > 10)
          if (turnSign > 0)
            pressDir(PAD_R, myAngle>90.0f?1.0f:myAngle/90.0f);
          else
            pressDir(PAD_L, myAngle>90.0f?1.0f:myAngle/90.0f);
        else 
        {
          pressDir(PAD_L, 0);
          stopTurn = true;
        }
      }  
    
      if (numButtons == 2)
      {
        if ((GTrickList[curTrick].button1 == PAD_DL) || 
           (GTrickList[curTrick].button1 == PAD_DR))
          pressButton(PAD_D);
          
        else if ((GTrickList[curTrick].button1 == PAD_UL) || 
                 (GTrickList[curTrick].button1 == PAD_UR))
          pressButton(PAD_U)    ;
        else
          pressButton(GTrickList[curTrick].button1);

        
        pressButton(GTrickList[curTrick].button2);
      }
      else if (numButtons == 3)
      {
        switch (trickButtonProg)
        {
          case -1:
            pressButton(GTrickList[curTrick].button1);
            trickButtonProg = 0;
            break;
          case 0:
            releaseButton(GTrickList[curTrick].button1);
            pressButton(GTrickList[curTrick].button2);
            trickButtonProg = 1;
            break;
          case 1:
            releaseButton(GTrickList[curTrick].button2);
            pressButton(GTrickList[curTrick].button3);
            trickButtonProg = 2;
            break;
          case 2:
            releaseButton(GTrickList[curTrick].button3);
            trickButtonProg = 3;
            break;
        }
      }

      if (stopTurn)
      {
        doneTrick = true;
        clearButtons();
      }

      if (doneTrick)
      {
        mySubstate = TRICK_LANDING;
      }
      lastHeight = ksctl->get_owner_po().get_position().y;
      return false;
      break;
    case TRICK_LANDING:
      // Rotate til we're ready to land
      dir2 = dir;
      dir2.y = 0;
      dir2.normalize();
      right2 = right;
      right2.y = 0;
      right2.normalize();
      Y = -1;
      
      myAngle = 180.0f*angle_between(dir2, land)/PI;
      // Rotate til we're ready to land

      if (myAngle > 90)
        X = turnSign*1;
      else if (myAngle > 10)
        X = turnSign * myAngle/90.0f;
      else
      {
        X = 0;
      }
      
      if ((lastHeight > ksctl->get_owner_po().get_position().y) && (ksctl->get_owner_po().get_position().y < launchHeight + 2))
      {
        ButtonO = false;
      }
      if (!ksctl->get_board_controller().inAirFlag)
        mySubstate = TRICK_DONE;
    
   
      lastHeight = ksctl->get_owner_po().get_position().y;

        
      return false;
      break;
    case TRICK_DONE:
      mySubstate = NO_SUBSTATE;
      return true;
      break;
    default:
      assert(0);
      break;
  }
  return false;
}


void AISurferController::pressButton(int which)
{
  switch(which)
  {
    case PAD_L:
      X = -1;
      break;
    case PAD_R:
      X = 1;
      break;
    case PAD_U:
      Y = -1;
      break;
    case PAD_D:
      Y = 1;
      break;
    case PAD_SQUARE:
      ButtonSq = true;
      break;
    case PAD_TRIANGLE:
      ButtonTr = true;
      break;
    case PAD_CIRCLE:
      ButtonO = true;
      break;
    case PAD_CROSS:
      ButtonX = true;
      break;
    case PAD_L1:
      ButtonL1 = true;
      break;
    case PAD_L2:
      ButtonL2 = true;
      break;
    case PAD_R1:
      ButtonR1 = true;
      break;
    case PAD_R2:
      ButtonR2 = true;
      break;
  }
}
void AISurferController::releaseButton(int which)
{
  switch(which)
  {
    case PAD_L:
      X = 0;
      break;
    case PAD_R:
      X = 0;
      break;
    case PAD_U:
      Y = 0;
      break;
    case PAD_D:
      Y = 0;
      break;
    case PAD_SQUARE:
      ButtonSq = false;
      break;
    case PAD_TRIANGLE:
      ButtonTr = false;
      break;
    case PAD_CIRCLE:
      ButtonO = false;
      break;
    case PAD_CROSS:
      ButtonX = false;
      break;
    case PAD_L1:
      ButtonL1 = false;
      break;
    case PAD_L2:
      ButtonL2 = false;
      break;
    case PAD_R1:
      ButtonR1 = false;
      break;
    case PAD_R2:
      ButtonR2 = false;
      break;
  }
}
bool AISurferController::buttonStatus(int which)
{
  switch(which)
  {
    case PAD_L:
      return X < -.5;
      break;
    case PAD_R:
      return X > .5;
      break;
    case PAD_U:
      return Y < -.5;
      break;
    case PAD_D:
      return Y > .5;
      break;
    case PAD_SQUARE:
      return ButtonSq;
      break;
    case PAD_TRIANGLE:
      return ButtonTr;
      break;
    case PAD_CIRCLE:
      return ButtonO;
      break;
    case PAD_CROSS:
      return ButtonX;
      break;
    case PAD_L1:
      return ButtonL1;
      break;
    case PAD_L2:
      return ButtonL2;
      break;
    case PAD_R1:
      return ButtonR1;
      break;
    case PAD_R2:
      return ButtonR2;
      break;
  }
  return false;
}
void AISurferController::pressDir(int which, float howmuch)
{ 
  assert(howmuch <= 1.05 && howmuch >=-.05);
  switch (which)
  {
    case PAD_L:
      X = -howmuch;
      break;
    case PAD_R:
      X = howmuch;
      break;
    case PAD_U:
      Y = -howmuch;
      break;
    case PAD_D:
      Y = howmuch;
      break;
    case PAD_UL:
      pressDir(PAD_L, howmuch);
      pressDir(PAD_U, howmuch);
      break;
    case PAD_UR:
      pressDir(PAD_R, howmuch);
      pressDir(PAD_U, howmuch);
      break;
    case PAD_DL:
      pressDir(PAD_L, howmuch);
      pressDir(PAD_D, howmuch);
      break;
    case PAD_DR:
      pressDir(PAD_R, howmuch);
      pressDir(PAD_D, howmuch);
      break;
  }
}
  