
#include "global.h"
#include "challenge_photo.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	PhotoChallenge class
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const float PhotoChallenge::TIME_RETICLE = 5.0f;
const float PhotoChallenge::TAKE_RANGE2 = 35.0f*35.0f;//17.0f*17.0f;
const float PhotoChallenge::TIME_SPECIAL_WAIT = 5.0f;

//	PhotoChallenge()
// Default constructor.
PhotoChallenge::PhotoChallenge()
{
	ksctrl = NULL;
	goal = GOAL_NOTHING;
	requiredScore = 0;
	state = STATE_NONE;
	recordChain = false;
	specialPhotoTimer = 0.0f;
	
	numCameramen = 0;
	activeCameramanIdx = -1;
	cameramen = NULL;

	numTaken = 0;
	numPhotos = 0;
	photos = NULL;
}

//	~PhotoChallenge()
// Destructor.
PhotoChallenge::~PhotoChallenge()
{
	delete [] cameramen;
	delete [] photos;
}

//	Init()
// One-time initialization for the challenge.
void PhotoChallenge::Init(kellyslater_controller * ks, const int g, const int reqScore, beach_object * firstObject,
						  const int maxPhotos, const int photoWidth, const int photoHeight)
{
	beach_object *	currObject = NULL;
	int				i = 0;
	
	ksctrl = ks;
	goal = g;
	requiredScore = reqScore;
	state = STATE_NONE;
	recordChain = false;

	// Count the number of cameramen in the beach
	activeCameramanIdx = -1;
	numCameramen = 1;	// 0th cameraman is a special extra one
	for (currObject = firstObject; currObject != NULL; currObject = currObject->next)
	{
		if (currObject->get_entity() && currObject->get_entity()->get_parsed_name() == "cameraman")
			numCameramen++;
	}

	// Allocate cameramen.
	cameramen = NEW Cameraman[numCameramen];
	for (currObject = firstObject, i = 1; currObject != NULL; currObject = currObject->next)
	{
		if (currObject->get_entity() && currObject->get_entity()->get_parsed_name() == "cameraman")
		{
			cameramen[i++].Init(currObject->get_entity());
		}
	}

	// Allocate photos.
	numTaken = 0;
	numPhotos = maxPhotos;
	if (numPhotos > 0) photos = NEW Photo[numPhotos];
	for (i = 0; i < numPhotos; i++)
		photos[i].Init(photoWidth, photoHeight);
}

//	Retry()
// Restarts the challenge: called when the player retries the level.
void PhotoChallenge::Retry(void)
{
	int i = 0;
	
	state = STATE_NONE;
	recordChain = false;
	specialPhotoTimer = 0.0f;

	activeCameramanIdx = -1;
	for (i = 0; i < numCameramen; i++)
		cameramen[i].Reset();

	numTaken = 0;
	for (i = 0; i < numPhotos; i++)
		photos[i].Reset();
}

#if defined(TOBY) && defined(TARGET_PS2)
static int toby_photo_hack = 0;
#include "compressedphoto.h"
extern bool nglExportTextureTGA( nglTexture* Tex, const char *Name );
#endif

