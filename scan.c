/****************************************************/
/* File: scan.c                                     */
/* The scanner implementation for the TINY compiler */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "util.h"
#include "scan.h"

/* states in scanner DFA */
typedef enum
   { START,           /* Initial state */
     IN_NUM,          /* Reading a number */
     IN_ID,           /* Reading an identifier or reserved word */
     IN_DIV,          /* After reading '/', looking for '/*' or '/' */
     IN_COMMENT,      /* Inside a comment */
     IN_CMT_END,      /* Inside a comment, after a '*' */
     IN_EQ_OR_ASSIGN, /* After reading '=', looking for '==' */
     IN_NE,           /* After reading '!', looking for '!=' */
     IN_LT_OR_LE,     /* After reading '<', looking for '<=' */
     IN_GT_OR_GE,     /* After reading '>', looking for '>=' */
     DONE             /* Token recognized */
   } StateType;


/* lexeme of identifier or reserved word */
char tokenString[MAXTOKENLEN+1];

/* BUFLEN = length of the input buffer for
   source code lines */
#define BUFLEN 256

static char lineBuf[BUFLEN]; /* holds the current line */
static int linepos = 0; /* current position in LineBuf */
static int bufsize = 0; /* current size of buffer string */
static int EOF_flag = FALSE; /* corrects ungetNextChar behavior on EOF */

/* getNextChar fetches the next non-blank character
   from lineBuf, reading in a new line if lineBuf is
   exhausted */
static int getNextChar(void)
{ if (!(linepos < bufsize))
  { lineno++;
    if (fgets(lineBuf,BUFLEN-1,source))
    { if (EchoSource) fprintf(listing,"%4d: %s",lineno,lineBuf);
      bufsize = strlen(lineBuf);
      linepos = 0;
      return lineBuf[linepos++];
    }
    else
    { EOF_flag = TRUE;
      return EOF;
    }
  }
  else return lineBuf[linepos++];
}

/* ungetNextChar backtracks one character
   in lineBuf */
static void ungetNextChar(void)
{ if (!EOF_flag) linepos-- ;}

/* lookup table of reserved words */
static struct
    { char* str;
      TokenType tok;
    } reservedWords[MAXRESERVED] =
   {{"if",IF},{"else",ELSE},{"while",WHILE},
    {"return",RETURN},{"int",INT},{"void",VOID}};

/* lookup an identifier to see if it is a reserved word */
/* uses linear search */
static TokenType reservedLookup (char * s)
{ int i;
  for (i=0;i<MAXRESERVED;i++)
    if (!strcmp(s,reservedWords[i].str))
      return reservedWords[i].tok;
  return ID;
}

/****************************************/
/* the primary function of the scanner  */
/****************************************/
/* function getToken returns the 
 * next token in source file
 */
