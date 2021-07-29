#ifndef GLOBAL_H
#define GLOBAL_H
////////////////////////////////////////////////////////////////////////////////
/*
  global.h

  header file that should be included by absolutely every .cpp file in the project.
  sets the warning level and std namespace for entire project.
  and puts in memory tracking safety
*/
////////////////////////////////////////////////////////////////////////////////
#include "users.h"

#ifndef __GNUC__
#pragma once
#endif

#if defined(_MSC_VER) || defined(_WIN32)
  #pragma warning(disable: 4786) // disable annoying debug info warning everywhere!
  #pragma warning(disable: 4291) // figure this one out later
  #pragma warning(disable: 4309) // don't remember
  #pragma warning(disable: 4706) // assignment in conditional
  #pragma warning(disable: 4101) // unreferences local variable sloppy

  // if we're not doing a debug compile
  #ifdef NDEBUG
    // this will prevent warnings on things like verify(a == b);
    #pragma warning(disable: 4553) // '==' : operator has no effect; did you intend '='?
  #endif // NDEBUG
#endif

#if defined(_WIN32) && defined(_M_IX86)
#if defined(_XBOX)
#ifndef TARGET_XBOX
#define TARGET_XBOX 1
#endif /* TARGET_XBOX JIV DEBUG */

#else
  #define TARGET_PC
#endif /* _XBOX JIV DEBUG */
//  #define __SGI_STL
#elif defined(__GNUC__)
  #define TARGET_PS2
  #define __GNU_STL
#elif defined(EPPC)
  #define TARGET_GC 1
  #define __MSL_STL 1
#elif defined(GEKKO)
  #define __GC_CPU
#else
  #error Unknown target platform!
#endif

#if defined(__KATANA__) || defined(__SET5__)
#ifndef VERSION_R10
  #define __SH4_CPU
#endif
#elif defined(_M_IX86) //|| defined(_WIN32)
  #define __X86_CPU
#elif defined(__PS2_EE__)
  #define __PS2_CPU
#elif defined(GEKKO)
  #define __GC_CPU
#else
  #error Unknown CPU!
#endif


//Uncomment for PAL build
//#define TV_PAL


// Translate Windows debug flag
#if defined(_DEBUG) && !defined(DEBUG)
#define DEBUG _DEBUG
#elif defined(DEBUG) && !defined(_DEBUG)
#define _DEBUG DEBUG
#endif

#ifdef TARGET_PS2
// Define standard allocator.  Allows us to control stl memory management.
// In particular, we can fix stl's annoying habit of not freeing small chunks
// of memory when asked to. (dc 01/31/02)
#define __STL_DEFAULT_ALLOCATOR(T) my_allocator< T >
//#define __STL_USE_SGI_ALLOCATORS
//#define __STL_COUNT_ALLOCS
#endif



#include <limits.h>
#include <float.h>


#include "types.h"
#include "project.h"  // identifies what project and version we are compiling


#define NO_FSTREAM

#if defined(TARGET_XBOX)
#include "xbglobals.h"
#include <time.h>
#endif /* TARGET_XBOX JIV DEBUG */

#if defined(TARGET_GC)
#include "gcglobals.h"
#endif /* TARGET_XBOX JIV DEBUG */

#if defined(TARGET_PC)
  #include "pc_port.h"

  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>

//P  #define __STL_DEFAULT_ALLOCATOR(T) my_alloc

  #include <time.h>
	#define nglPrintf printf
#elif defined(TARGET_MKS)
#endif

//#include "stdlib.h"     // defines their malloc/free for PC
#if defined(TARGET_PS2) || defined(TARGET_XBOX)
#include "malloc.h"     // defines their malloc/free for PS2
#endif

// these are our headers
#include <stdlib.h>  // prepares us for archalloc, defines our malloc/free
#include "archalloc.h"  // prepares us for archalloc, defines our malloc/free
//#include "stl_adapter.h"
#include "warnlvl.h"

#ifdef TARGET_GC
extern "C" int stricmp( const char* s1, const char* s2 );
extern "C" int strnicmp( const char* s1, const char* s2, int n );
extern "C" char* strlwr( char* s );
extern "C" char* strupr( char* s );
#endif


/*-------------------------------------------------------------------------------------------------------

  Useful macros for working with arrays and members

-------------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------------
// convenience for clearing out memory structures c-style.
// remember not to use this on virtual classes, or on classes
// where member-by-member construction is important
// (only use it if you know what you're doing)
-------------------------------------------------------------------------------------------------------*/
#define ZERO_STRUCTURE( x ) (memset( &x, 0, sizeof( x ) ))

/*-------------------------------------------------------------------------------------------------------
  ARRAY_ELEMENTS - Determines the number of elements in an array
-------------------------------------------------------------------------------------------------------*/
#define ARRAY_ELEMENTS( array ) \
  ( sizeof( array ) / sizeof( array[ 0 ] ) )

#ifdef TARGET_PS2
	// Include our custom stl allocator
	#include <stl_algobase.h>
	#include <stl_alloc.h>
	#include "custom_stl.h"
