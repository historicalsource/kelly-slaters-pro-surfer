#ifndef STRINGX_H
#define STRINGX_H

////////////////////////////////////////////////////////////////////////////////
//
//  New version of stringx, to reduce memory allocation and fragging
//

//  Here's a handy guide to using stringx for C programmers:
//
//  Old function          New function
//   strnew                ctor
//   strdup/strcpy         copy ctor, assignment
//   sprintf               + operator, itos() (strstream is gone)
//   strstr/strchr         find()
//   strrchr               rfind()
//   strupr                to_upper()
//   strlwr                to_lower()

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#if !defined(JOHNS_UNIT_TEST)

#ifdef _XBOX
#include "xbglobals.h"
#endif /* _XBOX */

#ifndef W32_STASH_BROWSER
#if !defined(TARGET_XBOX)
#include "global.h"
#endif /* TARGET_XBOX JIV DEBUG */
#endif

#if defined(TARGET_PS2)
typedef uint64 big_int;
#elif defined(TARGET_PC) || defined(TARGET_XBOX)
typedef uint64 big_int;
#elif defined(TARGET_GC)
typedef uint64 big_int;
#elif defined(W32_STASH_BROWSER)
#include <assert.h>
#include <iostream.h>
#include <math.h>
#include <stdlib.h>
typedef unsigned long int big_int;
#else
#endif

#define BIG_INT_SIZE sizeof(big_int)

#else // unit test

//#include <assert.h>	// May override our own assert.  (dc 06/14/02)
#include <iostream.h>
typedef unsigned long long big_int;
#define debug_print printf
#define error printf

#endif

class string_buf
{
protected:  
  friend class stringx;
  friend stringx operator+( const stringx& lhs, const stringx& rhs );
  friend stringx operator+( const char* lhs, const stringx& rhs );
  friend stringx operator+( const stringx& lhs, const char* rhs );

  friend inline bool operator==(const stringx& lhs, const stringx& rhs );
  friend inline bool operator!=( const stringx& lhs, const stringx& rhs );
  friend inline bool operator<( const stringx& lhs, const stringx& rhs );

  big_int *data;
  int ref_count;
  int char_length;
  int block_length;
  int max_blocks;

  inline int blocks_needed(int chars)
  {
    chars++; //leave room for \0
    if (chars % sizeof (big_int) != 0)
      return (chars / sizeof (big_int) + 1);
    else
      return (chars / sizeof (big_int));
  }

public:

  void set_buf(big_int *buf, int blocks)
  {
    data = buf;
    max_blocks = blocks;
    ref_count = 0;
    char_length = 0;
    block_length = blocks_needed(char_length);
    data[0] = 0;
  };

  // does not touch refcounts
  inline void copy_data_from(string_buf *buf)
  {
    assert(buf->data != NULL);
    assert(data != NULL);
    assert(buf->block_length <= max_blocks);
    block_length = buf->block_length;
    char_length = buf->char_length;
    memcpy(data, buf->data, block_length * sizeof (big_int));
  }

  inline big_int *get_data()
  {
    assert(data != NULL);
    return data;
  }

  inline void inc_ref()
  {
    ref_count++;
  }

  inline void dec_ref()
  {
    ref_count--;
    assert(ref_count >= 0);
  }

  inline int get_ref()
  {
    return ref_count;
  }

  inline void set_ref(int r)
  {
    ref_count = r;
  }

  void clear();

  inline void set_to_cstr(const char *str, int len = -1)
  {
    assert(data != NULL);
    if (len == -1) len = strlen(str);
    assert(len < max_blocks * (int)sizeof(big_int));
    char_length = len;
    block_length = blocks_needed(len);
    memcpy(data, str, len);

    // zero out the rest of the string
    memset((char *)data + len, 0, max_blocks * sizeof (big_int) - len);
  }

