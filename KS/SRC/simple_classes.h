#ifndef _SIMPLE_CLASSES_H_
#define _SIMPLE_CLASSES_H_

#include "global.h"

class simple_timer
{
protected:
  time_value_t time;

public:
  simple_timer(time_value_t t = 0.0f)
  {
    time = t;
  }
  simple_timer(const simple_timer &b)
  {
    time = b.time;
  }

  inline time_value_t get_time() const
  {
    return(time);
  }
  inline time_value_t get() const
  {
    return(time);
  }
  inline operator const time_value_t&() const
  {
    return(time);
  }

  inline simple_timer &operator=(const time_value_t t)
  {
    time = t;
    return(*this);
  }
  inline simple_timer &operator=(const simple_timer &b)
  {
    time = b.time;
    return(*this);
  }
  inline simple_timer &operator+=(const time_value_t t)
  {
    time += t;
    return(*this);
  }
  inline simple_timer &operator-=(const time_value_t t)
  {
    time -= t;
    if(time < 0.0f)
      time = 0.0f;

    return(*this);
  }
  inline simple_timer &operator*=(const rational_t t)
  {
    time *= t;
    return(*this);
  }
  inline simple_timer &operator/=(const rational_t t)
  {
    assert(t != 0.0f);
    time /= t;
    return(*this);
  }

  inline bool frame_advance(time_value_t t)
  {
    (*this) -= t;
    return(time <= 0.0f);
  }
  inline bool adv(time_value_t t)
  {
    return(frame_advance(t));
  }

  inline bool finished()
  {
    return(time <= 0.0f);
  }
};




class simple_elapse_timer
{
protected:
  time_value_t time;

public:
  simple_elapse_timer(time_value_t t = 0.0f)
  {
    time = t;
  }
  simple_elapse_timer(const simple_elapse_timer &b)
  {
    time = b.time;
  }

  inline void reset()
  {
    time = 0.0f;
  }

  inline time_value_t get_time() const
  {
    return(time);
  }
  inline time_value_t get() const
  {
    return(time);
  }
  inline operator const time_value_t&() const
  {
    return(time);
  }

  inline simple_elapse_timer &operator=(const time_value_t t)
  {
    time = t;
    return(*this);
  }
  inline simple_elapse_timer &operator=(const simple_elapse_timer &b)
  {
    time = b.time;
    return(*this);
  }
  inline simple_elapse_timer &operator+=(const time_value_t t)
  {
    time += t;
    return(*this);
  }
  inline simple_elapse_timer &operator-=(const time_value_t t)
  {
    time -= t;
    if(time < 0.0f)
      time = 0.0f;

    return(*this);
  }
  inline simple_elapse_timer &operator*=(const rational_t t)
  {
    time *= t;
    return(*this);
  }
  inline simple_elapse_timer &operator/=(const rational_t t)
  {
    assert(t != 0.0f);
    time /= t;
    return(*this);
  }

  inline void frame_advance(time_value_t t)
  {
    time += t;
  }
  inline void adv(time_value_t t)
  {
    frame_advance(t);
  }
};





class simple_oscillator
{
protected:
  rational_t start;
  rational_t delta;
  rational_t speed;
  rational_t cur;

public:
  inline void reset()
  {
    cur = start;
  }

  inline void setup(rational_t _start = 0.0f, rational_t _delta = 1.0f, rational_t _osc_per_sec = 1.0f)
  {
    start = _start;
    delta = _delta;
    speed = (__fabs(delta)*4.0f) * _osc_per_sec;

    reset();
  }

  simple_oscillator(rational_t _start = 0.0f, rational_t _delta = 1.0f, rational_t _osc_per_sec = 1.0f)
  {
    setup(_start, _delta, _osc_per_sec);
  }

  simple_oscillator(const simple_oscillator &b)
  { 
    start = b.start;
    delta = b.delta;
    speed = b.speed;
    cur = b.cur;
  }

  inline rational_t get_val() const
  {
    return(cur);
  }
  inline rational_t get() const
  {
    return(cur);
  }
  inline operator const rational_t&() const
  {
    return(cur);
  }

  inline simple_oscillator &operator=(const simple_oscillator &b)
  { 
    start = b.start;
    delta = b.delta;
    speed = b.speed;
    cur = b.cur;

    return(*this);
  }

  inline void frame_advance(time_value_t t)
  {
    if(delta > 0.0f)
    {
      cur += speed*t;

      if(cur > (start+delta))
      {
        cur = (start+delta);
        delta = -delta;
      }
    }
    else if(delta < 0.0f)
    {
      cur -= speed*t;

      if(cur < (start+delta))
      {
        cur = (start+delta);
        delta = -delta;
      }
    }
  }

  inline void adv(time_value_t t)
  {
    frame_advance(t);
  }
};

#define FRAME_ADV_ARRAY(a,n,t)      { for(int advArr = 0; advArr < n; ++advArr) a[advArr].frame_advance(t); }
#define FRAME_ADV_ARRAY_PTR(a,n,t)  { for(int advArr = 0; advArr < n; ++advArr) a[advArr]->frame_advance(t); }

#endif