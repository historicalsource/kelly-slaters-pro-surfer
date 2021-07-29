// script_object.cpp
#include "global.h"

#include "script_object.h"
#include "vm_thread.h"
#include "script_library_class.h"
#include "oserrmsg.h"
#include "chunkfile.h"
#include "profiler.h"
#include "debug.h"

#ifdef _SCC  // script compiler support
//#include "classreg.h"
//#include "symbol_list.h"
#include "vm_executable_vector.h"
#endif

#if _CONSOLE_ENABLE
#include "console.h"
#endif

#include "sl_debugger.h"

#if defined(TARGET_XBOX)
#include "ngl.h"
#endif /* TARGET_XBOX JIV DEBUG */

#define VM_STACKSIZE 256


// SUBCLASS script_object::instance

// Constructors

script_object::instance::instance(const stringx& n,int sz)
: name( n ),
  data( sz ),
  threads(),
  suspended( false )
{
}

script_object::instance::~instance()
{
  for ( thread_list::iterator i=threads.begin(); i!=threads.end(); ++i )
    delete *i;
  threads.resize(0);
}

// support for searches
script_object::instance::instance()
: name(),
  data(),
  threads(),
  suspended( false )
{
}

// Methods

vm_thread* script_object::instance::add_thread(const vm_executable* ex)
{
  vm_thread* nt = NEW vm_thread(this,ex,VM_STACKSIZE);
  assert(nt != NULL);
  threads.push_back(nt);
  // if this script object instance is suspended, we must suspend any thread
  // that is added here
  if ( suspended )
    nt->set_suspended( true );
  return nt;
}

vm_thread* script_object::instance::add_thread( const vm_executable* ex, const char* parms )
  {
  vm_thread* nt = add_thread( ex );
  assert(nt != NULL);

  if ( parms && !nt->get_data_stack().push( parms, ex->get_parms_stacksize() ) )
    error( get_name() + ": stack overflow spawning " + ex->get_fullname() );
  nt->PC = ex->get_start();
  return nt;
  }

// spawn a NEW thread via the given event callback
vm_thread* script_object::instance::add_thread( script_callback* cb, const vm_executable* ex, const char* parms )
  {
  vm_thread* nt = NEW vm_thread( this, ex, VM_STACKSIZE, cb );
  assert(nt != NULL);
  threads.push_back( nt );
  // if this script object instance is suspended, we must suspend any thread
  // that is added here
  if ( suspended )
    nt->set_suspended( true );
  if ( parms && !nt->get_data_stack().push( parms, ex->get_parms_stacksize() ) )
    error( get_name() + ": stack overflow spawning " + ex->get_fullname() );
  nt->PC = ex->get_start();
  return nt;
  }

void script_object::instance::kill_thread(const vm_executable* ex)
  {
  thread_list::iterator ti = threads.begin();
  while (ti != threads.end())
    {
    vm_thread* t = *ti;
    assert(t != NULL);
    if ( t->get_instance()==this && t->get_executable()==ex )
      {
      // found matching thread (same script object instance, same function);
      // kill it dead
      t->remove_from_local_region();
      t->remove_from_local_character();
//krPrintf("killing thread %s %s\n", t->ex->get_fullname().c_str(), t->inst->get_name().c_str());
      delete t;
      ti = threads.erase( ti );  // erase returns next value for iterator
      }
    else
      ++ti;
    }
  }

