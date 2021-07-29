// vm_thread.h
#ifndef _VM_THREAD_H
#define _VM_THREAD_H


#include "script_library_class.h"
#include "script_object.h"
#include "vm_stack.h"

#include <stack>
#include <set>

// turn this on when you want to profile script thread execution at runtime
#define THREAD_PROFILING 1

class vm_executable;
class region;
//!class character;
class script_callback;

class vm_thread
  {
  // Types
  protected:
    union argument_t
      {
      vm_num_t val;
      vm_str_t str;
      short word;
      char* sdr;
      script_library_class::function* lfr;
      vm_executable* sfr;
      unsigned binary;
      };

    enum flags_t
      {
      SUSPENDED =   0x0001,
      SUSPENDABLE = 0x0002,
      };

  // Data
  protected:
    // thread identification
    script_object::instance* inst;
    const vm_executable* ex;
    // thread flags
    unsigned int flags;
    // data stack
    vm_stack dstack;
    // program counter
    const unsigned short* PC;
    // program counter stack
    vector<const unsigned short*> PC_stack;
    // used when calling library functions
    script_library_class::function::entry_t entry;
    // if thread was spawned by an event callback, this points to the callback definition
    script_callback* my_callback;

    region * local_region;
//!    character * local_character;

    rational_t camera_priority;

    static unsigned int id_counter;
    unsigned int thread_id;

  // Constructors
  public:
    vm_thread();
    vm_thread(script_object::instance* i,const vm_executable* x,int sa);
    vm_thread(script_object::instance* i,const vm_executable* x,int sa,script_callback* cb);
    ~vm_thread();
    region * get_local_region() const {return local_region;}
    void set_local_region(region * reg) {local_region = reg; /*! local_character = NULL; !*/}
    void remove_from_local_region();
//!    character * get_local_character() const {return local_character;}
//!    void set_local_character(character * chr) {local_character = chr; local_region = NULL;}
    void remove_from_local_character();

  // Methods
  public:
    script_object::instance* get_instance() const { return inst; }
    const vm_executable* get_executable() const { return ex; }
    vm_stack& get_data_stack() { return dstack; }

    void set_flag( flags_t f, bool v ) { flags = v? (flags|f) : (flags&~f); }
    bool is_flagged( flags_t f ) const { return flags & f; }

    void set_suspended( bool v );
    bool is_suspended() const { return is_flagged( SUSPENDED ); }
    
    void set_suspendable( bool v );
    bool is_suspendable() const { return is_flagged( SUSPENDABLE ); }

    void set_camera_priority( rational_t pr );
    rational_t get_camera_priority() { return(camera_priority); }

    // Execute this thread until interrupted or terminated.
    // Return true if the thread should be killed.
    bool run();

    void spawn_subthread(vm_executable const * new_ex);

    void slf_error( const stringx& err );
    void slf_warning( const stringx& err );

    unsigned int get_thread_id() const     { return(thread_id); }


#if THREAD_PROFILING
    // counts number of operations performed on this thread in a given frame
    float prof_runtime;
    unsigned int prof_opcount;
#endif

  // Internal Methods
  protected:
    // data stack
    void dstack_init(int sa) { dstack.init(sa); }
    // program counter stack
    void push_PC() { PC_stack.push_back(PC); }
    void pop_PC();

    bool call_script_library_function( const argument_t& arg, const unsigned short* oldPC );
    void spawn_sub_thread( const argument_t& arg );
    void spawn_parallel_thread( const argument_t& arg );
    void create_event_callback( const argument_t& arg, bool one_shot );
    void create_static_event_callback( const argument_t& arg, bool one_shot );

  // Friends
  friend class script_object::instance;
  friend class script_object;            // remove me on sight -- gt
  };


#endif  // _VM_THREAD_H
