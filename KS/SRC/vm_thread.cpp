// vm_thread.cpp
#include "global.h"

#include "vm_thread.h"
#include "vm_executable.h"
#include "opcodes.h"
#include "script_object.h"
#include "oserrmsg.h"
#include "debug.h"
#include "ostimer.h"
#include "terrain.h"
#include "signals.h"
#include "app.h"
//!#include "character.h"
#include "wds.h"

#include "sl_debugger.h"

// CLASS vm_thread

// Constructors

unsigned int vm_thread::id_counter = 0;

vm_thread::vm_thread()
  : inst(NULL),
    ex(NULL),
    flags( vm_thread::SUSPENDABLE ),
    dstack(),
    PC(NULL),
    PC_stack(),
    entry(script_library_class::function::FIRST_ENTRY)
{
  PC_stack.reserve(8);
  local_region = NULL;
//!  local_character = NULL;
  #if THREAD_PROFILING
  prof_runtime = 0;
  prof_opcount = 0;
  #endif
  my_callback = NULL;
  camera_priority = 0;

  thread_id = ++id_counter;
}

#ifdef TARGET_XBOX
#pragma warning( disable : 4355 )  // 'this' used in base member initializer list (why is that wrong? -JDF)
// According to VC documentation, "this" is not guaranteed to give a valid pointer until after the constructor
// runs.  But it seems to be working okay.  (dc 05/21/02)
#endif

vm_thread::vm_thread(script_object::instance* i,const vm_executable* x,int sa)
  : inst(i),
    ex(x),
    flags( vm_thread::SUSPENDABLE ),
    dstack(sa,this),
    PC(ex->get_start()),
    PC_stack(),
    entry(script_library_class::function::FIRST_ENTRY)
{
  PC_stack.reserve(8);
  local_region = NULL;
//!  local_character = NULL;
  #if THREAD_PROFILING
  prof_runtime = 0;
  prof_opcount = 0;
  #endif
  my_callback = NULL;
  camera_priority = 0;

  thread_id = ++id_counter;
}

// create a thread spawned via the given event callback
vm_thread::vm_thread(script_object::instance* i,const vm_executable* x,int sa,script_callback* cb)
  : inst(i),
    ex(x),
    flags( vm_thread::SUSPENDABLE ),
    dstack(sa,this),
    PC(ex->get_start()),
    PC_stack(),
    entry(script_library_class::function::FIRST_ENTRY)
{
  PC_stack.reserve(8);
  local_region = NULL;
//!  local_character = NULL;
  #if THREAD_PROFILING
  prof_runtime = 0;
  prof_opcount = 0;
  #endif
  my_callback = cb;
  // by default, callback is disabled until this thread terminates
  cb->disable();
  camera_priority = 0;

  thread_id = ++id_counter;
}

vm_thread::~vm_thread()
{
  // if this thread was spawned via an event callback, re-enable the callback here
  if ( my_callback )
    my_callback->enable();
}


// Methods

void vm_thread::set_suspended( bool v )
{
  if ( !v || is_suspendable() )
    set_flag( SUSPENDED, v );
}

void vm_thread::set_suspendable( bool v )
{
  set_flag( SUSPENDABLE, v);
  if ( !v )
    set_suspended( false );
}

