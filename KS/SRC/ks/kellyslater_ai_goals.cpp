#include "global.h"

#include "kellyslater_ai_goals.h"
#include "ai_interface.h"
#include "ai_locomotion.h"
// BIGCULL #include "ai_senses.h"
#include "animation_interface.h"
#include "physical_interface.h"
#include "ai_actions.h"
#include "entity.h"
#include "collide.h"
#include "random.h"
#include "item.h"
#include "wds.h"
#include "terrain.h"
// BIGCULL #include "gun.h"
#include "hwaudio.h"
#include "switch_obj.h"
//#include "sound_interface.h"
// BIGCULL #include "damage_interface.h"
#include "kellyslater_controller.h"
#include "entity_maker.h"
#include "matfac.h"
#include "profiler.h"



surfer_ai_goal::surfer_ai_goal(ai_interface *own)
  : ai_goal(own)
{
  type = pstring("KELLYSLATER");

  exploded = false;
}

surfer_ai_goal::~surfer_ai_goal()
{
}

AI_GOAL_MAKE_COPY_MAC(surfer);

extern profiler_timer proftimer_adv_AI_hero;
rational_t surfer_ai_goal::frame_advance(time_value_t t)
{
  return(ai_goal::frame_advance(t));
}

void surfer_ai_goal::setup(int the_pad)
{
}

rational_t surfer_ai_goal::calculate_priority(time_value_t t)
{
//  #pragma todo("This code really should be in the camera, but is here to keep it active when he loses his goal. (JDB 4-03-01)")

  priority = 1.0f;
  return(priority);
}

void surfer_ai_goal::going_into_service()
{
  ai_goal::going_into_service();

}

void surfer_ai_goal::going_out_of_service()
{
  ai_goal::going_out_of_service();
}
