/*
 * test_utils.c - Unit tests for utility functions
 */

#include "minunit.h"
#include "../include/utils.h"
#include <string.h>
#include <stdlib.h>

/*
 * Test: trim_whitespace
 */
static const char* test_trim_whitespace(void) {
    char test1[] = "  hello  ";
    char* result = trim_whitespace(test1);
    mu_assert_str_eq("trim_whitespace removes leading/trailing spaces", "hello", result);
    
    char test2[] = "no spaces";
    result = trim_whitespace(test2);
    mu_assert_str_eq("trim_whitespace handles no spaces", "no spaces", result);
    
    char test3[] = "   ";
    result = trim_whitespace(test3);
    mu_assert_str_eq("trim_whitespace handles all spaces", "", result);
    
    return NULL;
}

/*
 * Test: safe_string_copy
 */
static const char* test_safe_string_copy(void) {
    char dest[10];
    
    safe_string_copy(dest, "hello", sizeof(dest));
    mu_assert_str_eq("safe_string_copy normal case", "hello", dest);
    
    safe_string_copy(dest, "this is way too long", sizeof(dest));
    mu_assert("safe_string_copy truncates correctly", strlen(dest) < sizeof(dest));
    mu_assert("safe_string_copy null terminates", dest[sizeof(dest)-1] == '\0' || dest[strlen(dest)] == '\0');
    
    return NULL;
}

/*
 * Test: string_contains
 */
static const char* test_string_contains(void) {
    mu_assert("string_contains finds substring", 
              string_contains("hello world", "world") == true);
    
    mu_assert("string_contains case insensitive", 
              string_contains("Hello World", "WORLD") == true);
    
    mu_assert("string_contains returns false for non-match", 
              string_contains("hello", "goodbye") == false);
    
    mu_assert("string_contains handles NULL", 
              string_contains(NULL, "test") == false);
    
    return NULL;
}

/*
 * Test: is_numeric
 */
static const char* test_is_numeric(void) {
    mu_assert("is_numeric recognizes digits", is_numeric("12345") == true);
    mu_assert("is_numeric rejects letters", is_numeric("abc") == false);
    mu_assert("is_numeric rejects mixed", is_numeric("123abc") == false);
    mu_assert("is_numeric handles empty", is_numeric("") == false);
    mu_assert("is_numeric handles NULL", is_numeric(NULL) == false);
    
    return NULL;
}

/*
 * Test: parse_hex
 */
static const char* test_parse_hex(void) {
    mu_assert_eq("parse_hex converts 0x10", 16, (int)parse_hex("10"));
    mu_assert_eq("parse_hex converts 0xFF", 255, (int)parse_hex("FF"));
    mu_assert_eq("parse_hex converts 0x0", 0, (int)parse_hex("0"));
    mu_assert_eq("parse_hex handles lowercase", 255, (int)parse_hex("ff"));
    
    return NULL;
}

/*
 * Test: parse_port
 */
static const char* test_parse_port(void) {
    /* Port 80 in hex */
    mu_assert_eq("parse_port converts port 80", 80, (int)parse_port("50"));
    
    /* Port 443 in hex */
    mu_assert_eq("parse_port converts port 443", 443, (int)parse_port("1BB"));
    
    /* Port 8080 in hex */
    mu_assert_eq("parse_port converts port 8080", 8080, (int)parse_port("1F90"));
    
    return NULL;
}

/*
 * Run all utility tests
 */
static const char* all_tests(void) {
    mu_run_test(test_trim_whitespace);
    mu_run_test(test_safe_string_copy);
    mu_run_test(test_string_contains);
    mu_run_test(test_is_numeric);
    mu_run_test(test_parse_hex);
    mu_run_test(test_parse_port);
    
    return NULL;
}

int main(void) {
    printf("\n");
    printf("════════════════════════════════════════\n");
    printf("Running Utils Tests\n");
    printf("════════════════════════════════════════\n\n");
    
    const char* result = all_tests();
    
    print_test_summary();
    
    if (result != NULL) {
        return 1;
    }
    
    return tests_failed > 0 ? 1 : 0;
}
