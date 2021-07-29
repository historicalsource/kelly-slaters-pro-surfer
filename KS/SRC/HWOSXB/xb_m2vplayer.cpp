#include "global.h"
#undef free
#undef malloc
#include "ngl.h"
#include "xb_m2vplayer.h"

#if defined(USE_MPEG2)
#include "mpeg2.h"
#else
#include "mpeg.h"
#endif /* USE_MPEG2 JIV DEBUG */

#include "game.h"
#include "time_interface.h"

m2v_player_t* m2v_create_player (const char* filename)
{
  STUB( "m2v_create_player" );

  m2v_player_t *retval = new m2v_player_t;

  bool noErr = m2v_player_load( retval,  filename );

  assert( noErr );
  assert( retval->foo );

  return retval;
}

bool m2v_player_load( m2v_player_t *h, const char *fname )
{
#if defined(TARGET_XBOX)
  nglSetTexturePath("\\");
  h->foo = nglLoadTextureA( "stub" );
  h->tex = *h->foo;

  return true;
#endif /* TARGET_XBOX JIV DEBUG */

  char temp[256]; // JIV FIXME

  nglPrintf( "transforming %s to D:\\movies\\ez.mpeg\n", fname );
  
  sprintf( h->mFileName, "D:\\movies\\ez.m2v" );

  
  h->handle = fopen( h->mFileName, "rb" );
  if(!h->handle)
    return false;

  OpenMPEG(h->handle, &h->img);
  RewindMPEG(h->handle,&h->img );

  h->foo = nglCreateTexture( NGLTF_32BIT, h->img.Width, h->img.Height );
  h->tex = *h->foo;
  
  assert( h->img.Size );

  h->pixels = (char *) malloc( h->img.Size  );
  h->mDone = false;
  
  return true;
}


void m2v_destroy_player (m2v_player_t* player)
{
  STUB( "m2v_destroy_player" );
  return;

  nglReleaseTexture( player->foo );

  fclose( player->handle );
  free( player->pixels );
  player->pixels = 0;

  delete player;
}

void m2v_frame_advance (m2v_player_t* player)
{
//    nglPrintf( "m2v_frame_advance\n" );

  return;

  if(player->mDone)
  {
    RewindMPEG(player->handle, &player->img);
  }
  
  // "clear"
  memset( player->pixels, 0x50, player->img.Size );

  int mMoreFrames = GetMPEGFrame(player->pixels);

  nglSetTextureXBKS( player->foo, 0, 0, player->pixels, player->img.Size );
  assert( player->foo );

  player->tex = *player->foo;
  player->mDone = mMoreFrames <= 0;

  return;
}