  inline int compare(const char *str) const
  {
    assert(str != NULL);
    assert(data != NULL);
    int i;

    const char *a = reinterpret_cast<const char *>(data);;
    for (i = 0; i < char_length; i++) {
      if (str[i] == '\0') return -1;
      if (a[i] == str[i]) continue;
      if (str[i] > a[i]) return 1;
      if (str[i] < a[i]) return -1;
    }
    if (str[i] == '\0') return 0; // same data and length
    return 1; // str is longer
  }
  
//#pragma todo("Optimize to use bigint comparisons. Watch for endianness issues.")

  inline int compare(const string_buf &buf) const
  {
    assert(buf.data != NULL);
    assert(data != NULL);
    int i;
    const char *a = reinterpret_cast<const char *>(data);
    const char *b = reinterpret_cast<const char *>(buf.data);
    int cmp_len = (char_length < buf.char_length ? char_length : buf.char_length);
    for (i = 0; i < cmp_len; i++) {
      // the compiler should be able to do this with a single test
      if (a[i] == b[i]) continue;
      if (b[i] > a[i]) return 1;
      if (b[i] < a[i]) return -1;
    }
    if (buf.char_length > char_length) return 1; // shorter strings first
    if (buf.char_length < char_length) return -1;
    return 0;
  }

  // Fast equality test. Uses big_int comparisons.
  inline bool is_equal(const string_buf &buf) const
  {
    assert(buf.data != NULL);
    assert(data != NULL);
    if (buf.block_length != block_length) return false;
    for (int i = 0; i < block_length; i++) {
      if (!(buf.data[i] == data[i])) return false;
    }
    return true;
  }

  inline bool is_equal(const char *str) const
  {
    // can't use fast test here because we don't know the alignment
    // or surroundings of str
    return (compare(str) == 0);
  }

  // Sets the length of the string. Pads leftover space with \0.
  inline void set_char_length(int len)
  {
    assert(len <= max_blocks * (int)sizeof (big_int));
    char_length = len;
    block_length = blocks_needed(len);
    memset((char *)data + len, 0, max_blocks * sizeof (big_int) - len);
  }

  inline int get_char_capacity() const
  {
    return max_blocks * sizeof (big_int) - 1;
  }

  void null_terminate() const;

};


class stringx
{
  friend class string_buf;

  protected:
    static string_buf strings[];
    static big_int string_pool[];
    static bool stringx_initialized;
    static string_buf *free_small_buffers[];   static unsigned int free_small_buffers_end;
    static string_buf *free_medium_buffers[];  static unsigned int free_medium_buffers_end;
    static string_buf *free_long_buffers[];    static unsigned int free_long_buffers_end;
    static string_buf *buf_cache[];            static unsigned int buf_cache_lru[256];

    string_buf *my_buf;

    // Checks whether a buffer is part of the strings[] array.
    bool is_buffer_mine(string_buf *buf) const;

    // Locates a buffer suitable for the given string.
    // Does not increment the reference count; do this yourself.
    static string_buf *find_empty_buffer(int capacity, const char* str_just_for_error_msgs );
    static string_buf *find_small_buffer();
    static string_buf *find_medium_buffer();
    static string_buf *find_large_buffer();
    
    // Locates a buffer in the cache, and increments the refcount.
    static string_buf *find_cached_string(const char *str, int len = -1);

    // Gets a buffer for a string and copies the data into it.
    // Searches the cache first, then falls back on find_empty_buffer.
    // Increments the reference count.
    static string_buf *acquire_buffer(const char *str, int len = -1);

    // Silently refuses to cache strings for which is_buffer_mine() returns false.
    static void add_buf_to_cache(string_buf *buf);

    // Decrements the refcount for my_buf, and sets my_buf to NULL.
    void release_buffer();

    // If another copy of the current buffer exists in the cache, frees the current
    // buffer and points at that one instead.
    bool aggressively_cache_buffer();

    char *chars;

  public:

    static int npos;
    typedef int size_type;  // why the heck this was ever done is beyond me, but here it is, for compatibility

