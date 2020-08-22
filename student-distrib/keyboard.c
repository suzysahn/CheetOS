#include "keyboard.h"
#include "lib.h"

/** IBM 101 Key Code Table
 -----------------------------------------------------------  
| 29| 02| 03| 04| 05| 06| 07| 08| 09| 0A| 0B| 0C| 0D|     0E| 
|-----------------------------------------------------------| 
|   0F| 10| 11| 12| 13| 14| 15| 16| 17| 18| 19| 1A| 1B|   2B| 
|-----------------------------------------------------------| 
|    3A| 1E| 1F| 20| 21| 22| 23| 24| 25| 26| 27| 28|      1C| 
|-----------------------------------------------------------| 
|      2A| 2C| 2D| 2E| 2F| 30| 31| 32| 33| 34| 35|        36|  
|-----------------------------------------------------------|  
|   1D|-5B|   38|                       39|-38|-5C|-5D|  -1D| 
 -----------------------------------------------------------  
**/
#define NUM_CHAR_HANDLE   58    /* number of keyboard chars we are handling -- is valid */

/* Array containing ascii codes for appropriate scancode translations */
char scan2ascii[NUM_CHAR_HANDLE][2] = {    // 2 versions: original version and shifted version
  { 0 , 0 }, {ESC,ESC}, {'1','!'}, {'2','@'}, {'3','#'}, {'4','$'}, {'5','%'}, {'6','^'}, // 00 - 
  {'7','&'}, {'8','*'}, {'9','('}, {'0',')'}, {'-','_'}, {'=','+'}, {BS ,BS }, {TAB,TAB}, // 08 -
  {'q','Q'}, {'w','W'}, {'e','E'}, {'r','R'}, {'t','T'}, {'y','Y'}, {'u','U'}, {'i','I'}, // 10 -
  {'o','O'}, {'p','P'}, {'[','{'}, {']','}'}, {CAP,CAP}, { 0 , 0 }, {'a','A'}, {'s','S'}, // 18 -
  {'d','D'}, {'f','F'}, {'g','G'}, {'h','H'}, {'j','J'}, {'k','K'}, {'l','L'}, {';',':'}, // 20 - 
  {SHI,IFT}, {'`','~'}, { 0 , 0 },{'\\','|'}, {'z','Z'}, {'x','X'}, {'c','C'}, {'v','V'}, // 28 - 
  {'b','B'}, {'n','N'}, {'m','M'}, {',','<'}, {'.','>'}, {'/','?'}, { 0 , 0 }, { 0 , 0 }, // 30 -
  { 0 , 0 }, {' ',' '},                                                                   // 38 -     
};

/*      LOCAL DEFINITIONS AND VARIABLES     */
/* following constants are indicating the scan codes for the special function keys */
#define CTRL         0x1D
#define SHIFT_LEFT   0x2A
#define SHIFT_RIGHT  0x36
#define ALT          0x38 
#define CAPS         0x3A        /* this key toggles */
#define BACKSPACE    0x0E        
#define KEY_L        0x26
#define KEY_ENTER    0x1C

#define PR_CONVER    0x80        /* converts the pressed and released scan codes */
#define CAPS_CONVER  0x20        /* converts upper and lower case alphabets */
#define CHAR_MASK    0177        /* mask to get character */
#define ASCII_MAX    0x7E        /* max ascii on keyboard */
#define ASCII_MIN    0x20        /* min ascii on keyboard */


