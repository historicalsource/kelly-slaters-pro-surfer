#include "xb_movie_defs.h"
#include "xb_movie_audiodec.h"

#include <stdio.h>

#define STUB(str) _STUB(str,__FILE__, __LINE__)
#define _STUB(str,f,l) fprintf( stderr, "[%s:%d] %s\n", f,l,str );

int audioDecDelete(AudioDec *ad)
{
  STUB( "audioDecDelete" );

  return 0;
}

void audioDecBeginPut(AudioDec *ad,
	u_char **ptr0, int *len0, u_char **ptr1, int *len1)
{
  STUB( "audioDecBeginPut" );

  return;
}

void audioDecEndPut(AudioDec *ad, int size)
{
  STUB( "audioDecEndPut" );

  return;
}

int audioDecIsPreset(AudioDec *ad)
{
  STUB( "audioDecIsPresent" );

  return 0;
}

void audioDecStart(AudioDec *ad)
{
  STUB( "audioDecStart" );

  return;
}

int audioDecSendToIOP(AudioDec *ad)
{
  STUB( "audioDecSendToIOP" );

  return 0;
}

void audioDecReset(AudioDec *ad)
{
  STUB( "audioDecReset" );

  return;
}

void audioDecPause(AudioDec *ad)
{
  STUB( "audioDecPause" );

  return;
}

void audioDecResume(AudioDec *ad)
{
  STUB( "audioDecResume" );

  return;
}

