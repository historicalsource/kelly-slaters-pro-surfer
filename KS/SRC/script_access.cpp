#include "global.h"
#include "script_object.h"
#include "vm_thread.h"
#include "script_access.h"

script_object *script::gso = NULL;  
script_object::instance *script::gsoi = NULL;
vm_thread *script::thread = NULL;

int script::find_function(const stringx &func)
{
  if(gso != NULL)
  {
    int idx = gso->find_func( func );

    if(idx >= 0)
      return(idx);

#if defined(BUILD_DEBUG)
    else
      warning("Function '%s' not found!", func.c_str());
#endif

  }
  else
    error("Script has not been initted yet!");

  return(-1);
}

bool script::new_thread(int function_id)
{
  if(gso != NULL && gsoi != NULL)
  {
    if(thread == NULL && function_id >= 0)
    {
      thread = gsoi->add_thread( &gso->get_func(function_id) );
      return(true);
    }
  }
  else
    error("Script has not been initted yet!");

  return(false);
}

bool script::push_arg(entity *ent)
{
  if(thread)
    thread->get_data_stack().push( (char*)&ent, 4 );

  return(thread != NULL);
}

bool script::push_arg(rational_t num)
{
  if(thread)
    thread->get_data_stack().push( (char*)&num, sizeof(num) );

  return(thread != NULL);
}

bool script::push_arg(const vector3d &vec)
{
  if(thread)
    thread->get_data_stack().push( (char*)&vec, sizeof(vec) );

  return(thread != NULL);
}

bool script::exec_thread(bool run)
{
  if(thread)
  {
    if(run)
      gsoi->run_single_thread( thread, false );

    thread = NULL;

    return(true);
  }

  return(false);
}