// execute all threads
#include "console.h"
void script_object::instance::run(bool ignore_suspended)
{
#if defined(TARGET_PC) && !defined(BUILD_BOOTABLE)
  if(g_script_debugger_running)
    g_sl_debugger.set_new_instance(this, threads.size());
#endif

  thread_list::iterator i = threads.begin();

  while ( i != threads.end() )
  {
    vm_thread* t = *i;
    assert(t != NULL);
    if ( ignore_suspended || !t->is_suspended() )
    {
      // execute thread (will run until finished or interrupted)
#if _CONSOLE_ENABLE && 0
      console_process("console_log c:\\console.txt", 0);
      console_log("execing thread %s %s\n", t->ex->get_fullname().c_str(), t->inst->get_name().c_str());
      console_process("console_log end", 0);
#endif

      if ( t->run() )
      {
        // thread has asked to be killed
        t->remove_from_local_region();
        t->remove_from_local_character();
//krPrintf("removing thread %s %s\n", t->ex->get_fullname().c_str(), t->inst->get_name().c_str());
        delete t;
        i = threads.erase( i );  // erase returns next value for iterator
      }
      else
        ++i;
    }
    else
      ++i;
  }
}

void script_object::instance::run_single_thread( vm_thread* t, bool ignore_suspended )
{
  if ( ignore_suspended || !t->is_suspended() )
  {
    // execute thread (will run until finished or interrupted)
    if ( t->run() )
    {
      // thread has asked to be killed
      t->remove_from_local_region();
      t->remove_from_local_character();
      delete t;
      // remove thread from list
//krPrintf("I shouldn't be called - thread %s %s\n", t->ex->get_fullname().c_str(), t->inst->get_name().c_str());

      thread_list::iterator i = threads.begin();

      for ( ; i!=threads.end(); ++i )
      {
        if ( *i == t )
        {
          threads.erase( i );  // erase returns next value for iterator
          break;
        }
      }
    }
  }
}


// suspend/unsuspend all threads
void script_object::instance::suspend()
  {
  thread_list::iterator i = threads.begin();
  thread_list::iterator i_end = threads.end();
  for ( ; i!=i_end; ++i )
    {
    vm_thread* t = *i;
    assert(t != NULL);
    t->set_suspended( true );
    }
  suspended = true;
  }

void script_object::instance::unsuspend()
  {
  thread_list::iterator i = threads.begin();
  thread_list::iterator i_end = threads.end();
  for ( ; i!=i_end; ++i )
    {
    vm_thread* t = *i;
    assert(t != NULL);
    t->set_suspended( false );
    }
  suspended = false;
  }


// for bug purposes; fixes a dangling pointer
void script_object::instance::clear_callback_references( script_callback *remove_me )
{
  thread_list::const_iterator i = threads.begin();
  thread_list::const_iterator i_end = threads.end();
  for ( ; i!=i_end; ++i )
  {
    vm_thread* t = *i;
    assert(t != NULL);
    if (t->my_callback == remove_me)
      t->my_callback = NULL;
  }
}

// for debugging purposes; dump information on all threads to a file
void script_object::instance::dump_threads( host_system_file_handle outfile ) const
  {
  thread_list::const_iterator i = threads.begin();
  thread_list::const_iterator i_end = threads.end();
  for ( ; i!=i_end; ++i )
    {
    vm_thread* t = *i;
    if (!t->is_suspended())
      {
      #if THREAD_PROFILING
      host_fprintf( outfile, "%s %s %f %i\n", name.c_str(), t->get_executable()->get_name().c_str(), t->prof_runtime, t->prof_opcount );
      #else
      host_fprintf( outfile, "%s %s\n", name.c_str(), t->get_executable()->get_name().c_str() );
      #endif
      }
    }
  }

bool script_object::instance::thread_exists(vm_thread* thread) const
{
  thread_list::const_iterator i = threads.begin();
  thread_list::const_iterator i_end = threads.end();
  for ( ; i!=i_end; ++i )
  {
    if((*i) == thread)
      return(true);
  }

  return(false);
}

bool script_object::instance::thread_exists(unsigned int thread_id) const
{
  thread_list::const_iterator i = threads.begin();
  thread_list::const_iterator i_end = threads.end();
  for ( ; i!=i_end; ++i )
  {
    if((*i)->thread_id == thread_id)
      return(true);
  }

  return(false);
}

