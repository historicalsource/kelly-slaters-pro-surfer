// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#include "osGameSaver.h"

#include "osfile.h"
#include <sys\types.h>
#include "career.h"
#include "xb_storagedevice.h"	// For CXBStorageDevice, copied from XDK samples (dc 07/03/02)

int GenericGameSaver::saved_game_size = -1;	// to be initialized later (dc 07/03/02)

saveInfo currentGame;
int savePort = INVALID_CARD_VALUE, saveSlot= INVALID_CARD_VALUE;
DEFINE_SINGLETON(GenericGameSaver)

// Copy a normal string to a unicode string

WCHAR *narrow_to_wide_strcpy(WCHAR *dest, char *src)
{
	int index = 0;
	do
    dest[index] = src[index];
	while(dest[index++] != 0);
	
	return dest;
}

// Copy a unicode string to a normal string

char *wide_to_narrow_strcpy(char *dest, WCHAR *src)
{
	int index = 0;
	do
    dest[index] = src[index] & 0xff;
	while(dest[index++] != 0);
	
	return dest;
}

GenericGameSaver::GenericGameSaver() 
{ 
	int port, slot;
	
	// Initialize the list of drive letters to indicate that 
	// none of the memory units are mounted
	for(port = 0; port < NUM_MEMORY_PORTS; port++)
		for(slot = 0; slot < NUM_MEMORY_SLOTS; slot++)
			memory_unit_drive_letter[port][slot] = 0;

	calcSavedGameSize();
}

// Adapted from TechCertGame in XDK.  (dc 07/03/02)
void GenericGameSaver::calcSavedGameSize() 
{
    // Hard drive assumed
    CXBStorageDevice HardDrive( 'U' );

    DWORD dwBlockSize   = HardDrive.GetBlockSize();

    DWORD dwSaveGameSize = HardDrive.GetFileBytes( sizeof(GlobalDataClass) + sizeof(XCALCSIG_SIGNATURE) )
		+ HardDrive.GetFileBytes( sizeof(saveInfo) + sizeof(*g_career) + 2 * sizeof(XCALCSIG_SIGNATURE) );

	// Meta data image sizes
	const DWORD IMAGE_META_HDR_SIZE = 2048;           // 2K
	const DWORD IMAGE_META_DATA_SIZE = (64 * 64) / 2; // DXT1 is 4 bits per pixel

    DWORD dwImageSize = HardDrive.GetFileBytes( IMAGE_META_HDR_SIZE + IMAGE_META_DATA_SIZE );
    DWORD dwOverhead = HardDrive.GetSaveGameOverhead();

    saved_game_size = (dwSaveGameSize + dwImageSize + dwOverhead + dwBlockSize - 1) / dwBlockSize;
}

/////////////////////////////////////////////////////////////////////////////////////////
int GenericGameSaver::saveSystemFile(int port, int slot, GlobalDataClass *data, void(*progressCallback)(void *, int), void *userCallbackData)
{	
	int error;
	XCALCSIG_SIGNATURE signature;
	FILE *savefile = NULL;

	// Calculate a checksum of the global data
	HANDLE hSig = XCalculateSignatureBegin(0);
	XCalculateSignatureUpdate(hSig, (PBYTE)data, sizeof(GlobalDataClass));
	XCalculateSignatureEnd(hSig, &signature);

	char savename[] = GLOBAL_DATA_SAVE_NAME;
	
	// Open the save game file
	if(NULL == (savefile = fopen(savename, "wb")))
	{
		nglPrintf("Error opening the global data file\n");
		if(progressCallback != NULL)
			progressCallback(userCallbackData, GSErrorOther);
		return GSErrorOther;
	}

	// Write the checksum and the picture data
	if(1 != fwrite(&signature, sizeof(XCALCSIG_SIGNATURE), 1, savefile) ||
	   1 != fwrite(data, sizeof(GlobalDataClass), 1, savefile))
	{
		nglPrintf("Error writing data to the global data file");
		if(progressCallback != NULL)
			progressCallback(userCallbackData, GSErrorOther);
		fclose(savefile);
		return GSErrorOther;
	}

	// close the picture file
	if(0 != fclose(savefile))
	{
		nglPrintf("Error closing the global data file"); 
		if(progressCallback != NULL)
			progressCallback(userCallbackData, GSErrorOther);
		return GSErrorOther;
	}
	savefile = NULL;

	progressCallback(userCallbackData, 100);
	// Success!
	return GSOk;
}

