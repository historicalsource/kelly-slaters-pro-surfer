/*--------------------------------------------------------------------------------------------------------

INPUTMGR.CPP
  
Input manager class implementation
	
--------------------------------------------------------------------------------------------------------*/
#include "global.h"
#include "wds.h"

#include "inputmgr.h"
//#include "rumble_manager.h"

//#include "joystick.h"  // for sega hack
#include "keyboard.h"
#include "game.h"
#include "AIController.h"
#include "frontendmanager.h"


#ifdef TARGET_XBOX
const float BOOLEAN_AXIS_FUDGE_FACTOR = 32.0f/255.0f;
#else
const float BOOLEAN_AXIS_FUDGE_FACTOR = 0.75f;
#endif
const float RATIONAL_DEADZONE_FACTOR = 0.15f;

#ifdef CONTROLLER_MONKEY
super_monkey g_super_monkey;
#endif

DEFINE_SINGLETON(input_mgr)


kbeventcallback kbevcb;
void* kbevudata;
kbcharcallback kbchcb;
void* kbchudata;

bool KB_register_event_callback(kbeventcallback cb,void* userdata)
{
	kbevcb=cb;
	kbevudata=userdata;
	return true;
}

bool KB_register_char_callback(kbcharcallback cb, void* userdata)
{
	kbchcb=cb;
	kbchudata=userdata;
	return true;
}

// these are for use by the window system or input device code
void KB_post_event_callback(KeyEvent eventtype, int key)
{
#if defined(DEBUG) && 0
	char buf[32];
	sprintf(buf," %s key %d",eventtype==kePress ? "press" : "release", key);
	debug_print(buf);
#endif
	if (kbevcb)
		kbevcb(eventtype,key,kbevudata);
}

void KB_post_char_callback(char ch)
{
#if defined(DEBUG) && 0
	char buf[32];
	sprintf(buf," char '%c' (0x%02x)",ch,ch);
	debug_print(buf);
#endif
	if (kbchcb)
		kbchcb(ch,kbchudata);
}


// THis is just to set some reasonable default values for the controller monkey
input_device::input_device()
{
	device_id = INVALID_DEVICE_ID;
	
#ifdef CONTROLLER_MONKEY
	//set the controller monkey probabilities to zero (no random button presses or moves)
	int i;
	for(i = 0; i < MONKEY_NUM_ABUTTONS; i++)
		abutton_probability[i] = 0.01f;
	for(i = 0; i < MONKEY_NUM_DBUTTONS; i++)
		dbutton_probability[i] = 0.01f;
	for(i = 0; i < MONKEY_NUM_STICKS; i++)
	{
		stick_move_probability[i] = 0.3f;
		stick_move_magnitude[i] = 0x00003000;
		stick_zero_probability[i] = 0.05f;
		stick_position[i][0] = 0;
		stick_position[i][1] = 0;
	}
	
	randomized = false;
	
#ifdef TARGET_XBOX
	unsigned random_seed = time(NULL);
	// print out the random seed for reproducability
	debug_print("Controller Monkey Random Seed: 0x%x\n", random_seed);
	srand(random_seed);
#endif // TARGET_XBOX
	
#endif
}

/*-------------------------------------------------------------------------------------------------------
input_device implementation
-------------------------------------------------------------------------------------------------------*/
// Implementation is required for virtual destructors, even if they are pure.
input_device::~input_device()
{
}


