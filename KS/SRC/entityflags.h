#ifndef ENTITYFLAGS_H
#define ENTITYFLAGS_H

// flags for state number in reading entities from scene file
// duplicated in \d2dev\data\d2lib.sh
enum
  {
  ACTIVE_FLAG           = 0x00000001,
  STATIONARY_FLAG       = 0x00000002,
  INVISIBLE_FLAG        = 0x00000004,
  COSMETIC_FLAG         = 0x00000008,
  WALKABLE_FLAG         = 0x00000010,
  REPULSION_FLAG        = 0x00000020,
  FORCE_LIGHT_FLAG      = 0x00000040,
  NONSTATIC_FLAG        = 0x00000080,
  NO_DISTANCE_CLIP_FLAG = 0x00000100,

  OVERIDE_MASK_FLAG     = 0x000001FF,

  // newly added for tool purposes
  BEAMABLE_FLAG         = 0x00000200,
  NO_BEAMABLE_FLAG      = 0x00000400,
  SCANABLE_FLAG         = 0x00000800,
  NO_SCANABLE_FLAG      = 0x00001000,
  CAMERA_COLL_FLAG      = 0x00002000,
  NO_CAMERA_COLL_FLAG   = 0x00004000,
  ENTITY_COLL_FLAG      = 0x00008000,
  NO_ENTITY_COLL_FLAG   = 0x00010000,
  ACTIONABLE_FLAG       = 0x00020000,
  NO_ACTIONABLE_FLAG    = 0x00040000,
  ACTION_FACING_FLAG    = 0x00080000,
  NO_ACTION_FACING_FLAG = 0x00100000,
  IS_DOOR_FLAG          = 0x00200000,
  NO_IS_DOOR_FLAG       = 0x00400000,
  DOOR_OPEN_FLAG        = 0x00800000,
  DOOR_CLOSED_FLAG      = 0x01000000,
  };

#endif