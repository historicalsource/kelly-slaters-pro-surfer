
#ifndef _tim2_h
#define _tim2_h

//----------------------------------------------------------------------------------------
//  @TIM2 structures.
//----------------------------------------------------------------------------------------
static const int psmtbl[] =
{
	0,
	1,
	2,
	3,
	4,
	5
};

typedef unsigned long  TIM2_UINT64;
typedef unsigned char  TIM2_UCHAR8;
typedef unsigned int   TIM2_UINT32;
typedef unsigned short TIM2_UINT16;

#pragma pack(push,1)

// constant definition for ClutType, ImageType in picture header
enum TIM2_gattr_type {
	TIM2_NONE = 0,          // no CLUT (for ClutType)
	TIM2_RGB16,             // 16 bit color (for both of ClutType, ImageType)
	TIM2_RGB24,             // 24 bit color (for ImageType)
	TIM2_RGB32,             // 32 bit color (for ClutType, ImageType)
	TIM2_IDTEX4,            // 16 color texture (for ImageType)
	TIM2_IDTEX8             // 256 color texture (for ImageType)
};

// TIM2 file header
typedef struct {
	TIM2_UCHAR8 FileId[4];              // file ID ('T','I','M','2' or 'C','L','T','2')
	TIM2_UCHAR8 FormatVersion;          // version of file format
	TIM2_UCHAR8 FormatId;               // ID of format
	TIM2_UINT16 Pictures;               // number of picture data
	TIM2_UCHAR8 pad[8];                 // for alignment
} TIM2_FILEHEADER;


// TIM2 picture header
struct TIM2_PICTUREHEADER {
	TIM2_UINT32 TotalSize;              // total size of the picture data in bytes
	TIM2_UINT32 ClutSize;               // CLUT data size in bytes
	TIM2_UINT32 ImageSize;              // image data size in bytes
	TIM2_UINT16 HeaderSize;             // amount of headers
	TIM2_UINT16 ClutColors;             // colors in CLUT
	TIM2_UCHAR8 PictFormat;             // picture format
	TIM2_UCHAR8 MipMapTextures;         // number of MIPMAP texture
	TIM2_UCHAR8 ClutType;               // CLUT type
	TIM2_UCHAR8 ImageType;              // image type
	TIM2_UINT16 ImageWidth;             // width of image (not in bits)
	TIM2_UINT16 ImageHeight;            // height of image (not in bits)

	TIM2_UINT64 GsTex0;                 // TEX0
	TIM2_UINT64 GsTex1;                 // TEX1
	TIM2_UINT32 GsTexaFbaPabe;          // bitfield of TEXA, FBA and PABE
	TIM2_UINT32 GsTexClut;              // TEXCLUT (lower 32 bits)
};


// TIM2 MIPMAP header
typedef struct {
	TIM2_UINT64 GsMiptbp1;              // MIPTBP1 (actual 64 bit image)
	TIM2_UINT64 GsMiptbp2;              // MIPTBP2 (actual 64 bit image)
	TIM2_UINT32 MMImageSize[0];         // image size of N-th MIPMAP texture in bytes
} TIM2_MIPMAPHEADER;

#pragma pack(pop)


#endif
