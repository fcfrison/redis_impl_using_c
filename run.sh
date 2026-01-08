#!/usr/bin/bash
export INT_TEST_DIR="tests/integration"
gcc -o $INT_TEST_DIR/test_set_integration $INT_TEST_DIR/test_set_integration.c && /
$INT_TEST_DIR/test_set_integration
