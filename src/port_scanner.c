/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2025 Aerlync Labs Inc.
 */

/*
 * port_scanner.c - Network port scanning functionality
 * 
 * This reads /proc/net/tcp and /proc/net/udp to find active network connections.
 * We work directly with Linux's /proc filesystem.
 */

#include "port_scanner.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>  /* For inet_ntop - converts IP addresses */

/*
 * port_list_create - Allocate and initialize a new PortList
 * 
 * malloc() allocates memory on the "heap".
 * We must free() it later or we get a "memory leak"
 */
PortList* port_list_create(void) {
    PortList* list = malloc(sizeof(PortList));
    if (!list) return NULL;
    
    /* Start with capacity for 16 ports, will grow as needed */
    list->capacity = 16;
    list->count = 0;
    list->ports = malloc(sizeof(PortInfo) * list->capacity);
    
    if (!list->ports) {
        free(list);
        return NULL;
    }
    
    return list;
}

/*
 * port_list_destroy - Free all memory used by a PortList
 * 
 * Always free what you malloc! Prevents memory leaks.
 */
void port_list_destroy(PortList* list) {
    if (!list) return;
    free(list->ports);
    free(list);
}

/*
 * port_list_add - Add a port to the list (resize if needed)
 * 
 * realloc() resizes allocated memory.
 * This is how we make a "dynamic array" that grows.
 */
bool port_list_add(PortList* list, const PortInfo* port) {
    if (!list || !port) return false;
    
    /* Need to grow? Double the capacity */
    if (list->count >= list->capacity) {
        size_t new_capacity = list->capacity * 2;
        PortInfo* new_ports = realloc(list->ports, sizeof(PortInfo) * new_capacity);
        
        if (!new_ports) return false;
        
        list->ports = new_ports;
        list->capacity = new_capacity;
    }
    
    /* Copy the port info into our array */
    list->ports[list->count] = *port;
    list->count++;
    
    return true;
}

/*
 * parse_ip_address - Convert hex IP to readable format
 * 
 * IP addresses in /proc are in hex, little-endian format.
 */
static void parse_ip_address(const char* hex_ip, char* output, size_t output_size) {
    unsigned long ip_hex = parse_hex(hex_ip);
    
    /* For IPv4, convert to dotted decimal notation */
    struct in_addr addr;
    addr.s_addr = ip_hex;
    inet_ntop(AF_INET, &addr, output, output_size);
}

/*
 * parse_tcp_state - Convert state number to readable string
 * 
 * TCP connections have different states.
 * These numbers come from the Linux kernel.
 */
static void parse_tcp_state(const char* state_hex, char* output) {
    unsigned int state = (unsigned int)parse_hex(state_hex);
    
    switch(state) {
        case 0x01: strcpy(output, "ESTABLISHED"); break;
        case 0x02: strcpy(output, "SYN_SENT"); break;
        case 0x03: strcpy(output, "SYN_RECV"); break;
        case 0x04: strcpy(output, "FIN_WAIT1"); break;
        case 0x05: strcpy(output, "FIN_WAIT2"); break;
        case 0x06: strcpy(output, "TIME_WAIT"); break;
        case 0x07: strcpy(output, "CLOSE"); break;
        case 0x08: strcpy(output, "CLOSE_WAIT"); break;
        case 0x09: strcpy(output, "LAST_ACK"); break;
        case 0x0A: strcpy(output, "LISTEN"); break;
        case 0x0B: strcpy(output, "CLOSING"); break;
        default: strcpy(output, "UNKNOWN"); break;
    }
}

/*
 * scan_tcp_ports - Parse /proc/net/tcp for active TCP connections
 * 
 * /proc/net/tcp format:
 * sl  local_address rem_address   st tx_queue rx_queue tr tm->when retrnsmt   uid  timeout inode
 * 0: 0100007F:1F90 00000000:0000 0A 00000000:00000000 00:00000000 00000000  1000        0 12345
 */
bool scan_tcp_ports(PortList* list) {
    FILE* file = fopen("/proc/net/tcp", "r");
    if (!file) return false;
    
    char line[MAX_LINE_LENGTH];
    
    /* Skip header line */
    if (!read_line(file, line, sizeof(line))) {
        fclose(file);
        return false;
    }
    
    /* Parse each connection line */
    while (read_line(file, line, sizeof(line))) {
        PortInfo port;
        port.protocol = PROTOCOL_TCP;
        
        char local_addr[64], remote_addr[64], state_hex[8];
        unsigned long inode;
        
        /* Parse the line - sscanf reads formatted input */
        int parsed = sscanf(line, 
            "%*d: %63[0-9A-F]:%hX %63[0-9A-F]:%hX %7s %*X:%*X %*X:%*X %*X %*d %*d %lu",
            local_addr, &port.port, 
            remote_addr, &port.remote_port,
            state_hex, &inode);
        
        if (parsed >= 5) {
            /* Convert addresses to readable format */
            parse_ip_address(local_addr, port.local_address, sizeof(port.local_address));
            parse_ip_address(remote_addr, port.remote_address, sizeof(port.remote_address));
            parse_tcp_state(state_hex, port.state);
            port.inode = inode;
            
            port_list_add(list, &port);
        }
    }
    
    fclose(file);
    return true;
}

/*
 * scan_udp_ports - Parse /proc/net/udp for active UDP connections
 * 
 * Similar to TCP but UDP doesn't have connection states
 */
bool scan_udp_ports(PortList* list) {
    FILE* file = fopen("/proc/net/udp", "r");
    if (!file) return false;
    
    char line[MAX_LINE_LENGTH];
    
    /* Skip header line */
    if (!read_line(file, line, sizeof(line))) {
        fclose(file);
        return false;
    }
    
    /* Parse each connection line */
    while (read_line(file, line, sizeof(line))) {
        PortInfo port;
        port.protocol = PROTOCOL_UDP;
        strcpy(port.state, "LISTENING");  /* UDP is connectionless */
        
        char local_addr[64], remote_addr[64];
        unsigned long inode;
        
        int parsed = sscanf(line, 
            "%*d: %63[0-9A-F]:%hX %63[0-9A-F]:%hX %*s %*X:%*X %*X:%*X %*X %*d %*d %lu",
            local_addr, &port.port, 
            remote_addr, &port.remote_port, &inode);
        
        if (parsed >= 3) {
            parse_ip_address(local_addr, port.local_address, sizeof(port.local_address));
            parse_ip_address(remote_addr, port.remote_address, sizeof(port.remote_address));
            port.inode = inode;
            
            port_list_add(list, &port);
        }
    }
    
    fclose(file);
    return true;
}

/*
 * scan_all_ports - Scan both TCP and UDP ports
 */
PortList* scan_all_ports(void) {
    PortList* list = port_list_create();
    if (!list) return NULL;
    
    scan_tcp_ports(list);
    scan_udp_ports(list);
    
    return list;
}

/*
 * find_port_by_number - Find a specific port in the list
 */
PortInfo* find_port_by_number(PortList* list, unsigned short port_number) {
    if (!list) return NULL;
    
    for (size_t i = 0; i < list->count; i++) {
        if (list->ports[i].port == port_number) {
            return &list->ports[i];
        }
    }
    
    return NULL;
}

/*
 * is_port_available - Check if a port is not in use
 */
bool is_port_available(unsigned short port_number) {
    PortList* list = scan_all_ports();
    if (!list) return true;  /* Assume available if we can't scan */
    
    bool available = (find_port_by_number(list, port_number) == NULL);
    
    port_list_destroy(list);
    return available;
}
