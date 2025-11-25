/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2025 Aerlync Labs Inc.
 */

/*
 * process_manager.c - Process management and /proc filesystem interaction
 *
 * This finds which process owns a network connection by matching socket inodes.
 * We read /proc/[pid]/fd/ to find file descriptors and match them.
 */

/* POSIX feature test macro - enables readlink() and kill() */
#define _POSIX_C_SOURCE 200809L

#include "process_manager.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>     /* For reading directories */
#include <unistd.h>     /* For readlink, getuid */
#include <signal.h>     /* For kill(), SIGTERM, SIGKILL */
#include <sys/stat.h>   /* For stat() */
#include <pwd.h>        /* For getpwuid() - getting username */

/*
 * find_pid_by_inode - Find which process owns a socket
 *
 * We iterate through /proc/[pid]/fd/ looking for socket:[inode]
 * This is a bit complex but very useful for system programming!
 */
pid_t find_pid_by_inode(unsigned long inode) 
{
	DIR *proc_dir = opendir("/proc");

	if (!proc_dir)
		return -1;

	struct dirent *entry;
	char inode_str[32];

	snprintf(inode_str, sizeof(inode_str), "socket:[%lu]", inode);

	/* Go through each directory in /proc */
	while ((entry = readdir(proc_dir)) != NULL) {
		/* Skip non-numeric directories (only PIDs are numeric) */
		if (!is_numeric(entry->d_name))
			continue;

		pid_t pid = atoi(entry->d_name);

		/* Check this process's file descriptors */
		char fd_path[256];

		snprintf(fd_path, sizeof(fd_path), "/proc/%d/fd", pid);

		DIR *fd_dir = opendir(fd_path);

		if (!fd_dir) continue;  /* Process might have exited or no permission */

		struct dirent *fd_entry;

		while ((fd_entry = readdir(fd_dir)) != NULL) {
		if (fd_entry->d_name[0] == '.')
			continue;

		    /* Build path to file descriptor symlink */
		    char fd_link[512];

		    snprintf(fd_link, sizeof(fd_link), "/proc/%d/fd/%s", pid, fd_entry->d_name);

		    /* Read where the symlink points to */
		    char link_target[256];
		ssize_t len = readlink(fd_link, link_target, sizeof(link_target) - 1);

		if (len > 0) {
			link_target[len] = '\0';

			/* Does it match our socket inode? */
			if (strcmp(link_target, inode_str) == 0) {
			    closedir(fd_dir);
			    closedir(proc_dir);
				return pid;
			}
		}
		}

		closedir(fd_dir);
	}

	closedir(proc_dir);
	return -1;  /* Not found */
}

/*
 * get_process_cmdline - Get the command line of a process
 *
 * /proc/[pid]/cmdline has null-separated arguments.
 * We convert nulls to spaces for readability.
 */
bool get_process_cmdline(pid_t pid, char *cmdline_buf, size_t buf_size) 
{
	char path[256];

	snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);

	FILE *file = fopen(path, "r");

	if (!file)
		return false;

	size_t bytes_read = fread(cmdline_buf, 1, buf_size - 1, file);

	cmdline_buf[bytes_read] = '\0';

	/* Replace null bytes with spaces for readability */
	for (size_t i = 0; i < bytes_read - 1; i++) {
		if (cmdline_buf[i] == '\0') {
		    cmdline_buf[i] = ' ';
		}
	}

	/* If empty, try reading comm (process name) instead */
	if (bytes_read == 0 || cmdline_buf[0] == '\0') {
		fclose(file);
		snprintf(path, sizeof(path), "/proc/%d/comm", pid);
		file = fopen(path, "r");
		if (!file)
			return false;

		if (fgets(cmdline_buf, buf_size, file)) {
		    /* Remove trailing newline */
		size_t len = strlen(cmdline_buf);

		if (len > 0 && cmdline_buf[len-1] == '\n') {
			cmdline_buf[len-1] = '\0';
		}
		}
	}

	fclose(file);
	return true;
}

/*
 * get_process_user - Get the username of the process owner
 *
 * We read /proc/[pid]/status to find the UID,
 * then use getpwuid to convert UID to username.
 */
bool get_process_user(pid_t pid, char *user_buf, size_t buf_size) 
{
	char path[256];

	snprintf(path, sizeof(path), "/proc/%d/status", pid);

	FILE *file = fopen(path, "r");

	if (!file)
		return false;

	char line[256];
	uid_t uid = 0;

	/* Look for "Uid:" line in status file */
	while (fgets(line, sizeof(line), file)) {
		if (strncmp(line, "Uid:", 4) == 0) {
		    /* Parse: Uid: real effective saved fsuid */
		    sscanf(line + 4, "%u", &uid);
		break;
		}
	}

	fclose(file);

	/* Convert UID to username */
	struct passwd *pw = getpwuid(uid);

	if (pw) {
		safe_string_copy(user_buf, pw->pw_name, buf_size);
		return true;
	}

	/* If we can't get username, just show UID */
	snprintf(user_buf, buf_size, "%u", uid);
	return true;
}

/*
 * get_process_info - Get all information about a process
 */
bool get_process_info(pid_t pid, ProcessInfo *info) 
{
	if (!info)
		return false;

	info->pid = pid;

	if (!get_process_cmdline(pid, info->cmdline, sizeof(info->cmdline))) {
		return false;
	}

	if (!get_process_user(pid, info->user, sizeof(info->user))) {
		strcpy(info->user, "unknown");
	}

	/* Extract just the command name from cmdline */
	char *space = strchr(info->cmdline, ' ');

	if (space) {
		size_t cmd_len = space - info->cmdline;

		if (cmd_len >= sizeof(info->command)) {
		    cmd_len = sizeof(info->command) - 1;
		}
		memcpy(info->command, info->cmdline, cmd_len);
		info->command[cmd_len] = '\0';
	} else {
		safe_string_copy(info->command, info->cmdline, sizeof(info->command));
	}

	/* Remove path, keep just the executable name */
	char *slash = strrchr(info->command, '/');

	if (slash) {
		memmove(info->command, slash + 1, strlen(slash + 1) + 1);
	}

	return true;
}

/*
 * kill_process - Send kill signal to a process
 *
 * kill() sends signals to processes.
 * SIGTERM (15) asks nicely, SIGKILL (9) forces termination.
 */
bool kill_process(pid_t pid, bool force) 
{
	int signal = force ? SIGKILL : SIGTERM;

	if (kill(pid, signal) == 0) {
		return true;
	}

	return false;
}
