/*-------------------------------------------------------------------------------------------------------

  PS2_INPUT.CPP - DirectInput implementation.
  
-------------------------------------------------------------------------------------------------------*/

#include "global.h"

#include "ps2_input.h" 
#include "ps2_file.h" 
#include "joystick.h"
#include "keyboard.h"
#include "random.h"

#include <eekernel.h>
#include <sifrpc.h>
#include <sifdev.h>

#define JOY_STICK_DEADZONE  50.0f

#if !defined(BUILD_BOOTABLE)
//#define ENABLE_USB_KEYB
#endif

static u_long128 dma_buf1[scePadDmaBufferMax] PACKING(64);
static u_long128 dma_buf2[scePadDmaBufferMax] PACKING(64);

#if SUPPORT_MULTITAP
static u_long128 dma_buf3[scePadDmaBufferMax] PACKING(64);
static u_long128 dma_buf4[scePadDmaBufferMax] PACKING(64);
static u_long128 dma_buf5[scePadDmaBufferMax] PACKING(64);
static u_long128 dma_buf6[scePadDmaBufferMax] PACKING(64);
static u_long128 dma_buf7[scePadDmaBufferMax] PACKING(64);
static u_long128 dma_buf8[scePadDmaBufferMax] PACKING(64);
#endif

static u_long128 *dma_buf[MAX_PS2_PADS] = 
{ 
	(u_long128 *)&dma_buf1,
		(u_long128 *)&dma_buf2
#if SUPPORT_MULTITAP
		,
		(u_long128 *)&dma_buf3,
		(u_long128 *)&dma_buf4,
		(u_long128 *)&dma_buf5,
		(u_long128 *)&dma_buf6,
		(u_long128 *)&dma_buf7,
		(u_long128 *)&dma_buf8
#endif
};

/*
static u_long128 dma_buf1[scePadDmaBufferMax] PACKING(64);
static u_long128 dma_buf2[scePadDmaBufferMax] PACKING(64);
*/

#define NUM_KEYS 0x64

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

/*ps2_keyboard_device::ps2_keyboard_device( int which_keyb )
{
	keyb_id = which_keyb;
	sceUsbKbSetArrangement( keyb_id, 0 );
	sceUsbKbSetRepeat( keyb_id , 30, 2 );
	
	device_id = KEYBOARD_DEVICE;
}

ps2_keyboard_device::~ps2_keyboard_device()
{
}

stringx ps2_keyboard_device::get_name() const
{
	return stringx( "ps2 usb keyboard" );
}

stringx ps2_keyboard_device::get_name(int axis) const
{
	return stringx( "a key" );
}

device_id_t ps2_keyboard_device::get_id() const
{
		  return device_id;
}


int ps2_keyboard_device::get_axis_count() const
{
	//debug_print( "STUB: get_axis_count()" );
	return 0;
}

axis_id_t ps2_keyboard_device::get_axis_id(int axis) const
{
				//debug_print( "STUB: get_axis_id(int axis)" );
				return axis;
}

rational_t ps2_keyboard_device::get_axis_state( axis_id_t axis, int control_axis ) const
{
	//debug_print( "STUB: get_axis_state()" );
	return 0.0f;
}

rational_t ps2_keyboard_device::get_axis_old_state( axis_id_t axis, int control_axis ) const
{
	//debug_print( "STUB: get_axis_old_state()" );
	return 0.0f;
}

rational_t ps2_keyboard_device::get_axis_delta( axis_id_t axis, int control_axis ) const
{
	//debug_print( "STUB: get_axis_delta()" );
	return 0.0f;
}

void ps2_keyboard_device::poll()
{
	int ret;
	USBKBINFO_t info;
	
	ret = sceUsbKbGetInfo(&info);
	if( ret != USBKB_OK )
	{
						  debug_print( "sceUsbKbGetInfo failed" );
						  return;
	}
	
	sceUsbKbSync( USBKB_WAIT, &ret );
	
	if( ret == USBKB_OK )
	{
		if( info.status[keyb_id] == 0 ) return;
	}
	
	ret = sceUsbKbRead( keyb_id, &kdata );
	
	if( ret != USBKB_OK )
	{
		debug_print( "sceUsbKbRead failed" );
		return;
	}
	
	sceUsbKbSync( USBKB_WAIT, &ret );
	
	if( ret != USBKB_OK )
	{
		debug_print( "sceUsbKbWait failed" );
		return;
	}
	
	if( kdata.len == 0 ) return;
	
	for( int i = 0; i < kdata.len; i++ )
	{
		int kcode = kdata.keycode[i];
		
		debug_print( "0x%X", kcode );
		if( kcode & USBKB_RAWDAT )
		{
			if( ( kcode&~USBKB_RAWDAT ) < NUM_KEYS )
				KB_post_event_callback(kePress, USBKB_to_KB[kcode&~USBKB_RAWDAT]);
			debug_print( "Arch: %d", USBKB_to_KB[kcode&~USBKB_RAWDAT]);
		}
		else
		{
			kcode &= 0x00ff;
			
			switch( kcode )
			{
			case 0x0a:
				KB_post_event_callback(kePress, KB_RETURN);
				break;
				
												case 0x08:
													KB_post_event_callback(kePress, KB_BACKSPACE);
													break;
													
												case 0x09:
													KB_post_event_callback(kePress, KB_TAB);
													break;
													
												case 0x60:
													KB_post_event_callback(kePress, KB_TILDE);
													break;
													
												case 0x7e:
													KB_post_char_callback( 0x5f );
													break;
													
												default:
													if( kcode <= 127 && kcode > 0 )
														KB_post_char_callback( (char)(kcode&0x00ff) );
			}
		}
	}
}
*/

