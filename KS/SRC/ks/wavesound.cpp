// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#if defined( _XBOX )
#include "refptr.h"
#include "hwrasterize.h"
#include "wds.h"
#include "po.h"
#include "app.h"
#include "game.h"
#include "conglom.h"
#include "kellyslater_controller.h"

#define nslPrintf(x) nglPrintf(x)

#endif /* _XBOX JIV DEBUG */
#include "wave.h"
#include "wavesound.h"
#include "ksnsl.h"
#include "waveenum.h"
#include "board.h"
#include "algebra.h"
#include "po.h"
#include "random.h"
#include "VOEngine.h"
#include "wavedata.h"
#include "ksngl.h"	// For KSNGL_TransposeMatrix

#define MAX_TUBE_EXTEND_DIST 8


WaveSound wSound;
void WaveSound::init( ) 
{ 
  nlVector3d pos;
//  vector3d v;
  int current_beach = g_game_ptr->get_beach_id ( );
  
  char soundfile[40];


  sprintf(soundfile, "%s_AMBTRK", strupr(BeachDataArray[g_game_ptr->get_first_beach()].stashfile));


  // A few variables for state
  is_paused     = false;
  wet           = NULL;
  crashed       = false;
  initialized   = true;

  // These variables control volume 
  // modulation for the regions

  faceModifier        = 0.5f;
  tubeModifier        = 0.5f;
  foamModifier        = 0.5f;
  lapModifier         = 0.5f;
  //underWaterModifier  = 1.1f;
  crashModifier       = 0.5f;
  ambVol              = 1.0f;
	
	dampenval						= 1.0f;
	
  sfx.playRain();




  ambsrc = nslLoadSource(strupr(soundfile));
  if (ambsrc != NSL_INVALID_ID)
  {
    ambtrk = nslAddSound(ambsrc);
    nslSetSoundParam(ambtrk, NSL_SOUNDPARAM_VOLUME, ambVol);
    if (nslGetSoundStatus(ambtrk) != NSL_SOUNDSTATUS_INVALID)
      nslPlaySound(ambtrk);
#ifdef KEVIN
		else
			assert(0 && "NO AMBTRK");
#endif

  }
#ifdef KEVIN
	else
		assert(0 && "NO AMBTRK");
#endif


  perspective   = HYBRID_PERSPECTIVE;
  showSpheres   = false;

  waveDir = BeachDataArray[current_beach].bdir;
  
  // Init emitter pos
  pos[0]        = 0;
  pos[1]        = 0; 
  pos[2]        = 0;

  // Create a few emitters
  
  for ( int i=0; i < MAX_REGION_SOUNDS; i++ )
  {
    int evenOrOdd = i%2;
    int currentSlot = i/2;
    foamEmitters[evenOrOdd][currentSlot] = nslCreateEmitter( pos );
    foamSnd[evenOrOdd][currentSlot] = NSL_INVALID_ID;
    foamPos[evenOrOdd][currentSlot][0] = -9999;
    faceEmitters[evenOrOdd][currentSlot] = nslCreateEmitter( pos );
    faceSnd[evenOrOdd][currentSlot] = NSL_INVALID_ID;
    facePos[evenOrOdd][currentSlot][0] = -9999;
    tubeEmitters[evenOrOdd][currentSlot] = nslCreateEmitter( pos );
    tubeSnd[evenOrOdd][currentSlot] = NSL_INVALID_ID;
    tubePos[evenOrOdd][currentSlot][0] = -9999;
  }
  
  underwater = false;
  numFaceSounds = -1;
  numTubeSounds = 0;
  numFoamSounds = 0;
  
  // Impact sound of a crash
  waveCrashSrc = nslLoadSource( "WAVE_WAVECRASH01" );
  if ( waveCrashSrc == NSL_INVALID_ID )
    nslPrintf( "FIXME: MISSING WAVE_WAVECRASH01 SOUND\n" ); 
  
  for ( int i=0; i < MAX_REGION_SOUNDS * 3; i++ )
		AddInSounds[i] = DropOutSounds[i] = NSL_INVALID_ID;
  
  // Start up the wave sounds

  // Load the wave sources
  // End of foam
  foamSrc[0] = nslLoadSource( "WAVE_EDGE01" );
  if ( foamSrc[0] == NSL_INVALID_ID )
    nslPrintf( "FIXME: MISSING WAVE_EDGE01 SOUND\n" );

  // Start of foam
  foamSrc[1] = nslLoadSource( "WAVE_CRASH01" );
  if ( foamSrc[1] == NSL_INVALID_ID )
    nslPrintf( "FIXME: MISSING WAVE_CRASH01 SOUND\n" );


  muffleSound=NSL_INVALID_ID;
	// End of face
  faceSrc[0] = nslLoadSource( "WAVE_FACE01" );
  if ( faceSrc[0] == NSL_INVALID_ID )
    nslPrintf( "FIXME: MISSING WAVE_FACE01 SOUND\n" );

  // Start of face  ( We swap the order on these two positions
  // The top position in code is not really the one we want
  // so yes, I know the order for face is different
  // with regard to WAVE_SoundEmitter
  faceSrc[1] = nslLoadSource( "WAVE_FACE02" );
  if ( faceSrc[1] == NSL_INVALID_ID )
    nslPrintf( "FIXME: MISSING WAVE_FACE02 SOUND\n" );



  // End of Tube
  tubeSrc[0] = nslLoadSource( "WAVE_TUBE01" );
  if ( tubeSrc[0] == NSL_INVALID_ID )
    nslPrintf( "FIXME: MISSING WAVE_TUBE01 SOUND\n" );

  // Start of Tube
  tubeSrc[1] = nslLoadSource( "WAVE_TUBE02" );
  if ( tubeSrc[1] == NSL_INVALID_ID )
    nslPrintf( "FIXME: MISSING WAVE_TUBE02 SOUND\n" );

	
	

	if (WAVE_GetHeight() < 3)
	{
		muffleSrc = nslLoadSource( "IG_SMALL_TUBE" );
		if ( tubeSrc[1] == NSL_INVALID_ID )
			nslPrintf( "FIXME: MISSING IG_SMALL_TUBE SOUND\n" );
	}
	else if (WAVE_GetHeight() < 5)
	{
		muffleSrc = nslLoadSource( "IG_MEDIUM_TUBE" );
		if ( tubeSrc[1] == NSL_INVALID_ID )
			nslPrintf( "FIXME: MISSING IG_MEDIUM_TUBE SOUND\n" );
	}
	else
	{
		muffleSrc = nslLoadSource( "IG_LARGE_TUBE" );
		if ( tubeSrc[1] == NSL_INVALID_ID )
			nslPrintf( "FIXME: MISSING IG_LARGE_TUBE SOUND\n" );
	}
  muffleVolume = 0.0;




  // Just the generic sound of underwater
	underwaterSnd1 = NSL_INVALID_ID;
	underwaterSnd2 = NSL_INVALID_ID;

  /*underwaterSrc = nslLoadSource( "WAVE_UNDERWATER01" );
  if ( underwaterSrc == NSL_INVALID_ID )
    nslPrintf( "FIXME: MISSING WAVE_UNDERWATER01 SOUND\n" );
  else
  {
    underwaterSnd = nslAddSound( underwaterSrc );
    if ( nslGetSoundStatus( underwaterSnd ) != NSL_SOUNDSTATUS_INVALID )
    {
      underWaterModifier = .1f;
      nslPlaySound( underwaterSnd );
    }
  }*/
	/*waterLapSnd = NSL_INVALID_ID;
  waterLapSrc = nslLoadSource( "WATER_LAP" );
  if ( waterLapSrc == NSL_INVALID_ID )
    nslPrintf( "FIXME: MISSING WATER_LAP SOUND\n" );


  if ( waterLapSrc != NSL_INVALID_ID )
  {
    waterLapSnd = nslAddSound( waterLapSrc );
    if ( nslGetSoundStatus( waterLapSnd ) != NSL_SOUNDSTATUS_INVALID )
    {
      lapModifier = 0.4f;
      //nslSetSoundParam( waterLapSnd, NSL_SOUNDPARAM_VOLUME, .4 );
      nslPlaySound( waterLapSnd );
    }
  }*/

	
	behindTheCamera     = nslCreateEmitter(pos);
}

void WaveSound::OnNewWave( )	// named to respond to global search on "OnNewWave" (dc 02/19/02) 
{ 
  int current_wave = WAVE_GetIndex ( );

  // Grab some info from the WaveDataArray

  faceVolume        = WaveDataArray[current_wave].faceVolume;
  tubeVolume        = WaveDataArray[current_wave].tubeVolume;
  foamVolume        = WaveDataArray[current_wave].foamVolume;
  waveCrashVolume   = WaveDataArray[current_wave].waveCrashVolume;
  underWaterVolume  = 1;

  lastTubePieces    = 1;

  tubeMin       = WaveDataArray[current_wave].tubeMin;
  tubeMax       = WaveDataArray[current_wave].tubeMax;
  foamMin       = WaveDataArray[current_wave].foamMin;
  foamMax       = WaveDataArray[current_wave].foamMax;
  faceMin       = WaveDataArray[current_wave].faceMin;
  faceMax       = WaveDataArray[current_wave].faceMax;
  waveCrashMin  = WaveDataArray[current_wave].waveCrashMin;
  waveCrashMax  = WaveDataArray[current_wave].waveCrashMax;
}