bool script_object::instance::thread_exists(vm_thread* thread, unsigned int thread_id) const
{
  thread_list::const_iterator i = threads.begin();
  thread_list::const_iterator i_end = threads.end();
  for ( ; i!=i_end; ++i )
  {
    if((*i) == thread)
      return((*i)->thread_id == thread_id);
  }

  return(false);
}

#if _CONSOLE_ENABLE
  void script_object::instance::dump_threads_to_console() const
    {
    thread_list::const_iterator i = threads.begin();
    thread_list::const_iterator i_end = threads.end();
    for ( ; i!=i_end; ++i )
      {
      vm_thread* t = *i;
      if (!t->is_suspended())
        {
        #if THREAD_PROFILING
        console_log( "%s %s %f %i", name.c_str(), t->get_executable()->get_name().c_str(), t->prof_runtime, t->prof_opcount );
        #else
        console_log( "%s %s", name.c_str(), t->get_executable()->get_name().c_str() );
        #endif
        }
      }
    }
#endif

// CLASS script_object

// Constructors

script_object::script_object()
:   name(),
    global_object(false),
#ifndef _RELEASE
    static_symbols(),
    symbols(),
#endif
    static_data(),
    data_blocksize(0),
    funcs(),
    instances()
  {
  }


script_object::~script_object()
  {
  _destroy();
  }


// Methods

int script_object::find_func( const stringx& func_fullname ) const
  {
  int i = 0;
  vector<vm_executable*>::const_iterator fi = funcs.begin();
  vector<vm_executable*>::const_iterator fi_end = funcs.end();
  for ( ; fi!=fi_end; ++fi,++i )
  {
    if ( (*fi)->get_fullname() == func_fullname )
      return i;
  }
  return -1;
  }

// return index of function corresponding to given PC (-1 if not found)
int script_object::find_func_by_address( const unsigned short* PC ) const
{
  int i = 0;
  vector<vm_executable*>::const_iterator fi = funcs.begin();
  vector<vm_executable*>::const_iterator fi_end = funcs.end();
  for ( ; fi!=fi_end; ++fi,++i )
  {
    vm_executable* ex = *fi;
    if ( (uint32)PC>=(uint32)ex->get_start() && (uint32)PC<(uint32)(ex->get_start()+ex->get_size()) )
      return i;
  }
  return -1;
}

void script_object::link(const script_manager& sm)
  {
  vector<vm_executable*>::iterator xi;
  for ( xi=funcs.begin(); xi!=funcs.end(); ++xi )
    (*xi)->_link( sm );
  }

script_object::instance* script_object::create_auto_instance()
{
  script_object::instance* inst = NULL;
  // look for default constructor
  const vm_executable& con = get_func(0);
  if ( con.get_name() == get_name() &&
       (is_global_object()? con.get_parms_stacksize()==0 : con.get_parms_stacksize()==4) )
  {
    // constructor found; create auto instance and thread to run it
    inst = NEW instance( "__auto", data_blocksize );
    instances.push_front( inst );  // global instance must run first!
    vm_thread* con_thread = inst->add_thread( &con );
    if ( !is_global_object() )
    {
      // push implicit THIS pointer
      con_thread->get_data_stack().push( (char*)&inst, 4 );
    }
  }
  return inst;
}

script_object::instance* script_object::find_instance( const stringx& name ) const
{
  instance_list::const_iterator i = instances.begin();
  instance_list::const_iterator i_end = instances.end();
  for ( ; i!=i_end; ++i )
  {
    instance* inst = *i;
    if ( inst->get_name() == name )
      return inst;
  }
  return NULL;
}


