/****************************************************/
/* File: symtab.h                                   */
/* Symbol table interface for the C-MINUS    compiler     */
/* (allows only one symbol table)                   */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "globals.h"
/* SIZE of the hash table */
#define SIZE 211

/* ====================================================
   DATA STRUCTURES (Definitions)
   ==================================================== */

/* List of line numbers where a variable appears */
typedef struct LineListRec
   { int lineno;
     struct LineListRec * next;
   } * LineList;

/* The bucket record (hash table node).
   This is in the .h so analyze.c can read 'type' and 'isParam'.
*/
typedef struct BucketListRec
   { char * name;
     LineList lines;
   int memloc;       /* Memory location */
     
     /* --- Nuevos campos para C-Minus --- */
     ExpType type;     /* Integer, Void, etc. */
   int isParam;      /* TRUE if it's a parameter */
   int isFunc;       /* TRUE if it's a function */
   int isArray;      /* TRUE if it's an array */
   int arraySize;    /* Size of the array */
   char * scopeName; /* To print the scope name */

   struct treeNode * node; /* Pointer to the declaration AST node */
     
     struct BucketListRec * next;
   } * BucketList;

/* ====================================================
   FUNCTION PROTOTYPES (Public Interface)
   ==================================================== */

/* Scope management (enter and exit blocks) */
void sc_push(char * scopeName);
void sc_pop(void);
char * sc_top_name(void);

/* Insert a symbol into the current scope with all its attributes */
/* Update st_insert prototype to accept the AST node */
void st_insert( char * name, int lineno, int loc, 
                ExpType type, int isFunc, int isParam, int isArray, int arraySize, struct treeNode * node );
/* Lookup a symbol (checks current scope -> parent -> global) */
BucketList st_lookup ( char * name );

/* Lookup symbol ONLY in the current scope (to check redefinitions) */
BucketList st_lookup_top ( char * name );

/* Print the complete symbol table */
void printSymTab(FILE * listing);

#endif
