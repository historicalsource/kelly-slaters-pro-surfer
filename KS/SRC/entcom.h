#ifndef ENTCOM_H
#define ENTCOM_H

#include "hwmath.h"
//!class character;
class entity;

class character_command
{
public:
  character_command(character * _chr, entity * _e1, entity * _e2, rational_t _radius);
  void process();

  bool is_active() const { return active; }
private:
  bool active;
  character * chr;
  entity * e1;
  entity * e2;
  rational_t radius;
};
#endif