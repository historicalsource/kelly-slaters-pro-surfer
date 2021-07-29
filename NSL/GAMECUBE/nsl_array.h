#ifndef _NSL_ARRAY_H
#define _NSL_ARRAY_H

#include <stdlib.h>

#ifndef NSL_NO_ASSERT_H
#include <assert.h>
#endif

#include "nl_gc.h"

template<class T>
class nsl_array
{
public:
	typedef nlUint32 size_type;

	typedef void* (*nsl_array_allocator)( size_type n, size_type align );
	typedef void (*nsl_array_deallocator)( void* p );

private:
  T* _data;        // array data
  size_type _size; // size of array
  nsl_array_allocator _allocate;
  nsl_array_deallocator _deallocate;

  size_type _start; // 'front' of array data
  size_type _end;   // 'back' of array data
  size_type _count; // number of items in array

public:
	nsl_array( )
		: _data( 0 ),	_size( 0 ),
			_allocate( 0 ), _deallocate( 0 ),
			_start( 0 ), _end( 0 ), _count( 0 )
	{
		// clear( ) would be useless
	}
	~nsl_array( )
	{
		_deallocate( _data );
	}

	void set_alloc_funcs( nsl_array_allocator _a, nsl_array_deallocator _d )
	{
		_allocate = _a;
		_deallocate = _d;
	}
	void init( size_type n )
	{
		_deallocate( _data );
		_data = (T*) _allocate( 2048, 32 );
		_size = n;
		clear( );
	}
  void clear( void )
	{
	  _start = 0;
	  _end = 0;
	  _count = 0;
	}
	void release( void )
	{
		clear( );
		_deallocate( _data );
		_data = 0;
		_size = 0;
	}

  size_type push( const T& _t )
  {

		if( _count >= _size ) {
			return 0;
		}

		_data[_end] = _t;
		++_count;
		++_end;

		if( _end >= _size ) {
			_end = 0;
		}

		return _count;
	}
  T pop( void )
  {

		if( _count <= 0 ) {
			return T( );
		}

		T _t = _data[_start];
		--_count;
		++_start;
		
		if( _start >= _size ) {
			_start = 0;		
		}

		return _t;
  }

  size_type find( const T& _t )
  {
 		size_type _where = _start;
 		
 		while( _where != _end ) {

			if( _data[_where] == _t ) {
				return _where;
			}

			++_where;

			if( _where > _size ) {
				_where = 0;
			}

 		}

		return _end;
  }
  void erase( const T& _t )
  {
  	size_type _where = find( _t );
  	
  	if( _where == _end ) {
			return;
  	}

		if( _end == 0 ) {
			_end = _size;		
		} else {
			--_end;
		}

		_data[_where] = _data[_end];

		--_count;
  }
	T operator[]( int n )
	{
		int idx = _start + n;

		if( idx > _size ) {
			idx = n - ( _size - _start );
		}

		return _data[idx];
	}

  bool empty( void ) const { return ( _count > 0 ); }
  size_type size( void ) const { return _count;}
  size_type remaining( void ) const { return ( _size - _count ); }
};

#endif