// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#if defined(TARGET_XBOX)
//#include <assert.h>	// May override our own assert.  (dc 06/14/02)
#include <stdlib.h>
#include "conglom.h"
#include "inputmgr.h"
#include "kellyslater_controller.h"
#include "wds.h"
#include "random.h"
#include "ksnsl.h"
#endif /* TARGET_XBOX JIV DEBUG */

#include "MusicMan.h"
#include "VOEngine.h"
#include "singleton.h"

DEFINE_SINGLETON(MusicMan)

Track::Track()
{
  paused = false;
}
Track::~Track()
{
}

nslSoundId Track::Play()
{
  if (!IsPlaying() && !paused)
  {
    mySndId = nslAddSound(mySrcId);
    if (mySndId != NSL_INVALID_ID)
      nslPlaySound(mySndId);

  }
  return mySndId;
}

bool Track::IsPlaying()
{
  return nslGetSoundStatus(mySndId) != NSL_SOUNDSTATUS_INVALID;
}
void Track::Stop()
{
  nslStopSound(mySndId);
  mySndId = NSL_INVALID_ID;
}
void Track::SetInfo(const char *name, const char *artist, nslSourceId src)
{
  paused = false;
  strcpy(songName, name);
  strcpy(artistName, artist);
  mySrcId = src;
  mySndId = NSL_INVALID_ID;

}


void Track::Pause()
{
  if ((!paused) && (nslGetSoundStatus(mySndId) != NSL_SOUNDSTATUS_INVALID))
    nslPauseSound(mySndId);
  paused =true;
}

void Track::Unpause()
{
  if ((paused)&& (nslGetSoundStatus(mySndId) != NSL_SOUNDSTATUS_INVALID))
    nslUnpauseSound(mySndId);
  paused =false;
}


MusicMan::MusicMan()
{
  inited = 0;
}

MusicMan::~MusicMan()
{

}

#if !defined(EVAN) && !defined(TARGET_XBOX)
#define AudioWarning(s) nslPrintf(s)
#else
#define AudioWarning(s) ((void)0)
#endif

MusicListing::MusicListing()
{
  for (int i=0; i < MAX_SONGS; i++)
  {
    order[i] = i;
  }
  totalSources = 0;
  current = 0;
  sucessfulLastPlay = false;
}

// This class plays a random source
MusicListing::~MusicListing()
{

}
void MusicListing::clearSources()
{
  for (int i=0; i < MAX_SONGS; i++)
    sources[i].SetInfo("", "", NSL_INVALID_ID);
  totalSources = 0;
}


void MusicListing::addSource( const char *artist, const char *name, nslSourceId s )
{
  sources[totalSources].SetInfo(name, artist, s);
  totalSources++;
}

bool MusicListing::isPlaying()
{
  if ((totalSources > 0) && (current < totalSources))
  {
    return sources[order[current]].IsPlaying();
  }
  return false;
}
void MusicListing::stop()
{
  sources[order[current]].Stop();
}
nslSoundId MusicListing::play()
{
  if (totalSources > 0)
  {
    currentSoundId = sources[order[current]].Play();
    sucessfulLastPlay = (currentSoundId != NSL_INVALID_ID);
    return currentSoundId;
  }
  else
  {
    currentSoundId = NSL_INVALID_ID;
    return NSL_INVALID_ID;
  }


}

nslSoundId MusicListing::playNext()
{
  int counter = 0;
  if (sucessfulLastPlay)
    current++;
  if (current >= totalSources)
    current = 0;
  while ((counter < MAX_SONGS) && (!enabled[order[current]]))
  {
    counter++;
    current++;
    if (current >= totalSources)
      current = 0;
  }
  if (!enabled[order[current]])
    return NSL_INVALID_ID;

  if (totalSources > 0)
  {
    currentSoundId = sources[order[current]].Play();
    sucessfulLastPlay = (currentSoundId != NSL_INVALID_ID);
    return currentSoundId;
  }
  else
  {
    currentSoundId = NSL_INVALID_ID;
    return NSL_INVALID_ID;
  }

}

void MusicListing::shutdown()
{
  for (int i=0; i < totalSources; i++)
  {
    sources[i].SetInfo("", "", NSL_INVALID_ID);
  }

  totalSources = 0;

}

