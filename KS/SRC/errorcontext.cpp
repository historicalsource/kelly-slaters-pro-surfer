
#include "global.h"
#include "errorcontext.h"
//P #include "memorycontext.h"

//! DEFINE_AUTO_SINGLETON(error_context)
DEFINE_SINGLETON(error_context)
//P memory_context g_memory_context;
//P bool memory_context::initialized = false;

ectx::ectx(const stringx & desc) 
{
  error_context::inst()->push_context(desc); 
}
	
ectx::~ectx() 
{
  error_context::inst()->pop_context(); 
}
