/* Some code from Game Programming Gems
 * "Portions Copyright (C) Greg Snook, 2000"
 */
#ifndef _AI_POLYPATH_H_
#define _AI_POLYPATH_H_

#include "global.h"
#include "ai_polypath_heap.h"
#include "portal.h"

// forward declaration required
class ai_path;
class ai_polypath_cell;
class entity;
class camera;

class ai_polypath
{
public:
	typedef	vector<ai_polypath_cell *> CELL_ARRAY;

	ai_polypath();
	~ai_polypath();

	ai_polypath_cell* closest_cell(const vector3d& pt, region_node *reg = NULL, entity *ent = NULL)const;

	bool los_test(ai_polypath_cell* start_cell, const vector3d& start_pos, ai_polypath_cell* end_cell, const vector3d& end_pos);
	bool find_path(ai_path& path, ai_polypath_cell* start_cell, const vector3d& start_pos, ai_polypath_cell* end_cell, const vector3d& end_pos);
	inline bool find_path(ai_path& path, const vector3d& start_pos, const vector3d& end_pos)
  {
    return(find_path(path, closest_cell(start_pos), start_pos, closest_cell(end_pos), end_pos));
  }

  inline int num_cells() const              { return((int)cells.size()); }
	inline ai_polypath_cell* cell(int index)  { return(cells[index]); }

  void render();
  void optimize();

protected:
  friend void serial_in(chunk_file &fs, ai_polypath *path);
  friend class PathsDialog2;

	CELL_ARRAY cells; // the cells that make up this mesh

	// path finding data...
	int session_id;
	ai_polypath_heap nav_heap;

	vector3d snap_to_cell(ai_polypath_cell* cell, const vector3d& pt);
	vector3d snap_to_mesh(ai_polypath_cell** cell_out, const vector3d& pt);

  void clear();
	void add_cell(const vector3d& a, const vector3d& b, const vector3d& c);
	void link();
};
class chunk_file;
void serial_in(chunk_file &fs, ai_polypath *path);




// forward declaration of our parents
class ai_path
{
public:

	// ----- ENUMERATIONS & CONSTANTS -----

	// definition of a waypoint
	struct waypoint
	{
		vector3d pos;		// 3D pos of waypoint
		ai_polypath_cell* cell;	// The cell which owns the waypoint
	};

	typedef list <waypoint> WAYPOINT_LIST;
	typedef WAYPOINT_LIST::const_iterator waypoint_ID;

	ai_path()
  {
  }

	~ai_path()
  {
  }

  inline void setup(ai_polypath* _parent, const vector3d& start_pt, ai_polypath_cell* start_cell, const vector3d& end_pt, ai_polypath_cell* end_cell)
  {
	  waypoints.resize(0);

	  parent = _parent;
	  start_point.pos = start_pt;
	  start_point.cell = start_cell;
	  end_point.pos = end_pt;
	  end_point.cell = end_cell;

	  // setup the waypoint list with our start and end points
	  waypoints.push_back(start_point);
  }

  //:	AddWayPoint
  //----------------------------------------------------------------------------------------
  //
  //	Adds a NEW waypoint to the end of the list
  //
  //-------------------------------------------------------------------------------------://
  inline void add_waypoint(const vector3d& pt, ai_polypath_cell* cell)
  {
	  waypoint NewPoint;

	  NewPoint.pos = pt;
	  NewPoint.cell = cell;

	  waypoints.push_back(NewPoint);
  }


	// ----- ACCESSORS --------------------
  inline void end_path()
  {
	  // cap the waypoint path with the last end_point
	  waypoints.push_back(end_point);
    cur_point = waypoints.begin();
    next_point = waypoints.begin();
    reset = true;
  }

  //= ACCESSORS ============================================================================
  inline ai_polypath* get_parent()const
  {
	  return(parent);
  }

  inline const waypoint& get_start_point()const
  {
	  return(start_point);
  }

  inline const waypoint&	get_end_point()const
  {
	  return(end_point);
  }

  inline WAYPOINT_LIST& get_waypoints()
  {
	  return(waypoints);
  }

	waypoint_ID get_furthest_visible_waypoint(const waypoint_ID& vantage_point)const
  {
	  // see if we are already talking about the last waypoint
	  if (vantage_point == waypoints.end())
		  return(vantage_point);

	  const waypoint& vantage = *vantage_point;
	  waypoint_ID test = vantage_point;
	  ++test;

	  if (test == waypoints.end())
	  {
		  return(test);
	  }

	  waypoint_ID visible = test;
	  ++test;

	  while (test != waypoints.end())
	  {
		  const waypoint& test_pt = *test;
		  if (!parent->los_test(vantage.cell, vantage.pos, test_pt.cell, test_pt.pos))
		  {
			  return(visible);
		  }
		  visible = test;
		  ++test;
	  }
	  return(visible);
  }

  void render( camera* camera_link );

  bool get_way_point( const vector3d &pos, const vector3d &lastpos, vector3d &local_dest );

private:
	ai_polypath*		parent;
	waypoint			  start_point;
	waypoint			  end_point;
	waypoint_ID     cur_point;
	waypoint_ID     next_point;
  vector3d        goto_start;
  bool reset;
	WAYPOINT_LIST		waypoints;
};

#endif

