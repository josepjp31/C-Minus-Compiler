# C-Minus Compiler

[![Language](https://img.shields.io/badge/Language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Parser Generator](https://img.shields.io/badge/Tools-Flex%20%7C%20Bison-orange.svg)](https://github.com/westes/flex)
[![Build](https://img.shields.io/badge/Build-Make-green.svg)](https://www.gnu.org/software/make/)

A modular front-end compiler for the **C-Minus** programming language implemented in C. The compiler translates C-Minus source code through lexical analysis, syntax validation with Abstract Syntax Tree (AST) construction, and hierarchical semantic type analysis.

---

## Architecture & Compiler Pipeline

The project is structured into three progressive compiler phases:

### 1. Lexical Analysis (Scanner)
* **Manual DFA:** A custom Deterministic Finite Automaton implemented in C (\`scan.c\`) with lookahead and backtracking (\`ungetNextChar\`) for operators (\`==\`, \`<=\`, \`>=\`, \`!=\`) and multi-line comments (\`/* ... */\`).
* **Flex Specification:** Regular-expression-based token recognition in \`cminus.l\` leveraging exclusive start conditions (\`%x COMMENT\`) for robust comment parsing.

### 2. Syntax Analysis (Parser & AST)
* **LALR(1) Grammar:** Implemented using Bison (\`cminus.y\`) according to the C-Minus grammar.
* **Ambiguity & Conflict Elimination:** Addressed classic dangling-else ambiguity by redesigning production rules into explicit \`matched_stmt\` and \`unmatched_stmt\` sets, eliminating shift/reduce warnings without resorting to artificial precedence rules.
* **AST Construction:** Generates a structured Abstract Syntax Tree (\`TreeNode\`) mapping declarations (\`DeclK\`), statements (\`StmtK\`), expressions (\`ExpK\`), and array boundaries.

### 3. Semantic Analysis & Type Checking
* **Nested Scopes & Symbol Table:** Stack-based scope hierarchy (\`ScopeList\` in \`symtab.c\`) supporting variable shadowing, local definitions, and lexical parent lookups.
* **Two-Pass Traversal:**
  * **Pass 1 (Symbol Resolution):** Constructs symbol entries across nested scopes, injects built-in I/O functions (\`input()\`, \`output()\`), and stores AST declaration references for signature checking.
  * **Pass 2 (Type Checker):** Traverses the AST in post-order to validate operand compatibility, array indexing scalarity, condition truth values, and function call arity.

---

## Project Structure

\`\`\`text
.
├── Makefile              # Build automation script
├── globals.h             # Global definitions and AST node structures
├── main.c                # Compiler entry point and phase dispatch
├── scan.c / scan.h       # Manual DFA lexical analyzer
├── cminus.l              # Flex lexical specification
├── cminus.y              # Bison parser grammar & AST builder
├── util.c / util.h       # AST tree printing and token utilities
├── symtab.c / symtab.h   # Stack-based scoped symbol table
├── analyze.c / analyze.h # Semantic analyzer and type checker
├── test.1.txt / test.2.txt # Sample test suites
└── results/              # Expected vs. actual compilation outputs
\`\`\`

---

## Build & Execution

### Prerequisites
* \`gcc\`
* \`flex\`
* \`bison\`
* \`make\`

### Compilation

\`\`\`bash
# Build the complete compiler with semantic analysis
make cminus_semantic

# Build the syntax analyzer (AST generation only)
make cminus_parser

# Build the lexical analyzer (DFA / Flex)
make cminus_cimpl
\`\`\`

### Running Tests

\`\`\`bash
# Run semantic type check on a program
./cminus_semantic test.1.txt

# Inspect AST output from the parser
./cminus_parser test.1.txt

# Inspect token stream from the scanner
./cminus_cimpl test.1.txt
\`\`\`

---

## Technical Challenges & Solutions

* **Token String Race Conditions in Bison Actions:** Bison actions defer execution until a rule reduces, at which point global scanner buffers can be overwritten by lookahead tokens. Resolved by creating intermediate non-terminal reduction steps (\`identifier: ID\`) that immediately capture and allocate the lexeme string.
* **Grammar Shift/Reduce Warnings:** Rather than suppressing warnings via \`%nonassoc\`, the grammar was restructured into strict matched/unmatched statement pairs, yielding zero compiler warnings.
* **Function Signature Verification:** The symbol table entries store pointers back to their origin AST nodes, allowing argument-parameter validation during invocation passes.

---

## Author
* **Josep Montoro Pascual**
