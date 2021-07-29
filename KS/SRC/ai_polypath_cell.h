/* Some code from Game Programming Gems
 * "Portions Copyright (C) Greg Snook, 2000"
 */
#ifndef _AI_POLYPATH_CELL_H_
#define _AI_POLYPATH_CELL_H_

#include "global.h"
#include "portal.h"

class ai_polypath_heap;
class terrain;

class ai_polypath_line
{
public:
	enum ePointClassification
	{
		ON_LINE,		// The point is on, or very near, the line
		LEFT_SIDE,		// looking from endpoint A to B, the test point is on the left
		RIGHT_SIDE		// looking from endpoint A to B, the test point is on the right
	};

	enum eLineClassification
	{
		COLLINEAR,			// both lines are parallel and overlap each other
		LINES_INTERSECT,	// lines intersect, but their segments do not
		SEGMENTS_INTERSECT,	// both line segments bisect each other
		A_BISECTS_B,		// line segment B is crossed by line A
		B_BISECTS_A,		// line segment A is crossed by line B
		PARALELL			// the lines are paralell
	};


	ai_polypath_line()
    : norm_calc(false)
  {
  }

	ai_polypath_line& operator=( const ai_polypath_line& src)
  {
    a = src.a;
	  b = src.b;
	  norm = src.norm;
	  norm_calc = src.norm_calc;
	  return (*this);
  }

  ai_polypath_line( const ai_polypath_line& src)
  {
    *this = src;
  }

	ai_polypath_line( const vector2d& PointA, const vector2d& PointB)
  {
    a = PointA;
    b = PointB;
    norm_calc = false;
  }

	~ai_polypath_line()
  {
  }

	// ----- MUTATORS ---------------------
	inline void set_point_a(const vector2d& pt)
  {
  	norm_calc = false;
    a = pt;
  }

	inline void set_point_b(const vector2d& pt)
  {
  	norm_calc = false;
    b = pt;
  }

	inline void set_points(const vector2d& pta, const vector2d& ptb)
  {
  	norm_calc = false;
    a = pta;
    b = ptb;
  }

	inline void set_points(rational_t ptAx, rational_t ptAy, rational_t ptBx, rational_t ptBy)
  {
	  a.x=ptAx;
	  a.y=ptAy;
	  b.x=ptBx;
	  b.y=ptBy;
	  norm_calc = false;
  }

	void get_direction(vector2d& dir)const
  {
	  dir = (b - a);
	  dir.normalize();
  }

private:
	void compute_norm() const
  {
	  //
	  // Get Normailized direction from A to B
	  //
	  get_direction(norm);

	  //
	  // Rotate by -90 degrees to get normal of line
	  //
	  rational_t OldYValue = norm.y;
	  norm.y = -norm.x;
	  norm.x = OldYValue;
	  norm_calc = true;
  }

public:
	inline rational_t signed_distance(const vector2d& pt) const
  {
	  if (!norm_calc)
		  compute_norm();

	  vector2d vec(pt - a);
	  
	  return dot(vec,norm);
  }

	inline ePointClassification classify_point(const vector2d& pt, rational_t epsilon = 0.0f) const
  {
    ePointClassification res = ON_LINE;
    rational_t dist = signed_distance(pt);
    
    if (dist > epsilon)
      res = RIGHT_SIDE;
    else if (dist < -epsilon)
      res = LEFT_SIDE;

    return(res);
  }

	eLineClassification intersection(const ai_polypath_line& line, vector2d* intersect=NULL)const
  {
	  rational_t Ay_minus_Cy = a.y - line.a.y;	
	  rational_t Dx_minus_Cx = line.b.x - line.a.x;	
	  rational_t Ax_minus_Cx = a.x - line.a.x;	
	  rational_t Dy_minus_Cy = line.b.y - line.a.y;	
	  rational_t Bx_minus_Ax = b.x - a.x;	
	  rational_t By_minus_Ay = b.y - a.y;	

	  rational_t Numerator = (Ay_minus_Cy * Dx_minus_Cx) - (Ax_minus_Cx * Dy_minus_Cy);
	  rational_t Denominator = (Bx_minus_Ax * Dy_minus_Cy) - (By_minus_Ay * Dx_minus_Cx);

	  // if lines do not intersect, return now
	  if (Denominator == 0.0f)
	  {
		  if (Numerator == 0.0f)
			  return COLLINEAR;

		  return PARALELL;
	  }

	  rational_t FactorAB = Numerator / Denominator;
	  rational_t FactorCD = ((Ay_minus_Cy * Bx_minus_Ax) - (Ax_minus_Cx * By_minus_Ay)) / Denominator;

	  // posting (hitting a vertex exactly) is not allowed, shift the results
	  // if they are within a minute range of the end vertecies
  /*	if (fabs(FactorCD) < 1.0e-6f)
	  {
		  FactorCD = 1.0e-6f;
	  }
	  if (fabs(FactorCD - 1.0f) < 1.0e-6f)
	  {
		  FactorCD = 1.0f - 1.0e-6f;
	  }
  */

	  // if an interection point was provided, fill it in now
	  if (intersect)
	  {
		  intersect->x = (a.x + (FactorAB * Bx_minus_Ax));
		  intersect->y = (a.y + (FactorAB * By_minus_Ay));
	  }

	  // now determine the type of intersection
	  if ((FactorAB >= 0.0f) && (FactorAB <= 1.0f) && (FactorCD >= 0.0f) && (FactorCD <= 1.0f))
	  {
		  return SEGMENTS_INTERSECT;
	  }
	  else if ((FactorCD >= 0.0f) && (FactorCD <= 1.0f))
	  {
		  return (A_BISECTS_B);
	  }
	  else if ((FactorAB >= 0.0f) && (FactorAB <= 1.0f))
	  {
		  return (B_BISECTS_A);
	  }

	  return LINES_INTERSECT;
  }

