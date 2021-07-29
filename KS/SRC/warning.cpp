// warning.cpp:  max steel specific stuff related to sega warnings


#include "global.h"

//P #include "warning.h"
#include "game.h"
#include "hwmovieplayer.h"
//P #include "popup.h"
//P #include "optionsfile.h"
//P #include "gamefile.h"
#include "localize.h"
//P #include "pda.h"
//P #include "pda_fe_main.h"

#include <cstdarg>


extern game *g_game_ptr;



void vmu_callback_t::storage_unit_changed( uint32 drive )
{
  if ( !g_game_ptr || !g_game_ptr->get_popup_holder() )
    return;

  g_game_ptr->set_cont_update_flag();

  if ( g_game_ptr->get_vmu_selector() && g_game_ptr->get_vmu_selector()->is_shown() )
  {
    g_game_ptr->get_vmu_selector()->init_vmu_menu();
  }
  else
  {
    bool vmu_removed = !g_game_ptr->get_vmu_checker()->does_vmu_exist(drive);
    
    // screen out storage_mgr-init-caused remount callbacks
    if ( g_game_ptr->get_vmu_checker()->removed[drive] == vmu_removed )
      return;

    // do a warning
    g_game_ptr->get_vmu_checker()->do_warning[drive] = true;

    // update removed flag for this VMU
    g_game_ptr->get_vmu_checker()->removed[drive] = vmu_removed;
  }
}


vmu_checker_t::vmu_checker_t()
{
  init();
}


void vmu_checker_t::init()
{
  for ( int i = 0; i < MAX_STORAGE_UNITS; ++i )
  {
    do_warning[i] = false;
    removed[i] = false;
  }

  cur_vmu_different = false;
  cur_vmu_removed = false;
  cur_vmu_full = false;
  skip_save = false;
  optionsfile_exists = false;
  optionsfile_time = default_storage_time;
  status_check_timer = 0.0f;
}


void vmu_checker_t::frame_advance( time_value_t time_inc )
{
  if ( movieplayer::inst()->is_playing() )
    return;

  if ( status_check_timer != 0.0f )
  {
    if ( (status_check_timer - time_inc) <= 0.0f )
    {
      check_cur_vmu_status();
      status_check_timer = 0.0f;
    }
    else
    {
      status_check_timer -= time_inc;
    }
  }

  for ( int i = 0; i < MAX_STORAGE_UNITS; ++i )
  {
    if ( do_warning[i] )
    {
      warn();
      break;
    }
  }
}


bool vmu_checker_t::does_vmu_exist( int slot )
{
  bool exists = false;
/*P
  storage_unit *unit = storage_mgr::inst()->get_unit( slot );
  if ( unit )
  {
    exists = true;
  }
  storage_mgr::inst()->release_unit( unit );
P*/
  if ( syvars->storage_unit_exists && slot == syvars->cur_storage_unit_slot )
  {
    cur_vmu_removed = !exists;
  }

  return exists;
}


bool vmu_checker_t::loading_is_valid() const
{
  return ( syvars->storage_unit_exists &&
           g_game_ptr->get_gamefile()->get_num_saved_files() > 0 &&
           !cur_vmu_removed &&
           !cur_vmu_different );
}



void vmu_checker_t::warn()
{
  for ( int i = 0; i < MAX_STORAGE_UNITS; ++i )
  {
    if ( do_warning[i] )
    {
      bool in_cur_slot = false;
      if ( syvars->storage_unit_exists && i == syvars->cur_storage_unit_slot )
        in_cur_slot = true;

      // WAS IT REMOVED?
      if ( removed[i] )
      {
        // if so, was it our slot?
        if ( in_cur_slot && !cur_vmu_different )
        {
          if ( g_game_ptr->get_pfe_pda() )
          {
            g_game_ptr->get_pfe_pda()->get_fe_main()->set_skip_load( true );
          }
          g_game_ptr->do_warning_process( "$VMURemoved", 5.0f );
        }
        // or not?
        else
        {
          //g_game_ptr->do_warning_process( "$OtherVMURemoved", 3.0f );
        }
      }
      // OR WAS IT INSERTED?
      else
      {
        // if so, was it our slot?
        if ( in_cur_slot )
        {
          verify_same_vmu();
          if ( cur_vmu_different )
          {
            g_game_ptr->do_warning_process( "$WrongVMU", 4.0f );
          }
          else
          {
            if ( !cur_vmu_full && optionsfile_exists )
              g_game_ptr->do_warning_process( "$VMUReinserted", 2.0f );
            if ( g_game_ptr->get_pfe_pda() )
              g_game_ptr->get_pfe_pda()->get_fe_main()->set_skip_load( false );
          }
        }
        // or not?
        else
        {
          g_game_ptr->do_warning_process( "$ControlRemoved", 2.0f, false,
            g_game_ptr->get_popup_holder()->get_controller_removed_popup() );
        }
      }

      do_warning[i] = false;
    }
  }
}


