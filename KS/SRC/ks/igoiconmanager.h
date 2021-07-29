
#ifndef INCLUDED_IGOICONMANAGER_H
#define INCLUDED_IGOICONMANAGER_H

#include "global.h"
#include "ngl.h"

#define MAX_ICON_SETTINGS     5
#define MAX_ICON_SPAWNS       16

// IGOIconManager: manages display of onscreen trick icons
class IGOIconManager
{
public:
  enum
  {
    ICON_BUTTON_ARROW,
    ICON_BUTTON_CROSS,
    ICON_BUTTON_CIRCLE,
    ICON_BUTTON_TRIANGLE,
    ICON_BUTTON_SQUARE,
    ICON_BUTTON_PLUS,
    ICON_BUTTON_COMMA,
    ICON_NUM_BUTTONS
  };

  enum
  {
    // Do not change order
    ICON_TYPE_AIR,
    ICON_TYPE_FACE,
    ICON_TYPE_TUBE,
    ICON_TYPE_SPECIAL_AIR,
    ICON_TYPE_SPECIAL_FACE,
    ICON_TYPE_SPECIAL_TUBE,
    ICON_TYPE_SPECIAL_ANY,
    ICON_TYPE_GENERIC_AIR,
    ICON_TYPE_GENERIC_FACE,
    ICON_TYPE_GENERIC_TUBE,
    ICON_NUM_TYPES
  };

  enum
  {
    ICON_TRICK_GENERIC_AIR  = -1,
    ICON_TRICK_GENERIC_FACE = -2,
    ICON_TRICK_GENERIC_TUBE = -3
  };

  enum
  {
    ICON_STATE_ACTIVE,
    ICON_STATE_QUEUED,
    ICON_STATE_GLOWING,
    ICON_STATE_FROZEN,
    ICON_STATE_NONE
  };

private:
  class IconResource
	{
	public:
		nglTexture      *texture;
		int 				    trickIdx;
    unsigned int    bitmask;
    unsigned char   timesDisplayed;
    unsigned char   iconType;
    bool            avail;

	public:
		IconResource();
		~IconResource();

		void Load(int type, int idx, int num, const stringx & texFilename);
	};


  struct IconSpawns
  {
    unsigned char   numIcons;
    unsigned char   air;
    unsigned char   face;
    unsigned char   tube;
    unsigned char   specialAir;
    unsigned char   specialFace;
    unsigned char   specialTube;
    unsigned char   genericAir;
    unsigned char   genericFace;
    unsigned char   genericTube;

    float           spawnTime;
    float           spawnMultiplier;

    unsigned int    trickListAir;
    unsigned char   trickListFace;
    unsigned short  trickListTube;
    unsigned short  trickListSpecialAir;
    unsigned char   trickListSpecialFace;
    unsigned char   trickListSpecialTube;

    float           velocity;
    float           accel;
    float           morphSpeed;

    unsigned char   bonusIcons;
    float           bonusAdder;
    float           bonusMultiplier;

    unsigned char   repeatWait[7];
    float           timerMult[6];
    float           holdTime[6];

    float           timerSpeedWipeout;
    float           timerSpeedLieOnBoard;

    unsigned char   buttonDisplay;
    float           buttonDelay;
    unsigned char   buttonStayBottom;

    unsigned char   maxSameIcon;
    unsigned char   maxSameIconGroup;
  };

public:
	class Icon
	{
	public:
		nglQuad			quad;
		IconResource*	resource;
    float y, vy;
    float sizeInterp;
    float colorInterp;
    //float fadeInterp;
		bool			show;
    unsigned char state;

	public:
		Icon();
		~Icon();

		void Initialize(IconResource & res);
		void Draw(void);
		void SetShow(const bool s = true);
	};

private:

  nglQuad      buttonQuad[5];
	nglTexture   *buttonTexture[ICON_NUM_BUTTONS];

  nglQuad      chainQuad[4];
  nglTexture   *chainTexture;
  unsigned char numChains;

  nglQuad      iceQuad[5];
  nglTexture   *iceTexture[6];
  bool         getIceTexture[6];
  unsigned char numFrozen;

  nglQuad      glowQuad[5];
  nglTexture   *glowTexture[3];
  unsigned char numGlowing;

