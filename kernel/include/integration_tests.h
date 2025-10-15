/*
 * Integration Test Suite Header
 * 
 * Public API for running integration tests
 */

#ifndef INTEGRATION_TESTS_H
#define INTEGRATION_TESTS_H

// Run complete integration test suite
// Returns 1 if all tests pass, 0 otherwise
int run_integration_tests(void);

// Run quick smoke test
// Returns 1 if smoke test passes, 0 otherwise
int run_smoke_test(void);

#endif // INTEGRATION_TESTS_H
