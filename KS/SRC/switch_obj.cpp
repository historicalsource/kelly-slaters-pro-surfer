#include "global.h"

#include "switch_obj.h"
//#include "physical_interface.h"
#include "wds.h"
#include "entity_maker.h"
#include "entityflags.h"
#include "hwaudio.h"

void switch_obj::init()
{
  state = _SWITCH_OFF;

  sound_device::inst()->load_sound( "switch_on" );
  sound_device::inst()->load_sound( "switch_off" );
  alarm = false;
}


switch_obj::switch_obj( const entity_id& _id, unsigned int _flags )
  : entity( _id, _flags )
{
  init();
  flavor = ENTITY_SWITCH;
}

switch_obj::switch_obj( chunk_file& fs,
        const entity_id& _id,
        entity_flavor_t _flavor,
        unsigned int _flags )
  : entity(fs, _id, _flavor, _flags)
{
  init();
}


entity* switch_obj::make_instance( const entity_id& _id,
                             unsigned int _flags ) const
{
  switch_obj* newit = NEW switch_obj( _id, _flags );
  newit->copy_instance_data( *((switch_obj *)this) );
  return (entity*)newit;
}

void switch_obj::copy_instance_data( const switch_obj& b )
{
  alarm = b.alarm;

  entity::copy_instance_data(b);
}


bool switch_obj::handle_enx_chunk( chunk_file& fs, stringx& label )
{
  if(label == "alarm")
  {
    alarm = true;
    return(true);
  }
  else
    return entity::handle_enx_chunk( fs, label );
}






void switch_obj::frame_advance(time_value_t t)
{
  entity::frame_advance(t);
}

void switch_obj::flick()
{
  state = (state == _SWITCH_ON) ? _SWITCH_OFF : _SWITCH_ON;

  raise_signal(SWITCH_TOGGLE);

  if(state == _SWITCH_ON)
  {
    raise_signal(SWITCH_ON);
    pstring snd("switch_on");
    sound_device::inst()->play_3d_sound( snd, get_abs_position() );
  }
  else
  {
    raise_signal(SWITCH_OFF);
    pstring snd("switch_off");
    sound_device::inst()->play_3d_sound( snd, get_abs_position() );
  }
}





/////////////////////////////////////////////////////////////////////////////
// Event signals
/////////////////////////////////////////////////////////////////////////////

// This static function must be implemented by every class which can generate
// signals, and is called once only by the application for each such class;
// the effect is to register the name and local id of each signal with the
// signal_manager.  This call must be performed before any signal objects are
// actually created for this class (via signaller::signal_ptr(); see signal.h).
void switch_obj::register_signals()
  {
  #define MAC(label,str)  signal_manager::inst()->insert( str, label );
  #include "switch_obj_signals.h"
  #undef MAC
  }

static const char* switch_signal_names[] =
  {
  #define MAC(label,str)  str,
  #include "switch_obj_signals.h"
  #undef MAC
  };

unsigned short switch_obj::get_signal_id( const char *name )
  {
  unsigned idx;

  for( idx = 0; idx < (sizeof(switch_signal_names)/sizeof(char*)); idx++ )
    {
    unsigned offset = strlen(switch_signal_names[idx])-strlen(name);

    if( offset > strlen( switch_signal_names[idx] ) )
      continue;

    if( !strcmp(name,&switch_signal_names[idx][offset]) )
      return( idx + PARENT_SYNC_DUMMY + 1 );
    }

  // not found
  return entity::get_signal_id( name );
  }

// This virtual function, used only for debugging purposes, returns the
// name of the given local signal
const char* switch_obj::get_signal_name( unsigned short idx ) const
  {
  assert( idx < N_SIGNALS );
  if ( idx <= PARENT_SYNC_DUMMY )
    return entity::get_signal_name( idx );
  else
    return switch_signal_names[idx-PARENT_SYNC_DUMMY-1];
  }
