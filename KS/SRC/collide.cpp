////////////////////////////////////////////////////////////
// Collide.cpp
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.
//
// contains all the collision algorithms.
////////////////////////////////////////////////////////////

#include "global.h"

//!#include "character.h"
//!#include "attrib.h"
#include "pmesh.h"
#include "hwmath.h"
#include "colmesh.h"
#include "collide.h"
#include "profiler.h"
#include "terrain.h"
//!#include "rigid.h"
#include "capsule.h"
#include "wds.h"
#include "bsp_collide.h"
#include "debug_render.h"

//  THIS FILE CONTAINS THE CONTENTS OF COLLIDE.CPP from, DBTS I.  SHOULD BE USEFUL
//  STUFF BUT HASN'T BEEN ORGANIZED YET.

// taken from beam.cpp
bool collide_segment_geometry( const vector3d& p0, const vector3d& p1,
                               const collision_geometry* cg,
                               vector3d& impact_pos, vector3d& impact_normal,
                               const po& my_po, bool rear_cull )
{
  if (!cg)
    return false;

  if (cg->get_type() == collision_geometry::CAPSULE)
  {
    const collision_capsule *cc = static_cast<const collision_capsule *>(cg);
    if ( collide_segment_capsule_accurate_result(p0, p1, *cc, impact_pos) )
    {
      impact_normal = p0 - impact_pos;
      impact_normal.normalize();
      return true;
    }
  }
  else if (cg->get_type() == collision_geometry::MESH)
  {
    // mesh collides need to be transformed in and out of the object's local space
    const cg_mesh *cm = static_cast<const cg_mesh *>(cg);
/*
    po inv_po = my_po.inverse();
    vector3d pt0 = inv_po.slow_xform(p0);
    vector3d pt1 = inv_po.slow_xform(p1);
*/
    vector3d pt0 = my_po.inverse_xform(p0);
    vector3d pt1 = my_po.inverse_xform(p1);

    vector3d wtf = pt1 - pt0;
    wtf.normalize();
    if ( collide_segment_mesh(pt0, pt1, cm, impact_pos, impact_normal, rear_cull ? PP_REAR_CULL : 0, wtf) )
    {
      impact_pos = my_po.slow_xform( impact_pos );
      impact_normal = my_po.non_affine_slow_xform( impact_normal );
      return true;
    }
  }

  return false;
}


bool collide_segment_entity( const vector3d& p0, const vector3d& p1,
                             const entity* ent,
                             vector3d* impact_pos, vector3d* impact_normal,
                             rational_t default_radius, bool rear_cull )
{
  rational_t erad;
// FIX - The colgeom radius was too small to catch the Sludge Monster, so the entity radius
//       is used exclusively, so as to not rule out cases which are actually valid.
//  if ( ent->get_colgeom() )
//    erad = ent->get_colgeom()->get_radius();
//  else
  erad = ent->get_radius();
  if ( erad <= 0.0f )
    erad = default_radius;
  if ( erad > 0.0f )
  {
    vector3d sphere_hit_pos;
    START_PROF_TIMER(proftimer_find_int_ent_ent_col_sph);
    bool hit_sphere = collide_segment_sphere( p0, p1, ent->get_abs_position(), erad, &sphere_hit_pos );
    STOP_PROF_TIMER(proftimer_find_int_ent_ent_col_sph);

    if ( hit_sphere )
    {
      collision_geometry *cg = ent->get_colgeom();

      if ( cg )
      {
/*!        if (ent->is_a_character() && ((character*)ent)->limb_valid(RIGHT_FOOT) && (cg->get_type() == collision_geometry::CAPSULE))
        {
          // Collide with extended capsule
          capsule cap = ((collision_capsule *)cg)->get_abs_capsule();

          vector3d footpos = (((character*)ent)->limb_ptr(RIGHT_FOOT)->get_body()->get_abs_position() +
                              ((character*)ent)->limb_ptr(LEFT_FOOT)->get_body()->get_abs_position()) * 0.5f;
          footpos.y += cap.radius * 0.4f;
          cap.base = footpos;

          if ( collide_segment_capsule_accurate_result(p0, p1, cap, *impact_pos) )
          {
            *impact_normal = p0 - *impact_pos;
            impact_normal->normalize();
            return true;
          }
        } else
!*/
          // Collide with the regular (un-extended) capsule
//          return collide_segment_geometry( p0, p1, cg, *impact_pos, *impact_normal, ent->get_abs_po(), rear_cull );
        START_PROF_TIMER(proftimer_find_int_ent_ent_col_geo);
        bool hit = collide_segment_geometry( p0, p1, cg, *impact_pos, *impact_normal, ent->get_abs_po(), rear_cull );
        STOP_PROF_TIMER(proftimer_find_int_ent_ent_col_geo);
        return(hit);
      }
      else
      {
        *impact_pos = sphere_hit_pos;
        *impact_normal = p0 - sphere_hit_pos;
        impact_normal->normalize();
        return true;
      }
    }
  }
  else if ( ent->get_colgeom() )
  {
    START_PROF_TIMER(proftimer_find_int_ent_ent_col_geo);
    bool hit = collide_segment_geometry( p0, p1, ent->get_colgeom(), *impact_pos, *impact_normal, ent->get_abs_po(), rear_cull );
    STOP_PROF_TIMER(proftimer_find_int_ent_ent_col_geo);
    return(hit);
  }
  return false;
}


bool collide_polygon_segment( const cface* f1, const cg_mesh* m1,
                              const vector3d& p1, const vector3d& p2,
                              vector3d& hit_loc )
{
  const vector3d& v0 = m1->get_vert(f1->get_vert_ref(0)).get_point();
  const vector3d& v1 = m1->get_vert(f1->get_vert_ref(1)).get_point();
  const vector3d& v2 = m1->get_vert(f1->get_vert_ref(2)).get_point();
  const vector3d& normal = f1->get_normal();

#ifndef TARGET_PS2

  rational_t d1 = dot( p1-v0, normal );
  rational_t d2 = dot( p2-v0, normal );

  if ( sgn(d1) == sgn(d2) )
  {
    // both on same side of poly plane
    return false;
  }
  else
  {
  //  rational_t denom = d1 - d2;
  //  vector3d on_plane = p2*(d1/denom) - p1*(d2/denom);
    rational_t inv_denom = 1.0f / (d1 - d2);
    vector3d on_plane = p2*(d1*inv_denom) - p1*(d2*inv_denom);
    hit_loc = on_plane;

    if ( dot( on_plane-v2, cross(normal,v0-v2) ) < 0.0f ||
         dot( on_plane-v1, cross(normal,v2-v1) ) < 0.0f ||
         dot( on_plane-v0, cross(normal,v1-v0) ) < 0.0f )
    {
      return false;
    }
    else
    {
      return true;
    }
  }

#else

	float ret;

  static sceVu0FVECTOR pt1;
  static sceVu0FVECTOR pt2;
  static sceVu0FVECTOR hit;

  memcpy(&pt1[0], &p1.x, 12);
  memcpy(&pt2[0], &p2.x, 12);

  // up to 12
  asm __volatile__(
  	"lqc2    vf4,0x0(%1) \n" // p1
	  "lqc2    vf5,0x0(%2) \n" // p2
	  "lqc2    vf6,0x0(%3) \n" // v0
	  "lqc2    vf7,0x0(%4) \n" // v1
	  "lqc2    vf8,0x0(%5) \n" // v2
	  "lqc2    vf9,0x0(%6) \n" // norm

    "vsub.xyz vf11, vf4, vf6 \n" // p1-v0
	  "vmul.xyz vf11, vf9, vf11 \n"
	  "vaddy.x vf11, vf11, vf11 \n"
	  "vaddz.x vf11, vf11, vf11 \n"
    "vsub.yzw vf11, vf11, vf11 \n" // v0-v2
	  "qmfc2   $8, vf11 \n" // dot( p1-v0, normal )

    "vsub.xyz vf12,vf5,vf6 \n" // p2-v0
	  "vmul.xyz vf12,vf9,vf12 \n"
	  "vaddy.x vf12,vf12,vf12 \n"
	  "vaddz.x vf12,vf12,vf12 \n"
    "vsub.yzw vf12, vf12, vf12 \n" // v0-v2
	  "qmfc2   $9 ,vf12 \n" //dot( p1-v0, normal )

    "move $10, $0 \n"
    "lui $10, 0x8000 \n"


    "mtc1    $8, $f8 \n"
    "mtc1    $9, $f9 \n"

    "and $8, $8, $10 \n"
    "and $9, $9, $10 \n"
    "beq $8, $9, _exit_cps \n"

  	"li.s    $f10, 1.0e0	 \n"
    "sub.s   $f11, $f8, $f9 \n"
    "div.s   $f12, $f10, $f11 \n" // inv_denom = 1.0f / (d1 - d2)

    "mul.s   $f8, $f12, $f8 \n" // (d1*inv_denom)
    "mul.s   $f9, $f12, $f9 \n" // (d2*inv_denom)

  	"mfc1    $9, $f9 \n"
	  "qmtc2   $9, vf11 \n"
	  "vmulx.xyz	vf4, vf4, vf11 \n" // p1*(d2*inv_denom)

    "mfc1    $8, $f8 \n"
	  "qmtc2   $8, vf11 \n"
	  "vmulx.xyz	vf5, vf5, vf11 \n" // p2*(d1*inv_denom)

  	"vsub.xyz	vf4, vf5, vf4 \n" // on_plane


    "vsub.xyz vf5, vf4, vf8 \n" // on_plane-v2

    "vsub.xyz vf10, vf6, vf8 \n" // v0-v2
	  "vopmula.xyz	ACC, vf9, vf10 \n"
	  "vopmsub.xyz	vf11, vf10, vf9 \n" // cross(normal,v0-v2)

    "vmul.xyz vf11, vf5, vf11 \n"
	  "vaddy.x vf11, vf11, vf11 \n"
	  "vaddz.x vf11, vf11, vf11 \n"
    "vsub.yzw vf11, vf11, vf11 \n" // v0-v2
	  "qmfc2   $8, vf11 \n" // dot( on_plane-v2, cross(normal,v0-v2)

    "and $8, $8, $10 \n"
    "bgtz $8, _exit_cps \n"


    "vsub.xyz vf5, vf4, vf7 \n" // on_plane-v1

    "vsub.xyz vf10, vf8, vf7 \n" // v2-v1
	  "vopmula.xyz	ACC, vf9, vf10 \n"
	  "vopmsub.xyz	vf11, vf10, vf9 \n" // cross(normal,v2-v1)

    "vmul.xyz vf11, vf5, vf11 \n"
	  "vaddy.x vf11, vf11, vf11 \n"
	  "vaddz.x vf11, vf11, vf11 \n"
    "vsub.yzw vf11, vf11, vf11 \n" // v0-v2
	  "qmfc2   $8, vf11 \n" // dot( on_plane-v2, cross(normal,v2-v1)

    "and $8, $8, $10 \n"
    "bgtz $8, _exit_cps \n"


    "vsub.xyz vf5, vf4, vf6 \n" // on_plane-v0

    "vsub.xyz vf10, vf7, vf6 \n" // v1-v0
	  "vopmula.xyz	ACC, vf9, vf10 \n"
	  "vopmsub.xyz	vf11, vf10, vf9 \n" // cross(normal,v1-v0)

    "vmul.xyz vf11, vf5, vf11 \n"
	  "vaddy.x vf11, vf11, vf11 \n"
	  "vaddz.x vf11, vf11, vf11 \n"
    "vsub.yzw vf11, vf11, vf11 \n" // v0-v2
	  "qmfc2   $8, vf11 \n" // dot( on_plane-v2, cross(normal,v1-v0)

    "and $8, $8, $10 \n"
    "bgtz $8, _exit_cps \n"


  	"sqc2 vf4, 0x0(%7) \n" // store hit_loc
    "mov.s %0, $f10 \n" // set ret as 1.0

    "b _finish_cps \n"

    "_exit_cps: \n"

  	"li.s    $f10, 0.0e0	 \n"
    "mov.s %0, $f10 \n" // set ret as 0.0

    "_finish_cps: \n"

	: "=f" (ret) :"r" (pt1) ,"r" (pt2) ,"r" (((sceVu0FVECTOR)&v0.x)) ,"r" (((sceVu0FVECTOR)&v1.x)) ,"r" (((sceVu0FVECTOR)&v2.x)) ,"r" (((sceVu0FVECTOR)&normal.x)) ,"r" (hit) : "$f8", "$f9", "$f10", "$f11", "$f12", "$8", "$9", "$10" );
//	: "=f" (ret) :"r" (pt1) ,"r" (pt2) ,"r" (vec0) ,"r" (vec1) ,"r" (vec2) ,"r" (norm) ,"r" (hit) : "$f8", "$f9", "$f10", "$f11", "$f12", "$8", "$9", "$10" );

  if(ret > 0.5f)
  {
    memcpy(&hit_loc.x, &hit[0], 12);
    return(true);
  }
  else
    return(false);

/*
  if((test_hit_PS2 != test_hit_PC) || (test_hit_PS2 && (hit_loc_PS2 - hit_loc).length() > 0.001f))
  {
    int warn_me = 0;
    warn_me += 1;
    error("Oops! bad asm conversion! (%d == %d) (<%.2f, %.2f, %.2f> == <%.2f, %.2f, %.2f>)", test_hit_PC, test_hit_PS2, hit_loc.x, hit_loc.y, hit_loc.z, hit_loc_PS2.x, hit_loc_PS2.y, hit_loc_PS2.z);
  }
*/
#endif
}


