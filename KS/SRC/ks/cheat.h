#ifndef __CHEATS_H_
#define __CHEATS_H_

enum cheats_enum
{
	CHEAT_RAINBOW,
	CHEAT_TRIPPY,
	CHEAT_FREAK_BOY,
	CHEAT_TONY_HAWK,
	CHEAT_TIKI_GOD,
	CHEAT_TRAVIS_PASTRANA,
	CHEAT_FPS_CAM,
	CHEAT_INCREASE_BALANCE,
	CHEAT_SUPER_BALANCE,
	CHEAT_INCREASE_JUMP,
	CHEAT_SUPER_JUMP,
	CHEAT_PERFECT_BALANCE,
	CHEAT_CAREER_PERSONALITY_SUIT,
	CHEAT_ALL_PERSONALITY_SUITS,
	CHEAT_BAILS_MOVIE,
	CHEAT_ESPN_MOVIE,
	CHEAT_ALL_LEVELS,
	CHEAT_ALL_BOARDS,
	CHEAT_UNLOCK_ALL_SURFERS,
	CHEAT_UNLOCK_ALL_SURFER_TRICKS,
	CHEAT_UNLOCK_ALL_SKILL_POINTS,
	CHEAT_MEGA_CHEAT,
	CHEAT_LAST
};

#define CHEAT_INVALID_CODE     -1
#define CHEAT_ALREADY_UNLOCKED -2

extern const char *CheatCode[CHEAT_LAST];

class Cheat
{
private:
	bool locked;
	bool on;

public:
	Cheat() { locked = true; on = false;}

	bool getLockedState() const { return locked; }
	void setLockedState(const bool l) { locked = l; }

	void turnOn(const bool turn_on) { on = turn_on; }
	bool isOn() const { return on; }

	static int  checkCodeUnlock(const stringx check);  // returns the -1 if not a match.  Otherwise, returns the cheat # unlocked.
	static bool checkMegaCodeUnlock(const stringx check);
	static int  getNumUnlocked();
};

extern Cheat g_session_cheats[CHEAT_LAST];

#endif // __CHEATS_H_
