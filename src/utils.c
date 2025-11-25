/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2025 Aerlync Labs Inc.
 */

/*
 * utils.c - Utility functions for the portmap tool
 * 
 * This file contains helper functions used throughout the program.
 * Functions must be declared (in .h file) before they're used.
 */

#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

/*
 * trim_whitespace - Removes leading and trailing whitespace from a string
 * 
 * This modifies the string in place and returns a pointer to it.
 * In C, strings are just arrays of characters ending with '\0' (null terminator)
 */
char* trim_whitespace(char* str) {
    char* end;
    
    /* Trim leading spaces - move pointer forward */
    while(isspace((unsigned char)*str)) str++;
    
    /* If string is all spaces, return empty string */
    if(*str == 0) return str;
    
    /* Trim trailing spaces - find the end and work backwards */
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    
    /* Write new null terminator */
    end[1] = '\0';
    
    return str;
}

/*
 * safe_string_copy - Safely copy a string with size checking
 * 
 * strcpy is dangerous because it can overflow buffers.
 * Always use safe alternatives like this or strncpy.
 */
void safe_string_copy(char* dest, const char* src, size_t dest_size) {
    if (dest_size == 0) return;
    
    /* Copy at most dest_size-1 characters, leaving room for null terminator */
    size_t i;
    for (i = 0; i < dest_size - 1 && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';  /* Always null-terminate */
}

/*
 * string_contains - Check if haystack contains needle (case-insensitive)
 * 
 * We convert both strings to lowercase for comparison.
 * tolower() is from ctype.h
 */
bool string_contains(const char* haystack, const char* needle) {
    if (!haystack || !needle) return false;
    
    size_t hlen = strlen(haystack);
    size_t nlen = strlen(needle);
    
    if (nlen > hlen) return false;
    
    /* Check each position in haystack */
    for (size_t i = 0; i <= hlen - nlen; i++) {
        bool match = true;
        for (size_t j = 0; j < nlen; j++) {
            if (tolower(haystack[i + j]) != tolower(needle[j])) {
                match = false;
                break;
            }
        }
        if (match) return true;
    }
    
    return false;
}

/*
 * read_file_contents - Read entire file into a string
 * 
 * This allocates memory with malloc(). The caller MUST free() it!
 * This is called "dynamic memory allocation"
 */
char* read_file_contents(const char* filepath) {
    FILE* file = fopen(filepath, "r");
    if (!file) return NULL;
    
    /* Find file size */
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    /* Allocate memory (+1 for null terminator) */
    char* contents = malloc(size + 1);
    if (!contents) {
        fclose(file);
        return NULL;
    }
    
    /* Read file */
    size_t read_size = fread(contents, 1, size, file);
    contents[read_size] = '\0';
    
    fclose(file);
    return contents;
}

/*
 * read_line - Read a single line from a file
 * 
 * fgets reads up to n-1 characters or until newline.
 * Returns false if end of file or error.
 */
bool read_line(FILE* file, char* buffer, size_t buffer_size) {
    if (!fgets(buffer, buffer_size, file)) {
        return false;
    }
    
    /* Remove trailing newline if present */
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len-1] == '\n') {
        buffer[len-1] = '\0';
    }
    
    return true;
}

/*
 * parse_hex - Convert hexadecimal string to unsigned long
 * 
 * strtoul converts string to unsigned long.
 * Second parameter is where it stopped parsing (we don't need it, so NULL)
 * Third parameter is the base (16 for hexadecimal)
 */
unsigned long parse_hex(const char* hex_str) {
    return strtoul(hex_str, NULL, 16);
}

/*
 * is_numeric - Check if string contains only digits
 */
bool is_numeric(const char* str) {
    if (!str || *str == '\0') return false;
    
    while (*str) {
        if (!isdigit((unsigned char)*str)) return false;
        str++;
    }
    return true;
}

/*
 * parse_port - Convert hex port string to port number
 * 
 * Port numbers in /proc files are in hex and network byte order.
 * We convert to host byte order (the byte order your CPU uses)
 */
unsigned short parse_port(const char* port_hex) {
    unsigned long port = parse_hex(port_hex);
    /* Port is already in host byte order in /proc/net files */
    return (unsigned short)port;
}

/*
 * die - Print error message and exit
 * 
 * exit() terminates the program.
 * 1 means error, 0 means success (Unix convention)
 */
void die(const char* message) {
    fprintf(stderr, "%sError: %s%s\n", COLOR_RED, message, COLOR_RESET);
    exit(1);
}

/*
 * die_errno - Print error with system error message
 * 
 * errno is a global variable set by system calls when they fail.
 * strerror converts errno to a human-readable message.
 */
void die_errno(const char* message) {
    fprintf(stderr, "%sError: %s: %s%s\n", 
            COLOR_RED, message, strerror(errno), COLOR_RESET);
    exit(1);
}
