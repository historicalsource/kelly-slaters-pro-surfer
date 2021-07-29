#include "ksreplay.h"

const unsigned int MAXREPLAYFRAMES          = 10800;  // 10800 = 3 mins * 60 seconds * 60 fps
const unsigned int MAXCOLLISIONS            = 100;


const unsigned int SLOMOFACTOR              = 5;
const unsigned int FASTFORWARDFACTOR        = 3;

const unsigned int MAINENTITY_UPDATEFRAMES  = 4;      // Save the PO info for main surfer every X frames
const unsigned int AIENTITY_UPDATEFRAMES    = 8;      // Save the PO info for ai   surfer every X frames

// Specifies to load and save the animation index when loading from file
#define KSREPLAY_ANIM_LOAD
#define KSREPLAY_ANIM_SAVE
#define ANIMNAMELEN               32

KSReplay ksreplay;

static void interpolate_quat(const quaternion &from, const quaternion &to, quaternion &res, float interp)
{
  const float DELTA = 0.001;    // Threshold of whether or not to do a simple linear interpolation

  float sinom, cosom, omega, s0, s1;

  quaternion q1 = from;
  quaternion q2 = to;

  // adjust signs (if necessary)
  cosom = q1.a*q2.a + q1.b*q2.b + q1.c*q2.c + q1.d*q2.d;

  if (cosom < 0.0)
  {
    cosom = -cosom;
    q2.a  = -q2.a;
		q2.b  = -q2.b;
		q2.c  = -q2.c;
		q2.d  = -q2.d;
  }

  // calculate coefficients
  if ((1.0 - cosom) > DELTA)
  {
    omega = acosf(cosom);
    sinom = sinf(omega);
    s0 = sinf((1.0 - interp) * omega) / sinom;
    s1 = sinf(interp * omega) / sinom;
  }
  else
  {
    // q1 and q2 quaternions are very close
	  //  ... so we can do a linear interpolation
    s0 = 1.0 - interp;
    s1 = interp;
  }

	// calculate final values
	res.a = s0*q1.a + s1*q2.a;
	res.b = s0*q1.b + s1*q2.b;
	res.c = s0*q1.c + s1*q2.c;
	res.d = s0*q1.d + s1*q2.d;
}

///////////////////////////////////////////////////////////////////////////////
// KSEntityPO
///////////////////////////////////////////////////////////////////////////////
KSEntityPO::KSEntityPO()
{
}

void KSEntityPO::Save( int player )
{
#ifdef USE_POS
  KSPo   = g_world_ptr->get_ks_controller(player)->get_owner_po();
  KSBPo  = g_world_ptr->get_board_ptr(player)->get_rel_po();
#else
  po KSPo   = g_world_ptr->get_ks_controller(player)->get_owner_po();
  po KSBPo  = g_world_ptr->get_board_ptr(player)->get_rel_po();

  KSPos     = KSPo.get_position();
  KSBPos    = KSBPo.get_position();
  KSRot     = KSPo.get_quaternion();
  KSBRot    = KSBPo.get_quaternion();
  BoardCurrent    = g_world_ptr->get_ks_controller(player)->get_board_controller().current;
  BoardMomentum   = g_world_ptr->get_ks_controller(player)->get_board_controller().rb->linMom;
#endif
}


void KSEntityPO::Restore( int player )
{
#ifdef USE_POS
  g_world_ptr->get_ks_controller(player)->set_owner_po(KSPo);
  g_world_ptr->get_board_ptr(player)->set_rel_po(KSBPo);
#else
  po KSPo(KSPos, KSRot, 1.0f);
  po KSBPo(KSBPos, KSBRot, 1.0f);

  g_world_ptr->get_ks_controller(player)->set_owner_po(KSPo);
  g_world_ptr->get_board_ptr(player)->set_rel_po(KSBPo);

  g_world_ptr->get_ks_controller(player)->get_board_controller().current   = BoardCurrent;
  g_world_ptr->get_ks_controller(player)->get_board_controller().rb->linMom = BoardMomentum;
#endif
}

void interpolate_entity_po(const KSEntityPO &po1, const KSEntityPO &po2, KSEntityPO &res, float interp)
{
  const float interp_threshold = 5.0f;    // If distances are this big, don't interpolate the pos or rot

#ifdef USE_POS
  interpolate_po(po1.KSPo, po2.KSPo, res.KSPo, interp);
  interpolate_po(po1.KSBPo, po2.KSBPo, res.KSBPo, interp);
#else
  // Board current
  res.BoardCurrent.x = (1.0f-interp)*po1.BoardCurrent.x + interp*po2.BoardCurrent.x;
  res.BoardCurrent.y = (1.0f-interp)*po1.BoardCurrent.y + interp*po2.BoardCurrent.y;
  res.BoardCurrent.z = (1.0f-interp)*po1.BoardCurrent.z + interp*po2.BoardCurrent.z;

  // Board Linear Momentum
  res.BoardMomentum.x = (1.0f-interp)*po1.BoardMomentum.x + interp*po2.BoardMomentum.x;
  res.BoardMomentum.y = (1.0f-interp)*po1.BoardMomentum.y + interp*po2.BoardMomentum.y;
  res.BoardMomentum.z = (1.0f-interp)*po1.BoardMomentum.z + interp*po2.BoardMomentum.z;

  // If distance changed a lot, don't interpolate anything
  if(fabsf(po1.KSPos.x - po2.KSPos.x) > interp_threshold || fabsf(po1.KSPos.y - po2.KSPos.y) > interp_threshold || fabsf(po1.KSPos.z - po2.KSPos.z) > interp_threshold)
  {
    // Horrible hack due to different behavior from restarting the level after a duck dive as opposed to a wipeout.
    if(ksreplay.mainEntityState[ksreplay.playframe].KSSuperState != SUPER_STATE_WIPEOUT)
      res = po1;
    else
      res = po2;
    return;
  }

  // Surfer position
  res.KSPos.x = (1.0f-interp)*po1.KSPos.x + interp*po2.KSPos.x;
  res.KSPos.y = (1.0f-interp)*po1.KSPos.y + interp*po2.KSPos.y;
  res.KSPos.z = (1.0f-interp)*po1.KSPos.z + interp*po2.KSPos.z;

  // Surfer rotation
  interpolate_quat(po1.KSRot, po2.KSRot, res.KSRot, interp);

  // Board position
  res.KSBPos.x = (1.0f-interp)*po1.KSBPos.x + interp*po2.KSBPos.x;
  res.KSBPos.y = (1.0f-interp)*po1.KSBPos.y + interp*po2.KSBPos.y;
  res.KSBPos.z = (1.0f-interp)*po1.KSBPos.z + interp*po2.KSBPos.z;

  // Board rotation
  interpolate_quat(po1.KSBRot, po2.KSBRot, res.KSBRot, interp);
#endif
}

