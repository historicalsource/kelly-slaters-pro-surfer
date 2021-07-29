// hwosgc files for gc link speedup

#include "hwosgc/gc_algebra.cpp"
#include "hwosgc/gc_arammgr.cpp"
#include "hwosgc/gc_errmsg.cpp"
#include "hwosgc/gc_file.cpp"
#include "hwosgc/gc_input.cpp"
#include "hwosgc/gc_rasterize.cpp"
#include "hwosgc/gc_texturemgr.cpp"
#include "hwosgc/gc_timer.cpp"
#include "hwosgc/gc_math.cpp"
#include "hwosgc/gc_movieplayer.cpp"
#include "hwosgc/gc_gamesaver.cpp"
#include "hwosgc/ngl_gc.cpp"
#include "hwosgc/ngl_instbank.cpp"
#include "hwosgc/ngl_fileio.cpp"
#include "hwosgc/nvl_gc_bink.cpp"
#include "hwosgc/nvlstream_gc.cpp"
#include "gc_main.cpp"
#include "gcstub.cpp"

#include "hwosgc/gc_ifl.cpp"

#include "gamecube/nl_gc.cpp"
#include "gamecube/nsl_gc.cpp"
#include "gamecube/nsl_gc_source.cpp"
#include "gamecube/nsl_gc_sound.cpp"
#include "gamecube/nsl_gc_emitter.cpp"
#include "gamecube/nsl_gc_stream.cpp"
#include "common/nsl.cpp"

//Put gc_alloc.cpp here because it undefs our malloc defines
#include "hwosgc/gc_alloc.cpp"

#ifdef TARGET_GC
// This file should remain the last file included. It does strange things.
#include "ksngl.cpp"
#endif
