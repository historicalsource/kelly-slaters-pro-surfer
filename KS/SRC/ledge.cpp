#include "global.h"

//!#include "ledge.h"
#include "faceflags.h"
#include "terrain.h"


ledge::ledge( const int _type, const vector3d& _0, const vector3d& _1, const vector3d& _2, const region *r )
{
#define MAX_VERT_DELTA_Y    .01f
#define MAX_SQUARE_RATIO    .0001f
  vector3d   N;

  if( (_type & TERFACE_LEDGE ) && (_type & TERFACE_CRAWLSPACELEDGE) ) type = 3;
  else if( _type & TERFACE_LEDGE ) type = 1;
  else if( _type & TERFACE_CRAWLSPACELEDGE ) type = 2;
  else //assert( false );
    warning( stringx("Illegal ledge type in ") + r->get_name() );
//  if( __fabs(_0.y - _1.y) < MAX_VERT_DELTA_Y )
  if( ((_0.y - _1.y) * (_0.y - _1.y)) / (_0 - _1).length2() < MAX_SQUARE_RATIO )
  {
    l_x = _0.x;
    l_z = _0.z;
    r_x = _1.x;
    r_z = _1.z;
    y = (_0.y > _1.y)? _0.y : _1.y;
  }
//  else if( __fabs(_0.y - _2.y) < MAX_VERT_DELTA_Y )
  else if( ((_0.y - _2.y) * (_0.y - _2.y)) / (_0 - _2).length2() < MAX_SQUARE_RATIO )
  {
    l_x = _2.x;
    l_z = _2.z;
    r_x = _0.x;
    r_z = _0.z;
    y = (_2.y > _0.y)? _2.y : _0.y;
  }
  else
  {
//    assert( __fabs( _1.y - _2.y) < MAX_VERT_DELTA_Y );
//    if( __fabs( _1.y - _2.y) > MAX_VERT_DELTA_Y ) warning( stringx("Non-horizontal ledge in ") + r->get_name() );
    if( ((_1.y - _2.y) * (_1.y - _2.y)) / (_1 - _2).length2() > MAX_SQUARE_RATIO ) 
      warning( stringx("Non-horizontal ledge in ") + r->get_name() );
    l_x = _1.x;
    l_z = _1.z;
    r_x = _2.x;
    r_z = _2.z;
    y = (_1.y > _2.y)? _1.y : _2.y;
  }
  N = cross( _1 - _0, _2 - _0 );
  if( __fabs(N.y) > MAX_VERT_DELTA_Y )
    warning( stringx("Non-vertical poly marked as a ledge in ") + r->get_name() );
  else
  {
    N.y = .0f;
    N.normalize();
    n_x = N.x;
    n_z = N.z;
//    assert( (r_z - l_z) * n_x - (r_x - l_x) * n_z > .0f );
    if( (r_z - l_z) * n_x - (r_x - l_x) * n_z < .0f )
      warning( stringx("Illegal ledge in ") + r->get_name() );
  }
#undef MAX_VERT_DELTA_Y
}


inline bool equ( const rational_t x, const rational_t y )
{
  if( __fabs(x - y) < 0.001f ) return true;
  else return false;
}


bool merge_ledge( ledge *l1, const ledge *l2 )
{
#define ALMOST_PARALLEL_DOT_VALUE   .999848f  // cos( 1 deg. )
  if( l1->type == l2->type && equ(l1->y, l2->y) && equ(l1->n_x, l2->n_x) && equ(l1->n_z, l2->n_z) )
  {
    vector2d  vl1( l1->l_x - l1->r_x, l1->l_z - l1->r_z ),
              vl2( l2->l_x - l2->r_x, l2->l_z - l2->r_z );
    if( dot(vl1, vl2) >= ALMOST_PARALLEL_DOT_VALUE )
    {
      if( equ(l1->l_x, l2->r_x) && equ(l1->l_z, l2->r_z) )        // merge to the left
      {
        l1->l_x = l2->l_x;
        l1->l_z = l2->l_z;
        return true;
      }
      else if( equ(l1->r_x, l2->l_x) && equ(l1->r_z, l2->l_z) )   // merge to the right
      {
        l1->r_x = l2->r_x;
        l1->r_z = l2->r_z;
        return true;
      }
    }
  }
  return false;
}


/*!void share_portal_ledges()
{
  for( portal_list::const_iterator pi = g_world_ptr->get_the_terrain().get_portals().begin(), pe = g_world_ptr->get_the_terrain().get_portals().end(); pi != pe; ++pi )
  {
    vr_pmesh* my_vr_pmesh = (vr_pmesh*)( *pi );
    for( int i = 0; i < my_vr_pmesh->get_max_faces(); ++i )
    {
      int   ledge_type, face_flags = my_vr_pmesh->reduced_faces[i].flags;

      if( face_flags & (TERFACE_LEDGE | TERFACE_CRAWLSPACELEDGE) )
      {
        if( (face_flags & TERFACE_LEDGE ) && (face_flags & TERFACE_CRAWLSPACELEDGE) ) ledge_type = 3;
        else if( face_flags & TERFACE_LEDGE ) ledge_type = 1;
        else if( face_flags & TERFACE_CRAWLSPACELEDGE ) ledge_type = 2;
        else warning( stringx("Illegal ledge type in portal") );

        region  *r_front = (*pi)->get_front()->get_data(),
                *r_back =  (*pi)->get_back()->get_data();
        ledge   *try_ledge_front = r_front->find_ledge( ledge_type,
                                                        my_vr_pmesh->get_xvert_unxform( my_vr_pmesh->get_wedge_ref(i, 0) ),
                                                        my_vr_pmesh->get_xvert_unxform( my_vr_pmesh->get_wedge_ref(i, 1) ),
                                                        my_vr_pmesh->get_xvert_unxform( my_vr_pmesh->get_wedge_ref(i, 2) )
                                                      ),
                *try_ledge_back = r_back->find_ledge  ( ledge_type,
                                                        my_vr_pmesh->get_xvert_unxform( my_vr_pmesh->get_wedge_ref(i, 0) ),
                                                        my_vr_pmesh->get_xvert_unxform( my_vr_pmesh->get_wedge_ref(i, 1) ),
                                                        my_vr_pmesh->get_xvert_unxform( my_vr_pmesh->get_wedge_ref(i, 2) )
                                                      );
        if( try_ledge_front != NULL && try_ledge_back == NULL )
          r_back->get_ledges().push_back( try_ledge_front );
        else if( try_ledge_front == NULL && try_ledge_back != NULL )
          r_front->get_ledges().push_back( try_ledge_back );
      }
    }
  }
}

!*/
