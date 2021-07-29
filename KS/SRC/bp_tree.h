// bp_tree.h
// Copyright (c) 1998-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.

#ifndef _BP_TREE_H
#define _BP_TREE_H

#include "binary_tree.h"
#include "hwmath.h"
#include <functional>
#include <iterator>
#include "debug_render.h"
#include "hyperplane.h"
//#include "terrain.h"  // for partition3

#define NEW_COLLISION_SCHEME
extern bool first_hit_this_frame;

class partition3;

extern partition3 *global_plane;

// CLASS bp_tree
// This class implements the Binary Partitioning Tree as an ADT.  It takes a
// partition type that provides an ordering for the given portion type via the
// comparison method operator>(const _Partition&,const _Portion&).
template <class _Partition,class _Portion>
class bp_tree : public binary_tree<_Partition>
{
public:
  typedef bp_tree<_Partition,_Portion> _Myt;
  typedef binary_tree<_Partition> _Tr;
	typedef _Tr::difference_type difference_type;
	typedef _Tr::_Nodeptr _Nodeptr;
	typedef _Tr::reference reference;
	typedef _Tr::const_reference const_reference;
	typedef _Tr::_Tptr _Tptr;
	typedef _Tr::_Ctptr _Ctptr;

  vector<partition3 *> plane_stack;

  // CLASS iterator
  class iterator;
  friend class iterator;
  class iterator : public bidirectional_iterator<_Partition,difference_type
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
      iterator(_Nodeptr _P) : _Ptr(_P) {}

    // Methods
    public:
      reference operator*() const {return (_Value(_Ptr)); }
      _Tptr operator->() const    {return ((_Tptr) &**this); }

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

    // Internal Methods
    protected:
      void _Inc() { _Ptr = _Right(_Ptr); }
      void _Dec() { _Ptr = _Left(_Ptr); }
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
      const_iterator(_Nodeptr _P) : iterator(_P) {}

    // Methods
    public:
      const_reference operator*() const {return (_Value(_Ptr)); }
      _Ctptr operator->() const         {return (&**this); }

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

  typedef pair<iterator,bool> _Pairib;
  typedef pair<iterator,iterator> _Pairii;
  typedef pair<const_iterator,const_iterator> _Paircc;

  // CLASS branch
  class branch;
  friend class branch;
  class branch
  {
    // Types
    public:
      enum dir_t
      {
        RIGHT,
        LEFT,
        NONE
      };

    // Data
    public:
      iterator parent;
      dir_t child;

    // Constructors
    public:
      branch() : parent(NULL),child(NONE) {}
      branch(const branch& b) : parent(b.parent),child(b.child) {}
      branch(const iterator& p) : parent(p),child(NONE) {}
      branch(_Nodeptr p) : parent(p),child(NONE) {}

      branch(const iterator& p,dir_t c)
      :   parent(p),
          child(c)
      {
      }

      branch(_Nodeptr p,dir_t c)
      :   parent(p),
          child(c)
      {
      }

      branch(const iterator& p,const _Portion& k)
      :   parent(p),
          child(dir(k,*p))
      {
      }

      branch(_Nodeptr p,const _Portion& k)
      :   parent(p),
          child(dir(k,_Value(p)))
      {
      }

      const branch& operator=(const branch& b)
      {
        parent = b.parent;
        child = b.child;
        return *this;
      }

    // Methods
    public:
      dir_t dir(const _Portion& k,const _Partition& v) const
      { return (v > k)? branch::LEFT : branch::RIGHT; }
      bool empty() const { return _Mynode()?false:true; }
      const iterator& get_parent() const { return parent; }
      iterator operator*() const { return iterator(_Mynode()); }

      branch& operator--()
      {
        if (_Mynode())
        {
          parent = **this;
          child = LEFT;
        }
        return *this;
      }

      branch& operator++()
      {
        if (_Mynode())
        {
          parent = **this;
          child = RIGHT;
        }
        return *this;
      }

      bool operator<(const branch& _X) const { assert(false); return false; }
      bool operator==(const branch& _X) const { return (parent==_X.parent && child==_X.child); }
      bool operator!=(const branch& _X) const { return (!(*this == _X)); }
      _Nodeptr _Mynode() const
      {
        switch (child)
        {
          case RIGHT:
            return _Right(parent._Mynode());
          case LEFT:
            return _Left(parent._Mynode());
          case NONE:
            return parent._Mynode();
        }
        return NULL;
      }
  };

// Constructors
public:
	explicit bp_tree()
		:   _Tr(), plane_stack(64)
  {
  }