script_object::instance* script_object::add_instance(const stringx& inst_name,chunk_file* fs_ptr,
                                                     char* implicit_parms_buffer,int parm_count,
                                                     const vm_executable::parms_list* check_parms)
  {
  error_context::inst()->push_context(inst_name);

  instance* inst = NEW instance(inst_name,data_blocksize);
  _add(inst);

//warning(inst_name + "  ADD INSTANCE");
if( inst_name == stringx("DOOR_BRIDGE_GATE") )
{
int x = 1;
++x;
}

  // if we have an fs_ptr, build the parameter list
  if (fs_ptr)
    {
    // find constructor
    const vm_executable& con = get_func(0);
    if (con.get_name() != name)
      {
      // can't create an instance if it doesn't have a constructor
      stringx err;
      err=fs_ptr->get_name()+": "+"script object "+name+" has no constructor; cannot create instance";
      error(err.c_str());
      }
    // create constructor thread
    vm_thread* con_thread = inst->add_thread(&con);
    // read parameters into local buffer
    const vm_executable::parms_list& con_pl = con.get_parameters();
    char buf[256];
    char* bp = buf;
    // first (implicit) parameter is always the instance's THIS pointer
    *(script_object::instance**)bp = inst;
    bp += 4;
    int i = 0;
    vm_executable::parms_list::const_iterator cpi = 0;
    if (check_parms)
      cpi = check_parms->begin();
    for (vm_executable::parms_list::const_iterator con_pli=con_pl.begin(); con_pli!=con_pl.end(); ++con_pli,++i)
      {
      if (parm_count == 0)
        (*con_pli)->read_value(*fs_ptr,bp);
      else
        {
        if (check_parms)
          {
          assert(cpi!=check_parms->end());
          // try to match constructor parameter type with corresponding type
          // expected by application (check_parms); a constructor parameter
          // type that is a descendant of the expected type is acceptable
          if (!(*cpi)->is_equal_or_descendent(*con_pli))
            {
            // constructor parameter type does not match input parameter type
            stringx err;
            err=fs_ptr->get_name()+": implicit parameter "+itos(i)+" type mismatch in constructor "+inst_name+"(); ";
            err+=(*con_pli)->get_name()+" is not a descendant of "+(*cpi)->get_name();
            error(err.c_str());
            }
          ++cpi;
          }
        // read off one of the implicit parameters
        memcpy(bp,implicit_parms_buffer, (*con_pli)->get_size());
        implicit_parms_buffer += (*con_pli)->get_size();
        parm_count--;
        }
      bp += (*con_pli)->get_size();
      }
    // push data from buffer onto constructor thread's data stack
    con_thread->get_data_stack().push(buf,bp-buf);
    }

  error_context::inst()->pop_context();
  return inst;
  }


// Build and add a NEW instance of this script object and initialize a
// constructor thread with the instance (implicit THIS) pointer plus the given
// stack data.
script_object::instance*
script_object::add_instance( const stringx& inst_name,
                             char* constructor_parms_buffer )
  {
  instance* inst = NEW instance( inst_name, data_blocksize );
  _add( inst );

  // find constructor
  const vm_executable& con = get_func(0);
  assert( con.get_name() == name );
  // create constructor thread
  vm_thread* con_thread = inst->add_thread( &con );
  // push implicit THIS pointer
  con_thread->get_data_stack().push( (char*)&inst, 4 );
  // push additional parameters
  int parmsize = con.get_parms_stacksize();
  if ( !con.is_static() )
    parmsize -= 4;
  con_thread->get_data_stack().push( constructor_parms_buffer, parmsize );

  return inst;
  }


// add a thread to run the given member function on the given instance
vm_thread* script_object::add_thread( instance* inst, int fidx )
{
  // create thread for given member function
  assert(fidx<(int)funcs.size());
  vm_thread* nt = inst->add_thread( funcs[fidx] );
  // push implicit THIS parameter (not necessary for static member functions,
  // but since this is a NEW thread it won't hurt anything)
  nt->get_data_stack().push((char*)&inst,4);
  return nt;
}


bool script_object::has_threads() const
{
  instance_list::const_iterator i = instances.begin();
  instance_list::const_iterator i_end = instances.end();
  for ( ; i!=i_end; ++i )
  {
    instance* inst = *i;
    if ( inst->has_threads() )
      return true;
  }
  return false;
}


