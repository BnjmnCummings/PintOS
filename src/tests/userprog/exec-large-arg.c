/* Tests argument passing to child processes for a single large argument. */

#include <syscall.h>
#include "tests/main.h"

void
test_main (void) 
{
  wait (exec ("child-args childaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaarrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrggggggggggggggggggggggggggggggggggggggggggg!!!!!!!!!!!!!!!!!!!!!!!!!!!.............................."));
}
