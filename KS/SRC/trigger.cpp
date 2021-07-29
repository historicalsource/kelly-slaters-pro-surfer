// trigger.cpp
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.

#include "global.h"

#include "trigger.h"
#include "wds.h"
#include "terrain.h"
#include "ai_interface.h"
#include "controller.h"


DEFINE_SINGLETON(trigger_manager)


// local workspace for adding trigger to regions


trigger *trigger_manager::new_trigger(stringx id, stringx type, chunk_file &fs)
{
	trigger *t = 0;

	if (type == stringx("point"))
		t = NEW point_trigger(id);
	else if (type == stringx("region"))
		t = NEW region_trigger(id);
	else if (type == stringx("entity"))
		t = NEW entity_trigger(id);
	else
		error("Unknown trigger type \"" + type + "\" on trigger \"" + id + "\"");

  t->read(fs);

  // add NEW trigger to managed list
  add( t );
	return t;
}

// create a point-radius trigger
trigger* trigger_manager::new_point_trigger( vector3d p, rational_t r )
{
  trigger* t = NEW point_trigger( "ANONYMOUS", p, r );
  add( t );
  return t;
}

// create an entity-radius trigger
trigger* trigger_manager::new_entity_trigger( entity* e, rational_t r )
{
  trigger* t = NEW entity_trigger( "ANONYMOUS", e, r );
  add( t );
  return t;
}

// create a convex box trigger
trigger* trigger_manager::new_box_trigger( entity* e )
{
  assert( e );
#ifdef ECULL
  trigger* t = NEW box_trigger( "ANONYMOUS", e );
  add( t );
  return t;
#else
	return NULL;
#endif

}

void trigger_manager::add( trigger* t )
{
	t->next = list;
	list = t;
}

void trigger_manager::remove( trigger* trem )
{
  trigger *tkill = 0;

  if( list == trem )
  {
    tkill = list;
    list = list->next;
  }
  else
  {
    for (trigger *t = list; t; t = t->next)
    {
      if( t->next == trem )
      {
        t->next = t->next->next;
        tkill = t->next;
        break;
      }
    }
  }
  if( tkill )
  {
    delete tkill;
  }
}

void trigger_manager::purge()
{
  while( list )
  {
    remove( list );
  }
  new_regions.resize(0);
}

void trigger_manager::update()
{
	for (trigger *t = list; t; t = t->next)
		t->update();
}

void trigger_manager::update_regions()
{
	for (trigger *t = list; t; t = t->next)
		t->update_region();
}

trigger *trigger_manager::find_instance(const stringx &id)
{
	for (trigger *t = list; t; t = t->next)
		if (t->id == id)
			return t;

	return NULL;
}


trigger::trigger( const stringx& _id )
: signaller(),
  id( _id ),
  next( NULL ),
  static_regions( false ),
  active( true ),
  occupied( false ),
  use_any_char( false )
{
}


void trigger::update()
{
  if( is_active() )
  {
    // check the hero
    bool hit = triggered( g_world_ptr->get_hero_ptr(g_game_ptr->get_active_player()) );

    if(hit)
      whodunnit = g_world_ptr->get_hero_ptr(g_game_ptr->get_active_player());

    if ( !hit && use_any_char )
    {
      list<ai_interface *>::const_iterator ai = ai_interface::get_all_ai_interfaces().begin();
      list<ai_interface *>::const_iterator ai_end = ai_interface::get_all_ai_interfaces().end();

      // check other characters in the region
      for ( ; !hit && ai!=ai_end; ++ai )
      {
        if((*ai)->is_active())
        {
          hit = triggered( (*ai)->get_my_entity() );

          if(hit)
            whodunnit = (*ai)->get_my_entity();
        }
      }
    }

    // should raise signal?
    if( hit )
    {
      if( !occupied )
        raise_signal( ENTER );
    }
    else
    {
      if( occupied )
        raise_signal( LEAVE );
    }

    occupied = hit;
  }
}


