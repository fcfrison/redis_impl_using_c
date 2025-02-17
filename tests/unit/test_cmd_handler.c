#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cmd_handler.h"
#include "../include/protocol.h"
#include "../include/simple_map.h"

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

// Test cases
void test_clean_up_kv_key_only() {
    KeyNode* key = create_key_node("test_key", 0, 0, 8);
    assert(key != NULL); // Ensure key creation was successful
    clean_up_kv(key, NULL);

    // If the function works correctly, the memory should be freed, and no assertions are needed.
    // This test ensures no memory leaks or crashes.
}

void test_clean_up_kv_value_only() {
    ValueNode* value = create_value_node_string("test_value", BULK_STR, 10);
    assert(value != NULL); // Ensure value creation was successful
    clean_up_kv(NULL, value);

    // If the function works correctly, the memory should be freed, and no assertions are needed.
    // This test ensures no memory leaks or crashes.
}

void test_clean_up_kv_both_key_and_value() {
    KeyNode* key = create_key_node("test_key", 0, 0, 8);
    assert(key != NULL); // Ensure key creation was successful
    ValueNode* value = create_value_node_string("test_value", BULK_STR, 10);
    assert(value != NULL); // Ensure value creation was successful
    clean_up_kv(key, value);

    // If the function works correctly, the memory should be freed, and no assertions are needed.
    // This test ensures no memory leaks or crashes.
}

void test_clean_up_kv_null_key_and_value() {
    clean_up_kv(NULL, NULL);

    // If the function works correctly, it should handle NULL inputs without crashing.
}

void test_clean_up_kv_value_other_type() {
    ValueNode* value = (ValueNode*)malloc(sizeof(ValueNode));
    value->content = NULL;
    value->dtype = UNDEF_DTYPE;
    clean_up_kv(NULL, value);

    // If the function works correctly, it should handle non-BULK_STR types without crashing.
}

void test_create_key_node_valid() {
    char* content = "test_key";
    unsigned int ex = 10;
    unsigned int px = 20;
    int size = strlen(content);

    KeyNode* key = create_key_node(content, ex, px, size);

    assert(key != NULL);
    assert(strcmp(key->content, content) == 0);
    assert(key->ex == ex);
    assert(key->px == px);
    assert(key->size == size);
    assert(key->input_time != NULL);

    free(key->content);
    free(key->input_time);
    free(key);
}

void test_create_key_node_null_content() {
    KeyNode* key = create_key_node(NULL, 10, 20, 8);
    assert(key == NULL);
}

void test_create_key_node_invalid_size() {
    char* content = "test_key";
    KeyNode* key = create_key_node(content, 10, 20, 0);
    assert(key == NULL);
}


void test_create_value_node_string_valid() {
    char* content = "test_value";
    RedisDtype dtype = BULK_STR;
    int size = strlen(content);

    ValueNode* value = create_value_node_string(content, dtype, size);

    assert(value != NULL);
    ValueNodeString* vns = (ValueNodeString*)value;
    assert(strcmp(vns->content, content) == 0);
    assert(vns->dtype == dtype);
    assert(vns->size == size);

    free(vns->content);
    free(vns);
}

void test_create_value_node_string_null_content() {
    ValueNode* value = create_value_node_string(NULL, BULK_STR, 10);
    assert(value == NULL);
}

void test_create_value_node_string_invalid_size() {
    char* content = "test_value";
    ValueNode* value = create_value_node_string(content, BULK_STR, 0);
    assert(value == NULL);
}

void test_create_value_node_valid_bulk_str() {
    BulkStringNode* bulk_node = (BulkStringNode*)malloc(sizeof(BulkStringNode));
    BaseNode*       base_node = (BaseNode*)calloc(1,sizeof(BaseNode));
    base_node->type = BULK_STR;
    base_node->next = NULL;
    base_node->prev = NULL;
    bulk_node->node = base_node;
    bulk_node->content = strdup("test_value");
    bulk_node->size = strlen("test_value");

    GenericNode* gnode = (GenericNode*)bulk_node;
    ValueNode* value = create_value_node(gnode);
    assert(strcmp(value->content,bulk_node->content)==0);
    assert(value != NULL); // Ensure value creation was successful

    free(bulk_node->content);
    free(bulk_node);
    free(value);
}

void test_create_value_node_invalid_type() {
    BaseNode base_node;
    base_node.type = UNDEF_DTYPE;

    GenericNode gnode;
    gnode.node = &base_node;

    ValueNode* value = create_value_node(&gnode);

    assert(value == NULL); // Ensure NULL is returned for invalid type
}

void test_create_value_node_null_gnode() {
    ValueNode* value = create_value_node(NULL);

    assert(value == NULL); // Ensure NULL is returned for NULL gnode
}

void test_compare_equal_keys() {
    KeyNode key1 = {"test_key", NULL, 0, 0, 8};
    KeyNode key2 = {"test_key", NULL, 0, 0, 8};

    void* result = compare(&key1, &key2);

    assert(result == &key1); // Ensure the keys are equal and return the first key
}

void test_compare_different_sizes() {
    KeyNode key1 = {"test_key", NULL, 0, 0, 8};
    KeyNode key2 = {"test_key_diff", NULL, 0, 0, 12};

    void* result = compare(&key1, &key2);

    assert(result == NULL); // Ensure NULL is returned for different sizes
}

void test_compare_different_content() {
    KeyNode key1 = {"test_key", NULL, 0, 0, 8};
    KeyNode key2 = {"test_kex", NULL, 0, 0, 8};

    void* result = compare(&key1, &key2);

    assert(result == NULL); // Ensure NULL is returned for different content
}

void test_compare_null_input() {
    void* result = compare(NULL, NULL);

    assert(result == NULL); // Ensure NULL is returned for NULL inputs
}


