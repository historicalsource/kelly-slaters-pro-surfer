// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#include "xb_math.h"

bool internal_fast_sin_cos_approx = false;

void fast_sin_cos_approx( fp rads, fp* sinx, fp* cosx )
{
  *sinx = fp(sin(rads));
  *cosx = fp(cos(rads));
}

rational_t fast_acos_lookup_table[_FAST_ACOS_LOOKUP_SIZE+1];
void init_fast_acos_table()
{
  static bool table_initted = false;

  if(!table_initted)
  {
    table_initted = true;
    for(int i=0; i<=_FAST_ACOS_LOOKUP_SIZE; ++i)
      fast_acos_lookup_table[i] = fp(acos(lookup_to_cos(i)));
  }
}

rational_t fast_sin_lookup_table[_FAST_SIN_LOOKUP_SIZE+1];
void init_fast_sin_table()
{
  static bool table_initted = false;

  if(!table_initted)
  {
    table_initted = true;
    for(int i=0; i<=_FAST_SIN_LOOKUP_SIZE; ++i)
      fast_sin_lookup_table[i] = fp(sin(_FAST_SIN_LOOKUP_THRESHOLD*((float)i / (float)_FAST_SIN_LOOKUP_SIZE)));

/*
    for(int i=0; i<=_FAST_SIN_LOOKUP_SIZE; ++i)
      fast_sin_lookup_table[i] = sin(_FAST_SIN_PI_DIV_2*((float)i / (float)_FAST_SIN_LOOKUP_SIZE));
*/
  }
}
