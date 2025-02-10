#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "../include/simple_map.h"


struct KeyNode{
    char*            content;
    struct timespec* input_time;
    unsigned int     ex;
    unsigned int     px;
    int              size;
};
struct ValueNode{
    void*       content;
    void*       dtype;
};
struct ValueNodeString{
    void*      content;
    int        dtype;
    int        size;
};

/*===========================================================*/
/*UNIT TESTS*/
/*===========================================================*/
// Test case 1: Remove an existing key
void test_remove_key_existing_key() {
    SimpleMap sm = {NULL, NULL, 10, 2};
    sm.keys = (KeyWrapper**)calloc(2, sizeof(KeyWrapper*));
    sm.values = (ValueWrapper**)calloc(2, sizeof(ValueWrapper*));
    
    KeyNode key_node1 = {"key1", NULL, 0, 0, 4};
    KeyNode key_node2 = {"key2", NULL, 0, 0, 4};
    ValueNodeString value_node1 = {strdup("value1"), 0, 6};
    ValueNodeString value_node2 = {strdup("value2"), 0, 6};
    
    sm.keys[0] = (KeyWrapper*)calloc(1, sizeof(KeyWrapper));
    sm.keys[0]->key = &key_node1;
    sm.values[0] = (ValueWrapper*)calloc(1, sizeof(ValueWrapper));
    sm.values[0]->value = &value_node1;
    
    sm.keys[1] = (KeyWrapper*)calloc(1, sizeof(KeyWrapper));
    sm.keys[1]->key = &key_node2;
    sm.values[1] = (ValueWrapper*)calloc(1, sizeof(ValueWrapper));
    sm.values[1]->value = &value_node2;
    
    KeyValuePair rmv_pair;
    KeyValuePair* result = remove_key(&sm, &key_node1, compare, &rmv_pair);
    
    assert(result == &rmv_pair);
    assert(rmv_pair.key == &key_node1);
    assert(rmv_pair.value == &value_node1);
    assert(sm.top == 1);
    assert(sm.keys[0]->key == &key_node2);
    assert(sm.values[0]->value == &value_node2);
    
    free(sm.keys[0]);
    free(sm.values[0]);
    free(sm.keys);
    free(sm.values);
    free(value_node1.content);
    free(value_node2.content);
}

// Test case 2: Remove a non-existent key
void test_remove_key_non_existent_key() {
    SimpleMap sm = {NULL, NULL, 4, 2};
    sm.keys = (KeyWrapper**)calloc(2, sizeof(KeyWrapper*));
    sm.values = (ValueWrapper**)calloc(2, sizeof(ValueWrapper*));
    
    KeyNode key_node1 = {"key1", NULL, 0, 0, 4};
    KeyNode key_node2 = {"key2", NULL, 0, 0, 4};
    ValueNodeString value_node1 = {strdup("value1"), 0, 6};
    ValueNodeString value_node2 = {strdup("value2"), 0, 6};
    
    sm.keys[0] = (KeyWrapper*)calloc(1, sizeof(KeyWrapper));
    sm.keys[0]->key = &key_node1;
    sm.values[0] = (ValueWrapper*)calloc(1, sizeof(ValueWrapper));
    sm.values[0]->value = &value_node1;
    
    sm.keys[1] = (KeyWrapper*)calloc(1, sizeof(KeyWrapper));
    sm.keys[1]->key = &key_node2;
    sm.values[1] = (ValueWrapper*)calloc(1, sizeof(ValueWrapper));
    sm.values[1]->value = &value_node2;
    
    KeyNode key_node3 = {"key3", NULL, 0, 0, 4};
    KeyValuePair rmv_pair;
    KeyValuePair* result = remove_key(&sm, &key_node3, compare, &rmv_pair);
    
    assert(result == NULL);
    assert(sm.top == 2);
    
    free(sm.keys[0]);
    free(sm.keys[1]);
    free(sm.values[0]);
    free(sm.values[1]);
    free(sm.keys);
    free(sm.values);
    free(value_node1.content);
    free(value_node2.content);
}

