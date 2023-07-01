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
#include <string.h>
#include <stdio.h>

/* the BASIC tokens */
char *tokens_1[128]=
{
    "","END","FOR","NEXT","DATA","INPUT","DIM","READ","LET","GOTO","RUN","IF",
       "RESTORE","GOSUB","RETURN","REM",                           /* 80-8F */
    "STOP","PRINT","CLEAR","LIST","NEW","ON","WAIT","DEF","POKE","CONT",
       "CSAVE","CLOAD","OUT","LPRINT","LLIST","CLS",               /* 90-9F */
    "WIDTH","ELSE","TRON","TROFF","SWAP","ERASE","ERROR","RESUME","DELETE",
       "AUTO","RENUM","DEFSTR","DEFINT","DEFSNG","DEFDBL","LINE",  /* A0-AF */
    "OPEN","FIELD","GET","PUT","CLOSE","LOAD","MERGE","FILES","LSET","RSET",
       "SAVE","LFILES","CIRCLE","COLOR","DRAW","PAINT",            /* B0-BF */
    "BEEP","PLAY","PSET","PRESET","SOUND","SCREEN","VPOKE","SPRITE","VDP",
       "BASE","CALL","TIME","KEY","MAX","MOTOR","BLOAD",           /* C0-CF */
    "BSAVE","DSKO$","SET","NAME","KILL","IPL","COPY","CMD","LOCATE","TO",
       "THEN","TABC","STEP","USR","FN","SPCL",                     /* D0-DF */
    "NOT","ERL","ERR","STRING$","USING","INSRT","","VARPTR","CSRLIN","ATTR$",
       "DSKI$","OFF","INKEY$","POINT",">","=",                     /* E0-EF */
    "<","+","-","*","/","^","AND","OR","XOR","EQV","IMP","MOD","\\","","",
       "{escape-code}"                                             /* F0-FF */

};

char *tokens_2[128]=
{
    "","LEFT$","RIGHT$","MID$","SGN","INT","ABS","SQR","RND","SIN","LOG",
       "EXP","COS","TAN","ATN","FRE",                             /* 80-8F */
    "INP","POS","LEN","STR$","VAL","ASC","CHR$","PEEK","VPEEK","SPACE$",
       "OCT$","HEX$","LPOS","BIN$","CINT","CSNG",                 /* 90-9F */
    "CDBL","FIX","STICK","STRIG","PDL","PAD","DSKF","FPOS","CVI","CVS","CVD",
       "EOF","LOC","LOF","MKI$","MK$",                            /* A0-AF */
    "MKD$","","","","","","","","","","","","","","","",          /* B0-BF */
    "","","","","","","","","","","","","","","","",              /* C0-CF */
    "","","","","","","","","","","","","","","","",              /* D0-DF */
    "","","","","","","","","","","","","","","","",              /* E0-EF */
    "","","","","","","","","","","","","","","",""               /* F0-FF */
};


/* read a byte from a file */
int readword(FILE *fd) { return getc(fd)+256*getc(fd); }

/* print character */
int printchar(FILE *fd)
{	char character=getc(fd);
	if (character==0) return -1;
	if (character!=1) printf("%c",character);
	if (character==1) printf("{%d}",getc(fd));
	return 0;
}

/* get exponent of floating point number */
int getexp(FILE *fd)
{	short exp;
	exp=getc(fd);
	if (exp>127) { exp=exp-128; printf("-"); }
	return exp>63 ? exp-64 : -exp;
}

/* print single precission floating point */
void printprecission(FILE *fd, int digits)
{	char  lo,hi,string[20];
        int   exp,i,j,flag;
        exp=getexp(fd);	

        /* get digits */
	for (i=j=0; i<digits*2; i++) {
	    if (i%2==0) { 
	       hi=getc(fd); lo=hi&0x0f; hi=hi>>4;
               string[j++]='0'+hi;
               }
	     else
	       string[j++]='0'+lo;
	    }
	string[j]='\0';

	/* insert dot */
	if (exp>digits*2) {
	   for (i=(digits*2+2);i>1;i--) string[i]=string[i-1];
           string[1]='.';
           if (exp>0) exp--; else exp++; }
         else {
           for (i=(digits*2+2);i>exp;i--) string[i]=string[i-1];
           string[exp]='.'; }

        /* delete extra zero's */
        for (i=flag=0; i<digits*2+2; i++)
            if (string[i]=='0') 
               if (flag!=0) ; 
                else 
               flag=i;
             else
            if (string[i]=='\0') 
               break; 
                else 
               flag=0;
        if (flag!=0) if (string[flag-1]=='.') string[flag-1]='\0';
         else string[flag]='\0';
        
	printf("%s",string);
	if (exp>digits*2) printf(" E%d",exp);
}