void WaveSound::pause( ) 
{
  is_paused = true;  
  

}

void WaveSound::unpause( ) 
{
  is_paused = false;  
  

}

void WaveSound::mute( ) 
{
  
  
}

void WaveSound::unmute( ) 
{
  
}

#ifdef DEBUG	// On XBox nglSphereMesh exists only in DEBUG
void draw_sphere( const vector3d &pos, rational_t radius, float color[4] )
{
	extern nglMesh *nglSphereMesh;
	nglRenderParams Params;
	nglMatrix Work;
	nglVector v1;//, v2;

	nglVector SphereColor(color[0], color[1], color[2], color[3]);
	nglSphereMesh->Sections[0].Material[0].Flags        |= NGLMAT_ALPHA;
	nglSphereMesh->Sections[0].Material[0].MapBlendMode  = NGLBM_BLEND;
	Params.Flags = NGLP_TINT | NGLP_SCALE;
  
	Params.TintColor = SphereColor;

	Params.Scale[0] = Params.Scale[1] = Params.Scale[2] = radius;
	
	nglIdentityMatrix( Work );
	
	v1[0] = pos.x;
	v1[1] = pos.y;
	v1[2] = pos.z;
	v1[3] = 1;
	
	KSNGL_TranslateMatrix( Work, Work, v1 );
	nglListAddMesh( nglSphereMesh, Work, &Params );

}


void WaveSound::drawSoundPoints( ) 
{
  vector3d pos;
  int i;
  float color[4];

  color[0] = 1; color[1] = 0; color[2] = 0.0f; color[3] = .7f;
  for ( i =0; i < numFoamSounds; i++ )
  {
    int evenOrOdd = i%2;
    int index     = i/2;
    pos.x = foamPos[evenOrOdd][index][0]; pos.y = foamPos[evenOrOdd][index][1]; pos.z = foamPos[evenOrOdd][index][2]; 
    draw_sphere( pos, foamMin, color );
  }
  color[0] = 0.0f; color[1] = 1.0f; color[2] = 0.0f; color[3] = 0.7f;
  
  for ( i =0; i < numTubeSounds; i++ )
  {
    int evenOrOdd = i%2;
    int index     = i/2;
    pos.x = tubePos[evenOrOdd][index][0]; pos.y = tubePos[evenOrOdd][index][1]; pos.z = tubePos[evenOrOdd][index][2]; 
    draw_sphere( pos, tubeMin, color );
  }

  
  color[0] = 0.0f; color[1] = 0; color[2] = 1.0f; color[3] = .7f;
  for ( i =0; i < numFaceSounds; i++ )
  {
    int evenOrOdd = i%2;
    int index     = i/2;
    pos.x = facePos[evenOrOdd][index][0]; pos.y = facePos[evenOrOdd][index][1]; pos.z = facePos[evenOrOdd][index][2]; 
    draw_sphere( pos, faceMin, color );
  }
}
#endif

