#include "global.h"
#include "game.h"
#include "gc_input.h"
#include "gc_file.h"
#include "joystick.h"
#include "ngl.h"
#include "osdevopts.h"
#ifdef CONTROLLER_MONKEY
#include "random.h"
#endif

static float clampf( float f, float n, float x )
{

	if( f < n ) {
		return n;
	} else if( f > x ) {
		return x;
	} else {
		return f;
	}

}

static u32 pad_motor_speed0[]= {MOTOR_START|33, MOTOR_STOP_HARD|132, MOTOR_END };//weak
static u32 pad_motor_speed1[]= {MOTOR_START|33, MOTOR_STOP|132, MOTOR_END };
static u32 pad_motor_speed2[]= {MOTOR_START|33, MOTOR_STOP|66,  MOTOR_END };
static u32 pad_motor_speed3[]= {MOTOR_START|33, MOTOR_STOP|33,  MOTOR_END };
static u32 pad_motor_speed4[]= {MOTOR_START|0xffffff, MOTOR_END};//strong

#define PAD_MOTOR_SPEED_NUM 5
static u32* pad_motor_speed[]=
{
		pad_motor_speed0,
		pad_motor_speed1,
		pad_motor_speed2,
		pad_motor_speed3,
		pad_motor_speed4,
		0
};
static	u32	pad_motor_speed_num= 4;
u32 pad_motor::default_commands[2];

pad_motor::pad_motor(u32 n_controller_id, u32 *n_commands, u32 on)
{
  controller_id= n_controller_id;
  set_commands( n_commands );
  current_command_start_time= 0;
  motor_on= on;
}

pad_motor::pad_motor(u32 n_controller_id)
{
	default_commands[0]= MOTOR_START|0xffffff;
	default_commands[1]= MOTOR_END;
  controller_id= n_controller_id;
  set_commands( default_commands );
  current_command_start_time= 0;
  motor_on= 0;
};

u32   pad_motor::get_new_command()
{
  u32  cur_time= OSTicksToMilliseconds( OSGetTime() );
  if(new_commands)
  {
    new_commands= 0;
    current_command_start_time= cur_time;
    return get_command();
  }
  if( (cur_time - current_command_start_time) > get_command_time())
  {
    current_command++;
    if( get_command()== MOTOR_END) current_command= commands;
    current_command_start_time= cur_time;
    return( get_command() );
  }
  else
  {
    return(MOTOR_NOP);
  }
}

void  pad_motor::turn_on()
{
  if(!motor_on)
	  set_commands(commands);
  motor_on= 1;
}

void pad_motor::turn_off()
{
  motor_on= 0;
  PADControlMotor(controller_id, PAD_MOTOR_STOP_HARD);
}

void pad_motor::work()
{
  if(motor_on)
	  switch ( get_new_command() )
	  {
	    case MOTOR_NOP:
	          break;
	    case MOTOR_START:
			      PADControlMotor(controller_id, PAD_MOTOR_RUMBLE);
	          break;
	    case MOTOR_STOP:
	          PADControlMotor(controller_id, PAD_MOTOR_STOP);
	          break;
	    case  MOTOR_STOP_HARD:
	          PADControlMotor(controller_id, PAD_MOTOR_STOP_HARD);
	          break;
    }
}

DEFINE_SINGLETON( gc_input_mgr )

// static class members
u32 gc_joypad_device::motor_bits;
PADStatus gc_joypad_device::pads[PAD_MAX_CONTROLLERS];
PADStatus gc_joypad_device::old_pads[PAD_MAX_CONTROLLERS];

gc_joypad_device::gc_joypad_device( int which_port )
{
  port = which_port;
  motor = new pad_motor( port );
  connected = false;
 
	memset( &pads[port], 0, sizeof( PADStatus ) );
	memset( &old_pads[port], 0, sizeof( PADStatus ) );

  poll( );
}

gc_joypad_device::~gc_joypad_device( )
{
	delete	motor;
}

stringx gc_joypad_device::get_name( void ) const
{
  return stringx( "gc input device" );
}

stringx gc_joypad_device::get_name( int axis ) const
{
  return stringx( "gc input device" );
}

device_id_t gc_joypad_device::get_id( void ) const
{
  return device_id;
}

void gc_joypad_device::vibrate( rational_t intensity )
{
	intensity = clampf( intensity, 0.0f, 0.999f );

  uint32 i= (uint32)( intensity/( 1.0f/(rational_t)PAD_MOTOR_SPEED_NUM ) );
  motor->set_commands( pad_motor_speed[i] );
	motor->turn_on();
}

