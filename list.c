/****************************************************************************\
 list v0.0

 This programm will convert a MSX-BASIC file into an ascii file.
 syntax: list [inputfile] [--help]
 If no inputfile was specified, list will automaticly uses the stdin.

 Vincent van Dam
 (vandam@ronix.ptf.hro.nl)

 Please notify me in case of modifications.
 Last modified: 26-6-97
\****************************************************************************/

/* Include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* the BASIC tokens */

// One byte tokens
char *tokens_1[128] = {
    "",        "END",    "FOR",    "NEXT",    /* 0x80-0x83 */
    "DATA",    "INPUT",  "DIM",    "READ",    /* 0x84-0x87 */
    "LET",     "GOTO",   "RUN",    "IF",      /* 0x88-0x8B */
    "RESTORE", "GOSUB",  "RETURN", "REM",     /* 0x8C-0x8F */
    "STOP",    "PRINT",  "CLEAR",  "LIST",    /* 0x90-0x93 */
    "NEW",     "ON",     "WAIT",   "DEF",     /* 0x94-0x97 */
    "POKE",    "CONT",   "CSAVE",  "CLOAD",   /* 0x98-0x9B */
    "OUT",     "LPRINT", "LLIST",  "CLS",     /* 0x9C-0x9F */
    "WIDTH",   "",       "TRON",   "TROFF",   /* 0xA0-0xA3 */
    "SWAP",    "ERASE",  "ERROR",  "RESUME",  /* 0xA4-0xA7 */
    "DELETE",  "AUTO",   "RENUM",  "DEFSTR",  /* 0xA8-0xAB */
    "DEFINT",  "DEFSNG", "DEFDBL", "LINE",    /* 0xAC-0xAF */
    "OPEN",    "FIELD",  "GET",    "PUT",     /* 0xB0-0xB3 */
    "CLOSE",   "LOAD",   "MERGE",  "FILES",   /* 0xB4-0xB7 */
    "LSET",    "RSET",   "SAVE",   "LFILES",  /* 0xB8-0xBB */
    "CIRCLE",  "COLOR",  "DRAW",   "PAINT",   /* 0xBC-0xBF */
    "BEEP",    "PLAY",   "PSET",   "PRESET",  /* 0xC0-0xC3 */
    "SOUND",   "SCREEN", "VPOKE",  "SPRITE",  /* 0xC4-0xC7 */
    "VDP",     "BASE",   "CALL",   "TIME",    /* 0xC8-0xCB */
    "KEY",     "MAX",    "MOTOR",  "BLOAD",   /* 0xCC-0xCF */
    "BSAVE",   "DSKO$",  "SET",    "NAME",    /* 0xD0-0xD3 */
    "KILL",    "IPL",    "COPY",   "CMD",     /* 0xD4-0xD7 */
    "LOCATE",  "TO",     "THEN",   "TABC",    /* 0xD8-0xDB */
    "STEP",    "USR",    "FN",     "SPC(",    /* 0xDC-0xDF */
    "NOT",     "ERL",    "ERR",    "STRING$", /* 0xE0-0xE3 */
    "USING",   "INSRT",  "",       "VARPTR",  /* 0xE4-0xE7 */
    "CSRLIN",  "ATTR$",  "DSKI$",  "OFF",     /* 0xE8-0xEB */
    "INKEY$",  "POINT",  ">",      "=",       /* 0xEC-0xEF */
    "<",       "+",      "-",      "*",       /* 0xF0-0xF3 */
    "/",       "^",      "AND",    "OR",      /* 0xF4-0xF7 */
    "XOR",     "EQV",    "IMP",    "MOD",     /* 0xF8-0xFB */
    "\\",      "",       "",       ""         /* 0xFC-0xFF */
};

// Two byte tokens prefixed with 0xFF
char *tokens_2[64] = {
    "",      "LEFT$",  "RIGHT$", "MID$",  /* 0x80-0x83 */
    "SGN",   "INT",    "ABS",    "SQR",   /* 0x84-0x87 */
    "RND",   "SIN",    "LOG",    "EXP",   /* 0x88-0x8B */
    "COS",   "TAN",    "ATN",    "FRE",   /* 0x8C-0x8F */
    "INP",   "POS",    "LEN",    "STR$",  /* 0x90-0x93 */
    "VAL",   "ASC",    "CHR$",   "PEEK",  /* 0x94-0x97 */
    "VPEEK", "SPACE$", "OCT$",   "HEX$",  /* 0x98-0x9B */
    "LPOS",  "BIN$",   "CINT",   "CSNG",  /* 0x9C-0x9F */
    "CDBL",  "FIX",    "STICK",  "STRIG", /* 0xA0-0xA3 */
    "PDL",   "PAD",    "DSKF",   "FPOS",  /* 0xA4-0xA7 */
    "CVI",   "CVS",    "CVD",    "EOF",   /* 0xA8-0xAB */
    "LOC",   "LOF",    "MKI$",   "MK$",   /* 0xAC-0xAF */
    "MKD$",  "",       "",       "",      /* 0xB0-0xB3 */
    "",      "",       "",       "",      /* 0xB4-0xB7 */
    "",      "",       "",       "",      /* 0xB8-0xBB */
    "",      "",       "",       "",      /* 0xBC-0xBF */
};

