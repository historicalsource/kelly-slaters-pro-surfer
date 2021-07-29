
#include "global.h"

#include "script_lib_list.h"
#include "script_lib_vector3d.h"
#include "algebra.h"
#include "vm_stack.h"
#include "vm_thread.h"


////////////////////////////////////////////////////////////////////////////////
// script library class:  vector3d_list
////////////////////////////////////////////////////////////////////////////////

list<vm_vector3d_list_t> script_vector3d_lists;
list<vm_num_list_t> script_num_lists;
list<vm_entity_list_t> script_entity_lists;
list<vm_str_list_t> script_stringx_lists;

void destroy_script_lists()
{
  list<vm_vector3d_list_t>::iterator vi = script_vector3d_lists.begin();
  while(vi != script_vector3d_lists.end())
  {
    if((*vi) != NULL)
      delete (*vi);

    ++vi;
  }
  script_vector3d_lists.resize(0);

  list<vm_num_list_t>::iterator ni = script_num_lists.begin();
  while(ni != script_num_lists.end())
  {
    if((*ni) != NULL)
      delete (*ni);

    ++ni;
  }
  script_num_lists.resize(0);

  list<vm_entity_list_t>::iterator ei = script_entity_lists.begin();
  while(ei != script_entity_lists.end())
  {
    if((*ei) != NULL)
      delete (*ei);

    ++ei;
  }
  script_entity_lists.resize(0);

  list<vm_str_list_t>::iterator si = script_stringx_lists.begin();
  while(si != script_stringx_lists.end())
  {
    if((*si) != NULL)
      delete (*si);

    ++si;
  }
  script_stringx_lists.resize(0);
}


void destroy_script_vector3d_list(vm_vector3d_list_t lst)
{
  list<vm_vector3d_list_t>::iterator i = script_vector3d_lists.begin();
  while(i != script_vector3d_lists.end())
  {
    if((*i) == lst)
    {
      delete (*i);
      i = script_vector3d_lists.erase(i);

      return;
    }

    ++i;
  }
}
vm_vector3d_list_t create_script_vector3d_list()
{
  vm_vector3d_list_t lst = NEW vector3d_list;
  script_vector3d_lists.push_back(lst);
  return(lst);
}


void destroy_script_num_list(vm_num_list_t lst)
{
  list<vm_num_list_t>::iterator i = script_num_lists.begin();
  while(i != script_num_lists.end())
  {
    if((*i) == lst)
    {
      delete (*i);
      i = script_num_lists.erase(i);

      return;
    }

    ++i;
  }
}
vm_num_list_t create_script_num_list()
{
  vm_num_list_t lst = NEW num_list;
  script_num_lists.push_back(lst);
  return(lst);
}


void destroy_script_entity_list(vm_entity_list_t lst)
{
  list<vm_entity_list_t>::iterator i = script_entity_lists.begin();
  while(i != script_entity_lists.end())
  {
    if((*i) == lst)
    {
      delete (*i);
      i = script_entity_lists.erase(i);

      return;
    }

    ++i;
  }
}
vm_entity_list_t create_script_entity_list()
{
  vm_entity_list_t lst = NEW entity_list;
  script_entity_lists.push_back(lst);
  return(lst);
}


void destroy_script_str_list(vm_str_list_t lst)
{
  list<vm_str_list_t>::iterator i = script_stringx_lists.begin();
  while(i != script_stringx_lists.end())
  {
    if((*i) == lst)
    {
      delete (*i);
      i = script_stringx_lists.erase(i);

      return;
    }

    ++i;
  }
}
vm_str_list_t create_script_str_list()
{
  vm_str_list_t lst = NEW str_list;
  script_stringx_lists.push_back(lst);
  return(lst);
}


// global script library function: vector3d_list create_vector3d_list()
class slf_create_vector3d_list_t : public script_library_class::function {
public:
    // constructor required
    slf_create_vector3d_list_t(const char* n): script_library_class::function(n) { }
    // library function execution
    virtual bool operator ()(vm_stack &stack, entry_t entry)
    {
		vm_vector3d_list_t result = create_script_vector3d_list();
    SLF_RETURN;
    SLF_DONE;
    }
};
//slf_create_vector3d_list_t slf_create_vector3d_list("create_vector3d_list()");