#endif
	#include <vector>
#ifdef TARGET_XBOX
	using namespace std;
#endif
	#include <list>
	#include <string>
	#if !defined(NO_FSTREAM)
		#include <fstream>
	#endif

	#include <ctype.h>
	#include <stdio.h>

//#include "debug.h"  // this gets internal compiler errors!

// stuff that's here just to precompile and make the build faster
#include <map>

// we have to include this after STL stuff because STL includes <assert.h>
// which bashes ours
#include "osassert.h"
#include "oserrmsg.h"	// needs osassert.h
#include "stringx.h"
#include "errorcontext.h"
#include "graph.h"
#include "instance.h"

#if defined(TARGET_XBOX)
#include "hwosxb/xb_string.h"
#endif /* TARGET_XBOX JIV DEBUG */

//! Stub function macro
#define STUBBED(stub, str) \
    static bool once_##stub = true; \
    if (once_##stub) { \
      debug_print("Warning: %s function has been disabled.\n", str); \
      once_##stub = false; \
    }

#ifdef TARGET_PS2

#define PRINT_BLACK   "\x1b\x1e"
#define PRINT_RED     "\x1b\x1f"
#define PRINT_GREEN   "\x1b\x20"
#define PRINT_YELLOW  "\x1b\x21"
#define PRINT_BLUE    "\x1b\x22"
#define PRINT_MAGENTA "\x1b\x23"
#define PRINT_CYAN    "\x1b\x24"
#define PRINT_LTGREY  "\x1b\x25\x25"

#define PRINT_BG_BLACK    "\x1b\x28"
#define PRINT_BG_RED      "\x1b\x29"
#define PRINT_BG_GREEN    "\x1b\x2A"
#define PRINT_BG_DKYELLOW "\x1b\x2B"
#define PRINT_BG_BLUE     "\x1b\x2C"
#define PRINT_BG_MAGENTA  "\x1b\x2D"
#define PRINT_BG_CYAN     "\x1b\x2E"
#define PRINT_BG_BKGRND   "\x1b\x2F"

#else

#define PRINT_BLACK
#define PRINT_RED
#define PRINT_GREEN
#define PRINT_YELLOW
#define PRINT_BLUE
#define PRINT_MAGENTA
#define PRINT_CYAN
#define PRINT_LTGREY

#define PRINT_BG_BLACK
#define PRINT_BG_RED
#define PRINT_BG_GREEN
#define PRINT_BG_DKYELLOW
#define PRINT_BG_BLUE
#define PRINT_BG_MAGENTA
#define PRINT_BG_CYAN
#define PRINT_BG_BKGRND

#endif

//---------------------------------------------------------------------------------------------
// Build description strings.
//---------------------------------------------------------------------------------------------
#if defined(BUILD_DEBUG)
#define BUILD_NAME "Debug"
#elif defined(BUILD_BOOTABLE)
#define BUILD_NAME "Bootable"
#elif defined(BUILD_FINAL)
#define BUILD_NAME "Final"
#endif

//---------------------------------------------------------------------------------------------
// FIXMEs / TODOs / NOTE macros
//---------------------------------------------------------------------------------------------
#ifdef __GNUC__

  #define _QUOTE(x) #x
  #define QUOTE(x) _QUOTE(x)
  #define __FILE__LINE__ __FILE__ "(" QUOTE(__LINE__) ") : "

  #define NOTE( x )  message( x )
  #define FILE_LINE  message( __FILE__LINE__ )

  #define TODO( x )  message( __FILE__LINE__"\n"           \
          " ------------------------------------------------\n" \
          "|  TODO :   " #x "\n" \
          " -------------------------------------------------\n" )

  #define FIXME( x )  message(  __FILE__LINE__"\n"           \
          " ------------------------------------------------\n" \
          "|  FIXME :  " #x "\n" \
          " -------------------------------------------------\n" )

  #define todo( x )  message( __FILE__LINE__" TODO :   " #x "\n" )

  #define fixme( x )  message( __FILE__LINE__" FIXME:   " #x "\n" )

#else

  #define _QUOTE(x) #x
  #define QUOTE(x) _QUOTE(x)
  #define __FILE__LINE__ __FILE__ "(" QUOTE(__LINE__) ") : "

  #define NOTE( x )  message( x )
  #define FILE_LINE  message( __FILE__LINE__ )

  #define TODO( x )  message( __FILE__LINE__"\n"           \
          " ------------------------------------------------\n" \
          "|  TODO :   " #x "\n" \
          " -------------------------------------------------\n" )

  #define FIXME( x )  message(  __FILE__LINE__"\n"           \
          " ------------------------------------------------\n" \
          "|  FIXME :  " #x "\n" \
          " -------------------------------------------------\n" )

  #define todo( x )  message( __FILE__LINE__" TODO :   " #x "\n" )

  #define fixme( x )  message( __FILE__LINE__" FIXME:   " #x "\n" )

#endif

#define countof(a) (sizeof(a) / sizeof(*a))

