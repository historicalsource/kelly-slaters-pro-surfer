/*
 * super-simple NSL Test App scaffolding.
 * 
 * currently configured as a streamer test for some NSL2 profiling and
 * data streaming tests...
 *
 * currently this is ps2 specific, but it wouldn't be hard to port it to
 * another platform.
 */

#include "../common/nsl.h"
#include "nsl_testapp_sys.h"
#include "libcdvd.h"
#include "math.h"
#include "../common/nl.h"
#include "nsl_ps2.h"
#include <sifdev.h>


fifo_queue<int> controllerCommands;

#define MAX_STR_LEN   256


void sleep(float msToSleep)
{
  start_timer();
  while ( check_time() * SECONDS_PER_TICK < msToSleep )
    /*spin*/;
}

// mmmm, tasty globals... what did you expect?  This is a test app.
nslSourceId testStereoStreamSrc = NSL_INVALID_ID;
nslSourceId testMonoStreamSrc = NSL_INVALID_ID;
nslSourceId testSpuSrc = NSL_INVALID_ID;


void process(int whichCommand)
{
  switch(whichCommand)
  {
    case NSL_PS2_TESTBUTTON_START:
      printf ("start pressed\n");
      break;
    case NSL_PS2_TESTBUTTON_TR:
      printf ("triangle pressed\n");
      break;
    case NSL_PS2_TESTBUTTON_O:
      printf ("circle pressed\n");
      break;
    case NSL_PS2_TESTBUTTON_X:
      printf ("X pressed\n");
      // this time, x-pressed will launch a new spu sound
      if ( testSpuSrc != NSL_INVALID_ID )
      {
        nslSoundId testSound = nslAddSound( testSpuSrc );
        if (testSound != NSL_INVALID_ID)
        {
          nslPlaySound( testSound );
        }
      }
      break;
    case NSL_PS2_TESTBUTTON_SQ:
      printf ("square pressed\n");
      break;
    case NSL_PS2_TESTBUTTON_UP:
      printf ("up pressed\n");
      break;
    case NSL_PS2_TESTBUTTON_DOWN:
      printf ("down pressed\n");
      break;
    case NSL_PS2_TESTBUTTON_LEFT:
      printf ("left pressed\n");
      break;
    case NSL_PS2_TESTBUTTON_RIGHT:
      printf ("right pressed\n");
      break;
    case NSL_PS2_TESTBUTTON_L1:
      printf ("L1 pressed\n");
      break;
    case NSL_PS2_TESTBUTTON_L2:
      printf ("L2 pressed\n");
      break;
    case NSL_PS2_TESTBUTTON_R1:
      printf ("R1 pressed\n");
      break;
    case NSL_PS2_TESTBUTTON_R2:
      printf ("R2 pressed\n");
      break;
    default:
      printf("bad button %d, not sure what to do.  ignoring.\n", whichCommand);
      break;
  }
}

#define FRAME_ADVANCE_TIME (16.667) // 60hz in milliseconds

int main( int argc, char *argv[] )
{
  unsigned long int t, p;
  
  systemInit();

  // Init nsl
  nslInit();
  nslSetHostStreamingPS2(true);
  nslSetRootDir("PS2SOUND");
  nslReset( "bonus\\menu" );

  // load in a test sound or two
  testStereoStreamSrc = nslLoadSource( "frontend" );
  testMonoStreamSrc = nslLoadSource( "KR_RIFLE_RELOAD" );
  testSpuSrc = nslLoadSource( "chime" );

  // Init the controller fifo_queue  
  controllerCommands.init(10);
  
  printf("Beginning test.\n");
  while (1)
  {
    while (!controllerCommands.size()) 
    {
      nslFrameAdvance( FRAME_ADVANCE_TIME );
      sleep( FRAME_ADVANCE_TIME );
      checkControlPad( &controllerCommands );
    }
    while (controllerCommands.size() > 0)
    {
      int whichCommand = controllerCommands.pop();
      process(whichCommand);
    }
  }


  nslShutdown();

  return 0;
}
