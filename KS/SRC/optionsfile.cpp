#include "global.h"

#include "osdevopts.h"
//P #include "optionsfile.h"
#include "game.h"
//P #include "osstorage.h"
#include "keyboard.h"
#include "joystick.h"
#include "localize.h"
//P #include "warning.h"
#ifdef TARGET_MKS
#include "vmu_beep.h"
#endif

extern game *g_game_ptr;


//------------------------------------------------------------------------

optionsfile_t::optionsfile_t()
{
  init();
}


//------------------------------------------------------------------------


optionsfile_t::~optionsfile_t()
{
}


//------------------------------------------------------------------------

void optionsfile_t::init()
{
  version_num = DEF_OPTFILE_VERSION_NUM;
  set_default_options();
  set_default_cont_configs();
}


//------------------------------------------------------------------------

bool optionsfile_t::save()
{
  bool err = false;

  if ( syvars->storage_unit_exists && !g_game_ptr->get_vmu_checker()->do_skip_save() && !g_game_ptr->get_vmu_checker()->cur_vmu_is_full() )
  {
    if ( g_game_ptr->get_vmu_checker()->cur_vmu_is_different() ||
         g_game_ptr->get_vmu_checker()->cur_vmu_was_removed() )
    {
      err = true;
    }
    else
    {
      stringx shortdesc = os_developer_options::inst()->get_string(os_developer_options::STRING_GAME_TITLE);
      stringx longtitle = os_developer_options::inst()->get_string(os_developer_options::STRING_GAME_LONG_TITLE);
      stringx longdesc = longtitle + " " + localize_text_safe("$Options");

/*P
      storage_unit *sup = storage_mgr::inst()->get_unit( syvars->cur_storage_unit_slot );
      if ( sup )
      {
        err = !sup->save_file( "MAXSTEEL.OPT", (void*)this, sizeof(optionsfile_t), shortdesc.c_str(), longdesc.c_str(), g_game_ptr->get_icon_data(),
          0, 0, (get_option(GAME_OPT_VMU_AUDIO) == 0) );
      }
      else
        err = true;
      storage_mgr::inst()->release_unit( sup );
P*/  
      if ( !err )
      {
        g_game_ptr->get_vmu_checker()->set_optionsfile_exists( true );
        g_game_ptr->get_vmu_checker()->update_optionsfile_time();
      }
    }
  }
  g_game_ptr->get_vmu_checker()->set_skip_save(false);

  return( err );
}

//------------------------------------------------------------------------



bool optionsfile_t::load()
{
  uint32 filesize = 1;
  
  g_game_ptr->get_vmu_checker()->set_optionsfile_exists( false );

  if ( syvars->storage_unit_exists )
  {
/*P
    storage_unit *sup = storage_mgr::inst()->get_unit( syvars->cur_storage_unit_slot );
    filesize = sup->load_file( "MAXSTEEL.OPT", (void*)this, sizeof(optionsfile_t), 0,
      false );  // no beep till we check option
    storage_mgr::inst()->release_unit( sup );
P*/
    if ( filesize )
    {
      if ( version_num != DEF_OPTFILE_VERSION_NUM )
      {
        extern bool g_do_version_warning;
        g_do_version_warning = true;
      }
#ifdef TARGET_MKS
      if ( get_option(GAME_OPT_VMU_AUDIO) == 0 )
      {
        vmu_happy_beep( syvars->cur_storage_unit_slot, BEEP_LENGTH );
        g_game_ptr->get_vmu_checker()->set_optionsfile_exists( true );
      }

      // update stereo/mono option from rom setting
      Sint32 res, stereomode;
      res = syCfgGetSoundMode( &stereomode );
      int sound_opt = ( stereomode == SYD_CFG_MONO ) ? 1 : 0;
      set_option( GAME_OPT_AUDIO, sound_opt );
#endif
      g_game_ptr->get_vmu_checker()->update_optionsfile_time();
    }
#ifdef TARGET_MKS
    else if ( get_option(GAME_OPT_VMU_AUDIO) == 0 )
    {
      vmu_sad_beep( syvars->cur_storage_unit_slot, BEEP_LENGTH*2 );
    }
#endif
  }
  return( filesize == 0 );
}


void optionsfile_t::set_option( game_option_e index, unsigned char val )
{
  options[index] = val;
}


//------------------------------------------------------------------------

