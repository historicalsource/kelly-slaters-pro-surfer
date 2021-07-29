#ifndef INPUTMGR_H
#define INPUTMGR_H
/*--------------------------------------------------------------------------------------------------------

INPUTMGR.H
  
Input manager class - keeps track of input devices in a configurable, device independent manner.
	
Operation:
1) The hwos?? library registers a set of input_device objects with the input_mgr singleton.
2) Then the application maps axes from these input devices onto game controls.
3) The application repeatedly calls poll_devices(), and calls get_control_state() to retrieve the
state of the mapped controls.
	
TODO: Implement input map reading.
		
TODO: If we want an interactive controller configuration screen, we will need to add enumeration
facilities to input_mgr and input_device.  We will also need to add some sort of duplicate mapping
prevention, and possibly some unregistration facilities.
		  
--------------------------------------------------------------------------------------------------------*/
#include "warnlvl.h"
#include "hwmath.h"
#include "singleton.h"
#include "timer.h"

/*#if defined(TARGET_PS2)
#include "HWOSPS2\ps2_timer.h"
#elif defined(TARGET_XBOX)
#include "hwosxb/xb_timer.h"
#elif defined(TARGET_GC)
#include "hwosgc/gc_timer.h"
#else
#error include timer header here
#endif 
*/

#include "commands.h"

#include <map>
#include <list>

// Use a map or a vector for device storage
// Any system that can have devices
// added / removed at will (i.e. consoles)
// should NOT use a map.
#if defined(TARGET_PC)
#define USE_STL_MAP_FOR_DEVICES 1
#else
#define USE_STL_MAP_FOR_DEVICES 0
#endif

#ifdef DEBUG
#define CONTROLLER_MONKEY	// Simulate random controller input (dc 03/08/02)
#endif

class rumble_manager;

// Basic type definitions
typedef int control_id_t;
typedef int axis_id_t;

// Invalid ID constants
const control_id_t  INVALID_CONTROL_ID  = -1;
const axis_id_t     INVALID_AXIS_ID     = -1;

enum device_id_t
{
	INVALID_DEVICE_ID = -1,
		
		// ORDER MATTERS!
		JOYSTICK1_DEVICE = 1,
		JOYSTICK2_DEVICE = 2,
		JOYSTICK3_DEVICE = 3,
		JOYSTICK4_DEVICE = 4,
		JOYSTICK5_DEVICE = 5,
		JOYSTICK6_DEVICE = 6,
		JOYSTICK7_DEVICE = 7,
		JOYSTICK8_DEVICE = 8,
		AI_JOYSTICK      = 9,
		KEYBOARD1_DEVICE = 10,
		
		MOUSE1_DEVICE = 11,
		
		ANY_LOCAL_JOYSTICK
};

// joystick 1 -> MAX_JOYSTICK_DEVICES
#define MAX_JOYSTICK_DEVICES 9 // Add one for the AI Joystick
#define DEVICE_ID_TO_JOYSTICK(id) (((id) - JOYSTICK1_DEVICE) + 1)
#define JOYSTICK_TO_DEVICE_ID(id) ((device_id_t)(((id) - 1) + JOYSTICK1_DEVICE))
#define IS_JOYSTICK_DEVICE(id) ((id) >= JOYSTICK1_DEVICE && (id) <= AI_JOYSTICK)
// zero based joystick index
#define DEVICE_ID_TO_JOYSTICK_INDEX(id) ((id) - JOYSTICK1_DEVICE)


// keyboard 1 -> MAX_KEYBOARD_DEVICES
#define MAX_KEYBOARD_DEVICES 1
#define DEVICE_ID_TO_KEYBOARD(id) (((id) - KEYBOARD1_DEVICE) + 1)
#define KEYBOARD_TO_DEVICE_ID(id) ((device_id_t)(((id) - 1) + KEYBOARD1_DEVICE))
#define IS_KEYBOARD_DEVICE(id) ((id) >= KEYBOARD1_DEVICE && (id) <= KEYBOARD1_DEVICE)
// zero based keyboard index
#define DEVICE_ID_TO_KEYBOARD_INDEX(id) ((id) - KEYBOARD1_DEVICE)