///////////////////////////////////////////////////////////////////////////////
// KSEntityState
///////////////////////////////////////////////////////////////////////////////
KSEntityState::KSEntityState()
{
}

void KSEntityState::Reset( void )
{
  // Only reset the data that doesn't get overwritten every frame
	KSLoop = KSBLoop = KSAnimCall = KSBAnimCall = KSWipeoutSplash = EndWave = false;
	KSBlend = KSBBlend = 0;
}

void KSEntityState::Save( int player )
{
  // Grab the surfer state
  KSState   = g_world_ptr->get_ks_controller(player)->get_current_state();
  KSSuperState = g_world_ptr->get_ks_controller(player)->get_super_state();

  // Grab anim info
  KSAnim    = g_world_ptr->get_ks_controller(player)->GetCurrentAnim();
  KSBAnim   = g_world_ptr->get_ks_controller(player)->GetCurrentBoardAnim();

  KSCurTrick = g_world_ptr->get_ks_controller(player)->GetCurrentTrick();

  // Grab frame info
  //KSFrame   = g_world_ptr->get_ks_controller(player)->GetCurrentFrame();
  //KSBFrame  = g_world_ptr->get_ks_controller(player)->GetBoardAnimTime();

  // Grab various flags
  KSInAir     = g_world_ptr->get_ks_controller(player)->get_board_controller().inAirFlag;
  KSInTube    = g_world_ptr->get_ks_controller(player)->get_board_controller().GetRegion() == WAVE_REGIONTUBE;
  KSDry       = g_world_ptr->get_ks_controller(player)->isDry();
  KSIKValid   = g_world_ptr->get_ks_controller(player)->get_ik_valid();
  KSWipedOut  = g_world_ptr->get_ks_controller(player)->get_board_controller().WipedOut ();
}

/*void KSEntityState::Restore( int player )
{
  // Set surfer states
  g_world_ptr->get_ks_controller(player)->set_state(KSState);
  g_world_ptr->get_ks_controller(player)->set_super_state(KSSuperState);

  //Set the anim and frame info
  g_world_ptr->get_ks_controller(player)->SetAnimAndFrame(KSAnim, KSBAnim, KSFrame, blendKS/100.0f, blendBoard/100.0f, KSLoop, BLoop, KSAnimCall, BAnimCall );

  // Set various flags
  g_world_ptr->get_ks_controller(player)->set_ik_valid(IKValid?true:false);
  g_world_ptr->get_ks_controller(player)->get_board_controller().wiped_out = KSWipedOut;
  g_world_ptr->get_ks_controller(player)->get_board_controller().inAirFlag = KSInAir;
}*/

void KSEntityState::SetKSAnimInfo(float blend, bool loop, float frame)
{
  KSBlend           = (char)(100.0f*blend);
  KSLoop      = loop;
  KSAnimCall  = true;
  //KSFrame       = frame;
}

void KSEntityState::SetKSBAnimInfo(float blend, bool loop, float frame)
{
  KSBBlend          = (char)(100.0f*blend);
  KSBLoop     = loop;
  KSBAnimCall = true;
  //KSBFrame      = frame;
}


///////////////////////////////////////////////////////////////////////////////
// KSReplayFrame
///////////////////////////////////////////////////////////////////////////////
KSReplayFrame::KSReplayFrame()
{
}

void KSReplayFrame::Save()
{
  wave_shiftx = WAVE_ShiftX;
  levelTime   = TIMER_GetLevelSec();
  totalTime   = TIMER_GetTotalSec();
}


///////////////////////////////////////////////////////////////////////////////
// KSReplay
///////////////////////////////////////////////////////////////////////////////
KSReplay::KSReplay()
{
	status            = REPLAY_IGNORE;

  playtime          = 0.0f;
  lastPlaytime      = 0.0f;
  slomo             = false;
  fastforward       = false;

  slomospeed        = SLOMOFACTOR;
  ffspeed           = FASTFORWARDFACTOR;

	playframe         = 0;
	lastPlayframe     = 0;
  interpFrame       = 0;

  frameTicks        = 0;
  interpTicks       = 0;

  aiSurfer          = false;
	numFrames         = 0;
	frame             = NULL;
	mainEntityState   = NULL;
	aiEntityState     = NULL;
	mainEntityPO      = NULL;
	aiEntityPO        = NULL;
	maxframes         = 0;

  noDraw            = false;

  collisions        = NULL;
  current_collision = 0;
  num_collisions    = 0;
}

KSReplay::~KSReplay()
{
  Term();
}

#ifdef DEBUG
const int safetybuffer = 512*1024;
#else

#if defined(TARGET_XBOX)
const int safetybuffer = 100*1024;
#elif defined(TARGET_GC)
const int safetybuffer = 512 * 1024; //ARGH!!! Stop taking all our memory!!
#else //TARGET_PS2
const int safetybuffer = 10*1024;
#endif

#endif

#define SUCKALLREMAININGRAM
#define MUSTHAVEFULLREPLAY

#ifndef MIN
#define MIN(a,b) ((a) <= (b) ? (a) : (b))
#endif

