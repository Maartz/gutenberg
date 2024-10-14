#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

/*** defines ***/

// 0x1F decimal 31
// As I want to bind exit to CTRL + Q 
// 01010001&00011111=00010001 so 17 in ASCII
#define CTRL_KEY(k) ((k) & 0x1F)

/*** data ***/

struct editorConfig {
  struct termios orig_termios;
};

struct editorConfig E;

/*** terminal ***/

void die(const char *s) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);

  perror(s);
  exit(1);
}

void disableRawMode(void) {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1) {
    die("tcsetattr");
  } 
}

void enableRawMode(void) {
  if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) die("tcgetattr");
  atexit(disableRawMode);

  struct termios raw = E.orig_termios;

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

char editorReadKey(void) {
  int nread;
  char c;

  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    // To make it work in Cygwin, I won’t treat EAGAIN as an error.
    if (nread == -1 && errno != EAGAIN) die("read");
  }
  return c;
}

/*** output ***/

void editorDrawRows(void) {
  int y;
  for (y = 0; y < 24; y++) {
    write(STDOUT_FILENO, "~\r\n", 3);
  }
}

// \x1b is ESC
// [2J is an escape sequence that tells the terminal to clear the entire screen
void editorRefreshScreen(void) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
  editorDrawRows();
  write(STDOUT_FILENO, "\x1b[H", 3);
}

/*** input ***/

void editorProcessKeypress(void) {
  char c = editorReadKey();

  switch (c) {
    case CTRL_KEY('q'):
      exit(0);
      break;
  }
}

/*** init ***/

int main(void) {
  enableRawMode();

  while (1) {
    editorRefreshScreen();
    editorProcessKeypress();
  }
  return 0;
}
