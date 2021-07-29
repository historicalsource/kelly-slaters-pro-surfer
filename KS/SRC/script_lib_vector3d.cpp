// script_lib_vector3d.cpp
//
// This file contains application-specific code and data that makes use of the
// Script Runtime Core to define script library classes and functions that
// provide the interface between the script language and the application.

#include "global.h"

#include "script_lib_vector3d.h"
#include "algebra.h"
#include "vm_stack.h"
#include "vm_thread.h"


///////////////////////////////////////////////////////////////////////////////
// script library class: vector3d
///////////////////////////////////////////////////////////////////////////////

// read a vector3d value from a stream
void slc_vector3d_t::read_value( chunk_file& fs, char* buf )
{
  serial_in( fs, (vector3d*)buf );
}


// script library constructor:  vector3d::vector3d(num,num,num)
class slf_vector3d_construct_t : public script_library_class::function
{
public:
  // constructor required
  slf_vector3d_construct_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vm_num_t x;
    vm_num_t y;
    vm_num_t z;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    /*  Code not necessary for now:  vector3d is equivalent to what's on the vm_stack!
        SLF_PARMS;
        vector3d result = vector3d(parms->x,parms->y,parms->z);
        SLF_RETURN;
    */
    SLF_DONE;
  }
};
//slf_vector3d_construct_t slf_vector3d_construct(slc_vector3d,"vector3d(num,num,num)");

