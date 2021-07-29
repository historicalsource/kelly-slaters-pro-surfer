/* Greg Taylor's new SUPER TESTER SHELL
 * written in a blind fury 4 days before Spider-Man alpha, based on
 * Kevin Schmidt's original tester.
 *
 * this one takes in a text file format to assign the various possible tests to
 * controller buttons.  Most of this framework is just stubbed out and it has
 * not yet been made fully cross-platform.  Eventually, when someone has the 
 * time, they might want to finish the job.  Ask me for details.
 *
 * The goal of this app is to eventually become something like NSL sound tool,
 * but be a test-bed application that is platform independent, which can be
 * mapped in many interesting ways and do lots of tests for you -- buildable
 * on any platform.  It's far from that right now tho =).
 *
 * Here's an example 'mapping file'

[BUTTON_MAPPING]
(1) PLAY_PAUSE 1 osc_e_out
(2) CYCLE_SOURCE 2 PUNCHMED
(3) PLAY 2 osc_e_out
(4) PAUSE_UNPAUSE 2
(5) STOP 2
(6) VOLUME_UP 2
(7) VOLUME_DOWN 2
(8) NONE
(9) NONE
(10) NONE
(11) NONE
(12) NONE
(S) NONE

[END]

 * Some good arguments to send it are :

 -m host0:SoundTest.txt -r cdrom0:\PS2SOUND\ -l levels\origin_a

 * Where soundtest.txt is the above mapping file.  
 * Currently I have only implemented the fire-and-forget play button mapping, but
 * I've put in the scaffolding to do much more interesting tests.  But I got what I needed
 * from it, and I want to go home to sleep now.  --GT
 */
#include "../common/nsl.h"
#include "nsl_testapp_sys.h"
#include "libcdvd.h"
#include "math.h"
#include "../common/nl.h"
#include "nsl_ps2.h"
#include "nsl_testapp.h"
#include <vector>
#include <algorithm>
#include <sifdev.h>
#include <ctype.h>
#include <assert.h>
using namespace std;


fifo_queue<int> controllerCommands;

#define MAX_STR_LEN   256
#define MAX_TEST_SNDS  10


struct cstring
{
  char cstr[MAX_STR_LEN];

  cstring()
  {
    clear();
  }
  cstring( const cstring &b )
  {
    copy( b );
  }
  cstring( const char *b )
  {
    copy( b );
  }
  void clear()
  {
    cstr[0] = '\0';
  }
  void copy( const char *b )
  {
    strncpy( cstr, b, MAX_STR_LEN );
    cstr[MAX_STR_LEN-1] = '\0';
  }
  void copy( const cstring &b )
  {
    strcpy( cstr, b.cstr );
  }
  cstring& operator = (const char *str) 
  {
    assert(str != NULL);

    copy( str );
    return *this;
  }
  cstring& operator = (const cstring &str) 
  {
    if (this != &str)
      copy( str );
    return *this;
  }
  bool operator == (const cstring &b) const
  {
    return (strcmp( cstr, b.cstr ) == 0);
  }
  bool operator == (char *b) const
  {
    return (strcmp( cstr, b ) == 0);
  }
  bool operator != (const cstring &b) const
  {
    return (strcmp( cstr, b.cstr ) != 0);
  }
  bool operator != (char *b) const
  {
    return (strcmp( cstr, b ) != 0);
  }
};

void tokenize( char *buf, char *delimeters, vector<cstring> &tokens )
{
  int i, j, k;
  int delim_count;
  cstring token;

  i = 0;
  j = 0;
  delim_count = strlen(delimeters);

  while ( buf[i] != '\0' )
  {
    for (k=0; k<delim_count; ++k)
    {
      if (buf[i] == delimeters[k])
        break;
    }
    if ( k == delim_count )
    {
      if (buf[i] == '/')
        token.cstr[j] = '\\'; // standardize slashes
      else 
        token.cstr[j] = toupper(buf[i]); // force to uppercase

      ++j;
    }
    else
    {
      token.cstr[j] = '\0';
      if (strlen(token.cstr) > 0)
        tokens.push_back( token );
      j = 0;
    }
    ++i;
  }
  if (j > 0)
  {
    token.cstr[j] = '\0';
    if (strlen(token.cstr) > 0)
      tokens.push_back( token );
  }
}

struct nslTestSound
{
  nslSoundId  nslId;
  int         sourceIndex;
  typedef enum {
    STATE_NO_ID,
    STATE_PLAYING,
    STATE_PAUSED
  } nslTestSoundStateEnum;
  nslTestSoundStateEnum state;
  nslTestSound()
  {
    clear();
  }
  void clear()
  {
    nslId = NSL_INVALID_ID;
    sourceIndex = 0;
    state = STATE_NO_ID;
  }
};

struct nslTestButton
{
  typedef enum
  {
    CMD_NONE,
    CMD_PLAY_PAUSE,
    CMD_CYCLE_SOURCE, // implemented
    CMD_PLAY,         // implemented
    CMD_PLAY_AND_CYCLE, // implemented
    CMD_PLAY_STOP,    // implemented
    CMD_PAUSE,
    CMD_UNPAUSE,
    CMD_PAUSE_UNPAUSE,
    CMD_PAUSE_ALL,
    CMD_UNPAUSE_ALL,
    CMD_PAUSE_UNPAUSE_ALL,
    CMD_STOP,         // implemented
    CMD_STOP_ALL,
    CMD_VOLUME_UP,
    CMD_VOLUME_DOWN,
    CMD_MASTER_VOLUME_UP,
    CMD_MASTER_VOLUME_DOWN,
    CMD_RESET
  } nslTestButtonCommand;

