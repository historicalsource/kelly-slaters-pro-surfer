#ifndef FLOATINGTEXT_H
#define FLOATINGTEXT_H
////////////////////////////////////////////////////////////////////////////////
/*
  floatingtext

  floating text usually attached to some other object
*/
////////////////////////////////////////////////////////////////////////////////
#include "billboard.h"
#include "instance.h"
//#include "material.h"


class vr_floating_text : public vr_billboard
{
public:
	vr_floating_text();
  vr_floating_text( int v, material* mat, bool _instanced = false );
	vr_floating_text( const stringx& s, material* mat, bool _instanced = false );

  virtual void render_batch( render_flavor_t flavor,
                             instance_render_info* viri,
                             int num_instances);
  void         set_diffuse_color( const color32& c ) { diffuse_color = c; }
private:
	stringx      text;		// the text to be displayed in the floatingtext
  color32      diffuse_color;  // unlike other visreps, this one has color
};

#endif
