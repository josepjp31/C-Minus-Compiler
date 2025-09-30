/****************************************************/
/* File: cminus.l                                     */
/* Lex specification for C-MINUS                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

%{
#include "globals.h"
#include "util.h"
#include "scan.h"
/* lexeme of identifier or reserved word */
char tokenString[MAXTOKENLEN+1];
%}

digit       [0-9]
number      {digit}+
letter      [a-zA-Z]
identifier  {letter}+
newline     \n
whitespace  [ \t]+
%x COMMENT

%%

"if"            {return IF;}
"return"        {return RETURN;}
"else"          {return ELSE;}
"int"           {return INT;}
"while"         {return WHILE;}
"void"          {return VOID;}

"=="            { return EQ; }      /* Equality */
"!="            { return NE; }      /* Inequality */
"<="            { return LE; }      /* Less than or Equal */
">="            { return GE; }      /* Greater than or Equal */
"/*"            { BEGIN(COMMENT); } /* Start of Comment */


<COMMENT>"*/"    { BEGIN(INITIAL); }
<COMMENT>\n      { lineno++; }
<COMMENT>.       { /* Ignore any other character */ }
<COMMENT><<EOF>> { 
    fprintf(listing, "Error: End of file inside an unclosed comment.\n");
    BEGIN(INITIAL);
    return ERROR;
}

"="             { return ASSIGN; }  /* Assignment  */
"<"             { return LT; }      /* Less than */
">"             { return GT; }      /* Greater than */
"+"             { return PLUS; }
"-"             { return MINUS; }
"*"             { return TIMES; }
"/"             { return OVER; }    /* Division (not start of comment) */
"("             { return LPAREN; }
")"             { return RPAREN; }
"["             { return LBRACE; }  /* Left Bracket */
"]"             { return RBRACE; }  /* Right Bracket */
"{"             { return LCURLY; }  /* Left Curly Brace */
"}"             { return RCURLY; }  /* Right Curly Brace */
";"             { return SEMI; }
","             { return COMMA; }


{number}        {return NUM;}
{identifier}    {return ID;}
{newline}       {lineno++;}
{whitespace}    {/* skip whitespace */}
.               {return ERROR;}

%%

TokenType getToken(void)
{ static int firstTime = TRUE;
  TokenType currentToken;
  if (firstTime)
  { firstTime = FALSE;
    lineno++;
    yyin = source;
    yyout = listing;
  }
  currentToken = yylex();
  strncpy(tokenString,yytext,MAXTOKENLEN);
  if (TraceScan) {
    fprintf(listing,"\t%d: ",lineno);
    printToken(currentToken,tokenString);
  }
  return currentToken;
}

