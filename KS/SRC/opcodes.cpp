// opcodes.cpp
#include "global.h"
#include "opcodes.h"


const char* opcode_t_str[] =
  {
  #define ENUMMAC(sym,str) str,
  #include "opcode_t"
  #undef ENUMMAC
  };

const char* opcode_arg_t_str[] =
  {
  #define ENUMMAC(sym,str) str,
  #include "opcode_arg_t"
  #undef ENUMMAC
  };

