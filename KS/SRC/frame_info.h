#ifndef _FRAME_INFO_H
#define _FRAME_INFO_H


#include "ostimer.h"

class frame_info
{
private:
  time_value_t age;
  int ifl_frame_boost;
  int ifl_frame_locked;
  static float ifl_frame_rate;

public:
  frame_info();
  static void set_frame_rate(float);
  static float get_frame_rate ();
  time_value_t get_age() const;
  void set_age(time_value_t a) { age = a; }
  int get_ifl_frame_boost () const;
  void set_ifl_frame_boost( int boost ) { ifl_frame_boost = boost; }
  int get_ifl_frame_locked () const;
  void set_ifl_frame_locked(int);
  void operator=(const frame_info &source);

  void compute_boost_for_play(int period);
  int time_to_frame_locked (int period = 0) const;
  int time_to_frame (int period = 0) const;
};


#endif  // _FRAME_INFO_H
