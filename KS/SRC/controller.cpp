////////////////////////////////////////////////////////////////////////////////
/*
  controller.cpp

  home of the highest level of player or AI control of game systems.

*/
////////////////////////////////////////////////////////////////////////////////
#include "global.h"

#include "controller.h"
//!#include "character.h"
//!#include "charhead.h"
#include "mcs.h"
#include "inputmgr.h"
#include "commands.h"
#include "osdevopts.h"
#include "menusys.h"
#include "FrontEndManager.h"

////////////////////////////////////////////////////////////////////////////////
// character_controller
////////////////////////////////////////////////////////////////////////////////


entity_controller::entity_controller( entity* ent )
{
  owner = ent;
  state = NONE;
}

entity_controller::~entity_controller()
{
}

player_controller * entity_controller::as_player_controller()
{
  stringx composite = owner->get_id().get_val() + " does not have a player controller.";
  error(composite.c_str());
  return NULL;
}

entity_controller* entity_controller::make_instance( entity *ent ) const
{
  entity_controller* c = NEW entity_controller( ent );
  c->copy_instance_data( *this );
  return c;
}

void entity_controller::copy_instance_data( const entity_controller& b )
{
  controller::copy_instance_data(b);

  state = b.state;
}


/*!
character_controller::character_controller( character* chr,
                                            character_head_fcs * _head_fcs )
  : entity_controller(chr)
{
  head_fcs = _head_fcs;
  recording = false;
}


character_controller::~character_controller()
{
}


// Control setting functions, which also can record info if move_editing.
// Just like the good old days!

void character_controller::set_target_speed_pct(rational_t v)
{
}


void character_controller::set_target_h_speed_pct(rational_t v)
{
}


void character_controller::set_d_theta_pct(rational_t v)
{
}


void character_controller::set_burst(vector3d dir, time_value_t time)
{
}


void character_controller::set_block_type(int b)
{
}


void character_controller::set_jump_flag(bool torf)
{
}


void character_controller::set_crouch_flag(bool torf)
{
}


void character_controller::set_front_crouch_flag(bool torf)
{
}


void character_controller::set_rear_crouch_flag(bool torf)
{
}


void character_controller::set_flip(bool torf)
{
}


void character_controller::set_neck_target_theta(rational_t t)
{
  if (head_fcs)
    head_fcs->controller_set_neck_target_theta(t);
}


void character_controller::set_neck_target_psi(rational_t t)
{
  if (head_fcs)
    head_fcs->controller_set_neck_target_psi(t);
}


void character_controller::set_neck_target_extend(rational_t t)
{
  if (head_fcs)
    head_fcs->controller_set_neck_target_extend(t);
}


void character_controller::set_head_target_psi(rational_t t)
{
  if (head_fcs)
    head_fcs->controller_set_head_target_psi(t);
}


void character_controller::set_head_target_phi(rational_t t)
{
  if (head_fcs)
    head_fcs->controller_set_head_target_phi(t);
}


void character_controller::set_jaw_target_psi(rational_t t)
{
  if (head_fcs)
    head_fcs->controller_set_jaw_target_psi(t);
}
!*/

////////////////////////////////////////////////////////////////////////////////
// mouselook_controller
////////////////////////////////////////////////////////////////////////////////
mouselook_controller::mouselook_controller(dolly_and_strafe_mcs* _move_cs,
                                           theta_and_psi_mcs* _angle_mcs)
  : controller(),
    move_cs(_move_cs),
    angle_mcs(_angle_mcs)
{
}

