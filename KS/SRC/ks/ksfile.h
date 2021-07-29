
#ifndef _ksfile_h
#define _ksfile_h


enum KSStashType {
	KSSTASH_SYSTEM ,
	KSSTASH_MOVIES ,
	KSSTASH_INTERFACE ,
	KSSTASH_ALLBEACH ,
	KSSTASH_BEACH ,
	KSSTASH_ALLSURF ,
	KSSTASH_SURFER ,
	KSSTASH_TOTAL ,
};

struct FileSpec
{
	stringx     name;
	KSStashType type;

	FileSpec();
	FileSpec(const char *str);
	FileSpec(const stringx &str);
	FileSpec(const char *str, const KSStashType st);
	FileSpec(const stringx &str, const KSStashType st);
	~FileSpec() {};
};

bool KSFILE_OpenStash( KSStashType type, stringx file );
void KSFILE_CloseStash( KSStashType type );










#endif


