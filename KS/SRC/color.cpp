#include "global.h"

#include "color.h"

/*
// basic colors
const color32 color32_white   ( 255,  255,  255,  255 );
const color32 color32_black   ( 0,  0,    0,    255 );
const color32 color32_grey    ( 127,  127,  127,  255 );
const color32 color32_red     ( 255,  0,    0,    255 );
const color32 color32_green   ( 0,    255,  0,    255 );
const color32 color32_blue    ( 0,    0,    255,  255 );
const color32 color32_cyan    ( 0,    255,  255,  255 );
const color32 color32_magenta ( 255,  0,    255,  255 );
const color32 color32_yellow  ( 255,  255,  0,    255 );
*/

color32 saturated_add(color32 c1,color32 c2)
{
  color32 result;
  int adder;
  adder = c1.c.b + c2.c.b;
  if(adder>255)
    adder=255;
  result.c.b = adder;
  adder = c1.c.r + c2.c.r;
  if(adder>255)
    adder=255;
  result.c.r = adder;
  adder = c1.c.g + c2.c.g;
  if(adder>255)
    adder=255;
  result.c.g = adder;
  adder = c1.c.a + c2.c.a;
  if(adder>255)
    adder=255;
  result.c.a = adder;
  return result;
}

color32 color32::operator*=( const vector4d& rgbaf )
{
  float temp;
  temp = c.r * rgbaf[0];
  temp = max( 0.0f,   temp );
  temp = min( 255.0f, temp );
  c.r = (uint8)temp;
  temp = c.g * rgbaf[1];
  temp = max( 0.0f,   temp );
  temp = min( 255.0f, temp );
  c.g = (uint8)temp;
  temp = c.b * rgbaf[2];
  temp = max( 0.0f,   temp );
  temp = min( 255.0f, temp );
  c.b = (uint8)temp;
  temp = c.a * rgbaf[3];
  temp = max( 0.0f,   temp );
  temp = min( 255.0f, temp );
  c.a = (uint8)temp;
  return *this;
}
