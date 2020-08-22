#include "lib.h"
#include "syscalls.h"
#include "filesys.h"
#include "paging.h"
#include "terminal.h"

/* declare variables */
uint8_t  pid_array[NUM_MAX_PROCESSES] = {0, 0, 0, 0, 0, 0};  /* array that folds the pids */
uint32_t curr_process = 0;                                   /* keeps track of which current process it is running */

/* open
 * DESCRIPTION: system call for open, attempts to open a file with its filename
 * INPUTS: filename
 * OUTPUTS: populates file array
 * RETURN VALUE: 0 on success, -1 on fail
 * SIDE EFFECTS: none
 */
int32_t open(const uint8_t* filename) {
    // DEBUG PRINT
    //printf("## open(%s)\n", filename);
    // Check file exist or not
    dentry_t dentry;
    if (read_dentry_by_name(filename, &dentry) == -1)
        return -1;

    int ret;
    switch (dentry.filetype) {
        case FILE_TYPE_DIR:
            return directory_open(filename);

        case FILE_TYPE_RTC:
            // Create RTC entry in file_array
            if (file_open(filename) == -1) return -1;

        case FILE_TYPE_FILE:
            ret = file_open(filename);
            //printf("## open(%s) - %d\n", filename, ret);
            return ret;
    }

    // Otherwise, error
    return -1;
}

/* close
 * DESCRIPTION: system call for close, attempt to close an opened file
 * INPUTS: fd index
 * OUTPUTS: free the fd
 * RETURN VALUE: 0 on success, -1 on failure
 * SIDE EFFECTS: changes file array
 */
int32_t close(int32_t fd) {
    // DEBUG PRINT
    //printf("## close() - %d\n", fd);
    // Get file_array pointer
    file_t * file_array = getCurrentProcessPCB()->file_array;

    // Don't close stdin or stdout, or anything out of bounds
    if (7 < fd || fd < 2) return -1;

    // If file isn't open, don't bother closing it
    if (file_array[fd].flags == FILE_AVAIL) return -1;

    // Call low-level close function
    int32_t *fp = (int32_t *) file_array[fd].file_ops_table_ptr;
    CLOSEFUNC func = (CLOSEFUNC)fp[CLOSE];
    return (*func)(fd);
}

/* read
 * DESCRIPTION: system call for read, attempt to read to only open file
 * INPUTS: fd index, buffer to write to, how many bytes to read
 * OUTPUTS: calls read function
 * RETURN VALUE: nbytes on success, -1 on failure
 * SIDE EFFECTS: none
 */
int32_t read(int32_t fd, void* buf, int32_t nbytes) {
    // DEBUG PRINT
    //printf("## read() - %d, %x, %d\n", fd, buf, nbytes);
    // Get file_array pointer
    file_t * file_array = getCurrentProcessPCB()->file_array;

    // Don't close stdin or stdout, or anything out of bounds
    if (7 < fd || fd < 0) return -1;

    // If file isn't open, don't bother closing it
    if (file_array[fd].flags == FILE_AVAIL) return -1;

    // Call low-level read function
    int32_t * fp = (int32_t *) file_array[fd].file_ops_table_ptr;
    READFUNC func = (READFUNC) fp[READ];
    return (*func)(fd, buf, nbytes);
}

/* write
 * DESCRIPTION: system call for write attempt to write to only open files
 * INPUTS: fd index, buffer to write to, how many bytes to write
 * OUTPUTS: calls write function 
 * RETURN VALUE: nbytes on success, -1 on failure
 * SIDE EFFECTS: none
 */
int32_t write(int32_t fd, const void* buf, int32_t nbytes) {
    // DEBUG PRINT
    //printf("## write() - %d, %s, %d\n", fd, buf, nbytes);
    // Get file_array pointer
    file_t * file_array = getCurrentProcessPCB()->file_array;

    // Don't close stdin or stdout, or anything out of bounds
    if (7 < fd || fd < 0) return -1;

    // If file isn't open, don't bother closing it
    if (file_array[fd].flags == FILE_AVAIL) return -1;

    // Call low-level read function
    int32_t * fp = (int32_t *) file_array[fd].file_ops_table_ptr;
    WRITEFUNC func = (WRITEFUNC) fp[WRITE];
    return (*func)(fd, buf, nbytes);
}    


/* halt
 * DESCRIPTION: system call for halt, attempt to halt programs that have 
                een opened, and is not the shell
 * INPUTS: status
 * OUTPUTS: system halted
 * RETURN VALUE: return 0 for success
 * SIDE EFFECTS: system stop, return to parent
 */
