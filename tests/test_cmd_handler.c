#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cmd_handler.h"
#include "../include/protocol.h"

void test_is_set_option_valid() {
    char state;

    // Test Case 1
    state = 0b00000000;
    assert(is_set_option_valid(&state, 7) == 1);

    // Test Case 2
    state = 0b00000000;
    assert(is_set_option_valid(&state, 9) == 0);

    // Test Case 3
    state = 0b00000000;
    assert(is_set_option_valid(&state, -2) == 0);

    // Test Case 4
    state = 0b01110000;
    assert(is_set_option_valid(&state, 3) == 1);

    // Test Case 5
    state = 0b01111000;
    assert(is_set_option_valid(&state, 3) == 0);

    // Test Case 6
    state = 0b01110100;
    assert(is_set_option_valid(&state, 3) == 0);

    // Test Case 7
    state = 0b01110000;
    assert(is_set_option_valid(&state, 2) == 1);
}

// Helper function to create a GenericNode
GenericNode* create_generic_node(RedisDtype type) {
    GenericNode* node = (GenericNode*)malloc(sizeof(GenericNode));
    node->node = (struct BaseNode*)malloc(sizeof(struct BaseNode));
    node->node->type = type;
    node->node->next = NULL;
    node->node->prev = NULL;
    return node;
}

// Test cases
void test_handle_set_options_valid() {
    char state = 0b01110000; // SET_BASIC state
    GenericNode* parsed_cmd[4] = {NULL, NULL, NULL, NULL};
    GenericNode* gnode = create_generic_node(BULK_STR);
    GenericNode* prev_gnode = gnode;
    handle_set_options(&state, parsed_cmd, &gnode, 2); // Set bit 2 (EX/PX)

    assert(state == 0b01110100); // State should be SET_GET
    assert(parsed_cmd[2] == prev_gnode); // gnode should be stored in parsed_cmd[2]
    assert(gnode == NULL); // gnode should advance to NULL (no next node)
    if(gnode && gnode->node){
        free(gnode->node);
    }
    if(gnode){
        free(gnode);
    }
}
// Test cases
void test_handle_set_options_valid_one() {
    char state = 0b01110000; // SET_BASIC state
    GenericNode* parsed_cmd[4] = {NULL, NULL, NULL, NULL};
    GenericNode* gnode = create_generic_node(BULK_STR);
    GenericNode* prev_gnode_zero = gnode;
    handle_set_options(&state, parsed_cmd, &gnode, 3); // Set bit 3 (NX/XX)
    gnode = create_generic_node(BULK_STR);
    GenericNode* prev_gnode_one = gnode;
    handle_set_options(&state, parsed_cmd, &gnode, 2); // Set bit 2 (GET)
    assert(state == 0b01111100); // State should be SET_GET
    assert(parsed_cmd[3] == prev_gnode_zero); // gnode should be stored in parsed_cmd[2]
    assert(parsed_cmd[2] == prev_gnode_one);
    assert(gnode == NULL); // gnode should advance to NULL (no next node)
    if(gnode && gnode->node){
        free(gnode->node);
    }
    if(gnode){
        free(gnode);
    }
}

void test_handle_set_options_invalid_bit_pos() {
    char state = 0b01110000; // SET_BASIC state
    GenericNode* parsed_cmd[4] = {NULL, NULL, NULL, NULL};
    GenericNode* gnode = create_generic_node(BULK_STR);

    handle_set_options(&state, parsed_cmd, &gnode, 5); // Invalid bit_pos

    assert(state == -1); // State should be -1 (error)
    assert(parsed_cmd[0] == NULL); // parsed_cmd should remain unchanged
    assert(gnode != NULL); // gnode should not advance
    
    if(gnode && gnode->node){
        free(gnode->node);
    }
    if(gnode){
        free(gnode);
    }
}

void test_handle_set_options_bit_already_set() {
    char state = 0b01110100; // SET_GET state (bit 2 already set)
    GenericNode* parsed_cmd[4] = {NULL, NULL, NULL, NULL};
    GenericNode* gnode = create_generic_node(BULK_STR);

    handle_set_options(&state, parsed_cmd, &gnode, 2); // Try to set bit 2 again

    assert(state == -1); // State should be -1 (error)
    assert(parsed_cmd[2] == NULL); // parsed_cmd should remain unchanged
    assert(gnode != NULL); // gnode should not advance

    if(gnode && gnode->node){
        free(gnode->node);
    }
    if(gnode){
        free(gnode);
    }
}

