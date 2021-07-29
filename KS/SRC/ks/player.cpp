//
// player information
//

// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#ifdef _XBOX
#include "beachdata.h"
#include "player.h"
#include "game.h"
#endif /* TARGET_XBOX JIV DEBUG */

#include "player.h"

SurferAttributes g_surfer_attribs;

SurferAttributes::SurferAttributes()
{
	use_debug_values = 0;

	player_rotate = 13;
	player_speed = 20;
	player_balance = 20;
	player_air = 13;

	board_rotate = 7;
	board_speed = 7;
	board_balance = 7;
	board_air = 7;

	int n, m;
	for (n = 0; n < SURFER_LAST; n++)
	{
		for (m = 0; m < NUM_ATTRIBS; m++)
		{
			player_attribs[n][ATTRIB_AIR] = player_air;
			player_attribs[n][ATTRIB_SPEED] = player_speed;
			player_attribs[n][ATTRIB_ROTATE] = player_rotate;
			player_attribs[n][ATTRIB_BALANCE] = player_balance;
		}
	}

	for (n = 0; n < NUM_BOARDS_ATTRS; n++)
	{
		for (m = 0; m < NUM_ATTRIBS; m++)
		{
			board_attribs[n][ATTRIB_AIR] = board_air;
			board_attribs[n][ATTRIB_SPEED] = board_speed;
			board_attribs[n][ATTRIB_ROTATE] = board_rotate;
			board_attribs[n][ATTRIB_BALANCE] = board_balance;
		}
	}
}


player_info	current_player_info;

player_info::player_info ()
{
	int i = 0;
	
	for (i = 0; i < MAX_ACC; i++)
		accomplishments[i] = NULL;
	
	numAccomplishments = 0;
}

player_info::~player_info ()
{

}

//	AddAccomplishment()
// Adds the specified entity to the player's list of accomplishments.
void player_info::AddAccomplishment(entity * ent)
{
	if (numAccomplishments < MAX_ACC)
		accomplishments[numAccomplishments++] = ent;
}

//	HasAccomplishment()
// Returns true if the specifed entity is in the players' list of accompishments.
bool player_info::HasAccomplishment(const entity * ent) const
{
	int		i;
	
	for (i = 0; i < numAccomplishments; i++)
	{
		if (accomplishments[i] == ent)
			return true;
	}

	return false;
}

//	ClearAccomplishments()
// Erases all of the player's accomplishments.
void player_info::ClearAccomplishments(void)
{
	int i;
	
	for (i = 0; i < MAX_ACC; i++)
	  accomplishments[i] = NULL;

	numAccomplishments = 0;
}

//	TakePhoto()
// Beach events trigger this function to take a snapshot of the current screen.
bool TakePhoto(float dt, void** data)
{
/*
	if (dt >= 0)
		g_game_ptr->take_snapshot();
	*/

  return false;
}