void KSReplay::Init( bool ai )
{
  if(maxframes > 0)
    return;

  aiSurfer = ai;

  // Create the buffer that saves collisions
  collisions = NEW Collision[MAXCOLLISIONS];
  assert(collisions && "Zero RAM left for replay");
  memset(collisions, 0, MAXCOLLISIONS*sizeof(Collision));
  num_collisions    = 0;
  current_collision = 0;

  float memperframe = 0.0f;

  // If in demo mode, allocate the number of frames already read from file
  if(dmm.inDemoMode())
    maxframes = dmm.replayFrames;
  else
  {
	  // Find out how much mem is free
	  int mfree = mem_get_largest_avail() - safetybuffer;
	  debug_print( "%d bytes free total, %d is largest available\n", mem_get_total_avail(), mem_get_largest_avail() );

		assert (mfree >= 0);	// otherwise maxframes will turn into a very large number (dc 07/01/02)

	  memperframe = (float)sizeof(KSReplayFrame) + (float)sizeof(KSEntityState)
		  + (float)(sizeof(KSEntityPO) + MAINENTITY_UPDATEFRAMES - 1)/MAINENTITY_UPDATEFRAMES;
	  if(aiSurfer)
	  {
		  memperframe += (float)sizeof(KSEntityState)
			  + (float)(sizeof(KSEntityPO) + AIENTITY_UPDATEFRAMES - 1)/AIENTITY_UPDATEFRAMES;
	  }

#ifndef SUCKALLREMAININGRAM
      maxframes = MIN(MAXREPLAYFRAMES,(int)(mfree/memperframe));
#else
      maxframes = (int)(mfree/memperframe);
#endif

    // Check if there is enough mem free
    if(maxframes <= 0)
    {
      maxframes = 0;
	    debug_print("Zero RAM left for replay buffer\n");
	    assert(0 && "Zero RAM left for replay buffer\n");
      return;
    }
    if (maxframes < (int) MAXREPLAYFRAMES)
    {
	    debug_print("Not enough RAM for full replay length: %d frames (about %d:%.2d minutes) max\n", maxframes, maxframes/3600, (maxframes/60)%60);
  #ifdef MUSTHAVEFULLREPLAY
	    assert(0 && "Not enough RAM for full replay\n");
  #endif
    }
  }

  // Create the replay buffers
	frame = NEW KSReplayFrame[maxframes];
  assert(frame);
  memset(frame, 0, sizeof(KSReplayFrame)*maxframes);

  mainEntityState = NEW KSEntityState[maxframes];
  assert(mainEntityState);
  memset(mainEntityState, 0, sizeof(KSEntityState)*maxframes);

  int maxmainpoframes = dmm.inDemoMode() ? dmm.replayMainPOFrames : (maxframes/MAINENTITY_UPDATEFRAMES);
  mainEntityPO = NEW KSEntityPO[maxmainpoframes];
  assert(mainEntityPO);
  memset(mainEntityPO, 0, sizeof(KSEntityPO)*maxmainpoframes);

  // Create the AI surfer buffers if necessary
  if(aiSurfer)
  {
    aiEntityState = NEW KSEntityState[maxframes];
    assert(aiEntityState);
    memset(aiEntityState, 0, sizeof(KSEntityState)*maxframes);

    int maxaipoframes = dmm.inDemoMode() ? dmm.replayAIPOFrames : (maxframes/AIENTITY_UPDATEFRAMES);
    aiEntityPO = NEW KSEntityPO[maxaipoframes];
    assert(aiEntityPO);
    memset(aiEntityPO, 0, sizeof(KSEntityPO)*maxaipoframes);
  }

  if(!dmm.inDemoMode())
  {
    float mneeded = (MAXREPLAYFRAMES*memperframe)/1024.0f;
    float mtotal  = (maxframes*memperframe)/1024.0f;
    float mextra  = mtotal-mneeded;

	  debug_print("Replay buffer allocated: %d frames (about %d:%.2d minutes) max\n",maxframes,maxframes/3600, (maxframes/60)%60);
	  debug_print("Memory needed for 3 minute replay:%5ik\n", (int)mneeded);
	  debug_print("Extra memory available:           %5ik\n", (int)mextra);
	  debug_print("Total memory assigned to replay:  %5ik\n", (int)mtotal);
  }
}

void KSReplay::Clear(uint32 s)
{
  seed                = s;
  numFrames           = 0;
  num_collisions      = 0;
  current_collision   = 0;
  status              = REPLAY_IGNORE;
}

void KSReplay::Term()
{
  if(collisions)
    delete [] collisions;
  collisions = NULL;
  num_collisions    = 0;
  current_collision = 0;

  if(frame)
    delete [] frame;
  frame = NULL;

  if(mainEntityState)
    delete [] mainEntityState;
  mainEntityState = NULL;

  if(mainEntityPO)
    delete [] mainEntityPO;
  mainEntityPO = NULL;

  if(aiEntityState)
    delete [] aiEntityState;
  aiEntityState = NULL;

  if(aiEntityPO)
    delete [] aiEntityPO;
  aiEntityPO = NULL;

  maxframes = 0;
}

void KSReplay::LoadFile(char *fname)
{
  int i;

  char bchName[32];

	bool memlocked = mem_malloc_locked();
	mem_lock_malloc(false);

  // Disable the stash only flag if set (since demo files are not in stashes)
  nglFileBuf file;
  bool stashOnly = os_developer_options::inst()->is_flagged(os_developer_options::FLAG_STASH_ONLY);
  os_developer_options::inst()->set_flag(os_developer_options::FLAG_STASH_ONLY, false);

  KSReadFile(fname, &file, 1);

  int offset = 0;

  sfr   = *((int *)(&file.Buf[offset]));
  offset += 4;

  brd   = *((int *)(&file.Buf[offset]));
  offset += 4;

  strcpy(bchName, (char *)(&file.Buf[8]));
  offset += 32;

  initWaterState = *((KSWaterState *)(&file.Buf[offset]));
  offset += sizeof(KSWaterState);

  numFrames   = *((int *)(&file.Buf[offset]));
  offset += 4;

  numMainPOFrames   = *((int *)(&file.Buf[offset]));
  offset += 4;

  numAIPOFrames   = *((int *)(&file.Buf[offset]));
  offset += 4;

  seed   = *((uint32 *)(&file.Buf[offset]));
  offset += 4;

  memcpy(frame, &file.Buf[offset], numFrames*sizeof(KSReplayFrame));
  offset += numFrames*sizeof(KSReplayFrame);

  memcpy(mainEntityState, &file.Buf[offset], numFrames*sizeof(KSEntityState));
  offset += numFrames*sizeof(KSEntityState);

  memcpy(mainEntityPO, &file.Buf[offset], numMainPOFrames*sizeof(KSEntityPO));
  offset += numMainPOFrames*sizeof(KSEntityPO);


  if(aiSurfer)
  {
    memcpy(aiEntityState, &file.Buf[offset], numFrames*sizeof(KSEntityState));
    offset += numFrames*sizeof(KSEntityState);

    memcpy(aiEntityPO, &file.Buf[offset], numAIPOFrames*sizeof(KSEntityPO));
    offset += numAIPOFrames*sizeof(KSEntityPO);
  }

  int numEntries = sizeof(CareerDataArray) / sizeof(CareerDataArray[0]);
  for(i=0; i<numEntries; i++)
  {
    if(0 == strcmp(bchName, CareerDataArray[i].name))
      break;
  }

  if(i == numEntries)
    assert(0 && "Beach file not found");

  bch = i;

#ifdef KSREPLAY_ANIM_LOAD
  int sAnimIndexLen, bAnimIndexLen;
  char *sAnimIndex = NULL;
  char *bAnimIndex = NULL;

  sAnimIndexLen = *((int *)(&file.Buf[offset]));
  offset += 4;
  if(sAnimIndexLen > 0)
  {
    sAnimIndex = NEW char[sAnimIndexLen];
    assert(sAnimIndex);
    memcpy(sAnimIndex, &file.Buf[offset], sAnimIndexLen);
    offset += sAnimIndexLen;
  }

  bAnimIndexLen = *((int *)(&file.Buf[offset]));
  offset += 4;
  if(bAnimIndexLen > 0)
  {
    bAnimIndex = NEW char[bAnimIndexLen];
    assert(bAnimIndex);
    memcpy(bAnimIndex, &file.Buf[offset], bAnimIndexLen);
    offset += bAnimIndexLen;
  }

  bool found;
  int a;
  for(unsigned f=0; f<numFrames; f++)
  {
    found = false;
    for(a=0; a<_SURFER_NUM_ANIMS; a++)
    {
      if(0 == strcmp(g_surfer_anims[a].c_str(), &sAnimIndex[ANIMNAMELEN*mainEntityState[f].KSAnim]))
      {
        found = true;
        break;
      }
    }
    if(found)
      mainEntityState[f].KSAnim = a;
    else
      mainEntityState[f].KSAnim = SURFER_ANIM_PLACEHOLDER;

    found = false;
    for(a=0; a<_BOARD_NUM_ANIMS; a++)
    {
      if(0 == strcmp(g_board_anims[a].c_str(), &bAnimIndex[ANIMNAMELEN*mainEntityState[f].KSBAnim]))
      {
        found = true;
        break;
      }
    }
    if(found)
      mainEntityState[f].KSBAnim = a;
    else
      mainEntityState[f].KSBAnim = BOARD_DUMMY_ANIM1;
  }

  delete [] sAnimIndex;
  delete [] bAnimIndex;
#endif
  KSReleaseFile(&file);
  os_developer_options::inst()->set_flag(os_developer_options::FLAG_STASH_ONLY, stashOnly);

	mem_lock_malloc(memlocked);
}

