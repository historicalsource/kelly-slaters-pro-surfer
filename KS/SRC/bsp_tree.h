// bsp_tree.h
//
// Copyright (c) 1998 Treyarch Invention LLC.  ALL RIGHTS RESERVED. 
//
#ifndef _BSP_TREE_H
#define _BSP_TREE_H

#include "algebra.h"
#include "bp_tree.h"
#include "hyperplane.h"
#include <stack>
#include <memory>
#include <iterator>

class sector;

#include "debug.h"

// TEMPLATE CLASS bsp_tree
// The Binary Space Partitioning Tree applies the Binary Partitioning Tree to
// N-dimensional spaces by the use of (N-1)-dimensional hyperplanes.  This
// class introduces the notion of visibility, providing iterators for ordering
// visible tree nodes based on viewpoint.
// NOTE: Since the partition and portion types (as well as the comparison
// method operator<(const _Portion&,const _Partition&)) are supplied by the
// user, their proper relationship as (N-1)-dimensional hyperplane and
// N-dimensional space must be guaranteed by the user.
class foo {};

template <class _Partition,class _Portion>
class bsp_tree : public bp_tree<_Partition,_Portion>
{
  // Types
  public:
    typedef bsp_tree<_Partition,_Portion> _Myt;
    typedef bp_tree<_Partition,_Portion> _Tr;
		typedef _Tr::value_type value_type;
		typedef _Tr::difference_type difference_type;
		typedef _Tr::_Nodeptr _Nodeptr;
		typedef _Tr::branch branch;
		typedef _Tr::reference reference;
		typedef _Tr::const_reference const_reference;
		typedef _Tr::_Tptr _Tptr;
		typedef _Tr::_Ctptr _Ctptr;

    // CLASS visibility_iterator
    // Provides a forward-only near-to-far iteration through the visible nodes of a subtree.
    class visibility_iterator;
    friend class visibility_iterator;
    typedef bidirectional_iterator<bp_tree<_Partition,_Portion>::value_type,bp_tree<_Partition,_Portion>::difference_type
#ifdef TARGET_XBOX
		, _Ctptr, const_reference
#endif
		> bidirectional_iterator_value_diff_t;

    class visibility_iterator : public bidirectional_iterator_value_diff_t
      {
      // Data
      protected:
        _Nodeptr _Ptr;
        _Portion _Key;
        stack<branch> _Bstack;

      // Constructors
      public:
        visibility_iterator() : _Ptr(NULL) {}

        visibility_iterator(const visibility_iterator& _X)
        :   _Ptr(_X._Ptr),
            _Key(_X._Key),
            _Bstack(_X._Bstack)
          {
          }

        visibility_iterator(_Nodeptr _P,const _Portion& _K)
        :   _Key(_K)
          {
          _push_to_visible_leaf(_P);
          }

      protected:
        visibility_iterator(const _Portion& _K)
        :   _Key(_K)
          {
          }

      // Methods
      public:
        reference operator*() const {return (_Value(_Ptr)); }
        _Tptr operator->() const    {return (&**this); }

        visibility_iterator& operator++()
          {
          if (_Ptr)
            _Inc();
          return (*this);
          }

        visibility_iterator operator++(int)
          {
          visibility_iterator _Tmp = *this;
          if (_Ptr)
            ++*this;
          return (_Tmp);
          }

        bool operator==(const visibility_iterator& _X) const { return (_Ptr == _X._Ptr); }
        bool operator!=(const visibility_iterator& _X) const { return (!(*this == _X)); }
        _Nodeptr _Mynode() const { return (_Ptr); }

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
        void _Inc()
          {
          if (_Left(_Ptr))
            _push_to_visible_leaf(_Left(_Ptr));
          else
            _pop_to_visible_leaf();
          }