// global script library function: vector3d_list destroy_vector3d_list(vector3d_list)
class slf_destroy_vector3d_list_t : public script_library_class::function {
public:
    // constructor required
    slf_destroy_vector3d_list_t(const char* n): script_library_class::function(n) { }
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_vector3d_list_t the_vector3d_list;
      };
    // library function execution
    virtual bool operator ()(vm_stack &stack, entry_t entry)
    {
    SLF_PARMS;
    destroy_script_vector3d_list(parms->the_vector3d_list);
    SLF_DONE;
    }
};
//slf_destroy_vector3d_list_t slf_destroy_vector3d_list("destroy_vector3d_list(vector3d_list)");


// script library function:  num vector3d_list::size()
class slf_vector3d_list_size_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_vector3d_list_size_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_vector3d_list_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_num_t result = parms->me->size();
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_vector3d_list_size_t slf_vector3d_list_size(slc_vector3d_list,"size()");


// script library function:  num vector3d_list::add(vector3d)
class slf_vector3d_list_add_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_vector3d_list_add_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_vector3d_list_t me;
      vector3d new_vec;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      parms->me->push_back(parms->new_vec);
      SLF_DONE;
      }
  };
//slf_vector3d_list_add_t slf_vector3d_list_add(slc_vector3d_list,"add(vector3d)");


// script library function:  vector3d_list_iterator vector3d_list::begin()
class slf_vector3d_list_begin_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_vector3d_list_begin_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_vector3d_list_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_vector3d_list_iterator_t result = parms->me->begin();
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_vector3d_list_begin_t slf_vector3d_list_begin(slc_vector3d_list,"begin()");

// script library function:  vector3d_list_iterator vector3d_list::end()
class slf_vector3d_list_end_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_vector3d_list_end_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_vector3d_list_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_vector3d_list_iterator_t result = parms->me->end();
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_vector3d_list_end_t slf_vector3d_list_end(slc_vector3d_list,"end()");

// script library function:  vector3d_list_iterator vector3d_list::rbegin()
class slf_vector3d_list_rbegin_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_vector3d_list_rbegin_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_vector3d_list_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_vector3d_list_iterator_t result = parms->me->end();
	  --result;
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_vector3d_list_begin_t slf_vector3d_list_begin(slc_vector3d_list,"rbegin()");

// script library function:  vector3d_list_iterator vector3d_list::rend()
class slf_vector3d_list_rend_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_vector3d_list_rend_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_vector3d_list_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_vector3d_list_iterator_t result = parms->me->begin();
	  --result;
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_vector3d_list_end_t slf_vector3d_list_end(slc_vector3d_list,"rend()");

// script library function:  vector3d_list_iterator vector3d_list::get_index(num)
class slf_vector3d_list_get_index_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_vector3d_list_get_index_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
		  vm_vector3d_list_t me;
		  vm_num_t index;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
	    #ifndef BUILD_BOOTABLE
		  if((parms->index >= parms->me->size()) || (parms->index < 0))
			  error("Index out of range %d", (int)parms->index);
	    #endif
	    vector3d result = (*parms->me)[(int)parms->index];
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_vector3d_list_end_t slf_vector3d_list_get_index(slc_vector3d_list,"get_index(num)");



////////////////////////////////////////////////////////////////////////////////
// script library class:  vector3d_list_iterator
////////////////////////////////////////////////////////////////////////////////

// script library function:  vector3d_list_iterator vector3d_list_iterator::operator++()
class slf_vector3d_list_iterator_inc_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_vector3d_list_iterator_inc_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_vector3d_list_iterator_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_vector3d_list_iterator_t result = ++parms->me;
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_vector3d_list_iterator_inc_t slf_vector3d_list_iterator_inc(slc_vector3d_list_iterator,"operator++()");

// script library function:  vector3d_list_iterator vector3d_list_iterator::operator--)
class slf_vector3d_list_iterator_dec_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_vector3d_list_iterator_dec_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_vector3d_list_iterator_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_vector3d_list_iterator_t result = --parms->me;
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_vector3d_list_iterator_inc_t slf_vector3d_list_iterator_inc(slc_vector3d_list_iterator,"operator--()");