void test_execute_set_get_success_set_new_item() {
    SimpleMap* sm     = create_simple_map();
    KeyNode* key      = create_key_node("test_key_1", 0, 0, strlen("test_key_1"));
    ValueNode* value  = create_value_node_string("value_key_1",BULK_STR,strlen("value_key_1"));
    KeyValuePair* kvp = create_key_val_pair(key,value);

    char* result = execute_set_get(sm, kvp);

    assert(result != NULL); // Ensure result is not NULL for successful set
    assert(strcmp(result, "$-1\r\n") == 0); // Ensure the correct response message
    assert(sm->keys[0]);
    assert(sm->values[0]);
    assert(sm->keys[0]->key == key);
    assert(sm->values[0]->value == value);
    free(result);
    free(sm->keys);
    free(sm->values);
    free(value);
    free(key);
}
// Key already exists
void test_execute_set_get_success_upgrade() {
    SimpleMap* sm       = create_simple_map();
    KeyValuePair* kvp   = create_key_val_pair(
                                    create_key_node("test_key_1", 0, 0, strlen("test_key_1")),
                                    create_value_node_string("value_key_1",BULK_STR,strlen("value_key_1"))
                                );
    // Simulate success upgrade
    char* result        = execute_set_get(sm, kvp);
    KeyValuePair* kvp_1 = create_key_val_pair(
                                    create_key_node("test_key_1", 0, 0, strlen("test_key_1")),
                                    create_value_node_string("value_key_66",BULK_STR,strlen("value_key_66"))
                                );
    result              = execute_set_get(sm, kvp_1);
    assert(result != NULL); // Ensure result is not NULL for successful upgrade
    assert(strcmp(result, "$11\r\nvalue_key_1\r\n") == 0); // Ensure the correct response message
    free(result);
    // This is not the proper way to free memory, because in this way memory leak will occur
    // but it's enough for tests
    free(sm->keys[0]);
    free(sm->values[0]);
}

void test_execute_set_get_error_values_arr_is_null() {
    // values array and 
    SimpleMap* sm = create_simple_map();
    free(sm->values);
    sm->values = NULL;
    KeyNode* key = create_key_node("test_key_1", 0, 0, strlen("test_key_1"));
    ValueNode* value = create_value_node_string("value_key_1",BULK_STR,strlen("value_key_1"));
    KeyValuePair* kvp = create_key_val_pair(key,value);
    // Simulate set failure
    char* result = execute_set_get(sm, kvp);

    assert(result == NULL); // Ensure NULL is returned for set error
    free(sm->keys);
}

void test_execute_set_get_null_inputs() {
    char* result = execute_set_get(NULL, NULL);

    assert(result == NULL); // Ensure NULL is returned for NULL inputs
    return;
}
void test_execute_set_get_error_null_key() {
    SimpleMap* sm       = create_simple_map();
    KeyValuePair* kvp   = create_key_val_pair(
                                    create_key_node("test_key_1", 0, 0, strlen("test_key_1")),
                                    create_value_node_string("value_key_1",BULK_STR,strlen("value_key_1"))
                                );
    char* content = ((KeyNode*)kvp->key)->content;
    ((KeyNode*)kvp->key)->content = NULL;
    free(content);
    // Simulate success upgrade
    char* result        = execute_set_get(sm, kvp);
    assert(result==NULL);
    free(sm);
    return;
}

// It's possible to set null content values;
void test_execute_set_get_sucess_null_content_value() {
    SimpleMap* sm       = create_simple_map();
    KeyValuePair* kvp   = create_key_val_pair(
                                    create_key_node("test_key_1", 0, 0, strlen("test_key_1")),
                                    create_value_node_string("value_key_1",BULK_STR,strlen("value_key_1"))
                                );
    char* content = ((ValueNode*)kvp->value)->content;
    ((ValueNode*)kvp->value)->content = NULL;
    free(content);
    // Simulate success upgrade
    char* result        = execute_set_get(sm, kvp);
    assert(strcmp(result, "$-1\r\n") == 0); // Ensure the correct response message
    assert(sm->keys[0]);
    assert(sm->values[0]);
    assert(((ValueNode*)sm->values[0]->value)->content == NULL);
    free(result);
    free(((KeyNode*)sm->keys[0]->key)->content);
    free(((KeyNode*)sm->keys[0]->key)->input_time);
    free(((KeyNode*)sm->keys[0]->key));
    free(sm->keys[0]);
    free(sm->keys);
    free(sm->values[0]->value);
    free(sm->values[0]);
    free(sm->values);
    return;
}
void test_execute_set_get_sucess_replace_null_content_value() {
    SimpleMap* sm       = create_simple_map();
    KeyValuePair* kvp   = create_key_val_pair(
                                    create_key_node("test_key_1", 0, 0, strlen("test_key_1")),
                                    create_value_node_string("value_key_1",BULK_STR,strlen("value_key_1"))
                                );
    char* content = ((ValueNode*)kvp->value)->content;
    ((ValueNode*)kvp->value)->content = NULL;
    free(content);
    // Simulate success upgrade
    char* result = execute_set_get(sm, kvp);
    KeyValuePair* kvp_1 = create_key_val_pair(
        create_key_node("test_key_1", 0, 0, strlen("test_key_1")),
        create_value_node_string("value_key_1",BULK_STR,strlen("value_key_1"))
    );
    
    result = execute_set_get(sm, kvp_1);

    assert(strcmp(result, "$-1\r\n") == 0); // Ensure the correct response message
  
    assert(sm->keys[0]);
    assert(sm->values[0]);
    assert(strcmp(((ValueNode*)sm->values[0]->value)->content,"value_key_1")==0);  
    free(result);
    free(((KeyNode*)sm->keys[0]->key)->content);
    free(((KeyNode*)sm->keys[0]->key)->input_time);
    free(((KeyNode*)sm->keys[0]->key));
    free(sm->keys[0]);
    free(sm->keys);
    free(sm->values[0]->value);
    free(sm->values[0]);
    free(sm->values);
    return;
}