// <<<< for consistency, time_inc should be computed in the mcs
void mouselook_controller::frame_advance(time_value_t time_inc)
{
//  assert(g_world_ptr && g_world_ptr->get_hero_ptr());
//  static vector3d last_good_pos = g_world_ptr->get_hero_ptr()->get_abs_position();

  static const float inputscale = 1.0F/300.0F;
  device_id_t current_controller = JOYSTICK_TO_DEVICE_ID(input_mgr::inst()->GetDefaultController());

/*
#if defined(TARGET_PC)
  static const float system_scale = 1.0f;
#else
  static const float system_scale = 0.25f;
#endif
*/

  input_mgr* inputmgr = input_mgr::inst();

  switch(os_developer_options::inst()->get_camera_state())
  {
    case 2:
    {

      rational_t speed = 5.0f;
      if( inputmgr->get_control_state( current_controller, EDITCAM_FAST ) == AXIS_MAX )
        speed *= 5.0f;
      if( inputmgr->get_control_state( current_controller, EDITCAM_SLOW ) == AXIS_MAX )
        speed *= 0.2f;

//      vector3d hit_loc;
//      if(!in_world(app::inst()->get_game()->get_current_view_camera()->get_abs_position(), 0.25f, ZEROVEC, app::inst()->get_game()->get_current_view_camera()->get_region(), hit_loc))
//        app::inst()->get_game()->get_current_view_camera()->set_rel_position(last_good_pos);
//      last_good_pos = app::inst()->get_game()->get_current_view_camera()->get_abs_position();

      float pitch=0.0F, yaw=0.0F;

      if( inputmgr->get_control_state( current_controller, EDITCAM_USE_MOUSE ) == AXIS_MAX )
      {
#if defined(TARGET_MKS)
        pitch = inputmgr->get_control_state( current_controller, USERCAM_PITCH );
        yaw = -inputmgr->get_control_state( current_controller, USERCAM_YAW );
#else
        pitch = inputmgr->get_control_delta( current_controller, USERCAM_PITCH );
        yaw = -inputmgr->get_control_delta( current_controller, USERCAM_YAW );
#endif
        if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_INVERT_MOUSE_Y))
          pitch = -pitch;
      }
      else
      {
        if( inputmgr->get_control_state( current_controller, EDITCAM_PITCH_UP ) == AXIS_MAX )
          pitch += 1.0f;
        if( inputmgr->get_control_state( current_controller, EDITCAM_PITCH_DOWN ) == AXIS_MAX )
          pitch -= 1.0f;

        if( inputmgr->get_control_state( current_controller, EDITCAM_YAW_LEFT ) == AXIS_MAX )
          yaw -= 1.0f;
        if( inputmgr->get_control_state( current_controller, EDITCAM_YAW_RIGHT ) == AXIS_MAX )
          yaw += 1.0f;

        pitch *= 100.0f;
        yaw *= -100.0f;

        if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_INVERT_KEYBOARD_Y))
          pitch = -pitch;

        //pitch *= time_inc; // I think this will be a good idea, make it framerate independent.  --Sean
        //pan *= time_inc;

        pitch *= time_inc;
        yaw *= time_inc;
      }

      angle_mcs->set_tilt_for_next_frame(pitch * speed * inputscale );
      angle_mcs->set_pan_for_next_frame(yaw * speed * inputscale );

      float dolly=0.0f, lift=0.0f, strafe=0.0f;

      if( inputmgr->get_control_state( current_controller, EDITCAM_FORWARD ) == AXIS_MAX )
        dolly += 1.0f;
      if( inputmgr->get_control_state( current_controller, EDITCAM_BACKWARD ) == AXIS_MAX )
        dolly -= 1.0f;

      if( inputmgr->get_control_state( current_controller, EDITCAM_UP ) == AXIS_MAX )
        lift += 1.0f;
      if( inputmgr->get_control_state( current_controller, EDITCAM_DOWN ) == AXIS_MAX )
        lift -= 1.0f;

      if( inputmgr->get_control_state( current_controller, EDITCAM_STRAFE_RIGHT ) == AXIS_MAX )
        strafe += 1.0f;
      if( inputmgr->get_control_state( current_controller, EDITCAM_STRAFE_LEFT ) == AXIS_MAX )
        strafe -= 1.0f;

#if defined(TARGET_MKS)
      strafe *= 2.0f;
      dolly *= 2.0f;
      lift *= 2.0f;
