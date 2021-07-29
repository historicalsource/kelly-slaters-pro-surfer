#include <string.h>
#include <stdarg.h>
#include "parser.h"

#define countof(x) (int)(sizeof(x)/sizeof((x)[0]))

char ioWork[1024];
char ioWorkPad[16]; // Cause I'm a paranoid off-by-one psychopath.

char ioErrorStr[] = "%s(%d)";
ioErrorCallback ioCallback = 0;

inline char ioBufGetC( ioFileBuf& Buf )
{
    char Ret;
    if ( Buf.Pushed )
    {
        Ret = Buf.Pushed;
        Buf.Pushed = 0;
    }
    else
    {
        if ( (unsigned int)( Buf.Pos - Buf.Buf ) < Buf.Size )
			Ret = *( Buf.Pos++ );
        else
			Ret = EOF;
    }
    return Ret;
}

inline void ioBufUngetC( char Ch, ioFileBuf& Buf )
{
    Buf.Pushed = Ch;
}

ioReadFileCallback ioReadFile = 0;
ioReleaseFileCallback ioReleaseFile = 0;

void ioSetReadFileCallback( ioReadFileCallback Callback )
{
    ioReadFile = Callback;
}

void ioSetReleaseFileCallback( ioReleaseFileCallback Callback )
{
    ioReleaseFile = Callback;
}

void ioCritError( ioFile* Txt, char* Msg, ... )
{
    static char Work[1024];  // Local buffer so as not to bash the main one.
    static char Title[IO_MAX_FILENAME_LENGTH + 16];
    va_list Args;
	
    va_start( Args, Msg );
    vsprintf( Work, Msg, Args );
    va_end( Args );
	
    sprintf( Title, ioErrorStr, Txt->CurFile->Name, Txt->CurFile->Line );
	
    if ( ioCallback )
        ioCallback( Title, Work );
}

void ioSetErrorCallback( ioErrorCallback Callback )
{
    ioCallback = Callback;
}

ioHashVal ioComputeHashVal( ioFile* File, char* Text ) 
{
    ioHashVal HashVal = 0;
	
    while ( Text[0] != '\0' ) 
    {
        HashVal += (ioHashVal)Text[0] + ( (ioHashVal)Text[1] << 8 );
        if ( Text[1] == '\0' )
            break;
        Text += 2;
    }
	
    return HashVal % File->DefHashSize;
}

void ioSetDefineVal( ioFile* File, char* Text, int Val ) 
{
    ioDefine* Define;
    ioHashVal HashVal;
	
    HashVal = ioComputeHashVal( File, Text );
	
    // See if it's already defined, if so redefine.
    for ( Define = File->DefHashTbl[HashVal]; Define != 0; Define = Define->Next )
    {
        if ( strcmp( Text, Define->Name ) == 0 ) 
        {
            Define->Val = Val;
            return;
        }
    }
	
    if ( File->DefinesLeft == 0 )
		ioCritError( File, "Out of Define Space." );
	
    // Create a new define entry.
    Define = &File->DefinesTable[--File->DefinesLeft];
    Define->Next = File->DefHashTbl[HashVal];
    File->DefHashTbl[HashVal] = Define;
    Define->Val = Val;
    strcpy( Define->Name, Text );
}

bool ioGetDefineVal( ioFile* File, char* Text, int* Val ) 
{
    ioDefine* Define;
    ioHashVal HashVal;
	
    HashVal = ioComputeHashVal( File, Text );
    for ( Define = File->DefHashTbl[HashVal]; Define != 0; Define = Define->Next )
    {
        if ( strcmp( Text, Define->Name ) == 0 ) 
        {
            *Val = Define->Val;
            return true;
        }
    }
	
    return false;
}

char *ioGetWord( ioFile * File ) 
{
    static char Key[ 128 ];
    static char White[] = " \t\v\r\f\255";
    int i;
    int ch;
    
    if ( !File->Open )
        return 0;
    
    if ( File->CurFile == 0 )
        return 0;
    
    i = 0;
    memset( Key, '\0', sizeof( Key ) );
    
    for (;;) 
    {
        ch = ioBufGetC( File->CurFile->File );
        
        if ( ch == EOF )
            return 0;
        if ( ch == '\n' ) 
        {
            ioBufUngetC( '\n', File->CurFile->File );
            return 0;
        }
        if ( ch == ';' || ch == '/' ) 
        {
            do {
                ch = ioBufGetC( File->CurFile->File );
                if ( ch == '\n' ) 
                {
                    ioBufUngetC( '\n', File->CurFile->File );
                    break;
                }
            } while ( ch != EOF );
            return 0;
        }
        if ( ch == '"' ) 
        {
            for (;;) 
            {
                ch = ioBufGetC( File->CurFile->File );
                if ( ch == EOF || ch == '"' ) 
                {
                    Key[i] = '\0';
                    return Key;
                }
                Key[i++] = ch;
                if ( i == countof(Key) - 1 ) 
                {
                    Key[countof(Key) - 1] = '\0';
                    do {
                        ch = ioBufGetC( File->CurFile->File );
                    } while ( ch != EOF && ch != '"' );
                    return Key;
                }
            }
        }
        if ( strchr( White, ch ) == 0 ) 
        {
            for (;;) 
            {
                if ( i < countof(Key) - 1 )
                    Key[i++] = ch;
                
                ch = ioBufGetC( File->CurFile->File );
                
                if ( ch == EOF || strchr( White, ch ) != 0 ) 
                {
                    Key[i] = '\0';
                    return Key;
                }
                if ( ch == '\n' ) 
                {
                    ioBufUngetC( '\n', File->CurFile->File );
                    Key[i] = '\0';
                    return Key;
                }
                if ( ch == ';' || ch == '/' ) 
                {
                    do {
                        ch = ioBufGetC( File->CurFile->File );
                        if ( ch == '\n' ) 
                        {
                            ioBufUngetC( '\n', File->CurFile->File );
                            break;
                        }
                    } while ( ch != EOF );
                    Key[i] = '\0';
                    return Key;
                }
            }
        }
    }
}

