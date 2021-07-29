#ifndef INSTANCE_H
#define INSTANCE_H

#include <set>
#include "debug.h"
#include "osalloc.h"

// This template is for creating an instance bank of objects that are constructed
// based on a passed stringx.  This stringx will commonly be interpereted by the class
// constructor as a filename, though this is not necessary.  Current examples are
// pmesh and texture instance_banks.

template <class T>
class instance_bank
{
// Types
public:
  struct ref_t
  {
    stringx label;
    T* ptr;
    unsigned int count;
    // partial constructors supplied to support set::find()
    ref_t( const stringx& _label ) : label(_label), ptr(NULL), count(0) {}
    ref_t( T* _ptr ) : label(), ptr(_ptr), count(0) {}
    // normal constructor
    ref_t( const stringx& _label, T* _ptr ) : label(_label), ptr(_ptr), count(1) {}
  };
  class less_by_label : public binary_function<const ref_t*,const ref_t*,bool>
  {
  public:
    bool operator()(const ref_t* a,const ref_t* b) const
    {
      return ( a->label < b->label );
    }
  };
  class less_by_ptr : public binary_function<const ref_t*,const ref_t*,bool>
  {
  public:
    bool operator()(const ref_t* a,const ref_t* b) const
    {
      return ( a->ptr < b->ptr );
    }
  };
  #ifdef TARGET_PS2
  typedef set<ref_t*,less_by_label,malloc_alloc> lref_set;
  typedef set<ref_t*,less_by_ptr,malloc_alloc> pref_set;
  #else
  typedef set<ref_t*,less_by_label> lref_set;
  typedef set<ref_t*,less_by_ptr> pref_set;
  #endif

// Data
public:
  lref_set refs_by_label;
  pref_set refs_by_ptr;

// Methods
public:
  instance_bank();
  ~instance_bank();

  T* new_instance( chunk_file& fs, const stringx& label, unsigned additional_flags );
  T* new_instance( const stringx& filename, unsigned additional_flags );
  T* new_instance( const T &copy );
  T* new_instance( T* ptr );

  // insert given object into instance bank with given label
  // NOTE: this only works if the label is unique
  void insert_new_object( T* ptr, const stringx& label );

  // returns NULL if not found
  T* find_instance_by_filename( const stringx& filename );
  T* find_instance( const stringx& label );

  void delete_instance(T * ptr);

  void purge();

  void debug_dump();

public:
  void insert( const stringx& label, T* ptr )
  {
    ref_t* r = NEW ref_t( label, ptr );
    refs_by_label.insert( r );
    refs_by_ptr.insert( r );
  }
};


template <class T>
instance_bank<T>::instance_bank()
{
}


template <class T>
instance_bank<T>::~instance_bank()
{
  lref_set::iterator i;
  for ( i=refs_by_label.begin(); i!=refs_by_label.end(); ++i )
  {
    ref_t* r = *i;
    assert( r->count );
    debug_print( "WARNING: Unreleased instance." );
    delete r;
  }
}


// This method is not particularly desirable, since it forces you to build the
// object before searching for it in the instance bank.  Since it is only used
// by the kludgey materials and since we intend to fix those in the near future,
// this method should soon go away.
template <class T>
T* instance_bank<T>::new_instance( const T& copy )
{
  ref_t copy_name_ref(copy.get_instance_name());
  lref_set::iterator i = refs_by_label.find( &copy_name_ref );
  if ( i == refs_by_label.end() )
  {
    // matching label not found; make NEW copy
    T* new_T = NEW T( copy );
    insert( copy.get_instance_name(), new_T );
    return new_T;
  }
  else
  {
    // matching label found; increment reference
    ++(*i)->count;
    return (*i)->ptr;
  }
}


template <class T>
T* instance_bank<T>::new_instance( chunk_file& fs, const stringx& label, unsigned additional_flags )
{
  ref_t label_ref(label);
  lref_set::iterator i = refs_by_label.find( &label_ref );
  if ( i == refs_by_label.end() )
  {
    // matching label not found; load NEW instance from file
    T* new_T = NEW T( fs, additional_flags );
    insert( label, new_T ); // note that the node goes on the dynamic heap, to preserve instance counts.
    return new_T;
  }
  else
  {
    // matching label found; skip file data and increment reference count
    T dummy( fs, 0 );
    ++(*i)->count;
    return (*i)->ptr;
  }
}