//	Update()
// Must be called every frame.
void PhotoChallenge::Update(const float dt)
{
	int		i = 0;
	int		scoringMethod = 1;

#if defined(TOBY) && defined(TARGET_PS2)
	if (toby_photo_hack)
	{
		CompressedPhoto	compressedPhoto;

		mem_lock_malloc(false);
		nglExportTextureTGA(photos[0].GetTexture(), "host:orig.tga");
		compressedPhoto.CopyFromTexture(photos[0].GetTexture());
		compressedPhoto.ExportToDDS("host:toby.dds");
		compressedPhoto.CopyToTexture(photos[1].GetTexture());
		nglExportTextureTGA(photos[1].GetTexture(), "host:toby.tga");
		numTaken++;
		mem_lock_malloc(true);
		toby_photo_hack = 0;
	}
#endif

	if (recordChain)
		photos[numTaken-1].SetScore(ksctrl->get_my_scoreManager().GetChain().GetScore());

	// Update cameramen.
	for (i = 0; i < numCameramen; i++)
		cameramen[i].Update(dt);

	if (specialPhotoTimer > 0.0f)
	{
		specialPhotoTimer -= dt;
		if (specialPhotoTimer < 0.0f)
			specialPhotoTimer = 0.0f;
	}
	
	switch (state)
	{
	case STATE_NONE :
		// Take surfer's picture if he is doing something interesting.
		if (numTaken < numPhotos &&
			(ksctrl->IsDoingTrick() || ksctrl->get_super_state() == SUPER_STATE_NORMAL_SURF || ksctrl->get_super_state() == SUPER_STATE_AIR))
		{
			// Check all cameramen in the beach.
			for (i = 0; i < numCameramen; i++)
			{
				// Cameraman 0 is special.
				if (i == 0 && goal == GOAL_PHOTO_3 && cameramen[i].GetState() != Cameraman::CSTATE_TAKING &&
					specialPhotoTimer == 0.0f && ksctrl->get_special_meter()->CanRegionLink())
				{
					activeCameramanIdx = i;
					cameramen[activeCameramanIdx].BeginTakingPicture(ksctrl, &photos[numTaken]);
					state = STATE_RETICLE;			
				}
				// Cameraman is valid and close?
				else if ((goal == GOAL_PHOTO_1 || goal == GOAL_PHOTO_2) &&
					cameramen[i].GetState() == Cameraman::CSTATE_NONE && cameramen[i].IsCloseToSurfer(ksctrl))
				{
					// Advance to reticle state.
					activeCameramanIdx = i;
					cameramen[activeCameramanIdx].BeginTakingPicture(ksctrl, &photos[numTaken]);
					state = STATE_RETICLE;			
					break;
				}
			}
		}
		break;

	case STATE_RETICLE :
		if (activeCameramanIdx != -1 && cameramen[activeCameramanIdx].GetState() == Cameraman::CSTATE_TOOK)
		{
			numTaken++;
			// Scoring method 0: entire chain
			if (scoringMethod == 0)
				recordChain = true;
			// Scoring method 1: chain up to when photo was taken
			else if (scoringMethod == 1)
				photos[numTaken-1].SetScore(ksctrl->get_my_scoreManager().GetChain().GetScore());
			// Scoring method 2: series up to when photo was taken
			else if (scoringMethod == 2)
			{
				if (!ksctrl->get_my_scoreManager().GetChain().series.empty())
					photos[numTaken-1].SetScore(ksctrl->get_my_scoreManager().GetChain().series.back().GetRawScore());
				else
					photos[numTaken-1].SetScore(0);
			}
			activeCameramanIdx = -1;
			state = STATE_TAKE;
		}
		break;

	case STATE_TAKE :
		if (g_game_ptr->get_snapshot_state() == game::SNAPSTATE_NONE)
		{
			if (goal == GOAL_PHOTO_3) specialPhotoTimer = TIME_SPECIAL_WAIT;
			photos[numTaken-1].Show(numTaken);
			state = STATE_SHOW;
		}
		break;

	case STATE_SHOW :
		state = STATE_NONE;
		break;
	}
}

#ifdef TOBY
void PhotoChallenge::Debug_TakePhoto(void)
{
	if (numTaken < numPhotos)
	{
		activeCameramanIdx = 0;
		cameramen[activeCameramanIdx].BeginTakingPicture(ksctrl, &photos[numTaken]);
		state = STATE_RETICLE;
	}
}
#endif

//	OnEvent()
// Responds to all incoming events.
void PhotoChallenge::OnEvent(const EVENT event, const int param1, const int param2)
{
	if (event == EVT_SCORING_CHAIN_END)
		recordChain = false;
}

//	CheckForCompletion()
// Checks if this goal was completed.
// Must be called every frame.
bool PhotoChallenge::CheckForCompletion(void)
{
	bool	completed = false;
	
	switch (goal)
	{
	case GOAL_PHOTO_1:
		if (!recordChain &&
			(*photos[0].GetScore() + *photos[1].GetScore() + *photos[2].GetScore() >= requiredScore))
			completed = true;
		break;
		
	case GOAL_PHOTO_2:
		if (!recordChain &&
			(*photos[0].GetScore() >= requiredScore || *photos[1].GetScore() >= requiredScore || *photos[2].GetScore() >= requiredScore))
			completed = true;
		break;
		
	case GOAL_PHOTO_3:
		if (!recordChain &&
			((*photos[0].GetScore() >= requiredScore && photos[0].IsOfSpecialTrick()) ||
			(*photos[1].GetScore() >= requiredScore && photos[1].IsOfSpecialTrick()) ||
			(*photos[2].GetScore() >= requiredScore && photos[2].IsOfSpecialTrick())))
			completed = true;
		break;

	default : assert(false);
	}

	return completed;
}

//	GetPhotoScore()
// Returns the score earned for the specified photo.
int * PhotoChallenge::GetPhotoScore(const int photoIdx) const
{
	assert(photoIdx >= 0 && photoIdx < numPhotos);

	return photos[photoIdx].GetScore();
}

