
#define ASM 
#include "keyboard.h"

/* rtc_handler()
* DESCRIPTION: interrupt handler for RTC 
* INPUT: none
* OUTPUT: none
* RETURN VALUE: none
* SIDE EFFECTS: sets up intflag and re-enabling interrupt
*/
.text
.globl rtc_handler
rtc_handler:
    PUSHAL                     
    call rtc_interrupt_handler       
    pushl $8                    # pass IRQ number
    call send_eoi               # call eoi
    addl $4, %esp               # leave
    POPAL                       
    IRET                        


/*
 * key_handler
 *   DESCRIPTION: handler for key press
 *   INPUTS: none
 *   OUTPUTS: the key pressed to the screen
 *   RETURN VALUE: none
 *   SIDE EFFECTS: writes to the screen for proper key pressed
 */
 .globl key_handler
key_handler:
    PUSHAL                      # Save all registers
    call handle_charpress       
    pushl $1                    # Pass IRQ number
    call send_eoi               # send to eoi
    addl $4, %esp               # leave
    POPAL                       
    IRET                        

/*
 * trap_handler
 *   DESCRIPTION: asm wrapper for when handle_trap executes
 *   INPUTS: none
 *   OUTPUTS: writes to screen
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
 .globl trap_handler
trap_handler:
    PUSHAL                      # Save all registers
    call handle_trap            # Dummy method, prints that trap was called
    POPAL
    IRET

/*
 * system_call_handler
 *   DESCRIPTION: Assembly wrapper for a c function for system calls
 *   INPUTS: %eax - syscall number
 *       %ebx - 1st argument 
 *       %ecx - 2nd argument
 *       %edx - 3rd argument
 *   OUTPUTS: output of system call called
 *   RETURN VALUE: %eax, if applicable 
 */
system_call_jump_table:
.long   0, halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn

.globl system_call_handler
system_call_handler:
  /* Save all registers and flags except for eax, the return value */
    pushl %gs
    pushl %fs
  pushl %es
    pushl %ds
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %esi
    pushl %edi
    pushl %ebp
    pushfl

    /* Pushing arguments - need to save all registers according to Appendix B. */
    pushl %ebp    /* Pushed "to avoid leaking information to the user programs" */
    pushl %edi    /* Pushed "to avoid leaking information to the user programs" */
    pushl %esi    /* Pushed "to avoid leaking information to the user programs" */
    pushl %edx    /* Argument 3 */
    pushl %ecx    /* Argument 2 */
    pushl %ebx    /* Argument 1 */

    /* Check to see if our System Call Number (stored in %EAX) is within bounds (Chkpt 3 - 1:10) */
    cmpl $1, %eax
    jl invalid
    cmpl $10, %eax
    jg invalid
  
  /* Call the correct system call according to the jumptable */
    call *system_call_jump_table(,%eax,4)
    jmp restore

invalid:
  movl $-1, %eax

restore:
    # Popping arguments - 6 Registers * 4 Bytes = 24
    addl $24, %esp

    # Restore all regs, except for eax, and flags
    popfl
    popl %ebp
    popl %edi
    popl %esi
    popl %edx
  popl %ecx
    popl %ebx
    popl %ds
    popl %es
    popl %fs
    popl %gs
    IRET

/*
 * context_switch
 *   DESCRIPTION: Assembly wrapper for a c function for switching processes 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 */
.globl context_switch
context_switch:
    #cli
    movl 4(%esp), %ecx # ecx has eip
    movl 8(%esp), %edx # edx has bottom_addr_of_page

# ss, esp, eflags, cs, eip - push in order!

# push ss
    pushl $0x002B  # USER_DS

# push esp
    pushl %edx

# push eflags 
    pushfl
    popl %eax
    orl $0x200, %eax  # 0x200 is the IF flag
    pushl %eax

# push cs (with privilege levels)
    pushl $0x0023  # USER_CS

# push eip
    pushl %ecx

# context switch by IRET
    IRET

/*
 * return_to_execute
 *   DESCRIPTION: Assembly wrapper for a c function for return_to_execute
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 */
.globl return_to_execute
return_to_execute:
# save and load ebp, ebx
  movl 8(%esp), %ebp
  movl 4(%esp), %ebx

# clear eax, load al, and change stack pointer
  xorl %eax, %eax
  movb 16(%esp), %al
  movl 12(%esp), %esp
  jmp *%ebx

/* 
 * do_execute
 *  DESCRIPTION: assembly wrapper for a c function for execute
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: - if success, -1 fail
 *  SIDE EFFECTS: changes the PCB
 */
.globl do_execute
do_execute:
  pushl %ebx
# syscall number 2
  movl  $2,%eax

# save and load ebx, ecx, and edx
  movl  8(%esp),%ebx
  MOVL  12(%esp),%ecx
  MOVL  16(%esp),%edx

# calls the interrupt 0x80 (128) 
  INT   $128
  POPL  %ebx
  RET