#define USER_PDE_4MB_BASE     0x87    /* offset to calculate the offset to parent */
int32_t halt(uint8_t status) {
    // DEBUG PRINT
//    printf("## halt() - %d\n", status);
    /* declare variables */
    uint32_t pcb_offset = 2;          /* offset to correctly manage process locations */
    pcb_t * current_pcb = ((pcb_t *)(BASE_PROCESS_POSITION - (curr_process + pcb_offset) * PROCESS_OFFSET)); /* holds current pcb */
    
    /* decalre parent's storeing variables */
    uint32_t parent_process = current_pcb->parent_num;      /* keeps track which process is the parent */
    uint32_t parent_ebp = current_pcb->parent_ebp;          /* holds parents's ebp for return stack to parent's state */
    uint32_t parent_esp = current_pcb->parent_esp;          /* holds parents's esp for return stack to parent's state */
    uint8_t* exec_ret_addr = current_pcb->exec_ret_addr;    /* execution return address */

    /* Restore to parent by calculating offset: data/paging */
    /*                      128MB       8MB                           4MB         USER PDE 4MB BASE VALUE  */
    //update_page_directory(0x08000000, 0x00800000 + parent_process * 0x00400000, 0x87);
    update_page_directory(C_128MB, C_8MB + parent_process * C_4MB, USER_PDE_4MB_BASE);

    /* Restore ESP to parent */
    tss.esp0 = BASE_PROCESS_POSITION - (parent_process + 1) * PROCESS_OFFSET - C_4B;
    
    /* mark current process as no longer active */
    pid_array[curr_process] = PROG_NOT_ACTIVE;
    curr_process = parent_process;

    /* Jump to execute return */
    return_to_execute(exec_ret_addr, parent_ebp, parent_esp, status);

    /* returns, but never back to the caller */
    return 0;
}


/* get_next_process_number
 * DESCRIPTION: returns the number of active process by looping
 * INPUTS: void
 * OUTPUTS: will get the number of active processes to use
 * RETURN VALUE: return index to store next process number for success, 
                 return -1 for failure
 * SIDE EFFECTS: none, just loops through the array
 */
int32_t get_next_process_number() {
    /* declare variables */
    int32_t i;

    /* loops through to find the next unused pid index to store new pid */
    for (i = 0; i < NUM_MAX_PROCESSES; i++) {
        if (pid_array[i] == PROG_NOT_ACTIVE) {
            pid_array[i] = PROG_ACTIVE;
            return i;
        }
    }

    /* can't find one */
    return -1;
}

/* execute
 * DESCRIPTION: system call for execute, execute the process
 * INPUTS: command as character array
 * OUTPUTS: sets new page directory and will set program in memory
 * RETURN VALUE: return 0 for success, else return -1 for fail
 * SIDE EFFECTS: modifies the PCB
 */
int32_t execute(const uint8_t* command) {
    // DEBUG PRINT
    //printf("## execute() - %s\n", command);
    /* declare variables */
    uint8_t filename[FILENAME_LEN];     /* holds the filename */
    uint8_t buffer[128];                /* buffer of size 128 */
    dentry_t dentry;                    /* program's file directory entry */
    uint8_t* program_image = (uint8_t *) 0x8048000; /* program's position */
    uint32_t next_process;              /* holds the next process id */
    uint32_t entry_point;               /* holds the entry location */
    uint32_t ret;                       /* return value */

    /* Returns first token and check string length */
    char* token = strtok((char*)command, " ");
    if (token == NULL || strlen(token) >= (FILENAME_LEN - 1))
        return -1;  // Filename error

    /* Get filename from command */
    strcpy((char*)filename, token);

	/* Read the directory entry with the specified filename */
	if (read_dentry_by_name(filename, &dentry) != 0)
		return -1; // Couldn't find matching directory entry
	if (dentry.filetype != FILE_TYPE_FILE)
		return -1; // Wrong file type

    /* Check if the file is a valid executable file, by checking */
	// the first 4 bytes to see if it matches the magic number. 
    // "7F 45 4C 46" is head. ".ELF"
	if (read_data(dentry.inode_num, 0, buffer, 4) != 4)
		return -1; // Read_data error
    if (buffer[0] != 0x7F || buffer[1] != 0x45 || buffer[2] != 0x4C || buffer[3] != 0x46)
		return -1; // File is not an executable

    // Cleanup arguments and copying into pcb->arg
    buffer[0] = '\0';
    uint8_t *ptr = buffer;

    token = strtok(NULL, " ");
    while (token != NULL) {
        if (ptr != buffer)     // not a first argument, add ' '
            memcpy(ptr++, " ", 2);
        ret = strlen(token);
        memcpy(ptr, token, ret + 1);
        ptr = ptr + ret;
        token = strtok(NULL, " ");
    }

    // Modifying page memory
    next_process =  get_next_process_number();
    // if reached max process 
    if (next_process == -1)  {
        printf("You have reached the maximum number of processes! \n");
        return 256;
    }
    //                    128MB       8MB                           4MB         USER PDE 4MB BASE VALUE 
    update_page_directory(0x08000000, 0x00800000 + next_process * 0x00400000, 0x87);

    // Get the entire file size and read into memory
    uint32_t size = flength(dentry.inode_num);
    // Read file into image memory
	if (read_data(dentry.inode_num, 0, program_image, size) != size) {
        update_page_directory(0x08000000, 0x00800000 + curr_process * 0x00400000, 0x87);
		return -1; // Read_data error
    }

    // Save esp in TSS
    tss.esp0 = BASE_PROCESS_POSITION - (next_process + 1) * PROCESS_OFFSET - C_4B;

    // Get process control block
    pcb_t *pcb = (pcb_t *)(BASE_PROCESS_POSITION - (next_process + 2) * PROCESS_OFFSET);

    // this should set files for stdin and stdout in file array
    init_file_array(pcb->file_array);

    // Give PCB parent process number
    pcb->parent_num = curr_process;

    // Save esp and ebp in PCB
    asm("\t movl %%esp,%0" : "=r" (pcb->parent_esp));
    asm("\t movl %%ebp,%0" : "=r" (pcb->parent_ebp));

    // Give PCB return address that execute will return to
    pcb->exec_ret_addr = &&end_of_execute;
    pcb->terminal_index = getCurrentProcessPCB()->terminal_index;

    // Fill pcb arguments
    strcpy((int8_t *)pcb->arg, (const int8_t*)buffer);
    pcb->num_char_in_arg = (uint8_t)strlen((const int8_t *)buffer);
    // DEBUG PRINT
    //printf("## file = %s, arg(%d) = \"%s\"\n", filename, pcb->num_char_in_arg, pcb->arg);

    // set current process as the process being switched into
    curr_process = next_process;
    pid_array[next_process] = PROG_ACTIVE;

    // Perform the context switch with entry point (bytes 24 - 27)
    entry_point = *((uint32_t*)(program_image + 24));
    context_switch(entry_point, C_128MB + C_4MB - C_4B);

end_of_execute:
    // move eax into return value
    asm("\t movl %%eax,%0" : "=r" (ret));

    // Return value passed by halt
    return ret;
}

