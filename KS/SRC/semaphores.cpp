#include "global.h"

#include "semaphores.h"

#ifdef TARGET_XBOX

// The semaphores
HANDLE LoadNewStashSema;
HANDLE AllocMemorySema;
HANDLE StallSurferLoadSema;
//Create them
void CreateAllSemaphores()
{
  LoadNewStashSema = CreateSemaphore(NULL, 1, 1, "STASHLOADSEMA");
  AllocMemorySema = CreateSemaphore(NULL, 1, 1, "ALLOCMEMORYSEMA");
  StallSurferLoadSema = CreateSemaphore(NULL, 1, 1, "StallSurferLoadSema");
}

#elif defined(TARGET_PS2)

// The Semaphores
int AllocMemorySema;
int LoadNewStashSema;
int StallSurferLoadSema;
//Create them
void CreateAllSemaphores()
{
  SemaParam s;
  s.initCount = 1;
  s.maxCount = 1;
  LoadNewStashSema = CreateSema(&s);
  AllocMemorySema = CreateSema(&s);
  StallSurferLoadSema = CreateSema(&s);
}

#else

// The Semaphores
int AllocMemorySema;
int LoadNewStashSema;
int StallSurferLoadSema;

void CreateAllSemaphores()
{


}
#endif
