#ifndef XB_INPUT_H
#define XB_INPUT_H

#include "inputmgr.h"

#include "xb_types.h"

/*-------------------------------------------------------------------------------------------------------

xb_input_device class.
  
The xb hwos is used as a placeholder for porting apps which use hwos to new platforms.
	
-------------------------------------------------------------------------------------------------------*/

#define MAX_XB_PADS 4
#define RDATA_SIZE 32
#define KEYB_SIZE 256

// the threshold for button presses on the analog buttons
// (mandated to a minimum of 0x20 by the xbox certification requirements)
#define BUTTON_THRESHOLD 0x20 
// the threshold for analog joystick dead zone
#define JOYSTICK_THRESHOLD 0x3000 // 37.5%

class demo_keyframe;

/*class xb_keyboard_device : public input_device
{
public:
	xb_keyboard_device(int which_keyb = 0);
	virtual ~xb_keyboard_device();

	virtual stringx get_name() const;
	virtual stringx get_name( int axis ) const;
	virtual device_id_t get_id() const;
  
	virtual int get_axis_count() const;
	virtual axis_id_t get_axis_id(int axis) const;
	virtual rational_t get_axis_state( axis_id_t axis, int control_axis ) const;
	virtual rational_t get_axis_old_state( axis_id_t axis, int control_axis ) const;
	virtual rational_t get_axis_delta( axis_id_t axis, int control_axis ) const;
	
	virtual void poll();
	  
	virtual void vibrate( rational_t intensity ) {};
	virtual void vibrate( int vibrator_flag, int vibrator_power, int vibrator_freq, int vibrator_inc ) {};
	virtual void stop_vibration() {};
	virtual bool is_vibrator_present() const{ return false;} ;
		
	private:
	int keyb_id;
	USBKBDATA_t kdata;
};
*/

class xb_joypad_device : public input_device
{
public:
	xb_joypad_device(int which_port = 0, int which_slot = 0);
	virtual ~xb_joypad_device();
	
	virtual stringx get_name() const;
	virtual stringx get_name( int axis ) const;
	virtual device_id_t get_id() const;
	
	virtual int get_axis_count() const;
	virtual axis_id_t get_axis_id(int axis) const;
	virtual rational_t get_axis_state( axis_id_t axis, int control_axis ) const;
	virtual rational_t get_axis_old_state( axis_id_t axis, int control_axis ) const;
	virtual rational_t get_axis_delta( axis_id_t axis, int control_axis ) const;
	
	virtual void vibrate( float intensity );
	virtual void vibrate( int vibrator_flag, int vibrator_power, int vibrator_freq, int vibrator_inc );
	virtual void stop_vibration();
	virtual bool is_vibrator_present() const;
	
	virtual void poll();
	
	bool is_inserted() { return pad_type; }
	
	void record_demo_start(const stringx &filename);
	void record_demo_stop();
	void playback_demo_start(const stringx &filename);
	void playback_demo_stop();
	
	bool is_port_open() const { return(port_opened); }
	virtual bool is_connected() const { return(disconnected == 0); }
	
	void reconnect( void );
	void disconnect( void );
	
	// For manually manipulating the button states.  Used by the Super Monkey.
	virtual void set_button_d(int button_num, bool state);
	virtual void set_button_a(int button_num, int state) { pad_state.Gamepad.bAnalogButtons[button_num] = state; }
	virtual void set_stick(int stick_num, int new_x, int new_y);
	virtual void clear_state() { memset(&pad_state, 0, sizeof(XINPUT_STATE)); }

private:
	XINPUT_STATE pad_state;
	XINPUT_STATE old_pad_state;  
	int Motor0Timer; 
	
	int pad_id;
	int term_id;
	int g_error_count;
	int phase;
	int state;
	
	char port_id;
	HANDLE hDevice;
	char slot_id;
	char pad_type;
	
	// Some data for the rumble.  This memory has to stay valid until the rumble calls are done.
	XINPUT_FEEDBACK rumble_data;

	bool port_opened;
	
	int disconnected;
	int was_disconnected;

	WORD rumble_resolution_left;
	WORD rumble_resolution_right;
	
	// demo mode
	bool playing_demo;
	bool recording_demo;
	unsigned int keyframe_count;
	unsigned int curr_keyframe;
	demo_keyframe *demo_data;
	unsigned int frames_since_change; // for demos
	os_file demo_log;
	
	static BYTE button_threshold(const BYTE button_val) 
	{ 
		if(button_val >= BUTTON_THRESHOLD) 
			return button_val;
		else return 0;
	}
	static int16 joystick_threshold(const int16 joystick_val) 
	{ 
		if(abs(joystick_val) >= JOYSTICK_THRESHOLD)
			return joystick_val;
		else return 0;
	}
	rational_t get_axis_state( axis_id_t axis ) const;
	rational_t xb_joypad_device::get_axis_state( axis_id_t axis, const XINPUT_GAMEPAD &gp ) const;
	void xb_joypad_device::calc_rumble_intensity(float intensity);
	
public:
	
#ifdef CONTROLLER_MONKEY
	// For the controller monkey.
	virtual void GetRandomInput();  // loads pad_state with random monkey input
#endif
	
	friend class xbox_input_mgr;
};

class demo_keyframe
{
public:
    demo_keyframe() : countdown(0) {}
    ~demo_keyframe(){}
	
private:
    unsigned char rdata[RDATA_SIZE];
    unsigned int countdown;
	
	friend xb_joypad_device;
};

/*-------------------------------------------------------------------------------------------------------

xb_input_mgr class 
  
This class contains and provides the singleton interface for the global DirectInput object.
	
-------------------------------------------------------------------------------------------------------*/
class xbox_input_mgr : public singleton
{
	DECLARE_SINGLETON(xbox_input_mgr)
		xbox_input_mgr();
	~xbox_input_mgr();
	
	static void poll( void );
	
private:
	void sort_pads( void );
	
	static int get_first_free_portid( void );
	
	static xb_joypad_device *pad[MAX_XB_PADS];
	int max_keyb;
	friend class input_mgr;
};

#endif // XB_INPUT_H
