/*
 * minunit.h - Minimal unit testing framework for C
 * 
 * Simple, lightweight testing framework with assertion macros.
 */

#ifndef MINUNIT_H
#define MINUNIT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Test statistics */
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

/* Color codes for output */
#define TEST_COLOR_GREEN  "\033[32m"
#define TEST_COLOR_RED    "\033[31m"
#define TEST_COLOR_YELLOW "\033[33m"
#define TEST_COLOR_RESET  "\033[0m"

/*
 * Assert macros
 */

#define mu_assert(message, test) do { \
    tests_run++; \
    if (!(test)) { \
        printf("%sFAIL:%s %s\n", TEST_COLOR_RED, TEST_COLOR_RESET, message); \
        tests_failed++; \
        return message; \
    } else { \
        printf("%sPASS:%s %s\n", TEST_COLOR_GREEN, TEST_COLOR_RESET, message); \
        tests_passed++; \
    } \
} while (0)

#define mu_assert_eq(message, expected, actual) do { \
    tests_run++; \
    if ((expected) != (actual)) { \
        printf("%sFAIL:%s %s (expected: %d, got: %d)\n", \
               TEST_COLOR_RED, TEST_COLOR_RESET, message, (int)(expected), (int)(actual)); \
        tests_failed++; \
        return message; \
    } else { \
        printf("%sPASS:%s %s\n", TEST_COLOR_GREEN, TEST_COLOR_RESET, message); \
        tests_passed++; \
    } \
} while (0)

#define mu_assert_str_eq(message, expected, actual) do { \
    tests_run++; \
    if (strcmp((expected), (actual)) != 0) { \
        printf("%sFAIL:%s %s (expected: \"%s\", got: \"%s\")\n", \
               TEST_COLOR_RED, TEST_COLOR_RESET, message, expected, actual); \
        tests_failed++; \
        return message; \
    } else { \
        printf("%sPASS:%s %s\n", TEST_COLOR_GREEN, TEST_COLOR_RESET, message); \
        tests_passed++; \
    } \
} while (0)

/*
 * Test runner macro
 */
#define mu_run_test(test) do { \
    const char* message = test(); \
    if (message) return message; \
} while (0)

/*
 * Print test summary
 */
static void print_test_summary(void) {
    printf("\n");
    printf("════════════════════════════════════════\n");
    printf("Tests run:    %d\n", tests_run);
    printf("%sPassed:       %d%s\n", TEST_COLOR_GREEN, tests_passed, TEST_COLOR_RESET);
    if (tests_failed > 0) {
        printf("%sFailed:       %d%s\n", TEST_COLOR_RED, tests_failed, TEST_COLOR_RESET);
    } else {
        printf("Failed:       %d\n", tests_failed);
    }
    printf("════════════════════════════════════════\n");
    
    if (tests_failed == 0 && tests_run > 0) {
        printf("%sAll tests passed!%s\n\n", TEST_COLOR_GREEN, TEST_COLOR_RESET);
    } else if (tests_failed > 0) {
        printf("%sSome tests failed!%s\n\n", TEST_COLOR_RED, TEST_COLOR_RESET);
    }
}

#endif /* MINUNIT_H */
