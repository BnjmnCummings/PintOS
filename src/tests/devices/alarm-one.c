/* Tests timer_sleep(1), which should return shortly after called.
   This test can expose a race-condition if the kernel attempts to unblock
   the thread before it has been blocked.
 */

#include <stdio.h>
#include "tests/devices/tests.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "devices/timer.h"

void
test_alarm_one (void) 
{
  timer_sleep (1);
  pass ();
} 
