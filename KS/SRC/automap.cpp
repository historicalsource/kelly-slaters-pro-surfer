#include "global.h"

#include "automap.h"
#include "oserrmsg.h"
//!#include "character.h"
//#include "physent.h"
#include "wds.h"
#include "terrain.h"


automap::automap()
:   pixels_per_meter(1),
    origin(0,0),
    regions()
  {
#if defined( SUPPORT_AUTOMAP_REVEAL )
  mask = NULL;
#endif
  }


automap::~automap()
  {
  _destroy();
  }


void automap::_destroy()
  {
  regions.clear();
#if defined( SUPPORT_AUTOMAP_REVEAL )
  delete mask;
  mask = NULL;
#endif
  }


#if defined( SUPPORT_AUTOMAP_REVEAL )

void automap::reveal( const vector3d& world_coord, rational_t reveal_radius )
  {
  // construct mask for given reveal radius
  unsigned short w = reveal_radius * 2 * pixels_per_meter;
  unsigned short h = w;
  if ( w % 8 )
    w += 8 - (w % 8);
  bitplane reveal_mask( w, h );
  reveal_mask.clear();
  unsigned short x, y;
  vector3d wc;
  rational_t meters_per_pixel = 1.0f / pixels_per_meter;
  rational_t start_d = -(reveal_radius - meters_per_pixel / 2);
  wc.z = start_d;
  for ( y=0; y<h && wc.z<reveal_radius; y++,wc.z+=meters_per_pixel )
    {
    wc.x = start_d;
    for ( x=0; x<w && wc.x<reveal_radius; x++,wc.x+=meters_per_pixel )
      {
      if ( xz_norm2(wc) < reveal_radius*reveal_radius )
        reveal_mask.set( x, y );
      }
    }

  // apply reveal mask to map mask
  vector2d map_coord = get_map_coord( world_coord );
  start_d = -(reveal_radius * pixels_per_meter) + 0.5f;
  map_coord.x += start_d;
  map_coord.y += start_d;
  mask->blit_or( map_coord.x, map_coord.y, reveal_mask );
  }

#endif


void serial_in( chunk_file& fs, automap* m )
  {
  m->_destroy();
  chunk_flavor cf;
  m->width = 256;
  m->height = 256;
  for ( serial_in(fs,&cf); cf!=CHUNK_END; serial_in(fs,&cf) )
    {
    if ( cf == chunk_flavor("image") )
      {
      // read filename for full map texture
      serial_in( fs, &m->image_filename );
      }
    else if ( cf == chunk_flavor("size") )
      {
      // read map width and height (used to create mask bitplane)
      serial_in( fs, &m->width );
      if ( (m->width%8) != 0 )
        error( fs.get_name() + ": automap '" + m->image_filename + "': width must be a multiple of 8" );
      serial_in( fs, &m->height );
      }
    else if ( cf == chunk_flavor("scale") )
      {
      // read world-to-map conversion scale
      serial_in( fs, &m->pixels_per_meter );
      }
    else if ( cf == chunk_flavor("origin") )
      {
      // read world origin expressed in map coords
      serial_in( fs, &m->origin );
      }
    else if ( cf == chunk_flavor("regions") )
      {
      // read list of regions
      stringx rname;
      for ( serial_in(fs,&rname); !(rname==chunkend_label); serial_in(fs,&rname) )
        {
        // find region matching given name
        region_node* rnode = g_world_ptr->get_the_terrain().find_region( rname );
        if ( rnode )
          {
          // point given region to this map and add region to list
          region* r = rnode->get_data();
          r->set_map( m );
          m->regions.push_back( r );
          }
        else
          error( fs.get_name() + ": automap '" + m->image_filename + "': region '" + rname + "' not found in terrain" );
        }
      }
    }
  }
