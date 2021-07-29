#ifndef _M2VPLAYER_H_
#define _M2VPLAYER_H_

#include <libipu.h>
#include <libmpeg.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_M2V_WIDTH   720
#define MAX_M2V_HEIGHT  448
#define MAX_M2V_MBX     (MAX_M2V_WIDTH/16)
#define MAX_M2V_MBY     (MAX_M2V_HEIGHT/16)

typedef struct
{
  // Data for uncached access (must be 64-bit aligned)
  sceIpuRGB32 rgb32[MAX_M2V_MBX * MAX_M2V_MBY];

  // Work space for MPEG decoder
  u_char mpegBuff[SCE_MPEG_BUFFER_SIZE(MAX_M2V_WIDTH, MAX_M2V_HEIGHT)];

  u_long128 *bs;
  int bsSize;
  sceMpeg mp;

  nglQuad quad;
  nglTexture tex;

} m2v_player_t;

m2v_player_t* m2v_create_player (const char* filename);
void m2v_destroy_player (m2v_player_t* player);
void m2v_frame_advance (m2v_player_t* player);

#ifdef __cplusplus
}
#endif

#endif // _M2VPLAYER_H_