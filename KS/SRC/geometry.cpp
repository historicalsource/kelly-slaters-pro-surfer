////////////////////////////////////////////////////////////////////////////////
/*
  geometry.cpp

  collection of useful geometry computation routines

*/
////////////////////////////////////////////////////////////////////////////////
#include "global.h"

#include "geometry.h"


rational_t inner_angle(rational_t a, rational_t b, rational_t c)
  {
  rational_t thecos = (a*a+b*b-c*c)/(2*a*b);
  //assert(__fabs(thecos)<=1);
  if (thecos>1)
    thecos = 1;
  else if (thecos<-1)
    thecos = -1;
  rational_t theangle = fast_acos(thecos);
  return theangle;
  }


rational_t opposite_side(rational_t a, rational_t angle, rational_t b)
  {
  return __fsqrt(a*a+b*b-2*a*b*fast_cos(angle));
  }
