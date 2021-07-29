#include "global.h"
#include <algorithm>
#include "signals.h"
#include "oserrmsg.h"
#include "vm_thread.h"


unsigned int signal_callback::id_counter = 0;

///////////////////////////////////////
// script callback data

script_callback::script_callback( script_object::instance* _inst, const vm_executable* _func, const char* _parms )
  : signal_callback(),
  inst( _inst ),
  func( _func )
{
  int psize = func->get_parms_stacksize();
  if ( psize )
  {
    parms = NEW char[psize];
    memcpy( parms, _parms, psize );
  }
  else
    parms = NULL;
}


script_callback::~script_callback()
{
  // fix up dangling pointers to me, change this code with care, it fixed a bug once.
  inst->clear_callback_references(this);

  if ( parms )
    delete[] parms;
}


// spawn script callback function
void script_callback::spawn(signaller*sgrptr)
{
    // NB: sgrptr is ignored
  if ( !is_disabled() )
  {
    vm_thread* newvmt;
    if ( is_one_shot() )
      newvmt = inst->add_thread( func, parms );
    else
      newvmt = inst->add_thread( this, func, parms );
    //newvmt->set_suspendable( is_suspendable() );  // TODO: figure this out
  }
}

const stringx &script_callback::get_func_name()
{
  return(func->get_fullname());
}

///////////////////////////////////////
// script code callback data
code_callback::code_callback( void (*fn)(signaller*,const char*), const char *cptr )
  : signal_callback()
{
  func = fn;
  parms = const_cast<char*>(cptr);
}

code_callback::~code_callback()
{
}

void code_callback::spawn(signaller*sgrptr)
{
  if( !is_disabled() )
  {
    func( sgrptr, parms );
  }
}

///////////////////////////////////////////////////////////////////////////////
// CLASS signal

signal::signal(signaller*sgrptr)
:   flags( 0 ),
    name( NULL ),
    outputs( NULL ),
    callbacks()
{
  flavor = SIGNAL;
  owner = sgrptr;
}


signal::signal( const char* _name, signaller*sgrptr )
:   flags( 0 ),
    name( _name ),
    outputs( NULL ),
    callbacks()
{
  flavor = SIGNAL;
  owner = sgrptr;
}


signal::~signal()
{
  if ( outputs != NULL )
  {
    delete outputs;
    outputs = NULL;
  }

  clear_callbacks();
}


// add an output link
void signal::link( signal* s )
{
  if ( outputs == NULL )
    outputs = NEW signal_list;
  signal_list::const_iterator i = find( outputs->begin(), outputs->end(), s );
  if ( i == outputs->end() )
    outputs->push_front( s );
}


// remove an output link
void signal::unlink( signal* s )
{
  if ( outputs )
  {
    signal_list::iterator i = find( outputs->begin(), outputs->end(), s );
    outputs->erase( i );
  }
}

void signal::clear_links()
{
  if ( outputs != NULL )
  {
    delete outputs;
    outputs = NULL;
  }
}

// find an output gated_signal matching the given parameters
signal* signal::find_AND( const signal* b ) const
{
  if ( outputs )
  {
    signal_list::const_iterator i = outputs->begin();
    signal_list::const_iterator i_end = outputs->end();
    for ( ; i!=i_end; i++ )
    {
      signal* s = *i;
      if ( s->get_flavor() == GATED_SIGNAL )
        {
        if ( ((gated_signal*)s)->match( gated_signal::AND, b ) )
          return s;
      }
    }
  }
  return NULL;
}

signal* signal::find_OR( const signal* b ) const
{
  if ( outputs )
  {
    signal_list::const_iterator i = outputs->begin();
    signal_list::const_iterator i_end = outputs->end();
    for ( ; i!=i_end; i++ )
    {
      signal* s = *i;
      if ( s->get_flavor() == GATED_SIGNAL )
      {
        if ( ((gated_signal*)s)->match( gated_signal::OR, b ) )
          return s;
      }
    }
  }
  return NULL;
}


// raise this signal!
void signal::raise()
{
//if ( !(flags & (DISABLED|RAISED)) )
  if ( !(flags & DISABLED) )
  {
    set_flag( RAISED );
    set_needs_refresh();
    // spawn script callbacks, if any
    do_callbacks();
    // raise the inputs of all linked output signals
    if ( outputs )
    {
      signal_list::const_iterator i = outputs->begin();
      signal_list::const_iterator i_end = outputs->end();
      for ( ; i!=i_end; i++ )
      {
        signal* s = *i;
        s->raise_input( this );
      }
    }
  }
}


