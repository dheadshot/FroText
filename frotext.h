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

#define PTF_BOOKMARK       0x4000
#define PTF_SELECT         0x8000

typedef struct arow  /* This bit partly inspired by Kilo */
{
  int iscmdline  /* is the line a command line? */
  char *rawtext; /* Raw text from the file */
  long rawlen;   /* Length of the raw row in characters */
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
  ptxdos=11,
  ptxcpc=12,
  other=100
} file_type;


arow *newrow(char *rawtext);
int findlastrow();
int loadfromfile();