void MusicListing::randomize()
{
  int currentSize = totalSources;
  int theList[MAX_SONGS];
  for (int i=0; i < totalSources; i++)
  {
    theList[i] = i;
  }

  for (int i=0; i < totalSources; i++)
  {
    int val = random(currentSize);
    order[i] = theList[val];
    if (val != currentSize)
      theList[val] = theList[currentSize - 1];
    theList[currentSize - 1] = -1;
    currentSize--;
  }

}
void MusicListing::swap(int a, int b)
{
  int temp = order[a];
  if (a == current)
    current = b;
  else if (b == current)
    current = a;
  order[a] = order[b];
  order[b] = temp;
}

void MusicListing::alphabetize()
{
  bool swapped= true;
  while (swapped)
  {
    swapped = false;
    for (int i=0; i < totalSources - 1; i++)
    {
      if (stricmp(sources[order[i]].songName, sources[order[i+1]].songName) > 0)
      {
        swapped = true;
        swap(i, i+1);
      }
    }
  }
}
nslSoundId MusicMan::playNext()
{
  if (!isPlaying() && !paused)
  {
    return musicTrack.playNext();
  };
  return NSL_INVALID_ID;
}

nslSoundId MusicMan::play()
{
  if (!isPlaying() && !paused)
  {
    return musicTrack.play();
  };
  return NSL_INVALID_ID;
}

void MusicMan::loadSources()
{

  paused = false;
  musicTrack.clearSources();


	// Load all the sources
	nslSourceId s = nslLoadSource("CHRISTOPHEGOZE");
  if (s != NSL_INVALID_ID)
    musicTrack.addSource("CHRISTOPHE GOZE","SIROCCO", s);
  else AudioWarning("FIXME: MISSING MUSIC SOUND\n");
	
  s = nslLoadSource("GREYBOY");
  if (s != NSL_INVALID_ID)
    musicTrack.addSource("GREY BOY","MASTERED THE ART", s);
  else AudioWarning("FIXME: MISSING MUSIC SOUND\n");

  // Jack johnson - middle man from Loose Change
  s = nslLoadSource("JACKJOHNSON");
  if (s != NSL_INVALID_ID)
    musicTrack.addSource("JACK JOHNSON","MIDDLE MAN", s);
  else AudioWarning("FIXME: MISSING MUSIC SOUND\n");

  s = nslLoadSource("NOTABOSSA");
  if (s != NSL_INVALID_ID)
    musicTrack.addSource("FUNKY LOWLIVES","NOTABOSSA", s);
  else AudioWarning("FIXME: MISSING MUSIC SOUND\n");

  s = nslLoadSource("SISE");
  if (s != NSL_INVALID_ID)
    musicTrack.addSource("SI SE","STEPPIN' OUT", s);
  else AudioWarning("FIXME: MISSING MUSIC SOUND\n");

  s = nslLoadSource("THUNDERBALL1");
  if (s != NSL_INVALID_ID)
    musicTrack.addSource("THUNDERBALL","HEART OF THE HUSTLER", s);
  else AudioWarning("FIXME: MISSING MUSIC SOUND\n");

  s = nslLoadSource("THUNDERBALL2");
  if (s != NSL_INVALID_ID)
    musicTrack.addSource("THUNDERBALL","VAI VAI", s);
  else AudioWarning("FIXME: MISSING MUSIC SOUND\n");

  s = nslLoadSource("TRUBYTRIO");
  if (s != NSL_INVALID_ID)
    musicTrack.addSource("TRUBY TRIO","ALEGRE", s);
  else AudioWarning("FIXME: MISSING MUSIC SOUND\n");

  // MO HORIZONS FOTO VIVA (NICOLA CONTE REMIX)
  s = nslLoadSource("VIVANICOLACON");
  if (s != NSL_INVALID_ID)
    musicTrack.addSource("MO HORIZONS","FOTO VIVA", s);
  else AudioWarning("FIXME: MISSING MUSIC SOUND\n");


  //------------------------------------------------
  // None of these sounds are actually in game yet.  
  // We will run out of room on the CD if we add them
  //   -KES 1/25/01

  
  s = nslLoadSource("SHURIKAN");
  if (s != NSL_INVALID_ID)
    musicTrack.addSource("SHUR I KAN","ADVANCE", s);

  s = nslLoadSource("NOJUST");
  if (s != NSL_INVALID_ID)
    musicTrack.addSource("SMITH AND MIGHTY","NO JUSTICE", s);

  s = nslLoadSource("SOULHOOL");
  if (s != NSL_INVALID_ID)
    musicTrack.addSource("SOUL HOOLIGAN","PSYCHADELIC SOUL", s);

  s = nslLoadSource("SPACE");
  if (s != NSL_INVALID_ID)
    musicTrack.addSource("THE SPACE COSSACKS","SOLARIS STOMP", s);

  s = nslLoadSource("THRDWRLD");
  if (s != NSL_INVALID_ID)
    musicTrack.addSource("THIRD WORLD","1865 (96 IN THE SHADE)", s);

  s = nslLoadSource("FADED");
  if (s != NSL_INVALID_ID)
    musicTrack.addSource("BEN HARPER","FADED", s);

  s = nslLoadSource("KAY");
  if (s != NSL_INVALID_ID)
    musicTrack.addSource("JEREMY KAY","BACK TO YOU", s);


  s = nslLoadSource("PERRY");
  if (s != NSL_INVALID_ID)
    musicTrack.addSource("PERRY FARRELL","NUA NUA", s);


  s = nslLoadSource("RANDC");
  if (s != NSL_INVALID_ID)
    musicTrack.addSource("RAE AND CHRISTIAN","READY TO ROLL", s);

  s = nslLoadSource("PILGRIMS");
  if (s != NSL_INVALID_ID)
    musicTrack.addSource("SATAN'S PILGRIMS","STEP IT UP", s);


  s = nslLoadSource("QUANTUM");
  if (s != NSL_INVALID_ID)
    musicTrack.addSource("QUANTUM DUB FORCE","QUANTUM ZONE", s);

	s = nslLoadSource("PEARL");
  if (s != NSL_INVALID_ID)
    musicTrack.addSource("PEARL JAM","WMA", s);
	
	s = nslLoadSource("GLOVE");
  if (s != NSL_INVALID_ID)
    musicTrack.addSource("G LOVE AND SPECIAL SAUCE","UNIFIED", s);
	
	s = nslLoadSource("PLAYON");
  if (s != NSL_INVALID_ID)
    musicTrack.addSource("RAE AND CHRISTIAN WITH JUNGLE BROS","PLAY ON", s);



  // We try to keep the current one persistent.
  if (musicTrack.getCurrent() > musicTrack.getNumSongs())
    musicTrack.setCurrent(0);
}

