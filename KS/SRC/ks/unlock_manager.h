#ifndef __UNLOCK_MANAGER_H_
#define __UNLOCK_MANAGER_H_

#include "Cheat.h"
#include "GlobalData.h"


//  this is basically a wrapper class to check cheats and global data for unlocked items
class UnlockingManager
{
public:

	UnlockingManager() {;}
	~UnlockingManager() {;}

	bool isSurferUnlocked(int surfer) const;
	bool isSurferPersUnlocked(int surfer) const;
	bool isSurferTrickUnlocked(int trick, bool ignore_cur_level = false) const;
	bool isSurferMovieUnlocked(int surfer) const;
	bool isSurferBoardUnlocked(int surfer, int board) const;
	bool isLevelUnlocked(int Level) const;
	bool isBeachUnlocked(int Level) const;
	bool isBeachBoardUnlocked(int Beach) const;
	bool isLocationBoardUnlocked(int Location) const;
	bool isCheatUnlocked(int whichCheat) const;
	bool isLocationMovieUnlocked(const int locationIdx) const;
	bool isBailsMovieUnlocked(void) const;
	bool isEspnMovieUnlocked(void) const;
};

extern UnlockingManager unlockManager;

#endif