#endif

      move_cs->set_dolly_for_next_frame( dolly*speed*time_inc );
      move_cs->set_lift_for_next_frame( lift*speed*time_inc );
      move_cs->set_strafe_for_next_frame( strafe*speed*time_inc );
    }
    break;

    default:
    {
      float pitch=0.0F, yaw=0.0F;

      rational_t speed = 10.0f;
      if( inputmgr->get_control_state( current_controller, USERCAM_FAST ) == AXIS_MAX )
        speed *= 5.0f;
      if( inputmgr->get_control_state( current_controller, USERCAM_SLOW ) == AXIS_MAX )
        speed *= 0.2f;

#if defined(TARGET_MKS)
      pitch = inputmgr->get_control_state( current_controller, USERCAM_PITCH ) * 2.0f;
      yaw = -inputmgr->get_control_state( current_controller, USERCAM_YAW ) * 2.0f;
#else
      pitch = inputmgr->get_control_delta( current_controller, USERCAM_PITCH );
      yaw = -inputmgr->get_control_delta( current_controller, USERCAM_YAW );
#endif

      if(os_developer_options::inst()->is_flagged(os_developer_options::FLAG_INVERT_MOUSE_Y))
        pitch = -pitch;

      angle_mcs->set_tilt_for_next_frame(pitch * speed * inputscale);
      angle_mcs->set_pan_for_next_frame(yaw * speed * inputscale);

      float dolly=0.0f, lift=0.0f, strafe=0.0f;

      if( inputmgr->get_control_state( current_controller, USERCAM_FORWARD ) == AXIS_MAX )
        dolly += 1.0f;
      if( inputmgr->get_control_state( current_controller, USERCAM_BACKWARD ) == AXIS_MAX )
        dolly -= 1.0f;

      if( inputmgr->get_control_state( current_controller, USERCAM_UP ) == AXIS_MAX || inputmgr->get_control_state( current_controller, USERCAM_DOWN ) == AXIS_MIN)
        lift += 1.0f;
      if( inputmgr->get_control_state( current_controller, USERCAM_DOWN ) == AXIS_MAX )
        lift -= 1.0f;

      if( inputmgr->get_control_state( current_controller, USERCAM_STRAFE_RIGHT ) == AXIS_MAX )
        strafe += 1.0f;
      if( inputmgr->get_control_state( current_controller, USERCAM_STRAFE_LEFT ) == AXIS_MAX )
        strafe -= 1.0f;

/*
#if defined(TARGET_MKS)
      strafe *= 2.0f;
      dolly *= 2.0f;
      lift *= 2.0f;
#endif
*/
      move_cs->set_dolly_for_next_frame( dolly*speed*time_inc );
      move_cs->set_lift_for_next_frame( lift*speed*time_inc );
      move_cs->set_strafe_for_next_frame( strafe*speed*time_inc );
    }
    break;
  }
}


////////////////////////////////////////////////////////////////////////////////
// joypad_usercam_controller
////////////////////////////////////////////////////////////////////////////////
joypad_usercam_controller::joypad_usercam_controller(dolly_and_strafe_mcs* _move_cs,
                                           theta_and_psi_mcs* _angle_mcs)
  : controller(),
    move_cs(_move_cs),
    angle_mcs(_angle_mcs)
{
}

extern game* g_game_ptr;
extern MenuSystem *menus;

	// producer screen shot hack
extern bool superduperpausehack;

