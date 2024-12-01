/* Wait for a process that will be killed for bad behavior, ensuring it loaded correctly. */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  int pid = exec ("child-bad");
  if (pid == -1){
    fail ("exec() returned invalid pid: %d", pid);
  }
  msg ("exec() returned a valid pid");
  msg ("wait() = %d", wait (pid));
}
