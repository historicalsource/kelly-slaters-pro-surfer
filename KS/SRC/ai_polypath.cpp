#include "global.h"
/* Some code from Game Programming Gems
 * "Portions Copyright (C) Greg Snook, 2000"
 */
#include "ai_polypath.h"
#include "ai_polypath_cell.h"
#include "ai_polypath_heap.h"
#include "debug_render.h"
#include "wds.h"
#include "entity.h"
#include "terrain.h"

ai_polypath::ai_polypath()
  : session_id(0)
{
	cells.resize(0);
}

ai_polypath::~ai_polypath()
{
	clear();
}

void ai_polypath::clear()
{
	CELL_ARRAY::iterator	cell_i = cells.begin();
	while(cell_i != cells.end())
	{
		ai_polypath_cell* cell = *cell_i;
    ++cell_i;

		delete cell;
	}

	cells.resize(0);
}

//:	AddCell
//----------------------------------------------------------------------------------------
//
//	Add a NEW cell, defined by the three vertices in clockwise order, to this mesh
//
//-------------------------------------------------------------------------------------://
void ai_polypath::add_cell(const vector3d& a, const vector3d& b, const vector3d& c)
{
  assert(a != b && b != c && c != a);
  ai_polypath_cell *cell = NEW ai_polypath_cell();
  cell->init(a, b, c);
	cells.push_back(cell);
}




vector3d ai_polypath::snap_to_cell(ai_polypath_cell* cell, const vector3d& pt)
{
	vector3d res = pt;

	if (!cell->is_point_in_cell_collumn(res))
		cell->force_to_cell_collumn(res);

	cell->map_vector_height_to_cell(res);

	return (res);
}

vector3d ai_polypath::snap_to_mesh(ai_polypath_cell** cell_out, const vector3d& pt)
{
	vector3d res = pt;

	*cell_out = closest_cell(res);

	return (snap_to_cell(*cell_out, res));
}