  nslTestButtonCommand cmd;
  int         testSnd;
  nslSoundId  lastIdPlayed;

  void critical_process() { /* anything about this button that needs to be processed every tick goes here */ }
  void process();
  void processPlay();
  void processCycleSource();
  void processPlayStop();
  void processStop();
  void clear() { testSnd = 0; cmd = CMD_NONE; }
};

struct nslTestAppOptions
{
  cstring mappingFile;
  cstring soundRoot;
  cstring levelName;
  nslTestButton button[NSL_TESTBUTTON_NUM];
  nslTestSound  testSound[MAX_TEST_SNDS];
  vector<cstring> soundSources;

  int parseArgs( int argc, char* argv[] );
  bool setup();
  void parseButtonMappings( vector<cstring>::iterator &it, vector<cstring>::iterator &it_end );
  void parseButtonMapping( nslTestButtonEnum whichButton, vector<cstring>::iterator &it, vector<cstring>::iterator &it_end );
  void parseSndFile( vector<cstring> &lines );
  void setSoundSource( nslTestButtonEnum whichButton, vector<cstring>::iterator &it );
  void setTestSound( nslTestButtonEnum whichButton, vector<cstring>::iterator &it );

  nslTestAppOptions()
  {
    soundRoot = "PS2SOUND\\";
  }

};

nslTestAppOptions opt;

// Used for circling a 3d sound around..

void setEmitterFromDegrees(float degrees, float distance, nslEmitterId e) 
{
  float radians = M_PI*degrees/180.0;
  nlVector3d v;

  v[1] = 0;
  v[0] = distance*sin(radians);
  v[2] = distance*cos(radians);
  nslSetEmitterPosition(e, v);
  
}

void nslTestButton::process() 
{ 
// This is a quick test for the reverb functions.
	int whichSource = opt.testSound[testSnd].sourceIndex;

#ifdef NSL_LOAD_SOURCE_BY_NAME
	nslSourceId src = nslGetSource( opt.soundSources[whichSource].cstr, false );
#endif
#ifdef NSL_LOAD_SOURCE_BY_ALIAS
	nslSourceId src = nslGetSource( atoi(opt.soundSources[whichSource].cstr), false );
#endif

	bool reverb_on = nslGetReverb(src);
	printf("The reverb setting was %d\n", reverb_on);
	nslSetReverb(src, !reverb_on);
	printf("It should have been toggled\n");
	reverb_on = nslGetReverb(src);
	printf("The reverb setting became %d\n", reverb_on);


  switch (cmd)
  {
    case CMD_PLAY:
      processPlay();
      break;
    case CMD_CYCLE_SOURCE:
      processCycleSource();
      break;
    case CMD_PLAY_AND_CYCLE:
      processPlay();
      processCycleSource();
      break;
    case CMD_PLAY_STOP:
      processPlayStop();
      break;
    case CMD_STOP:
      processStop();
      break;
    case CMD_NONE:
      printf("This button is unmapped\n"); 
      break;
    default:
      printf("Ouch %d %d\n", cmd, testSnd); 
      break;
  }
}

void nslTestButton::processPlayStop() 
{ 
  int whichSource = opt.testSound[testSnd].sourceIndex;
  assert(whichSource >= 0 && whichSource < opt.soundSources.size());
  switch (opt.testSound[testSnd].state)
  {
    case nslTestSound::STATE_NO_ID:
      {
#ifdef NSL_LOAD_SOURCE_BY_NAME
        nslSourceId src = nslGetSource( opt.soundSources[whichSource].cstr, false );
#endif
#ifdef NSL_LOAD_SOURCE_BY_ALIAS
        nslSourceId src = nslGetSource( atoi(opt.soundSources[whichSource].cstr), false );
#endif

        printf("PLAY_STOP: Playing (%d) sound %s\n", testSnd, opt.soundSources[whichSource].cstr);
        if ( src == NSL_INVALID_ID )
        {
          printf("Invalid source name %s\n", opt.soundSources[whichSource].cstr);
        }
        else
        {
          lastIdPlayed = nslAddSound( src );
          if ( lastIdPlayed == NSL_INVALID_ID )
          {
            printf("Could not acquire resources to play sound %s\n", opt.soundSources[whichSource].cstr);
          }
          else
          {
            nslPlaySound( lastIdPlayed );
            opt.testSound[testSnd].state = nslTestSound::STATE_PLAYING;
          }
        }
      }
      break;
    case nslTestSound::STATE_PLAYING:
    case nslTestSound::STATE_PAUSED:
      {
        printf("PLAY_STOP: Stopping (%d) sound %s\n", testSnd, opt.soundSources[whichSource].cstr);
        // stop previous
        if (nslGetSoundStatus( lastIdPlayed ) == NSL_INVALID_ID)
        {
          opt.testSound[testSnd].state = nslTestSound::STATE_NO_ID;
          processPlayStop();
        }
        else
        {
          nslStopSound( lastIdPlayed );
          opt.testSound[testSnd].state = nslTestSound::STATE_NO_ID;
        }
      }
      break;
  }
}

