#ifndef _FROTEXT_H_
#define _FROTEXT_H_

#define PROGVER "0.01.00 ALPHA"

#define PTF_NORMAL         0x0000
#define PTF_BOLD           0x0001
#define PTF_ITALIC         0x0002
#define PTF_UNDERLINE      0x0004
#define PTF_ENLARGE        0x0008
#define PTF_CONDENSED      0x0010
#define PTF_ELITE          0x0020
#define PTF_PROPORTIONAL   0x0040
#define PTF_QUALITY        0x0080
#define PTF_SUBSCRIPT      0x0100
#define PTF_SUPERSCRIPT    0x0200
#define PTF_DOUBLESTRIKE   0x0400
#define PTF_NORMALPICA     0x0800
/* ^ Might be wrong */

#define PTF_BOOKMARK       0x4000
#define PTF_SELECT         0x8000

typedef struct arow  /* This bit partly inspired by Kilo */
{
  int iscmdline  /* is the line a command line? */
  char *rawtext; /* Raw text from the file */
  long rawlen;   /* Length of rawtext in characters */
  long rawsize;  /* Number of bytes allocated to rawtext */
  char *edtext;  /* Text for the editor */
  long edlen;    /* Length of the editor row in characters */
  unsigned int *formattext;  /* The format of the text (length edlen) */
  unsigned int formatend;    /* The formatting at the end of the line */
  struct arow *next;  /* Pointer to the next row */
} arow;

typedef enum file_type
{
  txtnix=0,
  txtdos=1,
  txtmac=2,
  ptxnc=10,
  ptx5dos=11,
  ptx6dos=12
  ptxcpc=13,
  other=100
} file_type;


#define DEFAULTTABSPACE 8


arow *newrow(char *rawtext);
int findlastrow();
int findnthrow(unsigned long n);
int loadfromfile();
int createedtext(arow *row, unsigned int pformat);
int formatfromn(unsigned long n, unsigned int pformat);
int insertrow(unsigned long atrow, char *rawtext);
void freerow(arow *row);
int delrow(unsigned long atrow);
/* Returns: 1=success, 0=failure, 2=possibly corrupted, but deletion worked */
int insertstrinrow(arow *row, unsigned long atchar, unsigned long rownum, 
                   char *astr);
/* Returns: 1=success, 0=failure, 2=astr was blank, 3=could not reformat */


#endif
