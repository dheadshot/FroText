#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "qdinp2/qdinp2.h"

#include "frotext.h"


unsigned long scrw = 0, scrh = 0; /* Screen Width & Height */
file_type ftype; /* File type */
char *afn = NULL; /* Filename */
unsigned long uschanges = 0; /* Unsaved changes */
unsigned long cx=0, cy=0, ssx=0, ssy=0, sex=0, sey=0; /*Cursor X & Y, Selected Text Start & End X & Y */
int issel; /* Is text selected? */
unsigned long scrx = 0, scry = 0; /* Screen offset X and Y */

arow *rowroot = NULL, *rowptr = NULL;

arow *newrow(char *rawtext)
{
  if (rawtext == NULL)
  {
    errno = EINVAL;
    return NULL;
  }
  arow *therow = malloc(sizeof(arow));
  if (therow == NULL)
  {
    errno = ENOMEM;
    return NULL;
  }
  memset(therow,0,sizeof(arow));
  therow->rawlen = strlen(rawtext);
  therow->rawsize = (therow->rawlen*sizeof(char)) + 1; /* There is method in this madness! */
  therow->rawtext = malloc(therow->rawsize);
  if (therow->rawtext == NULL)
  {
    free(therow);
    errno = ENOMEM;
    return NULL;
  }
  therow->next = NULL;
  return therow;
}

int findlastrow()
{
  rowptr = rowroot;
  if (rowptr == NULL) return 0;
  while (rowptr->next != NULL) rowptr = rowptr->next;
  return 1;
}

int findnthrow(unsigned long n)
{
  unsigned long i;
  if (rowroot == NULL) return 0;
  for (rowptr = rowroot; rowptr->next != NULL; rowptr = rowptr->next)
  {
    if (i == n) break;
    i++;
  }
  if (i != n) return 0;
  return 1;
}

int loadfromfile()
{
  FILE *aff;
  char *astr = malloc(sizeof(char)*2049);
  if (astr == NULL)
  {
    errno = ENOMEM;
    return 0;
  }
  aff = fopen(afn,"r");
  if (aff == NULL) return 0;
  while (fgets(astr,2048,aff) != NULL
  {
    //Create Row and add to end
    if (findlastrow() == 0)
    {
      rowroot = newrow(astr);
      if (rowroot == NULL)
      {
        if (errno != ENOMEM)
        {
          /* WTF has happened?! */
        }
        return 0;
      }
    }
    else
    {
      rowptr->next = newrow(astr);
      if (rowroot == NULL)
      {
        if (errno != ENOMEM)
        {
          /* WTF has happened?! */
        }
        return 0;
      }
    }
  }
  if (ferror(aff))
  {
    errno = EIO;
    return 0;
  }
  return 1;
}

int createedtext(arow *row, unsigned int pformat)
{
  /* pformat = previous row's formatend - not used for txt */
  unsigned long numtabs = 0,i,j;
  if (row->edtext != NULL) free(row->edtext);
  if (row->formattext != NULL) free(row->formattext);
  switch (ftype)
  {
    case txtnix:
    case txtdos:
    case txtmac:
      /* Just expand tabs and replace control chars */
      for (i=0;i < row->rawlen;i++)
      {
        if (row->rawtext[i] == 9) numtabs++;
      }
      row->edtext = (char *) malloc(sizeof(char)*(1+row->rawlen+(numtabs*DEFAULTTABSPACE)));
      if (row->edtext == NULL) return 0;
      
      j = 0;
      for (i=0;i < row->rawlen;i++)
      {
        if (row->rawtext[i] == 9)
        {
          row->edtext[j] = ' ';
          j++;
          while (j % DEFAULTTABSPACE != 0)
          {
            row->edtext[j] = ' ';
            j++;
          }
        }
        else if (row->rawtext[i] == 127 || (row->rawtext[i] >0 && row->rawtext[i] < 32))
        {
          row->edtext[j] = '?';
          j++;
        }
        else
        {
          row->edtext[j] = row->rawtext[i];
          j++;
        }
      }
      row->edtext[j] = 0;
      row->edlen = j;
      row->formattext = (unsigned int *) malloc(sizeof(unsigned int)*j);
      if (row->formattext == NULL) return 0;
      memset(row->formattext,0,j*sizeof(unsigned int));
    break;
    
    case ptxnc:
    case ptx5dos:
    case ptx6dos:
      /* Expand tabs, convert control characters, hide formatting codes and format text */
    break;
    
    case ptxcpc:
      /* Expand tabs, convert control characters, hide formatting codes and format text */
    break;
    
    default:
      return 0;
    break;
  }
  return 1;
}
