/****************************************************/
/* File: symtab.c                                   */
/* Symbol table implementation for the C-MINUS compiler*/
/* (allows only one symbol table)                   */
/* Symbol table is implemented as a chained         */
/* hash table                                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"
#include "globals.h"
#include "util.h"

/* SHIFT is the power of two used as multiplier
   in hash function  */
#define SHIFT 4

/* ====================================================
  PRIVATE STRUCTURES (Only used within this file)
  ==================================================== */

/* Node for the scope list (scope stack) */
typedef struct ScopeListRec
   { char * funcName;              /* Scope name */
     int nestedLevel;              /* Nesting level */
     BucketList hashTable[SIZE];   /* This scope's own hash table */
     struct ScopeListRec * parent; /* Parent scope (previous on stack) */
     struct ScopeListRec * next;   /* For linear list (printing) */
   } * ScopeList;

/* Static variables (private) */
static ScopeList scopeStack = NULL; /* Current scope (top of stack) */
static ScopeList allScopes = NULL;  /* Historical list of scopes (for printing) */
static ScopeList lastScope = NULL;  /* Helper to build allScopes */

  /* ====================================================
    PRIVATE FUNCTIONS (Helpers)
    ==================================================== */


/* Hash function */
static int hash ( char * key )
{ int temp = 0;
  int i = 0;
  while (key[i] != '\0')
  { temp = ((temp << SHIFT) + key[i]) % SIZE;
    ++i;
  }
  return temp;
}

/* Helper to convert ExpType to string for printing */
static char* typeString(ExpType type, int isArray) {
    if (isArray) return "int[]";
    if (type == Integer) return "int";
    if (type == Void) return "void";
    return "error";
}

/* ====================================================
  PUBLIC FUNCTION IMPLEMENTATIONS
  ==================================================== */

/* Create a new scope and push it onto the stack */
void sc_push(char * scopeName)
{ 
  ScopeList newScope = (ScopeList) malloc(sizeof(struct ScopeListRec));
  newScope->funcName = copyString(scopeName);
  newScope->nestedLevel = (scopeStack == NULL) ? 0 : scopeStack->nestedLevel + 1;
  newScope->parent = scopeStack; 
  newScope->next = NULL;

  /* Initialize the new hash table */
  int i;
  for (i=0; i<SIZE; i++) newScope->hashTable[i] = NULL;

  /* Update the stack */
  scopeStack = newScope;

  /* Save in history for printing at the end */
  if (allScopes == NULL) {
      allScopes = newScope;
      lastScope = newScope;
  } else {
      lastScope->next = newScope;
      lastScope = newScope;
  }
}

/* Pop the current scope */
void sc_pop(void)
{
  if (scopeStack != NULL)
    scopeStack = scopeStack->parent;
}

/* Get current scope name */
char * sc_top_name(void)
{
    if (scopeStack != NULL) return scopeStack->funcName;
    return "global";
}

/* Insert symbol into the current scope */
void st_insert( char * name, int lineno, int loc, 
                ExpType type, int isFunc, int isParam, int isArray, int arraySize, TreeNode * node )
{ 
  if (scopeStack == NULL) return; 

  int h = hash(name);
  BucketList l = scopeStack->hashTable[h];
  
  while ((l != NULL) && (strcmp(name,l->name) != 0))
    l = l->next;
  
  if (l == NULL) 
  { 
    l = (BucketList) malloc(sizeof(struct BucketListRec));
    l->name = name;
    l->lines = (LineList) malloc(sizeof(struct LineListRec));
    l->lines->lineno = lineno;
    l->memloc = loc;
    l->lines->next = NULL;
    
    l->type = type;
    l->isFunc = isFunc;
    l->isParam = isParam;
    l->isArray = isArray;
    l->arraySize = arraySize;
    l->scopeName = scopeStack->funcName;
    l->node = node;

    l->next = scopeStack->hashTable[h];
    scopeStack->hashTable[h] = l;
  }
  else /* Already exists, just add usage line */
  { 
    LineList t = l->lines;
    while (t->next != NULL) t = t->next;
    t->next = (LineList) malloc(sizeof(struct LineListRec));
    t->next->lineno = lineno;
    t->next->next = NULL;
  }
} 

/* Lookup symbol in scope stack (Current -> Parent -> Global) */
BucketList st_lookup ( char * name )
{ 
  ScopeList scope = scopeStack;
  while (scope != NULL)
  {
      int h = hash(name);
      BucketList l = scope->hashTable[h];
      while ((l != NULL) && (strcmp(name,l->name) != 0))
        l = l->next;
      
      if (l != NULL) return l; /* Found */
      scope = scope->parent;
  }
  return NULL; /* No encontrado */
}

/* Lookup symbol ONLY in the current scope (do not search parents) */
BucketList st_lookup_top ( char * name )
{ 
  if (scopeStack == NULL) return NULL;

  int h = hash(name);
  BucketList l = scopeStack->hashTable[h];
  while ((l != NULL) && (strcmp(name,l->name) != 0))
    l = l->next;
  
  return l;
}/* =========================================================================
   PRINT SYMBOL TABLE
   ========================================================================= */
