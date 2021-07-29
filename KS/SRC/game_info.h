// game_info.h
#ifndef GAME_INFO_HEADER
#define GAME_INFO_HEADER

struct cheat_info_t
{
  bool enabled;

  bool change_camera;
  bool demigod_cheat;

  cheat_info_t()
  {
    #if defined(BUILD_BOOTABLE)
    enabled = false;
    #else
    enabled = true;
    #endif

    change_camera = false;
    demigod_cheat = false;
  }
};
extern cheat_info_t g_cheats;

struct debug_info_t
{
  bool dump_frame_info : 1;
  bool dump_threads : 1;
  bool render_spheres : 1;
  bool turtle_watch : 1;

  // PS2 specific
  bool halt_on_asserts : 1;
  bool stay_halted : 1;
  bool simulate32meg : 1;
  bool assert_screen : 1;
  bool mem_free_screen : 1;
  debug_info_t()
  {
    dump_frame_info = false;
    dump_threads = false;
    render_spheres = false;
    turtle_watch = false;

    assert_screen = false;
    halt_on_asserts = true;
    stay_halted = false;
    mem_free_screen = false;
#if defined(BUILD_BOOTABLE)
//&& !defined(AUTOBUILD) && (!defined(EVAN) || defined(MEMTRACK))
	simulate32meg = true;
#else
	simulate32meg = false;
#endif
  }
};
extern debug_info_t g_debug;

#endif
