
#include "global.h"
#include "mode_meterattack.h"

float MeterAttackMode::TIME_INITIAL = 60.0f;
int MeterAttackMode::TOLERANCE = 1000;
int MeterAttackMode::SCORE_DROP_SPEED = 10000;	// per second
int MeterAttackMode::SCORE_ATTACK_STRENGTH = 1000;	// how many points are required to deduct 1 second from other player's time?
float MeterAttackMode::TIME_LOSE = 15.0f;

//	MeterAttackMode()
// Default constructor.
MeterAttackMode::MeterAttackMode()
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
}

//	~MeterAttackMode()
// Destructor.
MeterAttackMode::~MeterAttackMode()
{

}

//	Initialize()
// Must be called before other member functions.
void MeterAttackMode::Initialize(kellyslater_controller ** controllers)
{
	int	i;

	for (i = 0; i < MAX_PLAYERS; i++)
		players[i].ks = controllers[i];

	Reset();
}

//	Reset()
// Call this method when starting a new Trick Attack game.
void MeterAttackMode::Reset(void)
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
void MeterAttackMode::Update(const float dt)
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
void MeterAttackMode::AdvanceSet(void)
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
void MeterAttackMode::BeginCombat(void)
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
void MeterAttackMode::BeginAttacking(const int playerIdx)
{
	assert(playerIdx >= 0 && playerIdx < MAX_PLAYERS);

	players[playerIdx].score = players[playerIdx].ks->get_my_scoreManager().GetScore();
	players[playerIdx].attacking = true;
}

//	FinishAttacking()
// Forces the current attack to finish quickly.
void MeterAttackMode::FinishAttacking(const int playerIdx)
{
	assert(playerIdx >= 0 && playerIdx < MAX_PLAYERS);

	while (players[playerIdx].attacking)
		Attack(playerIdx, 1.0f);
}

//	GetScore()
// Returns the specified player's displayed score.
// Use with BeginAttacking().
int MeterAttackMode::GetScore(const int playerIdx) const
{
	assert(playerIdx >= 0 && playerIdx < MAX_PLAYERS);

	return players[playerIdx].score;
}

//	GetLevelDuration()
// Returns the number of seconds the specified player has to surf in his next round.
float MeterAttackMode::GetLevelDuration(const int playerIdx) const
{
	assert(playerIdx >= 0 && playerIdx < MAX_PLAYERS);

	return players[playerIdx].time;
}

//	GetRemainingTime()
// Return the amount of time the specified player will have on his next run.
float MeterAttackMode::GetRemainingTime(const int playerIdx) const
{
	assert(playerIdx >= 0 && playerIdx < MAX_PLAYERS);

	return players[playerIdx].time;
}

//	IsAttacking()
// Returns true if the specified player is attacking.
bool MeterAttackMode::IsAttacking(const int playerIdx) const
{
	assert(playerIdx >= 0 && playerIdx < MAX_PLAYERS);

	return players[playerIdx].attacking;
}

//	Attack()
// Private helper function - use player's score to decrease other players' times.
void MeterAttackMode::Attack(const int playerIdx, const float dt)
{
	assert(playerIdx >= 0 && playerIdx < MAX_PLAYERS);

	int	j;
	
	if (players[playerIdx].attacking)
	{
		int	diff = int(float(SCORE_DROP_SPEED)*dt);
		
		// Deduct a certain amount from score and use it to attack.
		if (diff > players[playerIdx].score)
			diff = players[playerIdx].score;	
		players[playerIdx].score -= diff;
		players[playerIdx].attackScore += diff;
		
		// Use attack to deduct from other players' times.
		while (players[playerIdx].attackScore >= SCORE_ATTACK_STRENGTH)
		{
			players[playerIdx].attackScore -= SCORE_ATTACK_STRENGTH;
			for (j = 0; j < MAX_PLAYERS; j++)
			{
				if (playerIdx != j)
				{
					players[j].time -= 1.0f;

					if (players[j].time < 0.0f)
						players[j].time = 0.0f;
				}
			}
		}
		
		// All done attatcking?
		if (players[playerIdx].score == 0)
		{
			players[playerIdx].attacking = false;
			players[playerIdx].attackScore = 0;
		}
	}
}