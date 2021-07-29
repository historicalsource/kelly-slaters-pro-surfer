#include "global.h"

#include "membudget.h"


///////////////////////////////////////////////////////////////////////////////
// SINGLETON membudget_t; accessed via membudget()->
//
// This singleton maintains the memory budget for the application.

DEFINE_SINGLETON( membudget_t )


const char* membudget_category_names[] =
{
  #define MAC(a,b) #a,
  #include "membudget_categories.h"
  #undef MAC
};


membudget_t::membudget_t()
: caps( N_CATEGORIES, 0 ),
  usages( N_CATEGORIES, 0 )
{
#define MAC(a,b) caps[a]=b;
#include "membudget_categories.h"
#undef MAC
}


void membudget_t::reset_usages()
{
  for (vector<int>::iterator i = usages.begin(); i!=usages.end(); ++i)
    *i = 0;
}


///////////////////////////////////////////////////////////////////////////////
// membudget nonmember interface functions
///////////////////////////////////////////////////////////////////////////////

// report violations of memory budget (generates hard error)
void report_membudget_violations()
{
#ifndef BUILD_BOOTABLE
  bool err = false;
  int i;
  for ( i=0; i<membudget_t::N_CATEGORIES; ++i )
  {
    int usage = membudget()->get_usage( (membudget_t::category_t)i );
    int cap = membudget()->get_cap( (membudget_t::category_t)i );
    if ( usage > cap )
    {
      warning( "Out of memory for " + stringx(get_membudget_category_name((membudget_t::category_t)i)) +
               ": used=" + itos(usage) +
               ", budgeted=" + itos(cap) );
      err = true;
    }
  }
 #ifdef TARGET_MKS
  if ( err )
    error( "Program terminated due to one or more memory budget violations." );
 #endif
#endif // !BUILD_BOOTABLE
}


// return char string label for given membudget category
const char* get_membudget_category_name( membudget_t::category_t c )
{
  return membudget_category_names[c];
}
