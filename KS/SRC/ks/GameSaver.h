/***************************************************************************************
  GameSaver.h
  Save NOW with game saver!  Up to 50%, 60%, even 70% off!

  Implements a game saving interface
***************************************************************************************/
#ifndef GAMESAVER_H
#define GAMESAVER_H

#if defined(TARGET_XBOX) || defined(TARGET_GC)
#include "global.h"
#include "singleton.h"
#else
#include "libmc.h"
#endif /* TARGET_XBOX JIV DEBUG */
#define PRODUCT_CODE "BISLPS-99999"
#define MAXITEMS 20

#define MC_STACK_SIZE 0x1000
#define GSOk 0
#define GSErrorUnformatted -1
#define GSErrorOther -2
#define GSErrorCountNotLoadModules -3
#define GSErrorUnknownMedia -4
#define GSErrorNoMedia -5
#define GSErrorFileExists -6
#define GSErrorNotEnoughSpace -7
#define GSErrorDoesNotExist -8

typedef struct
{
  int type;
  char shortname[8];
  char desc[68];
  long int timestamp;
  int valid;
} saveInfo;

typedef struct
{
  int port;
  int slot;
  saveInfo *info;
  int maxItems;
  void *buffer;
  int size;
  void *iconData;
  int iconSize;
  bool overWrite;
  void(*progressCallback)(void *, int);
  void *userCallbackData;
} GameSaverThreadArgs;


class GenericGameSaver
{
public:
  GenericGameSaver() { };
  ~GenericGameSaver() {};

  // Here we pass the:
  // product code eg: BISLPS-99999
  // game Title: Kelly Slater Pro Surfer
  // the line break position in the title
  // the icon filename
  // the data of the icon file
  // the data size
  int init();
  int isFileValid( int port, int slot, char *shortname );
  int getIconData();
  void releaseIconData();
  saveInfo getFileInfo() {return fInfo;};
  void setFileInfo(saveInfo s);
  int getFileListing(int port, int slot, saveInfo *s, void(*progressCallback)(void *,float),void *userCallbackData);

#if defined(TARGET_PS2)
  char dastack[MC_STACK_SIZE] __attribute__ ((aligned(16)));
#else
 char dastack[MC_STACK_SIZE];
#endif /* TARGET_PS2 JIV DEBUG */
  void saveFile(int port, int slot,
               void *data, int size,
               bool overWrite,
               void(*progressCallback)(void *, int),
               void *userCallbackData);

  void readFile(int port, int slot,
               void *buffer, int maxSize,
               void (*progressCallback)(void *, int),
               void *userCallbackData);

  static int getInfo(int port, int slot, int *type, int *free, int *formatted);
  int format(int port, int slot);
  int deleteFile(int port, int slot, saveInfo s);

DECLARE_SINGLETON(GenericGameSaver);

private:
#if defined(TARGET_PS2)
  sceMcTblGetDir tble[MAXITEMS] __attribute__((aligned (64)));
#endif /* TARGET_XBOX JIV DEBUG */
  nglFileBuf icon;
  saveInfo fInfo;
  GameSaverThreadArgs *arg;
  static void saveFileHelper(void *args);
  static void readFileHelper(void *args);

  static int readData(int port, int slot,
               char *name,
               void *data, int maxSize);
  static int saveData(int port, int slot, char *name, void *data, int size, bool overWrite);
  static int setupIcons(int port, int slot, unsigned char gameTitle[33], int breakPoint, char *sysIconName, char *iconName, void *iconData, int iconDataSize);

  static int numUsers;
};

#endif




