// random.h
// convenience wrappers around the rand() function

#ifndef RANDOM_H
#define RANDOM_H

//#include "profiler.h"
#include <stdlib.h>

//extern profiler_timer proftimer_random;

#ifdef RAND_MAX
#undef RAND_MAX
#endif

#define RAND_MAX    32767

class Random
{
public:

  Random()
  {
    rand_next = (int)master_clock::inst()->elapsed();
  }

  Random(int seed)
  {
    rand_next = seed;
  }

  // Returns the seed that would put the random number stream back to its current state
  unsigned int getSeed()
  {
    return rand_next;
  }

  // Initializes with a random seed (based on the time since the program began) and return that seed
  unsigned int srand()
  {
    rand_next = (unsigned int)master_clock::inst()->elapsed();
    return rand_next;
  }

  // Set the random number seed to a specific value
  void srand(unsigned int seed)
  {
    rand_next = seed;
  }

  // Returns an int from 0 up to, and including, RAND_MAX
  int rand()
  {
    unsigned int r = NextRand();
    return r;
  }

  // Returns an int from 0 up to, but NOT including, 'end'
  int rand(int end)
  {
    assert(end > 0);
    unsigned int r = NextRand();
    return r % end;
  }

  // Returns an int from 'begin' up to, but NOT including, 'end'
  int rand(int begin, int end)
  {
    assert(begin < end);
    unsigned int r = NextRand();
    return r % (end-begin) + begin;
  }

  // Returns a float from 0.0 up to, and including, 1.0
  float randf()
  {
    unsigned int r = NextRand();
    return r * (1.0f/RAND_MAX);
  }

  // Returns a float from 0.0 up to, and including, end
  float randf(float end)
  {
    unsigned int r = NextRand();
    return r * (1.0f/RAND_MAX)*end;
  }

  // Returns a float from begin up to, and including, end
  float randf(float begin, float end)
  {
    unsigned int r = NextRand();
    return r * (1.0f/RAND_MAX)*(end-begin) + begin;
  }

private:
  unsigned int rand_next;

  unsigned int NextRand()
  {
    rand_next = rand_next*1103515245 + 12345;
    return (rand_next>>16)&RAND_MAX;
  }
};


extern Random *g_random_ptr;
extern Random *g_random_r_ptr;

////////////////////////////////////////////////////////////////////////////////////////
// Wrapper functions to produce random numbers that do not need to be reproduced
////////////////////////////////////////////////////////////////////////////////////////
// Returns a float from 0.0 up to, and including, 1.0
inline float random()
{
//  START_PROF_TIMER( proftimer_random );
  float r = g_random_ptr->randf();
//  STOP_PROF_TIMER( proftimer_random );
  return r;
}

// Returns a float from 0.0 up to, and including, end
inline float random(float end)
{
//  START_PROF_TIMER( proftimer_random );
  float r = g_random_ptr->randf(end);
//  STOP_PROF_TIMER( proftimer_random );
  return r;
}

// Returns a float from begin up to, and including, end
inline float random(float begin,float end)
{
//  START_PROF_TIMER( proftimer_random );
  float r = g_random_ptr->randf(begin, end);
//  STOP_PROF_TIMER( proftimer_random );
  return r;
}

// Returns an int from 0 up to, but NOT including, 'end'
inline int random(int end)
{
//  START_PROF_TIMER( proftimer_random );
  int i = g_random_ptr->rand(end);
//  STOP_PROF_TIMER( proftimer_random );
  return i;
}

// Returns an int from 'begin' up to, but NOT including, 'end'
inline int random(int begin,int end)
{
//  START_PROF_TIMER( proftimer_random );
  int i = g_random_ptr->rand(begin, end);
//  STOP_PROF_TIMER( proftimer_random );
  return i;
}


////////////////////////////////////////////////////////////////////////////////////////
// Wrapper functions to produce random numbers that will be reproduced in replay mode
////////////////////////////////////////////////////////////////////////////////////////
// Returns a float from 0.0 up to, and including, 1.0
inline float random_r()
{
//  START_PROF_TIMER( proftimer_random );
  float r = g_random_r_ptr->randf();
//  STOP_PROF_TIMER( proftimer_random );
  return r;
}

// Returns a float from 0.0 up to, and including, end
inline float random_r(float end)
{
//  START_PROF_TIMER( proftimer_random );
  float r = g_random_r_ptr->randf(end);
//  STOP_PROF_TIMER( proftimer_random );
  return r;
}

// Returns a float from begin up to, and including, end
inline float random_r(float begin,float end)
{
//  START_PROF_TIMER( proftimer_random );
  float r = g_random_r_ptr->randf(begin, end);
//  STOP_PROF_TIMER( proftimer_random );
  return r;
}

// Returns an int from 0 up to, but NOT including, 'end'
inline int random_r(int end)
{
//  START_PROF_TIMER( proftimer_random );
  int i = g_random_r_ptr->rand(end);
//  STOP_PROF_TIMER( proftimer_random );
  return i;
}

// Returns an int from 'begin' up to, but NOT including, 'end'
inline int random_r(int begin,int end)
{
//  START_PROF_TIMER( proftimer_random );
  int i = g_random_r_ptr->rand(begin, end);
//  STOP_PROF_TIMER( proftimer_random );
  return i;
}


#define PLUS_MINUS_ONE        (random(-1.0f, 1.0f))
#define ZERO_TO_ONE           (random())

#define VARIANCE(a,b)         (a + random(-b, b))
#define POS_VARIANCE(a,b)     (a + random(b))
#define NEG_VARIANCE(a,b)     (a - random(b))


#define PLUS_MINUS_ONE_R      (random_r(-1.0f, 1.0f))
#define ZERO_TO_ONE_R         (random_r())

#define VARIANCE_R(a,b)       (a + random_r(-b, b))
#define POS_VARIANCE_R(a,b)   (a + random_r(b))
#define NEG_VARIANCE_R(a,b)   (a - random_r(b))

#endif // RANDOM_H