void trigger::force_region( stringx id )
{
	region *r = NULL;
	region_node *rn = g_world_ptr->get_the_terrain().find_region(id);
	if (rn)
		r = rn->get_data();
	if (!r)
		error( id + ": unknown region \"" + id + "\" in trigger parameters");

  add_region( r );
	static_regions = true;
}

bool trigger::add_region( region* r )
{
  if ( r && in_regions.insert(r).second )
  {
    r->add( this );
    return true;
  }
  return false;
}


void trigger::set_active( bool torf )
{
  active = torf;
  // if trigger is deactivated while occupied, raise LEAVE signal
  if ( !active && occupied )
  {
    raise_signal( LEAVE );
    occupied = false;
  }
}



point_trigger::point_trigger( const stringx& _id )
: trigger( _id )
{
}

point_trigger::point_trigger( const stringx& _id, const vector3d& p, rational_t r )
: trigger( _id ),
  position( p ),
  radius( r )
{
}

void point_trigger::read(chunk_file &fs)
{
	serial_in(fs, &position);
	serial_in(fs, &radius);

	trigger::read(fs);
}

bool point_trigger::triggered(entity *e)
{
	vector3d v;
	v = e->get_abs_position() - position;
	return (v.length2() < radius*radius);
}

const vector3d& point_trigger::get_abs_position() const
{
  return position;
}

void point_trigger::update_region()
{
	if (!static_regions && radius && in_regions.empty())
	{
		sector *sec = g_world_ptr->get_the_terrain().find_sector(position);
    if ( sec )
//      error( id + ": trigger has been placed outside of world" );
//    else
    {
      // compute my region list starting from the center region
      region::prepare_for_visiting();
      _intersect( sec->get_region() );
      _update_regions();
    }
	}
}


///////////////////////////////////////////////////////////////////////////////
// point_trigger INTERNAL

// add trigger to given region and recurse into any adjacent intersected regions
void point_trigger::_intersect( region_node* r )
{
  // add NEW region to temp list
  region* rg = r->get_data();
  rg->visit();
  trigger_manager::inst()->new_regions.push_back( rg );
  sphere trig_sphere(position,radius);
  // check for intersection with portals leading from this region
  edge_iterator ei = r->begin();
  edge_iterator ei_end = r->end();
  for ( ; ei!=ei_end; ++ei )
  {
    // don't bother with regions we've already visited
    region_node* dest = (*ei).get_dest();
    if ( !dest->get_data()->already_visited() )
    {
      portal* port = (*ei).get_data();
      if ( port->touches_sphere(trig_sphere) )
        _intersect( dest );
    }
  }
}

#include <algorithm>

// remove trigger from regions no longer inhabited (according to NEW list), and
// add trigger to regions newly inhabited
void point_trigger::_update_regions()
{
  trig_region_pset::iterator i,j;
  for ( i=in_regions.begin(); i!=in_regions.end(); )
  {
    region* r = *i;
    if ( find( trigger_manager::inst()->new_regions.begin(), trigger_manager::inst()->new_regions.end(), r )
         == trigger_manager::inst()->new_regions.end() )
    {
      r->remove( this );
      j = i;
      ++j;
      in_regions.erase( i );
      i = j;
    }
    else
      ++i;
  }
  vector<region*>::iterator k;
  for ( k=trigger_manager::inst()->new_regions.begin(); k!=trigger_manager::inst()->new_regions.end(); ++k )
    add_region( *k );
}


#ifdef ECULL
//#pragma fixme( "is this default constructor useful at all? for a read() call? -mkv 4/6/01" )
box_trigger::box_trigger( const stringx& _id )
: trigger( _id )
{
  // empty
}

box_trigger::box_trigger( const stringx& _id, entity* _box )
: trigger( _id ),
  box( _box )
{
#ifdef _DEBUG
  stringx ename = _box->get_name( );
  debug_print( "constructing box trigger tied to '%s'", ename.c_str( ) );
#endif
}

void box_trigger::read( chunk_file& fs )
{
//#pragma todo( "read an entity name from the stream, perhaps? -mkv 4/6/01" )
  debug_print( "box_trigger::read() = STUB" );
}

