/* Executes and waits for a single child process. */

#include <syscall.h>
#include "tests/lib.h"

const char *test_name = "exec-exit";

int
main (void) 
{
  msg ("run");
  exit (exec ("child-simple"));
}
