#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include "screenio.h"


int inraw;
struct termios currentt, rawt;


int movetoraw()
{
  if (inraw) return 1;
  if (tcgetattr(STDIN_FILENO,&currentt) == -1)
  {
    return 0;
  }
  rawt = currentt;
  rawt.c_iflag &= ~ ( IXON | BRKINT | INPCK | ISTRIP | ICRNL );
  rawt.c_oflag &= ~ OPOST;
  rawt.c_cflag |= CS8;
  rawt.c_lflag &= ~( ECHO | ICANON | IEXTEN | ISIG );
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;
  if (tcsetattr(STDIN_FILENO,TCSAFLUSH,&rawt)<0) return 0;
  inraw = 1;
  return 1;
}

int movetonormal()
{
  if (!inraw) return 1;
  if (tcsetattr(STDIN_FILENO,TCSAFLUSH,&currentt) < 0) return 0;
  inraw = 0;
_  return 1;
}

int setcurpos(unsigned long px, unsigned long py)
{
  int ret = 0;
  char thecode[64] = "";
  movetoraw();
  snprintf(thecode, 63, "\x1b[%lu;%luH",py,px);
  if (write(STDOUT_FILENO,thecode, strlen(thecode)) == strlen(thecode)) ret = 1;
  movetonormal();
  return ret;
}

int getcurpos(unsigned long *px, unsigned long *py)
{
  /* Inspired by Kilo - I may need to do some termios thing here... */
  char buffer[64];
  int i = 0;
  
  if (!movetoraw()) return 0;
  
  if (write(STDOUT_FILENO,"\x1b[6n",4) != 4)
  {
    movetonormal();
    return 0;
  }
  while (i<63)
  {
    if (read(STDIN_FILENO,buffer+(i*sizeof(char)),1) != 1) break;
    if (buffer[i] == 'R') break;
    i++;
  }
  buffer[i] = 0;
  if (buffer[0]!=27 || buffer[1]!='[')
  {
    movetonormal();
    return 0;
  }
  if (sscanf(buffer+(2*sizeof(char)),"%lu;%lu",py,px) != 2)
  {
    movetonormal();
    return 0;
  }
  return 1;
}

int getscreensize(unsigned long *sw, unsigned long *sh)
{
  /* Returns 1 if worked, 1 if didn't, 2 if did but cursor in wrong place */
  struct winsize ws;
  if (ioctl(1,TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
  {
    unsigned long px, py;
    if (!getcurpos(&px,&py)) return 0;
    if (!setcurpos(999,999)) return 0;
    if (!getcurpos(&sw,&sh)) return 0;
    if (!setcurpos(px, py))
    {
      /* Erk! */
      return 2;
    }
  }
  else
  {
    *sw = ws.ws_col;
    *sh = ws.ws_row;
  }
  return 1;
}
