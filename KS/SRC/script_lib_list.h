#ifndef _SCRIPT_LIB_LIST_H_
#define _SCRIPT_LIB_LIST_H_

#include "script_library_class.h"
#include "vm_stack.h"
#include "script_lib_entity.h"
#include "algebra.h"

////////////////////////////////////////////////////////////////////////////////
// script library class:  vector3d_list
////////////////////////////////////////////////////////////////////////////////

typedef vector<vector3d> vector3d_list;

class slc_vector3d_list_t : public script_library_class
  {
  public:
    // constructor required
    slc_vector3d_list_t(const char* n,int sz,const char* p=NULL) : script_library_class(n,sz,p) {}
  };

// vm_stack data representation
typedef vector3d_list* vm_vector3d_list_t;

// pointer to single instance of library class
extern slc_vector3d_list_t* slc_vector3d_list;


////////////////////////////////////////////////////////////////////////////////
// script library class:  vector3d_list_iterator
////////////////////////////////////////////////////////////////////////////////

class slc_vector3d_list_iterator_t : public script_library_class
  {
  public:
    // constructor required
    slc_vector3d_list_iterator_t(const char* n,int sz,const char* p=NULL) : script_library_class(n,sz,p) {}
  };

// vm_stack data representation
typedef vector3d_list::iterator vm_vector3d_list_iterator_t;

// pointer to single instance of library class
extern slc_vector3d_list_iterator_t* slc_vector3d_list_iterator;



////////////////////////////////////////////////////////////////////////////////
// script library class:  num_list
////////////////////////////////////////////////////////////////////////////////

typedef vector<vm_num_t> num_list;

class slc_num_list_t : public script_library_class
  {
  public:
    // constructor required
    slc_num_list_t(const char* n,int sz,const char* p=NULL) : script_library_class(n,sz,p) {}
  };

// vm_stack data representation
typedef num_list* vm_num_list_t;

// pointer to single instance of library class
extern slc_num_list_t* slc_num_list;


////////////////////////////////////////////////////////////////////////////////
// script library class:  num_list_iterator
////////////////////////////////////////////////////////////////////////////////

class slc_num_list_iterator_t : public script_library_class
  {
  public:
    // constructor required
    slc_num_list_iterator_t(const char* n,int sz,const char* p=NULL) : script_library_class(n,sz,p) {}
  };

// vm_stack data representation
typedef num_list::iterator vm_num_list_iterator_t;

// pointer to single instance of library class
extern slc_num_list_iterator_t* slc_num_list_iterator;



////////////////////////////////////////////////////////////////////////////////
// script library class:  entity_list
////////////////////////////////////////////////////////////////////////////////

typedef vector<vm_entity_t> entity_list;

class slc_entity_list_t : public script_library_class
  {
  public:
    // constructor required
    slc_entity_list_t(const char* n,int sz,const char* p=NULL) : script_library_class(n,sz,p) {}
  };

// vm_stack data representation
typedef entity_list* vm_entity_list_t;

// pointer to single instance of library class
extern slc_entity_list_t* slc_entity_list;


////////////////////////////////////////////////////////////////////////////////
// script library class:  entity_list_iterator
////////////////////////////////////////////////////////////////////////////////

class slc_entity_list_iterator_t : public script_library_class
  {
  public:
    // constructor required
    slc_entity_list_iterator_t(const char* n,int sz,const char* p=NULL) : script_library_class(n,sz,p) {}
  };

// vm_stack data representation
typedef entity_list::iterator vm_entity_list_iterator_t;

// pointer to single instance of library class
extern slc_entity_list_iterator_t* slc_entity_list_iterator;



////////////////////////////////////////////////////////////////////////////////
// script library class:  str_list
////////////////////////////////////////////////////////////////////////////////

typedef vector<stringx> str_list;

class slc_str_list_t : public script_library_class
  {
  public:
    // constructor required
    slc_str_list_t(const char* n,int sz,const char* p=NULL) : script_library_class(n,sz,p) {}
  };

// vm_stack data representation
typedef str_list* vm_str_list_t;

// pointer to single instance of library class
extern slc_str_list_t* slc_str_list;


////////////////////////////////////////////////////////////////////////////////
// script library class:  str_list_iterator
////////////////////////////////////////////////////////////////////////////////

class slc_str_list_iterator_t : public script_library_class
  {
  public:
    // constructor required
    slc_str_list_iterator_t(const char* n,int sz,const char* p=NULL) : script_library_class(n,sz,p) {}
  };

// vm_stack data representation
typedef str_list::iterator vm_str_list_iterator_t;

// pointer to single instance of library class
extern slc_str_list_iterator_t* slc_str_list_iterator;


#endif