#if 0
static bool point_within_box( const vector3d& p,
                              const vector3d& l,
                              const convex_box& info )
{

  for( int i = 0; i < 6; i++ ) {
    float dx = p.x - l.x;
    float dy = p.y - l.y;
    float dz = p.z - l.z;
    const vector4d& plane = info.planes[i];

    float s = ( plane.x * dx ) + ( plane.y * dy ) + ( plane.z * dz ) + plane.w;

    if( s >= 0.0f ) {
      return false;
    }

  }

  return true;
}
#endif


bool box_trigger::triggered( entity* e )
{
  assert( box->has_box_trigger_ifc( ) );

  box_trigger_interface* bti = box->box_trigger_ifc();
  convex_box& info = bti->get_box_info( );
  box->update_abs_po( );
  vector3d bpos = box->get_abs_position( );
  vector3d epos = e->get_abs_position( );
  bool inside = info.point_inside( epos, bpos );

  return inside;

#if 0
  // please don't write like this, it's impossible to debug. the compiler will
  // eliminate temporaries if that's what you're worried about. -mkv
  return(box->box_trigger_ifc()->get_box_info().point_inside(e->get_abs_position(), box->get_abs_position()));
#endif
}

const vector3d& box_trigger::get_abs_position( void ) const
{
  return box->get_abs_position( );
}

void box_trigger::update_region( void )
{

  if( !static_regions && in_regions.empty( ) ) {
    terrain& ter = g_world_ptr->get_the_terrain( );
    // get our position from the box
    vector3d box_pos = box->get_abs_position( );
    sector* sec = ter.find_sector( box_pos );

    if( sec ) {
      region::prepare_for_visiting( );
      _intersect( sec->get_region( ) );
      _update_regions( );
    }

  }

}

void box_trigger::_intersect( region_node* r )
{
  region* rg = r->get_data( );
  rg->visit( );
  trigger_manager::inst()->new_regions.push_back( rg );

  assert( box->has_box_trigger_ifc( ) );
  box_trigger_interface* bti = box->box_trigger_ifc( );
  convex_box& info = bti->get_box_info( );
  sphere trig_sphere( box->terrain_position( ), info.bbox.radius( ) );

  edge_iterator ei, ei_end;

  for( ei = r->begin( ), ei_end = r->end( ); ei != ei_end; ei++ ) {
    region_node* dest = (*ei).get_dest( );

    if( !dest->get_data( )->already_visited( ) ) {
      portal* port = (*ei).get_data( );

      if( port->touches_sphere( trig_sphere ) ) {
        _intersect( dest );
      }

    }

  }

}

void box_trigger::_update_regions()
{
  trig_region_pset::iterator i, j;

//#pragma fixme( "when the # of triggers grows, this n^2 search will become expensive. -mkv 4/6/01" )
  for( i = in_regions.begin( ); i != in_regions.end( ); ) {
    region* r = *i;

    if( find( trigger_manager::inst()->new_regions.begin( ), trigger_manager::inst()->new_regions.end( ), r ) == trigger_manager::inst()->new_regions.end( ) ) {
      r->remove( this );
      j = i;
      j++;
      in_regions.erase( i );
      i = j;
    } else {
      i++;
    }

  }

//#pragma fixme( "why do we iterate again? why not just add if !found? -mkv 4/6/01" )
  vector<region*>::iterator k;

  for( k = trigger_manager::inst()->new_regions.begin( ); k != trigger_manager::inst()->new_regions.end( ); k++ ) {
    add_region( *k );
  }

}
#endif

region_trigger::region_trigger( const stringx& _id )
  : trigger( _id )
{
}

void region_trigger::read(chunk_file &fs)
{
	stringx reg_id;
	for ( serial_in(fs,&reg_id); reg_id!=chunkend_label; serial_in(fs,&reg_id) )
    force_region( reg_id );

	trigger::read(fs);
}

bool region_trigger::triggered(entity *e)
{
	region_node *r = e->get_region();
  if ( !r )
    return false;
  if ( in_regions.find( r->get_data() ) == in_regions.end() )
    return false;
	return true;
}

void region_trigger::update_region()
{
}