// script library function:  num vector3d_list_iterator::operator==(vector3d_list_iterator)
class slf_vector3d_list_iterator_is_equal_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_vector3d_list_iterator_is_equal_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_vector3d_list_iterator_t me;
      vm_vector3d_list_iterator_t you;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_num_t result = parms->me==parms->you;
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_vector3d_list_iterator_is_equal_t slf_vector3d_list_iterator_is_equal(slc_vector3d_list_iterator,"operator==(vector3d_list_iterator)");

// script library function:  num vector3d_list_iterator::operator!=(vector3d_list_iterator)
class slf_vector3d_list_iterator_not_equal_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_vector3d_list_iterator_not_equal_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_vector3d_list_iterator_t me;
      vm_vector3d_list_iterator_t you;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_num_t result = parms->me!=parms->you;
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_vector3d_list_iterator_not_equal_t slf_vector3d_list_iterator_not_equal(slc_vector3d_list_iterator,"operator!=(vector3d_list_iterator)");

// script library function:  vector3d vector3d_list_iterator::get_vector3d()
class slf_vector3d_list_iterator_get_vector3d_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_vector3d_list_iterator_get_vector3d_t(script_library_class* slc,const char * n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_vector3d_list_iterator_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vector3d result = *(parms->me);
      #ifdef _DEBUG
      if ( result.length() > 10000 )
        stack.get_thread()->slf_error( "vector3d_list_iterator::get_vector3d(): retrieved invalid vector3d value" );
      #endif
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_vector3d_list_iterator_get_vector3d_t slf_vector3d_list_iterator_get_vector3d(slc_vector3d_list_iterator,"get_vector3d()");



////////////////////////////////////////////////////////////////////////////////
// script library class:  num_list
////////////////////////////////////////////////////////////////////////////////



// global script library function: num_list create_num_list()
class slf_create_num_list_t : public script_library_class::function {
public:
    // constructor required
    slf_create_num_list_t(const char* n): script_library_class::function(n) { }
    // library function execution
    virtual bool operator ()(vm_stack &stack, entry_t entry)
    {
		vm_num_list_t result = create_script_num_list();
    SLF_RETURN;
    SLF_DONE;
    }
};
//slf_create_num_list_t slf_create_num_list("create_num_list()");


// global script library function: num_list destroy_num_list(num_list)
class slf_destroy_num_list_t : public script_library_class::function {
public:
    // constructor required
    slf_destroy_num_list_t(const char* n): script_library_class::function(n) { }
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_num_list_t the_num_list;
      };
    // library function execution
    virtual bool operator ()(vm_stack &stack, entry_t entry)
    {
    SLF_PARMS;
    destroy_script_num_list(parms->the_num_list);
    SLF_DONE;
    }
};
//slf_destroy_num_list_t slf_destroy_num_list("destroy_num_list(num_list)");


// script library function:  num num_list::size()
class slf_num_list_size_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_num_list_size_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_num_list_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_num_t result = parms->me->size();
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_num_list_size_t slf_num_list_size(slc_num_list,"size()");


// script library function:  num num_list::add(num)
class slf_num_list_add_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_num_list_add_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_num_list_t me;
      vm_num_t new_num;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      parms->me->push_back(parms->new_num);
      SLF_DONE;
      }
  };
//slf_num_list_add_t slf_num_list_add(slc_num_list,"add(num)");


// script library function:  num_list_iterator num_list::begin()
class slf_num_list_begin_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_num_list_begin_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_num_list_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_num_list_iterator_t result = parms->me->begin();
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_num_list_begin_t slf_num_list_begin(slc_num_list,"begin()");

// script library function:  num_list_iterator num_list::end()
class slf_num_list_end_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_num_list_end_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_num_list_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_num_list_iterator_t result = parms->me->end();
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_num_list_end_t slf_num_list_end(slc_num_list,"end()");

// script library function:  num_list_iterator num_list::rbegin()
class slf_num_list_rbegin_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_num_list_rbegin_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_num_list_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_num_list_iterator_t result = parms->me->end();
	    --result;
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_num_list_begin_t slf_num_list_begin(slc_num_list,"rbegin()");

