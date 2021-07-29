
#ifndef INCLUDED_SCORINGMANAGER_H
#define INCLUDED_SCORINGMANAGER_H

#include "global.h"
#include <list>
#include "types.h"
#include "trickdata.h"
#include "VOEngine.h"
#include "eventmanager.h"
#include "specialmeter.h"

class kellyslater_controller;
#define BIG_CHEER_POIMTS 250000
#define MED_CHEER_POINTS 50000
#define SM_CHEER_POINTS  10000
class ScoringManager : EventRecipient
{
public:
	
	// Moved here to allow for manual editing in the debug menu
	int				score;					// player's total socre for this level
	
	static float	MOUTH_DISTANCES[2];

	static float	SCALE_MOUTH_DISTS[3];
	static float	SCALE_LIP_DISTS[3];
	static float	SCALE_SPINS[9];
	static float	SCALE_LANDINGS[4];
	static float	SCALE_SERIES_MODS[2];
	static float	SCALE_REPEATS[5];
	static float	SCALE_REPEATS_FACE[5];
	static float	SCALE_AT_TUBE;
	static float	SCALE_WAVE[3];
	static int		METER_FILL_RATE;
	static int		METER_FILL_RATE_FACE;
	static float	METER_PENALTY_SLOPPY;
	static float	METER_PENALTY_JUNK;

	static int		CAP_LINK;
	static int		CAP_COOL;

	enum SPECIALTRICK
	{
		SPECIALTRICK_NONE,		// special trick not attempted
		SPECIALTRICK_SUCCEEDED,	// special trick performed, dolphin is present
		SPECIALTRICK_FAILED		// special trick failed, shark is present
	};

	enum LANDING
	{
		LAND_PERFECT,
		LAND_REGULAR,
		LAND_SLOPPY,
		LAND_JUNK,
	};

	// Trick flags.
	enum
	{
		MOD_AT_MOUTH,
		MOD_LAME,
	};

	// Series flags.
	enum
	{
		MOD_FROM_FLOATER,
		MOD_TO_FAKEY,
		MOD_HOP,
	};

	enum ATTRIBUTE
	{
		ATTR_DIST_LIP,				// float [0, 1] : sets distance from lip of wave
		ATTR_DIST_MOUTH,			// float [0, 1] : sets distance from mouth of tube
		ATTR_TIME_TOTAL,			// float : sets trick time
		ATTR_TIME_DELTA,			// float : adds to trick time
		ATTR_TO_FAKEY,				// bool  : true if series was landed fakey
		ATTR_FROM_FLOATER,			// bool  : true if series was done off a floater
		ATTR_HOP,					// bool  : true if series was done off a chop hop
		ATTR_LAME,					// bool  : true if trick is lame
		ATTR_AT_MOUTH,				// bool  : sets if trick was done facing mouth or not
		ATTR_NUM_SPINS_DELTA,		// int   : adds to series' number of spins
		ATTR_NUM_SPINS_TOTAL,		// int   : sets series' number of spins
		ATTR_LANDING,				// int   : sets series' landing type (enum LANDING)
		ATTR_FLAG_SET,				// int   : sets a trick or series flag to true
		ATTR_FLAG_UNSET,			// int   : sets a trick or series flag to false
	};

public:
	struct CHAININFO
	{
		int		points;			// total score for the chain
		int		facePoints;		// score of all face tricks in chain
		int		airPoints;		// score of all air tricks in chain
		int		tubePoints;		// score of all tube tricks in chain
		int		numTricks;
		bool	multiLocation;

		CHAININFO() { Reset(); }
		void Reset(void)
		{
			points = 0;
			numTricks = 0;
			multiLocation = false;
			facePoints = 0;
			airPoints = 0;
			tubePoints = 0;
		}
	};

	struct LevelTrick
	{
		int	numLandings;		// how many times this trick was performed in this level
	};

	// Trick: the atomical unit of a chain.
	class Trick
	{
	public:
		enum TYPE { TYPE_TRICK, TYPE_GAP };

	public:
		TYPE		type;			// normal trick or gap
		int			index;			// index into GTrickList or g_gapList
		int			flags;			// lame?
		float		time;			// amount of time trick was held (in seconds)
		int			numSpins;		// only applies to face tricks
		float		mouthDist;		// distance from the mouth of the tube [0, 1]
		float		lipDist;		// distance from the lip of the wave [0, 1]
		int			repetitions;	// records the repetitions per level

		// Creators.
		Trick();
		~Trick() { }

		// Accesors.
		int GetRawScore(const LevelTrick * levelTricks, const bool applySick = true) const;
		float GetRawSickness(const LevelTrick * levelTricks) const;
		bool IsInteresting(void) const;
		stringx GetText(void) const;

		// Operators.
		Trick & operator=(const Trick & right);
	};
	friend class Trick;
	typedef list<Trick>		TrickList;

	// Series: a grouping of tricks (usually a few arials)
	class Series
	{
	friend class ScoringManager;

	private:
		LevelTrick *	levelTricks;

	public:
		TrickList		tricks;
		int				numSpins;
		LANDING			landing;
		int				flags;		// fakey? from floater?

