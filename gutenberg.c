#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

struct termios orig_termios;

void disableRawMode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode() {
  tcgetattr(STDIN_FILENO, &orig_termios);
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
  raw.c_lflag &= ~(ECHO | ICANON | ISIG);

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main() {
  enableRawMode();

  char c;
  while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q') {
    if (iscntrl(c)) {
      printf("%d\n", c);
    }
    printf("%d ('%c')\n", c, c);
  }
  return 0;
}