void KSReplay::SaveFile(char *fname)
{
  bool was_locked = false;
  char newFname[30];

  // Open the file
  os_file replayData;
	if(os_file::is_system_locked())
	{
		os_file::system_unlock();
		was_locked = true;
	}
  if (fname != NULL)
    replayData.open( fname, os_file::FILE_WRITE );
  else
  {
    int replay=0;
    sprintf(newFname, "REPLAY%d.RPL", replay);
    while (os_file::file_exists(newFname))
    {
      replay++;
      sprintf(newFname, "REPLAY%d.RPL", replay);
    }
    replayData.open( newFname, os_file::FILE_WRITE );
  }

  sfr = g_game_ptr->GetSurferIdx(0);
  brd = g_game_ptr->GetBoardIdx(0);
  bch = g_game_ptr->get_level_id();

  replayData.write((void *)&sfr,    sizeof(int));
  replayData.write((void *)&brd,    sizeof(int));
  replayData.write(CareerDataArray[bch].name, 32);
  replayData.write((void *)&initWaterState, sizeof(KSWaterState));
	replayData.write((void *)&numFrames, sizeof(int));
	replayData.write((void *)&numMainPOFrames, sizeof(int));
	replayData.write((void *)&numAIPOFrames, sizeof(int));
	replayData.write((void *)&seed, sizeof(uint32));
	replayData.write((void *)frame, numFrames*sizeof(KSReplayFrame));
	replayData.write((void *)mainEntityState, numFrames*sizeof(KSEntityState));
	replayData.write((void *)mainEntityPO, numMainPOFrames*sizeof(KSEntityPO));
  if(aiSurfer)
  {
	  replayData.write((void *)aiEntityState, numFrames*sizeof(KSEntityState));
	  replayData.write((void *)aiEntityPO, numAIPOFrames*sizeof(KSEntityPO));
  }

#ifdef KSREPLAY_ANIM_SAVE
	bool memlocked = mem_malloc_locked();
	mem_lock_malloc(false);

  int sAnimIndexLen = ANIMNAMELEN * _SURFER_NUM_ANIMS;
  int bAnimIndexLen = ANIMNAMELEN * _BOARD_NUM_ANIMS;
  char *sAnimIndex = NEW char[ANIMNAMELEN * _SURFER_NUM_ANIMS];
  char *bAnimIndex = NEW char[ANIMNAMELEN * _BOARD_NUM_ANIMS];

  assert(sAnimIndex);
  assert(bAnimIndex);
  memset(sAnimIndex, 0, ANIMNAMELEN * _SURFER_NUM_ANIMS);
  memset(bAnimIndex, 0, ANIMNAMELEN * _BOARD_NUM_ANIMS);

  for(int i=0; i<_SURFER_NUM_ANIMS; i++)
  {
    assert((g_surfer_anims[i].length() < ANIMNAMELEN-2) && "Animation name longer than avail buffer. Try increasing 'ANIMNAMELEN'");
    strcpy(&sAnimIndex[i*ANIMNAMELEN], g_surfer_anims[i].c_str());
    sAnimIndex[i*ANIMNAMELEN + ANIMNAMELEN - 2] = '\r';  // So it's more easily viewed in a file
    sAnimIndex[i*ANIMNAMELEN + ANIMNAMELEN - 1] = '\n';  // So it's more easily viewed in a file
  }

  for(int i=0; i<_BOARD_NUM_ANIMS; i++)
  {
    assert((g_board_anims[i].length() < ANIMNAMELEN-2) && "Animation name longer than avail buffer. Try increasing 'ANIMNAMELEN'");
    strcpy(&bAnimIndex[i*ANIMNAMELEN], g_board_anims[i].c_str());
    bAnimIndex[i*ANIMNAMELEN + ANIMNAMELEN - 2] = '\r';  // So it's more easily viewed in a file
    bAnimIndex[i*ANIMNAMELEN + ANIMNAMELEN - 1] = '\n';  // So it's more easily viewed in a file
  }

	replayData.write((void *)&sAnimIndexLen, sizeof(int));
	replayData.write((void *)sAnimIndex,     sAnimIndexLen);
	replayData.write((void *)&bAnimIndexLen, sizeof(int));
	replayData.write((void *)bAnimIndex,     bAnimIndexLen);

  delete [] sAnimIndex;
  delete [] bAnimIndex;

	mem_lock_malloc(memlocked);
#else
  int z = 0;
	replayData.write((void *)&z, sizeof(int));
	replayData.write((void *)&z, sizeof(int));
#endif

  replayData.close();
	if(was_locked) os_file::system_lock();
}

void KSReplay::SaveAnimIndex()
{
}

/*
void KSReplay::PeekFile(char *fname, int *board, int *surfer, int *beach, unsigned int *frames, unsigned int *mainPOFrames, unsigned int *aiPOFrames )
{
  int i;
  os_file replayData;
  char bchName[32];

  replayData.open(fname, os_file::FILE_READ);
  replayData.read(surfer, sizeof(int));
  replayData.read(board, sizeof(int));
  replayData.read(bchName, 32);
  replayData.read(&initWaterState, sizeof(KSWaterState));
	replayData.read(frames,  sizeof(int));
	replayData.read(mainPOFrames,  sizeof(int));
	replayData.read(aiPOFrames,  sizeof(int));

  int numEntries = sizeof(CareerDataArray) / sizeof(CareerDataArray[0]);
  for(i=0; i<numEntries; i++)
  {
    if(0 == strcmp(bchName, CareerDataArray[i].name))
      break;
  }

  if(i == numEntries)
    assert(0 && "Beach file not found");

  *beach = i;

  replayData.close();
}
*/

