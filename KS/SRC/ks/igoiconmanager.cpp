
#include "global.h"
#include "igoiconmanager.h"
#include "random.h"
#include "wds.h"
#include "SoundScript.h"
#include "SoundData.h"
#include "unlock_manager.h"
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	IGOIconManager class
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static IGOIconManager::Icon* fallingIcon = NULL;

//	IGOIconManager()
// Default constructor.
IGOIconManager::IGOIconManager()
{
	int	i;

	// Allocate icon resources.
	numAirIconResources           = 6;
	numFaceIconResources          = 3;
	numTubeIconResources          = 3;
	numSpecialAirIconResources    = 16;
	numSpecialFaceIconResources   = 7;
	numSpecialTubeIconResources   = 4;
	airIconResources              = NEW IconResource[numAirIconResources];
	faceIconResources             = NEW IconResource[numFaceIconResources];
	tubeIconResources             = NEW IconResource[numTubeIconResources];
	specialAirIconResources       = NEW IconResource[numSpecialAirIconResources];
	specialFaceIconResources      = NEW IconResource[numSpecialFaceIconResources];
	specialTubeIconResources      = NEW IconResource[numSpecialTubeIconResources];
	genericIconResources          = NEW IconResource[3];

	airTricklist = NEW IconResource[numAirIconResources];
	faceTricklist = NEW IconResource[numFaceIconResources];
	tubeTricklist = NEW IconResource[numTubeIconResources];
	specialAirTricklist = NEW IconResource[numSpecialAirIconResources];
	specialFaceTricklist = NEW IconResource[numSpecialFaceIconResources];
	specialTubeTricklist = NEW IconResource[numSpecialTubeIconResources];
  numAirTricks          = 0;
  numFaceTricks         = 0;
  numTubeTricks         = 0;
  numSpecialAirTricks   = 0;
  numSpecialFaceTricks  = 0;
  numSpecialTubeTricks  = 0;

	// Load icon resources.
	nglSetTexturePath("challenges\\icontetris\\textures\\");

  airIconResources[0].Load(ICON_TYPE_AIR, TRICK_JUDO_AIR,   0, "igo_icon_judo");
  airIconResources[1].Load(ICON_TYPE_AIR, TRICK_MELON_GRAB, 1, "igo_icon_mellon");
  airIconResources[2].Load(ICON_TYPE_AIR, TRICK_NOSE_GRAB,  2, "igo_icon_nosegrab");
  airIconResources[3].Load(ICON_TYPE_AIR, TRICK_SHOVEIT,    3, "igo_icon_shoveit");
  airIconResources[4].Load(ICON_TYPE_AIR, TRICK_STALEFISH,  4, "igo_icon_stalefish");
  airIconResources[5].Load(ICON_TYPE_AIR, TRICK_TAIL_GRAB,  5, "igo_icon_tailgrab");
  //airIconResources[6].Load(ICON_TYPE_AIR, TRICK_GRAB,       6, "igo_icon_grab");

  faceIconResources[0].Load(ICON_TYPE_FACE, TRICK_SNAP,           0, "igo_icon_snap");
  faceIconResources[1].Load(ICON_TYPE_FACE, TRICK_REVERT_CUTBACK, 1, "igo_icon_revert_cutback");
  faceIconResources[2].Load(ICON_TYPE_FACE, TRICK_REBOUND,        2, "igo_icon_rebound");
  //faceIconResources[2].Load(ICON_TYPE_FACE, TRICK_TAIL_CHUCK,     2, "igo_icon_tailchuck");
  //iconResources[3].Load(ICON_TYPE_FACE, TRICK_TAILSLIDE, 0, "igo_icon_tailslide");

  tubeIconResources[0].Load(ICON_TYPE_TUBE, TRICK_ONE_HAND_DRAG,      0, "igo_icon_one_handdrag");
  tubeIconResources[1].Load(ICON_TYPE_TUBE, TRICK_TWO_HAND_DRAG,      1, "igo_icon_two_handdrag");
  tubeIconResources[2].Load(ICON_TYPE_TUBE, TRICK_ONE_HAND_ROOF_DRAG, 2, "igo_icon_roofdrag");

  specialAirIconResources[0].Load(ICON_TYPE_SPECIAL_AIR, TRICK_WALK,            0, "igo_icon_airwalk");
  specialAirIconResources[1].Load(ICON_TYPE_SPECIAL_AIR, TRICK_ALLEY_OOP,       1, "igo_icon_alleyoop");
  specialAirIconResources[2].Load(ICON_TYPE_SPECIAL_AIR, TRICK_HELICOPTER_720,  2, "igo_icon_heli");
  specialAirIconResources[3].Load(ICON_TYPE_SPECIAL_AIR, TRICK_MONKEYMAN,       3, "igo_icon_monkeyman");
  specialAirIconResources[4].Load(ICON_TYPE_SPECIAL_AIR, TRICK_NACNAC,          4, "igo_icon_nacnac");
  specialAirIconResources[5].Load(ICON_TYPE_SPECIAL_AIR, TRICK_SUPERMAN,        5, "igo_icon_superman");
  specialAirIconResources[6].Load(ICON_TYPE_SPECIAL_AIR, TRICK_BACK_FLIP,       6, "igo_icon_backflip");
  specialAirIconResources[7].Load(ICON_TYPE_SPECIAL_AIR, TRICK_JC_AIR,          7, "igo_icon_changeman");
  specialAirIconResources[8].Load(ICON_TYPE_SPECIAL_AIR, TRICK_FRONT_FLIP,      8, "igo_icon_frontflip");
  specialAirIconResources[9].Load(ICON_TYPE_SPECIAL_AIR, TRICK_CROSS_AIR,       9, "igo_icon_hangman");
  specialAirIconResources[10].Load(ICON_TYPE_SPECIAL_AIR, TRICK_INDIAN,         10, "igo_icon_indian");
  specialAirIconResources[11].Load(ICON_TYPE_SPECIAL_AIR, TRICK_LEFT_FLIP,      11, "igo_icon_leftflip");
  specialAirIconResources[12].Load(ICON_TYPE_SPECIAL_AIR, TRICK_RIGHT_FLIP,     12, "igo_icon_rightflip");
  specialAirIconResources[13].Load(ICON_TYPE_SPECIAL_AIR, TRICK_RODEO,          13, "igo_icon_rodeo");
  specialAirIconResources[14].Load(ICON_TYPE_SPECIAL_AIR, TRICK_SUN_CHILD,      14, "igo_icon_sunchild");
  specialAirIconResources[15].Load(ICON_TYPE_SPECIAL_AIR, TRICK_TWEAKER,        15, "igo_icon_tweaker");

  specialFaceIconResources[0].Load(ICON_TYPE_SPECIAL_FACE, TRICK_DARKSLIDE,     0, "igo_icon_darkslide");
  specialFaceIconResources[1].Load(ICON_TYPE_SPECIAL_FACE, TRICK_CRUZER,        1, "igo_icon_cruzer");
  specialFaceIconResources[2].Load(ICON_TYPE_SPECIAL_FACE, TRICK_CHEATERS5,     2, "igo_icon_cheaters5");
  specialFaceIconResources[3].Load(ICON_TYPE_SPECIAL_FACE, TRICK_HANGTEN,       3, "igo_icon_hangten");
  specialFaceIconResources[4].Load(ICON_TYPE_SPECIAL_FACE, TRICK_HEADSTAND,     4, "igo_icon_180_headstand");
  specialFaceIconResources[5].Load(ICON_TYPE_SPECIAL_FACE, TRICK_FACE_SHOVEIT,  5, "igo_icon_shoveit_face");
  specialFaceIconResources[6].Load(ICON_TYPE_SPECIAL_FACE, TRICK_MANUAL,        6, "igo_icon_manual");

  specialTubeIconResources[0].Load(ICON_TYPE_SPECIAL_TUBE, TRICK_COFFIN,      0, "igo_icon_coffin");
  specialTubeIconResources[1].Load(ICON_TYPE_SPECIAL_TUBE, TRICK_LAWNDART,    1, "igo_icon_lawndart");
  specialTubeIconResources[2].Load(ICON_TYPE_SPECIAL_TUBE, TRICK_CAVEMAN,     2, "igo_icon_caveman");
  specialTubeIconResources[3].Load(ICON_TYPE_SPECIAL_TUBE, TRICK_SPIT_EJECT,  3, "igo_icon_tubespit");

  genericIconResources[0].Load(ICON_TYPE_AIR,  ICON_TRICK_GENERIC_AIR,      0, "icon_generic_air");
  genericIconResources[1].Load(ICON_TYPE_FACE, ICON_TRICK_GENERIC_FACE,     1, "icon_generic_face");
  genericIconResources[2].Load(ICON_TYPE_TUBE, ICON_TRICK_GENERIC_TUBE,     2, "icon_generic_tube");

  // Load button textures, which are stored in font directory now
	nglSetTexturePath("interface\\font\\textures\\");
  buttonTexture[ICON_BUTTON_ARROW]     = nglLoadTexture("igo_icon_arrow");
  buttonTexture[ICON_BUTTON_CROSS]     = nglLoadTexture("igo_icon_x");
  buttonTexture[ICON_BUTTON_CIRCLE]    = nglLoadTexture("igo_icon_circle");
  buttonTexture[ICON_BUTTON_TRIANGLE]  = nglLoadTexture("igo_icon_triangle");
  buttonTexture[ICON_BUTTON_SQUARE]    = nglLoadTexture("igo_icon_square");

  // But these are not
	nglSetTexturePath("challenges\\icontetris\\textures\\");
  buttonTexture[ICON_BUTTON_PLUS]      = nglLoadTexture("igo_icon_plus");
  buttonTexture[ICON_BUTTON_COMMA]     = nglLoadTexture("igo_icon_comma");

  for(i=0; i<5; i++)
  {
	  nglInitQuad(&buttonQuad[i]);
	  nglSetQuadColor(&buttonQuad[i], NGL_RGBA32(255, 255, 255, 255));
	  nglSetQuadZ(&buttonQuad[i], 800.0f);
  }

  chainTexture = nglLoadTexture("igo_icon_chain");
  for(i=0; i<4; i++)
  {
	  nglInitQuad(&chainQuad[i]);
	  nglSetQuadColor(&chainQuad[i], NGL_RGBA32(255, 255, 255, 255));
	  nglSetQuadZ(&chainQuad[i], 800.0f);
	  nglSetQuadTex(&chainQuad[i], chainTexture);
  }

  glowTexture[0] = nglLoadTexture("icon_glow_triangle");
  glowTexture[1] = nglLoadTexture("icon_glow_square");
  glowTexture[2] = nglLoadTexture("icon_glow_oval");
  for(i=0; i<5; i++)
  {
	  nglInitQuad(&glowQuad[i]);
	  nglSetQuadColor(&glowQuad[i], NGL_RGBA32(255, 255, 255, 255));
	  nglSetQuadZ(&glowQuad[i], 800.0f);
    nglSetQuadBlend(&glowQuad[i], NGLBM_ADDITIVE);
  }

#ifdef USE_ICE
  iceTexture[0] = nglLoadTexture("icon_ice_triangle");
  iceTexture[1] = nglLoadTexture("icon_ice_square");
  iceTexture[2] = nglLoadTexture("icon_ice_oval");
#else
  iceTexture[0] = nglLoadTexture("icon_miss_01");
  iceTexture[1] = nglLoadTexture("icon_miss_02");
  iceTexture[2] = nglLoadTexture("icon_miss_03");
  iceTexture[3] = nglLoadTexture("icon_miss_04");
  iceTexture[4] = nglLoadTexture("icon_miss_05");
  iceTexture[5] = nglLoadTexture("icon_miss_06");
#endif

  for(i=0; i<5; i++)
  {
	  nglInitQuad(&iceQuad[i]);
	  nglSetQuadColor(&iceQuad[i], NGL_RGBA32(255, 255, 255, 255));
	  nglSetQuadZ(&iceQuad[i], 800.0f);
  }

	// Allocate icons.
	numIcons = 0;
	maxIcons = 5;
	icons = NEW Icon[maxIcons];

	// Initialize icons.
	for (i = 0; i < maxIcons; i++)
		icons[i].SetShow(true);

  scoreText       = NEW RandomText(NULL, "", 0, 0, 800, 1.0f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, color32(224, 224, 128, 255));
  scoreTextEffect = NEW RandomText(NULL, "", 0, 0, 800, 1.0f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, color32(224, 224, 128, 255));
  scoreText->NoFade(true);
  timerText = NULL;

  LoadDefaultSettings();
  LoadFile();

  Reset();
}