// <<<< for consistency, time_inc should be computed in the mcs
void joypad_usercam_controller::frame_advance(time_value_t time_inc)
{

	if (FEDone() && !superduperpausehack && (g_game_ptr->is_paused() || menus->IsActive() || IGOIsPaused()))
		return;

	if (FEDone() && !g_game_ptr->user_cam_is_on())
		return;
//  assert(g_world_ptr && g_world_ptr->get_hero_ptr());
//  static vector3d last_good_pos = g_world_ptr->get_hero_ptr()->get_abs_position();

  //static const float inputscale = 1.0F/300.0F;
  float inputscale = 0.00333333f;

  input_mgr* inputmgr = input_mgr::inst();

  switch(os_developer_options::inst()->get_camera_state())
  {
    case 2:
    {

      rational_t speed = 5.0f;
      if( inputmgr->get_control_state( JOYSTICK_DEVICE, EDITCAM_FAST ) == AXIS_MAX )
        speed *= 5.0f;
      if( inputmgr->get_control_state( JOYSTICK_DEVICE, EDITCAM_SLOW ) == AXIS_MAX )
        speed *= 0.2f;

      float pitch=0.0F, yaw=0.0F;

      if( inputmgr->get_control_state( JOYSTICK_DEVICE, EDITCAM_USE_MOUSE ) == AXIS_MAX )
      {
#if defined(TARGET_MKS)
        pitch = inputmgr->get_control_state( JOYSTICK_DEVICE, USERCAM_PITCH );
        yaw = -inputmgr->get_control_state( JOYSTICK_DEVICE, USERCAM_YAW );
#else
        pitch = inputmgr->get_control_delta( JOYSTICK_DEVICE, USERCAM_PITCH );
        yaw = -inputmgr->get_control_delta( JOYSTICK_DEVICE, USERCAM_YAW );
#endif
        if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_INVERT_MOUSE_Y))
          pitch = -pitch;
      }
      else
      {
        if( inputmgr->get_control_state( JOYSTICK_DEVICE, EDITCAM_PITCH_UP ) == AXIS_MAX )
          pitch += 1.0f;
        if( inputmgr->get_control_state( JOYSTICK_DEVICE, EDITCAM_PITCH_DOWN ) == AXIS_MAX )
          pitch -= 1.0f;

        if( inputmgr->get_control_state( JOYSTICK_DEVICE, EDITCAM_YAW_LEFT ) == AXIS_MAX )
          yaw -= 1.0f;
        if( inputmgr->get_control_state( JOYSTICK_DEVICE, EDITCAM_YAW_RIGHT ) == AXIS_MAX )
          yaw += 1.0f;

        pitch *= 100.0f;
        yaw *= -100.0f;

        if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_INVERT_KEYBOARD_Y))
          pitch = -pitch;

        //pitch *= time_inc; // I think this will be a good idea, make it framerate independent.  --Sean
        //pan *= time_inc;

        pitch *= time_inc;
        yaw *= time_inc;
      }

      angle_mcs->set_tilt_for_next_frame(pitch * speed * inputscale );
      angle_mcs->set_pan_for_next_frame(yaw * speed * inputscale );

      float dolly=0.0f, lift=0.0f, strafe=0.0f;

      if( inputmgr->get_control_state( JOYSTICK_DEVICE, EDITCAM_FORWARD ) == AXIS_MAX )
        dolly += 1.0f;
      if( inputmgr->get_control_state( JOYSTICK_DEVICE, EDITCAM_BACKWARD ) == AXIS_MAX )
        dolly -= 1.0f;

      if( inputmgr->get_control_state( JOYSTICK_DEVICE, EDITCAM_UP ) == AXIS_MAX )
        lift += 1.0f;
      if( inputmgr->get_control_state( JOYSTICK_DEVICE, EDITCAM_DOWN ) == AXIS_MAX )
        lift -= 1.0f;

      if( inputmgr->get_control_state( JOYSTICK_DEVICE, EDITCAM_STRAFE_RIGHT ) == AXIS_MAX )
        strafe += 1.0f;
      if( inputmgr->get_control_state( JOYSTICK_DEVICE, EDITCAM_STRAFE_LEFT ) == AXIS_MAX )
        strafe -= 1.0f;

      move_cs->set_dolly_for_next_frame( dolly*speed*time_inc );
      move_cs->set_lift_for_next_frame( lift*speed*time_inc );
      move_cs->set_strafe_for_next_frame( strafe*speed*time_inc );
    }
    break;

    default:
    {
      float pitch=0.0F, yaw=0.0F;
      float dolly,lift,strafe;

      rational_t speed = 10.0f;
	  if(!FEDone()) speed = 2.0f;

#ifdef DEBUG
	  if (inputmgr->get_control_state( ANY_LOCAL_JOYSTICK,USERCAM_SCREEN_SHOT) == AXIS_MAX )
	  {
		  if (!g_render_cube_map && inputmgr->get_control_state( ANY_LOCAL_JOYSTICK,USERCAM_GENERATE_CUBE_MAP) == AXIS_MAX )
		  {
			  g_render_cube_map = 1;
		  }
		  else 
		  {
			  g_screenshot = true;
		  }
	  }
#endif
    	input_mgr* inputmgr = input_mgr::inst();
      input_device *input_dev = inputmgr->get_device(JOYSTICK2_DEVICE);
      if (input_dev  && input_dev->is_connected())
      {
        if (inputmgr->get_control_state(JOYSTICK2_DEVICE, PAD2_USERCAM_FAST) == AXIS_MAX )
          speed *= 5.0f;
        if( inputmgr->get_control_state(JOYSTICK2_DEVICE, PAD2_USERCAM_SLOW ) == AXIS_MAX )
          speed *= 0.2f;

        pitch = inputmgr->get_control_state( JOYSTICK2_DEVICE,  PAD2_USERCAM_PITCH );
        yaw = -inputmgr->get_control_state( JOYSTICK2_DEVICE, PAD2_USERCAM_YAW );


        if(os_developer_options::inst()->is_flagged(os_developer_options::FLAG_INVERT_MOUSE_Y))
          pitch = -pitch;

        angle_mcs->set_tilt_for_next_frame(pitch * speed * inputscale);
        angle_mcs->set_pan_for_next_frame(yaw * speed * inputscale);

        dolly=0.0f; lift=0.0f; strafe=0.0f;

        if( inputmgr->get_control_state( JOYSTICK2_DEVICE, PAD2_USERCAM_FORWARD ) == AXIS_MAX )
          dolly += 1.0f;
        if( inputmgr->get_control_state( JOYSTICK2_DEVICE, PAD2_USERCAM_BACKWARD ) == AXIS_MAX )
          dolly -= 1.0f;

        if( inputmgr->get_control_state( JOYSTICK2_DEVICE, PAD2_USERCAM_UP ) == AXIS_MAX || inputmgr->get_control_state( JOYSTICK2_DEVICE, PAD2_USERCAM_DOWN ) == AXIS_MIN)
          lift += 1.0f;
        if( inputmgr->get_control_state( JOYSTICK2_DEVICE, PAD2_USERCAM_DOWN ) == AXIS_MAX )
          lift -= 1.0f;

        if( inputmgr->get_control_state( JOYSTICK2_DEVICE, PAD2_USERCAM_STRAFE_RIGHT ) == AXIS_MAX )
          strafe += 1.0f;
        if( inputmgr->get_control_state( JOYSTICK2_DEVICE, PAD2_USERCAM_STRAFE_LEFT ) == AXIS_MAX )
          strafe -= 1.0f;
      }  //  end if second controller plugged in.
      else
      {

        if (inputmgr->get_control_state( JOYSTICK_DEVICE,USERCAM_FAST) == AXIS_MAX )
          speed *= 5.0f;
        if( inputmgr->get_control_state( JOYSTICK_DEVICE, USERCAM_SLOW ) == AXIS_MAX )
          speed *= 0.2f;

        pitch = inputmgr->get_control_state( JOYSTICK_DEVICE, USERCAM_PITCH );
        yaw = -inputmgr->get_control_state( JOYSTICK_DEVICE, USERCAM_YAW );


        if(os_developer_options::inst()->is_flagged(os_developer_options::FLAG_INVERT_MOUSE_Y))
          pitch = -pitch;

        angle_mcs->set_tilt_for_next_frame(pitch * speed * inputscale);
        angle_mcs->set_pan_for_next_frame(yaw * speed * inputscale);

        dolly=0.0f; lift=0.0f; strafe=0.0f;

        if( inputmgr->get_control_state( JOYSTICK_DEVICE, USERCAM_FORWARD ) == AXIS_MAX )
          dolly += 1.0f;
        if( inputmgr->get_control_state( JOYSTICK_DEVICE, USERCAM_BACKWARD ) == AXIS_MAX )
          dolly -= 1.0f;

        if( inputmgr->get_control_state( JOYSTICK_DEVICE, USERCAM_UP ) == AXIS_MAX || inputmgr->get_control_state( JOYSTICK_DEVICE, USERCAM_DOWN ) == AXIS_MIN)
          lift += 1.0f;
        if( inputmgr->get_control_state( JOYSTICK_DEVICE, USERCAM_DOWN ) == AXIS_MAX )
          lift -= 1.0f;

        if( inputmgr->get_control_state( JOYSTICK_DEVICE, USERCAM_STRAFE_RIGHT ) == AXIS_MAX )
          strafe += 1.0f;
        if( inputmgr->get_control_state( JOYSTICK_DEVICE, USERCAM_STRAFE_LEFT ) == AXIS_MAX )
          strafe -= 1.0f;
      }  //  end if second controller not plugged in.

      move_cs->set_dolly_for_next_frame( dolly*speed*time_inc );
      move_cs->set_lift_for_next_frame( lift*speed*time_inc );
      move_cs->set_strafe_for_next_frame( strafe*speed*time_inc );
    }
    break;
  }
}