void KSReplay::Tick(bool running, time_value_t time_inc)
{
  if(frame == NULL)		// True when not recording a replay, such as in multiplayer mode
    return;


  switch(status)
	{
		case REPLAY_IGNORE:
      break;

		case REPLAY_RECORD  :
      if(SUPER_STATE_FLYBY != g_world_ptr->get_ks_controller(g_game_ptr->get_active_player())->get_super_state())  // Don't record during flyby
      {
			  if (running && time_inc>0)
				  SaveFrame();
      }
			break;

		case REPLAY_PLAYBACK:
      RestoreNextFrame(time_inc);
      if(Done())
        status = REPLAY_PAUSED;
			break;

		case REPLAY_PAUSED  :

      // Update camera
      app::inst()->get_game()->get_current_view_camera()->frame_advance (0.0f);
			break;
	}
}

bool KSReplay::IsPlaying()
{
  return status == REPLAY_PLAYBACK;
}

int KSReplay::NumSurfers()
{
  return aiSurfer ? 2 : 1;
}

void KSReplay::SetKSAnimInfo(kellyslater_controller *kscont, float blend, bool loop, float frame)
{
  if ((status == REPLAY_RECORD) && (numFrames < maxframes))
  {
    if(kscont == g_world_ptr->get_ks_controller(0))
      mainEntityState[numFrames].SetKSAnimInfo(blend, loop, frame);
    else if(aiSurfer && kscont == g_world_ptr->get_ks_controller(1))
      aiEntityState[numFrames].SetKSAnimInfo(blend, loop, frame);
  }
}

void KSReplay::SetKSBAnimInfo(kellyslater_controller *kscont, float blend, bool loop, float frame)
{
  if ((status == REPLAY_RECORD) && (numFrames < maxframes))
  {
    if(kscont == g_world_ptr->get_ks_controller(0))
      mainEntityState[numFrames].SetKSBAnimInfo(blend, loop, frame);
    else if(aiSurfer && kscont == g_world_ptr->get_ks_controller(1))
      aiEntityState[numFrames].SetKSBAnimInfo(blend, loop, frame);
  }
}

void KSReplay::SetCollisionInfo(beach_object *obj, entity *ent, const vector3d &dir)
{
  if(collisions == NULL || (unsigned)num_collisions >= MAXCOLLISIONS)
    return;

  collisions[num_collisions].obj = obj;
  collisions[num_collisions].ent = ent;
  collisions[num_collisions].dir = dir;
  collisions[num_collisions].timeStamp = TIMER_GetLevelSec();
  num_collisions++;
}

void KSReplay::SetWipeoutSplash(int player)
{
  if(mainEntityState == NULL)
    return;

  if(player == 0)
    mainEntityState[numFrames].KSWipeoutSplash = true;
}

void KSReplay::SetEndWave()
{
  if(mainEntityState)
    mainEntityState[numFrames].EndWave = true;
}

void KSReplay::ReplayFXUpdate(time_value_t time_inc)
{
  kellyslater_controller *kscont  = g_world_ptr->get_ks_controller(0);
  SurfBoardObjectClass *ksbcont   = &g_world_ptr->get_ks_controller(0)->get_board_controller();
  entity *owner                   = kscont->get_owner();
  entity *my_board                = kscont->GetBoard();

  int superState                  = mainEntityState[playframe].KSSuperState;
  int lastSuperState              = mainEntityState[lastPlayframe].KSSuperState;
  int state                       = mainEntityState[playframe].KSState;
  int lastState                   = mainEntityState[lastPlayframe].KSState;

  // Update board air time (used in certain fx)
  ksbcont->air_timer = (superState == SUPER_STATE_AIR) ? (ksbcont->air_timer + time_inc) : 0;

  // Keep track of whether or not the last just was a lip jump or chop hop
  static bool lipJump = true;
	if (state == STATE_LAUNCH)
		lipJump = true;
	else if (state == STATE_CHOP_HOP)
		lipJump = false;

  g_world_ptr->get_ks_controller(0)->get_board_controller().rb->my_po = g_world_ptr->get_board_ptr(0)->get_rel_po();
  g_world_ptr->get_ks_controller(0)->get_board_controller().rb->my_po.set_position(vector3d(0, 0, 0));
  g_world_ptr->get_ks_controller(0)->get_board_controller().normal = g_world_ptr->get_ks_controller(0)->get_board_controller().rb->my_po.get_y_facing();

  // Check for paddle splash
  if (mainEntityState[playframe].KSState == STATE_PADDLE)
  {
    vector3d pos, n (my_board->get_abs_po ().get_y_facing ());

  static float paddle_offset = -0.5f;

    pos = ((conglomerate*) owner)->get_member("BIP01 L HAND")->get_abs_position ();
    if (dot (n, pos - my_board->get_abs_position ()) > 0)
      kscont->left_hand_dry = true;
    else if (kscont->left_hand_dry)
    {
      vector3d handpos = pos + paddle_offset * owner->get_abs_po().get_x_facing();
      sfx.paddle(false, handpos.x, handpos.y, handpos.z, .5);
      ks_fx_create_paddle_splash (pos + paddle_offset * owner->get_abs_po().get_x_facing());

      kscont->left_hand_dry = false;
    }

    pos = ((conglomerate*) owner)->get_member("BIP01 R HAND")->get_abs_position ();
    if (dot (n, pos - my_board->get_abs_position ()) > 0)
      kscont->right_hand_dry = true;
    else if (kscont->right_hand_dry)
    {
      vector3d handpos = pos + paddle_offset * owner->get_abs_po().get_x_facing();
      sfx.paddle(true, handpos.x, handpos.y, handpos.z, .5);
      ks_fx_create_paddle_splash (pos + paddle_offset * owner->get_abs_po().get_x_facing());
      kscont->right_hand_dry = false;
    }
  }

  // Snap splash
  if(state != lastState)
  {
    if ((state == STATE_LEFTSNAP180) || (state == STATE_RIGHTSNAP180))
      ks_fx_create_snap_splash (((conglomerate*) owner)->get_member("BIP01 R FOOT")->get_abs_position(), my_board->get_abs_po().get_z_facing(), ksbcont->wave_center_hint, state == STATE_LEFTSNAP180, 0);
  }

  // Splash when landing
  if(superState != lastSuperState)
  {
    if(superState != SUPER_STATE_AIR)
    {
      if (lipJump)
        kscont->my_trail->create_big_landing_splash();
      //else
      //  ksctrl->my_trail->create_chophop_splash(collide_center.position);
    }
  }
}

void KSReplay::Record()
{
  status = REPLAY_RECORD;
}

extern void WAVE_Reset();