// script library function:  num_list_iterator num_list::rend()
class slf_num_list_rend_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_num_list_rend_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_num_list_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_num_list_iterator_t result = parms->me->begin();
	    --result;
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_num_list_end_t slf_num_list_end(slc_num_list,"rend()");

// script library function:  num_list_iterator num_list::get_index(num)
class slf_num_list_get_index_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_num_list_get_index_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
		  vm_num_list_t me;
		  vm_num_t index;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
	    #ifndef BUILD_BOOTABLE
		  if((parms->index >= parms->me->size()) || (parms->index < 0))
			  error("Index out of range %d", (int)parms->index);
	    #endif
	    vm_num_t result = (*parms->me)[(int)parms->index];
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_num_list_end_t slf_num_list_get_index(slc_vector3d_list,"get_index(num)");



////////////////////////////////////////////////////////////////////////////////
// script library class:  num_list_iterator
////////////////////////////////////////////////////////////////////////////////

// script library function:  num_list_iterator num_list_iterator::operator++()
class slf_num_list_iterator_inc_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_num_list_iterator_inc_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_num_list_iterator_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_num_list_iterator_t result = ++parms->me;
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_num_list_iterator_inc_t slf_num_list_iterator_inc(slc_num_list_iterator,"operator++()");

// script library function:  num_list_iterator num_list_iterator::operator--)
class slf_num_list_iterator_dec_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_num_list_iterator_dec_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_num_list_iterator_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_num_list_iterator_t result = --parms->me;
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_num_list_iterator_inc_t slf_num_list_iterator_inc(slc_num_list_iterator,"operator--()");

// script library function:  num num_list_iterator::operator==(num_list_iterator)
class slf_num_list_iterator_is_equal_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_num_list_iterator_is_equal_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_num_list_iterator_t me;
      vm_num_list_iterator_t you;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_num_t result = parms->me==parms->you;
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_num_list_iterator_is_equal_t slf_num_list_iterator_is_equal(slc_num_list_iterator,"operator==(num_list_iterator)");

// script library function:  num num_list_iterator::operator!=(num_list_iterator)
class slf_num_list_iterator_not_equal_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_num_list_iterator_not_equal_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_num_list_iterator_t me;
      vm_num_list_iterator_t you;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_num_t result = parms->me!=parms->you;
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_num_list_iterator_not_equal_t slf_num_list_iterator_not_equal(slc_num_list_iterator,"operator!=(num_list_iterator)");

// script library function:  vector3d vector3d_list_iterator::get_vector3d()
class slf_num_list_iterator_get_num_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_num_list_iterator_get_num_t(script_library_class* slc,const char * n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_num_list_iterator_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_num_t result = *(parms->me);
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_num_list_iterator_get_num_t slf_num_list_iterator_get_num(slc_num_list_iterator,"get_num()");



////////////////////////////////////////////////////////////////////////////////
// script library class:  entity_list
////////////////////////////////////////////////////////////////////////////////



// global script library function: num_list create_num_list()
class slf_create_entity_list_t : public script_library_class::function {
public:
    // constructor required
    slf_create_entity_list_t(const char* n): script_library_class::function(n) { }
    // library function execution
    virtual bool operator ()(vm_stack &stack, entry_t entry)
    {
		vm_entity_list_t result = create_script_entity_list();
    SLF_RETURN;
    SLF_DONE;
    }
};
//slf_create_entity_list_t slf_create_entity_list("create_entity_list()");


// global script library function: entity_list destroy_entity_list(entity_list)
class slf_destroy_entity_list_t : public script_library_class::function {
public:
    // constructor required
    slf_destroy_entity_list_t(const char* n): script_library_class::function(n) { }
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_entity_list_t the_entity_list;
      };
    // library function execution
    virtual bool operator ()(vm_stack &stack, entry_t entry)
    {
    SLF_PARMS;
    destroy_script_entity_list(parms->the_entity_list);
    SLF_DONE;
    }
};
//slf_destroy_entity_list_t slf_destroy_entity_list("destroy_num_list(entity_list)");


// script library function:  num num_list::size()
class slf_entity_list_size_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_entity_list_size_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_entity_list_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_num_t result = parms->me->size();
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_entity_list_size_t slf_entity_list_size(slc_entity_list,"size()");