void gc_joypad_device::vibrate( int vibrator_flag,
																int vibrator_power,
																int vibrator_freq,
																int vibrator_inc )
{
	vibrator_power /= 64;
	vibrator_power -= 1;
	
  if( vibrator_power >= PAD_MOTOR_SPEED_NUM )
    vibrator_flag = PAD_MOTOR_SPEED_NUM - 1;

  if( vibrator_power >= 0 )
  {
    motor->set_commands( pad_motor_speed[vibrator_flag] );
    motor->turn_on();
  }
  else
    motor->turn_off();
}

void gc_joypad_device::stop_vibration( void )
{
	motor->turn_off();
}

bool gc_joypad_device::is_vibrator_present( void ) const
{
  return ( ( motor_bits & ( PAD_CHAN0_BIT >> port ) ) != 0 );
}

int gc_joypad_device::get_axis_count( void ) const
{
  return JOY_GC_NUM_AXES;
}

axis_id_t gc_joypad_device::get_axis_id( int axis ) const
{
  return axis;
}

rational_t gc_joypad_device::get_axis_old_state( axis_id_t axis, int control_axis ) const
{
  return get_axis_state( axis, true );
}

rational_t gc_joypad_device::get_axis_delta( axis_id_t axis, int control_axis ) const
{
  return get_axis_state( axis, false ) - get_axis_state( axis, true );
}

rational_t gc_joypad_device::get_axis_state( axis_id_t axis, int control_axis ) const
{
  return get_axis_state( axis, false );
}

//#define LOG_AXIS_STATE 1

#ifdef LOG_AXIS_STATE
static const char* axis_name[] = {
	"d-pad x",
	"d-pad y",

	"d-pad x or analog x",
	"d-pad y or analog y",

	"left analog x",
	"left analog y",
	"dummy",

	"right analog x",
	"right analog y",
	"dummy",

	"button A",
	"button B",

	"dummy",

	"button X",
	"button Y",

	"dummy",

	"left shoulder",
	"right shoulder",

	"dummy",
	"dummy",

	"button start",
	"button Z",

	"disconnect"
};
#endif

#define JOY_STICK_DEADZONE  0.50f

// limit of an analog axis
#define GC_LSTICK_MAX_VALUE 80.0f
#define GC_RSTICK_MAX_VALUE 61.0f

rational_t gc_joypad_device::get_axis_state( axis_id_t axis, bool old ) const
{
	const rational_t limit = 74.0f;
	const rational_t limit_2 = limit * limit;

  rational_t dpad = 0.0f;
  rational_t stick = 0.0f;
  PADStatus* state = pads;
  rational_t value = 0.0f;

  if( old ) {
  	state = old_pads;
  }

  switch( axis ) {

  case JOY_GC_X:
		dpad = get_axis_state( JOY_GC_DX, old );
		stick = get_axis_state( JOY_GC_LX, old );

		if( __fabs( stick ) > __fabs( dpad ) ) {
			value = stick;
		} else {
			value = dpad;
		}

		break;

	case JOY_GC_Y:
		dpad = get_axis_state( JOY_GC_DY, old );
		stick = get_axis_state( JOY_GC_LY, old );

		if( __fabs( stick ) > __fabs( dpad ) ) {
			value = stick;
		} else {
			value = dpad;
		}

		break;

	case JOY_GC_DX:

		if( state[port].button & PAD_BUTTON_LEFT ) {
			value = -1.0f;
		} else if( state[port].button & PAD_BUTTON_RIGHT ) {
			value = 1.0f;
		} else {
			value = 0.0f;
		}

		break;

	case JOY_GC_DY:

		if( state[port].button & PAD_BUTTON_DOWN ) {
			value = 1.0f;
		} else if( state[port].button & PAD_BUTTON_UP ) {
			value = -1.0f;
		} else {
			value = 0.0f;
		}

		break;

	case JOY_GC_LX:
		value = state[port].stickX / GC_LSTICK_MAX_VALUE;
		value = clampf( value, -1.0f, 1.0f );

		if( value > -JOY_STICK_DEADZONE && value < JOY_STICK_DEADZONE ) {
			value = 0.0f;
		}

		break;

	case JOY_GC_LY:
		value = state[port].stickY / GC_LSTICK_MAX_VALUE;
		value = -value;
		value = clampf( value, -1.0f, 1.0f );

		if( value > -JOY_STICK_DEADZONE && value < JOY_STICK_DEADZONE ) {
			value = 0.0f;
		}

		break;

	case JOY_GC_RX:
		value = state[port].substickX / GC_RSTICK_MAX_VALUE;
		value = clampf( value, -1.0f, 1.0f );
		break;

	case JOY_GC_RY:
		value = state[port].substickY / GC_RSTICK_MAX_VALUE;
		value = -value;
		value = clampf( value, -1.0f, 1.0f );
		break;

	case JOY_GC_ANALOG_LEFT:
		// you might think, gee, then 128 != 75, and you'd
		// be right, but PADClamp makes this mistake, too
		value = (float) state[port].triggerLeft;
		value -= 30.0f;
		value = clampf( value, 0.0f, 150.0f );
		value /= 150.0f;
		break;

	case JOY_GC_LEFT:

		if( state[port].button & PAD_TRIGGER_L ) {
			value = 1.0f;
		}

		break;

	case JOY_GC_RIGHT:

		if( state[port].button & PAD_TRIGGER_R ) {
			value = 1.0f;
		}

		break;

	case JOY_GC_ANALOG_RIGHT:
		// same here with the hokiness
		value = (float) state[port].triggerRight;
		value -= 30.0f;
		value = clampf( value, 0.0f, 150.0f );
		value /= 150.0f;
		break;

	case JOY_GC_BTNA:

		if( state[port].button & PAD_BUTTON_A ) {
			value = 1.0f;
		}

		break;

	case JOY_GC_BTNB:

		if( state[port].button & PAD_BUTTON_B ) {
			value = 1.0f;
		}

		break;

	case JOY_GC_BTNX:

		if( state[port].button & PAD_BUTTON_X ) {
			value = 1.0f;
		}

		break;

	case JOY_GC_BTNY:

		if( state[port].button & PAD_BUTTON_Y ) {
			value = 1.0f;
		}

		break;

	case JOY_GC_BTNZ:

		if( state[port].button & PAD_TRIGGER_Z ) {
			value = 1.0f;
		}

		break;

	case JOY_GC_START:

		if( state[port].button & PAD_BUTTON_START ) {
			value = 1.0f;
		}

		break;

	case JOY_GC_DISCONNECT:

		if( state[port].err == PAD_ERR_NO_CONTROLLER ) {
			value = 1.0f;
		} else {
			value = 0.0f;
		}

		break;

	default:
		// dummy buttons fall through here
		break;
	}

#ifdef LOG_AXIS_STATE
	OSReport( "PAD query: [%s] %s = %f\n", old ? "old" : "current", axis_name[axis], value );
#endif

	return value;
}