//	~IGOIconManager()
// Destructor.
IGOIconManager::~IGOIconManager()
{
	delete [] airIconResources;
	delete [] faceIconResources;
	delete [] tubeIconResources;
	delete [] specialAirIconResources;
	delete [] specialFaceIconResources;
	delete [] specialTubeIconResources;
	delete [] airTricklist;
	delete [] faceTricklist;
	delete [] tubeTricklist;
	delete [] specialAirTricklist;
	delete [] specialFaceTricklist;
	delete [] specialTubeTricklist;
  delete [] genericIconResources;
	delete [] icons;
  delete scoreText;
  delete scoreTextEffect;
  delete timerText;
}

//	Reset()
// Clears the icons off the screen and resets timers.  Should be called when a new run begins.
void IGOIconManager::Reset(void)
{
  int i;

  numIcons = 0;

  currentSetting = 0;
  currentSpawn   = 0xFF;

  spawnDelay = initSpawnTime;
	spawnTimer = spawnDelay;
  lastDropTime = 0.0f;

  for(i=0; i<ICON_NUM_TYPES; i++)
    repeatCount[i] = 0;

  for(i=0; i<numAirIconResources; i++)
    airIconResources[i].timesDisplayed = 0;
  for(i=0; i<numFaceIconResources; i++)
    faceIconResources[i].timesDisplayed = 0;
  for(i=0; i<numTubeIconResources; i++)
    tubeIconResources[i].timesDisplayed = 0;
  for(i=0; i<numSpecialAirIconResources; i++)
    specialAirIconResources[i].timesDisplayed = 0;
  for(i=0; i<numSpecialFaceIconResources; i++)
    specialFaceIconResources[i].timesDisplayed = 0;
  for(i=0; i<numSpecialTubeIconResources; i++)
    specialTubeIconResources[i].timesDisplayed = 0;

  for(i=0; i<6; i++)
    getIceTexture[i] = true;

  nextIcon.show = false;
  nextIcon.resource = NULL;
  activeIcon = 0;
  completedIcons = 0;
  numEffectIcons = 0;
  effectType = 0;
  effectTime = 0.0f;
  chainedIcons = false;
  iconCounter = 0;
  iconSpawnCounter = 0;
  buttonTimer = 0.0f;
  prepareFailure = false;
  failure = false;
  enableNewIcons = true;
  drawButtons = false;
  showTimer = false;
  nextEnable = true;

  done_before = false;

  numChains = 0;
  numFrozen = 0;
  numGlowing = 0;

  time = 0.0f;
  timeLastQueueChange = 0.0f;

  lastIcon            = NULL;
  sameIconCount       = 0;
  lastIconGroup       = -1;
  sameIconGroupCount  = 0;

  prepPop = false;
  popTime = 0.0f;

  for(i=0; i<5; i++)
  {
    prepMidPop[i] = false;
    midPopTime[i] = 0.0f;
  }

  tsSize = 0;
}

//	LoadDefaualtSettings()
// Loads default settings (so game won't crash if a .icon file isn't found)
void IGOIconManager::LoadDefaultSettings(void)
{
  currentSetting    = 0;
  currentSpawn      = 0xFF;

  queueBottom       = 440;
  queueLeft         = 40;
  nextIconTop       = 40;
  activeIconWidth   = 96;
  activeIconHeight  = 96;
  smallIconWidth    = 66;
  smallIconHeight   = 66;
  buttonWidth       = 22;
  buttonHeight      = 22;

  counterX          = 250;
  counterY          = 58;
  counterWidth      = 64;
  counterHeight     = 64;

  nextEnable        = true;
  nextDelay         = 1.0f;
  nextTime          = 99.0f;

  done_before		= false;

  enableTimer       = true;
  showCounter       = true;
  showTimer         = false;

  initSpawnTime     = 2.0f;
  maxTweenTime      = 0.0f;

  scoreBonuses[0]   = 0;
  scoreBonuses[1]   = 1000;
  scoreBonuses[2]   = 2000;
  scoreBonuses[3]   = 5000;
  scoreBonuses[4]   = 25000;

  iconSpawns[0].numIcons    = 100;
  iconSpawns[0].air         = 40;
  iconSpawns[0].face        = 30;
  iconSpawns[0].tube        = 0;
  iconSpawns[0].specialAir  = 25;
  iconSpawns[0].specialFace = 5;
  iconSpawns[0].specialTube = 0;

  iconSpawns[0].spawnTime           = 7.0f;
  iconSpawns[0].spawnMultiplier     = 0.97f;

  iconSpawns[0].trickListAir          = 0xFFFFFFFF;
  iconSpawns[0].trickListFace         = 0xFF;
  iconSpawns[0].trickListTube         = 0xFFFF;
  iconSpawns[0].trickListSpecialAir   = 0xFFFF;
  iconSpawns[0].trickListSpecialFace  = 0xFF;
  iconSpawns[0].trickListSpecialTube  = 0xFF;

  iconSpawns[0].velocity            = 0.01f;
  iconSpawns[0].accel               = 600.0f;
  iconSpawns[0].morphSpeed          = 5.0f;

  iconSpawns[0].bonusIcons          = 5;
  iconSpawns[0].bonusAdder          = 5.0f;
  iconSpawns[0].bonusMultiplier     = 1.0f;

  iconSpawns[0].repeatWait[ICON_TYPE_AIR]           = 0;
  iconSpawns[0].repeatWait[ICON_TYPE_FACE]          = 0;
  iconSpawns[0].repeatWait[ICON_TYPE_TUBE]          = 0;
  iconSpawns[0].repeatWait[ICON_TYPE_SPECIAL_AIR]   = 0;
  iconSpawns[0].repeatWait[ICON_TYPE_SPECIAL_FACE]  = 0;
  iconSpawns[0].repeatWait[ICON_TYPE_SPECIAL_TUBE]  = 0;
  iconSpawns[0].repeatWait[ICON_TYPE_SPECIAL_ANY]   = 2;

  iconSpawns[0].timerMult[ICON_TYPE_AIR]            = 1.0f;
  iconSpawns[0].timerMult[ICON_TYPE_FACE]           = 1.0f;
  iconSpawns[0].timerMult[ICON_TYPE_TUBE]           = 1.7f;
  iconSpawns[0].timerMult[ICON_TYPE_SPECIAL_AIR]    = 1.7f;
  iconSpawns[0].timerMult[ICON_TYPE_SPECIAL_FACE]   = 1.7f;
  iconSpawns[0].timerMult[ICON_TYPE_SPECIAL_TUBE]   = 2.0f;

  iconSpawns[0].timerSpeedWipeout     = 0.5f;
  iconSpawns[0].timerSpeedLieOnBoard  = 0.5f;

  iconSpawns[0].buttonDisplay         = 2;
  iconSpawns[0].buttonDelay           = 0.0f;
  iconSpawns[0].buttonStayBottom      = false;

  iconSpawns[0].maxSameIcon           = 1;
  iconSpawns[0].maxSameIconGroup      = 2;

	if(timerText)
    delete timerText;
  timerText   = NEW TextString(NULL, "", queueLeft+smallIconWidth/2, nextIconTop+smallIconHeight+8, 800, 0.8f, Font::HORIZJUST_CENTER, Font::VERTJUST_TOP, color32(255, 255, 255, 255));

  //short d1;
  //float d2;
  //adjustCoords(queueLeft, queueBottom);
  //adjustCoords(d1, nextIconTop);
  //adjustSizes(activeIconWidth, activeIconHeight);
  //adjustSizes(smallIconWidth, smallIconHeight);
  //adjustSizes(d2, iconSpawns[0].velocity);
  //adjustSizes(d2, iconSpawns[0].accel);
  //adjustCoords(counterX, counterY);
  //adjustSizes(counterWidth, counterHeight);
}