char* ioGetKeyword( ioFile* File ) 
{
    int Val;
    char* Key;
    int ch;
    
    if ( !File || !File->Open || !File->CurFile )
        return 0;
    
    Key = 0;
    
    for (;;) 
    {
        do {
            ch = ioBufGetC( File->CurFile->File );
            while ( ch == EOF ) 
            {
                ioReleaseFile( &File->CurFile->File );
                free( File->CurFile );
                File->CurFile = 0;
                if ( File->StackTop == 0 )
                    return 0;
                File->CurFile = File->FileStack[--File->StackTop];
                ch = ioBufGetC( File->CurFile->File );
            }
        } while ( ch != '\n' );
        
        File->CurFile->Line++;
        
        Key = ioGetWord( File );
        
        if ( Key != 0 ) 
        {
            strupr( Key );
            
            if ( strcmp( Key, "#DEFINE" ) == 0 ) 
            {
                char DefName[IO_MAX_DEFINE_LENGTH + 1];
                
                if ( ( Key = ioGetString( File ) ) == 0 ) 
                {
                    ioCritError( File, "Define needs a name." );
                    return 0;
                }
                if ( strlen( Key ) > IO_MAX_DEFINE_LENGTH ) 
                {
                    ioCritError( File, "Length of define exceeds %d characters.", IO_MAX_DEFINE_LENGTH );
                    return 0;
                }
                strcpy( DefName, Key );
                strupr( DefName );
                if ( !ioTryGetInt( File, &Val ) )
                    Val = 0;
                ioSetDefineVal( File, DefName, Val );
            }
            else if ( strcmp( Key, "#INCLUDE" ) == 0 ) 
            {
                ioFileBuf F;
                
                if ( ( Key = ioGetString( File ) ) == 0 ) 
                {
                    ioCritError( File, "Include needs a file name." );
                    return 0;
                }
                if ( File->StackTop >= countof(File->FileStack) ) 
                {
                    ioCritError( File, "Exceeded %d include levels.", IO_MAX_INCLUDE_DEPTH  );
                    return 0;
                }
                if ( ioReadFile( Key, &F ) == false ) 
                {
                    ioCritError( File, "Could not open include file %s.", Key );
                    return 0;
                }
                ioBufUngetC( '\n', F );
                
                File->FileStack[File->StackTop++] = File->CurFile;
                File->CurFile = (ioFileInfo*)malloc( sizeof(ioFileInfo) );
                File->CurFile->File = F;
                strncpy( File->CurFile->Name, Key, sizeof(File->CurFile->Name) );
                File->CurFile->Line = 0;
            }
            else if ( strcmp( Key, "#MESSAGE" ) == 0 ) 
            {
                int i = 0;
                while ( ( ch = ioBufGetC( File->CurFile->File ) ) != '\n' )
                {
                    if ( i + 1 >= countof(ioWork) )
                        break;
                    ioWork[i] = ch;
                    i++;
                }
                ioWork[i] = '\0';
                
                ioBufUngetC( '\n', File->CurFile->File );
                
				//                ioWarning( File, ioWork );
            }
#if 0
            // TODO: Provide an interface for memory and then re-enable this stuff.
            else if ( strcmp( Key, "#MEMRESET" ) == 0 ) {
                memResetByteCount();
            }
            else if ( strcmp( Key, "#MEMCHECK" ) == 0 ) {
                memset( ioWork, 0, strlen( ioWork ) );
                
                while ( ( ch = ioBufGetC( File->CurFile->File ) ) != '\n' )
                {
                    ASSERT( strlen( ioWork ) < countof( ioWork ) );
                    ioWork[ strlen( ioWork ) ] = ch;
                }
                
                fioBufUngetC( '\n', File->CurFile->File );
                
                errWarning( "%s%d ", ioWork, memGetByteCount() );
            }
            else if ( strcmp( Key, "#MEMAVAIL" ) == 0 ) {
                errWarning( "Memory available: %d", MemAvail() );
            }
#endif
            else 
            {
                return Key;
            }
        }
    }
}

char* ioGetString( ioFile* File )
{
    char *Key = 0;
    
    if( ( Key = ioGetWord( File ) ) == 0 )
        ioCritError( File, "String data expected." );
    
    return Key;
}


