//
// ate.h
//   ATE file format
//
#ifndef _ate_h
#define _ate_h

#include "ngl_fixedstr.h"
typedef nglFixedString atestring;

#ifdef _MSC_VER	//	Set structure alignment packing value to 1 byte
	#pragma pack( push, 1 )
#endif

#ifdef _SONY
	typedef unsigned int   ATE_UINT32;
	typedef unsigned short ATE_UINT16;
	#define ate_file          int
	#define ate_filevalid(fp) (fp >= 0)
	#define ate_fopenr(n)     sceOpen(n,SCE_RDONLY)
	#define ate_fclose(fp)    sceClose(fp)
	#define ate_fseek(fp,i)   sceSeek(fp,i,SEEK_SET)
	#define ate_fread(buf,siz,fp) sceRead(fp,buf,siz)
#else
	typedef	unsigned int   ATE_UINT32;
	typedef	unsigned short ATE_UINT16;
	#define ate_filevalid(fp) (fp!=NULL)
	#define ate_file          FILE *
	#define ate_fopenr(n)     fopen(n,"rb")
	#define ate_fclose(fp)    fclose(fp)
	#define ate_fseek(fp,i)   fseek(fp,i,SEEK_SET)
	#define ate_fread(buf,siz,fp) fread(buf,siz,1,fp)
#endif

#ifndef _MAGIC_H

	typedef ATE_UINT32 Magic;

	inline Magic MAGIC( char s[] )
	{
		return ((Magic) s[3] << 24) | ((Magic) s[2] << 16) | ((Magic) s[1] << 8) | (Magic) s[0];
	}

#define _MAGIC_H

#endif
#if 0
#define ASSTR(a) ((const char *) a)
#else
#define ASSTR(a) (a.c_str())
#endif


#define ATEMAGIC MAGIC("ATEX")

#define VERSTAMP(major,minor) ((major << 16) + minor)

#define ATEVERMINOR  60
#define ATEVERMAJOR  0


struct ATEFileHeaderV0_60
{
	Magic  magic;
	ATE_UINT16 vermajor;
	ATE_UINT16 verminor;
	ATE_UINT32 items;
	ATE_UINT32 pad;
};

struct ATEFileEntryV0_60
{
	atestring     name;
	ATE_UINT32  hoff;
	ATE_UINT32  ioff;
	ATE_UINT32  poff;
	ATE_UINT32  pad;
};

typedef ATEFileHeaderV0_60 ATEFileHeader;
typedef ATEFileEntryV0_60 ATEFileEntry;

#ifdef _MSC_VER	//	Set structure alignment packing value to 1 byte
	#pragma pack( pop )
#endif

// Verify ATE File
bool ATEHeaderValid( char *atefile );


// by element routines
// Count textures
ATE_UINT32 ATEEntryCount( char *atefile );

// Get name of Nth texture
atestring &ATEEntryName( char *atefile, int i );

// Get header of texture by full name
char *ATEEntryHeader( char *atefile,const atestring & texentry );

// Get image of texture by full name
char *ATEEntryImage( char *atefile,const atestring & texentry );

// Get CLUT of texture by full name
char *ATEEntryPalette( char *atefile,const atestring & texentry );



// by element in animation routines
// Count animated textures
ATE_UINT32 ATETextureCount( char *atefile,const atestring & texname );

// Get name of Nth texture in an animation
atestring &ATETextureName( char *atefile,const atestring & texname, int i );

// Get header of Nth texture in an animation
char *ATETextureHeader( char *atefile,const atestring & texname, int i );

// Get image of Nth texture in an animation
char *ATETextureImage( char *atefile,const atestring & texname, int i );

// Get CLUT of Nth texture in an animation
char *ATETexturePalette( char *atefile,const atestring & texname, int i );



#endif