DEFINE_SINGLETON(ps2_input_mgr)

ps2_joypad_device *g_pad = NULL;

/*-------------------------------------------------------------------------------------------------------

  ps2_joypad_device class.
  
	The ps2 hwos is used as a placeholder for porting apps which use hwos to new platforms.
	
-------------------------------------------------------------------------------------------------------*/


ps2_joypad_device::ps2_joypad_device(int which_port, int which_slot)
{
	port_id = which_port;
	slot_id = which_slot;
	
	assert(port_id >= 0 && port_id < 2);
	assert(slot_id >= 0 && slot_id < 4);
	
	curr_rdata = rdata1;
	prev_rdata = rdata2;
	memset(rdata1, 0, 32);
	rdata1[2] = 0xff;
	rdata1[3] = 0xff;
	memset(rdata2, 0, 32);
	rdata2[2] = 0xff;
	rdata2[3] = 0xff;
	pad_type = -1;
	
	// demo mode stuff
	frames_since_change = 0;
	keyframe_count = 0;
	curr_keyframe = 0;
	demo_data = NULL;
	playing_demo = false;
	recording_demo = false;
	
	/* Open port */
	int ret = scePadPortOpen( port_id, slot_id, dma_buf[PORT_SLOT_TO_CONTROL_NUM(port_id,slot_id)]);
	if (ret==0)
		port_opened = false;
	else
		port_opened = true;
	
	//  device_id = JOYSTICK_TO_DEVICE_ID(PORT_SLOT_TO_CONTROL_NUM(port_id,slot_id)+1);
	
	disconnected = 0;
	was_disconnected = 0;
	pad_id = 0;
	term_id = 0;
	g_error_count = 0;
	phase = 0;
	state = 0;
	
	poll();
}

ps2_joypad_device::~ps2_joypad_device()
{
	if (demo_data != NULL)
		delete []demo_data;
}

stringx ps2_joypad_device::get_name() const
{
	return stringx( "ps2 input device" );
}

stringx ps2_joypad_device::get_name( int axis ) const
{
	return stringx( "ps2 input device" );
}

device_id_t ps2_joypad_device::get_id() const
{
	return device_id;
}

