#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"

#define SPACE_DELIM " "

#define FIRST_ADDRESS_UNDER_STACK_PAGE 0xBFFFF000 /* PHYS_BASE - 4KB */

#define MAX_ARGUMENTS 30              /* Maximum number of arguments allowed to be passed. */

#define LOAD_FAILURE -1               /* Error value when a load is unsuccessful. */

#define FOUR_BYTE_ALIGN_STACK_POINTER(esp) ((void *) ((unsigned long)*(esp) & ~0x3))

/* Uses type conversion to decrement provided void* pointer by a given number of bytes */
#define DEC_ESP_BY_BYTES(esp, num) ((void *) ((unsigned long) (esp) - (num)))

#define check_for_stack_overflow(esp) ({ \
    if ((unsigned long) (esp) <= FIRST_ADDRESS_UNDER_STACK_PAGE) {      \
        return false;                    \
    }                                    \
})

#define stack_push_element(esp, elem, type) ({ \
    *esp = DEC_ESP_BY_BYTES(*(esp), sizeof(type)); \
    check_for_stack_overflow(*esp);                 \
    **(type **)(esp) = (elem); \
    })

#define stack_push_string(esp, elem) ({ \
    *esp = DEC_ESP_BY_BYTES(*(esp), strlen((elem))+1); \
    check_for_stack_overflow(*esp);                     \
    strlcpy((char*) *(esp), (elem), strlen((elem))+1); \
    })

/* Stores the arguments needed to initialise a user process stack */
struct stack_entries 
{
   char* argv[MAX_ARGUMENTS];    /* Arguments array with a predefined maximum. */
   int    argc;                  /* Actual number of arguments. */
   char*  fn_copy;               /* Used in given implementation. */
   struct exec_waiter *waiter;   /* Synchronization structure including a semaphore and return boolean. */
};

tid_t process_execute (const char *file_name, struct exec_waiter *waiter);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

#endif /* userprog/process.h */