void test_handle_set_options_gnode_null() {
    char state = 0b01110000; // SET_BASIC state
    GenericNode* parsed_cmd[4] = {NULL, NULL, NULL, NULL};
    GenericNode* gnode = NULL; // gnode is NULL

    handle_set_options(&state, parsed_cmd, &gnode, 2); // Try to set bit 2

    assert(state == -1); // State should be -1 (error)
    assert(parsed_cmd[2] == NULL); // parsed_cmd should remain unchanged
    assert(gnode == NULL); // gnode should remain NULL

    // No need to free gnode since it's NULL
}

void test_handle_set_options_parsed_cmd_null() {
    char state = 0b01110000; // SET_BASIC state
    GenericNode** parsed_cmd = NULL; // parsed_cmd is NULL
    GenericNode* gnode = create_generic_node(BULK_STR);

    handle_set_options(&state, parsed_cmd, &gnode, 2); // Try to set bit 2

    assert(state == -1); // State should be -1 (error)
    assert(gnode != NULL); // gnode should not advance

    if(gnode && gnode->node){
        free(gnode->node);
    }
    if(gnode){
        free(gnode);
    }
}

void test_handle_set_options_state_null() {
    char* state = NULL; // state is NULL
    GenericNode* parsed_cmd[4] = {NULL, NULL, NULL, NULL};
    GenericNode* gnode = create_generic_node(BULK_STR);

    handle_set_options(state, parsed_cmd, &gnode, 2); // Try to set bit 2

    // No assertions since state is NULL (undefined behavior)
    // This test is to ensure the function doesn't crash

    if(gnode && gnode->node){
        free(gnode->node);
    }
    if(gnode){
        free(gnode);
    }
}

