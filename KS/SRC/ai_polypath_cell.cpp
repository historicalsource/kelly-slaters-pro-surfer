#include "global.h"
/* Some code from Game Programming Gems
 * "Portions Copyright (C) Greg Snook, 2000"
 */
#include "ai_polypath_cell.h"
#include "ai_polypath_heap.h"
#include "wds.h"
#include "terrain.h"
#include "collide.h"
#include "debug_render.h"


//#pragma todo("Share vertex data of cells with the main graph (use indices into an array/vector)")
void ai_polypath_cell::init(const vector3d& a, const vector3d& b, const vector3d& c)
{
	m_Vertex[VERT_A] = a;
	m_Vertex[VERT_B] = b;
	m_Vertex[VERT_C] = c;

	// object must be re-linked
	m_Link[SIDE_AB] = NULL;
	m_Link[SIDE_BC] = NULL;
	m_Link[SIDE_CA] = NULL;

	// now that the vertex pointers are set, compute additional data about the Cell
	compute_cell_data();

#ifndef BUILD_BOOTABLE
  sector_valid = 0;
#endif
}

void ai_polypath_cell::compute_cell_data()
{
  static rational_t inv_3 = 1.0f / 3.0f;

	// create 2D versions of our verticies
	vector2d Point1(m_Vertex[VERT_A].x, m_Vertex[VERT_A].z);
	vector2d Point2(m_Vertex[VERT_B].x, m_Vertex[VERT_B].z);
	vector2d Point3(m_Vertex[VERT_C].x, m_Vertex[VERT_C].z);

	// innitialize our sides
	m_Side[SIDE_AB].set_points(Point1, Point2);	// line AB
	m_Side[SIDE_BC].set_points(Point2, Point3);	// line BC
	m_Side[SIDE_CA].set_points(Point3, Point1);	// line CA

	m_CellPlane.set(m_Vertex[VERT_A], m_Vertex[VERT_B], m_Vertex[VERT_C]);

	// compute midpoint as centroid of polygon
	m_CenterPoint.x = ((m_Vertex[VERT_A].x + m_Vertex[VERT_B].x + m_Vertex[VERT_C].x)*inv_3);
	m_CenterPoint.y = ((m_Vertex[VERT_A].y + m_Vertex[VERT_B].y + m_Vertex[VERT_C].y)*inv_3);
	m_CenterPoint.z = ((m_Vertex[VERT_A].z + m_Vertex[VERT_B].z + m_Vertex[VERT_C].z)*inv_3);

	// compute the midpoint of each cell wall
	m_WallMidpoint[0].x = (m_Vertex[VERT_A].x + m_Vertex[VERT_B].x)*0.5f;
	m_WallMidpoint[0].y = (m_Vertex[VERT_A].y + m_Vertex[VERT_B].y)*0.5f;
	m_WallMidpoint[0].z = (m_Vertex[VERT_A].z + m_Vertex[VERT_B].z)*0.5f;

	m_WallMidpoint[1].x = (m_Vertex[VERT_C].x + m_Vertex[VERT_B].x)*0.5f;
	m_WallMidpoint[1].y = (m_Vertex[VERT_C].y + m_Vertex[VERT_B].y)*0.5f;
	m_WallMidpoint[1].z = (m_Vertex[VERT_C].z + m_Vertex[VERT_B].z)*0.5f;

	m_WallMidpoint[2].x = (m_Vertex[VERT_C].x + m_Vertex[VERT_A].x)*0.5f;
	m_WallMidpoint[2].y = (m_Vertex[VERT_C].y + m_Vertex[VERT_A].y)*0.5f;
	m_WallMidpoint[2].z = (m_Vertex[VERT_C].z + m_Vertex[VERT_A].z)*0.5f;

	// compute the distances between the wall midpoints
	vector3d WallVector;
	WallVector = m_WallMidpoint[0] - m_WallMidpoint[1];
	m_WallDistance[0] = WallVector.length();

	WallVector = m_WallMidpoint[1] - m_WallMidpoint[2];
	m_WallDistance[1] = WallVector.length();

	WallVector = m_WallMidpoint[2] - m_WallMidpoint[0];
	m_WallDistance[2] = WallVector.length();

  radius = 0.0f;
  for(int i=0; i<3; ++i)
  {
	  rational_t len = (m_Vertex[0] - m_CenterPoint).length2();
    if(len > radius)
      radius = len;
  }
  if(radius > 0.0f)
    radius = __fsqrt(radius);
}