//	GetPhotoTexture()
// Returns the texture of the specified photo.
nglTexture * PhotoChallenge::GetPhotoTexture(const int photoIdx)
{
	assert(photoIdx >= 0 && photoIdx < numPhotos);

	return photos[photoIdx].GetTexture();
}

bool PhotoChallenge::GetPhotoIsOfSpecialTrick(const int photoIdx)
{
	assert(photoIdx >= 0 && photoIdx < numPhotos);
	return photos[photoIdx].IsOfSpecialTrick();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	PhotoChallenge::Photo class
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//	Photo()
// Default constructor.
PhotoChallenge::Photo::Photo()
{
	texture = NULL;
	score = 0;
	isOfSpecialTrick = false;
}

//	~Photo()
// Destructor.
PhotoChallenge::Photo::~Photo()
{
	nglDestroyTexture(texture);
}

//	Init()
// One-time initialization.
void PhotoChallenge::Photo::Init(const int width, const int height)
{
#ifdef TARGET_XBOX
	texture = nglCreateTexture(NGLTF_32BIT|NGLTF_LINEAR, width, height);
#else
	texture = nglCreateTexture(NGLTF_32BIT, width, height);
#endif
	score = 0;
	isOfSpecialTrick = false;
}

//	Reset()
// Resets default values.
void PhotoChallenge::Photo::Reset(void)
{
	score = 0;
	isOfSpecialTrick = false;
}

//	Show()
// Displays the photo onscreen.
void PhotoChallenge::Photo::Show(const int label)
{
	frontendmanager.IGO->ShowPhoto(texture, &score, label);
}

//	CheckProperties()
// Checks the subject's current chain to see if surfer did a special trick.
void PhotoChallenge::Photo::CheckProperties(kellyslater_controller * subject)
{
	int	trickIdx;
	
	isOfSpecialTrick = false;
	
	if (subject)
	{
		trickIdx = subject->GetCurrentTrick();
		if (trickIdx >= 0 && GTrickList[trickIdx].flags & SpecialFlag)
			isOfSpecialTrick = true;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	PhotoChallenge::Cameraman class
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//	Cameraman()
// Default constructor.
PhotoChallenge::Cameraman::Cameraman()
{
	ent = NULL;
	state = CSTATE_NONE;
	destPhoto = NULL;
	targetKsctrl = NULL;
}

//	Init()
// One-time initializer for the cameraman.
void PhotoChallenge::Cameraman::Init(entity * e)
{
	ent = e;
	state = CSTATE_NONE;
	destPhoto = NULL;
	targetKsctrl = NULL;
}

//	Reset()
// Resets default values.
void PhotoChallenge::Cameraman::Reset(void)
{
	state = CSTATE_NONE;
}

//	Update()
// Must be called every frame.
void PhotoChallenge::Cameraman::Update(const float dt)
{
	switch (state)
	{
	case CSTATE_NONE :
		break;

	case CSTATE_TAKING :
		if (frontendmanager.IGO->GetCameraReticleFade() == 1.0f)
		{
			frontendmanager.IGO->HideCameraReticle();
			if (destPhoto)
			{
				g_game_ptr->take_snapshot(destPhoto->GetTexture());
				destPhoto->CheckProperties(targetKsctrl);
			}
			state = CSTATE_TOOK;
		}
		break;

	case CSTATE_TOOK :
		break;
	}
}

//	BeginTakingPicture()
// Fades in the reticle at a set speed.
// Cameraman will take the picture when reticle reaches full alpha.
void PhotoChallenge::Cameraman::BeginTakingPicture(kellyslater_controller * target, Photo * photo)
{
	frontendmanager.IGO->ShowCameraReticle(TIME_RETICLE);
	destPhoto = photo;
	targetKsctrl = target;
	state = CSTATE_TAKING;
}

//	IsCloseToSurfer()
// Returns true if the cameraman is within picture-taking range of the specified surfer.
bool PhotoChallenge::Cameraman::IsCloseToSurfer(kellyslater_controller * ksctrl) const
{
	float	dist2 = 0.0f;	// squared distance between cameraman and surfer
	
	if (ksctrl && ent && ent->is_active() && ent->is_visible())
	{
		dist2 = (ent->get_abs_position() - ksctrl->get_board_controller().my_board->get_abs_position()).length2();

		if (dist2 <= TAKE_RANGE2)
			return true;
	}
	return false;
}