/* read a byte from a file */
int readword(FILE *fd) { return getc(fd) + 256 * getc(fd); }

/* get exponent of floating point number */
int getexp(FILE *fd) {
  short exp;
  exp = getc(fd);
  if (exp > 127) {
    exp = exp - 128;
    printf("-");
  }
  return exp > 63 ? exp - 64 : -exp;
}

/* print single precission floating point */
void printprecission(FILE *fd, int digits) {
  char lo, hi, string[20];
  int exp, i, j, flag;
  exp = getexp(fd);

  /* get digits */
  for (i = j = 0; i < digits * 2; i++) {
    if (i % 2 == 0) {
      hi = getc(fd);
      lo = hi & 0x0F;
      hi = hi >> 4;
      string[j++] = '0' + hi;
    } else
      string[j++] = '0' + lo;
  }
  string[j] = '\0';

  /* insert dot */
  if (exp > digits * 2) {
    for (i = (digits * 2 + 2); i > 1; i--)
      string[i] = string[i - 1];
    string[1] = '.';
    if (exp > 0)
      exp--;
    else
      exp++;
  } else {
    for (i = (digits * 2 + 2); i > exp; i--)
      string[i] = string[i - 1];
    string[exp] = '.';
  }

  /* delete extra zero's */
  for (i = flag = 0; i < digits * 2 + 2; i++)
    if (string[i] == '0')
      if (flag != 0)
        ;
      else
        flag = i;
    else if (string[i] == '\0')
      break;
    else
      flag = 0;
  if (flag != 0) {
    if (string[flag - 1] == '.')
      string[flag - 1] = '\0';
    else
      string[flag] = '\0';
  }

  printf("%s", string);
  if (exp > digits * 2)
    printf(" E%d", exp);
}

/* The main routine */
int main(int argc, char *argv[]) { /* declarations */
  FILE *fd;
  short character;
  short forwarded;
  int linenumber;
  int goon;

  /* Check argument syntax */
  if (argc > 2) {
    fprintf(stderr,
            "%s: syntax error\n"
            "Usage: %s [filename] [--help]\n",
            argv[0], argv[0]);
    exit(1);
  }
  if (argc == 2 && !strcmp(argv[1], "--help")) {
    printf("%s [filename] [--help]\n", argv[0]);
    exit(1);
  }

  /* Open file or stdin */
  if (argc == 1)
    fd = stdin;
  else if ((fd = fopen(argv[1], "r")) == NULL) {
    fprintf(stderr, "%s: %s: cannot open file\n", argv[0], argv[1]);
    exit(1);
  }

  /* Read header and check if it is a BASIC programm */
  character = getc(fd);
  if (character != 0xff) {
    printf("%x\n", character);
    if (argc == 2)
      fprintf(stderr, "%s: %s: not an MSX-BASIC file\n", argv[0], argv[1]);
    else
      fprintf(stderr, "%s: not an MSX-BASIC file\n", argv[0]);
    exit(1);
  }

  /* Process the input file */
  /* read line number, and address were next line begins */
  while (readword(fd) != 0) {
    linenumber = readword(fd);
    printf("%d ", linenumber);
    /* read line of BASIC file */
    goon = 1;
    forwarded = 0;
    while (goon) {
      if (forwarded) {
        character = forwarded;
        forwarded = 0;
      } else
        character = getc(fd);
      if (character == 0)
        goon = 0;
      else if (character > 0x80 && character < 0xFF)
        printf("%s", tokens_1[character - 0x80]);
      else if (character == 0xFF)
        printf("%s", tokens_2[getc(fd) - 0x80]);
      else if (character == 0x3A) {
        character = getc(fd);
        if (character == 0xA1)
          printf("ELSE");
        else if (character == 0x8F) {
          character = getc(fd);
          if (character == 0xE6) {
            printf("'");
          } else {
            printf(":REM");
            forwarded = character;
          }
        } else {
          printf(":");
          forwarded = character;
        }
      } else if (character == 0x0B) /* octal number */
        printf("&O%o", readword(fd));
      else if (character == 0x0C) /* hexadecimal number */
        printf("&H%x", readword(fd));
      else if (character == 0x0D) /* line number: address */
        printf("{not yet implementated line reference}");
      else if (character == 0x0E) /* line number */
        printf("%d", readword(fd));
      else if (character == 0x0F) /* integer */
        printf("%d", getc(fd));
      else if (character == 0x1C) /* integer */
        printf("%d", readword(fd));
      else if (character == 0x1D) /* single precission */
        printprecission(fd, 3);
      else if (character == 0x1F) { /* double precission */
        printprecission(fd, 7);
        printf("#");
      } else if (character >= 0x11 && character <= 0x1A)
        printf("%d", character - 0x11);
      else
        printf("%c", character);
    }
    printf("\n");
  }

  /* Close file or stdin */
  fclose(fd);
}
