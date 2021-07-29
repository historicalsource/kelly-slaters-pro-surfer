#include <assert.h>

#include "ngl_ate.h"

bool ATENameMatch(const atestring &find,const atestring &entry )
{
	//int l=strlen(find.c_str());
	int l=strlen(find.c_str()); //find.length();
	return ( (l==0) || (strnicmp(ASSTR(find),ASSTR(entry),l)==0) );
}

bool ATEHeaderValid( char *atefile )
{
	assert(sizeof(ATEFileEntry)==48);
	assert(sizeof(ATEFileHeader)==16);
	ATEFileHeader *afh=(ATEFileHeader *) atefile;
	if ( afh )
	{
		if ( afh->magic!=ATEMAGIC ) return false;
		if ( afh->vermajor!=ATEVERMAJOR ) return false;
		if ( afh->verminor!=ATEVERMINOR ) return false;
		return true;
	}
	assert(0);
	return false;
}

ATEFileEntry *ATEEntryHead( char *atefile, int entry )
{
//	int h=sizeof(ATEFileHeader);
//	int e=sizeof(ATEFileEntry);
//	int p=sizeof(atestring);
	int foff=sizeof(ATEFileHeader)+ (entry*sizeof(ATEFileEntry));
	return (ATEFileEntry *) ( atefile + foff );
}

ATE_UINT32 ATETextureCount( char *atefile,const atestring &texname )
{
	ATEFileHeader *afh=(ATEFileHeader *) atefile;
	int rv=0;
	if ( afh->items )
	{
		for ( ATE_UINT32 i=0; i<afh->items; i++ )
		{
			ATEFileEntry *afe=ATEEntryHead(atefile,i);
			if ( afe && ATENameMatch(texname,afe->name) ) rv++;
		}
	}
	return rv;
}

ATEFileEntry *ATENthTextureEntryHead( char *atefile,const atestring &texname, int entry )
{
	ATEFileHeader *afh=(ATEFileHeader *) atefile;
	int rv=0;
	if ( afh->items )
	{
		for ( ATE_UINT32 i=0; i<afh->items; i++ )
		{
			ATEFileEntry *afe=ATEEntryHead(atefile,i);
			if ( afe && ATENameMatch(texname,afe->name) )
			{
				if ( rv==entry ) return afe;
				rv++;
			}
		}
	}
	assert(0);
	return NULL;
}

ATEFileEntry *ATEByNameEntryHead( char *atefile,const atestring &texname )
{
	ATEFileHeader *afh=(ATEFileHeader *) atefile;
//	int rv=0;
	if ( afh->items )
	{
		for ( ATE_UINT32 i=0; i<afh->items; i++ )
		{
			ATEFileEntry *afe=ATEEntryHead(atefile,i);
			if ( afe && texname==afe->name )
				return afe;
		}
	}
	assert(0);
	return NULL;
}

static atestring baditem=atestring("(no name)");

atestring &ATETextureName( char *atefile,const atestring &texname, int i )
{
	ATEFileEntry *afe=ATENthTextureEntryHead(atefile,texname,i);
	if ( afe )
	{
		return afe->name; //afe->name;
	}
	assert(0);
	return baditem; //atestring("(no name)");
}

char *ATETextureHeader( char *atefile,const atestring &texname, int i )
{
	ATEFileEntry *afe=ATENthTextureEntryHead(atefile,texname,i);
	if ( afe ) return atefile+afe->hoff;
	assert(0);
	return NULL;
}

char *ATETextureImage( char *atefile,const atestring &texname, int i )
{
	ATEFileEntry *afe=ATENthTextureEntryHead(atefile,texname,i);
	if ( afe ) return atefile+afe->ioff;
	assert(0);
	return NULL;
}

char *ATETexturePalette( char *atefile,const atestring &texname, int i )
{
	ATEFileEntry *afe=ATENthTextureEntryHead(atefile,texname,i);
	if ( afe ) return atefile+afe->poff;
	assert(0);
	return NULL;
}



ATE_UINT32 ATEEntryCount( char *atefile )
{
	ATEFileHeader *afh=(ATEFileHeader *) atefile;
	return afh?afh->items:0;
}

atestring &ATEEntryName( char *atefile, int i )
{
	ATEFileEntry *afe=ATEEntryHead(atefile,i);
	if ( afe )
	{
		return afe->name;
	}
	assert(0);
	return baditem; //atestring("(no name)");
}

char *ATEEntryHeader( char *atefile,const atestring &texentry )
{
	ATEFileEntry *afe=ATEByNameEntryHead(atefile,texentry);
	if ( afe ) return atefile+afe->hoff;
	assert(0);
	return NULL;
}

char *ATEEntryImage( char *atefile,const atestring &texentry )
{
	ATEFileEntry *afe=ATEByNameEntryHead(atefile,texentry);
	if ( afe ) return atefile+afe->ioff;
	assert(0);
	return NULL;
}

char *ATEEntryPalette( char *atefile,const atestring &texentry )
{
	ATEFileEntry *afe=ATEByNameEntryHead(atefile,texentry);
	if ( afe ) return atefile+afe->poff;
	assert(0);
	return NULL;
}





