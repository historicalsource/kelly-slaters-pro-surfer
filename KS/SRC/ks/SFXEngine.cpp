// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#include "SFXEngine.h"
#include <string.h>

#include "beach.h"
#include "floatobj.h"
#include "game.h"
#include "app.h"
#include "wds.h"
#include "random.h"
#include "common/nl.h"
#include "osdevopts.h"

SFXEngine sfx;




void SFXEngine::init( )
{
  hitSurferSounds.init( );
  hitPier.init( );
  whaleSounds.init( );
  buoySounds.init( );
  randomSounds.init( );
  thunderSounds.init( );
  didGoal.init( );
  
	// Just preload these
	nslLoadSource( "MOVEMENT_PADDLE1" );
	nslLoadSource( "MOVEMENT_PADDLE2" );

  // Init the hitting a person ones
  nslSourceId s = nslLoadSource( "HITSURFER1" );
  if ( s != NSL_INVALID_ID )
    hitSurferSounds.addSource( s );

  s = nslLoadSource( "HITSURFER2" );
  if ( s != NSL_INVALID_ID )
    hitSurferSounds.addSource( s );

  s = nslLoadSource( "HITSURFER3" );
  if ( s != NSL_INVALID_ID )
    hitSurferSounds.addSource( s );

  s = nslLoadSource( "HITSURFER3A" );
  if ( s != NSL_INVALID_ID )
    hitSurferSounds.addSource( s );

  hitSurferSounds.setProbability( 1 );

  // Init the hitting a pier ones
  s = nslLoadSource( "HITPIER1" );
  if ( s != NSL_INVALID_ID )
    hitPier.addSource( s );

  s = nslLoadSource( "HITPIER2" );
  if ( s != NSL_INVALID_ID )
    hitPier.addSource( s );

	randomTime = random(20) + 25;
  hitPier.setProbability( 1 );
  if ( g_game_ptr->get_beach_name( ) == "CORTES BANK" )
  {
    nslSourceId src = nslLoadSource( "AMB_LIGHTNING1" );
    if ( src != NSL_INVALID_ID )
      thunderSounds.addSource( src );

    src = nslLoadSource( "AMB_LIGHTNING2" );
    if ( src != NSL_INVALID_ID )
      thunderSounds.addSource( src );

    src = nslLoadSource( "AMB_THUNDER1" );
    if ( src != NSL_INVALID_ID )
      thunderSounds.addSource( src );

    src = nslLoadSource( "AMB_THUNDER2" );
    if ( src != NSL_INVALID_ID )
      thunderSounds.addSource( src );

    src = nslLoadSource( "AMB_THUNDER3" );
    if ( src != NSL_INVALID_ID )
      thunderSounds.addSource( src );

    thunderSounds.setProbability( 1.0 );
  }
	if ( g_game_ptr->get_beach_id() == BEACH_TEANIGHT)
	{
		s = nslLoadSource( "IG_VOLCANO_ERUPT" );
    if ( s != NSL_INVALID_ID )
      randomSounds.addSource( s );
	}
  if ( g_game_ptr->get_beach_name( ) == "ANTARCTICA" )
  {
    s = nslLoadSource( "AMB_WHALE1" );
    if ( s != NSL_INVALID_ID )
      randomSounds.addSource( s );

    s = nslLoadSource( "AMB_WHALE2" );
    if ( s != NSL_INVALID_ID )
      randomSounds.addSource( s );

    s = nslLoadSource( "AMB_WHALEBLOW" );
    if ( s != NSL_INVALID_ID )
      randomSounds.addSource( s );

    s = nslLoadSource( "AMB_THUNDER2" );
    if ( s != NSL_INVALID_ID )
      randomSounds.addSource( s );
  }
  else if (g_game_ptr->get_beach_id() != BEACH_INDOOR)
  {
    s = nslLoadSource( "AMB_BIRDS2" );
    if ( s != NSL_INVALID_ID )
      randomSounds.addSource( s );

    s = nslLoadSource( "AMB_BIRDS1" );
    if ( s != NSL_INVALID_ID )
      randomSounds.addSource( s );

    s = nslLoadSource( "SEAGULL1" );
    if ( s != NSL_INVALID_ID )
      randomSounds.addSource( s );
    s = nslLoadSource( "SEAGULL2" );
    if ( s != NSL_INVALID_ID )
      randomSounds.addSource( s );
    s = nslLoadSource( "SEAGULL3" );
    if ( s != NSL_INVALID_ID )
      randomSounds.addSource( s );

    s = nslLoadSource( "AMB_FIGHERJET1" );
    if ( s != NSL_INVALID_ID )
      randomSounds.addSource( s );

    s = nslLoadSource( "AMB_FIGHERJET2" );
    if ( s != NSL_INVALID_ID )
      randomSounds.addSource( s );

    s = nslLoadSource( "AMB_PASSENGERJET" );
    if ( s != NSL_INVALID_ID )
      randomSounds.addSource( s );

    s = nslLoadSource( "AMB_PASSENGERJET2" );
    if ( s != NSL_INVALID_ID )
      randomSounds.addSource( s );

    s = nslLoadSource( "AMB_PASSENGERJET3" );
    if ( s != NSL_INVALID_ID )
      randomSounds.addSource( s );

    s = nslLoadSource( "AMB_PLANE1" );
    if ( s != NSL_INVALID_ID )
      randomSounds.addSource( s );

    s = nslLoadSource( "AMB_PLANE2" );
    if ( s != NSL_INVALID_ID )
      randomSounds.addSource( s );


    s = nslLoadSource( "AMB_PLANE" );
    if ( s != NSL_INVALID_ID )
      randomSounds.addSource( s );


    if ( g_game_ptr->is_competition_level( ) )
    {
      s = nslLoadSource( "AMB_CRUISESANNOUNCE1" );
      if ( s != NSL_INVALID_ID )
        randomSounds.addSource( s );
    }

    s = nslLoadSource( "AMB_CRUISESHIPHORN" );
    if ( s != NSL_INVALID_ID )
      randomSounds.addSource( s );
	

  }
  randomSounds.setProbability( 1 );
	

  s = nslLoadSource( "BUOY1" );
  if ( s != NSL_INVALID_ID )
    buoySounds.addSource( s );

  s = nslLoadSource( "BUOY2" );
  if ( s != NSL_INVALID_ID )
    buoySounds.addSource( s );

  buoySounds.setProbability( .001 );

  s = nslLoadSource( "TADA" );
  if ( s != NSL_INVALID_ID )
    didGoal.addSource( s );

  didGoal.setProbability( 1 );

  dolphinBad = nslLoadSource( "AMB_DOLPHIN_DISAPPOINT" );
  dolphinGood = nslLoadSource( "AMB_DOLPHIN_CELEBRATE" );

  paused =false;
  thunderTime = -1;
}
void SFXEngine::thunder( )
{
  if ( g_game_ptr->get_beach_name( ) == "CORTES BANK" )
  {
    thunderTime = 0;
    //thunderSounds.play( );
  }
}
void SFXEngine::playRain( )
{
  stringx name;

  if ( g_game_ptr->get_beach_name( ) == "CORTES BANK" )
    name = "amb_rain01";
  else if ( g_game_ptr->get_beach_name( ) == "JEFFERSON BAY" )
    name = "amb_rain02";
  else
    return;

  nslSourceId rainSrc = nslLoadSource( name.c_str( ) );
  if ( rainSrc != NSL_INVALID_ID )
  {
    nslSoundId rainSnd = nslAddSound( rainSrc );
    if ( rainSnd != NSL_INVALID_ID )
    {
      nslPlaySound( rainSnd );
    }
  }

}
void SFXEngine::gotChallenge( )
{
  if (paused) return;
  if ( !didGoal.isPlaying( ) )
    didGoal.play( );
}