//---------------------------------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------------------------------
input_mgr::input_mgr()
{
	flags = 0;
	
	int index = 0;
	
	for(index = 0; index < MAX_JOYSTICK_DEVICES; ++index)
		joystick_devices[index] = NULL;
	for(index = 0; index < MAX_KEYBOARD_DEVICES; ++index)
		keyboard_devices[index] = NULL;
	for(index = 0; index < MAX_MOUSE_DEVICES; ++index)
		mouse_devices[index] = NULL;
	
	// Will this work?
	joystick_devices[DEVICE_ID_TO_JOYSTICK_INDEX(AI_JOYSTICK)] = new AISurferController();
	joystick_devices[DEVICE_ID_TO_JOYSTICK_INDEX(AI_JOYSTICK)]->device_id = AI_JOYSTICK;
	insert_device(joystick_devices[DEVICE_ID_TO_JOYSTICK_INDEX(AI_JOYSTICK)]);
	scan_devices();
	
#ifdef CONTROLLER_MONKEY
	// if we're in super monkey mode, start with the super monkey's seek main menu state
	if(os_developer_options::inst()->get_int(os_developer_options::INT_SUPER_MONKEY) != 0)
	{
		g_super_monkey.Init(get_joystick(JOYSTICK1_DEVICE), super_monkey::StateSeekMainMenu);
		get_joystick(JOYSTICK1_DEVICE)->set_monkey(true);
		os_developer_options::inst()->set_flag(os_developer_options::FLAG_REALISTIC_FE, false);
		os_developer_options::inst()->set_flag(os_developer_options::FLAG_NO_REPLAY, true);
	}
	else
	{
		// Otherwise just do plain old random
		g_super_monkey.Init(get_joystick(JOYSTICK1_DEVICE), super_monkey::StateRandom);
	
		if(os_developer_options::inst()->is_flagged(os_developer_options::FLAG_RANDOMIZE_CONTROLLER_1))
			get_joystick(JOYSTICK1_DEVICE)->set_monkey(true);
		if(os_developer_options::inst()->is_flagged(os_developer_options::FLAG_RANDOMIZE_CONTROLLER_2))
			get_joystick(JOYSTICK2_DEVICE)->set_monkey(true);
		if(os_developer_options::inst()->is_flagged(os_developer_options::FLAG_RANDOMIZE_CONTROLLER_3))
			get_joystick(JOYSTICK3_DEVICE)->set_monkey(true);
		if(os_developer_options::inst()->is_flagged(os_developer_options::FLAG_RANDOMIZE_CONTROLLER_4))
			get_joystick(JOYSTICK4_DEVICE)->set_monkey(true);
	}
#endif
	
	rumble_ptr = NULL;
	//rumble_ptr = new rumble_manager();
	
	SetDefaultController(0);
}

//---------------------------------------------------------------------------------------------------
// Frame_Advance
//---------------------------------------------------------------------------------------------------
void input_mgr::frame_advance(time_value_t t)
{
/*  assert (rumble_ptr && "This should never be NULL");

  if ( !rumble_ptr->is_rumble_finished() && !g_game_ptr->is_paused() )
    rumble_ptr->frame_advance(t);*/
}

#if defined(TARGET_XBOX)
static const char *device_id_to_str0( device_id_t id )
{
	switch(id)
	{
	case INVALID_DEVICE_ID:
		return "INVALID_DEVICE_ID";
	case JOYSTICK1_DEVICE:
		return "JOYSTICK1_DEVICE";
	case JOYSTICK2_DEVICE:
		return "JOYSTICK2_DEVICE";
	case JOYSTICK3_DEVICE:
		return "JOYSTICK3_DEVICE";    
	case JOYSTICK4_DEVICE:
		return "JOYSTICK4_DEVICE";    
	case JOYSTICK5_DEVICE:
		return "JOYSTICK5_DEVICE";    
	case JOYSTICK6_DEVICE:
		return "JOYSTICK6_DEVICE";
	case JOYSTICK7_DEVICE:
		return "JOYSTICK7_DEVICE";    
	case JOYSTICK8_DEVICE:
		return "JOYSTICK8_DEVICE";    
	case KEYBOARD1_DEVICE:
		return "KEYBOARD1_DEVICE";    
	case MOUSE1_DEVICE:
		return "MOUSE1_DEVICE";
	case ANY_LOCAL_JOYSTICK:
		return "ANY_LOCAL_JOYSTICK";    
	case AI_JOYSTICK:
		return "AI_JOYSTICK";
	default:
		break;
	}
	
	return 0;
}


#endif /* TARGET_XBOX JIV DEBUG */

//---------------------------------------------------------------------------------------------------
// Device access and control
//---------------------------------------------------------------------------------------------------

//void input_mgr::scan_devices(); // in HWOS section!

// Get a pointer to a specific device by id.  This function will return 0 if the device
// is not present.
input_device * input_mgr::get_device_from_map( device_id_t id ) const
{
#if USE_STL_MAP_FOR_DEVICES
	
	device_map_t::const_iterator it = device_map.find( id );
	
	if ( it == device_map.end() )
		return NULL;
	
	return (*it).second;
	
#else
	
	device_map_t::const_iterator it = device_map.begin();
	device_map_t::const_iterator it_end = device_map.end();
	while(it != it_end)
	{
		if((*it) && (*it)->get_id() == id)
			return(*it);
		
		++it;
	}
	
	/*  Get rid of all the warning messages for keyboard controls (dc 03/07/02)
	const char *dev_str = device_id_to_str0(id);
	nglPrintf("Unknown device %s\n", dev_str );
	*/
	
	return NULL;
	
#endif
}

