#include "rtc.h"
#include "i8259.h"
#include "lib.h"
#include "types.h"

/* local variables declared */
//volatile int intflag = 0;
int rtc_call_count = 0;
volatile int intflag;

/* rtc_init()
*	DESCRIPTION: initializes the rtc
*	INPUT: none
*	OUTPUT: none
*	RETURN VALUE: none
*	SIDE EFFECTS: initializes the time clock
*/
void rtc_init(){
	/* temp values of reg a and reg b */
	char tempA, tempB;

	//char tempB;

	/*set the clock frequency */
	outb(REG_A, REG_RTC_SEL);	/* disable nmi interrupt */
	tempA = inb(REG_RTC_VAL);	/* read current val of A */
	outb(REG_A, REG_RTC_SEL);	/* reset index */
	outb((tempA & MASK_RATE)| (MASK_RATE >> 4), REG_RTC_VAL);	/* set new frequency */


	/*disable the NMI, and write the temp value to turn on bit 6 of REG_B (turning on IRQ8)*/
	outb(REG_B, REG_RTC_SEL);	/* disable NMI interrupt */
	tempB = inb(REG_RTC_VAL);	/* read current val of B */
	outb(REG_B, REG_RTC_SEL);	/* reset index */
	outb(tempB | MASK_RTC, REG_RTC_VAL); /* turn on bit 6 */

	enable_irq(8); //enable arq for rtc
	intflag = 0;
	
	rtc_call_count = 0; /* rtc_call_count initialized */
}

/* rtc_open()
*	DESCRIPTION: initializes the rtc frequency to 2hz
*	INPUT: none
*	OUTPUT: none
*	RETURN VALUE: return 0
*	SIDE EFFECTS: none
*/
int32_t rtc_open(){
	uint32_t fl;
	cli_and_save(fl);
	int rtc_init_freq = 2;			/* set frequency to 2hz */
	int val_four = 4;

	rtc_init();						/* call rtc_init */
	rtc_write(&rtc_init_freq, val_four);
	restore_flags(fl);
	return 0;
}

/* rtc_write()
*	DESCRIPTION: able to change frequency by power of two
*	INPUT: none
*	OUTPUT: none
*	RETURN VALUE: return 0 if successful
*	SIDE EFFECTS: none 
*/
int32_t rtc_write(const void* buf, int32_t nbytes){
	int32_t frequency;
	char tempA;
	uint32_t flag;


	frequency = * (int *)buf; // obtain frequency value from buf
	cli_and_save(flag);


	if ((frequency == 2)){ // if frequency is 2
		frequency = 0x0F;
	}

	else if ((frequency == 4)){ // if frequency is 4
		frequency = 0x0E;
	}

	else if ((frequency == 8)){ // if frequency is 8
		frequency = 0x0D;
	}

	else if ((frequency == 16)){ // if frequency is 16
		frequency = 0x0C;
	}

	else if ((frequency == 32)){ // if frequency is 32
		frequency = 0x0B;
	}

	else if ((frequency == 64)){ // if frequency is 64
		frequency = 0x0A;
	}

	else if ((frequency == 128)){ // if frequency is 128
		frequency = 0x02;
	}

	else if ((frequency = 256)){ // if frequency is 256
		frequency = 0x01;
	}

	else if ((frequency == 512)){ // if frequency is 512
		frequency = 0x07;
	}

	else if ((frequency == 1024)){ // if frequency is 1024
		frequency = 0x06;
	}

	else {
		frequency = -1; // error if invalid 
	}

	
	outb(REG_A, REG_RTC_SEL); //select REG_A
	tempA = inb(REG_RTC_VAL); // get value from REG_A and save to tempA
	outb(REG_A, REG_RTC_SEL); // select REG_A
	outb((tempA & 0xF0) | frequency, REG_RTC_VAL ); //Write only to the top byte, & 0xF0 
	
	restore_flags(flag);

	return frequency; //sending frequency to check its values

}

int32_t rtc_read_helper(uint32_t inode, uint32_t read_index, uint8_t *buf, uint32_t nbytes){
	return rtc_read(buf, nbytes);
}

/* rtc_read()
*	DESCRIPTION: block until next interrupt
*	INPUT: none
*	OUTPUT: none
*	RETURN VALUE: 0 if successful
*	SIDE EFFECTS: none
*/
int32_t rtc_read(const void *buf, int32_t nbytes){
	while (1){
		if (intflag == 1){		/* stop interrupts */
			intflag = 0;        // reset intflag back to 0
			return 0;
		}
	}

	return -1;
}

/* rtc_close()
*	DESCRIPTION: does nothing right now
*	INPUT: none
*	OUTPUT: none
*	RETURN VALUE: 0 if successful
*	SIDE EFFECTS: none
*/
int32_t rtc_close(){
	//do nothing
	return 0;
}

/* rtc_interrupt_handler()
*	DESCRIPTION: takes account of the interrupts made on the rtc
*	INPUT: none
*	OUTPUT: none
*	RETURN VALUE: none
*	SIDE EFFECTS: none
*/
void rtc_interrupt_handler() {


    intflag = 1; // sets the flag for interrupt call
    rtc_call_count++; // increments on the number of interrupt calls

    outb(REG_C, REG_RTC_SEL); // REG_C is set, and next interrupt is read
    inb(REG_RTC_VAL);

	send_eoi(8);

}
