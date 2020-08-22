#include "terminal.h"
#include "lib.h"
#include "keyboard.h"
/*  ^^ includes files to be used/referenced */

/* ========================= DEFINING CONSTANTS ========================START=  */
#define MAX_INPUT_CHARS     128         /* max number of characters that can be
                                           taken from input before another '\n' */
/* ========================= DEFINING CONSTANTS ==========================END=  */

/*
 * terminal_putc()
 * DESCRIPTION: calls the modified putc in lib.c to output a single 
 *              character to the console/screen
 * INPUTS: input -- character taken from input and to be written to screen
 * OUTPUTS: modifies the video memory where the cursor is at
 * RETRUN VALUE: none
 * SIDE EFFECTS: modifies the terminal screen by placing the input 
 *               character in video memory
 */
void 
terminal_putc(char input) {
    /* call putc from lib.c */
    putc(input);
}


/*
 * terminal_read()
 * DESCRIPTION: Reads from the keyoard input buffer and copies to buffer in userspace
 * INPUTS: -- char *buffer: userspace buffer will hold input buffer data
 *         -- uint32_t num_bytes: maximum number of bytes to copy between buffers
 * OUTPUTS: none
 * RETURN VALUE: number of bytes copied between buffers
 * SIDE EFFECTS: fills user buffer with terminal's input buffer data
 */
int32_t 
terminal_read(int32_t fd, char *buffer, uint32_t num_chars) {
    //uint32_t read_flag;
    int copy_result;

    if (fd != 0) return -1;

    /* check if buffers exist/passed in correctly */
    if (buffer == NULL) 
        return -1;

    /* truncates the num_bytes copied to make sure no accessing nonexistent idx */
    if (num_chars > MAX_INPUT_CHARS) 
        num_chars = MAX_INPUT_CHARS;

    /* ============= START critical section to copy buffer ============= */
    //cli_and_save(read_flag);
    
    /* calls getline in keyboard.c: copy buffer and get number of bytes 
       that wasn't copied */
    sti();
    copy_result = getline(buffer, num_chars);
    
    //restore_flags(read_flag);
    /* ============== END critical section to copy buffer ============== */

    return copy_result;
}


/*
 * terminal_write()
 * DESCRIPTION: writes string (buf of chars) to screen until a null character
                or the number of chars to write is reached
 * INPUTS: -- uint8_t* buffer: holding the string to write to screen
 *         -- uint32_t characters: maximum number of characters to write
 * OUTPUTS: outputs to video memory
 * RETURN VALUE: number of characters wrote to video mem/screen (max at num_chars)
 * SIDE EFFECTS: writes to video memory/terminal screen
 */
int32_t terminal_write(int32_t fd, char *buffer, uint32_t num_chars) {
    //uint32_t read_flag;
    int32_t idx = 0;

    if (fd != 1) return -1;

    /* check if buffers exist/passed in correctly */
    if (buffer == NULL) 
        return -1;
    /* check for the case when there is no or invalid number of characters to write */
    if (num_chars == 0) 
        return 0;

    /* ============= START critical section to write buffer ============= */
    //cli_and_save(read_flag);

    while (buffer[idx] != '\0') {
        /* write char to screen */
        putc(buffer[idx]);
        /* check if max amount to copy is reached */
        if (++idx == num_chars)
            break;
    }

    //restore_flags(read_flag);
    /* ============== END critical section to write buffer ============== */

    return idx;
}



/*
 * terminal_open()
 * DESCRIPTION: called by syscalls, but no specific tasks to carryout
 * INPUTS: none
 * OUTPUTS: none
 * RETURN VALUE: returns 0 always
 * SIDE EFFECTS: none
 */
int32_t terminal_open(void) {
    /* return 0 always - no tasks */
    return 0;
}


/*
 * terminal_close()
 * DESCRIPTION: called by syscalls, but no specific tasks to carryout
 * INPUTS: NONE
 * OUTPUTS: NONE
 * RETURN VALUE: returns 0 always
 * SIDE EFFECTS: NONE
 */
int32_t terminal_close(void) {
    /* return 0 always - no tasks */
    return 0;
}


/*
 * clear_terminal()
 * DESCRIPTION: clears the terminal screen, sets cursor, and clear input buffer
 * INPUTS: none
 * OUTPUTS: blank, cleared terminal screen
 * RETURN VALUE: none
 * SIDE EFFECTS: clears the terminal screen, video memory, and reset cursor to 
 *               the top left corner
 */
void clear_terminal(void) {
    /* call clear screen from lib.c */
    clear();

    /* clear the buffers */
    //memset(buf, 0, 128);
}
