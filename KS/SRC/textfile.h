/*-------------------------------------------------------------------------------------------------------

  TEXTFILE.H - Text file streaming layer on top of binary file I/O

-------------------------------------------------------------------------------------------------------*/
#ifndef TEXTFILE_H
#define TEXTFILE_H

#include "osfile.h"
#include "mustash.h"

class text_file
{
  public:
    text_file(); 
    ~text_file();

    void open(const stringx & name, int _flags=os_file::FILE_READ);
    void close();

    inline const stringx get_name() const { return use_stash ? stringx(the_stash.get_name().c_str()) : io.get_name(); }
    #ifdef TARGET_GC
    inline bool at_eof() { return ( use_stash ? (bufpos>=bufamt && the_stash.at_eof()) : (bufpos>=bufamt && io.at_eof())) ; }
    #else
    inline bool at_eof() const { return use_stash ? (bufpos>=bufamt && the_stash.at_eof()) : (bufpos>=bufamt && io.at_eof()); }
    #endif

    // note: read_char and peek_char don't skip whitespace. everything else does.
    char peek_char();
    char read_char();  

    void read(char* c); // one char, -NOT- a string buffer
    void read(int* i);
    void read(float* f);
#if defined(TARGET_XBOX)
    void read(double* f);
#endif /* TARGET_XBOX JIV DEBUG */

    void read(stringx* s);
    void read(char* s, int maxlen);  // string buffer

		int readln(char* s, unsigned int maxlen, char delimiter = '\n', bool* hitDelimiter = NULL);

    void write(char c);
    void write(int i);
    void write(float f);
    void write(const stringx & s);

    void set_fp( unsigned int pos, os_file::filepos_t base )
    {
      if (use_stash)
        the_stash.set_fp( pos, (stash::filepos_t)base );
      else
        io.set_fp( pos, base );
    }

    bool operator!() const { return use_stash ? !the_stash.is_open() : !io; }

    // Integrating PFE parsing functionality
    int nextchar();
    int nextnonwhite();
    int skipuntilthischar(char c);
    int readuntilthischar(char c,char *buf,int buflen);
    int readuntilwhite(char *buf,int buflen);
    int readuntilnotdigit(char *buf,int buflen);
    int skipuntildigit();
    void keypushback(int c);
    bool is_open() const { return use_stash ? the_stash.is_open() : io.is_open(); }

    static bool text_file_exists(const stringx& name);

  private:
    void eat_whitespace();
    void refill_buf();

    os_file io;
    stash   the_stash;
    int     my_stash;
    char *  buf;
    int     bufpos;
    int     bufamt;
    bool    use_stash;

    // Integrating PFE parsing functionality
    int pushbackdata;
};

#endif
