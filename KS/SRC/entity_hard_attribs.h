/*** define your NUMBERS (ints, floats) here ***/
#ifndef PROCESS_STRINGS_ONLY

MAC( rational_t, "RATIONAL", max_turn_rate, "MAX_TURN_RATE", 5.0f  )
MAC( rational_t, "RATIONAL", run_stuck_distance, "RUN_STUCK_DISTANCE",  0.5f  )
MAC( rational_t, "RATIONAL", walk_stuck_distance, "WALK_STUCK_DISTANCE", 0.25f )


MAC( rational_t, "RATIONAL", ai_run_speed, "AI_RUN_SPEED", 6.0f  )
MAC( rational_t, "RATIONAL", ai_walk_speed, "AI_WALK_SPEED", 2.0f  )

MAC( rational_t, "RATIONAL", repulsion_radius, "REPULSION_RADIUS", 1.0f )
MAC( int, "INT", camera_collision, "CAMERA_COLLISION", 0 )

#endif


/*** define your STRINGS here ***/
#ifndef PROCESS_NUMBERS_ONLY

// formerly known as "name", used to determine which entities use a particular set of hard attributes
MAC( pstring, "PSTRING", group, "GROUP", "CHARACTER" )  

#endif
