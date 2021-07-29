////////////////////////////////////////////////////////////////////////////////

// light.h
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED. 

// light source class

////////////////////////////////////////////////////////////////////////////////

#ifndef LIGHT_H
#define LIGHT_H

#include "entity.h"
#include "color.h"

class light_source;
class vector3d;
class sphere;

enum light_flavor_t
{
  LIGHT_FLAVOR_POINT,            // omni light (treated as parallelpoint for critters)
  LIGHT_FLAVOR_SPOT,
  LIGHT_FLAVOR_DIRECTIONAL,      // ignores position:  things get lit by facing...typical light
  LIGHT_FLAVOR_PARALLELPOINT,    // directional light that has a location and faces center of object
  NUM_LIGHT_FLAVORS
};

class light_properties 
{
public:
  light_properties();
  light_properties( light_flavor_t _flavor,
                color _color,
                color _additive_color,
                rational_t _near_range,
                rational_t _cutoff_range, 
				u_int _lightcat = NGLMESH_LIGHTCAT_1);

  light_flavor_t get_flavor() const               { return flavor; }
  const color&   get_color() const                { return diffuse_color; }
  color32        get_color32() const              { return diffuse_color.to_color32(); }
  const color&   get_additive_color() const				{ return additive_color; }
  color32        get_additive_color32() const     { return additive_color.to_color32(); }
  rational_t     get_near_range() const           { return near_range; }
  rational_t     get_cutoff_range() const         { return cutoff_range; }
  //rational_t     get_recip_range() const          { return recip_range; }
  u_int			 get_lightcat() const			  { return lightcat; }

  void set_flavor(light_flavor_t f)         { flavor = f; }
  void set_color(const color& c)            { diffuse_color = c; }
  void set_additive_color(const color& c)   { additive_color = c; }
  void set_near_range(rational_t r);
  void set_cutoff_range(rational_t r);
  void set_lightcat(const u_int& l) 		{ lightcat = l; }

  rational_t get_influence(rational_t dist) const; // attenuation at given distance (1.0f to 0.0f)
  rational_t get_brightness(rational_t dist) const; // brightness at given distance

  enum
  {
    AFFECTS_TERRAIN=1
  };
  bool affects_terrain() const { return flags&AFFECTS_TERRAIN; }
protected:
  // what sort of light?
  light_flavor_t flavor;
  // what color light?
  color          diffuse_color;
  //color32        diffuse_color32;  // for integer math
  color					 additive_color;
  //color32        additive_color32;
  // three kinds of light falloff over distance

  rational_t     near_range;
  rational_t     cutoff_range;
  rational_t     recip_cutoff_minus_near;
  //rational_t     recip_range;
  //rational_t     cutoff_squared;

  u_int lightcat;
  unsigned flags;

  void recompute_range() // call whenever ranges change
  {
    assert(near_range>=0.0f);
#ifdef WEENIEASSERT  // this triggers every time KSPS starts so it's gon
    assert(cutoff_range>near_range);
#endif
    if (cutoff_range<=near_range*1.001f)
      recip_cutoff_minus_near = 1e10f;
    else
      recip_cutoff_minus_near = 1.0f/(cutoff_range-near_range);
  }

  friend class light_source;
  friend void serial_in( chunk_file& cf, light_properties* lp );
};

class light_source : public entity
{
public:
  light_source( const entity_id& _id, unsigned int _flags );

  light_source( const entity_id& _id = ANONYMOUS,
                entity_flavor_t _flavor = ENTITY_LIGHT_SOURCE,
                unsigned int _flags = 0 );

  light_source( const light_properties& _properties,
                entity* parent,
                const entity_id& _id = ANONYMOUS );

  virtual ~light_source();

/////////////////////////////////////////////////////////////////////////////
// entity class identification
public:
  virtual bool is_a_light_source() const { return true; }

// NEWENT File I/O
public:
  light_source( chunk_file& fs,
                const entity_id& _id,
                entity_flavor_t _flavor = ENTITY_LIGHT_SOURCE,
                unsigned int _flags = 0 );

// Old File I/O
public:
  light_source( const stringx& filename,
                const entity_id& _id = ANONYMOUS,
                unsigned int _flags = 0 );
private:
  void load( const stringx& filename );

// Instancing
public:
  virtual entity* make_instance( const entity_id& _id,
                                 unsigned int _flags ) const;
protected:
  void copy_instance_data( const light_source& b );

// Misc.
public:
  const light_properties& get_properties() const { assert(properties); return *properties; }
  bool has_properties() const { return (properties != NULL); }

  // virtual functions allow descendants to assert different notions of
  // position and radius for the purposes of computing entity's terrain
  // locale (i.e., sector and region(s) occupied)
  virtual rational_t terrain_radius() const;

  // these virtual functions allow types descended from entity to be
  // recognized when adding them to regions, so that the region class can
  // maintain lists of different entity types as desired
  virtual void add_me_to_region( region* r );
  virtual void remove_me_from_region( region* r );

  virtual void frame_advance(time_value_t t);

  virtual const color& 	get_color() const                  { return properties->get_color(); }
  virtual void   				set_color(const color& c)          { properties->set_color(c); }

  virtual const color& 	get_additive_color() const         { return properties->get_additive_color(); }
  virtual void   				set_additive_color(const color& c) { properties->set_additive_color(c); }

  virtual rational_t get_near_range() const         { return properties->get_near_range(); }
  virtual void       set_near_range(rational_t _r)  { properties->set_near_range(_r); }
  
  virtual u_int		 get_lightcat() const         { return properties->get_lightcat(); }
  virtual void       set_lightcat(u_int _r)		  { properties->set_lightcat(_r); }
  
  virtual rational_t get_cutoff_range() const       { return properties->get_cutoff_range(); }
  virtual void       set_cutoff_range(rational_t _r) { properties->set_cutoff_range(_r); }

  virtual rational_t get_dist(const vector3d& apos) const;
  virtual rational_t get_dist(const sphere& abound) const;
// Data
private:
  light_properties* properties;
};
  

#endif
