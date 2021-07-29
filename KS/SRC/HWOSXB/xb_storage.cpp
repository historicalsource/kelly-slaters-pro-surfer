// xb_storage.cpp  save game functions for Win32

#include "global.h"

#include "osstorage.h"
//#include <io.h>


static bool storage_init_called = false;   

DEFINE_SINGLETON(storage_mgr)

//---------------------------------------------------------------
storage_unit::storage_unit(const char* apath)
  : path(apath)
{
}

//---------------------------------------------------------------
storage_unit::~storage_unit() 
{
}

//---------------------------------------------------------------
uint32 storage_unit::get_block_size() const
{
  return 0;
}

//---------------------------------------------------------------
uint32 storage_unit::get_total_blocks() const
{
  return 0;
}

//---------------------------------------------------------------
uint32 storage_unit::get_free_blocks() const
{
  return 0;
}

//---------------------------------------------------------------
bool storage_unit::file_exists(const char* filename)
{
  return false;
}


//---------------------------------------------------------------
bool storage_unit::save_file(const char* name, 
                  const void* data, uint32 bytes,
                  const char* shortdesc, const char* longdesc,
                  const storage_icon* icon,
                  storage_progress_callback* callback,  // for progress bar
                  storage_time *time_saved,
                  bool do_beep, bool do_lcd)
{
  return false;
}

//---------------------------------------------------------------
// returns file size if successful, or 0 if error
// call with NULL for data to retrieve max file size, 
// then alloc buffer and call again
//---------------------------------------------------------------
uint32 storage_unit::load_file(const char* name, void* data, uint32 bufsize,
                         storage_progress_callback* callback,
                         bool do_beep, bool do_lcd) 
{
  return 0;
}


//---------------------------------------------------------------
bool storage_unit::get_file_name_list(vector<stringx> * namelist, vector<storage_time> * timelist, char *filter)
{
  return false;
}



bool storage_unit::get_file_info(vector<stringx> * namelist, vector<void*> * datalist,
                                 vector<storage_time> * timelist, char *filter,
                                 bool do_beep, bool do_lcd )
{
  return false;
}


bool storage_unit::get_file_time( const char *filename, storage_time *time )
{
  return false;
}


storage_unit_changed_callback* storage_unit_changed_callback::callback_list;

storage_unit_changed_callback::storage_unit_changed_callback()
{
}

storage_unit_changed_callback::~storage_unit_changed_callback()
{
}

void storage_unit_changed_callback::notify_storage_unit_changed(uint32 drive)
{
}

//---------------------------------------------------------------
storage_mgr::storage_mgr()
{
  assert(!storage_init_called); //  card already initialized
  //debug_print("Initializing backup subsystem");

  storage_init_called=true;
}


//---------------------------------------------------------------
storage_mgr::~storage_mgr() 
{
  assert(storage_init_called);
  debug_print("Shutting down backup subsystem");
  storage_init_called = false;
}

//---------------------------------------------------------------
uint32 storage_mgr::get_max_units() 
{
  return 0;
}

//---------------------------------------------------------------
storage_unit* storage_mgr::get_unit(uint32 slot) 
{
  return 0;
}

//---------------------------------------------------------------
void storage_mgr::release_unit( storage_unit *unit ) 
{
}

//---------------------------------------------------------------
storage_unit* storage_mgr::get_first_valid_unit() 
{
  return 0;
}

//---------------------------------------------------------------
storage_unit* storage_mgr::get_first_valid_unit_with_free_bytes(uint32 bytes) 
{
  return 0;
}

//---------------------------------------------------------------
bool storage_mgr::is_operation_in_progress() const 
{
  return false;
}

