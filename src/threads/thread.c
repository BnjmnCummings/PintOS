#include "threads/thread.h"
#include <debug.h>
#include <stddef.h>
#include <random.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "devices/timer.h"
#include "threads/flags.h"
#include "threads/interrupt.h"
#include "threads/intr-stubs.h"
#include "threads/palloc.h"
#include "threads/switch.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "threads/fixed-point.h"
#include "threads/malloc.h"
#ifdef USERPROG
#include "userprog/process.h"
#include "userprog/syscall.h"
#include "filesys/file.h"
#endif

/* Random value for struct thread's `magic' member.
   Used to detect stack overflow.  See the big comment at the top
   of thread.h for details. */
#define THREAD_MAGIC 0xcd6abf4b

/* List of processes in THREAD_READY state, that is, processes
   that are ready to run but not actually running. */
static struct list ready_list;

/* List of all processes.  Processes are added to this list
   when they are first scheduled and removed when they exit. */
static struct list all_list;

/* Idle thread. */
static struct thread *idle_thread;

/* Initial thread, the thread running init.c:main(). */
static struct thread *initial_thread;

/* Lock used by allocate_tid(). */
static struct lock tid_lock;

/* array of thread queues with priority equal to the index where they are stored.
   Threads with the same priority are stored in a list run in round-robin order. */
static struct list queue_array[QUEUE_ARRAY_SIZE];

/* Stack frame for kernel_thread(). */
struct kernel_thread_frame 
  {
    void *eip;                  /* Return address. */
    thread_func *function;      /* Function to call. */
    void *aux;                  /* Auxiliary data for function. */
  };

/* Statistics. */
static long long idle_ticks;    /* # of timer ticks spent idle. */
static long long kernel_ticks;  /* # of timer ticks in kernel threads. */
static long long user_ticks;    /* # of timer ticks in user programs. */
static int32_t load_avg;      /* Minutely estimate # of ready threads. */

/* Scheduling. */
#define TIME_SLICE 4            /* # of timer ticks to give each thread. */
static unsigned thread_ticks;   /* # of timer ticks since last yield. */

/* If false (default), use round-robin scheduler.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "-mlfqs". */
bool thread_mlfqs;

static void kernel_thread (thread_func *, void *aux);

static void idle (void *aux UNUSED);
static struct thread *running_thread (void);
static struct thread *next_thread_to_run (void);
static void init_thread (struct thread *, const char *name, int priority);
static bool is_thread (struct thread *) UNUSED;
static void *alloc_frame (struct thread *, size_t size);
static void schedule (void);
void thread_schedule_tail (struct thread *prev);
static tid_t allocate_tid (void);
static void thread_update_recent_cpu (struct thread *t, void *aux UNUSED);
static void thread_update_priority (struct thread *t, void *aux UNUSED);
static void recalculate_priority (struct thread *t);
static void mlfq_init (void);
static int mlfq_highest_priority (void);
static bool mlfq_is_empty (void);
static void mlfq_insert (struct thread *t);

static unsigned child_elem_hash (const struct hash_elem *, void * UNUSED);
static bool child_elem_less (const struct hash_elem *, const struct hash_elem *, void * UNUSED);
static void free_children (struct hash_elem *, void * UNUSED);
static void free_file (struct hash_elem *, void * UNUSED);

/* Initializes the threading system by transforming the code
   that's currently running into a thread.  This can't work in
   general and it is possible in this case only because loader.S
   was careful to put the bottom of the stack at a page boundary.

   Also initializes the run queue and the tid lock.

   After calling this function, be sure to initialize the page
   allocator before trying to create any threads with
   thread_create().

   It is not safe to call thread_current() until this function
   finishes. */
void
thread_init (void) 
{
  ASSERT (intr_get_level () == INTR_OFF);

  lock_init (&tid_lock);

  list_init (&all_list);

  /* Initialise mlfqs array */
  if (thread_mlfqs)
    mlfq_init ();
  else {
    list_init (&ready_list);
  }

  /* Initialise system-wide load average as zero */
  load_avg = INITIAL_LOAD_AVG;

  /* Set up a thread structure for the running thread. */
  initial_thread = running_thread ();
  init_thread (initial_thread, "main", PRI_DEFAULT);
  initial_thread->status = THREAD_RUNNING;
  initial_thread->tid = allocate_tid ();

  /* Initialise start thread niceness and recent_cpu */
  initial_thread->nice = NICE_DEFAULT;
  initial_thread->recent_cpu = RECENT_CPU_DEFAULT;

}

