#ifndef CUSTOM_STL_H
#define CUSTOM_STL_H

// Copied from stl_alloc.h, and modified slightly for our purposes.

#if defined(__STL_WIN32_THREADS) || defined(STL_SGI_THREADS) \
|| defined(__STL_PTHREADS)
#   define __STL_THREADS
#   define __STL_VOLATILE volatile
#else
#   define __STL_VOLATILE
#endif

#ifdef __SUNPRO_CC
#  define __PRIVATE public
// Extra access restrictions prevent us from really making some things
// private.
#else
#  define __PRIVATE private
#endif

#ifdef __STL_STATIC_TEMPLATE_MEMBER_BUG
#  define __USE_MALLOC
#endif

// This implements some standard node allocators.  These are
// NOT the same as the allocators in the C++ draft standard or in
// in the original STL.  They do not encapsulate different pointer
// types; indeed we assume that there is only one pointer type.
// The allocation primitives are intended to allocate individual objects,
// not larger arenas as with the original STL allocators.

#ifndef __STL_NO_FPRINTF
#ifndef __THROW_BAD_ALLOC
#  if defined(__STL_NO_BAD_ALLOC) || !defined(__STL_USE_EXCEPTIONS)
#    include <stdio.h>
#    include <stdlib.h>
#    define __THROW_BAD_ALLOC fprintf(stderr, "out of memory\n"); exit(1)
#  else /* Standard conforming out-of-memory handling */
#    include <new>
#    define __THROW_BAD_ALLOC throw bad_alloc()
#  endif
#endif     
#else
#define __THROW_BAD_ALLOC
#endif

#ifdef __STL_WIN32THREADS

#if defined(TARGET_XBOX)
#include <xtl.h>
#else
#   include <windows.h>
#endif

#endif

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#ifndef __RESTRICT
#  define __RESTRICT
#endif

#ifdef __STL_THREADS
# include <stl_threads.h>
# define __NODE_ALLOCATOR_THREADS true
# ifdef __STL_SGI_THREADS
// We test whether threads are in use before locking.
// Perhaps this should be moved into stl_threads.h, but that
// probably makes it harder to avoid the procedure call when
// it isn't needed.
extern "C" {
	extern int __us_rsthread_malloc;
}
// The above is copied from malloc.h.  Including <malloc.h>
// would be cleaner but fails with certain levels of standard
// conformance.
#   define __NODE_ALLOCATOR_LOCK if (threads && __us_rsthread_malloc) \
{ _S_node_allocator_lock._M_acquire_lock(); }
#   define __NODE_ALLOCATOR_UNLOCK if (threads && __us_rsthread_malloc) \
{ _S_node_allocator_lock._M_release_lock(); }
# else /* !__STL_SGI_THREADS */
#   define __NODE_ALLOCATOR_LOCK \
{ if (threads) _S_node_allocator_lock._M_acquire_lock(); }
#   define __NODE_ALLOCATOR_UNLOCK \
{ if (threads) _S_node_allocator_lock._M_release_lock(); }
# endif
#else
//  Thread-unsafe
#   define __NODE_ALLOCATOR_LOCK
#   define __NODE_ALLOCATOR_UNLOCK
#   define __NODE_ALLOCATOR_THREADS false
#endif

__STL_BEGIN_NAMESPACE

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma set woff 1174
#endif

// Malloc-based allocator.  Typically slower than default alloc below.
// Typically thread-safe and more storage efficient.
#ifdef __STL_STATIC_TEMPLATE_MEMBER_BUG
# ifdef __DECLARE_GLOBALS_HERE
void (* __malloc_alloc_oom_handler)() = 0;
// g++ 2.7.2 does not handle static template data members.
# else
extern void (* __malloc_alloc_oom_handler)();
# endif
#endif

