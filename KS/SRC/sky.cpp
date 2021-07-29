// sky.cpp

// This isn't a true skybox because it doesn't follow the camera Y position.
// A true skybox would always have the camera be in the exact center.

#include "global.h"

#include "project.h"
#include "sky.h"
#include "app.h"
#include "game.h"
#include "camera.h"


sky::sky( const entity_id& eid, entity_flavor_t _flavor )
: entity( eid, _flavor )
{
}


sky::sky( chunk_file& fs, const entity_id& _id, entity_flavor_t _flavor, unsigned int _flags )
: entity( fs, _id, _flavor, _flags )
{
}


entity* sky::make_instance( const entity_id& _id,
                            unsigned int _flags ) const
{
  sky* e = NEW sky( _id, ENTITY_SKY );
  e->copy_instance_data( *this );
  return (entity*)e;
}


void sky::render( camera* camera_link, rational_t detail, render_flavor_t flavor, rational_t entity_translucency_pct )
{
  // set sky position according to current_view_camera position
  vector3d p = app::inst()->get_game()->get_current_view_camera()->get_abs_position();
  p.y = get_abs_position().y;
  set_rel_position( p );
  // now do the render
  entity::render( camera_link, detail, flavor, entity_translucency_pct );
}
