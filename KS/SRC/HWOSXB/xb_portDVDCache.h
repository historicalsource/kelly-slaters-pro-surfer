//xb_portDVDCache.h --  handles asynchonous load of game files to utility drive!

#ifndef XB_PORTDVDCACHE_H
#define XB_PORTDVDCACHE_H

#ifdef TARGET_XBOX  
#include <xtl.h>
#else
#include <windows.h>
#endif

#define FNAME_SIZE          48

struct FileTableEntry
{
	char fname[ FNAME_SIZE ];
	volatile uint8 cached;
	uint8 subdirID;
};

enum FileCacheStatus
{
	fcsNotCached,
		fcsCantCache,
		fcsCaching,
		fcsCached,
};

struct SubDir
{
	char subdir[ FNAME_SIZE ] ;
};

class DVDCache
{
private: 
	FileTableEntry *FileCacheTable;
	SubDir *FileCacheSubdirTable;
	int numCacheSubdirs;
	int numCacheableFiles;
	
	volatile bool cacheActive;
	volatile HANDLE DVDCacheHandle;
	volatile bool CacheWorking;
	volatile DWORD  TotalPauseTime;
	long   CurrentFileID;
	long   FilesCached;
	DWORD  CurrentFileSize;
	DWORD  SrcSectorSize;
	DWORD  DestSectorSize;
	
	bool IsFileCached(const char *filename);
	static void __cdecl HandleDVDCache(void*); // a static function for the thread
	void HandleDVDCacheHelper(); // but this is the one that does all the work
	void TestCacheFile(const char *);
	void UpdateCacheFile();
	bool VerifyCacheFile(long FileID);
	void CacheFile(const char *filename);
	void CacheFile(const int file_to_cache);
	
public:
	
	DVDCache();
	
	void PauseDVDCache();
	void CacheLoadedFile(char *filename, void *data, long size);
	void StartCaching();
	void StopCaching();
	void InitDVDCache();
	bool finished();  // returns true if all the files have been cached
};

extern DVDCache gDVDCache;

#endif 