// Test case 3: NULL SimpleMap
void test_remove_key_null_simple_map() {
    KeyNode key_node1 = {"key1", NULL, 0, 0, 4};
    KeyValuePair rmv_pair;
    KeyValuePair* result = remove_key(NULL, &key_node1, compare, &rmv_pair);
    assert(result == NULL);
}

// Test case 4: NULL key
void test_remove_key_null_key() {
    SimpleMap sm = {NULL, NULL, 10, 0};
    KeyValuePair rmv_pair;
    KeyValuePair* result = remove_key(&sm, NULL, compare, &rmv_pair);
    assert(result == NULL);
}

// Test case 5: NULL compare function
void test_remove_key_null_compare_function() {
    SimpleMap sm = {NULL, NULL, 10, 0};
    KeyNode key_node1 = {"key1", NULL, 0, 0, 4};
    KeyValuePair rmv_pair;
    KeyValuePair* result = remove_key(&sm, &key_node1, NULL, &rmv_pair);
    assert(result == NULL);
}

// Test case 6: NULL rmv_pair
void test_remove_key_null_rmv_pair() {
    SimpleMap sm = {NULL, NULL, 10, 0};
    KeyNode key_node1 = {"key1", NULL, 0, 0, 4};
    KeyValuePair* result = remove_key(&sm, &key_node1, compare, NULL);
    assert(result == NULL);
}

// Test case 7: NULL keys or values array
void test_remove_key_null_keys_or_values() {
    SimpleMap sm = {NULL, NULL, 10, 0};
    KeyNode key_node1 = {"key1", NULL, 0, 0, 4};
    KeyValuePair rmv_pair;
    KeyValuePair* result = remove_key(&sm, &key_node1, compare, &rmv_pair);
    assert(result == NULL);
}
// Test case 8: Successfully double the arrays
void test_double_arrays_success() {
    SimpleMap sm = {NULL, NULL, 2, 1};
    sm.keys = (KeyWrapper**)calloc(2, sizeof(KeyWrapper*));
    sm.values = (ValueWrapper**)calloc(2, sizeof(ValueWrapper*));

    SimpleMap* result = __double_arrays(&sm);

    assert(result == &sm);
    assert(sm.capacity == 4);
    assert(sm.keys != NULL);
    assert(sm.values != NULL);

    free(sm.keys);
    free(sm.values);
}

// Test case 9: NULL SimpleMap
void test_double_arrays_null_simple_map() {
    SimpleMap* result = __double_arrays(NULL);
    assert(result == NULL);
}

// Test case 10: NULL keys or values array
void test_double_arrays_null_keys_or_values() {
    SimpleMap sm = {NULL, NULL, 2, 0};
    SimpleMap* result = __double_arrays(&sm);
    assert(result == NULL);
}

// Test case 11: Capacity is zero
void test_double_arrays_zero_capacity() {
    SimpleMap sm = {NULL, NULL, 0, 0};
    sm.keys = (KeyWrapper**)calloc(0, sizeof(KeyWrapper*));
    sm.values = (ValueWrapper**)calloc(0, sizeof(ValueWrapper*));

    SimpleMap* result = __double_arrays(&sm);

    assert(result == NULL);
    assert(sm.capacity == 0);

    free(sm.keys);
    free(sm.values);
}

// Test case 12: Realloc failure for keys
void test_double_arrays_realloc_keys_failure() {
    SimpleMap sm = {NULL, NULL, 2, 0};
    sm.keys = (KeyWrapper**)calloc(2, sizeof(KeyWrapper*));
    sm.values = (ValueWrapper**)calloc(2, sizeof(ValueWrapper*));

    // Force realloc to fail by passing a large size
    SimpleMap* result = __double_arrays(&sm);

    assert(result == NULL);

    free(sm.keys);
    free(sm.values);
}