TokenType getToken(void)
{ /* index for storing into tokenString */
  int tokenStringIndex = 0; 
  /* holds current state */
   StateType state = START;
   /* holds current token to be returned */
   TokenType currentToken;
   /* flag to store token string */
   int save;

   while (state != DONE)
   {  int c = getNextChar();
      save = TRUE;
      switch (state)
      {  case START:
            if (isdigit(c))
               state = IN_NUM;
            else if (isalpha(c))
               state = IN_ID;
            else if ((c == ' ') || (c == '\t') || (c == '\n'))
               save = FALSE; 
            else switch (c)
            {  case EOF:
                  save = FALSE;
                  state = DONE;
                  currentToken = ENDFILE;
                  break;
               case '/':
                  state = IN_DIV; /* Look for '/' or '/*' */
                  break;
               case '=':
                  state = IN_EQ_OR_ASSIGN; /* Look for '=' or '==' */
                  break;
               case '!':
                  save = FALSE; /* Don't save '!' yet, only if followed by '=' */
                  state = IN_NE;
                  break;
               case '<':
                  state = IN_LT_OR_LE; /* Look for '<' or '<=' */
                  break;
               case '>':
                  state = IN_GT_OR_GE; /* Look for '>' or '>=' */
                  break;
               case '+':
                  state = DONE;
                  currentToken = PLUS;
                  break;
               case '-':
                  state = DONE;
                  currentToken = MINUS;
                  break;
               case '*':
                  state = DONE;
                  currentToken = TIMES;
                  break;
               case '(':
                  state = DONE;
                  currentToken = LPAREN;
                  break;
               case ')':
                  state = DONE;
                  currentToken = RPAREN;
                  break;
               case '[': /* New symbol: open bracket */
                  state = DONE;
                  currentToken = LBRACE;
                  break;
               case ']': /* New symbol: close bracket */
                  state = DONE;
                  currentToken = RBRACE;
                  break;
               case '{': /* New symbol: open curly brace */
                  state = DONE;
                  currentToken = LCURLY;
                  break;
               case '}': /* New symbol: close curly brace */
                  state = DONE;
                  currentToken = RCURLY;
                  break;
               case ';': /* New symbol: semicolon */
                  state = DONE;
                  currentToken = SEMI;
                  break;
               case ',': /* New symbol: comma */
                  state = DONE;
                  currentToken = COMMA;
                  break;
               default:
                  state = DONE;
                  currentToken = ERROR;
                  break;
            }
            break;

         
         case IN_NUM:
            if (!isdigit(c))
            {  
               ungetNextChar();
               save = FALSE;
               state = DONE;
               currentToken = NUM;
            }
            break;

        
         case IN_ID:
            if (!isalpha(c) && !isdigit(c))
            {  
               ungetNextChar();
               save = FALSE;
               state = DONE;
               currentToken = ID;
            }
            break;


         /* Division and comments */
         case IN_DIV:
            if (c == '*')
            {  
               /* Found comment: /* */
               tokenStringIndex--; /* <--- FIX: Undo saving of '/' */
               save = FALSE;       /* Don't save '*' */
               state = IN_COMMENT;
            }
            else
            {  
               /* It's the OVER token (Division) */
               ungetNextChar();
               save = FALSE;
               state = DONE;
               currentToken = OVER;
            }
            break;

         /* Inside a comment */
         case IN_COMMENT:
            save = FALSE; /* NEVER save */
            if (c == EOF)
            {  state = DONE;
               currentToken = ERROR; 
            }
            else if (c == '*')
               state = IN_CMT_END;
            break;

          /* Inside a comment, looking for the closing */
         case IN_CMT_END:
            save = FALSE; /* NEVER save */
            if (c == EOF)
            {  state = DONE;
               currentToken = ERROR; 
            }
            else if (c == '/')
               state = START; /* Comment closed, return to clean start */
            else if (c != '*')
               state = IN_COMMENT; /* False positive */
            break;

         
         case IN_EQ_OR_ASSIGN:
            state = DONE;
            if (c == '=')
               currentToken = EQ;   /* == */
            else
            {  /* It's a single '=' token (ASSIGN) */
               ungetNextChar();
               save = FALSE;
               currentToken = ASSIGN;
            }
            break;
            
         /* Handling '!' (NE) */
         case IN_NE:
            state = DONE;
            if (c == '=')
               currentToken = NE;   /* != */
            else
            {  /* '!' not followed by '=', ERROR token */
               ungetNextChar();
               save = FALSE;
               currentToken = ERROR;
            }
            break;

         /* Handling '<' (LT or LE) */
         case IN_LT_OR_LE:
            state = DONE;
            if (c == '=')
               currentToken = LE;   /* <= */
            else
            {  /* It's a single '<' token (LT) */
               ungetNextChar();
               save = FALSE;
               currentToken = LT;
            }
            break;

         /* Handling '>' (GT or GE) */
         case IN_GT_OR_GE:
            state = DONE;
            if (c == '=')
               currentToken = GE;   /* >= */
            else
            {  /* It's a single '>' token (GT) */
               ungetNextChar();
               save = FALSE;
               currentToken = GT;
            }
            break;

         case DONE:
         default: /* should never happen */
            fprintf(listing,"Scanner Bug: state= %d\n",state);
            state = DONE;
            currentToken = ERROR;
            break;
      }
     if ((save) && (tokenStringIndex <= MAXTOKENLEN))
       tokenString[tokenStringIndex++] = (char) c;
     if (state == DONE)
     { tokenString[tokenStringIndex] = '\0';
       if (currentToken == ID)
         currentToken = reservedLookup(tokenString);
     }
   }
   if (TraceScan) {
     fprintf(listing,"\t%d: ",lineno);
     printToken(currentToken,tokenString);
   }
   return currentToken;
} /* end getToken */