/* following variables holds the status of the special keys, if pressed/been pressed or not */
static uint8_t ctrl_stat  = 0;
static uint8_t shift_stat = 0;
static uint8_t alt_stat   = 0;
static uint8_t caps_stat  = 0;
static uint16_t pushed_key = 0;   /* holds the key pushed -- input */
/*
* handle_charpress
*   DESCRIPTION:  Handle all keyboard inputs from keyboard, including special characters
*                 such as caplock, shifts, alts, and controls, and gives it to putc to properly 
*                 display them ont the terminal. 
*   INPUT:        none
*   OUTPUT:       none
*   RETURN VALUE: none
*   SIDE EFFECTS: processes keys to be printed
*/
void handle_charpress(void) {
  /* read in key pressed from data port */
  uint8_t scancode = inb(KEYBOARD_DATA_PORT);
  uint8_t keycode = 0;

  /* dependent on the input, set the status of each special character */
  switch (scancode) {
    case CTRL:                          /* control pressed */
      ctrl_stat = 1; break; 
    
    case (CTRL + PR_CONVER):                 /* control released */
      ctrl_stat = 0; break;
    
    case SHIFT_LEFT:                    /* shift pressed (left or right) */
    case SHIFT_RIGHT:
      shift_stat = 1; break;
    
    case (SHIFT_LEFT + PR_CONVER):           /* shift released (left or right) */
    case (SHIFT_RIGHT + PR_CONVER):
      shift_stat = 0; break;
    
    case ALT:                           /* alt pressed */ 
      alt_stat = 1; break;
    
    case (ALT + PR_CONVER):                  /* alt released */
      alt_stat = 0; break;
    
    case CAPS:                          /* caps lock on */
      caps_stat = caps_stat ? 0 : 1; 
      break;
    
    case (CAPS + PR_CONVER):                 /* caps lock off */
      break;

    default:
      if (alt_stat){                    /* alt + backspace allowed */
        if(scancode == BACKSPACE){
          keycode = '\b';
        } else { 
            return; 
          }
      }

      /* control + a-z */
      if (scancode < NUM_CHAR_HANDLE) {              
        if (ctrl_stat) {
          /* ctrl + backspace allowed */
          if(scancode == BACKSPACE) {
            keycode = '\b';
          } 
          /* ctrl + enter allowed */
          else if(scancode == KEY_ENTER) {   
            keycode = '\n';
          } 

          /* ctrl + L allowed */
          else if (scancode == KEY_L) {
            keycode = scan2ascii[scancode][0];
            if ('a' <= keycode && keycode <= 'z') {
              keycode = keycode - 'a' + 1;
              break;
            }
          } 
          else {      /* if not, alt and ctrl not allowed with any alphanumeric + symbols */
            return; 
          }
        }

        /* deal with regular keycodes */
        keycode = scan2ascii[scancode][shift_stat];

        /* deal with cap lock, only on alphabet characters */
        if (caps_stat) {
            if ('a' <= keycode && keycode <= 'z')
              keycode -= CAPS_CONVER;
            else if ('A' <= keycode && keycode <= 'Z')
              keycode += CAPS_CONVER;
        }

      }
  }

/** for testing
  if (scancode < 128)
    printf("%x = '%c', ", scancode, keycode);
  else
    printf("%x = '%c'\n", scancode, keycode);
**/

/* set the keycode to pushed_key */
  if (keycode)
    pushed_key = keycode;
}

/*
 * handle_trap
 *   DESCRIPTION: A temporary function that just shows system traps can be called
 *   INPUTS:  none
 *   OUTPUT: writes a message to the screen
 *   RETURN VALUE: none
 *   SIDE EFFECTS: writes to screen
 */
void handle_trap(void){
  printf("System trap was called.\n");
}

/*
 * getchar
 *    DESCRIPTION: helper function to pass the characters read to getline function
 *    INPUTS:      none
 *    OUTPUTS:     none
 *    RETURN VALUE:none
 *    SIDE EFFECTS:none
 */
char getchar(void) {
  char key;

  /* Wait until key input */
  while (1) {
    while (!pushed_key) ;

    if (pushed_key < PR_CONVER) {    /* if keypressed is pure alphanumeric characters, return key to getline*/
      key = (char)pushed_key;
      pushed_key = 0;
      return key;
    }

    pushed_key = 0;               /* set pushed_key to zero */
  }
}

/*
 * getline
 *    DESCRIPTION: given a buffer, putc on to terminal, following the rules
 *                 provided to us (CTRL-L clears, enters properly, backspaces)
 *    INPUTS:      pointer to buf, max characters we can print out 
 *    OUTPUTS:     lp - buf when enter is pressed
 *    RETURN VALUE: none
 *    SIDE EFFECTS: none
 */
int getline(char *buf, int max) {
	int c;          
	char *lp;

	for (lp = buf;;)                                           /* load buf onto lp */
		switch (c = getchar() & CHAR_MASK) { /*  if enter if pressed, then next line */
      case '\n':
      case '\r':
        *lp = '\0';
        //printf("enter pressed");
        putc('\n');
        return (lp - buf);                           /*  return how much printed */

      case '\b':                                           /*  backspace printed */
      case '\177':                             /* 177 is the backspace/esc ascii */
        if (lp > buf) {
          lp--;
          putc('\b');
        }
        break;

      case CTRL_L:                                /* clear the screen with CTRL_L */
        clear();    
        lp = buf;                                            /*  clear the buffer */
        break;

      default:
        if ((lp - buf) >= max) break;                   /* check reach to max - 1 */

        if (ASCII_MIN <= c && c <= ASCII_MAX) {             /*  print c to screen */
          *lp++ = c;
          putc(c);
        }
		}
}

/*
 * terminal
 *    DESCRIPTION: prints accordingly to screen, following the max 128 letters. 
 *    INPUTS:      none
 *    OUTPUTS:     none
 *    RETURN VALUE:none
 *    SIDE EFFECTS:none
 */
void terminal(void) {
  int len;
  char buf[128];                /* holds up to 127 char + '\n' char */

  while (1) {
    len = getline(buf, 128);    /* print contents of buffer */
  } 
}