// Add a device to the system.  Should usually only be called by scan_input_devices().
void input_mgr::insert_device( input_device * device )
{
#if USE_STL_MAP_FOR_DEVICES
	device_map[device->get_id()]=device;
#else
	if(find(device_map.begin(), device_map.end(), device) == device_map.end())
		device_map.push_back(device);
#endif
	
	if(IS_JOYSTICK_DEVICE(device->get_id()))
		joystick_devices[DEVICE_ID_TO_JOYSTICK_INDEX(device->get_id())] = device;
	else if(IS_KEYBOARD_DEVICE(device->get_id()))
		keyboard_devices[DEVICE_ID_TO_KEYBOARD_INDEX(device->get_id())] = device;
	else if(IS_MOUSE_DEVICE(device->get_id()))
		mouse_devices[DEVICE_ID_TO_MOUSE_INDEX(device->get_id())] = device;
}

//---------------------------------------------------------------------------------------------------
// Control registration and mapping support
//---------------------------------------------------------------------------------------------------

// Load an input map from a text file.
//void input_mgr::load_input_map( const char * fname );

// Register a game control.  Unregistration is not currently implemented.
void input_mgr::register_control( const game_control & control )
{
	assert( control_map.find( control.name ) == control_map.end() );
	control_map[control.name] = control;
}

// Register/unregister an input device axis->game control mapping.
// Overrides any existing mapping for the control.
void input_mgr::map_control( control_id_t control, const device_axis & da )
{
	control_map_t::iterator it = control_map.find( control );
	assert( it != control_map.end() );
	(*it).second.mapping.push_back( da );
}

// Translates the names into device and axis ids, and then maps the control.
void input_mgr::map_control( control_id_t control, device_id_t device, int axis )
{
	device_axis da;
	
	// Find the device and axis based on the names.
	input_device *device_ptr = get_device_from_map(device);
	if(device_ptr == NULL)
		return;
	
	da.device = device;
	da.control_axis = axis;
	da.axis   = device_ptr->get_axis_id(da.control_axis);
	
	if ( da.axis == INVALID_AXIS_ID )
		return;
	
	map_control( control, da );
}

void input_mgr::unmap_control( control_id_t control, device_axis & axis )
{
	control_map_t::iterator it = control_map.find( control );
	assert( it != control_map.end() );
	(*it).second.mapping.remove( axis );
}

void input_mgr::unmap_control( control_id_t control )
{
	control_map_t::iterator it = control_map.find( control );
	assert( it != control_map.end() );
	(*it).second.mapping = device_axis_list_t();      // <<<< does this cause a memory leak? check STL book
}

// Clear all axis->control mappings.
void input_mgr::clear_mapping()
{
	control_map_t::iterator it = control_map.begin();
	for ( ; it != control_map.end(); ++it )
		(*it).second.mapping = device_axis_list_t();
}

// Retrieve the current mapping of a game control.
bool input_mgr::is_control_mapped( control_id_t control ) const
{
	control_map_t::const_iterator it = control_map.find( control );
	assert( it != control_map.end() );
	return (*it).second.mapping.size() > 0;
}

const device_axis_list_t & input_mgr::get_control_mapping( control_id_t control ) const
{
	control_map_t::const_iterator it = control_map.find( control );
	assert( it != control_map.end() );
	return (*it).second.mapping;
}

//---------------------------------------------------------------------------------------------------
// Polling
//---------------------------------------------------------------------------------------------------

// Poll all devices in the system.  Device values will not change until this function is called.
// Generally you want to call this function each game cycle.
void input_mgr::poll_devices()
{
#if USE_STL_MAP_FOR_DEVICES
	device_map_t::iterator it = device_map.begin();
	for ( ; it != device_map.end(); ++it )
		( *it ).second->poll();
#else
	device_map_t::iterator it = device_map.begin();
	device_map_t::iterator it_end = device_map.end();
	for ( ; it != it_end; ++it )
		( *it )->poll();
#endif
}

