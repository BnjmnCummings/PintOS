#include "devices/swap.h"
#include "devices/block.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include <bitmap.h>
#include <debug.h>
#include <stdio.h>

/* Pointer to the swap device */
static struct block *swap_device;

/* Pointer to a bitmap to track used swap pages */
static struct bitmap *swap_bitmap;

/* Lock that protects swap_bitmap from unsynchronised access */
static struct lock swap_lock;

/* Number of sectors needed to store a page */
#define PAGE_SECTORS (PGSIZE / BLOCK_SECTOR_SIZE)

/* Sets up the swap space */
void
swap_init (void) 
{
  // locate the swap block allocated to the kernel
  swap_device = block_get_role (BLOCK_SWAP);
  if (swap_device == NULL) {
      printf ("no swap device--swap disabled\n");
      swap_bitmap = bitmap_create (0);
  } else {
    // create a bitmap with 1 slot per page-sized chunk of memory on the swap block
    swap_bitmap = bitmap_create (block_size (swap_device) / PAGE_SECTORS);
  }
  
  if (swap_bitmap == NULL){
    PANIC ("couldn't create swap bitmap");
  }
  lock_init (&swap_lock);
}

/* Swaps page at VADDR out of memory, returns the swap-slot used */
size_t
swap_out (const void *vaddr) 
{
  // find available swap-slot for the page to be swapped out
  lock_acquire (&swap_lock);
  size_t slot = bitmap_scan_and_flip (swap_bitmap, 0, 1, false);
  lock_release (&swap_lock);
  if (slot == BITMAP_ERROR) 
    return BITMAP_ERROR; 

  // calculate block sector from swap-slot number
  size_t sector = slot * PAGE_SECTORS;
  
  // loop over each sector of the page, copying it from memory into swap
  for (size_t i = 0; i < PAGE_SECTORS; i++)
    block_write (swap_device, sector + i, vaddr + i * BLOCK_SECTOR_SIZE);

  return slot;
}

/* Swaps page on disk in swap-slot SLOT into memory at VADDR */
void
swap_in (void *vaddr, size_t slot) 
{
  // calculate block sector from swap-slot number
  size_t sector = slot * PAGE_SECTORS;

  // loop over each sector of the page, copying it from swap into memory
  for (size_t i = 0; i < PAGE_SECTORS; i++)
    block_read (swap_device, sector + i, vaddr + i * BLOCK_SECTOR_SIZE);
  
  // clear the swap-slot previously used by this page
  swap_drop (slot);
}

/* Clears the swap-slot SLOT so that it can be used for another page */
void
swap_drop (size_t slot)
{
  bitmap_reset (swap_bitmap, slot);
}
