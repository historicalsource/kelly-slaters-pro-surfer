/*** define your NUMBERS (ints, floats) here ***/
#ifndef PROCESS_STRINGS_ONLY

MAC( basic, int, "INT", team, "TEAM", 0x00000100, -1, -1 )

//MAC( bounded, int, "INT", hit_points, "HIT_POINTS", 1000, 0, 1000 )
//MAC( bounded, int, "INT", armor_points, "ARMOR_POINTS", 1000, 0, 1000 )

#endif


/*** define your STRINGS here ***/
#ifndef PROCESS_NUMBERS_ONLY

// follow this form...  MAC(basic, pstring, "PSTRING", full_name, "FULL_NAME", "no name", "", "")

#endif

/*** TEAM DECODER RING (to be replaced by something in the tool, so it can be ugly for now) ***
 * The flags are stored in a 32-bit integer, and are grouped as follows:
 *
 *   EEEENNGG   where E's are enemy bits, N's are neutral bits, and G's are good bits.
 * 0x00000000
 *
 * the highest level bit in each group is reserved for the "BOSS" level thing, eg, an evil boss
 * would be 0x80000000, and spiderman is 0x00000080, the mayor (or some other neutral boss) would
 * be 0x00008000.  The levels below that are varying degrees of goodness/neutralness/evilness,
 * which can be used to group enemies in teams and the higher priority ones can take precidence in
 * AI.  For more info, see Jason Bare, Greg Taylor or the code itself.  This should be made simpler
 * by the development of the TOOL for Spiderman.
 */
