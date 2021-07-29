// hyperplane.h
#ifndef _HYPERPLANE_H
#define _HYPERPLANE_H


#include "hwmath.h"

// TEMPLATE CLASS hyperplane

enum split_enum_t
{
    SPLITT_Positive,
    SPLITT_Negative,
    SPLITT_Spanning,
    SPLITT_On
};

template <class spacial_t>
class hyperplane
{
  // Types
  public:
    typedef hyperplane<spacial_t> _Myt;

    enum division_t
    {
      HP_ON,
      BACK,
      FRONT,
      SPLIT,
      INVALID
    };

  // Data
  protected:
    rational_t d;
    spacial_t normal;

  // Constructors
  public:
    hyperplane() {}

    hyperplane(const hyperplane& b)
    :   normal(b.normal)
    {
      d = b.d;
    }

    hyperplane(const spacial_t& c,const spacial_t& n)
    :   normal(n)
    {
      normal.normalize();
      d = dot(normal,c);
    }

  // Operators
  public:
    bool operator<(const spacial_t& b) const { return (distance(b) > 0); }
    bool operator>(const spacial_t& b) const { return (distance(b) < 0); }

  // Methods
  public:
    spacial_t get_center() const { return d*normal; }
    const spacial_t& get_normal() const { return normal; }
    const rational_t get_offset() const { return d; }

    rational_t distance(const spacial_t& p) const     { return dot(p,normal)-d; }
    rational_t abs_distance(const spacial_t& p) const { return rational_t(__fabs(distance(p))); }
    spacial_t get_closest_point(const spacial_t& p) const { return p-distance(p)*normal; }

    bool is_same_plane(const _Myt& b) const
    {
      return (d==b.d && normal==b.normal);
    }

    bool is_neg_plane(const _Myt& b) const
    {
      return (d==-b.d && normal==-b.normal);
    }

    bool intersect(const spacial_t& p) const
    {
      return (abs_distance(p) < EPSILONF);
    }

    division_t division(const spacial_t& p) const
    {
      rational_t d = distance(p);
      if (d <= -EPSILONF)
        return BACK;
      if (d >= EPSILONF)
        return FRONT;
      return HP_ON;
    }

    // calculate the point of intersection across a hyperplane
    split_enum_t split(const spacial_t &p0, const spacial_t &p1, rational_t &d0, rational_t &d1, spacial_t &pi, spacial_t &n)
    {
      split_enum_t type = SPLITT_On;
      spacial_t    d;
      rational_t   t;

      d0 = distance(p0);
      d1 = distance(p1);
      if(d0 >= EPSILONF)
      {
        type = SPLITT_Positive;
      }
      else if(d0 <= -EPSILONF)
      {
        type = SPLITT_Negative;
      }
      if(d1 >= EPSILONF)
      {
        if(type == SPLITT_On) type = SPLITT_Positive;
        else if(type == SPLITT_Negative) type = SPLITT_Spanning;
      }
      else if(d1 <= -EPSILONF)
      {
        if(type == SPLITT_On) type = SPLITT_Negative;
        else if(type == SPLITT_Positive) type = SPLITT_Spanning;
      }
      if(type == SPLITT_Spanning)
      {
        d = p1 - p0;
        t = -d0 / dot(d, normal);
        pi = d * t + p0;
      }
      return type;
    }

    split_enum_t split(const vector3d &p, const rational_t r, spacial_t &n)
    {
      rational_t d = distance(p);
      if(d >= r)
        return SPLITT_Positive;
      if(d <= -r)
        return SPLITT_Negative;
      n = normal;
      return SPLITT_On;
    }

    split_enum_t split(const capsule &cap, capsule &pcap, capsule &ncap, spacial_t &n)
    {
      split_enum_t type = SPLITT_On;
      spacial_t    d, pi;
      rational_t   d0, d1, t;

      d0 = distance(cap.base);
      d1 = distance(cap.end);
      if((d0 + cap.radius) >= EPSILONF)
      {
          type = SPLITT_Positive;
      }
      else if((d0 - cap.radius) <= -EPSILONF)
      {
          type = SPLITT_Negative;
      }
      if((d1 + cap.radius) >= EPSILONF)
      {
          if(type == SPLITT_On) type = SPLITT_Positive;
          else if(type == SPLITT_Negative) type = SPLITT_Spanning;
      }
      else if((d1 - cap.radius) <= -EPSILONF)
      {
          if(type == SPLITT_On) type = SPLITT_Negative;
          else if(type == SPLITT_Positive) type = SPLITT_Spanning;
      }
      switch(type)
      {
        case SPLITT_On:
          // both the base and end are within radius distance so generate two NEW mini-capsules
          break;
        case SPLITT_Positive:
          // at least one end is positive, the other may be "on"
          break;
        case SPLITT_Negative:
          // at least one end is negative, the other may be "on"
          break;
        case SPLITT_Spanning:
          // one end is positive and the other is negative
          d = cap.end - cap.base;
          t = -d0 / dot(d, normal);
          pi = d * t + cap.base;
          n = normal;
          break;
      }
      return type;
    }

    void invert_normal() { normal = -normal; }

    void invert()
    {
      d=-d;
      normal = -normal;
    }
};


#endif  // _HYPERPLANE_H