bool collide_polygon_segment( face_ref f1, const vr_pmesh* m1,
                              const vector3d& p1, const vector3d& p2,
                              vector3d& hit_loc )
{
  const vector3d& v0 = m1->get_xvert_unxform(  m1->get_wedge_ref(f1,0) );
  const vector3d& v1 = m1->get_xvert_unxform(  m1->get_wedge_ref(f1,1) );
  const vector3d& v2 = m1->get_xvert_unxform(  m1->get_wedge_ref(f1,2) );

#ifndef TARGET_PS2

  vector3d normal = cross( v1-v0, v2-v0 ).normalize();
  rational_t d1 = dot( p1-v0, normal );
  rational_t d2 = dot( p2-v0, normal );

  if ( sgn(d1) == sgn(d2) )
  {
    // both on same side of poly plane
    return false;
  }
  else
  {
  //  rational_t denom = d1 - d2;
  //  vector3d on_plane = p2*(d1/denom) - p1*(d2/denom);
    rational_t inv_denom = 1.0f / (d1 - d2);
    vector3d on_plane = p2*(d1*inv_denom) - p1*(d2*inv_denom);
    hit_loc = on_plane;

    if ( dot( on_plane-v2, cross(normal,v0-v2) ) < 0.0f ||
         dot( on_plane-v1, cross(normal,v2-v1) ) < 0.0f ||
         dot( on_plane-v0, cross(normal,v1-v0) ) < 0.0f )
    {
      return false;
    }
    else
    {
      return true;
    }
  }

#else

	float ret;

  // Move this calculation into the VU code later.
  static vector3d normal __attribute__((aligned(16)));
  normal = cross( v1-v0, v2-v0 ).normalize();

  static sceVu0FVECTOR pt1;
  static sceVu0FVECTOR pt2;
  static sceVu0FVECTOR vec0;
  static sceVu0FVECTOR vec1;
  static sceVu0FVECTOR vec2;
  static sceVu0FVECTOR hit;

  memcpy(&pt1[0], &p1.x, 12);
  memcpy(&pt2[0], &p2.x, 12);
  memcpy(&vec0[0], &v0.x, 12);
  memcpy(&vec1[0], &v1.x, 12);
  memcpy(&vec2[0], &v2.x, 12);

  // up to 12
  asm __volatile__(
  	"lqc2    vf4,0x0(%1) \n" // p1
	  "lqc2    vf5,0x0(%2) \n" // p2
	  "lqc2    vf6,0x0(%3) \n" // v0
	  "lqc2    vf7,0x0(%4) \n" // v1
	  "lqc2    vf8,0x0(%5) \n" // v2
	  "lqc2    vf9,0x0(%6) \n" // norm

    "vsub.xyz vf11, vf4, vf6 \n" // p1-v0
	  "vmul.xyz vf11, vf9, vf11 \n"
	  "vaddy.x vf11, vf11, vf11 \n"
	  "vaddz.x vf11, vf11, vf11 \n"
    "vsub.yzw vf11, vf11, vf11 \n" // v0-v2
	  "qmfc2   $8, vf11 \n" // dot( p1-v0, normal )

    "vsub.xyz vf12,vf5,vf6 \n" // p2-v0
	  "vmul.xyz vf12,vf9,vf12 \n"
	  "vaddy.x vf12,vf12,vf12 \n"
	  "vaddz.x vf12,vf12,vf12 \n"
    "vsub.yzw vf12, vf12, vf12 \n" // v0-v2
	  "qmfc2   $9 ,vf12 \n" //dot( p1-v0, normal )

    "move $10, $0 \n"
    "lui $10, 0x8000 \n"


    "mtc1    $8, $f8 \n"
    "mtc1    $9, $f9 \n"

    "and $8, $8, $10 \n"
    "and $9, $9, $10 \n"
    "beq $8, $9, _exit_cps2 \n"

  	"li.s    $f10, 1.0e0	 \n"
    "sub.s   $f11, $f8, $f9 \n"
    "div.s   $f12, $f10, $f11 \n" // inv_denom = 1.0f / (d1 - d2)

    "mul.s   $f8, $f12, $f8 \n" // (d1*inv_denom)
    "mul.s   $f9, $f12, $f9 \n" // (d2*inv_denom)

  	"mfc1    $9, $f9 \n"
	  "qmtc2   $9, vf11 \n"
	  "vmulx.xyz	vf4, vf4, vf11 \n" // p1*(d2*inv_denom)

    "mfc1    $8, $f8 \n"
	  "qmtc2   $8, vf11 \n"
	  "vmulx.xyz	vf5, vf5, vf11 \n" // p2*(d1*inv_denom)

  	"vsub.xyz	vf4, vf5, vf4 \n" // on_plane


    "vsub.xyz vf5, vf4, vf8 \n" // on_plane-v2

    "vsub.xyz vf10, vf6, vf8 \n" // v0-v2
	  "vopmula.xyz	ACC, vf9, vf10 \n"
	  "vopmsub.xyz	vf11, vf10, vf9 \n" // cross(normal,v0-v2)

    "vmul.xyz vf11, vf5, vf11 \n"
	  "vaddy.x vf11, vf11, vf11 \n"
	  "vaddz.x vf11, vf11, vf11 \n"
    "vsub.yzw vf11, vf11, vf11 \n" // v0-v2
	  "qmfc2   $8, vf11 \n" // dot( on_plane-v2, cross(normal,v0-v2)

    "and $8, $8, $10 \n"
    "bgtz $8, _exit_cps2 \n"


    "vsub.xyz vf5, vf4, vf7 \n" // on_plane-v1

    "vsub.xyz vf10, vf8, vf7 \n" // v2-v1
	  "vopmula.xyz	ACC, vf9, vf10 \n"
	  "vopmsub.xyz	vf11, vf10, vf9 \n" // cross(normal,v2-v1)

    "vmul.xyz vf11, vf5, vf11 \n"
	  "vaddy.x vf11, vf11, vf11 \n"
	  "vaddz.x vf11, vf11, vf11 \n"
    "vsub.yzw vf11, vf11, vf11 \n" // v0-v2
	  "qmfc2   $8, vf11 \n" // dot( on_plane-v2, cross(normal,v2-v1)

    "and $8, $8, $10 \n"
    "bgtz $8, _exit_cps2 \n"


    "vsub.xyz vf5, vf4, vf6 \n" // on_plane-v0

    "vsub.xyz vf10, vf7, vf6 \n" // v1-v0
	  "vopmula.xyz	ACC, vf9, vf10 \n"
	  "vopmsub.xyz	vf11, vf10, vf9 \n" // cross(normal,v1-v0)

    "vmul.xyz vf11, vf5, vf11 \n"
	  "vaddy.x vf11, vf11, vf11 \n"
	  "vaddz.x vf11, vf11, vf11 \n"
    "vsub.yzw vf11, vf11, vf11 \n" // v0-v2
	  "qmfc2   $8, vf11 \n" // dot( on_plane-v2, cross(normal,v1-v0)

    "and $8, $8, $10 \n"
    "bgtz $8, _exit_cps2 \n"


  	"sqc2 vf4, 0x0(%7) \n" // store hit_loc
    "mov.s %0, $f10 \n" // set ret as 1.0

    "b _finish_cps2 \n"

    "_exit_cps2: \n"

  	"li.s    $f10, 0.0e0	 \n"
    "mov.s %0, $f10 \n" // set ret as 0.0

    "_finish_cps2: \n"

//	: "=f" (ret) :"r" (pt1) ,"r" (pt2) ,"r" (((sceVu0FVECTOR)&v0.x)) ,"r" (((sceVu0FVECTOR)&v1.x)) ,"r" (((sceVu0FVECTOR)&v2.x)) ,"r" (((sceVu0FVECTOR)&normal.x)) ,"r" (hit) : "$f8", "$f9", "$f10", "$f11", "$f12", "$8", "$9", "$10" );
	: "=f" (ret) :"r" (pt1) ,"r" (pt2) ,"r" (vec0) ,"r" (vec1) ,"r" (vec2) ,"r" (((sceVu0FVECTOR)&normal.x)) ,"r" (hit) : "$f8", "$f9", "$f10", "$f11", "$f12", "$8", "$9", "$10" );

  if(ret > 0.5f)
  {
    memcpy(&hit_loc.x, &hit[0], 12);
    return(true);
  }
  else
    return(false);

/*
  if((test_hit_PS2 != test_hit_PC) || (test_hit_PS2 && (hit_loc_PS2 - hit_loc).length() > 0.001f))
  {
    int warn_me = 0;
    warn_me += 1;
    error("Oops! bad asm conversion! (%d == %d) (<%.2f, %.2f, %.2f> == <%.2f, %.2f, %.2f>)", test_hit_PC, test_hit_PS2, hit_loc.x, hit_loc.y, hit_loc.z, hit_loc_PS2.x, hit_loc_PS2.y, hit_loc_PS2.z);
  }
*/
#endif
}



// We had the cool 'splits' optimization in effect in DBTS I (see below).
// It should probably be added here at some point.
bool collide_polygon_polygon( const cface* f1, const cg_mesh* m1,
                              const cface* f2, const cg_mesh* m2,
                              vector3d& hit_loc )
{
  const vector3d& v1_0 = m1->get_vert(f1->get_vert_ref(0)).get_point();
  const vector3d& v1_1 = m1->get_vert(f1->get_vert_ref(1)).get_point();
  const vector3d& v1_2 = m1->get_vert(f1->get_vert_ref(2)).get_point();

  const vector3d& v2_0 = m2->get_vert(f2->get_vert_ref(0)).get_point();
  const vector3d& v2_1 = m2->get_vert(f2->get_vert_ref(1)).get_point();
  const vector3d& v2_2 = m2->get_vert(f2->get_vert_ref(2)).get_point();

  if ( collide_polygon_segment( f1, m1, v2_0, v2_1, hit_loc ) ||
       collide_polygon_segment( f1, m1, v2_1, v2_2, hit_loc ) ||
       collide_polygon_segment( f1, m1, v2_2, v2_0, hit_loc ) )
    return true;
  else if ( collide_polygon_segment( f2, m2, v1_0, v1_1, hit_loc ) ||
            collide_polygon_segment( f2, m2, v1_1, v1_2, hit_loc ) ||
            collide_polygon_segment( f2, m2, v1_2, v1_0, hit_loc ) )
    return true;
  else
    return false;
}


bool collide_mesh_polygon( const cg_mesh* m1,
                           const cface* f2, const cg_mesh* m2,
                           vectorvector* hit_list,
                           vectorvector* normal_list1,
                           vectorvector* normal_list2,
                           unsigned int ct,
                           const vector3d& rel_vel )
{
  bool hit = false;
  bool no_rear_cull = !(ct & PP_REAR_CULL);
  if ( no_rear_cull || dot(f2->get_normal(),rel_vel)<=0 )
  {
    vector3d hit_loc;
    rational_t x2 = f2->get_center().x;
    rational_t r2 = f2->get_radius();
    int low = m1->get_low_index( x2 - r2 );
    if ( low >= 0 )
    {
      int high = m1->get_high_index( x2 + r2);
      if ( low <= high )
      {
        vector<cface>::const_iterator cfi = m1->get_cfaces().begin() + low;
        vector<cface>::const_iterator cfi_end = m1->get_cfaces().begin() + high;
        for ( ; cfi<=cfi_end; cfi++ )
        {
          const cface& f1 = *cfi;
          if ( no_rear_cull || dot(f1.get_normal(),rel_vel)>=0 )
          {
            if ( collide_polygon_polygon( &f1, m1, f2, m2, hit_loc ) )
            {
              hit = true;
              hit_list->push_back( hit_loc );
              normal_list1->push_back( f1.get_normal() );
              normal_list2->push_back( f2->get_normal() );
              if ( ct & ONE_HIT_PER_MESH )
								return hit;
              if ( ct & ONE_HIT_PER_M2_POLY )
                break;
            }
          }
        }
      }
    }
  }
  return hit;
}


