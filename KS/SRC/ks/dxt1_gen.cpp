
#include "global.h"
#include "dxt1_gen.h"

const int DXTCGen::ScanRange = 24;		// Uses a range of +/- ScanRange/2 for brute force
const int DXTCGen::ScanStep = 4;		// Step increment for the brute force scan


// ----------------------------------------------------------------------------
// Build palette for a 3 color + traparent black block
// ----------------------------------------------------------------------------
void DXTCGen::BuildCodes3(cbVector &v1, cbVector &v2)
{
	pVects[0] = v1;

	pVects[1][0] = v1[0];
	pVects[1][1] = (unsigned char)( ((int)v1[1] + (int)v2[1]) / 2 );
	pVects[1][2] = (unsigned char)( ((int)v1[2] + (int)v2[2]) / 2 );
	pVects[1][3] = (unsigned char)( ((int)v1[3] + (int)v2[3]) / 2 );
	
	pVects[2] = v2;
}

// ----------------------------------------------------------------------------
// Build palette for a 4 color block
// ----------------------------------------------------------------------------
void DXTCGen::BuildCodes4(cbVector &v1, cbVector &v2)
{
	pVects[0] = v1;
	pVects[3] = v2;

	pVects[1][0] = v1[0];
	pVects[1][1] = (unsigned char)( ((int)v1[1] * 2 + (int)v2[1]) / 3 );
	pVects[1][2] = (unsigned char)( ((int)v1[2] * 2 + (int)v2[2]) / 3 );
	pVects[1][3] = (unsigned char)( ((int)v1[3] * 2 + (int)v2[3]) / 3 );

	pVects[2][0] = v1[0];
	pVects[2][1] = (unsigned char)( ((int)v2[1] * 2 + (int)v1[1]) / 3 );
	pVects[2][2] = (unsigned char)( ((int)v2[2] * 2 + (int)v1[2]) / 3 );
	pVects[2][3] = (unsigned char)( ((int)v2[3] * 2 + (int)v1[3]) / 3 );
}


// ----------------------------------------------------------------------------
// Rebuild a single channel of the current palette (3 color)
// ----------------------------------------------------------------------------
void DXTCGen::BuildCodes3(int Channel, cbVector &v1, cbVector &v2)
{
	pVects[0][Channel] = v1[Channel];
	pVects[2][Channel] = v2[Channel];

	pVects[1][Channel] = (unsigned char)( ((int)v1[Channel] + (int)v2[Channel]) / 2 );
}

// ----------------------------------------------------------------------------
// Rebuild a single channel of the current palette (4 color)
// ----------------------------------------------------------------------------
void DXTCGen::BuildCodes4(int Channel, cbVector &v1, cbVector &v2)
{
	pVects[0][Channel] = v1[Channel];
	pVects[3][Channel] = v2[Channel];

	pVects[1][Channel] = (unsigned char)( ((int)v1[Channel] * 2 + (int)v2[Channel]) / 3 );
	pVects[2][Channel] = (unsigned char)( ((int)v2[Channel] * 2 + (int)v1[Channel]) / 3 );
}


// ----------------------------------------------------------------------------
// Compute the error Sum(dist^2) of a block when palettized
// ----------------------------------------------------------------------------
int DXTCGen::ComputeError(CodeBook & Pixels)
{
int Error = 0, Index, i;
int Count = Pixels.GetNumCodes();

	for(i=0; i<Count; i++)
	{
		Index = Vects.FindVectorSlow(Pixels[i]);
		Error += pVects[Index].DiffMag(Pixels[i]);
	}
	return Error;
}