bool IGOIconManager::GetTrick(char *tname, int &ttype, int &tidx)
{
  if(0 == stricmp(tname, "JudoAir"))    {ttype = ICON_TYPE_AIR; tidx = TRICK_JUDO_AIR;    return true;}
  if(0 == stricmp(tname, "MelonGrab"))  {ttype = ICON_TYPE_AIR; tidx = TRICK_MELON_GRAB;  return true;}
  if(0 == stricmp(tname, "NoseGrab"))   {ttype = ICON_TYPE_AIR; tidx = TRICK_NOSE_GRAB;   return true;}
  if(0 == stricmp(tname, "ShoveIt"))    {ttype = ICON_TYPE_AIR; tidx = TRICK_SHOVEIT;     return true;}
  if(0 == stricmp(tname, "Stalefish"))  {ttype = ICON_TYPE_AIR; tidx = TRICK_STALEFISH;   return true;}
  if(0 == stricmp(tname, "TailGrab"))   {ttype = ICON_TYPE_AIR; tidx = TRICK_TAIL_GRAB;   return true;}
  if(0 == stricmp(tname, "Grab"))       {ttype = ICON_TYPE_AIR; tidx = TRICK_GRAB;        return true;}

  if(0 == stricmp(tname, "Gouge"))          {ttype = ICON_TYPE_FACE; tidx = TRICK_GOUGE;          return true;}
  if(0 == stricmp(tname, "LaybackSlide"))   {ttype = ICON_TYPE_FACE; tidx = TRICK_LAYBACK_SLIDE;  return true;}
  if(0 == stricmp(tname, "TailChuck"))      {ttype = ICON_TYPE_FACE; tidx = TRICK_TAIL_CHUCK;     return true;}
  if(0 == stricmp(tname, "Tailslide"))      {ttype = ICON_TYPE_FACE; tidx = TRICK_TAILSLIDE;      return true;}
  if(0 == stricmp(tname, "Snap"))           {ttype = ICON_TYPE_FACE; tidx = TRICK_SNAP;           return true;}

  if(0 == stricmp(tname, "OneHandDrag"))      {ttype = ICON_TYPE_TUBE; tidx = TRICK_ONE_HAND_DRAG;      return true;}
  if(0 == stricmp(tname, "TwoHandDrag"))      {ttype = ICON_TYPE_TUBE; tidx = TRICK_TWO_HAND_DRAG;      return true;}
  if(0 == stricmp(tname, "OneHandRoofDrag"))  {ttype = ICON_TYPE_TUBE; tidx = TRICK_ONE_HAND_ROOF_DRAG; return true;}

  if(0 == stricmp(tname, "Airwalk"))    {ttype = ICON_TYPE_SPECIAL_AIR; tidx = TRICK_WALK;            return true;}
  if(0 == stricmp(tname, "AlleyOop"))   {ttype = ICON_TYPE_SPECIAL_AIR; tidx = TRICK_ALLEY_OOP;       return true;}
  if(0 == stricmp(tname, "Heli"))       {ttype = ICON_TYPE_SPECIAL_AIR; tidx = TRICK_HELICOPTER_720;  return true;}
  if(0 == stricmp(tname, "MonkeyMan"))  {ttype = ICON_TYPE_SPECIAL_AIR; tidx = TRICK_MONKEYMAN;       return true;}
  if(0 == stricmp(tname, "NacNac"))     {ttype = ICON_TYPE_SPECIAL_AIR; tidx = TRICK_NACNAC;          return true;}
  if(0 == stricmp(tname, "Superman"))   {ttype = ICON_TYPE_SPECIAL_AIR; tidx = TRICK_SUPERMAN;        return true;}
  if(0 == stricmp(tname, "BackFlip"))   {ttype = ICON_TYPE_SPECIAL_AIR; tidx = TRICK_BACK_FLIP;       return true;}
  if(0 == stricmp(tname, "Changeman"))  {ttype = ICON_TYPE_SPECIAL_AIR; tidx = TRICK_JC_AIR;        return true;}
  if(0 == stricmp(tname, "FrontFlip"))  {ttype = ICON_TYPE_SPECIAL_AIR; tidx = TRICK_FRONT_FLIP;      return true;}
  if(0 == stricmp(tname, "Hangman"))    {ttype = ICON_TYPE_SPECIAL_AIR; tidx = TRICK_CROSS_AIR;         return true;}
  if(0 == stricmp(tname, "Indian"))     {ttype = ICON_TYPE_SPECIAL_AIR; tidx = TRICK_INDIAN;          return true;}
  if(0 == stricmp(tname, "LeftFlip"))   {ttype = ICON_TYPE_SPECIAL_AIR; tidx = TRICK_LEFT_FLIP;       return true;}
  if(0 == stricmp(tname, "RightFlip"))  {ttype = ICON_TYPE_SPECIAL_AIR; tidx = TRICK_RIGHT_FLIP;      return true;}
  if(0 == stricmp(tname, "Rodeo"))      {ttype = ICON_TYPE_SPECIAL_AIR; tidx = TRICK_RODEO;           return true;}
  if(0 == stricmp(tname, "SunChild"))   {ttype = ICON_TYPE_SPECIAL_AIR; tidx = TRICK_SUN_CHILD;       return true;}
  if(0 == stricmp(tname, "Tweaker"))    {ttype = ICON_TYPE_SPECIAL_AIR; tidx = TRICK_TWEAKER;         return true;}

  if(0 == stricmp(tname, "Darkslide"))  {ttype = ICON_TYPE_SPECIAL_FACE; tidx = TRICK_DARKSLIDE;      return true;}

  if(0 == stricmp(tname, "Coffin"))     {ttype = ICON_TYPE_SPECIAL_TUBE; tidx = TRICK_COFFIN;         return true;}
  if(0 == stricmp(tname, "Lawndart"))   {ttype = ICON_TYPE_SPECIAL_TUBE; tidx = TRICK_LAWNDART;       return true;}
  if(0 == stricmp(tname, "Caveman"))    {ttype = ICON_TYPE_SPECIAL_TUBE; tidx = TRICK_CAVEMAN;        return true;}

  return false;
}

