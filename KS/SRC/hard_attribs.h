// This is where NEW attributes are added to the character_hard_attributes structure.  This is a fancy
// version of the EAC trick for keeping data all in one place.

// The entries, if it's not obvious, are:
// MAC("attribute name", data_type_name, variable_name, default_value)

MAC( "NAME", ATYPE_STRING, name, "Character" )

MAC( "MAX_TURN_RATE",       ATYPE_RATIONAL_T, max_turn_rate,       5.0f  )
MAC( "RUN_STUCK_DISTANCE",  ATYPE_RATIONAL_T, run_stuck_distance,  0.5f  )
MAC( "WALK_STUCK_DISTANCE", ATYPE_RATIONAL_T, walk_stuck_distance, 0.25f )


MAC( "AI_RUN_SPEED",  ATYPE_RATIONAL_T, ai_run_speed,  6.0f  )
MAC( "AI_WALK_SPEED", ATYPE_RATIONAL_T, ai_walk_speed, 2.0f  )

MAC( "REPULSION_CHARACTER_RADIUS", ATYPE_RATIONAL_T, repulsion_character_radius, 1.0f )
MAC( "CHARACTER_CAMERA_COLLISION", ATYPE_INT, character_camera_collision, 0 )