    stringx();
    stringx(const stringx &cp);
    stringx(const char *str, int len = -1);
    explicit stringx(float f);
    explicit stringx(int i);
    explicit stringx(unsigned int i);
    enum fmtd { fmt }; stringx(fmtd, const char *fmtp, ...); // i.e. stringx(stringx::fmt, "%d", 5);

    ~stringx();

    static void debug_dump_strings();

    // Allocates static buffers.
    // Call as early as possible in app init.
    static void init();

    // Attempts to locate a buffer that already contains the given text.
    // Note that this necessitates searching all buffers. While this is
    // fairly quick, do not do this in performance-critical cases.
//    string_buf *find_shareable_buffer(const char *text);

    // Copying and appending
    void copy(const char *str, int len = -1);
    void copy(stringx &cp);
    void append(const char *str, int len = -1);
    void append(const stringx &cp);
    inline void copy(char ch) { char buf[2]; buf[0] = ch; buf[1] = '\0'; copy(buf, 1); };
    inline void append(char ch) { char buf[2]; buf[0] = ch; buf[1] = '\0'; append(buf, 1); };

    // Operators
    stringx &operator=(const stringx &cp);
    stringx &operator=(const char *str);
    inline stringx &operator=(char ch) { copy(ch); return *this; };
    stringx &operator+=(const stringx &cp);
    stringx &operator+=(const char *str);
    inline stringx &operator+=(char ch) { append(ch); return *this; };

    // Utilities for lexing
    inline void remove_surrounding_whitespace() { remove_leading(" \n\t\r"); remove_trailing(" \n\t\r"); };
    void remove_leading(char *remove);
    void remove_trailing(char *remove);

    // Returns a portion of the string starting from start_index and up to but
    // not including any chars in delim.
    // For instance, read_token(" ", 0, true, true) on the stringx
    // "  hello world! " would return "hello" and set the string to " world! ".
    // If ignore_leading is true, leading characters matching delim will be ignored.
    // If chop is true, destructively removes the token from the string.
    stringx read_token(char *delim, int start_index = 0, bool ignore_leading = true, bool chop = false);

    // These are not guaranteed to account for extraneous characters. Trim the string first.
    float to_float() const { return (float)atof(chars); }
    int to_int() const { return atoi(chars); }

    // This has a certain amount of overhead. If you want faster access,
    // call lock() and get a direct pointer with c_str().
    inline char &operator[](int i)
    {
      lock();
      assert(my_buf);
      if (i < 0)
        i += length();  // Python-like negative indexing
      assert(i <= my_buf->char_length);
      return chars[i];
    }

    inline const char operator[](int i) const
    {
      assert(my_buf);
      if (i < 0)
        i += length();  // Python-like negative indexing
      assert(i <= my_buf->char_length);
      return chars[i];
    }


    // Call this before directly accessing the string's data.
    inline void lock()
    {
      assert(my_buf);
      if (my_buf->get_ref() > 1)
        fork_data();
    }
      
    // Decrements the refcount of the current data, finds a new buffer
    // that can accomodate new_len characters, and copies the data into
    // it. Increments the refcount of the new data.
    void fork_data(int new_len = -1);

    // Truncates the string to the given length.
    // Pads leftover space with nulls.
    void truncate(int new_len);

    // Returns a pointer to the string
    inline const char *c_str() const
    {
      assert(chars);
      return chars;
    }

    inline const char *data() const
    {
      assert(chars);
      return chars;
    }

    inline int size() const
    {
      assert(chars);
      return my_buf->char_length;
    }

    inline int length() const
    {
      return size();
    }

    inline bool empty() const
    {
      assert(chars);
      return (chars[0] == '\0');
    }

    // Makes sure the buffer can accomodate size characters.
    // If not, forks the buffer to one that can.
    void make_room(int size);

    void to_upper();
    void to_lower();
    stringx substr(int i = 0, int n = -1) const;
    
