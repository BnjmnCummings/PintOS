/* Waits for an indirect child process (grandchild).  
   This must fail, returning -1 from wait. */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  int grandchild = wait (exec ("exec-exit"));
  msg ("grandchild should be running by now");
  msg ("waiting for grandchild returned: %d", wait(grandchild));
}
