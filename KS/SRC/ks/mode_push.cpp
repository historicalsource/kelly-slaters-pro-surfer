
#include "global.h"
#include "mode_push.h"
#include "coords.h"

const float PushMode::SHARE_DELTA = 0.05f;
const float PushMode::SHARE_MAX = 0.80f;
const float PushMode::TIME_PUSH_EFFECT = 0.5f;	// number of seconds between chunk-chunk-chunk movements
const int PushMode::MAX_TIE_SHARES = 4;

//	PushMode()
// Default constructor.
PushMode::PushMode()
{
	// All players start with an equal share of the screen.
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		players[i].share = 1.0f/float(MAX_PLAYERS);
		players[i].extraShare = 0;
		players[i].ks = NULL;
		players[i].prevScore = 0;
	}
	attackTimer = 0.0f;

	CalcViewports();
}

//	~PushMode()
// Destructor.
PushMode::~PushMode()
{

}

//	Initialize()
// Must be called whenever the game mode is restarted.
void PushMode::Initialize(kellyslater_controller ** controllers)
{
	int	i;
	
	// Initialize each player.
	for (i = 0; i < MAX_PLAYERS; i++)
	{
		players[i].share = 1.0f/float(MAX_PLAYERS);
		players[i].extraShare = 0;
		players[i].ks = controllers[i];
		players[i].prevScore = 0;
	}
	attackTimer = 0.0f;
	
	CalcViewports();
}

// Sets the number of points required to push the other player over a bit.
void PushMode::SetDifficulty(const int points)
{
	assert(points > 0);
	scoreRequired = points;
}

//	Update()
// Must be called every frame.
void PushMode::Update(const float dt)
{
	int		i;
	bool	recalcViewports = false;

	// Notice score changes.
	for (i = 0; i < MAX_PLAYERS; i++)
		CalcExtraShares(i);

	// Limit tie shares.
	while (players[0].extraShare > MAX_TIE_SHARES && players[1].extraShare > MAX_TIE_SHARES)
	{
		players[0].extraShare -= MAX_TIE_SHARES;
		players[1].extraShare -= MAX_TIE_SHARES;
	}
	
	// Attack every now and then.
	attackTimer -= dt;
	if (attackTimer <= 0.0f)
	{
		attackTimer = TIME_PUSH_EFFECT;
		for (i = 0; i < MAX_PLAYERS; i++)
		{
			if (Attack(i))
				recalcViewports = true;
		}
	}

	if (recalcViewports)
		CalcViewports();
}

void PushMode::FinishCombat(void)
{
	int	i;
	
	while (players[0].extraShare > 0 || players[1].extraShare > 0)
	{
		for (i = 0; i < MAX_PLAYERS; i++)
			Attack(i);
	}

	CalcViewports();
}

//	GetPlayerShare()
// Returns the SCREEN OWNAGE PERCENTAGE of the specified player.
float PushMode::GetPlayerShare(const int playerIdx) const
{
	assert(playerIdx >= 0 && playerIdx < MAX_PLAYERS);
	
	return players[playerIdx].share;
}

//	GetPlayerViewport()
// Returns the viewport dimensions of the specified player.
const recti & PushMode::GetPlayerViewport(const int playerIdx) const
{
	assert(playerIdx >= 0 && playerIdx < MAX_PLAYERS);

	return players[playerIdx].viewport;
}

//	InCombat()
// Returns true if any players still have extra shares.
bool PushMode::InCombat(void) const
{
	int	i;
	
	// Check if any player has extra shares left to spend.
	for (i = 0; i < MAX_PLAYERS; i++)
	{
		if (players[i].extraShare > 0)
			return true;
	}

	return false;
}

