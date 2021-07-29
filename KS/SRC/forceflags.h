#ifndef FORCEFLAGS_H
#define FORCEFLAGS_H

// flags to force various kinds of rendering.  Used by iri.h and hwrasterize.h

enum 
{
  FORCE_TRANSLUCENCY       = 0x0100, // force all polygons to be rendered as translucent
  FORCE_ADDITIVE_BLENDING  = 0x0200, // force all polygons to be rendered as additive
  FORCE_COLOR_PARAMETER    = 0x0400, // use material color from force_color instead of material

  FORCE_SKIP_CLIP          = 0x1000, // force it not to clip at all
  FORCE_TRIVIAL_FRONT_CLIP = 0x2000, // force it to not clip at all except for taking away polys that cross front
  FORCE_JUST_FRONT_CLIP    = 0x4000,
  FORCE_SKIP_FRONT_CLIP    = 0x8000, // we know its clear of front clip plane, not sure about others

  FORCE_LIGHT              = 0x0001, // force all vertices to be lit instead of relying on vertex color
  FORCE_NO_LIGHT           = 0x0002, // force no lighting at all

  FORCE_UNZBUFFERED        = 0x0010, // force all polygons to be rendered without z buffering
};
  
#endif // FORCEFLAGS_H