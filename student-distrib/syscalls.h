#ifndef SYSCALLS_H_
#define SYSCALLS_H_

#include "types.h"
#include "lib.h"
#include "rtc.h"
#include "filesys.h"
#include "x86_desc.h"

#define OPEN 0
#define READ 1
#define WRITE 2
#define CLOSE 3
#define NUM_MAX_PROCESSES 6
#define NUM_MAX_OPEN_FILES 8
#define PROG_NOT_ACTIVE 0
#define PROG_ACTIVE 1
#define BASE_PROCESS_POSITION 0x800000
#define PROCESS_OFFSET 0x2000
#define C_128MB 0x8000000
#define C_8MB 	0x800000
#define C_4MB 	0x400000
#define C_4KB 	0x1000
#define C_4B 4

#define PAGE_BOTTOM     0x083FFFFC
#define PAGE_TOP		0x08048000
#define PAGE_BOTTOM     0x083FFFFC
#define PAGE_VIDMEM 	0x08400000
#define PAGE_VIDMAP		33

#define TERMINAL_BUFFER_SIZE	128

/* global to keep track of number of processes.
 * needed to decide which page directory to switch
 * to when setting up paging
 */
extern uint32_t num_processes;
extern uint32_t curr_process;

extern int32_t open(const uint8_t* filename);
extern int32_t close(int32_t fd);
extern int32_t read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t halt(uint8_t status);
extern int32_t execute(const uint8_t* command);
extern void return_to_execute(uint8_t* exec_ret_addr, uint32_t parent_ebp, uint32_t parent_esp, uint8_t status);
extern void context_switch(uint32_t entry_point, uint32_t position);
extern int32_t getargs(uint8_t* buf, int32_t nbytes);
extern int32_t vidmap(uint8_t** screen_start);
extern int32_t set_handler(int32_t signum, void* handler);
extern int32_t sigreturn(void);

extern pcb_t * getCurrentProcessPCB();
extern pcb_t * getProcessPCB(uint32_t pid);
extern uint32_t get_kernel_stack_bottom(uint32_t pid);

// typedefs for function pointers for close, read, write
typedef int32_t (*CLOSEFUNC)(int32_t);
typedef int32_t (*READFUNC)(int32_t, void*, int32_t);
typedef int32_t (*WRITEFUNC)(int32_t, const void*, int32_t);

#endif


