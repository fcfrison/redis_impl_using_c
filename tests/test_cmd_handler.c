#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
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
    printf("All tests passed!\n");
}

// Helper function to create a GenericNode
struct GenericNode* create_generic_node(RedisDtype type) {
    struct GenericNode* node = (struct GenericNode*)malloc(sizeof(struct GenericNode));
    node->node = (struct BaseNode*)malloc(sizeof(struct BaseNode));
    node->node->type = type;
    node->node->next = NULL;
    node->node->prev = NULL;
    return node;
}

// Test cases
void test_handle_set_options_valid() {
    char state = 0b01110000; // SET_BASIC state
    struct GenericNode* parsed_cmd[4] = {NULL, NULL, NULL, NULL};
    struct GenericNode* gnode = create_generic_node(BULK_STR);
    struct GenericNode* prev_gnode = gnode;
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
    struct GenericNode* parsed_cmd[4] = {NULL, NULL, NULL, NULL};
    struct GenericNode* gnode = create_generic_node(BULK_STR);
    struct GenericNode* prev_gnode_zero = gnode;
    handle_set_options(&state, parsed_cmd, &gnode, 3); // Set bit 3 (NX/XX)
    gnode = create_generic_node(BULK_STR);
    struct GenericNode* prev_gnode_one = gnode;
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
    struct GenericNode* parsed_cmd[4] = {NULL, NULL, NULL, NULL};
    struct GenericNode* gnode = create_generic_node(BULK_STR);

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
    struct GenericNode* parsed_cmd[4] = {NULL, NULL, NULL, NULL};
    struct GenericNode* gnode = create_generic_node(BULK_STR);

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
    struct GenericNode* parsed_cmd[4] = {NULL, NULL, NULL, NULL};
    struct GenericNode* gnode = NULL; // gnode is NULL

    handle_set_options(&state, parsed_cmd, &gnode, 2); // Try to set bit 2

    assert(state == -1); // State should be -1 (error)
    assert(parsed_cmd[2] == NULL); // parsed_cmd should remain unchanged
    assert(gnode == NULL); // gnode should remain NULL

    // No need to free gnode since it's NULL
}

void test_handle_set_options_parsed_cmd_null() {
    char state = 0b01110000; // SET_BASIC state
    struct GenericNode** parsed_cmd = NULL; // parsed_cmd is NULL
    struct GenericNode* gnode = create_generic_node(BULK_STR);

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
    struct GenericNode* parsed_cmd[4] = {NULL, NULL, NULL, NULL};
    struct GenericNode* gnode = create_generic_node(BULK_STR);

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
int main() {
    test_is_set_option_valid();
    test_handle_set_options_valid();
    test_handle_set_options_valid_one();
    test_handle_set_options_invalid_bit_pos();
    test_handle_set_options_bit_already_set();
    test_handle_set_options_gnode_null();
    test_handle_set_options_parsed_cmd_null();
    test_handle_set_options_state_null();
    puts("All tests passed");
    return 0;
}