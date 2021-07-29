#ifndef __GLOBAL_DATA_H
#define __GLOBAL_DATA_H

#include "cheat.h"
#include "boarddata.h"
#include "career.h"

#include "HighScoreFrontEnd.h"

// The highest that the handicap can ever go
#define MAX_HANDICAP 6

struct HighScoreData
{
	char initials[4];
	char character[32];		// character's name
	int score;
	int icons;				// only for icon challenge board
};



class GlobalDataClass
{
	private:
	class SurferData
	{
		public:
		bool boardsUnlocked[MAX_BOARDS];
		bool personalityUnlocked;
		bool tricksUnlocked[NUM_LEARNED_TRICKS_PER_SURFER + NUM_EARNED_TRICKS_PER_SURFER];
		bool unlocked;
		bool movieUnlocked;
		int  handicap;
		bool finished_last_level;
	}; 

	class BeachData
	{
		public:
		HighScoreData high_scores[HighScoreFrontEnd::NUM_ROWS];
		HighScoreData high_icons[HighScoreFrontEnd::NUM_ROWS];
		bool unlocked;
	};

	class LocationData
	{
	public:
		bool movieUnlocked;
		bool movieShown;
		bool boardUnlocked;
	};

	LocationData globalLocations[MAP_LOC_LAST];
	SurferData   globalSurfers[SURFER_LAST];
	BeachData    globalBeaches[BEACH_LAST];
	Cheat        globalCheats[CHEAT_LAST];

	bool bailsMovieUnlocked;
	bool espnMovieUnlocked;
	
public:
	
	GlobalDataClass();
	bool updateFromCareer(Career *c);
	void init();
	bool isSurferUnlocked(int surfer) const;
	void unlockSurfer(int surfer);
	
	bool isSurferPersUnlocked(int surfer) const;
	void unlockSurferPers(int surfer);
	
	bool isSurferBoardUnlocked(int surfer, int board) const;
	void unlockSurferBoard(int surfer, int board);

	bool isSurferTrickUnlocked(int surfer, int trick) const;
	void unlockSurferTrick(int surfer, int trick);

	bool isSurferMovieUnlocked(int surfer) const;
	void unlockSurferMovie(int surfer);

	bool isBeachUnlocked(int Level) const;
	void unlockBeach(int Level);

	bool isBeachBoardUnlocked(int Beach) const;
	void unlockBeachBoard(int Beach);

	// Do the same as beach, but diff arg
	bool isLocationBoardUnlocked(int Location) const;
	void unlockLocationBoard(int Location);

	bool isCheatUnlocked(int whichCheat) const;
	void unlockCheat(int whichCheat);

	bool isLocationMovieUnlocked(const int locationIdx) const { return globalLocations[locationIdx].movieUnlocked; }
	void unlockLocationMovie(const int locationIdx) { globalLocations[locationIdx].movieUnlocked = true; }

	bool setLocationMovieShown(const int locationIdx) const { return globalLocations[locationIdx].movieShown; }
	void isLocationMovieShown(const int locationIdx) const;

	bool isBailsMovieUnlocked() const { return bailsMovieUnlocked; }
	void unlockBailsMovie() { bailsMovieUnlocked = true; }

	bool isEspnMovieUnlocked() const { return espnMovieUnlocked; }
	void unlockEspnMovie() { espnMovieUnlocked = true; }

	void setMaxHandicap(int surferIdx, int hcap);
	int  getMaxHandicap(int surferIdx);

	bool isLastLevelCompleted(int surferIdx) const { return globalSurfers[surferIdx].finished_last_level; }
	void setLastLevelCompleted(int surferIdx) { globalSurfers[surferIdx].finished_last_level = true; }

	HighScoreData getBeachHighScore(int beach, int whichScore, bool icons);
	void          setBeachHighScore(int beach, int whichScore, bool icons, HighScoreData newScore);
	
	void unlockEverything();
};

extern GlobalDataClass globalCareerData;
#endif