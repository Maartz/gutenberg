#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

// 0x1F decimal 31
// As I want to bind exit to CTRL + Q 
// 01010001&00011111=00010001 so 17 in ASCII
#define CTRL_KEY(k) ((k) & 0x1F)

struct termios orig_termios;

void die(const char *s) {
  perror(s);
  exit(1);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
    die("tcsetattr");
  } 
}

void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
  atexit(disableRawMode);

  struct termios raw = orig_termios;

  /* 
   https://www.man7.org/linux/man-pages/man3/termios.3.html
   c_lflag is local flags
   strange &= and ~ are bitwise operations
   ECHO is 00000000000000000000000000001000 in binary
   ~(ECHO) is 11111111111111111111111111110111
   and &= is to set the fourth bit to be 1 but retain other bits to stay 1 as well.

   c_lflag flag constants:

   ISIG   When any of the characters INTR, QUIT, SUSP, or DSUSP are
           received, generate the corresponding signal.

   ICANON Enable canonical mode (described below). 
  */
  raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
  raw.c_oflag &= ~(OPOST);
  raw.c_iflag &= ~(BRKINT | IXON | ICRNL);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

int main() {
  enableRawMode();

  char c;
  while (1) {
    char c = '\0';
    // To make it work in Cygwin, I won’t treat EAGAIN as an error.
    if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");
    if (iscntrl(c)) {
      printf("%d\r\n", c);
    }
    printf("%d ('%c')\r\n", c, c);

    if (c == CTRL_KEY('q')) break;
  }
  return 0;
}