// Execute this thread until interrupted or terminated.
// Return true if the thread should be killed.
bool vm_thread::run()
{
#if defined(TARGET_PC) && !defined(BUILD_BOOTABLE)
  if(g_script_debugger_running)
  {
    g_sl_debugger.set_new_thread(this);
    g_sl_debugger.wait_thread();
  }
#endif
/*
static stringx ex_name;  // for debugging, remove me if I'm still here.
static stringx inst_name;// for debugging, remove me if I'm still here.
if (ex_name != ex->get_fullname() || inst_name != inst->get_name())
{
  ex_name = ex->get_fullname();
  inst_name = inst->get_name();

  nglPrintf("Thread %s %s\n\tPC 0x%p stack(0x%p to 0x%p) size %d\n", this->ex->get_fullname().c_str(), this->inst->get_name().c_str(),
       this->PC, this->PC_stack.begin(), this->PC_stack.end(), this->PC_stack.size());
}
*/
assert(PC_stack.end() >= PC_stack.begin());

  const unsigned short* oldPC;
  unsigned short opword;
  unsigned short dsize;
  opcode_t op;
  opcode_arg_t argtype;
  argument_t arg;

//	debug_print(  "enter vm_thread::run" );

  #if THREAD_PROFILING
  hires_clock_t runtime;
  prof_opcount = 0;  // counts number of operations performed in a given frame on this thread
  #endif

  bool running = true;
  bool kill_me = false;
  while (running)
  {
    #if THREAD_PROFILING
    ++prof_opcount;
    #endif
    oldPC = PC;
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
    // fill argument(s)
    switch (argtype)
    {
    case OP_ARG_NULL:   // no argument
      break;
    case OP_ARG_NUM:    // constant numeric value (4 bytes)
    case OP_ARG_NUMR:   // constant numeric value used in reverse position (4 bytes; only applies to non-commutative operations)
    case OP_ARG_STR:    // const string value (4 bytes)
      {
        // value must be extracted from two words
        arg.binary = (*PC++) << 16;
        arg.binary += *PC++;
        // result will be read as:
        //   NUM,NUMR - arg.val
        //   STR      - arg.str
      }
      break;
    case OP_ARG_SIG:    // global signal_id (2 bytes)
    case OP_ARG_PSIG:   // member signal_id (2 bytes)
    case OP_ARG_WORD:   // constant integer value (2 bytes)
    case OP_ARG_PCR:    // PC-relative address (2 bytes)
    case OP_ARG_SPR:    // SP-relative address (2 bytes)
    case OP_ARG_POPO:   // stack contents (pop) plus offset (2 bytes)
      // word argument
      arg.word = *PC++;
      break;
    case OP_ARG_SDR:    // static data member reference (4 bytes)
    case OP_ARG_SFR:    // script function reference (4 bytes)
    case OP_ARG_LFR:    // library function reference (4 bytes)
    case OP_ARG_CLV:    // class value reference (4 bytes)
      {
        // value must be extracted from two words
        arg.binary = (*PC++) << 16;
        arg.binary += *PC++;
        // result will be read as:
        //   for SDR - arg.sdr
        //   for SFR - arg.sfr
        //   for LFR - arg.lfr
        //   for CLV - arg.binary
      }
      break;
    default:
      assert(0);
    }

assert(PC_stack.end() >= PC_stack.begin());

    // execute decoded operation
    switch (op)
    {
    case OP_ADD:  // Add                     NULL,NUM
      assert(dsize==4);
      switch (argtype)
      {
      case OP_ARG_NULL:   // no argument
        // add two nums from stack
        {
          vm_num_t v = dstack.pop_num();
          dstack.top_num() += v;
        }
        break;
      case OP_ARG_NUM:    // constant numeric value (4 bytes)
        // add constant num to num from stack
        dstack.top_num() += arg.val;
        break;
      default:
        assert(0);
      }
      break;

    case OP_AND:  // Binary AND              NULL,NUM
      assert(dsize==4);
      switch (argtype)
      {
      case OP_ARG_NULL:   // no argument
        // binary-and two nums from stack
        {
          vm_num_t v = dstack.pop_num();
          dstack.top_num() = int(dstack.top_num()) & int(v);
        }
        break;
      case OP_ARG_NUM:    // constant numeric value (4 bytes)
        // binary-and constant num to num from stack
        dstack.top_num() = int(dstack.top_num()) & int(arg.val);
        break;
      default:
        assert(0);
      }
      break;

    case OP_BF:   // Branch if false         PCR
      assert(argtype==OP_ARG_PCR);
      // test top num from stack
      if ( !dstack.pop_num() )
      {
        // if false, add word argument to program counter
        (unsigned&)PC += arg.word;
      }
      break;

    case OP_BRA:  // Branch                  PCR
      assert(argtype==OP_ARG_PCR);
      // add word argument to program counter
      (unsigned&)PC += arg.word;
      break;

    case OP_BSL:  // Call library function   LFR
      assert( argtype == OP_ARG_LFR );
      // call library function
      running = call_script_library_function( arg, oldPC );
      break;

    case OP_BSR:  // Branch to subroutine    SFR
      assert(argtype==OP_ARG_SFR);
      // push current program counter (this goes onto a separate PC stack)
      push_PC();
      // jump to subroutine
      PC = arg.sfr->get_start();
      break;

    case OP_BST:  // Spawn sub-thread        SFR
      {
        assert( argtype == OP_ARG_SFR );
        // spawn a sub-thread
        spawn_sub_thread( arg );
      }
      break;

    case OP_BTH:  // Spawn parallel thread    SFR
      {
        assert(argtype==OP_ARG_SFR);
        // This NEW form of spawning allows a thread to be spawned on the local
        // script object instance (via a THIS pointer) rather than the currently-
        // running script object instance.
        spawn_parallel_thread( arg );
      }
      break;

    case OP_DEC:  // Decrement               NULL
      assert(dsize==4);
      assert(argtype==OP_ARG_NULL);
      // decrement num from stack
      dstack.top_num() -= 1;
      break;

    case OP_DIV:  // Divide                  NULL,NUM,NUMR
      assert(dsize==4);
      switch (argtype)
      {
      case OP_ARG_NULL:   // no argument
        // divide second num in stack by top num from stack
        {
          vm_num_t v = dstack.pop_num();
          dstack.top_num() /= v;
        }
        break;
      case OP_ARG_NUM:    // constant numeric value (4 bytes)
        // divide num from stack by constant num
        dstack.top_num() /= arg.val;
        break;
      case OP_ARG_NUMR:   // constant numeric value used in reverse position (4 bytes; only applies to non-commutative operations)
        // divide constant num by num from stack
        dstack.top_num() = arg.val / dstack.top_num();
        break;
      default:
        assert(0);
      }
      break;

    case OP_DUP:  // Copy top of stack       NULL,SPR,POPO,SDR
      switch (argtype)
      {
      case OP_ARG_NULL:   // no argument
        // push dsize bytes to top of stack
        if ( !dstack.push(dstack.get_SP()-dsize,dsize) )
          slf_error( "stack overflow" );
        break;
      case OP_ARG_SPR:    // SP-relative address (2 bytes)
        // copy dsize bytes to destination
        memcpy(dstack.get_SP()+arg.word,dstack.get_SP()-dsize,dsize);
        break;
      case OP_ARG_POPO:   // stack contents (pop) plus offset (2 bytes)
        {
          // get object address from top of stack
          script_object::instance* si = (script_object::instance*)dstack.pop_addr();
          #ifdef DEBUG
          if ( (unsigned)si == 0
            || (unsigned)si == 0x7B7B7B7B
            || (unsigned)si == 0x7D7D7D7D
            || (unsigned)si == 0x7F7F7F7F
            )
          {
            slf_error( "reference to bad or uninitialized script object instance value" );
          }
          #endif
          // copy dsize bytes to destination
          memcpy(si->get_buffer()+arg.word,dstack.get_SP()-dsize,dsize);
        }
        break;
      case OP_ARG_SDR:    // static data member reference (4 bytes)
        // copy dsize bytes to destination
        memcpy(arg.sdr,dstack.get_SP()-dsize,dsize);
        break;
      default:
        assert(0);
      }
      break;

    case OP_EQ:   // Equal                   NULL,NUM
      assert(dsize==4);
      switch (argtype)
      {
      case OP_ARG_NULL:   // no argument
        // compare two nums from stack
        {
          vm_num_t v = dstack.pop_num();
          dstack.top_num() = (dstack.top_num() == v);
        }
        break;
      case OP_ARG_NUM:    // constant numeric value (4 bytes)
        // compare constant num to num from stack
        dstack.top_num() = (dstack.top_num() == arg.val);
        break;
      default:
        assert(0);
      }
      break;

    case OP_GE:   // Greater or equal        NULL,NUM,NUMR
      assert(dsize==4);
      switch (argtype)
      {
      case OP_ARG_NULL:   // no argument
        // compare second num in stack to top num from stack
        {
          vm_num_t v = dstack.pop_num();
          dstack.top_num() = (dstack.top_num() >= v);
        }
        break;
      case OP_ARG_NUM:    // constant numeric value (4 bytes)
        // compare num from stack to constant num
        dstack.top_num() = (dstack.top_num() >= arg.val);
        break;
      case OP_ARG_NUMR:   // constant numeric value used in reverse position (4 bytes; only applies to non-commutative operations)
        // compare constant num to num from stack
        dstack.top_num() = (arg.val >= dstack.top_num());
        break;
      default:
        assert(0);
      }
      break;

    case OP_GT:   // Greater                 NULL,NUM,NUMR
      assert(dsize==4);
      switch (argtype)
      {
      case OP_ARG_NULL:   // no argument
        // compare second num in stack to top num from stack
        {
          vm_num_t v = dstack.pop_num();
          dstack.top_num() = (dstack.top_num() > v);
        }
        break;
      case OP_ARG_NUM:    // constant numeric value (4 bytes)
        // compare num from stack to constant num
        dstack.top_num() = (dstack.top_num() > arg.val);
        break;
      case OP_ARG_NUMR:   // constant numeric value used in reverse position (4 bytes; only applies to non-commutative operations)
        // compare constant num to num from stack
        dstack.top_num() = (arg.val > dstack.top_num());
        break;
      default:
        assert(0);
      }
      break;

    case OP_INC:  // Increment               NULL
      assert(dsize==4);
      assert(argtype==OP_ARG_NULL);
      // increment num from stack
      dstack.top_num() += 1;
      break;

    case OP_KIL:  // Kill thread             NULL,SFR
      switch (argtype)
      {
      case OP_ARG_NULL:   // no argument
        // kill my own thread
        kill_me = true;
        running = false;
        break;
      case OP_ARG_SFR:    // function member reference (4 bytes)
        // kill other thread(s)
        inst->kill_thread(arg.sfr);
        break;
      default:
        break;
      }
      break;


    case OP_LE:   // Less or equal           NULL,NUM,NUMR
      assert(dsize==4);
      switch (argtype)
      {
      case OP_ARG_NULL:   // no argument
        // compare second num in stack to top num from stack
        {
          vm_num_t v = dstack.pop_num();
          dstack.top_num() = (dstack.top_num() <= v);
        }
        break;
      case OP_ARG_NUM:    // constant numeric value (4 bytes)
        // compare num from stack to constant num
        dstack.top_num() = (dstack.top_num() <= arg.val);
        break;
      case OP_ARG_NUMR:   // constant numeric value used in reverse position (4 bytes; only applies to non-commutative operations)
        // compare constant num to num from stack
        dstack.top_num() = (arg.val <= dstack.top_num());
        break;
      default:
        assert(0);
      }
      break;

    case OP_LND:  // Logical AND             NULL,NUM
      assert(dsize==4);
      switch (argtype)
      {
      case OP_ARG_NULL:   // no argument
        // logical-and two nums from stack
        {
          vm_num_t v = dstack.pop_num();
          dstack.top_num() = (dstack.top_num() && v);
        }
        break;
      case OP_ARG_NUM:    // constant numeric value (4 bytes)
        // logical-and constant num to num from stack
        dstack.top_num() = (dstack.top_num() && arg.val);
        break;
      default:
        assert(0);
      }
      break;

    case OP_LNT:  // Logical NOT             NULL
      {
        assert(dsize==4);
        assert(argtype==OP_ARG_NULL);
        // logical-not num from stack
        float& floatref = dstack.top_num();
        floatref = floatref?0.0f:1.0f;
      }
      break;

    case OP_LOR:  // Logical OR              NULL,NUM
      assert(dsize==4);
      switch (argtype)
      {
      case OP_ARG_NULL:   // no argument
        // logical-or two nums from stack
        {
          vm_num_t v = dstack.pop_num();
          dstack.top_num() = (dstack.top_num() || v);
        }
        break;
      case OP_ARG_NUM:    // constant numeric value (4 bytes)
        // logical-or constant num to num from stack
        dstack.top_num() = (dstack.top_num() || arg.val);
        break;
      default:
        assert(0);
      }
      break;

    case OP_LT:   // Less                    NULL,NUM,NUMR
      assert(dsize==4);
      switch (argtype)
      {
      case OP_ARG_NULL:   // no argument
        // compare second num in stack to top num from stack
        {
          vm_num_t v = dstack.pop_num();
          dstack.top_num() = (dstack.top_num() < v);
        }
        break;
      case OP_ARG_NUM:    // constant numeric value (4 bytes)
        // compare num from stack to constant num
        dstack.top_num() = (dstack.top_num() < arg.val);
        break;
      case OP_ARG_NUMR:   // constant numeric value used in reverse position (4 bytes; only applies to non-commutative operations)
        // compare constant num to num from stack
        dstack.top_num() = (arg.val < dstack.top_num());
        break;
      default:
        assert(0);
      }
      break;

    case OP_MOD:  // Mod                     NULL,NUM,NUMR
      assert(dsize==4);
      switch (argtype)
      {
      case OP_ARG_NULL:   // no argument
        // mod second num in stack by top num from stack
        {
          vm_num_t v = dstack.pop_num();
          dstack.top_num() = int(dstack.top_num()) % int(v);
        }
        break;
      case OP_ARG_NUM:    // constant numeric value (4 bytes)
        // mod num from stack by constant num
        dstack.top_num() = int(dstack.top_num()) % int(arg.val);
        break;
      case OP_ARG_NUMR:   // constant numeric value used in reverse position (4 bytes; only applies to non-commutative operations)
        // mod constant num by num from stack
        dstack.top_num() = int(arg.val) % int(dstack.top_num());
        break;
      default:
        assert(0);
      }
      break;

    case OP_MUL:  // Multiply                NULL,NUM
      assert(dsize==4);
      switch (argtype)
      {
      case OP_ARG_NULL:   // no argument
        // multiply two nums from stack
        {
          vm_num_t v = dstack.pop_num();
          dstack.top_num() *= v;
        }
        break;
      case OP_ARG_NUM:    // constant numeric value (4 bytes)
        // multiply constant num to num from stack
        dstack.top_num() *= arg.val;
        break;
      default:
        assert(0);
      }
      break;

    case OP_NE:   // Not equal               NULL,NUM
      assert(dsize==4);
      switch (argtype)
      {
      case OP_ARG_NULL:   // no argument
        // compare two nums from stack
        {
          vm_num_t v = dstack.pop_num();
          dstack.top_num() = (dstack.top_num() != v);
        }
        break;
      case OP_ARG_NUM:    // constant numeric value (4 bytes)
        // compare constant num to num from stack
        dstack.top_num() = (dstack.top_num() != arg.val);
        break;
      default:
        assert(0);
      }
      break;

    case OP_NEG:  // Negate                  NULL
      assert(dsize==4);
      assert(argtype==OP_ARG_NULL);
      // negate num from stack
      dstack.top_num() = -dstack.top_num();
      break;

    case OP_NOP:  // No operation            NULL
      assert(argtype==OP_ARG_NULL);
      break;

    case OP_NOT:  // Binary NOT              NULL
      assert(dsize==4);
      assert(argtype==OP_ARG_NULL);
      // binary-not num from stack
      dstack.top_num() = ~int(dstack.top_num());
      break;

    case OP_OR:   // Binary OR               NULL,NUM
      assert(dsize==4);
      switch (argtype)
      {
      case OP_ARG_NULL:   // no argument
        // binary-or two nums from stack
        {
          vm_num_t v = dstack.pop_num();
          dstack.top_num() = int(dstack.top_num()) | int(v);
        }
        break;
      case OP_ARG_NUM:    // constant numeric value (4 bytes)
        // binary-or constant num to num from stack
        dstack.top_num() = int(dstack.top_num()) | int(arg.val);
        break;
      default:
        assert(0);
      }
      break;

    case OP_POP:  // Pop from stack          NULL,SPR,POPO,SDR
      switch (argtype)
      {
      case OP_ARG_NULL:   // no argument
        // pop dsize bytes
        dstack.pop(dsize);
        break;
      case OP_ARG_SPR:    // SP-relative address (2 bytes)
        // copy dsize bytes to destination
        memcpy(dstack.get_SP()+arg.word,dstack.get_SP()-dsize,dsize);
        // pop dsize bytes
        dstack.pop(dsize);
        break;
      case OP_ARG_POPO:   // stack contents (pop) plus offset (2 bytes)
        {
          // get object address from top of stack
          script_object::instance* si = (script_object::instance*)dstack.pop_addr();
          #ifdef DEBUG
          if ( (unsigned)si == 0
            || (unsigned)si == 0x7B7B7B7B
            || (unsigned)si == 0x7D7D7D7D
            || (unsigned)si == 0x7F7F7F7F
            )          {
            slf_error( "reference to bad or uninitialized script object instance value" );
          }
          #endif
          // copy dsize bytes to destination
          memcpy(si->get_buffer()+arg.word,dstack.get_SP()-dsize,dsize);
          // pop dsize bytes
          dstack.pop(dsize);
        }
        break;
      case OP_ARG_SDR:    // static data member reference (4 bytes)
        // copy dsize bytes to destination
        memcpy(arg.sdr,dstack.get_SP()-dsize,dsize);
        // pop dsize bytes
        dstack.pop(dsize);
        break;
      default:
        assert(0);
      }
      break;

    case OP_PSH:  // Push onto stack         NUM,STR,SPR,POPO,SDR,CLV
      switch (argtype)
      {
      case OP_ARG_NUM:    // constant numeric value (4 bytes)
        if ( !dstack.push(arg.val) )
          slf_error( "stack overflow" );
        break;
      case OP_ARG_STR:    // constant string value (4 bytes)
        if ( !dstack.push(arg.str) )
          slf_error( "stack overflow" );
        break;
      case OP_ARG_SPR:    // SP-relative address (2 bytes)
        // push dsize bytes to top of stack
        if ( !dstack.push(dstack.get_SP()+arg.word,dsize) )
          slf_error( "stack overflow" );
        break;
      case OP_ARG_POPO:   // stack contents (pop) plus offset (2 bytes)
        {
          // get object address from top of stack
          script_object::instance* si = (script_object::instance*)dstack.pop_addr();
          #ifdef DEBUG
          if ( (unsigned)si == 0
            || (unsigned)si == 0x7B7B7B7B
            || (unsigned)si == 0x7D7D7D7D
            || (unsigned)si == 0x7F7F7F7F
            )
          {
            slf_error( "reference to bad or uninitialized script object instance value" );
          }
          #endif
          // push dsize bytes to top of stack
          if ( !dstack.push(si->get_buffer()+arg.word,dsize) )
            slf_error( "stack overflow" );
        }
        break;
      case OP_ARG_SDR:    // static data member reference (4 bytes)
        // push dsize bytes to top of stack
        if ( !dstack.push(arg.sdr,dsize) )
          slf_error( "stack overflow" );
        break;
      case OP_ARG_CLV:   // class value (4 bytes)
        // push 4-byte value to top of stack
        if ( !dstack.push(arg.binary) )
          slf_error( "stack overflow" );
        break;
      case OP_ARG_SIG:    // global signal value (2-byte id becomes 4-byte signal pointer)
        // push address of global signal (signal object will be created if necessary; see signal.h)
        dstack.push( app::inst()->signal_ptr( arg.word ) );
        break;
      case OP_ARG_PSIG:   // stack contents (pop) provides context for member signal
        {
          // get object address from top of stack
          signaller* obj = (signaller*)dstack.pop_addr();
          if ( obj==NULL || (uint32)obj==UNINITIALIZED_SCRIPT_PARM )
            slf_error( "invalid signaller pointer in local signal expression" );
          // push address of member signal (signal object will be created if necessary; see signal.h)
          if ( !dstack.push(obj->signal_ptr(arg.word)) )
            slf_error( "stack overflow" );
        }
        break;
      default:
        assert(0);
      }
      break;

    case OP_RET:  // Return from subroutine  NULL
      assert(argtype==OP_ARG_NULL);
      // pop program counter from program counter stack
      pop_PC();
      if (!PC)
      {
        // this is a return from the root level; kill my thread
        kill_me = true;
        running = false;
      }
      break;

    case OP_SHL:  // Shift left              NULL,NUM,NUMR
      assert(dsize==4);
      switch (argtype)
      {
      case OP_ARG_NULL:   // no argument
        // shift second num in stack by top num from stack
        {
          vm_num_t v = dstack.pop_num();
          dstack.top_num() = int(dstack.top_num()) << int(v);
        }
        break;
      case OP_ARG_NUM:    // constant numeric value (4 bytes)
        // shift num from stack by constant num
        dstack.top_num() = int(dstack.top_num()) << int(arg.val);
        break;
      case OP_ARG_NUMR:   // constant numeric value used in reverse position (4 bytes; only applies to non-commutative operations)
        // shift constant num by num from stack
        dstack.top_num() = int(arg.val) << int(dstack.top_num());
        break;
      default:
        assert(0);
      }
      break;

    case OP_SHR:  // Shift right             NULL,NUM,NUMR
      assert(dsize==4);
      switch (argtype)
      {
      case OP_ARG_NULL:   // no argument
        // shift second num in stack by top num from stack
        {
          vm_num_t v = dstack.pop_num();
          dstack.top_num() = int(dstack.top_num()) >> int(v);
        }
        break;
      case OP_ARG_NUM:    // constant numeric value (4 bytes)
        // shift num from stack by constant num
        dstack.top_num() = int(dstack.top_num()) >> int(arg.val);
        break;
      case OP_ARG_NUMR:   // constant numeric value used in reverse position (4 bytes; only applies to non-commutative operations)
        // shift constant num by num from stack
        dstack.top_num() = int(arg.val) >> int(dstack.top_num());
        break;
      default:
        assert(0);
      }
      break;

    case OP_SPA:  // Add to Stack Pointer    WORD
      assert(argtype==OP_ARG_WORD);
      // Wipe uninitialized memory on the stack
#ifdef _DEBUG
      if (arg.word>0)
      {
        int stop = arg.word/4;
        assert (arg.word == stop*4);
        for (int i=0;i<stop;i++) ((unsigned int *)dstack.get_SP())[i] = UNINITIALIZED_SCRIPT_PARM;
      }
#endif
      dstack.move_SP(arg.word);
      break;

    case OP_SUB:  // Subtract                NULL,NUM,NUMR
      assert(dsize==4);
      switch (argtype)
      {
      case OP_ARG_NULL:   // no argument
        // subtract top num from second num in stack
        {
          vm_num_t v = dstack.pop_num();
          dstack.top_num() -= v;
        }
        break;
      case OP_ARG_NUM:    // constant numeric value (4 bytes)
        // subtract constant num from num from stack
        dstack.top_num() -= arg.val;
        break;
      case OP_ARG_NUMR:   // constant numeric value used in reverse position (4 bytes; only applies to non-commutative operations)
        // subtract num from stack from constant num
        dstack.top_num() = arg.val - dstack.top_num();
        break;
      default:
        assert(0);
      }
      break;

    case OP_XOR:  // Exclusive OR            NULL,NUM
      assert(dsize==4);
      switch (argtype)
      {
      case OP_ARG_NULL:   // no argument
        // binary-xor two nums from stack
        {
          vm_num_t v = dstack.pop_num();
          dstack.top_num() = int(dstack.top_num()) ^ int(v);
        }
        break;
      case OP_ARG_NUM:    // constant numeric value (4 bytes)
        // binary-xor constant num to num from stack
        dstack.top_num() = int(dstack.top_num()) ^ int(arg.val);
        break;
      default:
        assert(0);
      }
      break;

    case OP_STR_EQ:   // Equal                   NULL,STR
      assert(dsize==4);
      switch (argtype)
      {
      case OP_ARG_NULL:   // no argument
        // compare two strs from stack
        {
          vm_str_t v = dstack.pop_str();
          dstack.top_num() = (*dstack.top_str() == *v);
        }
        break;
      case OP_ARG_STR:    // constant string value (4 bytes)
        // compare constant str to str from stack
        dstack.top_num() = (*dstack.top_str() == *arg.str);
        break;
      default:
        assert(0);
      }
      break;

    case OP_STR_NE:   // Not equal               NULL,STR
      assert(dsize==4);
      switch (argtype)
      {
      case OP_ARG_NULL:   // no argument
        // compare two strs from stack
        {
          vm_str_t v = dstack.pop_str();
          dstack.top_num() = (*dstack.top_str() != *v);
        }
        break;
      case OP_ARG_STR:    // constant string value (4 bytes)
        // compare constant str to str from stack
        dstack.top_num() = (*dstack.top_str() != *arg.str);
        break;
      default:
        assert(0);
      }
      break;

    case OP_SIG_LND:  // SIGNAL Logical AND         NULL
      assert( dsize == 4 );
      switch ( argtype )
      {
      case OP_ARG_NULL:   // no argument
        // logical AND two signals from stack (creates a NEW signal)
        {
          vm_signal_t v = dstack.pop_signal();
          dstack.top_signal() = signal_manager::inst()->signal_AND( dstack.top_signal(), v );
        }
        break;
      default:
        assert( 0 );
      }
      break;

    case OP_SIG_LOR:  // SIGNAL Logical OR         NULL
      assert( dsize == 4 );
      switch ( argtype )
      {
      case OP_ARG_NULL:   // no argument
        // logical OR two signals from stack (creates a NEW signal)
        {
          vm_signal_t v = dstack.pop_signal();
          dstack.top_signal() = signal_manager::inst()->signal_OR( dstack.top_signal(), v );
        }
        break;
      default:
        assert( 0 );
      }
      break;

    case OP_ECB:  // Create event callback         SFR
      {
        assert( argtype == OP_ARG_SFR );
        // a nonstatic event callback will, when the associated signal is raised,
        // spawn a thread on the script object instance pointed to by the top stack value
        create_event_callback( arg, false );
      }
      break;

    case OP_SCB:  // Create static event callback   SFR
      {
        assert( argtype == OP_ARG_SFR );
        // a static event callback will, when the associated signal is raised,
        // spawn a sub-thread on this script object instance
        create_static_event_callback( arg, false );
      }
      break;

    case OP_ECO:  // Create one-shot event callback         SFR
      {
        assert( argtype == OP_ARG_SFR );
        // a nonstatic event callback will, when the associated signal is raised,
        // spawn a thread on the script object instance pointed to by the top stack value
        create_event_callback( arg, true );
      }
      break;

    case OP_SCO:  // Create static one-shot event callback   SFR
      {
        assert( argtype == OP_ARG_SFR );
        // a static event callback will, when the associated signal is raised,
        // spawn a sub-thread on this script object instance
        create_static_event_callback( arg, true );
      }
      break;

    default:
      assert(0);
    }
  }

//	debug_print(  "leave vm_thread::run" );

  #if THREAD_PROFILING
  float runningtime = runtime.elapsed();
  prof_runtime = runningtime * 1000.0f;  // milliseconds
  #endif

assert(PC_stack.end() >= PC_stack.begin());
  return kill_me;
}