//	LoadFile()
// Loads icon settings from file.
bool IGOIconManager::LoadFile(void)
{
  nglFileBuf fBuf;
  char *buf;
  char *line;
  char *token;

  numSettings     = 1;
  numSpawns       = 0;
  currentSetting  = 0;
  currentSpawn    = 0;

  stringx filename = stringx("levels\\") + g_game_ptr->get_beach_name() + stringx("\\") + g_game_ptr->get_beach_name() + stringx(".icon");

  if (!KSReadFile(filename.c_str(),  &fBuf, 1))
    return false;

  buf = (char *)fBuf.Buf;
  while(*buf != '\0')
  {
    // Get next line
    line = buf;
    while(*buf != '\n' && *buf != '\r' && *buf != '\0')
      buf++;
    if(*buf != '\0')
    {
      *buf = '\0';
      buf++;
    }
    while(*buf == '\n' || *buf == '\r')
      buf++;

    // Skip line if it begins with ;
    if(line[0] == ';')
      continue;

    // Parse line
    token = strtok(line, " \t");
    if(token == NULL)
      continue;
    if(0 == stricmp(token, "QueueLeftBottom"))
    {
      if((token = strtok(NULL, " \t")) == 0)  continue;
      queueLeft = atoi(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      queueBottom = atoi(token);
      //adjustCoords(queueLeft, queueBottom);
    }
    else if(0 == stricmp(token, "ActiveIconSize"))
    {
      if((token = strtok(NULL, " \t")) == 0)  continue;
      activeIconWidth = atoi(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      activeIconHeight = atoi(token);
      //adjustSizes(activeIconWidth, activeIconHeight);
    }
    else if(0 == stricmp(token, "SmallIconSize"))
    {
      if((token = strtok(NULL, " \t")) == 0)  continue;
      smallIconWidth = atoi(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      smallIconHeight = atoi(token);
      //adjustSizes(smallIconWidth, smallIconHeight);
    }
    else if(0 == stricmp(token, "ButtonSize"))
    {
      if((token = strtok(NULL, " \t")) == 0)  continue;
      buttonWidth = atoi(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      buttonHeight = atoi(token);
      //adjustSizes(buttonWidth, buttonHeight);     // Don't adjust button sizes yet, since they can be rotated...do it in OnQueueChange
    }
    else if(0 == stricmp(token, "CounterPos"))
    {
      if((token = strtok(NULL, " \t")) == 0)  continue;
      counterX = atoi(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      counterY = atoi(token);
    }
    else if(0 == stricmp(token, "CounterSize"))
    {
      if((token = strtok(NULL, " \t")) == 0)  continue;
      counterWidth = atoi(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      counterHeight = atoi(token);
    }
    else if(0 == stricmp(token, "DisplayNextIcon"))
    {
      if((token = strtok(NULL, " \t")) == 0)  continue;
      nextEnable = atoi(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      nextDelay = atof(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      nextTime = atof(token);
    }
    else if(0 == stricmp(token, "DisplayCounter"))
    {
      if((token = strtok(NULL, " \t")) == 0)  continue;
      showCounter = atoi(token);
    }
    else if(0 == stricmp(token, "DisplayTimer"))
    {
      if((token = strtok(NULL, " \t")) == 0)  continue;
      enableTimer = atoi(token);
    }
    else if(0 == stricmp(token, "InitSpawnTime"))
    {
      if((token = strtok(NULL, " \t")) == 0)  continue;
      initSpawnTime = atof(token);
    }
    else if(0 == stricmp(token, "MaxNoIconTime"))
    {
      if((token = strtok(NULL, " \t")) == 0)  continue;
      maxTweenTime = atof(token);
    }
    else if(0 == stricmp(token, "AnimSpeed"))
    {
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].velocity = atof(token);
      if(iconSpawns[currentSpawn].velocity == 0.0f)
        iconSpawns[currentSpawn].velocity = 0.01f;
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].accel = atof(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].morphSpeed = atof(token);
      //adjustSizes(dummy, iconSpawns[currentSpawn].velocity);
      //adjustSizes(dummy, iconSpawns[currentSpawn].accel);
    }
    else if(0 == stricmp(token, "Bonus"))
    {
      //if((token = strtok(NULL, " \t")) == 0)  continue;
      //iconSpawns[currentSpawn].bonusIcons = atoi(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      scoreBonuses[0] = atoi(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      scoreBonuses[1] = atoi(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      scoreBonuses[2] = atoi(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      scoreBonuses[3] = atoi(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      scoreBonuses[4] = atoi(token);
    }
    else if(0 == stricmp(token, "RepeatWait"))
    {
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].repeatWait[0] = atoi(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].repeatWait[1] = atoi(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].repeatWait[2] = atoi(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].repeatWait[3] = atoi(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].repeatWait[4] = atoi(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].repeatWait[5] = atoi(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].repeatWait[6] = atoi(token);
    }
    else if(0 == stricmp(token, "SpecialRepeatWait"))
    {
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].repeatWait[6] = atoi(token);
    }
    else if(0 == stricmp(token, "TimerMult"))
    {
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].timerMult[0] = atof(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].timerMult[1] = atof(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].timerMult[2] = atof(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].timerMult[3] = atof(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].timerMult[4] = atof(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].timerMult[5] = atof(token);
    }
    else if(0 == stricmp(token, "HoldTime"))
    {
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].holdTime[0] = atof(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].holdTime[1] = atof(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].holdTime[2] = atof(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].holdTime[3] = atof(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].holdTime[4] = atof(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].holdTime[5] = atof(token);
    }
    else if(0 == stricmp(token, "TimerSpeed"))
    {
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].timerSpeedWipeout = atof(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].timerSpeedLieOnBoard = atof(token);
    }
    else if(0 == stricmp(token, "WipeoutTimerSpeed"))
    {
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].timerSpeedWipeout    = atof(token);
      iconSpawns[currentSpawn].timerSpeedLieOnBoard = atof(token);
    }
    else if(0 == stricmp(token, "ButtonCombo"))
    {
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].buttonDisplay = atoi(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].buttonDelay = atof(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].buttonStayBottom = atoi(token);
    }
    else if(0 == stricmp(token, "MaxSame"))
    {
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].maxSameIcon = atoi(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].maxSameIconGroup = atoi(token);
    }
    else if(0 == stricmp(token, "spawn"))
    {
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].numIcons = atoi(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].air = atoi(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].face = atoi(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].tube = atoi(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].specialAir = atoi(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].specialFace = atoi(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].specialTube = atoi(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].genericAir = atoi(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].genericFace = atoi(token);
      if((token = strtok(NULL, " \t")) == 0)  continue;
      iconSpawns[currentSpawn].genericTube = atoi(token);

      if((token = strtok(NULL, " \t")) == 0)  continue;
      if(token[0] == '-')
        iconSpawns[currentSpawn].spawnTime = 0.0f;
      else
        iconSpawns[currentSpawn].spawnTime = atoi(token);

      if((token = strtok(NULL, " \t")) == 0)  continue;
      if(token[0] == '-')
        iconSpawns[currentSpawn].spawnMultiplier = 0.0f;
      else
        iconSpawns[currentSpawn].spawnMultiplier = atof(token);

      iconSpawns[currentSpawn].trickListAir         = 0;
      iconSpawns[currentSpawn].trickListFace        = 0;
      iconSpawns[currentSpawn].trickListTube        = 0;
      iconSpawns[currentSpawn].trickListSpecialAir  = 0;
      iconSpawns[currentSpawn].trickListSpecialFace = 0;
      iconSpawns[currentSpawn].trickListSpecialTube = 0;

      while((token = strtok(NULL, " \t")) != 0)
      {
        if(0 == stricmp(token, "+All"))
        {
          iconSpawns[currentSpawn].trickListAir         = 0xFFFFFFFF;
          iconSpawns[currentSpawn].trickListFace        = 0xFF;
          iconSpawns[currentSpawn].trickListTube        = 0xFFFF;
          iconSpawns[currentSpawn].trickListSpecialAir  = 0xFFFF;
          iconSpawns[currentSpawn].trickListSpecialFace = 0xFF;
          iconSpawns[currentSpawn].trickListSpecialTube = 0xFF;
        }
        else if(0 == stricmp(token, "-All"))
        {
          iconSpawns[currentSpawn].trickListAir         = 0;
          iconSpawns[currentSpawn].trickListFace        = 0;
          iconSpawns[currentSpawn].trickListTube        = 0;
          iconSpawns[currentSpawn].trickListSpecialAir  = 0;
          iconSpawns[currentSpawn].trickListSpecialFace = 0;
          iconSpawns[currentSpawn].trickListSpecialTube = 0;
        }
        else if(0 == stricmp(token, "+Air"))
          iconSpawns[currentSpawn].trickListAir           = 0xFFFFFFFF;
        else if(0 == stricmp(token, "-Air"))
          iconSpawns[currentSpawn].trickListAir           = 0;
        else if(0 == stricmp(token, "+Face"))
          iconSpawns[currentSpawn].trickListFace          = 0xFF;
        else if(0 == stricmp(token, "-Face"))
          iconSpawns[currentSpawn].trickListFace          = 0;
        else if(0 == stricmp(token, "+Tube"))
          iconSpawns[currentSpawn].trickListTube          = 0xFFFF;
        else if(0 == stricmp(token, "-Tube"))
          iconSpawns[currentSpawn].trickListTube          = 0;
        else if(0 == stricmp(token, "+SpecialAir"))
          iconSpawns[currentSpawn].trickListSpecialAir    = 0xFFFF;
        else if(0 == stricmp(token, "-SpecialAir"))
          iconSpawns[currentSpawn].trickListSpecialAir    = 0;
        else if(0 == stricmp(token, "+SpecialFace"))
          iconSpawns[currentSpawn].trickListSpecialFace   = 0xFF;
        else if(0 == stricmp(token, "-SpecialFace"))
          iconSpawns[currentSpawn].trickListSpecialFace   = 0;
        else if(0 == stricmp(token, "+SpecialTube"))
          iconSpawns[currentSpawn].trickListSpecialTube   = 0xFF;
        else if(0 == stricmp(token, "-SpecialTube"))
          iconSpawns[currentSpawn].trickListSpecialTube   = 0;
        else
        {
          int ttype, tidx;
          if(GetTrick(&token[1], ttype, tidx))
          {
            IconResource *tres = FindResource(tidx);

            if(tres)
            {
              switch(ttype)
              {
                case ICON_TYPE_AIR:
                  if(token[0] == '+')
                    iconSpawns[currentSpawn].trickListAir |= tres->bitmask;
                  else
                    iconSpawns[currentSpawn].trickListAir &= ~tres->bitmask;
                  break;
                case ICON_TYPE_FACE:
                  if(token[0] == '+')
                    iconSpawns[currentSpawn].trickListFace |= (unsigned char)tres->bitmask;
                  else
                    iconSpawns[currentSpawn].trickListFace &= (unsigned char)~tres->bitmask;
                  break;
                case ICON_TYPE_TUBE:
                  if(token[0] == '+')
                    iconSpawns[currentSpawn].trickListTube |= (unsigned short)tres->bitmask;
                  else
                    iconSpawns[currentSpawn].trickListTube &= (unsigned short)~tres->bitmask;
                  break;
                case ICON_TYPE_SPECIAL_AIR:
                  if(token[0] == '+')
                    iconSpawns[currentSpawn].trickListSpecialAir |= (unsigned short)tres->bitmask;
                  else
                    iconSpawns[currentSpawn].trickListSpecialAir &= (unsigned short)~tres->bitmask;
                  break;
                case ICON_TYPE_SPECIAL_FACE:
                  if(token[0] == '+')
                    iconSpawns[currentSpawn].trickListSpecialFace |= (unsigned char)tres->bitmask;
                  else
                    iconSpawns[currentSpawn].trickListSpecialFace &= (unsigned char)~tres->bitmask;
                  break;
                case ICON_TYPE_SPECIAL_TUBE:
                  if(token[0] == '+')
                    iconSpawns[currentSpawn].trickListSpecialTube |= (unsigned char)tres->bitmask;
                  else
                    iconSpawns[currentSpawn].trickListSpecialTube &= (unsigned char)~tres->bitmask;
                  break;
              }
            }
          }
        }
      }

      currentSpawn++;
      numSpawns++;

      if(currentSpawn != MAX_ICON_SPAWNS)
        iconSpawns[currentSpawn] = iconSpawns[currentSpawn-1];
    }
  }

  KSReleaseFile(&fBuf);

  short d1;
  float d2;
  adjustCoords(queueLeft, queueBottom);
  adjustCoords(d1, nextIconTop);
  adjustSizes(activeIconWidth, activeIconHeight);
  adjustSizes(smallIconWidth, smallIconHeight);
  adjustSizes(d2, iconSpawns[0].velocity);
  adjustSizes(d2, iconSpawns[0].accel);
  adjustCoords(counterX, counterY);
  adjustSizes(counterWidth, counterHeight);

  if(timerText)
    delete timerText;
  timerText   = NEW TextString(NULL, "", queueLeft+smallIconWidth/2, nextIconTop+smallIconHeight+8, 800, 0.8f, Font::HORIZJUST_CENTER, Font::VERTJUST_TOP, color32(255, 255, 255, 255));
  
  return numSettings > 0 && numSpawns > 0;
}

//	Draw()
// Draws all the icons onscreen.
void IGOIconManager::Draw(void)
{
  int i;

  timeLastQueueChange = time;

  if(!failure)
  {
    // Draw glowing textures
    for(i=0; i<numGlowing; i++)
      nglListAddQuad(&glowQuad[i]);

    // Draw Icons
	  for (i = 0; i < numIcons; i++)
		  icons[i].Draw();
    if(nextEnable && nextIcon.show)
      nextIcon.Draw();

    // Draw Effect Icons
	  for (i = 0; i < numEffectIcons; i++)
		  effectIcons[i].Draw();
    //for(i=0; i<5; i++)
    //  if(prepMidPop[i])
    //    effectIcons[i].Draw();

    // Draw frozen textures
    for(i=0; i<numFrozen; i++)
      nglListAddQuad(&iceQuad[i]);


    // Draw chain links
    for(i=0; i<numChains; i++)
      nglListAddQuad(&chainQuad[i]);

    // Draw Buttons
    if(activeIcon < numIcons && icons[activeIcon].resource->timesDisplayed < iconSpawns[currentSpawn].buttonDisplay &&
       buttonTimer >= iconSpawns[currentSpawn].buttonDelay)
      drawButtons = true;
    else
      drawButtons = false;

    if(drawButtons)
    {
	    for (i = 0; i<numButtons; i++)
		    nglListAddQuad(&buttonQuad[i]);
    }

    // Draw Spawn Countdown
    char text2[16];
    sprintf(text2, "%0.1f", spawnTimer);
	  timerText->changeText(text2);
    if(nextEnable && showTimer)
      timerText->Draw();
  }

  // Draw Score Bonus text
  if(numEffectIcons > 0 && scoreBonuses[numEffectIcons-1] != 0 && !prepareFailure && !failure )
  {
	  scoreText->Draw();
    if(prepPop)
      scoreTextEffect->Draw();
  }
}