void ps2_joypad_device::vibrate( float intensity )
{
	if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_RUMBLE))
	{
		
		if (pad_type != PS2_JOYPAD_DUALSHOCK2)
			return;
		unsigned char motors[] = {1, (unsigned char)(intensity*0xff), 0, 0, 0, 0};
		scePadSetActDirect(port_id, 0, motors);
	}
}
void ps2_joypad_device::vibrate( int vibrator_flag, int vibrator_power, int vibrator_freq, int vibrator_inc )
{
	if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_RUMBLE))
	{
		
		if (vibrator_freq == 0)
		{
			unsigned char motors[] = {0, (unsigned char)(vibrator_power), 0, 0, 0, 0};
			scePadSetActDirect(port_id, 0, motors);
		} 
		else if (vibrator_freq == 1)
		{
			unsigned char motors[] = {1, 0, 0, 0, 0, 0};
			scePadSetActDirect(port_id, 0, motors);
		} 
		else if (vibrator_freq == 2)
		{
			unsigned char motors[] = {1, (unsigned char)(vibrator_power), 0, 0, 0, 0};
			scePadSetActDirect(port_id, 0, motors);
		}
	}
	
}
void ps2_joypad_device::stop_vibration()
{
	if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_RUMBLE))
	{
		
		if (pad_type != PS2_JOYPAD_DUALSHOCK2)
			return;
		unsigned char motors[] = {0, 0, 0, 0, 0, 0};
		scePadSetActDirect(port_id, 0, motors);
	}
}
bool ps2_joypad_device::is_vibrator_present() const
{
	if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_RUMBLE))
	{
		return (pad_type == PS2_JOYPAD_DUALSHOCK2);
	}
	else 
	{
		return false;
	}
}

int ps2_joypad_device::get_axis_count() const
{
	return JOY_PS2_NUM_AXES;
}

axis_id_t ps2_joypad_device::get_axis_id(int axis) const 
{
	return axis;
}


rational_t ps2_joypad_device::get_axis_old_state( axis_id_t axis, int control_axis ) const
{
	if( axis == JOY_PS2_DISCONNECT )
	{
		if( was_disconnected == 1 )
			return 1.0f;
		return 0.0f;
	}
	else
	{
		return get_axis_state(axis, prev_rdata);
	}
}

rational_t ps2_joypad_device::get_axis_delta( axis_id_t axis, int control_axis ) const
{
	if( axis == JOY_PS2_DISCONNECT )
	{
		return (rational_t)(disconnected - was_disconnected);
	}
	else
	{
		return get_axis_state(axis, curr_rdata) - get_axis_state(axis, prev_rdata);
	}
}

rational_t ps2_joypad_device::get_axis_state( axis_id_t axis, int control_axis ) const
{
	if( axis == JOY_PS2_DISCONNECT )
	{
		if( disconnected == 1 )
			return 1.0f;
		return 0.0f;
	}
	else
	{
		return get_axis_state(axis, curr_rdata);
	}
}

// rdata[2] defines	
#define PS2_BTN_SELECT (unsigned char)0x01
#define PS2_BTN_L3     (unsigned char)0x02
#define PS2_BTN_R3     (unsigned char)0x04
#define PS2_BTN_START  (unsigned char)0x08
#define PS2_BTN_UP     (unsigned char)0x10
#define PS2_BTN_RIGHT  (unsigned char)0x20
#define PS2_BTN_DOWN   (unsigned char)0x40
#define PS2_BTN_LEFT   (unsigned char)0x80

// rdata[3] defines
#define PS2_BTN_L2     (unsigned char)0x01
#define PS2_BTN_R2     (unsigned char)0x02
#define PS2_BTN_L1     (unsigned char)0x04
#define PS2_BTN_R1     (unsigned char)0x08
#define PS2_BTN_TR     (unsigned char)0x10
#define PS2_BTN_O      (unsigned char)0x20
#define PS2_BTN_X      (unsigned char)0x40
#define PS2_BTN_SQ     (unsigned char)0x80