// Helper function to create a GenericNode
GenericNode* create_generic_node_w_next(RedisDtype type, GenericNode* next) {
    GenericNode* node = (GenericNode*)malloc(sizeof(GenericNode));
    node->node = (struct BaseNode*)malloc(sizeof(struct BaseNode));
    node->node->type = type;
    node->node->next = next;
    node->node->prev = NULL;
    return node;
}
// Test cases
void test_set_cmd_stage_a_valid() {
    GenericNode* node2 = create_generic_node_w_next(BULK_STR, NULL);
    GenericNode* node1 = create_generic_node_w_next(BULK_STR, node2);
    GenericNode* parsed_cmd[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
    char state = 0;

    GenericNode* result = set_cmd_stage_a(node1, &state, parsed_cmd);

    assert(state == 0b01110000); // Final state should be 0b01110000
    assert(parsed_cmd[5] == node1); // node1 should be stored in parsed_cmd[5]
    assert(parsed_cmd[4] == node2); // node2 should be stored in parsed_cmd[4]
    assert(result == NULL); // result should be NULL (end of nodes)

    if(node1 && node1->node){
        free(node1->node);
    }
    if(node1){
        free(node1);
    }
    if(node2 && node2->node){
        free(node2->node);
    }
    if(node2){
        free(node2);
    }
}

void test_set_cmd_stage_a_invalid_gnode_null() {
    GenericNode* gnode = NULL;
    GenericNode* parsed_cmd[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
    char state = 0;

    GenericNode* result = set_cmd_stage_a(gnode, &state, parsed_cmd);

    assert(state == -1); // State should be -1 (error)
    assert(result == NULL); // result should be NULL
    assert(parsed_cmd[5] == NULL); // parsed_cmd should remain unchanged
}

void test_set_cmd_stage_a_invalid_parsed_cmd_null() {
    GenericNode* node1 = create_generic_node_w_next(BULK_STR, NULL);
    GenericNode** parsed_cmd = NULL;
    char state = 0;

    GenericNode* result = set_cmd_stage_a(node1, &state, parsed_cmd);

    assert(state == -1); // State should be -1 (error)
    assert(result == NULL); // result should be NULL

    free(node1->node);
    free(node1);
}

void test_set_cmd_stage_a_invalid_node_type() {
    GenericNode* node2 = create_generic_node_w_next(UNDEF_DTYPE, NULL);
    GenericNode* node1 = create_generic_node_w_next(BULK_STR, node2);
    GenericNode* parsed_cmd[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
    char state = 0;

    GenericNode* result = set_cmd_stage_a(node1, &state, parsed_cmd);

    assert(state == -1); // State should be -1 (error)
    assert(parsed_cmd[5] == node1); // node1 should be stored in parsed_cmd[5]
    assert(parsed_cmd[4] == NULL); // parsed_cmd[4] should remain unchanged
    assert(result == NULL); // result should be NULL

    if(node1 && node1->node){
        free(node1->node);
    }
    if(node1){
        free(node1);
    }
    if(node2 && node2->node){
        free(node2->node);
    }
    if(node2){
        free(node2);
    }
}

void test_set_cmd_stage_a_invalid_node_next_null() {
    GenericNode* node1 = create_generic_node_w_next(BULK_STR, NULL);
    GenericNode* parsed_cmd[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
    char state = 0;

    GenericNode* result = set_cmd_stage_a(node1, &state, parsed_cmd);

    assert(state == -1); // State should be -1 (error)
    assert(parsed_cmd[5] == NULL); // node1 should be stored in parsed_cmd[5]
    assert(parsed_cmd[4] == NULL); // parsed_cmd[4] should remain unchanged
    assert(result == NULL); // result should be NULL

    if(node1 && node1->node){
        free(node1->node);
    }
    if(node1){
        free(node1);
    }
    
}

void test_set_cmd_stage_a_state_null() {
    GenericNode* node1 = create_generic_node_w_next(BULK_STR, NULL);
    GenericNode* parsed_cmd[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
    char* state = NULL;

    GenericNode* result = set_cmd_stage_a(node1, state, parsed_cmd);

    assert(!state);
    assert(!result);

    if(node1 && node1->node){
        free(node1->node);
    }
    if(node1){
        free(node1);
    }
}
// Helper function to create a BulkStringNode
BulkStringNode* create_bulk_string_node(const char* content, GenericNode* next) {
    BulkStringNode* node  = (BulkStringNode*)calloc(1,sizeof(BulkStringNode));
    BaseNode*       bnode = (BaseNode*)calloc(1,sizeof(BaseNode));
    bnode->type = BULK_STR;
    bnode->next = next;
    bnode->prev = NULL;
    node->node  = bnode;
    node->content = strdup(content);
    return node;
}
// Basic unit tests
void test_validate_set_cmd_null_state() {
    GenericNode* node = create_generic_node_w_next(BULK_STR, NULL);
    GenericNode** parsed_cmd = NULL;
    char* state = NULL;

    void* result = validate_set_cmd(node, state, &parsed_cmd);

    assert(result == NULL); // Function should return NULL for NULL state

    free(node->node);
    free(node);
}
GenericNode** 
gen_parsed_cmd_arr(unsigned int size){
    GenericNode** parsed_cmd = (GenericNode**)calloc(size,sizeof(GenericNode*));
    for (size_t i = 0; i < size; i++){
        parsed_cmd[i] = (GenericNode*)calloc(1,sizeof(GenericNode));
    }
    return parsed_cmd;
}
void
free_parsed_cmd_arr(GenericNode** parsed_cmd, int size){
    for (size_t i = 0; i < size; i++){
        free(parsed_cmd[i]);
    }
}
void test_validate_set_cmd_null_node() {
    GenericNode** parsed_cmd = NULL;
    char state = 0;

    void* result = validate_set_cmd(NULL, &state, &parsed_cmd);

    assert(result == NULL); // Function should return NULL for NULL node
}

void test_validate_set_cmd_invalid_node_type() {
    GenericNode* node = create_generic_node_w_next(UNDEF_DTYPE, NULL);
    GenericNode** parsed_cmd;
    char state = 0;
    void* result = validate_set_cmd(node, &state, &parsed_cmd);
    assert(result == NULL); // Function should return NULL for invalid node type   
    free(node->node);
    free(node);
}

void test_validate_set_cmd_memory_allocation_failure() {
    GenericNode* node = create_generic_node_w_next(BULK_STR, NULL);
    GenericNode** parsed_cmd = NULL;
    char state = 0;

    // Simulate memory allocation failure
    void* result = validate_set_cmd(node, &state, &parsed_cmd);

    assert(result == NULL); // Function should return NULL if memory allocation fails

    free(node->node);
    free(node);
}

void test_validate_set_cmd_valid_set_key_value() {
    GenericNode* value_node = (GenericNode*)create_bulk_string_node("value", NULL);
    GenericNode* key_node = (GenericNode*)create_bulk_string_node("key", value_node);
    GenericNode** parsed_cmd = NULL;
    char state = 0;

    void* result = validate_set_cmd(key_node, &state, &parsed_cmd);

    assert(result != NULL); // Function should return non-NULL for valid SET command
    assert(state == 0b01110000); // Final state should be 0b01110000
    assert(parsed_cmd != NULL); // parsed_cmd should be allocated

    free(key_node->node);
    free(key_node);
    free(value_node->node);
    free(value_node);
    free(parsed_cmd);
}

void test_validate_set_cmd_valid_set_key_value_nx() {
    GenericNode* nx_node    = (GenericNode*)create_bulk_string_node("NX", NULL);
    GenericNode* value_node = (GenericNode*)create_bulk_string_node("value", nx_node);
    GenericNode* key_node   = (GenericNode*)create_bulk_string_node("key", value_node);
    GenericNode** parsed_cmd = NULL;
    char state = 0;

    void* result = validate_set_cmd(key_node, &state, &parsed_cmd);

    assert(result != NULL); // Function should return non-NULL for valid SET command with NX
    assert(state == 0b01111000); // Final state should be 0b01111000
    assert(parsed_cmd != NULL); // parsed_cmd should be allocated

    free(key_node->node);
    free(key_node);
    free(value_node->node);
    free(value_node);
    free(nx_node->node);
    free(nx_node);
    free(parsed_cmd);
}
void test_validate_set_cmd_valid_set_key_value_ex() {
    GenericNode* ex_node_val    = (GenericNode*)create_bulk_string_node("10", NULL);
    GenericNode* ex_node    = (GenericNode*)create_bulk_string_node("EX", ex_node_val);
    GenericNode* value_node = (GenericNode*)create_bulk_string_node("value", ex_node);
    GenericNode* key_node   = (GenericNode*)create_bulk_string_node("key", value_node);
    GenericNode** parsed_cmd = NULL;
    char state = 0;

    void* result = validate_set_cmd(key_node, &state, &parsed_cmd);

    assert(result != NULL); // Function should return non-NULL for valid SET command with EX
    assert(state  == 0b01110011); // Final state should be 0b01110011
    assert(parsed_cmd != NULL); // parsed_cmd should be allocated

    free(key_node->node);
    free(key_node);
    free(value_node->node);
    free(value_node);
    free(ex_node->node);
    free(ex_node);
    free(ex_node_val->node);
    free(ex_node_val);
    free(parsed_cmd);
}

void test_validate_set_cmd_invalid_set_key_value_ex() {
    GenericNode* ex_node    = (GenericNode*)create_bulk_string_node("EX", NULL);
    GenericNode* value_node = (GenericNode*)create_bulk_string_node("value", ex_node);
    GenericNode* key_node   = (GenericNode*)create_bulk_string_node("key", value_node);
    GenericNode** parsed_cmd = NULL;
    char state = 0;

    void* result = validate_set_cmd(key_node, &state, &parsed_cmd);

    assert(result == NULL); // Function should return non-NULL for valid SET command with EX
    assert(state  == 0b01110010); // Final state should be 0b01110011
    assert(parsed_cmd != NULL); // parsed_cmd should be allocated

    free(key_node->node);
    free(key_node);
    free(value_node->node);
    free(value_node);
    free(ex_node->node);
    free(ex_node);
    free(parsed_cmd);
}

void test_validate_set_cmd_invalid_set_key_value_missing_value() {
    GenericNode* key_node    = (GenericNode*)create_bulk_string_node("key", NULL);
    GenericNode** parsed_cmd = NULL;
    char state = 0;

    void* result = validate_set_cmd(key_node, &state, &parsed_cmd);

    assert(result == NULL); // Function should return NULL for missing value

    free(key_node->node);
    free(key_node);
}

void test_validate_set_cmd_invalid_set_key_value_invalid_option() {
    GenericNode* invalid_node = (GenericNode*)create_bulk_string_node("INVALID", NULL);
    GenericNode* value_node   = (GenericNode*)create_bulk_string_node("value", invalid_node);
    GenericNode* key_node     = (GenericNode*)create_bulk_string_node("key", value_node);
    GenericNode** parsed_cmd = NULL;
    char state = 0;

    void* result = validate_set_cmd(key_node, &state, &parsed_cmd);

    assert(result == NULL); // Function should return NULL for invalid option

    free(key_node->node);
    free(key_node);
    free(value_node->node);
    free(value_node);
    free(invalid_node->node);
    free(invalid_node);
}
int main() {
    test_is_set_option_valid();
    test_handle_set_options_valid();
    test_handle_set_options_valid_one();
    test_handle_set_options_invalid_bit_pos();
    test_handle_set_options_bit_already_set();
    test_handle_set_options_gnode_null();
    test_handle_set_options_parsed_cmd_null();
    test_handle_set_options_state_null();
    test_set_cmd_stage_a_valid();
    test_set_cmd_stage_a_invalid_gnode_null();
    test_set_cmd_stage_a_invalid_parsed_cmd_null();
    test_set_cmd_stage_a_invalid_node_type();
    test_set_cmd_stage_a_invalid_node_next_null();
    test_set_cmd_stage_a_state_null();
    // Basic unit tests
    test_validate_set_cmd_null_state();
    test_validate_set_cmd_null_node();
    test_validate_set_cmd_invalid_node_type();
    test_validate_set_cmd_memory_allocation_failure();

    // SET command tests
    test_validate_set_cmd_valid_set_key_value();
    test_validate_set_cmd_valid_set_key_value_nx();
    test_validate_set_cmd_valid_set_key_value_ex();
    test_validate_set_cmd_invalid_set_key_value_ex();
    test_validate_set_cmd_invalid_set_key_value_missing_value();
    test_validate_set_cmd_invalid_set_key_value_invalid_option();

    puts("All tests passed");
    return 0;
}