void printSymTab(FILE * listing)
{ 
  int i;
  ScopeList scope;
  BucketList l;

  /* --------------------------------------------------------
    1. Print General Symbol Table
    -------------------------------------------------------- */
  fprintf(listing, "< Symbol Table >\n");
  fprintf(listing, " Symbol Name   Symbol Kind   Symbol Type    Scope Name   Location  Line Numbers\n");
  fprintf(listing, "-------------  -----------  -------------  ------------  --------  ------------\n");
  
  scope = allScopes;
  while (scope != NULL)
  {
      for (i=0; i<SIZE; ++i)
      { 
        if (scope->hashTable[i] != NULL)
        { 
          l = scope->hashTable[i];
          while (l != NULL)
          { 
            LineList t = l->lines;
            
            fprintf(listing,"%-15s", l->name);
            
            if (l->isFunc) 
                fprintf(listing,"%-13s","Function");
            else 
                fprintf(listing,"%-13s","Variable"); 

            fprintf(listing,"%-15s", typeString(l->type, l->isArray));
            fprintf(listing,"%-14s", l->scopeName);
            
            /* NOTE: 8 characters for Location plus 1 trailing space */
            /* Previously used %-10d, which produced one extra space */
            fprintf(listing,"%-8d ", l->memloc);
            
            while (t != NULL)
            { fprintf(listing,"%4d ",t->lineno);
              t = t->next;
            }
            fprintf(listing,"\n");
            l = l->next;
          }
        }
      }
      scope = scope->next;
  }

  /* --------------------------------------------------------
    2. Print Functions Table
    -------------------------------------------------------- */
  fprintf(listing, "\n\n< Functions >\n");
  fprintf(listing, "Function Name   Return Type   Parameter Name  Parameter Type\n");
  fprintf(listing, "-------------  -------------  --------------  --------------\n");

  scope = allScopes;
  while (scope != NULL && strcmp(scope->funcName, "global") != 0) scope = scope->next;

  if (scope != NULL) {
      for (i=0; i<SIZE; ++i) {
          l = scope->hashTable[i];
          while (l != NULL) {
              if (l->isFunc) {
                  fprintf(listing, "%-15s%-15s", l->name, typeString(l->type, FALSE));
                  
                  int firstParam = TRUE;
                  
                  if (l->node == NULL) {
                      if (strcmp(l->name, "input") == 0) {
                           fprintf(listing, "%-16s%-14s\n", "", "void");
                           firstParam = FALSE;
                      } else if (strcmp(l->name, "output") == 0) {
                           fprintf(listing, "\n"); 
                           fprintf(listing, "%-15s%-15s%-16s%-14s\n", "-", "-", "value", "int");
                           firstParam = FALSE;
                      }
                  } 
                  else {
                      TreeNode * funcNode = l->node;
                      TreeNode * params = funcNode->child[0]; 
                      
                      if (params == NULL) {
                           fprintf(listing, "%-16s%-14s\n", "", "void");
                           firstParam = FALSE;
                      } else {
                           int isVoidParam = (params->nodekind == DeclK && params->kind.decl == ParamK && params->type == Void);
                           
                           if (isVoidParam) {
                               fprintf(listing, "%-16s%-14s\n", "", "void");
                               firstParam = FALSE;
                           } else {
                               fprintf(listing, "\n"); 
                               while (params != NULL) {
                                   if (params->nodekind == DeclK && params->kind.decl == ParamK) {
                                        fprintf(listing, "%-15s%-15s%-16s%-14s\n", 
                                                "-", "-", 
                                                params->attr.name, 
                                                typeString(params->type, params->isArray));
                                   }
                                   params = params->sibling;
                               }
                               firstParam = FALSE;
                           }
                      }
                  }
                  if (firstParam) fprintf(listing, "\n"); 
              }
              l = l->next;
          }
      }
  }

  /* --------------------------------------------------------
    3. Print Global Symbols
    -------------------------------------------------------- */
  fprintf(listing, "\n\n< Global Symbols >\n");
  fprintf(listing, " Symbol Name   Symbol Kind   Symbol Type\n");
  fprintf(listing, "-------------  -----------  -------------\n");

  scope = allScopes;
  while (scope != NULL && strcmp(scope->funcName, "global") != 0) scope = scope->next;

  if (scope != NULL) {
      for (i=0; i<SIZE; ++i) {
          l = scope->hashTable[i];
          while (l != NULL) {
               fprintf(listing, "%-15s", l->name);
               if (l->isFunc) fprintf(listing, "%-13s", "Function");
               else fprintf(listing, "%-13s", "Variable");
               
               fprintf(listing, "%-13s\n", typeString(l->type, l->isArray));
               l = l->next;
          }
      }
  }

  /* --------------------------------------------------------
    4. Print Scopes
    -------------------------------------------------------- */
  fprintf(listing, "\n\n< Scopes >\n");
  fprintf(listing, " Scope Name   Nested Level   Symbol Name   Symbol Type\n");
  fprintf(listing, "------------  ------------  -------------  -----------\n");

  scope = allScopes;
  int previousScopeHadSymbols = FALSE;

  while (scope != NULL) {
      if (strcmp(scope->funcName, "global") != 0) {
          int hasSymbols = FALSE;
          for (i=0; i<SIZE; ++i) {
              if (scope->hashTable[i] != NULL) { hasSymbols = TRUE; break; }
          }

          if (hasSymbols) {
              if (previousScopeHadSymbols) fprintf(listing, "\n");

              for (i=0; i<SIZE; ++i) {
                  l = scope->hashTable[i];
                  while (l != NULL) {
                      fprintf(listing, "%-14s%-14d%-15s%-11s\n", 
                              scope->funcName, 
                              scope->nestedLevel, 
                              l->name, 
                              typeString(l->type, l->isArray));
                      l = l->next;
                  }
              }
              previousScopeHadSymbols = TRUE;
          }
      }
      scope = scope->next;
  }
  fprintf(listing, "\n");
}