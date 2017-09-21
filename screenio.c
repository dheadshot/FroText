#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

#include "screenio.h"


int getcurpos(unsigned long *px, unsigned long *py)
{
  /* Inspired by Kilo - I may need to do some termios thing here... */
  char buffer[64];
  int i = 0;
  
  if (write(STDOUT_FILENO,"\x1b[6n",4 != 4) return 0;
  while (i<63)
  {
    if (read(STDIN_FILENO,buffer+(i*sizeof(char)),1) != 1) break;
    if (buffer[i] == 'R') break;
    i++;
  }
  buffer[i] = 0;
  if (buffer[0]!=27 || buffer[1]!='[') return 0;
  if (sscanf(buffer+(2*sizeof(char)),"%lu;%lu",py,px) != 2) return 0;
  return 1;
}
