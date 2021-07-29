#ifndef _AUTOMAP_H
#define _AUTOMAP_H


#include "stringx.h"
#include "algebra.h"
#include "bitplane.h"

class region;


//#define SUPPORT_AUTOMAP_REVEAL


class automap
  {
  // Data
  public:
    stringx image_filename;
  private:
    rational_t pixels_per_meter;
    unsigned short width, height;
    vector2d origin;
    vector<region*> regions;

  // Methods
  public:
    automap();
    ~automap();

    const stringx& get_image_filename() const { return image_filename; }
    rational_t get_pixels_per_meter() const { return pixels_per_meter; }
    unsigned short get_width() const { return width; }
    unsigned short get_height() const { return height; }
    const vector2d& get_origin() const { return origin; }
    const vector<region*>& get_regions() const { return regions; }

    vector2d get_map_coord( const vector3d& world_coord )
      {
      vector2d mc( world_coord.x, -world_coord.z );
      mc *= pixels_per_meter;
      mc += origin;
      return mc;
      }

#if defined( SUPPORT_AUTOMAP_REVEAL )

  // automap revelation mask
  private:
    bitplane* mask;
  public:
    const bitplane* get_mask() const { return mask; }
    void reveal( const vector3d& world_coord, rational_t reveal_radius );

#endif

  private:
    void _destroy();

  friend void serial_in( chunk_file& fs, automap* map );
  };


#endif  // _AUTOMAP_H
