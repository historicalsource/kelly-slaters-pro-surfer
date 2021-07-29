#include "global.h"

//#if defined(TARGET_XBOX)
#include "game.h"
#include "po.h"
#include "wds.h"
#include "wave.h"
#include "conglom.h"
#include "kellyslater_controller.h"
//#endif /* TARGET_XBOX JIV DEBUG */

#include "projconst.h"
#include "rumbleManager.h"


#define MAX_RUMBLE_TIME 3.0f

rumbleManager rumbleMan;
rumbleManager::rumbleManager()
{
  rumbleLevel = 0;
  paused = false;
  on[0] = on[1] = on[2] = on[3] = true;
  currentStateTime[0] = currentStateTime[1] = 0;
  rumbleLevels[LANDING]             = 0.8f;
  rumbleLevels[IN_AIR]              = 0.0f;
  rumbleLevels[WIPING_OUT]          = 1.0f;
  rumbleLevels[UNDERWATER]          = 0.2f;
  rumbleLevels[GRINDING_OBJECT]     = 1.0f;
  rumbleLevels[FLOATER]             = 0.3f;
  rumbleLevels[IN_WASH]             = 0.4f;
  rumbleLevels[LIE_ON_BOARD_POCKET] = 0.25f;
  rumbleLevels[LIE_ON_BOARD_FACE]   = 0.3f;
  rumbleLevels[LIE_ON_BOARD_CHIN]   = 0.6f;
  rumbleLevels[STANDING_NEAR_TUBE]  = 0.5f;
  rumbleLevels[RUMBLE_NONE]         = 0.0f;

  rumbleVarPeriods[LANDING]             = 0.0f;
  rumbleVarPeriods[IN_AIR]              = 0.0f;
  rumbleVarPeriods[WIPING_OUT]          = 0.0f;
  rumbleVarPeriods[UNDERWATER]          = 1.0f;
  rumbleVarPeriods[GRINDING_OBJECT]     = 0.0f;
  rumbleVarPeriods[FLOATER]             = 1.5f;
  rumbleVarPeriods[IN_WASH]              = 0.5f;
  rumbleVarPeriods[LIE_ON_BOARD_POCKET] = 2.0f;
  rumbleVarPeriods[LIE_ON_BOARD_FACE]   = 3.0f;
  rumbleVarPeriods[LIE_ON_BOARD_CHIN]   = 2.0f;
  rumbleVarPeriods[STANDING_NEAR_TUBE]  = 2.5f;
  rumbleVarPeriods[RUMBLE_NONE]         = 0.0f;

  rumbleVarAmplitudes[LANDING]             = 0.0f;
  rumbleVarAmplitudes[IN_AIR]              = 0.0f;
  rumbleVarAmplitudes[WIPING_OUT]          = 0.0f;
  rumbleVarAmplitudes[UNDERWATER]          = 0.2f;
  rumbleVarAmplitudes[GRINDING_OBJECT]     = 0.0f;
  rumbleVarAmplitudes[FLOATER]             = 0.1f;
  rumbleVarAmplitudes[IN_WASH]             = 0.2f;
  rumbleVarAmplitudes[LIE_ON_BOARD_POCKET] = 0.1f;
  rumbleVarAmplitudes[LIE_ON_BOARD_FACE]   = 0.1f;
  rumbleVarAmplitudes[LIE_ON_BOARD_CHIN]   = 0.1f;
  rumbleVarAmplitudes[STANDING_NEAR_TUBE]  = 0.2f;
  rumbleVarAmplitudes[RUMBLE_NONE]         = 0.0f;

  rumbleFreqs[LANDING]             = 2.0f;
  rumbleFreqs[IN_AIR]              = 0.0f;
  rumbleFreqs[WIPING_OUT]          = 2.0f;
  rumbleFreqs[UNDERWATER]          = 0.0f;
  rumbleFreqs[GRINDING_OBJECT]     = 1.0f;
  rumbleFreqs[FLOATER]             = 0.0f;
  rumbleFreqs[IN_WASH]             = 0.0f;
  rumbleFreqs[LIE_ON_BOARD_POCKET] = 0.0f;
  rumbleFreqs[LIE_ON_BOARD_FACE]   = 0.0f;
  rumbleFreqs[LIE_ON_BOARD_CHIN]   = 0.0f;
  rumbleFreqs[STANDING_NEAR_TUBE]  = 0.0f;
  rumbleFreqs[RUMBLE_NONE]         = 0.0f;


};


