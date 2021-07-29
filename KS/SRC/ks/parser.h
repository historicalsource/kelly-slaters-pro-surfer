#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>

enum 
{
    IO_MAX_INCLUDE_DEPTH = 16,
    IO_MAX_DEFINE_LENGTH = 24,
    IO_MAX_FILENAME_LENGTH = 50,
};

struct ioFileBuf
{
    char* Buf;
    char* Pos;
    unsigned int Size;
    char Pushed;

    ioFileBuf() : Buf( 0 ), Pos( 0 ), Pushed( 0 ) {}
};

struct ioFileInfo
{
	int Line;
	char Name[IO_MAX_FILENAME_LENGTH];
	ioFileBuf File;
};

struct ioDefine 
{
	ioDefine* Next;
	int Val;
	char Name[IO_MAX_DEFINE_LENGTH + 1];
};

typedef unsigned int ioHashVal;

struct ioFile 
{
    bool Open;
	ioFileInfo*	CurFile;

    // Include file stack.
    int StackTop;
    ioFileInfo*	FileStack[IO_MAX_INCLUDE_DEPTH];

    // Hash table for fast define lookup.
	int DefHashSize;
	ioDefine** DefHashTbl;
    
    // Define table.
    int DefinesLeft;
	ioDefine* DefinesTable;
};

typedef bool (*ioReadFileCallback)( char* Name, ioFileBuf* File );
void ioSetReadFileCallback( ioReadFileCallback Callback );

typedef void (*ioReleaseFileCallback)( ioFileBuf* File );
void ioSetReleaseFileCallback( ioReleaseFileCallback Callback );

typedef void (*ioErrorCallback)( char* Info, char* Text );
void ioSetErrorCallback( ioErrorCallback Callback );

extern char ioErrorStr[];
void ioCritError( ioFile* File, char* Msg, ... );

bool ioOpen( ioFile *File, char *Name );
bool ioOpenDef( ioFile *File, char *Name, int MaxDefines );
void ioClose( ioFile *File );

char* ioGetWord( ioFile *File );
char* ioGetKeyword( ioFile *File );
char* ioGetString( ioFile *File );
int ioGetInt( ioFile *File );
float ioGetFloat( ioFile *File );
bool ioGetBool( ioFile *File);

bool ioTryGetString( ioFile *File, char** Val );
bool ioTryGetInt( ioFile *File, int* Val );
bool ioTryGetFloat( ioFile *File, float* Val );
bool ioTryGetBool( ioFile *File, bool* Val );

#endif

