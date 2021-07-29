#ifndef _INSTBANK_H_
#define _INSTBANK_H_

typedef unsigned int u_int;

class nglInstanceBank
{
public:
	// Skip list based instancing system used for meshes and textures.
	struct Instance
	{
		nglFixedString Key;
		void* Value;
		int RefCount;
		Instance* Forward[1];  // Variable sized array of forward pointers
	};

	// Custom allocators, with alignment
	typedef void* (*InstanceAlloc)( unsigned int n, unsigned int align );
	typedef void (*InstanceFree)( void* p );

	static InstanceAlloc SetAllocFunc( InstanceAlloc ialloc );
	static InstanceFree SetFreeFunc( InstanceFree ifree );

	Instance* NIL;
	Instance* Head;

	nglInstanceBank( );
	nglInstanceBank( InstanceAlloc ialloc, InstanceFree ifree );
	// Performs actual object bootstrapping
	void Init();
	// Creates a new node or adds a reference to an existing one
	Instance* Insert(const nglFixedString& Key, void* Value);
	// returns the new reference count, -1 if the node didn't exist.
	int Delete(const nglFixedString& Key);
	Instance* Search(const nglFixedString& Key);

private:
	enum
	{ 
		BitsInRandom = 15,
		MaxNumberOfLevels = 16,
		MaxLevel = 15,
	};

	int RandomsLeft;
	int RandomBits;
	int Level;    // Maximum level of the list

	static InstanceAlloc AllocFunc;
	static InstanceFree FreeFunc;

	Instance* NewNodeOfLevel(int l);
	int RandomLevel();
};

#endif