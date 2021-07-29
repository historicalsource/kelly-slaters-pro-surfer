#include "global.h"
/* Some code (heavily modified) from Game Programming Gems
 * "Portions Copyright (C) Dante Treglia II, 2000"
 */
#include "b_spline.h"

const rational_t INV_SIX = (1.0f/6.0f);
//#define INV_SIX 0.16666666666666666666666666666667f

/*
#define B_SPLINE(u, u_2, u_3, cntrl0, cntrl1, cntrl2, cntrl3) \
	( \
		( \
			(-1.0f*u_3 + 3.0f*u_2 - 3.0f*u + 1.0f) * (cntrl0) + \
			( 3.0f*u_3 - 6.0f*u_2 + 0.0f*u + 4.0f) * (cntrl1) + \
			(-3.0f*u_3 + 3.0f*u_2 + 3.0f*u + 1.0f) * (cntrl2) + \
			( 1.0f*u_3 + 0.0f*u_2 + 0.0f*u + 0.0f) * (cntrl3)   \
		) / 6.0f \
	) 
*/

#define B_SPLINE_OPT(u, u_2, u_3, cntrl0, cntrl1, cntrl2, cntrl3) \
	( \
		( \
			(3.0f*u_2 - 3.0f*u - u_3 + 1.0f) * (cntrl0) + \
			(3.0f*u_3 - 6.0f*u_2 + 4.0f) * (cntrl1) + \
			(-3.0f*u_3 + 3.0f*u_2 + 3.0f*u + 1.0f) * (cntrl2) + \
			(u_3) * (cntrl3)   \
		) * INV_SIX \
	) 

#define B_TANGENT(u, u_2, cntrl0, cntrl1, cntrl2, cntrl3) \
	( \
		( \
			(-1.0f*u_2 + 2.0f*u - 1.0f) * (cntrl0) + \
			( 3.0f*u_2 - 4.0f*u + 0.0f) * (cntrl1) + \
			(-3.0f*u_2 + 2.0f*u + 1.0f) * (cntrl2) + \
			( 1.0f*u_2 + 0.0f*u + 0.0f) * (cntrl3)   \
		) * 0.5f \
	) 

/*
#define CATMULL_ROM_SPLINE(u, u_2, u_3, cntrl0, cntrl1, cntrl2, cntrl3) \
	( \
		( \
			(-1.0f*u_3 + 2.0f*u_2 - 1.0f*u + 0.0f) * (cntrl0) + \
			( 3.0f*u_3 - 5.0f*u_2 + 0.0f*u + 2.0f) * (cntrl1) + \
			(-3.0f*u_3 + 4.0f*u_2 + 1.0f*u + 0.0f) * (cntrl2) + \
			( 1.0f*u_3 - 1.0f*u_2 + 0.0f*u + 0.0f) * (cntrl3)   \
		) * 0.5f \
	) 
*/

#define CATMULL_ROM_SPLINE_OPT(u, u_2, u_3, cntrl0, cntrl1, cntrl2, cntrl3) \
	( \
		( \
			( 2.0f*u_2 - u_3 - u) * (cntrl0) + \
			( 3.0f*u_3 - 5.0f*u_2 + 2.0f) * (cntrl1) + \
			(-3.0f*u_3 + 4.0f*u_2 + u) * (cntrl2) + \
			( u_3 - u_2) * (cntrl3)   \
		) * 0.5f \
	) 

#define CATMULL_ROM_TANGENT(u, u_2, cntrl0, cntrl1, cntrl2, cntrl3) \
	( \
		( \
			(-3.0f*u_2 +  4.0f*u - 1.0f) * (cntrl0) + \
			( 9.0f*u_2 - 10.0f*u + 0.0f) * (cntrl1) + \
			(-9.0f*u_2 +  8.0f*u + 1.0f) * (cntrl2) + \
			( 3.0f*u_2 -  2.0f*u + 0.0f) * (cntrl3)   \
		) * 0.5f \
	) 


void b_spline::rebuild()
{
	static rational_t u, u_2, u_3;
  static vector3d ctrl[4];

  curve_pts.resize(0);

  int num_ctrl_pts = control_pts.size();
  if(force_start)
    num_ctrl_pts += 2;

  if(num_ctrl_pts >= 4 && sub_divisions > 1)
  {
    curve_pts.reserve(((num_ctrl_pts-3)*sub_divisions)+1);

    vector<vector3d>::iterator ctrl_1 = control_pts.begin();
    vector<vector3d>::iterator ctrl_end = control_pts.end();

    vector<vector3d>::iterator ctrl_2 = ctrl_1;
    if(!force_start)
      ++ctrl_2;

    vector<vector3d>::iterator ctrl_3 = ctrl_2;
    ++ctrl_3;

    vector<vector3d>::iterator ctrl_4 = ctrl_3;
    ++ctrl_4;

    bool first_pass = true;
    for( ; (force_start ? (ctrl_4 <= ctrl_end) : (ctrl_4 < ctrl_end)); ++ctrl_2, ++ctrl_3, ++ctrl_4)
    {
      ctrl[1] = *ctrl_2;
      ctrl[2] = *ctrl_3;
      ctrl[0] = (first_pass && force_start) ? (ctrl[1] + (ctrl[1] - ctrl[2])) : *ctrl_1;
      ctrl[3] = ctrl_4 < ctrl_end ? *ctrl_4 : (ctrl[2] + (ctrl[2] - ctrl[1]));

		  for(int j = 0; j < sub_divisions; j++) 
      {
        vector3d curve_pt;

			  u = (rational_t)j / (rational_t)(sub_divisions-1);
			  u_2 = u * u;
			  u_3 = u_2 * u;

        switch(spline_type)
        {
          case _B_SPLINE:
          {
				    curve_pt.x = B_SPLINE_OPT(u, u_2, u_3, 
									    ctrl[0].x,
									    ctrl[1].x,
									    ctrl[2].x,
									    ctrl[3].x);

				    curve_pt.y = B_SPLINE_OPT(u, u_2, u_3, 
									    ctrl[0].y,
									    ctrl[1].y,
									    ctrl[2].y,
									    ctrl[3].y);

				    curve_pt.z = B_SPLINE_OPT(u, u_2, u_3, 
									    ctrl[0].z,
									    ctrl[1].z,
									    ctrl[2].z,
									    ctrl[3].z);
          }
          break;

          case _CATMULL_ROM:
          {
				    curve_pt.x = CATMULL_ROM_SPLINE_OPT(u, u_2, u_3, 
									    ctrl[0].x,
									    ctrl[1].x,
									    ctrl[2].x,
									    ctrl[3].x);

				    curve_pt.y = CATMULL_ROM_SPLINE_OPT(u, u_2, u_3, 
									    ctrl[0].y,
									    ctrl[1].y,
									    ctrl[2].y,
									    ctrl[3].y);

				    curve_pt.z = CATMULL_ROM_SPLINE_OPT(u, u_2, u_3, 
									    ctrl[0].z,
									    ctrl[1].z,
									    ctrl[2].z,
									    ctrl[3].z);
          }
          break;

          default:
            assert(0);
            break;
        }

        curve_pts.push_back(curve_pt);
		  }

      if(!first_pass || !force_start)
        ++ctrl_1;

      first_pass = false;
	  }
  }
  else
    curve_pts = control_pts;
}
