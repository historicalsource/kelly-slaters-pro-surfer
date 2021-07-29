#ifndef CRAWL_BOX_H
#define CRAWL_BOX_H

#include "box_trigger_interface.h"

#define CRAWL_NORMAL 0x00
#define CRAWL_SMALL  0x01
#define CRAWL_NONE   0x02

class crawl_box
{
public:
  int type;
	vector3d pos;
	convex_box box;
  bool forced;

	crawl_box( ) 
		: type( CRAWL_NORMAL ), forced( false ) { }
  crawl_box( const crawl_box& _box )
    : type( _box.type ), pos( _box.pos ), box( _box.box ) { }
  crawl_box( int _type, bool _forced, const vector3d& _po, const convex_box& _box )
    : type( _type ), pos( _po ), box( _box ), forced( _forced ) { }
  ~crawl_box( ) { }

  inline bool point_inside( const vector3d& pt ) const
  {
    bool b = box.point_inside( pt, pos );
    return b;
  }

  inline void set_forced( bool _b ) { forced = _b; }
  inline bool is_forced( void ) const { return forced; }

  inline void set_pos( const vector3d& _p ) { pos = _p; }
  inline const vector3d& get_pos( void ) const { return pos; }

  inline void set_type( int _t ) { type = _t; }
  inline int get_type( void ) const { return type; }

  inline void set_box( const convex_box& _b ) { box = _b; }
  inline const convex_box& get_box( void ) const { return box; }
};

#endif