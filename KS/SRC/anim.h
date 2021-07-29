#ifndef _ANIM_H
#define _ANIM_H


#include "ostimer.h"
#include <vector>
#include "anim_flavor.h"

#include "refptr.h"




///////////////////////////////////////////////////////////////////////////////
// TEMPLATE CLASS anim is the base for all animation playback classes, providing
// current time, total animation duration, and all basic animation methods.
///////////////////////////////////////////////////////////////////////////////

enum anim_flags_t
{
  ANIM_RELATIVE_TO_START = 0x0001,  // marks a track/animation as being start-relative
  ANIM_REVERSE           = 0x0002,  // play animation backwards
  ANIM_LOOPING           = 0x0004,  // loop animation
  ANIM_AUTOKILL          = 0x0008,  // automatically kill animation at end (see wds)
  ANIM_ATTACHED          = 0x0010,  // animation is currently operative on associated entity
  ANIM_SUSPENDED         = 0x0040,  // animation is suspended
  ANIM_ALLOW_CONTROLS    = 0x0080,  // allow entity controls to remain active while playing
  ANIM_COMPUTE_SECTOR    = 0x0100,  // compute entity BSP sector while playing
  ANIM_NONCOSMETIC       = 0x0200,  // animation is not cosmetic (only applies to non-character anims; see wds)
  ANIM_WAS_LOOPED        = 0x0400,  // animation looped during its last frame_advance
  ANIM_TWEEN             = 0x0800,  // animation should be tweened
  ANIM_VALID             = 0x1000,  // animation is valid
  ANIM_PO_FIXUP          = 0x2000,  // calls po::fixup every frame
  ANIM_FORCE_ABSOLUTE    = 0x4000,  // overrides RELATIVE_TO_START
};


// this structure is passed into anim::frame_advance()
class anim_control_t
{
private:
  time_value_t current_time;
  time_value_t time_delta;
  time_value_t duration;
  rational_t timescale_factor;
  rational_t cntrl_tween_duration;
  rational_t tween_timer;
  unsigned int flags;
  short loop_count;

public:
  anim_control_t() {}

  anim_control_t( time_value_t _current_time,
                   time_value_t _duration,
                   rational_t _timescale_factor,
                   unsigned int _flags,
                   short _loop_count = -1 )
  : current_time( _current_time ),
    time_delta( 0.0f ),
    duration( _duration ),
    timescale_factor( _timescale_factor ),
	cntrl_tween_duration(0.3f),
	tween_timer(0.0f),
    flags( _flags ),
    loop_count( _loop_count )
  {
  }

  inline time_value_t get_time() const { return current_time; }
  inline void set_time( time_value_t t ) { current_time = t; }

  inline time_value_t get_time_delta() const { return time_delta; }

  inline time_value_t get_duration() const { return duration; }

  inline rational_t get_timescale_factor() const { return timescale_factor; }
  inline void set_timescale_factor( rational_t v ) { timescale_factor = v; }

  inline rational_t get_tween_duration() const { return cntrl_tween_duration; }
  inline void set_tween_duration( rational_t dur ) { cntrl_tween_duration = dur; }

  inline rational_t get_tween_timer() const { return tween_timer; }
  inline void set_tween_timer( rational_t dur ) { tween_timer = dur; }

  inline short get_loop_count() const { return loop_count; }
  inline void set_loop_count( short v ) { loop_count = v; }

  inline unsigned int get_flags() const { return flags; }
  inline void set_flags( unsigned int _flags ) { flags = _flags; }

  inline void set_flag( anim_flags_t f ) { flags |= f; }
  inline void clear_flag( anim_flags_t f ) { flags &= ~f; }
  inline void set_flag( anim_flags_t f, bool torf ) { if (torf) set_flag(f); else clear_flag(f); }
  inline bool is_flagged( anim_flags_t f ) const { return (flags & f); }

