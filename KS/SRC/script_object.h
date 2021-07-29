// script_object.h
#ifndef _SCRIPT_OBJECT_H
#define _SCRIPT_OBJECT_H


#include "stringx.h"
#include "vm_symbol_list.h"
#include "vm_executable.h"
#include "so_data_block.h"
#include "chunkfile.h"
#include "ostimer.h"
#include <set>
#include <map>

class classreg;
class vm_thread;
class script_callback;


enum
  {
  USE_SUSPENDED = 0,
  IGNORE_SUSPENDED = 1
  };


class script_object
  {
  // Types
  public:
    class instance
      {
      // Types
      typedef list<vm_thread*> thread_list;

      // Data
      protected:
        stringx name;
        so_data_block data;
        thread_list threads;
        bool suspended;

      // Constructors
      public:
        instance(const stringx& n,int sz);
        ~instance();
      protected:
        instance();

      // Methods
      public:
        const stringx& get_name() const { return name; }
        char* get_buffer() const { return data.get_buffer(); }

        // thread management
        const thread_list& get_threads() const { return threads; }
        vm_thread* add_thread(const vm_executable* ex);
        vm_thread* add_thread( const vm_executable* ex, const char* parms );

        // spawn a NEW thread via the given event callback
        vm_thread* add_thread( script_callback* cb, const vm_executable* ex, const char* parms );

        vm_thread* spawn_subthread(const vm_executable* ex);
        void kill_thread(const vm_executable* ex);
        bool has_threads() const { return threads.size()? true : false; }
        // execute all threads
        void run(bool ignore_suspended);
        // run a single thread
        void run_single_thread( vm_thread* t, bool ignore_suspended );

        void suspend();
        void unsuspend();

        // for debugging purposes; dump information on all threads to a file
        void dump_threads( host_system_file_handle outfile ) const;
        void clear_callback_references( script_callback *remove_me );

        bool thread_exists(vm_thread* thread) const;
        bool thread_exists(unsigned int thread_id) const;
        bool thread_exists(vm_thread* thread, unsigned int thread_id) const;

#if _CONSOLE_ENABLE
        void dump_threads_to_console() const;
#endif

      // Friends
      friend class script_object;
      };

    // instance management
    typedef list<instance*> instance_list;
    typedef set<stringx> instance_name_list;

  // Data
  protected:
    stringx name;
    bool global_object;

#ifndef _RELEASE
    // debug info
    vm_symbol_list static_symbols;
    vm_symbol_list symbols;
#endif

    // static data
    so_data_block static_data;
    // non-static data
    int data_blocksize;

    // script functions
    vector<vm_executable*> funcs;

    // instance management
    instance_list instances;

    #ifndef BUILD_BOOTABLE
    instance_name_list instance_names;
    #endif

  // Constructors
  public:
    script_object();
    //script_object(const script_object& b);
    script_object(const classreg& cl,bool debuginfo);  // compiler support
    ~script_object();

  // Methods
  public:
void check_all_instances(); // debugging code, remove me please!!! -GT
    const stringx& get_name() const { return name; }
    bool is_global_object() const { return global_object; }
    char* get_static_data_buffer() const { return static_data.get_buffer(); }
    int get_static_data_size() const { return static_data.size(); }
    const vm_executable& get_func(int i) const { return *funcs[i]; }
    int find_func(const stringx& func_fullname) const;

    // return index of function corresponding to given PC (-1 if not found)
    int find_func_by_address( const unsigned short* PC ) const;

    // Link all SDR, SFR, and LFR references in the script functions to the
    // appropriate run-time addresses found via the given script manager.
    void link(const script_manager& sm);

    // succeeds only if script object has a default constructor
    instance* create_auto_instance();

    // instance management
    instance* find_instance(const stringx& name) const;
    instance* add_instance(const stringx& inst_name, chunk_file* fs_ptr=NULL,
                           char* implict_parms_buffer=NULL, int parm_count=0,
                           const vm_executable::parms_list* check_parms=NULL);
    instance* add_instance( const stringx& inst_name,
                            char* constructor_parms_buffer );

    int get_num_instances() const { return instances.size(); }

    vm_thread* add_thread(instance* inst,int fidx);

    bool has_threads() const;

    // execute all threads on all instances of this script object
    void run(bool ignore_suspended);

    // for debugging purposes; dump information on all threads to a file
    void dump_threads( host_system_file_handle outfile ) const;

#if _CONSOLE_ENABLE
    void dump_threads_to_console() const;
#endif

  // Internal Methods
  protected:
    void _destroy();
    void _clear();

    void _add(script_object::instance* inst);

  // Friends
#if !defined(NO_SERIAL_IN)
  friend void serial_in(chunk_file& io,script_object* so);
#endif

#if !defined(NO_SERIAL_OUT)
  friend void serial_out(chunk_file& io,const script_object& so);
#endif
  friend class script_manager;
  };