	// ----- ACCESSORS --------------------
	const vector2d& normal()const
  {
	  if (!norm_calc)
		  compute_norm();

	  return (norm);
  }

	inline rational_t length() const
  {
	  rational_t xdist = b.x-a.x;
	  rational_t ydist = b.y-a.y;

	  xdist *= xdist;
	  ydist *= ydist;

	  return(__fsqrt(xdist + ydist));
  }

	// ----- DATA -------------------------
	vector2d a;	// Endpoint A of our line segment
	vector2d b;	// Endpoint B of our line segment

	mutable vector2d norm;	// 'normal' of the ray. 
								// a vector pointing to the right-hand side of the line
								// when viewed from PointA towards PointB
	mutable bool norm_calc; // normals are only calculated on demand
};

class ai_polypath_plane
{
public:
  ai_polypath_plane()
  {
  }

	inline ai_polypath_plane& operator=( const ai_polypath_plane& src )
  {
    norm = src.norm;
    pt = src.pt;
    dist = src.dist;

  	return (*this);
  }

  ai_polypath_plane(const ai_polypath_plane& src)
  {
    *this = src;
  }

  inline void set(const vector3d& pt0, const vector3d& pt1, const vector3d& pt2)
  {
    vector3d ptA = (pt1 - pt0);
    vector3d ptB = (pt2 - pt0);
    norm = cross(ptA,ptB);
    norm.normalize();
    pt = pt0;

    dist = -dot(pt,norm);
  }
  
  ai_polypath_plane(const vector3d& pt0, const vector3d& pt1, const vector3d& pt2)
  {
    set(pt0, pt1, pt2);
  }

  ~ai_polypath_plane()
  {
  }

	// tests if data is identical
	inline friend bool operator==( const ai_polypath_plane& plane_a, const ai_polypath_plane& plane_b )
	{
		return (plane_a.norm == plane_b.norm && plane_a.pt==plane_b.pt);
	}

  inline rational_t solve_for_x(rational_t Y, rational_t Z)const
  {
	  //Ax + By + Cz + D = 0
	  // Ax = -(By + Cz + D)
	  // x = -(By + Cz + D)/A

	  if (norm.x != 0.0f)
	  {
		  return ( -(norm.y*Y + norm.z*Z + dist) / norm.x );
	  }

	  return (0.0f);
  }

	inline rational_t solve_for_y(rational_t X, rational_t Z)const
  {
	  //Ax + By + Cz + D = 0
	  // By = -(Ax + Cz + D)
	  // y = -(Ax + Cz + D)/B

	  if (norm.y != 0.0f)
	  {
		  return ( -(norm.x*X + norm.z*Z + dist) / norm.y );
	  }

	  return (0.0f);
  }

	inline rational_t solve_for_z(rational_t X, rational_t Y)const
  {
	  //Ax + By + Cz + D = 0
	  // Cz = -(Ax + By + D)
	  // z = -(Ax + By + D)/C

	  if (norm.z != 0.0f)
	  {
		  return ( -(norm.x*X + norm.y*Y + dist) / norm.z );
	  }

	  return (0.0f);
  }

  vector3d     norm;
  vector3d     pt;
  rational_t       dist;
};

class ai_polypath_cell
{
public:
	enum eCellVert
	{
		VERT_A = 0,
		VERT_B,
		VERT_C
	};

	enum eCellSide
	{
		SIDE_AB = 0,
		SIDE_BC,
		SIDE_CA
	};

	enum ePathResult
	{
		NO_RELATIONSHIP,		// the path does not cross this cell
		ENDING_CELL,			// the path ends in this cell	
		EXITING_CELL,			// the path exits this cell through side X
	};

	// ----- CREATORS ---------------------

	inline ai_polypath_cell()
    : m_SessionID(0)
  {
  }

