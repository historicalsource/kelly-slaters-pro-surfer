#ifndef WARNING_H
#define WARNING_H

//P #include "osstorage.h"
//P#include "gamefile.h"
#include "game.h"

/*P
class vmu_callback_t : storage_unit_changed_callback
{
public:
  vmu_callback_t() : storage_unit_changed_callback() {}
  ~vmu_callback_t() {}

  virtual void storage_unit_changed(uint32 drive); // called if a unit was inserted or removed
};
P*/


class vmu_checker_t
{
public:
  vmu_checker_t();
  ~vmu_checker_t() {}
  
  void init();

  void frame_advance( time_value_t time_inc );

  void warn();

  bool does_vmu_exist( int slot );  // sets cur_vmu_removed flag if slot is selected one
  void verify_same_vmu();           // sets cur_vmu_different flag
  void update_optionsfile_time();
  
  void pre_save_vmu_warnings();
  void warning_if_vmu_removed();
  void warning_if_vmu_different();
  void check_vmu_space_and_warn( bool at_save = true ); // sets cur_vmu_full flag and syvars->max_gamefile_slots
  void set_status_check_timer( time_value_t _status_check_timer = 4.0f ) { status_check_timer = _status_check_timer; }
  void check_cur_vmu_status();      // to detect changes after image load

  bool cur_vmu_is_different() const { return cur_vmu_different; }
  bool cur_vmu_was_removed() const { return cur_vmu_removed; }
  bool cur_vmu_is_full() const { return cur_vmu_full; }
  void set_skip_save( bool _skip_save ) { skip_save = _skip_save; }
  bool do_skip_save() const { return skip_save; }
  void set_optionsfile_exists( bool _optionsfile_exists ) { optionsfile_exists = _optionsfile_exists; }

  bool loading_is_valid() const;

  friend vmu_callback_t;

private:
  bool do_warning[MAX_STORAGE_UNITS];
  bool removed[MAX_STORAGE_UNITS];
  bool cur_vmu_different, cur_vmu_removed, cur_vmu_full;
  bool skip_save;
  bool optionsfile_exists;
//P  storage_time optionsfile_time; // for checking whether vmu has been changed before save
  time_value_t status_check_timer;
};


#endif // WARNING_H