#include "platform_defines.h"

#ifdef TARGET_GC
#include "ngl.h"
#include "hwosgc\ngl_version.h"
#endif


#ifdef TARGET_XBOX
// Extra #includes for precompiled headers (dc 01/22/02)

#include "beachdata.h"
#include "board.h"
#include "careerdata.h"
#include "kellyslater_controller.h"
#include "trickdata.h"
#include "ode.h"
#include "physics.h"
#include "player.h"
#include "replay.h"
#include "surferdata.h"
#include "trick_system.h"
#include "tricks.h"
#include "VOEngine.h"
#include "wave.h"
#include "waveenum.h"
// next most included
#include "aggvertbuf.h"
#include "algebra.h"
#include "anim.h"
#include "anim_flavor.h"
#include "app.h"
#include "archalloc.h"
#include "avltree.h"
#include "beam.h"
#include "billboard.h"
#include "bone.h"
#include "bound.h"
#include "box_trigger_interface.h"
#include "camera.h"
#include "capsule.h"
#include "cface.h"
#include "chunkfile.h"
#include "colgeom.h"
#include "colmesh.h"
#include "color.h"
#include "commands.h"
#include "conglom.h"
#include "constants.h"
#include "controller.h"
#include "convex_box.h"
#include "debug.h"
/*
#include "devoptflags.h"
#include "devoptints.h"
#include "devoptstrs.h"
*/
#include "entflavor.h"
#include "entity.h"
#include "entity_anim.h"
#include "entity_interface.h"
#include "entity_maker.h"
#include "entityflags.h"
#include "entityid.h"
#include "errorcontext.h"
#include "face.h"
#include "faceflags.h"
#include "fast_vector.h"
#include "file.h"
#include "filespec.h"
#include "frame_info.h"
#include "game.h"
#include "game_info_vars.h"
#include "game_process.h"
#include "generator.h"
#include "geomgr.h"
#include "global.h"
#include "graph.h"
#include "hull.h"
#include "hwmath.h"
#include "hwosxb\xb_algebra.h"
#include "hwosxb\xb_alloc.h"
#include "hwosxb\xb_errmsg.h"
#include "hwosxb\xb_file.h"
#include "hwosxb\xb_graphics.h"
#include "hwosxb\xb_math.h"
#include "hwosxb\xb_rasterize.h"
#include "hwosxb\xb_string.h"
#include "hwosxb\xb_texturemgr.h"
#include "hwosxb\xb_timer.h"
#include "hwosxb\xb_portDVDCache.h"
#include "hwrasterize.h"
#include "hyperplane.h"
#include "ini_parser.h"
#include "inputmgr.h"
#include "instance.h"
#include "item.h"
#include "kshooks.h"
#include "ksnsl.h"
#include "light.h"
#include "linear_anim.h"
#include "link_interface.h"
#include "map_e.h"
#include "material.h"
#include "maxskinbones.h"
#include "meshrefs.h"
#include "mic.h"
#include "mobject.h"
#include "mustash.h"
#include "ngl.h"
#include "osalloc.h"
#include "osassert.h"
#include "osdevopts.h"
#include "oserrmsg.h"
#include "osfile.h"
#include "ostimer.h"
#include "path.h"
#include "pc_port.h"
#include "plane.h"
#include "platform_defines.h"
#include "pmesh.h"
#include "po.h"
#include "po_anim.h"
#include "portal.h"
#include "profiler.h"
#include "project.h"
#include "pstring.h"
#include "random.h"
#include "rect.h"
#include "refptr.h"
#include "region.h"
#include "region_graph.h"
#include "render_data.h"
#include "renderflav.h"
#include "scene_anim.h"
#include "script_lib_mfg.h"
#include "script_library_class.h"
#include "script_object.h"
#include "signal_anim.h"
#include "signals.h"
#include "SimpleAssert.h"
#include "singleton.h"
#include "so_data_block.h"
#include "sphere.h"
#include "stashes.h"
#include "staticmem.h"
#include "stl_adapter.h"
#include "stringx.h"
#include "terrain.h"
#include "text_font.h"
#include "textfile.h"
#include "txtcoord.h"
#include "types.h"
#include "usefulmath.h"
#include "users.h"
#include "vert.h"
#include "visrep.h"
#include "vm_executable.h"
#include "vm_symbol.h"
#include "vm_symbol_list.h"
#include "warnlvl.h"
#include "wds.h"
#include "wedge.h"
#include "xbglobals.h"
#endif

#define sin(x) sinf(x)
#define cos(x) cosf(x)
#define acos(x) acosf(x)
#define asin(x) asinf(x)
#define atan(x) atanf(x)
#define atan2(x) atan2f(x)
#define tan(x) tanf(x)
#define ceil(x) ceilf(x)
#define exp(x) expf(x)
#define fabs(x) fabsf(x)
#define floor(x) floorf(x)
#define mod(x) modf(x)
#define pow(x,y) powf(x,y)
#define sqrt(x) sqrtf(x)


#endif
