#include "ngl_InstBank.h"

nglInstanceBank::nglInstanceBank()
{
	// empty
}

nglInstanceBank::nglInstanceBank(InstanceAlloc ialloc, InstanceFree ifree)
{
	SetAllocFunc(ialloc);
	SetFreeFunc(ifree);
}

void nglInstanceBank::Init()
{
	// Initialize the skiplist system.
	NIL = NewNodeOfLevel(0);

	const int n = nglFixedString::MAX_CHARS;
	char work[1024];
	memset(&work, 0xFF, n - 1);
	work[n - 1] = 0;
	nglFixedString tmp(work);
	NIL->Key = tmp;

	RandomBits = rand();
	RandomsLeft = BitsInRandom / 2;

	// init
	Level = 0;
	Head = NewNodeOfLevel(MaxNumberOfLevels);
	for (int32 i = 0; i < MaxNumberOfLevels; i++)
		Head->Forward[i] = NIL;
}

nglInstanceBank::InstanceAlloc nglInstanceBank::AllocFunc;
nglInstanceBank::InstanceFree nglInstanceBank::FreeFunc;

nglInstanceBank::InstanceAlloc nglInstanceBank::
SetAllocFunc(InstanceAlloc ialloc)
{
	InstanceAlloc old = AllocFunc;

	AllocFunc = ialloc;

	return old;
}

nglInstanceBank::InstanceFree nglInstanceBank::SetFreeFunc(InstanceFree ifree)
{
	InstanceFree old = FreeFunc;

	FreeFunc = ifree;

	return old;
}

nglInstanceBank::Instance *nglInstanceBank::NewNodeOfLevel(int32 l)
{
	size_t n = sizeof(struct Instance) + (l) * sizeof(Instance *);

	if (AllocFunc)
	{
		// 4 a good default?
		return (Instance *) AllocFunc(n, 4);
	}
	else
	{
		return (Instance *) malloc(n);
	}

}

int32 nglInstanceBank::RandomLevel()
{
	int32 b, level = 0;
	do
	{
		b = RandomBits & 3;
		if (!b)
			level++;
		RandomBits >>= 2;
		if (--RandomsLeft == 0)
		{
			RandomBits = rand();
			RandomsLeft = BitsInRandom / 2;
		}
	} while (!b);
	return level > MaxLevel ? MaxLevel : level;
}

nglInstanceBank::Instance *nglInstanceBank::Insert(const nglFixedString & Key, void *Value)
{
	int32 k;
	Instance *update[MaxNumberOfLevels];
	Instance *p;
	Instance *q;
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
		update[k] = p;
	} while (--k >= 0);
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
	} while (--k >= 0);
	return 0;
}

int32 nglInstanceBank::Delete(const nglFixedString & Key)
{
	int32 k, m;
	Instance *update[MaxNumberOfLevels];
	Instance *p;
	Instance *q;
	p = Head;
	k = m = Level;
	do
	{
		for (;;)
		{
			q = p->Forward[k];
			if (q->Key >= Key)
				break;
			p = q;
		}
		update[k] = p;
	} while (--k >= 0);
	if (q->Key == Key)
	{
		int32 RefCount = --q->RefCount;
		if (RefCount <= 0)
		{
			for (k = 0; k <= m && (p = update[k])->Forward[k] == q; k++)
				p->Forward[k] = q->Forward[k];

			if (FreeFunc)
			{
				FreeFunc(q);
			}
			else
			{
				free(q);
			}

			while (Head->Forward[m] == NIL && m > 0)
				m--;
			Level = m;
		}
		return RefCount;
	}
	else
		return -1;
}

nglInstanceBank::Instance *nglInstanceBank::Search(const nglFixedString & Key)
{
	int32 k;
	Instance *p;
	Instance *q;
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
	} while (--k >= 0);
	if (q->Key != Key)
		return 0;
	return q;
};

// unit test.  
#if 0
#define NUM_TESTS 1000
#include <stdio.h>

void TestInstanceBank()
{
	int32 nums[NUM_TESTS];
	nglFixedString teststr;
	int32 num_deleted = 0;

	nglInstanceBank testbank;
	testbank.Init();
	int32 i;
	for (i = 0; i < NUM_TESTS; i++)
	{
		nums[i] = rand() % 500;
		sprintf(teststr, "x%d", nums[i]);
		testbank.Insert(teststr, (void *) i);
	}
	for (i = 0; i < NUM_TESTS; i++)
	{
		sprintf(teststr, "x%d", nums[i]);
		nglInstanceBank::Instance * found = testbank.Search(teststr);
		NGLASSERT(found != testbank.NIL, "");
		NGLASSERT((found->RefCount > 1) || ((int32) (found->Value) == i), "");
	}
	for (i = 0; i < NUM_TESTS; i++)
	{
		if ((rand() % 4))
		{
			sprintf(teststr, "x%d", nums[i]);
			int32 stuff = testbank.Delete(teststr);
			NGLASSERT(stuff >= 0, "");
			num_deleted++;
		}
	}
	nglInstanceBank::Instance *Inst = testbank.Head->Forward[0];
	while (Inst != testbank.NIL)
	{
		num_deleted += Inst->RefCount;
		Inst->RefCount = 1;
		nglInstanceBank::Instance *Next = Inst->Forward[0];
		Inst = Next;
	}
	NGLASSERT(num_deleted == NUM_TESTS, "");
}
#endif
