#ifndef RENDER_DATA_H
#define RENDER_DATA_H

#include "rect.h"

class render_data
{
public:
  struct region_info
  {
    region_node* reg;
    rectf screen_rect;

    region_info(region_node* areg=0) : reg(areg) {}

    friend bool operator ==(const region_info& l,const region_info& r) { return l.reg==r.reg; }
  };

  typedef vector<region_info> region_list;
  region_list  regions;
  
  struct entity_info
  {
    entity* ent;
    rational_t extent;

    entity_info(entity* aent=0) : ent(aent) {}

    friend bool operator ==(const entity_info& l,const entity_info& r) { return l.ent==r.ent; }
  };

  typedef vector<entity_info> entity_list;
  entity_list entities;

  vector3d	 cam;

  render_data()
  {
    regions.reserve( 8 );
    entities.reserve( 256 );
  }
  void clear()
  {
    regions.resize(0);
    entities.resize(0);
  }
  ~render_data()
  {
    clear();
  }
};



#endif // RENDER_DATA_H
