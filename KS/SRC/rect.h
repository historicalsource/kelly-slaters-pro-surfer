////////////////////////////////////////////////////////////////////////////////
//
// rect.h
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED. 
//
// a rectangle
//
////////////////////////////////////////////////////////////////////////////////

#ifndef RECT_H
#define RECT_H

#include "algebra.h"

// Toby was here.
class recti
{
public:
	vector2di	tl;
	vector2di	br;
	
public:
	// Creators.
	recti() { }
	recti(int left,int top,int right,int bottom)
		: tl(left,top), br(right,bottom) { }
	recti(const vector2di& topleft, const vector2di& bottomright)  
		: tl(topleft), br(bottomright) { }
	
	// Modifiers.
	void set_left(const int i)		{ tl.x = i; }
	void set_right(const int i)		{ br.x = i; }
	void set_top(const int i)		{ tl.y = i; }
	void set_bottom(const int i)	{ br.y = i; }
	
	// Accessors.
	int get_left(void) const    { return tl.x; }
	int get_right(void) const   { return br.x; }
	int get_top(void) const     { return tl.y; }
	int get_bottom(void) const  { return br.y; }

	// Operators.
	const recti & operator=(const recti & r)
	{
		if (this != &r)
		{
			tl = r.tl;
			br = r.br;
		}

		return *this;
	}
};

class rectf
{
public:
	vector2d tl;
	vector2d br;
public:
	rectf() { }
	rectf(rational_t left, rational_t top, rational_t right, rational_t bottom)
		: tl(left, top), br(right, bottom) 
	{}
	rectf(const vector2d &topleft, const vector2d &bottomright)
		: tl(topleft), br(bottomright) 
	{}
	
	rational_t get_left() const        { return tl.x; }
	rational_t get_right() const       { return br.x; }
	rational_t get_top() const         { return tl.y; }
	rational_t get_bottom() const      { return br.y; }
	rational_t get_width() const       { return br.x - tl.x; }
	rational_t get_height() const      { return br.y - tl.y; }
	void set_left(rational_t left)     { tl.x = left; }
	void set_right(rational_t right)   { br.x = right; }
	void set_top(rational_t top)       { tl.y = top; }
	void set_bottom(rational_t bottom) { br.y = bottom; }
	
	bool empty() const { return ( get_width()<=0 || get_height()<=0 ); }
	
	// intersect this rectangle with the given rectangle (result stored back into this)
	void intersect( const rectf& b )
	{
		tl.x = max( tl.x, b.tl.x );
		tl.y = max( tl.y, b.tl.y );
		br.x = min( br.x, b.br.x );
		br.y = min( br.y, b.br.y );
	}
	
	void accumulate( const vector2d& p )
	{
		tl.x = min( tl.x, p.x );
		tl.y = min( tl.y, p.y );
		br.x = max( br.x, p.x );
		br.y = max( br.y, p.y );
	}
	
	const rectf& operator+=( const vector2d& p )
	{
		tl += p;
		br += p;
		return *this;
	}
	
	const rectf& operator+=( const rectf& b )
	{
		tl.x = min( tl.x, b.tl.x );
		tl.y = min( tl.y, b.tl.y );
		br.x = max( br.x, b.br.x );
		br.y = max( br.y, b.br.y );
		return *this;
	}
};

#endif // RECT_H