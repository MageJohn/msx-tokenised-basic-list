/****************************************************************************\
 * list v1.0
 *
 * This programm will convert a MSX-BASIC file into an ascii file.
 * syntax: list [inputfile] [--help]
 * If no inputfile was specified, list will automaticly uses the stdin.
 *
 * Original code: Vincent van Dam
 *
 * Modifications: Yuri Pieters
\****************************************************************************/

/* Include files */
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Macros */
#define print_error(...)                                                       \
  {                                                                            \
    fprintf(stderr, "%s: ", prog_name);                                        \
    fprintf(stderr, __VA_ARGS__);                                              \
  }

/* Constants */
enum {
  TOKENS_1_LEN = 127,
  TOKENS_2_LEN = 64,
};

/* the BASIC tokens */

// One byte tokens
char *tokens_1[TOKENS_1_LEN] = {
    NULL,      "END",    "FOR",    "NEXT",    /* 0x80-0x83 */
    "DATA",    "INPUT",  "DIM",    "READ",    /* 0x84-0x87 */
    "LET",     "GOTO",   "RUN",    "IF",      /* 0x88-0x8B */
    "RESTORE", "GOSUB",  "RETURN", "REM",     /* 0x8C-0x8F */
    "STOP",    "PRINT",  "CLEAR",  "LIST",    /* 0x90-0x93 */
    "NEW",     "ON",     "WAIT",   "DEF",     /* 0x94-0x97 */
    "POKE",    "CONT",   "CSAVE",  "CLOAD",   /* 0x98-0x9B */
    "OUT",     "LPRINT", "LLIST",  "CLS",     /* 0x9C-0x9F */
    "WIDTH",   "ELSE",   "TRON",   "TROFF",   /* 0xA0-0xA3 */
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
    "USING",   "INSRT",  "'",      "VARPTR",  /* 0xE4-0xE7 */
    "CSRLIN",  "ATTR$",  "DSKI$",  "OFF",     /* 0xE8-0xEB */
    "INKEY$",  "POINT",  ">",      "=",       /* 0xEC-0xEF */
    "<",       "+",      "-",      "*",       /* 0xF0-0xF3 */
    "/",       "^",      "AND",    "OR",      /* 0xF4-0xF7 */
    "XOR",     "EQV",    "IMP",    "MOD",     /* 0xF8-0xFB */
    "\\",      NULL,     NULL                 /* 0xFC-0xFE */
};

// Two byte tokens prefixed with 0xFF
char *tokens_2[TOKENS_2_LEN] = {
    NULL,    "LEFT$",  "RIGHT$", "MID$",  /* 0x80-0x83 */
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
    "MKD$",  NULL,     NULL,     NULL,    /* 0xB0-0xB3 */
    NULL,    NULL,     NULL,     NULL,    /* 0xB4-0xB7 */
    NULL,    NULL,     NULL,     NULL,    /* 0xB8-0xBB */
    NULL,    NULL,     NULL,     NULL,    /* 0xBC-0xBF */
};

// Global variable used for error printing
char *prog_name;

struct State {
  FILE *fd;
  uint8_t current;
  uint8_t previous;
  uint8_t buf[2];
  int32_t position;
};

void cleanup_and_exit(struct State *state, int status) {
  fclose(state->fd);
  exit(status);
}

/**
 * Read a byte from fd, and write it to byte.
 *
 * Returns false on success, and true on failure.
 */
bool get_byte(FILE *fd, uint8_t *byte) {
  int b = getc(fd);
  if (b == EOF) {
    if (feof(fd)) {
      // pad after EOF with nulls
      b = 0;
    } else {
      print_error("unexpected error reading from input\n");
      return true;
    }
  }
  // the integer downcast should succeed
  assert(b >= 0 && b <= UINT8_MAX);
  *byte = b;
  return false;
}

void init_state(struct State *state, FILE *fd) {
  if (fd == NULL) {
    print_error("panic: NULL pointer passed to init_state\n");
    exit(1);
  }
  *state = (struct State){
      .fd = fd,
      .position = -1,
  };

  if (get_byte(state->fd, &state->buf[0]) ||
      get_byte(state->fd, &state->buf[1]))
    cleanup_and_exit(state, 1);
}

/**
 * Get the position in the file of the current byte.
 */
uint16_t position(struct State *state) { return state->position; }

/**
 * Get the current character, which is the byte at state->position.
 *
 * Before advance_char or advance_word has been called for the first time, the
 * behaviour of this function is undefined.
 */
uint8_t current(struct State *state) { return state->current; }

