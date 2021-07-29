////////////////////////////////////////////////////////////////////////////////

// light.cpp
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.

// light source class

////////////////////////////////////////////////////////////////////////////////
#include "global.h"

//!#include "character.h"
#include "light.h"
#include "terrain.h"
#include "oserrmsg.h"
#include "sphere.h"

enum { CUTOFF_FACTOR=16 };

////////////////////////////////////////////////////////////////////////////////
//  light_properties
light_properties::light_properties()
  : flavor( LIGHT_FLAVOR_POINT ),
    diffuse_color( 1.0f,1.0f,1.0f,1.0f ),
    near_range( 1.0f ),
    cutoff_range( CUTOFF_FACTOR )
{
  additive_color = color(0,0,0,0);
  recompute_range();
  //recip_range = 1/near_range;
  //cutoff_squared = cutoff_range*cutoff_range;
  flags = 0;
}

#if defined(TARGET_XBOX) || defined(TARGET_GC)
light_properties::light_properties( light_flavor_t _flavor,
                                    color _color,
                                    color _additive_color,
                                    rational_t _near_range,
                                    rational_t _cutoff_range, 
                                    u_int _lightcat )
#else
light_properties::light_properties( light_flavor_t _flavor,
                                    color _color,
                                    color _additive_color,
                                    rational_t _near_range,
                                    rational_t _cutoff_range, 
									u_int _lightcat = NGLMESH_LIGHTCAT_1 )
#endif /* TARGET_XBOX JIV DEBUG */
  : flavor(_flavor),
    diffuse_color(_color),
    additive_color(_additive_color),
    near_range(_near_range),
    cutoff_range(_cutoff_range), 
    lightcat(_lightcat)
{
  recompute_range();
  //recip_range = 1/near_range;
  //cutoff_squared = cutoff_range*cutoff_range;
  flags = 0;
}

void light_properties::set_near_range(rational_t r)
{
  near_range = r;
  //recip_range = 1/r;
  recompute_range();
}


void light_properties::set_cutoff_range(rational_t r)
{
  cutoff_range = r;
  //cutoff_squared = r*r;
  recompute_range();
}

rational_t light_properties::get_influence(rational_t dist) const // attenuation at given distance (1.0f to 0.0f)
{
  if (flavor==LIGHT_FLAVOR_DIRECTIONAL || dist <= near_range)
    return 1.0f;
  if (dist>=cutoff_range)
    return 0.0F;
  return sqr(1.0f - (dist - near_range) * recip_cutoff_minus_near); // squaring gives attenuation a nice curve
}

rational_t light_properties::get_brightness(rational_t dist) const
{
  rational_t scale = get_influence(dist);
  if (!scale) return 0.0f;
  rational_t b=__fabs(diffuse_color.get_red())+__fabs(diffuse_color.get_green())+__fabs(diffuse_color.get_blue());
  //b+=__fabs(additive_color.get_red())+__fabs(additive_color.get_green())+__fabs(additive_color.get_blue());
  return b * scale;
}