rational_t ps2_joypad_device::get_axis_state( axis_id_t axis, unsigned char *rdata ) const
{
	rational_t dpad, astick;
	
	switch (axis) 
	{
    case JOY_PS2_X: // left analog horiz
		dpad = get_axis_state((axis_id_t)JOY_PS2_DX, rdata);
		astick = get_axis_state((axis_id_t)JOY_PS2_LX, rdata);
		if (__fabs(astick) > __fabs(dpad))
			return astick;
		return dpad;
		
    case JOY_PS2_Y: // left analog vert
		dpad = get_axis_state((axis_id_t)JOY_PS2_DY, rdata);
		astick = get_axis_state((axis_id_t)JOY_PS2_LY, rdata);
		if (__fabs(astick) > __fabs(dpad))
			return astick;
		return dpad;
		
    case JOY_PS2_DX: // just Dpad
		if ((rdata[2] & PS2_BTN_LEFT) == 0)
		{
		/*        if (pad_type == PS2_JOYPAD_DUALSHOCK2)
		return ((rational_t)rdata[9] - 127.0f)/127.0f;
        else 
			*/
			return -1.0f;
		}
		else if ((rdata[2] & PS2_BTN_RIGHT) == 0)
		{
		/*        if (pad_type == PS2_JOYPAD_DUALSHOCK2) 
		return ((rational_t)rdata[8] - 127.0f)/127.0f;
			else */
			return 1.0f;
		}
		return 0.0f;
		
    case JOY_PS2_DY: // just Dpad
		if ((rdata[2] & PS2_BTN_DOWN) == 0)
		{/*
		 if (pad_type == PS2_JOYPAD_DUALSHOCK2)
		 return ((rational_t)rdata[11] - 127.0f)/127.0f;
			else */
			return 1.0f;
		}
		else if ((rdata[2] & PS2_BTN_UP) == 0)
		{/*
		 if (pad_type == PS2_JOYPAD_DUALSHOCK2) 
		 return ((rational_t)rdata[10] - 127.0f)/127.0f;
			else */
			return -1.0f;
		}
		return 0.0f;
		
    case JOY_PS2_LX: // just Lstick
		if (pad_type == PS2_JOYPAD_DUALSHOCK || pad_type == PS2_JOYPAD_DUALSHOCK2)
		{
			float tmp = rdata[6] - 127.0f;
			if (tmp < -JOY_STICK_DEADZONE || tmp > JOY_STICK_DEADZONE)
				return ((rational_t)tmp)/127.0f;  
			else
				return 0.0f;
		}
		else
			return 0.0f;
		
    case JOY_PS2_LY: // just Lstick
		if (pad_type == PS2_JOYPAD_DUALSHOCK || pad_type == PS2_JOYPAD_DUALSHOCK2)
		{
			float tmp = rdata[7] - 127.0f;
			if (tmp < -JOY_STICK_DEADZONE || tmp > JOY_STICK_DEADZONE)
				return ((rational_t)tmp)/127.0f;  
			else
				return 0.0f;
		}
		else
			return 0.0f;
		
    case JOY_PS2_BTNL3: // L3
		if (pad_type == PS2_JOYPAD_DUALSHOCK || pad_type == PS2_JOYPAD_DUALSHOCK2) 
		{
			if ((rdata[2] & PS2_BTN_L3) == 0)
				return 1.0f;
		}
		return 0.0f;
		
    case JOY_PS2_RX: // right analog horiz
		if (pad_type == PS2_JOYPAD_DUALSHOCK || pad_type == PS2_JOYPAD_DUALSHOCK2)
			return ((rational_t)rdata[4] - 127.0f)/127.0f;
		else
			return 0.0f;
		
    case JOY_PS2_RY: // right analog vert
		if (pad_type == PS2_JOYPAD_DUALSHOCK || pad_type == PS2_JOYPAD_DUALSHOCK2)
			return ((rational_t)rdata[5] - 127.0f)/127.0f;
		else
			return 0.0f;
		
    case JOY_PS2_BTNR3: // R3
		if (pad_type == PS2_JOYPAD_DUALSHOCK || pad_type == PS2_JOYPAD_DUALSHOCK2)
		{
			if ((rdata[2] & PS2_BTN_R3) == 0)
				return 1.0f;
		}
		return 0.0f;
		
    case JOY_PS2_BTNX:  // A (ps2 x)
						/*if (pad_type == PS2_JOYPAD_DUALSHOCK2)
						{
						return (float)rdata[14] / 255.0f;
						}
						else 
		{*/
        if ((rdata[3] & PS2_BTN_X) == 0)
			return 1.0f;
		//      }
		return 0.0f;
		
    case JOY_PS2_BTNO:  // B (ps2 o)
						/*      if (pad_type == PS2_JOYPAD_DUALSHOCK2)
						{
						return (float)rdata[13] / 255.0f;
						}
						else 
		{*/
        if ((rdata[3] & PS2_BTN_O) == 0)
			return 1.0f;
		//    }
		return 0.0f;
		
    case JOY_PS2_BTNR1: // C (ps2 R1)
						/*      if (pad_type == PS2_JOYPAD_DUALSHOCK2)
						{
						return (float)rdata[17] / 255.0f;
						}
						else 
		{*/
        if ((rdata[3] & PS2_BTN_R1) == 0)
			return 1.0f;
		//      }
		return 0.0f;
		
    case JOY_PS2_BTNSQ: // X (ps2 sq)
						/*      if (pad_type == PS2_JOYPAD_DUALSHOCK2)
						{
						return (float)rdata[15] / 255.0f;
						}
						else 
		{*/
        if ((rdata[3] & PS2_BTN_SQ) == 0)
			return 1.0f;
		//      }
		return 0.0f;
		
    case JOY_PS2_BTNTR: // Y (ps2 tr)
						/*     if (pad_type == PS2_JOYPAD_DUALSHOCK2)
						{
						return (float)rdata[12] / 255.0f;
						}
						else 
		{*/
        if ((rdata[3] & PS2_BTN_TR) == 0)
			return 1.0f;
		//      }
		return 0.0f;
		
    case JOY_PS2_BTNL2: // L (ps2 L2)
						/*      if (pad_type == PS2_JOYPAD_DUALSHOCK2)
						{
						return (float)rdata[18] / 255.0f;
						}
						else 
		{*/
        if ((rdata[3] & PS2_BTN_L2) == 0)
			return 1.0f;
		//      }
		return 0.0f;
		
    case JOY_PS2_BTNL1: // Z (ps2 L1)
						/*      if (pad_type == PS2_JOYPAD_DUALSHOCK2)
						{
						return (float)rdata[16] / 255.0f;
						}
						else 
		{*/
        if ((rdata[3] & PS2_BTN_L1) == 0)
			return 1.0f;
		//      }
		return 0.0f;
		
    case JOY_PS2_BTNR2: // R (ps2 R2)
						/*      if (pad_type == PS2_JOYPAD_DUALSHOCK2)
						{
						return (float)rdata[19] / 255.0f;
						}
						else 
		{*/
        if ((rdata[3] & PS2_BTN_R2) == 0)
			return 1.0f;
		//      }
		return 0.0f;
		
    case JOY_PS2_START: // Start
		if ((rdata[2] & PS2_BTN_START) == 0)
			return 1.0f;
		return 0.0f;
		
    case JOY_PS2_SELECT:
		if ((rdata[2] & PS2_BTN_SELECT) == 0)
			return 1.0f;
		return 0.0f;
		
    default:
		assert("ps2_joypad_device::get_axis_state illegal axis queried." == 0);
		return 0.0f;
  }
}

