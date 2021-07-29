#include "global.h"
#include "cheat.h"

const char *CheatCode[CHEAT_LAST] =
{
	"0000000000",	// CHEAT_RAINBOW
	"9999999999",	// CHEAT_TRIPPY,
	"8888888888",	// CHEAT_FREAK_BOY,
	"7878787878",	// CHEAT_TONY_HAWK,
	"8787878787",	// CHEAT_TIKI_GOD,
	"7676767676",	// CHEAT_TRAVIS_PASTRANA,
	"7777777777",	// CHEAT_FPS_CAM,
	"6666666666",	// CHEAT_INCREASE_BALANCE,
	"5555555555",	// CHEAT_SUPER_BALANCE,
	"4444444444",	// CHEAT_INCREASE_JUMP,
	"3333333333",	// CHEAT_SUPER_JUMP
	"4444455555",	// CHEAT_PERFECT_BALANCE,
	"1212121212",	// CHEAT_UNLOCK_CAREER_PERSONALITY,
	"2121212121",	// CHEAT_UNLOCK_ALL_PERSONALITY_SUITS,
	"1313131313",	// CHEAT_UNLOCK_BAILS_MOVIE,
	"3131313131",	// CHEAT_UNLOCK_ESPN_MOVIE,
	"1414141414",	// CHEAT_UNLOCK_ALL_LEVELS,
	"4141414141",	// CHEAT_UNLOCK_ALL_BOARDS,
	"1515151515",	// CHEAT_UNLOCK_ALL_SURFERS,
	"5151515151",	// CHEAT_UNLOCK_ALL_SURFER_TRICKS,
	"3333311111",	// CHEAT_UNLOCK_ALL_SKILL_POINTS
	"1",			// CHEAT_MEGA_CHEAT
};

const char MegaCheatCode[] = "1";

Cheat g_session_cheats[CHEAT_LAST];

int Cheat::checkCodeUnlock(const stringx check)
{
	int i;
	for(i = 0; i < CHEAT_LAST; i++)
		if(CheatCode[i] == check) // if the code matches 
		{
			if(globalCareerData.isCheatUnlocked(i)) // If it is already unlocked
				return CHEAT_ALREADY_UNLOCKED;
			//globalCareerData.unlockCheat(i); // unlock the cheat
			assert(i < CHEAT_LAST);
			g_session_cheats[i].setLockedState(false);
			g_session_cheats[i].turnOn(true);
			debug_print("Unlocked cheat #%d with the following code: %s\n", i, CheatCode[i]);
			return i;
		}

	// Now check the mega cheat
	//if(checkMegaCodeUnlock(check))
		//return CHEAT_LAST;

	return CHEAT_INVALID_CODE;
}

bool Cheat::checkMegaCodeUnlock(const stringx check)
{

	if(MegaCheatCode == check) // if the code matches
	{
		globalCareerData.unlockEverything();
		debug_print("Unlocked all the cheats, beaches, boards, etc.\n");
		return true;
	}

	return false;
}

int Cheat::getNumUnlocked()
{
	int i, sum = 0;
	for(i = 0; i < CHEAT_LAST; i++)
		if(globalCareerData.isCheatUnlocked(i))
			sum++;

	return sum;
}