// script library function:  vector3d vector3d::operator+(vector3d)
class slf_vector3d_add_t : public script_library_class::function
{
public:
  // constructor required
  slf_vector3d_add_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vector3d me;
    vector3d b;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vector3d result = parms->me + parms->b;
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_vector3d_add_t slf_vector3d_add(slc_vector3d,"operator+(vector3d)");

// script library function:  vector3d vector3d::operator-(vector3d)
class slf_vector3d_subtract_t : public script_library_class::function
{
public:
  // constructor required
  slf_vector3d_subtract_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vector3d me;
    vector3d b;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vector3d result = parms->me - parms->b;
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_vector3d_subtract_t slf_vector3d_subtract(slc_vector3d,"operator-(vector3d)");

// script library function:  vector3d vector3d::operator*(num)
class slf_vector3d_mul_scalar_t : public script_library_class::function
{
public:
  // constructor required
  slf_vector3d_mul_scalar_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vector3d me;
    vm_num_t s;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vector3d result = parms->me * parms->s;
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_vector3d_mul_scalar_t slf_vector3d_mul_scalar(slc_vector3d,"operator*(num)");

// script library function:  vector3d vector3d::operator/(num)
class slf_vector3d_div_scalar_t : public script_library_class::function
{
public:
  // constructor required
  slf_vector3d_div_scalar_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vector3d me;
    vm_num_t s;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vector3d result = parms->me / parms->s;
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_vector3d_div_scalar_t slf_vector3d_div_scalar(slc_vector3d,"operator/(num)");

// script library function:  num vector3d::operator==(vector3d)
class slf_vector3d_equal_t : public script_library_class::function
{
public:
  // constructor required
  slf_vector3d_equal_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vector3d me;
    vector3d b;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_num_t result = (parms->me == parms->b);
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_vector3d_equal_t slf_vector3d_equal(slc_vector3d,"operator==(vector3d)");

// script library function:  num vector3d::operator!=(vector3d)
class slf_vector3d_not_equal_t : public script_library_class::function
{
public:
  // constructor required
  slf_vector3d_not_equal_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vector3d me;
    vector3d b;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_num_t result = (parms->me != parms->b);
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_vector3d_not_equal_t slf_vector3d_not_equal(slc_vector3d,"operator!=(vector3d)");

// script library function:  num vector3d::length()
class slf_vector3d_length_t : public script_library_class::function
{
public:
  // constructor required
  slf_vector3d_length_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vector3d me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_num_t result = parms->me.length();
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_vector3d_length_t slf_vector3d_length(slc_vector3d,"length()");

// script library function:  num vector3d::length2()
class slf_vector3d_length2_t : public script_library_class::function
{
public:
  // constructor required
  slf_vector3d_length2_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vector3d me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_num_t result = parms->me.length2();
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_vector3d_length2_t slf_vector3d_length2(slc_vector3d,"length2()");

// script library function:  num vector3d::xy_norm()
class slf_vector3d_xy_norm_t : public script_library_class::function
{
public:
  // constructor required
  slf_vector3d_xy_norm_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vector3d me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_num_t result = parms->me.xy_length();
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_vector3d_xy_norm_t slf_vector3d_xy_norm(slc_vector3d,"xy_norm()");

// script library function:  num vector3d::xz_norm()
class slf_vector3d_xz_norm_t : public script_library_class::function
{
public:
  // constructor required
  slf_vector3d_xz_norm_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vector3d me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_num_t result = parms->me.xz_length();
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_vector3d_xz_norm_t slf_vector3d_xz_norm(slc_vector3d,"xz_norm()");


// script library function:  num vector3d::x()
class slf_vector3d_x_t : public script_library_class::function
{
public:
  // constructor required
  slf_vector3d_x_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vector3d me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_num_t result = parms->me.x;
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_vector3d_x_t slf_vector3d_x(slc_vector3d,"x()");


// script library function:  num vector3d::y()
class slf_vector3d_y_t : public script_library_class::function
{
public:
  // constructor required
  slf_vector3d_y_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vector3d me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_num_t result = parms->me.y;
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_vector3d_y_t slf_vector3d_y(slc_vector3d,"y()");


// script library function:  num vector3d::z()
class slf_vector3d_z_t : public script_library_class::function
{
public:
  // constructor required
  slf_vector3d_z_t(script_library_class* slc,const char* n) : script_library_class::function(slc,n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vector3d me;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_num_t result = parms->me.z;
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_vector3d_z_t slf_vector3d_z(slc_vector3d,"z()");


// global script library function:  num dot(vector3d,vector3d)
class slf_dot3_t : public script_library_class::function
{
public:
  // constructor required
  slf_dot3_t(const char* n) : script_library_class::function(n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vector3d v1;
    vector3d v2;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_num_t result = dot(parms->v1,parms->v2);
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_dot3_t slf_dot3("dot(vector3d,vector3d)");

// global script library function:  vector3d cross(vector3d,vector3d)
class slf_cross3_t : public script_library_class::function
{
public:
  // constructor required
  slf_cross3_t(const char* n) : script_library_class::function(n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vector3d v1;
    vector3d v2;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vector3d result = cross(parms->v1,parms->v2);
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_cross3_t slf_cross3("cross(vector3d,vector3d)");

// global script library function:  vector3d normal(vector3d)
class slf_normal3_t : public script_library_class::function
{
public:
  // constructor required
  slf_normal3_t(const char* n) : script_library_class::function(n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vector3d v1;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vector3d result = parms->v1.normalize();
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_normal3_t slf_normal3("normal(vector3d)");

// global script library function:  num distance3d(vector3d,vector3d)
class slf_distance3d_t : public script_library_class::function
{
public:
  // constructor required
  slf_distance3d_t(const char* n) : script_library_class::function(n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vector3d v1;
    vector3d v2;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_num_t result = (parms->v1-parms->v2).length();
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_distance3d_t slf_distance3d("distance3d(vector3d,vector3d)");

// global script library function:  num angle_between(vector3d,vector3d)
class slf_angle_between3_t : public script_library_class::function
{
public:
  // constructor required
  slf_angle_between3_t(const char* n) : script_library_class::function(n) {}
  // library function parameters
  struct parms_t
  {
    // parameters
    vector3d v1;
    vector3d v2;
  };
  // library function execution
  virtual bool operator()(vm_stack& stack,entry_t entry)
  {
    SLF_PARMS;
    vm_num_t result = angle_between(parms->v1,parms->v2);
    SLF_RETURN;
    SLF_DONE;
  }
};
//slf_angle_between3_t slf_angle_between3("angle_between(vector3d,vector3d)");





void register_vector3d_lib()
{
  // pointer to single instance of library class
  slc_vector3d_t* slc_vector3d = NEW slc_vector3d_t("vector3d",12);

  NEW slf_vector3d_construct_t(slc_vector3d,"vector3d(num,num,num)");
  NEW slf_vector3d_add_t(slc_vector3d,"operator+(vector3d)");
  NEW slf_vector3d_subtract_t(slc_vector3d,"operator-(vector3d)");
  NEW slf_vector3d_mul_scalar_t(slc_vector3d,"operator*(num)");
  NEW slf_vector3d_div_scalar_t(slc_vector3d,"operator/(num)");
  NEW slf_vector3d_equal_t(slc_vector3d,"operator==(vector3d)");
  NEW slf_vector3d_not_equal_t(slc_vector3d,"operator!=(vector3d)");
  NEW slf_vector3d_length_t(slc_vector3d,"length()");
  NEW slf_vector3d_length2_t(slc_vector3d,"length2()");
  NEW slf_vector3d_xy_norm_t(slc_vector3d,"xy_norm()");
  NEW slf_vector3d_xz_norm_t(slc_vector3d,"xz_norm()");
  NEW slf_vector3d_x_t(slc_vector3d,"x()");
  NEW slf_vector3d_y_t(slc_vector3d,"y()");
  NEW slf_vector3d_z_t(slc_vector3d,"z()");
  NEW slf_dot3_t("dot(vector3d,vector3d)");
  NEW slf_cross3_t("cross(vector3d,vector3d)");
  NEW slf_normal3_t("normal(vector3d)");
  NEW slf_distance3d_t("distance3d(vector3d,vector3d)");
  NEW slf_angle_between3_t("angle_between(vector3d,vector3d)");
}