// Test case 13: Realloc failure for values
void test_double_arrays_realloc_values_failure() {
    SimpleMap sm = {NULL, NULL, 2, 0};
    sm.keys = (KeyWrapper**)calloc(2, sizeof(KeyWrapper*));
    sm.values = (ValueWrapper**)calloc(2, sizeof(ValueWrapper*));

    // Force realloc to fail for values
    SimpleMap* result = __double_arrays(&sm);

    assert(result == NULL);

    free(sm.keys);
    free(sm.values);
}
// Helper function to create a SimpleMap for testing
SimpleMap* create_test_simple_map(int capacity, int top) {
    SimpleMap* sm = (SimpleMap*)calloc(1, sizeof(SimpleMap));
    sm->capacity = capacity;
    sm->top = top;
    sm->keys = (KeyWrapper**)calloc(capacity, sizeof(KeyWrapper*));
    sm->values = (ValueWrapper**)calloc(capacity, sizeof(ValueWrapper*));
    return sm;
}

// Test case 14: Successfully set a KeyValuePair
void test__set_success() {
    SimpleMap* sm = create_test_simple_map(2, 0);
    KeyNode key_node = {"key1", NULL, 0, 0, 4};
    ValueNodeString value_node = {strdup("value1"), 0, 6};
    KeyValuePair kvp = {&key_node, &value_node};

    KeyValuePair* result = __set(sm, &kvp);

    assert(result == &kvp);
    assert(sm->top == 1);
    assert(sm->keys[1]->key == &key_node);
    assert(sm->values[1]->value == &value_node);

    free(sm->keys[1]);
    free(sm->values[1]);
    free(sm->keys);
    free(sm->values);
    free(sm);
    free(value_node.content);
}

// Test case 15: Set a KeyValuePair when the map is full (trigger __double_arrays)
void test__set_full_map() {
    SimpleMap* sm = create_test_simple_map(1, 0);
    KeyNode key_node1 = {"key1", NULL, 0, 0, 4};
    ValueNodeString value_node1 = {strdup("value1"), 0, 6};
    KeyValuePair kvp1 = {&key_node1, &value_node1};

    KeyValuePair* result = __set(sm, &kvp1);

    assert(result == &kvp1);
    assert(sm->top == 1);
    assert(sm->capacity == 2); // Capacity should have doubled
    assert(sm->keys[1]->key == &key_node1);
    assert(sm->values[1]->value == &value_node1);

    free(sm->keys[1]);
    free(sm->values[1]);
    free(sm->keys);
    free(sm->values);
    free(sm);
    free(value_node1.content);
}

// Test case 16: NULL SimpleMap
void test__set_null_simple_map() {
    KeyNode key_node = {"key1", NULL, 0, 0, 4};
    ValueNodeString value_node = {strdup("value1"), 0, 6};
    KeyValuePair kvp = {&key_node, &value_node};

    KeyValuePair* result = __set(NULL, &kvp);
    assert(result == NULL);

    free(value_node.content);
}

// Test case 17: NULL KeyValuePair
void test__set_null_key_value_pair() {
    SimpleMap* sm = create_test_simple_map(2, 0);
    KeyValuePair* result = __set(sm, NULL);
    assert(result == NULL);

    free(sm->keys);
    free(sm->values);
    free(sm);
}

// Test case 18: NULL key in KeyValuePair
void test__set_null_key() {
    SimpleMap* sm = create_test_simple_map(2, 0);
    ValueNodeString value_node = {strdup("value1"), 0, 6};
    KeyValuePair kvp = {NULL, &value_node};

    KeyValuePair* result = __set(sm, &kvp);
    assert(result == NULL);

    free(sm->keys);
    free(sm->values);
    free(sm);
    free(value_node.content);
}

// Test case 19: NULL keys or values array in SimpleMap
void test__set_null_keys_or_values() {
    SimpleMap sm = {NULL, NULL, 2, 0};
    KeyNode key_node = {"key1", NULL, 0, 0, 4};
    ValueNodeString value_node = {strdup("value1"), 0, 6};
    KeyValuePair kvp = {&key_node, &value_node};

    KeyValuePair* result = __set(&sm, &kvp);
    assert(result == NULL);

    free(value_node.content);
}