//:	RequestLink
//----------------------------------------------------------------------------------------
//
//	Navigation Mesh is created as a pool of raw cells. The cells are then compared against
//	each other to find common edges and create links. This routine is called from a
//	potentially adjacent cell to test if a link should exist between the two.
//
//-------------------------------------------------------------------------------------://
bool ai_polypath_cell::request_link(const vector3d& a, const vector3d& b, ai_polypath_cell* caller)
{
	// return true if we share the two provided verticies with the calling cell.
	if (m_Vertex[VERT_A] == a)
	{
		if (m_Vertex[VERT_B] == b)
		{
			m_Link[SIDE_AB] = caller;
			return (true);
		}
		else if (m_Vertex[VERT_C] == b)
		{
			m_Link[SIDE_CA] = caller;
			return (true);
		}
	}
	else if (m_Vertex[VERT_B] == a)
	{
		if (m_Vertex[VERT_A] == b)
		{
			m_Link[SIDE_AB] = caller;
			return (true);
		}
		else if (m_Vertex[VERT_C] == b)
		{
			m_Link[SIDE_BC] = caller;
			return (true);
		}
	}
	else if (m_Vertex[VERT_C] == a)
	{
		if (m_Vertex[VERT_A] == b)
		{
			m_Link[SIDE_CA] = caller;
			return (true);
		}
		else if (m_Vertex[VERT_B] == b)
		{
			m_Link[SIDE_BC] = caller;
			return (true);
		}
	}

	// we are not adjacent to the calling cell
	return (false);
}




ai_polypath_cell::ePathResult ai_polypath_cell::classify_path_to_cell(const ai_polypath_line& motion_path, ai_polypath_cell** next_cell, eCellSide& side, vector2d* intersect)const
{
	int interior_cnt = 0;

	// Check our motion_path against each of the three cell walls
	for (int i=0; i<3; ++i)
	{
		if (m_Side[i].classify_point(motion_path.b) != ai_polypath_line::RIGHT_SIDE)
		{
			if (m_Side[i].classify_point(motion_path.a) != ai_polypath_line::LEFT_SIDE)
			{
				ai_polypath_line::eLineClassification res = motion_path.intersection(m_Side[i], intersect);

				if (res == ai_polypath_line::SEGMENTS_INTERSECT || res == ai_polypath_line::A_BISECTS_B)
				{
					*next_cell = m_Link[i];
					side = (eCellSide)i;
					return (EXITING_CELL);
				}
			}
		}
		else
			interior_cnt++;
	}

  return((interior_cnt == 3) ? ENDING_CELL : NO_RELATIONSHIP);
}

void ai_polypath_cell::project_path_on_cell_wall(eCellSide side_number, ai_polypath_line& motion_path)const
{
	// compute the normalized vector of the cell wall in question
	vector2d WallNormal = m_Side[side_number].b - m_Side[side_number].a;
	WallNormal.normalize();

	// determine the vector of our current movement
	vector2d MotionVector = motion_path.b - motion_path.a;

	// compute dot product of our MotionVector and the normalized cell wall
	// this gives us the magnatude of our motion along the wall
	// our projected vector is then the normalized wall vector times our NEW found magnatude
	MotionVector = (WallNormal * dot(MotionVector,WallNormal));

	// redirect our motion path along the NEW reflected direction
	motion_path.set_point_b(motion_path.a + MotionVector);

	//
	// Make sure starting point of motion path is within the cell
	//
	vector2d NewPoint = motion_path.a;
	force_to_cell_collumn(NewPoint);
	motion_path.set_point_a(NewPoint);

	//
	// Make sure destination point does not intersect this wall again
	//
	NewPoint = motion_path.b;
	force_to_wall_interior(side_number, NewPoint);
	motion_path.set_point_b(NewPoint);

}

bool ai_polypath_cell::force_to_wall_interior(eCellSide side_number, vector2d& test_pt)const
{
	rational_t distance = m_Side[side_number].signed_distance(test_pt);
	rational_t epsilon = 0.001f;

	if (distance <= epsilon)
	{
		if (distance <= 0.0f)
			distance -= epsilon;

		distance = __fabs(distance);
		distance = (epsilon > distance ? epsilon : distance);

		// this point needs adjustment
		test_pt += (m_Side[side_number].normal() * distance);

		return (true);
	}
	return (false);
}

bool ai_polypath_cell::force_to_wall_interior(eCellSide side_number, vector3d& test_pt)const
{
	vector2d test_pt2D(test_pt.x,test_pt.z);
	if (force_to_wall_interior(side_number, test_pt2D))
	{
		test_pt.x = test_pt2D.x;
		test_pt.z = test_pt2D.y;

    return(true);
	}

	return (false);
}

