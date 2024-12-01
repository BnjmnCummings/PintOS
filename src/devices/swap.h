#ifndef DEVICES_SWAP_H
#define DEVICES_SWAP_H 1

#include <stddef.h>

void swap_init (void);
size_t swap_out (const void *vaddr);
void swap_in (void *vaddr, size_t slot);
void swap_drop (size_t slot);

#endif /* devices/swap.h */
