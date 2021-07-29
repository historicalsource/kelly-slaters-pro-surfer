#ifndef MARKER_H
#define MARKER_H

#include "entity.h"

//!class character;

class marker : public entity
{
public:
  marker(const entity_id & eid, entity_flavor_t _flavor = ENTITY_MARKER) : entity(eid, _flavor) 
  {
  }

  virtual entity* make_instance( const entity_id& _id,
                                 unsigned int _flags ) const;

  void copy_instance_data( const marker& b ) { entity::copy_instance_data(b); }

/////////////////////////////////////////////////////////////////////////////
// entity class identification
public:
  virtual bool is_a_marker() const            { return true; }
  virtual bool possibly_active() const        { return(false); }
  virtual bool possibly_aging() const         { return(false); }
};

inline marker * find_marker( const entity_id& id )
{
  return (marker*)entity_manager::inst()->find_entity( id, ENTITY_MARKER );
}







class rectangle_marker : public marker
{
public:
  rectangle_marker(const entity_id & eid, entity_flavor_t _flavor = ENTITY_MARKER) : marker(eid, _flavor)
  {
    x_rad = 0.0f;
    z_rad = 0.0f;
  }

  rational_t x_rad;
  rational_t z_rad;

  virtual entity* make_instance( const entity_id& _id,
                                 unsigned int _flags ) const;

  void copy_instance_data( const rectangle_marker& b ) 
  { 
    marker::copy_instance_data(b); 

    x_rad = b.x_rad;
    z_rad = b.z_rad;
  }

/////////////////////////////////////////////////////////////////////////////
// entity class identification
public:
  virtual bool is_a_rectangle_marker() const { return true; }
  virtual bool point_inside(const vector3d &pt);

  virtual bool parse_instance( const stringx& pcf, chunk_file& fs );
};

inline rectangle_marker * find_rectangle_marker( const entity_id& id )
{
  marker* mark = (marker *)entity_manager::inst()->find_entity( id, ENTITY_MARKER );
  return (mark && mark->is_a_rectangle_marker()) ? (rectangle_marker *)mark : NULL;
}







class cube_marker : public rectangle_marker
{
public:
  cube_marker(const entity_id & eid, entity_flavor_t _flavor = ENTITY_MARKER) : rectangle_marker(eid, _flavor)
  {
    y_rad = 0.0f;
  }

  rational_t y_rad;

  virtual entity* make_instance( const entity_id& _id,
                                 unsigned int _flags ) const;

  void copy_instance_data( const cube_marker& b ) 
  { 
    rectangle_marker::copy_instance_data(b); 

    y_rad = b.y_rad;
  }

/////////////////////////////////////////////////////////////////////////////
// entity class identification
public:
  virtual bool is_a_cube_marker() const { return true; }
  virtual bool point_inside(const vector3d &pt);
  virtual bool parse_instance( const stringx& pcf, chunk_file& fs );
};

inline cube_marker * find_cube_marker( const entity_id& id )
{
  marker* mark = (marker *)entity_manager::inst()->find_entity( id, ENTITY_MARKER );
  return (mark && mark->is_a_cube_marker()) ? (cube_marker *)mark : NULL;
}








/*!
class crawl_marker : public rectangle_marker
{
public:
  crawl_marker(const entity_id & eid, entity_flavor_t _flavor = ENTITY_CRAWL_MARKER) : rectangle_marker(eid, _flavor) 
  {
  }

  virtual entity* make_instance( const entity_id& _id,
                                 unsigned int _flags ) const;

  void copy_instance_data( const crawl_marker& b ) 
  { 
    rectangle_marker::copy_instance_data(b); 
  }

/////////////////////////////////////////////////////////////////////////////
// entity class identification
public:
  virtual bool is_a_crawl_marker() const { return true; }

  virtual bool parse_instance( const stringx& pcf, chunk_file& fs );

  virtual bool point_inside(const vector3d &pt);
  bool allow_crawl(character *chr, rational_t dir = 0.0f);

  // true for crawling, false for standing
  bool crawl_or_stand(character *chr, rational_t dir = 0.0f);

  vector3d get_crawl_pos(character *chr);
};


inline crawl_marker * find_crawl_marker( const entity_id& id )
{
  return (crawl_marker*)entity_manager::inst()->find_entity( id, ENTITY_CRAWL_MARKER );
}
!*/
#endif