bool collide_segment_mesh( const vector3d& p1, const vector3d& original_p2,
                           const cg_mesh* m2,
                           vector3d& hit_loc, vector3d& hit_normal,
                           unsigned int ct,
                           const vector3d& rel_vel )
{
  bool hit = false;
  bool first_hit = true;
  vector3d p2 = original_p2;

  vector3d m1pos = (p1 + p2) * 0.5f;
  rational_t dx = __fabs( p1.x - p2.x ) * 0.5f;
  int low = m2->get_low_index( m1pos.x - dx );
  if ( low >= 0 )
  {
    int high = m2->get_high_index( m1pos.x + dx );
    if ( low <= high )
    {
      rational_t m1rad = (p1 - p2).xz_length();
      bool no_rear_cull = !(ct & PP_REAR_CULL);
      vector<cface>::const_iterator cfi = m2->get_cfaces().begin() + low;
      vector<cface>::const_iterator cfi_end = m2->get_cfaces().begin() + high;
      for ( ; cfi<=cfi_end; ++cfi )
      {
        const cface& f2 = *cfi;
        vector3d xz_diff = f2.get_center() - m1pos;
        rational_t r = f2.get_radius() + m1rad;
        if ( xz_diff.x*xz_diff.x+xz_diff.z*xz_diff.z < r*r )
        {
          vector3d next_hit_loc;
          vector3d next_hit_normal = f2.get_normal();
          if ( no_rear_cull || dot(next_hit_normal,rel_vel)<=0.0f )
          {
            if ( collide_polygon_segment( &f2, m2, p1, p2, next_hit_loc ) )
            {
              if ( first_hit )
              {
                p2 = hit_loc = next_hit_loc;
                m1pos = (p1 + p2) * 0.5f;
                m1rad = (p1 - p2).xz_length();
                hit_normal = next_hit_normal;
                first_hit = false;
              }
              else
              {
                p2 = hit_loc = next_hit_loc;
                m1pos = (p1 + p2) * 0.5f;
                m1rad = (p1 - p2).xz_length();
                hit_normal = next_hit_normal;
              }
              hit = true;
            }
          }
        }
      }
    }
  }
  return hit;
}


bool collide_sphere_mesh( const vector3d& p, rational_t r, const vector3d& v,
                          const cg_mesh* m,
                          vector3d& hit_loc )
{
  bool hit = false;
  rational_t mind = FLT_MAX;

  int low = m->get_low_index( p.x - r );
  if ( low >= 0 )
  {
    int high = m->get_high_index( p.x + r );
    if ( low <= high )
    {
      vector<cface>::const_iterator cfi = m->get_cfaces().begin() + low;
      vector<cface>::const_iterator cfi_end = m->get_cfaces().begin() + high;
      for ( ; cfi<=cfi_end; ++cfi )
      {
        const cface& f = *cfi;
        rational_t tr = f.get_radius() + r;
        if ( dot(v,f.get_normal())<0.0001f && (p-f.get_center()).length2()<tr*tr )
        {
          vector3d tp;
          if ( collide_sphere_polygon( p, r, &f, m, tp ) )
          {
            // use the closest one
            rational_t d = ( p - tp ).length2();
            if ( d < mind )
            {
              mind = d;
              hit_loc = tp;
              hit = true;
            }
          }
        }
      }
    }
  }
  return hit;
}


bool collide_capsule_mesh( const collision_capsule* c1, const cg_mesh* m2,
                           vectorvector* hit_list,
                           vectorvector* normal_list1,
                           vectorvector* normal_list2,
                           unsigned int ct,
                           const vector3d& rel_vel )
{
  //profiler::inst()->add_counter( profiler::PCT_GENERIC4, 1 );

  bool hit = false;
  vector3d c1pos = c1->get_abs_position();
  rational_t c1rad = c1->get_radius();
  int low = m2->get_low_index( c1pos.x - c1rad );
  if ( low >= 0 )
  {
    int high = m2->get_high_index( c1pos.x + c1rad );
    if ( low <= high )
    {
      vector<cface>::const_iterator cfi = m2->get_cfaces().begin() + low;
      vector<cface>::const_iterator cfi_end = m2->get_cfaces().begin() + high;
      for ( ; cfi<=cfi_end; ++cfi )
      {
        const cface& f2 = *cfi;
        rational_t r = f2.get_radius() + c1rad;
        if ( (c1pos-f2.get_center()).length2() < r*r )
        {
//          vector3d hit_loc;
          if ( collide_capsule_polygon( *c1, &f2, m2, hit_list, normal_list1, normal_list2, rel_vel ) )
            hit = true;
        }
      }
    }
  }
  return hit;
}


bool collide_capsule_full_mesh( const collision_capsule* c1, const cg_mesh* m2,
                                vectorvector* hit_list,
                                vectorvector* normal_list1,
                                vectorvector* normal_list2,
                                unsigned int ct,
                                const vector3d& rel_vel )
{
  //profiler::inst()->add_counter( profiler::PCT_GENERIC4, 1 );

  bool hit = false;
  vector3d c1pos = c1->get_abs_position();
  rational_t c1rad = c1->get_radius();
  vector<cface>::const_iterator cfi = m2->get_cfaces().begin();
  vector<cface>::const_iterator cfi_end = m2->get_cfaces().end();
  for ( ; cfi<cfi_end; ++cfi )
  {
    const cface& f2 = *cfi;
    rational_t r = f2.get_radius() + c1rad;
    if ( (c1pos-f2.get_center()).length2() < r*r )
    {
//      vector3d hit_loc;
      if ( collide_capsule_polygon( *c1, &f2, m2, hit_list, normal_list1, normal_list2, rel_vel ) )
        hit = true;
    }
  }
  return hit;
}


bool collide_mesh_mesh( const cg_mesh* m1, const cg_mesh* m2,
                        vectorvector* hit_list,
                        vectorvector* normal_list1,
                        vectorvector* normal_list2,
                        unsigned int ct,
                        const vector3d& rel_vel,
						cface * hitFace)
{
  bool hit = false;
  bool no_rear_cull = !(ct & PP_REAR_CULL);
  vector3d m1pos = m1->get_abs_position();
  rational_t m1rad = m1->get_radius();
  vector<cface>::const_iterator cfi = m2->get_cfaces().begin();
  vector<cface>::const_iterator cfi_end = m2->get_cfaces().end();
  for ( ; cfi!=cfi_end; ++cfi )
  {
    const cface& f2 = *cfi;
    rational_t r = m1rad + f2.get_radius();
    if ( (no_rear_cull || dot(f2.get_normal(),rel_vel)<=0.0f) &&
         (m1pos-f2.get_center()).length2() < r*r )
    {
      rational_t x2 = f2.get_center().x;
      rational_t r2 = f2.get_radius();
      int low = m1->get_low_index( x2 - r2 );
      if ( low >= 0 )
      {
        int high = m1->get_high_index( x2 + r2 );
        if ( low <= high )
        {
          vector3d hit_loc;
          vector<cface>::const_iterator cfj = m1->get_cfaces().begin() + low;
          vector<cface>::const_iterator cfj_end = m1->get_cfaces().begin() + high;
          for ( ; cfj<=cfj_end; ++cfj )
          {
            const cface& f1 = *cfj;
            if ( no_rear_cull || dot(f1.get_normal(),rel_vel)>=0.0f )
            {
              if ( collide_polygon_polygon( &f1, m1, &f2, m2, hit_loc ) )
              {
                hit = true;
								if (hitFace) *hitFace = (*cfi);
                hit_list->push_back( hit_loc );
                normal_list1->push_back( f1.get_normal() );
                normal_list2->push_back( f2.get_normal() );
                if ( ct & ONE_HIT_PER_MESH )
									return hit;
                if ( ct & ONE_HIT_PER_M2_POLY )
                  break;
              }
            }
          }
        }
      }
    }
  }
  return hit;
}


bool collide_mesh_region( const cg_mesh* m1, const region* t,
                          vectorvector* hit_list,
                          vectorvector* normal_list1,
                          vectorvector* normal_list2,
                          unsigned int ct,
                          const vector3d& rel_vel )
{
#ifndef BSP_COLLISIONS
  profiler_start_timer(profiler::PTM_MESH_RGN );

  bool hit = false;
  vector3d m1pos = m1->get_abs_position();
  rational_t m1rad = m1->get_radius();

  cg_mesh * m2 = t->get_cg_mesh();
  int low_idx = t->get_low_index(m1pos.x-m1rad);
  if ( low_idx >= 0 )
  {
    int high_idx = t->get_high_index(m1pos.x+m1rad);
    vector<cface_replacement>::const_iterator cfi = t->get_sorted().begin() + low_idx;
    vector<cface_replacement>::const_iterator cfi_end = t->get_sorted().begin() + high_idx;
    for ( ; cfi<=cfi_end; ++cfi )
    {
      const cface_replacement& f2 = *cfi;
      rational_t r = f2.get_radius() + m1rad;
      if (norm2(m1pos-f2.get_center())<r*r && !f2.is_flagged(CFACE_FLAG_COSMETIC))
      {
        vector3d hit_loc;
        if  ( collide_mesh_polygon( m1, &f2, m2, hit_list, normal_list1, normal_list2, ct, rel_vel ) )
          hit = true;
      }
    }
  }

  profiler_stop_timer(profiler::PTM_MESH_RGN);

  return hit;
#else
  assert(0);
  return false;
#endif
}

#if defined(BSP_COLLISIONS) || defined(BSP_ELEVATION)

bool collide_segment_region( const vector3d& p1, const vector3d& p2,
                             const region* t,
                             vector3d& hit_loc, vector3d& hit_normal,
                             unsigned int ct,
                             const vector3d& rel_vel )
{
  bool hit;
  hit = g_world_ptr->get_the_terrain().find_intersection(p1, p2, hit_loc, hit_normal);
  return hit;
}

#else

bool collide_segment_region( const vector3d& p1, const vector3d& p2,
                             const region* t,
                             vector3d& hit_loc, vector3d& hit_normal,
                             unsigned int ct,
                             const vector3d& rel_vel )
{
  bool hit = false;
  vector3d m1pos = (p1+p2)*0.5F;

  //profiler_start_timer(profiler::PTM_SEG_RGN);

  // This is dependant on the USP tree being horizontal!
  rational_t m1rad = (p1-p2).xz_length();

  cg_mesh * m2 = t->get_cg_mesh();
  int low_idx = t->get_low_index(m1pos.x-m1rad);
  if ( low_idx >= 0 )
  {
    int high_idx = t->get_high_index(m1pos.x+m1rad);
    bool first_hit = true;
    vector<cface_replacement>::const_iterator cfi = t->get_sorted().begin() + low_idx;
    vector<cface_replacement>::const_iterator cfi_end = t->get_sorted().begin() + high_idx;
    for ( ; cfi<=cfi_end; ++cfi )
    {
      const cface_replacement& f2 = *cfi;
      rational_t r = f2.get_radius() + m1rad;
      vector3d xz_diff = m1pos-f2.get_center();
      if( xz_diff.x*xz_diff.x+xz_diff.z*xz_diff.z < r*r )
      {
        // Inside here we know that we're within a radius of a cface in the XZ plane
        if ( !f2.is_flagged(CFACE_FLAG_COSMETIC) )
        {
          vector3d next_hit_loc;
          vector3d next_hit_normal = f2.get_normal();
          rational_t dot_res = dot(next_hit_normal,rel_vel);
          if ( !(ct & PP_REAR_CULL) || (dot_res<=0) )
          {
            if ( collide_polygon_segment(f2, m2, p1, p2, next_hit_loc) )
            {
              if (first_hit)
              {
                hit_loc = next_hit_loc;
                hit_normal = next_hit_normal;
                first_hit = false;
              }
              else
              {
                if (dot(next_hit_loc,rel_vel)<dot(hit_loc,rel_vel))
                {
                  hit_loc = next_hit_loc;
                  hit_normal = next_hit_normal;
                }
              }
              hit = true;
            }
          }
        }
      }
    }
  }

  //profiler_stop_timer(profiler::PTM_SEG_RGN);

  return hit;
}

#endif

bool collide_segment_region_with_poly_data( const vector3d& p1, const vector3d& p2,
                                            const region* t,
                                            vector3d& hit_loc, vector3d& hit_normal,
                                            unsigned int ct,
                                            const vector3d& rel_vel,
                                            const vr_pmesh** pM,
                                            face_ref* rF )
{
  //profiler_start_timer(profiler::PTM_SEG_RGN);

  bool hit = false;
  vector3d m1pos = (p1+p2)*0.5F;

  // This is dependant on the USP tree being horizontal!
  rational_t m1rad = (p1-p2).xz_length();

  int low_idx = t->get_low_index( m1pos.x - m1rad );
  if ( low_idx >= 0 )
  {
    int high_idx = t->get_high_index( m1pos.x + m1rad );
    bool first_hit = true;
    vector<cface_replacement>::const_iterator cfi = t->get_sorted().begin() + low_idx;
    vector<cface_replacement>::const_iterator cfi_end = t->get_sorted().begin() + high_idx;
    for ( ; cfi<=cfi_end; ++cfi )
    {
      const cface_replacement& f2 = *cfi;
      if ( !f2.is_cosmetic() )
      {
        rational_t r = f2.get_radius() + m1rad;
        vector3d xz_diff = m1pos - f2.get_center();
        if ( xz_diff.x*xz_diff.x+xz_diff.z*xz_diff.z < r*r )
        {
          // Inside here we know that we're within a radius of a cface in the XZ plane
          vector3d next_hit_loc;
          vector3d next_hit_normal = f2.get_normal();
          rational_t dot_res = dot( next_hit_normal, rel_vel );
          if ( !(ct&PP_REAR_CULL) || dot_res<=0 )
          {
            if ( collide_polygon_segment( f2.rF, f2.pP, p1, p2, next_hit_loc ) )
            {
              if ( first_hit )
              {
                hit_loc = next_hit_loc;
                hit_normal = next_hit_normal;
                first_hit = false;
                // If they want the collision-face we hit (pathfinder) then showandtell
                if ( rF )
                {
                  *pM = f2.pP;
                  *rF = f2.rF;
                }
              }
              else
              {
                if (dot(next_hit_loc,rel_vel)<dot(hit_loc,rel_vel))
                {
                  hit_loc = next_hit_loc;
                  hit_normal = next_hit_normal;
                  // If they want the collision-face we hit (pathfinder) then showandtell
                  if( rF )
                  {
                    *pM = f2.pP;
                    *rF = f2.rF;
                  }
                }
              }
              hit = true;
            }
          }
        }
      }
    }
  }

  //profiler_stop_timer(profiler::PTM_SEG_RGN);

  return hit;
}