void gc_joypad_device::record_demo_start( const stringx &filename )
{
	// empty
}

void gc_joypad_device::record_demo_stop( void )
{
	// empty
}

void gc_joypad_device::playback_demo_start( const stringx &filename )
{
	// empty
}

void gc_joypad_device::playback_demo_stop( void )
{
	// empty
}

//#define LOG_AXIS_DELTAS 1

void gc_joypad_device::poll( void )
{
	PADStatus temp[PAD_MAX_CONTROLLERS];

	motor->work( );

#ifdef CONTROLLER_MONKEY
	if( randomized ) {
		g_super_monkey.SetControllerInput( );
	} else {
		motor_bits = PADRead( temp );
	}
#else
	motor_bits = PADRead( temp );
#endif
	
	//snipped long winded comments refering to well documented performance
	//we can't call PADRead more than once per vblank, so we save all the
	//data on the first calls that don't return read errors.
	//
	for( int i = 0; i < PAD_MAX_CONTROLLERS; ++i ) {

		if( temp[i].err == PAD_ERR_NONE ) {
			memcpy( &old_pads[i], &pads[i], sizeof( PADStatus ) );
			memcpy( &pads[i], &temp[i], sizeof( PADStatus ) );
		} else if ( temp[i].err == PAD_ERR_NO_CONTROLLER ) {
			memcpy( &pads[i], &temp[i], sizeof( PADStatus ) );
			PADReset( PAD_CHAN0_BIT >> i );
		}

	}

#ifdef LOG_AXIS_DELTAS

	// yuck
	if( old_pads[port].button != pads[port].button ) {
		OSReport( "PAD: button 0x%x -> 0x%x (SYXBALRZUDRL).\n", old_pads[port].button, pads[port].button );
	}

	if( old_pads[port].stickX != pads[port].stickX ) {
		OSReport( "PAD: stickX %d -> %d.\n", old_pads[port].stickX, pads[port].stickX );
	}

	if( old_pads[port].stickY != pads[port].stickY ) {
		OSReport( "PAD: stickY %d -> %d.\n", old_pads[port].stickY, pads[port].stickY );
	}

	if( old_pads[port].substickX != pads[port].substickX ) {
		OSReport( "PAD: substickX %d -> %d.\n", old_pads[port].substickX, pads[port].substickX );
	}

	if( old_pads[port].substickY != pads[port].substickY ) {
		OSReport( "PAD: substickY %d -> %d.\n", old_pads[port].substickY, pads[port].substickY );
	}

#endif
}

bool gc_joypad_device::is_inserted( void )
{
	return is_connected();
}

