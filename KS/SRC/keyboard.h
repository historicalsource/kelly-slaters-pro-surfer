#ifndef KEYBOARD_H
#define KEYBOARD_H

/*----------------------------------------------------------------------------------------------
 
  KEYBOARD.H - Keyboard input device constants.

  This module defines the various axis IDs that may be associated with a keyboard like input 
  device.  These constants are based on DirectInput's constants, since that is currently the
  only implementation supported.  The Shinobi code will probably translate these values.

  Note that an axis can be a button or a lever, and that they range from 0.0f to 1.0f.
 
----------------------------------------------------------------------------------------------*/

#include "inputmgr.h"

//const device_id_t KEYBOARD_DEVICE = 2;


enum KeyEvent 
{
  keRelease,
  kePress,
};


typedef void (*kbeventcallback)(KeyEvent eventtype, int key, void* userdata);
typedef void (*kbcharcallback) (char ch, void* userdata);

bool KB_register_event_callback(kbeventcallback,void* userdata);
bool KB_register_char_callback (kbcharcallback, void* userdata);

// these are for use by the window system or input device code
void KB_post_event_callback(KeyEvent eventtype, int key);
void KB_post_char_callback (char ch);


enum Key_Axes 
{
  KB_A=1,                   
  KB_B,                   
  KB_C,                   
  KB_D,                   
  KB_E,                   
  KB_F,                   
  KB_G,                   
  KB_H,                   
  KB_I,                   
  KB_J,                   
  KB_K,                   
  KB_L,                   
  KB_M,                   
  KB_N,                   
  KB_O,                   
  KB_P,                   
  KB_Q,                   
  KB_R,                   
  KB_S,                   
  KB_T,                   
  KB_U,                   
  KB_V,                   
  KB_W,                   
  KB_X,                   
  KB_Y,                   
  KB_Z,                   

  KB_1,                   
  KB_2,                   
  KB_3,                   
  KB_4,                   
  KB_5,                   
  KB_6,                   
  KB_7,                   
  KB_8,                   
  KB_9,                   
  KB_0,                   

  KB_LSHIFT,
  KB_RSHIFT,
  KB_LCONTROL,
  KB_RCONTROL,
  KB_LALT,
  KB_RALT,
  KB_LSPECIAL,
  KB_RSPECIAL,

  KB_RETURN,              
  KB_ESCAPE,              
  KB_BACKSPACE,           
  KB_TAB,                 
  KB_SPACE,               

  KB_MINUS,               
  KB_EQUALS,                
  KB_LBRACKET,            
  KB_RBRACKET,            
  KB_BACKSLASH,       
  KB_SEMICOLON,       
  KB_QUOTE,               
  KB_TILDE,               
  KB_COMMA,               
  KB_PERIOD,              
  KB_SLASH,               
  KB_CAPSLOCK,            

  KB_F1,	                 
  KB_F2,	                 
  KB_F3,	                 
  KB_F4,	                 
  KB_F5,	                 
  KB_F6,	                 
  KB_F7,	                 
  KB_F8,	                 
  KB_F9,	                 
  KB_F10,                 
  KB_F11,                 
  KB_F12,                 

  KB_PRINT,               
  KB_SCROLLLOCK,          
  KB_PAUSE,               

  KB_HOME,                
  KB_END,                 
  KB_PAGEUP,              
  KB_PAGEDOWN,            
  KB_INSERT,              
  KB_DELETE,              

  KB_RIGHT,               
  KB_LEFT,                
  KB_DOWN,                
  KB_UP,                  

  KB_NUMLOCK,             
  KB_DIVIDE,              
  KB_MULTIPLY,            
  KB_SUBTRACT,            
  KB_ADD,                 

  KB_NUMPADENTER,         

  KB_NUMPAD1, 
  KB_NUMPAD2, 
  KB_NUMPAD3, 
  KB_NUMPAD4, 
  KB_NUMPAD5,
  KB_NUMPAD6, 
  KB_NUMPAD7, 
  KB_NUMPAD8, 
  KB_NUMPAD9, 
  KB_NUMPAD0, 
  KB_DECIMAL, 
  
  KB_ACCESS, 
 
  KB_NUM_AXES
};

#endif