void test_execute_set_basic_success_set() {
    SimpleMap* sm = create_simple_map();
    KeyValuePair* kvp   = create_key_val_pair(
        create_key_node("test_key_1", 0, 0, strlen("test_key_1")),
        create_value_node_string("value_key_1",BULK_STR,strlen("value_key_1")));

    char* result = execute_set_basic(sm, kvp);

    assert(result != NULL); // Ensure result is not NULL for successful set
    assert(strcmp(result, "+OK\r\n") == 0); // Ensure the correct response message
    assert(strcmp(((KeyNode*)sm->keys[0]->key)->content,"test_key_1")==0);
    assert(strcmp(((ValueNodeString*)sm->values[0]->value)->content,"value_key_1")==0);
    free(result);
    free(((KeyNode*)sm->keys[0]->key)->content);
    free(((KeyNode*)sm->keys[0]->key)->input_time);
    free(((KeyNode*)sm->keys[0]->key));
    free(sm->keys[0]);
    free(sm->keys);
    free(sm->values[0]->value);
    free(sm->values[0]);
    free(sm->values);
    return;

}

void test_execute_set_basic_success_upgrade() {
    SimpleMap* sm = create_simple_map();
    KeyValuePair* kvp   = create_key_val_pair(
        create_key_node("test_key_1", 0, 0, strlen("test_key_1")),
        create_value_node_string("value_key_1",BULK_STR,strlen("value_key_1")));
    char* result = execute_set_basic(sm, kvp);
    KeyValuePair* kvp_1   = create_key_val_pair(
        create_key_node("test_key_1", 0, 0, strlen("test_key_1")),
        create_value_node_string("value_key_2",BULK_STR,strlen("value_key_2")));
    result = execute_set_basic(sm, kvp_1);
    assert(result != NULL); // Ensure result is not NULL for successful set
    assert(strcmp(result, "+OK\r\n") == 0); // Ensure the correct response message
    assert(strcmp(((KeyNode*)sm->keys[0]->key)->content,"test_key_1")==0);
    assert(strcmp(((ValueNodeString*)sm->values[0]->value)->content,"value_key_2")==0);
    free(result);
    free(((KeyNode*)sm->keys[0]->key)->content);
    free(((KeyNode*)sm->keys[0]->key)->input_time);
    free(((KeyNode*)sm->keys[0]->key));
    free(sm->keys[0]);
    free(sm->keys);
    free(sm->values[0]->value);
    free(sm->values[0]);
    free(sm->values);
}
void test_execute_set_basic_null_inputs() {
    char* result = execute_set_basic(NULL, NULL);

    assert(result == NULL); // Ensure NULL is returned for NULL inputs
}

void test_execute_set_basic_key_null_content() {
    SimpleMap* sm       = create_simple_map();
    KeyValuePair* kvp   = create_key_val_pair(
                                    create_key_node("test_key_1", 0, 0, strlen("test_key_1")),
                                    create_value_node_string("value_key_1",BULK_STR,strlen("value_key_1"))
                                );
    char* content = ((KeyNode*)kvp->key)->content;
    ((KeyNode*)kvp->key)->content = NULL;
    free(content);
    // Simulate success upgrade
    char* result = execute_set_basic(sm, kvp);
    assert(result==NULL);
    free(sm->keys);
    free(sm->values);
    free(sm);
    return;
    
}
void test_execute_set_basic_empty_key() {
    SimpleMap* sm = create_simple_map();
    ValueNode* vns = create_value_node_string("value_key_1",BULK_STR,strlen("value_key_1"));
    KeyValuePair* kvp = (KeyValuePair*)malloc(sizeof(KeyValuePair));
    kvp->key=NULL;
    kvp->value = (char*)vns;
    char* result = execute_set_basic(sm, kvp);
    assert(result==NULL);
    free(sm->keys);
    free(sm->values);
    free(sm);
    return;
}

// execute_set_cmd
void test_execute_set_cmd_set_basic() {
    SimpleMap* sm = create_simple_map();
    BulkStringNode* key_node = (BulkStringNode*)malloc(sizeof(BulkStringNode));
    BaseNode* bn = (BaseNode*)malloc(sizeof(BaseNode));
    bn->next = bn->prev = NULL;
    bn->type = BULK_STR;
    key_node->content = (char*)malloc(strlen("test_key")*sizeof(char));
    strcpy(key_node->content,"test_key");
    key_node->node = bn;
    key_node->size = 8;
    BulkStringNode* value_node = (BulkStringNode*)malloc(sizeof(BulkStringNode));
    BaseNode* bn_vn = (BaseNode*)malloc(sizeof(BaseNode));
    bn_vn->next = bn_vn->prev = NULL;
    bn_vn->type = BULK_STR;
    value_node->content = (char*)malloc(strlen("test_value")*sizeof(char));
    strcpy(value_node->content, "test_value");
    value_node->node = bn_vn;
    value_node->size = strlen(value_node->content);
    GenericNode** parsed_cmd = (GenericNode**)calloc(6,sizeof(GenericNode*));
    parsed_cmd[5] = (GenericNode*)key_node;
    parsed_cmd[4] = (GenericNode*)value_node;
    char* result = execute_set_cmd(SET_BASIC, parsed_cmd, sm);

    assert(result != NULL); // Ensure result is not NULL for SET_BASIC
    assert(strcmp(result, "+OK\r\n") == 0); // Ensure the correct response message
    assert(strcmp(((KeyNode*)sm->keys[0]->key)->content, "test_key")==0);
    assert(strcmp(((ValueNode*)sm->values[0]->value)->content, "test_value")==0);
    free(result);
    free(((KeyNode*)sm->keys[0]->key)->content);
    free(((KeyNode*)sm->keys[0]->key)->input_time);
    free(((KeyNode*)sm->keys[0]->key));
    free(sm->keys[0]);
    free(sm->keys);
    free(sm->values[0]->value);
    free(sm->values[0]);
    free(sm->values);
    return;
}

