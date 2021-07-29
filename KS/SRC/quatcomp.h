#ifndef QUATCOMP_H
#define QUATCOMP_H

class quatcomp
{
  // don't change this without also changing the ANMX format
public:
  int16 a;
  int16 b;
  int16 c;
  int16 d;

  bool operator==( const quatcomp& q ) const 
  {
    return ( ( a == q.a ) && ( b == q.b ) && ( c == q.c ) && ( d == q.d ) );
  }
};

#endif