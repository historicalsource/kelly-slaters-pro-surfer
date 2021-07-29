#ifndef VERTNORM_H
#define VERTNORM_H

// made it cache friendly for SH4 by padding
class vert_normal
  {
  public:
    vector3d vec;
    unsigned int pad;
  };

#endif