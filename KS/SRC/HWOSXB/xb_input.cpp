/*-------------------------------------------------------------------------------------------------------

  XB_INPUT.CPP - DirectInput implementation.
  
-------------------------------------------------------------------------------------------------------*/

#include "global.h"

#include "xb_input.h" 
#include "xb_file.h" 
#include "joystick.h"
#include "keyboard.h"
#include "demomode.h"
//#include "xb_devopts.h"

#include "ngl.h"

#define BREAK_ON_DISCONNECT() assert(0)

#define NUM_KEYS 0x64
#define RANDOM_DISCONNECT_VALUES 0

#if RANDOM_DISCONNECT_VALUES  
#include "random.h"
#endif /* RANDOM_DISCONNECT_VALUES JIV DEBUG */

#ifndef countof
#define countof(x) (sizeof x / sizeof *x)
#endif /* countof */

// JIV FIXME clean this up, move input input mgr or something
static XINPUT_CAPABILITIES g_Gamepads[MAX_XB_PADS ];

xb_joypad_device *xbox_input_mgr::pad[MAX_XB_PADS];

static const unsigned char USBKB_to_KB[NUM_KEYS] =
{
	0,    // USBKEYC_NO_EVENT    0x00
		0,    // USBKEYC_E_ROLLOVER  0x01
		0,    // USBKEYC_E_POSTFAIL  0x02
		0,    // USBKEYC_E_UNDEF     0x03
		KB_A, // USBKEYC_A           0x04
		KB_B, // USBKEYC_B           0x05
		KB_C, // USBKEYC_C           0x06
		KB_D, // USBKEYC_D           0x07
		KB_E, // USBKEYC_E           0x08
		KB_F,// USBKEYC_F           0x09
		KB_G,// USBKEYC_G           0x0A
		KB_H,// USBKEYC_H           0x0B
		KB_I,// USBKEYC_I           0x0C
		KB_J,// USBKEYC_J           0x0D
		KB_K,// USBKEYC_K           0x0E
		KB_L,// USBKEYC_L           0x0F
		KB_M,// USBKEYC_M           0x10
		KB_N,// USBKEYC_N           0x11
		KB_O,// USBKEYC_O           0x12
		KB_P,// USBKEYC_P           0x13
		KB_Q,// USBKEYC_Q           0x14
		KB_R,// USBKEYC_R           0x15
		KB_S,// USBKEYC_S           0x16
		KB_T,// USBKEYC_T           0x17
		KB_U,// USBKEYC_U           0x18
		KB_V,// USBKEYC_V           0x19
		KB_W,// USBKEYC_W           0x1A
		KB_Z,// USBKEYC_X           0x1B
		KB_Y,// USBKEYC_Y           0x1C
		KB_Z,// USBKEYC_Z           0x1D
		KB_1,// USBKEYC_1           0x1E
		KB_2,// USBKEYC_2           0x1F
		KB_3,// USBKEYC_3           0x20
		KB_4,// USBKEYC_4           0x21
		KB_5,// USBKEYC_5           0x22
		KB_6,// USBKEYC_6           0x23
		KB_7,// USBKEYC_7           0x24
		KB_8,// USBKEYC_8           0x25
		KB_9,// USBKEYC_9           0x26
		KB_0,// USBKEYC_0           0x27
		KB_RETURN,// USBKEYC_ENTER       0x28
		KB_ESCAPE,// USBKEYC_ESC         0x29
		KB_BACKSPACE,// USBKEYC_BS          0x2A
		KB_TAB,// USBKEYC_TAB         0x2B
		KB_SPACE,// USBKEYC_SPACE       0x2C
		KB_MINUS,// USBKEYC_MINUS       0x2D
		KB_EQUALS,// USBKEYC_EQUAL_101             0x2E  /* = and + */
		KB_LBRACKET,// USBKEYC_LEFT_BRACKET_101      0x2F  /* [ */
		KB_RBRACKET,// USBKEYC_RIGHT_BRACKET_101     0x30  /* ] */
		KB_BACKSLASH,// USBKEYC_BACKSLASH_101         0x31  /* \ and | */
		0,// USBKEYC_RIGHT_BRACKET_106     0x32
		KB_SEMICOLON,// USBKEYC_SEMICOLON             0x33  /* ; */
		KB_QUOTE,// USBKEYC_QUOTATION_101         0x34  /* ' and " */
		0,// USBKEYC_106_KANJI   0x35
		KB_COMMA,// USBKEYC_COMMA       0x36
		KB_PERIOD,// USBKEYC_PERIOD      0x37
		KB_SLASH,// USBKEYC_SLASH       0x38
		KB_CAPSLOCK,// USBKEYC_CAPS_LOCK   0x39
		KB_F1,// USBKEYC_F1          0x3a
		KB_F2,// USBKEYC_F2          0x3b
		KB_F3,// USBKEYC_F3          0x3c
		KB_F4,// USBKEYC_F4          0x3d
		KB_F5,// USBKEYC_F5          0x3e
		KB_F6,// USBKEYC_F6          0x3f
		KB_F7,// USBKEYC_F7          0x40
		KB_F8,// USBKEYC_F8          0x41
		KB_F9,// USBKEYC_F9          0x42
		KB_F10,// USBKEYC_F10         0x43
		KB_F11,// USBKEYC_F11         0x44
		KB_F12,// USBKEYC_F12         0x45
		KB_PRINT,// USBKEYC_PRINTSCREEN 0x46
		KB_SCROLLLOCK,// USBKEYC_SCROLL_LOCK 0x47
		KB_PAUSE,// USBKEYC_PAUSE       0x48
		KB_INSERT,// USBKEYC_INSERT      0x49
		KB_HOME,// USBKEYC_HOME        0x4a
		KB_PAGEUP,// USBKEYC_PAGE_UP     0x4b
		KB_DELETE,// USBKEYC_DELETE      0x4c
		KB_END,// USBKEYC_END         0x4d
		KB_PAGEDOWN,// USBKEYC_PAGE_DOWN   0x4e
		KB_RIGHT,// USBKEYC_RIGHT_ARROW 0x4f
		KB_LEFT,// USBKEYC_LEFT_ARROW  0x50
		KB_DOWN,// USBKEYC_DOWN_ARROW  0x51
		KB_UP,// USBKEYC_UP_ARROW    0x52
		KB_NUMLOCK,// USBKEYC_KPAD_NUMLOCK   0x53
		KB_SLASH,// USBKEYC_KPAD_SLASH     0x54
		0,// USBKEYC_KPAD_ASTERISK  0x55
		KB_MINUS,// USBKEYC_KPAD_MINUS     0x56
		KB_ADD,// USBKEYC_KPAD_PLUS      0x57
		KB_NUMPADENTER,// USBKEYC_KPAD_ENTER     0x58
		KB_NUMPAD1,// USBKEYC_KPAD_1         0x59
		KB_NUMPAD2,// USBKEYC_KPAD_2         0x5A
		KB_NUMPAD3,// USBKEYC_KPAD_3         0x5B
		KB_NUMPAD4,// USBKEYC_KPAD_4         0x5C
		KB_NUMPAD5,// USBKEYC_KPAD_5         0x5D
		KB_NUMPAD6,// USBKEYC_KPAD_6         0x5E
		KB_NUMPAD7,// USBKEYC_KPAD_7         0x5F
		KB_NUMPAD8,// USBKEYC_KPAD_8         0x60
		KB_NUMPAD9,// USBKEYC_KPAD_9         0x61
		KB_NUMPAD0,// USBKEYC_KPAD_0         0x62
		KB_DECIMAL // USBKEYC_KPAD_PERIOD    0x63
};

