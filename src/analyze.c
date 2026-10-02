/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"

/* ====================================================
   VARIABLES GLOBALES ESTÁTICAS
   ==================================================== */
static int location = 0;
static int globalLocation = 0;
static int preserveScope = FALSE;
static ExpType currentFuncType = Integer; 

/* ====================================================
   FUNCIONES AUXILIARES
   ==================================================== */

/* Traverse genérico */
static void traverse( TreeNode * t,
               void (* preProc) (TreeNode *),
               void (* postProc) (TreeNode *) )
{ 
  if (t != NULL)
  { preProc(t);
    { int i;
      for (i=0; i < MAXCHILDREN; i++)
        traverse(t->child[i],preProc,postProc);
    }
    postProc(t);
    traverse(t->sibling,preProc,postProc);
  }
}

static void nullProc(TreeNode * t)
{ 
    if (t==NULL) return;
    else return;
}

static void typeError(TreeNode * t, char * message)
{ fprintf(listing,"Type error at line %d: %s\n",t->lineno,message);
  Error = TRUE;
}

static void symbolError(TreeNode * t, char * message)
{ fprintf(listing,"Symbol error at line %d: %s\n",t->lineno,message);
  Error = TRUE;
}

/* Agregar funciones input() y output() */
static void addBuiltinFunctions(void)
{
    /* 1. Insertar funciones y aumentar globalLocation para que main tenga el ID correcto */
    st_insert("input", 0, globalLocation++, Integer, TRUE, FALSE, FALSE, 0, NULL);
    st_insert("output", 0, globalLocation++, Void, TRUE, FALSE, FALSE, 0, NULL);

    /* 2. Simular el scope interno de 'output' para que aparezca 'value' en la tabla */
    sc_push("output");
    /* Usamos 'location' local para parámetros */
    st_insert("value", 0, location++, Integer, FALSE, TRUE, FALSE, 0, NULL);
    sc_pop();
    
    /* Resetear location local */
    location = 0; 
}

/* ====================================================
   CONSTRUCCIÓN DE TABLA DE SÍMBOLOS (Paso 1)
   ==================================================== */

static void insertNode( TreeNode * t)
{ 
  switch (t->nodekind)
  { 
    case StmtK:
      switch (t->kind.stmt)
      { 
        case CompoundK:
          if (preserveScope) {
              preserveScope = FALSE; 
          } else {
              sc_push("compound"); 
          }
          break;
        default: break;
      }
      break;

    case DeclK:
      switch (t->kind.decl)
      { 
        case FunK:
            if (st_lookup_top(t->attr.name) != NULL) {
                fprintf(listing, "Error: Symbol \"%s\" is redefined at line %d\n", t->attr.name, t->lineno);
                Error = TRUE;
            } else {
                st_insert(t->attr.name, t->lineno, globalLocation++, t->type, TRUE, FALSE, FALSE, 0, t);
            }
            sc_push(t->attr.name); 
            location = 0;          
            preserveScope = TRUE;  
            break;

        case VarK:
        case ParamK:
            if (t->kind.decl == ParamK && t->type == Void && t->attr.name == NULL) {
                break; 
            }
            if (t->type == Void) {
                fprintf(listing, "Error: The void-type variable is declared at line %d (name : \"%s\")\n", t->lineno, t->attr.name);
                Error = TRUE;
                if (t->attr.name)
                   st_insert(t->attr.name, t->lineno, location++, Integer, FALSE, (t->kind.decl==ParamK), t->isArray, 0, t);
            }
            else if (st_lookup_top(t->attr.name) != NULL) {
                fprintf(listing, "Error: Symbol \"%s\" is redefined at line %d\n", t->attr.name, t->lineno);
                Error = TRUE;
            }
            else {
                st_insert(t->attr.name, t->lineno, location++, t->type, FALSE, (t->kind.decl==ParamK), t->isArray, 0, t);
            }
            break;
      }
      break;

    case ExpK:
      switch (t->kind.exp)
      { 
        case ConstK: t->type = Integer; break;
        case IdK:
        case CallK:
        {
           BucketList l = st_lookup(t->attr.name);
           if (l == NULL) {
              if (t->kind.exp == CallK) {
                 fprintf(listing, "Error: Undeclared function \"%s\" is called at line %d\n", t->attr.name, t->lineno);
                 st_insert(t->attr.name, t->lineno, globalLocation++, Undetermined, TRUE, FALSE, FALSE, 0, NULL);
                 t->type = Undetermined;
              } else {
                 fprintf(listing, "Error: Undeclared variable \"%s\" is used at line %d\n", t->attr.name, t->lineno);
                 t->type = Undetermined;
              }
              Error = TRUE;
           } else {
              LineList lines = l->lines;
              while (lines->next != NULL) lines = lines->next;
              lines->next = (LineList) malloc(sizeof(struct LineListRec));
              lines->next->lineno = t->lineno;
              lines->next->next = NULL;
              
              t->type = l->type;
              t->isArray = l->isArray;
           }
           break;
        }
        default: break;
      }
      break;
    default: break;
  }
}

static void afterNode(TreeNode * t)
{
    if (t->nodekind == StmtK && t->kind.stmt == CompoundK)
    {
        if (strcmp(sc_top_name(), "compound") == 0) {
            sc_pop();
        }
    }
    else if (t->nodekind == DeclK && t->kind.decl == FunK)
    {
        sc_pop(); 
    }
}