void signal::set_needs_refresh()
  {
  if ( !is_flagged( NEEDS_REFRESH ) )  // avoid duplicate entries on the refresh list
  {
    // tell the signal_manager we need a refresh this frame (see below)
    set_flag( NEEDS_REFRESH );
    signal_manager::inst()->needs_refresh( this );
  }
}


// virtual function processes the raising of an input
void signal::raise_input( signal* input, signaller*sgrptr )
{
  // for basic signals, raising an input is equivalent to raising the signal directly
  raise();
}


// This virtual function performs an internal reset (once per frame, initiated by the
// signal_manager; see below) of ephemeral changes accumulated while raising signals
// in the course of a game frame.
void signal::refresh()
{
  // RAISED protects a signal from being raised more than once per frame;
  // NEEDS_REFRESH protects a signal from being duplicated on the signal_manager's refresh list
  flags &= ~( RAISED | NEEDS_REFRESH );
}


// add a script callback for this signal
unsigned int signal::add_callback( script_object::instance* _inst, vm_executable* _func, char* _parms, bool one_shot )
{
  script_callback* new_callback = NEW script_callback( _inst, _func, _parms );
  new_callback->set_one_shot( one_shot );
  callbacks.push_back( new_callback );

  return(new_callback->get_id());
}

// add code callback for this signal
unsigned int signal::add_callback( void (*fn)(signaller*,const char*), char* _parms, bool one_shot )
{
  code_callback* new_callback = NEW code_callback( fn, static_cast<const char*>(_parms) );
  new_callback->set_one_shot( one_shot );
  callbacks.push_back( new_callback );

  return(new_callback->get_id());
}


// clear the script callback list
void signal::kill_callback(unsigned int callback_id)
{
  callback_list::iterator i = callbacks.begin();

  for ( ; i!=callbacks.end(); ++i )
  {
    signal_callback* sc = *i;
    if ( sc != NULL && sc->get_id() == callback_id)
    {
      callbacks.erase(i);
      delete sc;

      return;
    }
  }
}

void signal::clear_callbacks()
{
  callback_list::const_iterator i = callbacks.begin();
  callback_list::const_iterator i_end = callbacks.end();
  for ( ; i!=i_end; ++i )
  {
    signal_callback* sc = *i;
    if ( sc != NULL )
      delete sc;
  }
  callbacks.resize(0);
}


void signal::clear_script_callbacks()
{
  callback_list::iterator i = callbacks.begin();

  while(i!=callbacks.end())
  {
    signal_callback* sc = *i;
    if ( sc != NULL && sc->is_script_callback())
    {
      i = callbacks.erase(i);
      delete sc;
    }
    else
      ++i;
  }
}

void signal::clear_code_callbacks()
{
  callback_list::iterator i = callbacks.begin();

  while(i!=callbacks.end())
  {
    signal_callback* sc = *i;
    if ( sc != NULL && sc->is_code_callback())
    {
      i = callbacks.erase(i);
      delete sc;
    }
    else
      ++i;
  }
}

void signal::clear_script_callback(const stringx &name)
{
  callback_list::iterator i = callbacks.begin();

  for ( ; i!=callbacks.end(); ++i )
  {
    signal_callback* sc = *i;
    if ( sc != NULL && sc->is_script_callback() && ((script_callback *)sc)->get_func_name() == name)
    {
      callbacks.erase(i);
      delete sc;

      return;
    }
  }
}


