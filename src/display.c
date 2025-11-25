/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2025 Aerlync Labs Inc.
 */

/*
 * display.c - Output formatting and display functions
 *
 * This handles all user-facing output with colors and formatting.
 * printf formatting and ANSI color codes.
 */

#include "display.h"
#include "utils.h"
#include "process_manager.h"
#include <stdio.h>
#include <string.h>

#define VERSION "1.0.0"

/*
 * display_port_list - Show all ports in a nice table
 *
 * We calculate column widths and use printf width specifiers.
 */
void display_port_list(PortList *list, DisplayFormat format)
{
	if (!list || list->count == 0) {
		printf("No active network connections found.\n");
		return;
	}

	if (format == FORMAT_JSON) {
		/* JSON output for scripting */
		printf("[\n");
		for (size_t i = 0; i < list->count; i++) {
			PortInfo *p = &list->ports[i];
			ProcessInfo proc;
			pid_t pid = find_pid_by_inode(p->inode);

			printf("  {\n");
			printf("    \"port\": %u,\n", p->port);
			printf("    \"protocol\": \"%s\",\n",
			   p->protocol == PROTOCOL_TCP ? "TCP" : "UDP");
			printf("    \"local_address\": \"%s\",\n", p->local_address);
			printf("    \"state\": \"%s\"", p->state);

			if (pid > 0 && get_process_info(pid, &proc)) {
				printf(",\n    \"pid\": %d,\n", pid);
				printf("    \"process\": \"%s\",\n", proc.command);
				printf("    \"user\": \"%s\"\n", proc.user);
			} else {
				printf("\n");
			}

			printf("  }%s\n", i < list->count - 1 ? "," : "");
		}
		printf("]\n");
		return;
	}

	/* Table format */
	printf("\n");
	printf("%s%-8s %-10s %-20s %-15s %-12s %-20s%s\n",
	   COLOR_BOLD,
	   "PORT", "PROTOCOL", "ADDRESS", "STATE", "PID", "PROCESS/USER",
	   COLOR_RESET);
	printf("─────────────────────────────────────────────────────────────────────────────────\n");

	for (size_t i = 0; i < list->count; i++) {
		PortInfo *p = &list->ports[i];
		ProcessInfo proc;
		pid_t pid = find_pid_by_inode(p->inode);

		/* Color code by state */
		const char *color = COLOR_RESET;

		if (strcmp(p->state, "LISTEN") == 0 || strcmp(p->state, "LISTENING") == 0)
			color = COLOR_GREEN;
		else if (strcmp(p->state, "ESTABLISHED") == 0)
			color = COLOR_CYAN;

		printf("%s%-8u %-10s %-20s %-15s ",
			   color,
			   p->port,
			   p->protocol == PROTOCOL_TCP ? "TCP" : "UDP",
			   p->local_address,
			   p->state);

		if (pid > 0 && get_process_info(pid, &proc))
			printf("%-12d %s (%s)", pid, proc.command, proc.user);
		else
			printf("%-12s %s", "-", "-");

		printf("%s\n", COLOR_RESET);
	}

	printf("\n");
	printf("Total connections: %zu\n", (size_t)list->count);
}

/*
 * display_port_details - Show detailed info about a single port
 */
void display_port_details(const PortInfo *port, const ProcessInfo *process)
{
	if (!port)
		return;

	printf("\n");
	printf("%sPort Details:%s\n", COLOR_BOLD, COLOR_RESET);
	printf("  Port:          %u\n", port->port);
	printf("  Protocol:      %s\n", port->protocol == PROTOCOL_TCP ? "TCP" : "UDP");
	printf("  Local Address: %s\n", port->local_address);
	printf("  State:         %s\n", port->state);

	if (port->remote_port > 0)
		printf("  Remote:        %s:%u\n", port->remote_address, port->remote_port);

	if (process) {
		printf("\n%sProcess Information:%s\n", COLOR_BOLD, COLOR_RESET);
		printf("  PID:           %d\n", process->pid);
		printf("  Process:       %s\n", process->command);
		printf("  Command Line:  %s\n", process->cmdline);
		printf("  User:          %s\n", process->user);
	}

	printf("\n");
}

/*
 * display_port_availability - Show if a port is available
 */
void display_port_availability(unsigned short port, bool available)
{
	if (available)
		printf("%sPort %u is available%s\n",
		       COLOR_GREEN, port, COLOR_RESET);
	else
		printf("%sPort %u is in use%s\n",
		       COLOR_RED, port, COLOR_RESET);
}

/*
 * display_error - Show an error message
 */
void display_error(const char *message)
{
	fprintf(stderr, "%sError:%s %s\n", COLOR_RED, COLOR_RESET, message);
}

/*
 * display_success - Show a success message
 */
void display_success(const char *message)
{
	printf("%s%s%s\n", COLOR_GREEN, message, COLOR_RESET);
}

/*
 * display_warning - Show a warning message
 */
void display_warning(const char *message)
{
	printf("%sWarning: %s%s\n", COLOR_YELLOW, message, COLOR_RESET);
}

/*
 * display_help - Show usage information
 */
void display_help(const char *program_name)
{
	printf("\n");
	printf("%sportmap%s - Smart port manager for Linux\n", COLOR_BOLD, COLOR_RESET);
	printf("\nUsage:\n");
	printf("  %s %slist%s [options]          List all active ports\n",
	   program_name, COLOR_CYAN, COLOR_RESET);
	printf("  %s %skill%s <port>           Kill process using a port\n",
	   program_name, COLOR_CYAN, COLOR_RESET);
	printf("  %s %sfind%s <name>           Find ports by process name\n",
	   program_name, COLOR_CYAN, COLOR_RESET);
	printf("  %s %scheck%s <port>          Check if port is available\n",
	   program_name, COLOR_CYAN, COLOR_RESET);
	printf("  %s %shelp%s                  Show this help message\n",
	   program_name, COLOR_CYAN, COLOR_RESET);
	printf("  %s %sversion%s               Show version\n",
	   program_name, COLOR_CYAN, COLOR_RESET);

	printf("\nList Options:\n");
	printf("  --port <num>              Filter by specific port\n");
	printf("  --range <low>-<high>      Filter by port range\n");
	printf("  --tcp                     Show only TCP ports\n");
	printf("  --udp                     Show only UDP ports\n");
	printf("  --json                    Output in JSON format\n");

	printf("\nKill Options:\n");
	printf("  --force                   Force kill (SIGKILL instead of SIGTERM)\n");

	printf("\nExamples:\n");
	printf("  %s list\n", program_name);
	printf("  %s list --port 8080\n", program_name);
	printf("  %s kill 8080\n", program_name);
	printf("  %s find python\n", program_name);
	printf("  %s check 3000\n", program_name);
	printf("\n");
}

/*
 * display_version - Show version information
 */
void display_version(void)
{
	printf("portmap version %s\n", VERSION);
	printf("A smart port manager for Linux\n");
}