const char rumbleManager::regionNames[][30] = {
    "LANDING",
    "IN_AIR",
    "WIPING_OUT",
    "UNDERWATER",
    "GRINDING_OBJECT",
    "FLOATER",
    "IN_WASH",
    "LIE_ON_BOARD_POCKET",
    "LIE_ON_BOARD_FACE",
    "LIE_ON_BOARD_CHIN",
    "STANDING_NEAR_TUBE",
    "NONE"
};

float tubeDist;

rumbleManager::~rumbleManager()
{
  for( int i = 0; i < RUMBLE_PADS; i++ )
		on[ i ] = false;
  paused = false;
  rumbleLevel = 0;
};

void rumbleManager::turnOn(bool on_p, int controller)
{
  on[controller] = on_p;
}

void rumbleManager::init()
{
  rumbleLevel = 0;
  paused = false;
  input_mgr* inputmgr=input_mgr::inst();
  for (int i=0; i < g_game_ptr->get_num_players(); i++)
	{
		input_device *joyjoy=inputmgr->get_joystick(g_world_ptr->get_ks_controller(i)->get_joystick_num());
		if ( joyjoy )
		{
			joyjoy->vibrate(0, 0, 0, 0);
		}
    //inputmgr->get_joystick(g_world_ptr->get_ks_controller(i)->get_joystick_num())->vibrate(0, 0, 0, 0);
	}
}
bool rumbleManager::isOn( int controller )
{
  return on[controller];
}
void rumbleManager::shutdown()
{
  rumbleLevel = 0;
  paused = false;
  input_mgr* inputmgr=input_mgr::inst();
  for (int i=0; i < g_game_ptr->get_num_players(); i++)
	{
		input_device *joyjoy=inputmgr->get_joystick(g_world_ptr->get_ks_controller(i)->get_joystick_num());
		if ( joyjoy )
		{
    	joyjoy->vibrate(0, 0, 0, 0);
		}
    //inputmgr->get_joystick(g_world_ptr->get_ks_controller(i)->get_joystick_num())->vibrate(0, 0, 0, 0);
	}
}

void rumbleManager::pause()
{
  paused = true;
  input_mgr* inputmgr=input_mgr::inst();
  for (int i=0; i < g_game_ptr->get_num_players(); i++)
	{
		input_device *joyjoy=inputmgr->get_joystick(g_world_ptr->get_ks_controller(i)->get_joystick_num());
		if ( joyjoy )
		{
    	joyjoy->vibrate(0, 0, 0, 0);
		}
    //inputmgr->get_joystick(g_world_ptr->get_ks_controller(i)->get_joystick_num())->vibrate(0, 0, 0, 0);
	}
}

