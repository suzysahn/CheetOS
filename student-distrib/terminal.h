#ifndef TERMINAL_H
#define TERMINAL_H

/* includes files to be used/referenced */
#include "types.h"

#define TERMINAL_BUFFER_SIZE 128

/* write a single character to terminal screen */
void terminal_putc(char input);

/* reads FROM the keyboard buffer into buf */
int32_t terminal_read(int32_t fd, char *buffer, uint32_t num_chars);

/* writes TO the terminal screen from buf */
int32_t terminal_write(int32_t fd, char *buffer, uint32_t num_chars);

/* Called by systemcalls, but no specific tasks to carryout */
int32_t terminal_open(void);        /* potentially initializes terminal */
int32_t terminal_close(void);       /* potentially clears any terminal specific variables */

/* clears terminal screen and reset cursor and buffer */
void clear_terminal(void);

#endif