void optionsfile_t::set_cont_config( cont_config_e index, axis_id_t axid, device_id_t did )
{ 
  cont_configs[index] = axid;
  cont_type_configs[index] = did;
}

//------------------------------------------------------------------------

void optionsfile_t::set_cont_type_config( cont_config_e index, device_id_t did )
{ 
  cont_type_configs[index] = did;
}


//------------------------------------------------------------------------

// sets all options except controls

// NOTE WELL!!!!!!!!!!!!!!!!!!!!!!!
// change DEF_OPTFILE_VERSION_NUM every time defaults change!!

void optionsfile_t::set_default_options() // see pda_options_page::pda_options_page for meaning of most values (which are indices into option_values)
{

// NOTE WELL!!!!!!!!!!!!!!!!!!!!!!!
// change DEF_OPTFILE_VERSION_NUM every time defaults change!!

  // NB: THESE CONTROLLER ENTRIES MATCH UP AGAINST
  // THE set_default_cont_config() FUNCTION JUST BELOW

  unsigned char default_value[GAME_OPT_TOT_DEFINED] =
  {
    0,   //  GAME_OPT_LANGUAGE,            0
#ifdef BUILD_BOOTABLE
    1,   //  GAME_OPT_DIFFICULTY,          1   default to MEDIUM for the public
#else
    2,   //  GAME_OPT_DIFFICULTY,          2   default to HARD internally
#endif
    7,   //  GAME_OPT_MUSIC_VOLUME,        2   values are 0-10
    10,  //  GAME_OPT_SFX_VOLUME,          3
    5,   //  GAME_OPT_BRIGHTNESS,          4
    0,   //  GAME_OPT_JUMPPACK,            5
    0,   //  GAME_OPT_AUDIO,               6
    0,   //  GAME_OPT_VMU_AUDIO,           7   0 is on, 1 is off
    0,   //  GAME_OPT_CINEMATICS,          8
    0,   //  GAME_OPT_VIBRATOR,            9
    0,   //  GAME_OPT_HIGHEST_LEVEL,       10
    1,   //  GAME_OPT_PC_JOYSTICK_ENABLED, 11  on
    1,   //  GAME_OPT_PC_MOUSE_ENABLED,    12  on

    KB_W,   //  GAME_OPT_PC_CONFIG_FORWARD,
    KB_S,   //  GAME_OPT_PC_CONFIG_BACKWARD,
    KB_A,   //  GAME_OPT_PC_CONFIG_TURNLEFT,
    KB_D,   //  GAME_OPT_PC_CONFIG_TURNRIGHT,
    KB_LSHIFT,   //  GAME_OPT_PC_CONFIG_JUMP,
    KB_PAGEUP,   //  GAME_OPT_PC_CONFIG_PUNCH_USE,
    KB_PAGEDOWN,   //  GAME_OPT_PC_CONFIG_KICK,
    KB_LSHIFT,   //  GAME_OPT_PC_CONFIG_SNIPER_MODE,
    KB_SPACE,   //  GAME_OPT_PC_CONFIG_ACTIVATE,
    KB_BACKSPACE,   //  GAME_OPT_PC_CONFIG_DRAW_SHEATHE,
    KB_N,   //  GAME_OPT_PC_CONFIG_NTEK,
    KB_T,   //  GAME_OPT_PC_CONFIG_TURBO_NTEK,
    KB_BACKSLASH,   //  GAME_OPT_PC_CONFIG_STEALTH_NTEK,

    KEYBOARD_DEVICE,   //  GAME_OPT_PC_CONFIG_FORWARD_DEVICE,
    KEYBOARD_DEVICE,   //  GAME_OPT_PC_CONFIG_BACKWARD_DEVICE,
    KEYBOARD_DEVICE,   //  GAME_OPT_PC_CONFIG_TURNLEFT_DEVICE,
    KEYBOARD_DEVICE,   //  GAME_OPT_PC_CONFIG_TURNRIGHT_DEVICE,
    KEYBOARD_DEVICE,   //  GAME_OPT_PC_CONFIG_JUMP_DEVICE,
    KEYBOARD_DEVICE,   //  GAME_OPT_PC_CONFIG_PUNCH_USE_DEVICE,
    KEYBOARD_DEVICE,   //  GAME_OPT_PC_CONFIG_KICK_DEVICE,
    KEYBOARD_DEVICE,   //  GAME_OPT_PC_CONFIG_SNIPER_MODE_DEVICE,
    KEYBOARD_DEVICE,   //  GAME_OPT_PC_CONFIG_ACTIVATE_DEVICE,
    KEYBOARD_DEVICE,   //  GAME_OPT_PC_CONFIG_DRAW_SHEATHE_DEVICE,
    KEYBOARD_DEVICE,   //  GAME_OPT_PC_CONFIG_NTEK_DEVICE,
    KEYBOARD_DEVICE,   //  GAME_OPT_PC_CONFIG_TURBO_NTEK_DEVICE,
    KEYBOARD_DEVICE,   //  GAME_OPT_PC_CONFIG_STEALTH_NTEK_DEVICE,
  };

  int i;
  for ( i = 0; i < GAME_OPT_TOT_DEFINED; ++i )  // don't write over controls
  {
    set_option( game_option_e(i), default_value[i] );
  }

  // zero everything else out
  for ( i = GAME_OPT_TOT_DEFINED; i < GAME_OPT_TOTAL; ++i )
  {
    set_option( game_option_e(i), 0 );
  }
}


