// script_library_class.cpp
#include "global.h"

#include "script_library_class.h"
#include "script_object.h"
#include "vm_stack.h"
#include "vm_thread.h"
#include "oserrmsg.h"

//!DEFINE_AUTO_SINGLETON(slc_manager)
DEFINE_SINGLETON(slc_manager)

char* strdupcpp(const char* str);
extern void script_strings_deconstruct();

// CLASS script_library_class

// global script library class provides home for global library functions
script_library_class* slc_global = NULL;


// Constructors

int sizeof_funcs = sizeof(script_library_class::function_list);

script_library_class::script_library_class(const char* n,int sz,const char* p)
  : name(n),
    size(sz),
    parent_name(p),
    parent(NULL),
    funcs()
{
  // add myself to the script library class manager
  assert(slc_manager::is_inst());
  slc_manager::inst()->add(this);
}

// constructor provided for searching (see slc_manager::find())
script_library_class::script_library_class()
  : name(),
    size(0),
    parent_name(NULL),
    parent(NULL),
    funcs()
{
}

script_library_class::~script_library_class()
{
  for (function_list::iterator i=funcs.begin(); i!=funcs.end(); ++i)
    delete *i;
}


// Methods

const script_library_class::function* script_library_class::find(const char* n) const
{
  function bob(0);
  bob.name = strdupcpp(n);
  function_list::const_iterator fli = funcs.find(&bob);
  if (fli == funcs.end())
    return NULL;
  return *fli;
}


// SUBCLASS script_library_class::function

// Constructors

script_library_class::function::function(int dummy)
  : name("")
{
}

script_library_class::function::function(script_library_class* slc,const char* n)
{
  // add myself to the given script library class
  name = strdupcpp(n);
  slc->add(this);
}

script_library_class::function::function(const char* n)
{
  name = strdupcpp(n);
  // add myself to the global script library class
  //if(!slc_global) slc_global = NEW script_library_class("_global_slc",0);
  assert(slc_global);
  slc_global->add(this);
}

// constructor provided for script_library_class::find()
script_library_class::function::function(const stringx& n)
{
  name = strdupcpp(n.c_str());
}

script_library_class::function::~function()
{
  delete[] name;
}


// SINGLETON CLASS slc_manager

// Constructors (not public in a singleton)
slc_manager::slc_manager()
  : classes()
{
}

slc_manager::~slc_manager()
{
#ifdef KSCULL
  script_strings_deconstruct();
#endif
  for (class_list::iterator i=classes.begin(); i!=classes.end(); ++i)
    delete *i;
}


// Methods

void slc_manager::add(script_library_class* slc)
{
#ifdef DEBUG
  pair<class_list::iterator,bool> iret = classes.insert( slc );
  assert( iret.second );
#else
  classes.insert( slc );
#endif
}

// call this after static initialization, to establish parent links
void slc_manager::link_hierarchy()
{
  class_list::iterator i = classes.begin();
  class_list::iterator i_end = classes.end();
  for ( ; i!=i_end; ++i )
  {
    // for each class that has a parent_name, find that parent and link the pointer
    script_library_class* c = *i;
    if ( c->parent_name )
      c->parent = find( c->parent_name );
  }
}

script_library_class* slc_manager::find(const char* n) const
{
  // set up search argument
  // NOTE: we cannot call a public constructor here, since such would end up
  // calling slc_manager in an attempt to add the library class to the global list
  script_library_class slc;
  slc.name = n;
  class_list::const_iterator cli = classes.find(&slc);
  if (cli == classes.end())
    return NULL;
  return *cli;
}


// destroy library class with given name (if any)
void slc_manager::destroy( const stringx& n )
{
  script_library_class slc;
  slc.name = n;
  class_list::iterator cli = classes.find(&slc);
  if ( cli != classes.end() )
  {
    delete *cli;
    classes.erase( cli );
  }
}

void slc_manager::purge()
{
  class_list::iterator cli;
  for( cli = classes.begin(); cli != classes.end(); ++cli )
    (*cli)->purge();
}


// CLASS slc_script_object_t supports dummy slc entries for script objects, so
// that they may be referenced and linked at runtime.
// fill in for script language built-in type NUM

slc_script_object_t::slc_script_object_t(const script_manager& sm,const char* n)
  : script_library_class(n,4,NULL),
    sman(sm)
{
  sobj = sman.find_object(n);
  if (!sobj)
  {
    stringx err;
    err=stringx("script object ")+n+" not found\n";
    error(err.c_str());
  }
}

#ifndef NO_SERIAL_IN
// read a script object value (by id) from a stream
typedef script_object::instance* vm_script_object_t;  // vm_stack data representation
void slc_script_object_t::read_value(chunk_file& fs,char* buf)
{
  // read id
  stringx id;
  serial_in(fs,&id);
  // find script object instance and write value to buffer
  *(vm_script_object_t*)buf = (vm_script_object_t)find_instance(id);
}
#endif

// find named instance of script object
unsigned slc_script_object_t::find_instance(const stringx& n) const
{
  script_object::instance* soinst = sobj->find_instance(n);
  if (!soinst)
  {
    stringx err;
    err="script object instance "+n+" not found";
    error(err.c_str());
  }
  return (unsigned)soinst;
}


// CLASS slc_num_t supports script language built-in type NUM
slc_num_t* slc_num = NULL;

slc_num_t::slc_num_t(const char* n,int sz,const char* p)
  : script_library_class(n,sz,p)
{
}

#ifndef NO_SERIAL_IN
// read a num value from a stream
void slc_num_t::read_value(chunk_file& fs,char* buf)
{
  serial_in(fs,(vm_num_t*)buf);
}
#endif


// CLASS slc_str_t supports script language built-in type STR
slc_str_t* slc_str = NULL;

slc_str_t::slc_str_t(const char* n,int sz,const char* p)
  : script_library_class(n,sz,p)
{
}

// destructor needed to delete managed strings
slc_str_t::~slc_str_t()
{
  purge();
}

#ifndef NO_SERIAL_IN
// read a str value from a stream
void slc_str_t::read_value(chunk_file& fs,char* buf)
{
  // read NEW stringx and add to managed strings list
  stringx* s = NEW stringx;
  serial_in(fs,s);
  strings.push_back(s);
  // copy stringx pointer into buffer
  *(vm_str_t*)buf = s;
}
#endif

void slc_str_t::purge()
{
  for (vector<stringx*>::iterator i=strings.begin(); i!=strings.end(); ++i)
    delete *i;
  strings = vector<stringx*>();
}


///////////////////////////////////////////////////////////////////////////////
// Error message function for script library classes
///////////////////////////////////////////////////////////////////////////////
void verify_parms_integrity( script_library_class::function* the_function,
                             vm_stack* the_stack,
                             unsigned int* parms,
                             int parms_size )
{
  int i;
  for ( i=0; i<parms_size; ++i )
  {
    if ( parms[i] == UNINITIALIZED_SCRIPT_PARM )
    {
      // give bad parameter message <<<
      the_stack->get_thread()->slf_error( stringx("uninitialized parameters in call to ") + the_function->get_name() );
    }
  }
}