void WaveSound::SetFaceEmitterPositions( )
{

  int i,  targetSounds;
//  float delta;
  vector3d deltav, midSecondFace;//, v;
  float dist = 0.0f;
  float mainFaceLength;
  float secondaryFaceLength = 0;
  int secondaryFaceSounds, mainFaceSounds;

  // Calc the length of the breaks
  if ( !waveDir )
  {
    if ( WAVE_SoundEmitter[WAVE_SE_FACE].numsegment== 1 )
    {
      mainFaceLength      = ( WAVE_SoundEmitter[WAVE_SE_FACE].segment[0].start - WAVE_SoundEmitter[WAVE_SE_FACE].segment[0].stop ).length( );
    }
    else if ( WAVE_SoundEmitter[WAVE_SE_FACE].numsegment== 2 )
    {
      mainFaceLength      = ( WAVE_SoundEmitter[WAVE_SE_FACE].segment[1].start - WAVE_SoundEmitter[WAVE_SE_FACE].segment[1].stop ).length( );
      secondaryFaceLength = ( WAVE_SoundEmitter[WAVE_SE_FACE].segment[0].start - WAVE_SoundEmitter[WAVE_SE_FACE].segment[0].stop ).length( );
    }
    else 
    {
      mainFaceLength = secondaryFaceLength = 0;
    }
    
  }
  else
  {
    if ( WAVE_SoundEmitter[WAVE_SE_FACE].numsegment > 0 )
    {
      mainFaceLength          = ( WAVE_SoundEmitter[WAVE_SE_FACE].segment[0].start - WAVE_SoundEmitter[WAVE_SE_FACE].segment[0].stop ).length( );
      if ( WAVE_SoundEmitter[WAVE_SE_FACE].numsegment == 2 )
        secondaryFaceLength   = ( WAVE_SoundEmitter[WAVE_SE_FACE].segment[1].start - WAVE_SoundEmitter[WAVE_SE_FACE].segment[1].stop ).length( );
    }
    else
    {
      mainFaceLength = secondaryFaceLength = 0;
    }
  }

  // Calculate how many sounds we need
  if ( WAVE_SoundEmitter[WAVE_SE_FACE].numsegment == 1 ) 
    dist = mainFaceLength;
  else if ( WAVE_SoundEmitter[WAVE_SE_FACE].numsegment == 2 ) 
    dist = mainFaceLength + secondaryFaceLength;

  
  float totalDist;
  if ( !waveDir )
  {
    if ( WAVE_SoundEmitter[WAVE_SE_FACE].numsegment == 2 )
      totalDist = ( WAVE_SoundEmitter[WAVE_SE_TUBE].segment[0].start - WAVE_SoundEmitter[WAVE_SE_FACE].segment[1].stop ).length( );
    else if ( WAVE_SoundEmitter[WAVE_SE_FACE].numsegment == 1 )
      totalDist = ( WAVE_SoundEmitter[WAVE_SE_TUBE].segment[0].start - WAVE_SoundEmitter[WAVE_SE_FACE].segment[0].stop ).length( );
    else if ( WAVE_SoundEmitter[WAVE_SE_FACE].numsegment == 0 )
      totalDist = ( WAVE_SoundEmitter[WAVE_SE_TUBE].segment[0].start - WAVE_SoundEmitter[WAVE_SE_TUBE].segment[0].stop ).length( );
	else
		{ assert(0); return; }	// totalDist was uninitialized (dc 01/29/02)
  }
  else
  {
    if ( WAVE_SoundEmitter[WAVE_SE_TUBE].numsegment == 1 )
      totalDist = ( WAVE_SoundEmitter[WAVE_SE_FACE].segment[0].start - WAVE_SoundEmitter[WAVE_SE_TUBE].segment[0].stop ).length( );
    else if ( WAVE_SoundEmitter[WAVE_SE_TUBE].numsegment == 2 )
      totalDist = ( WAVE_SoundEmitter[WAVE_SE_FACE].segment[0].start - WAVE_SoundEmitter[WAVE_SE_TUBE].segment[1].stop ).length( );
    else if ( WAVE_SoundEmitter[WAVE_SE_TUBE].numsegment == 0 )
      totalDist = ( WAVE_SoundEmitter[WAVE_SE_FACE].segment[0].start - WAVE_SoundEmitter[WAVE_SE_FACE].segment[0].stop ).length( );
	else
		{ assert(0); return; }	// totalDist was uninitialized (dc 01/29/02)
  }

  if ( totalDist == 0 )
    targetSounds = 0;
  else
    targetSounds = ( int )( 8.0f * mainFaceLength/totalDist );

  if ( targetSounds > MAX_REGION_SOUNDS )
  {
    targetSounds = MAX_REGION_SOUNDS;
  }
  if (( WAVE_SoundEmitter[WAVE_SE_FACE].numsegment == 2 ) && (targetSounds < MAX_REGION_SOUNDS))
    targetSounds++;

 
  
  // The size of the face can only get smaller in the long run.  
  if ( numFaceSounds > targetSounds )
  {
    int toDrop =  numFaceSounds - targetSounds;
    for ( i =toDrop - 1; i >=0; i-- )
    {
      int oddOrEven = ( targetSounds + i )%2;
      int newSlot = ( targetSounds + i )/2;

      nslStopSound( faceSnd[oddOrEven][newSlot] );
      faceSnd[oddOrEven][newSlot] = NSL_INVALID_ID;
      facePos[oddOrEven][newSlot][0] = -9999;
      numFaceSounds--;
    }    
  }
  else if ( ( numFaceSounds == -1 ) && ( WAVE_SoundEmitter[WAVE_SE_FACE].numsegment > 0 ) )
  {
    numFaceSounds = 0;
    int toAdd = targetSounds;
    for ( i =0; i < toAdd; i++ )
    {
      int oddOrEven = ( targetSounds - toAdd + i )%2;
      int newSlot = ( targetSounds - toAdd + i )/2;
      if (faceSrc[oddOrEven] != NSL_INVALID_ID)
      {
        faceSnd[oddOrEven][newSlot] =  nslAddSound( faceSrc[oddOrEven] );
        if ( faceSnd[oddOrEven][newSlot] != NSL_INVALID_ID ) 
        {
          nslSetSoundEmitter( faceEmitters[oddOrEven][newSlot], faceSnd[oddOrEven][newSlot] );
          nslSetSoundParam( faceSnd[oddOrEven][newSlot], NSL_SOUNDPARAM_VOLUME, 0);
          nslPlaySound( faceSnd[oddOrEven][newSlot] );
        }
        numFaceSounds++;
      }
    }
  }
  else if ( ( numFaceSounds < targetSounds ) && ( numFaceSounds != -1 ) )
  {
    int toAdd = targetSounds - numFaceSounds;
    for ( i = 0; i < toAdd; i++ )
    {
      int oddOrEven = ( targetSounds - toAdd + i )%2;
      int newSlot = ( targetSounds - toAdd + i )/2;
      if (faceSrc[oddOrEven] != NSL_INVALID_ID)
      {
        faceSnd[oddOrEven][newSlot] = nslAddSound( faceSrc[oddOrEven] );
        if (faceSnd[oddOrEven][newSlot] != NSL_INVALID_ID)
        {
          nslSetSoundEmitter( faceEmitters[oddOrEven][newSlot], faceSnd[oddOrEven][newSlot] );
          nslSetSoundParam( faceSnd[oddOrEven][newSlot], NSL_SOUNDPARAM_VOLUME, 0);
          nslPlaySound( faceSnd[oddOrEven][newSlot] );
        }
        numFaceSounds++;
      }
    }   
    
  }
  else 
  {
    targetSounds = numFaceSounds;
  }

  
  if ( WAVE_SoundEmitter[WAVE_SE_FACE].numsegment <= 1 ) 
    secondaryFaceSounds = 0;
  else
   secondaryFaceSounds = 1;
  if (numFaceSounds > 0)
    mainFaceSounds = numFaceSounds - secondaryFaceSounds;
  else
    mainFaceSounds = 0;


  // Now place the sounds

  // Left break
  if ( waveDir )
  {
   
    deltav = WAVE_SoundEmitter[WAVE_SE_FACE].segment[0].start - WAVE_SoundEmitter[WAVE_SE_FACE].segment[0].stop;
    deltav.normalize( );
    deltav *= 2*faceMin;
    for ( i=0; i < mainFaceSounds; i++ )
    {
      int oddOrEven = ( i )%2;
      int newSlot = ( i )/2;
      float diff;
      vector3d oldPos, newPos;
      oldPos.x = facePos[oddOrEven][newSlot][0];
      oldPos.y = facePos[oddOrEven][newSlot][1];
      oldPos.z = facePos[oddOrEven][newSlot][2];
      newPos = WAVE_SoundEmitter[WAVE_SE_FACE].segment[0].stop + deltav/2 + deltav*i;

      // If its not a new sound, don't let it move by more that 1 meter
      if ( oldPos.x != -9999 )
      {
        diff = ( newPos - oldPos ).length( );
        if ( diff > 2 )
        { 
          newPos = oldPos + ( newPos - oldPos )/diff;
        }
      }
      facePos[oddOrEven][newSlot][0] = newPos.x;
      facePos[oddOrEven][newSlot][1] = newPos.y;
      facePos[oddOrEven][newSlot][2] = newPos.z;

      nslSetEmitterPosition( faceEmitters[oddOrEven][newSlot], facePos[oddOrEven][newSlot] );
    }

    // Breakage
    if ( WAVE_SoundEmitter[WAVE_SE_FACE].numsegment == 2 ) 
    {
      // We fill in the second face from the center out
      for ( i=0; i < secondaryFaceSounds; i++ )
      {
        int oddOrEven = ( mainFaceSounds + i )%2;
        int newSlot = ( mainFaceSounds + i )/2;
        float diff;
        vector3d oldPos, newPos;
        
        deltav = WAVE_SoundEmitter[WAVE_SE_FACE].segment[1].start - WAVE_SoundEmitter[WAVE_SE_FACE].segment[1].stop;
        deltav.normalize( );
        midSecondFace =  deltav;
        deltav        *= 2*faceMin;
        midSecondFace *= secondaryFaceLength /2;
        midSecondFace += WAVE_SoundEmitter[WAVE_SE_FACE].segment[1].stop;


        oldPos.x = facePos[oddOrEven][newSlot][0];
        oldPos.y = facePos[oddOrEven][newSlot][1];
        oldPos.z = facePos[oddOrEven][newSlot][2];
        newPos = midSecondFace + ( i%2?deltav*( ( int )( i+.5f )/2 ):deltav*i*( int )( i+.5f )/-2 );

        // If its not a new sound, don't let it move by more that 1 meter
        if ( oldPos.x != -9999 )
        {
          diff = ( newPos - oldPos ).length( );
          if ( diff > 2 )
          { 
            newPos = oldPos + ( newPos - oldPos )/diff;
          }
        }
        facePos[oddOrEven][newSlot][0] = newPos.x;
        facePos[oddOrEven][newSlot][1] = newPos.y;
        facePos[oddOrEven][newSlot][2] = newPos.z;

        nslSetEmitterPosition( faceEmitters[oddOrEven][newSlot], facePos[oddOrEven][newSlot] );
      }
    }
  }
  else  // Right Break
  {

    // One, continuous face
    if ( WAVE_SoundEmitter[WAVE_SE_FACE].numsegment == 1 ) 
    {
      deltav = WAVE_SoundEmitter[WAVE_SE_FACE].segment[0].stop - WAVE_SoundEmitter[WAVE_SE_FACE].segment[0].start;
      deltav.normalize( );
      deltav *= 2*faceMin;
      for ( i=0; i < mainFaceSounds; i++ )
      {
        int oddOrEven = ( i )%2;
        int newSlot = ( i )/2;
        float diff;
        vector3d oldPos, newPos;

        oldPos.x = facePos[oddOrEven][newSlot][0];
        oldPos.y = facePos[oddOrEven][newSlot][1];
        oldPos.z = facePos[oddOrEven][newSlot][2];
        newPos = WAVE_SoundEmitter[WAVE_SE_FACE].segment[0].start + deltav/2 + deltav*i;

        // If its not a new sound, don't let it move by more that 1 meter
        if ( oldPos.x != -9999 )
        {
          diff = ( newPos - oldPos ).length( );
          if ( diff > 2 )
          { 
            newPos = oldPos + ( newPos - oldPos )/diff;
          }
        }
        facePos[oddOrEven][newSlot][0] = newPos.x;
        facePos[oddOrEven][newSlot][1] = newPos.y;
        facePos[oddOrEven][newSlot][2] = newPos.z;

        nslSetEmitterPosition( faceEmitters[oddOrEven][newSlot], facePos[oddOrEven][newSlot] );
      }
    }
    // Breakage
    else if ( WAVE_SoundEmitter[WAVE_SE_FACE].numsegment == 2 ) 
    {
      deltav = WAVE_SoundEmitter[WAVE_SE_FACE].segment[1].stop - WAVE_SoundEmitter[WAVE_SE_FACE].segment[1].start;
      deltav.normalize( );
      deltav *= 2*faceMin;
      for ( i=0; i < mainFaceSounds; i++ )
      {
        int oddOrEven = ( i )%2;
        int newSlot = ( i )/2;
        float diff;
        vector3d oldPos, newPos;

        oldPos.x = facePos[oddOrEven][newSlot][0];
        oldPos.y = facePos[oddOrEven][newSlot][1];
        oldPos.z = facePos[oddOrEven][newSlot][2];
        newPos = WAVE_SoundEmitter[WAVE_SE_FACE].segment[1].start + deltav/2 + deltav*i;

        // If its not a new sound, don't let it move by more that 1 meter
        if ( oldPos.x != -9999 )
        {
          diff = ( newPos - oldPos ).length( );
          if ( diff > 2 )
          { 
            newPos = oldPos + ( newPos - oldPos )/diff;
          }
        }
        facePos[oddOrEven][newSlot][0] = newPos.x;
        facePos[oddOrEven][newSlot][1] = newPos.y;
        facePos[oddOrEven][newSlot][2] = newPos.z;

        nslSetEmitterPosition( faceEmitters[oddOrEven][newSlot], facePos[oddOrEven][newSlot] );
      }


      // We fill in the second face from the center out
      for ( i=0; i < secondaryFaceSounds; i++ )
      {
        int oddOrEven = ( mainFaceSounds + i )%2;
        int newSlot = ( mainFaceSounds + i )/2;
        float diff;
        vector3d oldPos, newPos; 

        deltav = WAVE_SoundEmitter[WAVE_SE_FACE].segment[0].start - WAVE_SoundEmitter[WAVE_SE_FACE].segment[0].stop;
        deltav.normalize( );
        midSecondFace =  deltav;
        deltav        *= 2*faceMin;
        midSecondFace *= secondaryFaceLength /2;
        midSecondFace += WAVE_SoundEmitter[WAVE_SE_FACE].segment[0].stop;

        oldPos.x = facePos[oddOrEven][newSlot][0];
        oldPos.y = facePos[oddOrEven][newSlot][1];
        oldPos.z = facePos[oddOrEven][newSlot][2];
        newPos = midSecondFace + ( i%2?deltav*( ( int )( i+.5f )/2 ):deltav*i*( int )( i+.5f )/-2 );

        // If its not a new sound, don't let it move by more that 1 meter
        if ( oldPos.x != -9999 )
        {
          diff = ( newPos - oldPos ).length( );
          if ( diff > 2 )
          { 
            newPos = oldPos + ( newPos - oldPos )/diff;
          }
        }
        facePos[oddOrEven][newSlot][0] = newPos.x;
        facePos[oddOrEven][newSlot][1] = newPos.y;
        facePos[oddOrEven][newSlot][2] = newPos.z;
        nslSetEmitterPosition( faceEmitters[oddOrEven][newSlot], facePos[oddOrEven][newSlot] );
      }


    }


  }


  
}


