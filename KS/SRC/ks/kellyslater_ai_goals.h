#ifndef _KELLYSLAYER_AI_GOALS_H_
#define _KELLYSLAYER_AI_GOALS_H_

#include "ai_goals.h"


class kellyslater_controller;
class surfer_ai_goal : public ai_goal
{
protected:
  friend class ai_interface;

  virtual rational_t frame_advance(time_value_t t);

  virtual ai_goal* make_copy(ai_interface *own);

  kellyslater_controller *kellyslater_controller_ptr;
  vector3d exploded_pos;
  bool exploded;

public:
  surfer_ai_goal(ai_interface *own);
  virtual ~surfer_ai_goal();

  void setup(int the_pad = 0);

  kellyslater_controller *get_kellyslater_controller_ptr()   {return kellyslater_controller_ptr;}

  virtual rational_t calculate_priority(time_value_t t);

  virtual void going_into_service();
  virtual void going_out_of_service();
};


#endif