bool ai_polypath_cell::force_to_cell_collumn(vector2d& test_pt)const
{
	// create a motion path from the center of the cell to our point
	ai_polypath_line test_path(vector2d(m_CenterPoint.x, m_CenterPoint.z), test_pt);
	vector2d intersect;
	eCellSide side;
	ai_polypath_cell* next_cell;

	ePathResult result = classify_path_to_cell(test_path, &next_cell, side, &intersect);
	// compare this path to the cell.

	if (result == EXITING_CELL)
	{
		vector2d path_direction(intersect.x - m_CenterPoint.x, intersect.y - m_CenterPoint.z);

		path_direction *= 0.9f;

		test_pt.x = m_CenterPoint.x + path_direction.x;
		test_pt.y = m_CenterPoint.z + path_direction.y;
		return (true);
	}
	else if (result == NO_RELATIONSHIP)
	{
		test_pt.x = m_CenterPoint.x;
		test_pt.y = m_CenterPoint.z;
		return (true);
	}

	return (false);
}

bool ai_polypath_cell::force_to_cell_collumn(vector3d& test_pt)const
{
	vector2d test_pt2D(test_pt.x,test_pt.z);
	if (force_to_cell_collumn(test_pt2D))
	{
		test_pt.x=test_pt2D.x;
		test_pt.z=test_pt2D.y;

    return(true);
	}

	return (false);
}

bool ai_polypath_cell::process_cell(ai_polypath_heap* heap)
{
	if (m_SessionID == heap->session_id())
	{
		// once we have been processed, we are closed
		m_Open  = false;

		// querry all our neigbors to see if they need to be added to the Open heap
		for (int i=0;i<3;++i)
		{
			if (m_Link[i])
			{
				// abs(i-m_ArrivalWall) is a formula to determine which distance measurement to use.
				// The Distance measurements between the wall midpoints of this cell
				// are held in the order ABtoBC, BCtoCA and CAtoAB.
				// We add this distance to our known m_ArrivalCost to compute
				// the total cost to reach the next adjacent cell.
				m_Link[i]->query_for_path(heap, this, m_ArrivalCost+m_WallDistance[abs(i-m_ArrivalWall)]);
			}
		}
		return(true);
	}
	return(false);
}

bool ai_polypath_cell::query_for_path(ai_polypath_heap* heap, ai_polypath_cell* caller, rational_t arrivalcost)
{
	if (m_SessionID!=heap->session_id())
	{
		// this is a NEW session, reset our internal data
		m_SessionID = heap->session_id();

		if (caller)
		{
			m_Open  = true;
			compute_heuristic(heap->goal());
			m_ArrivalCost = arrivalcost;

			// remember the side this caller is entering from
			if (caller == m_Link[0])
			{
				m_ArrivalWall = 0;
			}
			else if (caller == m_Link[1])
			{
				m_ArrivalWall = 1;
			}
			else if (caller == m_Link[2])
			{
				m_ArrivalWall = 2;
			}
		}
		else
		{
			// we are the cell that contains the starting location
			// of the A* search.
			m_Open  = false;
			m_ArrivalCost = 0;
			m_Heuristic = 0;
			m_ArrivalWall = 0;
		}

		// add this cell to the Open heap
		heap->add_cell(this);

		return(true);
	}
	else if (m_Open)
	{
		// m_Open means we are already in the Open Heap.
		// If this NEW caller provides a better path, adjust our data
		// Then tell the Heap to resort our position in the list.
		if ((arrivalcost + m_Heuristic) < (m_ArrivalCost + m_Heuristic))
		{
				m_ArrivalCost = arrivalcost;

				// remember the side this caller is entering from
				if (caller == m_Link[0])
				{
					m_ArrivalWall = 0;
				}
				else if (caller == m_Link[1])
				{
					m_ArrivalWall = 1;
				}
				else if (caller == m_Link[2])
				{
					m_ArrivalWall = 2;
				}

				// ask the heap to resort our position in the priority heap
				heap->adjust_cell(this);

				return(true);
		}
	}
	// this cell is closed
	return(false);
}

void ai_polypath_cell::compute_heuristic(const vector3d& goal)
{
	// our heuristic is the estimated distance (using the longest axis delta) between our
	// cell center and the goal location

	rational_t XDelta = __fabs(goal.x - m_CenterPoint.x);
	rational_t YDelta = __fabs(goal.y - m_CenterPoint.y);
	rational_t ZDelta = __fabs(goal.z - m_CenterPoint.z);

#ifdef _WIN32
	m_Heuristic = __max(__max(XDelta,YDelta), ZDelta);
#else
	m_Heuristic = max(max(XDelta,YDelta), ZDelta);
#endif
}