void serial_in( chunk_file& fs, light_properties* lp )
{
  chunk_flavor cf;
  for ( serial_in(fs,&cf); cf!=CHUNK_END; serial_in(fs,&cf) )
  {
    if ( chunk_flavor("type") == cf )
    {
      unsigned short flav;
      serial_in( fs, &flav );
      lp->flavor = (light_flavor_t)flav;
    }
    else if ( chunk_flavor("color") == cf )
    {
      serial_in( fs, &lp->diffuse_color );
    }
    else if ( chunk_flavor("addcolor") == cf )
    {
      serial_in( fs, &lp->additive_color );
    }
    /*else if ( chunk_flavor("kattn") == cf )
    {
      float garbage;
      serial_in( fs, &garbage );
    }
    else if ( chunk_flavor("lattn") == cf )
    {
      float garbage;
      serial_in( fs, &garbage );
    }
    else if ( chunk_flavor("qattn") == cf )
    {
      float garbage;
      serial_in( fs, &garbage );
    }
    else if ( chunk_flavor("range") == cf )
    {
      // old format light
      serial_in( fs, &lp->near_range );
      lp->cutoff_range = lp->near_range*CUTOFF_FACTOR;
      recompute_range();
      //lp->recip_range = 1.0f / lp->near_range;
      //lp->cutoff_squared = lp->cutoff_range*lp->cutoff_range;
    }*/
    else if ( chunk_flavor("nearr") == cf )
    {
      serial_in( fs, &lp->near_range );
      lp->cutoff_range = lp->near_range*CUTOFF_FACTOR;
      if (lp->near_range * 1.001f >= lp->cutoff_range)
        lp->cutoff_range = lp->near_range * 1.001f;
      lp->recompute_range();
      //lp->recip_range = 1.0f / lp->near_range;
      //lp->cutoff_squared = lp->cutoff_range*lp->cutoff_range;
    }
    else if ( chunk_flavor("cutoff") == cf )
    {
      //float garbage;
      serial_in( fs, &lp->cutoff_range );
      if (lp->near_range * 1.001f >= lp->cutoff_range)
        lp->near_range = lp->near_range / 1.001f;
      //lp->cutoff_squared = lp->cutoff_range*lp->cutoff_range;
      lp->recompute_range();
    }
    else if ( chunk_flavor("flags") == cf )
    {
      serial_in( fs, &lp->flags );
    }
    else
    {
      stringx composite = stringx("Unknown chunk type ") + cf.to_stringx() + (" in ")+fs.get_name();
      error(composite.c_str());
    }
  }
}


////////////////////////////////////////////////////////////////////////////////

light_source::light_source( const entity_id& _id, unsigned int _flags )
  :   entity( _id, ENTITY_LIGHT_SOURCE, _flags )
{
  properties = NEW light_properties;
}


light_source::light_source( const entity_id& _id,
                            entity_flavor_t _flavor,
                            unsigned int _flags )
  :   entity( _id, _flavor, _flags )
{
  properties = NEW light_properties;
}


light_source::light_source( const light_properties& _properties,
                            entity* _parent,
                            const entity_id& _id )
  :   entity( _id, ENTITY_LIGHT_SOURCE )
{
  properties = NEW light_properties( _properties );
  if (_parent != NULL)
    link_ifc()->set_parent( _parent );
}


light_source::~light_source()
{
  // any descendant of entity that overloads add_me_to_region() and
  // remove_me_from_region() (see entity.h) needs to call this cleanup method
  // in its destructor; the reason for this is that if we wait and let the
  // parent class (e.g., entity) call this method, it will end up using the
  // parent's version of remove_me_from_region() (which makes sense when you
  // think about it, since by that point the descendant class has already been
  // destroyed)
  remove_from_terrain();
  if ( properties )
    delete properties; // remove_from_terrain needs this
}


///////////////////////////////////////////////////////////////////////////////
// NEWENT File I/O
///////////////////////////////////////////////////////////////////////////////

light_source::light_source( chunk_file& fs,
                            const entity_id& _id,
                            entity_flavor_t _flavor,
                            unsigned int _flags )
  :   entity( _id, _flavor, _flags )
{
  chunk_flavor cf;
  properties = NEW light_properties;
  serial_in( fs, properties );
  serial_in( fs, &cf );
  if ( cf != CHUNK_END )
    error( fs.get_name() + ": unexpected chunk '" + cf.to_stringx() + "' in light node" );
}


///////////////////////////////////////////////////////////////////////////////
// Old File I/O
///////////////////////////////////////////////////////////////////////////////

light_source::light_source( const stringx& filename,
                            const entity_id& _id,
                            unsigned int _flags )
  :   entity( _id, ENTITY_LIGHT_SOURCE, _flags )
{
  load( filename + ".ent" );
}


