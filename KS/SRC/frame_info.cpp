#include "global.h"

#include "frame_info.h"


float frame_info::ifl_frame_rate = 30.0f;

frame_info::frame_info() 
{
  ifl_frame_locked = -1; //-1 initially meaning no frame is locked for rendering
  age = 0.0f;
  ifl_frame_boost = 0;
}

int frame_info::get_ifl_frame_locked() const 
{
  return ifl_frame_locked;
}

void frame_info::set_ifl_frame_locked(int frame_to_lock) 
{
  ifl_frame_locked=frame_to_lock;
}

int frame_info::get_ifl_frame_boost() const 
{
  return ifl_frame_boost;
}

time_value_t frame_info::get_age() const 
{
  return age;
}

float frame_info::get_frame_rate() 
{
  return ifl_frame_rate;
}

void frame_info::set_frame_rate(float new_frame_rate) 
{
  ifl_frame_rate=new_frame_rate;
}


void frame_info::operator= (const frame_info &source) 
{
  ifl_frame_locked=source.ifl_frame_locked;
  ifl_frame_boost=source.ifl_frame_boost;
  ifl_frame_rate=source.ifl_frame_rate;
  age = source.age;
}

int frame_info::time_to_frame_locked(int period) const 
{
  if (period==1)
    return 0;
  else if (ifl_frame_locked>=0)
    return ifl_frame_locked;
  else
    return time_to_frame(period);
}


int frame_info::time_to_frame(int period) const 
{
  if (period==1)
    return 0;
  else
  {
    int index = (int)(age * ifl_frame_rate) + ifl_frame_boost;
    if (period==0)
      return index;
    else 
      return (index % period);
  }
}


void frame_info::compute_boost_for_play(int period) 
{ 
  if (ifl_frame_locked >= 0) 
  {
    ifl_frame_boost = ifl_frame_locked - ((int)(age * ifl_frame_rate) % period);
    if (ifl_frame_boost < 0) ifl_frame_boost += period;
    ifl_frame_locked = -1; // Unlock ifl_frame
  }
}