// spawn script callbacks, if any
void signal::do_callbacks()
{
  if ( !is_flagged(CALLBACKS_DISABLED) )
  {
    {
      callback_list::iterator i = callbacks.begin();

      for ( ; i!=callbacks.end(); )
      {
        signal_callback* c = *i;
        c->spawn( owner );
        if ( c->is_one_shot() )
        {
          delete c;
          i = callbacks.erase( i );
        }
        else
          ++i;
      }
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
// CLASS gated_signal

gated_signal::gated_signal( type_t _type, signal* _input_a, signal* _input_b )
  :   signal(),
      type( _type ),
      flags( 0 ),
      input_a( _input_a ),
      input_b( _input_b )
{
  flavor = GATED_SIGNAL;
}


// virtual function processes the raising of an input
void gated_signal::raise_input( signal* input )
{
  // raise signal if gate condition has been satisfied
  if ( input == input_a )
    set_flag( RAISED_A );
  else if ( input == input_b )
    set_flag( RAISED_B );
  else
    assert( 0 );
  if ( type == AND )
  {
    if ( (flags&(RAISED_A|RAISED_B)) == (RAISED_A|RAISED_B) )
      raise();
  }
  else
    raise();
  // need a refresh in any case
  set_needs_refresh();
}


// return true if parameters match this gated_signal
bool gated_signal::match( type_t _type, const signal* input ) const
{
  return (_type==type && (input==input_b || input==input_a));
}


// This virtual function performs an internal reset (once per frame, initiated by the
// signal_manager; see below) of ephemeral changes accumulated while raising signals
// in the course of a game frame.
void gated_signal::refresh()
{
  signal::refresh();
  // these flags are used to evaluate whether the gate condition has been satisfied
  // at any time during a single game frame
  flags = 0;
}



///////////////////////////////////////////////////////////////////////////////
// CLASS signaller

signaller::signaller()
  :   flags( 0 ),
      signals( NULL )
{
}


signaller::~signaller()
{
  if ( signals != NULL )
  {
    signal_list::iterator i = signals->begin();
    signal_list::iterator i_end = signals->end();
    for ( ; i!=i_end; ++i )
    {
      signal* s = *i;
      if ( s != NULL )
        delete s;
    }
    delete signals;
    signals = NULL;
  }
}


void signaller::clear_callbacks()
{
  if ( signals != NULL )
  {
    signal_list::iterator i = signals->begin();
    signal_list::iterator i_end = signals->end();
    for ( ; i!=i_end; ++i )
    {
      if(*i)
      {
        (*i)->clear_callbacks();
        (*i)->clear_links();
      }
    }
  }
}

void signaller::clear_script_callbacks()
{
  if ( signals != NULL )
  {
    signal_list::iterator i = signals->begin();
    signal_list::iterator i_end = signals->end();
    for ( ; i!=i_end; ++i )
    {
      if(*i)
      {
        (*i)->clear_script_callbacks();
        (*i)->clear_links();
      }
    }
  }
}

void signaller::clear_code_callbacks()
{
  if ( signals != NULL )
  {
    signal_list::iterator i = signals->begin();
    signal_list::iterator i_end = signals->end();
    for ( ; i!=i_end; ++i )
    {
      if(*i)
        (*i)->clear_code_callbacks();
    }
  }
}

void signaller::clear_script_callback(const stringx &name)
{
  if ( signals != NULL )
  {
    signal_list::iterator i = signals->begin();
    signal_list::iterator i_end = signals->end();
    for ( ; i!=i_end; ++i )
    {
      if(*i)
        (*i)->clear_script_callback(name);
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
// CLASS signal_manager

DEFINE_SINGLETON( signal_manager )


signal_manager::signal_manager()
  :   signal_id_map(),
      refresh_list()
{
}


// return signaller-local index value for signal
unsigned short signal_manager::get_id( const stringx& name ) const
{
  signal_id_map_t::const_iterator i = signal_id_map.find( name );
  if ( i == signal_id_map.end() )
    error( "signal " + name + " not found" );
  return (*i).second;
}


// insert a NEW signal name and associated signaller-local index
void signal_manager::insert( const stringx& name, unsigned short id )
{
  pair< signal_id_map_t::iterator, bool > ib;
  ib = signal_id_map.insert( signal_id_map_t::value_type( name, id ) );
  assert( ib.second );
}


// create NEW signal consisting of logical AND of given signals
signal* signal_manager::signal_AND( signal* a, signal* b ) const
{
  // see if given signal already exists
  signal* sig = a->find_AND( b );
  if ( !sig )
  {
    // if not, create it
    sig = NEW gated_signal( gated_signal::AND, a, b );
    a->link( sig );
    b->link( sig );
  }
  return sig;
}


// create NEW signal consisting of logical OR of given signals
signal* signal_manager::signal_OR( signal* a, signal* b ) const
{
  // see if given signal already exists
  signal* sig = a->find_OR( b );
  if ( !sig )
  {
    // if not, create it
    sig = NEW gated_signal( gated_signal::OR, a, b );
    a->link( sig );
    b->link( sig );
  }
  return sig;
}


// this function is called when a signal object is affected such that it
// will need to be reset at the end of the game frame
void signal_manager::needs_refresh( signal* s )
{
  refresh_list.push_back( s );
}


// this function is called once per game frame to reset any signals that need it
// (see app::tick)
void signal_manager::do_refresh()
{
  signal_list::const_iterator i = refresh_list.begin();
  signal_list::const_iterator i_end = refresh_list.end();
  for ( ; i!=i_end; ++i ) {
		signal* s = (*i);
		assert( s );
    s->refresh();
	}
  refresh_list.resize(0);
}

// PEH BETA LOCK
void signal_manager::purge()
{
  refresh_list.resize(0);
  signal_id_map.clear();
}
