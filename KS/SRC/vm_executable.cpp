// vm_executable.cpp
#include "global.h"

#include "vm_executable.h"
#include "opcodes.h"
#include "script_object.h"
#include "script_library_class.h"
//!#include "character.h"
#include "wds.h"
#include "signals.h"

#ifdef _SCC  // functionality only used by script compiler
//#include "symbol.h"
//#include "constant.h"
//#include "classreg.h"
#endif

#include "oserrmsg.h"


// CLASS vm_executable

// Constructors

vm_executable::vm_executable( script_object* _owner )
:   owner( _owner ),
    name(),
    fullname(),
    parameters(),
    parms_stacksize( 0 ),
    static_func( false ),
    linked( false ),
    buffer( NULL ),
    buffer_len( 0 ),
    strings()
  {
  }

vm_executable::vm_executable(const vm_executable& b)
:   owner( b.owner ),
    name( b.name ),
    fullname( b.fullname ),
    parameters( b.parameters ),
    parms_stacksize( b.parms_stacksize ),
    static_func( b.static_func ),
    linked( b.linked ),
    buffer_len( b.buffer_len ),
    strings( b.strings )
  {
  buffer = NEW unsigned short[buffer_len];
  memcpy(buffer,b.buffer,buffer_len*2);
  }


vm_executable::~vm_executable()
  {
  _destroy();
  }


// Internal Methods

void vm_executable::_destroy()
  {
  if (buffer)
    delete[] buffer;
  }

void vm_executable::_clear()
  {
  parameters.resize(0);
  strings.resize(0);
  _destroy();
  linked = false;
  }

void vm_executable::_build_fullname()
  {
  fullname = name;
  fullname += "(";
  for (parms_list::const_iterator pli=parameters.begin(); pli!=parameters.end(); pli++)
    {
    if (pli != parameters.begin())
      fullname += ",";
    fullname += (*pli)->get_name();
    }
  fullname += ")";
  }

unsigned short vm_executable::_string_id(const stringx& s)
  {
  int idx = 0;
  vector<stringx const *>::iterator i=strings.begin();
  for (; i!=strings.end() && **i!=s; i++,idx++) continue;
  if (i == strings.end())
    {
    // string not already registered;
    // register NEW string
    stringx const * sptr;
    sptr = g_world_ptr->get_script_manager()->add_string(s);
    strings.push_back(sptr);
    }
  // return stringx index
  return idx;
  }

void vm_executable::_link(const script_manager& sm)
  {
  if ( !linked )  // cannot perform link more than once!
    {
    linked = true;
    unsigned short opword;
    opcode_t op;
    opcode_arg_t argtype;
    unsigned short dsize;
    for (unsigned short* PC=buffer; PC<buffer+buffer_len; )
      {
      // decode operation word (opcode,argtype,dsize flag)
      opword = *PC++;
      op = opcode_t(opword >> 8);
      argtype = opcode_arg_t(opword & OP_ARGTYPE_MASK);
      dsize = 4;
      if (opword & OP_DSIZE_FLAG)
        {
        // next word is dsize
        dsize = *PC++;
        }
      // link SDR, LFR, SFR, and CLV references to appropriate addresses
      switch (argtype)
        {
        case OP_ARG_NULL:   // no argument
          break;
        case OP_ARG_NUM:    // constant numeric value (4 bytes)
        case OP_ARG_NUMR:   // constant numeric value used in reverse position (4 bytes; only applies to non-commutative operations)
          PC += 2;
          break;
        case OP_ARG_STR:    // constant string value (4 bytes)
          {
          // first word is string_id, second is unused until after link
          unsigned short id = *PC++;
          PC++;
          // run-time value of argument is pointer to string
          unsigned addr = int(strings[id]);
          // value must be stored as consecutive words
          *(PC-2) = addr >> 16;
          *(PC-1) = addr & 0x0000FFFF;
          }
          break;
        case OP_ARG_WORD:   // constant integer value (2 bytes)
        case OP_ARG_PCR:    // PC-relative address (2 bytes)
        case OP_ARG_SPR:    // SP-relative address (2 bytes)
        case OP_ARG_POPO:   // stack contents (pop) plus offset (2 bytes)
          PC++;
          break;
        case OP_ARG_SDR:    // static data member reference (4 bytes)
          {
          // first word is script object string_id
          unsigned short id = *PC++;
          script_object* so = sm.find_object(*strings[id]);
          assert(so);
          // second word is static data offset
          unsigned short offset = *PC++;
          // run-time address of reference is address of script_object static
          // data block plus given offset
          assert( offset < so->get_static_data_size() );
          unsigned addr = int(so->get_static_data_buffer() + offset);
          // value must be stored as consecutive words
          *(PC-2) = addr >> 16;
          *(PC-1) = addr & 0x0000FFFF;
          }
          break;
        case OP_ARG_SFR:    // function member reference (4 bytes)
          {
          // first word is script object string_id
          unsigned short id = *PC++;
          script_object* so = sm.find_object(*strings[id]);
          assert(so);
          // second word is script function index
          unsigned short idx = *PC++;
          // run-time address of reference is address of given member function
          vm_executable& ex = (vm_executable&)so->get_func( idx );
          // make sure the referenced function gets linked (necessary because
          // we now support partial linking of script executable modules)
          ex._link( sm );
          unsigned addr = int(&ex);
          // value must be stored as consecutive words
          *(PC-2) = addr >> 16;
          *(PC-1) = addr & 0x0000FFFF;
          }
          break;
        case OP_ARG_LFR:    // script library function reference (4 bytes)
          {
          // first word is library class string_id
          unsigned short id = *PC++;
          const script_library_class* slc = slc_manager::inst()->find(*strings[id]);
          if (!slc)
            {
            stringx err;
            err="library class "+*strings[id]+" not found";
            error(err.c_str());
            }
          // second word is library function string_id
          id = *PC++;
          // run-time address of reference is address of given library function
          unsigned addr = int(slc->find(*strings[id]));
          if (!addr)
            {
            stringx err;
            err=stringx("library function ")+slc->get_name()+"::"+*strings[id]+" not found";
            error(err.c_str());
            }
          // value must be stored as consecutive words
          *(PC-2) = addr >> 16;
          *(PC-1) = addr & 0x0000FFFF;
          }
          break;
        case OP_ARG_CLV:    // class value reference (4 bytes)
          {
          // first word is library class string_id
          unsigned short id = *PC++;
          const script_library_class* slc = slc_manager::inst()->find(*strings[id]);
          if (!slc)
            {
            stringx err;
            err="library class "+*strings[id]+" not found";
            error(err.c_str());
            }
          // second word is class value string_id
          id = *PC++;
          // run-time address of reference is class instance (must be 4 bytes)
          assert(slc->get_size()==4);
          //((stringx *)strings[id])->to_upper();  // this is a string for entities, so it needed to be this all along...
          unsigned addr = slc->find_instance(*strings[id]);
          // value must be stored as consecutive words
          *(PC-2) = addr >> 16;
          *(PC-1) = addr & 0x0000FFFF;
          }
          break;
        case OP_ARG_SIG:    // global signal value (2 bytes)
        case OP_ARG_PSIG:   // member signal value (2 bytes)
          {
          // word is string_id of signal name
          unsigned short id = *PC++;
          // run-time value of argument is a non-unique 2-byte signal index local to the signaller
          const stringx& signame = *strings[id];
          id = signal_manager::inst()->get_id( signame );
          *(PC-1) = id;
          }
          break;
        default:
          assert(0);
        }
      }
    }
  }


