
#include "global.h"
#include "mode_timeattack.h"

float	TimeAttackMode::TIME_INITIAL = 60.0f;
int		TimeAttackMode::TOLERANCE = 1000;
float	TimeAttackMode::TIME_LOSE = 15.0f;

//	TimeAttackMode()
// Default constructor.
TimeAttackMode::TimeAttackMode()
{
	int	i;

	for (i = 0; i < MAX_PLAYERS; i++)
	{
		players[i].ks = NULL;
		players[i].time = TIME_INITIAL;
		players[i].prevScore = 0;
		players[i].score = 0;
		players[i].attacking = false;
		players[i].attackScore = 0;
	}

	gameNum = 0;
	setNum = 0;
	scoreAttackStrength = 1000;
	scoreDropSpeed = scoreAttackStrength*10;
}

//	~TimeAttackMode()
// Destructor.
TimeAttackMode::~TimeAttackMode()
{

}

//	Initialize()
// Must be called before other member functions.
void TimeAttackMode::Initialize(kellyslater_controller ** controllers)
{
	int	i;

	for (i = 0; i < MAX_PLAYERS; i++)
		players[i].ks = controllers[i];

	Reset();
}

// Sets how many points it takes to deduct 1 second from opponent's time.
void TimeAttackMode::SetDifficulty(const int points)
{
	assert(points > 0);
	scoreAttackStrength = points;
	scoreDropSpeed = scoreAttackStrength*10;
}

//	Reset()
// Call this method when starting a new Trick Attack game.
void TimeAttackMode::Reset(void)
{
	int	i;

	for (i = 0; i < MAX_PLAYERS; i++)
	{
		players[i].time = TIME_INITIAL;
		players[i].prevScore = 0;
		players[i].score = 0;
		players[i].attacking = false;
		players[i].attackScore = 0;
	}

	gameNum = 0;
	setNum = 0;
}

//	Update()
// Must be called every frame.
void TimeAttackMode::Update(const float dt)
{
	int	i;
	
	// If in attack mode, decrease this player's score while
	// decreasing other players' times.
	for (i = 0; i < MAX_PLAYERS; i++)
	{
		Attack(i, dt);
	}
	
	/*
	int	i, j;
	int	score;
	int	diff;

	// Player's score reduces the other player's time.
	for (i = 0; i < MAX_PLAYERS; i++)
	{
		score = players[i].ks->get_my_scoreManager().GetScore();
		diff = score - players[i].prevScore;

		if (diff >= TOLERANCE)
		{
			for (j = 0; j < MAX_PLAYERS; j++)
			{
				if (i != j)
				{
					players[j].time -= (diff)/TOLERANCE;
					if (players[j].time < 0.0f)
						players[j].time = 0.0f;
				}
			}
			
			players[i].prevScore = (score/TOLERANCE)*TOLERANCE;
		}
	}
	*/
}

//	AdvanceSet()
// Called when a player starts a new run.
void TimeAttackMode::AdvanceSet(void)
{
	int i;
	
	if (++setNum == 2)
	{
		setNum = 0;
		gameNum++;
	}

	for (i = 0; i < MAX_PLAYERS; i++)
	{
		players[i].prevScore = 0;
	}
}

//	this function sucks
void TimeAttackMode::BeginCombat(void)
{
	int i;
	
	for (i = 0; i < MAX_PLAYERS; i++)
	{
		players[i].score = players[i].ks->get_my_scoreManager().GetScore();
	}
}

//	BeginAttacking()
// Counts the specified player's score down to zero while decreasing
// the other player's time.
void TimeAttackMode::BeginAttacking(const int playerIdx)
{
	assert(playerIdx >= 0 && playerIdx < MAX_PLAYERS);

	players[playerIdx].score = players[playerIdx].ks->get_my_scoreManager().GetScore();
	players[playerIdx].attacking = true;
}

//	FinishAttacking()
// Forces the current attack to finish quickly.
void TimeAttackMode::FinishAttacking(const int playerIdx)
{
	assert(playerIdx >= 0 && playerIdx < MAX_PLAYERS);

	while (players[playerIdx].attacking)
		Attack(playerIdx, 1.0f);
}

//	GetScore()
// Returns the specified player's displayed score.
// Use with BeginAttacking().
int TimeAttackMode::GetScore(const int playerIdx) const
{
	assert(playerIdx >= 0 && playerIdx < MAX_PLAYERS);

	return players[playerIdx].score;
}

//	GetLevelDuration()
// Returns the number of seconds the specified player has to surf in his next round.
float TimeAttackMode::GetLevelDuration(const int playerIdx) const
{
	assert(playerIdx >= 0 && playerIdx < MAX_PLAYERS);

	return players[playerIdx].time;
}

//	GetRemainingTime()
// Return the amount of time the specified player will have on his next run.
float TimeAttackMode::GetRemainingTime(const int playerIdx) const
{
	assert(playerIdx >= 0 && playerIdx < MAX_PLAYERS);

	return players[playerIdx].time;
}

//	IsAttacking()
// Returns true if the specified player is attacking.
bool TimeAttackMode::IsAttacking(const int playerIdx) const
{
	assert(playerIdx >= 0 && playerIdx < MAX_PLAYERS);

	return players[playerIdx].attacking;
}

//	IsDoneAttacking()
// Returns true if the specified player can no longer attack.
bool TimeAttackMode::IsDoneAttacking(const int playerIdx) const
{
	int	otherPlayerIdx = playerIdx == 0 ? 1 : 0;
	
	assert(playerIdx >= 0 && playerIdx < MAX_PLAYERS);

	return players[playerIdx].score == 0 || players[otherPlayerIdx].time == 0.0f;
}

// Returns the number of seconds that the specified player will deduct from the other player's time.
float TimeAttackMode::GetAttackSec(const int playerIdx) const
{
	float	attackSec;
	
	assert(playerIdx >= 0 && playerIdx < 2);

	attackSec = players[playerIdx].ks->get_my_scoreManager().GetScore()/scoreAttackStrength;
	if (attackSec > TIME_INITIAL)
		attackSec = TIME_INITIAL;

	return attackSec;
}

//	Attack()
// Private helper function - use player's score to decrease other players' times.
void TimeAttackMode::Attack(const int playerIdx, const float dt)
{
	assert(playerIdx >= 0 && playerIdx < MAX_PLAYERS);

	int	j;
	
	if (players[playerIdx].attacking)
	{
		int	diff = int(float(scoreDropSpeed)*dt);
		
		// Deduct a certain amount from score and use it to attack.
		if (diff > players[playerIdx].score)
			diff = players[playerIdx].score;	
		players[playerIdx].score -= diff;
		players[playerIdx].attackScore += diff;
		
		// Use attack to deduct from other players' times.
		while (players[playerIdx].attackScore >= scoreAttackStrength)
		{
			players[playerIdx].attackScore -= scoreAttackStrength;

			j = playerIdx == 0 ? 1 : 0;
			players[j].time -= 1.0f;

			// Wasted the other player?
			if (players[j].time <= 0.0f)
			{
				players[playerIdx].attacking = false;
				if (players[playerIdx].attackScore > 0)
					players[playerIdx].score += players[playerIdx].attackScore;
				players[playerIdx].attackScore = 0;
				break;
			}
		}

		if (players[playerIdx].score == 0)
		{
			players[playerIdx].attacking = false;
			players[playerIdx].attackScore = 0;
		}
	}

}