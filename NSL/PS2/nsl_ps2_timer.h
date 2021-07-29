#ifndef _NSLPS2TIMERH_ 
#define _NSLPS2TIMERH_ 

#define PS2_CLOCK_SPEED 294912000
#define ONE_SIXTIEETH_SEC PS2_CLOCK_SPEED/60


__inline unsigned int volatile _nslGetCpuCycle(void)
{
    register int result;

    __asm__ volatile ("mfc0 %0,$9" : "=r" (result)); 

    return(result);
}


void _nslCheckFrameRate();

#endif