////////////////////////////////////////////////////////////////////////////////
/*
  entcom.cpp

  placeholder for giving entity commands.   Currently character commands only
  are supported

*/
////////////////////////////////////////////////////////////////////////////////
#include "global.h"

//!#include "entcom.h"
//!#include "character.h"
#include "controller.h"
//!#include "rigid.h"


////////////////////////////////////////////////////////////////////////////////
//  character_command
////////////////////////////////////////////////////////////////////////////////

character_command::character_command(character * _chr, entity * _e1, entity * _e2, rational_t _radius)
{
  chr = _chr;
  e1 = _e1;
  e2 = _e2;
  radius = _radius;
  active = true;
  chr->set_active(false);
  chr->get_controller()->set_active(false);
}


void character_command::process()
{
  assert (active == true);
  vector3d diff = e1->get_abs_position()-e2->get_abs_position();
  if (diff.length2() < radius*radius )
  {
    chr->set_active(true);
    chr->get_controller()->set_active(true);
    active = false;
  }
}

