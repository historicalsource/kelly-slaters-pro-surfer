// po_anim.cpp

#include "global.h"

#include "po_anim.h"
#include "linear_anim.h"
#include "oserrmsg.h"
#include "profiler.h"

inline vector3d anim_xform3d_1(const vector3d *rot, const vector3d &sv)
{
#if defined(TARGET_XBOX)
  assert( rot->is_valid() );
  assert( sv.is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */

  vector3d res;
  res.x = rot[0].x * sv.x + rot[1].x * sv.y + rot[2].x * sv.z;
  res.y = rot[0].y * sv.x + rot[1].y * sv.y + rot[2].y * sv.z;
  res.z = rot[0].z * sv.x + rot[1].z * sv.y + rot[2].z * sv.z;

#if defined(TARGET_XBOX)
  assert( res.is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */

  return(res);
}

inline void anim_create_rot_vectors(vector3d *mat, const quaternion &q)
{
  matrix4x4 rot_mat;
  q.to_matrix( &rot_mat );

  mat[0].x = rot_mat.x.x;
  mat[0].y = rot_mat.x.y;
  mat[0].z = rot_mat.x.z;
  mat[1].x = rot_mat.y.x;
  mat[1].y = rot_mat.y.y;
  mat[1].z = rot_mat.y.z;
  mat[2].x = rot_mat.z.x;
  mat[2].y = rot_mat.z.y;
  mat[2].z = rot_mat.z.z;

#if defined(TARGET_XBOX)
  assert( mat->is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */
}

PRS_track::PRS_track( linear_track<vector3d>* __P,
                      linear_track<quaternion>* __R,
                      linear_track<rational_t>* __S,
                      unsigned short __flags )
  :
    flags( __flags ),
    P( __P ),
    R( __R ),
    S( __S )
{
  _compute_duration();
}

#ifdef TARGET_GC

void PRS_track::endian_fixup_hack( void )
{
	fixup((unsigned char *)&(duration),sizeof(time_value_t));
	fixupuint(&(flags));
}

void PRS_track::endian_fixup_hack_P( void )
{
	assert(P);
	if (P) P->endian_fixup_hack();
}

void PRS_track::endian_fixup_hack_R( void )
{
	assert(R);
	if (R) R->endian_fixup_hack();
}

void PRS_track::endian_fixup_hack_S( void )
{
	assert(S);
	if (S) S->endian_fixup_hack();
}


#endif

void PRS_track::_compute_duration()
{
  duration = 0;
  if ( P!=NULL && P->get_duration()>duration )
    duration = P->get_duration();
  if ( R!=NULL && R->get_duration()>duration )
    duration = R->get_duration();
  if ( S!=NULL && S->get_duration()>duration )
    duration = S->get_duration();
}


#if !defined(NO_SERIAL_IN)
void PRS_track::internal_serial_in( chunk_file& fs )
{
  chunk_flavor cf;
  anim_track_flavor_t flavor;
  for ( serial_in(fs,&cf); cf!=CHUNK_END; serial_in(fs,&cf) )
  {
    if ( cf == chunk_flavor("P") )
    {
      // position track
      serial_in( fs, &flavor );
      switch ( flavor )
      {
        case LIN_VEC3_TRACK:
          P = NEW linear_track<vector3d>;
          break;
        default:
          error( fs.get_name() + ": bad position track flavor in PRS_track" );
      }
      P->internal_serial_in( fs );
    }
    else if ( cf == chunk_flavor("R") )
    {
      // rotation track
      serial_in( fs, &flavor );
      switch ( flavor )
      {
        case LIN_QUAT_TRACK:
          R = NEW linear_track<quaternion>;
          break;
        default:
          error( fs.get_name() + ": bad rotation track flavor in PRS_track" );
      }
      R->internal_serial_in( fs );
    }
    else if ( cf == chunk_flavor("S") )
    {
      // scale track
      serial_in( fs, &flavor );
      switch ( flavor )
      {
        case LIN_FLOAT_TRACK:
          S = NEW linear_track<rational_t>;
          break;
        default:
          error( fs.get_name() + ": bad scale track flavor in PRS_track" );
      }
      S->internal_serial_in( fs );
    }
    else if ( cf == chunk_flavor("flags") )
    {
      // PRS track flags
      serial_in( fs, &flags );
    }
    else
      error( fs.get_name() + ": unknown chunk found in PRS_track" );
  }
  _compute_duration();
}
#endif

PRS_track::~PRS_track()
{
  // PEH BETA (3 lines following)
  if(P) delete P;
  if(R) delete R;
  if(S) delete S;
}

/*
#if !defined(NO_SERIAL_OUT)
void PRS_track::internal_serial_out( chunk_file& fs ) const
{
  if ( P != NULL )
  {
    // position track
    serial_out( fs, chunk_flavor("P") );
    serial_out( fs, P->get_flavor() );
    P->internal_serial_out( fs );
  }
  if ( R != NULL )
  {
    // rotation track
    serial_out( fs, chunk_flavor("R") );
    serial_out( fs, R->get_flavor() );
    R->internal_serial_out( fs );
  }
  if ( S != NULL )
  {
    // scale track
    serial_out( fs, chunk_flavor("S") );
    serial_out( fs, S->get_flavor() );
    S->internal_serial_out( fs );
  }
  if ( flags )
  {
    serial_out( fs, chunk_flavor("flags") );
    serial_out( fs, flags );
  }
  // end of PRS_track
  serial_out( fs, CHUNK_END );
}
#endif
*/

//const int MAX_CONCURRENT_PO_ANIMS=384;
const int MAX_CONCURRENT_PO_ANIMS=1000;

STATICALLOCCLASSINSTANCE(po_anim,MAX_CONCURRENT_PO_ANIMS);
STATICALLOCCLASSINSTANCE(linear_anim<quaternion>,MAX_CONCURRENT_PO_ANIMS);
STATICALLOCCLASSINSTANCE(linear_anim<vector3d>,MAX_CONCURRENT_PO_ANIMS);
STATICALLOCCLASSINSTANCE(linear_anim<rational_t>,MAX_CONCURRENT_PO_ANIMS);


po_anim::po_anim()
: anim<po>(),
  P( NULL ),
  R( NULL ),
  S( NULL )
{
  set_valid( false );
}


po_anim::~po_anim()
{
#ifndef USINGSTATICSTLALLOCATIONS
	if (P) delete(P);
	if (R) delete(R);
	if (S) delete(S);
#else
	if (P) linear_anim<vector3d>::operator delete(P);
	if (R) linear_anim<quaternion>::operator delete(R);
	if (S) linear_anim<rational_t>::operator delete(S);
#endif
}


void po_anim::construct( const po& initial,
                         const PRS_track& track,
                         unsigned short anim_flags )
{
  anim<po>::construct( anim_flags );

  flags |= (track.get_flags() & FLAGS_INHERITED_FROM_TRACK);

  unsigned short subanim_flags = flags & FLAGS_PASSED_TO_SUBANIMS;

  P_start = initial.get_position();
  R_start = initial.get_quaternion();
  S_start = initial.get_scale();

  if ( track.get_P() != NULL )
  {
    // position track found
    if ( P == NULL )
      P = NEW linear_anim<vector3d>;
    linear_track<vector3d>* Pt = (linear_track<vector3d>*)track.get_P();
    ((linear_anim<vector3d>*)P)->construct( Pt, subanim_flags );
  }
  else if ( has_P() )
    P->set_valid( false );

  if ( track.get_R() != NULL )
  {
    if ( R == NULL )
      R = NEW linear_anim<quaternion>;
    linear_track<quaternion>* Rt = (linear_track<quaternion>*)track.get_R();
    ((linear_anim<quaternion>*)R)->construct( Rt,
                                              subanim_flags );
  }
  else if ( has_R() )
    R->set_valid( false );

  if ( track.get_S() != NULL )
  {
    if ( S == NULL )
      S = NEW linear_anim<rational_t>;
    linear_track<rational_t>* St = (linear_track<rational_t>*)track.get_S();
    ((linear_anim<rational_t>*)S)->construct( St,
                                              subanim_flags );
  }
  else if ( has_S() )
    S->set_valid( false );

  // in reverse start-relative anim, start values must be adjusted to correspond to bottom end of track
  if ( is_relative_to_start() && (anim_flags&ANIM_REVERSE) )
  {
    if ( has_S() )
    {
      rational_t s;
      S->get_value( track.get_S()->get_duration(), &s );
      S_start /= s;
    }
    if ( has_R() )
    {
      quaternion r;
      R->get_value( track.get_R()->get_duration(), &r );
      R_start = r.get_conjugate() * R_start;
    }

    anim_create_rot_vectors(&R_start_mat[0], R_start);

    if ( has_P() )
    {
      // the rel-position must be transformed by R_start and then subtracted from P_start
      vector3d p;
      P->get_value( track.get_P()->get_duration(), &p );
//      matrix4x4 rot;
//      R_start.to_matrix( &rot );
      P_start -= anim_xform3d_1( &R_start_mat[0], p );
    }
  }
  else
  {
    anim_create_rot_vectors(&R_start_mat[0], R_start);
  }
}


// force current_time to given value
void po_anim::set_time( time_value_t t )
{
  if ( has_P() )
    P->set_time( t );
  if ( has_R() )
    R->set_time( t );
  if ( has_S() )
    S->set_time( t );
}


void po_anim::frame_advance( const anim_control_t& ac, po* dest )
{
  if ( ac.was_looped() && is_relative_to_start() )
  {
    // the animation looped this frame and is start-relative,
    // so we need to reset the start value(s)
    reset_start_for_loop( ac );
  }

  // position
  vector3d p;
  if ( has_P() )
  {
    P->frame_advance( ac, &p );


#if defined(TARGET_XBOX)
    assert(p.is_valid());
#endif /* TARGET_XBOX JIV DEBUG */

    if ( is_relative_to_start() )
    {
      // when animation is start-relative, the rel-position must be transformed by R_start
      // and then added to P_start
//      matrix4x4 q_to_matrix;
//      R_start.to_matrix( &q_to_matrix );
      p = anim_xform3d_1(&R_start_mat[0],p) + P_start;

#if defined(TARGET_XBOX)
      assert(p.is_valid());
#endif /* TARGET_XBOX JIV DEBUG */
    }

  }
  else
    p = P_start;

#if defined(TARGET_XBOX)
  assert(p.is_valid());
#endif /* TARGET_XBOX JIV DEBUG */

  // rotation
  quaternion r;
  if ( has_R() )
  {
    R->frame_advance( ac, &r );

    if ( is_relative_to_start() )
		r = r * R_start;
  }
  else
    r = R_start;

  // scale
  rational_t s;
  if ( has_S() )
  {
    S->frame_advance( ac, &s );

    if ( is_relative_to_start() )
      s *= S_start;
  }
  else
    s = S_start;

  // return value
  *dest = po( p, r, s );
}

void po_anim::frame_advance( const anim_control_t& ac, vector3d &p, quaternion &r, rational_t &s )
{
  if ( ac.was_looped() && is_relative_to_start() )
  {
    // the animation looped this frame and is start-relative,
    // so we need to reset the start value(s)
    reset_start_for_loop( ac );
  }

  // position
  if ( has_P() )
  {
    P->frame_advance( ac, &p );

    if ( is_relative_to_start() )
    {
      // when animation is start-relative, the rel-position must be transformed by R_start
      // and then added to P_start
//      matrix4x4 q_to_matrix;
//      R_start.to_matrix( &q_to_matrix );
      p = anim_xform3d_1(&R_start_mat[0],p) + P_start;
    }
  }
  else
    p = P_start;

  // rotation
  if ( has_R() )
  {
    R->frame_advance( ac, &r );

    if ( is_relative_to_start() )
      r = r * R_start;
  }
  else
    r = R_start;

  // scale
  if ( has_S() )
  {
    S->frame_advance( ac, &s );

    if ( is_relative_to_start() )
      s *= S_start;
  }
  else
    s = S_start;
}


void po_anim::get_value( const anim_control_t& ac, po* dest ) const
{
  vector3d p;
  if ( has_P() )
  {
    P->get_value( ac, &p );
    if ( is_relative_to_start() )
    {
      // when animation is start-relative, the rel-position must be transformed by R_start
      // and then added to P_start
//      matrix4x4 q_to_matrix;
//      R_start.to_matrix( &q_to_matrix );
      p = anim_xform3d_1(&R_start_mat[0], p) + P_start;
    }
  }
  else
    p = P_start;
  quaternion r;
  if ( has_R() )
  {
    R->get_value( ac, &r );
    if ( is_relative_to_start() )
      r = r * R_start;
  }
  else
    r = R_start;
  rational_t s;
  if ( has_S() )
  {
    S->get_value( ac, &s );
    if ( is_relative_to_start() )
      s *= S_start;
  }
  else
    s = S_start;
  *dest = po( p, r, s );
}


void po_anim::get_value( time_value_t t, po* dest ) const
{
  vector3d p;
  if ( has_P() )
  {
    P->get_value( t, &p );
    if ( is_relative_to_start() )
    {
      // when animation is start-relative, the rel-position must be transformed by R_start
      // and then added to P_start
//      matrix4x4 q_to_matrix;
//      R_start.to_matrix( &q_to_matrix );
      p = anim_xform3d_1(&R_start_mat[0], p) + P_start;
    }
  }
  else
    p = P_start;
  quaternion r;
  if ( has_R() )
  {
    R->get_value( t, &r );
    if ( is_relative_to_start() )
      r = r * R_start;
  }
  else
    r = R_start;
  rational_t s;
  if ( has_S() )
  {
    S->get_value( t, &s );
    if ( is_relative_to_start() )
      s *= S_start;
  }
  else
    s = S_start;
  *dest = po( p, r, s );
}


// this virtual function allows animations to respond properly to a loop
// condition by resetting the start value (needed only in start-relative
// tracks)
void po_anim::reset_start_for_loop( const anim_control_t& ac )
{
  assert( is_relative_to_start() );
  if ( ac.is_reverse() )
  {
    if ( has_S() )
    {
      rational_t s;
      S->get_value( ac.get_duration(), &s );
      S_start /= s;
    }
    if ( has_R() )
    {
      quaternion r;
      R->get_value( ac.get_duration(), &r );
      R_start *= r.get_conjugate() * R_start;
    }

    anim_create_rot_vectors(&R_start_mat[0], R_start);

    if ( has_P() )
    {
      vector3d p;
      P->get_value( ac.get_duration(), &p );
      // rel-position must be transformed by R_start and then subtracted from P_start
//      matrix4x4 rot;
//      R_start.to_matrix( &rot );
      P_start -= anim_xform3d_1( &R_start_mat[0], p );
    }
  }
  else
  {
    if ( has_S() )
    {
      rational_t s;
      S->get_value( ac.get_duration(), &s );
      S_start *= s;
    }

    if ( has_R() )
    {
      quaternion r;
      R->get_value( ac.get_duration(), &r );
      R_start = r * R_start;
    }

    anim_create_rot_vectors(&R_start_mat[0], R_start);

    if ( has_P() )
    {
      vector3d p;
      P->get_value( ac.get_duration(), &p );
      // rel-position must be transformed by R_start and then added to P_start
//      matrix4x4 rot;
//      R_start.to_matrix( &rot );
      P_start += anim_xform3d_1( &R_start_mat[0], p );
    }
  }
}


// this virtual function allows the start values to be adjusted based on the
// given destination state and the current state of the animation
void po_anim::reset_start( const anim_control_t& ac, const po& dest )
{
  assert( is_relative_to_start() );
  // extract the destination value
  rational_t Sd = dest.get_scale();
  if ( has_S() )
  {
    // based on the current anim state, adjust the start value such that the
    // output value (start * anim) is equal to the desired dest value
    rational_t s;
    S->get_value( ac, &s );
    S_start = Sd / s;
  }
  else
    S_start = Sd;
  // extract the destination value
  quaternion Rd = dest.get_quaternion();
  if ( has_R() )
  {
    // based on the current anim state, adjust the start value such that the
    // output value (start * anim) is equal to the desired dest value
    quaternion r;
    R->get_value( ac, &r );
    R_start = r.get_conjugate() * Rd;
  }
  else
    R_start = Rd;

  anim_create_rot_vectors(&R_start_mat[0], R_start);

  // extract the destination value
  vector3d Pd = dest.get_position();
  if ( has_P() )
  {
    // based on the current anim state, adjust the start value such that the
    // output value (start + anim) is equal to the desired dest value
    vector3d p;
    P->get_value( ac, &p );
    // p, being a rel-position, must first be transformed by R_start
//    matrix4x4 Rs;
//    R_start.to_matrix( &Rs );
    //p = anim_xform3d_1_homog(Rs, p);
    p = anim_xform3d_1(&R_start_mat[0], p);
    P_start = Pd - p;
  }
  else
    P_start = Pd;
}


// call this function to compute the unadjusted position value of the animation at the current time
void po_anim::get_unadjusted_value( const anim_control_t& ac, vector3d* destP ) const
{
  if ( has_P() )
    P->get_value( ac, destP );
}

// call this function to compute the unadjusted position value of the animation at an arbitrary time
void po_anim::get_unadjusted_value( time_value_t t, vector3d* destP ) const
{
  if ( has_P() )
    P->get_value( t, destP );
}

// call this function to compute the unadjusted quaternion value of the animation at the current time
void po_anim::get_unadjusted_value( const anim_control_t& ac, quaternion* destR ) const
{
  if ( has_R() )
    R->get_value( ac, destR );
}

// call this function to compute the unadjusted quaternion value of the animation at an arbitrary time
void po_anim::get_unadjusted_value( time_value_t t, quaternion* destR ) const
{
  if ( has_R() )
    R->get_value( t, destR );
}

// call this function to compute the unadjusted scale value of the animation at the current time
void po_anim::get_unadjusted_value( const anim_control_t& ac, rational_t* destS ) const
{
  if ( has_S() )
    S->get_value( ac, destS );
}

// call this function to compute the unadjusted scale value of the animation at an arbitrary time
void po_anim::get_unadjusted_value( time_value_t t, rational_t* destS ) const
{
  if ( has_S() )
    S->get_value( t, destS );
}

void po_anim::reset_start_vals(void)
{
	S_start = 1.0f;
	//R_start = quaternion(1.0f, 0.0f, 0.0f, 0.0f);
	flags = flags & (~ANIM_RELATIVE_TO_START);
}
