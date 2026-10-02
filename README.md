# C-Minus Compiler

[![Language](https://img.shields.io/badge/Language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Tools](https://img.shields.io/badge/Tools-Flex%20%7C%20Bison-orange.svg)](https://github.com/westes/flex)
[![Build](https://img.shields.io/badge/Build-Make-green.svg)](https://www.gnu.org/software/make/)

A modular front-end compiler for the **C-Minus** programming language implemented in C. The system performs full lexical scanning, LALR(1) syntax analysis with Abstract Syntax Tree (AST) construction, and hierarchical semantic type analysis.

---

## Architecture & Compiler Phases

### 1. Lexical Analysis (Scanner)
* **Manual DFA:** Deterministic Finite Automaton implemented in C (\scan.c\) featuring controlled lookahead and backtracking (\ungetNextChar\) to resolve operators (\==\, \<=\, \>=\, \!=\) and comments (\/* ... */\).
* **Flex Specification:** Regular expression-based scanner in \cminus.l\ utilizing exclusive start conditions (\%x COMMENT\) to handle nested and multi-line comments.

### 2. Syntax Analysis (Parser & AST)
* **LALR(1) Grammar:** Implemented via Bison (\cminus.y\) conforming to the official C-Minus language specification.
* **Ambiguity Elimination:** Solved the classic dangling-else ambiguity by structuring statements into disjoint \matched_stmt\ and \unmatched_stmt\ production rules, achieving a clean compilation with zero shift/reduce conflicts.
* **AST Construction:** Builds an Abstract Syntax Tree with specialized node types (\DeclK\, \StmtK\, \ExpK\) tracking identifiers, function signatures, and array dimensions.

### 3. Semantic Analysis & Type Checking
* **Hierarchical Scope Management:** Implemented via a stack-based symbol table (\ScopeList\ in \symtab.c\) providing scope isolation and variable shadowing across global and local blocks.
* **Two-Pass Traversal:**
  * **Pass 1 (Symbol Table):** Pre-order AST traversal to register declarations, insert built-in I/O primitives (\input()\, \output()\), and bind declaration node references.
  * **Pass 2 (Type Checker):** Post-order AST traversal enforcing operand type compatibility, array indexing scalar validation, and function call arity checks.

---

## Project Structure

\\\	ext
.
├── Makefile              # Build automation
├── globals.h             # Core data structures and AST node definitions
├── main.c                # Compiler driver
├── scan.c / scan.h       # Manual DFA lexer implementation
├── cminus.l              # Flex scanner rules
├── cminus.y              # Bison LALR(1) grammar & AST generation
├── util.c / util.h       # AST serialization and helper functions
├── symtab.c / symtab.h   # Stack-based scoped symbol table
├── analyze.c / analyze.h # Semantic analysis & type checking routines
├── tests/                # Test suites & expected outputs
│   ├── 1-lexer/          # Lexical test files (.txt) & outputs
│   ├── 2-parser/         # Grammar test files (.txt) & AST trees
│   └── 3-semantic/       # Semantic test files (.cm) & symbol tables
└── docs/                 # Detailed technical project reports (PDF)
\\\

---

## Build & Execution

### Compilation

\\\ash
# Build complete semantic analyzer
make cminus_semantic

# Build syntax analyzer (AST generator)
make cminus_parser

# Build lexical analyzer (DFA / Flex)
make cminus_cimpl
\\\

### Running Verification Tests

\\\ash
# Run semantic analysis on test suites
./cminus_semantic tests/3-semantic/test_1.cm

# Run parser and compare AST with expected result
./cminus_parser tests/2-parser/test.1.txt > my_ast.txt
diff -y tests/2-parser/result.1.txt my_ast.txt

# Run lexical analysis
./cminus_cimpl tests/1-lexer/test.1.txt
\\\

---

## Technical Challenges & Solutions

* **Token Buffer Safety:** Bison actions defer execution until complete rule reduction, which could lead to buffer overwrite from Flex lookahead tokens. Resolved by employing intermediate reduction rules (\identifier: ID\) that immediately clone token lexemes into temporary AST nodes.
* **Dangling-Else Resolution:** Avoided suppression directives (\%nonassoc\) in favor of rewriting grammar productions into explicit matched/unmatched flows, completely eliminating grammar warnings.
* **Function Call Verification:** Extended symbol table records with direct pointers to function AST declaration nodes, enabling compile-time argument count and type matching.

---

## Author
* **Josep Montoro Pascual**
