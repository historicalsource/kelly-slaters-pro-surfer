#ifndef GC_STORAGE_H
#define GC_STORAGE_H

#include "singleton.h"
#include <card.h>
////////////////////////////////////////////
#define ICON_NAME				"smico.tpl"
#define ICON_SIZE				32
#define ICON_SPEED			CARD_STAT_SPEED_SLOW

#define BANER_WSIZE			96
#define BANER_HSIZE			32

#define COMMENT_SIZE    64
#define COMMENT_1				"SPIDER MAN"
#define COMMENT_2				"Save Game"
/////////////////////////////////////////////

class storage_time
{ 
  public:
    int foo;

  storage_time(int bar) { foo = bar; }
  storage_time() {}
  operator int() const { return foo; }
};

inline bool operator<( const storage_time& a, const storage_time& b )
{
  if ( a.foo < b.foo )
    return true;

  return false;
}

const storage_time default_storage_time = 0;

class storage_icon
{
public:
};

class storage_unit 
{
  friend class storage_mgr;
  
  storage_unit(const char* apath);
  storage_unit(const int  n_slot);

  ~storage_unit();
public:
  uint32 get_block_size();
  uint32 get_total_blocks();
  uint32 get_free_blocks();
  uint32 get_free_bytes() // convenience function
  {
    return get_free_blocks() * get_block_size();
  }
  
  bool ready() const;
  bool sync_to_good_state();

  bool file_exists(const char* filename);
  //
  
  bool unmount_card();
  bool mount_card();
  
  s32   find_file(const char *file_name, CARDStat *f_stat= NULL);
  s32   get_last_error();
  //
  uint32 get_file_size_bytes(const char* filename);
  uint32 get_file_size_blocks(const char* filename);

  bool get_file_name_list(vector<stringx> * namelist, vector<storage_time> *timelist, char *filter);

  bool get_file_info(vector<stringx> * v, vector<void*> * d,
                     vector<storage_time> * t, char *filter = 0,
                     bool do_beep = true, bool do_lcd = true);

  bool get_file_time( const char *filename, storage_time *time );  // doesn't do anything yet
  
  bool save_file(const char* filename,const void* data,uint32 size, 
                 const char* shortdesc=0, const char* longdesc=0,
                 const storage_icon* icon=0,
                 //                 const uint16* pImage=0,
                 //                 const storage_time* time=0,
                 storage_progress_callback* callback=0,
                 storage_time *time_saved=0,
                 bool do_beep=true, bool do_lcd=true);

  uint32 load_file(const char* filename,void* data,uint32 bufsize,
                   storage_progress_callback* callback=0,
                   bool do_beep=true, bool do_lcd=true); // returns actual size in bytes
  bool format();            // format memory card                   
  bool unformat() { return true; };            // unformat memory card                   

  uint32 get_unit() const 
  {
    return 0;
  }
protected:
  stringx 	path;
  
  s32				slot;
	s32 			mounted;
  s32				sector_size;
	s32				card_size;
	u8 				card_workarea[CARD_WORKAREA_SIZE] ATTRIBUTE_ALIGN(32);
	s32 			last_error;
};


// Merely deriving from this is enough to get it to work.  
// You don't have to register it with anything.

class storage_unit_changed_callback
{
  static storage_unit_changed_callback* callback_list;
  storage_unit_changed_callback* next;
public:
  storage_unit_changed_callback();
  virtual ~storage_unit_changed_callback();

  virtual void storage_unit_changed(uint32 drive)=0; // called if a unit was inserted or removed

  static void notify_storage_unit_changed(uint32 drive);
};


#endif // GC_STORAGE_H
