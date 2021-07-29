
#ifndef KSHOOKS_H
#define KSHOOKS_H

void KSCriticalError( const char* Text );
void KSDebugPrint( const char* Text );;
void* KSMemAllocate( u_int Size, u_int Align, const char *file, int line );
void* KSMemAlloc( u_int Size, u_int Align );
void* KSMemAllocNGL( u_int Size, u_int Align );
void* KSMemAllocNSL( u_int Size, u_int Align );
void* KSMemAllocNVL( u_int Size, u_int Align );
void KSMemFree( void* Ptr );
bool KSReadFile( const char* FileName, nglFileBuf* File, u_int Align );
void KSReleaseFile( nglFileBuf* File );

#endif