//	Update()
// Call often with time delta.
void IGOIconManager::Update(const float dt)
{
  int i;

  time += dt;

  int super_state = g_world_ptr->get_ks_controller(g_game_ptr->get_active_player())->get_super_state();

  if(super_state == SUPER_STATE_FLYBY)
    return;

  bool pp = false;

  if(prepPop)
  {
    popTime += dt;

    if((completedIcons < 5 && popTime > 0.7f) || popTime > 1.8f)
    {
      prepPop = false;
      PopCompletedIcons(true);
    }
    else
    {
      pp = true;
    }
  }

  for(i=0; i<5; i++)
  {
    if(prepMidPop[i])
    {
      midPopTime[i] += dt;

      if(midPopTime[i] > 0.5f)
      {
        prepMidPop[i] = false;
        PopIcon(i);
      }
      else
        pp = true;
    }
  }

  if(pp)
  {
    OnQueueChange();
    return;
  }

  if(effectType)
  {
    effectTime -= dt;
    startFail += dt;
    if(prepareFailure && prepareFailure < 5)
    {
      if(startFail > 0.12f)
        prepareFailure = 2;
      if(startFail > 0.24f)
        prepareFailure = 3;
      if(startFail > 0.36f)
        prepareFailure = 4;
      if(startFail > 0.48f)
        prepareFailure = 5;
    }
    if(effectTime < 0.0f)
    {
      if(prepareFailure)
      {
        failure = true;
        return;
      }
      effectType = 0;
      numEffectIcons = 0;
      completedIcons = 0;
      for(i=0; i<numIcons; i++)
        if(icons[i].vy == 0.0f)
          icons[i].vy = 0.01f;
    }
  }

  if(prepareFailure)
  {
    OnQueueChange();
    return;
  }

  if(super_state == SUPER_STATE_WIPEOUT)
    spawnTimer -= dt*iconSpawns[currentSpawn].timerSpeedWipeout;
  else if(super_state == SUPER_STATE_LIE_ON_BOARD)
    spawnTimer -= dt*iconSpawns[currentSpawn].timerSpeedLieOnBoard;
  else
    spawnTimer -= dt;

  lastDropTime += dt;

  if(spawnTimer < 0.0f)
    spawnTimer = 0.0f;

  if(super_state != SUPER_STATE_WIPEOUT && super_state != SUPER_STATE_LIE_ON_BOARD)
    buttonTimer += dt;

	if (nextIcon.resource == NULL || (enableNewIcons && spawnTimer <= 0.0f && effectType == 0))
	{
    int trickIdx, trickType;
    bool addTrick = GetNextTrick(trickIdx, trickType);

    // If the trick exists, add it, otherwise try again next frame
    if(addTrick)
    {
      if(nextIcon.resource == NULL)
        spawnTimer = initSpawnTime;
		  PushBack(trickIdx);
      if(numIcons > 0)
      {
		    spawnTimer = spawnDelay;
        spawnTimer *= iconSpawns[currentSpawn].timerMult[icons[numIcons-1].resource->iconType];
      }
      if(numIcons == 1)
        buttonTimer = 0.0f;

      if(!TIMER_IsInfiniteDuration() && TIMER_GetRemainingLevelSec() < 8.0f)
      {
        enableNewIcons = false;
        nextEnable = false;
      }
    }
	}

  OnQueueChange();
}


//  Last minute hack function.  Checks to see if this call of this function is the first time that the icon manager has had this many icons.
bool IGOIconManager::FirstTimeGettingThisMany(int goal, bool ignore_previous)
{
	if ((!done_before || ignore_previous) && iconCounter >= goal)
	{
		done_before = true;
		return true;
	}

	return false;
}


//	PushBack()
// Adds an icon with the specified trick to the screen display.
void IGOIconManager::PushBack(const int trickIdx)
{
  if(prepareFailure)
    return;

	IconResource *	res = FindResource(trickIdx);

  if(!res)
    return;

	if (numIcons+numEffectIcons >= maxIcons)
    PopCompletedIcons(true);

	if (numIcons >= maxIcons)
  {
    for(int i=0; i<numIcons; i++)
    {
      effectIcons[i] = icons[i];
      effectIcons[i].state = ICON_STATE_FROZEN;
      effectIcons[i].colorInterp = 0.0f;
    }
    prepareFailure = 1;
    numEffectIcons = 5;
    effectTime = 3.0f;
    startFail = 0.0f;
    effectType = 2;
    numIcons = 0;
		SoundScriptManager::inst()->playEvent(SS_ICON_FAILED_TRICKCHAIN);
    return;
  }

  bool inc = nextIcon.resource != NULL;
  if(nextIcon.resource)
  {
    lastDropTime = 0.0f;
	  icons[numIcons] = nextIcon;
    icons[numIcons].show = true;
		if (fallingIcon == NULL)
			fallingIcon = &icons[numIcons];
    icons[numIcons].state = (numIcons == activeIcon) ? ICON_STATE_ACTIVE : ICON_STATE_QUEUED;
    icons[numIcons].vy = iconSpawns[currentSpawn].velocity;
  }
  nextIcon.Initialize(*res);
  if(inc)
    numIcons++;
}

//	PopFront()
// Removes the top-most icon from the screen.
void IGOIconManager::PopFront(void)
{
  if(prepareFailure)
    return;

  failure = false;

	if (numIcons == 0)
		return;

  iconCounter++;

  icons[activeIcon].resource->timesDisplayed++;
	numIcons--;
	for (int i = 0; i < numIcons; i++)
		icons[i] = icons[i+1];


  if(numIcons == 0)
  {
    if(spawnTimer > maxTweenTime)
      spawnTimer = maxTweenTime;
  }
}

//	PopFront()
// Removes the top-most icon from the screen.
void IGOIconManager::PopCompletedIcons(bool success)
{
  if(prepareFailure)
    return;

	if (completedIcons == 0)
		return;

  failure = false;

  if(success)
  {
    iconCounter += completedIcons;
    int score = g_world_ptr->get_ks_controller(0)->my_scoreManager.GetScore();
    score += scoreBonuses[completedIcons-1];
    g_world_ptr->get_ks_controller(0)->my_scoreManager.SetScore(score);
    
    numEffectIcons = 0;
    completedIcons = 0;
    for(int i=0; i<numIcons; i++)
      if(icons[i].vy == 0.0f)
        icons[i].vy = 0.01f;
  }
	else
  {
		SoundScriptManager::inst()->playEvent(SS_ICON_FAILED_TRICKCHAIN);
    effectType = 2;
    for(int i=0; i<numEffectIcons; i++)
      effectIcons[i].state = ICON_STATE_FROZEN;
    effectTime = 2.0f;
  }

  for(int i=0; i<tsSize; i++)
    TrickChain(trickStack[i]);
  tsSize = 0;
}

void IGOIconManager::PopIcon(int icon)
{
  for(int i=icon; i<numIcons; i++)
  {
    if(i != numIcons-1)
      icons[i] = icons[i+1];
  }
  numIcons--;
  for(int i=icon; i<numIcons; i++)
    if(icons[i].vy == 0.0f)
      icons[i].vy = 0.01;

  iconCounter++;
}

//	TrickChain()
// Receives each trick that is completed, and acts upon it
void IGOIconManager::TrickChain(int trickIdx)
{
  if(prepareFailure)
    return;

  if(activeIcon >= numIcons)
    return;

  int region = g_world_ptr->get_ks_controller(0)->GetTrickRegion();



  for(int tc=activeIcon; tc<numIcons; tc++)
  {
	  #ifndef EVAN    // Evan is a lazy cheating bastard
	  if (trickIdx == icons[tc].resource->trickIdx ||
       (icons[tc].resource->trickIdx == ICON_TRICK_GENERIC_AIR && region == TREGION_AIR) ||
       (icons[tc].resource->trickIdx == ICON_TRICK_GENERIC_FACE && region == TREGION_FACE) ||
       (icons[tc].resource->trickIdx == ICON_TRICK_GENERIC_TUBE && region == TREGION_TUBE))
	  #endif
	  {
      if(tc > activeIcon)
      {
        if(prepMidPop[tc] == false)
        {
          prepMidPop[tc] = true;
          midPopTime[tc] = 0.0f;
          effectIcons[tc].state = ICON_STATE_GLOWING;
        }
        return;
      }

      if(prepPop && tsSize < 4)
      {
        trickStack[tsSize++] = trickIdx;
        return;
      }

      spawnDelay *= spawnTimerMultiplier;
      buttonTimer = 0.0f;

      chainedIcons = true;

      icons[activeIcon].resource->timesDisplayed++;

      effectIcons[numEffectIcons] = icons[activeIcon];
      numIcons--;
      for(int i=0; i<numIcons; i++)
        icons[i] = icons[i+1];
      effectIcons[numEffectIcons].state = ICON_STATE_GLOWING;
      effectIcons[numEffectIcons].colorInterp = 0.0f;
      completedIcons++;
      numEffectIcons++;
      icons[activeIcon].state = ICON_STATE_ACTIVE;

      if(activeIcon >= numIcons)
      {
        if(spawnTimer > maxTweenTime)
          spawnTimer = maxTweenTime;
      }
      if(completedIcons >= iconSpawns[currentSpawn].bonusIcons)
      {
        //float bonus = (iconSpawns[currentSpawn].bonusMultiplier-1) * g_world_ptr->get_ks_controller(0)->my_scoreManager.chain.GetMultiplier();
        //bonus += iconSpawns[currentSpawn].bonusAdder;
        //g_world_ptr->get_ks_controller(0)->my_scoreManager.chain.AddMultAdder(bonus);
			  SoundScriptManager::inst()->playEvent(SS_ICON_TETRIS);
 
        prepPop = true;
        popTime = 0.0f;
      }
		  else if (icons[activeIcon].vy != 0)
		  {
			  SoundScriptManager::inst()->playEvent(SS_ICON_DROP_COMPLETED);
		  }
		  else
		  {
			  SoundScriptManager::inst()->playEvent(SS_ICON_LAND_COMPLETED);
		  }

      return;
    }
  }
}

//	FinishChain()
// Trick chain is over, so clear completed icons
void IGOIconManager::FinishChain(bool success)
{
  if(prepareFailure)
    return;

  if(success)
  {
    if(!prepPop && completedIcons > 0)
    {
      chainedIcons = false;
      prepPop = true;
      popTime = 0.0f;
    }
  }
  else if(!prepPop)
  {
    tsSize = 0;
    scoreText->Break();
    PopCompletedIcons(false);
  }
}


