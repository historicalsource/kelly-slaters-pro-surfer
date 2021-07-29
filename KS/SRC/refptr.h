#ifndef REFPTR_H
#define REFPTR_H

// template extrefptr provides a reference-counted pointer object that
// maintains the reference count outside the controlled object.  This
// means 4 bytes extra overhead per controlled object and one extra
// dereference required to access the controlled object.  If you're
// really concerned with efficiency you should use refptr, below.
template <class TT>
class extrefptr
{
#ifndef __GNUC__
  typedef typename TT T;
#else
  typedef TT T;
#endif
  typedef extrefptr<TT> P;
  typedef unsigned int count_t;
protected:
  template <class T>
  struct ref_data
  {
    T* p;
    count_t count;
    ref_data( T* _ptr ) : p(_ptr), count(1) {}
    ~ref_data() { delete p; }
  };
  typedef ref_data<T> R;
  R* p;

public:
  extrefptr()
    : p( NULL )
  {
  }

  // Once an extrefptr is constructed from a given pointer, responsibility
  // for deleting the controlled object falls to this class and no non-
  // extrefptr references to the controlled object are allowed.
  extrefptr( T* p )
  {
    _new( p );
  }

  extrefptr( const P& b )
  {
    _copy( b );
  }

  ~extrefptr()
  {
    _destroy();
  }

private:
  inline void _new( T* p );
  inline void _copy( const P& b );
  inline void _destroy();

public:
  operator T*() const
  {
    assert( p );
    return p->p;
  }

  T* operator->() const
  {
    assert( p );
    return p->p;
  }

  operator bool() const { return p!=NULL; }

  count_t nrefs() const { return p ? p->count : 0; }

  const P& operator=( const P& b )
  {
    _destroy();
    _copy( b );
    return *this;
  }

  const P& operator=( T* p )
  {
    _destroy();
    _new( p );
    return *this;
  }

  bool operator==( const P& b ) const { return ( p == b.p ); }
  bool operator!=( const P& b ) const { return ( p != b.p ); }
}; // template class extrefptr

// These have to be outside the class declaration so that extrefptr<T>
// can be declared on an undefined T

template <class T>
inline void extrefptr<T>::_new( T* a )
{
  p = NEW R( a );
}

template <class T>
inline void extrefptr<T>::_copy( const P& r )
{
  p = r.p;
  if ( p )
    ++p->count;
}

template <class T>
inline void extrefptr<T>::_destroy()
{
  if ( p )
    if ( --p->count == 0 )
      delete p;
}



class ref   // a reference-counting class to use as a base for classes
{           // for which you want to declare refptr<T>
private:
  // doesn't really make sense to be able to copy reference-count objects
  ref(const ref&) : count(0) {} // copies start with no refs
  ref& operator =(const ref&) { return *this; } // no effect on refs

protected:
  unsigned count;

  ref(unsigned c) : count(c) {}
public:
  ref() : count(0) {}

  // generally you will want the dtor for the derived class to be virtual
  // but we won't force it on you.
 #if defined(DEBUG) && 1
  /*virtual*/ ~ref() { assert(count==0); }
 #endif

  bool operator ==(const ref& r) const { return this == &r; }
  bool operator < (const ref& r) const { return this <  &r; }

  unsigned refs() const {
    return count;
  }
  void addref() {
    ++count;
  }
  bool subref() {
   #if defined(DEBUG)
    assert(count>0 && count<0x80000000);
   #endif
    // returns true if still being referenced
    return --count!=0;
  }
};

template <class TT>  // works on anything derived publicly from ref
class refptr {     // reference-counted pointer class
  // If the memory must be freed by a special free routine, should
  // override class operator delete.
  // When our destruction or assignment would leave no references to
  // the pointed-to object, that object is deleted.
  // We try to leave no pointers to an object that has been deleted.
public:
#ifndef __GNUC__
  typedef typename TT T;
#else
  typedef TT T;
#endif

protected:
  T* p;
  typedef refptr<T> R;
public:
  refptr(T* a=0)     : p(a)  { addref(); }
  refptr(const R& a) : p(a)  { addref(); }
  ~refptr()                  { subref(); /*p=0;*/ }

  R& operator=(T* a)
  {
    if (p!=a)
    {
      subref();
      p=a;
      addref();
    }
    return *this;
  }
  R& operator=(const R& a) {
    subref();
    p=a.p;
    addref();
    return *this;
  }

  unsigned refs() const { return p ? p->refs() : 0; }

  bool operator ==(const R& r) const { return p == r.p; }
  bool operator < (const R& r) const { return p <  r.p; }

  operator T*() const { return p; }
  T& operator* () const { return *p; }
  T* operator->() const { return  p; }

  T* get() const  { return p; }
  T* release() {  // keep from deleting later
    T* tmp=p; p=0; return tmp;
  }
protected:
  void subref();
  void addref();
}; // template class refptr

// These have to be outside the class declaration so that refptr<T>
// can be declared on an undefined T

template <class T>
inline void refptr<T>::addref() {
  if (p)
    p->addref();
}
template <class T>
inline void refptr<T>::subref() {
  if (p)
    if (!p->subref())
      delete p;
}



#endif // REFPTR_H
