
#ifndef INCLUDED_DXT1_IMAGEDXT1_H
#define INCLUDED_DXT1_IMAGEDXT1_H

#include "global.h"
#include "dxt1_codebook.h"

class DXT1Color
{
public:
	DXT1Color() {;}
	DXT1Color(unsigned int c) : Col(c) {;}
	DXT1Color(unsigned char R, unsigned char G, unsigned char B, unsigned char A) : a(A), r(R), g(G), b(B) {;}

	union {
		struct { unsigned char r, g, b, a; };
		unsigned int	Col;
	};
};

struct DXTColBlock
{
	unsigned short col0;
	unsigned short col1;

	// no bit fields - use bytes
	unsigned char row[4];
};

struct DXTColor565
{
	unsigned int nBlue  : 5;		// order of names changes
	unsigned int nGreen : 6;		//  byte order of output to 32 bit
	unsigned int nRed	: 5;
};

class DXTColor8888
{
public:
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;

public:
	DXTColor8888 & operator=(const unsigned short c)
	{
		r = (c & 0xf800) >> 11;
		g = (c & 0x07e0) >> 5;
		b = (c & 0x001f);
		a = 0xff;
		
		return *this;
	}
};

class ImageDXTC
{
private:
	static const unsigned int Mask1565;
	static const unsigned int Mask0565;
	static const unsigned short ColorBits4[4];
	static const unsigned short ColorBits3[3];

private:
	int			XSize, YSize;
	unsigned char	AlphaValue;

private:
	
	void Emit1ColorBlock(unsigned short *pDest, DXT1Color c);
	void Emit2ColorBlock(unsigned short *pDest, DXT1Color c1, DXT1Color c2, DXT1Color *pSrc);
	void EmitMultiColorBlock3(unsigned short *pDest, CodeBook &cb, DXT1Color *pSrc);
	void EmitMultiColorBlock4(unsigned short *pDest, CodeBook &cb, DXT1Color *pSrc);
	inline unsigned short Make565(const DXT1Color & Col) const
	{
		return ((unsigned short)(Col.r >> 3) << 11) | ((unsigned short)(Col.g >> 2) << 5) | ((unsigned short)(Col.b >> 3));
	}
	/*
	inline unsigned short Make565(const DXT1Color & Col) const
	{
		return ((unsigned short)(Col.r >> 3) << 11) | ((unsigned short)(Col.g >> 2) << 5) | ((unsigned short)(Col.b >> 3));
	}
	*/

	void GetColorBlockColors(DXTColBlock * pBlock, DXTColor8888 * col_0, DXTColor8888 * col_1, DXTColor8888 * col_2, DXTColor8888 * col_3, unsigned short & wrd);
	void DecodeColorBlock(unsigned int * pImPos, DXTColBlock * pColorBlock, int width, unsigned int * col_0, unsigned int * col_1, unsigned int * col_2, unsigned int * col_3);

public:
	ImageDXTC();
	~ImageDXTC();

	void ReleaseAll(void);
	void SetSize(int x, int y);
	void CompressDXT1(DXT1Color * imagePixels, const int imageWidth, const int imageHeight, unsigned short * destBlocks);
	void DecompressDXT1(unsigned int * m_pDecompBytes, unsigned short * m_pCompBytes, int imageWidth, int imageHeight);

	int GetXSize(void) const { return XSize; }
	int GetYSize(void) const { return YSize; }
};

#endif INCLUDED_DXT1_IMAGEDXT1_H