// Default node allocator.
// With a reasonable compiler, this should be roughly as fast as the
// original STL class-specific allocators, but with less fragmentation.
// Default_alloc_template parameters are experimental and MAY
// DISAPPEAR in the future.  Clients should just use alloc for now.
//
// Important implementation properties:
// 1. If the client request an object of size > _MAX_BYTES, the resulting
//    object will be obtained directly from malloc.
// 2. In all other cases, we allocate an object of size exactly
//    _S_round_up(requested_size).  Thus the client has enough size
//    information that we can return the object to the proper free list
//    without permanently losing part of the object.
//

// The first template parameter specifies whether more than one thread
// may use this allocator.  It is safe to allocate an object from
// one instance of a default_alloc and deallocate it with another
// one.  This effectively transfers its ownership to the second one.
// This may have undesirable effects on reference locality.
// The second parameter is unreferenced and serves only to allow the
// creation of multiple default_alloc instances.
// Node that containers built on different allocator instances have
// different types, limiting the utility of this approach.
#ifdef __SUNPRO_CC
// breaks if we make these template class members:
enum {_ALIGN = 8};
enum {_MAX_BYTES = 128};
enum {_NFREELISTS = _MAX_BYTES/_ALIGN};
#endif

template <bool threads, int inst>
class __my_default_alloc_template {
	
private:
	// Really we should use static const int x = N
	// instead of enum { x = N }, but few compilers accept the former.
# ifndef __SUNPRO_CC
    enum {_ALIGN = 8};
    enum {_MAX_BYTES = 128};
    enum {_NFREELISTS = _MAX_BYTES/_ALIGN};
# endif
	static size_t
		_S_round_up(size_t __bytes) 
    { return (((__bytes) + _ALIGN-1) & ~(_ALIGN - 1)); }
	
__PRIVATE:
	union _Obj {
        union _Obj* _M_free_list_link;
        char _M_client_data[1];    /* The client sees this.        */
	};
private:
# ifdef __SUNPRO_CC
    static _Obj* __STL_VOLATILE _S_free_list[]; 
	// Specifying a size results in duplicate def for 4.1
# else
    static _Obj* __STL_VOLATILE _S_free_list[_NFREELISTS]; 
# endif
	static  size_t _S_freelist_index(size_t __bytes) {
        return (((__bytes) + _ALIGN-1)/_ALIGN - 1);
	}
	
	// Returns an object of size __n, and optionally adds to size __n free list.
	static void* _S_refill(size_t __n);
	// Allocates a chunk for nobjs of size size.  nobjs may be reduced
	// if it is inconvenient to allocate the requested number.
	static char* _S_chunk_alloc(size_t __size, int& __nobjs);
	
	// Chunk allocation state.
	static char* _S_start_free;
	static char* _S_end_free;
	static size_t _S_heap_size;
	
# ifdef __STL_THREADS
    static _STL_mutex_lock _S_node_allocator_lock;
# endif
	
    // It would be nice to use _STL_auto_lock here.  But we
    // don't need the NULL check.  And we do need a test whether
    // threads have actually been started.
    class _Lock;
    friend class _Lock;
    class _Lock {
	public:
		_Lock() { __NODE_ALLOCATOR_LOCK; }
		~_Lock() { __NODE_ALLOCATOR_UNLOCK; }
    };

public:
	
	/* __n must be > 0      */
	static void* allocate(size_t __n)
	{
		_Obj* __STL_VOLATILE* __my_free_list;
		_Obj* __RESTRICT __result;
		
		if (__n > (size_t) _MAX_BYTES) {
			return(malloc_alloc::allocate(__n));
		}
		__my_free_list = _S_free_list + _S_freelist_index(__n);
		// Acquire the lock here with a constructor call.
		// This ensures that it is released in exit or during stack
		// unwinding.
#       ifndef _NOTHREADS
		/*REFERENCED*/
		_Lock __lock_instance;
#       endif
		__result = *__my_free_list;
		if (__result == 0) {
			void* __r = _S_refill(_S_round_up(__n));
			return __r;
		}
		*__my_free_list = __result -> _M_free_list_link;
		return (__result);
	};
	
