# C-Minus Compiler

[![Language](https://img.shields.io/badge/Language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Parser Generator](https://img.shields.io/badge/Tools-Flex%20%7C%20Bison-orange.svg)](https://github.com/westes/flex)
[![Build](https://img.shields.io/badge/Build-Make-green.svg)](https://www.gnu.org/software/make/)

A modular front-end compiler for the **C-Minus** programming language implemented in C.

The compiler translates C-Minus source code through:

- **Lexical analysis**
- **Syntax validation**
- **Abstract Syntax Tree (AST) construction**
- **Hierarchical semantic type analysis**

---

## Architecture & Compiler Pipeline

The project is structured into three progressive compiler phases.

### 1. Lexical Analysis — Scanner

- **Manual DFA:** A custom Deterministic Finite Automaton implemented in C (`scan.c`) with lookahead and backtracking (`ungetNextChar`) for operators (`==`, `<=`, `>=`, `!=`) and multi-line comments (`/* ... */`).
- **Flex Specification:** Regular-expression-based token recognition in `cminus.l` leveraging exclusive start conditions (`%x COMMENT`) for robust comment parsing.

### 2. Syntax Analysis — Parser & AST

- **LALR(1) Grammar:** Implemented using Bison (`cminus.y`) according to the C-Minus grammar.
- **Ambiguity & Conflict Elimination:** Addressed classic dangling-else ambiguity by redesigning production rules into explicit `matched_stmt` and `unmatched_stmt` sets, eliminating shift/reduce warnings without resorting to artificial precedence rules.
- **AST Construction:** Generates a structured Abstract Syntax Tree (`TreeNode`) mapping declarations (`DeclK`), statements (`StmtK`), expressions (`ExpK`), and array boundaries.

### 3. Semantic Analysis & Type Checking

- **Nested Scopes & Symbol Table:** Stack-based scope hierarchy (`ScopeList` in `symtab.c`) supporting variable shadowing, local definitions, and lexical parent lookups.
- **Two-Pass Traversal:**
  - **Pass 1 — Symbol Resolution:** Constructs symbol entries across nested scopes, injects built-in I/O functions (`input()`, `output()`), and stores AST declaration references for signature checking.
  - **Pass 2 — Type Checker:** Traverses the AST in post-order to validate operand compatibility, array indexing scalarity, condition truth values, and function call arity.

---

## Project Structure

```text
.
├── Makefile                    # Multi-target build script with conditional macro dispatch
├── test.sh                     # Automated test runner with diff verification & color diagnostics
├── Dockerfile                  # Containerized build environment (Debian/GCC/Flex/Bison)
├── src/                        # Compiler source code
│   ├── globals.h               # Token definitions, AST structures, and compilation flags
│   ├── main.c                  # Driver program supporting conditional phase flags
│   ├── scan.c / scan.h         # Manual DFA scanner implementation (Phase 1)
│   ├── cminus.l                # Flex lexical specification (CRLF-safe)
│   ├── cminus.y                # Bison LALR(1) grammar & AST builder (Phase 2)
│   ├── util.c / util.h         # Token string conversion, AST visualizer, and syntax utilities
│   ├── symtab.c / symtab.h     # Stack-based scoped symbol table implementation
│   └── analyze.c / analyze.h   # Two-pass semantic type checker & analyzer (Phase 3)
└── tests/                      # Categorized test suites and reference outputs
    ├── 1-lexer/                # Lexical scanner test inputs and expected outputs
    ├── 2-parser/               # Parser test programs and expected AST trees
    └── 3-semantic/             # Valid programs and negative type-error test cases (.cm)
```

## Environment & Docker Setup

The project is developed and built inside a **Docker container** so that the build environment (GCC, Flex, Bison, Make, OS tools) is exactly the same on every machine and matches the grading environment. Developing outside Docker is discouraged because of potential setup and toolchain differences.

| | |
|---|---|
| **Host OS** | Windows (via WSL 2) or macOS |
| **Linux distribution (WSL)** | Ubuntu 22.04 (recommended) |
| **Container image** | `cs-compiler-hw:1.0` (built from the provided `Dockerfile`) |
| **Container name** | `CminusCompiler` |
| **Toolchain inside the container** | `gcc`, `flex`, `bison`, `make` |

### 1. Docker on Windows (WSL)

Docker is installed inside WSL, so WSL has to be set up first.

**a) Install WSL with Ubuntu 22.04** (PowerShell as Administrator):

```powershell
wsl --install Ubuntu-22.04
```

Create the default UNIX user when prompted (the username does not need to match your Windows username), then start WSL:

```powershell
wsl
```

Windows files are reachable from WSL through `/mnt/...` (e.g. `/mnt/c/Users/<user>/Downloads/`).

**b) Install and enable Docker inside WSL:**

```bash
sudo apt-get update
sudo apt install docker.io
sudo systemctl start docker
sudo systemctl enable docker
```

**c) Add your user to the `docker` group** (so `sudo` is not needed for every command):

```bash
sudo usermod -aG docker <user id>
exit
```

Then, back in PowerShell, restart WSL so the group change takes effect:

```powershell
wsl --shutdown
wsl
```

> From this point on, run all the commands inside the WSL terminal.

### 2. Docker on macOS

```bash
brew install --cask docker
```

Check the installation:

```bash
docker --version
```

#### 3. Linux

```bash
sudo apt-get update
sudo apt install docker.io git
sudo systemctl start docker
sudo usermod -aG docker $USER   # log out and back in afterwards
```

Check that Docker works:

```bash
docker --version
```

### 4. Project Setup

**a) Create a working directory** (any location works as long as you stay consistent):

