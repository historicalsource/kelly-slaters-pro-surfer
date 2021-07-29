// faceflags.h
#ifndef _FACEFLAGS_H
#define _FACEFLAGS_H


// terrain face flags
enum
  {
  TERFACE_WALKABLE           = 0x0100,  // run-time flag
  TERFACE_FORCE_WALKABLE     = 0x0100,  // .ter file flag
  TERFACE_FORCE_NOT_WALKABLE = 0x0200,  // .ter file flag
  TERFACE_COSMETIC           = 0x0400,
  TERFACE_WATER              = 0x0800,
  TERFACE_VEGETATION         = 0x1000,

  TERFACE_TOP_BORDER         = 0x0001,  // runtime render flag
  TERFACE_BOTTOM_BORDER      = 0x0002,  // runtime render flag
  TERFACE_LEFT_BORDER        = 0x0004,  // runtime render flag
  TERFACE_RIGHT_BORDER       = 0x0008,  // runtime render flag

  TERFACE_SURFTYPE_MASK      = 0x0070,  // 3 bits reserved for surface types
  TERFACE_LEDGE              = 0x2000,
  TERFACE_CRAWLSPACELEDGE    = 0x4000
  };

#define TERFACE_IS_BORDER (TERFACE_TOP_BORDER | TERFACE_BOTTOM_BORDER | TERFACE_LEFT_BORDER | TERFACE_RIGHT_BORDER)

#endif  // _FACEFLAGS_H
