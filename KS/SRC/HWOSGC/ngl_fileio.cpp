// this file declares a compatibility library for host file io on gamecube

#include "ngl_fileio.h"

extern "C" {

#if defined(NGL_GC_SN_DEBUGGER)

//////////////////////////////////////////////////////////////////////////
//
// SN Systems ProDG file i/o
//
//////////////////////////////////////////////////////////////////////////

#include <libsn.h>
#include <string.h>

static int _file_init = 0;

int GCopen(const char* file, unsigned int mode)
{
	if (!_file_init)
	{
		_file_init = 1;
		PCinit();
	}

	unsigned int handle = -1;
	char filename[256];

	// gets rid of const problem
	strcpy(filename, file);

	if (mode&NGL_FIO_CREATE)
	{
		handle = PCcreat(filename, 0);
	}
	else
	{
		// always open read/write
		handle = PCopen(filename, 2, 0);
	}

	return handle;
}

int GCwrite(int handle, const void* buffer, unsigned int count)
{
	return PCwrite(handle, (char*)buffer, count);
}
int GCread(int handle, void* buffer, unsigned int count)
{
	return PCread(handle, (char*)buffer, count);
}

int GCseek(int handle, long offset, int origin)
{
	return PClseek(handle, offset, origin);
}

void GCclose(int handle)
{
	PCclose(handle);
}

#elif defined(NGL_GC_MW_DEBUGGER)

//////////////////////////////////////////////////////////////////////////
//
// Metrowerks CodeWarrior file i/o
//
//////////////////////////////////////////////////////////////////////////

#include <stdio.h>

#define MAX_FILE_DESC 4

static FILE* _file_desc[MAX_FILE_DESC];

static int _store_fp(FILE* fp)
{
	for (int i=0; i<MAX_FILE_DESC; i++)
	{
		if (!_file_desc[i])
		{
			_file_desc[i] = fp;
			return i;
		}
	}

	return -1;
}

static void _clear_fp(FILE* fp)
{
	for (int i=0; i<MAX_FILE_DESC; i++)
		if (_file_desc[i] == fp)
			_file_desc[i] = NULL;
}

int GCopen(const char* file, unsigned int mode)
{
	char open_mode[3] = "rb";

	// set the mode string for fopen
	if (mode&NGL_FIO_READ)
		open_mode[0] = 'r';
	if (mode&NGL_FIO_WRITE)
		open_mode[0] = 'w';

	FILE* fp = fopen(file, open_mode);
	if (fp)
	{
		int fd = _store_fp(fp);
		if (fd >= 0)
			return fd;
	}

	return -1;
}

int GCwrite(int handle, const void* buffer, unsigned int count)
{
	FILE* fp = _file_desc[handle];
	return fwrite(buffer, count, 1, fp) * count;
}
int GCread(int handle, void* buffer, unsigned int count)
{
	FILE* fp = _file_desc[handle];
	return fread(buffer, count, 1, fp) * count;
}

int GCseek(int handle, long offset, int origin)
{
	FILE* fp = _file_desc[handle];
	int forigin = 0;

	switch (origin)
	{
	case NGL_FIO_SEEK_SET:	origin = SEEK_SET; break;
	case NGL_FIO_SEEK_CUR:	origin = SEEK_CUR; break;
	case NGL_FIO_SEEK_END:	origin = SEEK_END; break;
	}

	return fseek(fp, offset, origin);
}

void GCclose(int handle)
{
	FILE* fp = _file_desc[handle];
	_clear_fp(fp);
	fclose(fp);
}

#else

//////////////////////////////////////////////////////////////////////////
//
// Stub functions for undefined debuggers
//
//////////////////////////////////////////////////////////////////////////

int GCopen(const char* file, unsigned int mode)
{
	asm( trap );
	return 0;
}

int GCwrite(int handle, const void* buffer, unsigned int count)
{
	asm( trap );
	return 0;
}
int GCread(int handle, const void* buffer, unsigned int count)
{
	asm( trap );
	return 0;
}

int GCseek(int handle, long offset, int origin)
{
	asm( trap );
	return 0;
}

void GCclose(int handle)
{
	asm( trap );
}

#endif

};

