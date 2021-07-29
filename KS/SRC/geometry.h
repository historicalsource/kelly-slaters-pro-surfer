#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "algebra.h"

// inner_angle
//  Returns the positive angle between sides a and b in an (a,b,c) triangle.
rational_t inner_angle(rational_t a, rational_t b, rational_t c);

// opposite_side
//   Returns the length of the third side of an SAS = (a, angle, c) triangle.
// This is just the law of Cosines.
rational_t opposite_side(rational_t a, rational_t angle, rational_t b);

#endif