#include "ngl_dds.h"

extern LPDIRECT3DDEVICE8 nglDev;

// For some reasons (Micro$oft SUCKS !!!), the D3DFORMAT enumerated type is different
// from PC DX8 and XBox !?$%#@!

enum
{
	PC_D3DFMT_UNKNOWN = 0,

	PC_D3DFMT_R8G8B8 = 20,
	PC_D3DFMT_A8R8G8B8 = 21,
	PC_D3DFMT_X8R8G8B8 = 22,
	PC_D3DFMT_R5G6B5 = 23,
	PC_D3DFMT_X1R5G5B5 = 24,
	PC_D3DFMT_A1R5G5B5 = 25,
	PC_D3DFMT_A4R4G4B4 = 26,
	PC_D3DFMT_R3G3B2 = 27,
	PC_D3DFMT_A8 = 28,
	PC_D3DFMT_A8R3G3B2 = 29,
	PC_D3DFMT_X4R4G4B4 = 30,

	PC_D3DFMT_A8P8 = 40,
	PC_D3DFMT_P8 = 41,

	PC_D3DFMT_L8 = 50,
	PC_D3DFMT_A8L8 = 51,
	PC_D3DFMT_A4L4 = 52,

	PC_D3DFMT_V8U8 = 60,
	PC_D3DFMT_L6V5U5 = 61,
	PC_D3DFMT_X8L8V8U8 = 62,
	PC_D3DFMT_Q8W8V8U8 = 63,
	PC_D3DFMT_V16U16 = 64,
	PC_D3DFMT_W11V11U10 = 65,

	PC_D3DFMT_UYVY = MAKEFOURCC('U', 'Y', 'V', 'Y'),
	PC_D3DFMT_YUY2 = MAKEFOURCC('Y', 'U', 'Y', '2'),
	PC_D3DFMT_DXT1 = MAKEFOURCC('D', 'X', 'T', '1'),
	PC_D3DFMT_DXT2 = MAKEFOURCC('D', 'X', 'T', '2'),
	PC_D3DFMT_DXT3 = MAKEFOURCC('D', 'X', 'T', '3'),
	PC_D3DFMT_DXT4 = MAKEFOURCC('D', 'X', 'T', '4'),
	PC_D3DFMT_DXT5 = MAKEFOURCC('D', 'X', 'T', '5'),

	PC_D3DFMT_D16_LOCKABLE = 70,
	PC_D3DFMT_D32 = 71,
	PC_D3DFMT_D15S1 = 73,
	PC_D3DFMT_D24S8 = 75,
	PC_D3DFMT_D16 = 80,
	PC_D3DFMT_D24X8 = 77,
	PC_D3DFMT_D24X4S4 = 79,


	PC_D3DFMT_VERTEXDATA = 100,
	PC_D3DFMT_INDEX16 = 101,
	PC_D3DFMT_INDEX32 = 102,

	PC_D3DFMT_FORCE_DWORD = 0x7fffffff
};

/*-----------------------------------------------------------------------------
Description: Load a DDS texture file (simple/cubic texture).
Note:        Volume texture loading needs to be updated.
-----------------------------------------------------------------------------*/
HRESULT nglLoadAllMipSurfaces(LPDIRECT3DBASETEXTURE8 ptex, D3DCUBEMAP_FACES FaceType,
                              uint8 *&Data, uint32 NumMips, uint32 SrcPitch);

HRESULT nglLoadAllVolumeSurfaces(LPDIRECT3DVOLUMETEXTURE8 pvoltex, uint8 *Data, uint32 NumMips);

