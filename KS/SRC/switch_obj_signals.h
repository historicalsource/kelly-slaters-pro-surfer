// switch_obj_signals.h

// For each signal needed at the level of class entity, add a line of the form:
//   MAC( SIGNAL, "owner::SIGNAL" )

MAC( SWITCH_TOGGLE,   "switch_obj::SWITCH_TOGGLE" )
MAC( SWITCH_ON,       "switch_obj::SWITCH_ON" )
MAC( SWITCH_OFF,      "switch_obj::SWITCH_OFF" )