// mouse 1 -> MAX_MOUSE_DEVICES
#define MAX_MOUSE_DEVICES 1
#define DEVICE_ID_TO_MOUSE(id) (((id) - MOUSE1_DEVICE) + 1)
#define MOUSE_TO_DEVICE_ID(id) ((device_id_t)(((id) - 1) + MOUSE1_DEVICE))
#define IS_MOUSE_DEVICE(id) ((id) >= MOUSE1_DEVICE && (id) <= MOUSE1_DEVICE)
// zero based mouse index
#define DEVICE_ID_TO_MOUSE_INDEX(id) ((id) - MOUSE1_DEVICE)


// legacy support
#define JOYSTICK_DEVICE   JOYSTICK1_DEVICE
#define KEYBOARD_DEVICE   KEYBOARD1_DEVICE
#define MOUSE_DEVICE      MOUSE1_DEVICE

// Axis state constants
const rational_t    AXIS_MAX  = 1.0f;
const rational_t    AXIS_MID  = 0.0f;
const rational_t    AXIS_MIN  =-1.0f;

#if defined(CONTROLLER_MONKEY)
#ifdef TARGET_PS2
// I'm kind of guessing on these values....
#define MONKEY_NUM_ABUTTONS 4
#define MONKEY_NUM_DBUTTONS 16
#define MONKEY_NUM_STICKS 2

#define MONKEY_D_SELECT           0
#define MONKEY_D_LEFT_THUMB       1
#define MONKEY_D_RIGHT_THUMB      2
#define MONKEY_D_START            3
#define MONKEY_D_DPAD_UP          4
#define MONKEY_D_DPAD_RIGHT       5
#define MONKEY_D_DPAD_DOWN        6
#define MONKEY_D_DPAD_LEFT        7
#define MONKEY_D_L2               8
#define MONKEY_D_R2               9
#define MONKEY_D_L1               10
#define MONKEY_D_R1               11
#define MONKEY_D_TRIANGLE         12
#define MONKEY_D_CIRCLE           13
#define MONKEY_D_CROSS            14
#define MONKEY_D_SQUARE           15
#define MONKEY_D_BACK             MONKEY_D_SELECT

// since for PS2 we ignore analog button values, the PS2 set_button_a() should 
// pass through to the digital version.  As such, these constants are the same
// as the digital ones.

#define MONKEY_A_TRIANGLE         MONKEY_D_TRIANGLE
#define MONKEY_A_CIRCLE           MONKEY_D_CIRCLE
#define MONKEY_A_CROSS            MONKEY_D_CROSS
#define MONKEY_A_SQUARE           MONKEY_D_SQUARE

#elif defined(TARGET_XBOX)
#define MONKEY_NUM_ABUTTONS 8
#define MONKEY_NUM_DBUTTONS 8
#define MONKEY_NUM_STICKS 2

#define MONKEY_D_DPAD_UP          0
#define MONKEY_D_DPAD_DOWN        1
#define MONKEY_D_DPAD_LEFT        2
#define MONKEY_D_DPAD_RIGHT       3
#define MONKEY_D_START            4
#define MONKEY_D_BACK             5
#define MONKEY_D_LEFT_THUMB       6
#define MONKEY_D_RIGHT_THUMB      7
#define MONKEY_D_SELECT           MONKEY_D_BACK

#define MONKEY_A_A                0
#define MONKEY_A_B                1
#define MONKEY_A_X                2
#define MONKEY_A_Y                3
#define MONKEY_A_BLACK            4
#define MONKEY_A_WHITE            5
#define MONKEY_A_LEFT_TRIGGER     6
#define MONKEY_A_RIGHT_TRIGGER    7
#define MONKEY_A_CROSS            MONKEY_A_A
#define MONKEY_A_TRIANGLE         MONKEY_A_B

#elif defined(TARGET_GC)
// I've assumed in this that the digital and analog representations of A and B need
// to be in line with each other.  As such, whoever programs GetRandomInput() for
// the GC needs to make sure that they load corresponding random values into the
// digital and analog data for each button.
//   To jive with this theory, I've set these constants with the assumption that A
// and B are analog for the purposes of setting their randomness probabilities.
#define MONKEY_NUM_ABUTTONS 14
#define MONKEY_NUM_DBUTTONS 14
#define MONKEY_NUM_STICKS 2

