/*-------------------------------------------------------------------------------------------------------

  ESL.H - Enumerated stringx list support

  An enumerated stringx list (ESL) is an enumerated type with an array of strings containing a stringx for
  each element in the enumerated type, typically with the same name.

  This file provides a mechanism for making it easy to maintain consistency between the enumerated type
  and the stringx list by interleaving their definitions with macros and an include file.  It started out
  as a macro, but then I found out it wasn't possible to make the generator automatic so this file became
  something you just copy code out of.

  For an example of its use, see DEBUG.H and DEBUG.CPP.

  Usage:
  * Create a header file with the same name as the enumerated type you are making, for example "user_flag_t.h"

  * Add lines to the header file that look like ESL_ITEM( enum_name, str_name ) where enum_name is the name
    you want to appear in the enumerated type and str_name is the stringx with quotes included that you want
    to appear in the stringx list.

  * In the .h file you want to declare the enumerated stringx list in, copy the following code replacing your
    names for the parameters where enum_name is the name of the enumerated type you want to create, str_name 
    is the name of the stringx list you want to create, and file is the name of the file the list can be read 
    from.

  * Sample user_flag_t.h file:

  // USER_FLAG_T.H - Enumerated stringx list for user_flag_t.  Note that initializers and comments are fine.
  ESL_ITEM( UF_WB = 0,    "WB" )          // Wade Brainerd
  ESL_ITEM( UF_JF = 1,    "JF" )          // James Fristrom           
  ...

  * It should be ok to use this in a header file, since the strings are declared completely const.

// ------------ Begin copying code here --------------

enum enum_name
  {                                         
  #define ESL_ITEM( e, s ) e,               
  #include "file"
  #undef ESL_ITEM                           
  };                                        
                                            
const char * const str_name[] =
  {                                         
  #define ESL_ITEM( e, s ) s,               
  #include "file"
  #undef ESL_ITEM                           
  };                                        

// ------------ End copying code here ----------------

-------------------------------------------------------------------------------------------------------*/

#error ESL.H should never be included.  It's just code for copying.