void ps2_joypad_device::record_demo_start(const stringx &filename)
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

void ps2_joypad_device::record_demo_stop()
{
	if (demo_log.is_open())
		demo_log.close();
	recording_demo = false;
}

void ps2_joypad_device::playback_demo_start(const stringx &filename)
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

void ps2_joypad_device::playback_demo_stop()
{
	playing_demo = false;
	if (demo_data != NULL)
		delete []demo_data;
}

unsigned char ps2_joypad_device::rdata[32] PACKING(64);
void ps2_joypad_device::poll()
{
	int exid;
	//	int frame;
	//	int port;
	//	int ret;
	//	int pad = 0;
	
	// switch buffers
	if (playing_demo)
	{
		if (demo_data[curr_keyframe].countdown > 0)
			--demo_data[curr_keyframe].countdown;
		else
		{
			++curr_keyframe;
			if (curr_keyframe >= keyframe_count)
			{
				curr_keyframe = keyframe_count - 1;
				playing_demo = false;
			}
		}
		prev_rdata = curr_rdata;
		curr_rdata = (unsigned char *)demo_data[curr_keyframe].rdata;
	}
	
	/* Check controller status */
	state = scePadGetState(port_id,slot_id);
	/*
	if (state>=0 && state<=7)
	sprintf(mes_state,"%s", StateMessage[state] );
	else
	sprintf(mes_state,"%d", state);
	*/
	if (state==scePadStateDiscon)
	{
		g_error_count = 0;
		phase = 0;
		was_disconnected = disconnected;
		disconnected = 1;
	}
	else
	{
		was_disconnected = disconnected;    
		disconnected = 0;
	}
	
	/* Get controller information, actuator settings */
	switch(phase)
	{
	case 0:
		//			strcpy(mes_term, "");
		
		/* Is controller connected? */
		if (state != scePadStateStable &&
			state != scePadStateFindCTP1 )
			break;
		
		/* Get controller ID */
		pad_id = scePadInfoMode(port_id, slot_id, InfoModeCurID, 0 );
		if (pad_id==0) break;
		
		/* Get extended ID */
		exid = scePadInfoMode(port_id, slot_id, InfoModeCurExID,0);
		if (exid>0) pad_id = exid;
		
		switch(pad_id)
		{
			/* STANDARD */
		case 4:
			//				sprintf(mes_term, "%03x", pad_id);
			phase = 40;
			break;
			
			/* ANALOG */
		case 7:
			//				sprintf(mes_term, "%03x", pad_id);
			phase = 70;
			break;
			
		default:
			phase = 99;
			break;
		}
		//			sprintf(mes_term, "%03x", pad_id);
		break;
		
		/* Check to see if analog controller */
		case 40:
			if (scePadInfoMode(port_id, slot_id, InfoModeIdTable, -1)==0)
			{
				phase = 99;
				break;
			}
			phase++;
			
			/* Switch to analog mode */
		case 41:
			if (scePadSetMainMode(port_id, slot_id, 1, 0)==1)
			{
				phase++;
			}
			break;
			
		case 42:
			if (scePadGetReqState(port_id, slot_id)==scePadReqStateFaild)
			{
				phase--;
			}
			if (scePadGetReqState(port_id, slot_id)==scePadReqStateComplete)
			{
				phase = 0;
			}
			break;
			
			/* Check to see if DUALSHOCK2 */
		case 70:
			if (scePadInfoPressMode(port_id,slot_id)==1)
			{
				phase = 76;
				break;
			}
			phase = 99;
			break;
			
			/* Switch to pressure-sensitive mode */
		case 76:
			if (scePadEnterPressMode(port_id, slot_id)==1)
			{
				phase++;
			}
			break;
			
		case 77:
			if (scePadGetReqState(port_id, slot_id)==scePadReqStateFaild)
			{
				phase--;
			}
			if (scePadGetReqState(port_id, slot_id)==scePadReqStateComplete)
			{
				phase = 99;
			}
			break;
			
		default:
			if (state==scePadStateError) g_error_count++;
			if (state==scePadStateStable || state==scePadStateFindCTP1)
			{
				scePadRead(port_id, 0, rdata);
#ifdef CONTROLLER_MONKEY
				if (randomized && !get_axis_state(JOY_PS2_SELECT, rdata))
					g_super_monkey.SetControllerInput();
#endif
				if (playing_demo == false)
				{
					if (curr_rdata == rdata1) 
					{
						curr_rdata = rdata2;
						prev_rdata = rdata1;
					}
					else 
					{
						curr_rdata = rdata1;
						prev_rdata = rdata2;
					}
					
					memcpy (curr_rdata, rdata, sizeof(rdata));
				}
				// determine which type of pad we have
				pad_type = rdata[1];
				term_id = (rdata[1]>>4);
				
				if (term_id != pad_id)
				{
					phase = 0;
				}
			}
			else
			{
				pad_type = -1;
			}
			break;
	}
	
	if (recording_demo)
	{
		if (memcmp(curr_rdata, prev_rdata, RDATA_SIZE) == 0)
		{
			++frames_since_change;
		}
		else
		{
			// write out log
			demo_keyframe temp_keyframe;
			memcpy( &temp_keyframe.rdata, curr_rdata, RDATA_SIZE );
			temp_keyframe.countdown = frames_since_change;
			demo_log.write( &temp_keyframe, sizeof(demo_keyframe));
			frames_since_change = 0;
		}
	}
}

