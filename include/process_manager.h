/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2025 Aerlync Labs Inc.
 */

#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include <sys/types.h>
#include <stdbool.h>

/*
 * This header defines functions for managing processes
 */

/*
 * Structure to hold process information
 */
typedef struct {
	pid_t pid;                  /* Process ID */
	char command[256];          /* Command name */
	char cmdline[512];          /* Full command line */
	char user[64];              /* Username running the process */
	unsigned long inode;        /* Socket inode (to match with ports) */
} ProcessInfo;

/*
 * Function declarations
 */

/* Find the process ID that owns a specific socket inode */
pid_t find_pid_by_inode(unsigned long inode);

/* Get detailed information about a process by PID */
bool get_process_info(pid_t pid, ProcessInfo *info);

/* Get the username of the user running a process */
bool get_process_user(pid_t pid, char *user_buf, size_t buf_size);

/* Get the command line of a process */
bool get_process_cmdline(pid_t pid, char *cmdline_buf, size_t buf_size);

/* Kill a process by PID (with confirmation) */
bool kill_process(pid_t pid, bool force);

#endif /* PROCESS_MANAGER_H */