/* Starts preemptive thread scheduling by enabling interrupts.
   Also creates the idle thread. */
void
thread_start (void) 
{
  hash_init (&thread_current()->children, child_elem_hash, child_elem_less, NULL);
  hash_init (&thread_current()->files, file_elem_hash, file_elem_less, NULL);

  /* Create the idle thread. */
  struct semaphore idle_started;
  sema_init (&idle_started, 0);
  thread_create ("idle", PRI_MIN, idle, &idle_started);

  /* Start preemptive thread scheduling. */
  intr_enable ();

  /* Wait for the idle thread to initialize idle_thread. */
  sema_down (&idle_started);
}

/* Returns the number of threads currently in the ready list. 
   Disables interrupts to avoid any race-conditions on the ready list. */
size_t
threads_ready (void)
{
  enum intr_level old_level = intr_disable ();
  size_t ready_thread_count;
  /* Sum the number of threads in each index if mlfqs is active */
  if (thread_mlfqs) {
    ready_thread_count = 0;
    for (int i = 0; i < QUEUE_ARRAY_SIZE; i++) {
      ready_thread_count += list_size (&queue_array[i]);
    }
  } else {
    ready_thread_count = list_size (&ready_list);
  }
  intr_set_level (old_level);
  return ready_thread_count;
}

/* Called by the timer interrupt handler at each timer tick.
   Thus, this function runs in an external interrupt context. */
void
thread_tick (void) 
{
  struct thread *t = thread_current ();

  /* Update statistics. */
  if (t == idle_thread)
    idle_ticks++;
#ifdef USERPROG
  else if (t->pagedir != NULL)
    user_ticks++;
#endif
  else
    kernel_ticks++;

  if (thread_mlfqs) {
    /* Increments running thread's recent CPU usage */
    if (t != idle_thread)
      t->recent_cpu = FIXED_ADD_INT (t->recent_cpu, 1);

    /* Updates statistics every second */
    if (timer_ticks () % TIMER_FREQ == 0) {
      /* Calculate new load average */
      int32_t old = load_avg;
      int32_t prev = FIXED_DIV_INT (FIXED_MUL_INT (old, 59), 60);
      int32_t new = FIXED_DIV_INT (INT_TO_FIXED (threads_ready () + (t != idle_thread)), 60);
      load_avg = FIXED_ADD (prev, new);

      /* Update recent CPU usage value for every thread */
      thread_foreach (thread_update_recent_cpu, NULL);
    } else if (timer_ticks () % PRI_UPDATE_FREQUENCY == 0) {
      /* Updates thread priority every 4 ticks */
      thread_update_priority(thread_current(), NULL);
    }
    check_prio (mlfq_highest_priority ());
  }

  /* Enforce preemption. */
  if (++thread_ticks >= TIME_SLICE)
    intr_yield_on_return ();
}

/* Auxilliary function that updates recent CPU values. */
static void
thread_update_recent_cpu (struct thread *t, void *aux UNUSED)
{
  ASSERT (thread_mlfqs);
  ASSERT (intr_get_level () == INTR_OFF);

  /* Coefficient for recent CPU calculation */
  int32_t numer = FIXED_MUL_INT (load_avg, 2);
  int32_t denom = FIXED_ADD_INT (numer, 1);
  int32_t coeff = FIXED_DIV (numer, denom);

  t->recent_cpu = FIXED_ADD_INT (FIXED_MUL (coeff, t->recent_cpu), t->nice);

  /* Also update priority every 100 ticks */
  thread_update_priority(t, NULL);
}

/* Function that updates the given thread's priority
   based its recent_cpu and niceness */
