# C-Minus Compiler

[![Language](https://img.shields.io/badge/Language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Parser Generator](https://img.shields.io/badge/Tools-Flex%20%7C%20Bison-orange.svg)](https://github.com/westes/flex)
[![Build](https://img.shields.io/badge/Build-Make-green.svg)](https://www.gnu.org/software/make/)
[![Environment](https://img.shields.io/badge/Environment-Docker-2496ED.svg)](https://www.docker.com/)

A modular front-end compiler for the **C-Minus** programming language implemented in C.

The compiler translates C-Minus source code through:

- **Lexical analysis**
- **Syntax validation**
- **Abstract Syntax Tree (AST) construction**
- **Hierarchical semantic type analysis**

## Table of Contents

- [Quick Start](#quick-start)
- [Architecture & Compiler Pipeline](#architecture--compiler-pipeline)
- [The C-Minus Language](#the-c-minus-language)
- [Example Output](#example-output)
- [Project Structure](#project-structure)
- [Environment & Docker Setup](#environment--docker-setup)
- [Build & Execution](#build--execution)
- [Troubleshooting](#troubleshooting)
- [Technical Challenges & Solutions](#technical-challenges--solutions)
- [Author](#author)

---

## Quick Start

If you already have Docker installed (otherwise see [Environment & Docker Setup](#environment--docker-setup)):

```bash
git clone https://github.com/josepjp31/C-Minus-Compiler.git
cd C-Minus-Compiler

# Build the image and open a shell with the project mounted at /work
docker build -t cminus-compiler .
docker run --rm -it -v "$PWD":/work -w /work cminus-compiler

# Inside the container: build all phases and run the test suite
make all
./test.sh
```

Prefer a one-shot run without opening a shell?

```bash
docker run --rm -v "$PWD":/work -w /work cminus-compiler bash -c "make all && ./test.sh"
```

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

## The C-Minus Language

C-Minus is a small, C-like teaching language. This compiler recognizes the following lexical elements:

| Category | Elements |
|---|---|
| **Keywords** | `if`, `else`, `while`, `return`, `int`, `void` |
| **Arithmetic operators** | `+` `-` `*` `/` |
| **Relational operators** | `<` `<=` `>` `>=` `==` `!=` |
| **Other symbols** | `=` `;` `,` `(` `)` `[` `]` `{` `}` |
| **Identifiers / numbers** | `ID` (letters), `NUM` (digits) |
| **Comments** | `/* ... */` (multi-line, an unclosed comment is reported as an error) |
| **Built-in functions** | `input()`, `output()` |

A short sample program (Euclid's algorithm):

```c
int gcd (int u, int v)
{
    if (v == 0) return u;
    else return gcd(v, u - u / v * v);
}
```

---

## Example Output

Running the scanner (Phase 1) on a test file prints one line per token, preceded by its source line number:

```text
$ ./cminus_cimpl tests/1-lexer/test.1.txt

C-MINUS COMPILATION: test.1.txt
	4: reserved word: int
	4: ID, name= gcd
	4: (
	4: reserved word: int
	4: ID, name= u
	4: ,
	4: reserved word: int
	4: ID, name= v
	4: )
	5: {
	6: reserved word: if
	6: (
	6: ID, name= v
	6: ==
	6: NUM, val= 0
	6: )
	...
```

The parser prints the resulting AST and the semantic analyzer reports symbol-table and type errors with their line numbers.

---

## Project Structure

```text
.
├── Makefile                    # Multi-target build script with conditional macro dispatch
├── test.sh                     # Automated test runner with diff verification & color diagnostics
├── Dockerfile                  # Containerized build environment (Debian/GCC/Flex/Bison)
├── docs/                       # Project documentation
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

---

## Environment & Docker Setup

The compiler is meant to be run inside a **Docker container**, so the toolchain (GCC, Flex, Bison, Make) is identical on every machine. The only things you need to install on your computer are Docker and Git; everything else is defined in the `Dockerfile` of this repository.

| | |
|---|---|
| **Host OS** | Windows (via WSL 2), macOS or Linux |
| **Container image** | `cminus-compiler` (built from the `Dockerfile` in this repository) |
| **Toolchain inside the container** | `gcc`, `flex`, `bison`, `make` |

### 1. Install Docker

#### Windows (WSL 2)

Docker is installed inside WSL, so WSL has to be set up first.

**a) Install WSL with Ubuntu 22.04** (PowerShell as Administrator):

```powershell
wsl --install Ubuntu-22.04
```

Create the default UNIX user when prompted (it does not need to match your Windows username), then start WSL:

```powershell
wsl
```

**b) Install and enable Docker inside WSL:**

```bash
sudo apt-get update
sudo apt install docker.io git
sudo systemctl start docker
sudo systemctl enable docker
```

**c) Add your user to the `docker` group** (so `sudo` is not needed for every Docker command):

```bash
sudo usermod -aG docker $USER
exit
```

Then, back in PowerShell, restart WSL so the group change takes effect:

```powershell
wsl --shutdown
wsl
```

> From this point on, run all the commands inside the WSL terminal.
>
> Alternatively, you can install [Docker Desktop](https://www.docker.com/products/docker-desktop/) with its WSL 2 integration instead of steps b) and c).

#### macOS

```bash
brew install --cask docker
```

#### Linux

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

### 2. Clone the repository

```bash
git clone https://github.com/josepjp31/C-Minus-Compiler.git
cd C-Minus-Compiler
```

> **Windows users:** clone the repository inside the WSL filesystem (e.g. your home folder `~`), not under `/mnt/c/...`. This avoids line-ending (CRLF) problems with `test.sh` and the Makefile.

### 3. Build the Docker image

From the root of the repository (where the `Dockerfile` is):

```bash
docker build -t cminus-compiler .
```

### 4. Start the container

```bash
docker run --rm -it -v "$PWD":/work -w /work cminus-compiler
```

This opens a shell inside the container with the repository mounted at `/work`. Files are shared between your machine and the container, and `--rm` removes the container when you exit it (the image is kept, so the same command can be repeated at any time).

### 5. Build and run the project

Inside the container, follow [Build & Execution](#build--execution):

```bash
make all
./test.sh
```

---

## Build & Execution

> All commands below are executed **inside the Docker container** described above.

### Prerequisites

- `gcc`
- `flex`
- `bison`
- `make`

These are already provided by the Docker image; no manual installation is needed.

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

You can also compile your own C-Minus programs: put a `.cm` file anywhere inside the repository folder (it is mounted in the container) and pass its path to any of the executables.

#### Automated Test Suite

```bash
# Grant execution permissions (if not already set)
chmod +x test.sh

# Run all test cases with colored pass/fail status
./test.sh
```

The runner executes each phase on every test input under `tests/` and compares the output against the reference files with `diff`.

---

## Troubleshooting

| Problem | Solution |
|---|---|
| `permission denied while trying to connect to the Docker daemon socket` | Your user is not in the `docker` group yet. Run `sudo usermod -aG docker $USER`, then `wsl --shutdown` (Windows) or log out and back in (Linux). |
| `Cannot connect to the Docker daemon` | The service is not running. Start it with `sudo systemctl start docker` (or `sudo service docker start` if systemd is not enabled in your WSL). |
| `bad interpreter: /bin/bash^M` or `$'\r': command not found` when running `./test.sh` | The scripts have Windows (CRLF) line endings. Clone inside the WSL filesystem, or fix them with `sed -i 's/\r$//' test.sh`. |
| `./test.sh: Permission denied` | Run `chmod +x test.sh`. |
| Files created by the build are owned by `root` on the host | The container runs as root. Fix ownership with `sudo chown -R $USER:$USER .` |
| Outdated binaries after editing the sources | Run `make clean && make all`. |

---

## Technical Challenges & Solutions

- **Token String Race Conditions in Bison Actions:** Bison actions defer execution until a rule reduces, at which point global scanner buffers can be overwritten by lookahead tokens. Resolved by creating intermediate non-terminal reduction steps (`identifier: ID`) that immediately capture and allocate the lexeme string.

- **Grammar Shift/Reduce Warnings:** Rather than suppressing warnings via `%nonassoc`, the grammar was restructured into strict matched/unmatched statement pairs, yielding zero compiler warnings.

- **Function Signature Verification:** The symbol table entries store pointers back to their origin AST nodes, allowing argument-parameter validation during invocation passes.

---

## Author

**Josep Montoro Pascual** — [GitHub](https://github.com/josepjp31)
