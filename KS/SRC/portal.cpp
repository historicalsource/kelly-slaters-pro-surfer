// portal.cpp
// Copyright (C) 2000 Treyarch LLC    ALL RIGHTS RESERVED

#include "global.h"

#include "portal.h"
#include "plane.h"
#include "region.h"
#include "bound.h"


portal::portal()
  : vr_pmesh(vr_pmesh::NORMAL),
    inactive(false)
{
}

portal::portal( region_node* _front, region_node* _back )
  : vr_pmesh(vr_pmesh::NORMAL),
    front( _front ),
    back( _back ),
    inactive( false )
{
}

void portal::compute_info()
{
  // recursion guard because make_rectangle calls compute_info
  static bool recursing = false;
  if (recursing)
    return;
  recursing = true;

  cylinder_depth = 0;
  normal = ZEROVEC;
  if ( verts->empty() )
  {
    bound.set_center(ZEROVEC);
    bound.set_radius(1.0f);
  }
  else
  {
    // compute portal effective center (average of all verts is good enough)
    int i;
    vector3d effective_center=ZEROVEC;
    for ( i=verts->size(); --i>=0; )
      effective_center += (*verts)[i].get_point();
    effective_center *= 1.0f / verts->size();
    bound.set_center(effective_center);

    // compute normal (average of face normals)
    for ( i=get_max_faces(); --i>=0; )
    {
      normal += compute_face_normal(i); // weighted by twice face area
    }
    normal.normalize();

    // compute portal effective radius and cylinder depth (max distance of any point from effective center along cylinder axis)
    rational_t effective_radius2 = 0;
    if (verts->size() > 16)
      warning("portal [" + get_back()->get_data()->get_name() + "->" + get_front()->get_data()->get_name() + "] is too complex"); // I should change the mesh to a simple bounding box in this case
    for ( i=verts->size(); --i>=0; )
    {
      vector3d v = (*verts)[i].get_point() - effective_center;
      rational_t d = __fabs( dot( v, normal ) );
      cylinder_depth = max( cylinder_depth, d );
      effective_radius2 = max( effective_radius2, v.length2() );
    }
    bound.set_radius(__fsqrt(effective_radius2));

    nonplanarfudgefactor = cylinder_depth/bound.get_radius();
    if ( nonplanarfudgefactor > 0.1f )
      warning( "portal [" + get_back()->get_data()->get_name() + "->" + get_front()->get_data()->get_name() + "] is not sufficiently planar" );
    nonplanarfudgefactor *= -5.0f; // to range -0.5 to 0.0
    nonplanarfudgefactor -= 0.0001f;

    // convert portal to a quad if it is more complex than a quad
    //if (verts->size() > 4) // always do this because original quad verts may not have been in counterclockwise order
    {
      // accumulate cylinder-space bound rect using cylinder front vector as the normal
      // and the world up vector
      bounding_box bb;
      vector3d up = vector3d(0,1,0);
      vector3d right = cross(up,normal);

      if(right.length2() <= 1e-8)
      {
        up = ZVEC;
        right = cross(up,normal);
      }

      matrix4x4 c2w(right.x,right.y,right.z,0,
                    up.x,up.y,up.z,0,
                    normal.x,normal.y,normal.z,0,
                    bound.get_center().x,bound.get_center().y,bound.get_center().z,1);
      c2w.orthonormalize();
      matrix4x4 w2c=c2w.inverse();
      for ( i=verts->size(); --i>=0; )
      {
        vector3d v = xform3d_1(w2c, (*verts)[i].get_point());
        bb.accumulate(v);
      }
      make_rectangle();
      float npff = 1.0f - nonplanarfudgefactor*0.5f;
      (*verts)[0] = xverts[0].xyz = xform3d_1(c2w, vector3d(bb.vmin.x,bb.vmin.y,0)*npff);
      (*verts)[1] = xverts[1].xyz = xform3d_1(c2w, vector3d(bb.vmin.x,bb.vmax.y,0)*npff);
      (*verts)[2] = xverts[2].xyz = xform3d_1(c2w, vector3d(bb.vmax.x,bb.vmax.y,0)*npff);
      (*verts)[3] = xverts[3].xyz = xform3d_1(c2w, vector3d(bb.vmax.x,bb.vmin.y,0)*npff);
    }
  }
  vr_pmesh::compute_info();
  recursing = false;
}

