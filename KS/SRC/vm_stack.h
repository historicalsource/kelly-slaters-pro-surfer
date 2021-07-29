// vm_stack.h
#ifndef _VM_STACK_H
#define _VM_STACK_H


class stringx;
class vm_thread;
class signal;

typedef float vm_num_t;
typedef const stringx* vm_str_t;
typedef signal* vm_signal_t;


// CTT 06/15/00: This should be turned off for final release.
#if defined(BUILD_DEBUG) || defined(BUILD_FASTDEBUG)
  #define REPORT_OVERFLOW 1
#endif


class vm_stack
  {
  // Data
  public:
    int salloc;
    char* buffer;
    char* SP;
  private:
    // For debugging... to be able to print nice error messages
    vm_thread * my_thread;

  protected:
    void init(int sa);
    void set_SP(char* _sp) { SP = _sp; }

    void move_SP(int n)
      {
      assert(!(n&3));
      assert(!((unsigned)SP&3));
      SP += n;
      // check for stack underflow / overflow
      assert( size()>=0 && size()<=capacity() );
      }

    vm_num_t& top_num() { return *(vm_num_t*)(SP-sizeof(vm_num_t)); }
    vm_str_t& top_str() { return *(vm_str_t*)(SP-sizeof(vm_str_t)); }
    vm_signal_t& top_signal() { return *(vm_signal_t*)(SP-sizeof(vm_signal_t)); }

  // Constructors
  public:
    vm_stack() : salloc(0),buffer(NULL),SP(NULL), my_thread(NULL) {}
    vm_stack(int sa, vm_thread * _my_thread);
    ~vm_stack();

  // Methods
  public:
    char* get_SP() const  { return SP; }
    int size() const      { return SP-buffer; }
    int capacity() const  { return salloc; }
    vm_thread * get_thread() const {return my_thread;}

    void pop(int n)
      {
      move_SP( -n );
      }

    vm_num_t pop_num();

    vm_str_t pop_str()
      {
      pop(sizeof(vm_str_t));
      return *(vm_str_t*)SP;
      }

    vm_signal_t pop_signal()
      {
      pop( sizeof(vm_signal_t) );
      return *(vm_signal_t*)SP;
      }

    void* pop_addr()
      {
      pop(sizeof(void*));
      return *(void**)SP;
      }

    bool push( const char* src, int n );
    bool push(vm_num_t v)
      {
      *(vm_num_t*)SP = v;
      move_SP( sizeof(vm_num_t) );
#if REPORT_OVERFLOW
      // check for stack overflow
      if ( size() > capacity() )
        return false;
#endif
      return true;
      }

    bool push(vm_str_t v)
      {
      *(vm_str_t*)SP = v;
      move_SP( sizeof(vm_str_t) );
#if REPORT_OVERFLOW
      // check for stack overflow
      if ( size() > capacity() )
        return false;
#endif
      return true;
      }

    bool push( vm_signal_t v )
      {
      *(vm_signal_t*)SP = v;
      move_SP( sizeof(vm_signal_t) );
#if REPORT_OVERFLOW
      // check for stack overflow
      if ( size() > capacity() )
        return false;
#endif
      return true;
      }

    bool push(unsigned v)
      {
      *(int*)SP = v;
      move_SP( sizeof(int) );
#if REPORT_OVERFLOW
      // check for stack overflow
      if ( size() > capacity() )
        return false;
#endif
      return true;
      }

  // Friends
  friend class vm_thread;
  };


#endif  // _VM_STACK_H
