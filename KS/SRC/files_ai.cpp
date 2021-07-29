// ai files for ps2 link speedup

// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h" 
#include "AIController.cpp"
//#include "ai_constants.cpp"
#include "ai_interface.cpp"
#include "ai_actions.cpp"
/*
//#include "ai_actions_heli.cpp"
//#include "ai_actions_shocker.cpp"
//#include "ai_actions_vulture.cpp"
//#include "ai_actions_combat.cpp"
//#include "ai_actions_kraven.cpp"
*/
#include "ai_goals.cpp"
#include "ai_locomotion.cpp"
#include "ai_locomotion_direct.cpp"
/*
//#include "ai_goals_heli.cpp"
//#include "ai_goals_shocker.cpp"
//#include "ai_goals_vulture.cpp"
//#include "ai_goals_combat.cpp"
//#include "ai_goals_kraven.cpp"
//#include "ai_locomotion_walk.cpp"
//#include "ai_locomotion_fly.cpp"
//#include "ai_locomotion_heli.cpp"
//#include "ai_locomotion_winged.cpp"
//#include "ai_communication.cpp"
//#include "ai_cue.cpp"
//#include "ai_senses.cpp"
*/
#ifdef GCCULL
#include "ai_voice.cpp"
#endif

//#include "ai_aim.cpp"
/*
//#include "ai_script_lib.cpp"

*/
#include "ai_polypath.cpp"
#include "ai_polypath_cell.cpp"
