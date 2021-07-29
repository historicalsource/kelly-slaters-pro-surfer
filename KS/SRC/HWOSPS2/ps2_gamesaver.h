/***************************************************************************************
  PS2_GameSaver.h
  Save NOW with game saver!  Up to 50%, 60%, even 70% off!

  Implements a game saving interface
***************************************************************************************/
#ifndef PS2_GAMESAVER_H
#define PS2_GAMESAVER_H

#if defined(TARGET_XBOX) || defined(TARGET_GC)
#error should not be compiling this file!
#endif

#include "libmc.h"
#include "CompressedPhoto.h"
#include "GlobalData.h"

#define PRODUCT_CODE "BASLUS-20334"
#define MAXITEMS 20

#define MC_STACK_SIZE 0x2000
#define STACK_WATCH 0xBE 
#define CHECK_MC_STACK (!(GenericGameSaver::inst()->dastack[15] == STACK_WATCH && GenericGameSaver::inst()->dastack[MC_STACK_SIZE+1] == STACK_WATCH)?bad_mc_stack_error=true:0)

// The number of memory ports
#define NUM_MEMORY_PORTS 2
// the number of memory slots per port.
#define NUM_MEMORY_SLOTS 1

#define GSOk 0
#define GSErrorUnformatted -1
#define GSErrorOther -2
#define GSErrorCountNotLoadModules -3
#define GSErrorUnknownMedia -4
#define GSErrorNoMedia -5
#define GSErrorFileExists -6
#define GSErrorNotEnoughSpace -7
#define GSErrorDoesNotExist -8
#define GSErrorBadVersion -9

#define CURRENT_GAME_SAVE_VERSION 1.0

typedef struct
{
  float version;
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

extern saveInfo currentGame;
#define INVALID_CARD_VALUE -1

extern int savePort, saveSlot;
class GenericGameSaver
{
public:
  GenericGameSaver() { };
  ~GenericGameSaver() {};

  // Here we pass the:
  // product code eg: BASLUS-12345
  // game Title: Kelly Slater Pro Surfer
  // the line break position in the title
  // the icon filename
  // the data of the icon file
  // the data size
  int init();
  int isFileValid( int port, int slot, saveInfo s );
  int getIconData();
  void releaseIconData();
  saveInfo getFileInfo() {return fInfo;};
  void setFileInfo(saveInfo s);
  int getFileListing(int port, int slot, saveInfo *s, void(*progressCallback)(void *,float),void *userCallbackData);
	int getFirstCard() { return 0; }
	void calcSavedGameSize();
	stringx getCardString(int port, int slot);
	stringx getShortCardString(int port, int slot);
	
	stringx getSavingString(int port, int slot, stringx saveWhat);
	stringx getLoadingString(int port, int slot, stringx saveWhat);
	stringx getDeletingString(int port, int slot, stringx saveWhat);
	stringx getNotEnoughRoomString(int port, int slot);
	stringx getInsertCardString(int port, int slot);
	stringx getUnavailableCardString(int port, int slot);
	stringx getOverwriteString(int port, int slot);
	stringx getFormattingString(int port, int slot);

	// It has to be 16byte aligned,
	// so the first 16 bytes are "tags"
  unsigned char dastack[MC_STACK_SIZE+17] __attribute__ ((aligned(16)));
  void saveFile(int port, int slot,
               void *data, int size,
               bool overWrite,
               void(*progressCallback)(void *, int),
               void *userCallbackData);

  void readFile(int port, int slot,
               void *buffer, int maxSize,
               void (*progressCallback)(void *, int),
               void *userCallbackData);
	
	int  saveSystemFile( int port, int slot, GlobalDataClass *,  void(*progressCallback)(void *, int), void *userCallbackData);
	int  readSystemFile( int port, int slot, GlobalDataClass *, void(*progressCallback)(void *, int), void *userCallbackData);
	bool hasSystemFile( int port, int slot );

	int  savePhoto (int port, int slot, saveInfo *game, int beach, CompressedPhoto* data );
	int  loadPhoto (int port, int slot, saveInfo *game, int beach, CompressedPhoto* data );
	bool photoExists (int port, int slot, saveInfo *game, int beach);
	bool photoExists(const int beachIdx);

  static int getInfo(int port, int slot, int *type, int *free, int *formatted);
  int format(int port, int slot);
  int deleteFile(int port, int slot, saveInfo s);
  void ReleaseAll() {}  // unmount all the memory units

  int getSavedGameSize() { return saved_game_size; }

DECLARE_SINGLETON(GenericGameSaver);

private:
  sceMcTblGetDir tble[MAXITEMS] __attribute__((aligned (64)));
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

	static int saved_game_size;
};

#endif // PS2_GAMESAVER_H




