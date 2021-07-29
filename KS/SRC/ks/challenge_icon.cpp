
#include "global.h"
#include "challenge_icon.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	IconChallenge class
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//	IconChallenge()
// Default constructor.
IconChallenge::IconChallenge()
{
	ksctrl = NULL;
	
	// Currently hardcoded: later to be loaded from some data file.
	
	// Pipeline version.
	numIcons = 16;
	icons = NEW Icon[numIcons];
	icons[0].Init("ghosticon", 0, vector3d(35, 6, 10), 0.9f);	// flip
	icons[1].Init("ghosticon", 1, vector3d(40, 6, 10), 0.9f);	// grab1
	icons[2].Init("ghosticon", 1, vector3d(45, 5, 10), 0.9f);	// grab2
	icons[3].Init("ghosticon", 2, vector3d(45, 6, 10), 0.9f);	// 1000 points1
	icons[4].Init("ghosticon", 2, vector3d(53, 5, 10), 0.9f);	// 1000 points2
	icons[5].Init("ghosticon", 3, vector3d(45, 6, 10), 0.9f);	// 2000 points
	icons[6].Init("ghosticon", 4, vector3d(45, 6, 10), 0.9f);	// 5000 points
	icons[7].Init("ghosticon", 5, vector3d(45, 6, 10), 0.9f);	// special trick
	icons[8].Init("ghosticon", 6, vector3d(15, -1, 4), 0.9f);	// spin1
	icons[9].Init("ghosticon", 6, vector3d(40, -1, 4), 0.9f);	// spin2
	icons[10].Init("ghosticon", 6, vector3d(57, 0.5f, 8), 0.9f);	// spin3
	icons[11].Init("ghosticon", 6, vector3d(49, 6, 10), 0.9f);	// spin4
	icons[12].Init("ghosticon", 7, vector3d(10, 1.4f, 6), 0.9f);	// tube time 10 sec
	icons[13].Init("ghosticon", 8, vector3d(10, 1.4f, 6), 0.9f);	// tube time 20 sec
	icons[14].Init("ghosticon", 9, vector3d(10, 1.4f, 6), 0.9f);	// tube time 30 sec
	icons[15].Init("ghosticon", 10, vector3d(10, -0.1f, 6), 0.9f);	// tube trick
}

//	~IconChallenge()
// Destructor.
IconChallenge::~IconChallenge()
{
	delete [] icons;
}

//	Init()
// One-time initialization for the challenge.
void IconChallenge::Init(kellyslater_controller * ks)
{
	ksctrl = ks;
	arrangement.Init(icons);
}

//	Retry()
// Called whenever the level is restarted.
void IconChallenge::Retry(void)
{
	for (int i = 0; i < numIcons; i++)
		icons[i].Despawn();
	
	arrangement.Reset(icons);
}

//	Update()
// Called every frame.
void IconChallenge::Update(const float dt)
{
	int	i;
	
	arrangement.Update(ksctrl, dt);

	for (i = 0; i < numIcons; i++)
	{
		icons[i].Update(dt);
		//icons[i].Spawn();
	}
}