// Get the state of a mapped game function.
// Note that controls of type BOOLEAN still return a rational, either 0.0f or 1.0f.
// MODIFIED 11/10/01 for KSPS to allow binding to a random-access joystick and also to "any joystick". -DL
rational_t input_mgr::get_control_state( device_id_t dev_id, control_id_t control ) const
{
	control_map_t::const_iterator it = control_map.find( control );
	assert( it != control_map.end() );
	const game_control& ctrl = (*it).second;
	
	// If the control is not mapped, it's pinned at 0.0f.
	if ( !ctrl.mapping.size() )
	{
		return 0.0f;
	}
	
	
	const device_axis_list_t& dalist = ctrl.mapping;
	device_axis_list_t::const_iterator dait;
	
	input_device * device_ptr;
	rational_t value = 0.0f;
	
	for ( dait = dalist.begin(); dait != dalist.end(); ++dait )
	{
		//device_id_t dev_id = ( *dait ).device;
		if(dev_id == ANY_LOCAL_JOYSTICK)
		{ // need to check against all joystick ports
			for ( int i=JOYSTICK1_DEVICE; i<=JOYSTICK4_DEVICE; i++ )
			{
				device_ptr = get_device( (device_id_t) i );
				if(device_ptr)
					value += get_control_state_helper(device_ptr,(*dait).axis,(*dait).control_axis,ctrl.type);  //measures axis
			}
		}
		else
		{ // just check against specified controller port
			device_ptr = get_device( dev_id );
			if(device_ptr)
				value += get_control_state_helper(device_ptr,(*dait).axis,(*dait).control_axis,ctrl.type);  //measures axis
		}
	}
	return value;
}

rational_t input_mgr::get_control_state_helper(input_device *devptr, axis_id_t axis, int control_axis, control_t ctype ) const
{ // just a helper function for get_control_state function so we don't write duplicate code
	assert(devptr);
	rational_t v = devptr->get_axis_state( axis, control_axis );
	if ( ctype == CT_BOOLEAN )
	{
#ifdef TARGET_XBOX
		// Thresholds were dealt with in devptr->get_axis_state(), so we
		// can assume everything below the threshold has been floored to zero
		if ( v < 0.0f )
			v = AXIS_MIN;
		else if ( v > 0.0f )
			v = AXIS_MAX;
		else
			v = AXIS_MID;
#else
		if ( v < AXIS_MIN*BOOLEAN_AXIS_FUDGE_FACTOR )
			v = AXIS_MIN;
		else if ( v > AXIS_MAX*BOOLEAN_AXIS_FUDGE_FACTOR )
			v = AXIS_MAX;
		else
			v = AXIS_MID;
#endif
	}
	
	return v;
}



// Get the state delta of the control.  Returns ( current_state - previous_state ).
// MODIFIED 11/10/01 for KSPS to allow binding to a random-access joystick and also to "any joystick". -DL
rational_t input_mgr::get_control_delta( device_id_t dev_id, control_id_t control ) const
{
	control_map_t::const_iterator it = control_map.find( control );
	assert( it != control_map.end() );
	const game_control& ctrl = (*it).second;
	
	// If the control is not mapped, it's pinned at 0.0f.
	if ( !ctrl.mapping.size() )
	{
		return 0.0f;
	}
	
	const device_axis_list_t & dalist = ctrl.mapping;
	device_axis_list_t::const_iterator dait;
	
	input_device * device_ptr;
	rational_t value = 0.0f;
	
	for ( dait = dalist.begin(); dait != dalist.end(); dait++ )
	{
		//device_id_t dev_id = ( *dait ).device;
		
		//device_id_t dev_id = ( *dait ).device;
		if(dev_id == ANY_LOCAL_JOYSTICK)
		{ // need to check against all joystick ports
			for ( int i=JOYSTICK1_DEVICE; i<=JOYSTICK4_DEVICE; i++ )
			{
				device_ptr = get_device( (device_id_t) i );
				if(device_ptr)
					value += get_control_delta_helper(device_ptr,(*dait).axis,(*dait).control_axis,ctrl.type);  //measures axis
			}
		}
		else
		{
			device_ptr = get_device( dev_id );
			if(device_ptr)
				value += get_control_delta_helper(device_ptr,(*dait).axis,(*dait).control_axis,ctrl.type);
		}
		/*
		if(device != NULL)
		{
        if ( ctrl.type == CT_BOOLEAN )
        {
		rational_t v0 = device->get_axis_old_state( (*dait).axis, ( *dait ).control_axis );
		if ( v0 < AXIS_MIN*BOOLEAN_AXIS_FUDGE_FACTOR )
		v0 = AXIS_MIN;
		else if ( v0 > AXIS_MAX*BOOLEAN_AXIS_FUDGE_FACTOR )
		v0 = AXIS_MAX;
		else
		v0 = AXIS_MID;
		rational_t v1 = device->get_axis_state( (*dait).axis, ( *dait ).control_axis );
		if ( v1 < AXIS_MIN*BOOLEAN_AXIS_FUDGE_FACTOR )
		v1 = AXIS_MIN;
		else if ( v1 > AXIS_MAX*BOOLEAN_AXIS_FUDGE_FACTOR )
		v1 = AXIS_MAX;
		else
		v1 = AXIS_MID;
		v1 -= v0;
		if ( v1 < 0 )
		v1 = AXIS_MIN;
		else if ( v1 > 0 )
		v1 = AXIS_MAX;
		value += v1;
        }
        else
		value += device->get_axis_delta( ( *dait ).axis, ( *dait ).control_axis );
		}
		*/
	}
	
	return value;
}