void ps2_joypad_device::set_button_d(int button_num, bool state)
{
	int rdata_index = 2;

	if(button_num > 7)
	{
		rdata_index = 3;
		button_num  -= 8;
	}

	rdata[rdata_index] &= ~(1 << button_num);
}
void ps2_joypad_device::set_button_a(int button_num, int state)
{
	// Pass through to the digital version, since we're ignoring analog input from PS2
	set_button_d(button_num, state > 127);
}
void ps2_joypad_device::set_stick(int stick_num, int new_x, int new_y)
{
	if(stick_num == 0)
	{
		rdata[6] = new_x;
		rdata[7] = new_y;
	}
	else
	{
		rdata[4] = new_x;
		rdata[5] = new_y;
	}
}

#ifdef CONTROLLER_MONKEY
void ps2_joypad_device::GetRandomInput()
{
	int i, j;
	
	// Clear out the controller data
	clear_state();
	
	int shifter = 1;
	for(i = 0; i < 8; i++)
	{
		if (i == 0) //  select button
		{
			shifter <<= 1;
			continue;
		}
		
		if(random() < dbutton_probability[i])
			rdata[2] &= ~shifter ;
		
		shifter <<= 1;
	}
	shifter=1;
	for(;i < MONKEY_NUM_DBUTTONS; i++)
	{
		if(random() < dbutton_probability[i])
			rdata[3] &= ~shifter ;
		
		shifter <<= 1;
	}
	
	for(i = 0; i < MONKEY_NUM_STICKS; i++)
	{
		if(random() < stick_zero_probability[i])
		{
			rdata[i + 4] = 0;
			rdata[i + 5] = 0;
		}
		else if(random() < stick_move_probability[i])
		{
			int move[2];
			
			for(j = 0; j < 2; j++)
			{
				// calculate the move.  Don't let it go beyond the edges of possible values....
				move[j] = random(2 * stick_move_magnitude[i]) - stick_move_magnitude[i];
				if(rdata[i + j + 4] + move[j] >= 255)
					rdata[i + j + 4] = 255;
				else if(rdata[i + j + 4] + move[j] <= 0)
					rdata[i + j + 4] = 0;
				else
					rdata[i + j + 4] += move[j];
			}
		}
	}
}
#endif //CONTROLLER_MONKEY

