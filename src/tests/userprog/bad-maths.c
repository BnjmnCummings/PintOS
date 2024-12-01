/* This program attempts to divide by zero, which will trigger an internal interrupt.
   This should terminate the process with a -1 exit code. */

#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  msg ("Congratulations - you have successfully divided by zero: %d", 1/0);
  fail ("should have exited with -1");
}
