/*-------------------------------------------------------------------------------------------------------
  VECTOR_ALLOC.H 
  
    Simple vector based allocator - helps reduce alloced block counts, but doesn't support deallocation
    of individual blocks.
  
-------------------------------------------------------------------------------------------------------*/
#ifndef VECTOR_ALLOC_H
#define VECTOR_ALLOC_H


template<class T> class vector_allocator 
  {
  public:
    vector_allocator()                                      { i=0; }
    void clear()                                            { v.clear(); }
    void reserve(size_t n)                                  { v.reserve(n); }
	  void *allocate(size_t n)                                
      { 
      assert(i<(int)v.capacity()); 
      v.push_back(T()); 
      return &(v[i++]); 
      }
    void  deallocate(void *p, size_t n)                     {}
	  void *reallocate(void *p, size_t old_sz, size_t new_sz) { assert(false); return 0; }
  private:
    vector<T> v;
    int i;
};


#endif