```bash
mkdir ~/work
```

**b) Clone the repository**

```bash
git clone https://github.com/josepjp31/C-Minus-Compiler.git
cd C-Minus-Compiler
```
> **Windows users:** clone the repository inside the WSL filesystem (e.g. your home folder `~`), not under `/mnt/c/...`. This avoids line-ending (CRLF) problems with `test.sh` and the Makefile.

**c) Build the Docker image:**

From the root of the repository (where the `Dockerfile` is):

```bash
docker build -t cs-compiler-hw:1.0 .
```

**d) Start the container**, mounting the current directory as `/work`:

```bash
docker run --name CminusCompiler --rm -it -v "$PWD":/work -w /work cs-compiler-hw:1.0
```

This opens a shell inside a container named `CminusCompiler`. Because of `-v "$PWD":/work`, every file you edit on the host is immediately visible in the container (and vice versa), and `--rm` removes the container when you exit it. The image is kept, so the same command can be repeated at any time.

### Daily Workflow

```bash
# Windows: open WSL, then go to the working directory
wsl
cd \\\~/work

# Start the container
docker run --name CminusCompiler --rm -it -v "$PWD":/work -w /work cs-compiler-hw:1.0

# Inside the container: build, run, test
make all
./test.sh
```

## Build & Execution

### Prerequisites

- `gcc`
- `flex`
- `bison`
- `make`

### Compilation

```bash
# Build all three phase targets
make all

# Build individual targets:
make cminus_cimpl      # Phase 1: Manual DFA Lexical Analyzer
make cminus_parser     # Phase 2: Flex + Bison Syntax Analyzer (AST only)
make cminus_semantic   # Phase 3: Complete Compiler with Semantic Analyzer

# Clean intermediate objects and binaries
make clean
```

### Running Tests

#### Running Individual Phases

```bash
# 1. Scanner — Inspect token stream
./cminus_cimpl tests/1-lexer/test.1.txt

# 2. Parser — Generate and print the AST
./cminus_parser tests/2-parser/test.1.txt

# 3. Semantic Analyzer — Symbol table resolution & type verification
./cminus_semantic tests/3-semantic/test_1.cm
```
#### Automated Test Suite
```bash
# Grant execution permissions (if not already set)
chmod +x test.sh

# Run all test cases with colored pass/fail status
./test.sh
```

---

## Technical Challenges & Solutions

- **Token String Race Conditions in Bison Actions:** Bison actions defer execution until a rule reduces, at which point global scanner buffers can be overwritten by lookahead tokens. Resolved by creating intermediate non-terminal reduction steps (`identifier: ID`) that immediately capture and allocate the lexeme string.

- **Grammar Shift/Reduce Warnings:** Rather than suppressing warnings via `%nonassoc`, the grammar was restructured into strict matched/unmatched statement pairs, yielding zero compiler warnings.

- **Function Signature Verification:** The symbol table entries store pointers back to their origin AST nodes, allowing argument-parameter validation during invocation passes.

---

## Author

**Josep Montoro Pascual**