// script library function:  num entity_list::add(entity)
class slf_entity_list_add_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_entity_list_add_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_entity_list_t me;
      vm_entity_t new_ent;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      parms->me->push_back(parms->new_ent);
      SLF_DONE;
      }
  };
//slf_entity_list_add_t slf_entity_list_add(slc_entity_list,"add(entity)");


// script library function:  entity_list_iterator entity_list::begin()
class slf_entity_list_begin_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_entity_list_begin_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_entity_list_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_entity_list_iterator_t result = parms->me->begin();
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_entity_list_begin_t slf_entity_list_begin(slc_entity_list,"begin()");

// script library function:  entity_list_iterator entity_list::end()
class slf_entity_list_end_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_entity_list_end_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_entity_list_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_entity_list_iterator_t result = parms->me->end();
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_entity_list_end_t slf_entity_list_end(slc_entity_list,"end()");

// script library function:  entity_list_iterator entity_list::rbegin()
class slf_entity_list_rbegin_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_entity_list_rbegin_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_entity_list_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_entity_list_iterator_t result = parms->me->end();
	    --result;
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_entity_list_begin_t slf_entity_list_begin(slc_entity_list,"rbegin()");

// script library function:  entity_list_iterator num_list::rend()
class slf_entity_list_rend_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_entity_list_rend_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_entity_list_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_entity_list_iterator_t result = parms->me->begin();
	    --result;
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_entity_list_end_t slf_entity_list_end(slc_entity_list,"rend()");

// script library function:  num_list_iterator entity_list::get_index(num)
class slf_entity_list_get_index_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_entity_list_get_index_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
		  vm_entity_list_t me;
		  vm_num_t index;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
	    #ifndef BUILD_BOOTABLE
		  if((parms->index >= parms->me->size()) || (parms->index < 0))
			  error("Index out of range %d", (int)parms->index);
	    #endif
	    vm_entity_t result = (*parms->me)[(int)parms->index];
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_entity_list_end_t slf_entity_list_get_index(slc_entity_list,"get_index(num)");



////////////////////////////////////////////////////////////////////////////////
// script library class:  entity_list_iterator
////////////////////////////////////////////////////////////////////////////////

// script library function:  num_list_iterator num_list_iterator::operator++()
class slf_entity_list_iterator_inc_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_entity_list_iterator_inc_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_entity_list_iterator_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_entity_list_iterator_t result = ++parms->me;
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_entity_list_iterator_inc_t slf_entity_list_iterator_inc(slc_entity_list_iterator,"operator++()");

// script library function:  entity_list_iterator entity_list_iterator::operator--)
class slf_entity_list_iterator_dec_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_entity_list_iterator_dec_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_entity_list_iterator_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_entity_list_iterator_t result = --parms->me;
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_entity_list_iterator_inc_t slf_entity_list_iterator_inc(slc_entity_list_iterator,"operator--()");

// script library function:  num entity_list_iterator::operator==(entity_list_iterator)
class slf_entity_list_iterator_is_equal_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_entity_list_iterator_is_equal_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_entity_list_iterator_t me;
      vm_entity_list_iterator_t you;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_num_t result = parms->me==parms->you;
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_entity_list_iterator_is_equal_t slf_entity_list_iterator_is_equal(slc_entity_list_iterator,"operator==(entity_list_iterator)");

// script library function:  num entity_list_iterator::operator!=(entity_list_iterator)
class slf_entity_list_iterator_not_equal_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_entity_list_iterator_not_equal_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_entity_list_iterator_t me;
      vm_entity_list_iterator_t you;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_num_t result = parms->me!=parms->you;
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_entity_list_iterator_not_equal_t slf_entity_list_iterator_not_equal(slc_entity_list_iterator,"operator!=(entity_list_iterator)");