rational_t input_mgr::get_control_delta_helper(input_device *devptr, axis_id_t axis, int control_axis, control_t ctype ) const
{ // just a helper function for get_control_delta function so we don't write duplicate code
	assert(devptr);
	
	rational_t retval;
	
	if ( ctype == CT_BOOLEAN )
	{
		rational_t v0 = devptr->get_axis_old_state( axis, control_axis );
		if ( v0 < AXIS_MIN*BOOLEAN_AXIS_FUDGE_FACTOR )
			v0 = AXIS_MIN;
		else if ( v0 > AXIS_MAX*BOOLEAN_AXIS_FUDGE_FACTOR )
			v0 = AXIS_MAX;
		else
			v0 = AXIS_MID;
		
		rational_t v1 = devptr->get_axis_state( axis, control_axis );
		if ( v1 < AXIS_MIN*BOOLEAN_AXIS_FUDGE_FACTOR )
			v1 = AXIS_MIN;
		else if ( v1 > AXIS_MAX*BOOLEAN_AXIS_FUDGE_FACTOR )
			v1 = AXIS_MAX;
		else
			v1 = AXIS_MID;
		
		retval = v1 - v0;
		if ( retval < 0 )
			retval = AXIS_MIN;
		else if ( retval > 0 )
			retval = AXIS_MAX;
	}
	else
	{
		retval = devptr->get_axis_delta( axis, control_axis );
	}
	
	return retval;
}


// Returns 0 unless control moved to an axis limit this frame (as in, 'pulled the trigger')
rational_t input_mgr::get_control_trigger( device_id_t dev_id, control_id_t control ) const
{
	rational_t delta = get_control_delta( dev_id, control );
	if ( delta == get_control_state(dev_id, control) )
		return delta;
	return 0;
}

/*
rational_t input_mgr::vibrate_control( control_id_t control, float intensity )
{
if ( vibration_disabled( ) )
return 1.0f;

  control_map_t::iterator it = control_map.find( control );
  
	assert( it != control_map.end( ) );
	
	  game_control& ctrl = (*it).second;
	  
		if ( !( ctrl.mapping.size( ) ) )
		return 0.0f;
		
		  device_axis_list_t & dalist = ctrl.mapping;
		  device_axis_list_t::iterator dait;
		  
			for ( dait = dalist.begin(); dait != dalist.end(); dait++ )
			{
			device_id_t dev_id = ( *dait ).device;
			input_device * device = get_device( dev_id );
			
			  if(device != NULL)
			  device->vibrate( intensity );
			  }
			  
				return 1.0f;
				}
*/

bool input_mgr::is_rumble_finished()
{
	return true;
	//  return( rumble_ptr->is_rumble_finished() );
}

/*
rational_t input_mgr::stop_vibrating_control( control_id_t control )
{
if ( vibration_disabled( ) )
return 1.0f;

  control_map_t::iterator it = control_map.find( control );
  
	assert( it != control_map.end( ) );
	
	  game_control& ctrl = (*it).second;
	  
		if ( !( ctrl.mapping.size( ) ) )
		return 0.0f;
		
		  device_axis_list_t & dalist = ctrl.mapping;
		  device_axis_list_t::iterator dait;
		  
			for ( dait = dalist.begin(); dait != dalist.end(); dait++ )
			{
			device_id_t dev_id = ( *dait ).device;
			input_device * device = get_device( dev_id );
			
			  if(device != NULL)
			  device->stop_vibration( );
			  }
			  
				return 1.0f;
				}
*/
void input_mgr::reset_joystick_array()
{
	for(int i=0; i<MAX_JOYSTICK_DEVICES; ++i)
		joystick_devices[i] = get_device_from_map(JOYSTICK_TO_DEVICE_ID(i+1));
}

void input_mgr::reset_keyboard_array()
{
	for(int i=0; i<MAX_KEYBOARD_DEVICES; ++i)
		keyboard_devices[i] = get_device_from_map(KEYBOARD_TO_DEVICE_ID(i+1));
}

