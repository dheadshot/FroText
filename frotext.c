#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "qdinp2/qdinp2.h"

#include "frotext.h"


unsigned long scrw = 0, scrh = 0; /* Screen Width & Height */
file_type ftype; /* File type */
char *afn = NULL; /* Filename */
int showcodes = 0; /* Show PTX formatting codes? */
int showcmds = 1; /* Show PTX Command lines? */
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

int findnthrow(unsigned long n, short int inccmds)
{
  unsigned long i;
  if (rowroot == NULL) return 0;
  for (rowptr = rowroot; rowptr->next != NULL; rowptr = rowptr->next)
  {
    if (inccmds || rowptr->iscmdline == 0)
    {
      if (i == n) break;
      i++;
    }
  }
  if (i != n) return 0;
  return 1;
}

int istabrow(arow *row)
{
  if (row->iscmdline && (row->rawtext[1]=='-' || (row->rawtext[2] == '-' && row->rawlen>2) || (row->rawtext[3]=='-' && row->rawlen>3)))
  {
    return 1;
  }
  return 0;
}

unsigned long findlasttabrow(unsigned long beforen)
{
  unsigned long i,trow=0;
  if (rowroot == NULL) return 0;
  for (rowptr = rowroot; rowptr->next != NULL; rowptr = rowptr->next)
  {
    if (istabrow(rowptr))
    /*if (rowptr->iscmdline && (rowptr->rawtext[1]=='-' || (rowptr->rawtext[2] == '-' && rowptr->rawlen>2) || (rowptr->rawtext[3]=='-' && rowptr->rawlen>3)))*/
    {
      trow = i;
    }
    if (i == beforen) break;
    i++;
  }
  if (i != beforen) return 0;
  return trow;
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
  if (!findnthrow(n,1)) return 0;
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
    if (!findnthrow(prow,1))
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
  if (!findnthrow(atrow,1)) return 0;
  if (atrow == 0)
  {
    delrow = rowroot;
    rowroot = rowroot->next;
    freerow(delrow);
  }
  else
  {
    if (!findnthrow(prow,1)) return 0;
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
    if (!findnthrow(rownum - 1,1))
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
    if ((!findnthrow(rownum-1,1)) || (!formatfromn(rownum,rowptr->formatend))) return 2;
  }
  return 1;
}

unsigned long edoffsettorawoffset(arow *row, unsigned long edx, 
                                  unsigned long rownum)
{
  unsigned long i, j=0, k, res = edx, ltr = 0;
  short int in5 = 0;
  if (edx >= row->edlen) edx = edlen - 1;
  switch (ftype)
  {
    case txtnix:
    case txtdos:
    case txtmac:
      for (i=0;i < row->rawlen;i++)
      {
        if (j >= edx) break;
        if (row->rawtext[i] == '\t')
        {
          j++;
          while (j % DEFAULTTABSPACE != 0) j++;
        }
        else j++;
      }
      res = j;
    break;
    
    case ptxnc:
    case ptx5dos:
    case ptx6dos:
      ltr = findlasttabrow(rownum);
      for (i=0;i < row->rawlen;i++)
      {
        if (j >= edx) break;
        if (in5)
        {
          if (row->rawtext[i] >= 0x80 && row->rawtext[i] <= 0x89)
          {
            if (showcodes) j++;
          }
          else if (row->rawtext[i] == 0x8C || row->rawtext[i] == 0x8D || row->rawtext[i] == 0x93)
          {
            if (showcodes) j++;
          }
          else if (row->rawtext[i] == 0x8E || row->rawtext[i] == 0x91 || row->rawtext[i] == 0x92)
          {
            j++;
          }
          else if (row->rawtext[i] >=0xE0 && row->rawtext[i] <= 0xFA)
          {
            if (showcodes) j++;
          }
          in5 = 0;
        }
        else if (row->rawtext[i] == '\t')
        {
          j++;
          if (findnthrow(ltr,1) && istabrow(rowptr))
          {
            for (k=j;rowptr->rawtext[k]!='!';k++)
            {
              if (rowptr->rawtext=='R') break;
            }
            if (rowptr->rawtext[k] == '!') j = k; //This won't work for wrapping
            else
            {
              while (j % DEFAULTTABSPACE != 0) j++;
            }
          }
          else
          {
            while (j % DEFAULTTABSPACE != 0) j++;
          }
        }
        else if (row->rawlen[i] == 5)
        {
          in5 = 1;
        }
        else j++;
      }
      res = j;
      
    break;
    
    case ptxcpc:
      //Does the CPC have tab rows?
    break;
  }
  return res;
}

unsigned long realrownum(unsigned long rownum)
{
  arow *a = NULL;
  unsigned long n = 0;
  if (showcmds) return rownum;
  if (findnthrow(rownum,0)) return 0;
  a = rowptr;
  if (a == NULL) return 0;
  for (rowptr=rowroot;rowptr!=NULL;rowptr=rowptr->next)
  {
    if (rowptr == a) break;
    n++;
  }
  if (rowptr == NULL) return 0;
  return n;
}

