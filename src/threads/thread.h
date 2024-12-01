#ifndef THREADS_THREAD_H
#define THREADS_THREAD_H

#include <debug.h>
#include <list.h>
#include <stdint.h>
#include <hash.h>
#include "threads/synch.h"

/* States in a thread's life cycle. */
enum thread_status
  {
    THREAD_RUNNING,     /* Running thread. */
    THREAD_READY,       /* Not running but ready to run. */
    THREAD_BLOCKED,     /* Waiting for an event to trigger. */
    THREAD_DYING        /* About to be destroyed. */
  };

/* Thread identifier type.
   You can redefine this to whatever type you like. */
typedef int tid_t;
#define TID_ERROR ((tid_t) -1)          /* Error value for tid_t. */

/* Thread priorities. */
#define PRI_MIN 0                       /* Lowest priority. */
#define PRI_DEFAULT 31                  /* Default priority. */
#define PRI_MAX 63                      /* Highest priority. */

/* Niceness, recent_cpu and load_avg all start at 0 */
#define NICE_DEFAULT 0
#define RECENT_CPU_DEFAULT 0
#define INITIAL_LOAD_AVG 0

/* Priority is updated for every thread once every 4 ticks */
#define PRI_UPDATE_FREQUENCY 4

/* Size of the array containing each ready queue */
#define QUEUE_ARRAY_SIZE 64

/* A kernel thread or user process.

   Each thread structure is stored in its own 4 kB page.  The
   thread structure itself sits at the very bottom of the page
   (at offset 0).  The rest of the page is reserved for the
   thread's kernel stack, which grows downward from the top of
   the page (at offset 4 kB).  Here's an illustration:

        4 kB +---------------------------------+
             |          kernel stack           |
             |                |                |
             |                |                |
             |                V                |
             |         grows downward          |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             +---------------------------------+
             |              magic              |
             |                :                |
             |                :                |
             |               name              |
             |              status             |
        0 kB +---------------------------------+

   The upshot of this is twofold:

      1. First, `struct thread' must not be allowed to grow too
         big.  If it does, then there will not be enough room for
         the kernel stack.  Our base `struct thread' is only a
         few bytes in size.  It probably should stay well under 1
         kB.

      2. Second, kernel stacks must not be allowed to grow too
         large.  If a stack overflows, it will corrupt the thread
         state.  Thus, kernel functions should not allocate large
         structures or arrays as non-static local variables.  Use
         dynamic allocation with malloc() or palloc_get_page()
         instead.

   The first symptom of either of these problems will probably be
   an assertion failure in thread_current(), which checks that
   the `magic' member of the running thread's `struct thread' is
   set to THREAD_MAGIC.  Stack overflow will normally change this
   value, triggering the assertion. */
/* The `elem' member has a dual purpose.  It can be an element in
   the run queue (thread.c), or it can be an element in a
   semaphore wait list (synch.c).  It can be used these two ways
   only because they are mutually exclusive: only a thread in the
   ready state is on the run queue, whereas only a thread in the
   blocked state is on a semaphore wait list. */
struct thread
  {
    /* Owned by thread.c. */
    tid_t tid;                          /* Thread identifier. */
    enum thread_status status;          /* Thread state. */
    char name[16];                      /* Name (for debugging purposes). */
    uint8_t *stack;                     /* Saved stack pointer. */
    int effective_priority;             /* Max of base priority and donations */
    int priority;                       /* Priority. */
    struct donated_prio* donated_prios[2*MAX_DONATIONS]; /* Queue of donated priorities */
    struct lock* donated_lock;          /* Records the lock held by the donee */
    int nice;                           /* Niceness. */
    int32_t recent_cpu;                 /* Thread recent CPU usage. */
    struct list_elem allelem;           /* List element for all threads list. */

    /* Shared between thread.c and synch.c. */
    struct list_elem elem;              /* List element. */

#ifdef USERPROG
    /* Owned by userprog/process.c. */
    uint32_t *pagedir;                  /* Page directory. */
    struct hash files;                  /* Hash table of accessible files. */
    struct child_elem *as_child;        /* Child hash element for this thread's parent. */
    int exit_status;                    /* Return exit status. */
    struct hash children;               /* Hash table of child threads. */
    struct file *open_file;             /* Currently open file. */
#endif

    /* Owned by thread.c. */
    unsigned magic;                     /* Detects stack overflow. */
  };

/* Element representing child of a parent pushed to children table. */
struct child_elem
{
   tid_t tid;                       /* Child's thread identifier. */
   struct semaphore sema;           /* Semaphore for parent-child synchronization. */
   int exit_status;                 /* Return exit status. */
   bool waited;                     /* True if child has called process_wait(). */
   bool dead;                       /* True if child thread has been killed. */
   struct thread *parent;           /* Pointer to the thread's parent. */
   struct hash_elem hash_elem;      /* Hash table element. */
};


/* If false (default), use round-robin scheduler.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "mlfqs". */
extern bool thread_mlfqs;

void thread_init (void);
void thread_start (void);
size_t threads_ready (void);

void thread_tick (void);
void thread_print_stats (void);

typedef void thread_func (void *aux);
tid_t thread_create (const char *name, int priority, thread_func *, void *);

void thread_block (void);
void thread_unblock (struct thread *);

bool prio_compare (const struct list_elem *a,
                  const struct list_elem *b,
                  void *aux UNUSED);
bool compare_max_prio (const struct list_elem *a,
                  const struct list_elem *b,
                  void *aux UNUSED);
void check_prio (int prio);

struct thread *thread_current (void);
tid_t thread_tid (void);
const char *thread_name (void);

void thread_exit (void) NO_RETURN;
void thread_yield (void);

/* Performs some operation on thread t, given auxiliary data AUX. */
typedef void thread_action_func (struct thread *t, void *aux);
void thread_foreach (thread_action_func *, void *);

int thread_get_priority (void);
void thread_set_priority (int);

void donate_priority (struct lock *lock, struct donated_prio *p);
void revoke_priority (struct lock *lock);

int thread_get_nice (void);
void thread_set_nice (int);

int thread_get_recent_cpu (void);
int thread_get_load_avg (void);

struct child_elem *child_lookup (const int);

#endif /* threads/thread.h */