bool nglLoadDDS(LPDIRECT3DBASETEXTURE8 *BaseTexture, uint8 *Data)
{
	HRESULT hr;
	DDS_HEADER ddsh;
	LPDIRECT3DTEXTURE8 pmiptex = NULL;
	LPDIRECT3DCUBETEXTURE8 pcubetex = NULL;
	LPDIRECT3DVOLUMETEXTURE8 pvoltex = NULL;
	uint32 Magic;
	uint32 Width;
	uint32 Height;
	uint32 CubeMapFlags;
	uint32 Depth;
	uint32 NumMips;

	*BaseTexture = NULL;

	Magic = *((uint32 *) Data);
	Data += sizeof(Magic);

	if (Magic != MAKEFOURCC('D', 'D', 'S', ' '))
		return false;

	ddsh = *((DDS_HEADER *) Data);
	Data += sizeof(DDS_HEADER);

	if (ddsh.dwSize != sizeof(ddsh))
		return false;

	Width = ddsh.dwWidth;
	Height = ddsh.dwHeight;
	NumMips = ddsh.dwMipMapCount;

	if (NumMips == 0)
		NumMips = 1;

	CubeMapFlags = (ddsh.dwCubemapFlags & DDS_CUBEMAP_ALLFACES);

	if (ddsh.dwHeaderFlags & DDS_HEADER_FLAGS_VOLUME)
		Depth = ddsh.dwDepth;
	else
		Depth = 0;

	bool IsVolumeMap = (Depth > 0);

	D3DFORMAT fmt;

	if (!IsVolumeMap && ddsh.ddspf.dwFourCC == PC_D3DFMT_DXT1)
		fmt = D3DFMT_DXT1;
	else if (!IsVolumeMap && ddsh.ddspf.dwFourCC == PC_D3DFMT_DXT2)
		fmt = D3DFMT_DXT2;
	else if (!IsVolumeMap && ddsh.ddspf.dwFourCC == PC_D3DFMT_DXT3)
		fmt = D3DFMT_DXT3;
	else if (!IsVolumeMap && ddsh.ddspf.dwFourCC == PC_D3DFMT_DXT4)
		fmt = D3DFMT_DXT4;
	else if (!IsVolumeMap && ddsh.ddspf.dwFourCC == PC_D3DFMT_DXT5)
		fmt = D3DFMT_DXT5;
	else if (ddsh.ddspf.dwFlags == DDS_RGBA && ddsh.ddspf.dwRGBBitCount == 32
			 && ddsh.ddspf.dwABitMask == 0xff000000)
		fmt = D3DFMT_A8R8G8B8;
	else if (ddsh.ddspf.dwFlags == DDS_RGB && ddsh.ddspf.dwRGBBitCount == 16
			 && ddsh.ddspf.dwGBitMask == 0x000007e0)
		fmt = D3DFMT_R5G6B5;
	else if (ddsh.ddspf.dwFlags == DDS_RGBA && ddsh.ddspf.dwRGBBitCount == 16
			 && ddsh.ddspf.dwABitMask == 0x00008000)
		fmt = D3DFMT_A1R5G5B5;
	else if (ddsh.ddspf.dwFlags == DDS_RGBA && ddsh.ddspf.dwRGBBitCount == 16
			 && ddsh.ddspf.dwABitMask == 0x0000f000)
		fmt = D3DFMT_A4R4G4B4;
	else
		return false;  // Unsupported format.

	// Volume texture ?
	if (IsVolumeMap)
	{
		if (FAILED(hr =nglDev->CreateVolumeTexture(Width, Height, Depth, NumMips, 0, fmt, 0, &pvoltex)))
			return false;

		if (FAILED(hr = nglLoadAllVolumeSurfaces(pvoltex, Data, NumMips)))
			return false;

		*BaseTexture = pvoltex;
	}
	// Cube texture ?
	else if (CubeMapFlags > 0)
	{
		if (FAILED(hr = nglDev->CreateCubeTexture(Width, NumMips, 0, fmt, 0, &pcubetex)))
			return false;

		// A DX8 a cube map texture contains the 6 faces (this is not true for previous DX versions).
		// Thus this loader is only DX8 or higher compatible.
		D3DCUBEMAP_FACES CubeFaces[6] = {
			D3DCUBEMAP_FACE_POSITIVE_X, D3DCUBEMAP_FACE_NEGATIVE_X,
			D3DCUBEMAP_FACE_POSITIVE_Y, D3DCUBEMAP_FACE_NEGATIVE_Y,
			D3DCUBEMAP_FACE_POSITIVE_Z, D3DCUBEMAP_FACE_NEGATIVE_Z
		};

		for (int32 i = 0; i < 6; i++)
		{
			if (FAILED(hr = nglLoadAllMipSurfaces(pcubetex, CubeFaces[i], Data, NumMips, ddsh.dwPitchOrLinearSize)))
				return false;
		}

		*BaseTexture = pcubetex;
	}
	// "Simple" texture.
	else
	{
		if (FAILED(hr = nglDev->CreateTexture(Width, Height, NumMips, 0, fmt, 0, &pmiptex)))
			return false;

		if (FAILED(hr = nglLoadAllMipSurfaces(pmiptex, D3DCUBEMAP_FACE_FORCE_DWORD, Data, NumMips, ddsh.dwPitchOrLinearSize)))
			return false;

		*BaseTexture = pmiptex;
	}

	return true;
}