entity_trigger::entity_trigger( const stringx& _id )
  : trigger(_id),
    last_compute_sector_position_hash(FLT_MAX)
{
}

entity_trigger::entity_trigger( const stringx& _id, entity* e, rational_t r )
  : trigger(_id),
    ent( e ),
    radius( r ),
    last_compute_sector_position_hash(FLT_MAX)
{
}

void entity_trigger::read(chunk_file &fs)
{
	stringx ent_id;
	serial_in(fs, &ent_id);
	ent = entity_manager::inst()->find_entity(entity_id(ent_id.c_str()), IGNORE_FLAVOR);
	serial_in(fs, &radius);

	trigger::read(fs);
}

bool entity_trigger::triggered(entity *e)
{
	vector3d v;
	v = e->get_abs_position() - ent->get_abs_position();
	return (v.length2() < radius*radius);
}

const vector3d& entity_trigger::get_abs_position() const
{
  return ent->get_abs_position();
}

// Hash function for recording last frame's motion to see if we need to recompute sector.
// Designed for 'a' being a vector3d.
#define POSHASH(a) ((a).x+2.7f*(a).y+5.92f*(a).z)

void entity_trigger::update_region()
{
#ifdef ECULL
	if ( !static_regions && ent )
	{
    const vector3d& curpos = ent->get_abs_position();
    rational_t poshash = POSHASH(curpos);
    if ( poshash != last_compute_sector_position_hash )
      {
      last_compute_sector_position_hash = poshash;
      trigger_manager::inst()->new_regions.resize(0);
		  // compare my radius to that of the entity I am attached to
      if ( radius <= ent->terrain_radius() )
      {
        // if I am no larger than the entity, just copy the entity's region list
		    region_node_pset::const_iterator ri = ent->get_regions().begin();
		    region_node_pset::const_iterator ri_end = ent->get_regions().end();
		    for ( ; ri!=ri_end; ri++ )
			    trigger_manager::inst()->new_regions.push_back( (*ri)->get_data() );
      }
      else if ( ent->get_region() )
      {
        // otherwise, compute my own list starting from the entity's center region
        region::prepare_for_visiting();
        _intersect( ent->get_region() );
      }
      _update_regions();
	  }
  }
#endif
}

void trigger::register_signals()
{
#define MAC(label,str)  signal_manager::inst()->insert( str, label );
#include "trigger_signals.h"
#undef MAC
}


///////////////////////////////////////////////////////////////////////////////
// entity_trigger INTERNAL

// add trigger to given region and recurse into any adjacent intersected regions
void entity_trigger::_intersect( region_node* r )
{
  // add NEW region to temp list
  region* rg = r->get_data();
  rg->visit();
  trigger_manager::inst()->new_regions.push_back( rg );
  // check for intersection with portals leading from this region
  sphere trig_sphere( ent->terrain_position(), radius );
  edge_iterator ei = r->begin();
  edge_iterator ei_end = r->end();
  for ( ; ei!=ei_end; ei++ )
  {
    // don't bother with regions we've already visited
    region_node* dest = (*ei).get_dest();
    if ( !dest->get_data()->already_visited() )
    {
      portal* port = (*ei).get_data();
      // intersection of trigger sphere and portal
      if ( port->touches_sphere(trig_sphere) )
        _intersect( dest );
    }
  }
}

// remove trigger from regions no longer inhabited (according to NEW list), and
// add trigger to regions newly inhabited
void entity_trigger::_update_regions()
  {
  trig_region_pset::iterator i,j;
  for ( i=in_regions.begin(); i!=in_regions.end(); )
    {
    region* r = *i;
    if ( find( trigger_manager::inst()->new_regions.begin(), trigger_manager::inst()->new_regions.end(), r ) == trigger_manager::inst()->new_regions.end() )
      {
      r->remove( this );
      j = i;
      j++;
      in_regions.erase( i );
      i = j;
      }
    else
      i++;
    }
  vector<region*>::iterator k;
  for ( k=trigger_manager::inst()->new_regions.begin(); k!=trigger_manager::inst()->new_regions.end(); k++ )
    add_region( *k );
  }
