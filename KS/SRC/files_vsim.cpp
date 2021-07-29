// archengine files for ps2 link speedup

// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#ifndef TARGET_GC
#include "pstring.cpp"
#endif
#include "console.cpp"
#include "consoleCmds.cpp"
#include "consoleVars.cpp"
#include "file.cpp"
#include "chunkfile.cpp"
#include "filespec.cpp"
//#include "fstreamx.cpp" // not being used
#include "textfile.cpp"
#include "aggvertbuf.cpp"
#include "bitplane.cpp"
#include "color.cpp"
#include "fogmgr.cpp"
#include "frame_info.cpp"
#include "material.cpp"
#include "text_font.cpp"
#include "widget.cpp"
#include "inputmgr.cpp"
#include "geometry.cpp"
#include "hull.cpp"
#include "mustash.cpp"
#include "plane.cpp"
#include "po.cpp"
#include "debugutil.cpp"
#include "errorcontext.cpp"
#include "eventmanager.cpp"
#include "path.cpp"
#include "profiler.cpp"
#include "signal.cpp"
#include "singleton.cpp"
#include "stash_support.cpp"
#include "stringx.cpp"
#include "b_spline.cpp"
#include "opcodes.cpp"
#include "script_library_class.cpp"
#include "script_object.cpp"
#include "so_data_block.cpp"
#include "vm_executable.cpp"
#include "vm_stack.cpp"
#include "vm_symbol.cpp"
#include "vm_thread.cpp"
#include "zip_filter.cpp"
#ifdef TARGET_GC
#include "pstring.cpp"
#endif

  // leave this as the last file included. It undefines assert
#include "osassert.cpp"

