/*-------------------------------------------------------------------------------------------------------

  CHUNKFILE.H - Text or binary file I/O class.

  chunk_file objects are passed down through the serial_in functions as data is loaded.

  High level serial_in functions usually just call low level serial_in functions and therefore don't
  need to know whether the file is text or binary.

  Low level serial_in functions, and occasional others for load time optimization, need to detect
  whether the file is text or binary and use the appropriate text_file or os_file object within the
  chunk_file to read data.

-------------------------------------------------------------------------------------------------------*/
#ifndef CHUNKFILE_H
#define CHUNKFILE_H

#include "osfile.h"
#include "textfile.h"
#include "filespec.h"
#include "mustash.h"

class chunk_file
{
  public:
    static bool noquotes;

    enum chunk_file_flags_t
    {
      MODE_MASK   = 0x00FF,  // os_file mode is in low byte
      FILE_TEXT   = 0x0100,  // flag to force text mode
      FILE_BINARY = 0x0200,  // flag to force binary mode
    };
    enum chunk_file_t
    {
      CFT_NONE,
      CFT_TEXT,
      CFT_BINARY
    };

    chunk_file();

    void open(const stringx & name, int flags=os_file::FILE_READ);
    void close();

    bool operator!() const;

    void set_fp( unsigned int pos, os_file::filepos_t base );

    bool at_eof();

    stringx get_name();
    stringx get_filename();
    stringx get_dir();
    int     get_my_stash() { return my_stash; }
    inline chunk_file_t  get_type()    { return type; }
    inline os_file *     get_binary()  { assert(type==CFT_BINARY); return &binary; }
    inline text_file *   get_text()    { assert(type==CFT_TEXT);   return &text; }
    inline stash *       get_stash()   { assert(type==CFT_BINARY && the_stash.is_open()); return &the_stash; }

    bool          use_stash;

		int get_size( void );
		bool read( void *buf, int bytes );


  private:
    chunk_file_t  type;

    // only one of these can be active at a time.
    os_file       binary;
    text_file     text;
    stash         the_stash;
    int           my_stash;

  public:
    void set_type(chunk_file_t _type) { type=_type; }  // for CF_WRITE_BINARY mode to fake out serial_out.
};


class chunk_flavor
{
private:
  char flavor[16];
public:
  enum { CHUNK_FLAVOR_SIZE=16 };
  chunk_flavor(const char* s="UNREG") { assert( strlen( s )<CHUNK_FLAVOR_SIZE ); strcpy( flavor, s );}

  bool operator==( const chunk_flavor& cf ) const;
	#ifndef TARGET_PS2
  bool operator!=( const chunk_flavor& cf ) const;
	#endif
  const char* c_str( void ) const;
  stringx to_stringx( void ) const;

#if !defined(NO_SERIAL_IN)
  friend void serial_in(chunk_file& io,chunk_flavor* d);
#endif
#if !defined(NO_SERIAL_OUT)
  friend void serial_out(chunk_file& io,const chunk_flavor& d);
#endif
};


#if !defined(NO_SERIAL_IN)
/*-------------------------------------------------------------------------------------------------------
  basic serial_in's
-------------------------------------------------------------------------------------------------------*/
void serial_in(chunk_file& io,chunk_flavor* d);
void serial_in(chunk_file& io, int* d);
void serial_in(chunk_file& io, unsigned short* d);
void serial_in(chunk_file& io, unsigned int* d);
void serial_in(chunk_file& io, bool* d);
void serial_in(chunk_file& io, float* d);
void serial_in(chunk_file& io, double* d);
void serial_in(chunk_file& io, stringx* d);
#endif

#if !defined(NO_SERIAL_OUT)
/*-------------------------------------------------------------------------------------------------------
  basic serial_out's
-------------------------------------------------------------------------------------------------------*/
void serial_out(chunk_file& io,const chunk_flavor& d);
void serial_out(chunk_file& io,const int& d);
void serial_out(chunk_file& io,const short& d);
void serial_out(chunk_file& io,const unsigned short& d);
void serial_out(chunk_file& io,const unsigned int& d);
void serial_out(chunk_file& io,const float& d);
void serial_out(chunk_file& io,const double& d);
void serial_out(chunk_file& io,const stringx& d);
#endif


/*-------------------------------------------------------------------------------------------------------
  standard chunks
-------------------------------------------------------------------------------------------------------*/
extern const chunk_flavor CHUNK_EOF;
extern const chunk_flavor CHUNK_END;    // only flexible chunks need the chunkend tag.
extern const chunk_flavor CHUNK_SPF;    // seconds-per-frame for animatable files (.tam & .ban)
extern const chunk_flavor CHUNK_SCALE;  // scale factor for meshes or whatever else.

extern const stringx chunkend_label;

#endif
