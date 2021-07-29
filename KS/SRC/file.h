/*-------------------------------------------------------------------------------------------------------

  FILE.H - Text or binary file I/O class.

  app_file objects are passed down through the serial_in functions as data is loaded.

  High level serial_in functions usually just call low level serial_in functions and therefore don't
  need to know whether the file is text or binary.

  Low level serial_in functions, and occasional others for load time optimization, need to detect
  whether the file is text or binary and use the appropriate text_file or os_file object within the
  app_file to read data.

-------------------------------------------------------------------------------------------------------*/

#ifndef FILE_H
#define FILE_H

#include "osfile.h"
#include "textfile.h"
#include "filespec.h"
#include "mustash.h"

class app_file
{
  public:
    static bool noquotes;

    enum app_file_flags_t
    {
      MODE_MASK   = 0x00FF,  // os_file mode is in low byte
      FILE_TEXT   = 0x0100,  // flag to force text mode
      FILE_BINARY = 0x0200,  // flag to force binary mode
    };
    enum app_file_t
    {
      CFT_NONE,
      CFT_TEXT,
      CFT_BINARY
    };

    app_file();

    //void open(const stringx & name, int flags=os_file::FILE_READ);
    void open(const char *name, int flags=os_file::FILE_READ);
    void close();

    bool operator!() const;

    void set_fp( unsigned int pos, os_file::filepos_t base );

    bool at_eof();

    stringx get_name();
    stringx get_filename();
    stringx get_dir();

    inline app_file_t  get_type()    { return type; }
    inline os_file *     get_binary()  { assert(type==CFT_BINARY); return &binary; }
    inline text_file *   get_text()    { assert(type==CFT_TEXT);   return &text; }
    inline stash *       get_stash()   { assert(type==CFT_BINARY && the_stash.is_open()); return &the_stash; }

    bool          use_stash;

		int get_size( void );
		bool read( void *buf, int bytes );

    //bool get_memory_image(const pstring& _name, unsigned char *&buf,
    bool get_memory_image(const char * _name, unsigned char *&buf,
                                 unsigned int &buf_size, stash_index_entry *&hdr, int alignment=1 );


  private:
    app_file_t  type;

    // only one of these can be active at a time.
    os_file       binary;
    text_file     text;
    stash         the_stash;

  public:
    void set_type(app_file_t _type) { type=_type; }  // for CF_WRITE_BINARY mode to fake out serial_out.
};



#endif