/*-----------------------------------------------------------------------------
Description: Load all the mipmaps of a face.
-----------------------------------------------------------------------------*/
HRESULT nglLoadAllMipSurfaces(LPDIRECT3DBASETEXTURE8 ptex, D3DCUBEMAP_FACES FaceType, uint8 *&Data,
                              uint32 NumMips, uint32 SrcPitch)
{
	HRESULT hr;
	LPDIRECT3DSURFACE8 psurf;
	D3DSURFACE_DESC sd;
	D3DLOCKED_RECT lr;
	LPDIRECT3DTEXTURE8 pmiptex = NULL;
	LPDIRECT3DCUBETEXTURE8 pcubetex = NULL;
	uint32 Level;
	uint32 Bpp;

	if (FaceType == D3DCUBEMAP_FACE_FORCE_DWORD)
		pmiptex = (LPDIRECT3DTEXTURE8) ptex;
	else
		pcubetex = (LPDIRECT3DCUBETEXTURE8) ptex;

	for (Level = 0; Level < NumMips; Level++)
	{
		if (pmiptex != NULL)
			hr = pmiptex->GetSurfaceLevel(Level, &psurf);
		else
			hr = pcubetex->GetCubeMapSurface(FaceType, Level, &psurf);

		if (FAILED(hr))
		{
			return hr;
		}

		psurf->GetDesc(&sd);

		switch (sd.Format)
		{
				// Compressed formats.
			case D3DFMT_DXT1:
			case D3DFMT_DXT2:	// On the XBox, DXT2 and DXT3 have the same flag value.
			case D3DFMT_DXT4:	// On the XBox, DXT4 and DXT5 have the same flag value.
				Bpp = 0;		// Magic value indicates texture's memory is contiguous.
				break;
				// Uncompressed 32-bits formats.
			case D3DFMT_A8R8G8B8:
				Bpp = 4;
				break;
				// Uncompressed 16-bits formats.
			case D3DFMT_A1R5G5B5:
			case D3DFMT_A4R4G4B4:
			case D3DFMT_R5G6B5:
				Bpp = 2;
				break;
				// Unknown format.
			default:
				return E_FAIL;
		}

		if (pmiptex != NULL)
			hr = pmiptex->LockRect(Level, &lr, NULL, 0);
		else
			hr = pcubetex->LockRect(FaceType, Level, &lr, NULL, 0);

		if (FAILED(hr))
		{
			return hr;
		}

		// DXT1 to DXT5 formats.
		if (Bpp == 0)
		{
			memcpy(lr.pBits, Data, sd.Size);
			Data += sd.Size;
		}
		// Uncompressed formats.
		else
		{
			XGSwizzleRect(Data, NULL, NULL, lr.pBits, sd.Width, sd.Height,
						  NULL, Bpp);
			Data += sd.Size;
		}

		if (pmiptex != NULL)
			hr = pmiptex->UnlockRect(Level);
		else
			hr = pcubetex->UnlockRect(FaceType, Level);

		if (psurf != NULL)
		{
			psurf->Release();
			psurf = NULL;
		}
	}

	return S_OK;
}

/*-----------------------------------------------------------------------------
Description: Load the volume faces.
-----------------------------------------------------------------------------*/
HRESULT nglLoadAllVolumeSurfaces(LPDIRECT3DVOLUMETEXTURE8 pvoltex,
								 uint8 * Data, uint32 NumMips)
{
	HRESULT hr;
	D3DVOLUME_DESC vd;
	D3DBOX box;
	D3DLOCKED_BOX lb;
	uint8 *pbSlice;
	uint8 *pbRow;
	uint32 Level;
	uint32 numBytesPerRow;
	uint32 zp;
	uint32 yp;

	for (Level = 0; Level < NumMips; Level++)
	{
		pvoltex->GetLevelDesc(Level, &vd);
		box.Left = 0;
		box.Right = vd.Width;
		box.Top = 0;
		box.Bottom = vd.Height;
		box.Front = 0;
		box.Back = vd.Depth;
		hr = pvoltex->LockBox(Level, &lb, &box, 0);

		if (FAILED(hr))
			return hr;

		switch (vd.Format)
		{
			case D3DFMT_A8R8G8B8:
				numBytesPerRow = 4 * vd.Width;
				break;
			default:
				return E_FAIL;
		}

		pbSlice = (uint8 *) lb.pBits;

		// TODO:
		// This way of copying the texture is wrong. Need to swizzle the data.
		// (but for now we don't use volume textures...)
		for (zp = 0; zp < vd.Depth; zp++)
		{
			pbRow = pbSlice;
			for (yp = 0; yp < vd.Height; yp++)
			{
				memcpy(pbRow, Data, numBytesPerRow);
				Data += numBytesPerRow;
				pbRow += lb.RowPitch;
			}
			pbSlice += lb.SlicePitch;
		}

		pvoltex->UnlockBox(Level);
	}

	return S_OK;
}
