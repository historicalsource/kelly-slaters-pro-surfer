#ifndef _NSL_TESTAPPH_ 
#define _NSL_TESTAPPH_

#include "nsl_ps2.h"

//=======================================
// TESTING nslPlaySound 
//=======================================
void nslTestPlayInvalidId( void );                  // ASSERT
void nslTestPlayValidIdAfterStreaming( void ) ;
void nslTestPlayValidId( void );
void nslTestPlayAfterRunOut( void ) ;               // ASSERT
void nslTestPlayAfterStop( void ) ;                 // ASSERT


//=======================================
// TESTING nslIsSoundPlaying
//=======================================
void nslTestIsPlayingInvalidId( void ) ;
void nslTestIsPlayingValidIdAfterStreaming( void ) ;
void nslTestIsPlayingValidId( void ) ;
void nslTestIsPlayingAfterRunOut( void ) ;           
void nslTestIsPlayingAfterStop( void ) ;

//=============================================
// TESTING nslIsSoundReady
//=============================================
void nslTestIsSoundReadyInvalidId( void ) ;         // ASSERT
void nslTestIsSoundReadyValidId( void ) ;
void nslTestIsSoundReadyAfterRunOut( void ) ;       // ASSERT
void nslTestIsSoundReadyAfterStop( void ) ;         // ASSERT
void nslTestIsSoundReadyWhilePaused ( void ) ;

//=============================================
// TESTING nslPauseSound
//=============================================
void nslPauseSoundInvalidId( void );                // ASSERT
void nslPauseSoundValidIdPlaying( void );
void nslPauseSoundWhilePaused( void );
void nslPauseSoundAfterRunOut( void );              // ASSERT
void nslPauseSoundAfterStop( void );                // ASSERT

//=============================================
// TESTING nslUnpauseSound
//=============================================
void nslUnpauseSoundInvalidId( void  );             // ASSERT
void nslUnpauseSoundAfterRunOut( void );            // ASSERT
void nslUnpauseSoundAfterStop( void ) ;             // ASSERT
void nslPauseUnpauseMultiple( void ); 

//=============================================
// TESTING nslPauseGuardSound
//=============================================
void nslPauseGuardSoundInvalidId() ;                // ASSERT
void nslPauseGuardSoundValidIdAfterStart() ;  
void nslPauseGuardSoundPaused() ;
void nslPauseGuardSoundValidIdAfterStop() ;         // ASSERT
void nslPauseGuardSoundValidIdAfterRunOut();        // ASSERT

//=============================================
// TESTING nslPauseAllSounds
//=============================================
void nslPauseAllSoundsNoSounds() ;                  
void nslPauseAllSoundsValid() ;

//=============================================
// TESTING nslUnpauseAllSounds
//=============================================
void nslUnpauseAllSoundsPaused() ;
void nslUnpauseAllSoundsNoSounds() ;

//=============================================
// TESTING nslStopSound
//=============================================
void nslStopSoundInvalidId() ;                      // ASSERT
void nslStopSoundAfterRunOut();                     // ASSERT
void nslStopSoundAfterStop() ;                      // ASSERT

#endif 