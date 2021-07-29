#ifndef _PO_ANIM_H
#define _PO_ANIM_H


#include "anim.h"
#include "po.h"
#include "linear_anim.h"
#include "staticmem.h"

class entity_track_tree;

class PRS_track
  {
  // Data
  private:
    time_value_t duration;
    unsigned     flags;
    linear_track<vector3d>*   P;
    linear_track<quaternion>* R;
    linear_track<rational_t>* S;

  // Methods
  public:
    PRS_track()
      :   duration( 0 ),
          flags( 0 ),
          P( NULL ),
          R( NULL ),
          S( NULL )
      {
      }

    PRS_track( const PRS_track& b )
      :   duration( b.duration ),
          flags( b.flags ),
          P( b.P ),
          R( b.R ),
          S( b.S )
      {
      }

    PRS_track( linear_track<vector3d>* __P,
               linear_track<quaternion>* __R,
               linear_track<rational_t>* __S,
               unsigned short __flags);

    ~PRS_track();

    time_value_t get_duration() const { return duration; }

    const linear_track<vector3d>*   get_P() const { return P; }
    const linear_track<quaternion>* get_R() const { return R; }
    const linear_track<rational_t>* get_S() const { return S; }

    bool empty() const { return !((bool)P || (bool)R || (bool)S); }

    unsigned short get_flags() const { return flags; }
    bool is_relative_to_start() const { return (flags & ANIM_RELATIVE_TO_START); }

  private:
    void _compute_duration();

  public:
#if !defined(NO_SERIAL_IN)
    void internal_serial_in( chunk_file& fs );
#endif
#if !defined(NO_SERIAL_OUT)
    void internal_serial_out( chunk_file& fs ) const;
#endif
    friend void entity_track_tree_from_binary( entity_track_tree*, const char* fname, unsigned int ett_size );

#ifdef TARGET_GC
		void endian_fixup_hack( void );
		void endian_fixup_hack_P( void );
		void endian_fixup_hack_R( void );
		void endian_fixup_hack_S( void );
#endif

  };


class po_anim : public anim< po >
  {
  // Types
  public:
    typedef anim<vector3d>   P_t;
    typedef anim<quaternion> R_t;
    typedef anim<rational_t> S_t;

    enum
      {
      FLAGS_INHERITED_FROM_TRACK = ANIM_RELATIVE_TO_START,
      FLAGS_PASSED_TO_SUBANIMS = ANIM_REVERSE | ANIM_LOOPING
      };

  // Data
  private:
    P_t* P;
    R_t* R;
    S_t* S;
    vector3d P_start;
    quaternion R_start;
    rational_t S_start;

//    matrix4x4 R_start_mat;
    vector3d R_start_mat[3];

  // Methods
  public:
    po_anim();
    virtual ~po_anim();

    // Construct (or reconstruct) a po_anim with the given initial state,
    // animation track, and start time.  Note that the start time can be any
    // value; any value outside the range of the track (i.e., less than zero or
    // greater than the track's duration) will cause a transition key to be
    // created (see anim.h).
    void construct( const po& initial,
                    const PRS_track& track,
                    unsigned short anim_flags );

    const vector3d& get_P_start() const { return P_start; }
    const quaternion& get_R_start() const { return R_start; }
    const rational_t& get_S_start() const { return S_start; }

	void reset_start_vals(void);

    bool has_P() const { return ( P!=NULL && P->is_valid() ); }
    bool has_R() const { return ( R!=NULL && R->is_valid() ); }
    bool has_S() const { return ( S!=NULL && S->is_valid() ); }

    // force current_time to given value
    virtual void set_time( time_value_t t );

    virtual void frame_advance( const anim_control_t& ac, po* dest );
    virtual void frame_advance( const anim_control_t& ac, vector3d &p, quaternion &r, rational_t &s );

    // get current value (valid only when preceded by a frame_advance call with the same control data)
    virtual void get_value( const anim_control_t& ac, po* dest ) const;

    // get value of animation at a particular time
    virtual void get_value( time_value_t t, po* dest ) const;

    // this virtual function allows animations to respond properly to a loop
    // condition by resetting the start value (needed only in start-relative
    // tracks)
    void reset_start_for_loop( const anim_control_t& ac );

    // this virtual function allows the start values to be adjusted based on the
    // given destination state and the current state of the animation
    void reset_start( const anim_control_t& ac, const po& dest );

    // call this function to compute the unadjusted position value of the animation at the current time
    void get_unadjusted_value( const anim_control_t& ac, vector3d* destP ) const;
    // call this to compute the unadjusted position value of the animation at an arbitrary time
    void get_unadjusted_value( time_value_t t, vector3d* destP ) const;

	// call this function to compute the unadjusted quaternion value of the animation at the current time
    void get_unadjusted_value( const anim_control_t& ac, quaternion* destR ) const;
    // call this to compute the unadjusted quaternion value of the animation at an arbitrary time
    void get_unadjusted_value( time_value_t t, quaternion* destR ) const;

	// call this function to compute the unadjusted scale value of the animation at the current time
    void get_unadjusted_value( const anim_control_t& ac, rational_t* destS ) const;
    // call this to compute the unadjusted scale value of the animation at an arbitrary time
    void get_unadjusted_value( time_value_t t, rational_t* destS ) const;

		STATICALLOCCLASSHEADER

  };


#endif  // _PO_ANIM_H
