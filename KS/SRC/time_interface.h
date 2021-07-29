#ifndef _TIME_INTERFACE_H_
#define _TIME_INTERFACE_H_

#include "global.h"
#include "ostimer.h"
#include "entity_interface.h"

typedef enum 
{
  _TIME_WORLD     = 0,
  _TIME_ABSOLUTE  = 1,
  _TIME_RELATIVE  = 2
} eTimeMode;

extern rational_t g_time_dilation;

class time_interface : public entity_interface
{
protected:
  rational_t time_dilation;
  eTimeMode time_mode;

public:
  time_interface(entity *ent)
    : entity_interface(ent)
  {
    time_dilation = 1.0f;
    time_mode = _TIME_WORLD;
  }

  virtual ~time_interface()
  {
  }

  void copy(time_interface *b)
  {
    time_mode = b->time_mode;
    time_dilation = b->time_dilation;
  }

  rational_t calc_time_dilation()
  {
    switch(time_mode)
    {
      case _TIME_WORLD:
        return(g_time_dilation);

      case _TIME_ABSOLUTE:
        return(time_dilation);
      
      case _TIME_RELATIVE:
        return(g_time_dilation*time_dilation);

      default:
        return(1.0f);
    }
  }

  void set_mode(eTimeMode mode)   { assert(mode == _TIME_WORLD || mode == _TIME_ABSOLUTE || mode == _TIME_RELATIVE); time_mode = mode; }
  eTimeMode get_mode()            { return(time_mode); }

  void set_time_dilation(rational_t d)    { time_dilation = d; if(time_dilation < 0.0f) time_dilation = 0.0f; }
  rational_t get_time_dilation()          { return(time_dilation); }

  virtual bool get_ifc_num(const pstring &att, rational_t &val);
  virtual bool set_ifc_num(const pstring &att, rational_t val);
};

#define ENTITY_TIME_DILATION(e)           (e->has_time_ifc() ? e->time_ifc()->calc_time_dilation() : g_time_dilation)
#define CALC_ENTITY_TIME_DILATION(a, e)   (a*ENTITY_TIME_DILATION(e))
#define CALC_GLOBAL_TIME_DILATION(a)      (a*g_time_dilation)

#endif