// script library function:  entity entity_list_iterator::get_entity()
class slf_entity_list_iterator_get_entity_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_entity_list_iterator_get_entity_t(script_library_class* slc,const char * n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_entity_list_iterator_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_entity_t result = *(parms->me);
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_entity_list_iterator_get_num_t slf_num_list_iterator_get_num(slc_num_list_iterator,"get_entity()");




////////////////////////////////////////////////////////////////////////////////
// script library class:  str_list
////////////////////////////////////////////////////////////////////////////////



// global script library function: str_list create_str_list()
class slf_create_str_list_t : public script_library_class::function {
public:
    // constructor required
    slf_create_str_list_t(const char* n): script_library_class::function(n) { }
    // library function execution
    virtual bool operator ()(vm_stack &stack, entry_t entry)
    {
		vm_str_list_t result = create_script_str_list();
    SLF_RETURN;
    SLF_DONE;
    }
};
//slf_create_str_list_t slf_create_str_list("create_str_list()");


// global script library function: str_list destroy_str_list(str_list)
class slf_destroy_str_list_t : public script_library_class::function {
public:
    // constructor required
    slf_destroy_str_list_t(const char* n): script_library_class::function(n) { }
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_str_list_t the_str_list;
      };
    // library function execution
    virtual bool operator ()(vm_stack &stack, entry_t entry)
    {
    SLF_PARMS;
    destroy_script_str_list(parms->the_str_list);
    SLF_DONE;
    }
};
//slf_destroy_str_list_t slf_destroy_str_list("destroy_str_list(str_list)");


// script library function:  num str_list::size()
class slf_str_list_size_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_str_list_size_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_str_list_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_num_t result = parms->me->size();
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_str_list_size_t slf_str_list_size(slc_str_list,"size()");


// script library function:  num str_list::add(str)
class slf_str_list_add_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_str_list_add_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_str_list_t me;
      vm_str_t new_str;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      parms->me->push_back(*parms->new_str);
      SLF_DONE;
      }
  };
//slf_str_list_add_t slf_str_list_add(slc_str_list,"add(str)");


// script library function:  str_list_iterator str_list::begin()
class slf_str_list_begin_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_str_list_begin_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_str_list_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_str_list_iterator_t result = parms->me->begin();
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_str_list_begin_t slf_str_list_begin(slc_str_list,"begin()");

// script library function:  str_list_iterator str_list::end()
class slf_str_list_end_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_str_list_end_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_str_list_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_str_list_iterator_t result = parms->me->end();
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_str_list_end_t slf_str_list_end(slc_str_list,"end()");

// script library function:  entity_list_iterator entity_list::rbegin()
class slf_str_list_rbegin_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_str_list_rbegin_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_str_list_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_str_list_iterator_t result = parms->me->end();
	    --result;
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_str_list_begin_t slf_str_list_begin(slc_str_list,"rbegin()");

// script library function:  str_list_iterator str_list::rend()
class slf_str_list_rend_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_str_list_rend_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_str_list_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_str_list_iterator_t result = parms->me->begin();
	    --result;
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_str_list_end_t slf_str_list_end(slc_str_list,"rend()");

// script library function:  str_list_iterator str_list::get_index(num)
class slf_str_list_get_index_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_str_list_get_index_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
		  vm_str_list_t me;
		  vm_num_t index;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
	    #ifndef BUILD_BOOTABLE
		  if((parms->index >= parms->me->size()) || (parms->index < 0))
			  error("Index out of range %d", (int)parms->index);
	    #endif
	    vm_str_t result = &(*parms->me)[(int)parms->index];
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_str_list_end_t slf_str_list_get_index(slc_str_list,"get_index(num)");



////////////////////////////////////////////////////////////////////////////////
// script library class:  str_list_iterator
////////////////////////////////////////////////////////////////////////////////

// script library function:  num_list_iterator num_list_iterator::operator++()
class slf_str_list_iterator_inc_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_str_list_iterator_inc_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_str_list_iterator_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_str_list_iterator_t result = ++parms->me;
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_str_list_iterator_inc_t slf_str_list_iterator_inc(slc_str_list_iterator,"operator++()");

// script library function:  str_list_iterator str_list_iterator::operator--)
class slf_str_list_iterator_dec_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_str_list_iterator_dec_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_str_list_iterator_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_str_list_iterator_t result = --parms->me;
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_str_list_iterator_inc_t slf_str_list_iterator_inc(slc_str_list_iterator,"operator--()");

