#ifndef _RENDERFLAV_H
#define _RENDERFLAV_H


enum
{
  RENDER_OPAQUE_PORTION      =0x001,  // opaque portion only, non-clipped, progressive
  RENDER_TRANSLUCENT_PORTION =0x002,  // translucent portion only
  RENDER_CLIPPED_FULL_DETAIL =0x004,  // clipped, non-progressive, non-morphing
  RENDER_RAW_ICON_THROUGHPUT =0x008,  // raw throughput for screen "icons"
  RENDER_REAL_POINT_LIGHTS   =0x010,  // don't fake point lights as directional lights, because this geometry is too big (a terrain thing)
  RENDER_NO_LIGHTS           =0x020,  // render without lighting
  RENDER_ENTITY_WIDGET       =0x040,  // rendering an entity widget
  RENDER_LORES_MODEL         =0x080,  // rendering a lo res model
  RENDER_SHADOW_MODEL        =0x100,  // rendering a lo res model
};

typedef unsigned render_flavor_t;


#endif  // _RENDERFLAV_H