/*
#if defined(TARGET_PC)

  ////////////////////////////////////////////////////////////////////////////////
  // edit_controller
  ////////////////////////////////////////////////////////////////////////////////
  edit_controller::edit_controller(dolly_and_strafe_mcs* _move_cs,
                                             theta_and_psi_mcs* _angle_mcs) :
      move_cs(_move_cs),
      angle_mcs(_angle_mcs), controller()
    {
    }

  // <<<< for consistency, time_inc should be computed in the mcs
  void edit_controller::frame_advance(time_value_t time_inc)
  {
    if( input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, EDITCAM_PITCH_UP ) == AXIS_MAX )
    {
      if(os_developer_options::inst()->is_flagged(os_developer_options::FLAG_INVERT_MOUSE_Y))
        angle_mcs->set_tilt_for_next_frame( 2.0f / 30.0f );
      else
        angle_mcs->set_tilt_for_next_frame( -2.0f / 30.0f );
    }
    else if( input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, EDITCAM_PITCH_DOWN ) == AXIS_MAX )
    {
      if(os_developer_options::inst()->is_flagged(os_developer_options::FLAG_INVERT_MOUSE_Y))
        angle_mcs->set_tilt_for_next_frame( -2.0f / 30.0f );
      else
        angle_mcs->set_tilt_for_next_frame( 2.0f / 30.0f );
    }

    if( input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, EDITCAM_YAW_LEFT ) == AXIS_MAX )
      angle_mcs->set_pan_for_next_frame( -2.0f / -30.0f );
    else if( input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, EDITCAM_YAW_RIGHT ) == AXIS_MAX )
      angle_mcs->set_pan_for_next_frame( 2.0f / -30.0f );

    rational_t speed = 10;
    if( input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, EDITCAM_FAST ) == AXIS_MAX )
      speed = 50;

    if( input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, EDITCAM_SLOW ) == AXIS_MAX )
      speed = 2;

    if( input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, EDITCAM_FORWARD ) == AXIS_MAX )
      move_cs->set_dolly_for_next_frame( speed*time_inc );

    if( input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, EDITCAM_BACKWARD ) == AXIS_MAX )
      move_cs->set_dolly_for_next_frame( -speed*time_inc );

    if( input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, EDITCAM_UP ) == AXIS_MAX )
      move_cs->set_lift_for_next_frame( speed*time_inc );

    if( input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, EDITCAM_DOWN ) == AXIS_MAX )
      move_cs->set_lift_for_next_frame( -speed*time_inc );

    if( input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, EDITCAM_STRAFE_RIGHT ) == AXIS_MAX )
      move_cs->set_strafe_for_next_frame( speed*time_inc );

    if( input_mgr::inst()->get_control_state( JOYSTICK_DEVICE, EDITCAM_STRAFE_LEFT ) == AXIS_MAX )
      move_cs->set_strafe_for_next_frame( -speed*time_inc );

    }
#endif

*/
