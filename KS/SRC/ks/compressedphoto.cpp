
#include "global.h"
#include "compressedphoto.h"
#include "dxt1_imagedxt1.h"

#ifdef TARGET_XBOX
// This is defined in ngl_xbox.cpp and we need it for creating a compressed texture
extern IDirect3DDevice8* nglDev;
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#define FAILED(Status) ((HRESULT)(Status)<0)
#endif

//	CompressedPhoto()
// Default constructor.
CompressedPhoto::CompressedPhoto()
{
	Reset ();
}

//	~CompressedPhoto()
// Destructor.
CompressedPhoto::~CompressedPhoto()
{
}

void CompressedPhoto::Reset ()
{
	size = (PHOTO_WIDTH*PHOTO_HEIGHT*4)/8;
	valid = false;
}

//	CopyFromTexture()
// Copies the pixels from the specified texture.
// The texture must be 32-bit and be of size PHOTO_WIDTHxPHOTO_HEIGHT.
void CompressedPhoto::CopyFromTexture(nglTexture * texture)
{
	valid = true;

#ifdef TARGET_XBOX
	assert(nglDev != NULL);

	LPDIRECT3DTEXTURE8 pSrcTexture = texture->DXTexture.Simple;
	LPDIRECT3DSURFACE8 pDxt1GameImageSurface, pSrcSurface;
	D3DLOCKED_RECT lr;
	HRESULT hr;

	if (pSrcTexture == NULL)
		return;

	// Generate a DXT1 surface
	hr = nglDev->CreateImageSurface (PHOTO_WIDTH, PHOTO_HEIGHT, D3DFMT_DXT1, &pDxt1GameImageSurface);
	if (FAILED (hr))
		return;

	// Copy the source into the DXT1 surface
	hr = pSrcTexture->GetSurfaceLevel (0, &pSrcSurface);
	hr = D3DXLoadSurfaceFromSurface (pDxt1GameImageSurface, NULL, NULL, pSrcSurface, NULL, NULL, D3DX_DEFAULT, D3DCOLOR (0));
	SAFE_RELEASE (pSrcSurface);

	if (FAILED (hr))
	{
		SAFE_RELEASE (pDxt1GameImageSurface);
		return;
	}

	pDxt1GameImageSurface->LockRect (&lr, NULL, D3DLOCK_READONLY);
	memcpy (blocks, lr.pBits, size);
	pDxt1GameImageSurface->UnlockRect();

	SAFE_RELEASE (pDxt1GameImageSurface);
#endif // TARGET_XBOX


#if defined(TARGET_GC)
	ImageDXTC	dxt1;

	//Unswizzle texture
	u32 *unswizzled = (u32 *) malloc( PHOTO_WIDTH * PHOTO_HEIGHT * 4 );
	u8 *swizzled = (u8 *)texture->ImageData;
	u8 *ar_ptr, *gb_ptr;
	int tile_pitch = PHOTO_WIDTH / 4;

	for( int tile_y = 0; tile_y < PHOTO_HEIGHT / 4; tile_y++ )
	{
		for( int tile_x = 0; tile_x < PHOTO_WIDTH / 4; tile_x++ )
		{
			ar_ptr = swizzled + ( ( tile_y * ( PHOTO_WIDTH / 4 ) * 64 ) + tile_x * 64 );
			gb_ptr = ar_ptr + 32;

			for( int ofs_y = 0; ofs_y < 4; ofs_y++ )
				for( int ofs_x = 0; ofs_x < 4; ofs_x++ )
				{
					u32 pixel = ar_ptr[1] << 24 | gb_ptr[0] << 16 | gb_ptr[1] << 8 | ar_ptr[0];
					unswizzled[ ( ( tile_y * 4 + ofs_y ) * PHOTO_WIDTH ) + ( tile_x * 4 ) + ofs_x ] = pixel;
					ar_ptr += 2;
					gb_ptr += 2;
				}
		}
	}
	FILE *fp;
	dxt1.CompressDXT1( ( DXT1Color *) unswizzled, PHOTO_WIDTH, PHOTO_HEIGHT, (unsigned short *) blocks );
	free( unswizzled );
#elif defined(TARGET_PS2)
	ImageDXTC	dxt1;
	STOP_PS2_PC;	// Turn off profiling counters to avoid performance count exception. (dc 07/09/02)
	dxt1.CompressDXT1((DXT1Color *) texture->Data, PHOTO_WIDTH, PHOTO_HEIGHT, (unsigned short *) blocks);
	START_PS2_PC;
#endif// TARGET_GC
}