static void
thread_update_priority (struct thread *t, void *aux UNUSED)
{
  ASSERT (thread_mlfqs);
  ASSERT (intr_get_level () == INTR_OFF);

  if (t->status == THREAD_BLOCKED || t == idle_thread) {
    return;
  }

  int32_t recent_cpu_quarter = FIXED_DIV_INT (t->recent_cpu, 4);
  int32_t priority_difference = FIXED_ADD_INT (recent_cpu_quarter, (t->nice * 2));
  int32_t new_priority = FIXED_TO_INT_TRUNC (INT_SUB_FIXED (PRI_MAX, priority_difference));

  /* Caps priority between PRI_MIN and PRI_MAX */
  if (new_priority < PRI_MIN) {
    new_priority = PRI_MIN;
  } else if (new_priority > PRI_MAX) {
    new_priority = PRI_MAX;
  }

  /* Reserve space in the queue if priority is unchanged */
  if (t->priority != new_priority) {
    t->priority = new_priority;

    /* We check if updating should cause thread to yield elsewhere */
    if (t->status == THREAD_READY) {
      list_remove (&t->elem);
      mlfq_insert (t);
    }
  }
}

/* Prints thread statistics. */
void
thread_print_stats (void) 
{
  printf ("Thread: %lld idle ticks, %lld kernel ticks, %lld user ticks\n",
          idle_ticks, kernel_ticks, user_ticks);
}

/* Creates a new kernel thread named NAME with the given initial
   PRIORITY, which executes FUNCTION passing AUX as the argument,
   and adds it to the ready queue.  Returns the thread identifier
   for the new thread, or TID_ERROR if creation fails.

   If thread_start() has been called, then the new thread may be
   scheduled before thread_create() returns.  It could even exit
   before thread_create() returns.  Contrariwise, the original
   thread may run for any amount of time before the new thread is
   scheduled.  Use a semaphore or some other form of
   synchronization if you need to ensure ordering.

   The code provided sets the new thread's `priority' member to
   PRIORITY, but no actual priority scheduling is implemented.
   Priority scheduling is the goal of Problem 1-3. */
tid_t
thread_create (const char *name, int priority,
               thread_func *function, void *aux) 
{
  struct thread *t;
  struct kernel_thread_frame *kf;
  struct switch_entry_frame *ef;
  struct switch_threads_frame *sf;
  tid_t tid;
  enum intr_level old_level;

  ASSERT (function != NULL);
  /* Allocate thread. */
  t = palloc_get_page (PAL_ZERO);
  if (t == NULL)
    return TID_ERROR;

  /* Initialize thread. */
  init_thread (t, name, priority);
  tid = t->tid = allocate_tid ();

  /* Prepare thread for first run by initializing its stack.
     Do this atomically so intermediate values for the 'stack' 
     member cannot be observed. */
  old_level = intr_disable ();

  /* Stack frame for kernel_thread(). */
  kf = alloc_frame (t, sizeof *kf);
  kf->eip = NULL;
  kf->function = function;
  kf->aux = aux;

  /* Stack frame for switch_entry(). */
  ef = alloc_frame (t, sizeof *ef);
  ef->eip = (void (*) (void)) kernel_thread;

  /* Stack frame for switch_threads(). */
  sf = alloc_frame (t, sizeof *sf);

  sf->eip = switch_entry;
  sf->ebp = 0;

  /* inherit niceness and recent_cpu from current thread */
  t->nice = thread_current ()->nice;
  t->recent_cpu = thread_current ()->recent_cpu;

#ifdef USERPROG
  hash_init (&t->files, file_elem_hash, file_elem_less, NULL);
  hash_init (&t->children, child_elem_hash, child_elem_less, NULL);
  t->as_child = malloc(sizeof (struct child_elem));

  if (t->as_child == NULL)
    return TID_ERROR;
  
  sema_init(&t->as_child->sema, 0);
  t->as_child->dead = false;
  t->as_child->waited = false;
  t->as_child->tid = t->tid;
  t->as_child->parent = thread_current ();

  hash_insert(&thread_current ()->children, &t->as_child->hash_elem);
#endif

  intr_set_level (old_level);

  /* Add to run queue. */
  thread_unblock (t);
  check_prio (t->priority);

  return tid;
}

struct child_elem *
child_lookup (const int tid)
{
  struct thread *t = thread_current();
  struct child_elem temp;
  struct hash_elem *e;
  temp.tid = tid;
  e = hash_find (&t->children, &temp.hash_elem);
  return e != NULL ? hash_entry (e, struct child_elem, hash_elem) : NULL;
}

