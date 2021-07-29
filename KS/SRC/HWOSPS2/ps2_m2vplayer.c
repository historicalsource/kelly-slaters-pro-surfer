//
// MPEG2 video player
//

#include <stdio.h>
#include <eekernel.h>
#include <sifdev.h>
#include <libgraph.h>
#include "ps2_m2vplayer.h"

static int readFile (const char *filename, unsigned char **data, int *size);

// ============================================================================

// load mpeg file specified by 'filename'
m2v_player_t* m2v_create_player (const char *filename)
{
  m2v_player_t *player;
  int ret;

  player = (m2v_player_t*) memalign (64, sizeof (m2v_player_t));
	if (player == NULL)
    return NULL;

  memset(player, 0, sizeof(*player));

  //  Read IPU movie file
  ret = readFile (filename, (u_char**)&player->bs, &player->bsSize);
  if (!ret)
  {
    free (player);
    return NULL;
  }

  nglPrintf ("Loaded MPEG file: %s (%d bytes)\n", filename, player->bsSize);

  // Initialize IPU and MPEG library
  sceMpegInit ();

  // Create MPEG decoder
  sceMpegCreate (&player->mp, player->mpegBuff, sizeof (player->mpegBuff));

  //  Add bs data to MPEG decoder
  sceMpegAddBs (&player->mp, player->bs, player->bsSize);

  return player;
}

void m2v_destroy_player (m2v_player_t *player)
{
  //  Delete MPEG decoder
  sceMpegDelete (&player->mp);

  free (player->bs);
  free (player->tex.Data);

  free (player);
}

void m2v_frame_advance (m2v_player_t *player)
{
  int ret;
  u_char* DestData;
  int i, j, mbh;

  if (sceMpegIsEnd (&player->mp))
  {
    // Reset MPEG decoder to the initial state
    sceMpegReset (&player->mp);
    sceMpegAddBs (&player->mp, player->bs, player->bsSize);
  }

  if ((ret = sceMpegGetPicture (&player->mp, player->rgb32, MAX_M2V_MBX*MAX_M2V_MBY)) < 0)
  {
    nglPrintf ("sceMpegGetPicture Failed\n");
    return;
  }

	// Initialize texture
	if (player->mp.frameCount == 0)
  {
    // for the first frame
    player->tex.Width = 256;
    player->tex.WidthBit = 8;
    player->tex.Height = 256;
    player->tex.HeightBit = 8;

    player->tex.Format = SCE_GS_PSMCT32;
    player->tex.DataSize = player->tex.Width * player->tex.Height * 4;
    if (!player->tex.Data)
//      player->tex.Data = (u_int*)nglMemAlloc( player->tex.DataSize, 128 );
      player->tex.Data = (u_int*)memalign( 128, player->tex.DataSize );

    player->tex.SrcDataSize = player->tex.DataSize;

    // Convert datasize to GS local memory units.
    player->tex.DataSize /= 256;
    player->tex.ImageSize = player->tex.DataSize;
    player->tex.PaletteSize = 0;

    player->tex.Type = NGLTEX_TGA;
    player->tex.GsBaseTBP = player->tex.PaletteAddr = 0;
  }

  DestData = (u_char*)player->tex.Data;
  mbh = player->mp.height >> 4;

  // convert the macroblocks to flat texture data (assuming a 256x256 texture for now)
  for (i = 0; i < 256; i++)
  {
    int x, y;

    x = i / mbh;
    y = i % mbh;

    if (x * 16 >= 256)
      continue;

    for (j = 0; j < 16; j++)
      memcpy (DestData + x * 16 * 4 + (y * 16 + j) * 256 * 4, (char*)player->rgb32 + (((i * 256) + (j * 16)) * 4), 16 * 4);
  }
}

// ============================================================================

// Read data from a file
static int readFile (const char *filename, unsigned char **data, int *size)
{
  int fd;
  int ret = 1;
  int readcount;

  //  open a file to read and check its size
  fd = sceOpen (filename, SCE_RDONLY);
  if (fd < 0)
  {
    nglPrintf ("Cannot open %s\n", filename);
    return 0;
  }

  *size = sceLseek (fd, 0, SCE_SEEK_END);
  if (*size < 0)
  {
    nglPrintf ("sceLseek() fails (%s)\n", filename);
    sceClose (fd);
    return 0;
  }

  ret = sceLseek (fd, 0, SCE_SEEK_SET);
  if (ret < 0)
  {
    nglPrintf ("sceLseek() fails (%s)\n", filename);
    sceClose (fd);
    return 0;
  }

  // Allocate memory dynamically
  *data = (u_char*)memalign(64, *size);
  if (*data == NULL)
  {
    nglPrintf ("memalign(%d) fails\n", *size);
    sceClose (fd);
    return 0;
  }

  // read bs data
  readcount = sceRead (fd, *data, *size);
  if (readcount != *size)
  {
    nglPrintf ("Cannot read %s\n", filename);
    sceClose (fd);
    free (*data);
    return 0;
  }

  sceClose(fd);
  return 1;
}