bool portal::touches_sphere(const sphere& s) const
{
  if (!bound.intersects(s))
    return false;
  plane p(bound.get_center(),normal);
  if (__fabs(p.distance_above(s.get_center())) >= s.get_radius()+cylinder_depth)
  {
    // This check is kinda expensive and only matters for portals that
    // are very non-planar.  Fuck 'em.  --Sean
    /*
    int i;
    for (i=verts->size(); --i>=0; )
    {
      vector3d v = (*verts)[i].get_point();
      if (s.contains(v))
        return true;
    }
    */
    return false;
  }
  return true; // close enough
  /*
  if (bound.get_radius()<s.get_radius())
    return false; // no point in continuing.
  // check faces for intersection
  for (i=get_max_faces(); --i>=0; )
  {
    const vector3d& v0 = get_xvert_unxform( get_wedge_ref( i, 0) );
    const vector3d& v1 = get_xvert_unxform( get_wedge_ref( i, 1) );
    const vector3d& v2 = get_xvert_unxform( get_wedge_ref( i, 2) );
    plane p(v0,v1,v2); // involves a dot and a sqrt
    if (__fabs(p.distance_above(bound.get_center())) < bound.get_radius())
    {
      // see if sphere center is within poly edges + radius
      return true;
    }
  }
  */
}


bool portal::touches_segment( const vector3d& p1, const vector3d& p2 ) const
{
  vector3d v1 = p1 - get_effective_center();
  vector3d v2 = p2 - get_effective_center();
  rational_t d1 = dot( v1, get_cylinder_normal() );
  rational_t d2 = dot( v2, get_cylinder_normal() );
  // both points on same side of plane?
  if ( sgn(d1) == sgn(d2) )
    return false;
  // check against disc
  v2 = (v2 - v1) * (__fabs(d1) / (__fabs(d1)+__fabs(d2))) + v1;
  return ( v2.length2() < get_effective_radius()*get_effective_radius() );
}


inline void CLIP_VERT_IDX(vector3d& target_v,
                          const vector3d& source_v, const vector3d& clip_to_v,
                          float how_far)
{
  assert(how_far>=-0.0001F && how_far<=1.0001F);
  target_v = (clip_to_v - source_v) * how_far + source_v;
}

inline void CLIP_VERT_PLANE(vector3d& target_v,
                            const vector3d& source_v, const vector3d& clip_to_v,
                            float sdist_source, float sdist_clip_to)
{
  CLIP_VERT_IDX(target_v,source_v,clip_to_v, sdist_source / (sdist_source - sdist_clip_to));
}


static void ClipTriToPlaneDists(vector3d const* verts[4], // pointers to the verts, in order (3 in / 3 or 4 out)
                         vector3d buf[2],    // extra space that may be needed for the NEW vertices
                         const float clipdists[3])  // pre-computed signed distances to the plane for the 3 input verts
{
  static const int mod3table[5] = { 0,1,2,0,1 };

  // pass in 3 verts and a buffer, and it will either fill the buffer
  // (if needed) or change the pointer to NULL
  verts[3]=NULL;
  int nclipped=0;
  int clipped=-1;
  int notclipped=-1;
  for (int i=3; --i>=0; )
  {
    if (clipdists[i]<0.0F)
    {
      clipped=i;
      ++nclipped;
    }
    else
    {
      notclipped=i;
    }
  }
  switch (nclipped)
  {
    case 0:
      break;
    case 1:
    {
      int main=clipped;
      int prev=mod3table[main+2];
      int next=mod3table[main+1];
      CLIP_VERT_PLANE(buf[0], *(verts[prev]), *(verts[main]),
                      clipdists[prev], clipdists[main]);
      CLIP_VERT_PLANE(buf[1], *(verts[next]), *(verts[main]),
                      clipdists[next], clipdists[main]);
      for (int i=3; i>main+1; i=i-1)
        verts[i]=verts[i-1];
      verts[main  ]=&buf[0];
      verts[main+1]=&buf[1];
      break;
    }
    case 2:
    {
      int main=notclipped;
      int prev=mod3table[main+2];
      int next=mod3table[main+1];
      CLIP_VERT_PLANE(buf[0], *(verts[main]), *(verts[prev]),
                      clipdists[main], clipdists[prev]);
      CLIP_VERT_PLANE(buf[1], *(verts[main]), *(verts[next]),
                      clipdists[main], clipdists[next]);
      verts[prev]=&buf[0];
      verts[next]=&buf[1];
      break;
    }
    case 3:
      verts[0]=verts[1]=verts[2]=NULL;
      break;
    default:
      assert(false);
  }
}

