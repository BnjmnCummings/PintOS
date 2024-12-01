/* Passes a buffer to the read system call that starts in valid memory, but runs into kernel space.
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

  read (handle, (char *) 0xbfffffe0, 100);
  fail ("should not have survived read()");
}