void light_source::load( const stringx& filename )
{
  if (!properties)
    properties = NEW light_properties;
  light_properties* lp = properties;
  chunk_file fs;
  fs.open(filename);
  chunk_flavor cf;
  serial_in( fs, &cf );
  assert( cf == chunk_flavor("LIGHT"));
  for(;;)
  {
    serial_in( fs, &cf );
    if( cf == chunk_flavor("flavor"))
    {
      stringx flavor_name;
      serial_in( fs, &flavor_name );
      if(flavor_name=="POINT")
        lp->flavor=LIGHT_FLAVOR_POINT;
      else if(flavor_name=="DIRECTIONAL")
        lp->flavor=LIGHT_FLAVOR_DIRECTIONAL;
      else
      {
        stringx composite = stringx("Unknown light flavor in file")+fs.get_name();
        error(composite.c_str());
      }
    }
    else if( cf == chunk_flavor("color"))
    {
      serial_in( fs, &lp->diffuse_color );
    }
    else if ( chunk_flavor("addcolor") == cf )
    {
      serial_in( fs, &lp->additive_color );
    }
    /*else if( cf == chunk_flavor("kattn"))
    {
      float garbage;
      serial_in( fs, &garbage );
    }
    else if( cf == chunk_flavor("lattn"))
    {
      float garbage;
      serial_in( fs, &garbage );
    }
    else if( cf == chunk_flavor("qattn"))
    {
      float garbage;
      serial_in( fs, &garbage );
    }
    else if( cf == chunk_flavor("range"))
    {
      serial_in( fs, &lp->near_range );
      lp->cutoff_range = lp->near_range*CUTOFF_FACTOR;
      lp->recompute_range();
      //lp->recip_range = 1 / lp->near_range;
      //lp->cutoff_squared = lp->cutoff_range*lp->cutoff_range;
    }*/
    else if( cf == chunk_flavor("nearr"))
    {
      serial_in( fs, &lp->near_range );
      if (lp->near_range * 1.001f >= lp->cutoff_range)
        lp->cutoff_range = lp->near_range * 1.001f;
      lp->recompute_range();
      //lp->recip_range = 1 / lp->near_range;
    }
    else if( cf == chunk_flavor("cutoff"))
    {
      serial_in( fs, &lp->cutoff_range );
      if (lp->near_range * 1.001f >= lp->cutoff_range)
        lp->near_range = lp->cutoff_range / 1.001f;
      lp->recompute_range();
      //lp->cutoff_squared = lp->cutoff_range*lp->cutoff_range;
    }
    else if( cf == chunk_flavor("flags"))
    {
      stringx flagstr;
      serial_in( fs, &flagstr );
      if( flagstr.find( "AFFECTS_TERRAIN" )!=stringx::npos )
        lp->flags |= light_properties::AFFECTS_TERRAIN;
    }
    else if( cf == CHUNK_END )
    {
      break;
    }
    else
    {
      stringx composite = stringx("Unknown chunk type")+cf.to_stringx() +(" in ")+fs.get_name();
      error(composite.c_str());
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
// Instancing
///////////////////////////////////////////////////////////////////////////////

entity* light_source::make_instance( const entity_id& _id,
                                     unsigned int _flags ) const
{
  light_source* newls = NEW light_source( _id, _flags );
  newls->copy_instance_data( *this );
  return (entity*)newls;
}


void light_source::copy_instance_data( const light_source& b )
{
  entity::copy_instance_data( b );

  if ( b.properties )
    if (!properties)
      properties = NEW light_properties( b.get_properties() );
    else
      *properties = b.get_properties();
}


///////////////////////////////////////////////////////////////////////////////
// Misc.
///////////////////////////////////////////////////////////////////////////////

// these virtual functions allow types descended from entity to be
// recognized when adding them to regions, so that the region class can
// maintain lists of different entity types as desired
void light_source::add_me_to_region( region* r )
{
  r->add( this );
}

void light_source::remove_me_from_region( region* r )
{
  r->remove( this );
}

rational_t light_source::terrain_radius() const
{
  // it used to be cutoff range, which made sure that anywhere you could be
  // affected by a light you would be, which required more use of force_region
  // by the artists to make walls actually occlude light
  return get_properties().get_cutoff_range() * 0.5f;
}

rational_t light_source::get_dist(const vector3d& apos) const
{
  if( get_properties().get_flavor() != LIGHT_FLAVOR_DIRECTIONAL )
    return (apos-get_abs_position()).length();
  return 0.0F;
}

rational_t light_source::get_dist(const sphere& abound) const
{
  if( get_properties().get_flavor() != LIGHT_FLAVOR_DIRECTIONAL )
  {
    float d2 = (abound.get_center()-get_abs_position()).length2();
    if (d2 > sqr(abound.get_radius()))
      return __fsqrt(d2) - abound.get_radius();
  }
  return 0.0f;
}

void light_source::frame_advance(time_value_t t)
{}
