#ifndef GC_INPUT_H
#define GC_INPUT_H

#include "inputmgr.h"
#include "stringx.h"

#include <dolphin/pad.h>

#define   TIME_COMMAND_MASK   0x00ffffff
#define   MOTOR_COMMAND_MASK  0xff000000
#define   MOTOR_NOP           0
#define   MOTOR_START         0x01000000
#define   MOTOR_STOP          0x02000000
#define   MOTOR_STOP_HARD     0x03000000
#define   MOTOR_END           0xff000000

class pad_motor
{
	u32           controller_id;
	u32           *commands;
	u32           *current_command;
	u32           current_command_start_time;
	u32           motor_on;
	u32           new_commands;
	static  u32   default_commands[2];
	/////////////////////////////////////////
public:
	void    set_commands(u32 *n_command)  {commands= current_command=  n_command; new_commands= 1;}
	u32     get_command()                 { return (*current_command)&MOTOR_COMMAND_MASK;}
	u32     get_command_time()            { return (*current_command)&TIME_COMMAND_MASK;}
	u32     get_new_command();
	pad_motor(u32 n_controller_id, u32 *n_commands, u32 on);
	pad_motor(u32 n_controller_id);
	pad_motor(){ pad_motor(0); }
	~pad_motor(){};
	
	void 		set_port(u32 n_controller_id) {controller_id= n_controller_id;}
	void    turn_on();
	void    turn_off();
	void    work();
};

class gc_joypad_device : public input_device
{
public:
	gc_joypad_device( int which_port = 0 );
	virtual ~gc_joypad_device( );
	
	virtual stringx get_name( void ) const;
	virtual stringx get_name( int axis ) const;
	virtual device_id_t get_id( void ) const;
	
	virtual int get_axis_count( void ) const;
	virtual axis_id_t get_axis_id( int axis ) const;
	virtual rational_t get_axis_state( axis_id_t axis, int control_axis ) const;
	virtual rational_t get_axis_old_state( axis_id_t axis, int control_axis ) const;
	virtual rational_t get_axis_delta( axis_id_t axis, int control_axis ) const;
	
	virtual void vibrate( rational_t intensity );
	virtual void vibrate( int vibrator_flag, int vibrator_power, int vibrator_freq, int vibrator_inc );
	virtual void stop_vibration( void );
	virtual bool is_vibrator_present( void ) const;
	
	virtual void poll( void );
	virtual bool is_connected() const;
	bool is_inserted( void );
	
	void record_demo_start( const stringx &filename );
	void record_demo_stop( void );
	void playback_demo_start( const stringx &filename );
	void playback_demo_stop( void );
	
	// For manually manipulating the button states.  Used by the Super Monkey.
	virtual void set_button_d(int button_num, bool state);
	virtual void set_button_a(int button_num, int state);
	virtual void set_stick(int stick_num, int new_x, int new_y);
	virtual void clear_state() { memset(&pads[0], 0, sizeof(PADStatus)); }
	
private:
	// We share the PADStatus info because you can
	// only poll it for all controllers at the same time.
	// Same with the motor bits
	static u32 motor_bits;
	static PADStatus pads[PAD_MAX_CONTROLLERS];
	static PADStatus old_pads[PAD_MAX_CONTROLLERS];

	int port;
	pad_motor* motor;
	bool connected;
	
	rational_t get_axis_state( axis_id_t axis, bool old ) const;
	
public:

#ifdef CONTROLLER_MONKEY
	virtual void GetRandomInput();  // loads pads with random monkey input
#endif
};

class gc_input_mgr : public singleton
{
	DECLARE_SINGLETON( gc_input_mgr )
		
	gc_input_mgr( );
	~gc_input_mgr( );
	
private:
	gc_joypad_device* 				pads[PAD_MAX_CONTROLLERS];
	gc_joypad_device* 				get_pad( int i ) { return pads[i]; }
	
	friend class input_mgr;
};

#endif // GC_INPUT_H
