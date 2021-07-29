#ifndef _AI_LOCOMOTION_DIRECT_H_
#define _AI_LOCOMOTION_DIRECT_H_

#include "global.h"
#include "ai_locomotion.h"


class ai_locomotion_direct : public ai_locomotion
{
protected:
  virtual ai_locomotion *make_copy(ai_interface *own);
  void copy(ai_locomotion_direct *b);

  virtual void going_into_service();
  virtual void going_out_of_service();

  virtual void handle_chunk(chunk_file& fs, stringx &label);

public:
  ai_locomotion_direct(ai_interface *own);
  virtual ~ai_locomotion_direct();

  virtual bool set_destination(const vector3d &pos, rational_t radius = 2.0f, bool fast = true, bool path_find = true, bool force_finish = false);
  virtual bool process_movement(time_value_t t);
};

#endif
