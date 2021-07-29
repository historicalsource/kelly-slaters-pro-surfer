// pstring.h 
//
// Class for a 'packed' string.  Stored internally as a 64-bit integer (only 60-bits are used)
// it can be compared to another 'packed' string quickly.  Limited to 12 characters (excluding
// null terminator) and a 32-character sub-set of the ASCII set, see convert_tbl for the list
// of allowed symbols (all letters, underscore and the numbers 0, 1, 2, 3)
// 
// this header-file is stand-alone (relies on nothing but standard libraries)

#ifndef PACKED_STRING_HEADER
#define PACKED_STRING_HEADER

#include <string.h>

#ifndef ARCH_ENGINE
//#include <assert.h>
#include <stdio.h>
#define error nglPrintf
#endif

#ifdef WIN32
typedef unsigned __int64 uint64;
#elif defined(__GNUC__)
typedef unsigned long uint64;
#endif

#define PSTRING_SHIFT_START 58
#define PSTRING_SHIFT_BY 6
#define PSTRING_PCHAR_MASK 0x3f
#define PSTRING_PCHUNK_NUMBER 4
#define PSTRING_PCHUNK_LENGTH_MASK 0xf
#define PSTRING_PCHUNK_LENGTH_BITS 4
#define PSTRING_PCHUNK_LENGTH 10
#define PSTRING_MAX_LENGTH (PSTRING_PCHUNK_LENGTH * PSTRING_PCHUNK_NUMBER)
#define PSTRING_MAX_LENGTH_PLUS_ONE (PSTRING_MAX_LENGTH + 1)
#define PSTRING_CACHE_SIZE 12

//#ifdef ARCH_ENGINE
class stringx;
//#endif

/*** pstring ***/
class pstring
{
  private:
    static char output_cache[PSTRING_CACHE_SIZE][PSTRING_MAX_LENGTH_PLUS_ONE];
    static unsigned output_index;
    
    /* The 'alphabet' of a pstring (6-bit encoding) */
    static const char pc_to_ascii[64];

    /* assuming standard ascii (7-bit encoding) */
    static const char ascii_to_pc[128];

    /* pstring data itself */
    uint64 pchunk[PSTRING_PCHUNK_NUMBER];

    /*** pack_string ***/
	  void pack_string(const char *the_string);

    /*** unpack_string ***/
    const char * unpack_string() const;

  public:
    /*** constructors/destructors ***/
    pstring() {
      for (unsigned i=0; i<PSTRING_PCHUNK_NUMBER; ++i)
      {
        pchunk[i] = 0;
      }
    }

    pstring (const char *target_string)
    {
	    assert (target_string != NULL);

	    pack_string(target_string);
    }

//#ifdef ARCH_ENGINE
    pstring (const stringx &target_string);
//#endif

    /*** copy constructor ***/
    pstring (const pstring& new_guy)
    {
      assert(new_guy.is_set());
      for (unsigned i=0; i<PSTRING_PCHUNK_NUMBER; ++i)
      {
        pchunk[i] = new_guy.pchunk[i];
      }
    } 

    /*** Assignment operators ***/
    pstring& operator = (const pstring& new_guy)
    {
      if (&new_guy == this) 
        return *this;

      assert(new_guy.is_set());
           
      for (unsigned i=0; i<PSTRING_PCHUNK_NUMBER; ++i)
      {
        pchunk[i] = new_guy.pchunk[i];
      }

      return *this;
    }
    
    pstring& operator = (const char *str) 
    {
      assert(str != NULL);

      pack_string(str);
      return *this;
    }

//#ifdef ARCH_ENGINE
    pstring& operator = (const stringx &str);
//#endif

    /*** Type casts ***/
    operator const char *() const
    {
      return unpack_string();
    }

    /*** c_str ***/
    const char *c_str() const
    {
      // This method does the same thing as the cast overload but in some situations can
      // be more convenient
      return unpack_string();
    }

    /*** Logical operators ***/
    bool operator == (const pstring &check_me) const
    {
      assert(check_me.is_set());

      for (unsigned i=0; i<PSTRING_PCHUNK_NUMBER; ++i)
      {
        if (pchunk[i] != check_me.pchunk[i])
          return false;
      }
      return true;
    }
    bool operator == (const char *check_me) const
    {
      return(*this == pstring(check_me));
    }
//#ifdef ARCH_ENGINE
    bool operator == (const stringx &check_me) const;
//#endif