// script library function:  num str_list_iterator::operator==(str_list_iterator)
class slf_str_list_iterator_is_equal_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_str_list_iterator_is_equal_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_str_list_iterator_t me;
      vm_str_list_iterator_t you;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_num_t result = parms->me==parms->you;
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_str_list_iterator_is_equal_t slf_str_list_iterator_is_equal(slc_str_list_iterator,"operator==(str_list_iterator)");

// script library function:  num str_list_iterator::operator!=(str_list_iterator)
class slf_str_list_iterator_not_equal_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_str_list_iterator_not_equal_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_str_list_iterator_t me;
      vm_str_list_iterator_t you;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_num_t result = parms->me!=parms->you;
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_str_list_iterator_not_equal_t slf_str_list_iterator_not_equal(slc_str_list_iterator,"operator!=(str_list_iterator)");

// script library function:  entity str_list_iterator::get_str()
class slf_str_list_iterator_get_str_t : public script_library_class::function
  {
  public:
    // constructor required
    slf_str_list_iterator_get_str_t(script_library_class* slc,const char * n) : script_library_class::function(slc,n) {}
    // library function parameters
    struct parms_t
      {
      // parameters
      vm_str_list_iterator_t me;
      };
    // library function execution
    virtual bool operator()(vm_stack& stack,entry_t entry)
      {
      SLF_PARMS;
      vm_str_t result = &(*(parms->me));
      SLF_RETURN;
      SLF_DONE;
      }
  };
//slf_str_list_iterator_get_str_t slf_str_list_iterator_get_str(slc_str_list_iterator,"get_str()");