DEFINE_SINGLETON(xbox_input_mgr)

xb_joypad_device *g_pad = NULL;

/*-------------------------------------------------------------------------------------------------------

  xb_joypad_device class.
  
	The xb hwos is used as a placeholder for porting apps which use hwos to new platforms.
	
-------------------------------------------------------------------------------------------------------*/


xb_joypad_device::xb_joypad_device(int which_port, int which_slot)
{
	port_id = which_port;
	slot_id = which_slot;
	
	//assert(port_id >= 0 && port_id < 2);
	assert(slot_id >= 0 && slot_id < 4);
	
	pad_type = -1;
	
	// demo mode stuff
	frames_since_change = 0;
	
	
	/* Open port */
	// Get a mask of all currently available devices
	DWORD device_mask = XGetDevices(XDEVICE_TYPE_GAMEPAD);
	DWORD MUmask = XGetDevices(XDEVICE_TYPE_MEMORY_UNIT);
	
	XINPUT_POLLING_PARAMETERS pollparms;
	
	memset(&pollparms,0, sizeof pollparms);
	pollparms.fAutoPoll=false;
	pollparms.fInterruptOut=true;
	pollparms.bInputInterval=8;
	pollparms.bOutputInterval=8;
	
	if( device_mask & (1<<port_id) ) 
	{
		// Get a handle to the device
		hDevice = XInputOpen( XDEVICE_TYPE_GAMEPAD, XDEVICE_PORT0 + slot_id, 
			XDEVICE_NO_SLOT, &pollparms );
		
		// Store capabilites of the device
		XInputGetCapabilities( hDevice, &g_Gamepads[port_id] );
		
		if (g_Gamepads[port_id].SubType != XINPUT_DEVSUBTYPE_GC_GAMEPAD && 
			g_Gamepads[port_id].SubType != XINPUT_DEVSUBTYPE_GC_GAMEPAD_ALT &&
			g_Gamepads[port_id].SubType != XINPUT_DEVSUBTYPE_GC_ARCADE_STICK &&
			g_Gamepads[port_id].SubType != XINPUT_DEVSUBTYPE_GC_DIGITAL_ARCADE_STICK &&
			g_Gamepads[port_id].SubType != XINPUT_DEVSUBTYPE_GC_FLIGHT_STICK)
		{
			XInputClose(hDevice);
			hDevice = 0;
			pad_type = -1;
			disconnected = true;
			
			nglPrintf( "port_id %d not valid or something\n", port_id );
		}
		
		nglPrintf( "port_id %d seems connected\n", port_id );
		
		memset( &rumble_data, 0, sizeof rumble_data);
		port_opened = true;
		disconnected = false;
	}
	else
	{
		nglPrintf( "port_id %d not connected or something\n", port_id );
		
		hDevice = 0;
		pad_type = -1;
		port_opened = false;
		disconnected = true;
	}
	was_disconnected = 0;
	pad_id = 0;
	term_id = 0;
	g_error_count = 0;
	phase = 0;
	state = 0;
	
	// Set the pad_state struct to all zero so the controller monkey doesn't
	// inadvertently pass back bad values
	memset(&pad_state, 0, sizeof(XINPUT_STATE));
	
	poll();
}

