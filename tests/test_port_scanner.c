/*
 * test_port_scanner.c - Unit tests for port scanning functions
 */

#include "minunit.h"
#include "../include/port_scanner.h"
#include <string.h>
#include <stdlib.h>

/*
 * Test: port_list_create
 */
static const char* test_port_list_create(void) {
    PortList* list = port_list_create();
    
    mu_assert("port_list_create returns non-NULL", list != NULL);
    mu_assert_eq("port_list_create initializes count to 0", 0, (int)list->count);
    mu_assert("port_list_create sets capacity > 0", list->capacity > 0);
    mu_assert("port_list_create allocates ports array", list->ports != NULL);
    
    port_list_destroy(list);
    return NULL;
}

/*
 * Test: port_list_add
 */
static const char* test_port_list_add(void) {
    PortList* list = port_list_create();
    
    PortInfo port;
    port.port = 8080;
    port.protocol = PROTOCOL_TCP;
    port.inode = 12345;
    strcpy(port.local_address, "127.0.0.1");
    strcpy(port.state, "LISTEN");
    
    bool success = port_list_add(list, &port);
    mu_assert("port_list_add succeeds", success == true);
    mu_assert_eq("port_list_add increments count", 1, (int)list->count);
    mu_assert_eq("port_list_add stores correct port", 8080, (int)list->ports[0].port);
    
    port_list_destroy(list);
    return NULL;
}

/*
 * Test: port_list_add with growth
 */
static const char* test_port_list_growth(void) {
    PortList* list = port_list_create();
    size_t initial_capacity = list->capacity;
    
    /* Add more ports than initial capacity to trigger growth */
    for (int i = 0; i < (int)initial_capacity + 5; i++) {
        PortInfo port;
        port.port = 3000 + i;
        port.protocol = PROTOCOL_TCP;
        port.inode = i;
        
        bool success = port_list_add(list, &port);
        mu_assert("port_list_add succeeds during growth", success == true);
    }
    
    mu_assert("port_list grows capacity", list->capacity > initial_capacity);
    mu_assert_eq("port_list maintains correct count after growth", 
                 (int)initial_capacity + 5, (int)list->count);
    
    port_list_destroy(list);
    return NULL;
}

/*
 * Test: find_port_by_number
 */
static const char* test_find_port_by_number(void) {
    PortList* list = port_list_create();
    
    /* Add some test ports */
    for (int i = 0; i < 5; i++) {
        PortInfo port;
        port.port = 8000 + i;
        port.protocol = PROTOCOL_TCP;
        port.inode = i;
        port_list_add(list, &port);
    }
    
    /* Find existing port */
    PortInfo* found = find_port_by_number(list, 8002);
    mu_assert("find_port_by_number finds existing port", found != NULL);
    mu_assert_eq("find_port_by_number returns correct port", 8002, (int)found->port);
    
    /* Try to find non-existing port */
    PortInfo* not_found = find_port_by_number(list, 9999);
    mu_assert("find_port_by_number returns NULL for non-existing", not_found == NULL);
    
    port_list_destroy(list);
    return NULL;
}

/*
 * Test: Protocol enum values
 */
static const char* test_protocol_enum(void) {
    mu_assert("PROTOCOL_TCP has expected value", PROTOCOL_TCP == 0);
    mu_assert("PROTOCOL_UDP has expected value", PROTOCOL_UDP == 1);
    
    return NULL;
}

/*
 * Run all port scanner tests
 */
static const char* all_tests(void) {
    mu_run_test(test_port_list_create);
    mu_run_test(test_port_list_add);
    mu_run_test(test_port_list_growth);
    mu_run_test(test_find_port_by_number);
    mu_run_test(test_protocol_enum);
    
    return NULL;
}

int main(void) {
    printf("\n");
    printf("════════════════════════════════════════\n");
    printf("Running Port Scanner Tests\n");
    printf("════════════════════════════════════════\n\n");
    
    const char* result = all_tests();
    
    print_test_summary();
    
    if (result != NULL) {
        return 1;
    }
    
    return tests_failed > 0 ? 1 : 0;
}
