/* The main thread creates and acquires a lock. Then it creates two
   higher-priority threads (one medium priority and one high priority)
   that block on acquiring the lock, causing them to donate their 
   priorities to the main thread. 
   When the main thread releases the lock, the high priority thread 
   should acquire the lock, but then it will immediately drop its
   priority below that of the main thread.
   If the lock donations have been correctly preserved, then the
   high priority thread should still run with medium priority, and
   thus continue running without yielding to the main thread.
*/

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/init.h"
#include "threads/synch.h"
#include "threads/thread.h"

static thread_func medium_thread_func;
static thread_func high_thread_func;

void
test_priority_preservation (void) 
{
  msg ("main-thread starting...");  
  struct lock lock;

  /* This test does not work with the MLFQS. */
  ASSERT (!thread_mlfqs);

  /* Make sure our priority is the default. */
  ASSERT (thread_get_priority () == PRI_DEFAULT);

  lock_init (&lock);
  lock_acquire (&lock);
  
  msg("main-thread creating medium-priority thread...");
  thread_create ("medium-priority", PRI_DEFAULT + 5, medium_thread_func, &lock);
  msg ("main-thread continuing...");
  msg ("This thread should have priority %d.  Actual priority: %d.",
       PRI_DEFAULT + 5, thread_get_priority ());
  
  msg("main-thread creating high-priority thread...");
  thread_create ("high-priority", PRI_DEFAULT + 10, high_thread_func, &lock);
  msg ("main-thread continuing...");
  msg ("This thread should have priority %d.  Actual priority: %d.",
       PRI_DEFAULT + 10, thread_get_priority ());
  
  msg ("main-thread now releasing the lock...");
  lock_release (&lock);
  
  msg ("medium-priority thread must already have finished.");
  msg ("This should be the last line before finishing this test.");
}

static void
medium_thread_func (void *lock_) 
{
  msg ("medium-priority thread starting...");
  struct lock *lock = lock_;

  msg ("medium-priority thread trying to acquire the lock...");
  lock_acquire (lock);
  msg ("medium-priority thread got the lock.");
  lock_release (lock);
  msg ("medium-priority thread done.");
}

static void
high_thread_func (void *lock_) 
{
  msg ("high-priority thread starting...");  
  struct lock *lock = lock_;

  msg ("high-priority thread trying to acquire the lock...");
  lock_acquire (lock);
  msg ("high-priority thread got the lock.");
  msg ("high-priority thread about to drop to low priority...");
  thread_set_priority (PRI_DEFAULT - 10);
  msg ("This thread should still have effective priority %d.  Actual priority: %d.",
       PRI_DEFAULT +5, thread_get_priority ());
  lock_release (lock);
  msg ("We should not see this message, as pintos will close when the main-thread terminates.");
}