void input_mgr::reset_mouse_array()
{
	for(int i=0; i<MAX_MOUSE_DEVICES; ++i)
		mouse_devices[i] = get_device_from_map(MOUSE_TO_DEVICE_ID(i+1));
}

/////////////////////////////////////////////////////////////////////////////////////////
// Monkey input state machine
//
// Here are the implementations of SetControllerInput() for the different controller
// monkey states.
/////////////////////////////////////////////////////////////////////////////////////////

#ifdef CONTROLLER_MONKEY

int monkey_state_non_random::button_pressed;
int monkey_state_non_random::next_level;


void monkey_state::Init(input_device *cont, monkey_state *next_state)
{
	controller = cont;
	next = next_state;
	state_done = false;
}

void monkey_state_non_random::Init(input_device *cont, monkey_state *next_state)
{
	monkey_state::Init(cont, next_state);
	button_pressed = false;
	next_level = LEVEL_BELLS; // start out on Bells because Nick told me to :)
}

void monkey_state_seek_main_menu::SetControllerInput()
{
	controller->clear_state();
	
	if(!button_pressed)
	{
		if(!frontendmanager.gms ||                                           // the menu system hasn't initialized yet
			frontendmanager.gms->active != GraphicalMenuSystem::MainMenu)    // or we're not in the main menu yet
		{
			controller->set_button_a(MONKEY_A_CROSS, 255);
			button_pressed++;
		}
		else // we are in the main menu, so it's time to move on to the next state
			state_done = true;
	}
	else
		button_pressed = (button_pressed + 1) % SUPER_MONKEY_BUTTON_PRESS_DELAY;
}

void monkey_state_seek_career_entry::SetControllerInput()
{
	assert(frontendmanager.gms->active == GraphicalMenuSystem::MainMenu);
	controller->clear_state();
	
	if(!button_pressed)
	{
		if(frontendmanager.gms->menus[frontendmanager.gms->active]->highlighted->entry_num != MainFrontEnd::MainCareerEntry)
		{
			//frontendmanager.gms->menus[frontendmanager.gms->active]->OnRight(0);
			controller->set_button_d(MONKEY_D_DPAD_RIGHT, true);
			button_pressed++;
		}
		else
			state_done = true;
	}
	else
		button_pressed = (button_pressed + 1) % SUPER_MONKEY_BUTTON_PRESS_DELAY;
}

void monkey_state_seek_beach_menu::SetControllerInput()
{
	controller->clear_state();
	
	if(!button_pressed)
	{
		if(frontendmanager.gms->active != GraphicalMenuSystem::BeachMenu)
		{
			//frontendmanager.gms->menus[frontendmanager.gms->active]->OnCross(0);
			controller->set_button_a(MONKEY_A_CROSS, 255);
			button_pressed++;
		}
		else // we are in the beach menu, so it's time to move on to the next state
			state_done = true;
	}
	else
		button_pressed = (button_pressed + 1) % SUPER_MONKEY_BUTTON_PRESS_DELAY;
}

void monkey_state_seek_beach::SetControllerInput()
{
	controller->clear_state();
	
	if(!button_pressed)
	{
		if(frontendmanager.map->GetCurrentLocation() != BeachDataArray[CareerDataArray[next_level].beach].map_location)
		{
			//frontendmanager.gms->menus[frontendmanager.gms->active]->OnRight(0);
			controller->set_button_d(MONKEY_D_DPAD_RIGHT, true);
			button_pressed++;
		}
		else
			state_done = true;
	}
	else
		button_pressed = (button_pressed + 1) % SUPER_MONKEY_BUTTON_PRESS_DELAY;
}

void monkey_state_seek_challenge::SetControllerInput()
{
	static int up_or_down = 0;  // if up_or_down / 2 is even then go up; otherwise, go down
	controller->clear_state();
	
	if(!button_pressed)
	{
		if(frontendmanager.map->GetCurrentLevel() != next_level)
		{
			// Thiis setup makes it so that the monkey will hit down three times then 
			// up three times, to make sure it hits all the challenges
			if((up_or_down / 2) & 1) // if it's odd, go down
				controller->set_button_d(MONKEY_D_DPAD_DOWN, true);
			else
				controller->set_button_d(MONKEY_D_DPAD_UP, true);

			up_or_down++;
			button_pressed++;
		}
		else
			state_done = true;
	}
	else
		button_pressed = (button_pressed + 1) % SUPER_MONKEY_BUTTON_PRESS_DELAY;
}