	/* __p may not be 0 */
	static void deallocate(void* __p, size_t __n)
	{
		_Obj* __q = (_Obj*)__p;
		_Obj* __STL_VOLATILE* __my_free_list;
		
		if (__n > (size_t) _MAX_BYTES) {
			malloc_alloc::deallocate(__p, __n);
			return;
		}
		__my_free_list = _S_free_list + _S_freelist_index(__n);
		// acquire lock
#       ifndef _NOTHREADS
		/*REFERENCED*/
		_Lock __lock_instance;
#       endif /* _NOTHREADS */
		__q -> _M_free_list_link = *__my_free_list;
		*__my_free_list = __q;
		// lock is released here
	}
	
	static void* reallocate(void* __p, size_t __old_sz, size_t __new_sz);
} ;

typedef __my_default_alloc_template<__NODE_ALLOCATOR_THREADS, 0> my_alloc;
typedef __my_default_alloc_template<false, 0> my_single_client_alloc;

/* We allocate memory in large chunks in order to avoid fragmenting     */
/* the malloc heap too much.                                            */
/* We assume that size is properly aligned.                             */
/* We hold the allocation lock.                                         */
template <bool __threads, int __inst>
char*
__my_default_alloc_template<__threads, __inst>::_S_chunk_alloc(size_t __size, 
															   int& __nobjs)
{
    char* __result;
    size_t __total_bytes = __size * __nobjs;
    size_t __bytes_left = _S_end_free - _S_start_free;
	
    if (__bytes_left >= __total_bytes) {
        __result = _S_start_free;
        _S_start_free += __total_bytes;
        return(__result);
    } else if (__bytes_left >= __size) {
        __nobjs = (int)(__bytes_left/__size);
        __total_bytes = __size * __nobjs;
        __result = _S_start_free;
        _S_start_free += __total_bytes;
        return(__result);
    } else {
        size_t __bytes_to_get = 
			2 * __total_bytes + _S_round_up(_S_heap_size >> 4);
        // Try to make use of the left-over piece.
        if (__bytes_left > 0) {
            _Obj* __STL_VOLATILE* __my_free_list =
				_S_free_list + _S_freelist_index(__bytes_left);
			
            ((_Obj*)_S_start_free) -> _M_free_list_link = *__my_free_list;
            *__my_free_list = (_Obj*)_S_start_free;
        }
		// Added (dc 02/05/02)
extern void *StlSmallAlloc(size_t); 
		_S_start_free = (char*)StlSmallAlloc(__bytes_to_get);
		// End Added (dc 02/05/02)
		// Removed (dc 02/05/02)
/*
        _S_start_free = (char*)malloc(__bytes_to_get);
        if (0 == _S_start_free) {
            size_t __i;
            _Obj* __STL_VOLATILE* __my_free_list;
			_Obj* __p;
            // Try to make do with what we have.  That can't
            // hurt.  We do not try smaller requests, since that tends
            // to result in disaster on multi-process machines.
            for (__i = __size; __i <= _MAX_BYTES; __i += _ALIGN) {
                __my_free_list = _S_free_list + _S_freelist_index(__i);
                __p = *__my_free_list;
                if (0 != __p) {
                    *__my_free_list = __p -> _M_free_list_link;
                    _S_start_free = (char*)__p;
                    _S_end_free = _S_start_free + __i;
                    return(_S_chunk_alloc(__size, __nobjs));
                    // Any leftover piece will eventually make it to the
                    // right free list.
                }
            }
			_S_end_free = 0;	// In case of exception.
            _S_start_free = (char*)malloc_alloc::allocate(__bytes_to_get);
            // This should either throw an
            // exception or remedy the situation.  Thus we assume it
            // succeeded.
        }
*/
		// End removed (dc 02/05/02)
        _S_heap_size += __bytes_to_get;
        _S_end_free = _S_start_free + __bytes_to_get;
        return(_S_chunk_alloc(__size, __nobjs));
    }
}


