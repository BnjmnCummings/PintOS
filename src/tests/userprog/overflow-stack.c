/* Attempt to overflow the user stack by allocating a 4kB buffer and writing into it.
   The process must be terminated with -1 exit code until stack growth has been implemented in Task 3
*/

#include <string.h>
#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void)
{
  char stack_obj[4096];
  memset (stack_obj, 'a', sizeof stack_obj);
  memset (stack_obj+10, '\0', 1);
  msg ("buffer: %s", stack_obj);
}