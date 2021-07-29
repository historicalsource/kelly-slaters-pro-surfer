
#ifndef INCLUDED_DXT1_CODEBOOK_H
#define INCLUDED_DXT1_CODEBOOK_H

#include "global.h"
#include "dxt1_table.h"

class cbVector
{
private:
	unsigned char	pData[4];

public:
	cbVector() {;}
	~cbVector() {;}

	inline unsigned char *GetPtr(void) {return pData;}

	inline unsigned char &operator[](int i) {return pData[i];}
	cbVector &operator=(const cbVector & Vect) {*(int *)pData = *(int *)Vect.pData; return *this;}
	int operator==(cbVector &Vect) {return *(int *)pData == *(int *)Vect.pData;}

	int DiffMag(const cbVector & Vect) const;	// Magnitude of the difference between this and Vect (Dist ^ 2)

	friend class CodeBook;
};

class CodeBook
{
private:
	Table<cbVector>		VectList;
	Table<int>			usageCount;

public:
	virtual ~CodeBook() {;}

	void AddVector(cbVector &Vect);
	inline void SetSize(int Size) {VectList.Resize(Size); usageCount.Resize(Size);}
	inline void SetCount(int Count) {VectList.SetCount(Count); usageCount.SetCount(Count);}

	int FindVectorSlow(const cbVector &Vect);
	int UsageCount(const int i) { return usageCount[i]; }
	inline int GetNumCodes(void) const { return VectList.Count(); }

	cbVector &operator[](const int i) { return VectList[i]; }
	//const cbVector & operator[](const int i) { return VectList[i]; }
};

#endif INCLUDED_DXT1_CODEBOOK_H