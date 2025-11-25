/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2025 Aerlync Labs Inc.
 */

/* POSIX feature test macro - enables strdup() */
#define _POSIX_C_SOURCE 200809L

/*
 * main.c - Entry point for portmap tool
 * 
 * This parses command-line arguments and calls the appropriate functions.
 * This is where execution starts - the main() function.
 */

#include "port_scanner.h"
#include "process_manager.h"
#include "display.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>   /* For strcasecmp */
#include <ctype.h>     /* For tolower, isalnum */
#include <unistd.h>    /* For getopt */

/*
 * Command types - what the user wants to do
 */
typedef enum {
    CMD_LIST,
    CMD_KILL,
    CMD_FIND,
    CMD_CHECK,
    CMD_HELP,
    CMD_VERSION
} Command;

/*
 * Options for filtering
 */
typedef struct {
    unsigned short port;        /* Specific port to filter */
    unsigned short range_low;   /* Port range (low) */
    unsigned short range_high;  /* Port range (high) */
    bool tcp_only;              /* Show only TCP */
    bool udp_only;              /* Show only UDP */
    bool force;                 /* Force kill */
    DisplayFormat format;       /* Output format */
} Options;

/*
 * filter_port_list - Filter ports based on options
 * 
 * We create a new list with only matching ports.
 */
static PortList* filter_port_list(PortList* original, Options* opts) {
    if (!opts->port && !opts->range_low && !opts->tcp_only && !opts->udp_only) {
        return original;  /* No filtering needed */
    }
    
    PortList* filtered = port_list_create();
    if (!filtered) return original;
    
    for (size_t i = 0; i < original->count; i++) {
        PortInfo* p = &original->ports[i];
        bool include = true;
        
        /* Filter by specific port */
        if (opts->port && p->port != opts->port) {
            include = false;
        }
        
        /* Filter by port range */
        if (opts->range_low && (p->port < opts->range_low || p->port > opts->range_high)) {
            include = false;
        }
        
        /* Filter by protocol */
        if (opts->tcp_only && p->protocol != PROTOCOL_TCP) {
            include = false;
        }
        if (opts->udp_only && p->protocol != PROTOCOL_UDP) {
            include = false;
        }
        
        if (include) {
            port_list_add(filtered, p);
        }
    }
    
    return filtered;
}

/*
 * cmd_list - Handle "list" command
 */
static int cmd_list(Options* opts) {
    PortList* list = scan_all_ports();
    if (!list) {
        display_error("Failed to scan ports");
        return 1;
    }
    
    PortList* filtered = filter_port_list(list, opts);
    display_port_list(filtered, opts->format);
    
    if (filtered != list) {
        port_list_destroy(filtered);
    }
    port_list_destroy(list);
    
    return 0;
}

/*
 * cmd_kill - Handle "kill" command
 */
static int cmd_kill(unsigned short port, bool force) {
    PortList* list = scan_all_ports();
    if (!list) {
        display_error("Failed to scan ports");
        return 1;
    }
    
    /* Find all connections on this port */
    int found_count = 0;
    pid_t pids[256];  /* Support up to 256 processes on same port */
    ProcessInfo procs[256];
    
    for (size_t i = 0; i < list->count; i++) {
        if (list->ports[i].port == port && found_count < 256) {
            pid_t pid = find_pid_by_inode(list->ports[i].inode);
            if (pid > 0) {
                /* Check if we already have this PID */
                bool duplicate = false;
                for (int j = 0; j < found_count; j++) {
                    if (pids[j] == pid) {
                        duplicate = true;
                        break;
                    }
                }
                
                if (!duplicate) {
                    pids[found_count] = pid;
                    if (get_process_info(pid, &procs[found_count])) {
                        found_count++;
                    }
                }
            }
        }
    }
    
    port_list_destroy(list);
    
    if (found_count == 0) {
        display_error("Port not in use");
        return 1;
    }
    
    /* Display all processes using this port */
    printf("Found %d process%s using port %u:\n", 
           found_count, found_count > 1 ? "es" : "", port);
    for (int i = 0; i < found_count; i++) {
        printf("  %d. %s (PID %d, User: %s)\n", 
               i + 1, procs[i].command, pids[i], procs[i].user);
    }
    
    /* Ask for confirmation unless force is used */
    if (!force) {
        printf("Kill %s process%s? [y/N] ", 
               found_count > 1 ? "all" : "this",
               found_count > 1 ? "es" : "");
        char response[10];
        if (fgets(response, sizeof(response), stdin)) {
            if (response[0] != 'y' && response[0] != 'Y') {
                printf("Cancelled.\n");
                return 0;
            }
        }
    }
    
    /* Kill all processes */
    int killed = 0;
    int failed = 0;
    for (int i = 0; i < found_count; i++) {
        if (kill_process(pids[i], force)) {
            printf("Killed PID %d (%s)\n", pids[i], procs[i].command);
            killed++;
        } else {
            printf("Failed to kill PID %d (%s)\n", pids[i], procs[i].command);
            failed++;
        }
    }
    
    if (killed > 0) {
        display_success("Successfully killed process(es)");
    }
    
    if (failed > 0) {
        display_error("Some processes could not be killed (try with sudo?)");
        return 1;
    }
    
    return 0;
}

/*
 * cmd_find - Handle "find" command
 */