void nslTestButton::processStop() 
{ 
  // stop previous
  int whichSource = opt.testSound[testSnd].sourceIndex;
  assert(whichSource >= 0 && whichSource < opt.soundSources.size());
  printf("STOP: Stopping (%d) sound %s\n", testSnd, opt.soundSources[whichSource].cstr);
  if (nslGetSoundStatus( lastIdPlayed ) == NSL_INVALID_ID)
  {
    opt.testSound[testSnd].state = nslTestSound::STATE_NO_ID;
    processPlayStop();
  }
  else
  {
    nslStopSound( lastIdPlayed );
    opt.testSound[testSnd].state = nslTestSound::STATE_NO_ID;
  }
}


void nslTestButton::processPlay() 
{ 
  // play
  int whichSource = opt.testSound[testSnd].sourceIndex;
  assert(whichSource >= 0 && whichSource < opt.soundSources.size());
#ifdef NSL_LOAD_SOURCE_BY_NAME
  nslSourceId src = nslGetSource( opt.soundSources[whichSource].cstr, false );
#endif
#ifdef NSL_LOAD_SOURCE_BY_ALIAS
  nslSourceId src = nslGetSource( atoi(opt.soundSources[whichSource].cstr), false );
#endif
  printf("PLAY: Playing (%d) sound %s\n", testSnd, opt.soundSources[whichSource].cstr);
  if ( src == NSL_INVALID_ID )
  {
    printf("Invalid source name %s\n", opt.soundSources[whichSource].cstr);
  }
  else
  {
    lastIdPlayed = nslAddSound( src );
    if ( lastIdPlayed == NSL_INVALID_ID )
    {
      printf("Could not acquire resources to play sound %s\n", opt.soundSources[whichSource].cstr);
    }
    else
    {
      nslPlaySound( lastIdPlayed );
      opt.testSound[testSnd].state = nslTestSound::STATE_PLAYING;
    }
  }

}

void nslTestButton::processCycleSource() 
{ 
  // cycle
  opt.testSound[testSnd].sourceIndex++;
  if (opt.testSound[testSnd].sourceIndex >= opt.soundSources.size())
  {
    opt.testSound[testSnd].sourceIndex = 0;
  }
  printf("CYCLE: Cycling (%d) to source %s\n", testSnd, opt.soundSources[opt.testSound[testSnd].sourceIndex].cstr);
}


int nslTestAppOptions::parseArgs( int argc, char* argv[] )
{
  int i = 1;
  int inOptions = 1;

  while( ( i < argc ) && inOptions ) 
  {
    if( argv[i][0] != '-' ) 
    {
      inOptions = 0;
    } 
    else 
    {
      switch( argv[i][1] ) 
      {
        case 'M':
        case 'm':
          ++i;
          mappingFile = argv[i];
          ++i;
          break;

        case 'R':
        case 'r':
          ++i;
          soundRoot = argv[i];
          ++i;
          break;

        case 'L':
        case 'l':
          ++i;
          levelName = argv[i];
          ++i;
          break;

        default:
          printf( "parseArgs: unrecognized option \'%s\'\n", argv[i] );
          ++i;
          break;
      }
    }
  }

	return i;
}

#define ADVANCE_TOKEN { ++it; assert(it != it_end); }

void nslTestAppOptions::setSoundSource( nslTestButtonEnum whichButton, vector<cstring>::iterator &it )
{
  vector<cstring>::iterator source_it = find(opt.soundSources.begin(), opt.soundSources.end(), (*it));
  if ( source_it != opt.soundSources.end())
  {
    int idx = source_it - opt.soundSources.begin();
    testSound[button[whichButton].testSnd].sourceIndex = idx;
    assert(opt.soundSources[idx] == (*it));
  }
}

void nslTestAppOptions::setTestSound( nslTestButtonEnum whichButton, vector<cstring>::iterator &it )
{
  button[whichButton].testSnd = atoi((*it).cstr);
  assert(button[whichButton].testSnd >= 0 && button[whichButton].testSnd < MAX_TEST_SNDS);
}