void WaveSound::SetTubeEmitterPositions( )
{
  int i,  targetSounds;
//  float delta;
  vector3d deltav, midSecondTube;//, v;
//  float dist = 0.0f;
  float mainTubeLength;
  float secondaryTubeLength = 0;
  int secondaryTubeSounds, mainTubeSounds;

  // Calc the length of the breaks
  if ( !waveDir )
  {
    if ( WAVE_SoundEmitter[WAVE_SE_TUBE].numsegment > 0 )
    {
      mainTubeLength = ( WAVE_SoundEmitter[WAVE_SE_TUBE].segment[0].start - WAVE_SoundEmitter[WAVE_SE_TUBE].segment[0].stop ).length( );
      if ( WAVE_SoundEmitter[WAVE_SE_TUBE].numsegment==2 )
        secondaryTubeLength = ( WAVE_SoundEmitter[WAVE_SE_TUBE].segment[1].start - WAVE_SoundEmitter[WAVE_SE_TUBE].segment[1].stop ).length( );
    }
    else
    { 
      mainTubeLength = secondaryTubeLength = 0;
    }
  }
  else
  {
    if ( WAVE_SoundEmitter[WAVE_SE_TUBE].numsegment== 1 )
    {
      mainTubeLength = ( WAVE_SoundEmitter[WAVE_SE_TUBE].segment[0].start - WAVE_SoundEmitter[WAVE_SE_TUBE].segment[0].stop ).length( );
    }
    else if ( WAVE_SoundEmitter[WAVE_SE_TUBE].numsegment== 2 )
    {
      mainTubeLength = ( WAVE_SoundEmitter[WAVE_SE_TUBE].segment[1].start - WAVE_SoundEmitter[WAVE_SE_TUBE].segment[1].stop ).length( );
      secondaryTubeLength = ( WAVE_SoundEmitter[WAVE_SE_TUBE].segment[0].start - WAVE_SoundEmitter[WAVE_SE_TUBE].segment[0].stop ).length( );
    }
    else 
    {
      mainTubeLength = secondaryTubeLength = 0;
    }
  }

  float totalDist;
  if ( !waveDir )
  {
    if ( WAVE_SoundEmitter[WAVE_SE_FACE].numsegment == 2 )
      totalDist = ( WAVE_SoundEmitter[WAVE_SE_TUBE].segment[0].start - WAVE_SoundEmitter[WAVE_SE_FACE].segment[1].stop ).length( );
    else if ( WAVE_SoundEmitter[WAVE_SE_FACE].numsegment == 1 )
      totalDist = ( WAVE_SoundEmitter[WAVE_SE_TUBE].segment[0].start - WAVE_SoundEmitter[WAVE_SE_FACE].segment[0].stop ).length( );
    else if ( WAVE_SoundEmitter[WAVE_SE_FACE].numsegment == 0 )
      totalDist = ( WAVE_SoundEmitter[WAVE_SE_TUBE].segment[0].start - WAVE_SoundEmitter[WAVE_SE_TUBE].segment[0].stop ).length( );
	else
		{ assert(0); return; }	// totalDist was uninitialized (dc 01/29/02)
  }
  else
  {
    if ( WAVE_SoundEmitter[WAVE_SE_TUBE].numsegment == 1 )
      totalDist = ( WAVE_SoundEmitter[WAVE_SE_FACE].segment[0].start - WAVE_SoundEmitter[WAVE_SE_TUBE].segment[0].stop ).length( );
    else if ( WAVE_SoundEmitter[WAVE_SE_TUBE].numsegment == 2 )
      totalDist = ( WAVE_SoundEmitter[WAVE_SE_FACE].segment[0].start - WAVE_SoundEmitter[WAVE_SE_TUBE].segment[1].stop ).length( );
    else if ( WAVE_SoundEmitter[WAVE_SE_TUBE].numsegment == 0 )
      totalDist = ( WAVE_SoundEmitter[WAVE_SE_FACE].segment[0].start - WAVE_SoundEmitter[WAVE_SE_FACE].segment[0].stop ).length( );
	else
		{ assert(0); return; }	// totalDist was uninitialized (dc 01/29/02)
  }
      

  if ( totalDist == 0 )
    targetSounds = 0;
  else
    targetSounds = ( int )( 8.0f * mainTubeLength/totalDist );
  
  if ( targetSounds > MAX_REGION_SOUNDS )
  {

    targetSounds = MAX_REGION_SOUNDS;
  }

  if (( WAVE_SoundEmitter[WAVE_SE_TUBE].numsegment == 2 ) && (targetSounds < MAX_REGION_SOUNDS))
    targetSounds++;
  
  


  // Add and drop sounds
  if ( numTubeSounds < targetSounds ) 
  {
    int toAdd = targetSounds - numTubeSounds;
    for ( i =0; i < toAdd; i++ )
    {
      int oddOrEven = ( targetSounds - toAdd + i )%2;
      int newSlot = ( targetSounds - toAdd + i )/2;

      if (tubeSrc[oddOrEven] != NSL_INVALID_ID)
      {
        tubeSnd[oddOrEven][newSlot] =  nslAddSound( tubeSrc[oddOrEven] );
        if ( tubeSnd[oddOrEven][newSlot] != NSL_INVALID_ID ) 
        {
          nslSetSoundParam( tubeSnd[oddOrEven][newSlot], NSL_SOUNDPARAM_VOLUME, 0);
          nslSetSoundEmitter( tubeEmitters[oddOrEven][newSlot], tubeSnd[oddOrEven][newSlot] );
          nslPlaySound( tubeSnd[oddOrEven][newSlot] );
        }
        numTubeSounds++;
      }
    }
  } 
  else
  {
    // Never decrease the number of sounds for the tube.
    targetSounds = numTubeSounds;
  }
  /*else if ( numTubeSounds > targetSounds )
  {
    int toDrop =  numTubeSounds - targetSounds;
    for ( i =toDrop - 1; i >=0; i-- )
    {
      int oddOrEven = ( targetSounds + i )%2;
      int newSlot = ( targetSounds + i )/2;

      nslStopSound( tubeSnd[oddOrEven][newSlot] );
      tubeSnd[oddOrEven][newSlot] = NSL_INVALID_ID;
      tubePos[oddOrEven][newSlot][0] = -9999;
      numTubeSounds--;
    }    
  }*/


  if ((muffleSound == NSL_INVALID_ID) && (muffleSrc != NSL_INVALID_ID))
  {
    muffleSound = nslAddSound(muffleSrc);
    if (muffleSound != NSL_INVALID_ID)
    {
      nslSetSoundParam(muffleSound, NSL_SOUNDPARAM_VOLUME, 0.0);    
      nslPlaySound(muffleSound);
    }
  }
    


  if ( WAVE_SoundEmitter[WAVE_SE_TUBE].numsegment <= 1 ) 
    secondaryTubeSounds = 0;
  else
    secondaryTubeSounds = 1;
  if (numTubeSounds > 0)
    mainTubeSounds = numTubeSounds - secondaryTubeSounds;
  else
    mainTubeSounds = 0;


  // Now place the sounds

  // Right break
  if ( !waveDir )
  {


    deltav = WAVE_SoundEmitter[WAVE_SE_TUBE].segment[0].start - WAVE_SoundEmitter[WAVE_SE_TUBE].segment[0].stop;
    deltav.normalize( );
    deltav *= 2*tubeMin;
    for ( i=0; i < mainTubeSounds; i++ )
    {
      int oddOrEven = ( i )%2;
      int newSlot = ( i )/2;
      vector3d oldPos, newPos;
//      float diff;
      
      oldPos.x = tubePos[oddOrEven][newSlot][0];
      oldPos.y = tubePos[oddOrEven][newSlot][1];
      oldPos.z = tubePos[oddOrEven][newSlot][2];
      newPos = WAVE_SoundEmitter[WAVE_SE_TUBE].segment[0].stop + deltav/2 + deltav*i;
      /*if ( oldPos.x != -9999 )
      {
        diff = ( newPos - oldPos ).length( );
        if ( diff > 2 )
        { 
          newPos = oldPos + ( newPos - oldPos )/diff;
        }
      }*/
      tubePos[oddOrEven][newSlot][0] = newPos.x;
      tubePos[oddOrEven][newSlot][1] = newPos.y;
      tubePos[oddOrEven][newSlot][2] = newPos.z;
      nslSetEmitterPosition( tubeEmitters[oddOrEven][newSlot], tubePos[oddOrEven][newSlot] );
    }

    // Breakage
    if ( WAVE_SoundEmitter[WAVE_SE_TUBE].numsegment == 2 ) 
    {
      // We fill in the second tube from the center out
      for ( i=0; i < secondaryTubeSounds; i++ )
      {
        int oddOrEven = ( mainTubeSounds + i )%2;
        int newSlot = ( mainTubeSounds + i )/2;
        vector3d oldPos, newPos;
//        float diff;

        deltav = WAVE_SoundEmitter[WAVE_SE_TUBE].segment[1].start - WAVE_SoundEmitter[WAVE_SE_TUBE].segment[1].stop;
        deltav.normalize( );
        midSecondTube =  deltav;
        deltav        *= 2*tubeMin;
        midSecondTube *= secondaryTubeLength /2;
        midSecondTube += WAVE_SoundEmitter[WAVE_SE_TUBE].segment[1].stop;
        oldPos.x = tubePos[oddOrEven][newSlot][0];
        oldPos.y = tubePos[oddOrEven][newSlot][1];
        oldPos.z = tubePos[oddOrEven][newSlot][2];
        newPos = midSecondTube + ( i%2?deltav*( ( int )( i+.5 )/2 ):deltav*i*( int )( i+.5 )/-2 );

        // If its not a new sound, don't let it move by more that 1 meter
       /* if ( oldPos.x != -9999 )
        {
          diff = ( newPos - oldPos ).length( );
          if ( diff > 2 )
          { 
            newPos = oldPos + ( newPos - oldPos )/diff;
          }
        }*/
        tubePos[oddOrEven][newSlot][0] = newPos.x;
        tubePos[oddOrEven][newSlot][1] = newPos.y;
        tubePos[oddOrEven][newSlot][2] = newPos.z;

        
        nslSetEmitterPosition( tubeEmitters[oddOrEven][newSlot], tubePos[oddOrEven][newSlot] );
      }
    }
  }
  else  // Left Break
  {

    // One, continuous tube
    if ( WAVE_SoundEmitter[WAVE_SE_TUBE].numsegment == 1 ) 
    {
      deltav = WAVE_SoundEmitter[WAVE_SE_TUBE].segment[0].stop - WAVE_SoundEmitter[WAVE_SE_TUBE].segment[0].start;
      deltav.normalize( );
      deltav *= 2*tubeMin;
      for ( i=0; i < mainTubeSounds; i++ )
      {
        int oddOrEven = ( i )%2;
        int newSlot = ( i )/2;
        vector3d oldPos, newPos;
//        float diff;

        oldPos.x = tubePos[oddOrEven][newSlot][0];
        oldPos.y = tubePos[oddOrEven][newSlot][1];
        oldPos.z = tubePos[oddOrEven][newSlot][2];
        newPos = WAVE_SoundEmitter[WAVE_SE_TUBE].segment[0].start + deltav/2 + deltav*i;

       /* // If its not a new sound, don't let it move by more that 1 meter
        if ( oldPos.x != -9999 )
        {
          diff = ( newPos - oldPos ).length( );
          if ( diff > 2 )
          { 
            newPos = oldPos + ( newPos - oldPos )/diff;
          }
        }*/
        tubePos[oddOrEven][newSlot][0] = newPos.x;
        tubePos[oddOrEven][newSlot][1] = newPos.y;
        tubePos[oddOrEven][newSlot][2] = newPos.z;
        nslSetEmitterPosition( tubeEmitters[oddOrEven][newSlot], tubePos[oddOrEven][newSlot] );
      }
    }
    // Breakage
    else if ( WAVE_SoundEmitter[WAVE_SE_TUBE].numsegment == 2 ) 
    {
      deltav = WAVE_SoundEmitter[WAVE_SE_TUBE].segment[1].stop - WAVE_SoundEmitter[WAVE_SE_TUBE].segment[1].start;
      deltav.normalize( );
      deltav *= 2*tubeMin;
      for ( i=0; i < mainTubeSounds; i++ )
      {
        int oddOrEven = ( i )%2;
        int newSlot = ( i )/2;
        vector3d oldPos, newPos;
//        float diff;

        oldPos.x = tubePos[oddOrEven][newSlot][0];
        oldPos.y = tubePos[oddOrEven][newSlot][1];
        oldPos.z = tubePos[oddOrEven][newSlot][2];
        newPos = WAVE_SoundEmitter[WAVE_SE_TUBE].segment[1].start + deltav/2 + deltav*i;

        // If its not a new sound, don't let it move by more that 1 meter
       /* if ( oldPos.x != -9999 )
        {
          diff = ( newPos - oldPos ).length( );
          if ( diff > 2 )
          { 
            newPos = oldPos + ( newPos - oldPos )/diff;
          }
        }*/
        tubePos[oddOrEven][newSlot][0] = newPos.x;
        tubePos[oddOrEven][newSlot][1] = newPos.y;
        tubePos[oddOrEven][newSlot][2] = newPos.z;

        nslSetEmitterPosition( tubeEmitters[oddOrEven][newSlot], tubePos[oddOrEven][newSlot] );
      }


      // We fill in the second tube from the center out
      for ( i=0; i < secondaryTubeSounds; i++ )
      {
        int oddOrEven = ( mainTubeSounds + i )%2;
        int newSlot = ( mainTubeSounds + i )/2;
        vector3d oldPos, newPos;
//        float diff;

        deltav = WAVE_SoundEmitter[WAVE_SE_TUBE].segment[0].start - WAVE_SoundEmitter[WAVE_SE_TUBE].segment[0].stop;
        deltav.normalize( );
        midSecondTube =  deltav;
        deltav        *= 2*tubeMin;
        midSecondTube *= secondaryTubeLength /2;
        midSecondTube += WAVE_SoundEmitter[WAVE_SE_TUBE].segment[0].stop;

        oldPos.x = tubePos[oddOrEven][newSlot][0];
        oldPos.y = tubePos[oddOrEven][newSlot][1];
        oldPos.z = tubePos[oddOrEven][newSlot][2];
        newPos = midSecondTube + ( i%2?deltav*( ( int )( i+.5 )/2 ):deltav*i*( int )( i+.5 )/-2 );

        // If its not a new sound, don't let it move by more that 1 meter
     /*   if ( oldPos.x != -9999 )
        {
          diff = ( newPos - oldPos ).length( );
          if ( diff > 2 )
          { 
            newPos = oldPos + ( newPos - oldPos )/diff;
          }
        }*/
        tubePos[oddOrEven][newSlot][0] = newPos.x;
        tubePos[oddOrEven][newSlot][1] = newPos.y;
        tubePos[oddOrEven][newSlot][2] = newPos.z;

        nslSetEmitterPosition( tubeEmitters[oddOrEven][newSlot], tubePos[oddOrEven][newSlot] );
      }


    }


  }




}