void test_execute_set_cmd_set_get_no_previous_key() {
    SimpleMap* sm = create_simple_map();
    BulkStringNode* key_node = (BulkStringNode*)malloc(sizeof(BulkStringNode));
    BaseNode* bn = (BaseNode*)malloc(sizeof(BaseNode));
    bn->next = bn->prev = NULL;
    bn->type = BULK_STR;
    key_node->content = (char*)malloc(strlen("test_key")*sizeof(char));
    strcpy(key_node->content,"test_key");
    key_node->node = bn;
    key_node->size = 8;
    BulkStringNode* value_node = (BulkStringNode*)malloc(sizeof(BulkStringNode));
    BaseNode* bn_vn = (BaseNode*)malloc(sizeof(BaseNode));
    bn_vn->next = bn_vn->prev = NULL;
    bn_vn->type = BULK_STR;
    value_node->content = (char*)malloc(strlen("test_value")*sizeof(char));
    strcpy(value_node->content, "test_value");
    value_node->node = bn_vn;
    value_node->size = strlen(value_node->content);
    GenericNode** parsed_cmd = (GenericNode**)calloc(6,sizeof(GenericNode*));
    parsed_cmd[5] = (GenericNode*)key_node;
    parsed_cmd[4] = (GenericNode*)value_node;
    char* result = execute_set_cmd(SET_GET, parsed_cmd, sm);

    assert(result != NULL); // Ensure result is not NULL for SET_BASIC
    assert(strcmp(result, "$-1\r\n") == 0); // Ensure the correct response message
    assert(strcmp(((KeyNode*)sm->keys[0]->key)->content, "test_key")==0);
    assert(strcmp(((ValueNode*)sm->values[0]->value)->content, "test_value")==0);
    free(result);
    free(((KeyNode*)sm->keys[0]->key)->content);
    free(((KeyNode*)sm->keys[0]->key)->input_time);
    free(((KeyNode*)sm->keys[0]->key));
    free(sm->keys[0]);
    free(sm->keys);
    free(sm->values[0]->value);
    free(sm->values[0]);
    free(sm->values);
    return;
}
void test_execute_set_cmd_set_get_upgrade() {
    SimpleMap* sm = create_simple_map();
    BulkStringNode* key_node = (BulkStringNode*)malloc(sizeof(BulkStringNode));
    BaseNode* bn = (BaseNode*)malloc(sizeof(BaseNode));
    bn->next = bn->prev = NULL;
    bn->type = BULK_STR;
    key_node->content = (char*)malloc(strlen("test_key")*sizeof(char));
    strcpy(key_node->content,"test_key");
    key_node->node = bn;
    key_node->size = 8;
    BulkStringNode* value_node = (BulkStringNode*)malloc(sizeof(BulkStringNode));
    BaseNode* bn_vn = (BaseNode*)malloc(sizeof(BaseNode));
    bn_vn->next = bn_vn->prev = NULL;
    bn_vn->type = BULK_STR;
    value_node->content = (char*)malloc(strlen("test_value")*sizeof(char));
    strcpy(value_node->content, "test_value");
    value_node->node = bn_vn;
    value_node->size = strlen(value_node->content);
    GenericNode** parsed_cmd = (GenericNode**)calloc(6,sizeof(GenericNode*));
    parsed_cmd[5] = (GenericNode*)key_node;
    parsed_cmd[4] = (GenericNode*)value_node;
    char* result = execute_set_cmd(SET_GET, parsed_cmd, sm);


    BulkStringNode* new_key_node = (BulkStringNode*)malloc(sizeof(BulkStringNode));
    BaseNode* bn_new_key_node = (BaseNode*)malloc(sizeof(BaseNode));
    bn_new_key_node->next = bn_new_key_node->prev = NULL;
    bn_new_key_node->type = BULK_STR;
    new_key_node->content = (char*)malloc(strlen("test_key")*sizeof(char));
    strcpy(new_key_node->content,"test_key");
    new_key_node->node = bn_new_key_node;
    new_key_node->size = 8;
    BulkStringNode* new_value_node = (BulkStringNode*)malloc(sizeof(BulkStringNode));
    BaseNode* bn_vn_new_value_node = (BaseNode*)malloc(sizeof(BaseNode));
    bn_vn_new_value_node->next = bn_vn_new_value_node->prev = NULL;
    bn_vn_new_value_node->type = BULK_STR;
    new_value_node->content = (char*)malloc(strlen("test_value_66")*sizeof(char));
    strcpy(new_value_node->content, "test_value_66");
    new_value_node->node = bn_vn_new_value_node;
    new_value_node->size = strlen(new_value_node->content);
    parsed_cmd[5] = (GenericNode*)new_key_node;
    parsed_cmd[4] = (GenericNode*)new_value_node;
    result = execute_set_cmd(SET_GET, parsed_cmd, sm);
    assert(result != NULL); // Ensure result is not NULL for SET_BASIC
    assert(strcmp(result, "$10\r\ntest_value\r\n") == 0); // Ensure the correct response message
    assert(strcmp(((KeyNode*)sm->keys[0]->key)->content, "test_key")==0);
    assert(strcmp(((ValueNode*)sm->values[0]->value)->content, "test_value_66")==0);
    free(result);
    free(((KeyNode*)sm->keys[0]->key)->content);
    free(((KeyNode*)sm->keys[0]->key)->input_time);
    free(((KeyNode*)sm->keys[0]->key));
    free(sm->keys[0]);
    free(sm->keys);
    free(sm->values[0]->value);
    free(sm->values[0]);
    free(sm->values);
    return;
}