// Friends

#ifndef NO_SERIAL_IN
void serial_in(chunk_file& io,vm_executable* x)
  {
  x->_clear();

  chunk_flavor cf;
  int i;

  // header
  serial_in(io,&cf);
  assert(cf == CHUNK_VM_EXECUTABLE);
  serial_in(io,&x->name);

  // parameters (optional)
  serial_in(io,&cf);
  int psize = 0;
  if (cf == CHUNK_PARAMETERS)
    {
    serial_in(io,&i);
    x->parameters.reserve(i);
    for (; i; i--)
      {
      stringx s;
      serial_in(io,&s);
      script_library_class* slc = slc_manager::inst()->find(s);
      if (!slc)
        {
        stringx err;
        err="library class "+s+" not found.";
        error(err.c_str());
        }
      x->parameters.push_back(slc);
      psize += slc->get_size();
      }
    serial_in(io,&cf);
    }

  // parameters stacksize
  assert(cf == CHUNK_PARMS_STACKSIZE);
  serial_in(io,&x->parms_stacksize);

  // function is static iff the given stacksize matches the total size of all
  // parameters
  if ( x->parms_stacksize == psize )
    x->static_func = true;

  // store full version of function name
  x->_build_fullname();

  // string list
  serial_in(io,&cf);
  assert(cf == CHUNK_NUM_STRINGS);
  serial_in(io,&i);
  if ( i )
    x->strings.reserve( i );
  for (; i; i--)
    {
    stringx s;
    serial_in(io,&s);
    stringx const * sptr = g_world_ptr->get_script_manager()->add_string(s);
    x->strings.push_back(sptr);
    }

  // executable code
  serial_in(io,&cf);
  assert(cf == CHUNK_CODESIZE);
  int len;
  serial_in(io,&len);
  x->buffer_len = len / 2;
  serial_in(io,&cf);
  assert(cf == CHUNK_CODE);
  x->buffer = NEW unsigned short[x->buffer_len];
  unsigned short* bp = x->buffer;
  for (i=x->buffer_len; i; i--,bp++)
    serial_in(io,bp);
  }
#endif

#if !defined(NO_SERIAL_OUT)
void serial_out(chunk_file& io,const vm_executable& x)
  {
  // header
  serial_out(io,CHUNK_VM_EXECUTABLE);
  serial_out(io,x.name);

  // parameters (optional)
  if (x.parameters.size())
    {
    serial_out(io,CHUNK_PARAMETERS);
    serial_out(io,int(x.parameters.size()));
    for (vm_executable::parms_list::const_iterator pi=x.parameters.begin(); pi!=x.parameters.end(); pi++)
      serial_out(io,(*pi)->get_name());
    }

  // parameters stacksize
  serial_out(io,CHUNK_PARMS_STACKSIZE);
  serial_out(io,x.parms_stacksize);

  // string list
  serial_out(io,CHUNK_NUM_STRINGS);
  serial_out(io,int(x.strings.size()));
  for (vector<stringx const *>::const_iterator si=x.strings.begin(); si!=x.strings.end(); si++)
    serial_out(io,**si);

  // executable code
  serial_out(io,CHUNK_CODESIZE);
  serial_out(io,x.buffer_len*2);
  serial_out(io,CHUNK_CODE);
  unsigned short* bp = x.buffer;
  for (int i=x.buffer_len; i; i--,bp++)
    serial_out(io,*bp);
  }
#endif
