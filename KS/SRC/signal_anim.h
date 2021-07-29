#ifndef _SIGNAL_ANIM_H
#define _SIGNAL_ANIM_H

#include <map>
#include "anim.h"
#include "entflavor.h"
#include "project.h"


typedef unsigned short signal_id_t;

class entity_track_node;
class entity_track_tree;

class signal_key 
  {
  private:
    typedef uint32 signal_set;
    time_value_t timestamp;
    signal_set  key_flags;   

  // Methods
  public:
    signal_key()
      :   timestamp(0),
          key_flags(0)
      {
      }

    signal_key( signal_id_t _key_value )
      :   timestamp(0),
          key_flags( 1<<_key_value )
      {
      }

    signal_key( time_value_t _timestamp )
      :   timestamp( _timestamp ),
          key_flags(0)
      {
      }

    signal_key( time_value_t _timestamp, signal_id_t _key_value )
      :   timestamp( _timestamp ),
          key_flags( 1<<_key_value )
      {
      }

    time_value_t get_time() const { return timestamp; }

    void add_value( signal_id_t _key_value ) { key_flags |= (1<<_key_value); }
    const signal_set get_value() const { return key_flags; }
  };

class signal_track 
  {

  friend class signal_anim;

  // Types
  public:
                   
  // Data
  private:
    // WARNING:  do not add, remove or change data types without changing the .ANMX format first
    time_value_t duration;
    bool         valid;
    int          num_signals;
    signal_key*  signals;

  // Methods
  public:
    signal_track()
      :   duration( 0 ),
          valid( false ),
          signals()
      {
      }

    ~signal_track()
    {
      delete[] signals;  // containment issues
    }

    time_value_t get_duration() const { return duration; }

    bool empty() const { return num_signals==0; }

    bool is_valid() const { return valid; }

    void add_key( time_value_t timestamp, entity_flavor_t entity_flavor, stringx signaldata );

#if !defined(NO_SERIAL_IN)
    void internal_serial_in( chunk_file& fs );
#endif
#if !defined(NO_SERIAL_OUT)
    void internal_serial_out( chunk_file& fs ) const;
#endif
    friend void entity_track_tree_from_binary( entity_track_tree*, const char* fname, unsigned int ett_node );
    friend void debug_compare_nodes( entity_track_node* node1, entity_track_node* node2 );
  };

class signal_anim : public anim<signal_key> 
  {
  private:
    vector<signal_key> signals;
    time_value_t last_get_time;
  
  // Methods
  public:
    signal_anim();
    virtual ~signal_anim();

    void construct( const signal_track& track, unsigned short anim_flags );

    // force current_time to given value
    virtual void set_time( time_value_t t );

    void frame_advance( const anim_control_t& ac, vector<signal_id_t>* dest );
  };

#endif  // _SIGNAL_ANIM_H