void test_execute_set_cmd_null_parsed_cmd() {
    SimpleMap sm = {NULL, NULL, 10, 0};

    char* result = execute_set_cmd(SET_BASIC, NULL, &sm);

    assert(result == NULL); // Ensure NULL is returned for NULL parsed_cmd
}
void test_execute_set_cmd_null_sm() {
    BaseNode base_node = {BULK_STR, NULL, NULL};
    BulkStringNode key_node = {&base_node, "test_key", 8};
    GenericNode value_node = {&base_node};
    GenericNode* parsed_cmd[6] = {NULL, NULL, NULL, NULL, (GenericNode*)&value_node, (GenericNode*)&key_node};

    char* result = execute_set_cmd(SET_BASIC, parsed_cmd, NULL);

    assert(result == NULL); // Ensure NULL is returned for NULL sm
}
void test_execute_set_cmd_null_key_or_value() {
    SimpleMap sm = {NULL, NULL, 10, 0};
    GenericNode* parsed_cmd[6] = {NULL, NULL, NULL, NULL, NULL, NULL};

    char* result = execute_set_cmd(SET_BASIC, parsed_cmd, &sm);

    assert(result == NULL); // Ensure NULL is returned for NULL key or value
}

void test_execute_set_nx_xx_nx_success() {
    SimpleMap* sm            = create_simple_map();
    KeyNode* key             = create_key_node("test_key_1", 0, 0, strlen("test_key_1"));
    ValueNode* value         = create_value_node_string("value_key_1",BULK_STR,strlen("value_key_1"));
    KeyValuePair* kvp        = create_key_val_pair(key,value);
    GenericNode** parsed_cmd = (GenericNode**)calloc(6,sizeof(GenericNode*));
    BulkStringNode* nx_node = (BulkStringNode*)calloc(1, sizeof(BulkStringNode));
    BaseNode* bn = (BaseNode*)calloc(1, sizeof(BaseNode));
    bn->next = bn->prev = NULL;
    bn->type = BULK_STR;
    nx_node->content = (char*)malloc(sizeof(char)*strlen("NX"));
    strcpy(nx_node->content,"NX");
    nx_node->size = (int)strlen("NX");
    nx_node->node = bn;
    parsed_cmd[3] = (GenericNode*)nx_node;
    char* result = execute_set_nx_xx(sm, kvp, parsed_cmd);

    assert(result != NULL); // Ensure result is not NULL for NX success
    assert(strcmp(result, "+OK\r\n") == 0); // Ensure the correct response message
    assert(strcmp(((KeyNode*)sm->keys[0]->key)->content, "test_key_1")==0);
    assert(strcmp(((ValueNode*)sm->values[0]->value)->content, "value_key_1")==0);
    free(result);
    free(((KeyNode*)sm->keys[0]->key)->content);
    free(((KeyNode*)sm->keys[0]->key)->input_time);
    free(((KeyNode*)sm->keys[0]->key));
    free(sm->keys[0]);
    free(sm->keys);
    free(sm->values[0]->value);
    free(sm->values[0]);
    free(sm->values);
    free(nx_node->content);
    free(nx_node->node);
    free(nx_node);
}

void test_execute_set_nx_xx_nx_failure() {
    SimpleMap* sm            = create_simple_map();
    KeyNode* key             = create_key_node("test_key_1", 0, 0, strlen("test_key_1"));
    ValueNode* value         = create_value_node_string("value_key_1",BULK_STR,strlen("value_key_1"));
    KeyValuePair* kvp        = create_key_val_pair(key,value);
    GenericNode** parsed_cmd = (GenericNode**)calloc(6,sizeof(GenericNode*));
    BulkStringNode* nx_node = (BulkStringNode*)calloc(1, sizeof(BulkStringNode));
    BaseNode* bn = (BaseNode*)calloc(1, sizeof(BaseNode));
    bn->next = bn->prev = NULL;
    bn->type = BULK_STR;
    nx_node->content = (char*)malloc(sizeof(char)*strlen("NX"));
    strcpy(nx_node->content,"NX");
    nx_node->size = (int)strlen("NX");
    nx_node->node = bn;
    parsed_cmd[3] = (GenericNode*)nx_node;
    char* result = execute_set_nx_xx(sm, kvp, parsed_cmd);
    // create the same item
    key   = create_key_node("test_key_1", 0, 0, strlen("test_key_1"));
    value = create_value_node_string("value_key_1",BULK_STR,strlen("value_key_1"));
    kvp = create_key_val_pair(key,value);
    result = execute_set_nx_xx(sm, kvp, parsed_cmd);
    assert(result != NULL); // Ensure result is not NULL for NX success
    assert(strcmp(result, "$-1\r\n") == 0); // Ensure the correct response message
    assert(strcmp(((KeyNode*)sm->keys[0]->key)->content, "test_key_1")==0);
    assert(strcmp(((ValueNode*)sm->values[0]->value)->content, "value_key_1")==0);
    free(((KeyNode*)sm->keys[0]->key)->content);
    free(((KeyNode*)sm->keys[0]->key)->input_time);
    free(((KeyNode*)sm->keys[0]->key));
    free(sm->keys[0]);
    free(sm->keys);
    free(sm->values[0]->value);
    free(sm->values[0]);
    free(sm->values);
    free(nx_node->content);
    free(nx_node->node);
    free(nx_node);
    free(result);
}

