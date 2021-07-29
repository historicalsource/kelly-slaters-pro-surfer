#ifndef _LINEAR_ANIM_H
#define _LINEAR_ANIM_H


#include "anim.h"
#include "staticmem.h"

class entity_track_node;

///////////////////////////////////////////////////////////////////////////////
// TEMPLATE CLASS linear_key implements a linear keyframe for any animatable
// data type.  The user must provide an external function matching:
//
//    animatable_t linear_interpolate( const animatable_t& a,
//                                     const animatable_t& b,
//                                     rational_t r );
//
// This external function is responsible for performing a linear interpolation
// between the given animatable values using the given fraction (0.0f - 1.0f).
///////////////////////////////////////////////////////////////////////////////

template< class animatable_t >
class linear_key
  {
  // Data
  private:
    time_value_t timestamp;
    animatable_t key_value;

  // Methods
  public:
    linear_key()
      :   timestamp(0),
          key_value()
      {
      }

    linear_key( const linear_key& b )
      :   timestamp( b.timestamp ),
          key_value( b.get_value() )
      {
      }

    linear_key( animatable_t _key_value )
      :   timestamp(0),
          key_value( _key_value )
      {
      }

    linear_key( time_value_t _timestamp, animatable_t _key_value )
      :   timestamp( _timestamp ),
          key_value( _key_value )
      {
      }

    const animatable_t& get_value() const { return key_value; }
    void set_value( const animatable_t& v ) { key_value = v; }

    // interpolate function must be defined by user of template class
    animatable_t interpolate( const linear_key& b, rational_t r ) const;

    animatable_t get_value( time_value_t t, const linear_key& b ) const
      {
      assert( t>=get_time() && t<=b.get_time() );
      rational_t len = b.get_time() - get_time();
      rational_t d = t - get_time();
      return interpolate( b, len>0.0001f? d/len : 1.0f );
      }

    time_value_t get_time() const {return timestamp;}

    // The following I/O functions require the user to supply serial_in() and
    // serial_out() for the animatable type.

#if !defined(NO_SERIAL_IN)
    void internal_serial_in( chunk_file& fs )
      {
      serial_in( fs, &timestamp );
      serial_in( fs, &key_value );
      }
#endif

#if !defined(NO_SERIAL_OUT)
    void internal_serial_out( chunk_file& fs ) const
      {
      serial_out( fs, timestamp );
      serial_out( fs, key_value );
      }
#endif
#ifdef TARGET_GC
	void endian_fixup_hack( void )
	{
		fixup((unsigned char *) &(timestamp), sizeof(time_value_t));
		endian_fixup(key_value);
	}
#endif

  };


///////////////////////////////////////////////////////////////////////////////
// TEMPLATE CLASS linear_track provides a wrapper around a simple list of
// linear keys, and provides the meat for any linear_anim (see below).
///////////////////////////////////////////////////////////////////////////////

template< class animatable_t >
class linear_track
{
  // Types
public:
  typedef linear_key< animatable_t >  key_t;
  typedef key_t* iterator;

public:
  int         num_keys;
  key_t*      m_keys;

// Methods
public:
  linear_track()
    :   num_keys(0), m_keys(NULL)
  {
  }

  linear_track( const linear_track& b )
    :   num_keys(b.num_keys)
  {
    m_keys = NEW key_t[ num_keys ];
    memcpy( b.m_keys, m_keys, sizeof(key_t)*num_keys );
  }

  ~linear_track()
  {
    delete[] m_keys;   // we're going to have to make ownership flexible
  }

  time_value_t get_duration() const { return m_keys[num_keys-1].get_time(); }

  const animatable_t& get_end_value() const { return m_keys[num_keys-1].get_value(); }

#ifdef TARGET_GC
	void endian_fixup_hack( void )
	{
		if ( num_keys < 0 || num_keys > 0xFFFF )
		{
			fixupint(&(num_keys));
				//for ( int i=0; i<num_keys; i++ )
				//{
				//	m_keys[i].endian_fixup_hack();
				//}
			if ( num_keys && m_keys )
	      for ( iterator i=m_keys; i<m_keys+num_keys; i++ )
	        (*i).endian_fixup_hack();
		}
	}
#endif

#if !defined(NO_SERIAL_IN)
  void internal_serial_in( chunk_file& fs )
  {
    chunk_flavor cf;
    for ( serial_in(fs,&cf); cf!=CHUNK_END; serial_in(fs,&cf) )
    {
      if ( cf == chunk_flavor("keys") )
      {
        // read key list
        unsigned int nkeys;
        serial_in( fs, &nkeys );
        assert( m_keys==NULL );
        m_keys = NEW key_t[ nkeys ];
        iterator i;
        for ( i=m_keys; i<m_keys+nkeys; i++ )
          (*i).internal_serial_in( fs );
        num_keys = nkeys;
      }
      else
        error( fs.get_name() + ": unknown chunk found in linear_track" );
    }
  }
#endif
/*
#if !defined(NO_SERIAL_OUT)
  void internal_serial_out( chunk_file& fs ) const
  {
    // write key list
    serial_out( fs, chunk_flavor("keys") );
    serial_out( fs, (unsigned int)size() );
    const_iterator i;
    for ( i=begin(); i<end(); i++ )
      (*i).internal_serial_out( fs );
    // end of linear_track
    serial_out( fs, CHUNK_END );
  }
#endif
  */


};


///////////////////////////////////////////////////////////////////////////////
// TEMPLATE CLASS linear_anim is the linear variety of keyframe animation.
///////////////////////////////////////////////////////////////////////////////

template < class animatable_t >
class linear_anim : public key_anim< animatable_t,
                                     linear_key<animatable_t>,
                                     linear_track<animatable_t> >
{
// Types
public:
  typedef linear_key< animatable_t >  key_t;
  typedef linear_track< animatable_t >  track_t;
  typedef key_anim< animatable_t, key_t, track_t >  base_t;

// Methods
public:
  linear_anim()
  : base_t()
  {
  }

	STATICALLOCCLASSHEADER

};


#endif  // _LINEAR_ANIM_H
