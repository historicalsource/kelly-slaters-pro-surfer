// opcodes.h
#ifndef _OPCODES_H
#define _OPCODES_H


// The structure of an operation in the virtual machine language is:
//   opcode (8 bits), argument type (8 bits), argument(s)
enum opcode_t
  {
  #define ENUMMAC(sym,str) sym,
  #include "opcode_t"
  #undef ENUMMAC
  OP_INVALID
  };
enum opcode_arg_t
  {
  #define ENUMMAC(sym,str) sym,
  #include "opcode_arg_t"
  #undef ENUMMAC
  OP_ARG_INVALID
  };
#define OP_DSIZE_FLAG   0x0080
#define OP_ARGTYPE_MASK 0x007F

extern const char* opcode_t_str[];
extern const char* opcode_arg_t_str[];


#endif  // _OPCODES_H
