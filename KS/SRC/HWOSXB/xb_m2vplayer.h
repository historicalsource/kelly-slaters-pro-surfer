#ifndef _M2VPLAYER_H_
#define _M2VPLAYER_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_M2V_WIDTH   720
#define MAX_M2V_HEIGHT  448
#define MAX_M2V_MBX     (MAX_M2V_WIDTH/16)
#define MAX_M2V_MBY     (MAX_M2V_HEIGHT/16)

#include "ngl.h"

#ifdef USE_MPEG2
#include "mpeg2.h"
#else
#include "mpeg.h"
#endif /* USE_MPEG2 JIV DEBUG */

struct m2v_player_t
{
  nglQuad quad;
  nglTexture tex;
private:
  enum { MaxFileName = 32 };

#ifdef USE_MPEG2
  struct layer_data *handle;
#else
  FILE *handle;
#endif /* USE_MPEG2 JIV DEBUG */

  ImageDesc img;
  char *pixels;
  nglTexture *foo;
  bool mDone;
  char mFileName[MaxFileName];

  friend m2v_player_t* m2v_create_player (const char* filename);
  friend void m2v_destroy_player (m2v_player_t* player);
  friend void m2v_frame_advance (m2v_player_t* player);
  friend bool m2v_player_load( m2v_player_t *h, const char *fname );
};

m2v_player_t* m2v_create_player (const char* filename);
void m2v_destroy_player (m2v_player_t* player);
void m2v_frame_advance (m2v_player_t* player);
bool m2v_player_load( m2v_player_t *h, const char *fname );
  
/*  m2v_player_t* m2v_create_player (const char* filename); */
/*  void m2v_destroy_player (m2v_player_t* player); */
/*  void m2v_frame_advance (m2v_player_t* player); */

#ifdef __cplusplus
}
#endif

#endif // _M2VPLAYER_H_