bool collide_segment_sphere( const vector3d& p0, const vector3d& p1,
                             const vector3d& s, rational_t r,
                             vector3d* hit_loc )
{
  // use quadratic formula to see if line segment hits entity sphere
  vector3d v = p1 - p0;
  vector3d w = p0 - s;
  rational_t a = v.length2();
  if ( a < 0.000001f )
  {
    // segment is degenerate (single point)
    if ( w.length2() < r*r )
    {
      *hit_loc = p0;
      return true;
    }
    else
      return false;
  }
  rational_t b = 2.0f * dot( v, w );
  rational_t c = w.length2() - r*r;
  rational_t d = b*b - 4.0f*a*c;
  if ( d > 0.0f )
  {
    b *= -1.0f;
    d = __fsqrt( d );
    rational_t inv_f = 1.0f / (2.0f*a);
    rational_t root1 = (b - d) * inv_f;
    rational_t root2 = (b + d) * inv_f;
    if ( root2 >= 0.0f && root1 <= 1.0f )
    {
      *hit_loc = p0 + v*root1;
      return true;
    }
  }
  return false;
}


bool collide_sphere_polygon( const vector3d& p, rational_t r,
                             const cface* f, const cg_mesh* m,
                             vector3d& hit_loc )
{
  // check sphere against plane of polygon
  const vector3d& n = f->get_normal();
  rational_t d = dot( p-f->get_center(), n );
  if ( d<=-r || d>=r )
    return false;
  // project sphere into plane and check circle against polygon
  rational_t rr2 = r*r - d*d;
  int inside = 0;
  int i;
  for ( i=0; i<3; ++i )
  {
    const vector3d& v0 = m->get_vert_ptr( f->get_vert_ref(i) )->get_point();
    const vector3d& v1 = m->get_vert_ptr( f->get_vert_ref((i+1)%3) )->get_point();
    vector3d nn = cross( v1-v0, n ).normalize();
    rational_t dd = dot( p-v0, nn );
    if ( dd <= 0.0f )
      ++inside;
    else if ( dd*dd >= rr2 )
      return false;
    else
    {
      // project circle onto segment and check against endpoints
      vector3d nnn = v1 - v0;
      rational_t segd = nnn.length();
      nnn *= 1/segd;
      rational_t ddd = dot( p-v0, nnn );
      if ( ddd < 0.0f )
      {
        if ( ddd > segd-__fsqrt(rr2-dd*dd) )
        {
          hit_loc = v0;
          return true;
        }
      }
      else if ( ddd > segd )
      {
        if ( ddd < segd+__fsqrt(rr2-dd*dd) )
        {
          hit_loc = v1;
          return true;
        }
      }
      else
      {
        hit_loc = (p - n*d) - nn*dd;
        return true;
      }
    }
  }
  if ( inside == 3 )
  {
    hit_loc = p - n*d;
    return true;
  }
  return false;
}


bool collide_capsule_capsule( const capsule& cap1, const capsule& cap2,
                              vectorvector* hit_list,
                              vectorvector* normal_list1,
                              vectorvector* normal_list2,
                              rational_t& distance,
                              unsigned int ct,
                              const vector3d& rel_vel,
                              vector3d* core_diff )
{
  vector3d p1,p2,q1,q2;
  vector3d a,b,c,d;
  rational_t w,x,y1,y2,z;
  //rational_t rp,rq;

  // Copy capsule info to local variables
  p1 = cap1.base;
  p2 = cap1.end;
  //rp = cap1.radius;
  q1 = cap2.base;
  q2 = cap2.end;
  //rq = cap2.radius;
  rational_t sum_radii = cap1.radius + cap2.radius;

  a = p1;
  b = p2-p1;
  c = q1;
  d = q2-q1;

  // Optimized calculations
  vector3d a_plus_b = a+b;
  vector3d c_plus_d = c+d;
  vector3d c_minus_a = c-a;
  vector3d a_minus_c = a-c;

  // Think. Then code.
  w = dot(b,b);
  x = dot(d,d);
  y1 = dot(a_minus_c,b);
  y2 = dot(c_minus_a,d);
  z = dot(b,d);

  // Think harder. Then recode.
  float denom = (z*z-w*x);

  vector3d diff;
  vector3d hit_loc;
  // roughly parallel case
  if (denom>-0.1f)
  {
    vector3d dir = b;
    dir.normalize();
    rational_t a1 = dot(a,dir);
    rational_t a2 = dot(a_plus_b,dir);
    rational_t c1 = dot(c,dir);
    rational_t c2 = dot(c_plus_d,dir);
    vector3d hit_base;
    rational_t diff_sign = 1.0f;
    if (a1>c1 && a1>c2 && a2>c1 && a2>c2)
    {
      if (a1>a2)
      {
        if (c1>c2)
          diff = c_plus_d-a;
        else
          diff = c_minus_a;
        hit_base = a;
      }
      else
      {
        if (c1>c2)
          diff = c_plus_d-(a_plus_b);
        else
          diff = c-(a_plus_b);
        hit_base = a_plus_b;
      }
    }
    else if (a1<c1 && a1<c2 && a2<c1 && a2<c2)
    {
      if (a1<a2)
      {
        if (c1<c2)
          diff = c-(a_plus_b);
        else
          diff = c_plus_d-(a_plus_b);
        hit_base = a_plus_b;
      }
      else
      {
        if (c1<c2)
          diff = c_minus_a;
        else
          diff = c_plus_d-a;
        hit_base = a;
      }
    }
    else
    {
      diff = (c_minus_a)-dot(c_minus_a,dir)*dir;
      if ((a1<c1 && a1>=c2) || (a1<c2 && a1>=c1))
        hit_base = a;
      else if ((a2<c1 && a2>=c2) || (a2<c2 && a2>=c1))
        hit_base = a_plus_b;
      else if ((c1<a1 && c1>=a2) || (c1<a2 && c1>=a1))
      {
        hit_base = c;
        diff_sign = -1.0f;
      }
      else if ((c2<a1 && c2>=a2) || (c2<a2 && c2>=a1))
      {
        hit_base = c_plus_d;
        diff_sign = -1.0f;
      }
    }
    //    denom = -0.1f;   // kludge for now!!!!!! 1/29/98
//    hit_loc = hit_base + diff_sign*diff/2;
    hit_loc = hit_base + (diff*(diff_sign*0.5f));
    diff = -diff;
  }
  else
  {
    int mincase = 0;
    rational_t s[4], t[4];
    //rational_t s,t;
    //rational_t t0,t1,t2,t3,s0,s1,s2,s3;
    rational_t inv_denom = 1.0f / denom;
    t[0] = (y1*z + w*y2)*inv_denom;
    s[0] = (y2*z + x*y1)*inv_denom;

    // if this condition holds, the closest point is between one vertex and the other
    // segment, possible to a vertex of that segment as well.  So the cases are:
    //      point to edge
    //      point_to_point
    if (s[0]<0 || t[0]<0 || s[0]>1 || t[0]>1)
    {
      rational_t nb = 1.0f / b.length();
      rational_t nd = 1.0f / d.length();

      s[0]=0.0f;
      t[0] = dot(a_minus_c,d)*nd;
      if (t[0]<0.0f) t[0] = 0.0f;
      if (t[0]>1.0f) t[0] = 1.0f;
      vector3d a_to_cap2 = c+(d*t[0]);

      s[1]=1.0f;
      t[1] = dot(a_plus_b-c,d)*nd;
      if (t[1]<0.0f) t[1] = 0.0f;
      if (t[1]>1.0f) t[1] = 1.0f;
      vector3d a_plus_b_to_cap2 = c+(d*t[1]);

      t[2]=0.0f;
      s[2] = dot(c_minus_a,b)*nb;
      if (s[2]<0.0f) s[2] = 0.0f;
      if (s[2]>1.0f) s[2] = 1.0f;
      vector3d c_to_cap1 = a+(b*s[2]);

      t[3]=1.0f;
      s[3] = dot(c_plus_d-a,b)*nb;
      if (s[3]<0.0f) s[3] = 0.0f;
      if (s[3]>1.0f) s[3] = 1.0f;
      vector3d c_plus_d_to_cap1 = a+(b*s[3]);

      rational_t n_a_to_cap2 = (a-a_to_cap2).length();
      rational_t n_a_plus_b_to_cap2 = (a_plus_b-a_plus_b_to_cap2).length();
      rational_t n_c_to_cap1 = (c-c_to_cap1).length();
      rational_t n_c_plus_d_to_cap1 = (c_plus_d-c_plus_d_to_cap1).length();

      mincase = -1; // Force trigger of assertion if one of the following if's doesn't fire.
      rational_t min = 10e6;
      if (n_a_to_cap2<min)
      {
        min = n_a_to_cap2;
        mincase = 0;
      }
      if (n_a_plus_b_to_cap2<min)
      {
        min = n_a_plus_b_to_cap2;
        mincase = 1;
      }
      if (n_c_to_cap1<min)
      {
        min = n_c_to_cap1;
        mincase = 2;
      }
      if (n_c_plus_d_to_cap1<min)
      {
        min = n_c_plus_d_to_cap1;
        mincase = 3;
      }

      // Make sure mincase is in range 0..3
      assert ((mincase & 0x3) == mincase);

      /* OPTIMIZED - replaced with array version
      if (mincase==0)
      {
        t=t[0]; s=s[0];
      }
      else if (mincase==1)
      {
        t=t[1]; s=s[1];
      }
      else if (mincase==2)
      {
        t=t[2]; s=s[2];
      }
      else if (mincase==3)
      {
        t=t[3]; s=s[3];
      }
      else
        assert(0); */
    }

/*

      // check themax's on all the edges
      vector3d edge_min_diffs[4];
      rational_t edge_s[4];
      rational_t edge_t[4];

      rational_t t0s = y1/x;
      if (t0s<0) t0s = 0;
      else if (t0s>1) t0s = 1;
      edge_min_diffs[0] =(a+t*b)-(c+t0s*d);
      edge_t[0] = 0;
      edge_s[0] = t0s;

      rational_t t1s = (y1+z)/x;
      if (t1s<0) t1s = 0;
      else if (t1s>1) t1s = 1;
      edge_min_diffs[1] =(a+t*b)-(c+t1s*d);
      edge_t[1] = 0;
      edge_s[1] = t1s;

      rational_t s0t = y2/x;
      if (s0t<0) s0t = 0;
      else if (s0t>1) s0t = 1;
      edge_min_diffs[2] =(a+s0t*b)-(c+s*d);
      edge_s[2] = 0;
      edge_t[2] = s0t;

      rational_t s1t = (y2+z)/w;
      if (s1t<0) s1t = 0;
      else if (s1t>1) s1t = 1;
      edge_min_diffs[3] =(a+s1t*b)-(c+s*d);
      edge_s[3] = 1;
      edge_t[3] = s1t;

      rational_t min=10e6;
      for (int i=0;i<4;++i)
      {
        rational_t d = norm(edge_min_diffs[i]);
        if (d<min)
        {
          min = d;
          diff = edge_min_diffs[i];
          s = edge_s[i];
          t = edge_t[i];
        }
      }
    }

*/

    //  if (s<0) s=0;
    //  else if (s>1) s = 1;
    //  if (t<0) t=0;
    //  else if (t>1) t=1;

    vector3d a_plus_s_mult_b = (a+s[mincase]*b);
    vector3d c_plus_t_mult_d = (c+t[mincase]*d);

    diff = a_plus_s_mult_b-c_plus_t_mult_d;
    rational_t pct = cap1.radius/sum_radii;
    hit_loc = a_plus_s_mult_b*(1.0f-pct) + c_plus_t_mult_d*pct;
  }

  distance = diff.length();
  if (core_diff)
  {
    *core_diff = diff;
  }

  bool hit = (distance<sum_radii);
  if (hit)
  {
    vector3d normal = diff;
    if (distance)
      normal/=distance;

    hit_list->push_back(hit_loc);
    normal_list1->push_back(-normal);
    normal_list2->push_back(normal);
  }
  return hit;
}



