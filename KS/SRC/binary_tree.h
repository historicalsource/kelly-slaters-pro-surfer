// binary_tree.h
//
// Copyright (c) 1998 Treyarch Invention LLC.  ALL RIGHTS RESERVED.
//
#ifndef _BINARY_TREE_H
#define _BINARY_TREE_H


#include <stddef.h>
#include <memory>
#include "vector_alloc.h"
#include "stl_adapter.h"

  #undef _K
  #undef _V
  #undef _P
  #undef _N
  #undef _X
  #undef _Y

// TEMPLATE CLASS binary_tree

template <class _K>
class binary_tree_node
  {
  public:
  	// constructor
  	binary_tree_node<_K>() {}

    typedef void * _Genptr;
    _Genptr _Left, _Right;
    _K _Val;
  };

template <class _K>
class binary_tree
  {
  // Types
  public:

    typedef void * _Genptr;
    typedef binary_tree_node<_K> _Node;
    typedef _Node * _Nodeptr;
    typedef _Nodeptr & _Nodepref;
    typedef _K & _Vref;

    typedef binary_tree<_K> _Myt;
    typedef _K value_type;
    typedef size_t size_type;
    typedef size_t difference_type;

    typedef _K * _Tptr;
    typedef const _K * _Ctptr;
    typedef _K & reference;
    typedef const _K & const_reference;

    // CLASS iterator
    class iterator;
    friend class iterator;
    class iterator : public bidirectional_iterator<_K,difference_type
#ifdef TARGET_XBOX
		, _Ctptr, const_reference
#endif
	>
      {
      // Data
      protected:
        _Nodeptr _Ptr;

      // Constructors
      public:
        iterator() {}
        iterator(const iterator& _X) : _Ptr(_X._Ptr) {}
        iterator(_Nodeptr _R) : _Ptr(_R) {}

      // Operators
      public:
        reference operator*() const { return (_Value(_Ptr)); }
        _Tptr operator->() const    { return (&**this); }

        iterator& operator++()
          {
          if (_Ptr)
            _Inc();
          return (*this);
          }

        iterator operator++(int)
          {
          iterator _Tmp = *this;
          if (_Ptr)
            ++*this;
          return (_Tmp);
          }

        iterator& operator--()
          {
          if (_Ptr)
            _Dec();
          return (*this);
          }

        iterator operator--(int)
          {
          iterator _Tmp = *this;
          if (_Ptr)
            --*this;
          return (_Tmp);
          }

        bool operator==(const iterator& _X) const { return (_Ptr == _X._Ptr); }
        bool operator!=(const iterator& _X) const { return (!(*this == _X)); }
        _Nodeptr _Mynode() const { return (_Ptr); }

      // Methods
      public:
        // DEBUG OUTPUT
        void print() const
          {
#if defined(__DEBUGOUTPUT__H)
          DebugOutput::printline("node(0x%p)",_Ptr);
          DebugOutput::indent();
          DebugOutput::printline("L(0x%p)",_Left(_Ptr));
          DebugOutput::printline("R(0x%p)",_Right(_Ptr));
          _Value(_Ptr).print();
          DebugOutput::unindent();
#endif
          }

      // Internal Methods
      protected:
        void _Inc() { _Ptr = _Right(_Ptr); };
        void _Dec() { _Ptr = _Left(_Ptr); };
      };

    // CLASS reverse_iterator
    class reverse_iterator;
    friend class reverse_iterator;
    class reverse_iterator : public iterator
      {
      // Constructors
      public:
        reverse_iterator() {}
        reverse_iterator(const reverse_iterator& _X) : iterator(_X) {}
        reverse_iterator(_Nodeptr _R) : iterator(_R) {}

      // Operators
      public:
        reverse_iterator& operator++()
          {
          if (_Ptr)
            _Dec();
          return (*this);
          }

        reverse_iterator operator++(int)
          {
          reverse_iterator _Tmp = *this;
          if (_Ptr)
            ++*this;
          return (_Tmp);
          }

        reverse_iterator& operator--()
          {
          if (_Ptr)
            _Inc();
          return (*this);
          }

        reverse_iterator operator--(int)
          {
          reverse_iterator _Tmp = *this;
          if (_Ptr)
            --*this;
          return (_Tmp);
          }
      };

    // CLASS const_iterator
    class const_iterator;
    friend class const_iterator;
    class const_iterator : public iterator
      {
      // Constructors
      public:
        const_iterator() {}
        const_iterator(const const_iterator& _X) : iterator(_X) {}
        const_iterator(_Nodeptr _R) : iterator(_R) {}

      // Operators
      public:
        const_reference operator*() const { return (_Value(_Ptr)); }
        _Ctptr operator->() const         { return (&**this); }

        const_iterator& operator++()
          {
          if (_Ptr)
            _Inc();
          return (*this);
          }

        const_iterator operator++(int)
          {
          const_iterator _Tmp = *this;
          if (_Ptr)
            ++*this;
          return (_Tmp);
          }

        const_iterator& operator--()
          {
          if (_Ptr)
            _Dec();
          return (*this);
          }

        const_iterator operator--(int)
          {
          const_iterator _Tmp = *this;
          if (_Ptr)
            --*this;
          return (_Tmp);
          }

        bool operator==(const const_iterator& _X) const { return (_Ptr == _X._Ptr); }
        bool operator!=(const const_iterator& _X) const { return (!(*this == _X)); }
      };

    // CLASS const_reverse_iterator
    class const_reverse_iterator;
    friend class const_reverse_iterator;
    class const_reverse_iterator : public const_iterator
      {
      // Constructors
      public:
        const_reverse_iterator() {}
        const_reverse_iterator(const const_reverse_iterator& _X) : const_iterator(_X) {}
        const_reverse_iterator(_Nodeptr _R) : const_iterator(_R) {}

      // Operators
      public:
        const_reverse_iterator& operator++()
          {
          if (_Ptr)
            _Dec();
          return (*this);
          }

        const_reverse_iterator operator++(int)
          {
          const_reverse_iterator _Tmp = *this;
          if (_Ptr)
            ++*this;
          return (_Tmp);
          }

        const_reverse_iterator& operator--()
          {
          if (_Ptr)
            _Inc();
          return (*this);
          }

        const_reverse_iterator operator--(int)
          {
          const_reverse_iterator _Tmp = *this;
          if (_Ptr)
            --*this;
          return (_Tmp);
          }
      };

    typedef pair<iterator,bool> _Pairib;
    typedef pair<iterator,iterator> _Pairii;
    typedef pair<const_iterator,const_iterator> _Paircc;

  // Data
  protected:
    vector_allocator<_Node> allocator;
    _Nodeptr _Head;
    size_type _Size;

  // Constructors
  public:
    binary_tree()
      {
      _Init();
      }

    binary_tree(const _Myt& _X)
      {
      _Init();
      _Copy(_X);
      }

    binary_tree(const _K& _V)
      {
      _Init(_V);
      }

    ~binary_tree()
      {
      clear();
      }

  protected:
    binary_tree(_Nodeptr _X)
      {
      _Init();
      _Root() = _Copy(_X);
      }

  // Operators
  public:
    _Myt& operator=(const _Myt& _X)
      {
      if (this != &_X)
        {
        clear();
        _Copy(_X);
        }
      return (*this);
      }

  // Methods
  public:
    size_type size() const         { return (_Size); }
    bool empty() const             { return (size() == 0); }

    void reserve(int n)            { allocator.reserve(n); }

    void swap(_Myt& _X)
      {
      _Myt _Ts = *this; *this = _X,_X = _Ts;
      }

    friend void swap(_Myt& _X,_Myt& _Y) { _X.swap(_Y); }

  // Internal Methods
#ifndef __GNUC__
  protected:
#else
  public: // gnuc doesn't like these methods being 'protected'
#endif
    static inline _Nodepref _Left(_Nodeptr _P)   { return _Nodepref((*_P)._Left); }
    static inline _Nodepref _Right(_Nodeptr _P)  { return _Nodepref((*_P)._Right); }
    static inline _Vref _Value(_Nodeptr _P)      { return _Vref((*_P)._Val); }

  protected:

    static _Nodeptr _Min(_Nodeptr _P)
      {
      while (_Left(_P))
        _P = _Left(_P);
      return (_P);
      }

    static _Nodeptr _Max(_Nodeptr _P)
      {
      while (_Right(_P))
        _P = _Right(_P);
      return (_P);
      }

    static _Nodeptr _MinLeaf(_Nodeptr _P)
      {
      _P = _Min(_P);
      while (_Right(_P))
        _P = _Min(_Right(_P));
      return _P;
      }

    static _Nodeptr _MaxLeaf(_Nodeptr _P)
      {
      _P = _Max(_P);
      while (_Left(_P))
        _P = _Max(_Left(_P));
      return _P;
      }

    _Nodepref _Root()       { return _Nodepref(_Head); }
    _Nodepref _Root() const { return _Nodepref(_Head); }

    void _Init()
      {
      _Root() = NULL;
      _Size = 0;
      }

    void _Init(const _K& _V)
      {
      _Init();
      _Root() = _Buynode(_V);
      _Size = 1;
      }

    void _Copy(const _Myt& _X)
      {
      _Size = 0;
      _Root() = _Copy(_X._Root());
      }

    _Nodeptr _Copy(_Nodeptr _X)
      {
      if (_X)
        {
        _Nodeptr _R = _Buynode();
        _Size++;
        _Left(_R) = _Copy(_Left(_X));
        _Right(_R) = _Copy(_Right(_X));
        _Consval(&_Value(_R),_Value(_X));
        return _R;
        }
      else
        return NULL;
      }

    _Nodeptr _AddLeft(_Nodeptr _P,const _K& _V)
      {
      _Nodeptr _N = _Buynode(_V);
      _Left(_P) = _N;
      _Size++;
      return _N;
      }

    _Nodeptr _AddRight(_Nodeptr _P,const _K& _V)
      {
      _Nodeptr _N = _Buynode(_V);
      _Right(_P) = _N;
      _Size++;
      return _N;
      }

    void _Erase(_Nodeptr _X)
      {
      for (_Nodeptr _Y=_X; _Y; _X=_Y)
        {
        _Erase(_Right(_Y));
        _Y = _Left(_Y);
        _Destval(&_Value(_X));
        _Freenode(_X);
        }
      }

    _Nodeptr _Buynode()
      {
      _Nodeptr _N = (_Nodeptr)allocator.allocate(1);  // see stl_adapter.h
      _Left(_N) = _Right(_N) = NULL;
      return _N;
      }

    _Nodeptr _Buynode(const _K& _V)
      {
      _Nodeptr _N = _Buynode();
      _Consval(&_Value(_N),_V);
      return _N;
      }

    void _Consval(_Tptr _P,const _K& _V) { new((void *)_P) _K(_V); }
    void _Destval(_Tptr _P)              { (*_P).~_K(); }
    void _Freenode(_Nodeptr _N)          { allocator.deallocate(_N,1);}

  public:
    void clear()
      {
      _Erase(_Root());
      _Root() = NULL;
      _Size = 0;
      allocator.clear();
      }

  };


#endif  // _BINARY_TREE_H