void MusicMan::init()
{
  if (inited)
    shutdown();
  for (int i=0; i < MAX_SONGS; i++)
  {
    musicTrack.enabled[i] = true;
    musicTrack.order[i] = i;
  }


  volume = MUSIC_VOLUME_ADJUSTMENT_FACTOR;
  inited = 1;
  paused =false;
}


void MusicMan::tick(bool inGame)
{ 
  if (!inited) return;
  if (paused) return;
  if (inGame && (g_game_ptr->get_num_active_players() == 1))
  {
    // This 
    int currentRegion = g_world_ptr->get_ks_controller(g_game_ptr->get_active_player())->get_board_controller().GetRegion();	
    po p = g_world_ptr->get_hero_ptr(g_game_ptr->get_active_player())->get_abs_po();	
    vector3d crashSpot, pos = p.get_position();

    crashSpot.x  = WAVE_SoundEmitter[WAVE_SE_TUBE].line.stop[0];
    crashSpot.y  = WAVE_SoundEmitter[WAVE_SE_TUBE].line.stop[1];
    crashSpot.z  = WAVE_SoundEmitter[WAVE_SE_TUBE].line.stop[2];

    // First we find out if we are in the tube..
    if (currentRegion == WAVE_REGIONTUBE)
    {
      volume = .3;
    }
    else
    {
      float xdist = crashSpot.x - pos.x;
      float ydist = crashSpot.y - pos.y;
      float zdist = crashSpot.z - pos.z;
      float dist = sqrtf(xdist*xdist + ydist*ydist + zdist*zdist);

      if ((dist < MAX_TUBE_MUSIC_DIST) && (dist > 0))
      {
        volume = (MUSIC_VOLUME_ADJUSTMENT_FACTOR -.2f) * (dist)/(MAX_TUBE_MUSIC_DIST) + .2f;
      }
      else
      {
        volume = MUSIC_VOLUME_ADJUSTMENT_FACTOR;
      }
    }
  }

  if (!musicTrack.isPlaying() && !paused)
  {
    musicTrack.playNext();
  }
  else
  {
    if ((inGame) && (!paused) && (nslGetSoundStatus(musicTrack.getSoundId()) != NSL_SOUNDSTATUS_INVALID))
      nslSetSoundParam(musicTrack.getSoundId(), NSL_SOUNDPARAM_VOLUME, volume);
  }
}

void MusicMan::stop()
{
  if (musicTrack.isPlaying())
    musicTrack.stop();
}
void MusicMan::shutdown()
{
  stop();
  musicTrack.shutdown();

}