ai_polypath_cell* ai_polypath::closest_cell(const vector3d& pt, region_node *reg, entity *ent) const
{
  if(reg == NULL)
  {
    sector *the_sector = g_world_ptr->get_the_terrain().find_sector(pt);
    if(the_sector)
      reg = the_sector->get_region();
  }

	rational_t closest_dist = 3.4E+38f;
	rational_t closest_height = 3.4E+38f;

	bool found_home_cell = false;
	rational_t dist;
	ai_polypath_cell* closest_cell = NULL;

  CELL_ARRAY::const_iterator	cell_i;
  CELL_ARRAY::const_iterator	cell_iend;

  if(reg != NULL && reg->get_data() != NULL)
  {
	  cell_i = reg->get_data()->get_pathcells().begin();
    cell_iend = reg->get_data()->get_pathcells().end();
  }
  else
  {
	  cell_i = cells.begin();
    cell_iend = cells.end();
  }

	for(;cell_i != cell_iend; ++cell_i)
	{
		ai_polypath_cell* cell = *cell_i;

		if (cell->is_point_in_cell_collumn(pt))
		{
			vector3d new_pos(pt);
			cell->map_vector_height_to_cell(new_pos);
			dist = __fabs(new_pos.y - pt.y);

			if (found_home_cell)
			{
				if (dist < closest_height)
				{
					closest_cell = cell;
					closest_height = dist;
				}
			}
			else
			{
				closest_cell = cell;
				closest_height = dist;
				found_home_cell = true;
			}
		}

		if (!found_home_cell)
		{
			vector2d start(cell->center().x, cell->center().z);
			vector2d end(pt.x, pt.z);
			ai_polypath_line motion_path(start, end);

			ai_polypath_cell* next_cell;
			ai_polypath_cell::eCellSide wall_hit;
			vector2d intersection_pt;

			ai_polypath_cell::ePathResult res = cell->classify_path_to_cell(motion_path, &next_cell, wall_hit, &intersection_pt);

			if (res == ai_polypath_cell::EXITING_CELL)
			{
				vector3d closest_pt_3d(intersection_pt.x,0.0f,intersection_pt.y);
				cell->map_vector_height_to_cell(closest_pt_3d);

				closest_pt_3d -= pt;

				dist = closest_pt_3d.length2();

				if (dist < closest_dist)
				{
					closest_dist = dist;
					closest_cell = cell;
				}
			}
		}
	}


  // Sanity checkers:
  if(closest_cell == NULL)
  {
    if(reg == NULL || reg->get_data() == NULL)
      error("No polypath cells found in level (%d cells, <%.2f, %.2f, %.2f>, '%s : <%.2f, %.2f, %.2f>')!", cells.size(), pt.x, pt.y, pt.z, (ent != NULL ? ent->get_name().c_str() : "NO ENTITY"), (ent != NULL ? ent->get_abs_position().x : 0.0f), (ent != NULL ? ent->get_abs_position().y : 0.0f), (ent != NULL ? ent->get_abs_position().z : 0.0f) );
    else
    {
      warning("No polypath cells found in region '%s', searching whole graph (%d cells, <%.2f, %.2f, %.2f>, '%s : <%.2f, %.2f, %.2f>')!", reg->get_data()->get_name().c_str(), cells.size(), pt.x, pt.y, pt.z, (ent != NULL ? ent->get_name().c_str() : "NO ENTITY"), (ent != NULL ? ent->get_abs_position().x : 0.0f), (ent != NULL ? ent->get_abs_position().y : 0.0f), (ent != NULL ? ent->get_abs_position().z : 0.0f));

	    cell_i = cells.begin();
      cell_iend = cells.end();

	    for(;cell_i != cell_iend; ++cell_i)
	    {
		    ai_polypath_cell* cell = *cell_i;

		    if (cell->is_point_in_cell_collumn(pt))
		    {
			    vector3d new_pos(pt);
			    cell->map_vector_height_to_cell(new_pos);
			    dist = __fabs(new_pos.y - pt.y);

			    if (found_home_cell)
			    {
				    if (dist < closest_height)
				    {
					    closest_cell = cell;
					    closest_height = dist;
				    }
			    }
			    else
			    {
				    closest_cell = cell;
				    closest_height = dist;
				    found_home_cell = true;
			    }
		    }

		    if (!found_home_cell)
		    {
			    vector2d start(cell->center().x, cell->center().z);
			    vector2d end(pt.x, pt.z);
			    ai_polypath_line motion_path(start, end);

			    ai_polypath_cell* next_cell;
			    ai_polypath_cell::eCellSide wall_hit;
			    vector2d intersection_pt;

			    ai_polypath_cell::ePathResult res = cell->classify_path_to_cell(motion_path, &next_cell, wall_hit, &intersection_pt);

			    if (res == ai_polypath_cell::EXITING_CELL)
			    {
				    vector3d closest_pt_3d(intersection_pt.x,0.0f,intersection_pt.y);
				    cell->map_vector_height_to_cell(closest_pt_3d);

				    closest_pt_3d -= pt;

				    dist = closest_pt_3d.length2();

				    if (dist < closest_dist)
				    {
					    closest_dist = dist;
					    closest_cell = cell;
				    }
			    }
		    }
	    }

      if(closest_cell == NULL)
        error("No polypath cells found in level (second search)!");
    }
  }

	return (closest_cell);
}