void WaveSound::SetFoamEmitterPositions( )
{

  int i,  targetSounds;
//  float delta;
  vector3d deltav;//, midSecondFoam;//, v;
//  float dist = 0.0f;
  float mainFoamLength;
  float secondaryFoamLength = 0;
  int secondaryFoamSounds, mainFoamSounds;

  // Calc the length of the breaks
  if ( !waveDir )
  {
    if (WAVE_SoundEmitter[WAVE_SE_SPLASH].numsegment > 0)
    {
      mainFoamLength        = ( WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[0].start - WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[0].stop ).length( );
      if ( WAVE_SoundEmitter[WAVE_SE_SPLASH].numsegment==2 )
        secondaryFoamLength = ( WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[1].start - WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[1].stop ).length( );
    }
    else
    {
      mainFoamLength = secondaryFoamLength = 0;
    }
  }
  else
  {
    if ( WAVE_SoundEmitter[WAVE_SE_SPLASH].numsegment== 1 )
    {
      mainFoamLength      = ( WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[0].start - WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[0].stop ).length( );
	  secondaryFoamLength = 0;
    }
    else if ( WAVE_SoundEmitter[WAVE_SE_SPLASH].numsegment== 2 )
    {
      mainFoamLength = ( WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[1].start - WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[1].stop ).length( );
      secondaryFoamLength = ( WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[0].start - WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[0].stop ).length( );
    }
    else 
    {
      mainFoamLength = secondaryFoamLength = 0;
    }

  }


  // Calculate how many sounds we need
  float totalDist;
  if ( !waveDir )  //RIGHT
  {
    if ( WAVE_SoundEmitter[WAVE_SE_FACE].numsegment == 2 )
      totalDist = ( WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[0].start - WAVE_SoundEmitter[WAVE_SE_FACE].segment[1].stop ).length( );
    else if ( WAVE_SoundEmitter[WAVE_SE_FACE].numsegment == 1 )
      totalDist = ( WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[0].start - WAVE_SoundEmitter[WAVE_SE_FACE].segment[0].stop ).length( );
    else if ( WAVE_SoundEmitter[WAVE_SE_FACE].numsegment == 0 )
      totalDist = ( WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[0].start - WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[0].stop ).length( );
	else
		{ assert(0); return; }	// totalDist was uninitialized (dc 01/29/02)
  }
  else  //LEFT BREAK
  {
    if ( WAVE_SoundEmitter[WAVE_SE_TUBE].numsegment == 1 )
      totalDist = ( WAVE_SoundEmitter[WAVE_SE_FACE].segment[0].start - WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[0].stop ).length( );
    else if ( WAVE_SoundEmitter[WAVE_SE_TUBE].numsegment == 2 )
      totalDist = ( WAVE_SoundEmitter[WAVE_SE_FACE].segment[0].start - WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[1].stop ).length( );
    else if ( WAVE_SoundEmitter[WAVE_SE_TUBE].numsegment == 0 )
      totalDist = ( WAVE_SoundEmitter[WAVE_SE_FACE].segment[0].start - WAVE_SoundEmitter[WAVE_SE_FACE].segment[0].stop ).length( );
	else
		{ assert(0); return; }	// totalDist was uninitialized (dc 01/29/02)
  }

  if ( totalDist == 0 )
    targetSounds = 0;
  else
    targetSounds = ( int )( 8.0f * mainFoamLength/totalDist );


  if (targetSounds > MAX_REGION_SOUNDS)
  {

    targetSounds = MAX_REGION_SOUNDS;
  }
    
  if (( WAVE_SoundEmitter[WAVE_SE_SPLASH].numsegment == 2 ) && (targetSounds < MAX_REGION_SOUNDS))
    targetSounds++;


  // Foam will only grow in the long run.  
  if ( numFoamSounds < targetSounds ) 
  {
    int toAdd = targetSounds - numFoamSounds;
    for ( i =0; i < toAdd; i++ )
    {
      int oddOrEven = ( targetSounds - toAdd + i )%2;
      int newSlot = ( targetSounds - toAdd + i )/2;
      if (foamSrc[oddOrEven] != NSL_INVALID_ID)
      {
        foamSnd[oddOrEven][newSlot] =  nslAddSound( foamSrc[oddOrEven] );
        if ( foamSnd[oddOrEven][newSlot] != NSL_INVALID_ID ) 
        {
          nslSetSoundParam( foamSnd[oddOrEven][newSlot], NSL_SOUNDPARAM_VOLUME, 0);
          nslSetSoundEmitter( foamEmitters[oddOrEven][newSlot], foamSnd[oddOrEven][newSlot] );
          nslPlaySound( foamSnd[oddOrEven][newSlot] );
        }
        numFoamSounds++;
      }
    }
  } 

  if ( WAVE_SoundEmitter[WAVE_SE_SPLASH].numsegment <= 1 ) 
    secondaryFoamSounds = 0;
  else
   secondaryFoamSounds = 1;
   
  if (numFoamSounds > 0)
    mainFoamSounds = numFoamSounds - secondaryFoamSounds;
  else
    mainFoamSounds = 0;


  // Now place the sounds

  // Right break
  if ( !waveDir )
  {


    deltav = WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[0].start - WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[0].stop;
    deltav.normalize( );
    deltav *= 2*foamMin;
    for ( i=0; i < mainFoamSounds; i++ )
    {
      int oddOrEven = ( i )%2;
      int newSlot = ( i )/2;
      vector3d oldPos, newPos;
      float diff;

      oldPos.x = foamPos[oddOrEven][newSlot][0];
      oldPos.y = foamPos[oddOrEven][newSlot][1];
      oldPos.z = foamPos[oddOrEven][newSlot][2];
      newPos = WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[0].stop + deltav/2 + deltav*i;

      // If its not a new sound, don't let it move by more that 1 meter
      if ( oldPos.x != -9999 )
      {
        diff = ( newPos - oldPos ).length( );
        if ( diff > 2 )
        { 
          newPos = oldPos + ( newPos - oldPos )/diff;
        }
      }
      foamPos[oddOrEven][newSlot][0] = newPos.x;
      foamPos[oddOrEven][newSlot][1] = newPos.y;
      foamPos[oddOrEven][newSlot][2] = newPos.z;


      nslSetEmitterPosition( foamEmitters[oddOrEven][newSlot], foamPos[oddOrEven][newSlot] );
    }

    // Breakage
   /* if ( WAVE_SoundEmitter[WAVE_SE_SPLASH].numsegment == 2 ) 
    {
      // We fill in the second foam from the center out
      for ( i=0; i < secondaryFoamSounds; i++ )
      {
        int oddOrEven = ( mainFoamSounds + i )%2;
        int newSlot = ( mainFoamSounds + i )/2;
        vector3d oldPos, newPos;
        float diff;
        deltav = WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[1].start - WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[1].stop;
        deltav.normalize( );
        midSecondFoam =  deltav;
        deltav        *= 2*foamMin;
        midSecondFoam *= secondaryFoamLength /2;
        midSecondFoam += WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[1].stop;

        oldPos.x = foamPos[oddOrEven][newSlot][0];
        oldPos.y = foamPos[oddOrEven][newSlot][1];
        oldPos.z = foamPos[oddOrEven][newSlot][2];
        newPos = midSecondFoam + ( i%2?deltav*( ( int )( i+.5 )/2 ):deltav*i*( int )( i+.5 )/-2 );

        // If its not a new sound, don't let it move by more that 1 meter
        if ( oldPos.x != -9999 )
        {
          diff = ( newPos - oldPos ).length( );
          if ( diff > 2 )
          { 
            newPos = oldPos + ( newPos - oldPos )/diff;
          }
        }
        foamPos[oddOrEven][newSlot][0] = newPos.x;
        foamPos[oddOrEven][newSlot][1] = newPos.y;
        foamPos[oddOrEven][newSlot][2] = newPos.z;
        nslSetEmitterPosition( foamEmitters[oddOrEven][newSlot], foamPos[oddOrEven][newSlot] );
      }
    }*/
  }
  else  // Left Break
  {

    // One, continuous foam
    if ( WAVE_SoundEmitter[WAVE_SE_SPLASH].numsegment == 1 ) 
    {
      deltav = WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[0].stop - WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[0].start;
      deltav.normalize( );
      deltav *= 2*foamMin;
      for ( i=0; i < mainFoamSounds; i++ )
      {
        int oddOrEven = ( i )%2;
        int newSlot = ( i )/2;
        vector3d oldPos, newPos;
        float diff;
       
        oldPos.x = foamPos[oddOrEven][newSlot][0];
        oldPos.y = foamPos[oddOrEven][newSlot][1];
        oldPos.z = foamPos[oddOrEven][newSlot][2];
        newPos = WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[0].start + deltav/2 + deltav*i;

        // If its not a new sound, don't let it move by more that 1 meter
        if ( oldPos.x != -9999 )
        {
          diff = ( newPos - oldPos ).length( );
          if ( diff > 2 )
          { 
            newPos = oldPos + ( newPos - oldPos )/diff;
          }
        }
        foamPos[oddOrEven][newSlot][0] = newPos.x;
        foamPos[oddOrEven][newSlot][1] = newPos.y;
        foamPos[oddOrEven][newSlot][2] = newPos.z;
        nslSetEmitterPosition( foamEmitters[oddOrEven][newSlot], foamPos[oddOrEven][newSlot] );
      }
    }
    // Breakage
    else if ( WAVE_SoundEmitter[WAVE_SE_SPLASH].numsegment == 2 ) 
    {
      deltav = WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[1].stop - WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[1].start;
      deltav.normalize( );
      deltav *= 2*foamMin;
      for ( i=0; i < mainFoamSounds; i++ )
      {
        int oddOrEven = ( i )%2;
        int newSlot = ( i )/2;
        vector3d oldPos, newPos;
        float diff;
       
        oldPos.x = foamPos[oddOrEven][newSlot][0];
        oldPos.y = foamPos[oddOrEven][newSlot][1];
        oldPos.z = foamPos[oddOrEven][newSlot][2];
        newPos = WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[1].start + deltav/2 + deltav*i;

        // If its not a new sound, don't let it move by more that 1 meter
        if ( oldPos.x != -9999 )
        {
          diff = ( newPos - oldPos ).length( );
          if ( diff > 2 )
          { 
            newPos = oldPos + ( newPos - oldPos )/diff;
          }
        }
        foamPos[oddOrEven][newSlot][0] = newPos.x;
        foamPos[oddOrEven][newSlot][1] = newPos.y;
        foamPos[oddOrEven][newSlot][2] = newPos.z;

        nslSetEmitterPosition( foamEmitters[oddOrEven][newSlot], foamPos[oddOrEven][newSlot] );
      }


      // We fill in the second foam from the center out
      /*for ( i=0; i < secondaryFoamSounds; i++ )
      {
        int oddOrEven = ( mainFoamSounds + i )%2;
        int newSlot = ( mainFoamSounds + i )/2;
        vector3d oldPos, newPos;
        float diff;
        
        deltav = WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[0].start - WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[0].stop;
        deltav.normalize( );
        midSecondFoam =  deltav;
        deltav        *= 2*foamMin;
        midSecondFoam *= secondaryFoamLength /2;
        midSecondFoam += WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[0].stop;

       
        oldPos.x = foamPos[oddOrEven][newSlot][0];
        oldPos.y = foamPos[oddOrEven][newSlot][1];
        oldPos.z = foamPos[oddOrEven][newSlot][2];
        newPos = midSecondFoam + ( i%2?deltav*( ( int )( i+.5 )/2 ):deltav*i*( int )( i+.5 )/-2 );

        // If its not a new sound, don't let it move by more that 1 meter
        if ( oldPos.x != -9999 )
        {
          diff = ( newPos - oldPos ).length( );
          if ( diff > 2 )
          { 
            newPos = oldPos + ( newPos - oldPos )/diff;
          }
        }
        foamPos[oddOrEven][newSlot][0] = newPos.x;
        foamPos[oddOrEven][newSlot][1] = newPos.y;
        foamPos[oddOrEven][newSlot][2] = newPos.z;

        nslSetEmitterPosition( foamEmitters[oddOrEven][newSlot], foamPos[oddOrEven][newSlot] );
      }*/


    }


  }


}



