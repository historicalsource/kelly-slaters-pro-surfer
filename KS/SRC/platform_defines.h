// Platform-specific constant defines for various things used inside and outside of
// the Arch-Engine
#ifndef PLATFORM_DEFINES
#define PLATFORM_DEFINES

// per platform defines for texture/mesh/stash/fmc extensions and directories
#define PS2_TEXTURE_NAME  "tm2"
#define PS2_MESH_NAME     "ps2mesh"
#define PS2_STASH_NAME    "st2"
#define PS2_FMC_NAME      "ps2fmc"
#define PS2_LOG_SUFFIX    "_ps2.txt"
#define PS2_ANIM_NAME     "anmx"
#define PS2_ANIMTEX_NAME  "ate"

#define PC_TEXTURE_NAME   "tga"
#define PC_MESH_NAME      "txtmesh"
#define PC_STASH_NAME     "pcstash"
#define PC_FMC_NAME       "pcfmc"
#define PC_LOG_SUFFIX     "_pc.txt"
#define PC_ANIM_NAME      "anmx"
#define PC_ANIMTEX_NAME   "ate"

#define GC_TEXTURE_NAME   "gct"
#define GC_MESH_NAME      "gcmesh"
#define GC_STASH_NAME     "st2"
#define GC_FMC_NAME       "gcfmc"
#define GC_LOG_SUFFIX     "_gc.txt"
#define GC_ANIM_NAME      "anmx"
#define GC_ANIMTEX_NAME   "atg"

#define XBOX_TEXTURE_NAME "dds"
#define XBOX_MESH_NAME    "xbmesh"
#define XBOX_STASH_NAME   "st2"
#define XBOX_FMC_NAME     "xfmc"
#define XBOX_LOG_SUFFIX   "_xbox.txt"
#define XBOX_ANIM_NAME    "anmx"
#define XBOX_ANIMTEX_NAME "atx"

#ifdef TARGET_PS2
#define PLATFORM_TEXTURE_NAME PS2_TEXTURE_NAME
#define PLATFORM_MESH_NAME    PS2_MESH_NAME
#define PLATFORM_STASH_NAME   PS2_STASH_NAME
#define PLATFORM_ANIM_NAME    PS2_ANIM_NAME
#define PLATFORM_FMC_NAME     PS2_FMC_NAME
#define PLATFORM_LOG_SUFFIX   PS2_LOG_SUFFIX
#define PLATFORM_ANIMTEX_NAME PS2_ANIMTEX_NAME
#define PLATFORM_TEXTURE_PATH "tm2\\"
#define SINGLE_THREADED

#elif defined (TARGET_PC)
#define PLATFORM_TEXTURE_NAME PC_TEXTURE_NAME
#define PLATFORM_MESH_NAME    PC_MESH_NAME
#define PLATFORM_STASH_NAME   PC_STASH_NAME
#define PLATFORM_ANIM_NAME    PC_ANIM_NAME
#define PLATFORM_FMC_NAME     PC_FMC_NAME
#define PLATFORM_LOG_SUFFIX   PC_LOG_SUFFIX
#define PLATFORM_ANIMTEX_NAME PC_ANIMTEX_NAME
#define PLATFORM_TEXTURE_PATH "ps2 textures\\" // until we get the tga directory fully populated

#elif defined (TARGET_GC)
#define PLATFORM_TEXTURE_NAME GC_TEXTURE_NAME
#define PLATFORM_MESH_NAME    GC_MESH_NAME
#define PLATFORM_STASH_NAME   GC_STASH_NAME
#define PLATFORM_ANIM_NAME    GC_ANIM_NAME
#define PLATFORM_FMC_NAME     GC_FMC_NAME
#define PLATFORM_LOG_SUFFIX   GC_LOG_SUFFIX
#define PLATFORM_ANIMTEX_NAME GC_ANIMTEX_NAME
#define PLATFORM_TEXTURE_PATH "gct\\"

#elif defined (TARGET_XBOX)
#define PLATFORM_TEXTURE_NAME XBOX_TEXTURE_NAME
#define PLATFORM_MESH_NAME    XBOX_MESH_NAME
#define PLATFORM_STASH_NAME   XBOX_STASH_NAME
#define PLATFORM_ANIM_NAME    XBOX_ANIM_NAME
#define PLATFORM_FMC_NAME     XBOX_FMC_NAME
#define PLATFORM_LOG_SUFFIX   XBOX_LOG_SUFFIX
#define PLATFORM_ANIMTEX_NAME XBOX_ANIMTEX_NAME
#define PLATFORM_TEXTURE_PATH "dds\\"

#endif

#define PLATFORM_TEXTURE_EXT  "."PLATFORM_TEXTURE_NAME
#define PLATFORM_MESH_EXT     "."PLATFORM_MESH_NAME
#define PLATFORM_STASH_EXT    "."PLATFORM_STASH_NAME
#define PLATFORM_ANIM_EXT     "."PLATFORM_ANIM_NAME
#define PLATFORM_FMC_EXT      "."PLATFORM_FMC_NAME
#define PLATFORM_ANIMTEX_EXT  "."PLATFORM_ANIMTEX_NAME

// eventually define this this way, once we get all of the art moved
//#define PLATFORM_TEXTURE_PATH PLATFORM_TEXTURE_NAME"\\"

#endif