void vmu_checker_t::update_optionsfile_time()
{
  if ( optionsfile_exists )
  {
/*P
    storage_unit *unit = storage_mgr::inst()->get_unit( syvars->cur_storage_unit_slot );
    if ( unit->get_file_time( "MAXSTEEL.OPT", &optionsfile_time ) )
    {
      optionsfile_time = default_storage_time;
    }
    storage_mgr::inst()->release_unit( unit );
P*/
  }
}



void vmu_checker_t::verify_same_vmu()
{
  if ( !syvars->storage_unit_exists || cur_vmu_full || cur_vmu_removed || !optionsfile_exists )
    return;

  cur_vmu_different = false;

/*P
  storage_unit *unit = storage_mgr::inst()->get_unit( syvars->cur_storage_unit_slot );
  storage_time time = default_storage_time;
  if ( unit->get_file_time( "MAXSTEEL.OPT", &time )
#ifdef TARGET_MKS
    // lamely done this way because there is no "!=" operator for BUS_TIME
       || (optionsfile_time < time || time < optionsfile_time)
#endif
       )
  {
    cur_vmu_different = true;
  }
  storage_mgr::inst()->release_unit( unit );
P*/
}



void vmu_checker_t::pre_save_vmu_warnings()
{
#ifdef TARGET_MKS
  if ( !syvars->storage_unit_exists ) return;

  warning_if_vmu_removed();
  if ( !cur_vmu_removed )
  {
    warning_if_vmu_different();
  }
  if ( !cur_vmu_different )
  {
    check_vmu_space_and_warn();
  }
#endif
}




void vmu_checker_t::warning_if_vmu_removed()
{
  if ( syvars->storage_unit_exists && cur_vmu_removed )
  {
    g_game_ptr->do_warning_process( "$VMURemovedCancel", 0.0f, true, g_game_ptr->get_popup_holder()->get_cancel_save_vmu_removed_popup() );
  }
}


void vmu_checker_t::warning_if_vmu_different()
{
  if ( !syvars->storage_unit_exists || cur_vmu_full || cur_vmu_removed || !optionsfile_exists )
    return;

  verify_same_vmu();

  if ( cur_vmu_different )
  {
    g_game_ptr->do_warning_process( "$DiffVMUCancel", 0.0f, true, g_game_ptr->get_popup_holder()->get_cancel_save_vmu_different_popup() );
  }
}


void vmu_checker_t::check_vmu_space_and_warn( bool at_save )
{
  // if we already know it's not there, different, or full, skip out
  if ( !syvars->storage_unit_exists || cur_vmu_removed || cur_vmu_different || cur_vmu_full )
    return;

  cur_vmu_full = false;

/*P  storage_unit *unit = storage_mgr::inst()->get_unit( syvars->cur_storage_unit_slot );

  int our_blocks_used = g_game_ptr->get_gamefile()->get_num_saved_files() * GAMEFILE_BLOCKS;
  if ( optionsfile_exists )
    our_blocks_used += OPTIONSFILE_BLOCKS;

  int required_blocks = GAMEFILE_BLOCKS + OPTIONSFILE_BLOCKS;

  int blocks_needed = required_blocks - our_blocks_used;
  if ( blocks_needed < 0 )
    blocks_needed = 0;

  // not enough room?
  if ( blocks_needed > (int)unit->get_free_blocks() )
  {
    syvars->max_gamefile_slots = 0;
    cur_vmu_full = true;
    stringx message;
    if ( at_save )
    {
      message = localize_text_safe("$FullVMU1") + stringx(required_blocks) + localize_text_safe("$FullVMUCancel");
      g_game_ptr->do_warning_process( message, 0.0f, true, g_game_ptr->get_popup_holder()->get_cancel_save_vmu_full_popup() );
    }
    else
    {
      message = localize_text_safe("$FullVMU1") + stringx(required_blocks) + localize_text_safe("$FullVMU2");
      g_game_ptr->do_warning_process( message, 0.0f, true, g_game_ptr->get_popup_holder()->get_cancel_popup() );
    }
  }
  // otherwise set max gamefile slots
  else
  {
    int avail_gamefile_blocks = unit->get_free_blocks() +
                                (g_game_ptr->get_gamefile()->get_num_saved_files() * GAMEFILE_BLOCKS);
    syvars->max_gamefile_slots = avail_gamefile_blocks / GAMEFILE_BLOCKS;
    if ( syvars->max_gamefile_slots > MAX_GAMEFILE_SLOTS )
      syvars->max_gamefile_slots = MAX_GAMEFILE_SLOTS;
  }

  // update next gamefile save slot
  g_game_ptr->get_gamefile()->set_next_save_slot();

  storage_mgr::inst()->release_unit( unit );
P*/
}


void vmu_checker_t::check_cur_vmu_status()
{
  if ( syvars->storage_unit_exists )
  {
    bool vmu_removed = !does_vmu_exist(syvars->cur_storage_unit_slot);
    verify_same_vmu();

    // removed/inserted or different?
    if ( (removed[syvars->cur_storage_unit_slot] != vmu_removed) || cur_vmu_different )
    {
      removed[syvars->cur_storage_unit_slot] = vmu_removed;
      do_warning[syvars->cur_storage_unit_slot] = true;
    }
  }
}
