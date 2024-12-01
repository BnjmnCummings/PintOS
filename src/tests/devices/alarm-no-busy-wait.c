/* Creates 5 threads, each of which sleeps for 20 ticks. 
   Checks to ensure that the threads are genuinely sleeping (not busy-waiting)
   by inspecting the ready_list half-way though the sleep time. 
*/

#include <stdio.h>
#include "tests/devices/tests.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "devices/timer.h"

static void test_sleep (int thread_cnt, int iterations);

void
test_alarm_no_busy_wait (void) 
{
  test_sleep (5, 1);
}

/* Information about the test. */
struct sleep_test 
  {
    int64_t start;              /* Current time at start of test. */
    int iterations;             /* Number of iterations per thread. */

    /* Output. */
    struct lock output_lock;    /* Lock protecting output buffer. */
    int *output_pos;            /* Current position in output buffer. */
  };

/* Information about an individual thread in the test. */
struct sleep_thread 
  {
    struct sleep_test *test;     /* Info shared between all threads. */
    int id;                     /* Sleeper ID. */
    int duration;               /* Number of ticks to sleep. */
    int iterations;             /* Iterations counted so far. */
  };

static void sleeper (void *);

/* Runs THREAD_CNT threads thread sleep ITERATIONS times each. */
static void
test_sleep (int thread_cnt, int iterations) 
{
  struct sleep_test test;
  struct sleep_thread *threads;
  int *output;
  int i;

  /* This test does not work with the MLFQS. */
  ASSERT (!thread_mlfqs);

  msg ("Creating %d threads to sleep %d times each.", thread_cnt, iterations);
  msg ("Each thread sleeps for 20 ticks at a time,");
  msg ("Test is successful if the threads are not running"); 
  msg ("when they are supposed to be asleep.");

  /* Allocate memory. */
  threads = malloc (sizeof *threads * (int)(thread_cnt));
  output = malloc (sizeof *output * (int)(iterations * thread_cnt * 2));
  if (threads == NULL || output == NULL)
    PANIC ("couldn't allocate memory for test");

  /* Initialize test. */
  test.start = timer_ticks () + 100;
  test.iterations = iterations;
  lock_init (&test.output_lock);
  test.output_pos = output;

  /* Start threads. */
  ASSERT (output != NULL);
  for (i = 0; i < thread_cnt; i++)
    {
      struct sleep_thread *t = threads + i;
      char name[16];
      
      t->test = &test;
      t->id = i;
      t->duration = 20;
      t->iterations = 0;

      snprintf (name, sizeof name, "thread %d", i);
      thread_create (name, PRI_DEFAULT, sleeper, t);
    }
  
  /* yield the CPU so that the new threads have sufficient time to go to sleep */
  timer_sleep(10);
  
  /* now check that all of the threads are indeed asleep */
  
  /* Acquire the output lock so we cannot race with the sleeping threads */
  lock_acquire (&test.output_lock);

  /* Inspect and print the number of threads on the ready list . */
  size_t num_ready_threads = threads_ready();
  msg("%d threads on the ready list", num_ready_threads);
  
  if (num_ready_threads > 0)
      fail ("too many threads on the ready_list (they should all be asleep)!");

  pass ();  
  lock_release (&test.output_lock);
  
  /* Wait long enough for all the threads to finish. */
  timer_sleep (100 + thread_cnt * iterations * 20 + 100);
  
  /* Clean up after ourselves */
  free (output);
  free (threads);
}

/* Sleeper thread. */
static void
sleeper (void *t_) 
{
  struct sleep_thread *t = t_;
  struct sleep_test *test = t->test;
  int i;

  for (i = 1; i <= test->iterations; i++) 
    {
      int64_t sleep_until = test->start + i * t->duration;
      timer_sleep (sleep_until - timer_ticks ());
      lock_acquire (&test->output_lock);
      *test->output_pos++ = t->id;
      lock_release (&test->output_lock);
    }
}