//	GetFront()
// Returns the trick index of the icon at the front of the queue.
int IGOIconManager::GetFront(void)
{
	if (numIcons > 0)
		return icons[activeIcon].resource->trickIdx;

	return -1;
}
//	OnQueueChange()
// Private helper function that repositions the icons onscreen after they change.
void IGOIconManager::OnQueueChange(void)
{
  static float pci;

  int baseY = queueBottom;
  int buttonBaseY = baseY;
  int w, h, b;
	static bool first_showing=true;
  float dt = time - timeLastQueueChange;
  float dt_sqr = dt*dt;

  nextIcon.y = 30+smallIconHeight;
  if((lastDropTime > nextDelay) && (spawnTimer < nextTime+0.5) && (numIcons+numEffectIcons < maxIcons))
  {
    showTimer = enableTimer;
    pci += dt*iconSpawns[currentSpawn].morphSpeed;
    if(pci > 1.0f)
      pci = 1.0f;
    nglSetQuadRect(&nextIcon.quad, queueLeft, nextIcon.y-smallIconHeight, queueLeft+smallIconWidth, nextIcon.y);
	  nglSetQuadColor(&nextIcon.quad, NGL_RGBA32(255, 255, 255, FTOI(255.0f*pci + 0.0f*(1.0f-pci))));
		if (first_showing)
		{
			SoundScriptManager::inst()->playEvent(SS_ICON_POPUP);
			first_showing = false;
			
		}
    nextIcon.show = true;
  }
  else
  {
    showTimer = false;
    pci = 0.0f;
    nextIcon.show = false;
		first_showing = true;
	}

  // Set up effect icons
  numChains = 0;
  numFrozen = 0;
  numGlowing = 0;
  for(int i=0; i<numEffectIcons; i++)
  {
    float si = effectIcons[i].sizeInterp;
    float ci = effectIcons[i].colorInterp;

    effectIcons[i].show = true;

    if(!prepareFailure)
    {
      si -= dt*iconSpawns[currentSpawn].morphSpeed;
      if(si < 0.0f)
        si = 0.0f;
    }

    if(!prepareFailure || prepareFailure > i)
    {
      switch(effectIcons[i].state)
      {
        case ICON_STATE_GLOWING:
          ci += dt*iconSpawns[currentSpawn].morphSpeed;
          if(ci > 1.0f)
            ci = 1.0f;
          break;
        case ICON_STATE_FROZEN:
          ci -= dt*iconSpawns[currentSpawn].morphSpeed;
          if(ci < -1.0f)
            ci = -1.0f;
          break;
      }
    }

    effectIcons[i].sizeInterp = si;
    effectIcons[i].colorInterp = ci;

    w = FTOI((float)activeIconWidth*si + (float)smallIconWidth*(1.0f-si));
    h = FTOI((float)activeIconHeight*si + (float)smallIconHeight*(1.0f-si));
    b = 0;
    if(!iconSpawns[currentSpawn].buttonStayBottom)
      b = FTOI((float)buttonHeight*si);

	  nglSetQuadColor(&effectIcons[i].quad, NGL_RGBA32(255, 255, 255, 255));

    if(effectIcons[i].vy != 0.0f)
    {
      effectIcons[i].y  += dt*effectIcons[i].vy + 0.5f*dt_sqr*iconSpawns[currentSpawn].accel;
      effectIcons[i].vy += dt*iconSpawns[currentSpawn].accel;
      if(effectIcons[i].y > baseY)
			{
        if(i == 0 || effectIcons[i].vy > effectIcons[i-1].vy)
          effectIcons[i].vy = 0.0f;
			}
      else
        baseY = (int)effectIcons[i].y;
    }
    else
      effectIcons[i].y = baseY;

    if(i == numEffectIcons-1)
    {
      if(effectIcons[i].state == ICON_STATE_GLOWING)
        scoreText->unmakeRand();
      scoreText->changeText(stringx(scoreBonuses[numEffectIcons-1]));
      int sx = queueLeft + smallIconWidth/2;
      int sy = baseY - smallIconHeight/2;
      unadjustSizes(sx, sy);
      scoreText->changePos(sx, sy);
      if(prepPop)
      {
        scoreTextEffect->changeText(stringx(scoreBonuses[numEffectIcons-1]));
        scoreTextEffect->changePos(sx, sy);
        scoreTextEffect->color.c.a = (popTime < 0.5f) ? FTOI(255.0f*(0.5f-popTime)) : 0;
        scoreTextEffect->changeScale(1.0f + popTime*2.0f);
      }
    }

    float fi = 1.0f;
    // Update icons
    if(ci > 0.0f)
    {
      int glow = 0;
      if(effectIcons[i].resource->iconType == ICON_TYPE_AIR || effectIcons[i].resource->iconType == ICON_TYPE_SPECIAL_AIR)
      {
        nglSetQuadTex(&glowQuad[i], glowTexture[0]);
        glow = 11;
      }
      else if(effectIcons[i].resource->iconType == ICON_TYPE_FACE || effectIcons[i].resource->iconType == ICON_TYPE_SPECIAL_FACE)
      {
        nglSetQuadTex(&glowQuad[i], glowTexture[1]);
        glow = 5;
      }
      else if(effectIcons[i].resource->iconType == ICON_TYPE_TUBE || effectIcons[i].resource->iconType == ICON_TYPE_SPECIAL_TUBE)
      {
        nglSetQuadTex(&glowQuad[i], glowTexture[2]);
        glow = 8;
      }

      getIceTexture[i] = true;

      static float temp = 10.0f;
      static float os = 0.0f;
      static float od = 0.333f;

      float osc = 0.5f*(sinf(temp*TIMER_GetTotalSec()) + 1.0f);  // Oscilate (sp?) between 0 and 1
      glow += (int)(osc*glow);

      nglSetQuadColor(&glowQuad[i], NGL_RGBA32(255, 255, 255, FTOI((osc*od + os)*fabsf(ci)*255.0f)));
      nglSetQuadRect(&glowQuad[i], queueLeft-glow, baseY-b-h-glow, queueLeft+w+glow, baseY-b+glow);
      numGlowing++;
    }
    else if(ci < 0.0f)
    {
      if(effectTime < 0.333f)
        fi = 3.0f*effectTime;

#ifdef USE_ICE
      if(effectIcons[i].resource->iconType == ICON_TYPE_AIR || effectIcons[i].resource->iconType == ICON_TYPE_SPECIAL_AIR)
        nglSetQuadTex(&iceQuad[i], iceTexture[0]);
      else if(effectIcons[i].resource->iconType == ICON_TYPE_FACE || effectIcons[i].resource->iconType == ICON_TYPE_SPECIAL_FACE)
        nglSetQuadTex(&iceQuad[i], iceTexture[1]);
      else if(effectIcons[i].resource->iconType == ICON_TYPE_TUBE || effectIcons[i].resource->iconType == ICON_TYPE_SPECIAL_TUBE)
        nglSetQuadTex(&iceQuad[i], iceTexture[2]);
#else
      if(getIceTexture[i])
        nglSetQuadTex(&iceQuad[i], iceTexture[random(3)]);

      getIceTexture[i] = false;
#endif

      nglSetQuadColor(&iceQuad[i], NGL_RGBA32(255, 255, 255, FTOI(fabsf(ci)*fi*255.0f)));
      nglSetQuadRect(&iceQuad[i], queueLeft, baseY-b-h, queueLeft+w, baseY-b);
      numFrozen++;
    }

     // Update Chain link
    if(i > 0 && !prepareFailure)
    {
      nglSetQuadColor(&chainQuad[i-1], NGL_RGBA32(255, 255, 255, FTOI(fabsf(fi)*255.0f)));
      nglSetQuadRect(&chainQuad[i-1], queueLeft+smallIconWidth/2-5, baseY-15, queueLeft+smallIconWidth/2+5, baseY+15);
      numChains++;
    }

    nglSetQuadColor(&effectIcons[i].quad, NGL_RGBA32(255, 255, 255, FTOI(fabsf(fi)*255.0f)));
    nglSetQuadRect(&effectIcons[i].quad, queueLeft, baseY-b-h, queueLeft+w, baseY-b);
    baseY -= h+b;

 }

  // Set up icons
  for(int i=0; i<numIcons; i++)
  {
    float si = icons[i].sizeInterp;
    float ci = icons[i].colorInterp;

    icons[i].show = true;

    switch(icons[i].state)
    {
      case ICON_STATE_ACTIVE:
        si += dt*iconSpawns[currentSpawn].morphSpeed;
        if(si > 1.0f)
          si = 1.0f;
        ci += dt*iconSpawns[currentSpawn].morphSpeed;
        if(ci > 1.0f)
          ci = 1.0f;
        break;
      case ICON_STATE_QUEUED:
        si -= dt*iconSpawns[currentSpawn].morphSpeed;
        if(si < 0.0f)
          si = 0.0f;
        ci += dt*iconSpawns[currentSpawn].morphSpeed;
        if(ci > 1.0f)
          ci = 1.0f;
        break;
    }

    icons[i].sizeInterp = si;
    icons[i].colorInterp = ci;

    w = FTOI((float)activeIconWidth*si + (float)smallIconWidth*(1.0f-si));
    h = FTOI((float)activeIconHeight*si + (float)smallIconHeight*(1.0f-si));
    b = 0;
    if(!iconSpawns[currentSpawn].buttonStayBottom)
      b = FTOI((float)buttonHeight*si);

	  nglSetQuadColor(&icons[i].quad, NGL_RGBA32(255, 255, 255, 255));

    if(icons[i].vy != 0.0f)
    {
      icons[i].y  += dt*icons[i].vy + 0.5f*dt_sqr*iconSpawns[currentSpawn].accel;
      icons[i].vy += dt*iconSpawns[currentSpawn].accel;
      if(icons[i].y > baseY)
			{
				if (fallingIcon == &icons[i])
				{
					SoundScriptManager::inst()->playEvent(SS_ICON_LAND);
					fallingIcon = NULL;
				}
        if(i > 0 && icons[i].vy > icons[i-1].vy)
          icons[i].vy = 0.0f;
        else if(i == 0 && (numEffectIcons ==0 || icons[0].vy > effectIcons[numEffectIcons-1].vy))
          icons[i].vy = 0.0f;
			}
      else
        baseY = (int)icons[i].y;
    }
    else
      icons[i].y = baseY;

    // Update icons
    if(i == activeIcon)
    {
      if(iconSpawns[currentSpawn].buttonStayBottom)
      {
        nglSetQuadRect(&icons[i].quad, queueLeft, baseY-b-h, queueLeft+w, baseY-b);
        baseY -= h+b;
      }
      else
      {
        nglSetQuadRect(&icons[i].quad, queueLeft, baseY-b-h, queueLeft+w, baseY-b);
        baseY -= (h + b);
        buttonBaseY = baseY + h;
      }
    }
    else
    {
      // Update glowing effect
      nglSetQuadRect(&icons[i].quad, queueLeft, baseY-b-h, queueLeft+w, baseY-b);
      baseY -= h+b;
    }
  }

  drawButtons = false;

  // Set up buttons
  if(activeIcon < numIcons && icons[activeIcon].resource->trickIdx >= 0)
  {
    int b[3];
    b[0] = GTrickList[icons[activeIcon].resource->trickIdx].button1;
    b[1] = GTrickList[icons[activeIcon].resource->trickIdx].button2;
    b[2] = GTrickList[icons[activeIcon].resource->trickIdx].button3;

    int posx[5];
    if(b[2] == PAD_NONE)
    {
      posx[0] = queueLeft + (activeIconWidth>>1) - ((3*buttonWidth)>>1);
      posx[1] = queueLeft + (activeIconWidth>>1) - (buttonWidth>>1);
      posx[2] = queueLeft + (activeIconWidth>>1) + (buttonWidth>>1);
      numButtons = 3;
    }
    else
    {
      posx[0] = queueLeft + (activeIconWidth>>1) - ((5*buttonWidth)>>1);
      posx[1] = queueLeft + (activeIconWidth>>1) - ((3*buttonWidth)>>1);
      posx[2] = queueLeft + (activeIconWidth>>1) - (buttonWidth>>1);
      posx[3] = queueLeft + (activeIconWidth>>1) + (buttonWidth>>1);
      posx[4] = queueLeft + (activeIconWidth>>1) + ((3*buttonWidth)>>1);
      numButtons = 5;
    }

    for(int i=0; i<3; i++)
    {
      if(b[i] == PAD_NONE)
        continue;

      switch(b[i])
      {
        case PAD_U:
        case PAD_D:
        case PAD_L:
        case PAD_R:
        case PAD_UL:
        case PAD_UR:
        case PAD_DL:
        case PAD_DR:
          nglSetQuadTex(&buttonQuad[i<<1], buttonTexture[ICON_BUTTON_ARROW]);
          break;

        case PAD_CIRCLE:
			nglSetQuadTex(&buttonQuad[i<<1], buttonTexture[ICON_BUTTON_CIRCLE]);    
			break;
        case PAD_CROSS:     
			nglSetQuadTex(&buttonQuad[i<<1], buttonTexture[ICON_BUTTON_CROSS]);     
			break;
        case PAD_TRIANGLE:  
			nglSetQuadTex(&buttonQuad[i<<1], buttonTexture[ICON_BUTTON_TRIANGLE]);  
			break;
        case PAD_SQUARE:    
			nglSetQuadTex(&buttonQuad[i<<1], buttonTexture[ICON_BUTTON_SQUARE]);    
			break;
        default:
          assert(0 && "Invalid button combo");
      }

	  nglSetQuadMapFlags(&buttonQuad[i<<1], NGLMAP_CLAMP_U | NGLMAP_CLAMP_V | NGLMAP_BILINEAR_FILTER);
      nglSetQuadRect(&buttonQuad[i<<1], posx[i<<1], buttonBaseY, posx[i<<1]+buttonWidth, buttonBaseY+buttonHeight);
	    nglSetQuadColor(&buttonQuad[i<<1], NGL_RGBA32(255, 255, 255, 255));
	    nglSetQuadZ(&buttonQuad[i<<1], 800.0f);

      switch(b[i])
      {
        case PAD_D:  nglRotateQuad(&buttonQuad[i<<1], posx[i<<1]+(buttonWidth>>1), buttonBaseY+(buttonHeight>>1), PI);       break;
        case PAD_L:  nglRotateQuad(&buttonQuad[i<<1], posx[i<<1]+(buttonWidth>>1), buttonBaseY+(buttonHeight>>1), 1.5f*PI);  break;
        case PAD_R:  nglRotateQuad(&buttonQuad[i<<1], posx[i<<1]+(buttonWidth>>1), buttonBaseY+(buttonHeight>>1), 0.5f*PI);  break;
        case PAD_UL: nglRotateQuad(&buttonQuad[i<<1], posx[i<<1]+(buttonWidth>>1), buttonBaseY+(buttonHeight>>1), 1.75f*PI); break;
        case PAD_UR: nglRotateQuad(&buttonQuad[i<<1], posx[i<<1]+(buttonWidth>>1), buttonBaseY+(buttonHeight>>1), 0.25f*PI); break;
        case PAD_DL: nglRotateQuad(&buttonQuad[i<<1], posx[i<<1]+(buttonWidth>>1), buttonBaseY+(buttonHeight>>1), 1.25f*PI); break;
        case PAD_DR: nglRotateQuad(&buttonQuad[i<<1], posx[i<<1]+(buttonWidth>>1), buttonBaseY+(buttonHeight>>1), 0.75f*PI); break;
      }

      // Adjust button coord
      float sx = 1.0f, sy = 1.0f;
      adjustSizes(sx, sy);
      
      nglScaleQuad(&buttonQuad[i<<1], posx[i<<1]+(buttonWidth>>1), buttonBaseY+(buttonHeight>>1), sx, sy);

      if(i != 2 && b[i+1] != PAD_NONE)
      {
        if(b[2] != PAD_NONE)  // If it's a 3 button combo, only use comma's as separators
          nglSetQuadTex(&buttonQuad[(i<<1) + 1], buttonTexture[ICON_BUTTON_COMMA]);
        else if(i == 0)
        {
          if(b[1]>=PAD_U && b[1]<=PAD_DR)   // If second button in two button combo is a direction
            nglSetQuadTex(&buttonQuad[(i<<1) + 1], buttonTexture[ICON_BUTTON_COMMA]);
          else if(!(b[0]>=PAD_U && b[0]<=PAD_DR))   // If first button in two button combo is NOT a direction
            nglSetQuadTex(&buttonQuad[(i<<1) + 1], buttonTexture[ICON_BUTTON_COMMA]);
          else
            nglSetQuadTex(&buttonQuad[(i<<1) + 1], buttonTexture[ICON_BUTTON_PLUS]);
        }

		nglSetQuadMapFlags(&buttonQuad[(i<<1) + 1], NGLMAP_CLAMP_U | NGLMAP_CLAMP_V | NGLMAP_BILINEAR_FILTER);
        nglSetQuadRect(&buttonQuad[(i<<1) + 1], posx[(i<<1) + 1], buttonBaseY, posx[(i<<1) + 1]+buttonWidth, buttonBaseY+buttonHeight);
	      nglSetQuadColor(&buttonQuad[(i<<1) + 1], NGL_RGBA32(255, 255, 255, 255));
	      nglSetQuadZ(&buttonQuad[(i<<1) + 1], 800.0f);
      }
    }
  }

  timeLastQueueChange = time;
}