int  GenericGameSaver::readSystemFile( int port, int slot, GlobalDataClass *data, void(*progressCallback)(void *, int), void *userCallbackData)
{
	int error;
	XCALCSIG_SIGNATURE signature, actual_sig;
	FILE *savefile = NULL;
	GlobalDataClass temp_data;

	char savename[] = GLOBAL_DATA_SAVE_NAME;
	
	// Open the save game file
	if(NULL == (savefile = fopen(savename, "rb")))
	{
		nglPrintf("Error opening the global data file for reading\n");
		if(progressCallback != NULL)
			progressCallback(userCallbackData, GSErrorOther);
		return GSErrorOther;
	}

	// Write the checksum and the picture data
	if(1 != fread(&signature, sizeof(XCALCSIG_SIGNATURE), 1, savefile) ||
	   1 != fread(&temp_data, sizeof(GlobalDataClass), 1, savefile))
	{
		nglPrintf("Error reading data from the global data file");
		fclose(savefile);
		if(progressCallback != NULL)
			progressCallback(userCallbackData, GSErrorOther);
		return GSErrorOther;
	}

	// close the picture file
	if(0 != fclose(savefile))
	{
		nglPrintf("Error closing the global data file after reading"); 
		if(progressCallback != NULL)
			progressCallback(userCallbackData, GSErrorOther);
		return GSErrorOther;
	}
	savefile = NULL;

	// Calculate a checksum of the data we just read in
	HANDLE hSig = XCalculateSignatureBegin(0);
	XCalculateSignatureUpdate(hSig, (PBYTE)data, sizeof(GlobalDataClass));
	XCalculateSignatureEnd(hSig, &actual_sig);

	// Make sure that the data we read in was valid by checking the checksum
	bool valid = true;
	for(int i = 0; i < XCALCSIG_SIGNATURE_SIZE; i++)
		if(signature.Signature[i] != actual_sig.Signature[i])
			valid = false;

	// If the checksum was valid then copy over the data into the pointer we were given
	if(valid)
		*data = temp_data;
	else
	{
		if(progressCallback != NULL)
			progressCallback(userCallbackData, GSErrorOther);
		return GSErrorOther;
	}

	progressCallback(userCallbackData, 100);
	return GSOk;
}

bool  GenericGameSaver::hasSystemFile( int port, int slot )
{	
	HANDLE hFind;
	WIN32_FIND_DATA find_data;

	hFind = FindFirstFile(GLOBAL_DATA_SAVE_NAME, &find_data);
	if(INVALID_HANDLE_VALUE == hFind) // if no file was found
		return false;

	FindClose(hFind);
	return true;
}



///////////////////////////////////////////////////////////////////////////////////

void GenericGameSaver::MountMU(int port, int slot)
{
	int error = XMountMU(port, slot, &(memory_unit_drive_letter[port][slot]));
	
	switch (error)
	{
	// Relevant return codes from XDK documentation
	case ERROR_SUCCESS:
	case ERROR_ALREADY_ASSIGNED:
		memory_unit_status[port][slot] = GSOk;
		break;
	case ERROR_DISK_FULL:
		memory_unit_drive_letter[port][slot] = 0;
		memory_unit_status[port][slot] = GSErrorNotEnoughSpace;
		break;
	case ERROR_DEVICE_NOT_CONNECTED:
		memory_unit_drive_letter[port][slot] = 0;
		memory_unit_status[port][slot] = GSErrorNoMedia;
		break;
	case ERROR_OUTOFMEMORY:
		memory_unit_drive_letter[port][slot] = 0;
		memory_unit_status[port][slot] = GSErrorNotEnoughHDSpace;
		break;
	case ERROR_UNRECOGNIZED_VOLUME:
	default:
		memory_unit_drive_letter[port][slot] = 0;
		memory_unit_status[port][slot] = GSErrorUnformatted;
		break;
	}
}
///////////////////////////////////////////////////////////////////////////////////

int GenericGameSaver::CheckMountMU(int port, int slot)
{
	PXPP_DEVICE_TYPE device_type;
	DWORD removals, insertions;
	
	assert ((port < NUM_MEMORY_PORTS && port >= 0) || port == HARD_DRIVE_PORT)
	if (port == HARD_DRIVE_PORT)
	{
		// the caller wants access to the hard drive.  That's easy.
		return GSOk;
	}
	
	bool changed = XGetDeviceChanges(XDEVICE_TYPE_MEMORY_UNIT, &insertions, &removals);
	
	if(!changed)
	{
		return memory_unit_status[port][slot];
	}

	// Now insertions and removals are bit maps with the following characteristics:
	//
	// Mask Name:                 Value: 
	// XDEVICE_PORT0_TOP_MASK     (1<<0) 
	// XDEVICE_PORT1_TOP_MASK     (1<<1) 
	// XDEVICE_PORT2_TOP_MASK     (1<<2) 
	// XDEVICE_PORT3_TOP_MASK     (1<<3) 
	// XDEVICE_PORT0_BOTTOM_MASK  (1<<16) 
	// XDEVICE_PORT1_BOTTOM_MASK  (1<<17) 
	// XDEVICE_PORT2_BOTTOM_MASK  (1<<18) 
	// XDEVICE_PORT3_BOTTOM_MASK  (1<<19) 
	//
	// This means that the least significant two bits indicate which controller, and
	// bit 4 indicates top if unset, bottom if set.
	
	// We have to check for every slot, since the next call to XGetDeviceChanges 
	// will no longer register the new insertions we see on this call.
	int a, b;
	
	for(a = 0; a < NUM_MEMORY_PORTS; a++)
	{
		for(b = 0; b < NUM_MEMORY_SLOTS; b++)
		{
			int shift = a + 16 * b;
			
			// if the memory unit was removed
			if(removals & (1 << shift))
			{
				ResetMU(a, b);
				memory_unit_status[a][b] = GSErrorNoMedia;
			}
			// if a memory unit was inserted
			if(insertions & (1 << shift))
			{
				assert(memory_unit_drive_letter[a][b] == 0);
				MountMU(a, b);
			}
		}
	}

	return memory_unit_status[port][slot];
}