// Test case 20: __double_arrays failure
void test__set_double_arrays_failure() {
    SimpleMap* sm = create_test_simple_map(1, 0);
    KeyNode key_node1 = {"key1", NULL, 0, 0, 4};
    ValueNodeString value_node1 = {strdup("value1"), 0, 6};
    KeyValuePair kvp1 = {&key_node1, &value_node1};

    // Force __double_arrays to fail by setting capacity to 0
    sm->capacity = 0;

    KeyValuePair* result = __set(sm, &kvp1);
    assert(result == NULL);

    free(sm->keys);
    free(sm->values);
    free(sm);
    free(value_node1.content);
}
// Test case 21: Successfully upgrade a KeyValuePair at a valid position
void test_upgrade_success() {
    SimpleMap* sm = create_test_simple_map(2, 1);

    KeyNode key_node1 = {"key1", NULL, 0, 0, 4};
    ValueNodeString value_node1 = {strdup("value1"), 0, 6};
    sm->keys[0] = (KeyWrapper*)calloc(1, sizeof(KeyWrapper));
    sm->keys[0]->key = &key_node1;
    sm->values[0] = (ValueWrapper*)calloc(1, sizeof(ValueWrapper));
    sm->values[0]->value = &value_node1;

    KeyNode key_node2 = {"key2", NULL, 0, 0, 4};
    ValueNodeString value_node2 = {strdup("value2"), 0, 6};
    KeyValuePair kvp = {&key_node2, &value_node2};

    void* result = __upgrade(sm, 0, &kvp);

    assert(result == &value_node1); // Old value should be returned
    assert(sm->keys[0]->key == &key_node2); // Key should be updated
    assert(sm->values[0]->value == &value_node2); // Value should be updated
    assert(kvp.key == &key_node1); // Old key should be in the KeyValuePair
    assert(kvp.value == &value_node1); // Old value should be in the KeyValuePair

    free(sm->keys[0]);
    free(sm->values[0]);
    free(sm->keys);
    free(sm->values);
    free(sm);
    free(value_node1.content);
    free(value_node2.content);
}

// Test case 22: NULL SimpleMap
void test_upgrade_null_simple_map() {
    KeyNode key_node = {"key1", NULL, 0, 0, 4};
    ValueNodeString value_node = {strdup("value1"), 0, 6};
    KeyValuePair kvp = {&key_node, &value_node};

    void* result = __upgrade(NULL, 0, &kvp);
    assert(result == NULL);

    free(value_node.content);
}

// Test case 23: NULL KeyValuePair
void test_upgrade_null_key_value_pair() {
    SimpleMap* sm = create_test_simple_map(2, 0);
    void* result = __upgrade(sm, 0, NULL);
    assert(result == NULL);

    free(sm->keys);
    free(sm->values);
    free(sm);
}

// Test case 24: NULL key in KeyValuePair
void test_upgrade_null_key() {
    SimpleMap* sm = create_test_simple_map(2, 0);
    ValueNodeString value_node = {strdup("value1"), 0, 6};
    KeyValuePair kvp = {NULL, &value_node};

    void* result = __upgrade(sm, 0, &kvp);
    assert(result == NULL);

    free(sm->keys);
    free(sm->values);
    free(sm);
    free(value_node.content);
}

// Test case 25: NULL keys or values array in SimpleMap
void test_upgrade_null_keys_or_values() {
    SimpleMap sm = {NULL, NULL, 2, 0};
    KeyNode key_node = {"key1", NULL, 0, 0, 4};
    ValueNodeString value_node = {strdup("value1"), 0, 6};
    KeyValuePair kvp = {&key_node, &value_node};

    void* result = __upgrade(&sm, 0, &kvp);
    assert(result == NULL);

    free(value_node.content);
}