// ----------------------------------------------------------------------------
// Map a 4x4 pixel block to a 3 color w/transparent black DXT1 block
//
// (in)  Source = unique colors in "Pixels"  (1 - 16 codes)
// (in)  Pixels = 4 x 4 pixels to be remapped
// (out) Dest = 3 output colors that best approximate the input (0, 2 are the endpoints)
// ----------------------------------------------------------------------------
int DXTCGen::Execute3(CodeBook &Source, CodeBook & Pixels, CodeBook &Dest)
{
int Count, i, j, c;
int BestIndex[2], BestError;
int Error, Start0, Start1, End0, End1;
cbVector Best[2], Test[2];

	Vects.SetCount(3);
	pVects = &Vects[0];

	BestIndex[0] = BestIndex[1] = 0;
	BestError = 1 << 30;

	Count = Source.GetNumCodes();
	for(i=0; i<(Count-1); i++)
	{
		for(j=i+1; j<Count; j++)
		{
			BuildCodes3(Source[i], Source[j]);		// Build the interpolants
			Error = ComputeError(Pixels);			// Compute the RMS error for Pixels

			if(Error < BestError)
			{
				BestError = Error;
				BestIndex[0] = i;
				BestIndex[1] = j;
			}
		}
	}

	Best[0] = Source[BestIndex[0]];
	Best[1] = Source[BestIndex[1]];
	Test[0] = Best[0];
	Test[1] = Best[1];

	for(c=1; c<4; c++)
	{
		Test[0] = Best[0];
		Test[1] = Best[1];
		BuildCodes3(Test[0], Test[1]);				// Build the full set of interpolants

		Start0 = (int)Test[0][c] - ScanRange/2;
		End0 = Start0 + ScanRange;

		Start0 = max(int(0), Start0);
		End0 = min(255, End0);

		Start1 = (int)Test[1][c] - ScanRange/2;
		End1 = Start1 + ScanRange;

		Start1 = max(0, Start1);
		End1 = min(255, End1);

		for(i=Start0; i<=End0; i+=ScanStep)
		{
			Test[0][c] = (unsigned char)i;
			for(j=Start1; j<=End1; j+=ScanStep)
			{
				Test[1][c] = (unsigned char)j;
				BuildCodes3(c, Test[0], Test[1]);	// Build the channel interpolants
				Error = ComputeError(Pixels);		// Compute the RMS error for Pixels

				if(Error < BestError)
				{
					BestError = Error;
					Best[0][c] = (unsigned char)i;
					Best[1][c] = (unsigned char)j;
				}
			}
		}
	}

	BuildCodes3(Best[0], Best[1]);

	Dest.SetCount(3);
	Dest[0] = pVects[0];
	Dest[1] = pVects[1];
	Dest[2] = pVects[2];

	return BestError;
}

// ----------------------------------------------------------------------------
// Map a 4x4 pixel block to a 4 color DXT1 block
//
// (in)  Source = unique colors in "Pixels"  (1 - 16 codes)
// (in)  Pixels = input 4 x 4 pixels to be remapped
// (out) Dest = 4 output colors that best approximate the input (0, 3 are the endpoints)
// ----------------------------------------------------------------------------
int DXTCGen::Execute4(CodeBook &Source, CodeBook & Pixels, CodeBook &Dest)
{
int Count, i, j, c;
int BestIndex[2], BestError;
int Error, Start0, Start1, End0, End1;
cbVector Best[2], Test[2];

	Vects.SetCount(4);
	pVects = &Vects[0];

	BestIndex[0] = BestIndex[1] = 0;
	BestError = 1 << 30;

	Count = Source.GetNumCodes();
	for(i=0; i<(Count-1); i++)
	{
		for(j=i+1; j<Count; j++)
		{
			BuildCodes4(Source[i], Source[j]);		// Build the interpolants
			Error = ComputeError(Pixels);		// Compute the RMS error for pixels

			if(Error < BestError)
			{
				BestError = Error;
				BestIndex[0] = i;
				BestIndex[1] = j;
			}
		}
	}

	Best[0] = Source[BestIndex[0]];
	Best[1] = Source[BestIndex[1]];
	Test[0] = Best[0];
	Test[1] = Best[1];

	for(c=1; c<4; c++)
	{
		Test[0] = Best[0];
		Test[1] = Best[1];
		BuildCodes4(Test[0], Test[1]);				// Build the full set of interpolants

		Start0 = (int)Test[0][c] - ScanRange/2;
		End0 = Start0 + ScanRange;

		Start0 = max(0, Start0);
		End0 = min(255, End0);

		Start1 = (int)Test[1][c] - ScanRange/2;
		End1 = Start1 + ScanRange;

		Start1 = max(0, Start1);
		End1 = min(255, End1);

		for(i=Start0; i<=End0; i+=ScanStep)
		{
			Test[0][c] = (unsigned char)i;
			for(j=Start1; j<=End1; j+=ScanStep)
			{
				Test[1][c] = (unsigned char)j;
				BuildCodes4(c, Test[0], Test[1]);	// Build the channel interpolants
				Error = ComputeError(Pixels);		// Compute the RMS error for pixels

				if(Error < BestError)
				{
					BestError = Error;
					Best[0][c] = (unsigned char)i;
					Best[1][c] = (unsigned char)j;
				}
			}
		}
	}

	BuildCodes4(Best[0], Best[1]);

	Dest.SetCount(4);
	Dest[0] = pVects[0];
	Dest[1] = pVects[1];
	Dest[2] = pVects[2];
	Dest[3] = pVects[3];

	return BestError;
}