  bp_tree(const _Partition& _P)
    :   _Tr(_P), plane_stack(64)
  {
  }

  bp_tree(const _Myt& _X)
    :   _Tr(_X), plane_stack(64)
  {
  }

protected:
  bp_tree(_Nodeptr _X)
    :   _Tr(_X), plane_stack(64)
  {
  }

// Methods
public:
  iterator begin() { return iterator(_Root()); }
  iterator end()   { return iterator(NULL); }
  const_iterator begin() const { return const_iterator(_Root()); }
  const_iterator end()   const { return const_iterator(NULL); }

  // Insert NEW partition at leaf corresponding to given key.
  iterator insert(const _Partition& p,const _Portion& key)
  {
    return insert(find_leaf(key),p);
  }

  // Insert NEW partition at given leaf.
  iterator insert(const branch& l,const _Partition& p)
  {
    switch (l.child)
    {
      case branch::RIGHT:
        return _AddRight(l.parent._Mynode(),p);
      case branch::LEFT:
        return _AddLeft(l.parent._Mynode(),p);
      default:
        assert(empty());
        _Init(p);
        break;
    }
//    float f1 = 0;
//    float f2 = 5/f1;
    return iterator(NULL);
  }

  // Find leaf corresponding to given key.
  branch find_leaf(const _Portion& key) const
  {
    branch b;
    for (iterator i=begin(); i!=end(); )
    {
      b = branch(i,key);
      if (b.child == branch::LEFT)
        i--;
      else
        ++i;
    }
    return b;
  }


  struct recurse_data_t1
  {
    int calling_case;
    branch b;
    _Portion p0;
    _Portion p1;
    _Portion my_n;
    rational_t saved_d1;
    bool returned;
    bool retval;
#ifdef TARGET_GC
    bool pad_0;
    bool pad_1;
#endif
  };

  bool recurse_intersection( branch b,
                             const _Portion &p0,
                             const _Portion &p1,
                             const _Portion &my_n,
                             _Portion &ix,
                             _Portion &n )
  {
    // set up recursion stack: notice this isn't a vector, and therefore
    // doesn't need to be jiggied with in order to get it to allocate up
    // front (I didn't notice at first, -JF)
    static recurse_data_t1 recursion_stack[500];

    // initialize stack with entry value;
    recurse_data_t1* sp = recursion_stack;
    sp->calling_case = -1;
    sp->b = b;
    sp->p0 = p0;
    sp->p1 = p1;
    sp->my_n = my_n;
    sp->returned = false;

    branch c;
    bool retval = false;

    while ( sp >= recursion_stack )
    {
      if ( sp->returned )
      {
        // top of stack is a return from an earlier recursion
        if ( sp->calling_case==SPLITT_Spanning && !sp->retval )
        {
          // earlier recursion was the first half of a SPLITT_Spanning case that failed;
          // try the further segment now (pi to p1)
          _Portion pi = sp->p1;
          rational_t d1 = sp->saved_d1;
          sp--;  // pop recursion stack
          c = sp->b;
          _Portion split_n = _Value(c._Mynode()).get_normal();
          if ( d1 < 0.0f )
          {
            --c;
          }
          else
          {
            ++c;
            split_n *= -1;
          }
          ++sp;  // push recursion stack
          sp->calling_case = -1;
          sp->b = c;
          sp->p0 = pi;
          sp->p1 = (sp-1)->p1;
          sp->my_n = split_n;
          sp->returned = false;
        }
        else
        {
          // for any other returned recursion, simply hand the retval on down
          retval = sp->retval;
          --sp;  // pop recursion stack
          if ( sp >= recursion_stack )
          {
            sp->returned = true;
            sp->retval = retval;
          }
        }
      }
      else
      {
        sp->retval = false;
        if ( sp->b.empty() )
        {
          // if branch is empty, return from this recursion
          sp->returned = true;
          if ( sp->b.child != branch::RIGHT )
          {
            // if LEFT branch, save the intersection point and normal
            ix = sp->p0;
            n = sp->my_n;
            sp->retval = true;
          }
        }
        else
        {
          // split p0 to p1 across the partition
          rational_t d0, d1;
          _Portion pi;
          split_enum_t splt = _Value(sp->b._Mynode()).split( sp->p0, sp->p1, d0, d1, pi, n );
          switch ( splt )
          {
            case SPLITT_Spanning:
              // p0 to pi is the closer segment, so pass it on down to check for a collision
              c = sp->b;
              if ( d0 < 0.0f )
                --c;
              else
                ++c;
              ++sp;  // push recursion stack
              sp->calling_case = splt;
              sp->b = c;
              sp->p0 = (sp-1)->p0;
              sp->p1 = pi;
              sp->my_n = (sp-1)->my_n;
              sp->saved_d1 = d1;
              sp->returned = false;
              break;

            case SPLITT_Positive:
            case SPLITT_On:
              // p0 to p1 is on the positive side of the plane, so send it that way
              c = sp->b;
              ++c;
              ++sp;  // push recursion stack
              sp->calling_case = splt;
              sp->b = c;
              sp->p0 = (sp-1)->p0;
              sp->p1 = (sp-1)->p1;
              sp->my_n = (sp-1)->my_n;
              sp->returned = false;
              break;

            case SPLITT_Negative:
              // p0 to p1 is on the negative side of the plane, so send it that way
              c = sp->b;
              --c;
              ++sp;  // push recursion stack
              sp->calling_case = splt;
              sp->b = c;
              sp->p0 = (sp-1)->p0;
              sp->p1 = (sp-1)->p1;
              sp->my_n = (sp-1)->my_n;
              sp->returned = false;
              break;
          }
        }
      }
    }

    return retval;
  }

