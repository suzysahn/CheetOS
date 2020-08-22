#ifndef _RTC_H
#define _RTC_H
#include "lib.h"

/* DECLARATION OF CONSTANTS TO USE */
#define REG_A 			0x8A
#define REG_B 			0x8B
#define REG_C 			0x8C
#define REG_RTC_SEL 	0x70
#define REG_RTC_VAL		0x71
#define MASK_RTC	 	0x40
#define MASK_RATE       0xF0

/* FUNCTIONS DECLARED */

/* initializes the rtc */
void rtc_init();
/* opens rtc driver */
int32_t rtc_open();
/*  block until next interrupt */
int32_t rtc_read(const void *buf, int32_t nbytes);
/* able to change frequency by power of two */
int32_t rtc_write(const void* buf, int32_t nbytes);
/* closes rtc driver */
int32_t rtc_close();
/* handles interrupt for rtc*/
void rtc_interrupt_handler();

int32_t rtc_read_helper(uint32_t inode, uint32_t read_index, uint8_t *buf, uint32_t nbytes);



#endif

