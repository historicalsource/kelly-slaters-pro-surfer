
#ifndef INCLUDED_DXT1_GEN_H
#define INCLUDED_DXT1_GEN_H

#include "global.h"
#include "dxt1_codebook.h"

class DXTCGen
{
private:
	static const int ScanRange;
	static const int ScanStep;

private:
	CodeBook	Vects;
	cbVector	*pVects;

private:
	void BuildCodes3(cbVector &v1, cbVector &v2);
	void BuildCodes4(cbVector &v1, cbVector &v2);

	void BuildCodes3(int Channel, cbVector &v1, cbVector &v2);
	void BuildCodes4(int Channel, cbVector &v1, cbVector &v2);

	int ComputeError(CodeBook & Pixels);

public:
	int Execute3(CodeBook &Source, CodeBook & Pixels, CodeBook &Dest);
	int Execute4(CodeBook &Source, CodeBook & Pixels, CodeBook &Dest);
};

#endif INCLUDED_DXT1_GEN_H