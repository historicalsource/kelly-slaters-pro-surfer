#ifndef _staticmem_h
#define _staticmem_h

// these are macros for making things statically allocated
	// Game Cube STL bites
#ifndef TARGET_GC
#define USINGSTATICSTLALLOCATIONS
#endif

#ifndef USINGSTATICSTLALLOCATIONS

//#define STATICALLOCCLASSHEADER
#define STATICALLOCCLASSHEADER 	public:\
	static void check_mem_init(void) {}  \
	static void mem_cleanup(void) {}

#define STATICALLOCCLASSINSTANCE(c,n)

#else  // USINGSTATICSTLALLOCATIONS


















void nglPrintf( const char* Format, ... );


#define STATICALLOCCLASSHEADER 	public:\
	static bool meminit;                 \
	static bool *allocated;              \
	static void *membuffer;              \
  static int current_allocation;  \
																			 \
	static void check_mem_init(void);    \
  static void *mem_init_func;     \
  static void *mem_free_func;     \
	static void mem_cleanup(void);       \
																			 \
	void *operator new(size_t size);     \
	void *operator new(size_t size, unsigned int flags, const char *descript, int line ); \
	void operator delete( void* block );


#define STATICALLOCCLASSINSTANCE(c,n)                                                   \
	bool c::meminit=false;                                                                  \
	bool *c::allocated=NULL;                                                                \
	void *c::membuffer=NULL;                                                                \
  void *c::mem_init_func = NULL;                                                    \
  void *c::mem_free_func = NULL;                                                    \
  int c::current_allocation = 0; \
																																				                 	\
	void c::check_mem_init(void)                                                            \
	{                                                                                       \
		if ( !meminit )                                                                       \
		{                                                                                     \
			membuffer=malloc(n*sizeof(c));                                                      \
			if ( membuffer==NULL ) return;                                                      \
			memset(membuffer,0,n*sizeof(c));                                                    \
			allocated=(bool *) malloc(n*sizeof(bool));                                          \
			if ( allocated==NULL ) return;                                                      \
			memset(allocated,0,n*sizeof(bool));                                                 \
			meminit=true;                                                                       \
      if (mem_init_func) \
        { void (*func) () = (void (*) ()) mem_init_func; (*func) (); } \
		}                                                                                     \
	}                                                                                       \
																																				                 	\
	void c::mem_cleanup(void)                                                               \
	{                                                                                       \
		if ( meminit )                                                                        \
		{                                                                                     \
			free(membuffer);                                                                    \
			free(allocated);                                                                    \
			meminit=false;                                                                      \
      if (mem_free_func) \
        { void (*func) () = (void (*) ()) mem_free_func; (*func) (); } \
		}                                                                                     \
	}                                                                                       \
																																				                 	\
																																				                 	\
																																				                 	\
	void *c::operator new(size_t size)                                                      \
	{                                                                                       \
		assert(sizeof(c)==size);                                                              \
		check_mem_init();                                                                     \
		assert(meminit);                                                                      \
		for ( int i=0; i<n; i++ )                                                             \
		{                                                                                     \
			if ( !allocated[i] )                                                                \
			{                                                                                   \
        current_allocation = i;                                                           \
				allocated[i]=true;                                                                \
				void *rv=(void*)( ((char *)membuffer)+(i*sizeof(c)) );                            \
				/*memset(rv,0,sizeof(c));*/                                                       \
				return rv;                                                                        \
			}                                                                                   \
		}                                                                                     \
		nglPrintf("Not enough %s thingies available\n",#c);                                   \
		assert(0);                                                                            \
		return (void *)(-1);	/* just to avoid compiler warning (dc 01/21/02) */				      \
	}                                                                                       \
																																				                 	\
	void *c::operator new(size_t size, unsigned int flags, const char *descript, int line ) \
	{                                                                                       \
    return c::operator new(size);                                                         \
	}                                                                                       \
																																													\
	void c::operator delete( void* block )                                                  \
	{                                                                                       \
	  if (block==NULL) return;                                                              \
		for ( int i=0; i<n; i++ )                                                             \
		{                                                                                     \
			void *eaptr=(void*)( ((char *)membuffer)+(i*sizeof(c)) );                           \
			if ( allocated[i] && eaptr==block)                                                  \
			{                                                                                   \
				allocated[i]=false;                                                               \
				return;                                                                           \
			}                                                                                   \
		}                                                                                     \
		nglPrintf("Attempt to free an unknown %s\n",#c);                                      \
		assert(0);                                                                            \
	}

#endif  //  USINGSTATICSTLALLOCATIONS

#if 1 //ndef __MSL_STL

// should have something here to define specialstaticallocator to the default allocator

#else

#include <memory>

template <class T>
class specialstaticallocator
{
public:
	typedef size_t    size_type;
	typedef ptrdiff_t difference_type;
	typedef T*        pointer;
	typedef const T*  const_pointer;
	typedef T&        reference;
	typedef const T&  const_reference;
	typedef T         value_type;
#ifndef _MSL_NO_MEMBER_TEMPLATE
	template <class U> struct rebind { typedef allocator<U> other; };
#endif

	specialstaticallocator() _MSL_THROW;
#ifndef _MSL_NO_MEMBER_TEMPLATE
#ifndef _MSL_MUST_INLINE_MEMBER_TEMPLATE
	template <class U> specialstaticallocator(const specialstaticallocator<U>&) _MSL_THROW;
#else
	template <class U>
	inline
	specialstaticallocator(const specialstaticallocator<U>&) _MSL_THROW
	{
	}
#endif
#endif

	pointer address(reference x) const;
	const_pointer address(const_reference x) const;

	pointer allocate(size_type n, allocator<void>::const_pointer hint = 0);
	void deallocate(pointer p, size_type n);
	size_type max_size() const _MSL_THROW;

	void construct(pointer p, const T& val);
	void destroy(pointer p);
};

template <class T>
inline
specialstaticallocator<T>::specialstaticallocator() _MSL_THROW
{
}

#ifndef _MSL_NO_MEMBER_TEMPLATE
#ifndef _MSL_MUST_INLINE_MEMBER_TEMPLATE
	template <class T>
	template <class U>
	inline
	specialstaticallocator<T>::specialstaticallocator(const specialstaticallocator<U>&) _MSL_THROW
	{
	}
#endif
#endif

template <class T>
inline
typename specialstaticallocator<T>::pointer
specialstaticallocator<T>::address(reference x) const
{
	return &x;
}

template <class T>
inline
typename specialstaticallocator<T>::const_pointer
specialstaticallocator<T>::address(const_reference x) const
{
	return &x;
}

template <class T>
inline
typename specialstaticallocator<T>::pointer
specialstaticallocator<T>::allocate(size_type n, specialstaticallocator<void>::const_pointer)
{
#ifndef _MSL_NO_EXCEPTIONS
	return static_cast<pointer>(operator new(n * sizeof(T)));
#else
	pointer p = static_cast<pointer>(operator new(n * sizeof(T)));
	if (p == 0)
		__msl_error("Memory allocation failure");
	return p;
#endif
}

template <class T>
inline
void
specialstaticallocator<T>::deallocate(pointer p, size_type)
{
	T::operator delete(p);
}

template <class T>
inline
typename specialstaticallocator<T>::size_type
specialstaticallocator<T>::max_size() const _MSL_THROW
{
	return numeric_limits<size_type>::max() / sizeof(T);
}

template <class T>
inline
void
specialstaticallocator<T>::construct(pointer p, const T& val)
{
	new(p) T(val);
}

template <class T>
inline
void
specialstaticallocator<T>::destroy(pointer p)
{
	p->~T();
}

template <class T, class U>
inline
bool
operator==(const specialstaticallocator<T>&, const specialstaticallocator<U>&) _MSL_THROW
{
	return true;
}

template <class T, class U>
inline
bool
operator!=(const specialstaticallocator<T>&, const specialstaticallocator<U>&) _MSL_THROW
{
	return false;
}



template <class T>
class simplestaticallocator
{
public:
	typedef size_t    size_type;
	typedef ptrdiff_t difference_type;
	typedef T*        pointer;
	typedef const T*  const_pointer;
	typedef T&        reference;
	typedef const T&  const_reference;
	typedef T         value_type;
#ifndef _MSL_NO_MEMBER_TEMPLATE
	template <class U> struct rebind { typedef allocator<U> other; };
#endif

	simplestaticallocator() _MSL_THROW;
#ifndef _MSL_NO_MEMBER_TEMPLATE
#ifndef _MSL_MUST_INLINE_MEMBER_TEMPLATE
	template <class U> simplestaticallocator(const simplestaticallocator<U>&) _MSL_THROW;
#else
	template <class U>
	inline
	simplestaticallocator(const simplestaticallocator<U>&) _MSL_THROW
	{
	}
#endif
#endif

	pointer address(reference x) const;
	const_pointer address(const_reference x) const;

	pointer allocate(size_type n, allocator<void>::const_pointer hint = 0);
	void deallocate(pointer p, size_type n);
	size_type max_size() const _MSL_THROW;

	void construct(pointer p, const T& val);
	void destroy(pointer p);
};

template <class T>
inline
simplestaticallocator<T>::simplestaticallocator() _MSL_THROW
{
}

#ifndef _MSL_NO_MEMBER_TEMPLATE
#ifndef _MSL_MUST_INLINE_MEMBER_TEMPLATE
	template <class T>
	template <class U>
	inline
	simplestaticallocator<T>::simplestaticallocator(const simplestaticallocator<U>&) _MSL_THROW
	{
	}
#endif
#endif

template <class T>
inline
typename simplestaticallocator<T>::pointer
simplestaticallocator<T>::address(reference x) const
{
	return &x;
}

template <class T>
inline
typename simplestaticallocator<T>::const_pointer
simplestaticallocator<T>::address(const_reference x) const
{
	return &x;
}

template <class T>
inline
typename simplestaticallocator<T>::pointer
simplestaticallocator<T>::allocate(size_type n, simplestaticallocator<void>::const_pointer)
{
#ifndef _MSL_NO_EXCEPTIONS
	return static_cast<pointer>(operator new(n * sizeof(T)));
#else
	pointer p = static_cast<pointer>(operator new(n * sizeof(T)));
	if (p == 0)
		__msl_error("Memory allocation failure");
	return p;
#endif
}

template <class T>
inline
void
simplestaticallocator<T>::deallocate(pointer p, size_type)
{
	::operator delete(p);
}

template <class T>
inline
typename simplestaticallocator<T>::size_type
simplestaticallocator<T>::max_size() const _MSL_THROW
{
	return numeric_limits<size_type>::max() / sizeof(T);
}

template <class T>
inline
void
simplestaticallocator<T>::construct(pointer p, const T& val)
{
	new(p) T(val);
}

template <class T>
inline
void
simplestaticallocator<T>::destroy(pointer p)
{
	p->~T();
}

template <class T, class U>
inline
bool
operator==(const simplestaticallocator<T>&, const simplestaticallocator<U>&) _MSL_THROW
{
	return true;
}

template <class T, class U>
inline
bool
operator!=(const simplestaticallocator<T>&, const simplestaticallocator<U>&) _MSL_THROW
{
	return false;
}














#endif //__MSL_STL











/*  Metrowerks Standard Library  */

/*  $Date: 1999/12/09 17:59:44 $
 *  $Revision: 1.14.6.1 $
 *  $NoKeywords: $
 *
 *		Portions Copyright © 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/**
 **  memory
 **/

#if 0 //ndef _MEMORY
#define _MEMORY

#include <mslconfig>

#include <cstddef>
#include <cstring>
#ifndef _MSL_NO_WCHART
	#include <cwchar>
#endif
#include <new>
#include <limits>
#include <utility>
#include <iterator>

#ifndef RC_INVOKED

#pragma options align=native
#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import on
#endif

#ifndef _MSL_NO_CPP_NAMESPACE
	namespace std {
#endif

// Warning, __destroy is non-standard

template <class T>
inline
void
__destroy(T* pointer)
{
	pointer->~T ();
}

template <class ForwardIterator>
inline
void
__destroy(ForwardIterator first, ForwardIterator last, forward_iterator_tag)
{
	for(; first != last; ++first)
		__destroy(&*first);
}

template <class RandomAccessIterator>
inline
void
__destroy(RandomAccessIterator first, RandomAccessIterator last, random_access_iterator_tag)
{
	for(; first < last; ++first)
		__destroy(&*first);
}

template <class ForwardIterator>
inline
void
__destroy(ForwardIterator first, ForwardIterator last)
{
	__destroy(first, last, iterator_traits<ForwardIterator>::iterator_category());
}

//  lib.default.allocator, the default allocator:

template <class T> class allocator;

//  specialize for  void:

template <>
class allocator<void>
{
public:
	typedef size_t      size_type;
	typedef ptrdiff_t   difference_type;
	typedef void*       pointer;
	typedef const void* const_pointer;
	typedef void        value_type;
#ifndef _MSL_NO_MEMBER_TEMPLATE
	template <class U> struct rebind { typedef allocator<U> other; };
#endif
};

template <class T>
class allocator
{
public:
	typedef size_t    size_type;
	typedef ptrdiff_t difference_type;
	typedef T*        pointer;
	typedef const T*  const_pointer;
	typedef T&        reference;
	typedef const T&  const_reference;
	typedef T         value_type;
#ifndef _MSL_NO_MEMBER_TEMPLATE
	template <class U> struct rebind { typedef allocator<U> other; };
#endif

	allocator() _MSL_THROW;
#ifndef _MSL_NO_MEMBER_TEMPLATE
#ifndef _MSL_MUST_INLINE_MEMBER_TEMPLATE
	template <class U> allocator(const allocator<U>&) _MSL_THROW;
#else
	template <class U>
	inline
	allocator(const allocator<U>&) _MSL_THROW
	{
	}
#endif
#endif

	pointer address(reference x) const;
	const_pointer address(const_reference x) const;

	pointer allocate(size_type n, allocator<void>::const_pointer hint = 0);
	void deallocate(pointer p, size_type n);
	size_type max_size() const _MSL_THROW;

	void construct(pointer p, const T& val);
	void destroy(pointer p);
};

template <class T>
inline
allocator<T>::allocator() _MSL_THROW
{
}

#ifndef _MSL_NO_MEMBER_TEMPLATE
#ifndef _MSL_MUST_INLINE_MEMBER_TEMPLATE
	template <class T>
	template <class U>
	inline
	allocator<T>::allocator(const allocator<U>&) _MSL_THROW
	{
	}
#endif
#endif

template <class T>
inline
typename allocator<T>::pointer
allocator<T>::address(reference x) const
{
	return &x;
}

template <class T>
inline
typename allocator<T>::const_pointer
allocator<T>::address(const_reference x) const
{
	return &x;
}

template <class T>
inline
typename allocator<T>::pointer
allocator<T>::allocate(size_type n, allocator<void>::const_pointer)
{
#ifndef _MSL_NO_EXCEPTIONS
	return static_cast<pointer>(operator new(n * sizeof(T)));
#else
	pointer p = static_cast<pointer>(operator new(n * sizeof(T)));
	if (p == 0)
		__msl_error("Memory allocation failure");
	return p;
#endif
}

template <class T>
inline
void
allocator<T>::deallocate(pointer p, size_type)
{
	operator delete(p);
}

template <class T>
inline
typename allocator<T>::size_type
allocator<T>::max_size() const _MSL_THROW
{
	return numeric_limits<size_type>::max() / sizeof(T);
}

template <class T>
inline
void
allocator<T>::construct(pointer p, const T& val)
{
	new(p) T(val);
}

template <class T>
inline
void
allocator<T>::destroy(pointer p)
{
	p->~T();
}

template <class T, class U>
inline
bool
operator==(const allocator<T>&, const allocator<U>&) _MSL_THROW
{
	return true;
}

template <class T, class U>
inline
bool
operator!=(const allocator<T>&, const allocator<U>&) _MSL_THROW
{
	return false;
}

//  lib.storage.iterator, raw storage iterator:

template <class OutputIterator, class T>
class raw_storage_iterator
	: public iterator<output_iterator_tag, void, void, void, void>
{
public:
	explicit raw_storage_iterator(OutputIterator x);

	raw_storage_iterator& operator*();
	raw_storage_iterator& operator=(const T& element);
	raw_storage_iterator& operator++();
	raw_storage_iterator  operator++(int);
private:
	OutputIterator x_;
};

template <class OutputIterator, class T>
inline
raw_storage_iterator<OutputIterator, T>::raw_storage_iterator(OutputIterator x)
	: x_(x)
{
}

template <class OutputIterator, class T>
inline
raw_storage_iterator<OutputIterator, T>&
raw_storage_iterator<OutputIterator, T>::operator*()
{
	return *this;
}

template <class OutputIterator, class T>
inline
raw_storage_iterator<OutputIterator, T>&
raw_storage_iterator<OutputIterator, T>::operator=(const T& element)
{
	new(&*x_) T(element);
}

template <class OutputIterator, class T>
inline
raw_storage_iterator<OutputIterator, T>&
raw_storage_iterator<OutputIterator, T>::operator++()
{
	++x_;
	return *this;
}

template <class OutputIterator, class T>
inline
raw_storage_iterator<OutputIterator, T>
raw_storage_iterator<OutputIterator, T>::operator++(int)
{
	raw_storage_iterator tmp(*this);
	++x_;
	return return tmp;
}

//  lib.temporary.buffer, temporary buffers:
template <class T>
pair<T*, ptrdiff_t>
get_temporary_buffer(ptrdiff_t n)
{
	pair<T*, ptrdiff_t> result(0, 0);
	while (n > 0)
	{
		result.first = reinterpret_cast<T*>(new(nothrow) char [sizeof(T)*n]);
		if (result.first != 0)
		{
			result.second = n;
			break;
		}
		else
			n /= 2;
	}
	return result;
}

template <class T>
inline
void
return_temporary_buffer(T* p)
{
	delete [] reinterpret_cast<char*>(p);
}

// hh 980601  Added non-standard class.  This facilitates use of
//            get_temporary_buffer in a exception-safe manner.
//            Used in <algorithm>
template <class T>
class _TempVec
{
public:
	// types:
	typedef T&                                     reference;
//	typedef const T&                               const_reference;
	class                                          iterator;
//	class                                          const_iterator;
	typedef size_t                                 size_type;
	typedef ptrdiff_t                              difference_type;
	typedef T                                      value_type;
	typedef T*                                     pointer;
//	typedef const T*                               const_pointer;
//	typedef _STD::reverse_iterator<iterator>       reverse_iterator;
//	typedef _STD::reverse_iterator<const_iterator> const_reverse_iterator;

	class proxy
	{
	public:
		reference operator = (const T& rhs) const
		{
			if (cur_ - vec_->data_ < vec_->size_)
				*cur_ = rhs;
			else
			{
				new (cur_) T(rhs);
				++vec_->size_;
			}
			return *cur_;
		}

		operator const T& () const {return *cur_;}

		friend bool operator == (const proxy& x, const proxy& y) {return *x.cur_ == *y.cur_;}
		friend bool operator == (const proxy& x, const T& y) {return *x.cur_ == y;}
		friend bool operator == (const T& x, const proxy& y) {return x == *y.cur_;}

		friend bool operator != (const proxy& x, const proxy& y) {return *x.cur_ != *y.cur_;}
		friend bool operator != (const proxy& x, const T& y) {return *x.cur_ != y;}
		friend bool operator != (const T& x, const proxy& y) {return x != *y.cur_;}

		friend bool operator < (const proxy& x, const proxy& y) {return *x.cur_ < *y.cur_;}
		friend bool operator < (const proxy& x, const T& y) {return *x.cur_ < y;}
		friend bool operator < (const T& x, const proxy& y) {return x < *y.cur_;}

		friend bool operator <= (const proxy& x, const proxy& y) {return *x.cur_ <= *y.cur_;}
		friend bool operator <= (const proxy& x, const T& y) {return *x.cur_ <= y;}
		friend bool operator <= (const T& x, const proxy& y) {return x <= *y.cur_;}

		friend bool operator > (const proxy& x, const proxy& y) {return *x.cur_ > *y.cur_;}
		friend bool operator > (const proxy& x, const T& y) {return *x.cur_ > y;}
		friend bool operator > (const T& x, const proxy& y) {return x > *y.cur_;}

		friend bool operator >= (const proxy& x, const proxy& y) {return *x.cur_ >= *y.cur_;}
		friend bool operator >= (const proxy& x, const T& y) {return *x.cur_ >= y;}
		friend bool operator >= (const T& x, const proxy& y) {return x >= *y.cur_;}

	private:
		T* cur_;
		_TempVec<T>* vec_;

		proxy(T* cur, _TempVec<T>* vec) : cur_(cur), vec_(vec) {}

		friend class iterator;
	};
	friend class proxy;

	class iterator
		: public _STD::iterator<random_access_iterator_tag, T, ptrdiff_t, T*, T&>
	{
	public:
		iterator() {}
		proxy operator * () const {return proxy(cur_, vec_);}
		pointer operator -> () const {return cur_;}
		iterator& operator ++ () {++cur_; return *this;}
		iterator operator ++ (int) {iterator tmp(*this); ++(*this); return tmp;}
		iterator& operator -- () {--cur_; return *this;}
		iterator operator -- (int) {iterator tmp(*this); --(*this); return tmp;}
		iterator& operator += (difference_type n) {cur_ += n; return *this;}
		iterator operator + (difference_type n) const {return iterator(*this) += n;}
		iterator& operator -= (difference_type n) {cur_ -= n; return *this;}
		iterator operator - (difference_type n) const {return iterator(*this) -= n;}
		difference_type operator - (const iterator& rhs) const {return difference_type(cur_ - rhs.cur_);}
		proxy operator [] (size_type i) const {iterator tmp(*this); tmp += difference_type(i); return *tmp;}
		friend bool operator ==(const iterator& x, const iterator& y) {return x.cur_ == y.cur_;}
		friend bool operator !=(const iterator& x, const iterator& y) {return x.cur_ != y.cur_;}
		friend bool operator < (const iterator& x, const iterator& y) {return x.cur_ < y.cur_;}
		friend bool operator <=(const iterator& x, const iterator& y) {return x.cur_ <= y.cur_;}
		friend bool operator > (const iterator& x, const iterator& y) {return x.cur_ >  y.cur_;}
		friend bool operator >=(const iterator& x, const iterator& y) {return x.cur_ >= y.cur_;}
		friend iterator operator + (difference_type n, const iterator& rhs)
			{return iterator(rhs) += n;}
	private:
		pointer cur_;
		_TempVec<T>* vec_;

		iterator(T* cur, _TempVec<T>* vec) : cur_(cur), vec_(vec) {}

		friend class _TempVec<T>;
	};

	_TempVec(ptrdiff_t cap);
	~_TempVec();
	iterator begin();
	iterator end();
	ptrdiff_t size() const;
	ptrdiff_t capacity() const;
private:
	ptrdiff_t cap_;
	ptrdiff_t size_;
	T* data_;

	_TempVec(const _TempVec&);             // Not defined
	_TempVec& operator=(const _TempVec&);  // Not defined
};

template <class T>
inline
_TempVec<T>::_TempVec(ptrdiff_t cap)
{
	pair<T*, ptrdiff_t> buf = get_temporary_buffer<T>(cap);
	data_ = buf.first;
	cap_ = buf.second;
	size_ = 0;
}

template <class T>
inline
_TempVec<T>::~_TempVec()
{
	__destroy(data_, data_ + size_);
	return_temporary_buffer(data_);
}

template <class T>
inline
typename _TempVec<T>::iterator
_TempVec<T>::begin()
{
	return iterator(data_, this);
}

template <class T>
inline
typename _TempVec<T>::iterator
_TempVec<T>::end()
{
	return iterator(data_ + cap_, this);
}

template <class T>
inline
ptrdiff_t
_TempVec<T>::size() const
{
	return size_;
}

template <class T>
inline
ptrdiff_t
_TempVec<T>::capacity() const
{
	return cap_;
}

//  lib.specialized.algorithms, specialized algorithms:

// uninitialized_copy

template <class InputIterator, class ForwardIterator>
inline
ForwardIterator
uninitialized_copy(InputIterator first, InputIterator last, ForwardIterator result)
{
	ForwardIterator save = result;
#ifndef _MSL_NO_EXCEPTIONS
	try
	{
#endif
		for (; first != last; ++result, ++first)
			new (&*result) typename iterator_traits<ForwardIterator>::value_type(*first);
#ifndef _MSL_NO_EXCEPTIONS
	}
	catch (...)
	{
		__destroy(save, result);
		throw;
	}
#endif
	return result;
}

template <>
inline
char*
uninitialized_copy(char* first, char* last, char* result)
{
	size_t len = static_cast<size_t>(last - first);
	memcpy(result, first, len);
	return result + len;
}

#ifndef _MSL_NO_WCHART

	template <>
	inline
	wchar_t*
	uninitialized_copy(wchar_t* first, wchar_t* last, wchar_t* result)
	{
		size_t len = static_cast<size_t>(last - first);
		wmemcpy(result, first, len);
		return result + len;
	}

#endif

// uninitialized_fill

template <class ForwardIterator, class T>
inline
void
uninitialized_fill(ForwardIterator first, ForwardIterator last, const T& x)
{
	ForwardIterator save = first;
#ifndef _MSL_NO_EXCEPTIONS
	try
	{
#endif
		for (; first != last; ++first)
			new (&*first) typename iterator_traits<ForwardIterator>::value_type(x);
#ifndef _MSL_NO_EXCEPTIONS
	}
	catch (...)
	{
		__destroy(save, first);
		throw;
	}
#endif
}

template <>
inline
void
uninitialized_fill(char* first, char* last, const char& x)
{
	memset(first, x, static_cast<size_t>(last - first));
}

#ifndef _MSL_NO_WCHART

	template <>
	inline
	void
	uninitialized_fill(wchar_t* first, wchar_t* last, const wchar_t& x)
	{
		wmemset(first, x, static_cast<size_t>(last - first));
	}

#endif

// uninitialized_fill_n

template <class ForwardIterator, class Size, class T>
inline
void
uninitialized_fill_n(ForwardIterator first, Size n, const T& x)
{
	ForwardIterator save = first;
#ifndef _MSL_NO_EXCEPTIONS
	try
	{
#endif
		for (; n--; ++first)
			new (&*first) typename iterator_traits<ForwardIterator>::value_type(x);
#ifndef _MSL_NO_EXCEPTIONS
	}
	catch (...)
	{
		__destroy(save, first);
		throw;
	}
#endif
}

template <>
inline
void
uninitialized_fill_n(char* first, size_t n, const char& x)
{
	memset(first, x, n);
}

#ifndef _MSL_NO_WCHART

	template <>
	inline
	void
	uninitialized_fill_n(wchar_t* first, size_t n, const wchar_t& x)
	{
		wmemset(first, x, n);
	}

#endif

//  lib.auto.ptr, pointers:

#ifndef _MSL_USE_AUTO_PTR_96

// hh 980103 Nov. '97 version of auto_ptr added
// hh 980805 member template operators not supported yet.

template<class X> class auto_ptr;

template <class Y>
struct auto_ptr_ref
{
	auto_ptr<Y>& p_;
	auto_ptr_ref(const auto_ptr<Y>& a);
};

template <class Y>
inline
auto_ptr_ref<Y>::auto_ptr_ref(const auto_ptr<Y>& a)
	: p_(const_cast<auto_ptr<Y>&>(a))
{
}

template<class X>
class auto_ptr
{
public:
	typedef X element_type;

	//  lib.auto.ptr.cons construct/copy/destroy:
	explicit auto_ptr(X* p = 0) _MSL_THROW;
	auto_ptr(auto_ptr& a) _MSL_THROW;
#ifndef _MSL_NO_MEMBER_TEMPLATE
#ifndef _MSL_MUST_INLINE_MEMBER_TEMPLATE
	template<class Y> auto_ptr(auto_ptr<Y>& a) _MSL_THROW;
#else
	template<class Y>
	inline
	auto_ptr(auto_ptr<Y>& a) _MSL_THROW
		: ptr_(a.release())
	{
	}
#endif
#endif
	auto_ptr& operator=(auto_ptr& a) _MSL_THROW;
#ifndef _MSL_NO_MEMBER_TEMPLATE
#ifndef _MSL_MUST_INLINE_MEMBER_TEMPLATE
	template<class Y> auto_ptr& operator=(auto_ptr<Y>& a) _MSL_THROW;
#else
	template<class Y>
	inline
	auto_ptr&
	operator=(auto_ptr<Y>& a) _MSL_THROW
	{
		reset(a.release());
		return *this;
	}
#endif
#endif
	~auto_ptr() _MSL_THROW;

	//  lib.auto.ptr.members members:
	X& operator*() const _MSL_THROW;
	X* operator->() const _MSL_THROW;
	X* get() const _MSL_THROW;
	X* release() _MSL_THROW;
	void reset(X* p = 0) _MSL_THROW;

	//  lib.auto.ptr.conv conversions:
	auto_ptr(auto_ptr_ref<X> r) _MSL_THROW;
	auto_ptr& operator=(auto_ptr_ref<X> r) _MSL_THROW;
#if !defined (_MSL_NO_MEMBER_TEMPLATE) && __MWERKS__ > 0x2400
	template<class Y> operator auto_ptr_ref<Y>() _MSL_THROW;
	template<class Y> operator auto_ptr<Y>() _MSL_THROW;
#endif
private:
	X* ptr_;
};

template<class X>
inline
X*
auto_ptr<X>::release() _MSL_THROW
{
	X* tmp = ptr_;
	ptr_ = 0;
	return tmp;
}

template<class X>
inline
void
auto_ptr<X>::reset(X* p) _MSL_THROW
{
	if (ptr_)
		delete ptr_;
	ptr_ = p;
}

template<class X>
inline
auto_ptr<X>::auto_ptr(X* p) _MSL_THROW
	: ptr_(p)
{
}

template<class X>
inline
auto_ptr<X>::auto_ptr(auto_ptr& a) _MSL_THROW
	: ptr_(a.release())
{
}

#ifndef _MSL_NO_MEMBER_TEMPLATE
#ifndef _MSL_MUST_INLINE_MEMBER_TEMPLATE

	template<class X>
	template<class Y>
	inline
	auto_ptr<X>::auto_ptr(auto_ptr<Y>& a) _MSL_THROW
		: ptr_(a.release())
	{
	}

#endif
#endif

template<class X>
inline
auto_ptr<X>&
auto_ptr<X>::operator=(auto_ptr& a) _MSL_THROW
{
	reset(a.release());
	return *this;
}

#ifndef _MSL_NO_MEMBER_TEMPLATE
#ifndef _MSL_MUST_INLINE_MEMBER_TEMPLATE

	template<class X>
	template<class Y>
	inline
	auto_ptr<X>&
	auto_ptr<X>::operator=(auto_ptr<Y>& a) _MSL_THROW
	{
		reset(a.release());
		return *this;
	}

#endif
#endif

template<class X>
inline
auto_ptr<X>::~auto_ptr() _MSL_THROW
{
	delete ptr_;
}

template<class X>
inline
X&
auto_ptr<X>::operator*() const _MSL_THROW
{
	return *ptr_;
}

template<class X>
inline
X*
auto_ptr<X>::operator->() const _MSL_THROW
{
	return ptr_;
}

template<class X>
inline
X*
auto_ptr<X>::get() const _MSL_THROW
{
	return ptr_;
}

template<class X>
inline
auto_ptr<X>::auto_ptr(auto_ptr_ref<X> r) _MSL_THROW
	: ptr_(r.p_.release())
{
}

template<class X>
inline
auto_ptr<X>&
auto_ptr<X>::operator=(auto_ptr_ref<X> r) _MSL_THROW
{
	reset(r.p_.release());
	return *this;
}

#if !defined (_MSL_NO_MEMBER_TEMPLATE) && __MWERKS__ > 0x2400

	template<class X>
	template<class Y>
	inline
	auto_ptr<X>::operator auto_ptr_ref<Y>() _MSL_THROW
	{
		return *this;
	}

	template<class X>
	template<class Y>
	inline
	auto_ptr<X>::operator auto_ptr<Y>() _MSL_THROW
	{
		return auto_ptr<Y>(release());
	}

#endif

#else // _MSL_USE_AUTO_PTR_96

// hh 980103 Dec. '96 version of auto_ptr fixed

template <class X>
class auto_ptr
{
public :
	typedef X   element_type;
	explicit auto_ptr(X* p = 0) _MSL_THROW;
#ifndef _MSL_NO_MEMBER_TEMPLATE
	template <class Y> auto_ptr (const auto_ptr<Y>& a) _MSL_THROW;
	template <class Y> auto_ptr& operator=(const auto_ptr<Y>& a) _MSL_THROW;
#endif
	auto_ptr (const auto_ptr& a) _MSL_THROW;
	auto_ptr& operator=(const auto_ptr& a) _MSL_THROW;
	~auto_ptr();
	X& operator*() const _MSL_THROW;
	X* operator->() const _MSL_THROW;
	X* get() const _MSL_THROW;
	X* release() const _MSL_THROW;
	bool owns() const _MSL_THROW;
private:
	X* ptr_;
	mutable bool owns_;
};

template <class X>
inline
bool
auto_ptr<X>::owns() const _MSL_THROW
{
	return owns_;
}

template <class X>
inline
X*
auto_ptr<X>::release() const _MSL_THROW
{
	owns_ = false;
	return ptr_;
}

template <class X>
inline
auto_ptr<X>::auto_ptr(X* p) _MSL_THROW
	: ptr_(p),
	  owns_(p != 0)
{
}

#ifndef _MSL_NO_MEMBER_TEMPLATE

	template <class X>
	template <class Y>
	inline
	auto_ptr<X>::auto_ptr(const auto_ptr<Y>& a) _MSL_THROW
	{
		owns_ = a.owns();
		ptr_ = a.release();
	}

	template <class X>
	template <class Y>
	inline
	auto_ptr<X>&
	auto_ptr<X>::operator=(const auto_ptr<Y>& a) _MSL_THROW
	{
		if (owns_)
			delete ptr_;
		owns_ = a.owns();
		ptr_ = a.release();
		return *this;
	}

#endif

template <class X>
inline
auto_ptr<X>::auto_ptr(const auto_ptr& a) _MSL_THROW
{
	owns_ = a.owns_;
	ptr_ = a.release();
}

// hh 980923 rewrote op=
template <class X>
auto_ptr<X>&
auto_ptr<X>::operator=(const auto_ptr& a) _MSL_THROW
{
	if (this == &a)
		return *this;
	if (ptr_ == a.ptr_)
	{
		owns_ = owns_ || a.owns_;
		ptr_ = a.release();
	}
	else
	{
		if (owns_)
			delete ptr_;
		owns_ = a.owns_;
		ptr_ = a.release();
	}
	return *this;
}

template <class X>
inline
auto_ptr<X>::~auto_ptr()
{
	if (owns_)
		delete ptr_;
}

template <class X>
inline
X&
auto_ptr<X>::operator* () const _MSL_THROW
{
	return *ptr_;
}

template <class X>
inline
X*
auto_ptr<X>::operator-> () const _MSL_THROW
{
	return ptr_;
}

template <class X>
inline
X*
auto_ptr<X>::get() const _MSL_THROW
{
	return ptr_;
}

#endif // _MSL_USE_AUTO_PTR_96

// Warning, non-standard
// Credit:  Nathan C. Myers

template <class _Base, class _Member>
struct _EmptyMemberOpt
	: public _Base
{
	_EmptyMemberOpt();
	_EmptyMemberOpt(_Base const& __b);
	_EmptyMemberOpt(_Base const& __b, _Member const& __mem);

	_Member m_;
};

template <class _Base, class _Member>
inline
_EmptyMemberOpt<_Base, _Member>::_EmptyMemberOpt()
	: _Base(_Base()),
	  m_(_Member())
{
}

template <class _Base, class _Member>
inline
_EmptyMemberOpt<_Base, _Member>::_EmptyMemberOpt(_Base const& __b)
	: _Base(__b),
	  m_(_Member())
{
}

template <class _Base, class _Member>
inline
_EmptyMemberOpt<_Base, _Member>::_EmptyMemberOpt(_Base const& __b, _Member const& __mem)
	: _Base(__b),
	  m_(__mem)
{
}

#ifndef _MSL_NO_CPP_NAMESPACE
	} // namespace std
#endif

#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import reset
#endif
#pragma options align=reset

#endif // RC_INVOKED

#endif // _MEMORY

// hh 971220 fixed MOD_INCLUDE and MOD_C_INCLUDE
// hh 971222 made include guards standard
// hh 971222 added alignment wrapper
// hh 971227 silence unused warning
// hh 971227 removed unused argument from deallocate
// hh 971230 added RC_INVOKED wrapper
// hh 980103 fixed Dec. '96 version of auto_ptr
// hh 980103 added Nov. '97 version of auto_ptr
// hh 980106 removed #include <stdexcept>
//           <memory> can not throw a stdexecpt because it can not process strings.
// hh 980106 removed null pointer checks from allocate::construct and destroy
// hh 980522  Rewrote get/return_temporary_buffer because of concerns about
//            multi-threading.
// hh 980730 added (char*) cast to return_temporary_buffer
// hh 980805 member template operators not supported yet.
//           modified auto_ptr '97 to pre-member template functionality
// hh 980902 #ifdef'd out exception code when ndef MSIPL_EXCEPT
// hh 980923 fixed bug in the '96 auto_ptr::op=
// hh 981220 Added typename to appropriate return types
// hh 990315 Split destroy(first, last) into two methods to help compiler optimize
//           away empty loops.
// hh 990503 Rewrote.



#endif	//_staticmem_h













