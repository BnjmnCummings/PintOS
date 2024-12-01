/* Waits for an invalid pid.  This may fail, returning -1 from wait, 
   or terminate the process with -1 exit code.                    */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  int retval = wait ((pid_t) 0x0c020301);
  if (retval != -1)
    fail ("process should have called exit(-1) or invalid wait syscall should have returned -1");
}