        void _push_to_visible_leaf(_Nodeptr _P)
          {
          while (_P)
            {
            branch b(_P,_Key);
            if (b.child == branch::LEFT)
              {
              if (_Left(_P))
                {
                if (_Right(_P))
                  _Bstack.push(b);
                _P = _Left(_P);
                }
              else
                _P = _Right(_P);
              }
            else
              {
              _Bstack.push(b);
              _P = _Right(_P);
              }
            }
          _pop_to_visible_leaf();
          }

        void _pop_to_visible_leaf()
          {
          _Ptr = NULL;
          if (!_Bstack.empty())
            {
            branch b = _Bstack.top();
            _Bstack.pop();
            if (b.child == branch::LEFT)
              _push_to_visible_leaf(_Right(b.parent._Mynode()));
            else
              _Ptr = b.parent._Mynode();
            }
          }
      };

    // CLASS const_visibility_iterator
    class const_visibility_iterator;
    friend class const_visibility_iterator;
    class const_visibility_iterator : public visibility_iterator
      {
      // Constructors
      public:
        const_visibility_iterator()
        :   visibility_iterator()
          {
          }

        const_visibility_iterator(const const_visibility_iterator& _X)
        :   visibility_iterator(_X)
          {
          }

        const_visibility_iterator(_Nodeptr _P,const _Portion& _K)
        :   visibility_iterator(_P,_K)
          {
          }

      // Operators
      public:
        const_reference operator*() const {return (_Value(_Ptr)); }
        _Ctptr operator->() const         {return (&**this); }

        const_visibility_iterator& operator++()
          {
          if (_Ptr)
            _Inc();
          return (*this);
          }

        const_visibility_iterator operator++(int)
          {
          const_visibility_iterator _Tmp = *this;
          if (_Ptr)
            ++*this;
          return (_Tmp);
          }

        bool operator==(const const_visibility_iterator& _X) const { return (_Ptr == _X._Ptr); }
        bool operator!=(const const_visibility_iterator& _X) const { return (!(*this == _X)); }
      };

    // CLASS reverse_visibility_iterator
    // Provides a forward-only far-to-near iteration through the visible nodes of a subtree.
    class reverse_visibility_iterator;
    friend class reverse_visibility_iterator;
    class reverse_visibility_iterator : public visibility_iterator
      {
      // Constructors
      public:
        reverse_visibility_iterator()
        :   visibility_iterator()
          {
          }

        reverse_visibility_iterator(const reverse_visibility_iterator& _X)
        :   visibility_iterator(_X)
          {
          }

        reverse_visibility_iterator(_Nodeptr _P,const _Portion& _K)
        :   visibility_iterator(_K)
          {
          _push_to_visible_leaf(_P);
          }

      // Operators
      public:
        reverse_visibility_iterator& operator++()
          {
          if (_Ptr)
            _Inc();
          return (*this);
          }

        reverse_visibility_iterator operator++(int)
          {
          reverse_visibility_iterator _Tmp = *this;
          if (_Ptr)
            ++*this;
          return (_Tmp);
          }

      // Internal Methods
      protected:
        void _Inc()
          {
          if (_Right(_Ptr))
            _push_to_visible_leaf(_Right(_Ptr));
          else
            _pop_to_visible_leaf();
          }

        void _push_to_visible_leaf(_Nodeptr _P)
          {
          while (_P)
            {
            branch b(_P,_Key);
            if (b.child == branch::LEFT)
              {
              if (_Right(_P))
                {
                if (_Left(_P))
                  _Bstack.push(b);
                _P = _Right(_P);
                }
              else
                _P = _Left(_P);
              }
            else
              {
              _Bstack.push(b);
              _P = _Left(_P);
              }
            }
          _pop_to_visible_leaf();
          }

        void _pop_to_visible_leaf()
          {
          _Ptr = NULL;
          if (!_Bstack.empty())
            {
            branch b = _Bstack.top();
            _Bstack.pop();
            if (b.child == branch::LEFT)
              _push_to_visible_leaf(_Left(b.parent._Mynode()));
            else
              _Ptr = b.parent._Mynode();
            }
          }
      };

