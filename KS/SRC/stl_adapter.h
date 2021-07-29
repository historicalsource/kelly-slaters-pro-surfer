//
//  STL_ADAPTER.H
//
//  stuff to adapt to microsoft/sgi/metrowerks SGI.
//
#ifndef STL_ADAPTER_H
#define STL_ADAPTER_H

#if (!defined __MSL_STL) && defined(TARGET_XBOX)
#pragma warning(4: 4291) // disable no matching operator delete warning in sgi_stl for the xbox platform
#endif

///////////////////////////////////////////////////////////////////////////////
// Metrowerks Standard Template Library
#if 0 //defined(__MKS_STL)

#include <iterator>

// this came from stl_iterator_base.h in the SGI STL, since Metrowerks STL doesn't have an equivalent I can find.
template
  <
  class _Tp,
  class _Distance
  >
struct bidirectional_iterator
  {
  typedef bidirectional_iterator_tag iterator_category;
  typedef _Tp                        value_type;
  typedef _Distance                  difference_type;
  typedef _Tp*                       pointer;
  typedef _Tp&                       reference;
  };

// These macros substitute for standard methods of the allocator template class:

// Allocate N elements (element size is known by allocator object).
#define _ALLOCATE(n)  allocate(n)  // ??? is this correct for Metrowerks?
// De-allocate an array of N elements at the given address (N must match
// whatever was previously allocated).
#define _DEALLOCATE(p,n)  deallocate(p,n)


///////////////////////////////////////////////////////////////////////////////
// SGI Standard Template Library
#endif

#if defined(__SGI_STL)

#ifndef __THROW_BAD_ALLOC
#define __THROW_BAD_ALLOC
#endif
#define __STL_NO_EXCEPTION_HEADER
#define __STL_NO_EXCEPTIONS
#define __STL_USE_SGI_ALLOCATORS

#include "defalloc.h"

// These macros substitute for standard methods of the allocator template class:

// Allocate N elements (element size is known by allocator object).
#define _ALLOCATE(n)  allocate(n)
// De-allocate an array of N elements at the given address (N must match
// whatever was previously allocated).
#define _DEALLOCATE(p,n)  deallocate(p)

#elif defined(__GNU_STL)

using namespace std;

#elif defined(__MSL_STL)

#define _MSL_NO_MEMBER_TEMPLATE 1

#include <iterator>

template
  <
  class _Tp,
  class _Distance
  >
struct bidirectional_iterator
  {
  typedef bidirectional_iterator_tag iterator_category;
  typedef _Tp                        value_type;
  typedef _Distance                  difference_type;
  typedef _Tp*                       pointer;
  typedef _Tp&                       reference;
  };

using namespace std;

#else
///////////////////////////////////////////////////////////////////////////////
// Microsoft Standard Template Library

#include <xutility>

using namespace std;

template
  <
  class _Partition,
  class difference_type,
  class _Pointer,
  class _Reference
  >
class bidirectional_iterator : public _Bidit<_Partition,difference_type,_Pointer,_Reference>
  {
  };

// These macros substitute for standard methods of the allocator template class:

// Allocate an array of N elements (element size is known by allocator object).
#define _ALLOCATE(n)  allocate(n,NULL)
// De-allocate an array of N elements at the given address (N must match
// whatever was previously allocated).
#define _DEALLOCATE(p,n)  deallocate(p,n)


#endif


#endif