#define MONKEY_D_DPAD_LEFT        0
#define MONKEY_D_DPAD_RIGHT       1
#define MONKEY_D_DPAD_DOWN        2
#define MONKEY_D_DPAD_UP          3
#define MONKEY_D_Z                4
#define MONKEY_D_RIGHT_TRIGGER    5
#define MONKEY_D_LEFT_TRIGGER     6
#define MONKEY_D_A                8
#define MONKEY_D_B                9
#define MONKEY_D_X                10
#define MONKEY_D_Y                11
#define MONKEY_D_START            12
#define MONKEY_D_SELECT           13
#define MONKEY_D_BACK							MONKEY_D_SELECT

#define MONKEY_A_DPAD_LEFT        0
#define MONKEY_A_DPAD_RIGHT       1
#define MONKEY_A_DPAD_DOWN        2
#define MONKEY_A_DPAD_UP          3
#define MONKEY_A_Z                4
#define MONKEY_A_RIGHT_TRIGGER    5
#define MONKEY_A_LEFT_TRIGGER     6
#define MONKEY_A_A                8
#define MONKEY_A_B                9
#define MONKEY_A_X                10
#define MONKEY_A_Y                11
#define MONKEY_A_START            12
#define MONKEY_A_SELECT           13
#define MONKEY_A_BACK							MONKEY_A_SELECT
#define MONKEY_A_CROSS            MONKEY_A_A
#define MONKEY_A_TRIANGLE         MONKEY_A_B

#endif

// How often to press a button in non-random super monkey states (higher = less often)
#define SUPER_MONKEY_BUTTON_PRESS_DELAY 8

#endif // defined(CONTROLLER_MONKEY)

// Pure virtual input device class.  All input devices must implement these functions.
class input_device
{
protected:
	friend class input_mgr;
	friend class ps2_input_mgr;
	friend class gc_input_mgr;
	friend class xbox_input_mgr;
	device_id_t device_id;
	
#ifdef CONTROLLER_MONKEY
	// The controller Monkey!
	
	// true if we want a controller monkey rather than actual input
	bool  randomized;
	// the various probabilities of random button presses, etc, per frame
	float dbutton_probability[MONKEY_NUM_ABUTTONS];
	float abutton_probability[MONKEY_NUM_DBUTTONS];
	float stick_move_probability[MONKEY_NUM_STICKS];  // left is 1, right is 0
	float stick_zero_probability[MONKEY_NUM_STICKS];  // left is 1, right is 0
	int   stick_move_magnitude[MONKEY_NUM_STICKS];    // left is 1, right is 0
	int16 stick_position[MONKEY_NUM_STICKS][2];       // left is 1, right is 0
#endif
	
public:
	input_device();
	
	virtual stringx get_name() const = 0;
	virtual stringx get_name( int axis ) const = 0;
	virtual device_id_t get_id() const = 0;
	
	virtual int get_axis_count() const = 0;
	virtual axis_id_t get_axis_id(int axis) const = 0;
	virtual rational_t get_axis_state( axis_id_t axis, int control_axis ) const = 0;
	virtual rational_t get_axis_old_state( axis_id_t axis, int control_axis ) const = 0;
	virtual rational_t get_axis_delta( axis_id_t axis, int control_axis ) const = 0;
	
	virtual bool is_connected() const { return false; }
	
	virtual void poll() = 0;
	
	virtual unsigned char normalize( int raw ){return raw;} // CONVERT RAW INPUTS TO OUTSIDE USES (Keyboard)
	
	virtual ~input_device();
	
	
	friend class rumble_manager;
	
	virtual void vibrate( float intensity )=0;
	virtual void vibrate( int vibrator_flag, int vibrator_power, int vibrator_freq, int vibrator_inc )=0;
	virtual void stop_vibration()=0;
	virtual bool is_vibrator_present() const=0;

	// For manually manipulating the button states.  Used by the Super Monkey.
	virtual void set_button_d(int button_num, bool state) {}
	virtual void set_button_a(int button_num, int state) {}
	virtual void set_stick(int stick_num, int new_x, int new_y) {}
	virtual void clear_state() {}

#ifdef CONTROLLER_MONKEY
	// The controller monkey!  (random input)
	virtual void set_monkey(bool random_on = true) { randomized = random_on; }
	virtual void MonkeySetDigitalProbability(int button, float prob) { dbutton_probability[button] = prob; }
	virtual void MonkeySetAnalogProbability(int button, float prob) { abutton_probability[button] = prob; }
	virtual void MonkeySetStickMotion(bool left, float move_prob, float zero_prob, int move_magnitude)
	{
		stick_move_probability[(int)left] = move_prob;
		stick_zero_probability[(int)left] = zero_prob;
		stick_move_magnitude[(int)left]   = move_magnitude;
	}
	virtual void GetRandomInput() {}
#endif
};