// Private helper function: notices the specified player's score changes and increases his extraShares.
void PushMode::CalcExtraShares(const int playerIdx)
{
	int		score;
	int		diff;
	int		otherIdx;
	
	assert(playerIdx >= 0 && playerIdx < MAX_PLAYERS);

	// Ignore done players.
	if (!players[playerIdx].ks || players[playerIdx].share == 0.0f || players[playerIdx].share == 1.0f)
		return;

	// Get other player index.
	otherIdx = playerIdx == 0 ? 1 : 0;
		
	// Notice difference in score from the last time we checked.
	score = players[playerIdx].ks->get_my_scoreManager().GetScore();
	diff = score - players[playerIdx].prevScore;
			
	// Calculate extra shares.
	if (diff >= scoreRequired && players[playerIdx].share < 1.0f)
	{
		players[playerIdx].extraShare += diff/scoreRequired;
		players[playerIdx].prevScore += scoreRequired*(diff/scoreRequired);
	}
}

// Private helper function: causes the specified player to spend an extraShare to attack the other player.
// Returns true if the viewports have changed.
bool PushMode::Attack(const int playerIdx)
{
	bool	recalcViewports = false;
	bool	allowFatality = true;
	int		otherIdx;
	
	assert(playerIdx >= 0 && playerIdx < MAX_PLAYERS);

	// Ignore done players.
	if (!players[playerIdx].ks || players[playerIdx].share == 0.0f || players[playerIdx].share == 1.0f)
		return recalcViewports;

	// Get other player index.
	otherIdx = playerIdx == 0 ? 1 : 0;

	// Check if it's ok for this player to waste the other guy.
	if (players[otherIdx].extraShare > 0 || players[otherIdx].ks->IsDoingSomething())
		allowFatality = false;
	
	// Spend one extra share point.
	if (players[playerIdx].extraShare > 0)
	{
		// This player is about to waste the other guy.
		if (players[playerIdx].share == SHARE_MAX && allowFatality)
		{
			// Fatalities require double the points.
			if (players[playerIdx].extraShare >= 2)
			{
				// WASTED!
				players[playerIdx].share = 1.0f;
				players[playerIdx].extraShare = 0;
				players[otherIdx].share = 0.0f;
				players[otherIdx].extraShare = 0;
			}
		}
		// This player is about to push the other player a bit.
		else
		{
			// Attack.
			players[playerIdx].extraShare -= 1;
			players[playerIdx].share += SHARE_DELTA;
			players[otherIdx].share -= SHARE_DELTA;

			// Paranoia.
			if (players[playerIdx].share > SHARE_MAX)
				players[playerIdx].share = SHARE_MAX;
			if (players[playerIdx].share < 1.0f-SHARE_MAX)
				players[playerIdx].share = 1.0f-SHARE_MAX;
			if (players[otherIdx].share > SHARE_MAX)
				players[otherIdx].share = SHARE_MAX;
			if (players[otherIdx].share < 1.0f-SHARE_MAX)
				players[otherIdx].share = 1.0f-SHARE_MAX;
		}
		
		recalcViewports = true;
	}

	return recalcViewports;
}

//	CalcViewports()
// Private helper function - recalculates player viewports based on their shares.
void PushMode::CalcViewports(void)
{
	float	x, y;

	assert(MAX_PLAYERS == 2);	// does not support more than 2 players yet
	
	// Player 1.
	x = 0.0f;
	y = 0.0f;
	adjustCoords(x, y);
	players[0].viewport.set_left(int(x));
	players[0].viewport.set_top(int(y));
	x = 640.0f*players[0].share - 1.0f;
	y = 480.0f-1.0f;
	adjustCoords(x, y);
	//x -= 1.0f;
	//y -= 1.0f;
	players[0].viewport.set_right(int(x));
	players[0].viewport.set_bottom(int(y));
	
	// Player 2.
	x = 640.0f*players[0].share;
	y = 0.0f;
	adjustCoords(x, y);
	players[1].viewport.set_left(int(x));
	players[1].viewport.set_top(int(y));
	x = 640.0f-1.0f;
	y = 480.0f-1.0f;
	adjustCoords(x, y);
	//x -= 1.0f;
	//y -= 1.0f;
	players[1].viewport.set_right(int(x));
	players[1].viewport.set_bottom(int(y));
}