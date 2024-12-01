/* Fakes a system call with a bogus system call number at the very top of the
   stack. This system call is invalid, so the process must either be terminated
   with -1 exit code, or the system call must return an error value of -1.   */

#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{

  /* Invokes a system call with an invalid system call number at esp. */
  int retval;
  asm volatile ("pushl %[arg0]; pushl %[number]; int $0x30; addl $8, %%esp"
                 : "=a" (retval)                                           
                 : [number] "i" (42),                                        
                   [arg0] "g" (42)                                           
                 : "memory");
  msg ("invalid syscall did not terminate the process, so check its return value...");
  if (retval != -1)
    fail ("process should have called exit(-1) or invalid syscall should have returned -1");
}