    // CLASS const_reverse_visibility_iterator
    class const_reverse_visibility_iterator;
    friend class const_reverse_visibility_iterator;
    class const_reverse_visibility_iterator : public reverse_visibility_iterator
      {
      // Constructors
      public:
        const_reverse_visibility_iterator()
        :   reverse_visibility_iterator()
          {
          }

        const_reverse_visibility_iterator(const const_reverse_visibility_iterator& _X)
        :   reverse_visibility_iterator(_X)
          {
          }

        const_reverse_visibility_iterator(_Nodeptr _P,const _Portion& _K)
        :   reverse_visibility_iterator(_P,_K)
          {
          }

      // Operators
      public:
        const_reference operator*() const {return (_Value(_Ptr)); }
        _Ctptr operator->() const         {return (&**this); }

        const_reverse_visibility_iterator& operator++()
          {
          if (_Ptr)
            _Inc();
          return (*this);
          }

        const_reverse_visibility_iterator operator++(int)
          {
          const_reverse_visibility_iterator _Tmp = *this;
          if (_Ptr)
            ++*this;
          return (_Tmp);
          }

        bool operator==(const const_reverse_visibility_iterator& _X) const { return (_Ptr == _X._Ptr); }
        bool operator!=(const const_reverse_visibility_iterator& _X) const { return (!(*this == _X)); }
      };

  // Constructors
  public:
	  explicit bsp_tree()
		:   _Tr()
      {
      }

    bsp_tree(const _Partition& _P)
    :   _Tr(_P)
      {
      }

    bsp_tree(const _Myt& _X)
    :   _Tr(_X)
      {
      }

  protected:
    bsp_tree(_Nodeptr _X)
    :   _Tr(_X)
      {
      }

  // Methods
  public:
    visibility_iterator visibility_begin(const _Portion& viewpoint)
      { return visibility_iterator(_Root(),viewpoint); }
    visibility_iterator visibility_end()
      { return visibility_iterator(); }
    const_visibility_iterator visibility_begin(const _Portion& viewpoint) const
      { return const_visibility_iterator(_Root(),viewpoint); }
    const_visibility_iterator visibility_end() const
      { return const_visibility_iterator(); }
    reverse_visibility_iterator visibility_rbegin(const _Portion& viewpoint)
      { return reverse_visibility_iterator(_Root(),viewpoint); }
    reverse_visibility_iterator visibility_rend()
      { return reverse_visibility_iterator(); }
    const_reverse_visibility_iterator visibility_rbegin(const _Portion& viewpoint) const
      { return const_reverse_visibility_iterator(_Root(),viewpoint); }
    const_reverse_visibility_iterator visibility_rend() const
      { return const_reverse_visibility_iterator(); }

    // DEBUG OUTPUT
    void print() const
      {
#if defined(__DEBUGOUTPUT__H)
      DebugOutput::printline("bsp_tree");
      DebugOutput::indent();
      // preorder traversal
      for (preorder_iterator i=preorder_begin(); i!=preorder_end(); i++)
        i.print();
      DebugOutput::unindent();
#endif
      }
    };


  // This class makes use of the bsp_tree template and class partition3 to define
  // a means of partitioning the world space into minimal convex subspaces, known
  // as sectors, that in turn define the set of subspaces not occupied by world
  // geometry (that is, spaces legal for entities to move in).  It is assumed that
  // the world geometry forms a 2-manifold, legal polygonal representation of one
  // or more solid objects.
  class tree_t : public bsp_tree<partition3,vector3d>
    {
    // Types
    protected:
      typedef bsp_tree<partition3,vector3d> _Tr;
    // Constructors
    public:
      tree_t();
    // Methods
    public:
      // return sector corresponding to given world-space coordinate position;
      // return NULL if outside legal world space
      sector* find_sector(const vector3d& pos) const;
    friend class terrain;
    };


  class branch_vector: public vector<tree_t::branch>
  {
  };

#endif  // _BSP_TREE_H