/* Returns an object of size __n, and optionally adds to size __n free list.*/
/* We assume that __n is properly aligned.                                */
/* We hold the allocation lock.                                         */
template <bool __threads, int __inst>
void*
__my_default_alloc_template<__threads, __inst>::_S_refill(size_t __n)
{
    int __nobjs = 20;
    char* __chunk = _S_chunk_alloc(__n, __nobjs);
    _Obj* __STL_VOLATILE* __my_free_list;
    _Obj* __result;
    _Obj* __current_obj;
    _Obj* __next_obj;
    int __i;
	
    if (1 == __nobjs) return(__chunk);
    __my_free_list = _S_free_list + _S_freelist_index(__n);
	
    /* Build free list in chunk */
	__result = (_Obj*)__chunk;
	*__my_free_list = __next_obj = (_Obj*)(__chunk + __n);
	for (__i = 1; ; __i++) {
        __current_obj = __next_obj;
        __next_obj = (_Obj*)((char*)__next_obj + __n);
        if (__nobjs - 1 == __i) {
            __current_obj -> _M_free_list_link = 0;
            break;
        } else {
            __current_obj -> _M_free_list_link = __next_obj;
        }
	}
    return(__result);
}

template <bool threads, int inst>
void*
__my_default_alloc_template<threads, inst>::reallocate(void* __p,
													   size_t __old_sz,
													   size_t __new_sz)
{
    void* __result;
    size_t __copy_sz;
	
    if (__old_sz > (size_t) _MAX_BYTES && __new_sz > (size_t) _MAX_BYTES) {
        return(realloc(__p, __new_sz));
    }
    if (_S_round_up(__old_sz) == _S_round_up(__new_sz)) return(__p);
    __result = allocate(__new_sz);
    __copy_sz = __new_sz > __old_sz? __old_sz : __new_sz;
    memcpy(__result, __p, __copy_sz);
    deallocate(__p, __old_sz);
    return(__result);
}

#ifdef __STL_THREADS
template <bool __threads, int __inst>
_STL_mutex_lock
__my_default_alloc_template<__threads, __inst>::_S_node_allocator_lock
__STL_MUTEX_INITIALIZER;
#endif


template <bool __threads, int __inst>
char* __my_default_alloc_template<__threads, __inst>::_S_start_free = 0;

template <bool __threads, int __inst>
char* __my_default_alloc_template<__threads, __inst>::_S_end_free = 0;

template <bool __threads, int __inst>
size_t __my_default_alloc_template<__threads, __inst>::_S_heap_size = 0;

template <bool __threads, int __inst>
__my_default_alloc_template<__threads, __inst>::_Obj* __STL_VOLATILE
__my_default_alloc_template<__threads, __inst> ::_S_free_list[
# ifdef __SUNPRO_CC
_NFREELISTS
# else
__my_default_alloc_template<__threads, __inst>::_NFREELISTS
# endif
] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };
// The 16 zeros are necessary to make version 4.1 of the SunPro
// compiler happy.  Otherwise it appears to allocate too little
// space for the array.

# ifdef __STL_WIN32THREADS
// Create one to get critical section initialized.
// We do this onece per file, but only the first constructor
// does anything.
static my_alloc __node_allocator_dummy_instance;
# endif

template <class _Tp>
class my_allocator {
	typedef my_alloc _Alloc;          // The underlying allocator.
public:
	typedef size_t     size_type;
	typedef ptrdiff_t  difference_type;
	typedef _Tp*       pointer;
	typedef const _Tp* const_pointer;
	typedef _Tp&       reference;
	typedef const _Tp& const_reference;
	typedef _Tp        value_type;
	
	template <class _Tp1> struct rebind {
		typedef my_allocator<_Tp1> other;
	};
	
	my_allocator() __STL_NOTHROW {}
	my_allocator(const my_allocator&) __STL_NOTHROW {}
	template <class _Tp1> my_allocator(const my_allocator<_Tp1>&) __STL_NOTHROW {}
	~my_allocator() __STL_NOTHROW {}
	
