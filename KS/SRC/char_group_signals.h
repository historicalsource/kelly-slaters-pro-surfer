// char_group_signals.h

// For each signal needed at the level of class char_group, add a line of the form:
//   MAC( SIGNAL, "owner::SIGNAL" )

// signal raised when a member of the group is damaged
MAC( DAMAGED, "char_group::DAMAGED" )

// signal raised when a member of the group is killed
MAC( KILLED,  "char_group::KILLED" )

// signal raised when last living member is killed
MAC( ALL_DEAD, "char_group::ALL_DEAD" )