// Just a little fudge factor to make sure the monkey really gets 
// past the flyby state before going into limited random mode
#define NON_FLYBY_FRAME_COUNT 5

void monkey_state_load_level::SetControllerInput()
{
	controller->clear_state();
	
	if(!button_pressed)
	{
		if(g_world_ptr && g_world_ptr->get_ks_controller(0) != NULL &&
		   g_world_ptr->get_ks_controller(0)->get_super_state() != SUPER_STATE_FLYBY)
			super_state_flyby_count++;
		
		if(!FEDone() || 
			(frontendmanager.pms && frontendmanager.pms->draw && frontendmanager.pms->active == PauseMenuSystem::MapMenu) ||
			(g_world_ptr &&  g_world_ptr->get_ks_controller(0) == NULL) ||
			super_state_flyby_count < NON_FLYBY_FRAME_COUNT)
		{
			//frontendmanager.gms->menus[frontendmanager.gms->active]->OnCross(0);
			controller->set_button_a(MONKEY_A_CROSS, 255);
			button_pressed++;
		}
		else
		{
			state_done = true;
			// increment next_level so we open the subsequent level next time
			next_level++;
			next_level %= LEVEL_LAST;
		}
	}
	else
		button_pressed = (button_pressed + 1) % SUPER_MONKEY_BUTTON_PRESS_DELAY;
}

void monkey_state_limited_random::SetRandomness()
{
	// disable the back button, so we don't accidentally get into the debug menu
	controller->MonkeySetDigitalProbability(MONKEY_D_SELECT, 0.0f);
	controller->MonkeySetDigitalProbability(MONKEY_D_START, 0.0f);
}

void monkey_state_limited_random::SetControllerInput()
{
	static bool pressed = false;
	controller->clear_state();
	
	if(!frontendmanager.pms)
	{
		// do nothing.  Wait for level to load.
	}
	else if(frontendmanager.pms->draw) // if the pause menu is up for some reason
	{
		if (!pressed)
		{
			controller->set_button_d(MONKEY_D_START, true);	
			pressed = true;
		}
		else
			pressed = false;
	}
	else
	{
		monkey_state_random::SetControllerInput();
		if(TIMER_GetLevelSec() >= (float)(os_developer_options::inst()->get_int(os_developer_options::INT_SUPER_MONKEY)))
			state_done = true;
	}
}

void monkey_state_end_run::SetControllerInput()
{
	controller->clear_state();
	
	if(!button_pressed)
	{
		if(!frontendmanager.pms->draw) // if the pause menu isn't up)
		{
			controller->set_button_d(MONKEY_D_START, true);
			button_pressed++;
		}
		// else if the last menu item isn't highlighted
		else if((!g_game_ptr->is_competition_level() && frontendmanager.pms->menus[frontendmanager.pms->active]->highlighted->GetText() != ksGlobalTextArray[GT_MENU_END_RUN])||
			(g_game_ptr->is_competition_level() &&  frontendmanager.pms->menus[frontendmanager.pms->active]->highlighted->GetText() != ksGlobalTextArray[GT_MENU_END_COMP]))
		{
			//frontendmanager.gms->menus[frontendmanager.gms->active]->OnDown(0);
			controller->set_button_d(MONKEY_D_DPAD_DOWN, true);
			button_pressed++;
		}
		else // we're on the last entry, so select it and move to the next state
		{
			//frontendmanager.gms->menus[frontendmanager.gms->active]->OnCross(0);
			controller->set_button_a(MONKEY_A_CROSS, 255);
			button_pressed++;
			state_done = true;
		}
	}
	else
		button_pressed = (button_pressed + 1) % SUPER_MONKEY_BUTTON_PRESS_DELAY;
}