xb_joypad_device::~xb_joypad_device()
{
	if (demo_data != NULL)
		delete []demo_data;
}

stringx xb_joypad_device::get_name() const
{
	return stringx( "xb input device" );
}

stringx xb_joypad_device::get_name( int axis ) const
{
	return stringx( "xb input device" );
}

device_id_t xb_joypad_device::get_id() const
{
	return device_id;
}

#define BIG_RUMBLE_THRESHOLD 0.45f     // the point at which the heavy rumble engages
#define SMALL_RUMBLE_THRESHOLD 0.55f   // the point at which the light rumble maxes out

void xb_joypad_device::calc_rumble_intensity(float intensity)
{
	if(intensity > BIG_RUMBLE_THRESHOLD)
		rumble_data.Rumble.wLeftMotorSpeed = 
			(WORD)(((intensity - BIG_RUMBLE_THRESHOLD) / (1.0f - BIG_RUMBLE_THRESHOLD)) * (float)rumble_resolution_left);
	else
		rumble_data.Rumble.wLeftMotorSpeed = 0;
	if(intensity <= SMALL_RUMBLE_THRESHOLD)
		rumble_data.Rumble.wRightMotorSpeed = 
			(WORD)((intensity / SMALL_RUMBLE_THRESHOLD) * (float)rumble_resolution_right);
	else
		rumble_data.Rumble.wRightMotorSpeed = rumble_resolution_right;
}

void xb_joypad_device::vibrate( float intensity )
{
	if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_RUMBLE) && 
	    (rumble_resolution_left || rumble_resolution_right) &&
	    rumble_data.Header.dwStatus!=ERROR_IO_PENDING &&
		!disconnected)
	{
		calc_rumble_intensity(intensity);		
		XInputSetState( hDevice, &rumble_data );
	}
}

void xb_joypad_device::vibrate( int vibrator_flag, int vibrator_power, int vibrator_freq, int vibrator_inc )
{
	if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_RUMBLE) && 
	    (rumble_resolution_left || rumble_resolution_right) &&
	    rumble_data.Header.dwStatus!=ERROR_IO_PENDING &&
		!disconnected)
	{
		Motor0Timer = vibrator_inc;
		
		// parameters as passed in are 0..255, so shift them left to correspond to Xbox rumble range
		calc_rumble_intensity((float)vibrator_power / 255.0f);
		XInputSetState( hDevice, &rumble_data );
	}
}
void xb_joypad_device::stop_vibration()
{
	if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_RUMBLE))
	{
		// Wait for the rumble IO to be ready for another command
		while(rumble_data.Header.dwStatus!=ERROR_IO_PENDING)
			;
		Motor0Timer = 0; 
		rumble_data.Rumble.wLeftMotorSpeed = 0;
		rumble_data.Rumble.wRightMotorSpeed = 0;
		
		XInputSetState( hDevice, &rumble_data );
	}
}

