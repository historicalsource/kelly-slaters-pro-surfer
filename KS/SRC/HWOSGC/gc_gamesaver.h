/***************************************************************************************
  GC_GameSaver.h
  Save NOW with game saver!  Up to 50%, 60%, even 70% off!

  Implements a game saving interface
***************************************************************************************/
#ifndef GC_GAMESAVER_H
#define GC_GAMESAVER_H


#include "CompressedPhoto.h"
#include "GlobalData.h"
#define INVALID_CARD_VALUE -1
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
#define GSErrorIncompatible -10
#define GSErrorWrongRegion -11
#define GSErrorDamaged -12
#define GSErrorBadCRC -13
#define CURRENT_GAME_SAVE_VERSION 1.0

#define GC_GS_SHORTNAME_LEN 32

typedef struct
{
  int info_crc;
	int data_crc;
	float version;
  int type;
  char shortname[ GC_GS_SHORTNAME_LEN ];
  char desc[68];
  long int timestamp;
  int valid;
} saveInfo;


struct CARDStat;

void GSCallback(s32, s32);

extern saveInfo currentGame;
#define INVALID_CARD_VALUE -1

extern int savePort, saveSlot;

class GenericGameSaver
{
public:
  GenericGameSaver() { };
  ~GenericGameSaver() {};

  int init();
  int isFileValid( int slot, int _unused, char *shortname );
  int getIconData();
  void releaseIconData();
  saveInfo getFileInfo() {return fInfo;};
  void setFileInfo(saveInfo s);
  int getFileListing(int slot, int _unused, saveInfo *s, void(*progressCallback)(void *,float),void *userCallbackData);

  void saveFile(int slot, int _unused,
               void *data, int size,
               bool overWrite,
               void(*progressCallback)(void *, int),
               void *userCallbackData);

  void readFile(int slot, int _unused,
               void *buffer, int maxSize,
               void (*progressCallback)(void *, int),
               void *userCallbackData);

  int getInfo(int port, int slot, int *type, int *free, int *formatted);
  int format(int port, int slot);
  int deleteFile(int port, int slot, saveInfo s);
	int getFirstCard() { return 0; }
	
	stringx getErrorString(int port, int slot, int err);
	stringx getSavingString(int port, int slot, stringx saveWhat);
	stringx getLoadingString(int port, int slot, stringx saveWhat);
	stringx getDeletingString(int port, int slot, stringx saveWhat);
	stringx getNotEnoughRoomString(int port, int slot);
	stringx getInsertCardString(int port, int slot);
	stringx getUnavailableCardString(int port, int slot);
	stringx getOverwriteString(int port, int slot);
	stringx getFormattingString(int port, int slot);

	stringx getCardString(int port, int slot);
	stringx getShortCardString(int port, int slot);
	stringx getSlotString(int port, int slot);

	int  saveSystemFile( int port, int slot, GlobalDataClass *,  void(*progressCallback)(void *, int), void *userCallbackData);
	int  readSystemFile( int port, int slot, GlobalDataClass *, void(*progressCallback)(void *, int), void *userCallbackData);
	bool hasSystemFile( int port, int slot );

  int  savePhoto (int port, int slot, saveInfo *game, int beach, CompressedPhoto* data );
  int  loadPhoto (int port, int slot, saveInfo *game, int beach, CompressedPhoto* data );
  bool photoExists (int port, int slot, saveInfo *game, int beach);
  bool photoExists(const int beachIdx);

  void ReleaseAll() {}  // unmount all the memory units

  int getSavedGameSize() { return saved_game_size; }

DECLARE_SINGLETON(GenericGameSaver);

private:
  static void *gs_dispatch( void *param );
	void EXIT_CRITICAL();
	void ENTER_CRITICAL();
	void dispatch();
  int mount( int slot );
  int unmount( int slot );
  int setStatus( int slot, int file_no, struct CARDStat *stat );
  int getStatus( int slot, int file_no, struct CARDStat *stat );
  int CARDToGSError( int error );
  char *iconInit(struct CARDStat *f_stat, struct nglFileBuf *icon_file, int *size);
  int readData( int slot, saveInfo *sInfo, void *buffer, int size,
      void (*progressCallback)(void *, int), void *userCallbackData );
  int saveData( int slot, saveInfo *sInfo, void *buffer, int size, bool overWrite,
      void (*progressCallback)(void *, int), void *userCallbackData );
  
  saveInfo fInfo;
  void (*progress_callback)(void *,int);
  void *user_data;

	OSThread thread_data;
	OSSemaphore thread_sema;
	char *thread_stack;

	enum
	{
		DISPATCH_FILE_LISTING,
		DISPATCH_SAVE,
		DISPATCH_READ,
		DISPATCH_SAVE_SYSTEM,
		DISPATCH_LOAD_SYSTEM,
	};
	
	struct
	{
		int type;
		void (*progress_callback)(void *,int);
		void *user_callback_data;
		void *buffer;
		int slot;
		bool over_write;
		saveInfo *save_info;
		int size;
	} dispatch_data;

	static const int saved_game_size;
};

#endif // GC_GAMESAVER_H