void WaveSound::shutdown( void ) 
{
  initialized = false;
  crashed = false;
	behindTheCamera = NSL_GLOBAL_EMITTER_ID;
}

void WaveSound::setSoundVolumes( )
{
  int i;
  // The underwater rumble
  if ( nslGetSoundStatus( underwaterSnd1 ) != NSL_SOUNDSTATUS_INVALID )
    nslSetSoundParam( underwaterSnd1, NSL_SOUNDPARAM_VOLUME, underWaterVolume * dampenval );
	if ( nslGetSoundStatus( underwaterSnd2 ) != NSL_SOUNDSTATUS_INVALID )
    nslSetSoundParam( underwaterSnd2, NSL_SOUNDPARAM_VOLUME, underWaterVolume * dampenval);

  if ( nslGetSoundStatus( waveCrashSnd ) != NSL_SOUNDSTATUS_INVALID )
    nslSetSoundParam( waveCrashSnd, NSL_SOUNDPARAM_VOLUME, crashVolume*crashModifier * dampenval );
    
	if ( nslGetSoundStatus( muffleSound ) != NSL_SOUNDSTATUS_INVALID )
    nslSetSoundParam( muffleSound, NSL_SOUNDPARAM_VOLUME, muffleVolume  * dampenval);
  
  
  for ( i = 0; i < numFoamSounds; i++ )
  {
    int oddOrEven = i%2;
    int pos       = i/2;
    if ( nslGetSoundStatus( foamSnd[oddOrEven][pos] ) != NSL_SOUNDSTATUS_INVALID )
    {
      float vol = nslGetSoundParam( foamSnd[oddOrEven][pos], NSL_SOUNDPARAM_VOLUME );
      if (vol < foamVolume*foamModifier*dampenval)
      {
        vol+= .1f * foamVolume*foamModifier * dampenval;
        if (vol > foamVolume*foamModifier*dampenval)
          vol = foamVolume*foamModifier * dampenval;
      }
      else if ( vol > foamVolume*foamModifier)
      {
        vol-= .1f * foamVolume*foamModifier * dampenval;
        if (vol < foamVolume*foamModifier*dampenval)
          vol = foamVolume*foamModifier*dampenval;      
      }
      nslSetSoundParam( foamSnd[oddOrEven][pos], NSL_SOUNDPARAM_VOLUME, vol );
    }
  }


  for ( i = 0; i < numTubeSounds; i++ )
  {
    int oddOrEven = i%2;
    int pos       = i/2;
    if ( nslGetSoundStatus( tubeSnd[oddOrEven][pos] ) != NSL_SOUNDSTATUS_INVALID )
    {
      float vol = nslGetSoundParam( tubeSnd[oddOrEven][pos], NSL_SOUNDPARAM_VOLUME );
      if ( vol < tubeVolume*tubeModifier *dampenval)
      {
        vol+= .1f * tubeVolume*tubeModifier*dampenval;
        if (vol > tubeVolume*tubeModifier*dampenval)
          vol = tubeVolume*tubeModifier*dampenval;
      }
      else if ( vol > tubeVolume*tubeModifier *dampenval)
      {
        vol -= .1f * tubeVolume*tubeModifier*dampenval;
        if (vol < tubeVolume*tubeModifier*dampenval)
          vol = tubeVolume*tubeModifier*dampenval;
      }
      nslSetSoundParam( tubeSnd[oddOrEven][pos], NSL_SOUNDPARAM_VOLUME, vol );
    }
  }
  
  for ( int i=0; i < numFaceSounds; i++ )
  {
    int currentSlot = i/2;
    int oddOrEven = i %2;

    if ( nslGetSoundStatus( faceSnd[oddOrEven][currentSlot] ) != NSL_SOUNDSTATUS_INVALID )
    {
      float vol = nslGetSoundParam( faceSnd[oddOrEven][currentSlot], NSL_SOUNDPARAM_VOLUME );
      if ( vol < faceVolume*tubeModifier *dampenval)
      {
        vol+= .1f * faceVolume*tubeModifier*dampenval;
        if (vol > faceVolume*tubeModifier*dampenval)
          vol = faceVolume*tubeModifier*dampenval;
      }
      else if ( vol < faceVolume*tubeModifier*dampenval )
      {
        vol -= .1f * faceVolume*tubeModifier*dampenval;
        if (vol < faceVolume*tubeModifier*dampenval)
          vol = faceVolume*tubeModifier*dampenval;
      }

      nslSetSoundParam( faceSnd[oddOrEven][currentSlot], NSL_SOUNDPARAM_VOLUME, vol );
    }
  }

  /*if ( nslGetSoundStatus( waterLapSnd ) != NSL_SOUNDSTATUS_INVALID )
    nslSetSoundParam( waterLapSnd, NSL_SOUNDPARAM_VOLUME, waterLapVolume*lapModifier );*/

}