/**
 * Get the current word, which is made from the bytes at state->position and
 * state->position-1.
 *
 * Before advance_char has been called twice or advance_word has been called for
 * the first time, the behaviour of this function is undefined.
 */
uint16_t current_word(struct State *state) {
  return (state->current << 8) | state->previous;
}

/**
 * Look at the next character, which is the byte at state->position+1
 */
uint8_t peek(struct State *state, uint8_t incr) {
  assert(incr == 1 || incr == 2);
  if (incr == 1) {
    return state->buf[0];
  } else {
    return state->buf[1];
  }
}

/** Advance state->position by 1 */
void advance(struct State *state) {
  state->previous = state->current;
  state->current = state->buf[0];
  state->buf[0] = state->buf[1];
  if (get_byte(state->fd, &state->buf[1])) {
    cleanup_and_exit(state, 1);
  }
  state->position++;
}

/** Advance state->position by 2 */
void advance_word(struct State *state) {
  state->previous = state->buf[0];
  state->current = state->buf[1];
  if (get_byte(state->fd, &state->buf[0]) ||
      get_byte(state->fd, &state->buf[1]))
    cleanup_and_exit(state, 1);
  state->position += 2;
}

/** Advance state->position by 1 and return current_char(state). */
uint8_t next(struct State *state) {
  advance(state);
  return current(state);
}

/* Advance state->position by 2 and return current_word(state). */
uint16_t next_word(struct State *state) {
  advance_word(state);
  return current_word(state);
}

/* get exponent of floating point number */
int8_t get_exp(uint8_t exp) {
  // The first bit is the sign.
  if (exp & 0x80) {
    // This shouldn't happen, because tokenised BASIC stores a negative float as
    // the "-" token (0xF2) followed by a positive float. But there's no point
    // in not supporting it.
    exp &= 0x7F;
    printf("-");
  }
  // The exp is a number in the range -63 <= exp <= 63, stored with 64 added.
  return exp - 64;
}

enum FloatPrec { PREC_SINGLE, PREC_DOUBLE };
enum { FLT_BUF_SIZE = 8 };

/* print binary-coded decimal floating point number like an MSX would */
void print_bcd_float(uint8_t num_bytes[static FLT_BUF_SIZE],
                     enum FloatPrec prec) {
  /*
   * Start by extracting the necessary data to print the number
   */

  // first byte is the sign and exponent
  int8_t exp = get_exp(num_bytes[0]);

  enum { DigitsLen = 14 };
  uint8_t digits[DigitsLen];

  for (uint8_t i = 1; i < FLT_BUF_SIZE; i++) {
    digits[(i - 1) * 2] = '0' + ((num_bytes[i] & 0xF0) >> 4);
    digits[(i - 1) * 2 + 1] = '0' + (num_bytes[i] & 0x0F);
  }

  uint8_t sig_figs = 0;
  // search from the end of the buffer for the last non-zero digit
  for (uint8_t i = DigitsLen - 1; i >= 0; i--) {
    if (digits[i] != '0') {
      sig_figs = i + 1;
      break;
    }
  }

  /*
   * Now we have all relevant info, format the float into a buffer.
   */

  // max size: 14 digits + decimal point + E(+|-)\d\d + null terminator
  enum { StringLen = DigitsLen + 1 + 4 + 1 };
  char string[StringLen] = {0};

  uint8_t si = 0; // string index
  uint8_t di = 0; // digits index

  if (-1 <= exp && exp <= DigitsLen) {
    // for exponents in this range, MSX prints the number without scientific
    // notation

    // digits before the decimal point
    for (; di < exp;)
      string[si++] = digits[di++];

    if (di < sig_figs) {
      string[si++] = '.';

      // zeros between decimal point and the first significant figure
      for (int8_t zeros = -exp; zeros > 0; zeros--)
        string[si++] = '0';

      // digits after the decimal point
      for (; di < sig_figs;)
        string[si++] = digits[di++];

      // if it's a double precision float, indicate it with '#'
      if (prec == PREC_DOUBLE)
        string[si++] = '#';
    } else {
      // when there's no decimal point, indicate that it's a double or single
      // precision float
      string[si] = prec == PREC_DOUBLE ? '#' : '!';
    }
  } else {
    // for exponents in this range, MSX prints the number in scientific notation

    // single digit before the decimal point
    string[si++] = digits[di++];

    if (di < sig_figs) {
      string[si++] = '.';

      // digits after the decimal point
      for (; di < sig_figs;)
        string[si++] = digits[di++];
    }

    // The float is stored is 0.123456E+63, but is printed as 1.23456E+62. So
    // print the exponent minus one.
    snprintf(&string[si], StringLen - si, "E%+d", exp - 1);
  }

  printf("%s", string);
}