bool xb_joypad_device::is_vibrator_present() const
{
	if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_RUMBLE))
		return false;
	
	return rumble_resolution_left || rumble_resolution_right;
}

int xb_joypad_device::get_axis_count() const
{
	return JOY_XBOX_NUM_AXES;
}

axis_id_t xb_joypad_device::get_axis_id(int axis) const 
{
	return axis;
}


rational_t xb_joypad_device::get_axis_old_state( axis_id_t axis, int control_axis ) const
{
#if defined(TARGET_XBOX)
	if( axis == JOY_XBOX_DISCONNECT )
	{
		BREAK_ON_DISCONNECT();
		
		if( was_disconnected == 1 )
			return 1.0f;
		return 0.0f;
	}
	else
	{
		return get_axis_state( axis, old_pad_state.Gamepad );
	}
	STUB( "xb_joypad_device::get_axis_old_state" );
	
	return 1.0f;
#else
	if( axis == JOY_XBOX_DISCONNECT )
	{
		if( was_disconnected == 1 )
			return 1.0f;
		return 0.0f;
	}
	else
	{
		return get_axis_state(axis, prev_rdata);
	}
#endif /* TARGET_XBOX JIV DEBUG */
}

rational_t xb_joypad_device::get_axis_delta( axis_id_t axis, int control_axis ) const
{
#if defined(TARGET_XBOX)
	if( axis == JOY_XBOX_DISCONNECT )
	{
		BREAK_ON_DISCONNECT();
		
		return (rational_t)(disconnected - was_disconnected);
	}
	else
	{
		//      STUB( "xb_joypad_device::get_axis_delta" );
		
		return 0.0f;
	}
#else
	if( axis == JOY_XBOX_DISCONNECT )
	{
		return (rational_t)(disconnected - was_disconnected);
	}
	else
	{
		return get_axis_state(axis, curr_rdata) - get_axis_state(axis, prev_rdata);
	}
#endif /* TARGET_XBOX JIV DEBUG */
}

rational_t xb_joypad_device::get_axis_state( axis_id_t axis, int control_axis ) const
{
#if defined(TARGET_XBOX)
	if( axis == JOY_XBOX_DISCONNECT )
	{
		BREAK_ON_DISCONNECT();
		
		if( disconnected == 1 )
			return 1.0f;
		
		return 0.0f;
	}
	else
	{
		return get_axis_state(axis);
	}
#else
	if( axis == JOY_XBOX_DISCONNECT )
	{
		if( disconnected == 1 )
			return 1.0f;
		return 0.0f;
	}
	else
	{
		return get_axis_state(axis, curr_rdata);
	}
	
#endif /* TARGET_XBOX JIV DEBUG */
}

// rdata[2] defines	
#define XB_BTN_SELECT (unsigned char)0x01
#define XB_BTN_L3     (unsigned char)0x02
#define XB_BTN_R3     (unsigned char)0x04
#define XB_BTN_START  (unsigned char)0x08
#define XB_BTN_UP     (unsigned char)0x10
#define XB_BTN_RIGHT  (unsigned char)0x20
#define XB_BTN_DOWN   (unsigned char)0x40
#define XB_BTN_LEFT   (unsigned char)0x80

// rdata[3] defines
#define XB_BTN_L2     (unsigned char)0x01
#define XB_BTN_R2     (unsigned char)0x02
#define XB_BTN_L1     (unsigned char)0x04
#define XB_BTN_R1     (unsigned char)0x08
#define XB_BTN_TR     (unsigned char)0x10
#define XB_BTN_O      (unsigned char)0x20
#define XB_BTN_X      (unsigned char)0x40
#define XB_BTN_SQ     (unsigned char)0x80

rational_t xb_joypad_device::get_axis_state( axis_id_t axis ) const
{
	return get_axis_state( axis, pad_state.Gamepad );
}