void SFXEngine::paddle( bool left, float x, float y, float z, float volume )
  {
  if(os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
    return;
  if (paused) return;
  if ( left )
  {
    nslSourceId src = nslLoadSource( "MOVEMENT_PADDLE1" );
    if ( src != NSL_INVALID_ID )
    {
      nslSoundId snd = nslAddSound( src );
      if ( snd != NSL_INVALID_ID )
      {
        nlVector3d pos;
        pos[0] = x;
        pos[1] = y;
        pos[2] = z;
        nslSetSoundParam( snd, NSL_SOUNDPARAM_VOLUME, volume );
        nslPlaySound3D( snd, pos );

      }
    }
  }
  else
  {
    nslSourceId src = nslLoadSource( "MOVEMENT_PADDLE2" );
    if ( src != NSL_INVALID_ID )
    {
      nslSoundId snd = nslAddSound( src );
      if ( snd != NSL_INVALID_ID )
      {
        nlVector3d pos;
        pos[0] = x;
        pos[1] = y;
        pos[2] = z;
        nslSetSoundParam( snd, NSL_SOUNDPARAM_VOLUME, volume );
        nslPlaySound3D( snd, pos );

      }
    }
  }



}

void SFXEngine::tick( float delta_t )
{

  if (paused) return;
  if ( thunderTime != -1 )
  {
    thunderTime+=delta_t;
    if ( thunderTime > THUNDER_DELAY )
    {
      thunderTime = -1;
      thunderSounds.play( );
    }

  }
  po p = g_world_ptr->get_hero_ptr(g_game_ptr->get_active_player())->get_abs_po();	// SFXEngine::tick() does not correctly handle simultaneous players on the screen (multiplayer fixme?)

  vector3d crashSpot, pos = p.get_position();

  crashSpot.x  = WAVE_SoundEmitter[WAVE_SE_TUBE].line.stop[0];
  crashSpot.y  = WAVE_SoundEmitter[WAVE_SE_TUBE].line.stop[1];
  crashSpot.z  = WAVE_SoundEmitter[WAVE_SE_TUBE].line.stop[2];

  WaveRegionEnum currentRegion = g_world_ptr->get_ks_controller(g_game_ptr->get_active_player())->get_board_controller().GetRegion();
  // First we find out if we are in the tube..
  if (currentRegion == WAVE_REGIONTUBE)
  {
    volumeMod = .3;
  }
  else
  {
    float xdist = crashSpot.x - pos.x;
    float ydist = crashSpot.y - pos.y;
    float zdist = crashSpot.z - pos.z;
    float dist = sqrtf(xdist*xdist + ydist*ydist + zdist*zdist);

    if ((dist < MAX_TUBE_SFX_DIST) && (dist > 0))
    {
      volumeMod = (SFX_VOLUME_ADJUSTMENT_FACTOR -.2) * (dist)/(MAX_TUBE_SFX_DIST) + .2;
    }
    else
    {
      volumeMod = SFX_VOLUME_ADJUSTMENT_FACTOR;
    }
  }
	randomTime -= delta_t;
  if (g_game_ptr->get_num_players() == 1)
  {
    nlMatrix4x4 m;
    nslGetListenerPo(&m);
    vector3d v;
		if (randomTime < 0)
		{
			v.x = m[0][3];  v.y = m[1][3];  v.z = m[2][3];

			v.x += random( 40.0f ) - 20.0f;
			v.y += random( 20.0f );
			v.z += random( 40.0f ) - 20.0f;
			
			randomSounds.play3d( v.x, v.y, v.z, 10.0f, 30.0f );
			if (randomSounds.getLastSourcePlayed() ==(int) nslGetSource("IG_VOLCANO_ERUPT",false) && 
				(nslGetSoundStatus(randomSounds.getSoundId()) != NSL_SOUNDSTATUS_INVALID))
			{
				nslSetSoundEmitter(wSound.behindTheCamera, randomSounds.getSoundId());
			}
			
			randomTime += random(20) + 25;
		}
  }
  else // MULTIPLAYER
  {

  }

  for ( beach_object *fobj = g_beach_ptr->my_objects; fobj != NULL; fobj = fobj->next )
	{
        if (!fobj->is_active ())
          continue;

    entity *ent = ( ( water_object * )fobj )->get_entity( );

    if ( ent != NULL ) 
    { 
      if ( ent->get_parsed_name( ) == "whale" )
      {
        // get the frame
        if ( ( ((u_int) (FTOI(TIMER_GetTotalSec() / 60.f)))%ent->get_mesh( )->Sections[0].Material->Map->NFrames ) == 52 )
        {
          po whalePos = ent->get_abs_po( );
          whaleSounds.play3d( whalePos.get_position( ).x,whalePos.get_position( ).y,whalePos.get_position( ).z, 120, 150 );
        }
      }
      else if ( ent->get_parsed_name( ) == "buoyA" )
      {
        if ( fobj->spawned )  
        {
          po buoyPos = ent->get_abs_po( );
          buoySounds.play3d( buoyPos.get_position( ).x,buoyPos.get_position( ).y,buoyPos.get_position( ).z, 90, 150 );
        }
      }
    }
    
	}


  if (thunderSounds.isPlaying())
    nslSetSoundParam(thunderSounds.getSoundId(), NSL_SOUNDPARAM_VOLUME, volumeMod);
  
  
  if (whaleSounds.isPlaying())
    nslSetSoundParam(whaleSounds.getSoundId(), NSL_SOUNDPARAM_VOLUME, volumeMod);
  
  if (randomSounds.isPlaying())
    nslSetSoundParam(randomSounds.getSoundId(), NSL_SOUNDPARAM_VOLUME, volumeMod);  
}
void SFXEngine::shutdown( )
{
  hitSurferSounds.shutdown( );
  hitPier.shutdown( );
  whaleSounds.shutdown( );
  buoySounds.shutdown( );
  randomSounds.shutdown( );
  thunderSounds.shutdown( );

}

void SFXEngine::trickSuceeded( )
{
  if (paused) return;
  if ( dolphinGood != NSL_INVALID_ID )
    nslPlaySound( nslAddSound( dolphinBad ) );
}

void SFXEngine::trickFailed( )
{
  if (paused) return;
  if ( dolphinBad != NSL_INVALID_ID )
    nslPlaySound( nslAddSound( dolphinBad ) );
}

void SFXEngine::sprayed(entity *sprayed)
{
	SSEventId s=-1;
	nslSoundId snd=NSL_INVALID_ID;
  if (paused) return;
	if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
		return;
  char entity_name[30];
  if ( sprayed )
  {
    strcpy( entity_name, sprayed->get_parsed_name( ).c_str( ) );

		if ( strnicmp( entity_name, "fatbastard", strlen( "fatbastard" ) )==0 )
		{
			s = SoundScriptManager::inst()->playEvent(SS_TUBER_TALK, NULL);
			SoundScriptManager::inst()->endEvent(s, 4);
		}
		else if ( strnicmp( entity_name, "boogieboarder", strlen( "boogieboarder" ) )==0 )
		{
			s=SoundScriptManager::inst()->playEvent(SS_SPONGER_TALK, NULL);
			SoundScriptManager::inst()->endEvent(s, 4);
		}
		else if (strnicmp( entity_name, "jetskier", strlen( "jetskier" ) )==0 )
		{
			s=SoundScriptManager::inst()->playEvent(SS_JETSKI_TALK, NULL);
			SoundScriptManager::inst()->endEvent(s, 4);
		}
		else if (strnicmp( entity_name, "dingy", strlen( "dingy" ) )==0 )
		{
			s=SoundScriptManager::inst()->playEvent(SS_RAFTER_TALK, NULL);
			SoundScriptManager::inst()->endEvent(s, 4);
		}
		else if (strnicmp( entity_name, "kayak", strlen( "kayak" ) )==0 )
		{
			s=SoundScriptManager::inst()->playEvent(SS_KAYAK_TALK, NULL);
			SoundScriptManager::inst()->endEvent(s, 4);
		}
		else if (strnicmp( entity_name, "wind_surfer", strlen( "wind_surfer" ) )==0 )
		{
			s=SoundScriptManager::inst()->playEvent(SS_WINDSURFER_TALK, g_world_ptr->get_ks_controller(0)->get_my_board_model());
			SoundScriptManager::inst()->endEvent(s, 4);
		}
		else
		{
			s=SoundScriptManager::inst()->playEvent(SS_GENERIC_TALK, NULL);
			SoundScriptManager::inst()->endEvent(s, 4);
		}

		if (s > -1)
		{
			snd =SoundScriptManager::inst()->getSoundId(s);
			if (nslGetSoundStatus(snd) != NSL_SOUNDSTATUS_INVALID)
			{
				nslSetSoundEmitter(wSound.behindTheCamera, snd);	
			}
		}
  }


}
void SFXEngine::collided(entity *surfer,  entity *hit )
{
	SSEventId s=-1;
	nslSoundId snd=NSL_INVALID_ID;
  if (paused) return;
	if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
		return;
  char entity_name[30];
  if ( hit )
  {
    strcpy( entity_name, hit->get_parsed_name( ).c_str( ) );
 		
			
		if ( strnicmp( entity_name, "SEBASTIAN_PIER", strlen( "SEBASTIAN_PIER" ) )==0 )
    {
      if (surfer->get_abs_position().y > hit->get_abs_position().y)
				SoundScriptManager::inst()->playEvent(SS_HIT_PIER_TOP, hit);
			else
				SoundScriptManager::inst()->playEvent(SS_HIT_PIER_BOTTOM, hit);
    }
		else if ( strnicmp( entity_name, "fatbastard", strlen( "fatbastard" ) )==0 )
		{
			s = SoundScriptManager::inst()->playEvent(SS_HIT_TUBER, g_world_ptr->get_ks_controller(0)->get_my_board_model());
			SoundScriptManager::inst()->endEvent(s, 4);
			s = SoundScriptManager::inst()->playEvent(SS_TUBER_TALK, NULL);
			SoundScriptManager::inst()->endEvent(s, 4);
		}
		else if ( strnicmp( entity_name, "boogieboarder", strlen( "boogieboarder" ) )==0 )
		{
			s=SoundScriptManager::inst()->playEvent(SS_HIT_SPONGER, g_world_ptr->get_ks_controller(0)->get_my_board_model());
			SoundScriptManager::inst()->endEvent(s, 4);
			s=SoundScriptManager::inst()->playEvent(SS_SPONGER_TALK, NULL);
			SoundScriptManager::inst()->endEvent(s, 4);
		}
		else if ( strnicmp( entity_name, "iceberg", strlen( "iceberg" ) )==0 )
		{
			s=SoundScriptManager::inst()->playEvent(SS_HIT_ICEBERG, g_world_ptr->get_ks_controller(0)->get_my_board_model());
			SoundScriptManager::inst()->endEvent(s, 4);
		}
		else if ( strnicmp( entity_name, "pancake_ice", strlen( "pancake_ice" ) )==0 )
		{
			s=SoundScriptManager::inst()->playEvent(SS_HIT_PANCAKEICE, g_world_ptr->get_ks_controller(0)->get_my_board_model());
			SoundScriptManager::inst()->endEvent(s, 4);
		}
		else if (strnicmp( entity_name, "jetskier", strlen( "jetskier" ) )==0 )
		{
			s=SoundScriptManager::inst()->playEvent(SS_HIT_JETSKI, g_world_ptr->get_ks_controller(0)->get_my_board_model());
			SoundScriptManager::inst()->endEvent(s, 4);
			s=SoundScriptManager::inst()->playEvent(SS_JETSKI_TALK,NULL);
			SoundScriptManager::inst()->endEvent(s, 4);
		}
		else if (strnicmp( entity_name, "jetskier", strlen( "jetskier" ) )==0 )
		{
			s=SoundScriptManager::inst()->playEvent(SS_HIT_JETSKI, g_world_ptr->get_ks_controller(0)->get_my_board_model());
			SoundScriptManager::inst()->endEvent(s, 4);
			s=SoundScriptManager::inst()->playEvent(SS_JETSKI_TALK, NULL);
			SoundScriptManager::inst()->endEvent(s, 4);
		}
		else if (strnicmp( entity_name, "dingy", strlen( "dingy" ) )==0 )
		{
			s=SoundScriptManager::inst()->playEvent(SS_HIT_RAFTER, g_world_ptr->get_ks_controller(0)->get_my_board_model());
			SoundScriptManager::inst()->endEvent(s, 4);
			s=SoundScriptManager::inst()->playEvent(SS_RAFTER_TALK, NULL);
			SoundScriptManager::inst()->endEvent(s, 4);
		}
		else if (strnicmp( entity_name, "kayak", strlen( "kayak" ) )==0 )
		{
			s=SoundScriptManager::inst()->playEvent(SS_HIT_KAYAK, g_world_ptr->get_ks_controller(0)->get_my_board_model());
			SoundScriptManager::inst()->endEvent(s, 4);
			s=SoundScriptManager::inst()->playEvent(SS_KAYAK_TALK, NULL);
			SoundScriptManager::inst()->endEvent(s, 4);
		}
		else if (strnicmp( entity_name, "wind_surfer", strlen( "wind_surfer" ) )==0 )
		{
			s=SoundScriptManager::inst()->playEvent(SS_HIT_WINDSURFER, g_world_ptr->get_ks_controller(0)->get_my_board_model());
			SoundScriptManager::inst()->endEvent(s, 4);
			s=SoundScriptManager::inst()->playEvent(SS_WINDSURFER_TALK, g_world_ptr->get_ks_controller(0)->get_my_board_model());
			SoundScriptManager::inst()->endEvent(s, 4);
		}
		else
		{
			s=SoundScriptManager::inst()->playEvent(SS_HIT_GENERIC, g_world_ptr->get_ks_controller(0)->get_my_board_model());
			SoundScriptManager::inst()->endEvent(s, 4);
			s=SoundScriptManager::inst()->playEvent(SS_GENERIC_TALK, NULL);
			SoundScriptManager::inst()->endEvent(s, 4);
		}
		if (s > -1)
		{
			snd =SoundScriptManager::inst()->getSoundId(s);
			if (nslGetSoundStatus(snd) != NSL_SOUNDSTATUS_INVALID)
			{
				nslSetSoundEmitter(wSound.behindTheCamera, snd);	
			}
		}
		s = -1;
		if (g_game_ptr->GetSurferIdx(0) != SURFER_LISA_ANDERSEN)
			s = SoundScriptManager::inst()->playEvent(SS_MALE_GRUNT); 
		else
			s = SoundScriptManager::inst()->playEvent(SS_FEMALE_GRUNT); 
		
		if (s > -1)
			snd =SoundScriptManager::inst()->getSoundId(s);

		if (nslGetSoundStatus(snd) != NSL_SOUNDSTATUS_INVALID)
		{
			nslSetSoundParam(snd, NSL_SOUNDPARAM_VOLUME, random(35) + 40);
		}
  }
}