/* getargs
 * DESCRIPTION: system call for getargs
 * INPUTS:
 * OUTPUTS:
 * RETURN VALUE:
 * SIDE EFFECTS:
 */
int32_t getargs(uint8_t* buf, int32_t nbytes) {
    // DEBUG PRINT
    //printf("## getargs(%x, %d)\n", buf, nbytes);

    // Check if address of buf is within the address range covered
    if (buf == NULL) return -1;

    uint32_t esp;
    asm volatile ("\t movl %%esp,%0" : "=r"(esp));
    pcb_t *pcb = (pcb_t *)(esp & ~(0x1FFF));

    // Check for the return condition
    if (pcb->arg[0] == '\0') return -1;

    // Check buffer size
    if (nbytes < pcb->num_char_in_arg) return -1;

    // Adjust return size and copy it
    strcpy((int8_t *)buf, (const int8_t *)pcb->arg);

    // DEBUG PRINT
   // printf("## getargs() = %s\n", buf);

    return 0;;
}

/* vidmap
 * DESCRIPTION: system call for vidmap
 * INPUTS:
 * OUTPUTS:
 * RETURN VALUE:
 * SIDE EFFECTS:
 */
int32_t vidmap(uint8_t** screen_start) {
    // DEBUG PRINT
    //printf("## vidmap() : %x\n", screen_start);

    // Check if address of buf is within the address range covered
    if (screen_start == NULL || (uint32_t)screen_start < PAGE_TOP || (uint32_t)screen_start > PAGE_BOTTOM) 
        return -1;

    // map text-mode video memory into user space at virtual 256MB
    *screen_start = (uint8_t*) 0x10000000; // 256 MB

    //initialize page
    page_directory[PAGE_VIDMAP] = (uint32_t)video_pages | 7;
    video_pages[0] = (uint32_t)VIDEO | 7;

    // flush tlb
    asm volatile(
        "movl %cr3, %eax \n \
         movl %eax, %cr3"
    );

    //assign pointer to the start of video memory
    *screen_start = (uint8_t*)PAGE_VIDMEM;

    return 0;
}

/* set_handler
 * DESCRIPTION: system call for set_handler
 * INPUTS:
 * OUTPUTS:
 * RETURN VALUE:
 * SIDE EFFECTS:
 */
int32_t set_handler(int32_t signum, void* handler) {
    // DEBUG PRINT
    printf("## set_handler() : %d, %x\n", signum, handler);
    return -1;
}

/* sigreturn
 * DESCRIPTION: system call for sigreturn
 * INPUTS:
 * OUTPUTS:
 * RETURN VALUE:
 * SIDE EFFECTS:
 */
int32_t sigreturn(void) {
    // DEBUG PRINT
    printf("## sigreturn()\n");
    return -1;
}

/*
 * pcb_t * getCurrentProcessPCB()
 *   DESCRIPTION: returns pointer to the current PCB
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: A pointer to the active PCB
 *   SIDE EFFECTS: none
 */
pcb_t * getCurrentProcessPCB(){
  uint32_t esp;
  asm volatile ("\t movl %%esp,%0" : "=r"(esp));
  return (pcb_t *)(esp & ~(PCB_MASK));
}

pcb_t * getProcessPCB(uint32_t pid){
  return (pcb_t *)(0x800000 - (pid + 1) * 0x2000);
}

uint32_t get_kernel_stack_bottom(uint32_t pid){
  return 0x800000 - pid * 0x2000 - 4;
}