// collision_normal is NOT normalized
bool collide_capsule_polygon( collision_capsule const& c,
                              cface const* f2, const cg_mesh* m2,
                              vectorvector* hit_list,
                              vectorvector* normal_list1,
                              vectorvector* normal_list2,
                              const vector3d& rel_vel )
{
  // Strategy shall be to fake a projection of the capsule down onto the plane.
  // This image should be an ellipse, but we will approximate it by a 2D capsule.

  // What??  I think it is a 2D capsule.  What was I thinking... -PA
  //profiler::inst()->add_counter( profiler::PCT_GENERIC5, 1 );

  const vector3d& v0 = m2->get_vert(f2->get_vert_ref(0)).get_point();
  vector3d p1 = c.get_base() - v0;
  vector3d p2 = c.get_end() - v0;
  const vector3d& poly_normal = f2->get_normal();

  rational_t d1 = dot( p1, poly_normal );
  rational_t d2 = dot( p2, poly_normal );
  rational_t r = c.get_core_radius();

  // Both on same side of poly plane and far away  (Hopefully most of the cases)
  if ( (d1>r && d2>r) || (d1<-r && d2<-r) )
    return false;

  // if not, can still hit if the closest point on the poly plane to the capsule
  // is within radius and is inside the polygon.  (Of course, any point would do,
  // but the closest is the easiest to find.

  vector3d in_point;
  if ( d1>0.0f && d2>0.0f )   // both on one side.  chose an endpoint
  {
    if ( d1 < d2 )
      in_point = c.get_base() - poly_normal*d1;
    else
      in_point = c.get_end() - poly_normal*d2;
  }
  else if ( d1<0.0f && d2<0.0f )  // both on the other side
  {
    if ( d1 > d2 )
      in_point = c.get_base() - poly_normal*d1;
    else
      in_point = c.get_end() - poly_normal*d2;
  }
  else    // intersects the plane.
  {
    if ( d1 < 0.0f )
      d1 = -d1;
    if ( d2 < 0.0f )
      d2 = -d2;
    if ( d1+d2 < 0.01f )
      d1 = d2 = 0.01f;  // safety valve
    in_point = c.get_base() + (c.get_end() - c.get_base()) * (d1/(d1+d2));
  }

  const vector3d& v1 = m2->get_vert(f2->get_vert_ref(1)).get_point();
  const vector3d& v2 = m2->get_vert(f2->get_vert_ref(2)).get_point();

  vector3d hit_loc = in_point;

  bool hit;
  int which_side = sgn( dot( cross(v1-v0,in_point-v1), poly_normal ) );
  if ( sgn( dot( cross((v2-v1),in_point-v2), poly_normal ) ) == which_side &&
       sgn( dot( cross((v0-v2),in_point-v0), poly_normal ) ) == which_side )
    hit = true;
  else
    hit = false;

  // First, does an edge of the poly hit the capsule?

  if ( !hit )
  {
    vector3d edge_hit_loc;
    if ( collide_segment_capsule( v0, v1, c, edge_hit_loc ) ||
         collide_segment_capsule( v1, v2, c, edge_hit_loc ) ||
         collide_segment_capsule( v2, v0, c, edge_hit_loc ) )
    {
      hit_loc = edge_hit_loc;
      hit = true;
    }
  }

  if (hit)
  {
    hit_list->push_back( hit_loc );
    normal_list1->push_back( -f2->get_normal() );
    normal_list2->push_back( f2->get_normal() );
  }

  return hit;
}



// this is the old function used for player-world collisions
bool collide_segment_capsule( const vector3d& n1, const vector3d& n2,
                              const vector3d& base, const vector3d& end, const rational_t radius,
                              vector3d& hit_point )
{
  vector3d n,c1,c2,c,closest_point,seg_point;//, d;
  rational_t n_len,c_len,t,inside_t,dot_c_n,dot_c1_n,dot_c1_c,denom;
  rational_t s,inside_s,t1;

  n = n2-n1;

  c1 = base - n1;
  c2 = end - n1;

  c = c2-c1;

  n_len = n.length();
  if ( n_len > 0.0001f )
    n /= n_len;
  else
    n = YVEC;
  c_len = c.length();
  if ( c_len > 0.0001f )
    c /= c_len;
  else
    c = YVEC;

  dot_c_n = dot(c,n);
  dot_c1_n = dot(c1,n);
  dot_c1_c = dot(c1,c);

  denom = (1.0f-dot_c_n*dot_c_n);

  // First, "Is there a point on the capsule within radius of the segment line?"

  if (dot_c_n>0.9f || dot_c_n<-0.9f)
    t=0;
  else
    t = (dot_c_n*dot_c1_n - dot_c1_c)/denom;

  if (t<0.0f) inside_t = 0.0f;
  else if (t>c_len) inside_t = c_len;
  else inside_t = t;

  s = dot_c1_n+inside_t*dot_c_n;
  closest_point = s*n;

  if (s<0.0f) inside_s = 0.0f;
  else if (s>n_len) inside_s = n_len;
  else inside_s = s;
  seg_point = n*inside_s;
  hit_point = n1+seg_point;

  rational_t distance2 = ((c1+inside_t*c) - closest_point).length2();

  if (distance2>sqr(radius)) return false;      // closest point is too far away
  if (s==inside_s) return true;                 // closest point is in the segment

  // Otherwise, find the point on the capsule line closest to the "seg_point"
  // "Seg_point" is the point on the segment closest to the capsule.

  t1 = dot(seg_point-c1,c);

  if (t1<0.0f) t1=0.0f;
  else if (t1>c_len) t1=c_len;

  distance2 = (seg_point - (c1 + c*t1)).length2();

  if (distance2>sqr(radius))
    return false;
  return true;
}

#ifdef DEBUG
int last_segment_capsule_hit_result=0;
#endif


// this is the NEW version used for targeting
bool collide_segment_capsule_accurate_result( const vector3d& n1, const vector3d& n2,
                              const vector3d& base, const vector3d& end, const rational_t radius,
                              vector3d& hit_point )
{
#ifdef DEBUG
    last_segment_capsule_hit_result = 0;
#endif
  vector3d o = n1;
  vector3d d = n2-n1;
  vector3d center = (base+end)*0.5f;
//  vector3d normal = (base-end).normalize();
//  rational_t depth = (base-end).length()*0.5f;
  vector3d normal = (base-end);
  rational_t depth = normal.length();
  assert(depth != 0);	// else division by zero in next line (dc 07/11/02)
  normal *= (1.0f / depth);
  depth *= 0.5f;

//  po          cpo, ipo;
  rational_t  rad2 = radius * radius, tc1, tc2, t=2.0f;

  // transform segment to cylinder's local space
  vector3d vo = o - center;
  vector3d vd = d;
//  vector3d axis = cross(normal, YVEC);
  vector3d axis( -normal.z, 0.0f, normal.x );

  rational_t al2 = axis.length2();
  if (al2>=1e-8f)
  {
    rational_t sinv = __fsqrt(al2);
    axis /= sinv;
//    rational_t cosv = dot(normal, YVEC);
    rational_t cosv = normal.y;

    rational_t dp;
    dp = dot(axis, vo)*(1.0f-cosv);
    vo = vector3d(vo.x*cosv + axis.x*dp + (axis.y*vo.z-axis.z*vo.y)*sinv,
                  vo.y*cosv + axis.y*dp + (axis.z*vo.x-axis.x*vo.z)*sinv,
                  vo.z*cosv + axis.z*dp + (axis.x*vo.y-axis.y*vo.x)*sinv);

    dp = dot(axis, vd)*(1.0f-cosv);
    vd = vector3d(vd.x*cosv + axis.x*dp + (axis.y*vd.z-axis.z*vd.y)*sinv,
                  vd.y*cosv + axis.y*dp + (axis.z*vd.x-axis.x*vd.z)*sinv,
                  vd.z*cosv + axis.z*dp + (axis.x*vd.y-axis.y*vd.x)*sinv);
  }

  // intersect segment with cylinder's surface
  rational_t  a, b, c, D;

  a = vd.x * vd.x + vd.z * vd.z;
  b = 2.0f * ( vd.x * vo.x + vd.z * vo.z );
  c = vo.x * vo.x + vo.z * vo.z - rad2;
  D = b * b - 4.0f * a * c;    // solve quadratic equation
  if( D >= 0.0f && a != 0.0f )
  {
    D = __fsqrt( D );
    a = 1.0f / ( 2.0f * a );
    tc1 = (-b + D) * a;           // parameter of first intersection
    tc2 = (-b - D) * a;           // parameter of second intersection
    t = 2.0f;
    if(tc1 >= 0.0f && tc1 <= 1.0f )
    {
      if( tc2 >= 0.0f && tc2 <= 1.0f ) t = min( tc1, tc2 );
      else t = tc1;
    }
    else
      if( tc2 >= 0.0f && tc2 <= 1.0f )
        t = tc2;
  }
  if( t <= 0.0f || t >= 1.0f || __fabs(vo.y + vd.y * t) > depth )
  {
    t = 2.0f;   // closest point of the intersection is out of cylinder's volume
    // check hemisphere caps
    rational_t dt,a,rdl2=1.0f/d.length2();
    dt = dot(d,base-o)*rdl2;
    a = rad2 - (d*dt + o - base).length2();
    if (a >= 0.0f) // no intersection
    {
      a = __fsqrt(a * rdl2);
      if (dt - a >= 0.0f)
      {
        t = min( t, dt - a );
#ifdef DEBUG
        last_segment_capsule_hit_result = 2;
#endif
      }
    }

    dt = dot(d,end-o)*rdl2;
    a = rad2 - (d*dt + o - end).length2();
    if (a >= 0.0f) // no intersection
    {
      a = __fsqrt(a * rdl2);
      if (dt - a >= 0.0f)
      {
        t = min( t, dt - a );
#ifdef DEBUG
        last_segment_capsule_hit_result = 2;
#endif
      }
    }
  }
  else // if (t >= 0.0f && t <= 1.0f)
  {
#ifdef DEBUG
    last_segment_capsule_hit_result = 1;
#endif
  }

  if( t >= 0.0f && t <= 1.0f )
  {
    hit_point = o + d * t;
    return true;
  }
  return false;
}



bool collide_segment_cylinder( const vector3d& o, const vector3d& d,
                               const vector3d& center, const vector3d& normal, const rational_t radius, const rational_t depth,
                               vector3d& hit_point )
{
//  po          cpo, ipo;
  rational_t  rad2 = radius * radius, tu = -1.0f, tl = -1.0f, tc1, tc2, t;

  // transform segment to cylinder's local space
  #if 0
  matrix4x4   m;
  vector3d vo2,vd2;
  if( normal != YVEC )
  {
//    cpo.set_rot( cross(normal, YVEC), fast_acos(dot(YVEC, normal)) );
    cpo.set_rot( vector3d( -normal.z, 0.0f, normal.x ), fast_acos(normal.y) );
  }
  else
  {
    cpo = po_identity_matrix;
  }
  cpo.set_rel_position( center );
  ipo = cpo.inverse();
  m = ipo.get_matrix();
  vo2 = xform3d_1_homog(m,o);  // transformed segment's origin
  vd2 = xform3d_1_homog(m, o + d) - vo2;  // transformed segment's delta
  #else
  vector3d vo = o - center;
  vector3d vd = d;
//  vector3d axis = cross(normal, YVEC);
  vector3d axis( -normal.z, 0.0f, normal.x );

  rational_t al2 = axis.length2();
  if (al2>=1e-8f)
  {
    rational_t sinv = __fsqrt(al2);
    axis /= sinv;
//    rational_t cosv = dot(normal, YVEC);
    rational_t cosv = normal.y;

    rational_t dp;
    dp = dot(axis, vo)*(1.0f-cosv);
    vo = vector3d(vo.x*cosv + axis.x*dp + (axis.y*vo.z-axis.z*vo.y)*sinv,
                  vo.y*cosv + axis.y*dp + (axis.z*vo.x-axis.x*vo.z)*sinv,
                  vo.z*cosv + axis.z*dp + (axis.x*vo.y-axis.y*vo.x)*sinv);

    dp = dot(axis, vd)*(1.0f-cosv);
    vd = vector3d(vd.x*cosv + axis.x*dp + (axis.y*vd.z-axis.z*vd.y)*sinv,
                  vd.y*cosv + axis.y*dp + (axis.z*vd.x-axis.x*vd.z)*sinv,
                  vd.z*cosv + axis.z*dp + (axis.x*vd.y-axis.y*vd.x)*sinv);
  }
  #endif

  // intersect segment with cylinder's top and bottom planes
  if( vd.y != 0.0f )
  {
    vector3d    vtmp;
    rational_t inv_vd_y = 1.0f / vd.y;

    t = ( depth - vo.y ) * inv_vd_y;
    if( t >= 0.0f && t <= 1.0f )
    {
      vtmp = vo + t * vd;
      if( vtmp.x * vtmp.x + vtmp.z * vtmp.z <= rad2 ) tu = t;   // segment intersects cylinder's top
    }

    t = ( -depth - vo.y ) * inv_vd_y;
    if( t >= 0.0f && t <= 1.0f )
    {
      vtmp = vo + t * vd;
      if( vtmp.x * vtmp.x + vtmp.z * vtmp.z <= rad2 ) tl = t;   // segment intersects cylinder's bottom
    }
  }

  // intersect segment with cylinder's surface
  rational_t  a, b, c, D;

  a = vd.x * vd.x + vd.z * vd.z;
  b = 2.0f * ( vd.x * vo.x + vd.z * vo.z );
  c = vo.x * vo.x + vo.z * vo.z - rad2;
  D = b * b - 4.0f * a * c;   // solve quadratic equation
  if( D >= .0f && a != .0f )
  {
    D = __fsqrt( D );
    a = 1.0f / ( 2.0f * a );
    tc1 = (-b + D) * a;           // parameter of first intersection
    tc2 = (-b - D) * a;           // parameter of second intersection
    t = -1.0f;
    if(tc1 >= 0.0f && tc1 <= 1.0f )
    {
      if( tc2 >= .0f && tc2 <= 1.0f ) t = min( tc1, tc2 );
      else t = tc1;
    }
    else if( tc2 >= 0.0f && tc2 <= 1.0f ) t = tc2;
  }
  if( t >= 0.0f && __fabs(vo.y + vd.y * t) > depth ) t = -1.0f;   // closest point of the intersection is out of cylinder's volume

  // get the closest point if any
  if( t >= 0.0f )
  {
    if( tu >= 0.0f ) t = min( t, tu );
    if( tl >= 0.0f ) t = min( t, tl );
  }
  else if( tu >= 0.0f )
  {
    t = tu;
    if( tl >= 0.0f ) t = min( t, tl );
  }
  else if( tl >= 0.0f )
  {
    t = tl;
  }

  if( t >= 0.0f )
  {
    hit_point = o + t * d;
    return true;
  }
  return false;
}