///////////////////////////////////////////////////////////////////////////////////

void GenericGameSaver::ResetMU(int port, int slot)
{
	if(port > NUM_MEMORY_PORTS || port < 0)
	{
		// the caller wants to wrap up the HD.  Nothing to do.
		return;
	}
	
	// If the card has been mounted then unmount it
	if(memory_unit_drive_letter[port][slot] != 0)
		int error = XUnmountMU(port, slot);
	
	memory_unit_drive_letter[port][slot] = 0;
}

///////////////////////////////////////////////////////////////////////////////////


int GenericGameSaver::deleteFile(int port, int slot, saveInfo s)
{
	int error = CheckMountMU(port, slot);
	if(error != GSOk)
	{
		return error;
	}
	
	WCHAR savename[MAX_GAMENAME];
	char drive[]= "X:\\";
	if(port > NUM_MEMORY_PORTS || port < 0)
	{
		// the caller wants access to the hard drive.  That's drive U: for save games.
		drive[0] = 'U';
	}
	else
		drive[0] = memory_unit_drive_letter[port][slot];
	
	narrow_to_wide_strcpy(savename, s.shortname);
	
	if(ERROR_SUCCESS != XDeleteSaveGame(drive, savename))
	{
//		ResetMU(port, slot);
		return GSErrorOther;
	}
	
//	ResetMU(port, slot);
	
	return GSOk;
}

/////////////////////////////////////////////////////////////////////////////////////////