//	CopyToTexture()
// Copies this photo's pixels to the specified texture.
// The texture must be 32-bit and be of size PHOTO_WIDTHxPHOTO_HEIGHT.
void CompressedPhoto::CopyToTexture(nglTexture * texture) const
{
#ifdef TARGET_XBOX
	LPDIRECT3DTEXTURE8 pDstTexture = texture->DXTexture.Simple;
	LPDIRECT3DSURFACE8 pDxt1GameImageSurface, pDstSurface;
	D3DLOCKED_RECT lr;
	HRESULT hr;

	if (pDstTexture == NULL)
		return;

	// Generate a DXT1 surface
	hr = nglDev->CreateImageSurface (PHOTO_WIDTH, PHOTO_HEIGHT, D3DFMT_DXT1, &pDxt1GameImageSurface);
	if (FAILED (hr))
		return;

	// Copy the data into the DXT1 surface
	pDxt1GameImageSurface->LockRect (&lr, NULL, D3DLOCK_READONLY);
	memcpy (lr.pBits, blocks, size);
	pDxt1GameImageSurface->UnlockRect();

	// Copy from the DXT1 surface to the nglTexture
	hr = pDstTexture->GetSurfaceLevel (0, &pDstSurface);
	hr = D3DXLoadSurfaceFromSurface (pDstSurface, NULL, NULL, pDxt1GameImageSurface, NULL, NULL, D3DX_DEFAULT, D3DCOLOR (0));

	SAFE_RELEASE (pDstSurface);
	SAFE_RELEASE (pDxt1GameImageSurface);
#endif // TARGET_XBOX

#ifdef TARGET_GC
	ImageDXTC	dxt1;
	//Swizzle texture
	u32 *unswizzled = (u32 *) malloc( PHOTO_WIDTH * PHOTO_HEIGHT * 4 );
	u8 *swizzled = (u8 *)texture->ImageData;
	u8 *ar_ptr, *gb_ptr;
	int tile_pitch = PHOTO_WIDTH / 4;

	dxt1.DecompressDXT1((unsigned int *) unswizzled, (unsigned short *) blocks, PHOTO_WIDTH, PHOTO_HEIGHT);
	
	for( int tile_y = 0; tile_y < PHOTO_HEIGHT / 4; tile_y++ )
	{
		for( int tile_x = 0; tile_x < PHOTO_WIDTH / 4; tile_x++ )
		{
			ar_ptr = swizzled + ( ( tile_y * ( PHOTO_WIDTH / 4 ) * 64 ) + tile_x * 64 );
			gb_ptr = ar_ptr + 32;

			for( int ofs_y = 0; ofs_y < 4; ofs_y++ )
				for( int ofs_x = 0; ofs_x < 4; ofs_x++ )
				{
					u32 pixel = unswizzled[ ( ( tile_y * 4 + ofs_y ) * PHOTO_WIDTH ) + ( tile_x * 4 ) + ofs_x ];
					ar_ptr[0] = 0xff /*& ( pixel >> 0 )*/;
					ar_ptr[1] = 0xff & ( pixel >> 24 );
					gb_ptr[0] = 0xff & ( pixel >> 16 );
					gb_ptr[1] = 0xff & ( pixel >> 8 );
					
					ar_ptr += 2;
					gb_ptr += 2;
				}
		}
	}
	DCStoreRange( swizzled, PHOTO_WIDTH * PHOTO_HEIGHT * 4 );
	GXInvalidateTexAll();
	free( unswizzled );
#elif defined(TARGET_PS2)
	ImageDXTC	dxt1;
	dxt1.DecompressDXT1((unsigned int *) texture->Data, (unsigned short *) blocks, PHOTO_WIDTH, PHOTO_HEIGHT);
#endif
}

//	ExportToDDS()
// Debug function to save compressed photo as a .dds file.
void CompressedPhoto::ExportToDDS(const char * filename)
{
#ifdef TARGET_PS2

	int				fd = sceOpen((char*) filename, SCE_WRONLY | SCE_TRUNC | SCE_CREAT);
	char			label[5] = "DDS ";
	unsigned int	header[31];
	
	// Write file label.
	sceWrite(fd, &label, 4);
	
    // Write file header.
	memset(&header, 0, 124);
	header[0] = 124;			// size
	header[1] = 528391;			// flags
	header[2] = PHOTO_WIDTH;	// width
	header[3] = PHOTO_HEIGHT;	// height
	header[4] = size;			// linear size
	header[18] = 32;			// pixel format size
	header[19] = 4;				// pixel format flags
	header[20] = 827611204;		// pixel format FourCC
	header[26] = 4096;			// caps
	sceWrite(fd, header, 124);
	
	// Write compressed image.
	sceWrite(fd, blocks, (PHOTO_WIDTH*PHOTO_HEIGHT*4)/8);
    
	sceClose(fd);
#endif
}
