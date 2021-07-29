
/************************************
*      KELLYSLATERS PRO SURFER 		*
*	(C) Interactive Republic Corp	*
*									*
*		Author:  Andi Smithers		*
*		  12th October 2000			*
*									*
*        	   aligned.h 			*
*-----------------------------------*
* 									*
*									*
************************************/

#ifndef __KSPS_ALIGNED_H_
#define __KSPS_ALIGNED_H_

#ifdef WIN32

	#define ALIGNED(NUM)

#elif defined(TARGET_GC)
	#define ALIGNED(NUM) __attribute__ ((aligned(NUM)))
//__attribute__((aligned (32)))
#else
	// ps2 aligns macros
	#define ALIGNED(NUM) __attribute__ ((aligned((NUM))))

#endif

#endif//__KSPS_ALIGNED_H_
