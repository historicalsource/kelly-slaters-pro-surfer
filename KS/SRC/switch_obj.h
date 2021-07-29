#ifndef _SWITCH_OBJ_H_
#define _SWITCH_OBJ_H_

#include "entity.h"

class switch_obj : public entity
{
public:
  typedef enum
  {
    _SWITCH_OFF = 0,
    _SWITCH_ON = 1
  } switch_state;

protected:
  void init();

  virtual void copy_instance_data( const switch_obj& b );

  switch_state state;
  bool alarm;

public:
  // Constructors
  switch_obj( const entity_id& _id, unsigned int _flags );

  switch_obj( chunk_file& fs,
          const entity_id& _id,
          entity_flavor_t _flavor = ENTITY_SWITCH,
          unsigned int _flags = 0 );

  virtual entity* make_instance( const entity_id& _id,
                                 unsigned int _flags ) const;


  // Functions
  virtual void frame_advance(time_value_t t);
  virtual void flick();

  virtual bool handle_enx_chunk( chunk_file& fs, stringx& label );

  switch_state get_state() const  { return(state); }
  void set_state(switch_state s)  { state = s; }

  bool is_alarm() const           { return(alarm); }

/////////////////////////////////////////////////////////////////////////////
// Event signals
/////////////////////////////////////////////////////////////////////////////
public:
  // enum of local signal ids (for coding convenience and readability)
  enum signal_id_t
    {
    // a descendant class uses the following line to append its local signal ids after the parent's
    PARENT_SYNC_DUMMY = entity::N_SIGNALS - 1,
    #define MAC(label,str)  label,
    #include "switch_obj_signals.h"
    #undef MAC
    N_SIGNALS
    };

  // This static function must be implemented by every class which can generate
  // signals, and is called once only by the application for each such class;
  // the effect is to register the name and local id of each signal with the
  // signal_manager.  This call must be performed before any signal objects are
  // actually created for this class (via signaller::signal_ptr(); see signal.h).
  static void register_signals();

  static unsigned short get_signal_id( const char *name );

private:
  // Every descendant of signaller that expects to generate signals and has
  // defined its own local list of signal ids should implement this virtual
  // function for the construction of the signal list, so that it will reserve
  // exactly the number of signal pointers required, on demand.
  virtual signal_list* construct_signal_list() { return NEW signal_list( N_SIGNALS, (signal*)NULL ); }

protected:
  // This virtual function, used only for debugging purposes, returns the
  // name of the given local signal
  virtual const char* get_signal_name( unsigned short idx ) const;
};

#endif