//
//-------------------------DISTANCE FUNCTIONS-----------------------------------------//
//

rational_t dist_point_polygon( const vector3d& p,
                               const vector3d& v0, const vector3d& v1, const vector3d& v2)
{
  // Strategy shall be to fake a projection of the capsule down onto the plane.
  // This image should be an ellipse, but we will approximate it by a 2D capsule.

  // What??  I think it is a 2D capsule.  What was I thinking... -PA

  vector3d poly_normal,p1,in_point,temp; // p2,proj1,proj2,
  rational_t d1;
  bool hit=0;
  int num_verts,which_side,i;
  vector3d hit_loc;

  num_verts = 3;
  poly_normal = cross(v1-v0,v2-v1);
  poly_normal.normalize();
  p1 = p-v0;

  d1 = dot(p1,poly_normal);

  // if not, can still hit if the closest point on the poly plane to the capsule
  // is within radius and is inside the polygon.  (Of course, any point would do,
  // but the closest is the easiest to find.
  bool interior = true;

  // simplified things...
  vector3d vlist[4];
  vlist[0] = v0;
  vlist[1] = v1;
  vlist[2] = v2;
  vlist[3] = v0;

  in_point = p-d1*poly_normal;

  // Moving right along

  hit_loc = in_point;

  // Compare the three to make sure they have the same sign...
  // I think we already know the sign actually
  which_side = sgn(dot(cross((vlist[1]-vlist[0]),in_point-vlist[1]),poly_normal));
  for (i=1;i<3;++i)
  {
    temp = cross((vlist[i+1]-vlist[i]),in_point-vlist[i+1]);
    if (sgn(dot(temp,poly_normal))!=which_side)
      break;
  }
  if (i==3) hit=true;

  // Else, nearest edge.

  vector3d edge_hit_loc;
  rational_t max_dist = 1e11f;
  if (!hit)
  {
    interior = false;
    for (i=0;/* !hit && */ i<3;++i)
    {
      rational_t dist = dist_point_segment(p,vlist[i],vlist[i+1],edge_hit_loc);
      if (dist<max_dist)
      {
        hit_loc = edge_hit_loc;
        max_dist = dist;
        ++hit;
      }
    }
  }
  return (hit_loc-p).length();
}


rational_t dist_point_segment( const vector3d& p,
                               const vector3d& n1, const vector3d& n2,
                               vector3d& hit_point )
{
  vector3d n = n2-n1;
  rational_t n_len = n.length();
  // cover the zero length segment case
  if (n_len > 0.0f)
    n /= n_len;

  vector3d c1 = p - n1;

  rational_t ofs = dot(c1,n);
  if (ofs<0.0f) ofs = 0.0f;
  else if (ofs>n_len) ofs = n_len;

  hit_point = n1+ofs*n;

  return (hit_point-p).length();
}

//
//-------------------------PARTITION3 FUNCTIONS--------------------------------------//
//


bool collide_sphere_partition3( const vector3d& pos, rational_t r,
                                const partition3& h1,
                                vector3d& hit_point )
{
  rational_t dist = h1.distance(pos);

  if (dist>0.0f)
    hit_point = pos-dist*h1.get_normal();
  else
    hit_point = pos;
  return dist<r;
}


bool collide_sphere_two_partition3s( const vector3d& pos, rational_t r,
                                     const partition3& h1,
                                     const partition3& h2,
                                     vector3d& hit_point )
{
  vector3d hit_point1,hit_point2;
  if (collide_sphere_partition3(pos, r, h1, hit_point1) &&
      collide_sphere_partition3(pos, r, h2, hit_point2) )
  {
    // To hit the intersection of partition3s, either the sphere hits "face1"
    if (h2.distance(hit_point1)<0.0f)
    {
      hit_point = hit_point1;
      return true;
    }
    // or the sphere hits "face1"
    else if (h1.distance(hit_point2)<0.0f)
    {
      hit_point = hit_point2;
      return true;
    }
    // or the sphere hits the line the line which is the
    // intersection of h1 and h2 (the intersection line)
    else
    {
      // this is a vector in the direction of the intersection line.
      // It is not normalized.
      vector3d line_dir = cross(h1.get_normal(), h2.get_normal());
      vector3d rel_center = h1.get_center();
      rational_t d2 = line_dir.length2();

      // parallel
      if (d2<.000001f)
      {
        if ( dot(h1.get_normal(), h2.get_normal()) < 0.0f )
          return false;         // facing away from each other

        if ( h1.get_offset() < h2.get_offset() )
          return collide_sphere_partition3(pos, r, h1, hit_point);
        else
          return collide_sphere_partition3(pos, r, h2, hit_point);
      }

      // not parallel
      vector3d p1 = (pos-rel_center)-dot(h1.get_normal(),(pos-rel_center))*h1.get_normal();  // projection of pos onto plane 1
      vector3d p2 = h2.get_center() - rel_center;             // any old point on plane 2

      // this is a vector lying on plane 1 which points from p1 directly toward the intersection line.
      // It is not normalized.
      vector3d plane_dir = cross( line_dir, h1.get_normal());

      // this is the scale factor such that p1-t*plane_dir is the projection of pos onto the intersection line
      rational_t t = dot(p1-p2,h2.get_normal()) / dot(plane_dir, h2.get_normal());

      hit_point = p1-t*plane_dir + rel_center;

      return (pos-hit_point).length2() < r*r;
    }
  }
  return false;
}

#include "pmesh.h"


rational_t det3( vector3d r1, vector3d r2, vector3d r3 );
rational_t det3( vector3d r1, vector3d r2, vector3d r3 )
{
  return ( r1.x * (r2.y * r3.z - r2.z * r3.y) +
           r1.y * (r2.z * r3.x - r2.x * r3.z) +
           r1.z * (r2.x * r3.y - r2.y * r3.x) );
}

bool show_me = false;

bool collide_sphere_three_partition3s_almost_parallel( const vector3d& pos, rational_t r,
                                                       const partition3& h1,
                                                       const partition3& h2,
                                                       const partition3& h3,
                                                       vector3d& hit_point );
bool collide_sphere_three_partition3s_almost_parallel( const vector3d& pos, rational_t r,
                                                       const partition3& h1,
                                                       const partition3& h2,
                                                       const partition3& h3,
                                                       vector3d& hit_point )
{
  vector3d hit_point1, hit_point2;

  if ( !collide_sphere_two_partition3s(pos, r, h1, h3, hit_point1)
       || !collide_sphere_two_partition3s(pos, r, h2, h3, hit_point2) )
    return false;

  if ( dot(h1.get_normal(), h2.get_normal()) >= 0.0f )
    hit_point = h1.distance(pos) < h2.distance(pos) ? hit_point2 : hit_point1;
  else
    hit_point = h1.distance(pos) < 0.0f ? hit_point2 : hit_point1;

  return true;
}

bool collide_sphere_three_partition3s( const vector3d& pos, rational_t r,
                                       const partition3& h1,
                                       const partition3& h2,
                                       const partition3& h3,
                                       vector3d& hit_point )
{
  vector3d hit_point1, hit_point2, hit_point3;

  // wash out degenerate cases, where two of the three planes are essentially parallel.
//  rational_t d1 = h1.get_offset();
//  rational_t d2 = h2.get_offset();
//  rational_t d3 = h3.get_offset();
  if (dot(h1.get_normal(),h2.get_normal())>0.999f)
    return collide_sphere_three_partition3s_almost_parallel( pos, r, h1, h2, h3, hit_point );
  else if (dot(h2.get_normal(),h3.get_normal())>0.999f)
    return collide_sphere_three_partition3s_almost_parallel( pos, r, h2, h3, h1, hit_point );
  else if (dot(h1.get_normal(),h3.get_normal())>0.999f)
    return collide_sphere_three_partition3s_almost_parallel( pos, r, h3, h1, h2, hit_point );

  /*
    if (d3<d1)
    return collide_sphere_two_partition3s(pos, r, h3, h2, hit_point);
    else
    return collide_sphere_two_partition3s(pos, r, h1, h2, hit_point);
  */
  // A sphere collides with the intersection of all 3 if :
  //  1. it collides with some pair of planes and the hit_point lies behind the third plane   ..or..
  if ( collide_sphere_two_partition3s(pos, r, h1, h2, hit_point3) &&
       collide_sphere_two_partition3s(pos, r, h2, h3, hit_point1) &&
       collide_sphere_two_partition3s(pos, r, h1, h3, hit_point2) )
  {
    if (h1.distance(hit_point1)<0.0f)
    {
      hit_point = hit_point1;
      return true;
    }
    else if (h2.distance(hit_point2)<0.0f)
    {
      hit_point = hit_point2;
      return true;
    }
    else if (h3.distance(hit_point3)<0.0f)
    {
      hit_point = hit_point3;
      return true;
    }
    //  2. it intersects the intersection point of all three planes.
    else
    {
      // If we got this far, we must be in the following situation:
      //    1. The sphere intersects every pair of partition3s
      //    2. The intersection points with each pair lie in front of the third plane, hence outside
      //       the common intersection.
      //    Upshot is that at this stage, the sphere will intersect the volume iff it contains the
      // intersection point of the three planes.

      // Use Cramer's rule
      vector3d  r4( h1.get_offset(), h2.get_offset(), h3.get_offset() );
//      rational_t invdet = 1.0f / det3( h1.get_normal(), h2.get_normal(), h3.get_normal() );
      // can't overflow because of checks at top of function
      hit_point.x = det3(r4, h2.get_normal(), h3.get_normal());
      hit_point.y = det3(h1.get_normal(), r4, h3.get_normal());
      hit_point.z = det3(h1.get_normal(), h2.get_normal(), r4);
      return (pos-hit_point).length2() < r*r;
    }
  }

  return false;
}


static bool find_intersection_with_entities( const vector3d& p0, const vector3d& p1, region_node* rn, unsigned int flags, vector3d* hit_loc, vector3d* hit_normal, region_node** hit_region=NULL, entity** hit_entity=NULL );

// Return true if given line segment hits any entity marked with the camera collision or scannable flag;
// hit_loc and hit_normal will be set accordingly.
bool find_intersection( const vector3d& p0, const vector3d& p1,
                        region_node* start_region,
                        unsigned int flags,
                        vector3d* hit_loc, vector3d* hit_normal,
                        region_node** hit_region, entity** hit_entity )
{
  ADD_PROF_COUNT(profcounter_find_intersection, 1);
  START_PROF_TIMER(proftimer_find_intersection);
  bool hit = false;
  if ( flags & FI_COLLIDE_WORLD )
  {
    START_PROF_TIMER(proftimer_find_int_world);
    hit = g_world_ptr->get_the_terrain().find_intersection( p0, p1, *hit_loc, *hit_normal );
    STOP_PROF_TIMER(proftimer_find_int_world);
  }

  if ( start_region && (flags&FI_COLLIDE_ENTITIES_MASK) )
  {
    START_PROF_TIMER(proftimer_find_int_ent);
    vector3d tp1;
    if ( hit )
      tp1 = *hit_loc;
    else
      tp1 = p1;
    region::prepare_for_visiting();
    entity::prepare_for_visiting();
    hit |= find_intersection_with_entities( p0, tp1,
                                            start_region,
                                            flags,
                                            hit_loc, hit_normal,
                                            hit_region, hit_entity );
    STOP_PROF_TIMER(proftimer_find_int_ent);
  }
  STOP_PROF_TIMER(proftimer_find_intersection);

  return hit;
}

