// This is where NEW attributes are added to the character_soft_attributes structure.  This is a fancy
// version of the EAC trick for keeping data all in one place.

// The entries, if it's not obvious, are:
// MAC("attribute name", data_type_name, variable_name, default_value)

MAC( hit_points )
MAC( armor_points )

MAC( ammo_clips )
MAC( ammo_points )

MAC( "TEAM", ATYPE_INT, team, MONSTER_TEAM_1 )

MAC( "FULL_HIT_POINTS",   ATYPE_INT, full_hit_points,  200 )
MAC( "MAX_AMMO_POINTS",   ATYPE_INT, max_ammo_points,   30 )
MAC( "ARMOR_POINTS",      ATYPE_INT, armor_points,       0 )