static unsigned
child_elem_hash (const struct hash_elem *c_, void *aux UNUSED)
{
  const struct child_elem *c = hash_entry (c_, struct child_elem, hash_elem);
  return hash_int(c->tid);
}

static bool
child_elem_less (const struct hash_elem *a_, const struct hash_elem *b_,
void *aux UNUSED)
{
  const struct child_elem *a = hash_entry (a_, struct child_elem, hash_elem);
  const struct child_elem *b = hash_entry (b_, struct child_elem, hash_elem);
  return a->tid < b->tid;
}

/* If new priority is higher than current thread, yield CPU */
void
check_prio (int prio)
{
  if (thread_current () != idle_thread && thread_get_priority () < prio) {
    if (intr_context ()) {
      intr_yield_on_return ();
    } else {
      thread_yield ();
    }
  }
}

/* Puts the current thread to sleep.  It will not be scheduled
   again until awoken by thread_unblock().

   This function must be called with interrupts turned off.  It
   is usually a better idea to use one of the synchronization
   primitives in synch.h. */
void
thread_block (void) 
{
  ASSERT (!intr_context ());
  ASSERT (intr_get_level () == INTR_OFF);

  thread_current ()->status = THREAD_BLOCKED;
  schedule ();
}

/* Transitions a blocked thread T to the ready-to-run state.
   This is an error if T is not blocked.  (Use thread_yield() to
   make the running thread ready.)

   This function does not preempt the running thread.  This can
   be important: if the caller had disabled interrupts itself,
   it may expect that it can atomically unblock a thread and
   update other data. */
void
thread_unblock (struct thread *t) 
{
  enum intr_level old_level;

  ASSERT (is_thread (t));

  old_level = intr_disable ();
  ASSERT (t->status == THREAD_BLOCKED);
  t->status = THREAD_READY;
  if (thread_mlfqs) {
    mlfq_insert (t);
    thread_update_priority (t, NULL);
  } else {
    list_push_back (&ready_list, &t->elem);
  }

  intr_set_level (old_level);
}

/* Helper function to order ready_list by priority */
bool
prio_compare (const struct list_elem *a,
             const struct list_elem *b,
             void *aux UNUSED) {
  struct thread *a1 = list_entry (a, struct thread, elem);
  struct thread *b1 = list_entry (b, struct thread, elem);
  return a1->effective_priority >= b1->effective_priority;
}

/* Helper function to find max priority in a list */
bool
compare_max_prio (const struct list_elem *a,
             const struct list_elem *b,
             void *aux UNUSED) {
  return !prio_compare (a, b, NULL);
}

/* Returns the name of the running thread. */
const char *
thread_name (void) 
{
  return thread_current ()->name;
}

/* Returns the running thread.
   This is running_thread() plus a couple of sanity checks.
   See the big comment at the top of thread.h for details. */
struct thread *
thread_current (void) 
{
  struct thread *t = running_thread ();
  
  /* Make sure T is really a thread.
     If either of these assertions fire, then your thread may
     have overflowed its stack.  Each thread has less than 4 kB
     of stack, so a few big automatic arrays or moderate
     recursion can cause stack overflow. */
  ASSERT (is_thread (t));
  ASSERT (t->status == THREAD_RUNNING);

  return t;
}

/* Returns the running thread's tid. */
tid_t
thread_tid (void) 
{
  return thread_current ()->tid;
}

/* Deschedules the current thread and destroys it.  Never
   returns to the caller. */
void
thread_exit (void) 
{
  ASSERT (!intr_context ());

#ifdef USERPROG
  process_exit ();

  struct thread *cur = thread_current ();

  intr_disable ();

  /* If parent is dead free the child_elem, otherwise sema_up to tell parent it has died */
  if (cur->as_child->dead) {
    free(cur->as_child);
  } else {
    cur->as_child->dead = true;
    sema_up(&cur->as_child->sema);
  }

  /* free hash tables and all their elements */
  hash_destroy(&cur->children, free_children);
  hash_destroy(&cur->files, &free_file);
#endif

  /* Remove thread from all threads list, set our status to dying,
     and schedule another process.  That process will destroy us
     when it calls thread_schedule_tail(). */
  list_remove (&thread_current ()->allelem);

  thread_current ()->status = THREAD_DYING;
  schedule ();
  NOT_REACHED ();
}