void nslTestAppOptions::parseButtonMapping( nslTestButtonEnum whichButton, vector<cstring>::iterator &it, vector<cstring>::iterator &it_end )
{
  ADVANCE_TOKEN;
  if ( (*it) == "PLAY" )
  {
    button[whichButton].clear();
    button[whichButton].cmd = nslTestButton::CMD_PLAY;
    ADVANCE_TOKEN;
    assert(button[whichButton].testSnd >= 0 && button[whichButton].testSnd < MAX_TEST_SNDS);
    if ((*it).cstr[0] == '[' || (*it).cstr[0] == '(')
      return;

    // optional testSnd assignment
    setTestSound( whichButton, it );
    ADVANCE_TOKEN;
    if ((*it).cstr[0] == '[' || (*it).cstr[0] == '(')
      return;

    // optional sound source 
    setSoundSource( whichButton, it );
  }
  else if ( (*it) == "PLAY_STOP" )
  {
    button[whichButton].clear();
    button[whichButton].cmd = nslTestButton::CMD_PLAY_STOP;
    ADVANCE_TOKEN;
    assert(button[whichButton].testSnd >= 0 && button[whichButton].testSnd < MAX_TEST_SNDS);
    if ((*it).cstr[0] == '[' || (*it).cstr[0] == '(')
      return;

    // optional testSnd assignment
    setTestSound( whichButton, it );
    ADVANCE_TOKEN;
    if ((*it).cstr[0] == '[' || (*it).cstr[0] == '(')
      return;

    // optional sound source 
    setSoundSource( whichButton, it );
  }
  else if ( (*it) == "PLAY_PAUSE" )
  {
    printf("Unimplemented button assignment %s\n", (*it).cstr);
    button[whichButton].clear();
    while ((*it).cstr[0] != '[' && (*it).cstr[0] != '(')
    {
      ADVANCE_TOKEN;
    }
    return;
  }
  else if ( (*it) == "PLAY_PAUSE" )
  {
    printf("Unimplemented button assignment %s\n", (*it).cstr);
    button[whichButton].clear();
    while ((*it).cstr[0] != '[' && (*it).cstr[0] != '(')
    {
      ADVANCE_TOKEN;
    }
    return;
  }
  else if ( (*it) == "CYCLE_SOURCE" )
  {
    button[whichButton].clear();
    button[whichButton].cmd = nslTestButton::CMD_CYCLE_SOURCE;
    ADVANCE_TOKEN;
    assert(button[whichButton].testSnd >= 0 && button[whichButton].testSnd < MAX_TEST_SNDS);
    if ((*it).cstr[0] == '[' || (*it).cstr[0] == '(')
      return;

    // optional testSnd assignment
    setTestSound( whichButton, it );
    ADVANCE_TOKEN;
    if ((*it).cstr[0] == '[' || (*it).cstr[0] == '(')
      return;

    // optional sound source 
    setSoundSource( whichButton, it );
  }
  else if ( (*it) == "PLAY_AND_CYCLE" )
  {
    button[whichButton].clear();
    button[whichButton].cmd = nslTestButton::CMD_PLAY_AND_CYCLE;
    ADVANCE_TOKEN;
    assert(button[whichButton].testSnd >= 0 && button[whichButton].testSnd < MAX_TEST_SNDS);
    if ((*it).cstr[0] == '[' || (*it).cstr[0] == '(')
      return;

    // optional testSnd assignment
    setTestSound( whichButton, it );
    ADVANCE_TOKEN;
    if ((*it).cstr[0] == '[' || (*it).cstr[0] == '(')
      return;

    // optional sound source 
    setSoundSource( whichButton, it );
  }
  else if ( (*it) == "PAUSE" )
  {
    printf("Unimplemented button assignment %s\n", (*it).cstr);
    button[whichButton].clear();
    while ((*it).cstr[0] != '[' && (*it).cstr[0] != '(')
    {
      ADVANCE_TOKEN;
    }
    return;
  }
  else if ( (*it) == "UNPAUSE" )
  {
    printf("Unimplemented button assignment %s\n", (*it).cstr);
    button[whichButton].clear();
    while ((*it).cstr[0] != '[' && (*it).cstr[0] != '(')
    {
      ADVANCE_TOKEN;
    }
    return;
  }
  else if ( (*it) == "PAUSE_UNPAUSE" )
  {
    printf("Unimplemented button assignment %s\n", (*it).cstr);
    button[whichButton].clear();
    while ((*it).cstr[0] != '[' && (*it).cstr[0] != '(')
    {
      ADVANCE_TOKEN;
    }
    return;
  }
  else if ( (*it) == "PAUSE_ALL" )
  {
    printf("Unimplemented button assignment %s\n", (*it).cstr);
    button[whichButton].clear();
    while ((*it).cstr[0] != '[' && (*it).cstr[0] != '(')
    {
      ADVANCE_TOKEN;
    }
    return;
  }
  else if ( (*it) == "UNPAUSE_ALL" )
  {
    printf("Unimplemented button assignment %s\n", (*it).cstr);
    button[whichButton].clear();
    while ((*it).cstr[0] != '[' && (*it).cstr[0] != '(')
    {
      ADVANCE_TOKEN;
    }
    return;
  }
  else if ( (*it) == "PAUSE_UNPAUSE_ALL" )
  {
    printf("Unimplemented button assignment %s\n", (*it).cstr);
    button[whichButton].clear();
    while ((*it).cstr[0] != '[' && (*it).cstr[0] != '(')
    {
      ADVANCE_TOKEN;
    }
    return;
  }
  else if ( (*it) == "STOP" )
  {
    button[whichButton].clear();
    button[whichButton].cmd = nslTestButton::CMD_STOP;
    ADVANCE_TOKEN;
    assert(button[whichButton].testSnd >= 0 && button[whichButton].testSnd < MAX_TEST_SNDS);

    // required test sound assignment
    setTestSound( whichButton, it );
  }
  else if ( (*it) == "STOP_ALL" )
  {
    printf("Unimplemented button assignment %s\n", (*it).cstr);
    button[whichButton].clear();
    while ((*it).cstr[0] != '[' && (*it).cstr[0] != '(')
    {
      ADVANCE_TOKEN;
    }
    return;
  }
  else if ( (*it) == "VOLUME_UP" )
  {
    printf("Unimplemented button assignment %s\n", (*it).cstr);
    button[whichButton].clear();
    while ((*it).cstr[0] != '[' && (*it).cstr[0] != '(')
    {
      ADVANCE_TOKEN;
    }
    return;
  }
  else if ( (*it) == "VOLUME_DOWN" )
  {
    printf("Unimplemented button assignment %s\n", (*it).cstr);
    button[whichButton].clear();
    while ((*it).cstr[0] != '[' && (*it).cstr[0] != '(')
    {
      ADVANCE_TOKEN;
    }
    return;
  }
  else if ( (*it) == "MASTER_VOLUME_UP" )
  {
    printf("Unimplemented button assignment %s\n", (*it).cstr);
    button[whichButton].clear();
    while ((*it).cstr[0] != '[' && (*it).cstr[0] != '(')
    {
      ADVANCE_TOKEN;
    }
    return;
  }
  else if ( (*it) == "MASTER_VOLUME_DOWN" )
  {
    printf("Unimplemented button assignment %s\n", (*it).cstr);
    button[whichButton].clear();
    while ((*it).cstr[0] != '[' && (*it).cstr[0] != '(')
    {
      ADVANCE_TOKEN;
    }
    return;
  }
  else if ( (*it) == "RESET" )
  {
    printf("Unimplemented button assignment %s\n", (*it).cstr);
    button[whichButton].clear();
    while ((*it).cstr[0] != '[' && (*it).cstr[0] != '(')
    {
      ADVANCE_TOKEN;
    }
    return;
  }
  else if ( (*it) == "NONE" )
  {
    button[whichButton].clear();
  }
  else
  {
    printf("Unknown button assignment %s\n", (*it).cstr);
    button[whichButton].clear();
    while ((*it).cstr[0] != '[' && (*it).cstr[0] != '(')
    {
      ADVANCE_TOKEN;
    }
    return;
  }
  // get the next token ready
  ADVANCE_TOKEN;
}