void rumbleManager::unpause()
{
  paused = false;
  input_mgr* inputmgr=input_mgr::inst();
  for (int i=0; i < g_game_ptr->get_num_players(); i++)
	{
		input_device *joyjoy=inputmgr->get_joystick(g_world_ptr->get_ks_controller(i)->get_joystick_num());
		if ( joyjoy )
		{
    	joyjoy->vibrate(0, 0, 0, 0);
		}
    //inputmgr->get_joystick(g_world_ptr->get_ks_controller(i)->get_joystick_num())->vibrate(0, 0, 0, 0);
	}
}
void rumbleManager::tick(float delta_t)
{
  if (paused) return;

	// HACK so we don't rumble during flyby
	kellyslater_controller *ksctrl = g_world_ptr->get_ks_controller( 0 );
	if( ksctrl->get_super_state() == SUPER_STATE_FLYBY )
		return;

	//  int currentPlayer = 0;
  int numRuns = g_game_ptr->get_num_active_players();

  for (int i=0; i < numRuns; i++)
  {
    int currentPlayer;

    // Assign the correct value to currentPlayer
    if (g_game_ptr->get_num_players() > 1 && g_game_ptr->get_num_active_players() == 1)	// if multiplayer alternating
      currentPlayer         = g_game_ptr->get_active_player();
    else
      currentPlayer         = i;
		int joy_num = DEVICE_ID_TO_JOYSTICK_INDEX( g_world_ptr->get_ks_controller(currentPlayer)->get_joystick_num() );
		if(on[ joy_num ] && g_world_ptr->get_ks_controller(currentPlayer)->is_active())
		{

			ks_laststate[currentPlayer] = ks_state[currentPlayer];
			ks_state[currentPlayer]     = g_world_ptr->get_ks_controller(currentPlayer)->get_current_state();

			rumbleLevel                       = 0;
			rumbleFreq                        = 0;
			varianceAmplitude                 = 0;
			variancePeriod                    = 0;
			lastRegion[currentPlayer]         = currentRegion[currentPlayer];
			currentRegion[currentPlayer]      = g_world_ptr->get_ks_controller(currentPlayer)->get_board_controller().GetRegion();

			bool wet = !g_world_ptr->get_ks_controller(currentPlayer)->isDry();
			lastRumbleState[currentPlayer] = currentRumbleState[currentPlayer];

			// Figure out our state
			if (wet && ((lastRumbleState[currentPlayer] != UNDERWATER) && (lastRumbleState[currentPlayer] != WIPING_OUT)))
				currentRumbleState[currentPlayer] = WIPING_OUT;
			else if (ks_state[currentPlayer] == STATE_LIEONBOARD)
			{// lying down
				switch (currentRegion[currentPlayer])
				{
					case WAVE_REGIONPOCKET:   currentRumbleState[currentPlayer] = LIE_ON_BOARD_POCKET;  break;
					case WAVE_REGIONFACE:     currentRumbleState[currentPlayer] = LIE_ON_BOARD_FACE;    break;
					case WAVE_REGIONCHIN:     currentRumbleState[currentPlayer] = LIE_ON_BOARD_CHIN;    break;
					default:                  currentRumbleState[currentPlayer] = RUMBLE_NONE;	        break;
				}
			}
			else if ((lastRumbleState[currentPlayer] == LANDING) && (currentStateTime[currentPlayer] < .08f))
				currentRumbleState[currentPlayer] = LANDING;
			else if ((ks_state[currentPlayer] == STATE_LANDING) || (ks_state[currentPlayer] == STATE_JUNK_LANDING))
				currentRumbleState[currentPlayer] = LANDING;
			else if ((currentStateTime[currentPlayer] < .03f) && (lastRumbleState[currentPlayer] == WIPING_OUT))    // have just they wiped out?
				currentRumbleState[currentPlayer] = WIPING_OUT;
			else if (wet)
				currentRumbleState[currentPlayer] = UNDERWATER;
			else if (g_world_ptr->get_ks_controller(currentPlayer)->get_board_controller().inAirFlag)
				currentRumbleState[currentPlayer] = IN_AIR;
			else if (ks_state[currentPlayer] != STATE_LIEONBOARD)  // Not lying down
			{
				// In front?
				if (currentRegion[currentPlayer] == WAVE_REGIONWASH)
					currentRumbleState[currentPlayer] = IN_WASH;
				else if (g_world_ptr->get_ks_controller(currentPlayer)->get_board_controller().IsGrindingObject())// Grinding
					currentRumbleState[currentPlayer] = GRINDING_OBJECT;
				else if ((g_world_ptr->get_ks_controller(currentPlayer)->get_board_controller().state == BOARD_GRIND) ||
								 (g_world_ptr->get_ks_controller(currentPlayer)->get_board_controller().state == BOARD_FLOAT))
					currentRumbleState[currentPlayer] = FLOATER;
				else
					currentRumbleState[currentPlayer] = STANDING_NEAR_TUBE;

			}



			// Deal with time
			if (lastRumbleState[currentPlayer] != currentRumbleState[currentPlayer])
				currentStateTime[currentPlayer] = 0;
			else
				currentStateTime[currentPlayer] += delta_t;


			switch (currentRumbleState[currentPlayer])
			{
				case STANDING_NEAR_TUBE:
					tubeDist = g_world_ptr->get_ks_controller(currentPlayer)->Tube_Distance();
					if ((tubeDist < MIN_TUBE_DIST) && (WAVE_GetStage() != WAVE_StageBuilding))// IN TUBE
					{
						rumbleLevel = rumbleLevels[currentRumbleState[currentPlayer]]*(tubeDist - MIN_TUBE_DIST)/(MAX_TUBE_DIST - MIN_TUBE_DIST);
						if (rumbleLevel > rumbleLevels[currentRumbleState[currentPlayer]])
							rumbleLevel = rumbleLevels[currentRumbleState[currentPlayer]];
						varianceAmplitude   = rumbleVarAmplitudes[currentRumbleState[currentPlayer]];
						variancePeriod      = rumbleVarPeriods[currentRumbleState[currentPlayer]];
						rumbleFreq          = rumbleFreqs[currentRumbleState[currentPlayer]];
					}
					else
					{
						rumbleLevel         = 0;
						varianceAmplitude   = 0;
						variancePeriod      = 0;
						rumbleFreq          = 0;
					}
					break;
				default:
					rumbleLevel         = rumbleLevels[currentRumbleState[currentPlayer]];
					varianceAmplitude   = rumbleVarAmplitudes[currentRumbleState[currentPlayer]];
					variancePeriod      = rumbleVarPeriods[currentRumbleState[currentPlayer]];
					rumbleFreq          = rumbleFreqs[currentRumbleState[currentPlayer]];
					break;
			}
				// PLEASE don't divide by zero
			if (variancePeriod!=0.0f)
				rumbleLevel += varianceAmplitude * sinf( 2 * PI * currentStateTime[currentPlayer]/variancePeriod );

			if( currentStateTime[currentPlayer] > MAX_RUMBLE_TIME )
				rumbleLevel *= MAX_RUMBLE_TIME / currentStateTime[currentPlayer];
			
			if (rumbleLevel > 1)
				rumbleLevel = 1;
			else if (rumbleLevel < 0)
				rumbleLevel = 0;

			if (rumbleLevel < .1)
				rumbleLevel = 0.0f;
			input_mgr* inputmgr=input_mgr::inst();
			inputmgr->get_joystick(g_world_ptr->get_ks_controller(currentPlayer)->get_joystick_num())->vibrate(rumbleFreq, (int)(rumbleLevel*255), 0, 0);
		}
		else // !on[currentPlayer]
		{
			input_mgr* inputmgr=input_mgr::inst();
			inputmgr->get_joystick(g_world_ptr->get_ks_controller(currentPlayer)->get_joystick_num())->vibrate(0, 0, 0, 0);
		}
  }

}
void rumbleManager::drawCurrentState()
{
  if (drawState)
  {
    char theState[30];


    if (STANDING_NEAR_TUBE == currentRumbleState[0])
      sprintf(theState, "%s %f", regionNames[currentRumbleState[0]], tubeDist);
    else
      sprintf(theState, "%s", regionNames[currentRumbleState[0]]);

/*	Replaced by new API. (dc 05/30/02)
		KSNGL_SetFont (0);
		KSNGL_SetFontColor (0x80000080);
		KSNGL_SetFontZ (0.2f);
		KSNGL_SetFontScale (1, 1);
*/


		nglListAddString (nglSysFont, 300, 22, 0.2f, 0x80000080, theState);
  }
}
int writeText(const int fd, char *text)
{
#if defined(TARGET_PS2)
	return sceWrite(fd, text, strlen(text));
#else
	return 0;
#endif // !defined(TARGET_XBOX)
}


void rumbleManager::writeLevels()
{
#if defined(TARGET_PS2)
  int fout = sceOpen("host0:rumbleLevels.txt", SCE_WRONLY | SCE_TRUNC | SCE_CREAT);
  if (fout < 0)
    return;

  writeText(fout, "RumbleLevels\n");
  writeText(fout, "Location\t\tIntensity\tAmplitudeVariance\tPeriodVariance\tRumbleFreq\n");
  for (int i=0; i < RUMBLE_STATE_END; i++)
  {
    char textstr[200];
    sprintf(textstr, "%s\t\t%f\t%f\t%f\t%f\n", regionNames[i], rumbleLevels[i], rumbleVarAmplitudes[i], rumbleVarPeriods[i], rumbleFreqs[i]);
    writeText(fout, textstr);
  }
  sceClose(fout);
#endif

}