// find nearest intersection of line segment with any entity matching the given flags
static bool find_intersection_with_entities( const vector3d& p0, const vector3d& p1,
                                             region_node* rn,
                                             unsigned int flags,
                                             vector3d* hit_loc, vector3d* hit_normal,
                                             region_node** hit_region,
                                             entity** hit_entity )
{
  bool hit = false;
  region* rg = rn->get_data();
  rg->visit();
  if ( (p1-p0).length2() > 0.000001f )
  {
    vector3d tp1 = p1;
    region::entity_list::const_iterator ei, ei_end;

    if ( !(flags & FI_COLLIDE_CAMERA) )
    {
//      ei = rg->get_entities().begin();
//      ei_end = rg->get_entities().end();
      ei = rg->get_possible_collide_entities().begin();
      ei_end = rg->get_possible_collide_entities().end();
    }
    else
    {
      ei = rg->get_camera_collision_entities().begin();
      ei_end = rg->get_camera_collision_entities().end();
    }

    for ( ; ei!=ei_end; ++ei )
    {
      entity* e = *ei;
      if ( e && !e->already_visited() && e->are_collisions_active())
      {
        START_PROF_TIMER(proftimer_find_int_ent_ent);
        e->visit();
        if(
            (
              ( (flags&FI_COLLIDE_ENTITY) && e->has_entity_collision() )
              || ( (flags&FI_COLLIDE_BEAMABLE) && e->is_beamable() )
              || ( (flags&FI_COLLIDE_SCANNABLE) && e->is_scannable() )
              || ( (flags&FI_COLLIDE_AI_LOS) && e->is_ai_los_block() && e->get_colgeom() && !e->has_ai_ifc() )
              || ( (flags&FI_COLLIDE_CAMERA) && e->has_camera_collision() )
            )
            &&
           !(
              (flags & FI_COLLIDE_HINTS) &&
              (
                ((flags & FI_COLLIDE_ENTITY_NO_CAPSULES) && e->get_colgeom() && e->get_colgeom()->get_type() == collision_geometry::CAPSULE)
                || ((flags & FI_COLLIDE_ENTITY_VISIBLE_ONLY) && !e->is_visible())
              )
            )
          )
        {
          START_PROF_TIMER(proftimer_find_int_ent_ent_col);
          bool collided = collide_segment_entity( p0, tp1, e, hit_loc, hit_normal, 0.0f, !(flags&FI_COLLIDE_ENTITY_NO_REAR_CULL));
          STOP_PROF_TIMER(proftimer_find_int_ent_ent_col);

          if ( collided )
          {
            hit = true;
            if ( hit_region )
              *hit_region = rn;
            if ( hit_entity )
              *hit_entity = e;
            // stop if we're inside an entity
            if ( dot(tp1-p0,*hit_loc-p0) <= 0.01f )
              break;
            tp1 = *hit_loc;
          }
        }
        STOP_PROF_TIMER(proftimer_find_int_ent_ent);
      }
    }

    START_PROF_TIMER(proftimer_find_int_ent_reg);
    if ( !hit )
    {
      // segment didn't hit an entity in this region;
      // if segment intersects a portal, check entities in adjacent region
      edge_iterator pi;
      edge_iterator pi_end = rn->end();
      for ( pi=rn->begin(); pi!=pi_end; ++pi )
      {
        region_node* dest = (*pi).get_dest();
        if ( !dest->get_data()->already_visited() )
        {
          portal* port = (*pi).get_data();
          if (port->is_active())
          {
            // check for intersection with sphere
            vector3d dummy_hit_loc;

            START_PROF_TIMER(proftimer_find_int_ent_reg_col);
            bool col = collide_segment_sphere( p0, tp1, port->get_effective_center(), port->get_effective_radius(), &dummy_hit_loc );
            STOP_PROF_TIMER(proftimer_find_int_ent_reg_col);

//            #pragma fixme("I am not sure if the 'touches_segment' function works, but it is a little cheaper than the 'collide_segment_sphere' JDB 05-08-01")
//            bool col = port->touches_segment(p0, tp1);
            if ( col )
            {
              // check for collision with mesh
              int i;
              for ( i=0; i<port->get_max_faces(); ++i )
              {
                START_PROF_TIMER(proftimer_find_int_ent_reg_col);
                col = collide_polygon_segment( i, port, p0, tp1, dummy_hit_loc );
                STOP_PROF_TIMER(proftimer_find_int_ent_reg_col);

                if ( col )
                {
                  STOP_PROF_TIMER(proftimer_find_int_ent_reg);
                  return find_intersection_with_entities( p0, tp1,
                                                          dest,
                                                          flags,
                                                          hit_loc, hit_normal,
                                                          hit_region, hit_entity);
                }
              }
            }
          }
        }
      }
    }
    STOP_PROF_TIMER(proftimer_find_int_ent_reg);
  }
  return hit;
}


// find intersection of sphere with any entity marked for camera collision
static bool find_intersection_with_camera_collision_entity( const vector3d& p,
                                                            rational_t r,
                                                            const vector3d& v,
                                                            region_node* rn,
                                                            vector3d& hit_loc )
{
//  bool hit = false;
  region* rg = rn->get_data();
  rg->visit();
  region::entity_list::const_iterator ei = rg->get_camera_collision_entities().begin();
  region::entity_list::const_iterator ei_end = rg->get_camera_collision_entities().end();
  for ( ; ei!=ei_end; ++ei )
  {
    entity* e = *ei;
    if ( e && !e->already_visited() && e->are_collisions_active() )
    {
      e->visit();
      // compare spheres
      if ( (p-e->get_abs_position()).length2() < sqr(r+e->get_radius()) )
      {
        // check collision mesh
        if ( e->get_colgeom()->get_type() == collision_geometry::MESH )
        {
          // vector3d local_p = e->get_abs_po().inverse_xform( p );
          vector3d local_p = e->get_abs_po().fast_inverse_xform( p );

          // vector3d local_v = e->get_abs_po().non_affine_inverse_xform( v );
          vector3d local_v = e->get_abs_po().fast_non_affine_inverse_xform( v );

          if ( collide_sphere_mesh( local_p, r, local_v, static_cast<cg_mesh*>( e->get_colgeom() ), hit_loc ) )
          {
            // hit_loc = e->get_abs_po().slow_xform( hit_loc );
            hit_loc = e->get_abs_po().fast_8byte_xform( hit_loc );
            return true;
          }
        }
      }
    }
  }
  // CTT 090899 - My first instinct is to say that this recursion is unnecessary.
  /*
    // sphere didn't hit an entity in this region;
    // if sphere intersects a portal, check entities in adjacent region
    edge_iterator pi;
    edge_iterator pi_end = rn->end();
    for ( pi=rn->begin(); pi!=pi_end; ++pi )
    {
      region_node* dest = (*pi).get_dest();
      if ( !dest->get_data()->already_visited() )
      {
        portal* port = (*pi).get_data();
        // compare spheres
        if ( norm(p-port->get_effective_position()) < r+port->get_effective_radius() )
        {
          // check mesh
          int i;
          vector3d dummy_hit_loc;
          for ( i=0; i<port->get_max_faces(); ++i )
          {
            if ( collide_sphere_polygon( p, r, port->get_face_ptr(i), port, dummy_hit_loc ) )
            {
              return find_intersection_with_camera_collision_entity( p,
              r,
              v,
              dest,
              hit_loc );
            }
          }
        }
      }
    }
  */
  return false;
}


bool in_world( const vector3d& p,
               rational_t r,
               const vector3d& v,
               region_node* start_region,
               vector3d& hit_loc,
               bool camera)
{
  bool hit = false;
  if ( camera && start_region != NULL )
  {
    region::prepare_for_visiting();
    entity::prepare_for_visiting();
    hit = find_intersection_with_camera_collision_entity( p, r, v, start_region, hit_loc );
  }

  if ( !hit )
  {
    vector3d n;
    return g_world_ptr->get_the_terrain().in_world( p, r, v, n, hit_loc );
  }
  return false;
}




















































