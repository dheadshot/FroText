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

int formatfromn(unsigned long n, unsigned int pformat)
{
  if (!findnthrow(n)) return 0;
  while (rowptr != NULL)
  {
    if (!createedtext(rowptr,pformat)) return 0;
    pformat = rowptr->formatend;
    rowptr = rowptr->next;
  }
  return 1;
}

int insertrow(unsigned long atrow, char *rawtext)
{
  unsigned long prow = atrow - 1;
  arow *therow = newrow(rawtext);
  if (therow == NULL) return 0;
  if (atrow == 0 || rowroot == NULL)
  {
    therow->next = rowroot;
    rowroot = therow;
    if (!createedtext(therow,0)) return 0;
  }
  else
  {
    if (!findnthrow(prow))
    {
      /* Just add it to the bottom */
      findlastrow();
      rowptr->next = therow;
    }
    else
    {
      therow->next = rowptr->next;
      rowptr->next = therow;
    }
    if (!createedtext(therow,rowptr->formatend)) return 0;
  }
  return 1;
}

void freerow(arow *row)
{
  if (row->rawtext != NULL) free(row->rawtext);
  if (row->edtext != NULL) free(row->edtext);
  if (row->formattext != NULL) free(row->formattext);
  free(row);
}

int delrow(unsigned long atrow)
{
  /* Returns: 1=success, 0=failure, 2=possibly corrupted, but deletion worked */
  unsigned long prow = atrow - 1;
  unsigned int formatend = 0;
  arow *delrow;
  if (rowroot == NULL) return 0;
  if (!findnthrow(atrow)) return 0;
  if (atrow == 0)
  {
    delrow = rowroot;
    rowroot = rowroot->next;
    freerow(delrow);
  }
  else
  {
    if (!findnthrow(prow)) return 0;
    delrow = rowptr->next;
    rowptr->next = delrow->next;
    freerow(delrow);
    formatend = rowptr->formatend;
  }
  uschanges++;
  /* Format all from atrow, starting with formatend */
  if (!formatfromn(atrow,formatend)) return 2;
  return 1;
}

int insertstrinrow(arow *row, unsigned long atchar, unsigned long rownum, 
                   char *astr)
{
  /* Returns: 1=success, 0=failure, 2=astr was blank, 3=could not reformat */
  unsigned long aslen = strlen(astr);
  if (aslen == 0) return 2;
  if (atchar > row->rawlen)
  {
    if (atchar+aslen+1 > row->rawsize)
    {
      row->rawsize = (atchar +1+aslen) * sizeof(char);
      row->rawtext = (char *) realloc(row->rawtext,row->rawsize);
      if (row->rawtext == NULL) return 0;
    }
    unsigned long i;
    for (i=row->rawlen;i<atchar;i++) row->rawtext[i] = ' ';
    strcpy(row->rawtext+(sizeof(char)*atchar),astr);
  }
  else
  {
    if (aslen+1+row->rawlen > row->rawsize)
    {
      row->rawsize = (row->rawlen +1+aslen)*sizeof(char);
      row->rawtext = (char *) realloc(row->rawtext,row->rawsize);
      if (row->rawtext == NULL) return 0;
    }
    memmove(row->rawtext+(sizeof(char)*(atchar+aslen)), 
            row->rawtext+(sizeof(char)*atchar), 
            sizeof(char)*(row->rawlen-atchar+1));
    memcpy(row->rawtext+(sizeof(char)*atchar),astr,aslen*sizeof(char));
  }
  uschanges++;
  if (rownum == 0)
  {
    if (!formatfromn(rownum, 0)) return 3;
  }
  else
  {
    if (!findnthrow(rownum - 1))
    {
      return 3;
    }
    if (!formatfromn(rownum, rowptr->formatend)) return 3;
  }
  return 1;
}

int delstrinrow(arow *row, unsigned long slen, unsigned long spos, 
                unsigned long rownum)
{
  /* Returns: 1=success, 0=failure, 2=could not reformat */
  if (spos >= row->rawlen) return 0;
  if (slen == 0) return 0;
  if (spos+slen>row->rawlen) slen=row->rawlen-spos;
  memmove(row->rawtext+(sizeof(char)*spos), 
          row->rawtext+(sizeof(char)*(spos+slen)), 
          sizeof(char)*(row->rawlen+1-(spos+slen)));
  row->rawlen -= slen;
  uschanges++;
  if (rownum == 0 && (!formatfromn(0,0))) return 2;
  if (rownum >0)
  {
    if ((!findnthrow(rownum-1)) || (!formatfromn(rownum,rowptr->formatend))) return 2;
  }
  return 1;
}
