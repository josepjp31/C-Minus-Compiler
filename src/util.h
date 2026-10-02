/****************************************************/
/* File: util.h                                     */
/* Utility functions for the TINY compiler          */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _UTIL_H_
#define _UTIL_H_

#include "globals.h"

/* Procedure printToken prints a token 
 * and its lexeme to the listing file
 */
void printToken( int token, const char* tokenString );

/* Function newStmtNode creates a new statement
 * node for syntax tree construction
 */

 /* Prototypes for the AST node-creation functions */
TreeNode * newDeclNode(DeclKind kind);
TreeNode * newStmtNode(StmtKind kind);
TreeNode * newExpNode(ExpKind kind);


/* Function newExpNode creates a new expression 
 * node for syntax tree construction
 */


/* Function copyString allocates and makes a new
 * copy of an existing string
 */
char * copyString( char * s );

/* procedure printTree prints a syntax tree to the 
 * listing file using indentation to indicate subtrees
 */
void printTree( TreeNode * tree );

#endif
