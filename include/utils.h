/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2025 Aerlync Labs Inc.
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

/*
 * Color codes for terminal output
 * These are ANSI escape sequences that make text colorful in the terminal
 */
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_BOLD    "\033[1m"

/* Maximum line length when reading files */
#define MAX_LINE_LENGTH 1024

/*
 * String manipulation functions
 */

/* Trims whitespace from both ends of a string */
char *trim_whitespace(char *str);

/* Safely copies a string with size limit (safer than strcpy) */
void safe_string_copy(char *dest, const char *src, size_t dest_size);

/* Checks if a string contains a substring (case-insensitive) */
bool string_contains(const char *haystack, const char *needle);

/*
 * File reading utilities
 */

/* Reads an entire file into a dynamically allocated string */
char *read_file_contents(const char *filepath);

/* Reads a single line from a file */
bool read_line(FILE *file, char *buffer, size_t buffer_size);

/*
 * Number parsing utilities
 */

/* Parses a hexadecimal string to unsigned long (for parsing /proc files) */
unsigned long parse_hex(const char *hex_str);

/* Checks if a string is a valid number */
bool is_numeric(const char *str);

/* Converts port number from network byte order to host byte order */
unsigned short parse_port(const char *port_hex);

/*
 * Error handling
 */

/* Prints an error message and exits the program */
void die(const char *message);

/* Prints an error message with errno details */
void die_errno(const char *message);

#endif /* UTILS_H */
