#ifndef _SCRIPT_ACCESS_H_
#define _SCRIPT_ACCESS_H_

class script_object;
class script_object::instance;
class vm_thread;
class stringx;
class vector3d;
class entity;

class script
{
private:
  static script_object *gso;
  static script_object::instance *gsoi;
  static vm_thread *thread;

public:
  // Only called by WDS (init, destroy)
  inline static void init(script_object *_gso, script_object::instance *_gsoi)
  {
#ifdef KSCULL
    gso = _gso;
    gsoi = _gsoi;
    assert(gso && gsoi);
#endif
  }
  inline static void destroy()
  {
#ifdef KSCULL
    gso = NULL;
    gsoi = NULL;
#endif
  }


  // Script function code access
  static int find_function(const stringx &func);
  static bool new_thread(int function_id);
  static bool push_arg(entity *ent);
  static bool push_arg(rational_t num);
  static bool push_arg(const vector3d &vec);
  static bool exec_thread(bool run = false);
  

  // Helper functions
  inline static bool new_thread(const stringx &func)
  {
    int id = find_function(func);
    if(id >= 0)
      return(new_thread(id));
    else
      return(false);
  }

  // func()
  inline static bool function(const stringx &func, bool run = false)
  {
    if(new_thread(func))
      return(exec_thread(run));
    else
      return(false);
  }

  // func(num)
  inline static bool function(const stringx &func, rational_t num, bool run = false)
  {
    if(new_thread(func))
    {
      push_arg(num);
      return(exec_thread(run));
    }
    else
      return(false);
  }

  // func(vector3d)
  inline static bool function(const stringx &func, const vector3d &vec, bool run = false)
  {
    if(new_thread(func))
    {
      push_arg(vec);
      return(exec_thread(run));
    }
    else
      return(false);
  }

  // func(entity)
  inline static bool function(const stringx &func, entity *ent, bool run = false)
  {
    if(new_thread(func))
    {
      push_arg(ent);
      return(exec_thread(run));
    }
    else
      return(false);
  }

  // func(entity, entity)
  inline static bool function(const stringx &func, entity *ent1, entity *ent2, bool run = false)
  {
    if(new_thread(func))
    {
      push_arg(ent1);
      push_arg(ent2);
      return(exec_thread(run));
    }
    else
      return(false);
  }

  // func(entity, entity, entity)
  inline static bool function(const stringx &func, entity *ent1, entity *ent2, entity *ent3, bool run = false)
  {
    if(new_thread(func))
    {
      push_arg(ent1);
      push_arg(ent2);
      push_arg(ent3);
      return(exec_thread(run));
    }
    else
      return(false);
  }

  // func(entity, num)
  inline static bool function(const stringx &func, entity *ent, rational_t num, bool run = false)
  {
    if(new_thread(func))
    {
      push_arg(ent);
      push_arg(num);
      return(exec_thread(run));
    }
    else
      return(false);
  }

  // func(entity, entity, num)
  inline static bool function(const stringx &func, entity *ent1, entity *ent2, rational_t num, bool run = false)
  {
    if(new_thread(func))
    {
      push_arg(ent1);
      push_arg(ent2);
      push_arg(num);
      return(exec_thread(run));
    }
    else
      return(false);
  }

  // func(entity, vector3d)
  inline static bool function(const stringx &func, entity *ent, const vector3d &vec1, bool run = false)
  {
    if(new_thread(func))
    {
      push_arg(ent);
      push_arg(vec1);
      return(exec_thread(run));
    }
    else
      return(false);
  }

  // func(entity, vector3d, vector3d)
  inline static bool function(const stringx &func, entity *ent, const vector3d &vec1, const vector3d &vec2, bool run = false)
  {
    if(new_thread(func))
    {
      push_arg(ent);
      push_arg(vec1);
      push_arg(vec2);
      return(exec_thread(run));
    }
    else
      return(false);
  }

  // func(entity, entity, vector3d, vector3d)
  inline static bool function(const stringx &func, entity *ent1, entity *ent2, const vector3d &vec1, const vector3d &vec2, bool run = false)
  {
    if(new_thread(func))
    {
      push_arg(ent1);
      push_arg(ent2);
      push_arg(vec1);
      push_arg(vec2);
      return(exec_thread(run));
    }
    else
      return(false);
  }

  // func(entity, vector3d, num)
  inline static bool function(const stringx &func, entity *ent, const vector3d &vec, rational_t num, bool run = false)
  {
    if(new_thread(func))
    {
      push_arg(ent);
      push_arg(vec);
      push_arg(num);
      return(exec_thread(run));
    }
    else
      return(false);
  }
};

#endif