//------------------------------------------------------------------------

// NOTE WELL!!!!!!!!!!!!!!!!!!!!!!!
// change DEF_OPTFILE_VERSION_NUM every time defaults change!!


void optionsfile_t::set_default_cont_configs()
{
// NOTE WELL!!!!!!!!!!!!!!!!!!!!!!!
// change DEF_OPTFILE_VERSION_NUM every time defaults change!!

  axis_id_t default_value[TOTAL_CONFIGS] =
  {
    JOY_BTNA, // CONFIG_ATTACK
    JOY_BTNB, // CONFIG_KICK
    JOY_BTNX, // CONFIG_ACTIVATE
    JOY_BTNY, // CONFIG_JUMP
    JOY_BTNR, // CONFIG_GUNTOGGLE
    JOY_BTNL, // CONFIG_NTEKCHARGE
  };
  int i;
  for( i = 0; i < TOTAL_CONFIGS; ++i )
  {
    set_cont_config( cont_config_e(i), default_value[i] );
  }


  // DEFAULT MAPPING FOR PC KEYBOARD HERO CONTROL
  // MATCHES AGAINST
  //enum cont_config_e
  // LOCATED IN GAME.H
//   { "FORWARD",  PC_CONFIG_FORWARD },
//   { "BACKWARD",  PC_CONFIG_BACKWARD },
//   { "TURN LEFT",  PC_CONFIG_TURNLEFT },
//   { "TURN RIGHT",  PC_CONFIG_TURNRIGHT },
//   { "JUMP",           PC_CONFIG_JUMP },

//   { "PUNCH",          PC_CONFIG_PUNCH_USE },
//   { "KICK",           PC_CONFIG_KICK },
//   { "SNIPER",         PC_CONFIG_SNIPER_MODE },
//   { "ACTIVATE",       PC_CONFIG_ACTIVATE },

//   { "DRAW-SHEATHE",   PC_CONFIG_DRAW_SHEATHE },
//   { "N-TEK",          PC_CONFIG_NTEK },
//   { "TURBO",          PC_CONFIG_TURBO_NTEK },
//   { "STEALTH",        PC_CONFIG_STEALTH_NTEK }
axis_id_t pcdef[] =
{
  KB_UP,
  KB_DOWN,
  KB_LEFT,
  KB_RIGHT,
  KB_Y,
  KB_A,
  KB_B,
  KB_Z,
  KB_X,
  KB_R,
  KB_L,
  KB_A,
  KB_B,
};
device_id_t pcdev[] =
{
  KEYBOARD_DEVICE,
  KEYBOARD_DEVICE,
  KEYBOARD_DEVICE,
  KEYBOARD_DEVICE,
  KEYBOARD_DEVICE,
  KEYBOARD_DEVICE,
  KEYBOARD_DEVICE,
  KEYBOARD_DEVICE,
  KEYBOARD_DEVICE,
  KEYBOARD_DEVICE,
  KEYBOARD_DEVICE,
  KEYBOARD_DEVICE,
  KEYBOARD_DEVICE,
};

  for( i = PC_CONFIG_FIRST; i < PC_CONFIG_LAST; ++i )
  {
    set_cont_config( cont_config_e(i), pcdef[ i - PC_CONFIG_FIRST ], pcdev[ i - PC_CONFIG_FIRST ] );
  }
}


