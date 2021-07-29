#ifndef _LENSFLARE_H_
#define _LENSFLARE_H_

#include "global.h"

#include "entity.h"

class mat_fac;

class lensflare : public entity
{
protected:
  void init();
  virtual void copy_instance_data( const lensflare& b );

  float brightness; // max range for amount (default=1.0)
  float speed;      // max rate at which amount can change per second - response rate when line-of-sight changes
  float farrange;   // at what distance the flare shows up.

  float amount;     // used to scale the light's alpha and to handle fading based on speed
  float target;     // value for 'amount' we're currently after.

  struct flare
  {
    flare() : offset( 0.0f ), size( 0.0f ), texture( NULL ), col( color( 1, 1, 1, .5f ) ) {}
    float offset;     // offsets from light source toward camera (0.0 == source, 1.0 == center of screen, -1.0 == behind source)
    float size;       // size of flare
    mat_fac* texture; // texture (additive is forced)
    color col;        // flare color
  };

  // number of flares. max=8.
  int nflares;
  flare flares[8];

  // only checks for line-of-sight about 4 or 8 times per second (based on quality)
  float los_time;
  float los_freq;

public:
  lensflare(const entity_id& _id, entity_flavor_t _flavor, unsigned int _flags,
                     float _speed, float _losfreq, float _farrange,
                     int _nflares, stringx *textures, color *theColors, float *offsets, float *sizes);

  lensflare( const lensflare& e );
  lensflare( const entity_id& _id, unsigned int _flags = 0 );
  lensflare( chunk_file& fs, const entity_id& _id, entity_flavor_t _flavor = ENTITY_LENSFLARE, unsigned int _flags = 0 );
  virtual ~lensflare();

  virtual entity* make_instance( const entity_id& _id, unsigned int _flags ) const;

  virtual void frame_advance(time_value_t t);
  virtual void render( camera* camera_link, rational_t detail, render_flavor_t flavor, rational_t entity_translucency_pct );
  virtual render_flavor_t render_passes_needed() const { return RENDER_TRANSLUCENT_PORTION; }
  virtual bool possibly_active() const { return true; }
};

#endif