bool ai_polypath::find_path(ai_path& path, ai_polypath_cell* start_cell, const vector3d& start_pos, ai_polypath_cell* end_cell, const vector3d& end_pos)
{
	bool found_path = false;

	// Increment our path finding session ID
	// This Identifies each pathfinding session
	// so we do not need to clear out old data
	// in the cells from previous sessions.
	++session_id;

	// load our data into the NavigationHeap object
	// to prepare it for use.
	nav_heap.setup(session_id, start_pos);

	// We are doing a reverse search, from end_cell to start_cell.
	// Push our end_cell onto the Heap at the first cell to be processed
	end_cell->query_for_path(&nav_heap, 0, 0);

	// process the heap until empty, or a path is found
	while(nav_heap.not_empty() && !found_path)
	{
		ai_polypath_node node;

		// pop the top cell (the open cell with the lowest cost) off the Heap
		nav_heap.get_top(node);

		// if this cell is our start_cell, we are done
		if(node.cell == start_cell)
			found_path = true;
		else
		{
			// Process the Cell, Adding it's neighbors to the Heap as needed
			node.cell->process_cell(&nav_heap);
		}
	}

	// if we found a path, build a waypoint list
	// out of the cells on the path
	if (found_path)
	{
		ai_polypath_cell* test_cell = start_cell;
		vector3d new_way_pt;

		// Setup the Path object, clearing out any old data
		path.setup(this, start_pos, start_cell, end_pos, end_cell);

		// Step through each cell linked by our A* algorythm
		// from start_cell to end_cell
		while (test_cell && test_cell != end_cell)
		{
			// add the link point of the cell as a way point (the exit wall's center)
			int link_wall = test_cell->arrival_wall();

			new_way_pt = test_cell->wall_midpoint(link_wall);
			new_way_pt = snap_to_cell(test_cell, new_way_pt); // just to be sure

			path.add_waypoint(new_way_pt, test_cell);

			// and on to the next cell
			test_cell = test_cell->link(link_wall);
		}

		// cap the end of the path.
		path.end_path();

		return(true);
	}
	return(false);
}


bool ai_polypath::los_test(ai_polypath_cell* start_cell, const vector3d& start_pos, ai_polypath_cell* end_cell, const vector3d& end_pos)
{
	ai_polypath_line motion_path(vector2d(start_pos.x,start_pos.z), vector2d(end_pos.x,end_pos.z));
	ai_polypath_cell* next_cell = start_cell;
	ai_polypath_cell::eCellSide wall_num;
	ai_polypath_cell::ePathResult res;

#if defined(BUILD_DEBUG)
  ai_polypath_cell* cells[2] = { NULL, NULL };
  int i = 0;
#endif

	while((res = next_cell->classify_path_to_cell(motion_path, &next_cell, wall_num, 0)) == ai_polypath_cell::EXITING_CELL)
	{
#if defined(BUILD_DEBUG)
    if(next_cell && cells[i] == next_cell)
    {
      warning("Infinite loop detected. Degenerate TRI data in polypath, please re-export '.path' file");
      return(false);
    }

    cells[i] = next_cell;
    ++i;
    if(i > 1)
      i = 0;
#endif

    if (!next_cell)
      return(false);
	}

	return (res == ai_polypath_cell::ENDING_CELL && next_cell == end_cell);
}

void ai_polypath::link()
{
	CELL_ARRAY::iterator IterA = cells.begin();

	while (IterA != cells.end())
	{
		ai_polypath_cell* cellA = *IterA;
		CELL_ARRAY::iterator IterB = cells.begin();

		while (IterB != cells.end())
		{
			ai_polypath_cell* cellB = *IterB;

			if (IterA != IterB)
			{
				if (!cellA->link(ai_polypath_cell::SIDE_AB) && cellB->request_link(cellA->vertex(0), cellA->vertex(1), cellA))
				{
					cellA->set_link(ai_polypath_cell::SIDE_AB, cellB);
				}
				else if (!cellA->link(ai_polypath_cell::SIDE_BC) && cellB->request_link(cellA->vertex(1), cellA->vertex(2), cellA))
				{
					cellA->set_link(ai_polypath_cell::SIDE_BC, cellB);
				}
				else if (!cellA->link(ai_polypath_cell::SIDE_CA) && cellB->request_link(cellA->vertex(2), cellA->vertex(0), cellA))
				{
					cellA->set_link(ai_polypath_cell::SIDE_CA, cellB);
				}
			}

			++IterB;
		}

		++IterA;
	}
}