  inline ai_polypath_cell& operator=( const ai_polypath_cell& src)
  {
	  if (this != &src)
	  {
		  m_CellPlane = src.m_CellPlane;		
		  m_CenterPoint = src.m_CenterPoint;	
		  m_SessionID= src.m_SessionID;
		  m_ArrivalCost= src.m_ArrivalCost;
		  m_Heuristic= src.m_Heuristic;
		  m_Open= src.m_Open;
		  m_ArrivalWall= src.m_ArrivalWall;

		  for (int i=0; i<3; ++i)
		  {
			  m_Vertex[i] = src.m_Vertex[i];
			  m_Side[i] = src.m_Side[i];
			  m_Link[i] = src.m_Link[i];
			  m_WallMidpoint[i] = src.m_WallMidpoint[i];
			  m_WallDistance[i] = src.m_WallDistance[i];
		  }
	  }

	  return (*this);
  }

  inline ai_polypath_cell( const ai_polypath_cell& src) 
  { 
    *this = src; 
  }

	inline ~ai_polypath_cell()
  {
  }

	void init(const vector3d& a, const vector3d& b, const vector3d& c);
	void compute_cell_data();

	bool request_link(const vector3d& a, const vector3d& b, ai_polypath_cell* caller);
	inline void set_link(eCellSide side, ai_polypath_cell* caller)
  {
  	m_Link[side] = caller;
  }

	ePathResult classify_path_to_cell(const ai_polypath_line& motion_path, ai_polypath_cell** next_cell, eCellSide& side, vector2d* intersect)const;

	void project_path_on_cell_wall(eCellSide side_number, ai_polypath_line& motion_path)const;

	inline void map_vector_height_to_cell(vector3d& motion_path)const
  {
  	motion_path.y = m_CellPlane.solve_for_y(motion_path.x, motion_path.z);
  }

	bool force_to_cell_collumn(vector3d& test_pt)const;
	bool force_to_cell_collumn(vector2d& test_pt)const;
	bool force_to_wall_interior(eCellSide side_number, vector2d& test_pt)const;
	bool force_to_wall_interior(eCellSide side_number, vector3d& test_pt)const;

	bool process_cell(ai_polypath_heap* heap);
	bool query_for_path(ai_polypath_heap* heap, ai_polypath_cell* caller, rational_t arrivalcost);

	inline bool is_point_in_cell_collumn(const vector2d& test_pt)const
  {
  	return(m_Side[0].classify_point(test_pt) == ai_polypath_line::RIGHT_SIDE && m_Side[1].classify_point(test_pt) == ai_polypath_line::RIGHT_SIDE && m_Side[2].classify_point(test_pt) == ai_polypath_line::RIGHT_SIDE);
  }

	inline bool is_point_in_cell_collumn(const vector3d& test_pt)const
  {
	  vector2d pt(test_pt.x,test_pt.z);

	  return (is_point_in_cell_collumn(pt));
  }

  inline const vector3d& vertex(int Vert)const
  {
	  return(m_Vertex[Vert]);
  }

  inline const vector3d& center()const
  {
	  return(m_CenterPoint);
  }

  inline ai_polypath_cell* link(int side)const
  {
	  return(m_Link[side]);
  }

  inline rational_t arrival_cost()const
  {
	  return(m_ArrivalCost);
  }

  inline rational_t heuristic()const
  {
	  return(m_Heuristic);
  }

  inline rational_t pathfinding_cost()const
  {
	  return(m_ArrivalCost + m_Heuristic);
  }

  inline int arrival_wall()const
  {
	  return(m_ArrivalWall);
  }

  inline const vector3d wall_midpoint(int Side)const
  {
	  return(m_WallMidpoint[Side]);
  }

  void render();

private:
  friend class ai_polypath;

	ai_polypath_plane	m_CellPlane;		// A plane containing the cell triangle
	vector3d m_Vertex[3];		// pointers to the verticies of this triangle held in the NavigationMesh's vertex pool
	vector3d m_CenterPoint;		// The center of the triangle
  rational_t radius;
	ai_polypath_line	m_Side[3];			// a 2D line representing each cell Side
	ai_polypath_cell* m_Link[3];// pointers to cells that attach to this cell. A NULL link denotes a solid edge.

	// Pathfinding Data...
	int		m_SessionID;		// an identifier for the current pathfinding session.
	rational_t	m_ArrivalCost;		// total cost to use this cell as part of a path
	rational_t	m_Heuristic;		// our estimated cost to the goal from here
	bool	m_Open;				// are we currently listed as an Open cell to revisit and test?
	int		m_ArrivalWall;		// the side we arrived through.
	vector3d m_WallMidpoint[3];	// the pre-computed midpoint of each wall.
	rational_t	m_WallDistance[3];	// the distances between each wall midpoint of sides (0-1, 1-2, 2-0)
  
	void compute_heuristic(const vector3d& goal); // estimates the distance to the goal for A*

  void compute_sector(terrain& ter, bool use_high_res_intersect);
  void _intersect( region_node* r, const sphere &the_sphere, bool use_high_res_intersect );

#ifndef BUILD_BOOTABLE
  char sector_valid;
#endif
};

#endif