  // flags that are relevant to anim_control_t
  inline bool is_reverse() const { return is_flagged(ANIM_REVERSE); }
  inline bool is_looping() const { return is_flagged(ANIM_LOOPING); }
  inline bool is_autokill() const { return is_flagged(ANIM_AUTOKILL); }
  inline bool is_suspended() const { return is_flagged(ANIM_SUSPENDED); }
  inline bool is_allow_controls() const { return is_flagged(ANIM_ALLOW_CONTROLS); }
  inline bool is_compute_sector() const { return is_flagged(ANIM_COMPUTE_SECTOR); }
  inline bool is_noncosmetic() const { return is_flagged(ANIM_NONCOSMETIC); }
  inline bool is_tween() const { return is_flagged(ANIM_TWEEN); }
  inline bool was_looped() const { return is_flagged(ANIM_WAS_LOOPED); }
  inline bool is_po_fixup() const { return(is_flagged(ANIM_PO_FIXUP)); }
  inline bool is_done_tween() const { return (!is_tween() || (tween_timer >= cntrl_tween_duration)); }

  inline bool is_finished() const
  {
    if ( is_looping() && (loop_count==-1 || loop_count>0) )
      return false;
    if ( is_reverse() )
      return ( current_time <= 0 );
    else
      return ( current_time >= duration );
  }

  inline void frame_advance( time_value_t t )
  {
    time_delta = t;
	tween_timer += t;
    t *= timescale_factor;
    clear_flag( ANIM_WAS_LOOPED );
    if ( !is_suspended() && !is_finished() && duration > 0.01f  )
    {
      if ( is_reverse() )
      {
        current_time -= t;
        if ( is_looping()
          && current_time < 0
          && ( loop_count==-1 || loop_count>0 )
          )
        {
          while ( current_time < 0 )
            current_time += duration;
          set_flag( ANIM_WAS_LOOPED );
          if ( loop_count > 0 )
            --loop_count;
        }
      }
      else
      {
        current_time += t;
        if ( is_looping()
          && current_time > duration
          && ( loop_count==-1 || loop_count>0 )
          )
        {
          while ( current_time > duration )
            current_time -= duration;
          set_flag( ANIM_WAS_LOOPED );
          if ( loop_count > 0 )
            --loop_count;
        }
      }
    }
  }
};



template < class animatable_t >
class anim
{
// Data
protected:
  unsigned short flags;

public:
  anim()
  : flags( 0 )
  {
  }

  anim( unsigned int _flags )
  : flags( _flags )
  {
  }

  void construct( unsigned short _flags )
  {
    flags = _flags | ANIM_VALID;
  }

  inline unsigned short get_flags() const { return flags; }

  virtual void set_flag( anim_flags_t f ) { flags |= f; }
  virtual void clear_flag( anim_flags_t f ) { flags &= ~f; }
  virtual void set_flag( anim_flags_t f, bool torf ) { if (torf) set_flag(f); else clear_flag(f); }
  inline bool is_flagged( anim_flags_t f ) const { return (flags & f); }

  // flags that are relevant to individual animations
  inline bool is_attached() const { return is_flagged( ANIM_ATTACHED ); }
  inline bool is_relative_to_start() const { return is_flagged( ANIM_RELATIVE_TO_START ); }
  inline bool is_valid() const { return is_flagged( ANIM_VALID ); }

  inline void set_valid( bool v ) { set_flag( ANIM_VALID, v ); }

  virtual void set_time( time_value_t t ) {};

  // advance this animation according to the given frame control data and
  // write the resulting animation value into <dest>
  virtual void frame_advance( const anim_control_t& ac, animatable_t* dest ) {};

  // call this function to retrieve the value of the animation at the current time
  // (this is only valid when preceded by a frame_advance call using the same control data)
  virtual void get_value( const anim_control_t& ac, animatable_t* dest ) const {};

  // call this virtual function to compute the value of the animation at an arbitrary time
  virtual void get_value( time_value_t t, animatable_t* dest ) const {};
};