	pointer address(reference __x) const { return &__x; }
	const_pointer address(const_reference __x) const { return &__x; }
	
	// __n is permitted to be 0.  The C++ standard says nothing about what
	// the return value is when __n == 0.
	_Tp* allocate(size_type __n, const void* = 0) {
		return __n != 0 ? static_cast<_Tp*>(_Alloc::allocate(__n * sizeof(_Tp))) 
			: 0;
	}
	
	// __p is not permitted to be a null pointer.
	void deallocate(pointer __p, size_type __n)
    { _Alloc::deallocate(__p, __n * sizeof(_Tp)); }
	
	size_type max_size() const __STL_NOTHROW 
    { return size_t(-1) / sizeof(_Tp); }
	
	void construct(pointer __p, const _Tp& __val) { new(__p) _Tp(__val); }
	void destroy(pointer __p) { __p->~_Tp(); }
};

template<>
class my_allocator<void> {
	typedef size_t      size_type;
	typedef ptrdiff_t   difference_type;
	typedef void*       pointer;
	typedef const void* const_pointer;
	typedef void        value_type;
	
	template <class _Tp1> struct rebind {
		typedef my_allocator<_Tp1> other;
	};
};


template <class _T1, class _T2>
inline bool operator==(const my_allocator<_T1>&, const my_allocator<_T2>&) 
{
	return true;
}

template <class _T1, class _T2>
inline bool operator!=(const my_allocator<_T1>&, const my_allocator<_T2>&)
{
	return false;
}

// Allocator adaptor to turn an SGI-style allocator (e.g. alloc, malloc_alloc)
// into a standard-conforming allocator.   Note that this adaptor does
// *not* assume that all objects of the underlying alloc class are
// identical, nor does it assume that all of the underlying alloc's
// member functions are static member functions.  Note, also, that 
// __allocator<_Tp, alloc> is essentially the same thing as allocator<_Tp>.

template <class _Tp, class _Alloc>
struct __my_allocator {
	_Alloc __underlying_alloc;
	
	typedef size_t    size_type;
	typedef ptrdiff_t difference_type;
	typedef _Tp*       pointer;
	typedef const _Tp* const_pointer;
	typedef _Tp&       reference;
	typedef const _Tp& const_reference;
	typedef _Tp        value_type;
	
	template <class _Tp1> struct rebind {
		typedef __my_allocator<_Tp1, _Alloc> other;
	};
	
	__my_allocator() __STL_NOTHROW {}
	__my_allocator(const __my_allocator& __a) __STL_NOTHROW
		: __underlying_alloc(__a.__underlying_alloc) {}
	template <class _Tp1> 
		__my_allocator(const __my_allocator<_Tp1, _Alloc>& __a) __STL_NOTHROW
		: __underlying_alloc(__a.__underlying_alloc) {}
	~__my_allocator() __STL_NOTHROW {}
	
	pointer address(reference __x) const { return &__x; }
	const_pointer address(const_reference __x) const { return &__x; }
	
	// __n is permitted to be 0.
	_Tp* allocate(size_type __n, const void* = 0) {
		return __n != 0 
			? static_cast<_Tp*>(__underlying_alloc.allocate(__n * sizeof(_Tp))) 
			: 0;
	}
	
	// __p is not permitted to be a null pointer.
	void deallocate(pointer __p, size_type __n)
    { __underlying_alloc.deallocate(__p, __n * sizeof(_Tp)); }
	
	size_type max_size() const __STL_NOTHROW 
    { return size_t(-1) / sizeof(_Tp); }
	
	void construct(pointer __p, const _Tp& __val) { new(__p) _Tp(__val); }
	void destroy(pointer __p) { __p->~_Tp(); }
};

template <class _Alloc>
class __my_allocator<void, _Alloc> {
	typedef size_t      size_type;
	typedef ptrdiff_t   difference_type;
	typedef void*       pointer;
	typedef const void* const_pointer;
	typedef void        value_type;
	