void nslTestAppOptions::parseButtonMappings( vector<cstring>::iterator &it, vector<cstring>::iterator &it_end )
{
  while (it != it_end && (*it).cstr[0] != '[')
  {
    if ( (*it) == "(1)" )
    {
      parseButtonMapping( NSL_TESTBUTTON_B1, it, it_end );
    }
    else if ( (*it) == "(2)" )
    {
      parseButtonMapping( NSL_TESTBUTTON_B2, it, it_end );
    }
    else if ( (*it) == "(3)" )
    {
      parseButtonMapping( NSL_TESTBUTTON_B3, it, it_end );
    }
    else if ( (*it) == "(4)" )
    {
      parseButtonMapping( NSL_TESTBUTTON_B4, it, it_end );
    }
    else if ( (*it) == "(5)" )
    {
      parseButtonMapping( NSL_TESTBUTTON_B5, it, it_end );
    }
    else if ( (*it) == "(6)" )
    {
      parseButtonMapping( NSL_TESTBUTTON_B6, it, it_end );
    }
    else if ( (*it) == "(7)" )
    {
      parseButtonMapping( NSL_TESTBUTTON_B7, it, it_end );
    }
    else if ( (*it) == "(8)" )
    {
      parseButtonMapping( NSL_TESTBUTTON_B8, it, it_end );
    }
    else if ( (*it) == "(9)" )
    {
      parseButtonMapping( NSL_TESTBUTTON_B9, it, it_end );
    }
    else if ( (*it) == "(10)" )
    {
      parseButtonMapping( NSL_TESTBUTTON_B10, it, it_end );
    }
    else if ( (*it) == "(11)" )
    {
      parseButtonMapping( NSL_TESTBUTTON_B11, it, it_end );
    }
    else if ( (*it) == "(12)" )
    {
      parseButtonMapping( NSL_TESTBUTTON_B12, it, it_end );
    }
    else if ( (*it) == "(S)" )
    {
      parseButtonMapping( NSL_TESTBUTTON_START, it, it_end );
    }
    else
    {
      ADVANCE_TOKEN;
    }
  }
}

void nslTestAppOptions::parseSndFile( vector<cstring> &lines )
{
  vector<cstring>::iterator it;
  vector<cstring>::iterator it_end = lines.end();
  opt.soundSources.clear();
  for ( it = lines.begin(); it != it_end; ++it )
  {
    if ( (*it).cstr[0] == ';' )
    { 
      // skip comments
      continue;
    }
    else if ( strstr((*it).cstr, "COLLECTION ") != NULL )
    {
      break;
    }
    else
    {
      // an SND line of some sort
      vector<cstring> tokens;
      tokenize( (*it).cstr, " \n\r\t", tokens );

      // grab the source name
      vector<cstring>::iterator token_it = tokens.begin();
      opt.soundSources.push_back(*token_it);
    }
  }
}


