#ifndef _SCREENIO_H_
#define _SCREENIO_H_

int movetoraw();
int movetonormal();
int setcurpos(unsigned long px, unsigned long py);
int getcurpos(unsigned long *px, unsigned long *py);
int getscreensize(unsigned long *sw, unsigned long *sh);
/* Returns 1 if worked, 1 if didn't, 2 if did but cursor in wrong place */

#endif
