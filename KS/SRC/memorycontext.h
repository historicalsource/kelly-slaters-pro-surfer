/*-------------------------------------------------------------------------------------------------------
	
	Memory Context

	Provides a stack of strings that describe the current operating context of the program.
	The entire list of strings are printed out when a memory problem occurs to better provide 
  information on where the memory problem occurred.

	For convenience, make a local ectx object in your function to describe the operation of the function.
	When the function exits, the ectx object will be destroyed and the context removed.

	Be careful that you always remove your context.

-------------------------------------------------------------------------------------------------------*/
#ifndef MEMORY_CONTEXT_H
#define MEMORY_CONTEXT_H

#include "singleton.h"
#include "stringx.h"

class memory_context 
{
public:
	memory_context() { init(); }
  enum { MCTX_SIZE=16 };

	inline void push_context( const char* context) 
	{ 
    if( !initialized ) { init(); }
		assert( stack_pointer < MCTX_SIZE);
  	strcpy( context_stack[stack_pointer], context );
    ++stack_pointer;
	}

  inline void pop_context()
    {
      if( !initialized ) { init(); }
      --stack_pointer;
    }

	inline const char* get_context()
	{
    if( !initialized ) { init(); }
    return context_stack[stack_pointer-1];
	}

private:
	char context_stack[MCTX_SIZE][64];
  int  stack_pointer;
  static bool initialized;
  void init()
	{
    if( !initialized )
    {
      strcpy(context_stack[0],"Global");
      stack_pointer = 1;
      initialized = true;
    }
	}
};

extern memory_context g_memory_context;

#endif // MEMORY_CONTEXT_H