void register_list_lib()
{
  //***  vector3d_list  ***
  // pointer to single instance of library class
  slc_vector3d_list_t* slc_vector3d_list = NEW slc_vector3d_list_t("vector3d_list",4);

  NEW slf_create_vector3d_list_t("create_vector3d_list()");
  NEW slf_destroy_vector3d_list_t("destroy_vector3d_list(vector3d_list)");
  NEW slf_vector3d_list_size_t(slc_vector3d_list,"size()");
  NEW slf_vector3d_list_add_t(slc_vector3d_list,"add(vector3d)");
  NEW slf_vector3d_list_begin_t(slc_vector3d_list,"begin()");
  NEW slf_vector3d_list_end_t(slc_vector3d_list,"end()");
  NEW slf_vector3d_list_rbegin_t(slc_vector3d_list,"rbegin()");
  NEW slf_vector3d_list_rend_t(slc_vector3d_list,"rend()");
  NEW slf_vector3d_list_get_index_t(slc_vector3d_list,"get_index(num)");

  // pointer to single instance of library class
  slc_vector3d_list_iterator_t* slc_vector3d_list_iterator = NEW slc_vector3d_list_iterator_t("vector3d_list_iterator",sizeof(vm_vector3d_list_iterator_t));

  NEW slf_vector3d_list_iterator_inc_t(slc_vector3d_list_iterator,"operator++()");
  NEW slf_vector3d_list_iterator_dec_t(slc_vector3d_list_iterator,"operator--()");
  NEW slf_vector3d_list_iterator_is_equal_t(slc_vector3d_list_iterator,"operator==(vector3d_list_iterator)");
  NEW slf_vector3d_list_iterator_not_equal_t(slc_vector3d_list_iterator,"operator!=(vector3d_list_iterator)");
  NEW slf_vector3d_list_iterator_get_vector3d_t(slc_vector3d_list_iterator,"get_vector3d()");
  //***  End of vector3d_list  ***

  //***  num_list  ***
  // pointer to single instance of library class
  slc_num_list_t* slc_num_list = NEW slc_num_list_t("num_list",4);

  NEW slf_create_num_list_t("create_num_list()");
  NEW slf_destroy_num_list_t("destroy_num_list(num_list)");
  NEW slf_num_list_size_t(slc_num_list,"size()");
  NEW slf_num_list_add_t(slc_num_list,"add(num)");
  NEW slf_num_list_begin_t(slc_num_list,"begin()");
  NEW slf_num_list_end_t(slc_num_list,"end()");
  NEW slf_num_list_rbegin_t(slc_num_list,"rbegin()");
  NEW slf_num_list_rend_t(slc_num_list,"rend()");
  NEW slf_num_list_get_index_t(slc_num_list,"get_index(num)");

  // pointer to single instance of library class
  slc_num_list_iterator_t* slc_num_list_iterator = NEW slc_num_list_iterator_t("num_list_iterator",sizeof(vm_num_list_iterator_t));

  NEW slf_num_list_iterator_inc_t(slc_num_list_iterator,"operator++()");
  NEW slf_num_list_iterator_dec_t(slc_num_list_iterator,"operator--()");
  NEW slf_num_list_iterator_is_equal_t(slc_num_list_iterator,"operator==(num_list_iterator)");
  NEW slf_num_list_iterator_not_equal_t(slc_num_list_iterator,"operator!=(num_list_iterator)");
  NEW slf_num_list_iterator_get_num_t(slc_num_list_iterator,"get_num()");
  //***  End of num_list  ***

  //***  entity_list  ***
  // pointer to single instance of library class
  slc_entity_list_t* slc_entity_list = NEW slc_entity_list_t("entity_list",4);

  NEW slf_create_entity_list_t("create_entity_list()");
  NEW slf_destroy_entity_list_t("destroy_entity_list(entity_list)");
  NEW slf_entity_list_size_t(slc_entity_list,"size()");
  NEW slf_entity_list_add_t(slc_entity_list,"add(entity)");
  NEW slf_entity_list_begin_t(slc_entity_list,"begin()");
  NEW slf_entity_list_end_t(slc_entity_list,"end()");
  NEW slf_entity_list_rbegin_t(slc_entity_list,"rbegin()");
  NEW slf_entity_list_rend_t(slc_entity_list,"rend()");
  NEW slf_entity_list_get_index_t(slc_entity_list,"get_index(num)");

  // pointer to single instance of library class
  slc_entity_list_iterator_t* slc_entity_list_iterator = NEW slc_entity_list_iterator_t("entity_list_iterator",sizeof(vm_entity_list_iterator_t));

  NEW slf_entity_list_iterator_inc_t(slc_entity_list_iterator,"operator++()");
  NEW slf_entity_list_iterator_dec_t(slc_entity_list_iterator,"operator--()");
  NEW slf_entity_list_iterator_is_equal_t(slc_entity_list_iterator,"operator==(entity_list_iterator)");
  NEW slf_entity_list_iterator_not_equal_t(slc_entity_list_iterator,"operator!=(entity_list_iterator)");
  NEW slf_entity_list_iterator_get_entity_t(slc_entity_list_iterator,"get_entity()");
  //***  End of entity_list  ***

  //***  str_list  ***
  // pointer to single instance of library class
  slc_str_list_t* slc_str_list = NEW slc_str_list_t("str_list",4);

  NEW slf_create_str_list_t("create_str_list()");
  NEW slf_destroy_str_list_t("destroy_str_list(str_list)");
  NEW slf_str_list_size_t(slc_str_list,"size()");
  NEW slf_str_list_add_t(slc_str_list,"add(str)");
  NEW slf_str_list_begin_t(slc_str_list,"begin()");
  NEW slf_str_list_end_t(slc_str_list,"end()");
  NEW slf_str_list_rbegin_t(slc_str_list,"rbegin()");
  NEW slf_str_list_rend_t(slc_str_list,"rend()");
  NEW slf_str_list_get_index_t(slc_str_list,"get_index(num)");

  // pointer to single instance of library class
  slc_str_list_iterator_t* slc_str_list_iterator = NEW slc_str_list_iterator_t("str_list_iterator",sizeof(vm_str_list_iterator_t));

  NEW slf_str_list_iterator_inc_t(slc_str_list_iterator,"operator++()");
  NEW slf_str_list_iterator_dec_t(slc_str_list_iterator,"operator--()");
  NEW slf_str_list_iterator_is_equal_t(slc_str_list_iterator,"operator==(str_list_iterator)");
  NEW slf_str_list_iterator_not_equal_t(slc_str_list_iterator,"operator!=(str_list_iterator)");
  NEW slf_str_list_iterator_get_str_t(slc_str_list_iterator,"get_str()");
  //***  End of str_list  ***
}