void KSReplay::Play()
{
	frontendmanager.IGO->OnReplayStart();
  frontendmanager.pms->replay_mode = true;

  g_random_r_ptr->srand(seed);

  firstFrame        = true;

  playframe         = 0;
  lastPlayframe     = 0;
  playtime          = 0.0f;
  lastPlaytime      = 0.0f;

	status            = REPLAY_PLAYBACK;

  slomo             = false;
  fastforward       = false;

  prepareSlomo      = false;
  prepareNormal     = false;

  current_collision = 0;

  WAVE_Reset();
  //init_water_state.Restore();
  if (g_beach_ptr)
    g_beach_ptr->reset ();

  g_world_ptr->get_replay_cam_ptr()->reset();

	g_world_ptr->get_ks_controller(g_game_ptr->get_active_player())->get_board_controller().ResetPhysics();
  replay_camera *replayCam = g_world_ptr->get_replay_cam_ptr();
  app::inst()->get_game()->set_player_camera(app::inst()->get_game()->get_active_player(), (camera*)replayCam);

	if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
	{
    if(dmm.inDemoMode())
		  nslUnpauseAllSounds();
		MusicMan::inst()->unpause();
		//SoundScriptManager::inst()->unpause();
		//if (level_is_loaded())
		//{
		//	sfx.unpause();
		//	wSound.unpause();
		//	VoiceOvers.unpause();
		//}
	}

  // Restore the first frame right away in case replay is paused while rewound
  RestoreNextFrame(0.01f);
}

void KSReplay::Resume()
{
  if(Done())
  {
    Play();
    return;
  }

	frontendmanager.IGO->OnReplayStart();
  frontendmanager.pms->replay_mode = true;

	Pause(false);
  slomo         = false;
  fastforward   = false;
  prepareSlomo      = false;
  prepareNormal     = false;

  g_world_ptr->get_replay_cam_ptr()->reset();

  replay_camera *replayCam = g_world_ptr->get_replay_cam_ptr();
  app::inst()->get_game()->set_player_camera(app::inst()->get_game()->get_active_player(), (camera*)replayCam);

	if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
	{
    if(dmm.inDemoMode())
		  nslUnpauseAllSounds();
		MusicMan::inst()->unpause();
		//SoundScriptManager::inst()->unpause();
		//if (level_is_loaded())
		//{
		//	sfx.unpause();
		//	wSound.unpause();
		//	VoiceOvers.unpause();
		//}
	}

  // Restore the first frame right away in case replay is paused while rewound
  RestoreNextFrame(0.0f);
}

void KSReplay::Restart()
{
  Play();
  firstFrame        = true;
}

void KSReplay::Stop()
{
  g_world_ptr->get_ks_controller(0)->set_state(0);
  Pause(true);
}

void KSReplay::Pause(bool paused)
{
  if(paused)
  {
	  if(status == REPLAY_RECORD)
		  status = REPLAY_IGNORE;
	  else if(status == REPLAY_PLAYBACK)
		  status = REPLAY_PAUSED;
  }
  else
  {
	  if(status == REPLAY_IGNORE)
		  status = REPLAY_RECORD;
	  else if(status == REPLAY_PAUSED)
		  status = REPLAY_PLAYBACK;
  }
}

void KSReplay::SpeedSlow()
{
  slomo = true;
  fastforward = false;
}

void KSReplay::SpeedNormal()
{
  slomo = false;
  fastforward = false;
}

void KSReplay::SpeedFast()
{
  slomo = false;
  fastforward = true;
}

float KSReplay::Playspeed()
{
  if(slomo)
    return 1.0f/slomospeed;
  else if(fastforward)
    return (float)ffspeed;
  else
    return 1.0f;
}

bool KSReplay::Done()
{
  return playframe >= numFrames;
}

bool KSReplay::NoDraw()
{
  return noDraw;
}

int KSReplay::MainPOFrames()
{
  return MAINENTITY_UPDATEFRAMES;
}

int KSReplay::AIPOFrames()
{
  return AIENTITY_UPDATEFRAMES;
}

int KSReplay::SloMoFrames()
{
  return SLOMOFACTOR;
}

int KSReplay::FastForwardFrames()
{
  return FASTFORWARDFACTOR;
}

//void KSReplay::MainEntityPos(int frame, vector3d &pos)
//{
//}

replay_camera::ReplayCamRegion KSReplay::MainEntityRegion(int f)
{
  if(f == -1)
    f = playframe;

  if(mainEntityState[f].KSSuperState == SUPER_STATE_WIPEOUT)
    return replay_camera::RCR_WIPEOUT;
  else if(mainEntityState[f].KSInAir)
    return replay_camera::RCR_AIR;
  else if(mainEntityState[f].KSInTube)
    return replay_camera::RCR_TUBE;
  else
    return replay_camera::RCR_FACE;
}

void KSReplay::SaveFrame()
{
	unsigned int maxmainpoframes = dmm.inDemoMode() ? dmm.replayMainPOFrames : (maxframes/MAINENTITY_UPDATEFRAMES);
	unsigned int maxaipoframes   = dmm.inDemoMode() ? dmm.replayAIPOFrames   : (maxframes/AIENTITY_UPDATEFRAMES);

  if (numFrames>=maxframes || numFrames/MAINENTITY_UPDATEFRAMES>=maxmainpoframes || (aiSurfer && numFrames/AIENTITY_UPDATEFRAMES>=maxaipoframes))
    return;

  if(numFrames == 0)
  {
    mainEntityState[0].Reset();
    if(aiSurfer)
      aiEntityState[0].Reset();

    initWaterState.Save();
  }

  // BIG HACK, FIXME: find a better way
  // Don't save the frame if the current anim is the PLACEHOLDER, which seems to happen
  //  for a single frame at the beginning of a run, or just after a wipeout.
  //if(SURFER_ANIM_PLACEHOLDER == g_world_ptr->get_ks_controller(0)->GetCurrentAnim())
  //  return;

  frame[numFrames].Save();
  mainEntityState[numFrames].Save(0);
  if(numFrames%MAINENTITY_UPDATEFRAMES == 0)
  {
		#ifndef BUILD_FINAL
	  int maxmainpoframes = dmm.inDemoMode() ? dmm.replayMainPOFrames : (maxframes/MAINENTITY_UPDATEFRAMES);
		assert((int) (numFrames/MAINENTITY_UPDATEFRAMES) < maxmainpoframes);
		#endif
    mainEntityPO[numFrames/MAINENTITY_UPDATEFRAMES].Save(0);
    numMainPOFrames = numFrames/MAINENTITY_UPDATEFRAMES + 1;
  }

  if(aiSurfer)
  {
    aiEntityState[numFrames].Save(1);
    if(numFrames%AIENTITY_UPDATEFRAMES == 0)
    {
      aiEntityPO[numFrames/AIENTITY_UPDATEFRAMES].Save(1);
      numAIPOFrames = numFrames/AIENTITY_UPDATEFRAMES + 1;
    }
  }

  numFrames++;

  if (numFrames < maxframes)
  {
	  mainEntityState[numFrames].Reset();

    if (aiSurfer)
	    aiEntityState[numFrames].Reset();
  }
}

