#ifndef NGL_VUDEFS_H
#define NGL_VUDEFS_H

// Command list indices.
#define NGLCMD_SKIN               0
#define NGLCMD_DIFFUSE            1
#define NGLCMD_DETAIL             2
#define NGLCMD_ENVIRONMENT        3
#define NGLCMD_LIGHT              4
#define NGLCMD_PROJECTEDTEXTURES  5
#define NGLCMD_FOG                6
#define NGLCMD_VERTEX_LIGHT       7
#define NGLCMD_NUM_LISTS          8

// General mesh renderer VU memory addresses. Formulas follow in comments:
#define NGLMEM_COMMAND_LIST_INDEX   0
#define NGLMEM_COMMAND_LIST_ADDRS   1
#define NGLMEM_COMMAND_LIST_START   (NGLMEM_COMMAND_LIST_ADDRS+NGLCMD_NUM_LISTS)
#define NGLMEM_COMMAND_LIST_END     92     // Don't forget to keep this up to date!

#define NGLMEM_VERTEX_COUNT         92     //
#define NGLMEM_MESH_NAME            93     //
#define NGLMEM_FLAGS                95     // General rendering flags.
#define NGLMEM_TINT_COLOR           96     //
#define NGLMEM_TEXTURE_SCROLL       97     //
#define NGLMEM_FOG_VAL              98     //
#define NGLMEM_FOG_COLOR            99     //
#define NGLMEM_LOCAL_TO_VIEW        100     //
#define NGLMEM_LOCAL_TO_SCREEN      104     //
#define NGLMEM_LOCAL_TO_CLIP        108     //
#define NGLMEM_LOCAL_TO_WORLD       112     //
#define NGLMEM_VIEW_TO_WORLD        116     //
#define NGLMEM_VIEW_TO_LOCAL        120     //
#define NGLMEM_WORLD_TO_VIEW        124     //
#define NGLMEM_WORLD_TO_SCREEN      128     //
#define NGLMEM_WORLD_TO_CLIP        132     //
#define NGLMEM_WORLD_TO_LOCAL       136     //
#define NGLMEM_CLIP_TO_SCREEN       140     //
#define NGLMEM_SCREEN_MIN           144     //
#define NGLMEM_SCREEN_MAX           145     //
#define NGLMEM_STRIP_GIFTAG         146     //
#define NGLMEM_TRIFAN_GIFTAG        147     //
#define NGLMEM_DUMMY_GIFTAG         148     //

#define NGLMEM_SRCBUF               149     // WARNING: These constants are also defined in meshcvt, changes must be reflected there:
#define NGLMEM_BUFFER0              150     // ...
#define NGLMEM_CLIP_BUF             923     // ...
#define NGLMEM_END                  1024

// Addresses relative to ITOP
#define NGLMEM_GIFTAG               0
#define NGLMEM_VERT_START           1

// Particle system VU memory addresses.
#define NGLMEM_PART_DATA            0

// General rendering flags.
#define NGLVU_CLIP                0x0001
#define NGLVU_CLIP_PERFECT        0x0002
#define NGLVU_BACKFACE            0x0004
#define NGLVU_TINT                0x0008
#define NGLVU_TEXTURE_SCROLL      0x0010

#endif