bool nslTestAppOptions::setup()
{
  if (mappingFile.cstr[0] != '\0')
  {
    // load settings from a map file
    int map = sceOpen(mappingFile.cstr, SCE_RDONLY);
    if ( map < 0 )
    {
      printf("Error opening mapping file %s.\n", mappingFile.cstr);
      exit(-1);
    }
    vector<cstring> tokens;
    vector<cstring> lines;

    sceLseek(map, 0, SCE_SEEK_CUR);
    int size = sceLseek(map, 0, SCE_SEEK_END);
    sceLseek(map, 0, SCE_SEEK_SET);

    char *buf = NULL;
    if (size > 0)
    {
      buf = (char *)malloc( size );
      sceRead( map, buf, size );
      buf[size-1] = '\0';
    }
    sceClose( map );
    tokenize( buf, "\t\n\r ", tokens );
    free( buf );

    // load level snd file
    cstring sndName;
    strcpy( sndName.cstr, opt.levelName.cstr );
    strcat( sndName.cstr, ".SND" );
    strupr( sndName.cstr );
    cstring levelSndName( "host0:" );
    strcat( levelSndName.cstr, opt.soundRoot.cstr );
    strcat( levelSndName.cstr, "ENGLISH\\" );
    strcat( levelSndName.cstr, sndName.cstr );
    if (strstr( opt.soundRoot.cstr, "cdrom0:") != NULL)
    {
      strcat( levelSndName.cstr, ";1" );
    }

    int snd = sceOpen(levelSndName.cstr, SCE_RDONLY);
    if ( snd < 0 )
    {
      printf("Error opening snd file %s.\n", levelSndName.cstr);
      exit(-1);
    }

    sceLseek(snd, 0, SCE_SEEK_CUR);
    size = sceLseek(snd, 0, SCE_SEEK_END);
    sceLseek(snd, 0, SCE_SEEK_SET);

    buf = NULL;
    if (size > 0)
    {
      buf = (char *)malloc( size );
      sceRead( snd, buf, size );
      buf[size-1] = '\0';
    }
    sceClose( snd );
    tokenize( buf, "\n\r", lines );
    free( buf );

    parseSndFile( lines );

    // top level parsing of map file
    vector<cstring>::iterator it;
    vector<cstring>::iterator it_end = tokens.end();
    for ( it = tokens.begin(); it != it_end; ++it )
    {
      if ( (*it) == "[BUTTON_MAPPING]" )
      {
        // grab all of the button mappings
        ADVANCE_TOKEN
        parseButtonMappings( it, it_end );
      }
      else if ( (*it) == "[END]" )
      {
        // special end marker
        break;
      }
    }
  }
  return true;
}

void sleep(float msToSleep)
{
  start_timer();
  while ( check_time() * SECONDS_PER_TICK < msToSleep )
    /*spin*/;
}

void reset( char *fname )
{
  nslReset( fname );
  // load all of our sources
  vector<cstring>::iterator it = opt.soundSources.begin();
  vector<cstring>::iterator it_end = opt.soundSources.end();
  for ( ; it != it_end; ++it )
  {
#ifdef NSL_LOAD_SOURCE_BY_NAME
    nslLoadSource((*it).cstr);
#endif
#ifdef NSL_LOAD_SOURCE_BY_ALIAS
    nslLoadSource(atoi((*it).cstr));
#endif
  }
}