#if USE_STL_MAP_FOR_DEVICES
typedef map<device_id_t, input_device *> device_map_t;
#else
typedef vector<input_device *> device_map_t;
#endif

// Device/axis combination class.
// The location of an axis on a device, used to specify what axes are mapped to what
// controls.
class device_axis
{
public:
	device_axis()
	{}
	
	device_axis( device_id_t dev, axis_id_t axis )
		: device( dev )
		, axis( axis )
	{}
	
	// Note that the default copy constructors are allowed to be made, since they
	// currently work just fine.
	
private:
	// Info necessary to access the state of the axis.
	device_id_t device;
	axis_id_t   axis;
	int control_axis;
	
	friend class input_mgr;
	friend bool operator==( const device_axis & lhs, const device_axis & rhs );
};

inline bool operator==( const device_axis & lhs, const device_axis & rhs )
{
	return lhs.device == rhs.device && lhs.axis == rhs.axis;
}

typedef list<device_axis> device_axis_list_t;

enum control_t
{
	CT_BOOLEAN,
		CT_RATIONAL,
};

// Game control class.  Includes the name of the control and type of value expected.
class game_control
{
public:
	game_control()
	{}
	
	game_control( control_id_t _name, control_t _type )
		: name( _name )
		, type( _type )
	{}
	
private:
	control_id_t  name;
	control_t     type;
	
	// Device & axis that this control is currently mapped to.
	device_axis_list_t mapping;
	
	friend class input_mgr;
};

// The control map is a map instead of a vector, since I expect we will eventually set up a numbering scheme
// like Player = 100-199, Debug = 200-299, etc. where not every control ID is mapped and there are large gaps.
//typedef vector<game_control> control_map_t;
typedef map<control_id_t, game_control> control_map_t;

// Input manager class.  Keeps track of all input devices and game control mappings.
class input_mgr : public singleton
{
public:
	// Auto-singleton support.  Use input_mgr::inst()->function_name( parameters);
	//static input_mgr * inst();
	DECLARE_SINGLETON(input_mgr)
		
		enum
	{
		_INPUT_DISABLE_KEYBOARD  = 0x00000001,
			_INPUT_DISABLE_VIBRATION = 0x00000002,
	};
	
	//---------------------------------------------------------------------------------------------------
	// Device access and control
	//---------------------------------------------------------------------------------------------------
	
	// Scan the system for recognized input devices.
	// This function scans the system (PC or Dreamcast) for input devices and inserts them into mgr's device map.
	// Linking to different hwos??.lib files makes this prototype point to different functions.
	// It must be implemented in each hwos?? library, along with the platform specific input devices.
	void scan_devices();
	
	// Get a pointer to a specific device by name.  This function will return 0 if the device
	// is not present.
	input_device * get_device_from_map( device_id_t id ) const;
	//input_device * get_device() const { return( (*device_map.begin()).second); }
	
	// Add a device to the system.  Is usually only called by scan_input_devices()
	void insert_device( input_device * device );
	
	//---------------------------------------------------------------------------------------------------
	// Control registration and mapping support
	//---------------------------------------------------------------------------------------------------
	
	// Load an input map from a text file.
	void load_input_map( const char * fname );
	
	// Register a game control.  Unregistration is not currently implemented.
	void register_control( const game_control & control );
	
	// Register/unregister an input device axis->game control mapping.
	
	// Adds the mapping to any existing mappings for the control.
	void map_control( control_id_t control, const device_axis& da );
	// Translates the control name into an axis id, and then maps the control.
	void map_control( control_id_t control, device_id_t device, int axis );
	
	// Removes the mapping from the control.
	void unmap_control( control_id_t control, device_axis& axis );
	// Removes all mappings from the control
	void unmap_control( control_id_t control );
	
	// Clear all axis->control mappings.
	void clear_mapping();
	
	// Retrieve the current mapping of a game control.
	bool is_control_mapped( control_id_t control ) const;
	const device_axis_list_t & get_control_mapping( control_id_t control ) const;
	
	//---------------------------------------------------------------------------------------------------
	// Polling functions
	//---------------------------------------------------------------------------------------------------
	
