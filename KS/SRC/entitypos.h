#ifndef ENTITYPOS_H
#define ENITTYPOS_H

#include "algebra.h"

class entity;

class entity_position
  {
  public:
    entity * relative_to;
    vector3d position;
  };

#endif
