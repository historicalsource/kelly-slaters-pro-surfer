#ifndef CLIP_FLAGS_H
#define CLIP_FLAGS_H


/*
  WARNING!!!
    you only have 8 bits for flags. The other 24 bits are used for normal information.
*/

enum
  {
/*
  CLIP_FRONT   = 0x00000001,
  CLIP_LEFT    = 0x00000002,
  CLIP_RIGHT   = 0x00000004,
  CLIP_TOP     = 0x00000008,
  CLIP_BOTTOM  = 0x00000010,
  CLIP_BACK    = 0x00000020,
  CLIP_BACKFACE= 0x00000040,
  CLIP_NOTINLOD= 0x00000080,
  FLAG_SELFLIT = 0x00000100
*/
  CLIP_BACKFACE= 0x00000001,
  CLIP_NOTINLOD= 0x00000002,
  FLAG_SELFLIT = 0x00000004,

  NORMAL_MASK  = 0xffffff00,
  FLAG_MASK    = 0x000000ff
  };
  
#endif