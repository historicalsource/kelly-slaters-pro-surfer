#include "global.h"

#include "render_interface.h"

#ifdef TARGET_PS2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#include "ngl_ps2.h"
#endif

render_interface::render_interface(entity *_my_entity)
      : entity_interface(_my_entity)
{
#ifdef TARGET_PS2
  m_mesh = NULL;
#endif
}