// Test case 26: Invalid position (negative)
void test_upgrade_invalid_position_negative() {
    SimpleMap* sm = create_test_simple_map(2, 0);
    KeyNode key_node = {"key1", NULL, 0, 0, 4};
    ValueNodeString value_node = {strdup("value1"), 0, 6};
    KeyValuePair kvp = {&key_node, &value_node};

    void* result = __upgrade(sm, -1, &kvp);
    assert(result == NULL);

    free(sm->keys);
    free(sm->values);
    free(sm);
    free(value_node.content);
}

// Test case 27: Invalid position (greater than top)
void test_upgrade_invalid_position_greater_than_top() {
    SimpleMap* sm = create_test_simple_map(2, 0);
    KeyNode key_node = {"key1", NULL, 0, 0, 4};
    ValueNodeString value_node = {strdup("value1"), 0, 6};
    KeyValuePair kvp = {&key_node, &value_node};

    void* result = __upgrade(sm, 1, &kvp);
    assert(result == NULL);

    free(sm->keys);
    free(sm->values);
    free(sm);
    free(value_node.content);
}

// Test case 28: Successfully set a new KeyValuePair (KEY_NOT_FOUND)
void test_set_new_key_value_pair() {
    SimpleMap* sm = create_test_simple_map(2, -1);
    KeyNode key_node = {"key1", NULL, 0, 0, 4};
    ValueNodeString value_node = {strdup("value1"), 0, 6};
    KeyValuePair kvp = {&key_node, &value_node};

    KeyValuePair* result = set(sm, &kvp, compare);

    assert(result == &kvp);
    assert(sm->top == 0);
    assert(sm->keys[0]->key == &key_node);
    assert(sm->values[0]->value == &value_node);

    free(sm->keys[0]);
    free(sm->values[0]);
    free(sm->keys);
    free(sm->values);
    free(sm);
    free(value_node.content);
}

// Test case 29: Successfully upgrade an existing KeyValuePair
void test_set_upgrade_existing_key_value_pair() {
    SimpleMap* sm = create_test_simple_map(2, 0);

    KeyNode key_node1 = {"key1", NULL, 0, 0, 4};
    ValueNodeString value_node1 = {strdup("value1"), 0, 6};
    sm->keys[0] = (KeyWrapper*)calloc(1, sizeof(KeyWrapper));
    sm->keys[0]->key = &key_node1;
    sm->values[0] = (ValueWrapper*)calloc(1, sizeof(ValueWrapper));
    sm->values[0]->value = &value_node1;
    sm->top = 0;

    KeyNode key_node2 = {"key1", NULL, 0, 0, 4}; // Same key as key_node1
    ValueNodeString value_node2 = {strdup("value2"), 0, 6};
    KeyValuePair kvp = {&key_node2, &value_node2};

    KeyValuePair* result = set(sm, &kvp, compare);

    assert(result == &value_node1); // Old value should be returned
    assert(sm->keys[0]->key == &key_node2); // Key should be updated
    assert(sm->values[0]->value == &value_node2); // Value should be updated
    assert(kvp.key == &key_node1); // Old key should be in the KeyValuePair
    assert(kvp.value == &value_node1); // Old value should be in the KeyValuePair

    free(sm->keys[0]);
    free(sm->values[0]);
    free(sm->keys);
    free(sm->values);
    free(sm);
    free(value_node1.content);
    free(value_node2.content);
}

// Test case 30: NULL SimpleMap
void test_set_null_simple_map() {
    KeyNode key_node = {"key1", NULL, 0, 0, 4};
    ValueNodeString value_node = {strdup("value1"), 0, 6};
    KeyValuePair kvp = {&key_node, &value_node};

    KeyValuePair* result = set(NULL, &kvp, compare);
    assert(result == NULL);

    free(value_node.content);
}

// Test case 31: NULL KeyValuePair
void test_set_null_key_value_pair() {
    SimpleMap* sm = create_test_simple_map(2, 0);
    KeyValuePair* result = set(sm, NULL, compare);
    assert(result == NULL);

    free(sm->keys);
    free(sm->values);
    free(sm);
}