  // find intersection of a ray
  bool find_intersection(const _Portion &p0, const _Portion &p1, _Portion &pi, _Portion &n)
  {
    _Portion my_n; // dummy
    return recurse_intersection(branch(begin()), p0, p1, my_n, pi, n);
  }

  typedef vector<_Partition *> plane_list_t;
  typedef vector<int> hit_dir_list_t;
  typedef vector<_Portion> hit_list_t;

  static plane_list_t plane_list; //(100);
  static plane_list_t show_plane_list; //(100);
  static hit_list_t hit_list;  //(100);
  static hit_dir_list_t hit_dir_list; //(100);

  bool in_world( const vector3d &p, const rational_t r, const vector3d &v, vector3d &n, vector3d &pi )
  {
    plane_list.resize(0);
    show_plane_list.resize(0);
    hit_list.resize(0);
    hit_dir_list.resize(0);

//    vector3d nada = ZEROVEC;
#ifdef DEBUG
    if (first_hit_this_frame)
    {
      clear_planes();
    }
#endif
    bool outval = !recurse_intersection(branch(begin()), p, r, v, n, pi, plane_list, show_plane_list, hit_list, hit_dir_list);
    return outval;
  }

	bool recurse_intersection(branch b, const vector3d &p, const rational_t r, const vector3d &v, vector3d &n, vector3d &pi, plane_list_t &plane_list, plane_list_t &show_plane_list, hit_list_t &hit_list, hit_dir_list_t & hit_dir_list)
  {
		bool                 intersect = false;
		vector3d             np;
		branch               c;
		plane_stack.push_back(&_Value(b._Mynode()));
		global_plane = plane_stack.back();

    /*
		vector<_Partition *> dup_plane_list;
		vector<_Partition *> dup_show_plane_list;
		vector<_Portion> dup_hit_list;
		vector<int> dup_hit_dir_list;
    */

		if ( b.empty() )
    {
			if ( b.child == branch::RIGHT )
      {
				plane_stack.pop_back();
				return false;
      }
			else
      {
#ifdef NEW_COLLISION_SCHEME
        // our plane list contains all the planes which the sphere was within touching distance of
				// reject a bunch by polygon bounding distances here
				// handle the trivial cases
#if !defined(BUILD_BOOTABLE) && 0
        if (first_hit_this_frame)
        {
          draw_planes(show_plane_list, hit_dir_list, p);
        }
#endif
				if ( plane_list.size() == 0 )
				{
					plane_stack.pop_back();
          n = YVEC;
          pi = p;
          return true;
        }
				else if(plane_list.size() == 1 )
        {
          pi = plane_list[0]->get_closest_point(p);
          n = plane_list[0]->get_normal();
					plane_stack.pop_back();

          return((pi-p).length2() < (r*r));
//					return true;
        }
				else if ( plane_list.size() == 2 )
        {
					plane_stack.pop_back();
          bool outval = collide_sphere_two_partition3s(p,r,*plane_list[0],*plane_list[1],pi);

          vector3d p_minus_pi = p-pi;

          if ( dot(plane_list[0]->get_normal(),p_minus_pi)>dot(plane_list[1]->get_normal(),p_minus_pi) )
            n = plane_list[0]->get_normal();
          else
            n = plane_list[1]->get_normal();

          return outval && (p_minus_pi.length2() < (r*r));
//          return outval;
        }
				else
        {
          pi = p;
          rational_t max_hit_dist2=0.0f;
          intersect = true;
          vector3d hit_loc;
          rational_t rad2 = r*r;

          // Loop through every triple of planes.  Any plane lying entirely in front of the sphere (backside[i] = true)
          // counts as an automatic hit, hence the strange inner loop.
          int plane_list_size = (int)plane_list.size();
          int plane_list_size_m1 = plane_list_size-1;
          int plane_list_size_m2 = plane_list_size-2;
          for (int i=0;intersect && i<plane_list_size_m2;++i)
          {
            _Partition *plane_list_i = plane_list[i];

            for (int j=i+1;intersect && j<plane_list_size_m1;++j)
            {
              _Partition *plane_list_j = plane_list[j];

              for (int k=j+1;intersect && k<plane_list_size;++k)
              {
                _Partition *plane_list_k = plane_list[k];

//                assert(plane_list_size == plane_list.size());

                intersect &= collide_sphere_three_partition3s(p,r,*plane_list_i,*plane_list_j,*plane_list_k,hit_loc);

                // Choose the furthest hit loc, assuming it'll be the one you want.
                if (intersect)
                {
                  rational_t hit_dist2 = (hit_loc-p).length2();
                  if (hit_dist2 > max_hit_dist2 && hit_dist2 < rad2)
                  {
                    pi = hit_loc;
                    max_hit_dist2 = hit_dist2;
                    vector3d p_minus_pi = p-pi;

                    // choose the most opposed normal.
                    if ( dot(plane_list_i->get_normal(),p_minus_pi)>dot(plane_list_j->get_normal(),p_minus_pi) )
                      if ( dot(plane_list_i->get_normal(),p_minus_pi)>dot(plane_list_k->get_normal(),p_minus_pi) )
                        n = plane_list_i->get_normal();
                      else
                        n = plane_list_k->get_normal();
                    else if ( dot(plane_list_j->get_normal(),p_minus_pi)>dot(plane_list_k->get_normal(),p_minus_pi) )
                        n = plane_list_j->get_normal();
                      else
                        n = plane_list_k->get_normal();
                  }
                }
              }
            }
          }

					plane_stack.pop_back();
          return intersect;
        }
#else

      // Sort the normals to make a good choice of n.
      int i;
      float min_dist = 10000.0f;
      int lesser=-1;
      vector3d avg_hit_point = ZEROVEC;
      for ( i=0;i<plane_list.size();++i )
      {
        float dist;
//        if( 1 || dot(rel_vel,normal_list1[i])>=0.0f && (n=dot(rel_vel,normal_list2[i]))<=0.0f )
//          {

          // Experiment, clip to the one we're least behind
          //n = -dot(e1->get_updated_closest_point_along_dir(normal_list2[i])-hit_list[i],normal_list2[i]);
          dist = -dot(p-hit_list[i],plane_list[i]->get_normal());
          avg_hit_point += hit_list[i];
          if( dist<min_dist )
          {
            lesser = i;
            min_dist = dist;
          }
          assert(lesser!=-1);
//          }
      }
      n = plane_list[lesser]->get_normal();
      pi = hit_list[lesser];
/*
      // ahh hell, lets choose the point which we are most directly hitting...
      int greater = -1;
      int max_perp = -10000.0f;
      for(i=0;i<plane_list.size();i++)
        {
        float perp;
        vector3d diff_dir = p-hit_list[i];
        diff_dir.normalize();
        perp = -dot(diff_dir, v);
        if( perp>max_perp )
          {
          greater = i;
          max_perp = perp;
          }
        assert(greater!=-1);
        }

      pi = hit_list[greater]; // avg_hit_point/plane_list.size(); //hit_list[lesser];
*/
				plane_stack.pop_back();
        return true;
#endif
      }
    }

		// split p0 to p1 across the partition
		switch ( plane_stack.back()->split( p, r, np ) )
    {
			case SPLITT_Positive:
        {
				c = b;
				++c;
				partition3 * plane = plane_stack.back();
				show_plane_list.push_back( plane );
        hit_dir_list.push_back(1);

        // we need to do this because the ultimate collision detection will only collide with the backspaces of
        // each plane.
        plane_stack.back()->invert();

//        hit_list.push_back(p);
				intersect = recurse_intersection_gate(c, p, r, v, n, pi,plane_list,show_plane_list, hit_list, hit_dir_list);

        // and here we undo the damage.
        plane_stack.back()->invert();

				show_plane_list.pop_back();
        hit_dir_list.pop_back();
        }
				break;
			case SPLITT_Negative:
				c = b;
				--c;
				show_plane_list.push_back(plane_stack.back());
        hit_dir_list.push_back(-1);
//        hit_list.push_back(p);
				intersect = recurse_intersection_gate(c, p, r, v, n, pi, plane_list,show_plane_list, hit_list, hit_dir_list);
				show_plane_list.pop_back();
        hit_dir_list.pop_back();
				break;
			case SPLITT_On:
        {
        c = b;
        --c;
        // ignore any infinitely thin subspace
        bool hit_plane = false;
        if ( c.empty() || !plane_stack.back()->is_neg_plane(_Value(c._Mynode())) )
          hit_plane = true;

        if ( hit_plane )
          {
				  // send the sphere down the negative side and add this plane to the list of planes for the back end check
				  // which validates a collision
          _Portion hit_point;
          vector3d normal = plane_stack.back()->get_normal();
          rational_t d = plane_stack.back()->distance(p);
          hit_point = p-normal*d;
				  plane_list.push_back(plane_stack.back());
				  show_plane_list.push_back(plane_stack.back());
          hit_list.push_back(hit_point);
          hit_dir_list.push_back(2);
				  if ( dot(v, np) < 0.0001f )
            n = np;
          }

        rational_t right_r, left_r;
        vector3d right_p, left_p;
        right_r = left_r = r;
        right_p = left_p = p;
#ifndef NEW_COLLISION_SCHEME
        if ( hit_plane )
        {
          rational_t dist = plane_stack.back()->distance(p);
          if (dist>0)
          {
            right_r = (dist+r)*0.5F;
            left_r = r-right_r;
            right_p = p+(right_r-dist)*plane_stack.back()->get_normal();
            left_p = right_p - (right_r+left_r)*plane_stack.back()->get_normal();
          }
          else
          {
            left_r = (-dist+r)*0.5F;
            right_r = r-left_r;
            left_p = p-(left_r+dist)*plane_stack.back()->get_normal();
            right_p = left_p + (right_r+left_r)*plane_stack.back()->get_normal();
          }
        }
#endif

				// duplicate the list for sending to the negative side

        /*
        dup_plane_list = plane_list;
        dup_show_plane_list = show_plane_list;
        dup_hit_list = hit_list;
        dup_hit_dir_list = hit_dir_list;
        dup_hit_dir_list.pop_back();
        dup_hit_dir_list.push_back(-2);
        */

        if ( hit_plane )
        {
          // recurse behind hit plane
				  intersect = recurse_intersection_gate( c, right_p, right_r, v, n, pi, plane_list, show_plane_list, hit_list, hit_dir_list );
        }
        else
        {
          // infinitely thin subspace can contain no relevant data,
          // so recurse forward of child backplane
				  intersect = recurse_intersection_gate( ++c, right_p, right_r, v, n, pi, plane_list, show_plane_list, hit_list, hit_dir_list );
        }
			  if ( !intersect )
        {
					c = b;
					++c;

          // we need to do this because the ultimate collision detection will only collide with the backspaces of
          // each plane.
          plane_stack.back()->invert();

					intersect = recurse_intersection_gate( c, left_p, left_r, v, n, pi, plane_list, show_plane_list, hit_list, hit_dir_list );

          // and here we undo the damage.
          plane_stack.back()->invert();
        }

        if ( hit_plane )
        {
				  plane_list.pop_back();
				  show_plane_list.pop_back();
          hit_list.pop_back();
          hit_dir_list.pop_back();
        }
        }
				break;
      default:
        break;
		}

		plane_stack.pop_back();

		return intersect;
  }

  // inline depth blocker to help metrowerks not suck.
  bool recurse_intersection_gate(branch &b, const vector3d &p, const rational_t r, const vector3d &v, vector3d &n, vector3d &pi, plane_list_t &plane_list, plane_list_t &show_plane_list, hit_list_t &hit_list, hit_dir_list_t & hit_dir_list)
  {
    return recurse_intersection(b, p, r, v, n, pi,plane_list,show_plane_list, hit_list, hit_dir_list);
  }

};


#endif  // _BP_TREE_H
