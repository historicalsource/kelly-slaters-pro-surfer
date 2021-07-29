#ifndef PS2_INPUT_H
#define PS2_INPUT_H

#include "inputmgr.h"

#include "ps2_types.h"

#include <eetypes.h>
#include <libpad.h>
#include <libusbkb.h>

/*-------------------------------------------------------------------------------------------------------

  ps2_input_device class.
  
	The ps2 hwos is used as a placeholder for porting apps which use hwos to new platforms.
	
-------------------------------------------------------------------------------------------------------*/

#define SUPPORT_MULTITAP 0

#define PS2_MAX_PORTS 2

#if SUPPORT_MULTITAP
#define PS2_MAX_SLOTS 4
#else
#define PS2_MAX_SLOTS 1
#endif

#define MAX_PS2_PADS (PS2_MAX_PORTS*PS2_MAX_SLOTS)
#define PORT_SLOT_TO_CONTROL_NUM(port, slot) (((port)*PS2_MAX_SLOTS)+(slot))

#define RDATA_SIZE 32

#define KEYB_SIZE 256

class demo_keyframe;

/*class ps2_keyboard_device : public input_device
{
public:
ps2_keyboard_device(int which_keyb = 0);
virtual ~ps2_keyboard_device();

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

// I'm kind of guessing on these values....
#define PS2_NUM_ABUTTONS 4
#define PS2_NUM_DBUTTONS 16
#define PS2_NUM_STICKS 2

class ps2_joypad_device : public input_device
{
public:
	ps2_joypad_device(int which_port = 0, int which_slot = 0);
	virtual ~ps2_joypad_device();
	
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
	
	// For manually manipulating the button states.  Used by the Super Monkey.
	virtual void set_button_d(int button_num, bool state);
	virtual void set_button_a(int button_num, int state);
	virtual void set_stick(int stick_num, int new_x, int new_y);
	virtual void clear_state();

#ifdef CONTROLLER_MONKEY
	// For the controller monkey.
	virtual void GetRandomInput();
#endif

private:
	
	int pad_id;
	int term_id;
	int g_error_count;
	int phase;
	int state;
	
	
	unsigned char rdata1[RDATA_SIZE];
	unsigned char rdata2[RDATA_SIZE];
	unsigned char *curr_rdata;
	unsigned char *prev_rdata;
	static unsigned char rdata[32] PACKING(64);
	
	char port_id;
	char slot_id;
	char pad_type;
	
	bool port_opened;
	
	int disconnected;
	int was_disconnected;
	
	// demo mode
	bool playing_demo;
	bool recording_demo;
	unsigned int keyframe_count;
	unsigned int curr_keyframe;
	demo_keyframe *demo_data;
	unsigned int frames_since_change; // for demos
	os_file demo_log;
	
	
	rational_t get_axis_state( axis_id_t axis, unsigned char *rdata ) const;
};

class demo_keyframe
{
public:
    demo_keyframe() : countdown(0) {}
    ~demo_keyframe(){}
	
private:
    unsigned char rdata[RDATA_SIZE];
    unsigned int countdown;
	
	friend ps2_joypad_device;
};

/*-------------------------------------------------------------------------------------------------------

  ps2_input_mgr class 
  
	This class contains and provides the singleton interface for the global DirectInput object.
	
-------------------------------------------------------------------------------------------------------*/
class ps2_input_mgr : public singleton
{
	DECLARE_SINGLETON(ps2_input_mgr)
		ps2_input_mgr();
	~ps2_input_mgr();
	
private:
    ps2_joypad_device *pad[MAX_PS2_PADS];
    int max_keyb;
	friend class input_mgr;
};

#define	PS2_JOYPAD_DIGITAL	  (char)0x40
#define PS2_JOYPAD_ANALOG     (char)0x50
#define PS2_JOYPAD_DUALSHOCK  (char)0x73
#define PS2_JOYPAD_DUALSHOCK2 (char)0x79

#endif // PS2_INPUT_H