void script_object::check_all_instances()
{
/////////////////// I am debugging code, why am I still here?  kill me please! -- GT
  for ( instance_list::iterator i=instances.begin(); i!=instances.end(); ++i )
  {
    instance* inst = *i;
    instance::thread_list::iterator j = inst->threads.begin();
    while ( j != inst->threads.end() )
    {
      vm_thread* t = *j;
      assert(t != NULL);

      // check threads for goodness
      if (t->PC_stack.begin() > t->PC_stack.end())
      {
        nglPrintf("Bad thread %s %s\n\n", t->ex->get_fullname().c_str(), t->inst->get_name().c_str());
        assert(false);
      }
      ++j;
    }
  }
/////////////////////////////////////
}

// execute all threads on all instances of this object
void script_object::run( bool ignore_suspended )
{
#if defined(TARGET_PC) && !defined(BUILD_BOOTABLE)
  if(g_script_debugger_running)
    g_sl_debugger.set_new_object(this, instances.size());
#endif

// remove me on sight!!! debugging only
//check_all_instances();

  for ( instance_list::iterator i=instances.begin(); i!=instances.end(); ++i )
  {
    instance* inst = *i;
    inst->run( ignore_suspended );
  }
}


// for debugging purposes; dump information on all threads to a file
void script_object::dump_threads( host_system_file_handle outfile ) const
{
  instance_list::const_iterator i = instances.begin();
  instance_list::const_iterator i_end = instances.end();
  for ( ; i!=i_end; ++i )
  {
    instance* inst = *i;
    inst->dump_threads( outfile );
  }
}


#if _CONSOLE_ENABLE
void script_object::dump_threads_to_console() const
{
  instance_list::const_iterator i = instances.begin();
  instance_list::const_iterator i_end = instances.end();
  for ( ; i!=i_end; ++i )
  {
    instance* inst = *i;
    inst->dump_threads_to_console();
  }
}
#endif

// Internal Methods

void script_object::_destroy()
{
  instance_list::iterator ii;
  for ( ii=instances.begin(); ii!=instances.end(); ++ii )
    delete *ii;
  vector<vm_executable*>::iterator vi;
  for ( vi=funcs.begin(); vi!=funcs.end(); ++vi )
    delete *vi;
}

void script_object::_clear()
{
  _destroy();
  static_symbols.resize(0);
  symbols.resize(0);
  static_data.clear();
  funcs.resize(0);
}

void script_object::_add( instance* inst )
{
  #ifndef BUILD_BOOTABLE
  pair<instance_name_list::iterator,bool> iret = instance_names.insert( inst->get_name() );
  if ( !iret.second )
  {
    error( "Duplicate script instance name " + inst->name );
  }
  #endif
  instances.push_back( inst );
}


// Friend functions

#ifndef NO_SERIAL_IN
void serial_in(chunk_file& io,script_object* so)
  {
  so->_clear();

  chunk_flavor cf;
  int i;

  // global object?
  serial_in(io,&cf);
  if (cf == CHUNK_GLOBAL)
    so->global_object = true;

  // static symbols (optional)
  serial_in(io,&cf);
  if (cf == CHUNK_STATIC_SYMBOLS)
    {
    serial_in(io,&i);
    for (; i; i--)
      {
      vm_symbol s;
      serial_in(io,&s);
#ifndef _RELEASE
      so->static_symbols.push_back(s);
#endif
      }
    serial_in(io,&cf);
    }

  // static data blocksize
  assert(cf == CHUNK_STATIC_BLOCKSIZE);
  int static_blocksize;
  serial_in(io,&static_blocksize);
  so->static_data.init(static_blocksize);

  // non-static symbols (optional)
  serial_in(io,&cf);
  if (cf == CHUNK_DATA_SYMBOLS)
    {
    serial_in(io,&i);
    for (; i; i--)
      {
      vm_symbol s;
      serial_in(io,&s);
#ifndef _RELEASE
      so->symbols.push_back(s);
#endif
      }
    serial_in(io,&cf);
    }

  // non-static data blocksize
  assert(cf == CHUNK_DATA_BLOCKSIZE);
  serial_in(io,&so->data_blocksize);

  // functions
  serial_in(io,&cf);
  assert(cf == CHUNK_FUNCS);
  serial_in(io,&i);

  so->funcs.reserve(i);
  for (; i; i--)
    {
    vm_executable* x = NEW vm_executable( so );
    serial_in( io, x );
    so->funcs.push_back( x );
    }

  // A global script object automatically creates a thread to run its own
  // default constructor (if any).
  if ( so->is_global_object() )
    so->create_auto_instance();
  }