/* Frees a file given its hash_elem. */
static void
free_file (struct hash_elem *e, void *aux UNUSED)
{
  struct file_elem *f = hash_entry (e, struct file_elem, hash_elem);
  file_close(f->faddr);
  free(f);
}

/* Frees child_elem if needed, or sets dead to true. */
static void
free_children (struct hash_elem *e, void *aux UNUSED)
{
  struct child_elem *a = hash_entry (e, struct child_elem, hash_elem);
  if (a->dead) {
    free(a);
  } else {
    a->dead = true;
  }
}

/* Yields the CPU.  The current thread is not put to sleep and
   may be scheduled again immediately at the scheduler's whim. */
void
thread_yield (void) 
{
  struct thread *cur = thread_current ();
  enum intr_level old_level;
  
  ASSERT (!intr_context ());

  old_level = intr_disable ();

  if (cur != idle_thread) {
    if (thread_mlfqs)
      mlfq_insert (cur);
    else
      list_push_back (&ready_list, &cur->elem);
  }

  cur->status = THREAD_READY;
  schedule ();
  intr_set_level (old_level);
}

/* Invoke function 'func' on all threads, passing along 'aux'.
   This function must be called with interrupts off. */
void
thread_foreach (thread_action_func *func, void *aux)
{
  struct list_elem *e;

  ASSERT (intr_get_level () == INTR_OFF);

  for (e = list_begin (&all_list); e != list_end (&all_list);
       e = list_next (e))
    {
      struct thread *t = list_entry (e, struct thread, allelem);
      func (t, aux);
    }
}

/* Sets the current thread's priority to 'new_priority'. */
void
thread_set_priority (int new_priority) 
{
  ASSERT (!thread_mlfqs);

  enum intr_level old_level = intr_disable ();
  /* Extract highest priority element form the ready list */
  thread_current ()->priority = new_priority;
  recalculate_priority (thread_current ());

  if (!list_empty (&ready_list)) {
      check_prio (list_entry (list_max (&ready_list, compare_max_prio, NULL),
                        struct thread, elem)->priority);
  }
  intr_set_level (old_level);
}

/* Returns the current thread's priority. */
int
thread_get_priority (void) 
{
  return thread_current ()->effective_priority;
}

static void
recalculate_priority (struct thread *t)
{
  int prio;
  /* Compares maximum donated priority to base priority */
  enum intr_level old_level = intr_disable ();
  if (t->donated_prios[0] != NULL && t->donated_prios[0]->priority > t->priority) {
      prio = t->donated_prios[0]->priority;
  } else {
      prio = t->priority;
  }
  t->effective_priority = prio;
  intr_set_level (old_level);
}

/* Donates given priority to the lock holder and every other lock in the chain */
void
donate_priority (struct lock *lock, struct donated_prio *p)
{
  ASSERT(intr_get_level() == INTR_OFF);

  struct thread* t = lock->holder;

  /* Donate this threads priority to target thread */
  array_insert_ordered_prio (t->donated_prios, p);
  array_insert_ordered_prio (lock->donated_prios, p);

  recalculate_priority (t);

  /* Donate to existing donee */
  if (t->donated_lock != NULL){
    donate_priority (t->donated_lock, p);
  }
}

void
revoke_priority (struct lock *lock)
{
  struct thread *cur = thread_current ();

  for (int i = 0; i < MAX_DONATIONS && lock->donated_prios[i] != NULL; i++) {
    array_remove_prio (cur->donated_prios, lock->donated_prios[i]);
  }

  recalculate_priority (cur);
}

/* Sets the current thread's nice value to NICE. */
void
thread_set_nice (int nice) 
{
  ASSERT (thread_mlfqs);

  enum intr_level old_level = intr_disable ();

  thread_current ()->nice = nice;
  thread_update_priority (thread_current (), NULL);

  /* check if updating should cause thread to yield */
  if (!mlfq_is_empty ()) {
    check_prio (list_entry (
      list_front (&queue_array[mlfq_highest_priority ()]),
      struct thread, elem)->priority);
  }

  intr_set_level (old_level);
}