//	FindResource()
// Returns the icon resource with the specified trick index.
IGOIconManager::IconResource* IGOIconManager::FindResource(const int trickIdx)
{
	int i;

  if(trickIdx == ICON_TRICK_GENERIC_AIR)
    return &genericIconResources[0];
  else if(trickIdx == ICON_TRICK_GENERIC_FACE)
    return &genericIconResources[1];
  else if(trickIdx == ICON_TRICK_GENERIC_TUBE)
    return &genericIconResources[2];

	for (i = 0; i < numAirIconResources; i++)
	{
		if (airIconResources[i].trickIdx == trickIdx)
			return &airIconResources[i];
	}
	for (i = 0; i < numFaceIconResources; i++)
	{
		if (faceIconResources[i].trickIdx == trickIdx)
			return &faceIconResources[i];
	}
	for (i = 0; i < numTubeIconResources; i++)
	{
		if (tubeIconResources[i].trickIdx == trickIdx)
			return &tubeIconResources[i];
	}
	for (i = 0; i < numSpecialAirIconResources; i++)
	{
		if (specialAirIconResources[i].trickIdx == trickIdx)
			return &specialAirIconResources[i];
	}
	for (i = 0; i < numSpecialFaceIconResources; i++)
	{
		if (specialFaceIconResources[i].trickIdx == trickIdx)
			return &specialFaceIconResources[i];
	}
	for (i = 0; i < numSpecialTubeIconResources; i++)
	{
		if (specialTubeIconResources[i].trickIdx == trickIdx)
			return &specialTubeIconResources[i];
	}

	return NULL;
}