void buildSymtab(TreeNode * syntaxTree)
{ 
  sc_push("global");
  globalLocation = 0;
  location = 0;
  
  addBuiltinFunctions();
  preserveScope = FALSE; 
  
  traverse(syntaxTree,insertNode,afterNode);
  
  if (TraceAnalyze) { 
      fprintf(listing,"\n\n"); 
      printSymTab(listing); 
  }
  
  /* IMPORTANTE: No hacemos pop del scope global aquí. */
  /* sc_pop(); */ 
}

/* ====================================================
   CHEQUEO DE TIPOS (Paso 2)
   ==================================================== */

static void checkCall(TreeNode * t)
{
    BucketList l = st_lookup(t->attr.name);
    if (l == NULL) return; 

    TreeNode * arg = t->child[0]; 
    TreeNode * param = NULL;
    
    if (l->node == NULL) { /* Built-in */
        if (strcmp(l->name, "input") == 0) {
            if (arg != NULL) {
                fprintf(listing, "Error: Invalid function call at line %d (name : \"%s\")\n", t->lineno, t->attr.name);
                Error = TRUE;
            }
        } 
        else if (strcmp(l->name, "output") == 0) {
            if (arg == NULL || arg->sibling != NULL || arg->type != Integer || arg->isArray) {
                fprintf(listing, "Error: Invalid function call at line %d (name : \"%s\")\n", t->lineno, t->attr.name);
                Error = TRUE;
            }
        }
    }
    else { /* User defined */
        TreeNode * funcDecl = l->node;
        param = funcDecl->child[0]; 

        if (param != NULL && param->kind.decl == ParamK && param->type == Void) {
            param = NULL; 
        }

        while (arg != NULL && param != NULL) {
            if (arg->type != param->type || arg->isArray != param->isArray) {
                 fprintf(listing, "Error: Invalid function call at line %d (name : \"%s\")\n", t->lineno, t->attr.name);
                 Error = TRUE;
                 return; 
            }
            arg = arg->sibling;
            param = param->sibling;
        }

        if (arg != NULL || param != NULL) {
            fprintf(listing, "Error: Invalid function call at line %d (name : \"%s\")\n", t->lineno, t->attr.name);
            Error = TRUE;
        }
    }
}

static void checkNode(TreeNode * t)
{ 
  switch (t->nodekind)
  { 
    case ExpK:
      switch (t->kind.exp)
      { 
        case ConstK: 
            t->type = Integer; 
            break;
        
        case CallK:
           checkCall(t);
           break;
        
        case OpK:
          if ((t->child[0]->type != Integer) || (t->child[1]->type != Integer) ||
              (t->child[0]->isArray) || (t->child[1]->isArray)) {
            fprintf(listing,"Error: Invalid operation at line %d\n", t->lineno);
            Error = TRUE;
          }
          t->type = Integer;
          break;
        
        case AssignK:
          if (t->child[0]->type == Void || t->child[1]->type == Void)
             fprintf(listing,"Error: Invalid assignment at line %d\n", t->lineno);
          else if (t->child[0]->isArray && !t->child[1]->isArray) 
             fprintf(listing,"Error: Invalid assignment at line %d\n", t->lineno);
          else if (!t->child[0]->isArray && t->child[1]->isArray)
             fprintf(listing,"Error: Invalid assignment at line %d\n", t->lineno);
          else 
             t->type = Integer;
          break;
          
        case IdK:
          if (t->type != Undetermined) {
             if (t->child[0] != NULL) {
                if (t->isArray == FALSE) {
                    fprintf(listing, "Error: Invalid array indexing at line %d (name \"%s\"). Indexing can only be allowed for int[] variables\n", t->lineno, t->attr.name);
                    Error = TRUE;
                } else if (t->child[0]->type != Integer) {
                    fprintf(listing, "Error: Invalid array indexing at line %d (name : \"%s\"). indices should be integer\n", t->lineno, t->attr.name);
                    Error = TRUE;
                }
                t->type = Integer; 
                t->isArray = FALSE; 
             }
          }
          break;
        default: break;
      }
      break;

    case StmtK:
      switch (t->kind.stmt)
      { 
        case IfK:
        case WhileK:
          if (t->child[0]->type != Integer) {
             fprintf(listing,"Error: Invalid condition at line %d\n", t->lineno);
             Error = TRUE;
          }
          break;
        case ReturnK: {
            TreeNode * expr = t->child[0];
            if (currentFuncType == Void) {
                if (expr != NULL) { 
                    fprintf(listing,"Error: Invalid return at line %d\n", t->lineno); 
                    Error = TRUE; 
                }
            } else if (currentFuncType == Integer) {
                if (expr == NULL || expr->type != Integer) { 
                    fprintf(listing,"Error: Invalid return at line %d\n", t->lineno); 
                    Error = TRUE; 
                }
            }
        } break;
        default: break;
      }
      break;
    default: break;
  }
}

static void typeCheckTraversal(TreeNode * t) {
    if (t != NULL) {
        if (t->nodekind == DeclK && t->kind.decl == FunK) { 
            currentFuncType = t->type; 
        }
        
        int i;
        for (i=0; i < MAXCHILDREN; i++) typeCheckTraversal(t->child[i]);
        
        checkNode(t);
        
        typeCheckTraversal(t->sibling);
    }
}

void typeCheck(TreeNode * syntaxTree) { 
    typeCheckTraversal(syntaxTree); 
}