int doinsertstr(char *astr, unsigned long formattedlen)
{
  /* Formattedlen = Length of astr when formatted */
  unsigned long slen = strlen(astr);
  arow *therow;
  if (slen == 0) return 0;
  unsigned long fx, fy, fyr;
  fx = scrx+cx;
  fy = scry+cy;
  while (!findnthrow(fy,showcmds))
  {
    if (!insertrow(ULONG_MAX,"")) return 0; /* Always add to the end */
  }
  fyr = realrownum(fy);
  therow = rowptr;
  fx = edoffsettorawoffset(therow,fx,fyr);
  if (!insertstrinrow(therow,fx,fyr,astr)) return 0;
  if (cx==scrw-formattedlen) scrx +=formattedlen;
  else cx += formattedlen;
  uschanges++;
  return 1;
}

int donl()
{
  arow *therow;
  unsigned long fx, fy, fyr, rfx;
  unsigned int pformat;
  fx = scrx+cx;
  fy = scry+cy;
  if (!findnthrow(fy,showcmds))
  {
    if (!insertrow(ULONG_MAX,"")) return 0; /* Always add to the end */
  }
  else
  {
    fyr = realrownum(fy);
    therow = rowptr;
    if (fx>therow->edlen)
    {
      fx = therow->edlen;
      if (!insertrow(fyr+1,"")) return 0; /* Add row after current */
    }
    else if (fx == 0)
    {
      if (!insertrow(fyr,"")) return 0; /* Add row before current */
    }
    else
    {
      /* Split current */
      rfx = edoffsettorawoffset(therow, fx, fyr);
      if (insertrow(fyr+1,therow->rawtext+(rfx*sizeof(char)))<1) return 0;
      therow->rawtext[rfx] = 0;
      therow->rawlen = rfx;
    }
    if (findnthrow(fyr-1,1))
    {
      pformat = rowptr->formatend;
    } else pformat = 0;
    formatfromn(fyr,pformat);
  }
  /* Fix cursor */
  if (cy >= scrh-1)
  {
    scrx++;
  }
  else
  {
    cy++;
  }
  cx = 0;
  scrx = 0;
  uschanges++;
  return 1;
}

int dodelchar(short int isdelright) /* isdelright=0 for BS, 1 for DEL */
{
  /* Returns: 0=OoM, 1=OK, 2=No Action required, 3=Worked except formatting, 
             -1=Unknown Error */
  arow *therow = NULL, *prevrow, *nextrow;
  unsigned long fx, fy, fyr, rfx, pfy, pfyr, nufx, nfy, nfyr;
  unsigned int pformat;
  int rcode;
  fx = scrx+cx;
  fy = scry+cy;
  if (!findnthrow(fy,showcmds)) return 2;
  therow = rowptr;
  fyr = realrownum(fy);
  if (fx == 0 && fy == 0 && isdelright == 0) return 2;
  if (fx == 0 && isdelright == 0)
  {
    pfy = fy-1;
    pfyr = realrownum(pfy);
    if (!findnthrow(pfy,showcmds)) return -1;
    prevrow = rowptr;
    nufx = prevrow->edlen
    rcode = insertstrinrow(prevrow,prevrow->rawlen+1,pfyr,therow->rawtext);
    if (rcode == 0) return -1; /* Or should this code be 0? */
    rcode = delrow(fyr);
    if (rcode != 1) return -1; /* Maybe if it's 0 it should be returning 0? */
    therow = NULL;
    if (cy == 0) scry--;
    else cy--;
    cx = nufx;
    while (cx>scrw)
    {
      scrx += scrw;
      cx -= scrw;
    }
  }
  else if (fx >= therow->edlen && isdelright != 0)
  {
    nfy = fy+1;
    nfyr = realrownum(nfy);
    if (findnthrow(nfy,showcmds))
    {
      nextrow = rowptr;
      rcode = insertstrinrow(therow,therow->rawlen+1,fyr,nextrow->rawtext);
      if (rcode == 0) return -1; /* Or should this code be 0? */
      rcode = delrow(nfyr);
      if (rcode != 1) return -1; /* Maybe if it's 0 it should be returning 0? */
      nextrow = NULL;
    }
  }
  else if (isdelright == 0)
  {
    rcode = delstrinrow(therow,1,fx-1,fyr);
    if (rcode == 0) return -1; /* Or should this code be 0? */
    if (cx == 0 && scrx > 0) scrx--;
    else cx--;
  }
  else
  {
    rcode = delstrinrow(therow,1,fx,fyr);
    if (rcode == 0) return -1; /* Or should this code be 0? */
  }
  rcode = 1;
  if (therow != NULL)
  {
    if (findnthrow(fy-1,showcmds))
    {
      /* Do some sort of update of the row formatting at this point! */
      prevrow = rowptr;
      rcode = formatfromn(fyr,prevrow->formatend);
    }
    else rcode = 0;
  }
  else
  {
    if (findnthrow(pfy-1,showcmds))
    {
      /* Do some sort of update of the row formatting at this point! */
      therow = rowptr;
      rcode = formatfromn(pfyr,therow->formatend);
    }
    else rcode = formatfromn(pfyr,0);
  }
  uschanges++;
  if (rcode != 0) return 1;
  else return 3;
}
