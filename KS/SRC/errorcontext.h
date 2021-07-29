/*-------------------------------------------------------------------------------------------------------
	
	Error Context

	Provides a stack of strings that describe the current operating context of the program.
	The entire list of strings are printed out when an error occurs to better provide information on 
	where the error occurred.

	For convenience, make a local ectx object in your function to describe the operation of the function.
	When the function exits, the ectx object will be destroyed and the context removed.

	Be careful that you always remove your context.

-------------------------------------------------------------------------------------------------------*/
#ifndef ERROR_CONTEXT
#define ERROR_CONTEXT

#include "singleton.h"
#include "stringx.h"
#include <vector>

class error_context : public singleton
{
  private:
		vector<stringx> context_stack;

	public:

		enum { ECTX_STACK_SIZE = 64 };

		error_context() 
		{
			context_stack.reserve(ECTX_STACK_SIZE);
		}

		~error_context()
		{
			context_stack.resize(0);
		}

  	DECLARE_SINGLETON(error_context)

		inline void push_context( const stringx & context ) 
		{ 
			assert( context_stack.size() < ECTX_STACK_SIZE );
  		context_stack.push_back(context); 
		}

		inline void pop_context() { context_stack.pop_back(); }

		inline stringx get_context() const
		{
			stringx work;
			for ( vector<stringx>::const_iterator it = context_stack.begin(); it != context_stack.end(); ++it )
				work += "[" + *it + "]->";
			return work;
		}

    inline vector<stringx> const & get_stack() { return context_stack; }
};

class ectx
{
	public:
		ectx(const stringx & desc);
		~ectx();
};

#endif