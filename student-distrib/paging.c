#include "paging.h"


/* arr for page directory, aligned to 4KB memory */
uint32_t page_directory[NUM_ENTRIES] __attribute__((aligned(ALIGN_BITS)));

/* arr for page table entries in first 4MB, aligned to 4KB memory */
uint32_t page_table[NUM_ENTRIES] __attribute__((aligned(ALIGN_BITS)));

/* array for video memroy pages. 128-132MB page tabley, align each to 4kB */
uint32_t video_pages[NUM_ENTRIES] __attribute__((aligned(4096)));

/* paging_init
 *   DESCRIPTION: Called by kernel.c to initialize paging. 
 * 	 			  Sets up 4kb pages for first 4MB including video memory, and 
 * 				  Sets up one page of 4MB for kernel, location defined in makefile. 
 *   INPUT: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECT: Arrays for page_directory and page_table is declared and initialized.
 * 				  Bits for each pages initialized to 0, sttribute bits marked present correspondingly.
 * 				  Change the control registers bits to enable paging.
 */
void paging_init()
{
 	// 	printf(" Paging Initialization! \n");
	unsigned int i;
  
  	/* 0x02 = 10 mark each page tables in directory as read/write & not present
  	 * NOTE: only kernel-mode can access them (supervisor) from os dev */
	for(i = 0; i < NUM_ENTRIES; i++) {
		page_directory[i] = 0x00000002;
	}

	/* initialize page table for 0MB ~ 4MB containing video memory */
	for (i = 0; i < NUM_ENTRIES; i++) {
		if (i >= VM_START && i <= VM_START + 3) {			/* video memory is 4 bytes */
			page_table[i] = (i * 0x1000) | 3; 				/* 0x03 = 11 read/write, mark vid mem as present */
		}													/* 12 bits skipped: 0x1000 */
		else{
			page_table[i] = (i * 0x1000) | 2; 				/* 0x02 = 10 read/write, mark non vid mem as not present */
		}
	}

  	/* set first page table address and also mar read/write and as present on first entry of page_directory */
	page_directory[0] = ((unsigned int)page_table) | 3;   	/* 0x03 = 11 read/write, mark vid mem as present */

	/* set second entry in page directories to point to kernel 4MB page
	 * 0x3(11): mark supervisor: access kernel, read/write, present
	 * 0x80(1000 0000): mark as 4MB page size for kernel entry
	 * 0x400000: address to 4MB the start of kernel page defined by makefile
	 */
	page_directory[1] = 0x00400083;

	/* set control registers to initialize paging */
    __asm__ (
        /* load page_directory address to cr3 */
        "movl $page_directory, %%ebx \n"
	   	"movl %%ebx, %%cr3        \n"
      
        /* enable Page Size Entension(bit4) in cr4 for 4MB page */
        "movl %%cr4, %%ebx        \n"
        "orl $0x00000010, %%ebx   \n"
        "movl %%ebx, %%cr4        \n"

        /* enable paging(bit31) and protected mode enable(bit0) in cr0 */
        "movl %%cr0, %%ebx         \n"
        "orl $0x80000001, %%ebx    \n"
        "movl %%ebx, %%cr0"
      
        : 
        : "a"(page_directory)
        : "%ebx", "cc"
    ); 
}

/*
 * Update page directory mapping from virt_address to point
 * to phys_address.
 */
void update_page_directory(uint32_t virt_address, uint32_t phys_address, uint16_t flags) {
    page_directory[virt_address >> PDE_IDX_SHIFT] = phys_address | flags;
    flush_tlb();
}

/*
 * Update userspace page table to map virt_address to point to system video memory
 */
void remap_video(uint32_t virt_address) {
    // add entry to PD pointing to video page table
    page_directory[virt_address >> PDE_IDX_SHIFT] = (uint32_t)page_table | PAGE_TABLE_PRESENT_ENTRY;
    // point first entry to physical video memory
    page_table[VID_MEM_PT_INDEX] = VIDEO | PAGE_TABLE_PRESENT_ENTRY;
    flush_tlb();
}

/*
 * Flush the TLB by writing to CR3.  We don't actually
 * want to change the value, so write CR3 back to CR3.
 */
void flush_tlb(void){
    asm volatile("                   \n\
        movl %%cr3, %%eax            \n\
        movl %%eax, %%cr3            \n\
        "
        : /* no outputs */
        : /* no inputs  */
        : "eax" /* eax clobbered bc it's clobberin' time */
    );
}


