

#include "ksfile.h"
#include "mustash.h"

typedef stash *pstash;

pstash stashes[KSSTASH_TOTAL] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL };

// the stash file system sucks

bool KSFILE_OpenStash( KSStashType type, stringx file )
{
	KSFILE_CloseStash(type);

}


void KSFILE_CloseStash( KSStashType type )
{
	if ( stashes[type] )
	{

	}
}




FileSpec::FileSpec()
{
	name=stringx("");
	type=KSSTASH_SYSTEM;
}

FileSpec::FileSpec(const char *str)
{
	name=str;
	type=KSSTASH_SYSTEM;
}

FileSpec::FileSpec(const stringx &str)
{
	name=str;
	type=KSSTASH_SYSTEM;
}

FileSpec::FileSpec(const char *str, const KSStashType st)
{
	name=str;
	type=st;
}

FileSpec::FileSpec(const stringx &str, const KSStashType st)
{
	name=str;
	type=st;
}