#endif

#if !defined(NO_SERIAL_OUT)
void serial_out(chunk_file& io,const script_object& so)
{
  // begin script object chunk
  serial_out(io,so.name);

  // global object?
  serial_out(io,so.is_global_object()?CHUNK_GLOBAL:CHUNK_STANDARD);

  vm_symbol_list::const_iterator sli;

  // static symbols (optional)
  if ( !so.static_symbols.empty() )
  {
    serial_out(io,CHUNK_STATIC_SYMBOLS);
    serial_out(io,int(so.static_symbols.size()));
    for ( sli=so.static_symbols.begin(); sli!=so.static_symbols.end(); ++sli )
      serial_out( io, *sli );
  }

  // static blocksize
  serial_out(io,CHUNK_STATIC_BLOCKSIZE);
  serial_out(io,so.static_data.size());

  // non-static symbols (optional)
  if ( !so.symbols.empty() )
  {
    serial_out(io,CHUNK_DATA_SYMBOLS);
    serial_out(io,int(so.symbols.size()));
    for ( sli=so.symbols.begin(); sli!=so.symbols.end(); ++sli )
      serial_out(io,*sli);
  }

  // non-static blocksize
  serial_out(io,CHUNK_DATA_BLOCKSIZE);
  serial_out(io,so.data_blocksize);

  // functions
  serial_out(io,CHUNK_FUNCS);
  serial_out(io,int(so.funcs.size()));
  vector<vm_executable*>::const_iterator sfi;
  for ( sfi=so.funcs.begin(); sfi!=so.funcs.end(); ++sfi )
    serial_out( io, **sfi );

  serial_out(io,stringx(sendl));
}
#endif


// CLASS script_manager

// Constructors

script_manager::script_manager()
: script_objects(),
  script_objects_by_name()
{
  time_inc = 0.0f;
}

script_manager::~script_manager()
{
  _destroy();
}


// Methods

void script_manager::clear()
{
  _destroy();
}

script_object* script_manager::find_object( const stringx& name ) const
{
  name_sobj_map::const_iterator i = script_objects_by_name.find( name );
  if ( i == script_objects_by_name.end() )
    return NULL;
  else
    return (*i).second;
}


// return pointer to executable that corresponds to given PC (NULL if not found)
const vm_executable* script_manager::find_function_by_address( const unsigned short* PC ) const
{
  sobj_list::const_iterator i = script_objects.begin();
  sobj_list::const_iterator i_end = script_objects.end();
  for ( ; i!=i_end; ++i )
  {
    script_object* so = *i;
    int f = so->find_func_by_address( PC );
    if ( f != -1 )
      return &so->get_func( f );
  }
  return NULL;
}


#ifndef NO_SERIAL_IN
void script_manager::load(const char* filename)
  {
  chunk_file io;
  chunk_flavor cf;
  io.open(filename);
  serial_in( io, &cf );
  if ( cf != chunk_flavor("scrobjs") )
    error( stringx(filename) + ": bad format; file must be updated" );
  for (serial_in(io,&cf); cf!=CHUNK_END; serial_in(io,&cf))
    {
    if (cf == CHUNK_SCRIPT_OBJECT)
      {
      script_object* so = NEW script_object;

if(so==0)
warning(stringx(filename)+" NULL");

      // begin script object chunk
      serial_in( io, &so->name );
      serial_in( io, so );
      _add( so );
      }
    else
      error( stringx(filename) + ": bad format; file must be updated" );
    }
  }
