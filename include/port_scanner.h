/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2025 Aerlync Labs Inc.
 */

#ifndef PORT_SCANNER_H
#define PORT_SCANNER_H

#include <stddef.h>
#include <stdbool.h>

/*
 * This header defines the data structures and functions for scanning network ports
 */

/* Maximum number of ports we can track at once */
#define MAX_PORTS 1024

/* Protocol types */
typedef enum {
	PROTOCOL_TCP,
	PROTOCOL_UDP,
	PROTOCOL_UNKNOWN
} Protocol;

/*
 * Structure to hold information about a single port connection
 * In C, we use 'struct' to group related data together
 */
typedef struct {
	unsigned short port;        /* Port number (e.g., 8080, 443) */
	Protocol protocol;          /* TCP or UDP */
	unsigned long inode;        /* Socket inode number (used to find process) */
	char local_address[46];     /* Local IP address (46 bytes for IPv6) */
	char remote_address[46];    /* Remote IP address */
	unsigned short remote_port; /* Remote port number */
	char state[16];             /* Connection state (LISTEN, ESTABLISHED, etc.) */
} PortInfo;

/*
 * Structure to hold information about all scanned ports
 * This is like a dynamic array that can grow
 */
typedef struct {
	PortInfo *ports;            /* Pointer to array of PortInfo structures */
	size_t count;               /* Number of ports currently stored */
	size_t capacity;            /* Maximum capacity before we need to resize */
} PortList;

/*
 * Function declarations
 */

/* Initialize a new PortList (allocates memory) */
PortList *port_list_create(void);

/* Free memory used by a PortList */
void port_list_destroy(PortList *list);

/* Add a port to the list (will resize if needed) */
bool port_list_add(PortList *list, const PortInfo *port);

/* Scan all active TCP ports and add them to the list */
bool scan_tcp_ports(PortList *list);

/* Scan all active UDP ports and add them to the list */
bool scan_udp_ports(PortList *list);

/* Scan all ports (both TCP and UDP) */
PortList *scan_all_ports(void);

/* Find a port by port number in the list */
PortInfo *find_port_by_number(PortList *list, unsigned short port_number);

/* Check if a specific port is available (not in use) */
bool is_port_available(unsigned short port_number);

#endif /* PORT_SCANNER_H */