/*
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//                             THE OLD                                //
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
#if 0

vector on_plane;


int collide_capsules(capsule & cap1,capsule & cap2,fixed & distance,vector & diff)
{
  vector p1,p2,q1,q2;
  vector a,b,c,d;
  fixed w,x,y1,y2,z;
  fixed s,t;
  fixed rp,rq;
  // Copy capsule info to local variables
  p1 = cap1.base;
  p2 = cap1.end;
  rp = cap1.radius;
  q1 = cap2.base;
  q2 = cap2.end;
  rq = cap2.radius;

  a = p1;
  b = p2-p1;
  c = q1;
  d = q2-q1;

  // Think. Then code.
  w = dot(b,b);
  x = dot(d,d);
  y1 = dot(a-c,b);
  y2 = dot(c-a,d);
  z = dot(b,d);

  // Think harder. Then recode.
  float denom = z*z-w*x;

  if (denom>-0.1f) denom = -0.1f;   // kludge for now!!!!!! 1/29/98

  //{
  s = (y1*z + w*y2)/denom;
  t = (y2*z + x*y1)/denom;

  if (s<0) s=0;
  else if (s>1) s = 1;
  if (t<0) t=0;
  else if (t>1) t=1;
  diff = (a+t*b)-(c-s*d);
  //}
#if 0
  else    // this is the essentially parallel case... simpler.
  {
    // NOT DONE YET 1/26/98 PTA <<<< can cause crashes in extreme cases.  When used only for projectiles against actors,
    // a parallel case is pretty unlikely though.
  }
#endif
  distance = norm(diff);
  return (distance<rp+rq);
}


#if 0

int collide_view_segment_polygon(vector & p1,vector & p2,polygon * p,int & splits)
{
  vertex * * vlist;
  int num_verts,i,s,s_prev,fail;
  vector normal,on_plane,edge_vect;
  fixed d1,d2,denom;

  vlist = p->get_vertex_list();
  num_verts = p->num_sides();
  normal = p->get_normal()->view;

  d1 = dot(p1-vlist[0]->view,normal);
  d2 = dot(p2-vlist[0]->view,normal);

  // Both on same side of poly plane
  if(sgn(d1)==sgn(d2))    // (((d1^d2)&0x80000000)==0)
  {
    splits = 0;
    return 0;
  }

  else splits = 1;

  // The sign issues are correct here.
  denom = d1-d2;
  on_plane = d1/denom*p2 - d2/denom*p1;

  s = 0;
  fail = false;
  for (i=0;i<num_verts;++i)
  {
    edge_vect = vlist[i+1]->view-vlist[i]->view;
    s_prev = s;
    s = sgn(dot(cross(on_plane-vlist[i]->view,edge_vect),normal));
    if (s+s_prev==0)
    {
      fail = true;
      i = num_verts;
    }
  }
  return !fail;
}


int collide_view_poly_poly(polygon * p1,polygon * p2)
{
  int i,j,hit,nums_verts[2],splits,try_count;
  vertex * * vlists[2];
  polygon * polys[2];

  polys[0] = p1;
  polys[1] = p2;
  vlists[0] = p1->get_vertex_list();
  vlists[1] = p2->get_vertex_list();
  nums_verts[0] = p1->num_sides();
  nums_verts[1] = p2->num_sides();
  hit = 0;
  try_count = 1;
  for (i=0;i<try_count && !hit;++i)
    for (j=0;j<nums_verts[i] && !hit;++j)
    {
      if (collide_view_segment_polygon(vlists[i][j+1]->view,vlists[i][j]->view,polys[1-i],splits))
        hit = true;
      if (splits) try_count = 2;
    }
  return hit;
}

int collide_view_poly_poly_list(polygon * p1,polygon * plist,int pcount,int mark_hits)
{
  int i,j,k,hit,nums_verts[2],splits,try_count;
  vertex * * vlists[2];
  polygon * polys[2];

  polys[1] = p1;
  vlists[1] = p1->get_vertex_list();
  nums_verts[1] = p1->num_sides();

  hit = 0;
  for (k=0;k<pcount && !hit;++k)
  {
    polys[0] = &(plist[k]);
    vlists[0] = polys[0]->get_vertex_list();
    nums_verts[0] = polys[0]->num_sides();
    hit = 0;
    try_count = 1;
    for (i=0;i<try_count && !hit;++i)
      // if not marking hits on polygons, we can bail as soon as we score a hit
      for (j=0;j<nums_verts[i] && (!hit || !mark_hits);++j)
      {
        if (collide_view_segment_polygon(vlists[i][j+1]->view,vlists[i][j]->view,polys[1-i],splits))
          hit = true;
        if (splits) try_count = 2;
      }
    if (mark_hits && hit) polys[0]->set_hit(1);
  }
  return hit;
}

#endif


int collide_world_polygon_sphere(polygon * p,vector ctr,float radius)
{
  // Strategy shall be to fake a projection of the capsule down onto the plane.
  // This image should be an ellipse, but we will approximate it by a 2D capsule.

  // What??  I think it is a 2D capsule.  What was I thinking... -PA

  vertex * * vlist;
  vector rel_v[MAX_VERTICES+1],poly_normal,p1,proj1,in_point,temp;
  fixed d1;
  int num_verts,i,hit;

  vlist = p->get_vertex_list();
  poly_normal = p->get_normal()->world;
  p1 = ctr-vlist[0]->world;

  d1 = dot(p1,poly_normal);

  // Far away from poly plane.
  if (d1>radius || d1<-radius)
  {
    return 0;
  }

  hit = 0;
  float plane_radius = __fsqrt(radius*radius-d1*d1);

  num_verts = p->num_sides();
  for (i=0;i<=num_verts;++i)
  {
    rel_v[i] = vlist[i]->world-vlist[0]->world;
  }
  proj1 = p1-d1*poly_normal;
  float cross_norm,vlen;
  // Interior or exterior but near an edge?
  for (i=0;i<num_verts;++i)
  {
    if ((cross_norm = dot(cross(proj1-rel_v[i],rel_v[i+1]-rel_v[i]),poly_normal))>0)
    {
      float edge_len = norm(rel_v[i+1]-rel_v[i]);
      vlen = cross_norm/edge_len;
      if (vlen<plane_radius)  // within plane_radius of the line?
      {
        float ep = dot(proj1-rel_v[i],rel_v[i+1]-rel_v[i])/edge_len;
        if (ep<0)
          if (ep*ep+vlen*vlen>plane_radius) break;  // nope
          else if (ep>edge_len)
            if ((ep-edge_len)*(ep-edge_len)+vlen*vlen>plane_radius) break; // nope
      }
      else break; //nope
    }
  }
  hit = (i==num_verts);
  return hit;
}

// collision_normal is NOT normalized
int collide_world_capsule_poly(capsule & c, polygon * p,int & hit_count,vector * hit_list,int & interior)
{
  // Strategy shall be to fake a projection of the capsule down onto the plane.
  // This image should be an ellipse, but we will approximate it by a 2D capsule.

  // What??  I think it is a 2D capsule.  What was I thinking... -PA

  vertex * * vlist;
  vector v0,v1,v2,poly_normal,p1,p2,proj1,proj2,in_point,temp;
  fixed d1,d2,r;
  int hit=0,num_verts,which_side,i;
  hit_count = 0;

  vlist = p->get_vertex_list();
  num_verts = p->num_sides();
  v0 = vlist[0]->world;
  v1 = vlist[1]->world;
  v2 = vlist[2]->world;
  poly_normal = p->get_normal()->world;
  p1 = c.base-v0;
  p2 = c.end-v0;

  v1 -= v0;
  v2 -= v0;

  d1 = dot(p1,poly_normal);
  d2 = dot(p2,poly_normal);
  r = c.radius;

  // Both on same side of poly plane and far away  (Hopefully most of the cases)
  if ((d1>r && d2>r)||(d1<-r && d2<-r))
  {
    return 0;
  }

  // if not, can still hit if the closest point on the poly plane to the capsule
  // is within radius and is inside the polygon.  (Of course, any point would do,
  // but the closest is the easiest to find.
  interior = true;
  if (!hit)
  {
    if (d1>0 && d2>0)   // both on one side.  chose an endpoint
    {
      if (d1<d2)
        in_point = c.base-d1*poly_normal;
      else
        in_point = c.end-d2*poly_normal;
    }
    else if (d1<0 && d2<0)  // both on the other side
    {
      if (d1>d2)
        in_point = c.base-d1*poly_normal;
      else
        in_point = c.end-d2*poly_normal;
    }
    else    // intersects the plane.
    {
      if (d1<0) d1=-d1;
      if (d2<0) d2=-d2;
      if (d1+d2<0.01f) d1 = d2 = 0.01f;  // safety valve
      in_point = c.base + (c.end-c.base)*(d1/(d1+d2));
    }

    // Moving right along
    hit_list[hit_count++] = in_point;

    which_side = sgn(dot(cross((vlist[1]->world-vlist[0]->world),in_point-vlist[1]->world),poly_normal));
    for (i=1;i<num_verts;++i)
    {
      temp = cross((vlist[i+1]->world-vlist[i]->world),in_point-vlist[i+1]->world);
      if (sgn(dot(temp,poly_normal))!=which_side)
        break;
    }
    if (i==num_verts) hit=true;
  }

  // First, does an edge of the poly hit the capsule?

  if (!hit)
  {
    interior = false;
    --hit_count;
    for (i=0; i<num_verts;++i)
    {
      if (collide_segment_capsule(vlist[i]->world,vlist[i+1]->world,c,hit_list[hit_count]))
      {
        ++hit;
        ++hit_count;
      }
    }
  }
  return hit;
}

int collide_segment_capsule(vector & n1,vector & n2,capsule & cap,vector & hit_point)
{
  vector n,c1,c2,d,c,closest_point,seg_point;
  fixed n_len,c_len,t,inside_t,dot_c_n,dot_c1_n,dot_c1_c,distance,denom;
  fixed s,inside_s,t1;

  n = n2-n1;

  c1 = cap.base - n1;
  c2 = cap.end - n1;

  c = c2-c1;

  n_len = double_norm(n);
  n = n/n_len;
  c_len = double_norm(c);
  c = c/c_len;

  dot_c_n = dot(c,n);
  dot_c1_n = dot(c1,n);
  dot_c1_c = dot(c1,c);

  denom = (1-dot_c_n*dot_c_n);

  // First, "Is there a point on the capsule within radius of the segment line?"[

  if (dot_c_n>_f(0.9f) || dot_c_n<_f(-0.9f))
    t=0;
  else
    t = (dot_c_n*dot_c1_n - dot(c1,c))/denom;

  if (t<0) inside_t = 0;
  else if (t>c_len) inside_t = c_len;
  else inside_t = t;

  s = dot_c1_n+inside_t*dot_c_n;
  closest_point = s*n;

  if (s<0) inside_s = 0;
  else if (s>n_len) inside_s = n_len;
  else inside_s = s;
  seg_point = n*inside_s;
  hit_point = n1+seg_point;

  distance = double_norm( (c1+inside_t*c) - closest_point );

  if (distance>cap.radius) return 0;      // closest point is too far away
  if (s==inside_s) return 1;              // closest point is in the segment

  // Otherwise, find the point on the capsule line closest to the "seg_point"
  // "Seg_point" is the point on the segment closest to the capsule.

  t1 = dot(seg_point-c1,c);

  if (t1<0) t1=0;
  else if (t1>c_len) t1=c_len;

  distance = norm(seg_point - (c1 + c*t1));

  if (distance>cap.radius) return 0;
  return 1;
}


int collide_world_segment_polygon(vector& p1,vector& p2,polygon* p)
{
  vertex** vlist = p->get_vertex_list();
  vector &normal = p->get_normal()->world;
  vector on_plane;
  fixed d1,d2;

  d1 = dot(p1-vlist[0]->world,normal);
  d2 = dot(p2-vlist[0]->world,normal);

  if (sgn(d1) == sgn(d2))
    return 0;  // both on same side of poly plane

  // The sign issues are correct here.
  float denom = d1 - d2;
  on_plane = d1/denom*p2 - d2/denom*p1;

  vector edge_vec;
  for (int i=p->num_sides(); i; --i,++vlist)
  {
    if (dot(on_plane-vlist[0]->world,cross(normal,vlist[1]->world-vlist[0]->world)) < 0)
      return 0;
  }
  return 1;
}

int collide_world_segment_polygon(vector& p1,vector& p2,polygon* p,int& splits,vector& loc)
{
  vertex** vlist = p->get_vertex_list();
  vector &normal = p->get_normal()->world;
  vector on_plane;
  fixed d1,d2;

  d1 = dot(p1-vlist[0]->world,normal);
  d2 = dot(p2-vlist[0]->world,normal);

  if (sgn(d1) == sgn(d2))
  {
    // both on same side of poly plane
    splits = 0;
    return 0;
  }
  else
    splits = 1;

  // The sign issues are correct here.
  float denom = d1 - d2;
  on_plane = d1/denom*p2 - d2/denom*p1;
  loc = on_plane;

  for (int i=p->num_sides(); i; --i,++vlist)
  {
    if (dot(on_plane-vlist[0]->world,cross(normal,vlist[1]->world-vlist[0]->world)) < 0)
      return 0;
  }
  return 1;
}

// Super-accurate version of collide_world_segment_polygon().
// Accuracy is achieved by subdividing into triangles so that collision surcface
// is always planar.
int collide_world_segment_polygon_no_holes(vector& p1,vector& p2,polygon* p,int& splits,vector& loc)
{
  vertex** vlist = p->get_vertex_list();
  vector normal = p->get_normal()->world;
  vector on_plane;
  float d1,d2,denom;
  int ns = p->num_sides();
  int k;

  for (k=2; k<ns+1; k+=2)
  {
    vector& zero = vlist[k-2]->world;
    if (k > 2)
    {
      // every triangle after the first (which has same normal as polygon)
      // needs NEW normal computed
      normal = cross(vlist[k-1]->world-zero,vlist[k]->world-zero);
      normal.normalize();
    }
    d1 = dot(p1-zero,normal);
    d2 = dot(p2-zero,normal);
    if (sgn(d1) == sgn(d2))
    {
      // both on same side of poly plane
      splits = 0;
      if (__fabs(d1)<1 || __fabs(d2)<1)
        continue;  // possibly another triangle would get a different result
      else
        return 0;
    }
    else
      splits = 1;
    // compute intersection with plane
    denom = d1 - d2;
    loc = on_plane = d1/denom*p2 - d2/denom*p1;
    // allow margin of error so we get no holes
    if (dot(on_plane-zero,cross(normal,vlist[k-1]->world-zero)) < -1 ||
        dot(on_plane-vlist[k-1]->world,cross(normal,vlist[k]->world-vlist[k-1]->world)) < -1 ||
        dot(on_plane-vlist[k]->world,cross(normal,zero-vlist[k]->world)) < -1)
      continue;
    return 1;
  }
  return 0;
}


int collide_world_poly_poly(polygon * p1,polygon * p2,vector & loc)
{
  int i,j,hit,nums_verts[2],splits,try_count;
  vertex * * vlists[2];
  polygon * polys[2];

  polys[0] = p1;
  polys[1] = p2;
  vlists[0] = p1->get_vertex_list();
  vlists[1] = p2->get_vertex_list();
  nums_verts[0] = p1->num_sides();
  nums_verts[1] = p2->num_sides();
  hit = 0;
  try_count = 1;
  for (i=0;i<try_count && !hit;++i)
    for (j=0;j<nums_verts[i] && !hit;++j)
    {
      if (collide_world_segment_polygon(vlists[i][j+1]->world,vlists[i][j]->world,polys[1-i],splits,loc))
        hit = true;
      if (splits) try_count = 2;
    }
  return hit;
}

int collide_world_poly_poly_list(polygon * p1,polygon * plist,int pcount,int mark_hits,vector & loc,vector dir)
{
  int i,j,k,hit,nums_verts[2],splits,try_count;
  int no_dir;
  vertex * * vlists[2];
  polygon * polys[2];

  no_dir = (dir.x==0 && dir.y == 0 && dir.z == 0)?1:0;
  polys[1] = p1;
  vlists[1] = p1->get_vertex_list();
  nums_verts[1] = p1->num_sides();

  hit = 0;get_entit
  for (k=0;k<pcount && !hit;++k)
    {
    polys[0] = &(plist[k]);
    if (no_dir || dot(polys[0]->get_normal()->world,dir)<0)
      {
      vlists[0] = polys[0]->get_vertex_list();
      nums_verts[0] = polys[0]->num_sides();
      hit = 0;
      try_count = 1;
      for (i=0;i<try_count && !hit;++i)
        // if not marking hits on polygons, we can bail as soon as we score a hit
        for (j=0;j<nums_verts[i] && (!hit || !mark_hits);++j)
          {
          if (collide_world_segment_polygon(vlists[i][j+1]->world,vlists[i][j]->world,polys[1-i],splits,loc))
            hit = true;
          if (splits) try_count = 2;
          }
      if (mark_hits && hit) polys[0]->set_hit(1);
      }
    }
  return hit;
}


// Optimization: If the cube is "big" relative to the polygon,
// then a collision will (almost always) mean that a vertex of
// the polygon is inside the cube.
vector center_offset_kludge;
int collide_world_polygon_big_cube(polygon * p,rigid_view * box,fixed radius)
{
  vector points[MAX_VERTICES+1];
  int num_verts = p->get_base_num_verts();
  vertex * * vlist = p->get_vertex_list();
  vector box_pos = box->get_abs_position()+center_offset_kludge;
  so3 box_unrot = *(box->get_rot());
  box_unrot.invert();
  for (int i=0;i<=num_verts;++i)
  {
    points[i] = vlist[i]->world-box_pos;
    box_unrot.xform(points[i],points[i]);
  }
  fixed r2 = radius*radius;
  for (i=0;i<num_verts;++i)
  {
    fixed difx2 = points[i].x;
    difx2 *= difx2;
    fixed dify2 = points[i].y;
    dify2 *= dify2;
    fixed difz2 = points[i].z;
    difz2 *= difz2;
    if (difx2<r2 && dify2<r2 && difz2<r2) break;
  }
  return (i!=num_verts);
}


int collide_world_plane_sphere(vector p,vector n,vector ctr,float radius)
{ // assume normal n is unit length
  float d;
  d=__fabs(dot(n,ctr-p));
  if(d<radius)
    return 1;
  else
    return 0;
}

#endif
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//                             END THE OLD                            //
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
*/
