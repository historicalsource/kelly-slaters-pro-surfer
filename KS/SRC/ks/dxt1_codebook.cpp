
#include "global.h"
#include "dxt1_codebook.h"

int cbVector::DiffMag(const cbVector & Vect) const
{
	return (pData[0]-Vect.pData[0])*(pData[0]-Vect.pData[0]) +
		(pData[1]-Vect.pData[1])*(pData[1]-Vect.pData[1]) +
		(pData[2]-Vect.pData[2])*(pData[2]-Vect.pData[2]) +
		(pData[3]-Vect.pData[3])*(pData[3]-Vect.pData[3]);
}

int CodeBook::FindVectorSlow(const cbVector & Vect)
{
int		i, Count, Closest, ClosestIndex, TestMag;

	Count = VectList.Count();

	Closest = Vect.DiffMag(VectList[0]);
	ClosestIndex = 0;

	for(i=1; i<Count; i++)
	{
		TestMag = Vect.DiffMag(VectList[i]);
		if(TestMag < Closest)
		{
			Closest = TestMag;
			ClosestIndex = i;
			if(Closest == 0) break;
		}
	}

	return ClosestIndex;
}

void CodeBook::AddVector(cbVector &Vect)
{
int i, Count;

	Count = VectList.Count();
	for(i=0; i<Count; i++)
	{
		if(Vect == VectList[i])
		{
			usageCount[i]++;
			return;
		}
	}
	i = 1;
	VectList.Append(Vect);
	usageCount.Append(i);
}
