
#ifndef SEMAPHORESH
#define SEMAPHORESH

void CreateAllSemaphores();

#ifdef TARGET_XBOX

// XBOX SEMA STUFF
extern HANDLE AllocMemorySema;
extern HANDLE LoadNewStashSema;
extern HANDLE StallSurferLoadSema;
#define EXITTHREAD      ExitThread(1)
#define RELEASESEMA(x)  ReleaseSemaphore(x, 1, NULL)
#define GETSEMA(x)      WaitForSingleObject(x, INFINITE)
#define POLLSEMA(x)     (WaitForSingleObject(x, 0) == WAIT_OBJECT_0)
#define PASSTHREADCONTROL(x) {}

#elif defined (TARGET_PS2)

// PS2 SEMA STUFF
extern int LoadNewStashSema;
extern int AllocMemorySema;
extern int StallSurferLoadSema;
#define PASSTHREADCONTROL(x) RotateThreadReadyQueue(x)
#define EXITTHREAD      ExitDeleteThread()
#define RELEASESEMA(x)  SignalSema(x)
#define GETSEMA(x)      WaitSema(x)
#define POLLSEMA(x)     (PollSema(x) >= 0)


#else

// GC Sema stuff
extern int StallSurferLoadSema;
extern int LoadNewStashSema;
extern int AllocMemorySema;
#define PASSTHREADCONTROL(x) {}
#define EXITTHREAD      {}
#define RELEASESEMA(x)  {}
#define GETSEMA(x)      {}
#define POLLSEMA(x)    {}

#endif

#endif