	template <class _Tp1> struct rebind {
		typedef __my_allocator<_Tp1, _Alloc> other;
	};
};

template <class _Tp, class _Alloc>
inline bool operator==(const __my_allocator<_Tp, _Alloc>& __a1,
                       const __my_allocator<_Tp, _Alloc>& __a2)
{
	return __a1.__underlying_alloc == __a2.__underlying_alloc;
}

#ifdef __STL_FUNCTION_TMPL_PARTIAL_ORDER
template <class _Tp, class _Alloc>
inline bool operator!=(const __my_allocator<_Tp, _Alloc>& __a1,
                       const __my_allocator<_Tp, _Alloc>& __a2)
{
	return __a1.__underlying_alloc != __a2.__underlying_alloc;
}
#endif /* __STL_FUNCTION_TMPL_PARTIAL_ORDER */

// Another allocator adaptor: _Alloc_traits.  This serves two
// purposes.  First, make it possible to write containers that can use
// either SGI-style allocators or standard-conforming allocator.
// Second, provide a mechanism so that containers can query whether or
// not the allocator has distinct instances.  If not, the container
// can avoid wasting a word of memory to store an empty object.

// This adaptor uses partial specialization.  The general case of
// _Alloc_traits<_Tp, _Alloc> assumes that _Alloc is a
// standard-conforming allocator, possibly with non-equal instances
// and non-static members.  (It still behaves correctly even if _Alloc
// has static member and if all instances are equal.  Refinements
// affect performance, not correctness.)

// There are always two members: allocator_type, which is a standard-
// conforming allocator type for allocating objects of type _Tp, and
// _S_instanceless, a static const member of type bool.  If
// _S_instanceless is true, this means that there is no difference
// between any two instances of type allocator_type.  Furthermore, if
// _S_instanceless is true, then _Alloc_traits has one additional
// member: _Alloc_type.  This type encapsulates allocation and
// deallocation of objects of type _Tp through a static interface; it
// has two member functions, whose signatures are
//    static _Tp* allocate(size_t)
//    static void deallocate(_Tp*, size_t)

// The version for the default allocator.

template <class _Tp, class _Tp1>
struct _Alloc_traits<_Tp, my_allocator<_Tp1> >
{
	static const bool _S_instanceless = true;
	typedef simple_alloc<_Tp, my_alloc> _Alloc_type;
	typedef my_allocator<_Tp> allocator_type;
};

// Versions for the __allocator adaptor used with the predefined
// SGI-style allocators.

template <class _Tp, class _Tp1, int __inst>
struct _Alloc_traits<_Tp, 
__my_allocator<_Tp1, __malloc_alloc_template<__inst> > >
{
	static const bool _S_instanceless = true;
	typedef simple_alloc<_Tp, __malloc_alloc_template<__inst> > _Alloc_type;
	typedef __my_allocator<_Tp, __malloc_alloc_template<__inst> > allocator_type;
};

template <class _Tp, class _Tp1, bool __thr, int __inst>
struct _Alloc_traits<_Tp, 
__my_allocator<_Tp1, 
__my_default_alloc_template<__thr, __inst> > >
{
	static const bool _S_instanceless = true;
	typedef simple_alloc<_Tp, __my_default_alloc_template<__thr,__inst> > 
		_Alloc_type;
	typedef __my_allocator<_Tp, __my_default_alloc_template<__thr,__inst> > 
		allocator_type;
};

template <class _Tp, class _Tp1, class _Alloc>
struct _Alloc_traits<_Tp, __my_allocator<_Tp1, debug_alloc<_Alloc> > >
{
	static const bool _S_instanceless = true;
	typedef simple_alloc<_Tp, debug_alloc<_Alloc> > _Alloc_type;
	typedef __my_allocator<_Tp, debug_alloc<_Alloc> > allocator_type;
};

#endif