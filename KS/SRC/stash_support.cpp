/* stash_support.cpp
 *
 * support functions to prepare and fixup memory images of various types
 * these functions aren't included in stash.cpp to reduce it's header
 * dependency and show a clearer seperation of who is in charge of what
 * tasks.
 */
// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#include "stash_support.h"

#include <stdio.h>

#if defined(TARGET_XBOX)
#include "ngl.h" // for nglPrintf
#endif /* TARGET_XBOX JIV DEBUG */

/*
**  TGA File Header
*/
typedef struct _TgaHeader
{                           
    unsigned char   IdLength;            /* Image ID Field Length      */
    unsigned char   CmapType;            /* Color Map Type             */
    unsigned char   ImageType;           /* Image Type                 */
    /*
    ** Color Map Specification
    */
    unsigned char	CmapIndex[2];           /* First Entry Index          */
    unsigned char   CmapLength[2];          /* Color Map Length           */
    unsigned char   CmapEntrySize;       /* Color Map Entry Size       */
    /*
    ** Image Specification
    */
    unsigned short   X_Origin;            /* X-origin of Image          */
    unsigned short   Y_Origin;            /* Y-origin of Image          */
    unsigned short   ImageWidth;          /* Image Width                */
    unsigned short   ImageHeight;         /* Image Height               */
    unsigned char   PixelDepth;          /* Pixel Depth                */
    unsigned char   ImagDesc;            /* Image Descriptor           */
} TGAHEADER;


/*** prepare_tga_memory_image ***/
// checks and works over the data from a targa file to be like how we
// want it in memory for the game.
bool prepare_tga_memory_image(unsigned char *&file_data, unsigned &file_length, 
       unsigned &pal_offset, unsigned short &width, unsigned short &height,
       unsigned char &bit_depth, unsigned char &palettized_flag)
{
  TGAHEADER *hdr = (TGAHEADER*)file_data;
  unsigned char *data = NULL;
  unsigned data_size = 0;

  switch (hdr->ImageWidth) 
  {
	  case 256:  case 128:  case 64:  break;
    default:
  		nglPrintf( "Invalid texture width: %d\n", hdr->ImageWidth );
  		return false;
  }
  width = hdr->ImageWidth;

  switch (hdr->ImageHeight) 
  {
	  case 256:  case 128:  case 64:  case 32:  case 16:  case 8:  case 4:  case 2: break;
    default:
  		nglPrintf( "Invalid texture height: %d\n", hdr->ImageHeight );
  		return false;
  }
  height = hdr->ImageHeight;

  pal_offset = 0;
  palettized_flag = 0;

	if ( hdr->CmapType )
	{
		nglPrintf( "Palettized TGAs not supported, yet.\n" );
		return false;
	}
	else
	{
		if ( hdr->ImageType != 2 )
		{
			nglPrintf( "Invalid TGA Image Type: %d\n", hdr->ImageType );
			return false;
		}

		if ( hdr->PixelDepth == 16 )
		{
			nglPrintf( "16-bit TGAs not supported.\n");
      return false;
		}
    if ( hdr->PixelDepth == 24 )
		{
			data_size = width * height * 4;
			data = (unsigned char *)malloc( data_size );
      bit_depth = 24;

  		unsigned char* SrcData = (unsigned char*)hdr + sizeof(TGAHEADER);
			unsigned char* DestData = (unsigned char*)data;
			for ( int i = 0; i < width * height; i++ )
			{
				DestData[i * 4 + 0] = SrcData[i * 3 + 2];
				DestData[i * 4 + 1] = SrcData[i * 3 + 1];
				DestData[i * 4 + 2] = SrcData[i * 3 + 0];
				DestData[i * 4 + 3] = 255;
			}
		}
		if ( hdr->PixelDepth == 32 )
		{
			data_size = width * height * 4;
			data = (unsigned char *)malloc( data_size );
      bit_depth = 32;

  		unsigned char* SrcData = (unsigned char*)hdr + sizeof(TGAHEADER);
			unsigned char* DestData = (unsigned char*)data;
			for ( int i = 0; i < width * height; i++ )
			{
				DestData[i * 4 + 0] = SrcData[i * 4 + 2];
				DestData[i * 4 + 1] = SrcData[i * 4 + 1];
				DestData[i * 4 + 2] = SrcData[i * 4 + 0];
				DestData[i * 4 + 3] = SrcData[i * 4 + 3];
			}
		}		
	}

  file_data = data;
  free(hdr);
  file_length = data_size;

  return true;
}
