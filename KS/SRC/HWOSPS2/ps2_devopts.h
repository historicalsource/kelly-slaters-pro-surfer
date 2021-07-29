#ifndef PS2_DEVOPTS_H
#define PS2_DEVOPTS_H
////////////////////////////////////////////////////////////////////////////////
/*
  mull_devopts

  singleton class that describes the develeper settings/options, currently
  set in an ini file
*/
////////////////////////////////////////////////////////////////////////////////

#include "algebra.h"

////////////////////////////////////////////////////////////////////////////////
class os_developer_options : public singleton
{
public:
  // construct and read the ini file
  os_developer_options();

  enum flags_t
  {
#define MAC(x,y,d) x,
#include "devoptflags.h"
#undef MAC
    NUM_FLAGS
  };

  enum strings_t
  {
#define MAC(x,y,d) x,
#include "devoptstrs.h"
#undef MAC
    NUM_STRINGS
  };

  enum ints_t
  {
#define MAC(x,y,d) x,
#include "devoptints.h"
#undef MAC
    NUM_INTS
  };

  bool is_flagged( flags_t _flag ) const { return flags[_flag]; }
  bool get_flag( flags_t _flag ) const { return flags[_flag]; }
  void set_flag( flags_t _flag, bool v ) { flags[_flag] = v; }
  void toggle_flag( flags_t _flag ) { flags[_flag] = 1-flags[_flag]; }
  const stringx& get_string( strings_t _string ) const { return strings[_string]; }
  int get_int( ints_t _int ) const { return ints[_int]; }
  void set_int( ints_t _int, int a ) { ints[_int] = a; }

  // check for memory leaks
  //bool leak_checking_on() const  { return is_flagged( FLAG_LEAK_CHECKING ); }

  // start the game running in a window
  //bool is_window_default()    const  { return is_flagged( FLAG_WINDOW_DEFAULT ); }

  // show FPS and poly-count stats
  //bool show_stats_on()        const  { return is_flagged( FLAG_SHOW_STATS ); }

  // use indexed primitives (default) rather than vertex masses
  //bool indexed_primitives_on() const { return is_flagged( FLAG_INDEXED_PRIMITIVES ); }

  // dump heap once game is running
  //bool memory_tracking_on() const    { return is_flagged( FLAG_MEMORY_TRACKING ); }

  //  get scene name
  const stringx& get_scene_name() const    { return get_string( STRING_SCENE_NAME ); }

  //  get hero name
  const stringx& get_hero_name(int hero_num) const
      {
        assert(STRING_HERO_NAME_0 + hero_num < STRING_END_HERO_NAMES);
        return get_string( (strings_t)(STRING_HERO_NAME_0 + hero_num) );
      }

  const stringx& get_song_name() const    { return get_string( STRING_SONG_NAME ); }

  const stringx& get_exe_name() const     { return exe_name; }

  // temporary

  int get_camera_state() const { return get_int( INT_CAMERA_STATE ); }
  void set_camera_state(int i) { ints[INT_CAMERA_STATE]=i; }

  //bool are_textures_lores() const { return is_flagged( FLAG_TEXTURES_LORES); }
  //bool show_debug_interface() const { return is_flagged(FLAG_INTERFACE_DEBUG); }
  // debugging
  //bool get_show_debug_info() const { return is_flagged( FLAG_SHOW_DEBUG_INFO); }
  //void set_show_debug_info(bool i) { flags[ FLAG_SHOW_DEBUG_INFO ];}

  //void toggle_show_debug_info() { toggle_flag( FLAG_SHOW_DEBUG_INFO );}

  //bool get_debug_log_on() const { return is_flagged( FLAG_DEBUG_LOG ); }

  bool get_gravity_enabled() const { return is_flagged( FLAG_GRAVITY ); }
  void set_gravity_enabled(bool i) { flags[FLAG_GRAVITY]=i; }
  void toggle_gravity_enabled() { toggle_flag( FLAG_GRAVITY ); }

  bool get_no_warnings() const { return is_flagged( FLAG_NO_WARNINGS ); }
  bool get_no_lighting_warnings() const { return is_flagged( FLAG_NO_LIGHTING_WARNINGS ); }

  bool get_lock_step() const { return is_flagged( FLAG_LOCK_STEP ); }
  void set_lock_step(bool i) { flags[FLAG_LOCK_STEP]=i; }
  void toggle_lock_step() { toggle_flag( FLAG_LOCK_STEP ); }

  bool is_texture_dump_requested() const { return is_flagged( FLAG_TEXTURE_DUMP ); }

  bool is_locked_hero() const { return is_flagged( FLAG_LOCKED_HERO ); }

  bool is_move_editor() const { return is_flagged( FLAG_MOVE_EDITOR ); }

  //vector<stringx> * get_entity_name_list() { return &entity_name_list; }
  //vector<vector3d> * get_entity_loc_list() { return &entity_loc_list; }

  //void set_debugaipath(bool on) { flags[FLAG_AI_PATH_DEBUG]=on; }

  // the instance of the command line

  DECLARE_SINGLETON(os_developer_options)

private:
  bool    flags[NUM_FLAGS];
  stringx strings[NUM_STRINGS];
  int     ints[NUM_INTS];
  stringx exe_name;

  //vector<stringx> entity_name_list;
  //vector<vector3d> entity_loc_list;

private:
  void parse( stringx command_line );
};

#endif