void ai_polypath::optimize()
{
	CELL_ARRAY::iterator cell_i = cells.begin();
	CELL_ARRAY::iterator cell_i_end = cells.end();

	while (cell_i != cell_i_end)
	{
    (*cell_i)->compute_sector(g_world_ptr->get_the_terrain(), true);
		++cell_i;
  }
}


void ai_polypath::render()
{
#ifndef BUILD_BOOTABLE
	CELL_ARRAY::iterator cell_i = cells.begin();
	CELL_ARRAY::iterator cell_i_end = cells.end();

	while (cell_i != cell_i_end)
	{
		(*cell_i)->render();
		++cell_i;
  }
#endif
}







//#pragma todo("this is crude. need to implement better loading JDB 03-09-01")
void serial_in(chunk_file &fs, ai_polypath *path)
{
  bool warned = false;

  static vector<vector3d> pts;
  pts.resize(0);

  path->clear();

  stringx label;
  for(serial_in(fs, &label); label != chunkend_label && label.size() > 0; serial_in(fs, &label))
  {
    if(label == "pt")
    {
      vector3d pt;
      serial_in(fs, &pt);
      pts.push_back(pt);
    }
    else if(label == "tri")
    {
      int index[3];
      for(int i=0; i<3; ++i)
        serial_in(fs, &index[i]);

      if(index[0] < (int) pts.size() && index[1] < (int) pts.size() && index[2] < (int) pts.size() && index[0] != index[1] && index[1] != index[2] && index[2] != index[0])
      {
        vector3d ptA = (pts[index[1]] - pts[index[0]]);
        vector3d ptB = (pts[index[2]] - pts[index[0]]);
        vector3d norm = cross(ptA,ptB);

        if(norm.y < 0.0f)
        {
          if(!warned)
            warning("PolyPath '%s' has bad data, please re-export!", fs.get_filename().c_str());

          warned = true;
          int temp = index[1];
          index[1] = index[2];
          index[2] = temp;
        }

        path->add_cell(pts[index[0]], pts[index[1]], pts[index[2]]);
      }
    }
  }

  path->link();
}



void ai_path::render( camera* camera_link )
{
#ifndef BUILD_BOOTABLE
  waypoint_ID p = waypoints.begin();
  waypoint_ID p_end = waypoints.end();

  while(p != p_end)
  {
    vector3d posa = (*p).pos;
    ++p;

    render_marker(camera_link, posa, color32(255, 0, 255, 192), 0.15f);

    if(p != p_end)
    {
      vector3d posb = (*p).pos;

      render_beam(posa, posb, color32(255, 0, 0, 128), 0.05f);
    }
  }

  waypoint_ID now = waypoints.begin();
  waypoint_ID next = get_furthest_visible_waypoint(now);

  while(next != p_end)
  {
    render_beam((*now).pos, (*next).pos, color32(0, 0, 255, 128), 0.05f);

    now = next;
    next = get_furthest_visible_waypoint(now);
  }

#endif
}

bool ai_path::get_way_point( const vector3d &pos, const vector3d &lastpos, vector3d &local_dest )
{
  if(reset)
  {
    goto_start = pos;
    next_point = get_furthest_visible_waypoint(cur_point);
    reset = false;
  }

  vector3d delta = (*next_point).pos - pos;
  delta.y = 0.0f;
  vector3d move_delta = (*next_point).pos - goto_start;
  move_delta.y = 0.0f;
  if(dot(delta, move_delta) < 0.0f)
  {
    cur_point = next_point;
    next_point = get_furthest_visible_waypoint(cur_point);
    goto_start = pos;
  }

  if(next_point == waypoints.end() || cur_point == waypoints.end())
    return(false);
  else
  {
    local_dest = (*next_point).pos;
    return(true);
  }
}
