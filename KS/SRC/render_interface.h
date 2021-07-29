#ifndef ENTITY_RENDER_INTERFACE_CLASS_HEADER
#define ENTITY_RENDER_INTERFACE_CLASS_HEADER

#include "entity_interface.h"

class entity;

#ifdef TARGET_PS2
#include "ngl_ps2.h"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
/*  
    Design notes:

  Rather than make this an abstract base class off of which mesh and billboard descend, like
  we had for Draconus / Max Steel, I'm keeping this binary-file-format friendly
*/
////////////////////////////////////////////////////////////////////////////////////////////////////

enum render_interface_t
{
  RENDER_IFC_MESH, 
  RENDER_IFC_BILLBOARD
};

class vr_pmesh;
class vr_billboard;

class render_interface : public entity_interface
{
  private:
    int           m_type;
#ifdef TARGET_PS2                   // once xbox support comes online we'll remove this conditional and obsolete_mesh
    nglMesh*       m_mesh;
#else
    vr_pmesh*     m_obsolete_mesh; 
#endif
    vr_billboard* m_billboard;

  public:
    render_interface(entity *_my_entity);

    void render();
};

#endif//ENTITY_RENDER_INTERFACE_CLASS_HEADER
 