rational_t xb_joypad_device::get_axis_state( axis_id_t axis, const XINPUT_GAMEPAD &gp ) const
{
#if RANDOM_DISCONNECT_VALUES  
	if(disconnected)
	{
		switch (axis) 
		{
		case JOY_XBOX_X: // left analog horiz
		case JOY_XBOX_Y: // left analog vert
		case JOY_XBOX_DX: // just Dpad
		case JOY_XBOX_DY: // just Dpad
		case JOY_XBOX_LX: // just Lstick
		case JOY_XBOX_LY: // just Lstick
		case JOY_XBOX_RX: // right analog horiz
		case JOY_XBOX_RY: // right analog vert
		case JOY_XBOX_BTNL3: // L3
		case JOY_XBOX_BTNR3: // R3
		case JOY_XBOX_BTNX:  // A (xb x)
		case JOY_XBOX_BTNO:  // B (xb o)
		case JOY_XBOX_BTNL1: // Z (xb L1)
		case JOY_XBOX_BTNR1: // C (xb R1)
		case JOY_XBOX_BTNSQ: // X (xb sq)
		case JOY_XBOX_BTNTR: // Y (xb tr)
		case JOY_XBOX_BTNL2: // L (xb L2)
		case JOY_XBOX_BTNR2: // R (xb R2)
			return random(255) / 255.0f;
		case JOY_XBOX_START: // Start
		default:
			return 0.0f;
		}
	}
#else
	if(disconnected) 
		return 0.0f;
#endif /* !RANDOM_DISCONNECT_VALUES JIV DEBUG */
	
	rational_t dpad, astick;
	
	float retval;
	int16 y_val; // needed for interpretation of the analog stick's Y value
	
	switch (axis) 
	{
    case JOY_XBOX_X: // left analog horiz
		dpad = get_axis_state((axis_id_t)JOY_XBOX_DX);
		astick = get_axis_state((axis_id_t)JOY_XBOX_LX);
		if (__fabs(astick) > __fabs(dpad))
			return astick;
		return dpad;
		
    case JOY_XBOX_Y: // left analog vert
		dpad = get_axis_state((axis_id_t)JOY_XBOX_DY);
		astick = get_axis_state((axis_id_t)JOY_XBOX_LY);
		if (__fabs(astick) > __fabs(dpad))
			return astick;
		return dpad;
		
    case JOY_XBOX_DX: // just Dpad
		if(gp.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
		{
			return -1.0f;
		}
		else if(gp.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
		{
			return 1.0f;
		}
		
		return 0.0f;
    case JOY_XBOX_DY: // just Dpad
		if(gp.wButtons & XINPUT_GAMEPAD_DPAD_UP)
		{
			return -1.0f;
		}
		else if(gp.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
		{
			return 1.0f;
		}
		
		return 0.0f;
    case JOY_XBOX_LX: // just Lstick
		retval = joystick_threshold(gp.sThumbLX) / 32767.0f;
		return retval;
    case JOY_XBOX_LY: // just Lstick
		// we're going to negate the Y value, so we have to first make sure
		// that it's not -32768, because the negative of that doesn't fit
		// in a short int
		y_val = gp.sThumbLY == -32768 ? -32767 : gp.sThumbLY;
		retval = joystick_threshold(-y_val) / 32767.0f;
		return retval;
    case JOY_XBOX_BTNL3: // L3
		if(gp.wButtons & XINPUT_GAMEPAD_LEFT_THUMB)
		{
			return 1.0f;
		}
		
		return 0.0f;
    case JOY_XBOX_RX: // right analog horiz
		retval = joystick_threshold(gp.sThumbRX) / 32767.0f;
		return retval;
    case JOY_XBOX_RY: // right analog vert
		// we're going to negate the Y value, so we have to first make sure
		// that it's not -32768, because the negative of that doesn't fit
		// in a short int
		y_val = gp.sThumbRY == -32768 ? -32767 : gp.sThumbRY;
		retval = joystick_threshold(-y_val) / 32767.0f;
		return retval;
    case JOY_XBOX_BTNR3: // R3
		if(gp.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)
		{
			return 1.0f;
		}
		
		return 0.0f;
    case JOY_XBOX_BTNA:	// (xb x)
		return button_threshold(gp.bAnalogButtons[XINPUT_GAMEPAD_A]) / 255.0f;
    case JOY_XBOX_BTNB:	// (xb o)
		return button_threshold(gp.bAnalogButtons[XINPUT_GAMEPAD_B]) / 255.0f;
    case JOY_XBOX_BTNL:	// (xb L1)
		return button_threshold(gp.bAnalogButtons[XINPUT_GAMEPAD_LEFT_TRIGGER]) / 255.0f;
    case JOY_XBOX_BTNR:	// (xb R1)
		return button_threshold(gp.bAnalogButtons[XINPUT_GAMEPAD_RIGHT_TRIGGER]) / 255.0f;
    case JOY_XBOX_BTNX:	// (xb sq)
		return button_threshold(gp.bAnalogButtons[XINPUT_GAMEPAD_X]) / 255.0f;
    case JOY_XBOX_BTNY:	// (xb tr)
		return button_threshold(gp.bAnalogButtons[XINPUT_GAMEPAD_Y]) / 255.0f;
    case JOY_XBOX_BTNWHT:	// (xb L2)
		return button_threshold(gp.bAnalogButtons[XINPUT_GAMEPAD_WHITE]) / 255.0f;
    case JOY_XBOX_BTNBLK:	// (xb R2)
		return button_threshold(gp.bAnalogButtons[XINPUT_GAMEPAD_BLACK]) / 255.0f;
    case JOY_XBOX_START: // Start
		if(gp.wButtons & XINPUT_GAMEPAD_START)
		{
			return 1.0f;
		}
		
		return 0.0f;
    case JOY_XBOX_SELECT:
		if(gp.wButtons & XINPUT_GAMEPAD_BACK)
		{
			return 1.0f;
		}
		
		return 0.0f;
    default:
		assert("xb_joypad_device::get_axis_state illegal axis queried." == 0);
		return 0.0f;
  }
}

void xb_joypad_device::record_demo_start(const stringx &filename)
{
	int frame_lock;
	assert(demo_log.is_open() == false);
	demo_log.open(filename, os_file::FILE_WRITE);
	demo_log.write((void *)"demo", 4);
	if (os_developer_options::inst()->get_int(os_developer_options::INT_FRAME_LOCK) == 0)
		frame_lock = 30;
	else
		frame_lock = os_developer_options::inst()->get_int(os_developer_options::INT_FRAME_LOCK);
	demo_log.write(&frame_lock, sizeof(frame_lock));
	
	recording_demo = true;
}

void xb_joypad_device::record_demo_stop()
{
	if (demo_log.is_open())
		demo_log.close();
	recording_demo = false;
}

void xb_joypad_device::playback_demo_start(const stringx &filename)
{
	assert(demo_log.is_open() == false);
	demo_log.open(filename, os_file::FILE_READ);
	
	char id_string[5];
	id_string[4] = '\0';
	demo_log.read(id_string, 4);
	
	if (strcmp(id_string, "demo") != 0)
	{
		warning("Demo log %s is not a demo log...Not playing back demo.");
		demo_log.close();
		return;
	}
	
	int frame_lock;
	demo_log.read(&frame_lock, sizeof(frame_lock));
	debug_print("Playback of %s demo is overriding framelock to be %d", filename.c_str(), frame_lock);
	os_developer_options::inst()->set_int(os_developer_options::INT_FRAME_LOCK, frame_lock);
	
	// 8 byte header, "demo" + frame_lock
	keyframe_count = (demo_log.get_size() - 8) / sizeof(demo_keyframe);
	assert((demo_log.get_size() - 8) % sizeof(demo_keyframe) == 0);
	demo_data = new demo_keyframe[keyframe_count];
	demo_log.read(demo_data, keyframe_count * sizeof(demo_keyframe));
	demo_log.close();
	curr_keyframe = 0;
	playing_demo = true;
}

void xb_joypad_device::playback_demo_stop()
{
	playing_demo = false;
	if (demo_data != NULL)
		delete []demo_data;
}

int xbox_input_mgr::get_first_free_portid( void )
{
	for( int i = 0; i < MAX_XB_PADS; i++ )
		if(!pad[i]->is_connected())
			return i;
		
		return -1;
}

void xbox_input_mgr::poll( void )
{
	DWORD insertions, removals;
	
	BOOL change = XGetDeviceChanges( XDEVICE_TYPE_GAMEPAD,
		&insertions,
		&removals );
	
	if(!change)
		// nothing has been removed or inserted since we last checked.
		return;
	
	int port_id;
	for(port_id = 0; port_id < 4; port_id++)
	{
		if(insertions & (XDEVICE_PORT0_MASK << port_id))
		{
			
			nglPrintf( "insertion at port %d\n", port_id );
			
			// Whoever put this in, I don't know what they were thinking
			//pad[port_id]->port_id = get_first_free_portid();

			pad[port_id]->reconnect();

			// If we're in a demo right now, a reconnected controller needs to pull it back to the FE
			if(dmm.inDemoMode())
				dmm.keyHit();
		}
		if(removals & (XDEVICE_PORT0_MASK << port_id))
		{
			nglPrintf( "removal at port %d\n", port_id );
			pad[port_id]->disconnect();
		}
	}
	
}


void xb_joypad_device::poll()
{
	xbox_input_mgr::poll();
	
	if(port_id == -1)
	{
		disconnected = true;
		return;
	}
	
	if(!hDevice)
	{
		disconnected = true;
		// missing
		return;
	}
	
	// save old state
	old_pad_state = pad_state;
	
	// get current state 
	DWORD inputres;
	
#ifdef CONTROLLER_MONKEY
	if(randomized)
	{
		g_super_monkey.SetControllerInput();
		inputres = ERROR_SUCCESS;
	}
	else
#endif
		inputres = XInputGetState( hDevice, &pad_state);
	
	if (inputres != ERROR_SUCCESS)
	{
		g_error_count = 0;
		phase = 0;
		was_disconnected = disconnected;
		disconnected = true;
		
		nglPrintf( "must have lost controller %d\n", port_id );
		return; // must have lost it
	}
	
	was_disconnected = disconnected;
	disconnected = false;
	
	// get current inputs
	inputres = XInputPoll(hDevice);
}

xbox_input_mgr::xbox_input_mgr()
{
	XDEVICE_PREALLOC_TYPE devprealloc[] =
	{ 
		{XDEVICE_TYPE_GAMEPAD, 4},
		{XDEVICE_TYPE_MEMORY_UNIT, 8},
	};
	
	XInitDevices(countof(devprealloc), devprealloc);
	
	DWORD then = timeGetTime();
	DWORD now;
	
	/* wait .5 seconds */
	while((now = timeGetTime()) < (then + 500))
	{
		continue;
	}
	stringx debug_dir("C:\\");
	
	// JIV FIXME do the remapping something something
	g_pad = NULL;
	int joystick_num = 1;
	
	for(int _port=0; _port < MAX_XB_PADS; ++_port)
	{
		pad[_port] = new xb_joypad_device(_port, _port);
		if(pad[_port]->is_port_open() && pad[_port]->is_connected())
		{
			pad[_port]->device_id = JOYSTICK_TO_DEVICE_ID(joystick_num);
			++joystick_num; 
			
			if(g_pad == NULL)
			{
				g_pad = pad[_port];
			}
		}
		else
		{
			pad[_port]->device_id = INVALID_DEVICE_ID;
		}
	}
	
	if(g_pad == NULL)
	{
		g_pad = pad[0];
	}
	
	// input manager is braindead, sort devices if something isn't
	// plugged into the first few slots
	
	//  sort_pads();
}

bool xb_inserted_devices = false;
xbox_input_mgr::~xbox_input_mgr()
{
	for(int i=0; i<MAX_XB_PADS; ++i)
	{
		if (pad[i])
			delete pad[i];
	}
	
	xb_inserted_devices = false;
}


void input_mgr::scan_devices()
{
	debug_print( "scanning devices\n" );
	
	if(!xb_inserted_devices)
	{
		xb_inserted_devices = true;
		for(int i=0; i<MAX_XB_PADS; ++i)
		{
			xbox_input_mgr::inst()->pad[i]->device_id = JOYSTICK_TO_DEVICE_ID(i+1);
			assert(IS_JOYSTICK_DEVICE(xbox_input_mgr::inst()->pad[i]->device_id));
			insert_device(xbox_input_mgr::inst()->pad[i]);
		}
		
		// scan for keyboard
		// we add a keyboard 0 without scanning, the poll method checks for the presence
		// of a keyboard each call. Seems to be the way sony recommends
		// of course the documentation is in japanese so its kind of hard
		// for me to be an expert on the subject
#ifdef ENABLE_USB_KEYB
		insert_device( new xb_keyboard_device( 0 ) );
#endif
	}
}

void xbox_input_mgr::sort_pads(void)
{
	for( int i = 0; i < MAX_XB_PADS; i++ )
		for( int j = i; j < MAX_XB_PADS; j++ )
			if(pad[j]->is_connected() && !pad[i]->is_connected())
			{
				xb_joypad_device *temp = pad[i];
				pad[i] = pad[j];
				pad[j] = temp;
			}
}

void xb_joypad_device::reconnect( void )
{
	assert( port_id >= 0 );
	assert( port_id < MAX_XB_PADS );  
	
	nglPrintf( "reconnecting port_id %d\n", port_id );
	
	XINPUT_POLLING_PARAMETERS pollparms;
	
	memset(&pollparms,0, sizeof pollparms);
	
	pollparms.fAutoPoll=false;
	pollparms.fInterruptOut=true;
	pollparms.bInputInterval=8;
	pollparms.bOutputInterval=8;
	
	// Get a handle to the device
	hDevice = XInputOpen( XDEVICE_TYPE_GAMEPAD, XDEVICE_PORT0 + slot_id, 
		XDEVICE_NO_SLOT, &pollparms );
	
	// Store capabilites of the device
	XInputGetCapabilities( hDevice, &g_Gamepads[port_id] );
	
	if (g_Gamepads[port_id].SubType != XINPUT_DEVSUBTYPE_GC_GAMEPAD && 
		g_Gamepads[port_id].SubType != XINPUT_DEVSUBTYPE_GC_GAMEPAD_ALT &&
		g_Gamepads[port_id].SubType != XINPUT_DEVSUBTYPE_GC_ARCADE_STICK &&
		g_Gamepads[port_id].SubType != XINPUT_DEVSUBTYPE_GC_FLIGHT_STICK)
	{
		XInputClose(hDevice);
		hDevice = 0;
		pad_type = -1;
		disconnected = true;
		
		nglPrintf( "port_id %d not valid or something\n", port_id );
	}
	
	nglPrintf( "port_id %d reconnected\n", port_id );
    
	memset( &rumble_data, 0, sizeof rumble_data );
	
	port_opened = true;
	was_disconnected = disconnected;
	disconnected = false;
	
	pad_id = 0;
	term_id = 0;
	g_error_count = 0;
	phase = 0;
	state = 0;
	
	return;
}

void xb_joypad_device::disconnect( void )
{
    nglPrintf( "disconnecting port_id %d\n", port_id );
	
    XInputClose(hDevice);
    hDevice = 0;
	
    port_opened = false;
    was_disconnected = disconnected;
    disconnected = true;
	
    nglPrintf( "port_id %d disconnected\n", port_id );
}

void xb_joypad_device::set_button_d(int button_num, bool state) 
{ 
	if(state) // set the bit on
		pad_state.Gamepad.wButtons |= 1 << button_num; 
	else // set the bit off
		pad_state.Gamepad.wButtons &= ~(1 << button_num); 
}

void xb_joypad_device::set_stick(int stick_num, int new_x, int new_y)
{
	if(stick_num == 0) // left stick
	{
		pad_state.Gamepad.sThumbLX = new_x;
		pad_state.Gamepad.sThumbLY = new_y;
	}
	else // right stick
	{
		pad_state.Gamepad.sThumbRX = new_x;
		pad_state.Gamepad.sThumbRY = new_y;
	}
}

////////////////////////////////////////////////////////////////////////////////////
// CONTROLLER MONKEY FUNCTIONS
////////////////////////////////////////////////////////////////////////////////////

// GetRandomInput() loads pad_state with random monkey input

#ifdef CONTROLLER_MONKEY
void xb_joypad_device::GetRandomInput()
{
	int i, j;
	
	pad_state.Gamepad.wButtons = 0;
	
	int shifter = 1;
	for(i = 0; i < MONKEY_NUM_DBUTTONS; i++)
	{
		if(random() < dbutton_probability[i])
			pad_state.Gamepad.wButtons |= shifter ;
		shifter <<= 1;
	}
	
	for(i = 0; i < MONKEY_NUM_ABUTTONS; i++)
	{
		if(random() < abutton_probability[i])
			pad_state.Gamepad.bAnalogButtons[i] = random(0x20, 0xff);
		else
			pad_state.Gamepad.bAnalogButtons[i] = 0;
		
	}
	for(i = 0; i < MONKEY_NUM_STICKS; i++)
	{
		if(random() < stick_zero_probability[i])
		{
			stick_position[i][0] = 0;
			stick_position[i][1] = 0;
		}
		else if(random() < stick_move_probability[i])
		{
			int move[2];
			
			for(j = 0; j < 2; j++)
			{
				// calculate the move.  Don't let it go beyond the edges of possible values....
				move[j] = random(2 * stick_move_magnitude[i]) - stick_move_magnitude[i];
				if(stick_position[i][j] + move[j] >= 0x8000)
					stick_position[i][j] = 0x7fff;
				else if(stick_position[i][j] + move[j] < -0x8000)
					stick_position[i][j] = -0x8000;
				else
					stick_position[i][j] += move[j];
			}
		}
	}
	
	pad_state.Gamepad.sThumbRX = stick_position[0][0];
	pad_state.Gamepad.sThumbRY = stick_position[0][1];
	pad_state.Gamepad.sThumbLX = stick_position[1][0];
	pad_state.Gamepad.sThumbLY = stick_position[1][1];
}
#endif