/*-------------------------------------------------------------------------------------------------------
  TYPES.H - simple data types
-------------------------------------------------------------------------------------------------------*/
#ifndef TYPES_H
#define TYPES_H

// some useful type definitions.
typedef signed char        int8;
typedef unsigned char      uint8;
typedef short signed int   int16;
typedef short unsigned int uint16;
typedef signed int         int32;
typedef unsigned int       uint32;

#if defined(TARGET_PC) || defined(TARGET_XBOX)
typedef __int64 int64;
typedef unsigned __int64 uint64;
#elif defined(TARGET_PS2)
typedef long int64;
typedef unsigned long uint64;
#elif defined(TARGET_GC)
typedef long long int64;
typedef unsigned long long uint64;

typedef unsigned int u_int;

#elif defined(TARGET_NULL)
typedef long int64;
typedef unsigned long uint64;
#endif

typedef float              float32;
typedef double             float64;

#endif