extern entity *rain_entity;

void KSReplay::RestoreNextFrame(time_value_t time_inc)
{
  int ticks = (int)(time_inc*60.0f + 0.5f);   // Number of 1/60th of a sec that have passed since last restore

  if(playframe >= numFrames)
    return;


  frameTicks  = (int)((frame[playframe+1].totalTime - frame[playframe].totalTime)*60.0f + 0.5f);
  if(frameTicks == 0)	// This is needed because somehow frames get recorded faster than 60 fps
    frameTicks = 1;		// sometimes at the beginning (perhaps it is rendered but not drawn to screen?)
  interpTicks = slomo ? frameTicks*slomospeed : frameTicks;

  lastPlayframe = playframe;
  if(firstFrame)
  {
    assert(playframe == 0);

    interpFrame = 0;

    // Reset the animation the first frame, so an animation will start over if the replay is reset and the same animation called
    g_world_ptr->get_ks_controller(0)->SetAnimAndFrame(0, 0, 0, 0, 0, false, false, true, true);

    initWaterState.Restore();

    g_world_ptr->get_ks_controller(0)->get_owner()->set_visible(true);          // Needed for demo mode
    g_world_ptr->get_ks_controller(0)->GetBoardModel()->set_visible(true);      // Needed for demo mode

  }
  else
  {
    if(fastforward)
    {
      playframe += ffspeed;
      interpFrame = 0;
    }
    else
    {
      interpFrame += ticks;
      if(interpFrame >= interpTicks)
      {
        if(prepareSlomo)
        {
          slomo = true;
          fastforward = false;
        }
        else if(prepareNormal)
        {
          slomo = false;
          fastforward = false;
        }
        prepareSlomo = prepareNormal = false;

        interpFrame = 0;
        playframe++;
      }
    }
  }

  mainPOFrame = playframe/MAINENTITY_UPDATEFRAMES;
  aiPOFrame   = (aiSurfer) ? playframe/AIENTITY_UPDATEFRAMES : 0;

  if(playframe >= numFrames-1 || mainPOFrame >= numMainPOFrames-1 || (aiSurfer && aiPOFrame >= numAIPOFrames-1))
  {
    playframe = numFrames;
    return;
  }

  float interp = (float)interpFrame/interpTicks;

  float mainPOInterp    = ((float)(playframe%MAINENTITY_UPDATEFRAMES) + interp) / MAINENTITY_UPDATEFRAMES;
  float aiPOInterp      = ((float)(playframe%AIENTITY_UPDATEFRAMES)   + interp) / AIENTITY_UPDATEFRAMES;

  lastPlaytime = playtime;
  playtime = frame[playframe].totalTime + interp*(frame[playframe+1].totalTime - frame[playframe].totalTime);
  float dt = playtime - lastPlaytime;
  if(dt <= 0.0f)
	dt = 0.01f;
  float levelSec = (1.0f-interp)*frame[playframe].levelTime + interp*frame[playframe+1].levelTime;

  TIMER_SetTotalSec(playtime, dt);
  TIMER_SetLevelSec(levelSec);

  // Check for collisions that are supposed to happen
  while((current_collision < num_collisions) && (collisions[current_collision].timeStamp <= levelSec))
  {
    collisions[current_collision].obj->collide(collisions[current_collision].ent, collisions[current_collision].dir);
    current_collision++;
  }

  // So surfer doesn't dissappear under water
  g_world_ptr->get_ks_controller(0)->get_owner()->SetCull(false);
  //g_world_ptr->get_ks_controller(0)->get_owner()->frame_advance(dt);          // Needed to update the lighting when going in or out of water
  //if(aiSurfer)
  //  g_world_ptr->get_ks_controller(1)->get_owner()->frame_advance(dt);

  // Restore state stuff/advance surfer and board anims
  if(interpFrame == 0)  // same as: (interp == 0.0f)
  {
    g_world_ptr->get_ks_controller(0)->set_state(mainEntityState[playframe].KSState);
    g_world_ptr->get_ks_controller(0)->set_super_state(mainEntityState[playframe].KSSuperState);
    g_world_ptr->get_ks_controller(0)->currentTrick = mainEntityState[playframe].KSCurTrick;

    // Hack: Sometimes the placeholder anim gets saved at the beginning of a run, so replace with get on board anim
    if(mainEntityState[playframe].KSAnim == SURFER_ANIM_PLACEHOLDER)
    {
      debug_print("Replacing placeholder anim with get on board anim");
      #ifdef BRONER
        assert(0 && "Replacing placeholder anim with get on board anim");
      #endif
      mainEntityState[playframe].KSAnim = SURFER_ANIM_W_1_R_GOB;
      mainEntityState[playframe].KSBAnim = BOARD_ANIM_W_1_R_GOB;
    }
    g_world_ptr->get_ks_controller(0)->SetAnimAndFrame(mainEntityState[playframe].KSAnim, mainEntityState[playframe].KSBAnim, playtime, mainEntityState[playframe].KSBlend/100.0f, mainEntityState[playframe].KSBBlend/100.0f, mainEntityState[playframe].KSLoop, mainEntityState[playframe].KSBLoop, mainEntityState[playframe].KSAnimCall, mainEntityState[playframe].KSBAnimCall);

    if(firstFrame)
    {
      // If the replay is paused during the first frame, the anim never changes, so advance the anim
      g_world_ptr->get_ks_controller(0)->SetAnimAndFrame(mainEntityState[playframe].KSAnim, mainEntityState[playframe].KSBAnim, playtime);
      if(aiSurfer)
        g_world_ptr->get_ks_controller(1)->SetAnimAndFrame(aiEntityState[playframe].KSAnim, aiEntityState[playframe].KSBAnim, playtime);
    }

    g_world_ptr->get_ks_controller(0)->get_board_controller().wiped_out = mainEntityState[playframe].KSWipedOut;
    g_world_ptr->get_ks_controller(0)->get_board_controller().inAirFlag = mainEntityState[playframe].KSInAir;
    g_world_ptr->get_ks_controller(0)->set_ik_valid(mainEntityState[playframe].KSIKValid?true:false);
    g_world_ptr->get_ks_controller(0)->dry = mainEntityState[playframe].KSDry;

    if(mainEntityState[playframe].KSWipeoutSplash)
      ks_fx_start_wipeout_splash(0);

    if(aiSurfer)
    {
      g_world_ptr->get_ks_controller(1)->set_state(aiEntityState[playframe].KSState);
      g_world_ptr->get_ks_controller(1)->set_super_state(aiEntityState[playframe].KSSuperState);
      g_world_ptr->get_ks_controller(1)->currentTrick = aiEntityState[playframe].KSCurTrick;

      // Hack: Sometimes the placeholder anim gets saved at the beginning of a run, so replace with get on board anim
      if(aiEntityState[playframe].KSAnim == SURFER_ANIM_PLACEHOLDER)
      {
        debug_print("Replacing placeholder anim with get on board anim");
        #ifdef BRONER
          assert(0 && "Replacing placeholder anim with get on board anim");
        #endif
        aiEntityState[playframe].KSAnim = SURFER_ANIM_W_1_R_GOB;
        aiEntityState[playframe].KSBAnim = BOARD_ANIM_W_1_R_GOB;
      }

      g_world_ptr->get_ks_controller(1)->SetAnimAndFrame(aiEntityState[playframe].KSAnim, aiEntityState[playframe].KSBAnim, playtime, aiEntityState[playframe].KSBlend/100.0f, aiEntityState[playframe].KSBBlend/100.0f, aiEntityState[playframe].KSLoop, aiEntityState[playframe].KSBLoop, aiEntityState[playframe].KSAnimCall, aiEntityState[playframe].KSBAnimCall);

      g_world_ptr->get_ks_controller(1)->get_board_controller().wiped_out = aiEntityState[playframe].KSWipedOut;
      g_world_ptr->get_ks_controller(1)->get_board_controller().inAirFlag = aiEntityState[playframe].KSInAir;
      g_world_ptr->get_ks_controller(1)->set_ik_valid(aiEntityState[playframe].KSIKValid?true:false);
      g_world_ptr->get_ks_controller(1)->dry = mainEntityState[playframe].KSDry;
    }
  }
  else
  {
    g_world_ptr->get_ks_controller(0)->SetAnimAndFrame(mainEntityState[playframe].KSAnim, mainEntityState[playframe].KSBAnim, playtime);
    if(aiSurfer)
      g_world_ptr->get_ks_controller(1)->SetAnimAndFrame(aiEntityState[playframe].KSAnim, aiEntityState[playframe].KSBAnim, playtime);
  }

  // Interpolate the position/orientation of the surfer(s) and board(s)
  KSEntityPO mainPOEntInterp, aiPOEntInterp;

  // If next PO frame is the first frame after a wipeout, don't interpolate the position
  if(mainEntityState[playframe].KSSuperState == SUPER_STATE_WIPEOUT && mainEntityState[(mainPOFrame+1)*MAINENTITY_UPDATEFRAMES].KSSuperState != SUPER_STATE_WIPEOUT)
    mainPOEntInterp = mainEntityPO[mainPOFrame];
  else
    interpolate_entity_po(mainEntityPO[mainPOFrame], mainEntityPO[mainPOFrame+1], mainPOEntInterp, mainPOInterp);
  mainPOEntInterp.Restore(0);

  // Calculate and restore the board velocity
  //float lmdt = frame[(mainPOFrame+1)*2].totalTime - frame[mainPOFrame*2].totalTime;
  //vector3d LinMom = vector3d( (mainEntityPO[mainPOFrame+1].KSBPos.x - mainEntityPO[mainPOFrame].KSBPos.x)/lmdt,
  //                            (mainEntityPO[mainPOFrame+1].KSBPos.y - mainEntityPO[mainPOFrame].KSBPos.y)/lmdt, 
  //                            (mainEntityPO[mainPOFrame+1].KSBPos.z - mainEntityPO[mainPOFrame].KSBPos.z)/lmdt );

  //g_world_ptr->get_ks_controller(0)->get_board_controller().rb->linMom = LinMom;

  if(aiSurfer)
  {
    // If next PO frame is the first frame after a wipeout, don't interpolate the position
    if(aiEntityState[playframe].KSSuperState == SUPER_STATE_WIPEOUT && aiEntityState[(aiPOFrame+1)*AIENTITY_UPDATEFRAMES].KSSuperState != SUPER_STATE_WIPEOUT)
      aiPOEntInterp = aiEntityPO[aiPOFrame];
    else
      interpolate_entity_po(aiEntityPO[aiPOFrame], aiEntityPO[aiPOFrame+1], aiPOEntInterp, aiPOInterp);
    aiPOEntInterp.Restore(1);
  }

  // Update beach_objects
	for (beach_object *fobj = g_beach_ptr->my_objects; fobj != NULL; fobj = fobj->next)
	{
		if (!fobj->is_active ())
			continue;

		if (!fobj->is_physical ())
			continue;

    world_dynamics_system* wds = app::inst()->get_game()->get_world();
    entity_anim_tree *anm;
    entity *ent[3];
    ent[0] = fobj->get_entity();
    ent[1] = fobj->is_surfing_object() ? ((surfing_object *)fobj)->my_board_entity : NULL;
    ent[2] = fobj->is_surfing_object() ? ((surfing_object *)fobj)->my_third_entity : NULL;

    for(int e=0; e<3; e++)
    {
      if(ent[e])
      {
        anm = ent[e]->get_anim_tree(ANIM_PRIMARY);

        if (anm)
        {
          if ( wds->eligible_for_frame_advance(anm) )
          {
            if ( anm->is_valid() && anm->is_relative_to_start() )
              anm->reset_root_position();

            anm->frame_advance( CALC_ENTITY_TIME_DILATION(dt, anm->get_entity()) );
          }
          if ( anm->is_autokill() && anm->is_finished() )
            wds->kill_anim( anm );
        }
      }
    }
  }

  // Update the beach
  g_beach_ptr->update(dt);

  //if(rain_entity && g_game_ptr->get_beach_id() == BEACH_JEFFERSONBAY || g_game_ptr->get_beach_id() == BEACH_CORTESBANK)
  //  rain_entity->frame_advance(dt);

  // Update all the active entities
  vector<entity*>::iterator ei, ei_end;
  ei_end = g_world_ptr->active_entities.end();
  for ( ei=g_world_ptr->active_entities.begin(); ei!=ei_end; ++ei )
  {
	  entity* e = (*ei);
	  if ( e )
	  {
		  rational_t e_time_inc = CALC_ENTITY_TIME_DILATION(dt, e);
		  e->frame_advance( e_time_inc );
	  }
  }

  // Update the wave
  WAVE_ReplayTick();
  WAVE_ShiftX = (1.0f-interp)*frame[playframe].wave_shiftx + interp*frame[playframe+1].wave_shiftx;

  for(unsigned p=lastPlayframe+1; p<=playframe; p++)
  {
    if(mainEntityState[p].EndWave)
    {
      // End the wave
      WAVE_EndWave();

      // Make sure the camera hasn't been switched from the replay camera
      replay_camera *replayCam = g_world_ptr->get_replay_cam_ptr();
      app::inst()->get_game()->set_player_camera(app::inst()->get_game()->get_active_player(), (camera*)replayCam);
    }
  }

  // Check for various special effects that need to be started
  ReplayFXUpdate(dt);

  // Update camera
  app::inst()->get_game()->get_current_view_camera()->frame_advance(dt);

  firstFrame = false;
}

