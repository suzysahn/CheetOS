#ifndef PAGING_H
#define PAGING_H
#include "lib.h"


/* include types to use u32int */
#include "types.h"

/* ============================== VARIABLE DECLARATIONS ======================START= */
/* video memory address location described in lib.c */
#define VM_START        0x0B8
/* number of entries in page directory and page table (4kb size) */
#define NUM_ENTRIES     1024
/* bits to align paging to */
#define ALIGN_BITS	    4096

#define PDE_IDX_SHIFT               22          /* Page directory index in virt. address        */
#define PAGE_TABLE_PRESENT_ENTRY    0x7         /* USER/READ+WRITE/PRESENT                      */
#define VIDEO                       0xB8000     /* Address of video memory page                 */

#define BITS_4KB_ALIGN              12
#define VID_MEM_PT_INDEX            (VIDEO >> BITS_4KB_ALIGN)

/* declare global page directory array */
extern uint32_t page_directory[NUM_ENTRIES];
/* declare global page table for 0MB ~ 4MB (1024 entries) */
extern uint32_t page_table[NUM_ENTRIES];
/* declare global video page table for 128MB ~ 132MB (1024 entries) */
uint32_t video_pages[NUM_ENTRIES];
/* =============================================================================END= */


/* ============================== FUNCTION DECLARATIONS ======================START= */
/* funtion to initialize pages in 0MB ~ 4MB(video memory) & 4MB ~ 8MB (kernal)  */
extern void paging_init();
extern void update_page_directory(uint32_t virt_address, uint32_t phys_address, uint16_t flags);
extern void remap_video(uint32_t virt_address);
extern void flush_tlb(void);
/* =============================================================================END= */

#endif

