/***************************************************************************************
  xb_GameSaver.h
  Save NOW with game saver!  Up to 50%, 60%, even 70% off!

  Implements a game saving interface
***************************************************************************************/
#ifndef XB_GAMESAVER_H
#define XB_GAMESAVER_H

#include "global.h"
#include "singleton.h"
#include "GlobalData.h"
#define MAXITEMS 20
#define SAVE_FILENAME "savegame.bob"
//#define INFO_FILENAME "savegame.nfo"	// Not used anymore on Xbox.  Too many TRC issues. (dc 07/03/02)
#define PICTURE_FILE_EXTENSION ".xpr"
#define GLOBAL_DATA_SAVE_NAME "T:\\globaldata.dat"

// The number of memory ports
#define NUM_MEMORY_PORTS 4
// the number of memory slots per port.
#define NUM_MEMORY_SLOTS 2

#define MC_STACK_SIZE 0x1000

// Since -1 is valid for the HD
#define INVALID_CARD_VALUE -2
#define HARD_DRIVE_PORT -1


#define GSOk                         0
#define GSErrorUnformatted          -1
#define GSErrorCountNotLoadModules  -2
#define GSErrorUnknownMedia         -3
#define GSErrorNoMedia              -4
#define GSErrorFileExists           -5
#define GSErrorNotEnoughSpace       -6
#define GSErrorDoesNotExist         -7
#define GSErrorNotEnoughHDSpace		-8 // Not enough space on hard drive to mount a MU
#define GSErrorOther                -9 // This has to be the last one

typedef struct       // This is the struct that describes a saved game on the PS2....
{
	int type;
	char shortname[MAX_GAMENAME]; // the name of the saved game file 
	char desc[68];     // The description that's displayed in-game
	long int timestamp;
	int valid;         // whether the saved game file is valid or not (?)
	int version;
} saveInfo;
extern saveInfo currentGame;
extern int savePort, saveSlot;
typedef struct
{
	int mem_port, mem_slot;
	saveInfo *info;
	void *buffer;
	int size;
	bool overWrite;
	void(*progressCallback)(void *, int);
	void *userCallbackData;
} GameSaverThreadArgs;


class GenericGameSaver
{
public:
	GenericGameSaver();
	~GenericGameSaver() {}
	
	// Here we pass the:
	// product code eg: BISLPS-99999
	// game Title: Kelly Slater Pro Surfer
	// the line break position in the title
	// the icon filename
	// the data of the icon file
	// the data size
	int init();
	bool isFileValid(const void *buffer, 
		const unsigned bufsize, 
		const XCALCSIG_SIGNATURE &signature);
	void CalcSignature(const void *buffer,
		const unsigned bufsize, 
		XCALCSIG_SIGNATURE *signature);
	saveInfo getFileInfo() {return fInfo;};
	void setFileInfo(saveInfo s);
	int getFileListing(int port, int slot, saveInfo *s, void(*progressCallback)(void *,float),void *userCallbackData);
	int getFirstCard() { return -1; }
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
	
	char dastack[MC_STACK_SIZE];
	void saveFile(int port, int slot,
		void *data, int size,
		bool overWrite,
		void(*progressCallback)(void *, int),
		void *userCallbackData);
	
	void readFile(int port, int slot,
		void *buffer, int maxSize,
		void (*progressCallback)(void *, int),
		void *userCallbackData);
	
	static int getInfo(int port, int slot, int *type, int *free, int *formatted, char *card_name = NULL);
	int format(int port, int slot);
	int deleteFile(int port, int slot, saveInfo s);
	
	// For saving once per card stuff	
	int  saveSystemFile( int port, int slot, GlobalDataClass *,  void(*progressCallback)(void *, int), void *userCallbackData);
	int  readSystemFile( int port, int slot, GlobalDataClass *, void(*progressCallback)(void *, int), void *userCallbackData);
	bool hasSystemFile( int port, int slot );
	
	void ReleaseAll(); // unmount all the memory units
	
	void calcSavedGameSize();
	int getSavedGameSize() { assert(saved_game_size != -1); return saved_game_size; }
	
	DECLARE_SINGLETON(GenericGameSaver);
	
private:
	nglFileBuf icon;
	saveInfo fInfo;
	GameSaverThreadArgs arg;
	
	// for each memory unit this is the drive letter that the unit is 
	// mounted on.  If the unit isn't mounted then its drive leter is
	// set to zero.  Memory units should only be mounted as needed.
	char memory_unit_drive_letter[NUM_MEMORY_PORTS][NUM_MEMORY_SLOTS];
	int memory_unit_status[NUM_MEMORY_PORTS][NUM_MEMORY_SLOTS];
	
	void MountMU(int port, int slot);      // mount the MU unconditionally
	int CheckMountMU(int port, int slot);  // check whether the MU is mounted, and, if not, mount it
	void ResetMU(int port, int slot);      // Unmount the given MU and make it mountable again
	
	HANDLE MemoryCardSema; // a sempahpore to make sure only one thread accesses fInfo at a time
	
	friend DWORD WINAPI SaveThreadProc(LPVOID lpThreadParameter);
	friend DWORD WINAPI SaveHDThreadProc(LPVOID lpThreadParameter);
	friend DWORD WINAPI LoadThreadProc(LPVOID lpThreadParameter);
	//  static void saveFileHelper(void *args);
	//  static void readFileHelper(void *args);
	
	
	
	//  static int readData(int port, int slot,
	//               char *name,
	//               void *data, int maxSize);
	//  static int saveData(int port, int slot, char *name, void *data, int size, bool overWrite);
	//  static int setupIcons(int port, int slot, unsigned char gameTitle[33], int breakPoint, char *sysIconName, char *iconName, void *iconData, int iconDataSize);
	
	//  static int numUsers;
	
	static int saved_game_size;
};


// The thread function for saving a file on the MU
DWORD WINAPI SaveThreadProc(LPVOID lpThreadParameter);
// The thread function for saving a file on the HD
DWORD WINAPI SaveHDThreadProc(LPVOID lpThreadParameter);
// The thread function for loading a file from the MU
DWORD WINAPI LoadThreadProc(LPVOID lpThreadParameter);

#endif // XB_GAMESAVER_H