/* The main routine */
void main(int argc, char *argv[])
{       /* declarations */
	FILE   *fd;
	short  character;
	short  forwarded;
	short  exp;
	int    linenumber;
	int    lineaddress;
	int    goon;

        /* Check argument syntax */
	if (argc>2) {
           fprintf(stderr,"%s: syntax error\n"
           "Usage: %s [filename] [--help]\n",argv[0], argv[0]);
           exit(1); }
        if (argc==2 && !strcmp(argv[1],"--help")) {
           printf("%s [filename] [--help]\n",argv[0]);
           exit(1); }
           
        /* Open file or stdin */
	if (argc==1) 
           fd=stdin; 
         else 
           if ((fd=fopen(argv[1],"r"))==NULL) {
              fprintf(stderr,"%s: %s: cannot open file\n",argv[0],argv[1]);
              exit(1); }
              
        /* Read header and check if it is a BASIC programm */
        character=getc(fd);
        if (character!=0xff) {
 	   printf("%x\n",character);
           if (argc==2) 
              fprintf(stderr,"%s: %s: not an MSX-BASIC file\n",argv[0],argv[1]);
            else
              fprintf(stderr,"%s: not an MSX-BASIC file\n",argv[0]);
          exit(1); }

        /* Process the input file */
        /* read line number, and address were next line begins */
        while (lineaddress=readword(fd)!=0) {
		linenumber=readword(fd);
		printf("%d ",linenumber);
                /* read line of BASIC file */
                goon=1; forwarded=0;
		while(goon) {
		     if (forwarded) {
		        character=forwarded;
		        forwarded=0; }
		      else
		        character=getc(fd);
		     if (character==0) goon=0;
		      else
		     if (character>0x80 && character<0xff)
		     	printf("%s",tokens_1[character-0x80]);
                      else
                     if (character==0xff)
                        printf("%s",tokens_2[getc(fd)-0x80]);
                      else  
                     if (character==0x3a) {
                        character=getc(fd);
                        if (character==0xa1) 
                           printf("ELSE");
                         else
                        if (character==0x8f) {
                           character=getc(fd);
                           if (character==0xe6) {
                              printf("'");
                              while (!printchar(fd));
                              break; }
                            else {
                              fprintf(stderr,"%s: %s: error in MSX-BASIC file\n",
                                             argv[0],argv[1]);
                              fclose(fd);
                              exit(1); }
                           }
                         else {
                           printf(":");
                           forwarded=character; }
                        }
                      else
                     if (character==0x0b)          /* octal number */
                        printf("&O%o",readword(fd));
                      else
                     if (character==0x0c)          /* hexadecimal number */ 
                        printf("&H%x",readword(fd));
                      else
                     if (character==0x0d)          /* line number: address */
                        printf("{not yet implementated line reference}");
                      else
                     if (character==0x0e)          /* line number */
                        printf("%d",readword(fd));
                      else
                     if (character==0x0f)          /* integer */
                        printf("%d",getc(fd));
                      else
                     if (character==0x1c)          /* integer */
                        printf("%d",readword(fd));
                      else
                     if (character==0x1d)          /* single precission */
                        printprecission(fd,3);
                      else
                     if (character==0x1f) {        /* double precission */
			printprecission(fd,7);
			printf("#"); }
                      else
                     if (character>=0x11 && character<=0x1a)
                        printf("%d",character-0x11);
                      else
                     printf("%c",character);
		}
                printf("\n");
        }
        
        /* Close file or stdin */
        fclose(fd);
}

