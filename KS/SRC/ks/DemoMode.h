#ifndef _DEMOHODEH
#define _DEMOHODEH

// The timeout interval
#define NUMBER_OF_REPLAYS 12

//#define DEMOMODE_OFF

class DemoModeManager
{
private:
  float totalTime;
  float timeoutDelay;
  float demoDuration;
  float demoTime;
  bool inDemo;
  bool played;
  bool demoStarted;
  void GoGame();
  char fname[24];
	bool wasInTitle;
public:
  unsigned int  replayFrames;
  unsigned int  replayMainPOFrames;
  unsigned int  replayAIPOFrames;
  nglFileBuf    replayFile;

public:
  DemoModeManager();
  void Init();
  bool tick(float timeInc);
  bool keyHit();
  void clear();
  bool inDemoMode();
  void ExitToFrontEnd();
	bool ReturnFromDemoToTitle();
	bool ReturnFromDemoToMain();
  bool wasInDemo;
};

extern DemoModeManager dmm;
#endif