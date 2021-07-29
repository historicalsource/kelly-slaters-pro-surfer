// signals.h
#ifndef _SIGNALS_H
#define _SIGNALS_H


#include "singleton.h"
#include <list>
#include <map>
#include "fast_vector.h"
#include "script_object.h"

#define DEBUG_SIGNAL_NAMES 1

class signaller;

class signal_callback
{
 public:
  signal_callback(){ disabled=one_shot=false; id=id_counter++; }
  virtual ~signal_callback(){}

  virtual void spawn(signaller*sgrptr=0) = 0;

  void disable() { disabled = true; }
  void enable() { disabled = false; }
  bool is_disabled() const { return disabled; }

  void set_one_shot( bool tf ) { one_shot = tf; }
  bool is_one_shot() const { return one_shot; }

  unsigned int get_id() const { return(id); }

  virtual bool is_code_callback() { return(false); }
  virtual bool is_script_callback() { return(false); }

 protected:
    char* parms;
    bool disabled;
    bool one_shot;
    unsigned int id;

    static unsigned int id_counter;
};

class script_callback : public signal_callback
  {
  private:
    script_object::instance* inst;
    const vm_executable* func;

  public:
    script_callback( script_object::instance* _inst, const vm_executable* _func, const char* _parms );
    virtual ~script_callback();

    virtual bool is_script_callback() { return(true); }
    const stringx &get_func_name();

  virtual void spawn(signaller*sgrptr=0);
  };

class code_callback : public signal_callback
  {
  public:
    code_callback( void (*fn)(signaller*,const char*), const char *cptr );
    virtual ~code_callback();

    virtual void spawn(signaller*sgrptr=0);

    virtual bool is_code_callback() { return(true); }

  private:
    void (*func)(signaller*,const char*);
  };


class signal
  {
  // Types
  public:
    enum flavor_t
      {
      SIGNAL,
      GATED_SIGNAL,
      N_FLAVORS
      };

    enum flags_t
      {
      RAISED             = 0x0001,  // true when signal has been raised this frame
      NEEDS_REFRESH      = 0x0002,  // true when signal has been added to the refresh list this frame (see signal_manager)
      DISABLED           = 0x0004,  // true when signal is disabled
      CALLBACKS_DISABLED = 0x0008,  // true when signal callbacks are disabled
      };

    typedef list<signal*> signal_list;
    typedef list<signal_callback*> callback_list;

  // Data
  protected:
    flavor_t flavor;
  private:
    unsigned int flags;

    #if DEBUG_SIGNAL_NAMES
    const char* name;
    #endif

    signal_list* outputs;
    callback_list callbacks;

  // Methods
  public:
    signal(signaller*sgrptr=0);
    signal( const char* _name, signaller*sgrptr=0 );
    virtual ~signal();

    flavor_t get_flavor() const { return flavor; }

    void set_flag( flags_t f ) { flags |= f; }
    void clear_flag( flags_t f ) { flags &= ~f; }
    bool is_flagged( flags_t f ) const { return (flags & f); }

    // tell the signal_manager that this signal needs to be refreshed this frame
    void set_needs_refresh();

    // add an output link
    void link( signal* s );
    // remove an output link
    void unlink( signal* s );

    void clear_links();

    // find an output gated_signal matching the given parameters
    signal* find_AND( const signal* b ) const;
    signal* find_OR( const signal* b ) const;

    // raise this signal!
    void raise();

    // was I raised?
    bool raised() { return(is_flagged(RAISED)); }

    // add a script callback for this signal
    unsigned int add_callback( script_object::instance* _inst, vm_executable* _func, char* _parms, bool one_shot=false );
    unsigned int add_callback( void (*fn)(signaller*,const char*), char* _parms, bool one_shot=false );
    // clear the script callback list
    void kill_callback(unsigned int callback_id);

    void clear_callbacks();
    void clear_script_callbacks();
    void clear_code_callbacks();
    void clear_script_callback(const stringx &name);

    // This virtual function performs an internal reset (once per frame, initiated by the
    // signal_manager; see below) of ephemeral changes accumulated while raising signals
    // in the course of a game frame.
    virtual void refresh();

  private:
    signaller *owner;

    // process the raising of an input
    virtual void raise_input( signal* input, signaller*sgrptr=0 );

    // spawn script callbacks, if any
    void do_callbacks();
  };


