#ifndef WARNLVL_H
#define WARNLVL_H

////////////////////////////////////////////////////////////////////////////////
/*
  turn off stupid warnings and turn on smart ones.
*/
////////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER

#pragma warning(3: 4706)  // assignment within conditional expression: turning on
#pragma warning(4: 4018)  // signed/unsigned mismatch

// let's turn on these warnings and get rid of some of these double constants!
#pragma warning(4: 4114)  // return: truncation from 'const double' to 'float'
#pragma warning(4: 4244)  // return: truncation from 'const double' to 'float'
#pragma warning(4: 4305)  // truncation from 'const double' to 'float'

#pragma warning(4: 4786)  // identifier truncated
#pragma warning(4: 4800)  // performance warning, int to bool
#pragma warning(4: 4804)  // unsafe use of bool.  (I don't feel sanguine about turning this one off)
//#pragma warning(4: 4530)  // C++ exception handler used, but unwind semantics...
#pragma warning(disable: 4786) // disable annoying debug info warning

#pragma warning(disable: 4065) // disable annoying default but no case warning

#endif // _MSC_VER

#endif // WARNLVL_H