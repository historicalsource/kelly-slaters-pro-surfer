#include <stdlib.h>
#include <assert.h>

#include "ngl_fixedstr.h"
#include "ngl_instbank.h"
#include <string.h>

nglInstanceBank::nglInstanceBank()
{
}

nglInstanceBank::nglInstanceBank( InstanceAlloc ialloc, InstanceFree ifree )
{
	SetAllocFunc( ialloc );
	SetFreeFunc( ifree );
}

void nglInstanceBank::Init( )
{
	// Initialize the skiplist system.
	NIL = NewNodeOfLevel(0);
  memset( &NIL->Key, 0xFF, sizeof(nglFixedString) );
	RandomBits = rand();
	RandomsLeft = BitsInRandom / 2;

	Level = 0;
	Head = NewNodeOfLevel(MaxNumberOfLevels);
	for (int i = 0; i < MaxNumberOfLevels; i++)
		Head->Forward[i] = NIL;
}

nglInstanceBank::InstanceAlloc nglInstanceBank::AllocFunc;
nglInstanceBank::InstanceFree nglInstanceBank::FreeFunc;

nglInstanceBank::InstanceAlloc nglInstanceBank::SetAllocFunc( InstanceAlloc ialloc )
{
	InstanceAlloc old = AllocFunc;

	AllocFunc = ialloc;

	return old;
}

nglInstanceBank::InstanceFree nglInstanceBank::SetFreeFunc( InstanceFree ifree )
{
	InstanceFree old = FreeFunc;

	FreeFunc = ifree;

	return old;
}

nglInstanceBank::Instance* nglInstanceBank::NewNodeOfLevel(int l)
{
	size_t n = sizeof( struct Instance ) + ( l ) * sizeof( Instance* );

	if( AllocFunc ) {
		// 4 a good default?
		return (Instance*) AllocFunc( n, 4 );
	} else {
		return (Instance*) malloc( n );
	}

}

int nglInstanceBank::RandomLevel()
{
	int b, level = 0;
	do
	{
		b = RandomBits & 3;
		if (!b) level++;
		RandomBits >>= 2;
		if (--RandomsLeft == 0)
		{
			RandomBits = rand();
			RandomsLeft = BitsInRandom / 2;
		}
	} while (!b);
	return level > MaxLevel ? MaxLevel : level;
}

nglInstanceBank::Instance* nglInstanceBank::Insert(const nglFixedString& Key, void* Value)
{
	int k;
	Instance* update[MaxNumberOfLevels];
	Instance* p;
	Instance* q;
	p = Head;
	k = Level;
	do
	{
		for (;;)
		{
			q = p->Forward[k];
			if (q->Key >= Key) break;
			p = q;
		}
		update[k] = p;
	} while(--k >= 0);
	if (q->Key == Key)
	{
		q->RefCount++;
		return q;
	}
	k = RandomLevel();
	if (k > Level)
	{
		k = ++Level;
		update[k] = Head;
	};
	q = NewNodeOfLevel(k);
	q->Key = Key;
	q->Value = Value;
	q->RefCount = 1;
	do
	{
		p = update[k];
		q->Forward[k] = p->Forward[k];
		p->Forward[k] = q;
	} while(--k >= 0);
	return 0;
}

int nglInstanceBank::Delete(const nglFixedString& Key)
{
	int k, m;
	Instance* update[MaxNumberOfLevels];
	Instance* p;
	Instance* q;
	p = Head;
	k = m = Level;
	do
	{
		for (;;)
		{
			q = p->Forward[k];
			if (q->Key >= Key) break;
			p = q;
		}
		update[k] = p;
	} while(--k >= 0);
	if (q->Key == Key)
	{
    int RefCount = --q->RefCount;
    if( RefCount <= 0 )
    {
		  for (k = 0; k <= m && (p = update[k])->Forward[k] == q; k++)
			  p->Forward[k] = q->Forward[k];

			if( FreeFunc ) {
				FreeFunc( q );
			} else {
			  free(q);
			}

		  while (Head->Forward[m] == NIL && m > 0)
			  m--;
		  Level = m;
    }
    return RefCount;
	} else
		return -1;
}

nglInstanceBank::Instance* nglInstanceBank::Search(const nglFixedString& Key)
{
	int k;
	Instance* p;
	Instance* q;
	p = Head;
	k = Level;
	do
	{
		for (;;)
		{
			q = p->Forward[k];
			if (q->Key >= Key)
				break;
			p = q;
		}
	} while ( --k >= 0 );
	if (q->Key != Key)
		return 0;
	return q;
};

