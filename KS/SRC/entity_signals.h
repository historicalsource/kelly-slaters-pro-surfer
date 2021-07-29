// entity_signals.h

// For each signal needed at the level of class entity, add a line of the form:
//   MAC( SIGNAL, "owner::SIGNAL" )

MAC( ACTIVATED_BY_CHARACTER, "entity::ACTIVATED_BY_CHARACTER" )
MAC( DAMAGED,   "entity::DAMAGED" )
MAC( DESTROYED, "entity::DESTROYED" )
MAC( RADIO_DETONATE, "entity::RADIO_DETONATE" )

// signal raised when gun is fired by the character
MAC( USE_ITEM, "entity::USE_ITEM" )

// signal to perform the action appropriate to the current animation (e.g., draw, holster, throw, etc. )
MAC( ANIM_ACTION, "entity::ANIM_ACTION" )

// cue was sent to the brain
//MAC( DREAD_NET_CUE, "entity::DREAD_NET_CUE" )

  // ATTACK SEQUENCING
MAC( ATTACK_BEGIN, "entity::ATTACK_BEGIN" )
MAC( ATTACK_END, "entity::ATTACK_END" )
// signal raised when any attack is initiated by the character
MAC( ATTACK, "entity::ATTACK" )

MAC( FOOTSTEP_L, "entity::FOOTSTEP_L" )
MAC( FOOTSTEP_R, "entity::FOOTSTEP_R" )



MAC( AI_STATE_IDLE, "entity::AI_STATE_IDLE" )
MAC( AI_STATE_ALERTED, "entity::AI_STATE_ALERTED" )
MAC( AI_STATE_COMBAT, "entity::AI_STATE_COMBAT" )

MAC( AI_STATE_INCREMENT, "entity::AI_STATE_INCREMENT" )
MAC( AI_STATE_DECREMENT, "entity::AI_STATE_DECREMENT" )

MAC( AI_BOSS_SIGNAL_1, "entity::AI_BOSS_SIGNAL_1" )
MAC( AI_BOSS_SIGNAL_2, "entity::AI_BOSS_SIGNAL_2" )
MAC( AI_BOSS_SIGNAL_3, "entity::AI_BOSS_SIGNAL_3" )
MAC( AI_BOSS_SIGNAL_4, "entity::AI_BOSS_SIGNAL_4" )

MAC( GENERIC_SIGNAL_1, "entity::GENERIC_SIGNAL_1" )
MAC( GENERIC_SIGNAL_2, "entity::GENERIC_SIGNAL_2" )
MAC( GENERIC_SIGNAL_3, "entity::GENERIC_SIGNAL_3" )
MAC( GENERIC_SIGNAL_4, "entity::GENERIC_SIGNAL_4" )

MAC( FADED_OUT, "entity::FADED_OUT" )
MAC( SCENE_ANIM_FINISHED, "entity::SCENE_ANIM_FINISHED" )

MAC( BOUNCED, "entity::BOUNCED" )
