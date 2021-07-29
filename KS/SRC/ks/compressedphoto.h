
#ifndef INCLUDED_COMPRESSEDPHOTO_H
#define INCLUDED_COMPRESSEDPHOTO_H

#include "global.h"
#include "ngl.h"

// CompressedPhoto: compressed image easily saved to a file.
class CompressedPhoto
{
public:
	enum { PHOTO_WIDTH = 128 };
	enum { PHOTO_HEIGHT = 128 };	

public:
	// Creators.
	CompressedPhoto();
	~CompressedPhoto();

	// Modifiers.
	void CopyFromTexture(nglTexture * texture);
	void Reset ();

	// Accessors.
	void CopyToTexture(nglTexture * texture) const;
	void ExportToDDS(const char * filename);

	bool IsValid () const
		{ return valid; }

	CompressedPhoto& operator=(CompressedPhoto& photo)
	{
		memcpy (blocks, photo.blocks, size);
		valid = true;
		return *this;
	};

private:
	unsigned char	blocks[(PHOTO_WIDTH*PHOTO_HEIGHT*4)/8];
	int size;
	bool valid;
};

#endif INCLUDED_COMPRESSEDPHOTO_H