void WaveSound::wentUnderWater( bool under )
{
//  int i=0;
  underwater = under;
  if ( under )
  {
    // The underwater rumble
		underwaterEventFront = SoundScriptManager::inst()->startEvent(SS_UNDERWATER_FRONT, NULL);
		underwaterSnd1	      = SoundScriptManager::inst()->getSoundId(underwaterEventFront);

		underwaterEventFront = SoundScriptManager::inst()->startEvent(SS_UNDERWATER_REAR, NULL);
		underwaterSnd2	      = SoundScriptManager::inst()->getSoundId(underwaterEventFront);
		
		if (nslGetSoundStatus( underwaterSnd1 ) != NSL_SOUNDSTATUS_INVALID)
			nslDampenGuardSound(underwaterSnd1);

		if (nslGetSoundStatus( underwaterSnd2 ) != NSL_SOUNDSTATUS_INVALID)
		{
			nslSetSoundEmitter(behindTheCamera, underwaterSnd2);
			nslDampenGuardSound(underwaterSnd2);
		}

    nslDampenAllSounds(.2f);
  }
  else
  {
		SoundScriptManager::inst()->endEvent(SS_UNDERWATER_FRONT);
		SoundScriptManager::inst()->endEvent(SS_UNDERWATER_REAR);
    nslUndampenAllSounds();
    //underWaterModifier  = 0.1f;
  }
}

 

