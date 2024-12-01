/* Passes a buffer to the write system call that starts in valid memory, but runs into kernel space.
   The process must be terminated with -1 exit code. 
*/

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  int handle;
  CHECK ((handle = open ("sample.txt")) > 1, "open \"sample.txt\"");

  write (handle, (char *) 0xbffffff0, 32);
  fail ("should have exited with -1");
}