// Test case 32: NULL key in KeyValuePair
void test_set_null_key() {
    SimpleMap* sm = create_test_simple_map(2, 0);
    ValueNodeString value_node = {strdup("value1"), 0, 6};
    KeyValuePair kvp = {NULL, &value_node};

    KeyValuePair* result = set(sm, &kvp, compare);
    assert(result == NULL);

    free(sm->keys);
    free(sm->values);
    free(sm);
    free(value_node.content);
}

// Test case 33: __find returns FIND_KEY_ERROR
void test_set_find_error() {
    SimpleMap sm = {NULL, NULL, 2, 0};
    KeyNode key_node = {"key1", NULL, 0, 0, 4};
    ValueNodeString value_node = {strdup("value1"), 0, 6};
    KeyValuePair kvp = {&key_node, &value_node};

    KeyValuePair* result = set(&sm, &kvp, compare);
    assert(result == NULL);

    free(value_node.content);
}

// Test case 34: __set failure (e.g., due to __double_arrays failure)
void test_set_failure() {
    SimpleMap* sm = create_test_simple_map(1, 0);
    KeyNode key_node1 = {"key1", NULL, 0, 0, 4};
    ValueNodeString value_node1 = {strdup("value1"), 0, 6};
    KeyValuePair kvp1 = {&key_node1, &value_node1};

    // Force __set to fail by setting capacity to 0
    sm->capacity = 0;

    KeyValuePair* result = set(sm, &kvp1, compare);
    assert(result == NULL);

    free(sm->keys);
    free(sm->values);
    free(sm);
    free(value_node1.content);
}
// Test case 35: Successfully get an existing KeyValuePair
void test_get_existing_key_value_pair() {
    SimpleMap* sm = create_test_simple_map(2, 0);

    KeyNode key_node = {"key1", NULL, 0, 0, 4};
    ValueNodeString value_node = {strdup("value1"), 0, 6};
    sm->keys[0] = (KeyWrapper*)calloc(1, sizeof(KeyWrapper));
    sm->keys[0]->key = &key_node;
    sm->values[0] = (ValueWrapper*)calloc(1, sizeof(ValueWrapper));
    sm->values[0]->value = &value_node;
    sm->top = 0;

    KeyValuePair* result = get(sm, &key_node, compare);

    assert(result != NULL);
    assert(result->key == &key_node);
    assert(result->value == &value_node);

    free(result);
    free(sm->keys[0]);
    free(sm->values[0]);
    free(sm->keys);
    free(sm->values);
    free(sm);
    free(value_node.content);
}

// Test case 36: Key not found (KEY_NOT_FOUND)
void test_get_key_not_found() {
    SimpleMap* sm = create_test_simple_map(2, 0);

    KeyNode key_node1 = {"key1", NULL, 0, 0, 4};
    ValueNodeString value_node1 = {strdup("value1"), 0, 6};
    sm->keys[0] = (KeyWrapper*)calloc(1, sizeof(KeyWrapper));
    sm->keys[0]->key = &key_node1;
    sm->values[0] = (ValueWrapper*)calloc(1, sizeof(ValueWrapper));
    sm->values[0]->value = &value_node1;
    sm->top = 0;

    KeyNode key_node2 = {"key2", NULL, 0, 0, 4}; // Key not in the map
    KeyValuePair* result = get(sm, &key_node2, compare);

    assert(result == NULL);

    free(sm->keys[0]);
    free(sm->values[0]);
    free(sm->keys);
    free(sm->values);
    free(sm);
    free(value_node1.content);
}

// Test case 37: NULL SimpleMap
void test_get_null_simple_map() {
    KeyNode key_node = {"key1", NULL, 0, 0, 4};
    KeyValuePair* result = get(NULL, &key_node, compare);
    assert(result == NULL);
}

// Test case 38: NULL key
void test_get_null_key() {
    SimpleMap* sm = create_test_simple_map(2, 0);
    KeyValuePair* result = get(sm, NULL, compare);
    assert(result == NULL);

    free(sm->keys);
    free(sm->values);
    free(sm);
}