    inline stringx slice(int start, int end) const
    // Similar to the Python slice operator.
    // Returns a substring from start up to but not including end.
    // stringx foo("foobar"); foo.slice(2,4) would return "ob".
    // Negative indices are counted from the end of the string.
    {
      if (start < 0)
        start += length();
      if (end < 0)
        end += length();
      assert(start <= end);
      return substr(start, end-start);
    };

    int find(const char *s) const; 
    int find(int pos, char c) const;
    int rfind(char c) const;
    int rfind(char c, int pos) const;
    int rfind(const char *str) const;

    friend stringx operator+( const stringx& lhs, const stringx& rhs );
    friend stringx operator+( const char* lhs, const stringx& rhs );
    friend stringx operator+( const stringx& lhs, const char* rhs );

    friend inline bool operator==(const stringx& lhs, const stringx& rhs );
    friend inline bool operator==(const stringx& lhs, const char *rhs );
    friend inline bool operator!=( const stringx& lhs, const stringx& rhs );
    friend inline bool operator!=(const stringx& lhs, const char *rhs );
    friend inline bool operator<( const stringx& lhs, const stringx& rhs );

};


inline stringx ftos(float f)
{
  return stringx(f);
}


inline stringx itos(int i)
{
  return stringx(i);
}


inline bool operator==(const stringx& lhs, const stringx& rhs)
{
  return lhs.my_buf->is_equal(*rhs.my_buf);
}


inline bool operator==(const stringx& lhs, const char *rhs)
{
  return lhs.my_buf->is_equal(rhs);
}


inline bool operator!=(const stringx& lhs, const stringx& rhs)
{
  return !(lhs == rhs);
}


inline bool operator!=(const stringx& lhs, const char *rhs )
{
  return !lhs.my_buf->is_equal(rhs);
}


inline bool operator<( const stringx& lhs, const stringx& rhs )
{
  return (lhs.my_buf->compare(*rhs.my_buf) == 1);
}


inline stringx operator+( const stringx& lhs, const stringx& rhs )
{
  stringx foo;

  foo.make_room(lhs.my_buf->char_length + rhs.my_buf->char_length);
  foo.append(lhs);
  foo.append(rhs);

  return foo;
}

inline stringx operator+( const char* lhs, const stringx& rhs )
{
  stringx foo;
  int len = strlen(lhs);

  foo.make_room(len + rhs.my_buf->char_length);
  foo.append(lhs, len);
  foo.append(rhs);

  return foo;
}

inline stringx operator+( const stringx& lhs, const char* rhs )
{
  stringx foo;
  int len = strlen(rhs);

  foo.make_room(len + lhs.my_buf->char_length);
  foo.append(lhs);
  foo.append(rhs, len);

  return foo;
}

inline stringx get_dir( const stringx& path )
{
  stringx retval;
  // everything from the colon if there is one to the last slash
  stringx::size_type colon;
  stringx::size_type last_slash;
  colon = path.find(":");
  if( colon == stringx::npos )
    colon = 0;
  last_slash = path.rfind('\\');
  assert( colon != stringx::npos );
  retval = path.substr( colon, last_slash+1-colon );
  return retval;
}

inline stringx get_fname_wo_ext( const stringx& path )
{
  stringx retval;
  // everything from the last slash to the dot
  stringx::size_type last_slash;
  stringx::size_type dot;
  last_slash = path.rfind('\\');
  // side note:  if there is no slash, last_slash = npos which is -1, so when you add one to it
  // you get the beginning of the string, conveniently, which is what we want.
  dot = path.rfind('.');
  if( dot == stringx::npos )
    dot = path.size();
  retval = path.substr( last_slash+1, dot-(last_slash+1) );
  return retval;
}

inline stringx get_ext( const stringx& path )
{
  stringx retval;
  stringx::size_type dot;
  dot = path.rfind('.');
  if( dot == stringx::npos )
    return retval;
  retval = path.substr( dot+1, stringx::npos );
  return retval;
}



//#define empty_string (stringx(""))
//#define sendl (stringx("\n"))
extern stringx empty_string;
extern stringx sendl;

#endif