/* Returns the current thread's nice value. */
int
thread_get_nice (void) 
{
  return thread_current ()->nice;
}

/* Returns 100 times the system load average. */
int
thread_get_load_avg (void) 
{
  enum intr_level old_level = intr_disable (); 
  int load = FIXED_TO_INT (FIXED_MUL_INT (load_avg, 100));
  intr_set_level (old_level);
  return load;
}

/* Returns 100 times the current thread's recent_cpu value. */
int
thread_get_recent_cpu (void) 
{
  return FIXED_TO_INT (FIXED_MUL_INT (thread_current ()->recent_cpu, 100));
}

/* Idle thread.  Executes when no other thread is ready to run.

   The idle thread is initially put on the ready list by
   thread_start().  It will be scheduled once initially, at which
   point it initializes idle_thread, "up"s the semaphore passed
   to it to enable thread_start() to continue, and immediately
   blocks.  After that, the idle thread never appears in the
   ready list.  It is returned by next_thread_to_run() as a
   special case when the ready list is empty. */
static void
idle (void *idle_started_ UNUSED) 
{
  struct semaphore *idle_started = idle_started_;
  idle_thread = thread_current ();
  sema_up (idle_started);

  for (;;) 
    {
      /* Let someone else run. */
      intr_disable ();
      thread_block ();

      /* Re-enable interrupts and wait for the next one.

         The `sti' instruction disables interrupts until the
         completion of the next instruction, so these two
         instructions are executed atomically.  This atomicity is
         important; otherwise, an interrupt could be handled
         between re-enabling interrupts and waiting for the next
         one to occur, wasting as much as one clock tick worth of
         time.

         See [IA32-v2a] "HLT", [IA32-v2b] "STI", and [IA32-v3a]
         7.11.1 "HLT Instruction". */
      asm volatile ("sti; hlt" : : : "memory");
    }
}

/* Function used as the basis for a kernel thread. */
static void
kernel_thread (thread_func *function, void *aux) 
{
  ASSERT (function != NULL);

  intr_enable ();       /* The scheduler runs with interrupts off. */
  function (aux);       /* Execute the thread function. */
  thread_exit ();       /* If function() returns, kill the thread. */
}

/* Returns the running thread. */
struct thread *
running_thread (void) 
{
  uint32_t *esp;

  /* Copy the CPU's stack pointer into `esp', and then round that
     down to the start of a page.  Because `struct thread' is
     always at the beginning of a page and the stack pointer is
     somewhere in the middle, this locates the curent thread. */
  asm ("mov %%esp, %0" : "=g" (esp));
  return pg_round_down (esp);
}

/* Returns true if T appears to point to a valid thread. */
static bool
is_thread (struct thread *t)
{
  return t != NULL && t->magic == THREAD_MAGIC;
}

/* Does basic initialization of T as a blocked thread named
   NAME. */
static void
init_thread (struct thread *t, const char *name, int priority)
{
  enum intr_level old_level;

  ASSERT (t != NULL);
  ASSERT (PRI_MIN <= priority && priority <= PRI_MAX);
  ASSERT (name != NULL);

  memset (t, 0, sizeof *t);
  t->status = THREAD_BLOCKED;
  strlcpy (t->name, name, sizeof t->name);
  t->stack = (uint8_t *) t + PGSIZE;
  t->priority = priority;
  t->effective_priority = priority;
  t->magic = THREAD_MAGIC;

  array_init_prio (t->donated_prios);

  old_level = intr_disable ();
  list_push_back (&all_list, &t->allelem);
  intr_set_level (old_level);
}

/* Allocates a SIZE-byte frame at the top of thread T's stack and
   returns a pointer to the frame's base. */
static void *
alloc_frame (struct thread *t, size_t size) 
{
  /* Stack data is always allocated in word-size units. */
  ASSERT (is_thread (t));
  ASSERT (size % sizeof (uint32_t) == 0);

  t->stack -= size;
  return t->stack;
}

