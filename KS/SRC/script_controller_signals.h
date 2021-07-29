// script_controller_signals.h

// For each signal needed at the level of class entity, add a line of the form:
//   MAC( SIGNAL, "owner::SIGNAL" )

MAC( X_PRESSED,         "script_controller::X_PRESSED" )
MAC( X_RELEASED,        "script_controller::X_RELEASED" )
MAC( SQUARE_PRESSED,    "script_controller::SQUARE_PRESSED" )
MAC( SQUARE_RELEASED,   "script_controller::SQUARE_RELEASED" )
MAC( TRIANGLE_PRESSED,  "script_controller::TRIANGLE_PRESSED" )
MAC( TRIANGLE_RELEASED, "script_controller::TRIANGLE_RELEASED" )
MAC( CIRCLE_PRESSED,    "script_controller::CIRCLE_PRESSED" )
MAC( CIRCLE_RELEASED,   "script_controller::CIRCLE_RELEASED" )
MAC( L1_PRESSED,        "script_controller::L1_PRESSED" )
MAC( L1_RELEASED,       "script_controller::L1_RELEASED" )
MAC( L2_PRESSED,        "script_controller::L2_PRESSED" )
MAC( L2_RELEASED,       "script_controller::L2_RELEASED" )
MAC( R1_PRESSED,        "script_controller::R1_PRESSED" )
MAC( R1_RELEASED,       "script_controller::R1_RELEASED" )
MAC( R2_PRESSED,        "script_controller::R2_PRESSED" )
MAC( R2_RELEASED,       "script_controller::R2_RELEASED" )

MAC( LEFT_PRESSED,      "script_controller::LEFT_PRESSED" )
MAC( LEFT_RELEASED,     "script_controller::LEFT_RELEASED" )
MAC( RIGHT_PRESSED,     "script_controller::RIGHT_PRESSED" )
MAC( RIGHT_RELEASED,    "script_controller::RIGHT_RELEASED" )

MAC( UP_PRESSED,        "script_controller::UP_PRESSED" )
MAC( UP_RELEASED,       "script_controller::UP_RELEASED" )
MAC( DOWN_PRESSED,      "script_controller::DOWN_PRESSED" )
MAC( DOWN_RELEASED,     "script_controller::DOWN_RELEASED" )

MAC( START_PRESSED,     "script_controller::START_PRESSED" )
MAC( START_RELEASED,    "script_controller::START_RELEASED" )
