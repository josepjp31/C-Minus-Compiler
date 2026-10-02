/****************************************************/
/* File: cminus.y                                   */
/* The C-MINUS Yacc/Bison specification file        */
/* (Adapted from TINY)                              */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/
%{
#define YYPARSER /* distinguishes Yacc output from other code files */

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

#define YYSTYPE TreeNode *
static TreeNode * savedTree; /* stores syntax tree for later return */
static int yylex(void);

%}

/* C-MINUS Tokens */
%token IF ELSE WHILE RETURN INT VOID
%token ID NUM
%token ASSIGN EQ NE LT LE GT GE
%token PLUS MINUS TIMES OVER
%token LPAREN RPAREN LBRACE RBRACE LCURLY RCURLY
%token SEMI COMMA
%token ERROR 

/* Precedence and Associativity */
/* Lowest precedence */
%right ASSIGN
%left EQ NE
%left LT LE GT GE
%left PLUS MINUS
%left TIMES OVER
/* Highest precedence */

%% /* Grammar for C-MINUS */

/* 1. program */
program     : declaration_list
                 { savedTree = $1; } 
            ;

/* 2. declaration-list */
declaration_list : declaration_list declaration
                 { YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $2;
                     $$ = $1; }
                   else $$ = $2;
                 }
            | declaration
                 { $$ = $1; }
            ;

/* 3. declaration */
declaration : var_declaration
                 { $$ = $1; }
            | fun_declaration
                 { $$ = $1; }
            ;

/* 5. type-specifier */
type_specifier : INT
                 { $$ = (YYSTYPE)malloc(sizeof(struct treeNode)); /* Temp node */
                   $$->type = Integer;
                 }
            | VOID
                 { $$ = (YYSTYPE)malloc(sizeof(struct treeNode)); /* Temp node */
                   $$->type = Void;
                 }
            ;

/* 4 & 6. var-declaration */
var_declaration : type_specifier identifier SEMI
            { YYSTYPE t = newDeclNode(VarK);
              t->attr.name = $2->attr.name; /* Get from temp node */
              t->lineno = $2->lineno;      /* Get from temp node */
              t->type = $1->type;
              free($1); /* Free temp type node */
              free($2); /* Free temp identifier node */
              $$ = t;
            }
          | type_specifier identifier LBRACE constant RBRACE SEMI
            { YYSTYPE t = newDeclNode(VarK);
              t->attr.name = $2->attr.name; /* Get from temp node */
              t->lineno = $2->lineno;      /* Get from temp node */
              t->type = $1->type;
              t->isArray = TRUE;
              t->child[0] = $4; /* Assign ConstK node */
              free($1); /* Free temp type node */
              free($2); /* Free temp identifier node */
              $$ = t;
            }
          ;

/* 6. fun-declaration */
fun_declaration : type_specifier identifier LPAREN params RPAREN compound_stmt
            { YYSTYPE t = newDeclNode(FunK);
              t->attr.name = $2->attr.name; /* Get from temp node */
              t->lineno = $2->lineno;      /* Get from temp node */
              t->type = $1->type;
              t->child[0] = $4; /* params */
              t->child[1] = $6; /* compound_stmt */
              free($1); /* Free temp type node */
              free($2); /* Free temp identifier node */
              $$ = t;
            }
          ;

/* 7. params */
params : param_list
            { $$ = $1; }
       | VOID
            { $$ = newDeclNode(ParamK);
              $$->attr.name = NULL; /* Indicates void parameter */
              $$->type = Void;
            }
       | /* empty */
            { $$ = newDeclNode(ParamK);
              $$->attr.name = NULL; /* Indicates void parameter */
              $$->type = Void;
            }
       ;

/* 8. param-list */
param_list : param_list COMMA param
            { YYSTYPE t = $1;
              while (t->sibling != NULL)
                t = t->sibling;
              t->sibling = $3;
              $$ = $1;
            }
          | param
            { $$ = $1; }
          ;

/* 9. param */
param : type_specifier identifier
            { $$ = newDeclNode(ParamK);
              $$->attr.name = $2->attr.name; /* Get from temp node */
              $$->lineno = $2->lineno;      /* Get from temp node */
              $$->type = $1->type;
              free($1); /* Free temp type node */
              free($2); /* Free temp identifier node */
            }
      | type_specifier identifier LBRACE RBRACE
            { $$ = newDeclNode(ParamK);
              $$->attr.name = $2->attr.name; /* Get from temp node */
              $$->lineno = $2->lineno;      /* Get from temp node */
              $$->type = $1->type;
              $$->isArray = TRUE;
              free($1); /* Free temp type node */
              free($2); /* Free temp identifier node */
            }
      ;

