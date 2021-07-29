#include "pstring.h"
#include <stdio.h>
#include "global.h"
#include "game.h"

//H safe_global_variable_block* syvars;

void system_idle() // do system message-processing so the app doesn't appear hung to the OS
{
}

int main() {
  pstring bobname = "bob";

  printf("Bob's name is %s\n", bobname.c_str());

  return 0;
}
