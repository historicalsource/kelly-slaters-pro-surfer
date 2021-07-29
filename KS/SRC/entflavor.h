#ifndef ENTFLAVOR_H
#define ENTFLAVOR_H
////////////////////////////////////////////////////////////////////////////////
/*
  run-time type-information for entities

  Bjarne would turn over in his grave, but this is the nineties.  Oh wait, he's
  not dead.  Maybe he's mellowed out since writing the grey book.
*/
////////////////////////////////////////////////////////////////////////////////

enum entity_flavor_t
{
  ENTITY_CAMERA,
  ENTITY_ENTITY,
  ENTITY_MARKER,
  ENTITY_MIC,
  ENTITY_LIGHT_SOURCE,
  ENTITY_PARTICLE_GENERATOR,
  ENTITY_PHYSICAL,
  ENTITY_ITEM,
  ENTITY_LIGHT,
  ENTITY_MOBILE,
  ENTITY_CONGLOMERATE,
  ENTITY_TURRET,
  ENTITY_BEAM,
  ENTITY_SCANNER,
  ENTITY_MORPHABLE_ITEM,
  ENTITY_SKY,
  ENTITY_MANIP,
  ENTITY_SWITCH,
  ENTITY_BOX_TRIGGER,
  ENTITY_POLYTUBE,
  ENTITY_LENSFLARE,
  NUM_ENTITY_FLAVORS,
  IGNORE_FLAVOR
};

#endif