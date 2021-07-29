#ifndef DEBUG_H
#define DEBUG_H
/*-------------------------------------------------------------------------------------------------------

  DEBUG.H - Project debugging options, macros and functions.

  This module interacts heavily with the other modules and the operating system, so it's implemented
  in HWOS??.lib.

  Actually, its in D2 now but will move to HWOS?? soon.

-------------------------------------------------------------------------------------------------------*/
#include "algebra.h"

//
// OK, here's the list of possible DEBUG #defines.  Define them in 
// project settings->C++->preprocessor->preprocessor definitions for VC++,
// or in prefix_dc.h for Metrowerks:
//
// DEBUG:         Enables assert, other debugging code.
// NDEBUG:        No debugging.  None of the DEBUG* symbols will be defined if this is defined.
// DEBUG_ARRAYS:  Array debugging means that accesses to arrays should be checked to make sure they're 
//                valid before they occur.  See vectorx.h.
// DEBUG_MAX:     Maximum debugging means that all the data validation code debugging code we ever write 
//                to track down hard bugs gets turned on.
// 
// Note that DEBUG_ARRAYS and DEBUG_MAX can slow the game down a lot.  So they should only be turned on when
// debugging something, and they're not for normal use.
//

#include "users.h"

#ifndef BUILD_FINAL

void debug_print(const char* fmt, ...);

// Output to debug window/log file.
void debug_print(const stringx& str);

// Print on screen.
void debug_print(const vector2di& pos, const stringx& str);

// <<<<< Not implemented yet.
// Hey, 3d debugging info - why not?  Give it coords, it will get displayed w/o clipping (on top of 
// everything) and w/o scaling at the right screen location
void debug_print(vector3d pos, const stringx& str);

// same things without the user_t argument (UF_ALL assumed)
void debug_print(const char* fmt, ...);
void debug_print(const stringx& str);
void debug_print(const vector2di& pos, const stringx& str);
void debug_print(vector3d pos, const stringx& str);

#else   // #ifdef DEBUG
// Note: names are taken out to avoid unreferenced parameter warnings.
inline void debug_print( const char*, ... )         {}
inline void debug_print( const stringx& )           {}
inline void debug_print( vector2d, const stringx& ) {}
inline void debug_print( vector3d, const stringx& ) {}

#endif  // #ifdef DEBUG

// compatibility stuff for existing debug code.
void debug_out( char*, int, const char* );

#if defined(DEBUG) 
    #define DEBUG_MSG(str)    debug_out( __FILE__, __LINE__, str )
#else
    #define DEBUG_MSG(str)    
#endif

// extra debug
#if defined(DEBUG_MAX)
    #define XDEBUG_MSG(str)   debug_out( __FILE__, __LINE__, str )
#else
    #define XDEBUG_MSG(str)
#endif

#endif  // #ifdef DEBUG_H