
#ifndef INCLUDED_CAREER_H
#define INCLUDED_CAREER_H

#include "beachdata.h"
#include "scoringmanager.h"
#include "GameData.h"
#include "surferdata.h"	// for SURFER_LAST
#include "careerdata.h"
#include "cheat.h"
#include "boarddata.h"
#include "compressedphoto.h"

#define CHAIN_SIZE 10
#define NUM_PHOTOS 3
#define MAX_PHOTOS 8

#define SPIN_INCREMENT    2
#define SPEED_INCREMENT   2
#define JUMP_INCREMENT    2
#define BALANCE_INCREMENT 2
#define NUM_EARNED_TRICKS_PER_SURFER 14  // number of tricks unlockable with special meter challenges
#define NUM_LEARNED_TRICKS_PER_SURFER 2  // the number of tricks the surfer can learn through GOAL_LEARN_NEW_TRICK
#define NUM_INITIAL_TRICKS_PER_SURFER 2  // the number of special tricks the surfer knows at the start of a career

// Career object: contains career state data.
// This object is serializable.
/*	Not supported by PS2 compiler (dc 01/21/02)
#ifndef TARGET_GC
#pragma pack(1)
#endif
*/

// Not perfect.. but pretty good. 
#define CAREER_DATA_VERSION  sizeof(Career)
class Career
{
public:
	class Level
	{
	private:
		int my_id;
		bool is_new;
		bool advanced;
		bool unlocked;
		bool goals[MAX_GOALS_PER_LEVEL];
	public:
		Level();

		void Reset();
		void ResetGoals();

		void SetMyId(int id) {my_id = id;}
		void Unlock(void);
		void SetGoalDone(const int goalIdx);
		void CheckUnlockNextLevels(const int levelIdx); // unlock next levels if their all previous levels have been completed
		void Advance() { advanced = true; }

		int GetNumGoalsDone() const;
		int GetNextAvailableLevel() const; // returns the index of the next level in the 
		
		bool IsAdvanced(void) const { return advanced; };
		bool IsGoalDone(const int goalIdx) const;
		bool IsUnlocked(void) const { return unlocked; }
		bool IsNew(void) const { return is_new; }
		bool IsAdvanceGoalDone() const { return goals[0]; }
		bool IsBoardGoalDone() const;
	};
	
	class Beach
	{
	private:
		int my_id;
		bool unlocked;
		bool shown;
	public:
		Beach();

		void Reset();

		void SetMyId(int id) {my_id = id;}
		void Unlock(void);
		void Show(void) { shown = true; }
		
		bool IsShown(void) const { return shown; }
		bool IsUnlocked(void) const { return unlocked; }
	};

	class Location
	{
	private:
		int my_id;
		bool movie_unlocked;
		bool movie_shown;
		bool board_unlocked;

	public:
		Location();

		void Reset();
		void SetMyId(int id) { my_id           = id;   }
		void UnlockBoard();
		void UnlockMovie();
		void SetMovieShown();
		bool CheckShowMovie(); // show the location movie if it hasn't been shown yet

		bool IsBoardUnlocked() const { return board_unlocked; }
		bool IsMovieUnlocked() const { return movie_unlocked; }
		bool IsMovieShown() const    { return movie_shown;    }
	};

private:
	// changeable surfer-specific data
	int spin, speed, jump, balance;
	int current_beach;
	int my_id;
	int my_board_id;	// last board used
	char myInitials[4];
	int num_special_challenges_completed;
	bool using_personality_suit;

	// the number of special tricks that have been unlocked with special meter challenge
	int num_earned_tricks; 
	// all the special tricks that have been unlocked
	bool learned_tricks[NUM_EARNED_TRICKS_PER_SURFER + NUM_LEARNED_TRICKS_PER_SURFER]; 

	bool new_goal_passed[MAX_GOALS_PER_LEVEL]; // true if a new goal has been passed since the last call to startNewRun()
	bool goal_passed[MAX_GOALS_PER_LEVEL];     // true if the goal has been passed since the last call to startNewRun()
	bool boards[MAX_BOARDS];                   // true if the board is available
	int  totalStatPointsIncreased;

	CompressedPhoto compressedPhotos[MAX_PHOTOS];

	// array of level indices in the order the main goals were completed
	int levels_completed_order[LEVEL_LAST];

	int total_env_challenges;  // set this in the constructor, after career is loaded
	int env_challenges_passed; // the total number of env challenges that have been passed.  When this
	                           // is equal to total_env_challenges the bails movie should be unlocked.