int main( int argc, char *argv[] )
{
  unsigned long int t, p;
  
  bool paused = false;

  opt.parseArgs( argc, argv );
  systemInit();
  nslInit( );
  nslSetHostStreamingPS2(true);
  if (!opt.setup())
  {
    printf("Error initializing test setup.  Call your local NSL programmer.\n");
    exit(-1);
  }



  // Init the controller fifo_queue  
  controllerCommands.init(10);
  
  // Init nsl
  cstring fname;
  nslSetRootDir( opt.soundRoot.cstr );
  strcpy( fname.cstr, opt.levelName.cstr );
  strcat( fname.cstr, ".SND" );

  reset( fname.cstr );

  printf("Beginning test.\n");
  while (1)
  {
    while (!controllerCommands.size()) 
    {
      nslFrameAdvance(0.01667f);
      // do critical processing
      for (int i=0; i<NSL_TESTBUTTON_NUM; ++i)
        opt.button[i].critical_process();

      checkControlPad(&controllerCommands);
      sleep(0.01667f);
    }
    while (controllerCommands.size() > 0)
    {
      int whichCommand = controllerCommands.pop();
      opt.button[whichCommand].process();
    }
  }

  /*
  nslSourceId src = nslLoadSource("KS_A_36");
  nslLoadSource("EXPL_CIE");
  nslLoadSource("CITYLOW");

  // Wait for button press
  
  printf("TESTING nslPlaySound\n==========================\n");
  
  printf("Please press a key to test PlayInvalidId\n");
  printf("Should assert\n");
  while (!controllerCommands.size()) 
    checkControlPad(&controllerCommands);
  controllerCommands.pop();
  nslTestPlayInvalidId();
  
  printf("Please press a key to test PlayValidIdAfterStreaming\n");
  while (!controllerCommands.size()) 
    checkControlPad(&controllerCommands);
  controllerCommands.pop();
  nslTestPlayValidIdAfterStreaming() ;

  printf("Please press a key to test PlayValidId\n");
  while (!controllerCommands.size()) 
    checkControlPad(&controllerCommands);
  controllerCommands.pop();
  nslTestPlayValidId();

  printf("Please press a key to test PlayAfterRunOut\n");
  printf("Should assert\n");
  while (!controllerCommands.size()) 
    checkControlPad(&controllerCommands);
  controllerCommands.pop();
  nslTestPlayAfterRunOut() ;

  printf("Please press a key to test PlayAfterStop\n");
  printf("Should assert\n");
  while (!controllerCommands.size()) 
    checkControlPad(&controllerCommands);
  controllerCommands.pop();
  nslTestPlayAfterStop() ;

  printf("TESTING nslIsSoundPlaying\n===============================\n");
  
  printf("Please press a key to test IsPlayingInvalidId\n");
  while (!controllerCommands.size()) 
    checkControlPad(&controllerCommands);
  controllerCommands.pop();
  nslTestIsPlayingInvalidId(  ) ;
  
  printf("Please press a key to test IsPlayingValidIdAfterStreaming\n");
  while (!controllerCommands.size()) 
    checkControlPad(&controllerCommands);
  controllerCommands.pop();
  nslTestIsPlayingValidIdAfterStreaming(  ) ;
  
  printf("Please press a key to test IsPlayingValidId\n");
  while (!controllerCommands.size()) 
    checkControlPad(&controllerCommands);
  controllerCommands.pop();
  nslTestIsPlayingValidId(  ) ;
  
  printf("Please press a key to test IsPlayingAfterRunOut\n");
  while (!controllerCommands.size()) 
    checkControlPad(&controllerCommands);  
  controllerCommands.pop();
  nslTestIsPlayingAfterRunOut(  ) ;

    printf("Please press a key to test IsPlayingAfterStop\n");
  while (!controllerCommands.size()) 
    checkControlPad(&controllerCommands);    
  controllerCommands.pop();
  nslTestIsPlayingAfterStop(  ) ;



  printf("TESTING nslIsSoundReady\n=============================\n");

  printf("Please press a key to test IsSoundReadyInvalidId\n");
  printf("Should assert\n");
  while (!controllerCommands.size()) 
    checkControlPad(&controllerCommands);  
  controllerCommands.pop();
  nslTestIsSoundReadyInvalidId(  ) ;
  

  printf("Please press a key to test IsSoundReadyValidId\n");
  while (!controllerCommands.size()) 
    checkControlPad(&controllerCommands);  
  controllerCommands.pop();
  nslTestIsSoundReadyValidId(  ) ;

  printf("Please press a key to test IsSoundReadyAfterRunOut\n");
  printf("Should assert\n");
  while (!controllerCommands.size()) 
    checkControlPad(&controllerCommands);  
  controllerCommands.pop();
  nslTestIsSoundReadyAfterRunOut(  ) ;

  printf("Please press a key to test IsSoundReadyWhilePaused\n");
  while (!controllerCommands.size()) 
    checkControlPad(&controllerCommands);  
  controllerCommands.pop();
  nslTestIsSoundReadyWhilePaused();

  printf("Please press a key to test IsSoundReadyAfterStop\n");
  printf("Should assert\n");
  while (!controllerCommands.size()) 
    checkControlPad(&controllerCommands);  
  controllerCommands.pop();
  nslTestIsSoundReadyAfterStop(  ) ;


  printf("TESTING nslPauseSound\n=============================\n");

  printf("Please press a key to test nslPauseSoundInvalidId\n");
  printf("Should assert\n");
  while (!controllerCommands.size()) 
    checkControlPad(&controllerCommands);  
  controllerCommands.pop();
  nslPauseSoundInvalidId();

  printf("Please press a key to test nslPauseSoundValidIdPlaying\n");
  while (!controllerCommands.size()) 
    checkControlPad(&controllerCommands);  
  controllerCommands.pop();
  nslPauseSoundValidIdPlaying();
  
  printf("Please press a key to test nslPauseSoundWhilePaused\n");
  while (!controllerCommands.size()) 
    checkControlPad(&controllerCommands);  
  controllerCommands.pop();
  nslPauseSoundWhilePaused();

  printf("Please press a key to test nslPauseSoundAfterRunOut\n");
  printf("Should assert\n");
  while (!controllerCommands.size()) 
    checkControlPad(&controllerCommands);  
  controllerCommands.pop();
  nslPauseSoundAfterRunOut();

  printf("Please press a key to test nslPauseSoundAfterStop\n");
  printf("Should assert\n");
  while (!controllerCommands.size()) 
    checkControlPad(&controllerCommands);  
  controllerCommands.pop();
  nslPauseSoundAfterStop();


  printf("TESTING nslUnpauseSound\n=============================\n");
 
  printf("Please press a key to test nslUnpauseSoundInvalidId\n");
  printf("Should assert\n");

  while (!controllerCommands.size()) 
    checkControlPad(&controllerCommands);  
  controllerCommands.pop();
  nslUnpauseSoundInvalidId();

  printf("Please press a key to test nslUnpauseSoundAfterStop\n");
  printf("Should assert\n");

  while (!controllerCommands.size()) 
    checkControlPad(&controllerCommands);  
  controllerCommands.pop();
  nslUnpauseSoundAfterStop();

  printf("Please press a key to test nslUnpauseSoundAfterRunOut\n");
  printf("Should assert\n");

  while (!controllerCommands.size()) 
    checkControlPad(&controllerCommands);  
  controllerCommands.pop();
  nslUnpauseSoundAfterRunOut();
  
  printf("Please press a key to test nslPauseUnpauseMultiple\n");
  while (!controllerCommands.size()) 
    checkControlPad(&controllerCommands);  
  controllerCommands.pop();
  nslPauseUnpauseMultiple(); 

  printf("TESTING nslPauseGuardSound\n=============================\n");
  printf("Please press a key to test nslPauseGuardSoundInvalidId\n");
  printf("Should assert\n");
  while (!controllerCommands.size()) 
    checkControlPad(&controllerCommands);  
  controllerCommands.pop();
  nslPauseGuardSoundInvalidId() ;                // ASSERT

  printf("Please press a key to test nslPauseGuardSoundValidIdAfterStart\n");
  while (!controllerCommands.size()) 
    checkControlPad(&controllerCommands);  
  controllerCommands.pop();
  nslPauseGuardSoundValidIdAfterStart() ;
 
  printf("Please press a key to test nslPauseGuardSoundPaused\n");
  printf("Should assert\n");
  while (!controllerCommands.size()) 
    checkControlPad(&controllerCommands);  
  controllerCommands.pop();
  nslPauseGuardSoundPaused() ;

  printf("Please press a key to test nslPauseGuardSoundValidIdAfterStop\n");
  printf("Should assert\n");
  while (!controllerCommands.size()) 
    checkControlPad(&controllerCommands);  
  controllerCommands.pop();
  nslPauseGuardSoundValidIdAfterStop() ;         // ASSERT

  printf("Please press a key to test nslPauseGuardSoundValidIdAfterRunOut\n");
  printf("Should assert\n");
  while (!controllerCommands.size()) 
    checkControlPad(&controllerCommands);  
  controllerCommands.pop();
  nslPauseGuardSoundValidIdAfterRunOut();        // ASSERT


  printf("TESTING nslPauseAllSounds\n=============================\n");
  printf("Please press a key to test nslPauseAllSoundsNoSounds\n");

  while (!controllerCommands.size()) 
    checkControlPad(&controllerCommands);  
  controllerCommands.pop();
  nslPauseAllSoundsNoSounds() ;                  
  
  
  printf("Please press a key to test nslPauseAllSoundsValid\n");
  while (!controllerCommands.size()) 
    checkControlPad(&controllerCommands);  
  controllerCommands.pop();
  nslPauseAllSoundsValid() ;

    printf("TESTING nslUnpauseAllSounds\n=============================\n");
  printf("Please press a key to test nslUnpauseAllSoundsNoSounds\n");

  while (!controllerCommands.size()) 
    checkControlPad(&controllerCommands);  
  controllerCommands.pop();
  nslUnpauseAllSoundsNoSounds() ;                  
  
  
  printf("Please press a key to test nslUnpauseAllSoundsPaused\n");
  while (!controllerCommands.size()) 
    checkControlPad(&controllerCommands);  
  controllerCommands.pop();
  nslUnpauseAllSoundsPaused() ;

  
  while (1)
  {
    printf("Please press a key to test nslUnpauseAllSoundsPaused\n");
    while (!controllerCommands.size()) {
      nslFrameAdvance(1);
      checkControlPad(&controllerCommands);  
    }
    controllerCommands.pop();
    nslPlaySound(nslAddSound(src));
  }


  nglVector v;
  v[0] = 0;
  v[1] = 0;
  v[2] = -1;
  nslEmitterId e   = nslCreateEmitter(v);

  //  OLD TEST
  // Setup our sources
  nslSourceId src1 = nslLoadSource( "STEALTH" );
  nslSourceId src2 = nslLoadSource( "STEAMVNT" );
  nslSourceId src3 = nslLoadSource( "EXPL_CEI" );
  
  // Setup our sounds
  nslSoundId s1 = NSL_INVALID_ID;
  nslSoundId s2;
  nslSoundId s3 = nslAddSound(src2);
  
  nslSetSoundEmitter(e, s3);
  nslSetSoundRange(s3, 0, 1.2);
  

  // Start the background sounds
  nslPlaySound(s3);

  // Init the timer
  t = get_cpu_cycle();
  float counter = 250;  
  // Loop with a .01 second delay
  while (1) {
    while ((get_cpu_cycle() - t) < PS2_CLOCK_SPEED/100);
    t = get_cpu_cycle();
    nslFrameAdvance(.1);
        
    
    setEmitterFromDegrees(counter, 1, e);
    // Get controller info
    checkControlPad(&controllerCommands);
    
    // Deal with it
    while (controllerCommands.size()) {
      // Examine buttons
      int button = controllerCommands.pop();
      switch (button) {
        case NSL_TESTBUTTON_B5:        
          counter += .1;
        break;
        case NSL_TESTBUTTON_B6:        
          counter -= .1;
        break;
        case NSL_TESTBUTTON_B7:        
          counter -= .01;
        break;
        case NSL_TESTBUTTON_B8:        
          counter += .01;
        break;
        case NSL_TESTBUTTON_B3:
          if (nslIsSoundPlaying(s1)) {
            nslStopSound(s1);
          } else {
            s1 = nslAddSound(src3);
            nslPlaySound(s1);
          }
        break;
        case NSL_TESTBUTTON_START:
          if (paused) {
            nslUnpauseAllSounds();
            paused=false;
          } 
          else 
          {
            nslPauseAllSounds();
            paused=true;
          }
        break;
      } 
    } 
         
 
  }

  */




  nslShutdown();

  return 0;
}
