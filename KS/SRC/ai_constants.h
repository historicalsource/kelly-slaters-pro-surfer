#ifndef _AI_CONSTANTS_H_
#define _AI_CONSTANTS_H_

#include "global.h"
#include "pstring.h"

extern pstring r_arm_aimer;
extern pstring l_arm_aimer;
extern pstring head_aimer;
extern pstring chest_aimer;

// repulsion constants
# define AI_REPULSION_ANGLE_THRESHOLD       0.1f    
# define AI_REPULSION_FOLLOW_ANGLE          0.3f
# define AI_REPULSION_WAIT_TIME             1.0f
# define AI_REPULSION_HOLD_TIME             0.5f




// Animation slot defs
#define AI_ANIM_IDLE            ANIM_PRIMARY_A

#define AI_ANIM_LOCOMOTION      ANIM_PRIMARY_B

#define AI_ANIM_AIM_A           ANIM_SECONDARY_A
#define AI_ANIM_PARTIAL_A       ANIM_SECONDARY_A
#define AI_ANIM_LOCO_OVERRIDE   ANIM_SECONDARY_A

#define AI_ANIM_AIM_B           ANIM_SECONDARY_B
#define AI_ANIM_PARTIAL_B       ANIM_SECONDARY_B

#define AI_ANIM_JOCKEY          ANIM_SECONDARY_C

#define AI_ANIM_ATTACK          ANIM_TERTIARY_A
#define AI_ANIM_WOUNDED         ANIM_TERTIARY_A

#define AI_ANIM_FULL_BODY       ANIM_TERTIARY_B

#define AI_ANIM_ABSOLUTE        ANIM_TERTIARY_C

#endif