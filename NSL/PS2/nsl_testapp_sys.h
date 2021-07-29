
void systemInit (void);

typedef enum {
  NSL_TESTBUTTON_START = 0,
  NSL_TESTBUTTON_B1,
  NSL_TESTBUTTON_B2,
  NSL_TESTBUTTON_B3,
  NSL_TESTBUTTON_B4,
  NSL_TESTBUTTON_B5,
  NSL_TESTBUTTON_B6,
  NSL_TESTBUTTON_B7,
  NSL_TESTBUTTON_B8,
  NSL_TESTBUTTON_B9,
  NSL_TESTBUTTON_B10,
  NSL_TESTBUTTON_B11,
  NSL_TESTBUTTON_B12,
  NSL_TESTBUTTON_NUM
} nslTestButtonEnum;

// mappings for the PS2
typedef enum {
  NSL_PS2_TESTBUTTON_START = NSL_TESTBUTTON_START,
  NSL_PS2_TESTBUTTON_TR =    NSL_TESTBUTTON_B1,
  NSL_PS2_TESTBUTTON_O =     NSL_TESTBUTTON_B2,
  NSL_PS2_TESTBUTTON_X =     NSL_TESTBUTTON_B3,
  NSL_PS2_TESTBUTTON_SQ =    NSL_TESTBUTTON_B4,
  NSL_PS2_TESTBUTTON_UP =    NSL_TESTBUTTON_B5,
  NSL_PS2_TESTBUTTON_DOWN =  NSL_TESTBUTTON_B6,
  NSL_PS2_TESTBUTTON_LEFT =  NSL_TESTBUTTON_B7,
  NSL_PS2_TESTBUTTON_RIGHT = NSL_TESTBUTTON_B8,
  NSL_PS2_TESTBUTTON_L1 =    NSL_TESTBUTTON_B9,
  NSL_PS2_TESTBUTTON_L2 =    NSL_TESTBUTTON_B10,
  NSL_PS2_TESTBUTTON_R1 =    NSL_TESTBUTTON_B11,
  NSL_PS2_TESTBUTTON_R2 =    NSL_TESTBUTTON_B12,
  NSL_PS2_TESTBUTTON_NUM =   NSL_TESTBUTTON_NUM
} nslTestButtonPS2MappingEnum;


#include "fifo_queue.h"

void checkControlPad ( fifo_queue<int> *controllerCommands );
int createControllerThread(  );
void start_timer();
int check_time();

#define PS2_CLOCK_SPEED 294912000
const unsigned int PS2_TICK_RATE = PS2_CLOCK_SPEED >> 16;   // This now has ~.25ms accuracy.
const float SECONDS_PER_TICK = 1.0f/PS2_TICK_RATE;


__inline unsigned int volatile get_cpu_cycle(void)
{
    register int result;

    __asm__ volatile ("mfc0 %0,$9" : "=r" (result)); 

    return(result);
}

#define DVD_BOOT