/* 10. compound-stmt */
compound_stmt : LCURLY local_declarations statement_list RCURLY
            { $$ = newStmtNode(CompoundK);
              $$->child[0] = $2; /* local_declarations */
              $$->child[1] = $3; /* statement_list */
              $$->lineno = lineno; /* Use lineno of RCURLY */
            }
            ;

/* 11. local-declarations */
local_declarations : local_declarations var_declaration
            { YYSTYPE t = $1;
              if (t != NULL)
              { while (t->sibling != NULL)
                  t = t->sibling;
                t->sibling = $2;
                $$ = $1;
              }
              else $$ = $2;
            }
            | /* empty */
            { $$ = NULL; }
            ;

/* 12. statement-list */
statement_list : statement_list statement
            { YYSTYPE t = $1;
              if (t != NULL)
              { while (t->sibling != NULL)
                  t = t->sibling;
                t->sibling = $2;
                $$ = $1;
              }
              else $$ = $2;
            }
            | /* empty */
            { $$ = NULL; }
            ;

/* 13. statement */
/* MODIFIED RULE for unambiguous grammar */
statement : matched_stmt { $$ = $1; }
          | unmatched_stmt { $$ = $1; }
          ;

/* 14. expression-stmt */
expression_stmt : expression SEMI
            { $$ = $1; }
                | SEMI
            { $$ = NULL; }
            ;

/* 15. selection-stmt & 16. iteration_stmt (NEW RULES COMBINED) */

/* A 'matched_stmt' is an if-else, a while that contains a matched_stmt,
   or a simple statement (other_stmt). */
matched_stmt : IF LPAREN expression RPAREN matched_stmt ELSE matched_stmt
            { $$ = newStmtNode(IfK);
              $$->lineno = $3->lineno;
              $$->child[0] = $3; /* condition */
              $$->child[1] = $5; /* then-part */
              $$->child[2] = $7; /* else-part */
            }
             | WHILE LPAREN expression RPAREN matched_stmt
            { $$ = newStmtNode(WhileK);
              $$->lineno = $3->lineno;
              $$->child[0] = $3; /* condition */
              $$->child[1] = $5; /* body */
            }
             | other_stmt { $$ = $1; }
             ;

/* An 'unmatched_stmt' is an if without else, or a while that contains an unmatched_stmt. */
unmatched_stmt : IF LPAREN expression RPAREN statement
            { $$ = newStmtNode(IfK);
              $$->lineno = $3->lineno;
              $$->child[0] = $3; /* condition */
              $$->child[1] = $5; /* then-part */
            }
               | IF LPAREN expression RPAREN matched_stmt ELSE unmatched_stmt
            { $$ = newStmtNode(IfK);
              $$->lineno = $3->lineno;
              $$->child[0] = $3; /* condition */
              $$->child[1] = $5; /* then-part */
              $$->child[2] = $7; /* else-part */
            }
               | WHILE LPAREN expression RPAREN unmatched_stmt
            { $$ = newStmtNode(WhileK);
              $$->lineno = $3->lineno;
              $$->child[0] = $3; /* condition */
              $$->child[1] = $5; /* body */
            }
            ;

/* NEW RULE: Simple statements that cannot be 'unmatched' */
other_stmt : expression_stmt { $$ = $1; }
           | compound_stmt   { $$ = $1; }
           | return_stmt     { $$ = $1; }
           ;

/* 17. return-stmt */
return_stmt : RETURN SEMI
            { $$ = newStmtNode(ReturnK);
              $$->lineno = lineno; 
            }
            | RETURN expression SEMI
            { $$ = newStmtNode(ReturnK);
              $$->lineno = $2->lineno;
              $$->child[0] = $2; /* return value */
            }
            ;

/* 18. expression */
expression : var ASSIGN expression
            { $$ = newExpNode(AssignK);
              $$->lineno = $1->lineno;
              $$->child[0] = $1; /* var */
              $$->child[1] = $3; /* expression */
            }
           | simple_expression
            { $$ = $1; }
           ;

/* 19. var */
var : identifier
            { $$ = $1; } /* Pass the IdK node up */
    | identifier LBRACE expression RBRACE
            { $$ = $1; /* Pass the IdK node */
              $$->child[0] = $3; /* Attach index expression */
            }
    ;