const chunk_flavor CHUNK_NUM_SCRIPT_OBJECTS   ("Nscrobjs");
const chunk_flavor CHUNK_SCRIPT_OBJECT        ("scrobj");
const chunk_flavor CHUNK_STATIC_SYMBOLS       ("Nstatic");
const chunk_flavor CHUNK_STATIC_BLOCKSIZE     ("statsize");
const chunk_flavor CHUNK_DATA_SYMBOLS         ("Ndata");
const chunk_flavor CHUNK_DATA_BLOCKSIZE       ("datasize");
const chunk_flavor CHUNK_FUNCS                ("Nfuncs");
const chunk_flavor CHUNK_GLOBAL               ("global");
const chunk_flavor CHUNK_STANDARD             ("standard");


// CLASS script_manager:
// Manages a list of script objects; provides a method for linking compiled
// member references to the corresponding run-time addresses.
class script_manager
  {
  // Types
  public:
    class sobj_less : public binary_function<const script_object*,const script_object*,bool>
      {
      public:
        bool operator()(const script_object* a,const script_object* b) const
          {
          return (a->get_name() < b->get_name());
          }
      };
    typedef list<script_object*> sobj_list;
    typedef map<stringx,script_object*> name_sobj_map;
    typedef set<stringx> string_set_t;

  // Data
  protected:
    sobj_list script_objects;
    name_sobj_map script_objects_by_name;
    string_set_t string_set;
    time_value_t time_inc;

  // Constructors
  public:
    script_manager();
    ~script_manager();

  // Methods
  public:
void check_all_objects( ); // debugging code, please remove me!  --GT
    void clear();
    script_object* find_object(const stringx& name) const;

    // return pointer to executable that corresponds to given PC (NULL if not found)
    const vm_executable* find_function_by_address( const unsigned short* PC ) const;

    void load(const char* filename);

    // Link all SDR, SFR, and LFR references found in the managed script object
    // member functions to the appropriate run-time addresses.
    // NOTE:  This should be performed once only, after all script objects
    // have been loaded and added to the manager.  (We are forced to make load
    // and link separate steps in order to allow linking to script object
    // instances.)
    void link();

    // execute all threads on all script object instances
    void run(time_value_t t = 0.0f, bool ignore_suspended = USE_SUSPENDED);

    inline time_value_t get_time_inc() const { return(time_inc); }

    bool has_threads() const;

    // for debugging purposes; dump information on all threads to a file
    void dump_threads() const;

#if _CONSOLE_ENABLE
    void dump_threads_to_console() const;
#endif

    stringx const * add_string(stringx const & st);

  // Internal Methods
  protected:
    void _destroy();
    void _clear();
    void _add(script_object* so);
  };

#define SCRIPT_TIME_INC (g_world_ptr->get_script_manager()->get_time_inc())

#endif  // _SCRIPT_OBJECT_H