//	OnEvent()
// Handles all incoming events.
void IconChallenge::OnEvent(const EVENT event, const int param1, const int param2)
{
	if (param1 != ksctrl->get_player_num())
		return;
	
	if (event == EVT_SCORING_SERIES_END)
		arrangement.OnEvent(event, ksctrl, param2);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	IconChallenge::Arrangement class
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//	Arrangement()
// Default constructor.
IconChallenge::Arrangement::Arrangement()
{
	numSequences = 0;
	currSequenceIdx = 0;
	completed = false;
}

//	Init()
// One-time initialization for the arrangement.
void IconChallenge::Arrangement::Init(Icon * icons)
{
	Reset(icons);
}

//	Reset()
// Called when the challenge is restarted.
void IconChallenge::Arrangement::Reset(Icon * icons)
{
	for (int i = 0; i < MAX_ARRANGEMENT_SIZE; i++)
		sequences[i].Reset();

	currSequenceIdx = 0;
	numSequences = 4;
	sequences[0].PushTask(&icons[9], Task::TYPE_FACE_SPIN);
	sequences[0].PushTask(&icons[1], Task::TYPE_AIR_GRAB);

	sequences[1].PushTask(&icons[8], Task::TYPE_FACE_SPIN);
	sequences[1].PushTask(&icons[12], Task::TYPE_TUBE_TIME_10);
	sequences[1].PushTask(&icons[15], Task::TYPE_TUBE_TRICK);

	sequences[2].PushTask(&icons[7], Task::TYPE_AIR_SPECIAL);
	sequences[2].PushTask(&icons[1], Task::TYPE_AIR_GRAB);

	sequences[3].PushTask(&icons[2], Task::TYPE_AIR_GRAB);
	sequences[3].PushTask(&icons[11], Task::TYPE_AIR_SPIN);
	sequences[3].PushTask(&icons[4], Task::TYPE_AIR_POINTS_1000);
	sequences[3].PushTask(&icons[10], Task::TYPE_FACE_SPIN);

	completed = false;

	Spawn();
}

//	Update()
// Called every frame.
void IconChallenge::Arrangement::Update(kellyslater_controller * ksctrl, const float dt)
{
	if (!completed)
	{
		sequences[currSequenceIdx].Update(ksctrl, dt);
		
		if (sequences[currSequenceIdx].IsCompleted())
		{
			sequences[currSequenceIdx].Despawn();
			currSequenceIdx++;
			
			if (currSequenceIdx == numSequences)
			{
				currSequenceIdx = 0;
				completed = true;
			}
			else
				sequences[currSequenceIdx].Spawn();
		}
	}
}

//	Spawn()
// Makes the arramgenent's icons appear.
void IconChallenge::Arrangement::Spawn(void)
{
	sequences[currSequenceIdx].Spawn();
}

//	Despawn()
// Makes the arrangement's icons disappear.
void IconChallenge::Arrangement::Despawn(void)
{
	sequences[currSequenceIdx].Despawn();
}

//	OnEvent()
// Handles all incoming events.
void IconChallenge::Arrangement::OnEvent(const EVENT event, kellyslater_controller * ksctrl, const int param2)
{
	for (int i = 0; i < numSequences; i++)
		sequences[i].OnEvent(event, ksctrl, param2);
}

//	GetCurrentSequence()
// Returns the arrangement's current sequence of tasks.
const IconChallenge::Sequence * IconChallenge::Arrangement::GetCurrentSequence(void) const
{
	if (currSequenceIdx >= 0 && currSequenceIdx < numSequences)
		return &sequences[currSequenceIdx];
	else
		return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	IconChallenge::Sequence class
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//	Sequence()
// Default constructor.
IconChallenge::Sequence::Sequence()
{
	numTasks = 0;
	completed = false;
}

//	Reset()
// Resets the sequence to its default state.
void IconChallenge::Sequence::Reset(void)
{
	for (int i = 0; i < MAX_SEQUENCE_SIZE; i++)
		tasks[i].Reset();
	
	numTasks = 0;
	completed = false;
}

//	Update()
// Called every frame.
void IconChallenge::Sequence::Update(kellyslater_controller * ksctrl, const float dt)
{
	int			i;
	int			numCompleted = 0;

	// Update all tasks.
	for (i = 0; i < numTasks; i++)
		tasks[i].Update(ksctrl, dt);
	
	// Check for task completions.
	if (!completed)
	{
		for (i = 0; i < numTasks; i++)
		{
			if (tasks[i].IsCompleted())
				numCompleted++;
		}
		
		// Sequence is complete if every task is complete.
		if (numCompleted == numTasks)
			completed = true;
	}
}

//	PushTask()
// Adds the specified icon to the sequence.
void IconChallenge::Sequence::PushTask(IconChallenge::Icon * i, const IconChallenge::Task::TYPE t)
{
	assert(numTasks < MAX_SEQUENCE_SIZE-1);

	tasks[numTasks++].Init(i, t);
}

//	Spawn()
// Makes the sequence's icons appear.
void IconChallenge::Sequence::Spawn(void)
{
	for (int i = 0; i < numTasks; i++)
		tasks[i].Spawn();
}

//	Despawn()
// Makes the sequence's icons disappear.
void IconChallenge::Sequence::Despawn(void)
{
	for (int i = 0; i < numTasks; i++)
		tasks[i].Despawn();
}

//	OnEvent()
// Handles all incoming events.
void IconChallenge::Sequence::OnEvent(const EVENT event, kellyslater_controller * ksctrl, const int param2)
{
	for (int i = 0; i < numTasks; i++)
		tasks[i].OnEvent(event, ksctrl, param2);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	IconChallenge::Task class
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const float IconChallenge::Task::REQUIRED_DIST2 = 6.0f*6.0f;

//	Task()
// Default constructor.
IconChallenge::Task::Task()
{
	icon = NULL;
	completed = false;
	type = TYPE_NONE;
	watchChain = false;
}

//	Reset()
// Sets the task to its default state.
void IconChallenge::Task::Reset(void)
{
	icon = NULL;
	completed = false;
	type = TYPE_NONE;
	watchChain = false;
}

//	Init()
// Sets the task's properties.
void IconChallenge::Task::Init(IconChallenge::Icon * i, const IconChallenge::Task::TYPE t)
{
	icon = i;
	type = t;
	completed = false;
	watchChain = false;
}

//	Update()
// Must be called every frame.
void IconChallenge::Task::Update(kellyslater_controller * ksctrl, const float dt)
{
	if (!completed)
		CheckForCompletion(ksctrl);
}

//	Spawn()
// Makes the task's icon appear.
void IconChallenge::Task::Spawn(void)
{
	if (icon) icon->Spawn();
}

//	Despawn()
// Makes the task's icon disappear.
void IconChallenge::Task::Despawn(void)
{
	if (icon) icon->Despawn();
}

//	CheckForCompletion()
// Private helper function - checks if the player has met the requirements for this task.
void IconChallenge::Task::CheckForCompletion(kellyslater_controller * ksctrl)
{
	entity *	ent;
	float		dist2;
	int			currTrickIdx, compTrickIdx;
	//int			facePoints, airPoints, tubePoints;
	
	// Test: icon must be valid.
	if (!icon)
		return;
	
	// Test: entity must be valid.
	ent = icon->GetEntity();
	if (!ent)
		return;
	
	// Test: player must be close to icon.
	dist2 = (ent->get_abs_position() - ksctrl->GetBoardMember()->get_abs_position()).length2();
	if (dist2 > REQUIRED_DIST2 && ksctrl->GetTrickRegion() != TREGION_TUBE)
		return;

	currTrickIdx = ksctrl->GetCurrentTrick();
	compTrickIdx = ksctrl->GetCompletedTrick();

	switch (type)
	{
	// Test: type must be valid.
	case TYPE_NONE :
		return;

	// Test: player must be executing a flip trick.
	case TYPE_AIR_FLIP :
		if ((currTrickIdx != -1) &&
			(GTrickList[currTrickIdx].flags & AirFlag) &&
			((GTrickList[currTrickIdx].flags & ManualFlag) == 0))
			watchChain = true;
		return;

	// Test: player must be executing a grab trick.
	case TYPE_AIR_GRAB :
		if ((currTrickIdx != -1) &&
			(GTrickList[currTrickIdx].flags & AirFlag) &&
			(GTrickList[currTrickIdx].flags & ManualFlag))
			watchChain = true;
		return;

	// Test: player must be executing an air series.
	case TYPE_AIR_POINTS_1000 :
	case TYPE_AIR_POINTS_2000 :
	case TYPE_AIR_POINTS_5000 :
		if (ksctrl->GetTrickRegion() == TREGION_AIR) watchChain = true;
		return;

	// Test: player must be executing an air series.
	case TYPE_AIR_SPIN :
		if (ksctrl->GetTrickRegion() == TREGION_AIR) watchChain = true;
		return;

	// Test: player must be executing a special trick.
	case TYPE_AIR_SPECIAL :
		if ((currTrickIdx != -1) &&
			(GTrickList[currTrickIdx].flags & SpecialFlag))
			watchChain = true;
		return;

	// Test: player must be executing a tailslide.
	case TYPE_FACE_SPIN :
		if ((compTrickIdx != -1) &&
			(GTrickList[compTrickIdx].flags & FaceFlag) &&
			(compTrickIdx == TRICK_TAILSLIDE))
			watchChain = true;
		return;

	// Test: player must be in the tube.
	case TYPE_TUBE_TIME_10 :
		if (ksctrl->GetTrickRegion() == TREGION_TUBE) watchChain = true;
		return;

	// Test: player must be executing a tube trick.
	case TYPE_TUBE_TRICK :
		if ((compTrickIdx != -1) &&
			(GTrickList[compTrickIdx].flags & TubeFlag) &&
			((GTrickList[compTrickIdx].flags & NormalRideFlag) == 0) &&
			((GTrickList[compTrickIdx].flags & ModRideFlag) == 0))
			watchChain = true;
		return;

	// Test: type must be valid.
	default :
		return;
	};
}

//	OnEvent()
// Handles all incoming events.
void IconChallenge::Task::OnEvent(const EVENT event, kellyslater_controller * ksctrl, const int param2)
{
	const ScoringManager::Chain *				chain;
	const ScoringManager::Series *				series;
	int											seriesFacePoints = 0;
	int											seriesAirPoints = 0;
	int											seriesTubePoints = 0;
	bool										gotIt = false;

	// Get pointers to scoring chain and the last series in the chain.
	if (!ksctrl) return;
	chain = &(ksctrl->get_my_scoreManager().GetChain());
	if (!chain) return;
	if (chain->series.empty()) return;
	series = &(chain->series.back());
	if (!series) return;

	series->GetPartialRawScores(seriesFacePoints, seriesAirPoints, seriesTubePoints);
	
	// Process series end event.
	if (event == EVT_SCORING_SERIES_END && watchChain)
	{
		switch (type)
		{
		// Air tricks: a successful landing is all that's required.
		case TYPE_AIR_FLIP :
		case TYPE_AIR_GRAB :
			if (param2 == 1) gotIt = true;
			break;

		// Air points: series must be worth enough.
		case TYPE_AIR_POINTS_1000 :
			if (param2 == 1 && seriesAirPoints >= 1000) gotIt = true;
			break;
		case TYPE_AIR_POINTS_2000 :
			if (param2 == 1 && seriesAirPoints >= 2000) gotIt = true;
			break;
		case TYPE_AIR_POINTS_5000 :
			if (param2 == 1 && seriesAirPoints >= 5000) gotIt = true;
			break;

		// Air spins: series must have enough revolutions.
		case TYPE_AIR_SPIN :
			if (param2 == 1 && series->numSpins >= 2) gotIt = true;
			break;

		// Air special tricks: don't wipeout.
		case TYPE_AIR_SPECIAL :
			if (param2 == 1) gotIt = true;
			break;

		// Face spins: just don't wipeout.
		case TYPE_FACE_SPIN :
			if (param2 == 1) gotIt = true;
			break;

		// Tube time: series must have a ride entry with a long enough time.
		case TYPE_TUBE_TIME_10 :
			if (param2 == 1 && series->GetTubeTime() > 10.0f) gotIt = true;
			break;

		// Tube trick: don't wipeout.
		case TYPE_TUBE_TRICK :
			if (param2 == 1) gotIt = true;
			break;

		default : assert(0);
		};
		
		if (gotIt)
		{
			completed = true;
			Despawn();

			// Play sfx.
			nslSourceId src = NSL_INVALID_ID;
			nslSoundId snd = NSL_INVALID_ID;
			src = nslLoadSource("ICONDONE");
			if (src != NSL_INVALID_ID)
				snd = nslAddSound(src);
			if (snd != NSL_INVALID_ID)
				nslPlaySound(snd);
		}
		
		watchChain = false;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	IconChallenge::Icon class
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float IconChallenge::Icon::PULSE_MIN = 0.5f;
float IconChallenge::Icon::PULSE_MAX = 1.0f;
float IconChallenge::Icon::PULSE_SPEED = 1.0f;

//	Icon()
// Default constructor.
IconChallenge::Icon::Icon()
{
	parentEnt = NULL;
	fgChildEnt = NULL;
	bgChildEnt = NULL;

	pulseColor = color(PULSE_MIN, PULSE_MIN, PULSE_MIN, 1.0f);
	pulseDir = 1;
}

//	~Icon()
// Destructor.
IconChallenge::Icon::~Icon()
{
	// Entity maker deallocates entity for us.
}

//	Init()
// Must be called after constructing.
void IconChallenge::Icon::Init(const stringx name, const int textureIdx, const vector3d pos, const float scale)
{
	parentEnt = LoadEntity(name);
	if (parentEnt)
	{
		fgChildEnt = ((conglomerate *) parentEnt)->get_member("TEXT");
		assert(fgChildEnt);
		fgChildEnt->SetTextureFrame(textureIdx);
		
		bgChildEnt = ((conglomerate *) parentEnt)->get_member("DISK");
		assert(bgChildEnt);
		bgChildEnt->SetTextureFrame(textureIdx);
	}

	position = pos;
}

//	Update()
// Must be called every frame.
void IconChallenge::Icon::Update(const float dt)
{
	float	c;

	// Don't update unspawned icons.
	if (parentEnt)
	{
		if (!parentEnt->is_active() || !parentEnt->is_visible())
			return;
	}
	
	// Rotate icon so that it always faces the camera.
	if (parentEnt)
	{
		parentEnt->set_rel_po(po(-1,0,0, 0,1,0, 0,0,-1, 0,0,0) * g_game_ptr->get_current_view_camera()->get_rel_po());
		parentEnt->set_rel_position(position);
	}

	// Make background color pulse.
	if (pulseDir == 1)
	{
		c = pulseColor.r + dt*((PULSE_MAX-PULSE_MIN)/PULSE_SPEED);
		if (c >= PULSE_MAX)
		{
			c = PULSE_MAX;
			pulseDir = -1;
		}

		pulseColor.r = c;
		pulseColor.g = c;
		pulseColor.b = c;
		pulseColor.a = 1.0f;
	}
	else
	{
		c = pulseColor.r - dt*((PULSE_MAX-PULSE_MIN)/PULSE_SPEED);
		if (c <= PULSE_MIN)
		{
			c = PULSE_MIN;
			pulseDir = 1;
		}

		pulseColor.r = c;
		pulseColor.g = c;
		pulseColor.b = c;
		pulseColor.a = 1.0f;
	}
	bgChildEnt->set_render_color(pulseColor.r*255.0f, pulseColor.g*255.0f, pulseColor.b*255.0f, pulseColor.a*255.0f);
}

//	Spawn()
// Makes the icon appear.
void IconChallenge::Icon::Spawn(void)
{
	if (parentEnt)
	{
		parentEnt->set_visible(true);
		parentEnt->set_active(true);
	}
}

//	Despawn()
// Makes the icon disappear.
void IconChallenge::Icon::Despawn(void)
{
	if (parentEnt)
	{
		parentEnt->set_active(false);
		parentEnt->set_visible(false);
	}
}

//	IsSpawned()
// Returns true if the icon is visible.
bool IconChallenge::Icon::IsSpawned(void) const
{
	return parentEnt && parentEnt->is_active() && parentEnt->is_visible();
}

//	LoadEntity()
// Private helper function - loads the specified entity.
entity * IconChallenge::Icon::LoadEntity(const stringx name) const
{
	stringx		meshPath = "items\\"+name+"\\entities\\";
	stringx		texturePath = "items\\"+name+"\\" + os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\";
	entity *	e = NULL;
  
	nglSetMeshPath(meshPath.c_str());
	nglSetTexturePath(texturePath.c_str());
	
	if (!file_finder_exists(meshPath+name, ".ent", 0))
	{
		nglPrintf ("Could not find entity %s\n", name.c_str ());
	}
	else
	{
		e = g_entity_maker->create_entity_or_subclass(meshPath+name, entity_id::make_unique_id(), po_identity_matrix, empty_string, entity::INACTIVE, NULL);
		
		if (e != NULL)
		{
			e->set_visible(false);
			e->set_parsed_name(name);
		}
	}

	return e;
}