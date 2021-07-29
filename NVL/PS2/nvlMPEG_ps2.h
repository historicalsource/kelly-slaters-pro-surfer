#ifndef NVLMPEG_PS2_H
#define NVLMPEG_PS2_H

#define NVLMPEG_PRIORITY    1

// Load and start playing a movie.
// pGSDBuf should refer an initialized sceGsDBuff structure
bool    nvlMPEGLoad         ( char *bsfilename, void* pGSDBuf );
// Advance currently playing movie. Should be called every frame
int     nvlMPEGAdvance      ();
// Stop currently playing movie and free all allocated resources
void    nvlMPEGStop         ();
// Set volume, 0.0 - no sound; 1.0 - maximum
void    nvlMPEGSetVolume    ( float vol );
// Set callback for memory allocation
void    nvlMPEGSetMemoryAllocCallback   ( void* (*func)(int alighnment, int size) );
// Set callback for memory free
void    nvlMPEGSetMemoryFreeCallback    ( void (*func)(void*) );

#endif  // NVLMPEG_PS2_H