	// Poll all devices in the system.  Device values will not change until this function is called.
	// Generally you want to call this function each game cycle.
	void poll_devices();
	
	// Get the state of a mapped game function.
	// Note that controls of type BOOLEAN still return a rational, either 0.0f or 1.0f.
	rational_t get_control_state( device_id_t dev_id, control_id_t control ) const;
	
	// Get the state delta of the control.  Returns ( current_state - previous_state ).
	rational_t get_control_delta( device_id_t dev_id, control_id_t control ) const;
	
	// Returns 0 unless control moved to an axis limit this frame (as in, 'pulled the trigger')
	rational_t get_control_trigger( device_id_t dev_id, control_id_t control ) const;
	
	
	/* DEPRECATED 09.12.01 -- Please use rumble_manager class instead
	rational_t vibrate_control( control_id_t control, float intensity );
	rational_t stop_vibrating_control( control_id_t control );
	*/
	
	rumble_manager *get_vibrator() const { return( rumble_ptr ); }
	
	void disable_keyboard()  { flags |= _INPUT_DISABLE_KEYBOARD; }
	void enable_keyboard()   { flags &= ~_INPUT_DISABLE_KEYBOARD; }
	bool keyboard_disabled() const { return(flags & _INPUT_DISABLE_KEYBOARD); }
	bool keyboard_enabled()  const { return(!keyboard_disabled()); }
	
	//pfe_page_options::retrieve_option( OPTION_VIBRATOR )
	void disable_vibration()  { flags |= _INPUT_DISABLE_VIBRATION; }
	void enable_vibration()   { flags &= ~_INPUT_DISABLE_VIBRATION; }
	bool vibration_disabled() const { return(flags & _INPUT_DISABLE_VIBRATION); }
	bool vibration_enabled()  const { return(!vibration_disabled()); }
	
	inline input_device *get_joystick(device_id_t id) const
	{
		assert(IS_JOYSTICK_DEVICE(id));
		return(joystick_devices[DEVICE_ID_TO_JOYSTICK_INDEX(id)]);
	}
	
	inline input_device *get_joystick_index(int index) const
	{
		assert(index >= 0 && index < MAX_JOYSTICK_DEVICES);
		return(joystick_devices[index]);
	}
	
	inline input_device *get_keyboard(device_id_t id) const
	{
		assert(IS_KEYBOARD_DEVICE(id));
		return(keyboard_devices[DEVICE_ID_TO_KEYBOARD_INDEX(id)]);
	}
	
	inline input_device *get_keyboard_index(int index) const
	{
		assert(index >= 0 && index < MAX_KEYBOARD_DEVICES);
		return(keyboard_devices[index]);
	}
	
	inline input_device *get_mouse(device_id_t id) const
	{
		assert(IS_MOUSE_DEVICE(id));
		return(mouse_devices[DEVICE_ID_TO_MOUSE_INDEX(id)]);
	}
	
	inline input_device *get_mouse_index(int index) const
	{
		assert(index >= 0 && index < MAX_MOUSE_DEVICES);
		return(mouse_devices[index]);
	}
	
	inline input_device *get_device(device_id_t id) const
	{
		switch(id)
		{
		case JOYSTICK1_DEVICE:
		case JOYSTICK2_DEVICE:
		case JOYSTICK3_DEVICE:
		case JOYSTICK4_DEVICE:
		case JOYSTICK5_DEVICE:
		case JOYSTICK6_DEVICE:
		case JOYSTICK7_DEVICE:
		case JOYSTICK8_DEVICE:
			return(joystick_devices[DEVICE_ID_TO_JOYSTICK_INDEX(id)]);
			break;
			
		case KEYBOARD1_DEVICE:
			return(keyboard_devices[DEVICE_ID_TO_KEYBOARD_INDEX(id)]);
			break;
			
		case MOUSE1_DEVICE:
			return(mouse_devices[DEVICE_ID_TO_MOUSE_INDEX(id)]);
			break;
			
		default:
			return(get_device_from_map(id));
			break;
		}
	}
	
	
	void frame_advance(const time_value_t t);
	bool is_rumble_finished();
	//  void set_rumble_finished(bool finished) { rumble_finished = finished; }
	
	int GetDefaultController() {return default_controller;}
	void SetDefaultController(int cont) {default_controller = cont;}
	
private:
	input_mgr();
	
	rumble_manager *rumble_ptr;
	device_map_t device_map;
	control_map_t control_map;
	