/* 20. simple-expression */
simple_expression : additive_expression relop additive_expression
            { $$ = newExpNode(OpK);
              $$->lineno = $1->lineno;
              $$->attr.op = $2->attr.op; /* relop */
              $$->child[0] = $1;
              $$->child[1] = $3;
              free($2); /* Free temp node */
            }
                  | additive_expression
            { $$ = $1; }
            ;

/* 21. relop */
relop : LE { $$ = newExpNode(OpK); $$->attr.op = LE; $$->lineno = lineno; }
      | LT { $$ = newExpNode(OpK); $$->attr.op = LT; $$->lineno = lineno; }
      | GT { $$ = newExpNode(OpK); $$->attr.op = GT; $$->lineno = lineno; }
      | GE { $$ = newExpNode(OpK); $$->attr.op = GE; $$->lineno = lineno; }
      | EQ { $$ = newExpNode(OpK); $$->attr.op = EQ; $$->lineno = lineno; }
      | NE { $$ = newExpNode(OpK); $$->attr.op = NE; $$->lineno = lineno; }
      ;

/* 22. additive-expression */
additive_expression : additive_expression addop term
            { $$ = newExpNode(OpK);
              $$->lineno = $1->lineno;
              $$->attr.op = $2->attr.op; /* addop */
              $$->child[0] = $1;
              $$->child[1] = $3;
              free($2); /* Free temp node */
            }
                    | term
            { $$ = $1; }
            ;

/* 23. addop */
addop : PLUS  { $$ = newExpNode(OpK); $$->attr.op = PLUS; $$->lineno = lineno; }
      | MINUS { $$ = newExpNode(OpK); $$->attr.op = MINUS; $$->lineno = lineno; }
      ;

/* 24. term */
term : term mulop factor
            { $$ = newExpNode(OpK);
              $$->lineno = $1->lineno;
              $$->attr.op = $2->attr.op; /* mulop */
              $$->child[0] = $1;
              $$->child[1] = $3;
              free($2); /* Free temp node */
            }
     | factor
            { $$ = $1; }
     ;

/* 25. mulop */
mulop : TIMES { $$ = newExpNode(OpK); $$->attr.op = TIMES; $$->lineno = lineno; }
      | OVER  { $$ = newExpNode(OpK); $$->attr.op = OVER; $$->lineno = lineno; }
      ;

/* 26. factor */
factor : LPAREN expression RPAREN
            { $$ = $2; }
       | var
            { $$ = $1; }
       | call
            { $$ = $1; }
       | constant
            { $$ = $1; } /* Pass the ConstK node up */
       ;

/* 27. call */
call : identifier LPAREN args RPAREN
            { $$ = newExpNode(CallK);
              $$->attr.name = $1->attr.name; /* Get from temp node */
              $$->lineno = $1->lineno;      /* Get from temp node */
              $$->child[0] = $3; /* args */
              free($1); /* Free temp identifier node */
            }
     ;

/* 28. args */
args : arg_list
            { $$ = $1; }
     | /* empty */
            { $$ = NULL; }
     ;

/* 29. arg-list */
arg_list : arg_list COMMA expression
            { YYSTYPE t = $1;
              while (t->sibling != NULL)
                t = t->sibling;
              t->sibling = $3;
              $$ = $1;
            }
         | expression
            { $$ = $1; }
         ;

/* ================================================== */
/* === NEW INTERMEDIATE RULES TO CAPTURE VALUES === */
/* ================================================== */

/* Rule to immediately capture the value of an ID */
identifier : ID
            { $$ = newExpNode(IdK); /* Create a temporary node */
              $$->attr.name = copyString(tokenString);
              $$->lineno = lineno;
            }
            ;

/* Rule to immediately capture the value of a NUM */
constant : NUM
            { $$ = newExpNode(ConstK); /* Create a temporary node */
              $$->attr.val = atoi(tokenString);
              $$->lineno = lineno;
            }
            ;

%%

int yyerror(char * message)
{ fprintf(listing,"Syntax error at line %d: %s\n",lineno,message);
  fprintf(listing,"Current token: ");
  printToken(yychar,tokenString);
  Error = TRUE;
  return 0;
}

/* yylex calls getToken to make Yacc/Bison output
 * compatible with ealier versions of the TINY scanner
 */
static int yylex(void)
{ return getToken();
}

TreeNode * parse(void)
{ yyparse();
  return savedTree;
}