#endif

void script_manager::link()
{
  sobj_list::const_iterator i = script_objects.begin();
  sobj_list::const_iterator i_end = script_objects.end();
  for ( ; i!=i_end; ++i )
    (*i)->link( *this );
}

// execute all threads on all script object instances
void script_manager::run(time_value_t t, bool ignore_suspended )
{
#ifndef PROJECT_KELLYSLATER

  time_inc = t;

#if defined(TARGET_PC) && !defined(BUILD_BOOTABLE)
  if(g_script_debugger_running)
    g_sl_debugger.set_new_frame(script_objects.size());
#endif

  sobj_list::const_iterator i = script_objects.begin();
  sobj_list::const_iterator i_end = script_objects.end();
  for ( ; i!=i_end; ++i )
  {
    script_object* so = *i;
    if ( so->get_num_instances() )
    {
      so->run( ignore_suspended );
    }
  }
#endif
}

// check all script objects, debugging code, please remove me if I'm still here
void script_manager::check_all_objects( )
{
  sobj_list::const_iterator i = script_objects.begin();
  sobj_list::const_iterator i_end = script_objects.end();
  for ( ; i!=i_end; ++i )
  {
    script_object* so = *i;

//    if ( so->get_num_instances() )
    if (so != NULL)
    {
      so->check_all_instances();
    }
  }
}

bool script_manager::has_threads() const
{
  sobj_list::const_iterator i = script_objects.begin();
  sobj_list::const_iterator i_end = script_objects.end();
  for ( ; i!=i_end; ++i )
  {
    script_object* so = *i;
    if ( so->has_threads() )
      return true;
  }
  return false;
}


// for debugging purposes; dump information on all threads to a file
void script_manager::dump_threads() const
{
  host_system_file_handle outfile = host_fopen( "\\steel\\dump\\scriptdump.txt", HOST_WRITE );
  host_fprintf( outfile, "instance thread time ops\n" );
  sobj_list::const_iterator i = script_objects.begin();
  sobj_list::const_iterator i_end = script_objects.end();
  for ( ; i!=i_end; ++i )
  {
    script_object* so = *i;
    so->dump_threads( outfile );
  }
  host_fclose( outfile );
}

#if _CONSOLE_ENABLE
void script_manager::dump_threads_to_console() const
{
  console_log("" );
  console_log("instance   thread   time   ops" );

  sobj_list::const_iterator i = script_objects.begin();
  sobj_list::const_iterator i_end = script_objects.end();
  for ( ; i!=i_end; ++i )
  {
    script_object* so = *i;
    so->dump_threads_to_console();
  }
}
#endif

stringx const* script_manager::add_string( const stringx& s)
{
  return &(*(string_set.insert(s).first));
}

// Internal Methods

void script_manager::_destroy()
{
  sobj_list::const_iterator i = script_objects.begin();
  sobj_list::const_iterator i_end = script_objects.end();
  for ( ; i!=i_end; ++i )
  {
    // destroy fake script_library_class entry for this script object
    slc_manager::inst()->destroy( (*i)->name );
    delete *i;
  }
  script_objects.resize(0);
  script_objects_by_name.clear();
  string_set.clear();
}

void script_manager::_add( script_object* so )
{
  // add to name (searchable) map
  typedef script_object* soptr_t;
  soptr_t& mapped_so = script_objects_by_name[ so->get_name() ];
  assert( mapped_so == NULL );
  mapped_so = so;
  // add to simple list
  if ( so->is_global_object() )
    script_objects.push_front( so );  // global script object must run first!
  else
    script_objects.push_back( so );
  // add fake script_library_class entry to support script object linkage
  script_library_class* new_slc = NEW slc_script_object_t( *this, so->name.c_str() );
  // do a fake call on the NEW entry to make GNUC happy
  new_slc->get_size();
}