	private:
		float GetScale(void) const;

	public:
		// Creators.
		Series();
		~Series() { }

		// Accesors.
		int GetRawScore(void) const;
		void GetPartialRawScores(int & facePoints, int & airPoints, int & tubePoints) const;
		float GetRawSickness(void) const;
		int GetNumMultTricks(void) const;
		bool IsInteresting(void) const;
		bool HasTrick(const int flags) const;
		int GetTrickCount(const int trickIdx) const;
		float GetTubeTime(void) const;

		// Operators.
		Series & operator=(const Series & right);
	};
	friend class Series;
	typedef list<Series>	SeriesList;

	// Chain: an entire trick combo.
	class Chain
	{
	friend class ScoringManager;

	private:
		LevelTrick *	levelTricks;
		float			multAdder;

	public:
		SeriesList	series;

	private:
		float GetScale(void) const;

	public:
		// Creators.
		Chain();
		~Chain() { }

		// Accesors.
		int GetScore(void) const;
		float GetSickness(void) const;
		int GetRawScore(void) const;
		float GetRawSickness(void) const;
		void GetPartialScores(int & facePoints, int & airPoints, int & tubePoints) const;
		void GetPartialRawScores(int & facePoints, int & airPoints, int & tubePoints) const;
		float GetMultiplier(void) const;
		int GetNumTricks(void) const;
		bool IsInteresting(void) const;
		bool HasTrick(const int flags) const;
		int GetTrickCount(const int trickIdx) const;
		
		// Modifiers.
		void SetMultAdder(float m);  // Needed for tetris icon
		void AddMultAdder(float m);  // Needed for tetris icon

		// Operators.
		Chain & operator=(const Chain & right);
	};
	friend class Chain;

protected:
	kellyslater_controller *	ksctrl;
	int				playerIdx;
	SpecialMeter *	specialMeter;

	LevelTrick		levelTricks[TRICK_NUM];	// level's trick repetitions
	Chain			bestChain;				// best successful chain for this level
	int				bestChainScore;			// points for best chain
	float			longestTubeRide;		// longest tube ride for this level
	float			longestFloater;
	float			longestAir;				// longest air time for this level (wtf?)
	
	Chain			chain;					// current chain of tricks
	
	
	int				facePoints;				// player's score from face tricks
	int				airPoints;				// player's score from air tricks
	int				tubePoints;				// player's score from tube tricks
	int				num360spins;			// number of 360s (or better) that the player did.
	int				num540spins;			// number of 540s (or better) that the player did.
	
	TRICKREGION		prevTrickRegion;		// player's trick region in the previous frame
	float			lipDist;				// player's current distance from the wave lip [0, 1]
	float			mouthDist;				// player's current distance from the mouth of the tube [0, 1]
	
	SPECIALTRICK	specialTrick;			// special trick performed?
	
	CHAININFO		lastChainInfo;

public:
  friend class IGOIconManager;

	// Creators.
	ScoringManager();
	~ScoringManager();

	// Modifiers.
	static void stl_prealloc(void);

	void SetKsctrl(kellyslater_controller * ks);
	void Reset(void);
	void Update(const float dt);
	void OnEvent(const EVENT event, const int param1 = 0, const int param2 = 0);

	void AddTrick(const int trickIdx);
	void AddGap(int gapIdx);

	void UpdateLastTrick(const ATTRIBUTE attr, const int i, const int flags = 0);
	void UpdateLastTrick(const ATTRIBUTE attr, const float f, const int flags = 0);
	void UpdateLastTrick(const ATTRIBUTE attr, const bool b, const int flags = 0);
	void UpdateLastSeries(const ATTRIBUTE attr, const int i);
	void UpdateLastSeries(const ATTRIBUTE attr, const bool b);

	void SetMouthDist(const float dist);
	void SetLipDist(const float dist);
	void SetScore(const int s) { score = s; }
	void AddPoints(const int s) { score += s; }

	// Accessors.
	void FinishChain(const bool successful = false);
	const Chain & GetChain(void) const { return chain; }
	int GetScore(void) const { return score; }
	void GetPartialScores(int & fPoints, int & aPoints, int & tPoints) const { fPoints = facePoints; aPoints = airPoints; tPoints = tubePoints; }
	int Get360Spins() const { return num360spins; }
	int Get540Spins() const { return num540spins; }
	int GetNumTrickLandings(void) const;
	const Chain & GetBestChain(void) const { return bestChain; }
	int GetBestChainScore(void) const { return bestChainScore; }
	float GetLongestTubeRide(void) const { return longestTubeRide; }
	float GetLongestFloater(void) const { return longestFloater; }
	float GetLongestAir(void) const { return longestAir; }
	SPECIALTRICK GetSpecialTrickState(void) const { return specialTrick; }
	float GetMouthDist(void) const { return mouthDist; }
	float GetLipDist(void) const { return lipDist; }
	CHAININFO * GetLastChainInfo(void) { return &lastChainInfo; }
};

extern void LoadScoringSystem(void);
extern void SaveScoringSystem(void);

#endif INCLUDED_SCORINGMANAGER_H