class gated_signal : public signal
  {
  // Types
  public:
    enum type_t
      {
      AND,
      OR
      };
    enum flags_t
      {
      RAISED_A = 0x0001,  // input_a was raised
      RAISED_B = 0x0002,  // input_b was raised
      };

    // Data
  private:
    unsigned short type;
    unsigned short flags;
    signal* input_a;
    signal* input_b;

  // Methods
  public:
    gated_signal( type_t _type, signal* _input_a, signal* _input_b );

    void set_flag( flags_t f ) { flags |= f; }
    void clear_flag( flags_t f ) { flags &= ~f; }
    bool is_flagged( flags_t f ) const { return (flags & f); }

    // return true if parameters match this gated_signal
    bool match( type_t _type, const signal* input ) const;

    // This virtual function performs an internal reset (once per frame, initiated by the
    // signal_manager; see below) of ephemeral changes accumulated while raising signals
    // in the course of a game frame.
    virtual void refresh();

  private:
    // raise given input and return true if gate condition is satisfied
    virtual void raise_input( signal* input );
  };


class signaller
  {
  // Types
  public:
    enum flags_t
      {
      DISABLED = 0x0001,  // true if signaller is disabled (cannot raise signals)
      };

    typedef fast_vector< signal* > signal_list;

    enum signal_id_t
      {
      N_SIGNALS
      };

  // Data
  private:
    unsigned int flags;
    signal_list* signals;

  // Methods
  public:
    signaller();
    virtual ~signaller();

    void set_flag( flags_t f ) { flags |= f; }
    void clear_flag( flags_t f ) { flags &= ~f; }
    bool is_flagged( flags_t f ) const { return (flags & f); }

    virtual bool is_an_entity() const { return(false); }
    virtual bool is_a_trigger() const { return(false); }
//    virtual bool is_dread_net() const { return(false); }

    void disable() { set_flag(DISABLED); }
    void enable() { clear_flag(DISABLED); }

    signal_list::size_t n_signals() const { return signals? signals->size() : 0; }

    // given local signal id, return corresponding pointer;
    // this will construct objects as required, and never return NULL
    signal* signal_ptr( signal_list::size_t idx )
      {
      if ( signals == NULL )
        signals = construct_signal_list();
      signal_list& sl = *signals;
      if ( sl[idx] == NULL )
        {
        #if DEBUG_SIGNAL_NAMES
        sl[idx] = NEW signal( get_signal_name(idx), this );
        #else
        sl[idx] = NEW signal(this);
        #endif
        }
      return sl[idx];
      }

    // this will raise the given signal, if present (non-NULL)
    virtual void raise_signal( signal_list::size_t idx ) const
      {
      if ( signals && !is_flagged(DISABLED) )
        {
        signal_list& sl = *signals;
        if ( sl[idx] )
          sl[idx]->raise();
        }
      }

    static unsigned short get_signal_id( const char *name )
      {
        return (unsigned short)-1;    // return invalid id in order for caller to print complete error msg (with entity name)
      }


    void clear_callbacks();
    void clear_script_callbacks();
    void clear_code_callbacks();
    void clear_script_callback(const stringx &name);

/*
    bool signal_was_raised(signal_list::size_t sig_id)
    {
      if ( signals == NULL )
        signals = construct_signal_list();
      signal_list& sl = *signals;
      return(sl[idx] != NULL && sl[idx]->raised());
    }
*/

  private:
    // Every descendant of signaller that expects to generate signals and has
    // defined its own local list of signal ids should implement this virtual
    // function for the construction of the signal list, so that it will reserve
    // exactly the number of signal pointers required, on demand.
    virtual signal_list* construct_signal_list() { return NEW signal_list( N_SIGNALS, (signal*)NULL ); }

  protected:
    // This virtual function, used only for debugging purposes, returns the
    // name of the given local signal
    virtual const char* get_signal_name( unsigned short idx ) const { return ""; }
  };


class signal_manager : public singleton
  {
  // SINGLETON
  public:
    // returns a pointer to the single instance
    DECLARE_SINGLETON( signal_manager )

  // Constructors (not public in a singleton)
  private:
    signal_manager();
    signal_manager( const signal_manager& );
    signal_manager& operator=( const signal_manager& );

  // Types
  public:
    typedef map< stringx, unsigned short > signal_id_map_t;
    typedef vector< signal* > signal_list;

  // Data
  private:
    signal_id_map_t signal_id_map;
    signal_list refresh_list;

  // Methods
  public:
    // return signaller-local index value for signal
    unsigned short get_id( const stringx& name ) const;
    // insert a NEW signal name and associated signaller-local index
    void insert( const stringx& name, unsigned short id );

    // create NEW signal consisting of logical AND of given signals
    signal* signal_AND( signal* a, signal* b ) const;
    // create NEW signal consisting of logical OR of given signals
    signal* signal_OR( signal* a, signal* b ) const;

    // this function is called when a signal object is affected such that it
    // will need to be reset at the end of the game frame (see app:tick)
    void needs_refresh( signal* s );
    // this function is called once per game frame to reset any signals that need it
    void do_refresh();

    void purge(); // PEH BETA LOCK
  };


#endif  // _SIGNAL_H