int GenericGameSaver::getInfo(int port, int slot, int *type, int *free, int *formatted, char *card_name)
{
	if(inst()->CheckMountMU(port, slot) == GSOk)
	{
		*formatted = 1;  // This is an assumption we can make because otherwise it wouldn't have mounted....
		
		char drive[] = "X:\\";
		if(port > NUM_MEMORY_PORTS || port < 0)
		{
			// the caller wants access to the hard drive.  That's drive U: for save games.
			drive[0] = 'U';
		}
		else
			drive[0] = inst()->memory_unit_drive_letter[port][slot];
		
		unsigned __int64 FreeBytesAvailable = 0;
		unsigned __int64 TotalNumberOfBytes = 0;
		unsigned __int64 TotalNumberOfFreeBytes = 0;
		
		if(!GetDiskFreeSpaceEx(
			drive, 
			(PULARGE_INTEGER)&FreeBytesAvailable, 
			(PULARGE_INTEGER)&TotalNumberOfBytes, 
			(PULARGE_INTEGER)&TotalNumberOfFreeBytes))
		{
			return GSErrorOther;
		}
		
		*free = FreeBytesAvailable / 0x4000; // block size is 16K
		
		if(card_name != NULL)
		{
			WCHAR w_card_name[MAX_MUNAME];
			if(drive[0] != 'U') // if this is a memory unit
			{
				XMUNameFromDriveLetter( drive[0], w_card_name, MAX_MUNAME);
				wide_to_narrow_strcpy(card_name, w_card_name);
			}
			else // it's the hard disk
				strcpy(card_name, ksGlobalTextArray[GT_FE_MENU_XBOX_HARD_DRIVE].c_str());
		}
		
#if 0	// Man, we're doing this at least once per frame.  That's insane.  (dc 07/06/02)
		// Check to make sure that we can actually create a file here....
		char fname[20];
		sprintf(fname, "%s%s", drive, "this.sux");
		HANDLE f = CreateFile(fname, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
		if( f == INVALID_HANDLE_VALUE )
		{
			*free = 0;
		}
		else
		{
			CloseHandle(f);
			DeleteFile(fname);
		}
#endif

//		inst()->ResetMU(port, slot);
		return GSOk;
	}
	
//	inst()->ResetMU(port, slot);
	return inst()->memory_unit_status[port][slot];
}

/////////////////////////////////////////////////////////////////////////////////////////

int GenericGameSaver::init()
{
	int port, slot;
	
	// Initialize the list of drive letters to indicate that 
	// none of the memory units are mounted
	ReleaseAll();

	DWORD inserted = XGetDevices(XDEVICE_TYPE_MEMORY_UNIT);
	
	// Now inserted is a bit map with the following characteristics:
	//
	// Mask Name:                 Value: 
	// XDEVICE_PORT0_TOP_MASK     (1<<0) 
	// XDEVICE_PORT1_TOP_MASK     (1<<1) 
	// XDEVICE_PORT2_TOP_MASK     (1<<2) 
	// XDEVICE_PORT3_TOP_MASK     (1<<3) 
	// XDEVICE_PORT0_BOTTOM_MASK  (1<<16) 
	// XDEVICE_PORT1_BOTTOM_MASK  (1<<17) 
	// XDEVICE_PORT2_BOTTOM_MASK  (1<<18) 
	// XDEVICE_PORT3_BOTTOM_MASK  (1<<19) 
	//
	// This means that the least significant two bits indicate which controller, and
	// bit 4 indicates top if unset, bottom if set.
	
	// We have to check for every slot, since this call to XGetDeviceChanges 
	// wiped out the previous one.
	int a, b;
	
	for(a = 0; a < NUM_MEMORY_PORTS; a++)
		for(b = 0; b < NUM_MEMORY_SLOTS; b++)
		{
			int shift = a + 16 * b;
			
			// if the memory unit is present
			if(inserted & (1 << shift))
			{
				// The MU is there, so mount it
				MountMU(a, b);
			}
			else
				memory_unit_status[a][b] = GSErrorNoMedia;
		}
		
	// Create a sempaphore that only allows one thread to have access to it at a time
	MemoryCardSema = CreateSemaphore(NULL, 1, 1, "GenericGameSaver");
		
	icon.Buf = NULL;
		
	return GSOk;
}


/////////////////////////////////////////////////////////////////////////////////////////


int GenericGameSaver::format(int port, int slot)
{
	assertmsg (false, "Game cannot format memory units on Xbox");
	return GSErrorOther;
}

/////////////////////////////////////////////////////////////////////////////////////////

int GenericGameSaver::getFileListing(int port, int slot, saveInfo *info, void(*progressCallback)(void *, float), void *userData)
{
	XGAME_FIND_DATA xgfd;
	HANDLE hFind;
	FILE *infofile = NULL;
	int num_saves = 0;
	XCALCSIG_SIGNATURE signature;
	int error = GSOk;
	
	// if the card isn't mounted already, do it now.
	if(CheckMountMU(port, slot) != GSOk)
	{
		return 0;
	}
	
	char drive_string[] = "X:\\";
	if(port > NUM_MEMORY_PORTS || port < 0)
	{
		// the caller wants access to the hard drive.  That's drive U: for save games.
		drive_string[0] = 'U';
	}
	else
		drive_string[0] = memory_unit_drive_letter[port][slot];
	
	// Start finding the save games
	hFind = XFindFirstSaveGame( drive_string, &xgfd );
	
	if( INVALID_HANDLE_VALUE != hFind )
	{
		do 
		{
			// copy over the string.  The name stored on disk is 
			// unicode, so we have to convert to ascii (we can't 
			// just use strcpy)
			wide_to_narrow_strcpy(info[num_saves].shortname, xgfd.szSaveGameName);
			
			char savename[MAX_PATH + 20];
			strcpy(savename, xgfd.szSaveGameDirectory);
			strcat(savename, SAVE_FILENAME);
			
			if(NULL == (infofile = fopen(savename, "rb")))
			{
				nglPrintf("Error opening the saved game info file for reading on port %d, slot %d, drive %s", port, slot, drive_string);
				error = GSErrorOther;
				goto filelistdone;
			}

			// Read in the save game info
			if(sizeof(saveInfo) != fread(&(info[num_saves]), 1, sizeof(saveInfo), infofile))
			{
				nglPrintf("Error reading the saved game file on port %d, slot %d, drive %s\n", arg.mem_port, arg.mem_slot, drive_string);
				error = GSErrorOther;
				goto filelistdone;
			}
			
			// Get the signature of the saved data
			if(1 != fread(&signature, sizeof(XCALCSIG_SIGNATURE), 1, infofile))
			{
				nglPrintf("Error reading the saved game signature on port %d, slot %d, drive %s\n", arg.mem_port, arg.mem_slot, drive_string);
				error = GSErrorOther;
				goto filelistdone;
			}
			
			fclose(infofile);
			infofile = NULL;

			// Check the signature on the saved data
			if(!GenericGameSaver::inst()->isFileValid(&(info[num_saves]), sizeof(saveInfo), signature)
				|| (info[num_saves].version != CAREER_DATA_VERSION))
			{
				nglPrintf("Bad saved game signature on port %d, slot %d, drive %s\n", arg.mem_port, arg.mem_slot, drive_string);
				error = GSErrorOther;
				goto filelistdone;
			}

			error = GSOk;

filelistdone:
			if (infofile)
			{
				fclose(infofile);
			}

			if(error != GSOk && progressCallback != NULL)
			{
				progressCallback(userData, error);
			}

			// We need to list all files, even corrupted ones.  That's because the user may try to
			// save a file of the same name as a corrupted file, and he won't understand why the 
			// save fails.  Also, the user may want to delete the corrupted file, so he'll need to see 
			// it.  Most games won't know at this stage whether a file is corrupted or not,
			// so we're not obligated to screen files here.  (dc 07/07/02)
			info[num_saves].valid = error;
#ifndef CHECK_VALIDITY
			if(error != GSOk)
			{
				info[num_saves].valid = GSOk;
				strcpy(info[num_saves].desc, info[num_saves].shortname);
				info[num_saves].type = 0;
			}
#endif
			num_saves++;
		} while( XFindNextSaveGame( hFind, &xgfd ) );
		
		XFindClose( hFind );
	}
	
//	ResetMU(port, slot);
	
	return num_saves;
}


/////////////////////////////////////////////////////////////////////////////////////////

void GenericGameSaver::readFile(int port, int slot,
								void *buffer, int maxSize,
								void (*progressCallback)(void *, int),
								void *userCallbackData)
{
	arg.userCallbackData = userCallbackData;
	arg.progressCallback = progressCallback;
	arg.buffer = buffer;
	arg.size = maxSize;
	arg.info = &fInfo;
	arg.mem_port = port;
	arg.mem_slot = slot;
	
	if (arg.info->valid != GSOk)
	{
		if(progressCallback)
		{
			// if it's one of the standard errors
			if(arg.info->valid < 0 && arg.info->valid > GSErrorOther)
				progressCallback(userCallbackData, arg.info->valid);
			else
				progressCallback(userCallbackData, GSErrorOther);
		}
		return;
	}
	
#ifdef THREADED	// We're fast enough to do this synchronously right now. (dc 07/07/02)
	if (NULL == CreateThread(NULL, 0, LoadThreadProc, &arg, 0, NULL))
	{
		if(progressCallback)
			progressCallback(userCallbackData, GSErrorOther);
		return;
	}
#else
	LoadThreadProc(&arg);
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////

DWORD WINAPI LoadThreadProc(LPVOID lpThreadParameter)
{
	GameSaverThreadArgs &arg = *((GameSaverThreadArgs *)lpThreadParameter);
	XGAME_FIND_DATA game_find_data;
	HANDLE hFindSave;
	HANDLE hFindFile;
	WIN32_FIND_DATA file_find_data;
	FILE *savefile = NULL;
	int num_saves = 0;
	XCALCSIG_SIGNATURE signature;
	int retval = GSOk;
	
	// Get the semaphore so we can modify arg.info
	WaitForSingleObjectEx(GenericGameSaver::inst()->MemoryCardSema, 1000, false);
	
	// if the card isn't mounted already, do it now.
	int error = GenericGameSaver::inst()->CheckMountMU(arg.mem_port, arg.mem_slot);
	if(error != GSOk)
	{
		retval = error;
		goto loadprocend;
	}
	
	char drive_string[] = "X:\\";
	if(arg.mem_port > NUM_MEMORY_PORTS || arg.mem_port < 0)
	{
		// the caller wants access to the hard drive.  That's drive U: for save games.
		drive_string[0] = 'U';
	}
	else
		drive_string[0] = GenericGameSaver::inst()->memory_unit_drive_letter[arg.mem_port][arg.mem_slot];
	
	// create a wide character string for doing the string 
	// compare when finding the appropriate saved game
	WCHAR w_savename[MAX_GAMENAME];
	narrow_to_wide_strcpy(w_savename, arg.info->shortname);
	
	// Find the first saved game
	hFindSave = XFindFirstSaveGame( drive_string, &game_find_data);
	int cmp = wcscmp(game_find_data.szSaveGameName, w_savename);
	bool no_more_files = (hFindSave == INVALID_HANDLE_VALUE);
	
	while(!no_more_files && cmp != 0) // while this isn't the one we want, and there are more saves...
	{
		// get the next saved game.
		no_more_files = !XFindNextSaveGame( hFindSave, &game_find_data);
		// compare this save name to the one we're trying to find
		cmp = wcscmp(game_find_data.szSaveGameName, w_savename);
	}
	
	XFindClose(hFindSave);
	
	if(cmp != 0) // if we didn't find the save we want...
	{
		retval = GSErrorDoesNotExist;
		goto loadprocend;
	}
	
	if(arg.progressCallback != NULL)
		arg.progressCallback(arg.userCallbackData, 10);
	
	// Now the path we're going to use should be in game_find_data.szSaveGameDirectory
	char savename[MAX_PATH + 20];
	
	strcpy(savename, game_find_data.szSaveGameDirectory);
	strcat(savename, SAVE_FILENAME);
	
	// Open the save game info file
	if(NULL == (savefile = fopen(savename, "rb")))
	{
		nglPrintf("Error opening the saved game info file on port %d, slot %d, drive %s\n", arg.mem_port, arg.mem_slot, drive_string);
		
		retval = GSErrorOther;
		goto loadprocend;
	}

	if(arg.progressCallback != NULL)
		arg.progressCallback(arg.userCallbackData, 20);
	
	// Read the data
	fread(arg.info, 1, sizeof(saveInfo), savefile);
	if(ferror(savefile))
	{
		nglPrintf("Error reading the saved game info file on port %d, slot %d, drive %s\n", arg.mem_port, arg.mem_slot, drive_string);
		
		retval = GSErrorOther;
		goto loadprocend;
	}
	
	if(arg.progressCallback != NULL)
		arg.progressCallback(arg.userCallbackData, 30);
	
	// Get the signature of the saved data
	if(1 != fread(&signature, sizeof(XCALCSIG_SIGNATURE), 1, savefile))
	{
		nglPrintf("Error reading the saved game signature on port %d, slot %d, drive %s\n", arg.mem_port, arg.mem_slot, drive_string);
		
		retval = GSErrorOther;
		goto loadprocend;
	}
	
	// Check the signature on the saved data
	if(!GenericGameSaver::inst()->isFileValid(arg.info, sizeof(saveInfo), signature))
	{
		retval = GSErrorOther;
		goto loadprocend;
	}
	
	if(arg.progressCallback != NULL)
		arg.progressCallback(arg.userCallbackData, 40);
	
	// Read the data
	fread(arg.buffer, 1, arg.size, savefile);
	if(ferror(savefile))
	{
		nglPrintf("Error reading the saved game file on port %d, slot %d, drive %s\n", arg.mem_port, arg.mem_slot, drive_string);
		
		retval = GSErrorOther;
		goto loadprocend;
	}
	
	if(arg.progressCallback != NULL)
		arg.progressCallback(arg.userCallbackData, 50);
	
	// Get the signature of the saved data
	if(1 != fread(&signature, sizeof(XCALCSIG_SIGNATURE), 1, savefile))
	{
		nglPrintf("Error reading the saved game signature on port %d, slot %d, drive %s\n", arg.mem_port, arg.mem_slot, drive_string);
		
		retval = GSErrorOther;
		goto loadprocend;
	}
	
	// Check the signature on the saved data
	if(!GenericGameSaver::inst()->isFileValid(arg.buffer, arg.size, signature))
	{
		retval = GSErrorOther;
		goto loadprocend;
	}
	
	if(arg.progressCallback != NULL)
		arg.progressCallback(arg.userCallbackData, 80);
	
	// close the save game file
	if(0 != fclose(savefile))
	{
		nglPrintf("Error closing the saved game file on port %d, slot %d, drive %s\n", arg.mem_port, arg.mem_slot, drive_string);
		
		retval = GSErrorOther;
		goto loadprocend;
	}
	savefile = NULL;
	
	if(arg.progressCallback != NULL)
		arg.progressCallback(arg.userCallbackData, 90);

	retval = GSOk;
	
loadprocend:
	if (savefile)
	{
		fclose(savefile);
	}
	
	// Must come before callback, since callback remounts MU.  (dc 07/06/02)
//	GenericGameSaver::inst()->ResetMU(arg.mem_port, arg.mem_slot);

	// Release the semaphore
	ReleaseSemaphore(GenericGameSaver::inst()->MemoryCardSema, 1, NULL);

	if(arg.progressCallback != NULL)
	{
		if(retval != GSOk)
		{
			arg.progressCallback(arg.userCallbackData, retval);
		}
		else
		{
			arg.progressCallback(arg.userCallbackData, 100);
		}
	}
	
	return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////

bool GenericGameSaver::isFileValid(const void *buffer, 
                                   const unsigned bufsize, 
                                   const XCALCSIG_SIGNATURE &signature)
{
	HANDLE hSig = XCalculateSignatureBegin(0);
	
	XCalculateSignatureUpdate(hSig, (PBYTE)buffer, bufsize);
	
	XCALCSIG_SIGNATURE new_sig;
	XCalculateSignatureEnd(hSig, &new_sig);
	
	for(int a = 0; a < XCALCSIG_SIGNATURE_SIZE; a++)
	{
		if (new_sig.Signature[a] != signature.Signature[a])
			return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

void GenericGameSaver::CalcSignature(const void *buffer,
                                     const unsigned bufsize, 
                                     XCALCSIG_SIGNATURE *signature)
{
	HANDLE hSig = XCalculateSignatureBegin(0);
	
	XCalculateSignatureUpdate(hSig, (PBYTE)buffer, bufsize);
	
	XCalculateSignatureEnd(hSig, signature);
}

/////////////////////////////////////////////////////////////////////////////////////////

void GenericGameSaver::saveFile(int port, int slot,
								void *data, int size,
								bool overWrite,
								void (*progressCallback)(void *, int),
								void *userCallbackData)
{
	// Load up the thread's arguments
	arg.mem_port         = port;
	arg.mem_slot         = slot;
	arg.info             = &fInfo;
	arg.buffer           = data;
	arg.size             = size;
	arg.overWrite        = overWrite;
	arg.progressCallback = progressCallback;
	arg.userCallbackData = userCallbackData;
	
	//return;	// The following call crashes the front end (dc 04/23/02)
	
#ifdef THREADED	// We're fast enough to do this synchronously right now. (dc 07/07/02)
	// Create the thread 
	if (NULL == CreateThread(NULL, 0, SaveThreadProc, &arg, 0, NULL))
	{
		if(progressCallback)
			progressCallback(userCallbackData, GSErrorOther);
		return;
	}
#else
	SaveThreadProc(&arg);
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////

DWORD WINAPI SaveThreadProc(LPVOID lpThreadParameter)
{
	GameSaverThreadArgs &arg = *((GameSaverThreadArgs *)lpThreadParameter);
	int error = 0;
	char drive_string[] = "X:\\";
	char save_game_path[MAX_PATH + 1] = "";  // This is where XCreateSaveGame will put 
	                                         // the path to use for saving the game
	FILE *savefile = NULL;
	XCALCSIG_SIGNATURE signature;
	int retval = GSOk;
	
	// Wait for the semaphore
	WaitForSingleObjectEx(GenericGameSaver::inst()->MemoryCardSema, 1000, false);
	
	// Make sure the memory card is in the slot
	error = GenericGameSaver::inst()->CheckMountMU(arg.mem_port, arg.mem_slot);
	if(error != GSOk)
	{
		retval = error;
		goto saveprocend;
	}
	
	if(arg.progressCallback != NULL)
		arg.progressCallback(arg.userCallbackData, 5);
	
	if(arg.mem_port > NUM_MEMORY_PORTS || arg.mem_port < 0)
	{
		// the caller wants access to the hard drive.  That's drive U: for save games.
		drive_string[0] = 'U';
	}
	else
		drive_string[0] = GenericGameSaver::inst()->memory_unit_drive_letter[arg.mem_port][arg.mem_slot];
	
	// Now we need to create a unicode string to pass to XCreateSaveGame
	WCHAR w_savename[MAX_GAMENAME];
	narrow_to_wide_strcpy(w_savename, arg.info->desc);
	
	int open_condition;
	if(arg.overWrite)
		open_condition = OPEN_ALWAYS;
	else
		open_condition = CREATE_NEW;
	
	// tag is the number that will be added after the tilde for duplicate filenames
	int tag = 1;
	int savename_len = wcslen(w_savename);
	error = XCreateSaveGame(drive_string, w_savename, open_condition, 0, save_game_path, MAX_PATH + 1);
	
	// If it's a duplicated file name then try again with tags on the end to differentiate
	while(error == ERROR_ALREADY_EXISTS)
	{
		tag++;
		char tag_string[8];
		sprintf(tag_string, "~%d", tag);
		narrow_to_wide_strcpy(w_savename + savename_len, tag_string);
		error = XCreateSaveGame(drive_string, w_savename, CREATE_NEW, 0, save_game_path, MAX_PATH + 1);
	}
	
	if(ERROR_SUCCESS != error)
	{
		nglPrintf("Error creating the saved game on port %d, slot %d, drive %s\n", arg.mem_port, arg.mem_slot, drive_string);
		
		if(arg.progressCallback != NULL)
		{
			if(error == ERROR_FILE_EXISTS)
			{
				retval = GSErrorFileExists;
			}
			else
			{
				retval = GSErrorOther;
			}
			goto saveprocend;
		}
	}
	
	// Copy the new save name into arg.info.shortname so the program'll know 
	// the filename when it loads it back in
	wide_to_narrow_strcpy(arg.info->shortname, w_savename);
	
	if(arg.progressCallback != NULL)
		arg.progressCallback(arg.userCallbackData, 10);
	
	// Now the path we're going to use should be in save_game_path
	char savename[MAX_PATH + 20];
	
	strcpy(savename, save_game_path);
	strcat(savename, SAVE_FILENAME);
	
	// Open the save game info file
	if(NULL == (savefile = fopen(savename, "wb")))
	{
		nglPrintf("Error opening the saved game info file on port %d, slot %d, drive %s\n", arg.mem_port, arg.mem_slot, drive_string);
		
		retval = GSErrorOther;
		goto saveprocend;
	}
	
	if(arg.progressCallback != NULL)
		arg.progressCallback(arg.userCallbackData, 20);
	
	// Write the data
	arg.info->timestamp = time(NULL);
	arg.info->valid     = GSOk;
	arg.info->version   = CAREER_DATA_VERSION;
	if(sizeof(saveInfo) != fwrite(arg.info, 1, sizeof(saveInfo), savefile))
	{
		nglPrintf("Error writing the saved game info file on port %d, slot %d, drive %s\n", arg.mem_port, arg.mem_slot, drive_string);
		
		retval = GSErrorOther;
		goto saveprocend;
	}
	
	if(arg.progressCallback != NULL)
		arg.progressCallback(arg.userCallbackData, 30);
	
	// Get the signature of the saved data
	GenericGameSaver::inst()->CalcSignature(arg.info, sizeof(saveInfo), &signature);
	if(1 != fwrite(&signature, sizeof(XCALCSIG_SIGNATURE), 1, savefile))
	{
		nglPrintf("Error writing the saved game signature on port %d, slot %d, drive %d\n", arg.mem_port, arg.mem_slot, drive_string);
		
		retval = GSErrorOther;
		goto saveprocend;
	}
	
	if(arg.progressCallback != NULL)
		arg.progressCallback(arg.userCallbackData, 40);
	
	// Write the data
	if(arg.size != fwrite(arg.buffer, 1, arg.size, savefile))
	{
		nglPrintf("Error writing the saved game file on port %d, slot %d, drive %s\n", arg.mem_port, arg.mem_slot, drive_string);
		
		retval = GSErrorOther;
		goto saveprocend;
	}
	
	if(arg.progressCallback != NULL)
		arg.progressCallback(arg.userCallbackData, 80);
	
	// Get the signature of the saved data
	GenericGameSaver::inst()->CalcSignature(arg.buffer, arg.size, &signature);
	if(1 != fwrite(&signature, sizeof(XCALCSIG_SIGNATURE), 1, savefile))
	{
		nglPrintf("Error writing the saved game signature on port %d, slot %d, drive %d\n", arg.mem_port, arg.mem_slot, drive_string);
		
		retval = GSErrorOther;
		goto saveprocend;
	}
	
	// close the save game file
	if(0 != fclose(savefile))
	{
		nglPrintf("Error closing the saved game file on port %d, slot %d, drive %s\n", arg.mem_port, arg.mem_slot, drive_string);
		
		retval = GSErrorOther;
		goto saveprocend;
	}
	savefile = NULL;
	
	if(arg.progressCallback != NULL)
		arg.progressCallback(arg.userCallbackData, 90);
	
	// Now we've saved the data, so let's save the icon metadata.
	
	// Provide the metadata image file.
	// Append the image file name to the end of the save game path name
	// and copy our image file to the save game directory.
	/*  strcpy( savename, save_game_path);
	strcat( savename, "saveimage.xbx");
	if(!CopyFile( "d:\\saveimage.xbx", savename, FALSE ))
	{
    nglPrintf("Error copying icon for save file\n");
	}
	*/

	retval = GSOk;

saveprocend:
	if (savefile)
	{
		fclose(savefile);
	}

	// Must come before callback, since callback remounts MU.  (dc 07/06/02)
//	GenericGameSaver::inst()->ResetMU(arg.mem_port, arg.mem_slot);
	
	ReleaseSemaphore(GenericGameSaver::inst()->MemoryCardSema, 1, NULL);

	if(arg.progressCallback != NULL)
	{
		if(retval != GSOk)
		{
			arg.progressCallback(arg.userCallbackData, retval);
		}
		else
		{
			arg.progressCallback(arg.userCallbackData, 100);
		}
	}
	
	return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////

void GenericGameSaver::setFileInfo(saveInfo s)
{
	WaitForSingleObjectEx(MemoryCardSema, 1000, false);
	fInfo = s;
	ReleaseSemaphore(MemoryCardSema, 1, NULL);
};

/////////////////////////////////////////////////////////////////////////////////////////

void GenericGameSaver::ReleaseAll()
{
	int port, slot;
	
	for(port = 0; port < NUM_MEMORY_PORTS; port++)
		for(slot = 0; slot < NUM_MEMORY_SLOTS; slot++)
		{
			if(memory_unit_drive_letter[port][slot])
			{
				int error = XUnmountMU(port, slot);
				if(error != 0)
					nglPrintf("Error unmounting memory unit from %c on port %d slot %d: %x", memory_unit_drive_letter[port][slot], port, slot, error);
				memory_unit_drive_letter[port][slot] = 0;
				memory_unit_status[port][slot] = 0;
			}
		}
}
stringx GenericGameSaver::getShortCardString(int port, int slot)
{
	return getCardString(port, slot);
}

stringx GenericGameSaver::getCardString(int port, int slot)
{
	char errortxt[100];
	int type, free, formatted;
	char card_name[MAX_MUNAME] = "";	// initialize to strlen 0, in case doesn't get filled in (dc 07/07/02)
	int ret;

	assert(port != INVALID_CARD_VALUE && slot != INVALID_CARD_VALUE);

	if (port == -1)
		return ksGlobalTextArray[GT_FE_MENU_XBOX_HARD_DRIVE];
	
	ret = getInfo(port, slot,&type, &free, &formatted, card_name);
	if (ret != GSOk || strlen(card_name) == 0)
	{
		sprintf(errortxt, ksGlobalTextArray[GT_MEMORY_CARD_XBOX].c_str(), port+1, slot + 'A');
	}
	else
	{
		if (strlen(card_name) > 10)
		{
			card_name[10] = '.';
			card_name[11] = '.';
			card_name[12] = '.';
			card_name[13] = '\0';
		}
		// Don't know why this string is called GT_MEMORY_CARD_ERROR_XBOX, since there's no error.  (dc 07/05/02)
		sprintf(errortxt, ksGlobalTextArray[GT_MEMORY_CARD_ERROR_XBOX].c_str(), port+1, slot + 'A', card_name);
	}

	return stringx(errortxt);
}
stringx GenericGameSaver::getSavingString(int port, int slot, stringx saveWhat)
{
	char error[200];
	sprintf(error, ksGlobalTextArray[GT_MC_SAVING_XB].c_str(), saveWhat.c_str(), getCardString(port, slot).c_str());
	return stringx(error);
}
stringx GenericGameSaver::getLoadingString(int port, int slot, stringx saveWhat)
{
	char error[200];
	sprintf(error, ksGlobalTextArray[GT_MC_LOADING_XB].c_str(), saveWhat.c_str());
	return stringx(error);
}
stringx GenericGameSaver::getDeletingString(int port, int slot, stringx saveWhat)
{
	char error[200];
	sprintf(error, ksGlobalTextArray[GT_MC_DELETING_XB].c_str(), saveWhat.c_str());
	return stringx(error);
}

stringx GenericGameSaver::getNotEnoughRoomString(int port, int slot)
{
	stringx retval(stringx::fmt, ksGlobalTextArray[GT_FE_MENU_NO_SPACE].c_str(), inst()->getCardString(port, slot).c_str(), 
		getSavedGameSize(), ksGlobalTextArray[GT_MC_BLOCKS].c_str());
	retval.to_upper();

	return retval;
}

stringx GenericGameSaver::getInsertCardString(int port, int slot)
{
	return ksGlobalTextArray[GT_FE_MENU_NO_CARD]+" "+getCardString(port, slot);
}

stringx GenericGameSaver::getUnavailableCardString(int port, int slot)
{
	switch (memory_unit_status[port][slot])
	{
	case GSErrorNotEnoughSpace:
		return ksGlobalTextArray[GT_FE_MENU_NO_MOUNT];
	case GSErrorNotEnoughHDSpace:
		return ksGlobalTextArray[GT_FE_MENU_NO_MOUNT_HD];
	case GSErrorUnformatted:
	default:
		return ksGlobalTextArray[GT_FE_MENU_CORRUPT];
	}
}

stringx GenericGameSaver::getOverwriteString(int port, int slot)
{
	return ksGlobalTextArray[GT_FE_MENU_OVERWRITE].c_str();
}

stringx GenericGameSaver::getFormattingString(int port, int slot)
{
	char sentance2[100];
	if (port != -1)
		sprintf(sentance2, ksGlobalTextArray[GT_FE_MENU_FORMATTING].c_str(), getCardString(port, slot).c_str(), getCardString(port, slot).c_str());
	else
		sprintf(sentance2, ksGlobalTextArray[GT_FE_MENU_FORMATTING_HD].c_str(), getCardString(port, slot).c_str());
	return stringx(sentance2);

}