  Icon         effectIcons[5];
  unsigned char numEffectIcons;
  unsigned char effectType;
  float         effectTime;

	IconResource*	airIconResources;
	IconResource*	faceIconResources;
	IconResource*	tubeIconResources;
	IconResource*	specialAirIconResources;
	IconResource*	specialFaceIconResources;
	IconResource*	specialTubeIconResources;
	IconResource*	genericIconResources;
	int				numAirIconResources;
	int				numFaceIconResources;
	int				numTubeIconResources;
	int				numSpecialAirIconResources;
	int				numSpecialFaceIconResources;
	int				numSpecialTubeIconResources;

	IconResource*	airTricklist;
	IconResource*	faceTricklist;
	IconResource*	tubeTricklist;
	IconResource*	specialAirTricklist;
	IconResource*	specialFaceTricklist;
	IconResource*	specialTubeTricklist;
	int				numAirTricks;
	int				numFaceTricks;
	int				numTubeTricks;
	int				numSpecialAirTricks;
	int				numSpecialFaceTricks;
	int				numSpecialTubeTricks;

	int				maxIcons;
	Icon*			icons;
	Icon 			nextIcon;
  bool      nextEnable;
  float     nextDelay;
  float     nextTime;

	int				numIcons;
  int       iconCounter;
  int       iconSpawnCounter;
  int       activeIcon;
  int       completedIcons;
  bool      chainedIcons;
  int       numButtons;
  bool      drawButtons;

  bool      enableMode;

  short     queueBottom;
  short     queueLeft;
  short     nextIconTop;
  short     activeIconWidth;
  short     activeIconHeight;
  short     smallIconWidth;
  short     smallIconHeight;
  short     buttonWidth;
  short     buttonHeight;

  float     initSpawnTime;
  float     spawnDelay;
	float			spawnTimer;
  float     lastDropTime;
  float     spawnTimerMultiplier;
  float     buttonTimer;
  float     maxTweenTime;

  IconResource *lastIcon;
  int           sameIconCount;
  int           lastIconGroup;
  int           sameIconGroupCount;

  unsigned char repeatCount[ICON_NUM_TYPES];

  bool      failure;
  int       prepareFailure;
  bool      enableNewIcons;
  float     startFail;

  bool      prepPop;
  float     popTime;
  int       trickStack[4];
  int       tsSize;

  bool      prepMidPop[5];
  float     midPopTime[5];

  IconSpawns    iconSpawns[MAX_ICON_SPAWNS];
  unsigned char numSettings;
  unsigned char numSpawns;
  unsigned char currentSetting;
  unsigned char currentSpawn;

	TextString *	timerText;

  int           scoreBonuses[5];
	RandomText *	scoreText;
	RandomText *	scoreTextEffect;
  bool showCounter;
  bool showTimer;
  bool enableTimer;

  float time;
  float timeLastQueueChange;

private:
	void OnQueueChange(void);
	void SetButtonText(void);
	IconResource* FindResource(const int trickIdx);
  bool GetNextTrick(int & trickIdx, int & trickType);
  bool GetTrick(char *tname, int &ttype, int &tidx);
  bool done_before;

public:
	// Creators.
	IGOIconManager();
	~IGOIconManager();

	// Modifiers.
	void Reset(void);
  void LoadDefaultSettings(void);
  bool LoadFile(void);
	void Update(const float dt);
	void Draw(void);
	void PushBack(const int trickIdx);
	void PopFront(void);
	void PopIcon(int icon);
	void PopCompletedIcons(bool success);
  void TrickChain(int trickIdx);
  void FinishChain(bool success);
  void setFont(Font *font)  {scoreText->setFont(font); scoreTextEffect->setFont(font); timerText->setFont(font);}
	
	// Accessors.
	bool IsQueueEmpty(void) { return numIcons == 0; }
	int GetFront(void);
	bool Failed() { return failure; }
	void EnableNewIcons(bool en=true) {enableNewIcons = en;}
	int IconsCleared() {return iconCounter;}
	bool ChainedIcons() {return chainedIcons;}
	bool FirstTimeGettingThisMany(int goal, bool ignore_previous = false);
};

#endif INCLUDED_IGOICONMANAGER_H
