#ifndef NGL_FILEIO_H
#define NGL_FILEIO_H

// this file declares a compatibility library for host file io on gamecube
extern "C" {

enum
{
	NGL_FIO_CREATE = 0x0001,
	NGL_FIO_READ   = 0x0002,
	NGL_FIO_WRITE  = 0x0004
};

enum
{
	NGL_FIO_SEEK_SET,
	NGL_FIO_SEEK_CUR,
	NGL_FIO_SEEK_END
};

int  GCopen (const char* file, unsigned int mode);
int  GCwrite(int handle, const void* buffer, unsigned int count);
int  GCread (int handle, void* buffer, unsigned int count);
int  GCseek (int handle, long offset, int origin);
void GCclose(int handle);

};

#endif // _FILEIO_H_