void test_execute_set_nx_xx_xx_success() {
    //XX -- Only set the key if it already exists.

    SimpleMap* sm            = create_simple_map();
    KeyNode* key             = create_key_node("test_key_1", 0, 0, strlen("test_key_1"));
    ValueNode* value         = create_value_node_string("value_key_1",BULK_STR,strlen("value_key_1"));
    KeyValuePair* kvp        = create_key_val_pair(key,value);
    GenericNode** parsed_cmd = (GenericNode**)calloc(6,sizeof(GenericNode*));
    BulkStringNode* nx_node = (BulkStringNode*)calloc(1, sizeof(BulkStringNode));
    BaseNode* bn = (BaseNode*)calloc(1, sizeof(BaseNode));
    bn->next = bn->prev = NULL;
    bn->type = BULK_STR;
    nx_node->content = (char*)malloc(sizeof(char)*strlen("XX"));
    strcpy(nx_node->content,"XX");
    nx_node->size = (int)strlen("XX");
    nx_node->node = bn;
    parsed_cmd[3] = (GenericNode*)nx_node;
    char* result = execute_set_basic(sm, kvp);
    KeyNode* key_1             = create_key_node("test_key_1", 0, 0, strlen("test_key_1"));
    ValueNode* value_1         = create_value_node_string("value_key_11",BULK_STR,strlen("value_key_11"));
    KeyValuePair* kvp_1        = create_key_val_pair(key_1,value_1);
    kvp = create_key_val_pair(key_1,value_1);
    result = execute_set_nx_xx(sm, kvp_1, parsed_cmd);
    assert(result != NULL); // Ensure result is not NULL for NX success
    assert(strcmp(result, "+OK\r\n") == 0); // Ensure the correct response message
    assert(strcmp(((KeyNode*)sm->keys[0]->key)->content, "test_key_1")==0);
    assert(strcmp(((ValueNode*)sm->values[0]->value)->content, "value_key_11")==0);
    free(((KeyNode*)sm->keys[0]->key)->content);
    free(((KeyNode*)sm->keys[0]->key)->input_time);
    free(((KeyNode*)sm->keys[0]->key));
    free(sm->keys[0]);
    free(sm->keys);
    free(sm->values[0]->value);
    free(sm->values[0]);
    free(sm->values);
    free(nx_node->content);
    free(nx_node->node);
    free(nx_node);
    free(result);
    return;
}


void test_execute_set_nx_xx_xx_failure() {
    SimpleMap* sm            = create_simple_map();
    KeyNode* key             = create_key_node("test_key_1", 0, 0, strlen("test_key_1"));
    ValueNode* value         = create_value_node_string("value_key_1",BULK_STR,strlen("value_key_1"));
    KeyValuePair* kvp        = create_key_val_pair(key,value);
    GenericNode** parsed_cmd = (GenericNode**)calloc(6,sizeof(GenericNode*));
    BulkStringNode* nx_node = (BulkStringNode*)calloc(1, sizeof(BulkStringNode));
    BaseNode* bn = (BaseNode*)calloc(1, sizeof(BaseNode));
    bn->next = bn->prev = NULL;
    bn->type = BULK_STR;
    nx_node->content = (char*)calloc(strlen("XX"),sizeof(char));
    strcpy(nx_node->content,"XX");
    nx_node->size = (int)strlen("XX");
    nx_node->node = bn;
    parsed_cmd[3] = (GenericNode*)nx_node;
    char* result  = execute_set_nx_xx(sm, kvp, parsed_cmd);
    assert(result != NULL); // Ensure result is not NULL for NX success
    assert(strcmp(result, "$-1\r\n") == 0); // Ensure the correct response message
    free(sm->keys);
    free(sm->values);
    free(nx_node->content);
    free(nx_node->node);
    free(nx_node);
    free(result);
}