    bool operator != (const pstring &check_me) const
    {
      assert(check_me.is_set());

      for (unsigned i=0; i<PSTRING_PCHUNK_NUMBER; ++i)
      {
        if (pchunk[i] != check_me.pchunk[i])
          return true;
      }
      return false;
    }
    bool operator != (const char *check_me) const
    {
      return(!(*this == check_me));
    }
//#ifdef ARCH_ENGINE
    bool operator != (const stringx &check_me) const
    {
      return(!(*this == check_me));
    }
//#endif
    bool operator > (const pstring &check_me) const
    {
      assert(check_me.is_set());

      for (unsigned i=0; i<PSTRING_PCHUNK_NUMBER; ++i)
      {
        if (pchunk[i] != check_me.pchunk[i])
          return pchunk[i] > check_me.pchunk[i];
      }
      return false;
    }

    bool operator >= (const pstring &check_me) const
    {
      assert(check_me.is_set());

      for (unsigned i=0; i<PSTRING_PCHUNK_NUMBER; ++i)
      {
        if (pchunk[i] != check_me.pchunk[i])
          return pchunk[i] > check_me.pchunk[i];
      }
      return true;
    }

    bool operator < (const pstring &check_me) const
    {
      assert(check_me.is_set());

      for (unsigned i=0; i<PSTRING_PCHUNK_NUMBER; ++i)
      {
        if (pchunk[i] != check_me.pchunk[i])
          return pchunk[i] < check_me.pchunk[i];
      }
      return false;
    }

    bool operator <= (const pstring &check_me) const
    {
      assert(check_me.is_set());

      for (unsigned i=0; i<PSTRING_PCHUNK_NUMBER; ++i)
      {
        if (pchunk[i] != check_me.pchunk[i])
          return pchunk[i] < check_me.pchunk[i];
      }
      return true;
    }

    /*** is_set() ***/
    bool is_set() const
    {
      return pchunk[0] != (uint64)0;
    }

    /*** length() ***/
    unsigned length () const
    {
      register unsigned check;
      register unsigned ret_val = 0;
      
      for (unsigned i=0; i<PSTRING_PCHUNK_NUMBER; ++i)
      {
        check = unsigned (pchunk[i] & PSTRING_PCHUNK_LENGTH_MASK);
        if (check == 0)
          return ret_val;
        else
          ret_val += check;
      }
      return ret_val;
    }

    bool concatinate(const char *the_string);

    // optimized version, if you are doing multiple crop operations with the same descriptor, generate
    // the mask once with crop_mask() and pass the return value into this version of crop.
    void crop_optimized(uint64 versus)
    {
      uint64 check;
      int shift, len;
      for (int j=PSTRING_PCHUNK_NUMBER; j>0; --j)
      {
        if(pchunk[j] == 0)
          continue;

        check = pchunk[j] ^ versus;

        for (len=0, shift=PSTRING_SHIFT_START; shift>0; shift-=PSTRING_SHIFT_BY, ++len)
        {
          // check for a descriptor 
          if ((check & ((uint64)PSTRING_PCHAR_MASK << shift)) == 0)
          {
            // nuke the rest of this string
            pchunk[j] &= ((uint64)-1) << (shift + PSTRING_SHIFT_BY);
            pchunk[j] |= len;
            for (++j; j<PSTRING_PCHUNK_NUMBER; j++)
              pchunk[j] = 0;
            return;
          }
        }
      }
    }

    // useful to the outside for optimizing multiple crop operations with the same descriptor
    uint64 crop_mask(char ch)
    {
      assert(ch >= 0); // make sure it's a 7-bit ascii value

      int shift, j;
      uint64 pch = ascii_to_pc[ch];
      uint64 versus = 0;
      for (j=0, shift = PSTRING_SHIFT_START; j<PSTRING_PCHUNK_LENGTH; ++j, shift -= PSTRING_SHIFT_BY)
        versus |= uint64 (pch << shift);
      return versus;
    }

    void crop(char ch)
    {
      // crops off the rest of the string after the descriptor is reached (including the descriptor)
      crop_optimized(crop_mask(ch));
    }

};


#endif