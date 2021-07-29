#ifndef OPTIONSFILE_H
#define OPTIONSFILE_H


#include "inputmgr.h"

enum game_option_e
{
  GAME_OPT_LANGUAGE,
  GAME_OPT_DIFFICULTY,
  GAME_OPT_MUSIC_VOLUME,
  GAME_OPT_SFX_VOLUME,
  GAME_OPT_BRIGHTNESS,
  GAME_OPT_JUMPPACK,
  GAME_OPT_AUDIO,
  GAME_OPT_VMU_AUDIO,
  GAME_OPT_CINEMATICS,
  GAME_OPT_VIBRATOR,
  GAME_OPT_HIGHEST_LEVEL,
  GAME_OPT_PC_JOYSTICK_ENABLED,
  GAME_OPT_PC_MOUSE_ENABLED,

  GAME_OPT_PC_CONFIG_FORWARD,
  GAME_OPT_PC_CONFIG_BACKWARD,
  GAME_OPT_PC_CONFIG_TURNLEFT,
  GAME_OPT_PC_CONFIG_TURNRIGHT,
  GAME_OPT_PC_CONFIG_JUMP,
  GAME_OPT_PC_CONFIG_PUNCH_USE,
  GAME_OPT_PC_CONFIG_KICK,
  GAME_OPT_PC_CONFIG_SNIPER_MODE,
  GAME_OPT_PC_CONFIG_ACTIVATE,
  GAME_OPT_PC_CONFIG_DRAW_SHEATHE,
  GAME_OPT_PC_CONFIG_NTEK,
  GAME_OPT_PC_CONFIG_TURBO_NTEK,
  GAME_OPT_PC_CONFIG_STEALTH_NTEK,

  GAME_OPT_PC_CONFIG_FORWARD_DEVICE,
  GAME_OPT_PC_CONFIG_BACKWARD_DEVICE,
  GAME_OPT_PC_CONFIG_TURNLEFT_DEVICE,
  GAME_OPT_PC_CONFIG_TURNRIGHT_DEVICE,
  GAME_OPT_PC_CONFIG_JUMP_DEVICE,
  GAME_OPT_PC_CONFIG_PUNCH_USE_DEVICE,
  GAME_OPT_PC_CONFIG_KICK_DEVICE,
  GAME_OPT_PC_CONFIG_SNIPER_MODE_DEVICE,
  GAME_OPT_PC_CONFIG_ACTIVATE_DEVICE,
  GAME_OPT_PC_CONFIG_DRAW_SHEATHE_DEVICE,
  GAME_OPT_PC_CONFIG_NTEK_DEVICE,
  GAME_OPT_PC_CONFIG_TURBO_NTEK_DEVICE,
  GAME_OPT_PC_CONFIG_STEALTH_NTEK_DEVICE,

  GAME_OPT_MESHES,
  GAME_OPT_DETAIL,

  GAME_OPT_TOT_DEFINED,
  GAME_OPT_TOTAL = 64,
};

enum cont_config_e
{
  CONFIG_ATTACK,
  CONFIG_KICK,
  CONFIG_ACTIVATE,
  CONFIG_JUMP,
  CONFIG_GUNTOGGLE,
  CONFIG_NTEKCHARGE,
  TOTAL_CONFIGS,

  PC_CONFIG_FIRST = TOTAL_CONFIGS,
  PC_CONFIG_FORWARD = TOTAL_CONFIGS,
  PC_CONFIG_BACKWARD,
  PC_CONFIG_TURNLEFT,
  PC_CONFIG_TURNRIGHT,
  PC_CONFIG_JUMP,

  PC_CONFIG_PUNCH_USE,
  PC_CONFIG_KICK,
  PC_CONFIG_SNIPER_MODE,
  PC_CONFIG_ACTIVATE,

  PC_CONFIG_DRAW_SHEATHE,
  PC_CONFIG_NTEK,
  PC_CONFIG_TURBO_NTEK,
  PC_CONFIG_STEALTH_NTEK,

  //  PC_CONFIG_MOUSE_TURN_ENABLED,

  TOTAL_ALL_CONFIGS,
  PC_CONFIG_LAST = TOTAL_ALL_CONFIGS,
  TOTAL_PC_CONFIGS = TOTAL_ALL_CONFIGS - TOTAL_CONFIGS
};



#define DEF_OPTFILE_VERSION_NUM   1001 // change when optionsfile_t changes!!
#define OPTIONSFILE_BLOCKS  18 // change this as needed also

class optionsfile_t
{
public:
  optionsfile_t();
  ~optionsfile_t();

  void init();

  void set_default_options();
  void set_default_cont_configs();

  bool save();
  bool load();

  unsigned char get_option( game_option_e index ) { return options[index]; }
  void set_option( game_option_e index, unsigned char val );

  axis_id_t get_cont_config( cont_config_e index ) { return cont_configs[index]; }
  void set_cont_config(  cont_config_e index, axis_id_t axid, device_id_t did = JOYSTICK_DEVICE );

  device_id_t get_cont_type_config( cont_config_e index ) { return ( cont_type_configs[index] ); }
  void set_cont_type_config( cont_config_e index, device_id_t did );

protected:
  short version_num; // change DEF_OPTFILE_VERSION_NUM every time optionsfile_t's data format changes!!
  unsigned char options[GAME_OPT_TOTAL];  // DID YOU ADD OR DELETE A VAR?  CHANGE VERSION!!!!!!!!
  axis_id_t cont_configs[TOTAL_ALL_CONFIGS];  // DID YOU ADD OR DELETE A VAR?  CHANGE VERSION!!!!!!!!
  device_id_t cont_type_configs[TOTAL_ALL_CONFIGS];  // DID YOU ADD OR DELETE A VAR?  CHANGE VERSION!!!!!!!!
};


#endif // OPTIONSFILE_H