bool vm_thread::call_script_library_function( const argument_t& arg, const unsigned short* oldPC )
{
  char* oldSP = dstack.get_SP();
  if ( !((*arg.lfr)(dstack,entry)) )
  {
    // library function has not generated a return value yet;
    // next cycle will call library function again
    PC = oldPC;
    dstack.set_SP( oldSP );
    // flag signifies re-call of library function
    entry = script_library_class::function::RECALL_ENTRY;
    // interrupt thread (will resume next frame)
    return false;
  }
  else
  {
    entry = script_library_class::function::FIRST_ENTRY;  // reset for next library call
    return true;
  }
}


void vm_thread::spawn_sub_thread( const argument_t& arg )
{
  // create NEW thread with initial stack data copied from current stack,
  // based on function parameters stack size
  vm_thread* newvmt = inst->add_thread(arg.sfr);

  // spawned thread inherits the suspendability of the parent thread
  newvmt->set_suspendable( is_suspendable() );

  // spawned thread inherits camera priority
  newvmt->set_camera_priority( get_camera_priority() );

  int psize = arg.sfr->get_parms_stacksize();
  if ( psize )
  {
    if ( !newvmt->dstack.push(dstack.get_SP()-psize,psize) )
      slf_error( "stack overflow spawning " + arg.sfr->get_fullname() );
  }
  newvmt->PC = arg.sfr->get_start();
  // pop function parameters from local stack
  dstack.pop( psize );
}