/* The main routine */
int main(int argc, char *argv[]) {
  struct State state_struct;
  // allow all references to state to be uniform here and in other functions.
  struct State *state = &state_struct;

  // init the global state
  prog_name = argc > 0 ? argv[0] : "list";

  // Check argument syntax
  if (argc > 2) {
    print_error("syntax error\n"
                "Usage: %s [filename] [--help]\n",
                prog_name);
    exit(1);
  }

  if (argc == 2 &&
      (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
    printf("%s [filename] [--help]\n", prog_name);
    exit(0);
  }

  /* Open file or stdin */
  if (argc == 1)
    init_state(state, stdin);
  else {
    FILE *fd = fopen(argv[1], "r");
    if (fd == NULL) {
      print_error("cannot open file %s\n", argv[1]);
      exit(1);
    }
    init_state(state, fd);
  }

  /* Read header and check if it is a BASIC program */
  if (next(state) != 0xFF) {
    print_error("not an MSX-BASIC file\n");
    cleanup_and_exit(state, 1);
  }

  /* Process the input file */

  // the next word here is the address of the next line. Addresses are file
  // position + 0x8000 (on the MSX2);
  while (next_word(state) != 0) {
    // print the line number
    printf("%d ", next_word(state));

    /* read line of BASIC file */
    while ((next(state)) != 0) {
      if (current(state) == 0x3A) {
        // 0x3A is the ':' char, and is skipped in a few cases

        if (peek(state, 1) == 0xA1) {
          // the tuple of 0x3A 0xA1 is "ELSE".
          continue;
        } else if (peek(state, 1) == 0x8F && peek(state, 2) == 0xE6) {
          // the triplet of 0x3A 0x8F 0xE6 is handled specially as a single
          // quote ("'") comment
          advance(state);
          continue;
        }
      }

      if (current(state) >= 0x80 && current(state) < (TOKENS_1_LEN + 0x80)) {
        char *token = tokens_1[current(state) - 0x80];
        if (token != NULL)
          printf("%s", token);
        else {
          print_error("invalid token byte 0x%02X at position 0x%04X\n",
                      current(state), position(state));
        }
      } else if (current(state) == 0xFF) {
        advance(state);
        if (current(state) >= 0x80 && current(state) < (TOKENS_2_LEN + 0x80)) {
          char *token = tokens_2[current(state) - 0x80];
          if (token != NULL)
            printf("%s", token);
          else {
            print_error(
                "invalid token byte pair 0xFF 0x%02X at position 0x%04X\n",
                current(state), position(state));
          }
        } else {
          print_error("invalid byte 0x%02X following 0xFF at position 0x%04X\n",
                      current(state), position(state));
        }
      } else if (current(state) == 0x0B) /* octal number */
        printf("&O%o", next_word(state));
      else if (current(state) == 0x0C) /* hexadecimal number */
        printf("&H%x", next_word(state));
      else if (current(state) == 0x0D) {
        /* line number: absolute address. this form should only
         * ever exist in MSX RAM after the RUN command */
        advance_word(state);
        printf("{not yet implementated line reference}");
      } else if (current(state) == 0x0E) /* line number */
        printf("%d", next_word(state));
      else if (current(state) == 0x0F) /* integer from 10 to 255 */
        printf("%d", next(state));
      else if (current(state) == 0x1C) /* integer from 256 to 32767 */
        printf("%d", next_word(state));
      else if (current(state) == 0x1D) { /* single precision */
        uint8_t num_buf[FLT_BUF_SIZE] = {0};
        for (uint8_t i = 0; i < 4; i++) {
          num_buf[i] = next(state);
        }
        print_bcd_float(num_buf, PREC_SINGLE);
      } else if (current(state) == 0x1F) { /* double precision */
        uint8_t num_buf[FLT_BUF_SIZE];
        for (uint8_t i = 0; i < 8; i++) {
          num_buf[i] = next(state);
        }
        print_bcd_float(num_buf, PREC_DOUBLE);
      } else if (current(state) >= 0x11 && current(state) <= 0x1A)
        printf("%d", current(state) - 0x11);
      else if (isprint(current(state)))
        printf("%c", current(state));
      else {
        print_error(
            "non-printable byte 0x%02X encountered at position 0x%04X\n",
            current(state), position(state));
      }
    }
    printf("\n");
  }

  cleanup_and_exit(state, 0);
}
