/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2025 Aerlync Labs Inc.
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include "port_scanner.h"
#include "process_manager.h"
#include <stdbool.h>

/*
 * This header defines functions for displaying information to the user
 */

/* Display format options */
typedef enum {
	FORMAT_TABLE,       /* Pretty table format */
	FORMAT_JSON,        /* JSON format for scripting */
	FORMAT_SIMPLE       /* Simple line-by-line format */
} DisplayFormat;

/*
 * Function declarations
 */

/* Display a list of ports in a formatted table */
void display_port_list(PortList *list, DisplayFormat format);

/* Display a single port with its process information */
void display_port_details(const PortInfo *port, const ProcessInfo *process);

/* Display port availability check result */
void display_port_availability(unsigned short port, bool available);

/* Display an error message in a user-friendly way */
void display_error(const char *message);

/* Display a success message */
void display_success(const char *message);

/* Display a warning message */
void display_warning(const char *message);

/* Display help/usage information */
void display_help(const char *program_name);

/* Display version information */
void display_version(void);

#endif /* DISPLAY_H */