//	GetNextTrick()
// Picks a random trick based on the current settings
bool IGOIconManager::GetNextTrick(int & trickIdx, int & trickType)
{
  int i;

  if(currentSpawn == 0xFF || iconSpawnCounter >= iconSpawns[currentSpawn].numIcons)
  {
    currentSpawn++;
    iconSpawnCounter = 0;
    if(iconSpawns[currentSpawn].spawnTime > 0.0f)
      spawnDelay = iconSpawns[currentSpawn].spawnTime;
    if(iconSpawns[currentSpawn].spawnMultiplier > 0.0f)
      spawnTimerMultiplier = iconSpawns[currentSpawn].spawnMultiplier;
  }

  // Create the trick lists
  numAirTricks          = 0;
  numFaceTricks         = 0;
  numTubeTricks         = 0;
  numSpecialAirTricks   = 0;
  numSpecialFaceTricks  = 0;
  numSpecialTubeTricks  = 0;

  int t;
  for(t=0; t<numAirIconResources; t++)
  {
    if(airIconResources[t].avail && (iconSpawns[currentSpawn].trickListAir & airIconResources[t].bitmask) &&
       (lastIcon != &airIconResources[t] || sameIconCount < iconSpawns[currentSpawn].maxSameIcon))
      airTricklist[numAirTricks++] = airIconResources[t];
  }
  for(t=0; t<numFaceIconResources; t++)
  {
    if(faceIconResources[t].avail && (iconSpawns[currentSpawn].trickListFace & faceIconResources[t].bitmask) &&
       (lastIcon != &faceIconResources[t] || sameIconCount < iconSpawns[currentSpawn].maxSameIcon))
      faceTricklist[numFaceTricks++] = faceIconResources[t];
  }
  for(t=0; t<numTubeIconResources; t++)
  {
    if(tubeIconResources[t].avail && (iconSpawns[currentSpawn].trickListTube & tubeIconResources[t].bitmask) &&
       (lastIcon != &tubeIconResources[t] || sameIconCount < iconSpawns[currentSpawn].maxSameIcon))
      tubeTricklist[numTubeTricks++] = tubeIconResources[t];
  }
  for(t=0; t<numSpecialAirIconResources; t++)
  {
    if(specialAirIconResources[t].avail && (iconSpawns[currentSpawn].trickListSpecialAir & specialAirIconResources[t].bitmask) &&
       (lastIcon != &specialAirIconResources[t] || sameIconCount < iconSpawns[currentSpawn].maxSameIcon))
      specialAirTricklist[numSpecialAirTricks++] = specialAirIconResources[t];
  }
  for(t=0; t<numSpecialFaceIconResources; t++)
  {
    if(specialFaceIconResources[t].avail && (iconSpawns[currentSpawn].trickListSpecialFace & specialFaceIconResources[t].bitmask) &&
       (lastIcon != &specialFaceIconResources[t] || sameIconCount < iconSpawns[currentSpawn].maxSameIcon))
      specialFaceTricklist[numSpecialFaceTricks++] = specialFaceIconResources[t];
  }
  for(t=0; t<numSpecialTubeIconResources; t++)
  {
    if(specialTubeIconResources[t].avail && (iconSpawns[currentSpawn].trickListSpecialTube & specialTubeIconResources[t].bitmask) &&
       (lastIcon != &specialTubeIconResources[t] || sameIconCount < iconSpawns[currentSpawn].maxSameIcon))
      specialTubeTricklist[numSpecialTubeTricks++] = specialTubeIconResources[t];
  }

  // Get a random trick
  int ra  = iconSpawns[currentSpawn].air;
  int rf  = iconSpawns[currentSpawn].face;
  int rt  = iconSpawns[currentSpawn].tube;
  int rsa = iconSpawns[currentSpawn].specialAir;
  int rsf = iconSpawns[currentSpawn].specialFace;
  int rst = iconSpawns[currentSpawn].specialTube;
  int rga = iconSpawns[currentSpawn].genericAir;
  int rgf = iconSpawns[currentSpawn].genericFace;
  int rgt = iconSpawns[currentSpawn].genericTube;

  if(numAirTricks == 0 || repeatCount[ICON_TYPE_AIR] > 0 || (lastIconGroup == ICON_TYPE_AIR && sameIconGroupCount >= iconSpawns[currentSpawn].maxSameIconGroup))
    ra = 0;
  if(numFaceTricks == 0 || repeatCount[ICON_TYPE_FACE] > 0 || (lastIconGroup == ICON_TYPE_FACE && sameIconGroupCount >= iconSpawns[currentSpawn].maxSameIconGroup))
    rf = 0;
  if(numTubeTricks == 0 || repeatCount[ICON_TYPE_TUBE] > 0 || (lastIconGroup == ICON_TYPE_TUBE && sameIconGroupCount >= iconSpawns[currentSpawn].maxSameIconGroup))
    rt = 0;
  if(numSpecialAirTricks == 0 || repeatCount[ICON_TYPE_SPECIAL_AIR] > 0 || repeatCount[ICON_TYPE_SPECIAL_ANY] > 0 || (lastIconGroup == ICON_TYPE_AIR && sameIconGroupCount >= iconSpawns[currentSpawn].maxSameIconGroup))
    rsa = 0;
  if(numSpecialFaceTricks == 0 || repeatCount[ICON_TYPE_SPECIAL_FACE] > 0 || repeatCount[ICON_TYPE_SPECIAL_ANY] > 0 || (lastIconGroup == ICON_TYPE_FACE && sameIconGroupCount >= iconSpawns[currentSpawn].maxSameIconGroup))
    rsf = 0;
  if(numSpecialTubeTricks == 0 || repeatCount[ICON_TYPE_SPECIAL_TUBE] > 0 || repeatCount[ICON_TYPE_SPECIAL_ANY] > 0 || (lastIconGroup == ICON_TYPE_TUBE && sameIconGroupCount >= iconSpawns[currentSpawn].maxSameIconGroup))
    rst = 0;
  if(repeatCount[ICON_TYPE_AIR] > 0 || (lastIconGroup == ICON_TYPE_AIR && sameIconGroupCount >= iconSpawns[currentSpawn].maxSameIconGroup))
    rga = 0;
  if(repeatCount[ICON_TYPE_FACE] > 0 || (lastIconGroup == ICON_TYPE_FACE && sameIconGroupCount >= iconSpawns[currentSpawn].maxSameIconGroup))
    rgf = 0;
  if(repeatCount[ICON_TYPE_TUBE] > 0 || (lastIconGroup == ICON_TYPE_TUBE && sameIconGroupCount >= iconSpawns[currentSpawn].maxSameIconGroup))
    rgt = 0;

  // No tube tricks on small waves
	int current_beach = g_game_ptr->get_beach_id();
  if(BeachDataArray[current_beach].is_small_wave || current_beach == BEACH_MAVERICKS)  // No tube on Mavericks, and no general way to check if there's no tube
  {
    rt  = 0;
    rst = 0;
    rgt = 0;
  }

  int trickPercent = ra + rf + rt + rsa + rsf + rst + rga + rgf + rgt;
  if(trickPercent == 0)
  {
    for(i=0; i<ICON_NUM_TYPES; i++)
      repeatCount[i] = 0;
    return false;
  }
  int trickRand = random(trickPercent);

  if(trickRand < ra)
    trickType = ICON_TYPE_AIR;
  else if(trickRand < ra + rf)
    trickType = ICON_TYPE_FACE;
  else if(trickRand < ra + rf + rt)
    trickType = ICON_TYPE_TUBE;
  else if(trickRand < ra + rf + rt + rsa)
    trickType = ICON_TYPE_SPECIAL_AIR;
  else if(trickRand < ra + rf + rt + rsa + rsf)
    trickType = ICON_TYPE_SPECIAL_FACE;
  else if(trickRand < ra + rf + rt + rsa + rsf + rst)
    trickType = ICON_TYPE_SPECIAL_TUBE;
  else if(trickRand < ra + rf + rt + rsa + rsf + rst + rga)
    trickType = ICON_TYPE_GENERIC_AIR;
  else if(trickRand < ra + rf + rt + rsa + rsf + rst + rga + rgf)
    trickType = ICON_TYPE_GENERIC_FACE;
  else
    trickType = ICON_TYPE_GENERIC_TUBE;

  switch(trickType)
  {
    case ICON_TYPE_AIR:           trickIdx = airTricklist[random(numAirTricks)].trickIdx;   break;
    case ICON_TYPE_FACE:          trickIdx = faceTricklist[random(numFaceTricks)].trickIdx; break;
    case ICON_TYPE_TUBE:          trickIdx = tubeTricklist[random(numTubeTricks)].trickIdx; break;
    case ICON_TYPE_SPECIAL_AIR:   trickIdx = specialAirTricklist[random(numSpecialAirTricks)].trickIdx;   break;
    case ICON_TYPE_SPECIAL_FACE:  trickIdx = specialFaceTricklist[random(numSpecialFaceTricks)].trickIdx; break;
    case ICON_TYPE_SPECIAL_TUBE:  trickIdx = specialTubeTricklist[random(numSpecialTubeTricks)].trickIdx; break;
    case ICON_TYPE_GENERIC_AIR:   trickIdx = ICON_TRICK_GENERIC_AIR;  trickType = ICON_TYPE_AIR;  break;
    case ICON_TYPE_GENERIC_FACE:  trickIdx = ICON_TRICK_GENERIC_FACE; trickType = ICON_TYPE_FACE; break;
    case ICON_TYPE_GENERIC_TUBE:  trickIdx = ICON_TRICK_GENERIC_TUBE; trickType = ICON_TYPE_TUBE; break;
  }

  // Should never pick a trick not in the surfer's trickbook
  if((trickIdx != ICON_TRICK_GENERIC_AIR) && (trickIdx != ICON_TRICK_GENERIC_FACE) && (trickIdx != ICON_TRICK_GENERIC_TUBE))
    assert(unlockManager.isSurferTrickUnlocked(trickIdx));

  for(i=0; i<ICON_NUM_TYPES; i++)
    if(repeatCount[i] > 0)
      repeatCount[i]--;
  repeatCount[trickType] = iconSpawns[currentSpawn].repeatWait[trickType];
  if(trickType >= ICON_TYPE_SPECIAL_AIR)
    repeatCount[ICON_TYPE_SPECIAL_ANY] = iconSpawns[currentSpawn].repeatWait[ICON_TYPE_SPECIAL_ANY];
  iconSpawnCounter++;

  IconResource *tmp = FindResource(trickIdx);
  if(tmp == lastIcon)
    sameIconCount++;
  else
    sameIconCount = 1;
  lastIcon = tmp;

  if(trickType == ICON_TYPE_AIR || trickType == ICON_TYPE_SPECIAL_AIR)
  {
    if(lastIconGroup != ICON_TYPE_AIR)
    {
      lastIconGroup = ICON_TYPE_AIR;
      sameIconGroupCount = 0;
    }
    sameIconGroupCount++;
  }
  else if(trickType == ICON_TYPE_FACE || trickType == ICON_TYPE_SPECIAL_FACE)
  {
    if(lastIconGroup != ICON_TYPE_FACE)
    {
      lastIconGroup = ICON_TYPE_FACE;
      sameIconGroupCount = 0;
    }
    sameIconGroupCount++;
  }
  else if(trickType == ICON_TYPE_TUBE || trickType == ICON_TYPE_SPECIAL_TUBE)
  {
    if(lastIconGroup != ICON_TYPE_TUBE)
    {
      lastIconGroup = ICON_TYPE_TUBE;
      sameIconGroupCount = 0;
    }
    sameIconGroupCount++;
  }

  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	IGOIconManager::IconResource class
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//	IconResource()
// Default constructor.
IGOIconManager::IconResource::IconResource()
{
	texture = NULL;
	trickIdx = -1;
  timesDisplayed = 0;
}

//	~IconResource()
// Destructor.
IGOIconManager::IconResource::~IconResource()
{

}

//	Load()
// Initializes the resource and laods the specified texture.
void IGOIconManager::IconResource::Load(int type, int idx, int num, const stringx & texFilename)
{
  iconType  = (unsigned char)type;
	trickIdx  = idx;
	texture   = nglLoadTexture(texFilename.c_str());

  if(idx <= TRICK_NONE)
  {
    bitmask   = 0xFFFFFFFF;
    avail     = true;
  }
  else
  {
	int i;
    bitmask   = (0x01 << num);
	int current_surfer = g_game_ptr->GetSurferIdx(g_game_ptr->get_active_player());

	// Check through all the tricks in this surfer's trickbook.  If the trick isn't there, avail = false
	bool trick_exists = false;
	for(i = 0; i < TRICKBOOK_SIZE; i++)
	{
		if(SurferDataArray[current_surfer].trickBook[i] == idx)
		{
			trick_exists = true;
			break;
		}
	}
    avail     = trick_exists && unlockManager.isSurferTrickUnlocked(idx);
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	IGOIconManager::Icon class
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//	Icon()
// Default constructor.
IGOIconManager::Icon::Icon()
{
	show = true;
	resource = NULL;
}

//	~Icon()
// Destructor.
IGOIconManager::Icon::~Icon()
{

}

//	Initialize()
// Initializes this icon with the specified resource.
void IGOIconManager::Icon::Initialize(IconResource & res)
{
	show = true;
	resource = &res;
  y = 0;
  sizeInterp = 0.0f;
  colorInterp = 1.0f;

	nglInitQuad(&quad);
	nglSetQuadMapFlags(&quad, NGLMAP_CLAMP_U | NGLMAP_CLAMP_V | NGLMAP_BILINEAR_FILTER);
	nglSetQuadTex(&quad, res.texture);
	nglSetQuadColor(&quad, NGL_RGBA32(255, 255, 255, 255));
	nglSetQuadZ(&quad, 800.0f);
}

//	Draw()
// Adds this icon to the current render list.
void IGOIconManager::Icon::Draw(void)
{
	if (show)
		nglListAddQuad(&quad);
}

//	SetShow()
// Toggles the display of this icon.
void IGOIconManager::Icon::SetShow(const bool s)
{
	show = s;
}