	// The following vars are all -1 if the thing hasn't been unlocked yet.  
	// Otherwise, they correspond to the goal that unlocked the thing
	int super_board_was_unlocked;  // true if in this run the super board was unlocked
	int espn_movie_was_unlocked;
	int bails_movie_was_unlocked;

	bool awarded[MAX_GOALS_PER_LEVEL];  // each one is true if the corresponding reward was awarded on this run
	                                    // Note that this isn't necessarily the case, since, for instance, a
	                                    // special surfer may have already been unlocked before.
public:
	// all the object arrays that are accessible by the caller
	Level    levels[LEVEL_LAST];
	Beach    beaches[BEACH_LAST];
	Location locations[MAP_LOC_LAST];
	bool IsStarted()  {  return my_id != SURFER_LAST; }

	// Annoying But True(TM): we have to have this one be public :(
	ksConfigData cfg;

	Career();
	
	void init();
	void StartNewCareer();
	void ClearAllGoals(); // clears all the goal completion from all levels (for E3)
	bool IsTrickOpen(int trick_num, bool ignore_current_level = false);

	void IncreaseSpin(int inc);
	void IncreaseSpeed(int inc);
	void IncreaseJump(int inc);
	void IncreaseBalance(int inc);
	int GetSpin()    { return spin;    }
	int GetSpeed()   { return speed;   }
	int GetJump()    { return jump;    }
	int GetBalance() { return balance; }
	stringx GetInitials();
	void    SetInitials(stringx initials);
	CompressedPhoto* GetPhotoForLevel (int index);
	bool PhotoExistsForLevel (int level);
	void SavePhotoForLevel (CompressedPhoto* photo, int level);

	void SetMyId(int id);
	void SetMyBoardId(int id) { my_board_id = id; }
	int GetMyBoardId(void) { return my_board_id; }
	void UnlockBoard(const int boardIdx);
	bool IsBoardUnlocked(const int boardIdx) const;
	bool IsMapUnlocked(const int mapIdx) const;
	void OnGoalReDone(const int levelIdx, const int goalIdx);
	void OnGoalDone(const int levelIdx, const int goalIdx);
	void SetCurrentBeach(int beachIdx) { current_beach = beachIdx; }
	int  GetCurrentBeach() { return current_beach; }
	void StartNewRun();
	bool WasNewGoalPassed(int goalIdx = -1); // returns true if a new goal was passed since last call to StartNewRun().
	bool WasAnyGoalPassed(int goalIdx = -1); // returns true if a previously passed goal was passed since last call to StartNewRun().
	int  WasNewLevelUnlocked();              // returns true if a new level was unlocked since last call to StartNewRun().
	int  WasNewBeachUnlocked();              // returns true if a new beach was unlocked since last call to StartNewRun().
	bool WasAwarded(int goalIdx) const { return awarded[goalIdx]; }
	bool WasSuperBoardUnlocked() const { return super_board_was_unlocked != -1; }
	bool WasBailsMovieUnlocked() const { return bails_movie_was_unlocked != -1; }
	bool WasEspnMovieUnlocked() const  { return espn_movie_was_unlocked  != -1; }
	int  GoalThatUnlockedSuperBoard() const { return super_board_was_unlocked; }
	int  GoalThatUnlockedBailsMovie() const { return bails_movie_was_unlocked; }
	int  GoalThatUnlockedEspnMovie()  const { return espn_movie_was_unlocked; }
	void EarnNextTrick();
	void UnlockTrick(int which_one);
	void MarkClean(); // Mark clean on save
	bool IsDirty();   // Mark dirty any time we change something
	static bool GetGoalText(int level_num, int goal_num, stringx &goal_text);
	bool GetRewardText(int level_num, int goal_num, stringx &reward_text);
	int  GetSurferIdx();
	bool IsUsingPersonality() { return using_personality_suit; }
	void SetUsingPersonality(bool ups) { using_personality_suit = ups; }
	int  GetNumEarnedTricks() const { return num_earned_tricks; }
	bool AllEnvChallengesDone() const { return total_env_challenges == env_challenges_passed; }
	int GetNumEnvChallengesDone() const { return env_challenges_passed; }
	int	 GetLevelCompletedAt(int index) { return levels_completed_order[index]; }
protected:
	bool IsAdvanceGoalDone(const int levelIdx) const { return levels[levelIdx].IsGoalDone(0); }
	void CheckUnlockNextLevels(const int levelIdx);
};

/*	Not supported by PS2 compiler (dc 01/21/02)
#ifndef TARGET_GC
#pragma pack()
#endif
*/

// Global career vairable.  See career.cpp
extern Career *g_career;

#endif INCLUDED_CAREER_H
