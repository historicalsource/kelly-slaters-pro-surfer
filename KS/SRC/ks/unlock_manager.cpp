#include "unlock_manager.h"
#include "global.h"
#include "career.h"
//#include "FrontEndManager.h"

UnlockingManager unlockManager;

bool UnlockingManager::isSurferUnlocked(int surfer) const
{
	if (g_session_cheats[CHEAT_MEGA_CHEAT].isOn() || g_session_cheats[CHEAT_UNLOCK_ALL_SURFERS].isOn())
		return true;

	if (g_session_cheats[CHEAT_FREAK_BOY].isOn() && (surfer == SURFER_SURFREAK))
		return true;

	if (g_session_cheats[CHEAT_TONY_HAWK].isOn() && (surfer == SURFER_TONY_HAWK))
		return true;

	if (g_session_cheats[CHEAT_TIKI_GOD].isOn() && (surfer == SURFER_TIKI_GOD))
		return true;

	if (g_session_cheats[CHEAT_TRAVIS_PASTRANA].isOn() && (surfer == SURFER_TRAVIS_PASTRANA))
		return true;

	return globalCareerData.isSurferUnlocked(surfer);
}

bool UnlockingManager::isSurferPersUnlocked(int surfer) const
{
	return (g_session_cheats[CHEAT_MEGA_CHEAT].isOn() || globalCareerData.isSurferPersUnlocked(surfer) ||
			g_session_cheats[CHEAT_ALL_PERSONALITY_SUITS].isOn());
}

bool UnlockingManager::isSurferTrickUnlocked(int trick, bool ignore_cur_level) const
{
	return (g_session_cheats[CHEAT_MEGA_CHEAT].isOn() ||
			g_session_cheats[CHEAT_UNLOCK_ALL_SURFER_TRICKS].isOn() || g_career->IsTrickOpen(trick, ignore_cur_level));
}

bool UnlockingManager::isSurferMovieUnlocked(int surfer) const
{
	return (g_session_cheats[CHEAT_MEGA_CHEAT].isOn() || globalCareerData.isSurferMovieUnlocked(surfer));
}

bool UnlockingManager::isLevelUnlocked(int Level) const
{
	return (g_session_cheats[CHEAT_MEGA_CHEAT].isOn() || g_session_cheats[CHEAT_ALL_LEVELS].isOn() ||
			g_career->levels[Level].IsUnlocked());
}

bool UnlockingManager::isBeachUnlocked(int Level) const
{
	if (g_session_cheats[CHEAT_MEGA_CHEAT].isOn() || g_session_cheats[CHEAT_ALL_LEVELS].isOn())
		return true;

	int mode;
	bool in_game = frontendmanager.fe_done;
	if (in_game)
		mode = g_game_ptr->get_game_mode();
	else
		mode = frontendmanager.tmp_game_mode;

	if (mode == GAME_MODE_CAREER)
		return g_career->beaches[Level].IsUnlocked();
	else
		return globalCareerData.isBeachUnlocked(Level);
}

bool UnlockingManager::isSurferBoardUnlocked(int surfer, int board) const
{
	if (g_session_cheats[CHEAT_MEGA_CHEAT].isOn() || g_session_cheats[CHEAT_ALL_BOARDS].isOn())
		return true;

	if (board == 0)
		return true;

	int mode;
	bool in_game = frontendmanager.fe_done;
	if (in_game)
		mode = g_game_ptr->get_game_mode();
	else
		mode = frontendmanager.tmp_game_mode;

	if (mode == GAME_MODE_CAREER)
		return g_career->IsBoardUnlocked(board);
	else
		return globalCareerData.isSurferBoardUnlocked(surfer, board);
}

bool UnlockingManager::isBeachBoardUnlocked(int Beach) const
{
	return globalCareerData.isBeachBoardUnlocked(Beach);
}

bool UnlockingManager::isLocationBoardUnlocked(int Location) const
{
	if (g_session_cheats[CHEAT_MEGA_CHEAT].isOn() || g_session_cheats[CHEAT_ALL_BOARDS].isOn())
		return true;

	int mode;
	bool in_game = frontendmanager.fe_done;
	if (in_game)
		mode = g_game_ptr->get_game_mode();
	else
		mode = frontendmanager.tmp_game_mode;

	if (mode == GAME_MODE_CAREER)
		return g_career->locations[Location].IsBoardUnlocked();
	else
		return globalCareerData.isLocationBoardUnlocked(Location);
}

bool UnlockingManager::isCheatUnlocked(int whichCheat) const
{
	return globalCareerData.isCheatUnlocked(whichCheat);
}

bool UnlockingManager::isLocationMovieUnlocked(const int locationIdx) const
{
	return (g_session_cheats[CHEAT_MEGA_CHEAT].isOn() || globalCareerData.isLocationMovieUnlocked(locationIdx));
}

bool UnlockingManager::isBailsMovieUnlocked(void) const
{
	return (g_session_cheats[CHEAT_MEGA_CHEAT].isOn() || globalCareerData.isBailsMovieUnlocked());
}

bool UnlockingManager::isEspnMovieUnlocked(void) const
{
	return (g_session_cheats[CHEAT_MEGA_CHEAT].isOn() || globalCareerData.isEspnMovieUnlocked());
}
