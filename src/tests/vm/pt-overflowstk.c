/* Recursively allocates and writes 64 kB objects on the stack until we overflow.
   Expects to be able to recurse at least 30 times (consuming ~2MB stack usage).
   The process must be terminated with -1 exit code 
   BEFORE reaching a recursive depth of 1,562 (~100MB stack usage). */

#include <string.h>
#include "tests/arc4.h"
#include "tests/lib.h"
#include "tests/main.h"

void recurse_count(int count);
void recurse_to_overflow(int count);

void
test_main (void)
{
  int i = 1;
  recurse_count(i);
}

void recurse_count(int count)
{
  // consume 64kB of the stack
  char stk_obj[65536];
  struct arc4 arc4;

  // perform some operations that write to the stack
  arc4_init (&arc4, "foobar", 6);
  memset (stk_obj, 0, sizeof stk_obj);
  arc4_crypt (&arc4, stk_obj, sizeof stk_obj);
  msg ("grew stack %d times", count++);    
  
  // recurse (only print up to 30 times)
  if(count <= 30){
    recurse_count(count);
  } else {
    recurse_to_overflow(count);
  }
}

void recurse_to_overflow(int count)
{
  if(count >= 1562){
    fail ("stack has grown beyond any reasonable limit (>100MB)");
  }
  
  // consume 64kB of the stack
  char stk_obj[65536];
  struct arc4 arc4;

  // perform some operations that write to the stack
  arc4_init (&arc4, "foobar", 6);
  memset (stk_obj, 0, sizeof stk_obj);
  arc4_crypt (&arc4, stk_obj, sizeof stk_obj);
  
  // recurse so that we will eventually overflow the stack
  recurse_to_overflow(++count);
}
