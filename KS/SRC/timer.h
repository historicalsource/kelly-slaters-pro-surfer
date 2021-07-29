#ifndef TIMER_H
#define TIMER_H

#if defined(TARGET_XBOX)
#include "hwosxb/xb_timer.h"
#else
#include "hwosps2/ps2_timer.h"
#endif /* TARGET_XBOX JIV DEBUG */

// Use the accessors below, rather than referencing these directly.  (dc 06/10/02)
extern float TIMER_TotalSec;
extern float TIMER_FrameSec;
extern float TIMER_LevelSec;
extern float TIMER_LevelDuration;
extern bool TIMER_InfiniteDuration;

/*	Not currently used (dc 06/10/02)
class KSTimerState
{
public:
	float TotalSec;
	float LevelSec;
	float FrameSec;
	
	KSTimerState(void) {TotalSec = LevelSec = FrameSec = 0;}
	//virtual ~KSTimerState(void) {}
	
	virtual void Save( void );
	virtual void Restore( void );
	virtual void Reset( void );
	virtual void Pause( void );
	virtual void WriteToDisk( os_file dataFile );
	virtual void ReadFromDisk( os_file dataFile );
};
*/

void TIMER_Init(void);
void TIMER_Init(const float duration);
void TIMER_Reset(void);

void TIMER_Tick(const time_value_t time_inc, const bool tickLevel = true);
void TIMER_NoTick(void);

void TIMER_SetTotalSec(const float total_sec, const float frame_sec);
void TIMER_SetLevelSec(const float sec);

inline float TIMER_GetFrameSec(void) { return TIMER_FrameSec; }
// Returns the total number of seconds that have elapsed since the timer was set.
inline float TIMER_GetTotalSec(void) { return TIMER_TotalSec; }
// Returns the number of seconds that have elapsed in this level.
inline float TIMER_GetLevelSec(void) { return TIMER_LevelSec; }
inline float TIMER_GetRemainingLevelSec(void) 
{ assert (!TIMER_InfiniteDuration); return TIMER_LevelDuration - TIMER_LevelSec; }
inline bool TIMER_IsInfiniteDuration(void) { return TIMER_InfiniteDuration; }

#endif