bool gc_joypad_device::is_connected( void ) const
{
	return( pads[port].err != PAD_ERR_NO_CONTROLLER );
}

gc_input_mgr::gc_input_mgr( )
{

	// as a pedantic point, why 
	for( int i = 0; i < PAD_MAX_CONTROLLERS; i++ ) {
		pads[i] = new gc_joypad_device( i );
	}

}

gc_input_mgr::~gc_input_mgr( )
{

	for( int i = 0; i < PAD_MAX_CONTROLLERS; i++ ) {
		delete pads[i];
	}

}

void input_mgr::scan_devices( void )
{

	for( int i = 0; i < PAD_MAX_CONTROLLERS; ++i ) {
		input_device* id = gc_input_mgr::inst( )->get_pad( i );
		id->device_id = JOYSTICK_TO_DEVICE_ID( i + 1 );
		insert_device( id );
	}

}

//
// Monkeys, monkeys everywhere, and not a programmer to code.
//

void gc_joypad_device::set_button_d(int button_num, bool state) 
{ 
	if(state) 
		pads[0].button |= 1 << button_num; 
	else 
		pads[0].button &= ~(1 << button_num);

	// Sync up the analog and digital versions of the buttons
	if((1 << button_num) == PAD_BUTTON_A)
	{
		if(state == true)
			pads[0].analogA = 255;
		else
			pads[0].analogA = 0;
	}
	else if ((1 << button_num) == PAD_BUTTON_B)
	{
		if(state == true)
			pads[0].analogB = 255;
		else
			pads[0].analogB = 0;
	}
}

void gc_joypad_device::set_button_a(int button_num, int state) 
{ 
	#ifdef CONTROLLER_MONKEY
	switch(button_num)
	{
	case MONKEY_A_LEFT_TRIGGER: 
		pads[0].triggerLeft = state;
		if ( pads[0].triggerLeft < 128 )
			pads[0].button &= ~(PAD_TRIGGER_L);
		else
			pads[0].button |= PAD_TRIGGER_L;
		break;
	case MONKEY_A_RIGHT_TRIGGER: 
		pads[0].triggerLeft = state;
		if ( pads[0].triggerRight < 128 )
			pads[0].button &= ~(PAD_TRIGGER_R);
		else
			pads[0].button |= PAD_TRIGGER_L;
		break;
	case MONKEY_A_A:
		pads[0].analogA = state;
		if ( pads[0].analogA < 128 )
			pads[0].button &= ~(PAD_BUTTON_A);
		else
			pads[0].button |= PAD_BUTTON_A;
		break;
	case MONKEY_A_B:
		pads[0].analogB = state;
		if ( pads[0].analogB < 128 )
			pads[0].button &= ~(PAD_BUTTON_B);
		else
			pads[0].button |= PAD_BUTTON_B;
		break;
	}
#endif	
}

void gc_joypad_device::set_stick(int stick_num, int new_x, int new_y)
{
	if(stick_num == 0)
	{
		pads[0].stickX = new_x;
		pads[0].stickY = new_y;
	}
	else
	{
		pads[0].substickX = new_x;
		pads[0].substickY = new_y;
	}
}

#ifdef CONTROLLER_MONKEY
void gc_joypad_device::GetRandomInput()
{
	int i, j;

	pads[0].button = 0;

	int shifter = 1;

		// 13 monkey buttons
	for(i = 0; i < 13; i++)
	{
	if(random() < dbutton_probability[i])
	  pads[0].button |= shifter ;
	shifter <<= 1;
	}

	pads[0].stickX=random(-128,127);
	pads[0].stickY=random(-128,127);
	pads[0].substickX=random(-128,127);
	pads[0].substickY=random(-128,127);
	pads[0].triggerLeft=random(0,255);
	pads[0].triggerRight=random(0,255);
	pads[0].analogA=random(0,255);
	pads[0].analogB=random(0,255);

	pads[0].button &= ~(0x80); // no button at 0x80

		// I'm not sure if 128 is the right value to check here
	if ( pads[0].analogA < 128 )
		pads[0].button &= ~(PAD_BUTTON_A);
	else
		pads[0].button |= PAD_BUTTON_A;

	if ( pads[0].analogB < 128 )
		pads[0].button &= ~(PAD_BUTTON_B);
	else
		pads[0].button |= PAD_BUTTON_B;

	if ( pads[0].triggerLeft < 128 )
		pads[0].button &= ~(PAD_TRIGGER_L);
	else
		pads[0].button |= PAD_TRIGGER_L;

	if ( pads[0].triggerRight < 128 )
		pads[0].button &= ~(PAD_TRIGGER_R);
	else
		pads[0].button |= PAD_TRIGGER_L;
}
#endif
