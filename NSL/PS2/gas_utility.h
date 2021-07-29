/********************************************************
 * gas_utility.h
 * Kevin Schmidt 6/29/01 - schmidt@treyarch.com
 * For NSL
 ********************************************************
 * This file simply provides a few utilities 
 * useful in dealing with GAS.
 ********************************************************/


#ifndef GAS_UTILITY_H
#define GAS_UTILITY_H
#include "ps2/gas.h"
// Simplify talking to gas
int nslPs2GasRpc (int command, const char *strarg, int intarg0, int intarg1, int intarg2, int intarg3 );
int nslPs2GasRpcInit (void);

#endif