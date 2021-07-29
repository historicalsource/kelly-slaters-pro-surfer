#ifndef JOYSTICK_H
#define JOYSTICK_H

/*----------------------------------------------------------------------------------------------
 
  JOYSTICK.H - Joystick input device constants.

  This module defines the various axis IDs that may be associated with a generic joystick input 
  device.  These constants are based on DirectInput's constants, since that is currently the
  only implemtenation supported.  The Shinobi code will probably translate these values.

  Note that an axis can be a button or a lever, and that they range from 0.0f to 1.0f.
 
----------------------------------------------------------------------------------------------*/

#include "inputmgr.h"

/*
const device_id_t JOYSTICK_DEVICE = 1;
*/
/*----------------------------------------------------------------------------------------------

  Axis IDs:

  When you want to map an axis to a game control in code, just pass in the axis' id.  For example:

    const control_id_t PLAYER_FORWARD = 0;
    input_mgr::inst()->map_control( PLAYER_FORWARD, device_axis( JOYSTICK_DEVICE, JOY_Y ) );

----------------------------------------------------------------------------------------------*/

enum Joy_Axes
{
  JOY_DX,   // just Dpad
  JOY_DY,   // just Dpad

  JOY_X,    // Lstick and dpad
  JOY_Y,    // Lstick and dpad

  JOY_LX,   // left analog horiz
  JOY_LY,   // left analog vert
  JOY_Z,    // L3

  JOY_RX,   // right analog horiz
  JOY_RY,   // right analog horiz
  JOY_RZ,   // R3
      
  JOY_BTNA, // A 
  JOY_BTNB, // B 
  JOY_BTNC, // C 
  JOY_BTNX, // X 
  JOY_BTNY, // Y 
  JOY_BTNZ, // Z 

  JOY_BTNL, // L 
  JOY_BTNR, // R 
  JOY_BTNL2, // L 
  JOY_BTNR2, // R 

  JOY_BTNSTART, // Start
  JOY_BTNSELECT, // Select
  
  JOY_DISCONNECT, // disconnection axis (heh?)
  JOY_NUM_AXES
};

#define JOY_BTNL1 JOY_BTNL
#define JOY_BTNR1 JOY_BTNR
#define JOY_BTNL3 JOY_Z
#define JOY_BTNR3 JOY_RZ

// parallel mapping to Joy_Axes for PS2 Joystick
enum PS2_Axes
{
  JOY_PS2_DX,  // just Dpad
  JOY_PS2_DY,  // just Dpad

  JOY_PS2_X,  // Lstick and dpad
  JOY_PS2_Y,  // Lstick and dpad

  JOY_PS2_LX, // left analog horiz
  JOY_PS2_LY, // left analog vert
  JOY_PS2_BTNL3, // L3

  JOY_PS2_RX, // right analog horiz
  JOY_PS2_RY, // right analog vert
  JOY_PS2_BTNR3, // R3
      
  JOY_PS2_BTNX,  // A (ps2 x)
  JOY_PS2_BTNO,  // B (ps2 o)

  JOY_PS2_DUMMMY1,  // placeholder 

  JOY_PS2_BTNSQ, // X (ps2 sq)
  JOY_PS2_BTNTR, // Y (ps2 tr)

  JOY_PS2_DUMMMY2,  // placeholder 

  JOY_PS2_BTNL1, // Z (ps2 L1)
  JOY_PS2_BTNR1, // C (ps2 R1)
  JOY_PS2_BTNL2, // L (ps2 L2)
  JOY_PS2_BTNR2, // R (ps2 R2)

  JOY_PS2_START, // Start
  JOY_PS2_SELECT, // Select

  JOY_PS2_DISCONNECT,

  JOY_PS2_NUM_AXES
};

// parallel mapping to Joy_Axes for PS2 Joystick
enum XBOX_Axes
{
  JOY_XBOX_DX,  // just Dpad
  JOY_XBOX_DY,  // just Dpad

  JOY_XBOX_X,  // Lstick and dpad
  JOY_XBOX_Y,  // Lstick and dpad

  JOY_XBOX_LX, // left analog horiz
  JOY_XBOX_LY, // left analog vert
  JOY_XBOX_BTNL3, // L3

  JOY_XBOX_RX, // right analog horiz
  JOY_XBOX_RY, // right analog vert
  JOY_XBOX_BTNR3, // R3
      
  JOY_XBOX_BTNA,  // A
  JOY_XBOX_BTNB,  // B

  JOY_XBOX_DUMMMY1,  // placeholder 

  JOY_XBOX_BTNX, // X
  JOY_XBOX_BTNY, // Y

  JOY_XBOX_DUMMMY2,  // placeholder 

  JOY_XBOX_BTNL,		// Z (XBOX L)
  JOY_XBOX_BTNR,		// C (XBOX R)
  JOY_XBOX_BTNBLK, // L (XBOX Black)
  JOY_XBOX_BTNWHT, // R (XBOX White)

  JOY_XBOX_START, // Start
  JOY_XBOX_SELECT, // Select

  JOY_XBOX_DISCONNECT,

  JOY_XBOX_NUM_AXES
};

enum GC_Axes
{
	JOY_GC_DX, // d-pad horiz
	JOY_GC_DY, // d-pad vert
	
	JOY_GC_X,  // d-pad OR analog left horiz
	JOY_GC_Y,  // d-pad OR analog left vert
	
	JOY_GC_LX, // analog left horiz
	JOY_GC_LY, // analog left vert
	JOY_GC_DUMMY1, // placeholder
	
	JOY_GC_RX,     // analog right horiz
	JOY_GC_RY,     // analog right vert
	JOY_GC_DUMMY2, // placeholder
	
	JOY_GC_BTNA, // A
	JOY_GC_BTNX, // X
	
	JOY_GC_DUMMY3, // placeholder
	
	JOY_GC_BTNB,   // B
	JOY_GC_BTNY,   // Y
	
	JOY_GC_ANALOG_LEFT, // after the fact analog values for left
	
	JOY_GC_LEFT,   // left shoulder
	JOY_GC_RIGHT,  // right shoulder
	JOY_GC_ANALOG_RIGHT, // after the fact analog values for right
	JOY_GC_BTNZ,   // Z
	
	JOY_GC_START,  // start
	JOY_GC_DUMMY6, // placeholder
	
	JOY_GC_DISCONNECT, // Huh?
	
	JOY_GC_NUM_AXES
};

/*
// Movement axes
const char * const JOY_X     = "X";       
const char * const JOY_Y     = "Y";       
const char * const JOY_Z     = "Z";       
// buttons                              
const char * const JOY_BTN0  = "Button A";
const char * const JOY_BTN1  = "Button B";
const char * const JOY_BTN2  = "Button C";
const char * const JOY_BTN3  = "Button X";      
const char * const JOY_BTN4  = "Button Y";
const char * const JOY_BTN5  = "Button Z";
const char * const JOY_BTN6  = "Button L";      
const char * const JOY_BTN7  = "Button R";
const char * const JOY_BTN8  = "Start";      
const char * const JOY_BTN9  = "Shift";      
*/

#endif
