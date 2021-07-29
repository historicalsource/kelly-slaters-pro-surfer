/* Some code from Game Programming Gems
 * "Portions Copyright (C) Greg Snook, 2000"
 */
#ifndef _AI_POLYPATH_HEAP_H
#define _AI_POLYPATH_HEAP_H

#include "global.h"
#include "ai_polypath_cell.h"
#ifdef TARGET_PS2
#include "algo.h"
//#include "heap.h"
#include "stl_heap.h"
#endif

class ai_polypath_node
{
public:
	ai_polypath_node(): cell(0), cost(0){}
	~ai_polypath_node() {}

  ai_polypath_cell* cell; // pointer to the cell in question
	rational_t cost;        // (g + h) in A* represents the cost of traveling through this cell

	inline bool operator < (const ai_polypath_node& b )
	{
		// To compare two nodes, we compare the cost or `f' value, which is the
		// sum of the g and h values defined by A*.
		return (cost < (b.cost));
	}

	inline bool operator > (const ai_polypath_node& b )
	{
		// To compare two nodes, we compare the cost or `f' value, which is the
		// sum of the g and h values defined by A*.
		return (cost > (b.cost));
	}

	inline bool operator == (const ai_polypath_node& b )
	{
		// Two nodes are equal if their components are equal
		return ((cell == b.cell) && (cost == b.cost));
	}
};

inline bool operator < ( const ai_polypath_node& a, const ai_polypath_node& b )
{
	return (a.cost < b.cost);
}

inline bool operator > ( const ai_polypath_node& a, const ai_polypath_node& b )
{
	return (a.cost > b.cost);
}

inline bool operator == ( const ai_polypath_node& a, const ai_polypath_node& b )
{
	return ((a.cell == b.cell) && (a.cost == b.cost));
}




class ai_polypath_heap
{
public:

	typedef vector<ai_polypath_node> CONTAINER;
	greater<ai_polypath_node> comp;

	ai_polypath_heap()
  {
  }

	~ai_polypath_heap()
  {
  }


  inline void setup(int _session_id, const vector3d& _goal)
  {
	  the_goal = _goal;
	  sessionID = _session_id;
	  nodes.resize(0);
  }

	inline void add_cell(ai_polypath_cell* cell)
  {
	  ai_polypath_node new_node;

	  new_node.cell = cell;
	  new_node.cost = cell->pathfinding_cost();

	  nodes.push_back(new_node);
#ifndef TARGET_XBOX	// This code is probably never used by us anyhow.  (dc 02/05/02)
	  push_heap( nodes.begin(), nodes.end(), comp );
#endif
  }

private:
	inline CONTAINER::iterator find_node_interator(ai_polypath_cell* cell)
  {
	  for( CONTAINER::iterator i = nodes.begin(); i != nodes.end(); ++i )
	  {
	    if( (*i).cell == cell )
		    return i;
	  }

	  return nodes.end();
  }

public:
	inline void adjust_cell(ai_polypath_cell* cell)
  {
	  CONTAINER::iterator iter = find_node_interator(cell);

	  if (iter != nodes.end())
	  {
		  // update the node data
		  (*iter).cell = cell;
		  (*iter).cost = cell->pathfinding_cost();

#ifndef TARGET_XBOX	// This code is probably never used by us anyhow.  (dc 02/05/02)
		  // reorder the heap
		  push_heap( nodes.begin(), iter+1, comp );
#endif
	  }
  }

	inline bool not_empty()const
  {
  	return(nodes.size() ? true:false);
  }

	inline void get_top(ai_polypath_node& n)
  {
	  n = nodes.front();
#ifndef TARGET_XBOX	// This code is probably never used by us anyhow.  (dc 02/05/02)
	  pop_heap( nodes.begin(), nodes.end(), comp );
#endif
	  nodes.pop_back();
  }

	inline int session_id()const
  {
    return(sessionID);
  }

	inline const vector3d& goal()const
  {
    return(the_goal);
  }

private:
	CONTAINER nodes;
	int sessionID;
	vector3d the_goal;
};

#endif  // end of file      ( ai_polypath_heap.h )