void vm_thread::spawn_parallel_thread( const argument_t& arg )
{
  // create NEW thread with initial stack data copied from current stack,
  // based on function parameters stack size
  script_object::instance* local_inst = (script_object::instance*)dstack.pop_addr();
  if ( local_inst==NULL || (uint32)local_inst==UNINITIALIZED_SCRIPT_PARM )
  {
    slf_error( "(local) spawn " + arg.sfr->get_fullname() + ": invalid local script object instance pointer" );
  }
  vm_thread* newvmt = local_inst->add_thread( arg.sfr );

  // spawned thread inherits the suspendability of the parent thread
  newvmt->set_suspendable( is_suspendable() );

  // spawned thread inherits camera priority
  newvmt->set_camera_priority( get_camera_priority() );

  int psize = arg.sfr->get_parms_stacksize();
  if ( psize )
  {
    if ( !newvmt->dstack.push(dstack.get_SP()-psize,psize) )
      error( local_inst->get_name() + ": stack overflow spawning " + arg.sfr->get_fullname() );
  }
  newvmt->PC = arg.sfr->get_start();
  // pop function parameters from local stack
  dstack.pop( psize );
}


void vm_thread::create_event_callback( const argument_t& arg, bool one_shot )
{
  // pop local script object instance pointer (instance to which NEW thread will be added)
  script_object::instance* local_inst = (script_object::instance*)dstack.pop_addr();
  if ( local_inst==NULL || (uint32)local_inst==UNINITIALIZED_SCRIPT_PARM )
  {
    slf_error( "(local) event callback " + arg.sfr->get_fullname() + ": invalid local script object instance pointer" );
  }
  // pop function parameters
  dstack.pop( arg.sfr->get_parms_stacksize() );
  char* parms = dstack.get_SP();
  // pop signal to which callback applies
  vm_signal_t v = dstack.pop_signal();
  // add script callback with parameters copied from current stack
  v->add_callback( local_inst, arg.sfr, parms, one_shot );
}


