// membudget.h
#ifndef MEMORY_BUDGET_HEADER
#define MEMORY_BUDGET_HEADER

#include "singleton.h"
#include <vector>

///////////////////////////////////////////////////////////////////////////////
// SINGLETON membudget_t; accessed via membudget()->
//
// This singleton maintains the memory budget for the application.

#include "singleton.h"
#include <vector>

class membudget_t : public singleton
{
public:
  // returns a pointer to the single instance
  DECLARE_SINGLETON( membudget_t )

// Types
public:
  enum category_t
  {
    #define MAC(a,b) a,
    #include "membudget_categories.h"
    #undef MAC
    N_CATEGORIES,
  };

// Data
private:
  vector<int> caps;
  vector<int> usages;

// Constructors (not public in a singleton)
private:
  membudget_t();
  membudget_t( const membudget_t& );
  membudget_t& operator=( const membudget_t& );

// Methods
public:
  int get_cap( category_t c ) const { return caps[c]; }
  void use( category_t c, int v ) { usages[c] += v; }
  void unuse( category_t c, int v ) { usages[c] -= v; }
  int get_usage( category_t c ) const { return usages[c]; }
  void reset_usage( category_t c ) { usages[c] = 0; }
  void reset_usages();
};

// provide more convenient syntax for singleton access
inline membudget_t* membudget() { return membudget_t::inst(); }


///////////////////////////////////////////////////////////////////////////////
// membudget nonmember interface functions
///////////////////////////////////////////////////////////////////////////////

// report violations of memory budget (generates hard error)
void report_membudget_violations();

// return char string label for given membudget category
const char* get_membudget_category_name( membudget_t::category_t c );

#endif