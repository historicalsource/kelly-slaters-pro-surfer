#include "global.h"

#include "osdevopts.h"

float TIMER_TotalSec = 0.0f;
float TIMER_FrameSec = 0.0f;
float TIMER_LevelSec = 0.0f;
float TIMER_LevelDuration = 0.0f;
bool TIMER_InfiniteDuration = false;

static struct 
{
	u_int ConstantFrameRate : 1;
}
TimerDebug = 
{
	0, // ConstantFrameRate
};

void TIMER_Reset(void)
{
	TIMER_TotalSec = 0.0f;
	TIMER_LevelSec = 0.0f;
}

void TIMER_Init(void)
{
	//  default is to use what's in the beach database.
	if ((float) os_developer_options::inst()->get_int(os_developer_options::INT_LEVEL_DURATION) != -1)
		TIMER_LevelDuration = (float) os_developer_options::inst()->get_int(os_developer_options::INT_LEVEL_DURATION);
	else if(g_game_ptr->get_game_mode() == GAME_MODE_CAREER)
		// If we're in career mode get the duration from the career database
		TIMER_LevelDuration = CareerDataArray[g_game_ptr->get_level_id()].level_duration;
	else // not in career mode, so get it from the beach database
		TIMER_LevelDuration = BeachDataArray[g_game_ptr->get_beach_id()].level_duration;

	if (TIMER_LevelDuration <= 0)
	{
		TIMER_LevelDuration = 0;
		TIMER_InfiniteDuration = true;
	}
	else 
	{
		TIMER_InfiniteDuration = false;
	}

	TIMER_Reset();
}

// New overloaded method: specify the level duration.
void TIMER_Init(const float duration)
{
	TIMER_LevelDuration = duration;
	if (TIMER_LevelDuration <= 0)
	{
		TIMER_LevelDuration = 0;
		TIMER_InfiniteDuration = true;
	}
	else 
	{
		TIMER_InfiniteDuration = false;
	}

	TIMER_Reset();
}

void TIMER_Tick(const time_value_t time_inc, const bool levelTick)
{
#if defined(TARGET_XBOX)
	assert( time_inc > 0.0f );
#endif /* TARGET_XBOX JIV DEBUG */
	
	if (TimerDebug.ConstantFrameRate)
	{
		TIMER_FrameSec = 1.0f / 60.f;
	}
	else
	{
		TIMER_FrameSec = time_inc;
	}
	
	TIMER_TotalSec += TIMER_FrameSec;
	if (levelTick) TIMER_LevelSec += TIMER_FrameSec;
}

void TIMER_NoTick(void)
{
	TIMER_FrameSec = 0.0f;
}

void TIMER_SetTotalSec(const float total_sec, const float frame_sec)
{
	TIMER_TotalSec = total_sec;
	TIMER_FrameSec = frame_sec;
}

void TIMER_SetLevelSec(const float sec)
{
	TIMER_LevelSec = sec;
}

/*	Not currently used (dc 06/10/02)
void KSTimerState::Save( void )
{
TotalSec = TIMER_TotalSec;
FrameSec = TIMER_FrameSec;
LevelSec = TIMER_LevelSec;
}

void KSTimerState::Restore( void )
{
	TIMER_TotalSec = TotalSec;
	TIMER_FrameSec = FrameSec;
	TIMER_LevelSec = LevelSec;
}

void KSTimerState::Reset( void )
{
}

void KSTimerState::Pause( void )
{
	TIMER_FrameSec = 0;
}


void KSTimerState::WriteToDisk( os_file dataFile )
{
	dataFile.write((void *)&FrameSec, sizeof(float));
	dataFile.write((void *)&TotalSec, sizeof(float));
	dataFile.write((void *)&LevelSec, sizeof(float));
}

void KSTimerState::ReadFromDisk( os_file dataFile )
{
	dataFile.read((void *)&FrameSec, sizeof(float),   0);
	dataFile.read((void *)&TotalSec, sizeof(float),   0);
	dataFile.read((void *)&LevelSec, sizeof(float),   0);
}
*/