	int default_controller; // the index of the currently selected controller
	
	unsigned int flags;
	
	// private helper functions-- do not use
	rational_t get_control_state_helper(input_device *devptr, axis_id_t axis, int control_axis, control_t ctype ) const;
	rational_t get_control_delta_helper(input_device *devptr, axis_id_t axis, int control_axis, control_t ctype ) const;
	
protected:
	input_device *joystick_devices[MAX_JOYSTICK_DEVICES];
	input_device *keyboard_devices[MAX_KEYBOARD_DEVICES];
	input_device *mouse_devices[MAX_MOUSE_DEVICES];
	
	void reset_joystick_array();
	void reset_keyboard_array();
	void reset_mouse_array();
};


#ifdef CONTROLLER_MONKEY


// To implement the super monkey I'm making it a state machine.  There'll be three
// types of states.  The first type is completely random, no limits.  There'll
// only be one of those.  Second, the states that use the 
// controls to load a career level (including exiting the current level if 
// necessary).  Finally, there's a state to do limited randomness (random but no
// possibility of hitting back or start, so that it won't exit the game).

class monkey_state
{
protected:
	input_device *controller;
	monkey_state *next;

	bool state_done; // true if it's ready to move on to the next state
public:
	
	virtual void Init(input_device *cont, monkey_state *next_state);
	virtual void SetControllerInput() = 0;
	virtual void StartState() { state_done = false; }

	void GoToNextState(monkey_state **state) // goes to next state if this state is finished
	{ 
		if(state_done)
		{
			*state = next; 
			(*state)->StartState();
		}
	}
};

class monkey_state_random : public monkey_state
{
public:
	virtual void SetControllerInput() { controller->GetRandomInput(); }
};

// a pure virtual class for the non-random monkey states
class monkey_state_non_random : public monkey_state
{
protected:
	static int button_pressed;
	static int next_level;
public:
	virtual void Init(input_device *cont, monkey_state *next_state);
};

// for when we're in the menu system and we want to find the main menu
class monkey_state_seek_main_menu : public monkey_state_non_random
{
public:
	virtual void SetControllerInput();
};

// for when we're in the main menu and we want to select "career"
class monkey_state_seek_career_entry : public monkey_state_non_random
{
public:
	virtual void SetControllerInput();
};

// for when we have "career" highlighted then we want to X through to the beach select screen
class monkey_state_seek_beach_menu : public monkey_state_non_random
{
public:
	virtual void SetControllerInput();
};

// for when we're on beach select and we're trying to find the right beach
class monkey_state_seek_beach : public monkey_state_non_random
{
public:
	virtual void SetControllerInput();
};

class monkey_state_seek_challenge : public monkey_state_non_random
{
public:
	virtual void SetControllerInput();
};

class monkey_state_load_level : public monkey_state_non_random
{
private:
	int super_state_flyby_count;  // the number of frames when flyby _wasn't_ the current super state
public:
	virtual void StartState() { monkey_state_non_random::StartState(); super_state_flyby_count = 0; }
	virtual void SetControllerInput();
};

class monkey_state_limited_random : public monkey_state_random
{
private:
	void SetRandomness();
public:
	virtual void Init(input_device *cont, monkey_state *next_state)
	{
		monkey_state::Init(cont, next_state);
		SetRandomness();
	}
	virtual void SetControllerInput();
};

class monkey_state_end_run : public monkey_state_non_random
{
public:
	virtual void SetControllerInput();
};

class monkey_state_seek_retry_menu : public monkey_state_non_random
{
public:
	virtual void SetControllerInput();
};

class monkey_state_quit_to_map : public monkey_state_non_random
{
public:
	virtual void SetControllerInput();
};

class super_monkey
{
public:
	enum
	{
		StateRandom,
		StateSeekMainMenu,
		StateSeekCareerEntry,
		StateSeekBeachMenu,
		StateSeekBeach,
		StateSeekChallenge,
		StateLoadLevel,
		StateLimitedRandom,
		StateEndRun,
		StateSeekRetryMenu,
		StateQuitToMap,
		StateLast
	};

	super_monkey();
	virtual ~super_monkey();
	void Init(input_device *cont, int first_state);
	virtual void SetControllerInput();

private:
	monkey_state *states[StateLast];
	monkey_state *current_state;
};

extern super_monkey g_super_monkey;

#endif // CONTROLLER_MONKEY

#endif