void WaveSound::frameAdvance( float timeDelta )
{
  nlVector3d  listenerPos, forw, up,behind;
  po cameraPo, surferPo;
	vector3d pos, dir;

  // If we aren't init'ed, just return
  if ( !initialized || is_paused) return;

	// This perspective uses the camera for orientation 
	// but the surfer for position
	// the one exception is that muting based on being underwater
	// or above is based on the camera's position.
  int heroNum = 0;
  cameraPo = app::inst( )->get_game( )->get_current_view_camera( )->get_abs_po( );
  if (!g_game_ptr->is_splitscreen())
  {
    heroNum = g_game_ptr->get_active_player();
    surferPo = g_world_ptr->get_hero_ptr( heroNum )->get_abs_po( );
    memcpy( &listenerPos, &TO_NLVECTOR3D( surferPo.get_position( ) ), sizeof listenerPos );
    memcpy( &up  , &TO_NLVECTOR3D( cameraPo.get_y_facing( ) ), sizeof up );
    memcpy( &forw, &TO_NLVECTOR3D( cameraPo.get_z_facing( ) ), sizeof forw );
  }
  else
  {
		po surfer2Po, btwnPo,
		// Place it between them
    surferPo = g_world_ptr->get_hero_ptr( 0 )->get_abs_po( );
    surfer2Po = g_world_ptr->get_hero_ptr( 0 )->get_abs_po( );
    matrix4x4 p;

    (slerp(surferPo.get_quaternion(), surfer2Po.get_quaternion(), .5)).to_matrix(&p);
    btwnPo = po(p);
    btwnPo.set_position((surferPo.get_position( ) - surfer2Po.get_position())/2 + surfer2Po.get_position());
    memcpy( &listenerPos, &TO_NLVECTOR3D(btwnPo.get_position()), sizeof(nlVector3d));
    memcpy( &up,          &TO_NLVECTOR3D(btwnPo.get_y_facing()), sizeof(nlVector3d));
    memcpy( &forw,        &TO_NLVECTOR3D(btwnPo.get_z_facing()), sizeof(nlVector3d));
  }
  
	// Still deal with the camera for underwater and such
  if ( UNDERWATER_CameraUnderwater( heroNum ) ) 
  {
    if ( !underwater )
      wentUnderWater( true );
  }
  else 
  {
    if ( underwater )
      wentUnderWater( false );
  }
	
	// A little stuff to get an emitter behind the surfer
	pos.x = listenerPos[0]; pos.y = listenerPos[1]; pos.z = listenerPos[2];
	dir.x = forw[0];        dir.y = forw[1];        dir.z = forw[2];
	pos = pos - 6*dir;
	memcpy(&behind, &TO_NLVECTOR3D( pos ), sizeof behind );
	nslSetEmitterPosition(behindTheCamera, behind);

	if (g_game_ptr->is_competition_level() && 
			(nslGetSoundStatus(crowd) == NSL_SOUNDSTATUS_INVALID) &&
			g_world_ptr->get_ks_controller(0)->get_super_state() != SUPER_STATE_FLYBY)
	{
		SSEventId s    = SoundScriptManager::inst()->startEvent(SS_CROWD_AMBIENT);
		crowd = SoundScriptManager::inst()->getSoundId(s);
		if (nslGetSoundStatus(crowd) != NSL_SOUNDSTATUS_INVALID)
			nslSetSoundEmitter(behindTheCamera, crowd);
	}
  // Setup listener/surfer
  nslSetListenerPosition( listenerPos );
  nslSetListenerOrientation( forw, up );
	
  // Are we playing sounds?
  if ( !is_paused ) 
  { 
//    nlVector3d pos, pos1, pos2;
    vector3d v ;

    if ( !wet )
    {

      /*  
        Here we set up the wave Emitter positions
        The wave: ( aren't I an artist? )
                   FC = face
                   FM = foam
                   TB = tube
                        
        The count ( 0, 1 ) always starts from the left, so for a left break:
                                ____________ _____
                    ....-----'''            \     """"""":::.....
             ..---''  \       /               | <----TB 1--->    '''':::....
          --'  <-FC0->|<-TB0->| <---FC1----->/                               \
       '''             \_____/              %%%%<- FM 1 --------->%%%%%%%%%%%%
                        <FM1>                       

        
      */    
      // Foam

      // Do we have a new crash?
      // Compare the last tube start 
      if ( ( WAVE_SoundEmitter[WAVE_SE_TUBE].numsegment == 2 ) && 
          ( lastTubePieces == 1 ) )
      {
        if (waveCrashSrc != NSL_INVALID_ID)
        {
          waveCrashSnd = nslAddSound( waveCrashSrc );
          if ( nslGetSoundStatus( waveCrashSnd ) != NSL_SOUNDSTATUS_INVALID )
          { 
            nlVector3d crashPos;
            // Set the emitter position to the new location
            // Right
            if ( !waveDir )
              v = ( WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[1].start - WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[1].stop ) / 2 + WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[1].stop;
            else
              v = ( WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[0].start - WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[0].stop ) / 2 + WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[0].stop;
            crashPos[0] = v.x; crashPos[1] = v.y; crashPos[2] = v.z;
            nslSetSoundRange( waveCrashSnd, waveCrashMin, waveCrashMax );
            nslSetSoundParam( waveCrashSnd, NSL_SOUNDPARAM_VOLUME, crashVolume * crashModifier );
            nslPlaySound3D( waveCrashSnd, crashPos );
          }
        }
      }



      if ( ( WAVE_SoundEmitter[WAVE_SE_TUBE].numsegment == 1 ) && 
          ( lastTubePieces == 2 ) )
      {
        if (waveCrashSrc != NSL_INVALID_ID)
        {
          waveCrashSnd = nslAddSound( waveCrashSrc );
          if ( nslGetSoundStatus( waveCrashSnd ) != NSL_SOUNDSTATUS_INVALID )
          { 
            nlVector3d crashPos;
            // Set the emitter position to the new location
            // Right
            if ( !waveDir )
            {
              v = WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[0].stop;
            }
            else
            {
              v = WAVE_SoundEmitter[WAVE_SE_SPLASH].segment[0].start;
            }
            v.x += 4;
            crashPos[0] = v.x; crashPos[1] = v.y; crashPos[2] = v.z;
            nslSetSoundRange( waveCrashSnd, waveCrashMin, waveCrashMax );
            nslSetSoundParam( waveCrashSnd, NSL_SOUNDPARAM_VOLUME, crashVolume * crashModifier );
            nslPlaySound3D( waveCrashSnd, crashPos );
          }
        }
      }


      SetFaceEmitterPositions( );
      SetFoamEmitterPositions( );
      SetTubeEmitterPositions( );
      lastTubePieces = WAVE_SoundEmitter[WAVE_SE_TUBE].numsegment;
    }

    // Wipeout detection
  }



  // Tube calculations

  if (g_game_ptr->get_num_active_players() == 1)
  {
    kellyslater_controller *ksctl = g_game_ptr->get_world()->get_ks_controller(g_game_ptr->get_active_player());
    vector3d crashSpot, pos = g_world_ptr->get_hero_ptr(g_game_ptr->get_active_player())->get_abs_po().get_position();

    if (BeachDataArray[g_game_ptr->get_beach_id()].bdir)
    {
			if ((WAVE_SoundEmitter[WAVE_SE_TUBE].numsegment > 0) && (WAVE_GetStage() != WAVE_StageBuilding))
			{
				crashSpot.x  = WAVE_SoundEmitter[WAVE_SE_TUBE].line.start[0];
				crashSpot.y  = WAVE_SoundEmitter[WAVE_SE_TUBE].line.start[1];
				crashSpot.z  = WAVE_SoundEmitter[WAVE_SE_TUBE].line.start[2];
			}
			else if (WAVE_GetStage() != WAVE_StageBuilding)
			{
				crashSpot.x  = WAVE_SoundEmitter[WAVE_SE_SPLASH].line.start[0];
				crashSpot.y  = WAVE_SoundEmitter[WAVE_SE_SPLASH].line.start[1];
				crashSpot.z  = WAVE_SoundEmitter[WAVE_SE_SPLASH].line.start[2];
			}
    }
    else
    {
			if ((WAVE_SoundEmitter[WAVE_SE_TUBE].numsegment > 0) && (WAVE_GetStage() != WAVE_StageBuilding))
			{
				crashSpot.x  = WAVE_SoundEmitter[WAVE_SE_TUBE].line.stop[0];
				crashSpot.y  = WAVE_SoundEmitter[WAVE_SE_TUBE].line.stop[1];
				crashSpot.z  = WAVE_SoundEmitter[WAVE_SE_TUBE].line.stop[2];
			}
			else if (WAVE_GetStage() != WAVE_StageBuilding)
			{
				crashSpot.x  = WAVE_SoundEmitter[WAVE_SE_SPLASH].line.stop[0];
				crashSpot.y  = WAVE_SoundEmitter[WAVE_SE_SPLASH].line.stop[1];
				crashSpot.z  = WAVE_SoundEmitter[WAVE_SE_SPLASH].line.stop[2];
			}
    }

    // First we find out if we are in the tube..
    if (ksctl->get_board_controller().GetRegion() == WAVE_REGIONTUBE)
    {
      tubeModifier = 1.0f;
      muffleVolume = .8f;
      ambVol = .3f;
    }
    else if (WAVE_GetStage() != WAVE_StageBuilding)
    {
      float xdist = crashSpot.x - pos.x;
      float ydist = crashSpot.y - pos.y;
      float zdist = crashSpot.z - pos.z;
      float dist = sqrtf(xdist*xdist + ydist*ydist + zdist*zdist);

      if ((dist < MAX_TUBE_EXTEND_DIST*WAVE_GetHeight()) && (dist > 0))
      {
        if (WAVE_SoundEmitter[WAVE_SE_TUBE].numsegment > 0)
				{
					tubeModifier = .3f *  (MAX_TUBE_EXTEND_DIST*WAVE_GetHeight() - dist)/(MAX_TUBE_EXTEND_DIST*WAVE_GetHeight()) + .6f;
					ambVol = .3f + dist/MAX_TUBE_EXTEND_DIST*.4f*WAVE_GetHeight();
				}

        muffleVolume = (MAX_TUBE_EXTEND_DIST*WAVE_GetHeight() - dist)/(MAX_TUBE_EXTEND_DIST*WAVE_GetHeight());
      }
			else if ((dist < 0) && (WAVE_GetStage() != WAVE_StageBuilding))
			{	
				muffleVolume = 1.0f;
			}
			else 
      {
        tubeModifier        = 0.5f;
        ambVol              = 0.7f;
        muffleVolume = 0.0f;
      }
    }
  }

	
	
 
  setSoundVolumes( );
#ifdef DEBUG	// On XBox nglSphereMesh exists only in DEBUG
  if ( showSpheres )
    drawSoundPoints( );
#endif
}