static int cmd_find(const char* search_term) {
    PortList* list = scan_all_ports();
    if (!list) {
        display_error("Failed to scan ports");
        return 1;
    }
    
    PortList* filtered = port_list_create();
    
    for (size_t i = 0; i < list->count; i++) {
        PortInfo* p = &list->ports[i];
        pid_t pid = find_pid_by_inode(p->inode);
        
        if (pid > 0) {
            ProcessInfo proc;
            if (get_process_info(pid, &proc)) {
                bool match = false;
                
                /* Check for exact match in command name first */
                if (strcasecmp(proc.command, search_term) == 0) {
                    match = true;
                }
                
                /* If no exact match, check for word boundary match in cmdline */
                if (!match) {
                    char* cmdline_lower = strdup(proc.cmdline);
                    char* search_lower = strdup(search_term);
                    
                    if (cmdline_lower && search_lower) {
                        /* Convert both to lowercase */
                        for (char* p = cmdline_lower; *p; p++) *p = tolower(*p);
                        for (char* p = search_lower; *p; p++) *p = tolower(*p);
                        
                        /* Look for word boundaries */
                        char* pos = cmdline_lower;
                        while ((pos = strstr(pos, search_lower)) != NULL) {
                            /* Check if this is a word boundary */
                            bool start_ok = (pos == cmdline_lower || !isalnum(*(pos-1)));
                            bool end_ok = !isalnum(*(pos + strlen(search_lower)));
                            
                            if (start_ok && end_ok) {
                                match = true;
                                break;
                            }
                            pos++;
                        }
                    }
                    
                    free(cmdline_lower);
                    free(search_lower);
                }
                
                if (match) {
                    port_list_add(filtered, p);
                }
            }
        }
    }
    
    display_port_list(filtered, FORMAT_TABLE);
    
    port_list_destroy(filtered);
    port_list_destroy(list);
    
    return 0;
}

/*
 * cmd_check - Handle "check" command
 */
static int cmd_check(unsigned short port) {
    bool available = is_port_available(port);
    display_port_availability(port, available);
    
    return available ? 0 : 1;
}

/*
 * parse_port_range - Parse "low-high" format
 */
static bool parse_port_range(const char* range_str, unsigned short* low, unsigned short* high) {
    char* dash = strchr(range_str, '-');
    if (!dash) return false;
    
    *low = atoi(range_str);
    *high = atoi(dash + 1);
    
    return (*low > 0 && *high > 0 && *low <= *high);
}

/*
 * main - Entry point
 * 
 * Every C program starts executing here.
 * argc = argument count, argv = argument vector (array of strings)
 */
int main(int argc, char* argv[]) {
    /* Check if running with root privileges for some operations */
    /* Note: We don't require root for listing, only for killing */
    
    if (argc < 2) {
        display_help(argv[0]);
        return 1;
    }
    
    Command cmd;
    Options opts = {0};  /* Initialize all fields to 0 */
    opts.format = FORMAT_TABLE;
    
    /* Parse the command */
    const char* cmd_str = argv[1];
    if (strcmp(cmd_str, "list") == 0 || strcmp(cmd_str, "ls") == 0) {
        cmd = CMD_LIST;
    } else if (strcmp(cmd_str, "kill") == 0) {
        cmd = CMD_KILL;
    } else if (strcmp(cmd_str, "find") == 0) {
        cmd = CMD_FIND;
    } else if (strcmp(cmd_str, "check") == 0) {
        cmd = CMD_CHECK;
    } else if (strcmp(cmd_str, "help") == 0 || strcmp(cmd_str, "--help") == 0 || strcmp(cmd_str, "-h") == 0) {
        cmd = CMD_HELP;
    } else if (strcmp(cmd_str, "version") == 0 || strcmp(cmd_str, "--version") == 0) {
        cmd = CMD_VERSION;
    } else {
        display_error("Unknown command. Use 'portmap help' for usage.");
        return 1;
    }
    
    /* Parse options */
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            opts.port = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--range") == 0 && i + 1 < argc) {
            if (!parse_port_range(argv[++i], &opts.range_low, &opts.range_high)) {
                display_error("Invalid port range format. Use: --range 3000-4000");
                return 1;
            }
        } else if (strcmp(argv[i], "--tcp") == 0) {
            opts.tcp_only = true;
        } else if (strcmp(argv[i], "--udp") == 0) {
            opts.udp_only = true;
        } else if (strcmp(argv[i], "--json") == 0) {
            opts.format = FORMAT_JSON;
        } else if (strcmp(argv[i], "--force") == 0 || strcmp(argv[i], "-f") == 0) {
            opts.force = true;
        }
    }
    
    /* Execute command */
    switch (cmd) {
        case CMD_LIST:
            return cmd_list(&opts);
            
        case CMD_KILL:
            if (argc < 3 || !is_numeric(argv[2])) {
                display_error("Usage: portmap kill <port>");
                return 1;
            }
            return cmd_kill(atoi(argv[2]), opts.force);
            
        case CMD_FIND:
            if (argc < 3) {
                display_error("Usage: portmap find <process_name>");
                return 1;
            }
            return cmd_find(argv[2]);
            
        case CMD_CHECK:
            if (argc < 3 || !is_numeric(argv[2])) {
                display_error("Usage: portmap check <port>");
                return 1;
            }
            return cmd_check(atoi(argv[2]));
            
        case CMD_HELP:
            display_help(argv[0]);
            return 0;
            
        case CMD_VERSION:
            display_version();
            return 0;
            
        default:
            display_error("Unknown command");
            return 1;
    }
    
    return 0;
}