void monkey_state_seek_retry_menu::SetControllerInput()
{
	controller->clear_state();
	
	if(!button_pressed)
	{
		// if we aren't in the end run menu and we aren't in the comp end menu and we either 
		// aren't in the saveconfirm menu or we are and the last entry is highlighted
		if(frontendmanager.pms->active != PauseMenuSystem::EndRunMenu && 
		   frontendmanager.pms->active != PauseMenuSystem::CompEndMenu &&
		   (frontendmanager.pms->active != PauseMenuSystem::SaveConfirmMenu ||
		    (frontendmanager.pms->active == PauseMenuSystem::SaveConfirmMenu &&
		     frontendmanager.pms->menus[frontendmanager.pms->active]->entries->previous == 
		     frontendmanager.pms->menus[frontendmanager.pms->active]->highlighted
			)
		   )
		  )
		{
			//frontendmanager.gms->menus[frontendmanager.gms->active]->OnCross(0);
			controller->set_button_a(MONKEY_A_CROSS, 255);
			button_pressed++;
		}
		// else if we're in the saveconfirm menu and the last entry isn't highlighted
		else if(frontendmanager.pms->active == PauseMenuSystem::SaveConfirmMenu && // if it's asking us whether we want to save
		        frontendmanager.pms->menus[frontendmanager.pms->active]->highlighted->GetText() != ksGlobalTextArray[GT_FE_MENU_NO]) // and the last item isn't yet highlighted
		{
			controller->set_button_d(MONKEY_D_DPAD_DOWN, true);
			button_pressed++;
		}
		else // we're on the Retry menu, so go on to the next state
		{
			state_done = true;
		}
	}
	else
		button_pressed = (button_pressed + 1) % SUPER_MONKEY_BUTTON_PRESS_DELAY;
}

void monkey_state_quit_to_map::SetControllerInput()
{
	static bool confirm=false;
	controller->clear_state();
	
	if(!button_pressed)
	{
		if (confirm)
		{
			controller->set_button_a(MONKEY_A_CROSS, 255);
			button_pressed++;
			state_done = true;
			confirm=false;
		}
		else
		{
			assert(frontendmanager.pms->active == PauseMenuSystem::EndRunMenu || frontendmanager.pms->active == PauseMenuSystem::CompEndMenu);

			// if we aren't on the last entry yet
			if(frontendmanager.pms->menus[frontendmanager.pms->active]->highlighted->GetText() != ksGlobalTextArray[GT_MENU_BEACH_SELECT]) 
			{
				// move down in the menu
				//frontendmanager.gms->menus[frontendmanager.gms->active]->OnDown(0);
				controller->set_button_d(MONKEY_D_DPAD_DOWN, true);
				button_pressed++;
			}
			else // we're on the last entry, so press cross and go on to the next state
			{
				//frontendmanager.gms->menus[frontendmanager.gms->active]->OnCross(0);
				controller->set_button_a(MONKEY_A_CROSS, 255);
				button_pressed++;
				confirm = true;
			}
		}
	}
	else
		button_pressed = (button_pressed + 1) % SUPER_MONKEY_BUTTON_PRESS_DELAY;
}

super_monkey::super_monkey()
{
	states[StateRandom]          = NEW monkey_state_random;
	states[StateSeekMainMenu]    = NEW monkey_state_seek_main_menu;
	states[StateSeekCareerEntry] = NEW monkey_state_seek_career_entry;
	states[StateSeekBeachMenu]   = NEW monkey_state_seek_beach_menu;
	states[StateSeekBeach]       = NEW monkey_state_seek_beach;
	states[StateSeekChallenge]   = NEW monkey_state_seek_challenge;
	states[StateLoadLevel]       = NEW monkey_state_load_level;
	states[StateLimitedRandom]   = NEW monkey_state_limited_random;
	states[StateEndRun]          = NEW monkey_state_end_run;
	states[StateSeekRetryMenu]   = NEW monkey_state_seek_retry_menu;
	states[StateQuitToMap]       = NEW monkey_state_quit_to_map;
}

super_monkey::~super_monkey()
{
	int i;
	for(i = 0; i < StateLast; i++)
		delete states[i];
}

void super_monkey::Init(input_device *cont, int first_state)
{
	states[StateRandom]->Init(cont, states[StateRandom]);
	states[StateSeekMainMenu]->Init(cont, states[StateSeekCareerEntry]);
	states[StateSeekCareerEntry]->Init(cont, states[StateSeekBeachMenu]);
	states[StateSeekBeachMenu]->Init(cont, states[StateSeekBeach]);
	states[StateSeekBeach]->Init(cont, states[StateSeekChallenge]);
	states[StateSeekChallenge]->Init(cont, states[StateLoadLevel]);
	states[StateLoadLevel]->Init(cont, states[StateLimitedRandom]);
	states[StateLimitedRandom]->Init(cont, states[StateEndRun]); // change this
	states[StateEndRun]->Init(cont, states[StateSeekRetryMenu]);
	states[StateSeekRetryMenu]->Init(cont, states[StateQuitToMap]);
	states[StateQuitToMap]->Init(cont, states[StateSeekBeach]);

	current_state = states[first_state];
}

void super_monkey::SetControllerInput()
{ 
	current_state->SetControllerInput(); 
	current_state->GoToNextState(&current_state); 
}

#endif // CONTROLLER_MONKEY