ps2_input_mgr::ps2_input_mgr()
{
	nglPrintf("ps2 input init\n");
	
	sceSifInitRpc(0);
	
	sceCdSync(0);
	
	stringx debug_dir(nglHostPrefix);
	debug_dir += "c:\\usr\\local\\sce\\iop\\modules\\";
	
	
	// $jim - I moved this to ps2_storage.cpp as I need it earlier than here for memcard init.
	
	if (load_iop_module("sio2man.irx", debug_dir.c_str()) == false)
	{
		nglPrintf("Can't load module sio2man\n");
		assert(0);
	}
	
	
	debug_dir = nglHostPrefix;
	debug_dir += "c:\\usr\\local\\sce\\iop\\modules\\";
	if (load_iop_module("padman.irx", debug_dir.c_str()) == false)
	{
		nglPrintf("Can't load module padman\n");
		assert(0);
	}
	
	// --- pad open ---
	scePadInit(0);
	
	g_pad = NULL;
	int joystick_num = 1;
	for(int _port=0; _port < PS2_MAX_PORTS; ++_port)
	{
		for(int _slot=0; _slot < PS2_MAX_SLOTS; ++_slot)
		{
			int control_num = PORT_SLOT_TO_CONTROL_NUM(_port, _slot);
			pad[control_num] = new ps2_joypad_device(_port, _slot);
			if(pad[control_num]->is_port_open() && pad[control_num]->is_connected())
			{
				pad[control_num]->device_id = JOYSTICK_TO_DEVICE_ID(joystick_num);
				++joystick_num; 
				if(g_pad == NULL)
					g_pad = pad[control_num];
			}
			else
				pad[control_num]->device_id = INVALID_DEVICE_ID;
		}
	}
	if(g_pad == NULL)
		g_pad = pad[0];
	
#ifdef ENABLE_USB_KEYB
	debug_dir = nglHostPrefix;
	debug_dir += "c:\\usr\\local\\sce\\iop\\modules\\";
	if (load_iop_module("usbd.irx", debug_dir.c_str()) == false)
	{
		nglPrintf("Can't load module usbd\n");
		Exit(0);
	}
	
	debug_dir = nglHostPrefix;
	debug_dir += "c:\\usr\\local\\sce\\iop\\modules\\";
	if (load_iop_module("usbkb.irx", debug_dir.c_str()) == false)
	{
		nglPrintf("Can't load module usbkb\n");
		Exit(0);
	}
	
	
	// sceUsbKbInit uses an untracked _malloc_r
	int ret = sceUsbKbInit( &max_keyb );
	if( ret == USBKB_NG )
	{
		debug_print( "Could not init usbkb" );
		max_keyb = 0;
	}
	else
		nglPrintf( "ps2 keyboard init\n" );
#endif //ENABLE_USB_KEYB
	
}