int ioGetInt( ioFile* File )
{
    int Val;
    if ( !ioTryGetInt( File, &Val ) ) 
    {
        ioCritError( File, "Integer data expected." );
        return 0;
    }
	
    return Val;
}

float ioGetFloat( ioFile* File )
{
    float Val;
	
    if ( !ioTryGetFloat( File, &Val ) ) 
    {
        ioCritError(File,"Floating point data expected.");
        return 0.0f;
    }
	
    return Val;
}

bool ioGetBool( ioFile *File )
{
    bool Val;
    if ( !ioTryGetBool( File, &Val ) ) 
    {
        ioCritError( File, "Boolean data expected" );
        return false;
    }
	
    return Val;
}

bool ioTryGetString( ioFile *File, char **Val )
{
    char *Key = 0;
    
    if ( ( Key = ioGetWord( File ) ) == 0 )
        return false;
    
    *Val = Key;
    return true;
}

bool ioTryGetInt( ioFile *File, int* Val )
{
    static char* GoodChar="-0123456789";
    char* OriginalBuf = 0;
    char* Buf = 0;
    int i;
    
    if ( ( Buf = ioGetWord( File ) ) == 0 )
        return false;
    
    OriginalBuf = Buf;
    strupr( Buf );
    
    if ( ioGetDefineVal( File, Buf, Val ) ) 
        return true;
    
    i = strlen(Buf);
    while ( i )
    {
        if ( strchr( GoodChar, Buf[--i] ) == 0 )
        {
            ioCritError( File, "%s: Bad integer.", OriginalBuf );
            return false;  
        }
    }
	
    *Val = atoi( Buf );
    return true;
}

bool ioTryGetFloat( ioFile *File, float* Val )
{
    static char* GoodChar="-.0123456789";
    char* OriginalBuf = 0;
    char* Buf = 0;
    int i;
    
    if ( ( Buf = ioGetWord( File ) ) == 0 )
        return false;
    
    OriginalBuf = Buf;
    strupr( Buf );
    
    i = strlen(Buf);
    while ( i )
    {
        if ( strchr( GoodChar, Buf[--i] ) == 0 )
        {
            ioCritError( File, "%s: Bad float.", OriginalBuf );
            return false;  
        }
    }
	
    *Val = (float)atof( Buf );
    return true;
}

bool ioTryGetBool( ioFile* File, bool* Val )
{
    char* Key = 0;
    int DefVal;
    
    if ( ( Key = ioGetWord( File ) ) == 0 )
        return false;
    
    strupr( Key );
    if( !strcmp( Key, "TRUE" ) )
    {
        *Val = true;
        return true;
    }
    
    if( !strcmp( Key, "FALSE" ) )
    {
        *Val = false;
        return true;
    }
    
    if ( ioGetDefineVal( File, Key, &DefVal ) ) 
    {
        *Val = DefVal != 0;
        return true;
    }
    
    return false;
}


bool ioOpen( ioFile* File, char* Name ) 
{
    return ioOpenDef( File, Name, 256 );
}

bool ioOpenDef( ioFile* File, char* Name, int MaxDefines ) 
{
    ioFileBuf F;
    int i;
	
    static int HashSizes[] = { 8191, 4093, 2039, 1021, 509, 251, 127, 61, 31, 13, 7, 3, 2, 1 };
	
    if ( !File )
        return false;
	
    memset( File, 0, sizeof(ioFile) );
	
    if ( !Name || !MaxDefines )
        return false;
	
    if ( ioReadFile( Name, &F ) == false )
        return false;
	
    ioBufUngetC( '\n', F );
	
    File->CurFile = (ioFileInfo*)malloc( sizeof(ioFileInfo) );
    File->CurFile->File = F;
    strncpy( File->CurFile->Name, Name, sizeof(File->CurFile->Name) );
    File->CurFile->Line = 0;
	
    File->StackTop = 0;
	
    for ( i = 0; true; i++ ) 
    {
        if (    HashSizes[i] <= MaxDefines / 2
			|| i >= countof(HashSizes) - 1 )
        {
            File->DefHashSize = HashSizes[i];
            break;
        }
    }
    File->DefHashTbl = (ioDefine**)malloc( sizeof(ioDefine*) * File->DefHashSize );
    for ( i = 0; i < File->DefHashSize; i++ )
        File->DefHashTbl[i] = 0;
    File->DefinesLeft = MaxDefines;
    File->DefinesTable = (ioDefine*)malloc( sizeof(ioDefine) * MaxDefines );
    File->Open = true;
	
    return true;
}

void ioClose( ioFile* File ) 
{
    if ( !File || !File->Open )
        return;
    
    if ( File->CurFile != 0 ) 
    {
        for (;;) 
        {
            ioReleaseFile( &File->CurFile->File );
            free( File->CurFile );
            File->CurFile = 0;
            if ( File->StackTop == 0 )
                break;
            File->CurFile = File->FileStack[--File->StackTop];
        }
    }
    
    free( File->DefinesTable );
    free( File->DefHashTbl );
    
    File->Open = false;
}
