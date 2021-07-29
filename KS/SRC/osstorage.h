#ifndef OSSTORAGE_H
#define OSSTORAGE_H
/*-------------------------------------------------------------------------------------------------------
  OSSTORAGE.H - Include file redirect for file module.
-------------------------------------------------------------------------------------------------------*/
#include "singleton.h"


class storage_unit;

enum {
    STORAGE_OK               =   0,
    STORAGE_GENERIC          = - 1,
    STORAGE_UNFORMATTED      = - 2,
    STORAGE_FULL             = - 3,
    STORAGE_NO_OBJECT_ACCESS = - 4,   // this includes file/path not found, file already exists,
                                      // file was not open
    STORAGE_NO_ACCESS_RIGHT  = - 5,   // file can't be opened with RD, WR or without both, or
                                      // read or write doesn't match open attribs.
    STORAGE_WRONG_MEDIA      = - 6,   // for the cases of wrong memcard formats (PocketStation & 128K)
    STORAGE_2MANY_FILES      = - 7,   // too many files open
    STORAGE_BUSY             = - 8,   // storage operation is in progress, try later
    STORAGE_NO_MEDIA         = -10    // -10 or less means no media detected

};



//---------------------------------------------------------------
class storage_mgr : public singleton
{
  storage_mgr();
  ~storage_mgr();

  DECLARE_SINGLETON(storage_mgr)
public:
  uint32 get_max_units(); // how many slots there can be units in
  storage_unit* get_unit(uint32 slot); // returns null if there's not a unit present
  void release_unit( storage_unit *unit );
  storage_unit* get_first_valid_unit();
  storage_unit* get_first_valid_unit_with_free_bytes(uint32 bytes);
  bool is_operation_in_progress() const; // is a save or load occurring?
};


//---------------------------------------------------------------
class storage_progress_callback
{
public:
  virtual void storage_progress(uint32 count, uint32 max)=0;
};


//---------------------------------------------------------------


#if defined(TARGET_PC)
#include "hwospc\w32_storage.h"
#elif defined(TARGET_MKS)
#include "hwosmks\vmu_storage.h"
#elif defined(TARGET_PS2)
#include "hwosps2\ps2_storage.h"
#elif defined(TARGET_NULL)
#include "hwosnull\null_storage.h"
#elif defined(TARGET_XBOX)
#include "hwosxb/xb_storage.h"
#elif defined(TARGET_GC)
#include "hwosgc\gc_storage.h"
#endif

#endif  // OSSTORAGE_H