/* Chooses and returns the next thread to be scheduled.  Should
   return a thread from the run queue, unless the run queue is
   empty.  (If the running thread can continue running, then it
   will be in the run queue.)  If the run queue is empty, return
   idle_thread. */
static struct thread *
next_thread_to_run (void) 
{
  if (thread_mlfqs) {
    if (mlfq_is_empty ())
      return idle_thread;
    else
      return list_entry (list_pop_front (&queue_array[mlfq_highest_priority ()]),
                         struct thread, elem);
  } else {
    if (list_empty (&ready_list))
      return idle_thread;
    else {
      struct list_elem *next = list_max (&ready_list, compare_max_prio, NULL);
      list_remove(next);
      return list_entry (next, struct thread, elem);
    }
  }

}

/* Completes a thread switch by activating the new thread's page
   tables, and, if the previous thread is dying, destroying it.

   At this function's invocation, we just switched from thread
   PREV, the new thread is already running, and interrupts are
   still disabled.  This function is normally invoked by
   thread_schedule() as its final action before returning, but
   the first time a thread is scheduled it is called by
   switch_entry() (see switch.S).

   It's not safe to call printf() until the thread switch is
   complete.  In practice that means that printf()s should be
   added at the end of the function.

   After this function and its caller returns, the thread switch
   is complete. */
void
thread_schedule_tail (struct thread *prev)
{
  struct thread *cur = running_thread ();
  
  ASSERT (intr_get_level () == INTR_OFF);

  /* Mark us as running. */
  cur->status = THREAD_RUNNING;

  /* Start new time slice. */
  thread_ticks = 0;

#ifdef USERPROG
  /* Activate the new address space. */
  process_activate ();
#endif

  /* If the thread we switched from is dying, destroy its struct
     thread.  This must happen late so that thread_exit() doesn't
     pull out the rug under itself.  (We don't free
     initial_thread because its memory was not obtained via
     palloc ().) */
  if (prev != NULL && prev->status == THREAD_DYING && prev != initial_thread) 
    {
      ASSERT (prev != cur);
      palloc_free_page (prev);
    }
}

/* Schedules a new process.  At entry, interrupts must be off and
   the running process's state must have been changed from
   running to some other state.  This function finds another
   thread to run and switches to it.

   It's not safe to call printf() until thread_schedule_tail()
   has completed. */
static void
schedule (void) 
{
  struct thread *cur = running_thread ();
  struct thread *next = next_thread_to_run ();
  struct thread *prev = NULL;

  ASSERT (intr_get_level () == INTR_OFF);
  ASSERT (cur->status != THREAD_RUNNING);
  ASSERT (is_thread (next));

  if (cur != next)
    prev = switch_threads (cur, next);
  thread_schedule_tail (prev);
}

/* Returns a tid to use for a new thread. */
static tid_t
allocate_tid (void) 
{
  static tid_t next_tid = 1;
  tid_t tid;

  lock_acquire (&tid_lock);
  tid = next_tid++;
  lock_release (&tid_lock);

  return tid;
}

/* Initialises the mlfq by creating an empty list behind each index. */
static void
mlfq_init (void)
{
  ASSERT (thread_mlfqs);

  for (int i = 0; i < QUEUE_ARRAY_SIZE; i++)
    list_init (&queue_array[i]);
}

/* Returns the highest priority non-empty queue in the mlfq.
   Returns 0 if the mlfq is empty. */
static int
mlfq_highest_priority (void)
{
  ASSERT (thread_mlfqs);

  int index = PRI_MAX;
  while (index > 0 && list_empty (&queue_array[index]))
    index--;
  return index;
}

/* Returns true if each queue in the mlfq is empty. */
static bool
mlfq_is_empty (void)
{
  ASSERT (thread_mlfqs);
  int i;
  for (i = 0; i < QUEUE_ARRAY_SIZE; i++)
    if (!list_empty (&queue_array[i]))
      return false;
  return true;
}

static void
mlfq_insert (struct thread *t)
{
  ASSERT (thread_mlfqs);
  list_push_back (&queue_array[t->priority], &t->elem);
}

/* Offset of `stack' member within `struct thread'.
   Used by switch.S, which can't figure it out on its own. */
uint32_t thread_stack_ofs = offsetof (struct thread, stack);