// Test case 39: NULL compare function
void test_get_null_compare_function() {
    SimpleMap* sm = create_test_simple_map(2, 0);
    KeyNode key_node = {"key1", NULL, 0, 0, 4};
    KeyValuePair* result = get(sm, &key_node, NULL);
    assert(result == NULL);

    free(sm->keys);
    free(sm->values);
    free(sm);
}

// Test case 6: __find returns FIND_KEY_ERROR
void test_get_find_key_error() {
    SimpleMap sm = {NULL, NULL, 2, 0};
    KeyNode key_node = {"key1", NULL, 0, 0, 4};
    KeyValuePair* result = get(&sm, &key_node, compare);
    assert(result == NULL);
}

// Test case 7: __find returns KEY_ARR_ERROR
void test_get_key_arr_error() {
    SimpleMap sm = {NULL, NULL, 2, 0};
    KeyNode key_node = {"key1", NULL, 0, 0, 4};
    KeyValuePair* result = get(&sm, &key_node, compare);
    assert(result == NULL);
}

// Test case 8: Memory allocation failure for KeyValuePair
void test_get_memory_allocation_failure() {
    SimpleMap* sm = create_test_simple_map(2, 0);

    KeyNode key_node = {"key1", NULL, 0, 0, 4};
    ValueNodeString value_node = {strdup("value1"), 0, 6};
    sm->keys[0] = (KeyWrapper*)calloc(1, sizeof(KeyWrapper));
    sm->keys[0]->key = &key_node;
    sm->values[0] = (ValueWrapper*)calloc(1, sizeof(ValueWrapper));
    sm->values[0]->value = &value_node;
    KeyNode key_node_1 = {"key2", NULL, 0, 0, 4};
    ValueNodeString value_node_1 = {strdup("value2"), 0, 6};
    sm->keys[1] = (KeyWrapper*)calloc(1, sizeof(KeyWrapper));
    sm->keys[1]->key = &key_node_1;
    sm->values[1] = (ValueWrapper*)calloc(1, sizeof(ValueWrapper));
    sm->values[1]->value = &value_node_1;
    sm->top = 0;

    // Force memory allocation failure for KeyValuePair
    KeyValuePair* result = get(sm, &key_node_1, compare);

    assert(result == NULL);

    free(sm->keys[0]);
    free(sm->values[0]);
    free(sm->keys[1]);
    free(sm->values[1]);
    free(sm->keys);
    free(sm->values);
    free(sm);
    free(value_node.content);
}


// Main function to run all tests
int main() {
    test_remove_key_existing_key();
    test_remove_key_non_existent_key();
    test_remove_key_null_simple_map();
    test_remove_key_null_key();
    test_remove_key_null_compare_function();
    test_remove_key_null_rmv_pair();
    test_remove_key_null_keys_or_values();
    test_double_arrays_success();
    test_double_arrays_null_simple_map();
    test_double_arrays_null_keys_or_values();
    test_double_arrays_zero_capacity();
    test_double_arrays_realloc_keys_failure();
    test_double_arrays_realloc_values_failure();
    test__set_success();
    test__set_full_map();
    test__set_null_simple_map();
    test__set_null_key_value_pair();
    test__set_null_key();
    test__set_null_keys_or_values();
    test__set_double_arrays_failure();
    test_upgrade_success();
    test_upgrade_null_simple_map();
    test_upgrade_null_key_value_pair();
    test_upgrade_null_key();
    test_upgrade_null_keys_or_values();
    test_upgrade_invalid_position_negative();
    test_upgrade_invalid_position_greater_than_top();
    test_set_new_key_value_pair();
    test_set_upgrade_existing_key_value_pair();
    test_set_null_simple_map();
    test_set_null_key_value_pair();
    test_set_null_key();
    test_set_find_error();
    test_set_failure();
    test_get_existing_key_value_pair();
    test_get_key_not_found();
    test_get_null_simple_map();
    test_get_null_key();
    test_get_null_compare_function();
    test_get_find_key_error();
    test_get_key_arr_error();
    test_get_memory_allocation_failure();
    printf("All tests passed!\n");
    return 0;
}