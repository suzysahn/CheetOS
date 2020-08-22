#include "idt.h"
#include "lib.h"
#include "x86_desc.h"

#define num_interrupt 0x20 /* 32 defined IDT entries*/

/*
 * stop
 *   DESCRIPTION: stops the system indefinitely
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: turning off interrupts and not returning back into the function
 */
void stop(void){
    cli();                                /* turn off interrupts */
    while(1);                             /* keep spinning, make everything stop */
}

/*
 * init_idt
 *   DESCRIPTION: intialize the IDT with passing in of pointers to each interrupt
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN_VALUE: none
 *   SIDE EFFECTS: sets up the entries to idt.
 */
void init_idt(void){
    SET_IDT_ENTRY(idt[0],  &divide_error);           //IDT 00
    SET_IDT_ENTRY(idt[1],  &debug);                  //IDT 01
    SET_IDT_ENTRY(idt[2],  &nmi);                    //IDT 02
    SET_IDT_ENTRY(idt[3],  &breakpoint);             //IDT 03
    SET_IDT_ENTRY(idt[4],  &overflow);               //IDT 04
    SET_IDT_ENTRY(idt[5],  &bounds);                 //IDT 05
    SET_IDT_ENTRY(idt[6],  &invalid_op);             //IDT 06
    SET_IDT_ENTRY(idt[7],  &device_not_available);   //IDT 07
    SET_IDT_ENTRY(idt[8],  &double_fault);           //IDT 08
    SET_IDT_ENTRY(idt[9],  &segment_overrun);        //IDT 09
    SET_IDT_ENTRY(idt[10], &invalid_TSS);            //IDT 10
    SET_IDT_ENTRY(idt[11], &segment_not_present);    //IDT 11
    SET_IDT_ENTRY(idt[12], &stack_segment);          //IDT 12
    SET_IDT_ENTRY(idt[13], &general_protection);     //IDT 13
    SET_IDT_ENTRY(idt[14], &page_fault);             //IDT 14
    SET_IDT_ENTRY(idt[15], &generic_error);          //IDT 15: Reserved
    SET_IDT_ENTRY(idt[16], &fp);                     //IDT 16
    SET_IDT_ENTRY(idt[17], &alignment_check);        //IDT 17
    SET_IDT_ENTRY(idt[18], &machine_check);          //IDT 18

    /* Use the generic_handler for rest of idt entry*/
    uint32_t i;
    for(i = 19; i < num_interrupt; i++)
      SET_IDT_ENTRY(idt[i], &generic_error);

    /* We can't handle the rest right now, so pass them as generic */
    for(; i < NUM_VEC; i++) 
      SET_IDT_ENTRY(idt[i], &generic_error);
}

/*
 * divide_error
 *   DESCRIPTION: Handle divide by 0 exception
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: calls blue_screen to kernel panic and stop
 */
void divide_error(void){
    blue_screen();
    printf("Division by 0");
    stop();
}

/*
 * debug
 *   DESCRIPTION: Handle debug exception
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: calls blue_screen to kernel panic and stop
 */
void debug(void){
    blue_screen();
    printf("Debug Exception");
    stop();
}

/*
 * nmi
 *   DESCRIPTION: Handle non-maskable interrupt
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: calls blue_screen to kernel panic and stop
 */
void nmi(void){
    blue_screen();
    printf("Non-Maskable Interrupt");
    stop();
}

/*
 * breakpoint
 *   DESCRIPTION: Handle breakpoint exception
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: calls blue_screen to kernel panic and stop
 */
void breakpoint(void){
    blue_screen();
    printf("Breakpoint (INT3)");
    stop();
}

/*
 * overflow
 *   DESCRIPTION: Handle overflow exceptions with EFLAGS
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: calls blue_screen to kernel panic and stop
 */
void overflow(void){
    blue_screen();
    printf("Overflow with EFLAGS[OF] Set");
    stop();
}

/*
 * bounds
 *   DESCRIPTION: Handle bound exception
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: calls blue_screen to kernel panic and stop
 */
void bounds(void){
    blue_screen();
    printf("Debug Exception");
    stop();
}

/*
 * invalid_op
 *   DESCRIPTION: Handle debug exception
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: calls blue_screen to kernel panic and stop
 */
void invalid_op(void){
    blue_screen();
    printf("Invalid Opcode");
    stop();
}

/*
 * device_not_available
 *   DESCRIPTION: Handle exception when floating point unit is missing
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: calls blue_screen to kernel panic and stop
 */
void device_not_available(void){
    blue_screen();
    printf("Floating Point Unit Missing");
    stop();
}

/*
 * double_fault
 *   DESCRIPTION: Handle double fault
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: calls blue_screen to kernel panic and stop
 */
void double_fault(void){
    blue_screen();
    printf("Double Fault");
    stop();
}

/*
 * segment_overrun
 *   DESCRIPTION: Handle coprocessor segment oveerun
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: calls blue_screen to kernel panic and stop
 */
void segment_overrun(void){
    blue_screen();
    printf("Coprocessor Segment Overrun");
    stop();
}

/*
 * invalid_TSS
 *   DESCRIPTION: Handle invalid TSS exception
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: calls blue_screen to kernel panic and stop
 */
void invalid_TSS(void){
    blue_screen();
    printf("Invalid TSS");
    stop();
}

/*
 * segment_not_present
 *   DESCRIPTION: Handle segment-not-present exception
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: calls blue_screen to kernel panic and stop
 */
void segment_not_present(void){
    blue_screen();
    printf("Segment Not Present");
    stop();
}

/*
 * stack_segment
 *   DESCRIPTION: Handle stack exception
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: calls blue_screen to kernel panic and stop
 */
void stack_segment(void){
    blue_screen();
    printf("Stack Exception");
    stop();
}

/*
 * general_protection
 *   DESCRIPTION: Handle general protection exception
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: calls blue_screen to kernel panic and stop
 */
void general_protection(void){
    blue_screen();
    printf("General Protection Exception");
    stop();
}

/*
 * page_fault
 *   DESCRIPTION: Handle page fault exception
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: calls blue_screen to kernel panic and stop
 */
void page_fault(void){
    blue_screen();
    printf("Page Fault");
    stop();
}

/*
 * generic_error
 *   DESCRIPTION: Handle errors we can't handle yet (not defined)
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: calls blue_screen to kernel panic and stop
 */
void generic_error(void){
    blue_screen();
    printf("Undefined Exception");
    stop();
}

/*
 * fp
 *   DESCRIPTION: Handle floating point error exception
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: calls blue_screen to kernel panic and stop
 */
void fp(void){
    blue_screen();
    printf("Floating Point Error");
    stop();
}

/*
 * alignment_check
 *   DESCRIPTION: Handle alignment check exception
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: calls blue_screen to kernel panic and stop
 */
void alignment_check(void){
    blue_screen();
    printf("Alignment Check");
    stop();
}

/*
 * machine_check
 *   DESCRIPTION: Handle machine check exception
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: calls blue_screen to kernel panic and stop
 */
void machine_check(void){
    blue_screen();
    printf("Machine Check Exception");
    stop();
}