bool ps2_inserted_devices = false;
ps2_input_mgr::~ps2_input_mgr()
{
	for(int i=0; i<MAX_PS2_PADS; ++i)
	{
		if (pad[i])
			delete pad[i];
	}
	
	ps2_inserted_devices = false;
	
	/*  if (pad2)
    delete pad2;
	*/
}


void input_mgr::scan_devices()
{
	if(!ps2_inserted_devices)
	{
		ps2_inserted_devices = true;
		for(int i=0; i<MAX_PS2_PADS; ++i)
		{
			ps2_input_mgr::inst()->pad[i]->device_id = JOYSTICK_TO_DEVICE_ID(i+1);
			assert(IS_JOYSTICK_DEVICE(ps2_input_mgr::inst()->pad[i]->device_id));
			insert_device(ps2_input_mgr::inst()->pad[i]);
		}
		
		// scan for keyboard
		// we add a keyboard 0 without scanning, the poll method checks for the presence
		// of a keyboard each call. Seems to be the way sony recommends
		// of course the documentation is in japanese so its kind of hard
		// for me to be an expert on the subject
#ifdef ENABLE_USB_KEYB
		insert_device( new ps2_keyboard_device( 0 ) );
#endif
	}
	/*   Don't do controller autoswitch-thingy.
	g_pad = NULL;
	int joystick_num = 1;
	int disconnected_joystick_num = MAX_PS2_PADS;
	bool resort_pads = false;
	for(int _port=0; _port < PS2_MAX_PORTS; ++_port)
	{
    for(int _slot=0; _slot < PS2_MAX_SLOTS; ++_slot)
    {
	int control_num = PORT_SLOT_TO_CONTROL_NUM(_port, _slot);
	if(ps2_input_mgr::inst()->pad[control_num]->is_port_open() && ps2_input_mgr::inst()->pad[control_num]->is_connected())
	{
	device_id_t new_id = JOYSTICK_TO_DEVICE_ID(joystick_num);
	if(ps2_input_mgr::inst()->pad[control_num]->device_id != new_id)
	resort_pads = true;
	
	  ps2_input_mgr::inst()->pad[control_num]->device_id = new_id;
	  ++joystick_num; 
	  
        if(g_pad == NULL)
		g_pad = ps2_input_mgr::inst()->pad[control_num];
		}
		else
		{
        device_id_t new_id = JOYSTICK_TO_DEVICE_ID(disconnected_joystick_num);
        if(ps2_input_mgr::inst()->pad[control_num]->device_id != new_id)
		resort_pads = true;
		
		  ps2_input_mgr::inst()->pad[control_num]->device_id = new_id;
		  --disconnected_joystick_num;
		  }
		  }
		  }
		  
			if(resort_pads)
			reset_joystick_array();
			
			  if(g_pad == NULL)
			  g_pad = ps2_input_mgr::inst()->pad[0];
	*/
	
	/*
	static bool pad1_last = false;
	
	  debug_print( "scan_devices" );
	  //  ps2_input_mgr::inst()->pad1->poll();
	  //  ps2_input_mgr::inst()->pad2->poll();
	  
		if (ps2_input_mgr::inst()->pad1->is_inserted() != pad1_last) 
		{
		// change of controller state
		pad1_last = !pad1_last;
		if (pad1_last == true)
		{
		// insert pad
		insert_device(ps2_input_mgr::inst()->pad1);
		}
		else
		{
		// remove pad
		}
		}
		/ *
		if (ps2_input_mgr::inst()->pad2->is_inserted() != pad2_last) 
		{
		// change of controller state
		pad2_last = !pad2_last;
		if (pad2_last == true)
		{
		// insert pad
		insert_device(ps2_input_mgr::inst()->pad2);
		}
		else
		{
		// remove pad
		}
		}
		* /
	*/
}


void ps2_joypad_device::clear_state()
{
	memset(rdata, 0xFF, RDATA_SIZE); 
	rdata[1] =PS2_JOYPAD_DUALSHOCK2;
	rdata[6] =127;
	rdata[7] =127;
}