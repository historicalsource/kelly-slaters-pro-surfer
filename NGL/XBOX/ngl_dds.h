#ifndef NGL_DDS_H
#define NGL_DDS_H

#include <xtl.h>
#include <xgraphics.h>
#include "ngl_types.h"

bool nglLoadDDS(LPDIRECT3DBASETEXTURE8 *BaseTexture, uint8 *Data);

/*-----------------------------------------------------------------------------
Description: DDS header and types definition.
-----------------------------------------------------------------------------*/

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3) \
((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) | \
((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
#endif

struct DDS_PIXELFORMAT
{
	DWORD dwSize;
	DWORD dwFlags;
	DWORD dwFourCC;
	DWORD dwRGBBitCount;
	DWORD dwRBitMask;
	DWORD dwGBitMask;
	DWORD dwBBitMask;
	DWORD dwABitMask;
};

#define DDS_FOURCC 0x00000004  // DDPF_FOURCC
#define DDS_RGB    0x00000040  // DDPF_RGB
#define DDS_RGBA   0x00000041  // DDPF_RGB | DDPF_ALPHAPIXELS

#define DDS_HEADER_FLAGS_TEXTURE    0x00001007  // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT
#define DDS_HEADER_FLAGS_MIPMAP     0x00020000  // DDSD_MIPMAPCOUNT
#define DDS_HEADER_FLAGS_VOLUME     0x00800000  // DDSD_DEPTH
#define DDS_HEADER_FLAGS_PITCH      0x00000008  // DDSD_PITCH
#define DDS_HEADER_FLAGS_LINEARSIZE 0x00080000  // DDSD_LINEARSIZE

#define DDS_SURFACE_FLAGS_TEXTURE 0x00001000    // DDSCAPS_TEXTURE
#define DDS_SURFACE_FLAGS_MIPMAP  0x00400008    // DDSCAPS_COMPLEX | DDSCAPS_MIPMAP
#define DDS_SURFACE_FLAGS_CUBEMAP 0x00000008    // DDSCAPS_COMPLEX

#define DDS_CUBEMAP_POSITIVEX 0x00000600        // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEX
#define DDS_CUBEMAP_NEGATIVEX 0x00000a00        // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEX
#define DDS_CUBEMAP_POSITIVEY 0x00001200        // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEY
#define DDS_CUBEMAP_NEGATIVEY 0x00002200        // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEY
#define DDS_CUBEMAP_POSITIVEZ 0x00004200        // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEZ
#define DDS_CUBEMAP_NEGATIVEZ 0x00008200        // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEZ

#define DDS_CUBEMAP_ALLFACES ( DDS_CUBEMAP_POSITIVEX | DDS_CUBEMAP_NEGATIVEX |\
                               DDS_CUBEMAP_POSITIVEY | DDS_CUBEMAP_NEGATIVEY |\
                               DDS_CUBEMAP_POSITIVEZ | DDS_CUBEMAP_NEGATIVEZ )

#define DDS_FLAGS_VOLUME 0x00200000  // DDSCAPS2_VOLUME


struct DDS_HEADER
{
	DWORD dwSize;
	DWORD dwHeaderFlags;
	DWORD dwHeight;
	DWORD dwWidth;
	DWORD dwPitchOrLinearSize;
	DWORD dwDepth;  // only if DDS_HEADER_FLAGS_VOLUME is set in dwHeaderFlags
	DWORD dwMipMapCount;
	DWORD dwReserved1[11];
	DDS_PIXELFORMAT ddspf;
	DWORD dwSurfaceFlags;
	DWORD dwCubemapFlags;
	DWORD dwReserved2[3];
};

#endif