void test_execute_set_nx_xx_invalid_option() {
    SimpleMap* sm            = create_simple_map();
    KeyNode* key             = create_key_node("test_key_1", 0, 0, strlen("test_key_1"));
    ValueNode* value         = create_value_node_string("value_key_1",BULK_STR,strlen("value_key_1"));
    KeyValuePair* kvp        = create_key_val_pair(key,value);
    GenericNode** parsed_cmd = (GenericNode**)calloc(6,sizeof(GenericNode*));
    BulkStringNode* nx_node = (BulkStringNode*)calloc(1, sizeof(BulkStringNode));
    BaseNode* bn = (BaseNode*)calloc(1, sizeof(BaseNode));
    bn->next = bn->prev = NULL;
    bn->type = BULK_STR;
    nx_node->content = (char*)malloc(sizeof(char)*strlen("INVALID"));
    strcpy(nx_node->content,"INVALID");
    nx_node->size = (int)strlen("INVALID");
    nx_node->node = bn;
    parsed_cmd[3] = (GenericNode*)nx_node;
    char* result = execute_set_nx_xx(sm, kvp, parsed_cmd);
    assert(result == NULL); // Ensure NULL is returned for invalid option
    free(sm->keys);
    free(sm->values);
    free(nx_node->content);
    free(nx_node->node);
    free(nx_node);
    free(result);
}
void test_execute_set_nx_xx_null_parsed_cmd() {
    SimpleMap* sm            = create_simple_map();
    KeyNode* key             = create_key_node("test_key_1", 0, 0, strlen("test_key_1"));
    ValueNode* value         = create_value_node_string("value_key_1",BULK_STR,strlen("value_key_1"));
    KeyValuePair* kvp        = create_key_val_pair(key,value);
    char* result = execute_set_nx_xx(sm, kvp, NULL);
    assert(result == NULL); // Ensure NULL is returned for NULL parsed_cmd
    free(sm->keys);
    free(sm->values);
    free(sm);
}
void test_execute_set_nx_xx_null_option() {
    SimpleMap* sm            = create_simple_map();
    KeyNode* key             = create_key_node("test_key_1", 0, 0, strlen("test_key_1"));
    ValueNode* value         = create_value_node_string("value_key_1",BULK_STR,strlen("value_key_1"));
    KeyValuePair* kvp        = create_key_val_pair(key,value);
    GenericNode* parsed_cmd[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
    char* result = execute_set_nx_xx(sm, kvp, parsed_cmd);

    assert(result == NULL); // Ensure NULL is returned for NULL option
    free(sm->keys);
    free(sm->values);
    free(sm);
}

void test_execute_set_nxxx_get_nx_success() {
    SimpleMap* sm = create_simple_map();
    KeyNode* key = create_key_node("test_key_1", 0, 0, strlen("test_key_1"));
    ValueNode* value = create_value_node_string("value_key_1", BULK_STR, strlen("value_key_1"));
    KeyValuePair* kvp = create_key_val_pair(key, value);
    GenericNode** parsed_cmd = (GenericNode**)calloc(6, sizeof(GenericNode*));
    BulkStringNode* nx_node = (BulkStringNode*)calloc(1, sizeof(BulkStringNode));
    BaseNode* bn = (BaseNode*)calloc(1, sizeof(BaseNode));
    bn->next = bn->prev = NULL;
    bn->type = BULK_STR;
    nx_node->content = (char*)malloc(sizeof(char) * strlen("NX"));
    strcpy(nx_node->content, "NX");
    nx_node->size = (int)strlen("NX");
    nx_node->node = bn;
    parsed_cmd[3] = (GenericNode*)nx_node;

    char* result = execute_set_nxxx_get(sm, kvp, parsed_cmd);

    assert(result != NULL); // Ensure result is not NULL for NX success
    assert(strcmp(result, "$-1\r\n") == 0); // Ensure the correct response message

    free(result);
    free(parsed_cmd);
    free(nx_node->content);
    free(nx_node);
    free(bn);
    clean_up_kv(key, value);
}
void test_execute_set_nxxx_get_nx_failure() {
    SimpleMap* sm            = create_simple_map();
    KeyNode* key             = create_key_node("test_key_1", 0, 0, strlen("test_key_1"));
    ValueNode* value         = create_value_node_string("value_key_1", BULK_STR, strlen("value_key_1"));
    KeyValuePair* kvp        = create_key_val_pair(key, value);
    GenericNode** parsed_cmd = (GenericNode**)calloc(6, sizeof(GenericNode*));
    BulkStringNode* xx_node  = (BulkStringNode*)calloc(1, sizeof(BulkStringNode));
    BaseNode* bn             = (BaseNode*)calloc(1, sizeof(BaseNode));
    bn->next = bn->prev = NULL;
    bn->type = BULK_STR;
    xx_node->content = (char*)malloc(sizeof(char) * strlen("NX"));
    strcpy(xx_node->content, "NX");
    xx_node->size = (int)strlen("NX");
    xx_node->node = bn;
    parsed_cmd[3] = (GenericNode*)xx_node;

    // Simulate existing key
    char* result = execute_set_nxxx_get(sm, kvp, parsed_cmd);
    key          = create_key_node("test_key_1", 0, 0, strlen("test_key_1"));
    value        = create_value_node_string("value_key_10", BULK_STR, strlen("value_key_10"));
    kvp          = create_key_val_pair(key, value);
    result       = execute_set_nxxx_get(sm, kvp, parsed_cmd);
    assert(result != NULL); // Ensure result is not NULL for XX success
    assert(strcmp(result, "$11\r\nvalue_key_1\r\n") == 0); // Ensure the correct response message

    free(result);
    free(((KeyNode*)sm->keys[0]->key)->content);
    free(((KeyNode*)sm->keys[0]->key)->input_time);
    free(((KeyNode*)sm->keys[0]->key));
    free(((ValueNodeString*)sm->values[0]->value)->content);
    free(((ValueNodeString*)sm->values[0]->value));
    free(sm->keys);
    free(sm->values);
    free(sm);
    free(parsed_cmd);

    free(xx_node->content);
    free(xx_node);
    free(bn);
}
void test_execute_set_nxxx_get_xx_success() {
    SimpleMap* sm = create_simple_map();
    KeyNode* key = create_key_node("test_key_1", 0, 0, strlen("test_key_1"));
    ValueNode* value = create_value_node_string("value_key_1", BULK_STR, strlen("value_key_1"));
    KeyValuePair* kvp = create_key_val_pair(key, value);
    GenericNode** parsed_cmd = (GenericNode**)calloc(6, sizeof(GenericNode*));
    BulkStringNode* xx_node = (BulkStringNode*)calloc(1, sizeof(BulkStringNode));
    BaseNode* bn = (BaseNode*)calloc(1, sizeof(BaseNode));
    bn->next = bn->prev = NULL;
    bn->type = BULK_STR;
    xx_node->content = (char*)malloc(sizeof(char) * strlen("NX"));
    strcpy(xx_node->content, "NX");
    xx_node->size = (int)strlen("NX");
    xx_node->node = bn;
    parsed_cmd[3] = (GenericNode*)xx_node;

    // Simulate existing key
    char* result = execute_set_nxxx_get(sm, kvp, parsed_cmd);
    key       = create_key_node("test_key_1", 0, 0, strlen("test_key_1"));
    value     = create_value_node_string("value_key_10", BULK_STR, strlen("value_key_10"));
    kvp       = create_key_val_pair(key, value);
    free(xx_node->content);
    xx_node->content = (char*)malloc(sizeof(char) * strlen("XX"));
    strcpy(xx_node->content, "XX");
    result = execute_set_nxxx_get(sm, kvp, parsed_cmd);
    assert(result != NULL); // Ensure result is not NULL for XX success
    assert(strcmp(result, "$11\r\nvalue_key_1\r\n") == 0); // Ensure the correct response message

    free(result);
    free(((KeyNode*)sm->keys[0]->key)->content);
    free(((KeyNode*)sm->keys[0]->key)->input_time);
    free(((KeyNode*)sm->keys[0]->key));
    free(((ValueNodeString*)sm->values[0]->value)->content);
    free(((ValueNodeString*)sm->values[0]->value));
    free(sm->keys);
    free(sm->values);
    free(sm);
    free(parsed_cmd);

    free(xx_node->content);
    free(xx_node);
    free(bn);
}

void test_execute_set_nxxx_get_xx_failure() {
    SimpleMap* sm            = create_simple_map();
    KeyNode*   key           = create_key_node("test_key_1", 0, 0, strlen("test_key_1"));
    ValueNode* value         = create_value_node_string("value_key_1", BULK_STR, strlen("value_key_1"));
    KeyValuePair* kvp        = create_key_val_pair(key, value);
    GenericNode** parsed_cmd = (GenericNode**)calloc(6, sizeof(GenericNode*));
    BulkStringNode* xx_node  = (BulkStringNode*)calloc(1, sizeof(BulkStringNode));
    BaseNode* bn = (BaseNode*)calloc(1, sizeof(BaseNode));
    bn->next = bn->prev = NULL;
    bn->type = BULK_STR;
    xx_node->content = (char*)malloc(sizeof(char) * strlen("XX"));
    strcpy(xx_node->content, "XX");
    xx_node->size = (int)strlen("XX");
    xx_node->node = bn;
    parsed_cmd[3] = (GenericNode*)xx_node;

    // Simulate existing key
    char* result = execute_set_nxxx_get(sm, kvp, parsed_cmd);

    assert(result != NULL); // Ensure result is not NULL for XX success
    assert(strcmp(result, "$-1\r\n") == 0); // Ensure the correct response message

    free(result);
    free(sm->keys);
    free(sm->values);
    free(sm);
    free(parsed_cmd);

    free(xx_node->content);
    free(xx_node);
    free(bn);
}

void test_execute_set_nxxx_get_invalid_option() {
    SimpleMap* sm = create_simple_map();
    KeyNode* key = create_key_node("test_key_1", 0, 0, strlen("test_key_1"));
    ValueNode* value = create_value_node_string("value_key_1", BULK_STR, strlen("value_key_1"));
    KeyValuePair* kvp = create_key_val_pair(key, value);
    GenericNode** parsed_cmd = (GenericNode**)calloc(6, sizeof(GenericNode*));
    BulkStringNode* invalid_node = (BulkStringNode*)calloc(1, sizeof(BulkStringNode));
    BaseNode* bn = (BaseNode*)calloc(1, sizeof(BaseNode));
    bn->next = bn->prev = NULL;
    bn->type = BULK_STR;
    invalid_node->content = (char*)malloc(sizeof(char) * strlen("INVALID"));
    strcpy(invalid_node->content, "INVALID");
    invalid_node->size = (int)strlen("INVALID");
    invalid_node->node = bn;
    parsed_cmd[3] = (GenericNode*)invalid_node;

    char* result = execute_set_nxxx_get(sm, kvp, parsed_cmd);

    assert(result==NULL); // Ensure NULL is returned for invalid option
    free(parsed_cmd);
    free(invalid_node->content);
    free(invalid_node);
    free(bn);
    free(sm->keys);
    free(sm->values);
    free(sm);
};

void test_execute_set_nxxx_get_null_parsed_cmd() {
    SimpleMap* sm = create_simple_map();
    KeyNode* key = create_key_node("test_key_1", 0, 0, strlen("test_key_1"));
    ValueNode* value = create_value_node_string("value_key_1", BULK_STR, strlen("value_key_1"));
    KeyValuePair* kvp = create_key_val_pair(key, value);

    char* result = execute_set_nxxx_get(sm, kvp, NULL);

    assert(result==NULL);
    free(sm->keys);
    free(sm->values);
    free(sm);
}
void test_execute_set_nxxx_get_null_option() {
    SimpleMap* sm = create_simple_map();
    KeyNode* key = create_key_node("test_key_1", 0, 0, strlen("test_key_1"));
    ValueNode* value = create_value_node_string("value_key_1", BULK_STR, strlen("value_key_1"));
    KeyValuePair* kvp = create_key_val_pair(key, value);
    GenericNode** parsed_cmd = (GenericNode**)calloc(6, sizeof(GenericNode*));

    char* result = execute_set_nxxx_get(sm, kvp, parsed_cmd);

    assert(result == NULL); // Ensure NULL is returned for NULL option

    free(parsed_cmd);
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
    
    // clean_up_kv
    test_clean_up_kv_key_only();
    test_clean_up_kv_value_only();
    test_clean_up_kv_both_key_and_value();
    test_clean_up_kv_null_key_and_value();
    test_clean_up_kv_value_other_type();
    
    // create_key_node
    test_create_key_node_valid();
    test_create_key_node_null_content();
    test_create_key_node_invalid_size();
    
    // create_value_node_string
    test_create_value_node_string_valid();
    test_create_value_node_string_null_content();
    test_create_value_node_string_invalid_size();

    test_create_value_node_valid_bulk_str();
    test_create_value_node_invalid_type();
    test_create_value_node_null_gnode();
    
    test_compare_equal_keys();
    test_compare_different_sizes();
    test_compare_different_content();
    test_compare_null_input();
    // execute_set_get
    test_execute_set_get_success_set_new_item();
    test_execute_set_get_success_upgrade();
    test_execute_set_get_error_values_arr_is_null();
    test_execute_set_get_null_inputs();
    test_execute_set_get_error_null_key();
    test_execute_set_get_sucess_null_content_value();
    test_execute_set_get_sucess_replace_null_content_value();

    // execute_set_basic
    test_execute_set_basic_success_set();
    test_execute_set_basic_success_upgrade();
    test_execute_set_basic_null_inputs();
    test_execute_set_basic_key_null_content();
    test_execute_set_basic_empty_key();

    // execute_set_cmd
    test_execute_set_cmd_set_basic();
    test_execute_set_cmd_set_get_no_previous_key();
    test_execute_set_cmd_set_get_upgrade();
    test_execute_set_cmd_null_parsed_cmd();
    test_execute_set_cmd_null_sm();
    test_execute_set_cmd_null_key_or_value();
    
    // execute_set_nx_xx
    test_execute_set_nx_xx_nx_success();
    test_execute_set_nx_xx_nx_failure();
    test_execute_set_nx_xx_xx_success();
    test_execute_set_nx_xx_xx_failure();
    test_execute_set_nx_xx_invalid_option();
    test_execute_set_nx_xx_null_parsed_cmd();
    test_execute_set_nx_xx_null_option();

    // execute_set_nxxx_get_nx
    test_execute_set_nxxx_get_nx_success();
    test_execute_set_nxxx_get_nx_failure();
    test_execute_set_nxxx_get_xx_success();
    test_execute_set_nxxx_get_xx_failure();
    test_execute_set_nxxx_get_invalid_option();
    test_execute_set_nxxx_get_null_parsed_cmd();
    test_execute_set_nxxx_get_null_option();
    puts("All tests passed");
    return 0;
}