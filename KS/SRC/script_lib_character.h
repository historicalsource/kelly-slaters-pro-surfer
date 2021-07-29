// script_lib_character.h
#ifndef _SCRIPT_LIB_CHARACTER_H
#define _SCRIPT_LIB_CHARACTER_H


#include "script_library_class.h"
//!#include "char_group.h"


//!class character;


///////////////////////////////////////////////////////////////////////////////
// script library class: character
///////////////////////////////////////////////////////////////////////////////

class slc_character_t : public script_library_class
  {
  public:
    // constructor required
    slc_character_t(const char* n,int sz,const char* p=NULL) : script_library_class(n,sz,p) {}
    // read a character value (by id) from a stream
    virtual void read_value(chunk_file& fs,char* buf);
    // find named instance of character
    virtual unsigned find_instance(const stringx& n) const;
  };

// vm_stack data representation
//!typedef character* vm_character_t;

// pointer to single instance of library class
extern slc_character_t* slc_character;


///////////////////////////////////////////////////////////////////////////////
// script library class: char_group
///////////////////////////////////////////////////////////////////////////////

class slc_char_group_t : public script_library_class
  {
  public:
    // constructor required
    slc_char_group_t(const char* n,int sz,const char* p=NULL) : script_library_class(n,sz,p) {}
    // read a char_group value (by id) from a stream
    virtual void read_value(chunk_file& fs,char* buf);
    // find named instance of char_group
    virtual unsigned find_instance(const stringx& n) const;
  };

// vm_stack data representation
typedef char_group* vm_char_group_t;

// pointer to single instance of library class
extern slc_char_group_t* slc_char_group;


////////////////////////////////////////////////////////////////////////////////
// script library class:  char_group_iterator
////////////////////////////////////////////////////////////////////////////////

class slc_char_group_iterator_t : public script_library_class
  {
  public:
    // constructor required
    slc_char_group_iterator_t(const char* n,int sz,const char* p=NULL) : script_library_class(n,sz,p) {}
  };

// vm_stack data representation
typedef char_group::iterator vm_char_group_iterator_t;

// pointer to single instance of library class
extern slc_char_group_iterator_t* slc_char_group_iterator;


#endif  // _SCRIPT_LIB_CHARACTER_H