///////////////////////////////////////////////////////////////////////////////
// TEMPLATE CLASS key_anim allows the playback of a keyframe track.
// To advance the animation's playback state, call frame_advance(); to compute
// the resultant value, call get_value().
///////////////////////////////////////////////////////////////////////////////

template < class animatable_t, class key_t, class track_t >
class key_anim : public anim< animatable_t >
{
// Data
private:
#ifndef __GNUC__
  typename track_t* track;
  typename track_t::iterator current_key;
#else
  track_t* track;
  typename track_t::iterator current_key;
#endif

// Methods
public:
  key_anim()
    :   anim<animatable_t>(),
        track( NULL )
  {
  }

#ifndef __GNUC__
  void construct( typename track_t* _track,
#else
  void construct( track_t* _track,
#endif
                  unsigned short _flags )
  {
    anim<animatable_t>::construct( _flags );
    track = _track;
  }

  // force current_time to given value
  virtual void set_time( time_value_t t )
  {
    sync_key( t );
  }

  // synchronize current_key with current_time (only necessary when
  // current_time is changed non-incrementally)
  inline void sync_key( time_value_t t )
  {
    current_key = track->m_keys;
    typename track_t::iterator next_key = current_key;
    ++next_key;
    while ( next_key!=(track->m_keys + track->num_keys) && t>=(*next_key).get_time() )
    {
      ++current_key;
      ++next_key;
    }
  }

  virtual void frame_advance( const anim_control_t& ac, animatable_t* dest )
  {
    assert( track != NULL );
    if ( ac.was_looped() )
    {
      // a loop has occured this frame;
      // need to sync up
      sync_key( ac.get_time() );
    }
    else if ( ac.is_reverse() )
    {
      // going backward
      while ( current_key!=track->m_keys && ac.get_time()<(*current_key).get_time() )
      {
        // advance to the next key in the track
        --current_key;
      }
    }
    else
    {
      // going forward
      typename track_t::iterator next_key = current_key;
      ++next_key;
      while ( next_key!=(track->m_keys + track->num_keys) && ac.get_time()>=(*next_key).get_time() )
      {
        // advance to the next key in the track
        current_key = next_key;
        ++next_key;
      }
    }
    if ( dest != NULL )
    {
      // return value
      get_value( ac, dest );
    }
  }

  // call this function to retrieve the value of the animation at the current time
  // (this is only valid when preceded by a frame_advance call using the same control data)
  virtual void get_value( const anim_control_t& ac, animatable_t* dest ) const
  {
    // compute the value of the animation
    if ( ac.get_time()<(*current_key).get_time() || ac.get_time()>ac.get_duration() )
    {
      // off the front or back of the track
      *dest = (*current_key).get_value();
    }
    else
    {
      // in the track
      typename track_t::iterator next_key = current_key;
      ++next_key;
      if ( next_key == (track->m_keys + track->num_keys) )
      {
        // we've hit the end of the track
        *dest = (*current_key).get_value();
      }
      else
      {
        // interpolate between current key and the next
        *dest = (*current_key).get_value( ac.get_time(), *next_key );
      }
    }
  }

  // call this virtual function to compute the value of the animation at an
  // arbitrary time
  virtual void get_value( time_value_t t, animatable_t* dest ) const
  {
    assert( track!=NULL && dest!=NULL );
    typename track_t::iterator i0 = track->m_keys;
    typename track_t::iterator i1 = i0;
    typename track_t::iterator i_end = (track->m_keys + track->num_keys);
    ++i1;
    while ( i1!=i_end && t>=(*i1).get_time() )
    {
      ++i0;
      ++i1;
    }
    const key_t& k0 = *i0;
    const key_t& k1 = *i1;
    if ( t<k0.get_time() || i1==i_end )
    {
      // off the front or back of the track
      *dest = k0.get_value();
    }
    else
    {
      // in the track;
      // interpolate between current key and the next
      *dest = k0.get_value( t, k1 );
    }
  }

  friend class po_anim;
};


#endif  // _ANIM_H