template <class T>
T* instance_bank<T>::new_instance( const stringx& filename, unsigned additional_flags )
{
  // we will use the file basename as the instance label
  filespec fspec( filename );
  ref_t fspec_ref(fspec.name);
  lref_set::iterator i = refs_by_label.find( &fspec_ref );
  if ( i == refs_by_label.end() )
  {
    // matching label not found; load NEW instance from file
    T* new_T = NEW T( filename.data(), additional_flags );
    insert( fspec.name, new_T );
    return new_T;
  }
  else
  {
    // matching label found; increment reference count
    ++(*i)->count;
    return (*i)->ptr;
  }
}


template <class T>
T* instance_bank<T>::new_instance( T* ptr )
{
  ref_t ptr_ref(ptr);
  pref_set::iterator i = refs_by_ptr.find( &ptr_ref );
  assert( i != refs_by_ptr.end() );
  ++((*i)->count);
  return ptr;
}


template <class T>
void instance_bank<T>::insert_new_object( T* ptr, const stringx& label )
{
  ref_t label_ref = label;
#ifdef DEBUG
  lref_set::iterator i = refs_by_label.find( &label_ref );
  assert( i == refs_by_label.end() );
#endif
  insert( label, ptr );
}


template <class T>
T* instance_bank<T>::find_instance_by_filename( const stringx& filename )
{
  // we will use the file basename as the instance label
  filespec fspec( filename );
  ref_t tmpref(fspec.name);
  lref_set::iterator i = refs_by_label.find( &tmpref );
  if ( i == refs_by_label.end() )
    return NULL;
  return (*i)->ptr;
}


template <class T>
T* instance_bank<T>::find_instance( const stringx& label )
{
  ref_t label_ref = label;
  lref_set::iterator i = refs_by_label.find( &label_ref );
  if ( i == refs_by_label.end() )
    return NULL;
  return (*i)->ptr;
}


template <class T>
void instance_bank<T>::delete_instance( T* ptr )
{
  if ( ptr )
  {
    ref_t ptr_ref = ptr;
    pref_set::iterator pi = refs_by_ptr.find( &ptr_ref );
    assert( pi != refs_by_ptr.end() );
    if ( (*pi)->count > 1 )
      --(*pi)->count;
    else
    {
      lref_set::iterator li = refs_by_label.find( *pi );
      assert( li != refs_by_label.end() );
      delete (*pi)->ptr;
      delete *pi;
      refs_by_label.erase( li );
      refs_by_ptr.erase( pi );
    }
  }
}

//--------------------------------------------------------------
template <class T>
void instance_bank<T>::purge()
{
  lref_set::iterator i = refs_by_label.begin();
  lref_set::iterator i_end = refs_by_label.end();

  while(i != i_end)
  {
    lref_set::iterator del = i;
    ++i;

    #if defined(DEBUG) && 0
    debug_print("%s ptr=%p",(*del)->label.c_str(),(*del)->ptr);
    #endif
    delete (*del)->ptr;
    delete *del;
  }
/*
  for ( i=refs_by_label.begin(); i!=refs_by_label.end(); ++i )
  {
    delete (*i)->ptr;
    delete *i;
  }
*/
  // unlike erase(), these functions should deallocate the sets

  // PEH TEST
/*   refs_by_label = lref_set(); */
/*   refs_by_ptr   = pref_set(); */
  refs_by_label.clear();
  refs_by_ptr.clear();
}
//--------------------------------------------------------------


template <class T>
void instance_bank<T>::debug_dump()
{
  lref_set::const_iterator i;
  for (i=refs_by_label.begin();i!=refs_by_label.end();++i)
  {
    debug_print((*i)->label);
  }
}

// There used to be a reference-counted smart ptr template here but
// I moved it into refptr.h  --Sean

#endif  // INSTANCE_H
