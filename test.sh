#!/usr/bin/env bash
# ==============================================================================
# Automated validation script for the C-Minus compiler
# Runs build and tests for the Scanner, Parser, and Semantic Analyzer
# ==============================================================================

set -o pipefail

# Colors for terminal output
GREEN="\033[0;32m"
RED="\033[0;31m"
BLUE="\033[1;34m"
NC="\033[0m"

TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0
TMP_OUT=$(mktemp)

cleanup() {
    rm -f "$TMP_OUT"
}
trap cleanup EXIT

print_status() {
    local test_name="$1"
    local status="$2"
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    if [ "$status" -eq 0 ]; then
        echo -e "  [${GREEN}PASS${NC}] $test_name"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        echo -e "  [${RED}FAIL${NC}] $test_name"
        FAILED_TESTS=$((FAILED_TESTS + 1))
    fi
}

echo -e "\n${BLUE}=== 1. Project build ===${NC}"
make clean > /dev/null 2>&1
if make all > /dev/null 2>&1; then
    echo -e "  [${GREEN}PASS${NC}] 'make all' completed successfully."
else
    echo -e "  [${RED}FAIL${NC}] Error during compilation with 'make all'."
    exit 1
fi

# Normalize Windows line endings (CRLF -> LF) in test suites if they exist
find tests/ -type f \( -name "*.txt" -o -name "*.cm" \) -exec sed -i 's/\r$//' {} + 2>/dev/null

echo -e "\n${BLUE}=== 2. Phase 1 validation: Scanner (cminus_cimpl) ===${NC}"

# Lexer Test 1
./cminus_cimpl tests/1-lexer/test.1.txt > "$TMP_OUT" 2>&1
diff -u <(tail -n +3 tests/1-lexer/result.1.txt) <(tail -n +3 "$TMP_OUT") > /dev/null 2>&1
print_status "Phase 1: tests/1-lexer/test.1.txt vs result.1.txt" $?

# Lexer Test 2
./cminus_cimpl tests/1-lexer/test.2.txt > "$TMP_OUT" 2>&1
diff -u <(tail -n +3 tests/1-lexer/result.2.txt) <(tail -n +3 "$TMP_OUT") > /dev/null 2>&1
print_status "Phase 1: tests/1-lexer/test.2.txt vs result.2.txt" $?


echo -e "\n${BLUE}=== 3. Phase 2 validation: Parser (cminus_parser) ===${NC}"

# Parser Test 1
./cminus_parser tests/2-parser/test.1.txt > "$TMP_OUT" 2>&1
diff -u <(tail -n +3 tests/2-parser/result.1.txt) <(tail -n +3 "$TMP_OUT") > /dev/null 2>&1
print_status "Phase 2: tests/2-parser/test.1.txt vs result.1.txt" $?

# Parser Test 2
./cminus_parser tests/2-parser/test.2.txt > "$TMP_OUT" 2>&1
diff -u <(tail -n +3 tests/2-parser/result.2.txt) <(tail -n +3 "$TMP_OUT") > /dev/null 2>&1
print_status "Phase 2: tests/2-parser/test.2.txt vs result.2.txt" $?


echo -e "\n${BLUE}=== 4. Phase 3 validation: Semantic analysis (cminus_semantic) ===${NC}"

# Semantic Test 1: Valid case (must finish type checking without error)
./cminus_semantic tests/3-semantic/test_1.cm > "$TMP_OUT" 2>&1
grep -q "Type Checking Finished" "$TMP_OUT" && ! grep -qi "Error:" "$TMP_OUT"
print_status "Phase 3: test_1.cm (valid gcd/IO case)" $?

# Semantic Test 2: Valid case with arrays and scopes
./cminus_semantic tests/3-semantic/test_2.cm > "$TMP_OUT" 2>&1
grep -q "Type Checking Finished" "$TMP_OUT" && ! grep -qi "Error:" "$TMP_OUT"
print_status "Phase 3: test_2.cm (valid scopes and arrays case)" $?

# Semantic Test 3: Error case (invalid call)
./cminus_semantic tests/3-semantic/test_3.cm > "$TMP_OUT" 2>&1
grep -q "Error: Invalid function call" "$TMP_OUT"
print_status "Phase 3: test_3.cm (invalid call detection)" $?

# Semantic Test 4: Error case (non-integer indexing)
./cminus_semantic tests/3-semantic/test_4.cm > "$TMP_OUT" 2>&1
grep -q "Error: Invalid array indexing" "$TMP_OUT"
print_status "Phase 3: test_4.cm (invalid index detection)" $?


echo -e "\n${BLUE}=== Test Summary ===${NC}"
echo -e "Total: $TOTAL_TESTS | Passed: ${GREEN}$PASSED_TESTS${NC} | Failed: ${RED}$FAILED_TESTS${NC}"

if [ "$FAILED_TESTS" -eq 0 ]; then
    echo -e "${GREEN}All compiler phases passed successfully.${NC}\n"
    exit 0
else
    echo -e "${RED}Failures were detected in one or more phases.${NC}\n"
    exit 1
fi
