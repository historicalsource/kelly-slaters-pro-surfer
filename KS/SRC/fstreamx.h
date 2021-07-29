#ifndef _FSTREAMX_H
#define _FSTREAMX_H
/*-------------------------------------------------------------------------------------------------------

  FSTREAMX.H - Custom file handling classes

-------------------------------------------------------------------------------------------------------*/
#include "stringx.h"

#if !defined(NO_FSTREAM)

////////////////////////////////////////////////////////////////////////////////
//  These lines are BAD.  See stringx.h for why.  Don't reinclude them.
//
//  #include <fstream>
//  using namespace std;
////////////////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------------------------------------------

  FSTREAMX - Custom file stream.  
  
  This class will be the cross platform way to read files.  It will also support transparently 
  finding and reading files from library files.

  WARNING: Do not cast this class to any of its bases and try to use it.  It will break.

-------------------------------------------------------------------------------------------------------*/
class fstreamx : public fstream
  {
  public:
    fstreamx() {}
    fstreamx( const stringx& _fname, int mode );
    const stringx& get_fname() const {return fname;}  // includes directory portion
    stringx get_dir() const;
    void open( const stringx& _fname, int mode );  

    // the root_dir is prepended to all filename requests
    static void    set_root_dir( const stringx& _root_dir );
    static const stringx& get_root_dir() { return *proot_dir;}
    static void    set_pre_root_dir( const stringx& _pre_root_dir );
    static const stringx& get_pre_root_dir() { return *pre_root_dir;}
    static bool    file_exists( const stringx& filename );
  private:
    stringx fname;
    static stringx *proot_dir;
    static stringx *pre_root_dir;
    void check_static_root( void );
  };


/*-------------------------------------------------------------------------------------------------------

  ASYNC_FSTREAMX - Asynchronous reading facilities

  This class provides a way to get file data without pausing for the data to be available.  It supports
  all the features of fstreamx.

  Actually, right now it it's just a version of fstreamx with a different interface.
  
  Note that right now there's no checking implemented, so you can read right past the end
  of the stream w/o any problems.  Also, you can do reads on an unopened file.

  I may end up implementing this through stream_buf later, if there are problems.

  WARNING: Do not cast this class to any of its bases and try to use it.  It will break.

-------------------------------------------------------------------------------------------------------*/
class async_fstreamx : public fstreamx
  {
  public:
    // No file name is taken, since objects of this class are designed to be recycled.
    async_fstreamx() {}
    
    inline bool is_data_ready() const { return data_ready; }

    // If len = 0, the whole file is read.
    inline void start_reading( const char * fname, int pos = 0, int len = 0 )
      {
      fstreamx::open( stringx(fname), ios::in );
      seekg( pos );
      data_ready = true;
      }

  private:
    bool data_ready;

    // Note: some more members of fstreamx & fstream will probably need to be private.
    void open(const char *s, int mode = ios::in | ios::out);  
    //                        ^  made it int instead of ios::open_mode for fstream.h compatibility
  };

#else
// if NO_FSTREAM is set, automatically set NO_SERIAL_OUT.
#if 0
#ifndef NO_SERIAL_OUT
#define NO_SERIAL_OUT
#endif
#endif
#endif

#endif  // _FSTREAMX_H