static vector<region_node*> cell_regions(32);
void ai_polypath_cell::compute_sector(terrain& ter, bool use_high_res_intersect)
{
#ifndef BUILD_BOOTABLE
  sector_valid = 0;
#endif

  sector* sec = ter.find_sector( m_CenterPoint );
  if (sec)
  {
    assert( sec->get_region() );

    cell_regions.resize(0);
    region::prepare_for_visiting();

    sphere my_sphere(m_CenterPoint, radius);

    _intersect( sec->get_region(), my_sphere, use_high_res_intersect );

    vector<region_node*>::iterator reg;
    for ( reg=cell_regions.begin(); reg!=cell_regions.end(); ++reg )
      (*reg)->get_data()->add(this);

#ifndef BUILD_BOOTABLE
    sector_valid = 1;
#endif
  }
  else
  {
    sector* sec[3];
    for(int i=0; i<3; ++i)
      sec[i] = ter.find_sector( m_Vertex[i] );

    if(sec[0] != NULL && sec[1] != NULL && sec[2] != NULL && sec[0]->get_region() == sec[1]->get_region() && sec[1]->get_region() == sec[2]->get_region())
    {
      sec[0]->get_region()->get_data()->add(this);
#ifndef BUILD_BOOTABLE
      sector_valid = -1;
#endif
    }
  }

#ifndef BUILD_BOOTABLE
  if(!sector_valid)
    warning("A polypath triangle is outside the world and cannot be auto-fixed! (center <%.2f, %.2f, %.2f>), please check and re-export!!", m_CenterPoint.x, m_CenterPoint.y, m_CenterPoint.z);
#endif
}


void ai_polypath_cell::_intersect( region_node* r, const sphere &the_sphere, bool use_high_res_intersect )
{
  // add region to list
  r->get_data()->visit();
  cell_regions.push_back( r );

  // check for intersection with portals leading from this region
  edge_iterator tei = r->begin();
  edge_iterator tei_end = r->end();
  for ( ; tei!=tei_end; ++tei )
  {
    // don't bother with regions we've already visited
    region_node* dest = (*tei).get_dest();
    if ( !dest->get_data()->already_visited() )
    {
      portal* port = (*tei).get_data();
      // don't need to recurse across inactive portals, unless you're a door or doorframe!
      // intersection of entity sphere and portal cylinder
      if ( port->touches_sphere(the_sphere) )
      {
        if ( use_high_res_intersect )
        {
          const vector3d& v0 = the_sphere.get_center();
          vector3d hit_loc;

          bool intersected = false;
          for ( int j=0; j<port->get_max_faces() && !intersected; ++j )
          {
            for ( int i=0; i<3 && !intersected; ++i )
            {
              vector3d v1 = m_Vertex[i];
              vector3d v2 = i < 2 ? m_Vertex[i+1] : m_Vertex[0];
              vector3d v3 = m_WallMidpoint[i];

              if ( collide_polygon_segment( j, port, v0, v1, hit_loc )    // center to vertex
                || collide_polygon_segment( j, port, v1, v2, hit_loc )    // side of triangle
                || collide_polygon_segment( j, port, v0, v3, hit_loc ))   // center to midpoint of side
              {
                _intersect( dest, the_sphere, use_high_res_intersect );
                intersected = true;
                break;
              }
            }
          }
        }
        else
          _intersect( dest, the_sphere, use_high_res_intersect );
      }
    }
  }
}


void ai_polypath_cell::render()
{
#ifndef BUILD_BOOTABLE
  render_beam(m_Vertex[0], m_Vertex[1], color32(0, 255, 255, 128), 0.05f);
  render_beam(m_Vertex[1], m_Vertex[2], color32(0, 255, 255, 128), 0.05f);
  render_beam(m_Vertex[2], m_Vertex[0], color32(0, 255, 255, 128), 0.05f);

  switch(sector_valid)
  {
    case -1:
      render_triangle(m_Vertex[0], m_Vertex[1], m_Vertex[2], color32(255, 255, 0, 96), true);
      break;

    case 0:
      render_triangle(m_Vertex[0], m_Vertex[1], m_Vertex[2], color32(255, 0, 0, 96), true);
      break;

    case 1:
    default:
      render_triangle(m_Vertex[0], m_Vertex[1], m_Vertex[2], color32(0, 255, 0, 96), true);
      break;
  }
#endif
}