void vm_thread::create_static_event_callback( const argument_t& arg, bool one_shot )
{
  // pop function parameters
  dstack.pop( arg.sfr->get_parms_stacksize() );
  char* parms = dstack.get_SP();
  // pop signal to which callback applies
  vm_signal_t v = dstack.pop_signal();
  // add script callback with parameters copied from current stack
  v->add_callback( inst, arg.sfr, parms, one_shot );
}



// Internal Methods

// program counter stack
void vm_thread::pop_PC()
{
  if ( !PC_stack.empty() )
  {
    PC = PC_stack.back();
    PC_stack.pop_back();
  }
  else
    PC = NULL;
}


void vm_thread::remove_from_local_region()
{
  if ( local_region )
  {
    local_region->remove_local_thread(this);
  }
}


void vm_thread::remove_from_local_character()
{
STUBBED(vm_thread_remove_from_local_character, "vm_thread::remove_from_local_character");
/*!  if ( local_character )
  {
    local_character->remove_local_thread(this);
  }
!*/
}


void vm_thread::slf_error( const stringx& err )
{
  // obtain pointer to vm_executable that corresponds to current PC
  const vm_executable* cur_ex = g_world_ptr->get_script_manager()->find_function_by_address( PC );
  if ( cur_ex )
    error( inst->get_name() + ": " + cur_ex->get_owner()->get_name() + "::" + cur_ex->get_fullname() + ":\n" + err );
  else
    error( inst->get_name() + ": (UNKNOWN FUNCTION!):\n" + err );
}


void vm_thread::slf_warning( const stringx& err )
{
  // obtain pointer to vm_executable that corresponds to current PC
  const vm_executable* cur_ex = g_world_ptr->get_script_manager()->find_function_by_address( PC );
  if ( cur_ex )
    warning( inst->get_name() + ": " + cur_ex->get_owner()->get_name() + "::" + cur_ex->get_fullname() + ":\n" + err );
  else
    warning( inst->get_name() + ": (UNKNOWN FUNCTION!):\n" + err );
}


void vm_thread::set_camera_priority( rational_t pr )
{
  camera_priority = pr;
}

