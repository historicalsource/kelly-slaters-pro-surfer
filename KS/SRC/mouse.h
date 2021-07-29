#ifndef MOUSE_H
#define MOUSE_H

/*----------------------------------------------------------------------------------------------
 
  MOUSE.H - Mouse input device constants.

  This module defines the various axis IDs that may be associated with a generic mouse input 
  device.  These constants are based on DirectInput's constants, since that is currently the
  only implementation supported.  The Shinobi code will probably translate these values.

  Note that an axis can be a button or a lever, and that they range from 0.0f to 1.0f.
 
----------------------------------------------------------------------------------------------*/

#include "inputmgr.h"

/*----------------------------------------------------------------------------------------------

  Axis IDs:

  When you want to map an axis to a game control in code, just pass in the axis' id.  For example:

    const control_id_t PLAYER_FORWARD = 0;
    input_mgr::inst()->map_control( PLAYER_FORWARD, device_axis( MOUSE_DEVICE, MSE_BUTTON0 ) );

----------------------------------------------------------------------------------------------*/

enum mouse_axes
{
  MSE_X,
  MSE_Y,      
  MSE_Z,      

  MSE_BTN0,
  MSE_BTN1,
  MSE_BTN2,

  MSE_NUM_AXES
};

#endif