void ClipTriToPlane(const plane& clipto,       // the plane to clip to, in same coordsys as verts
                    vector3d const* verts[4],  // pointers to the verts, in order (3 in / 3 or 4 out)
                    vector3d buf[2])           // extra space that may be needed for the NEW vertices
{
  float sdists[3];
  for (int i=0; i<3; ++i)
    sdists[i] = clipto.distance_above(*(verts[i]));
  ClipTriToPlaneDists(verts,buf,sdists);
}


// assumes quad is convex and planar
static void ClipQuadToPlaneDists(vector3d const* verts[5], // pointers to the verts, in order (4 in / 3,4 or 5 out)
                         vector3d buf[2],           // extra space that may be needed for the NEW vertices
                         const float clipdists[4])  // pre-computed signed distances to the plane for the 4 input verts
{
  // pass in 4 verts and a buffer, and it will either fill the buffer
  // (if needed) or change the pointer to NULL
  verts[4]=NULL;
  int nclipped=0;
  int clipped=-1;
  int notclipped=-1;
  for (int i=4; --i>=0; )
  {
    if (clipdists[i]<0.0F)
    {
      clipped=i;
      ++nclipped;
    }
    else
    {
      notclipped=i;
    }
  }
  switch (nclipped)
  {
    case 0: // no clipping needed, result is a quad
      break;
    case 1: // 1 vert clipped, result is a 5-sided poly
    {
      int main=clipped;
      int prev=(main+3)&3;
      int next=(main+1)&3;
      CLIP_VERT_PLANE(buf[0], *(verts[prev]), *(verts[main]),
                      clipdists[prev], clipdists[main]);
      CLIP_VERT_PLANE(buf[1], *(verts[next]), *(verts[main]),
                      clipdists[next], clipdists[main]);
      for (int i=4; i>main+1; i=i-1)
        verts[i]=verts[i-1];
      verts[main  ]=&buf[0];
      verts[main+1]=&buf[1];
      break;
    }
    case 2: // result is another quad
    {
      int mainp=notclipped;
      int mainn=notclipped;
      int next=(mainn+1)&3;
      if (clipdists[next]<0.0f)
        mainp=(mainp+3)&3;
      else
      {
        mainn=next;
        next=(next+1)&3;
      }
      int prev=(mainp+3)&3;
      CLIP_VERT_PLANE(buf[0], *(verts[mainp]), *(verts[prev]),
                      clipdists[mainp], clipdists[prev]);
      CLIP_VERT_PLANE(buf[1], *(verts[mainn]), *(verts[next]),
                      clipdists[mainn], clipdists[next]);
      verts[prev]=&buf[0];
      verts[next]=&buf[1];
      break;
    }
    case 3:  // result is a tri
    {
      int main=notclipped;
      int prev=(main+3)&3;
      int next=(main+1)&3;
      CLIP_VERT_PLANE(buf[0], *(verts[main]), *(verts[prev]),
                      clipdists[main], clipdists[prev]);
      CLIP_VERT_PLANE(buf[1], *(verts[main]), *(verts[next]),
                      clipdists[main], clipdists[next]);
      verts[prev]=&buf[0];
      verts[next]=&buf[1];
      switch (main)
      {
        case 0:
          verts[2]=verts[3];
          break;
        case 1:
          break;
        case 2:
          verts[0]=verts[3];
          break;
        case 3:
          verts[1]=verts[2];
          verts[2]=verts[3];
          break;
        default:
          assert(false);
      }
      verts[3]=NULL;
      break;
    }
    case 4: // whole quad clipped
      verts[0]=verts[1]=verts[2]=verts[3]=NULL;
      break;
    default:
      assert(false);
  }
}

void ClipQuadToPlane(const plane& clipto,     // the plane to clip to, in same coordsys as verts
                    vector3d const* verts[5], // pointers to the verts, in order (4 in / 3, 4, or 5 out)
                    vector3d buf[2])          // extra space that may be needed for the NEW vertices
{
  float sdists[4];
  for (int i=0; i<4; ++i)
    sdists[i] = clipto.distance_above(*(verts[i]));
  ClipQuadToPlaneDists(verts,buf,sdists);
}
