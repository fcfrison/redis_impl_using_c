#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "../include/simple_map.h"

/*
===========================================================
UNIT TESTS
----------

The tests were developed with the help of AI algorithms.
===========================================================
*/





void* 
compare(const void* a, const void* b){
    const KeyNode* el_a     = (const KeyNode*)a;
    const KeyNode* el_b     = (const KeyNode*)b;
    if(!el_a || !el_b){
        return NULL;
    }
    if(el_a->size!=el_b->size){
            return NULL;
        }
    if(memcmp(el_a->content, el_b->content,el_a->size)==0){
        return (void*)el_a;
    }
    return NULL;
}

// Include the necessary struct definitions and function declarations here
// (Assuming they are already defined as provided in the question)

// Helper function to create a KeyNode
KeyNode* create_key_node(char* content, unsigned int ex, unsigned int px) {
    KeyNode* node = (KeyNode*)malloc(sizeof(KeyNode));
    node->content = strdup(content);
    node->input_time = (struct timespec*)malloc(sizeof(struct timespec));
    clock_gettime(CLOCK_REALTIME, node->input_time);
    node->ex = ex;
    node->px = px;
    node->size = strlen(content);
    return node;
}

// Helper function to create a ValueNodeString
ValueNodeString* create_value_node_string(char* content) {
    ValueNodeString* node = (ValueNodeString*)malloc(sizeof(ValueNodeString));
    node->content = strdup(content);
    node->dtype = 1; // Assuming 1 represents a string type
    node->size = strlen(content);
    return node;
}

// Helper function to free a SimpleMap
void free_simple_map(SimpleMap* sm) {
    for (int i = 0; i <= sm->top; i++) {
        free(sm->keys[i]);
        free(sm->values[i]);
    }
    free(sm->keys);
    free(sm->values);
    free(sm);
}

// Test 1: Remove a key from an empty map
void test_remove_key_empty_map() {
    SimpleMap* sm = create_simple_map();
    assert(sm != NULL);

    KeyNode* key_to_remove = create_key_node("key1", 0, 0);
    KeyValuePair rmv_pair = {NULL, NULL};

    void* result = remove_key(sm, key_to_remove, compare, &rmv_pair);

    assert(result == NULL);
    assert(rmv_pair.key == NULL);
    assert(rmv_pair.value == NULL);

    free(key_to_remove);
    free_simple_map(sm);
}

// Test 2: Remove a key that exists in the map
void test_remove_key_existing_key() {
    SimpleMap* sm = create_simple_map();
    assert(sm != NULL);

    KeyNode* key1 = create_key_node("key1", 0, 0);
    ValueNodeString* value1 = create_value_node_string("value1");

    sm->keys[0] = (KeyWrapper*)malloc(sizeof(KeyWrapper));
    sm->keys[0]->key = key1;
    sm->values[0] = (ValueWrapper*)malloc(sizeof(ValueWrapper));
    sm->values[0]->value = value1;
    sm->top = 0;

    KeyValuePair rmv_pair = {NULL, NULL};
    void* result = remove_key(sm, key1, compare, &rmv_pair);

    assert(result == key1);
    assert(rmv_pair.key == key1);
    assert(rmv_pair.value == value1);
    assert(sm->top == -1);

    free(key1);
    free(value1);
    free_simple_map(sm);
}

// Test 3: Remove a key that does not exist in the map
void test_remove_key_non_existing_key() {
    SimpleMap* sm = create_simple_map();
    assert(sm != NULL);

    KeyNode* key1 = create_key_node("key1", 0, 0);
    ValueNodeString* value1 = create_value_node_string("value1");

    sm->keys[0] = (KeyWrapper*)malloc(sizeof(KeyWrapper));
    sm->keys[0]->key = key1;
    sm->values[0] = (ValueWrapper*)malloc(sizeof(ValueWrapper));
    sm->values[0]->value = value1;
    sm->top = 0;

    KeyNode* key_to_remove = create_key_node("key2", 0, 0);
    KeyValuePair rmv_pair = {NULL, NULL};

    void* result = remove_key(sm, key_to_remove, compare, &rmv_pair);

    assert(result == NULL);
    assert(rmv_pair.key == NULL);
    assert(rmv_pair.value == NULL);
    assert(sm->top == 0);

    free(key1);
    free(value1);
    free(key_to_remove);
    free_simple_map(sm);
}

// Test 4: Remove a key from a map with multiple keys
void test_remove_key_multiple_keys() {
    SimpleMap* sm = create_simple_map();
    assert(sm != NULL);

    // Resize the map to accommodate multiple keys
    sm->keys = (KeyWrapper**)realloc(sm->keys, 2 * sizeof(KeyWrapper*));
    sm->values = (ValueWrapper**)realloc(sm->values, 2 * sizeof(ValueWrapper*));
    sm->capacity = 2;

    KeyNode* key1 = create_key_node("key1", 0, 0);
    KeyNode* key2 = create_key_node("key2", 0, 0);
    ValueNodeString* value1 = create_value_node_string("value1");
    ValueNodeString* value2 = create_value_node_string("value2");

    sm->keys[0] = (KeyWrapper*)malloc(sizeof(KeyWrapper));
    sm->keys[0]->key = key1;
    sm->values[0] = (ValueWrapper*)malloc(sizeof(ValueWrapper));
    sm->values[0]->value = value1;

    sm->keys[1] = (KeyWrapper*)malloc(sizeof(KeyWrapper));
    sm->keys[1]->key = key2;
    sm->values[1] = (ValueWrapper*)malloc(sizeof(ValueWrapper));
    sm->values[1]->value = value2;
    sm->top = 1;

    KeyValuePair rmv_pair = {NULL, NULL};
    void* result = remove_key(sm, key1, compare, &rmv_pair);

    assert(result == key1);
    assert(rmv_pair.key == key1);
    assert(rmv_pair.value == value1);
    assert(sm->top == 0);
    assert(sm->keys[0]->key == key2);
    assert(sm->values[0]->value == value2);

    free(key1);
    free(key2);
    free(value1);
    free(value2);
    free_simple_map(sm);
}

// Test 5: Remove the last key in the map
void test_remove_key_last_key() {
    SimpleMap* sm = create_simple_map();
    assert(sm != NULL);

    KeyNode* key1 = create_key_node("key1", 0, 0);
    ValueNodeString* value1 = create_value_node_string("value1");

    sm->keys[0] = (KeyWrapper*)malloc(sizeof(KeyWrapper));
    sm->keys[0]->key = key1;
    sm->values[0] = (ValueWrapper*)malloc(sizeof(ValueWrapper));
    sm->values[0]->value = value1;
    sm->top = 0;

    KeyValuePair rmv_pair = {NULL, NULL};
    void* result = remove_key(sm, key1, compare, &rmv_pair);

    assert(result == key1);
    assert(rmv_pair.key == key1);
    assert(rmv_pair.value == value1);
    assert(sm->top == -1);

    free(key1);
    free(value1);
    free_simple_map(sm);
}
// Test 6: Double the capacity of an empty map
void test_double_arrays_empty_map() {
    SimpleMap* sm = create_simple_map();
    assert(sm != NULL);

    SimpleMap* result = double_arrays(sm);

    assert(result != NULL);
    assert(sm->capacity == 2); // Capacity should double from 1 to 2
    assert(sm->keys != NULL);
    assert(sm->values != NULL);

    free_simple_map(sm);
}

// Test 7: Double the capacity of a map with one element
void test_double_arrays_one_element() {
    SimpleMap* sm = create_simple_map();
    assert(sm != NULL);

    // Add one element to the map
    sm->keys[0] = (KeyWrapper*)malloc(sizeof(KeyWrapper));
    sm->values[0] = (ValueWrapper*)malloc(sizeof(ValueWrapper));
    sm->top = 0;

    SimpleMap* result = double_arrays(sm);

    assert(result != NULL);
    assert(sm->capacity == 2); // Capacity should double from 1 to 2
    assert(sm->keys != NULL);
    assert(sm->values != NULL);

    free_simple_map(sm);
}

// Test 8: Double the capacity of a map with multiple elements
void test_double_arrays_multiple_elements() {
    SimpleMap* sm = create_simple_map();
    assert(sm != NULL);

    // Manually resize the map to accommodate multiple elements
    sm->keys = (KeyWrapper**)realloc(sm->keys, 2 * sizeof(KeyWrapper*));
    sm->values = (ValueWrapper**)realloc(sm->values, 2 * sizeof(ValueWrapper*));
    sm->capacity = 2;

    // Add two elements to the map
    sm->keys[0] = (KeyWrapper*)malloc(sizeof(KeyWrapper));
    sm->values[0] = (ValueWrapper*)malloc(sizeof(ValueWrapper));
    sm->keys[1] = (KeyWrapper*)malloc(sizeof(KeyWrapper));
    sm->values[1] = (ValueWrapper*)malloc(sizeof(ValueWrapper));
    sm->top = 1;

    SimpleMap* result = double_arrays(sm);

    assert(result != NULL);
    assert(sm->capacity == 4); // Capacity should double from 2 to 4
    assert(sm->keys != NULL);
    assert(sm->values != NULL);

    free_simple_map(sm);
}

// Test 9: Double the capacity when realloc fails for keys
void test_double_arrays_realloc_keys_fails() {
    SimpleMap* sm = create_simple_map();
    assert(sm != NULL);

    // Force realloc to fail for keys
    sm->keys = NULL;

    SimpleMap* result = double_arrays(sm);

    assert(result == NULL); // Function should return NULL if realloc fails
    assert(sm->capacity == 1); // Capacity should remain unchanged

    free_simple_map(sm);
}

// Test 10: Double the capacity when realloc fails for values
void test_double_arrays_realloc_values_fails() {
    SimpleMap* sm = create_simple_map();
    assert(sm != NULL);

    // Force realloc to fail for values
    sm->values = NULL;

    SimpleMap* result = double_arrays(sm);

    assert(result == NULL); // Function should return NULL if realloc fails
    assert(sm->capacity == 1); // Capacity should remain unchanged

    free_simple_map(sm);
}

// Test 11: Double the capacity of a NULL map
void test_double_arrays_null_map() {
    SimpleMap* sm = NULL;

    SimpleMap* result = double_arrays(sm);

    assert(result == NULL); // Function should return NULL for a NULL map
}

// Test 12: Double the capacity of a map with zero capacity
void test_double_arrays_zero_capacity() {
    SimpleMap* sm = create_simple_map();
    assert(sm != NULL);

    // Set capacity to zero
    sm->capacity = 0;

    SimpleMap* result = double_arrays(sm);

    assert(result != NULL);
    assert(sm->capacity == 0); // Capacity should remain zero

    free_simple_map(sm);
}


// Helper function to create a KeyValuePair
KeyValuePair create_key_value_pair(void* key, void* value) {
    KeyValuePair kvp;
    kvp.key = key;
    kvp.value = value;
    return kvp;
}

// Test 13: Set a key-value pair in an empty map
void test_set_empty_map() {
    SimpleMap* sm = create_simple_map();
    assert(sm != NULL);

    KeyNode* key = (KeyNode*)malloc(sizeof(KeyNode));
    ValueNodeString* value = (ValueNodeString*)malloc(sizeof(ValueNodeString));
    KeyValuePair kvp = create_key_value_pair(key, value);

    SimpleMap* result = set(sm, &kvp);

    assert(result != NULL);
    assert(sm->top == 0);
    assert(sm->keys[0]->key == key);
    assert(sm->values[0]->value == value);

    free(key);
    free(value);
    free_simple_map(sm);
}

// Test 14: Set a key-value pair in a map with one element
void test_set_one_element() {
    SimpleMap* sm = create_simple_map();
    assert(sm != NULL);

    // Add one element to the map
    KeyNode* key1 = (KeyNode*)malloc(sizeof(KeyNode));
    ValueNodeString* value1 = (ValueNodeString*)malloc(sizeof(ValueNodeString));
    KeyValuePair kvp1 = create_key_value_pair(key1, value1);
    set(sm, &kvp1);

    // Add another element
    KeyNode* key2 = (KeyNode*)malloc(sizeof(KeyNode));
    ValueNodeString* value2 = (ValueNodeString*)malloc(sizeof(ValueNodeString));
    KeyValuePair kvp2 = create_key_value_pair(key2, value2);

    SimpleMap* result = set(sm, &kvp2);

    assert(result != NULL);
    assert(sm->top == 1);
    assert(sm->keys[1]->key == key2);
    assert(sm->values[1]->value == value2);
    assert(sm->capacity == 2); // Capacity should have doubled

    free(key1);
    free(value1);
    free(key2);
    free(value2);
    free_simple_map(sm);
}

// Test 15: Set a key-value pair in a full map
void test_set_full_map() {
    SimpleMap* sm = create_simple_map();
    assert(sm != NULL);

    // Fill the map to its initial capacity
    KeyNode* key1 = (KeyNode*)malloc(sizeof(KeyNode));
    ValueNodeString* value1 = (ValueNodeString*)malloc(sizeof(ValueNodeString));
    KeyValuePair kvp1 = create_key_value_pair(key1, value1);
    set(sm, &kvp1);

    // Add another element to trigger doubling
    KeyNode* key2 = (KeyNode*)malloc(sizeof(KeyNode));
    ValueNodeString* value2 = (ValueNodeString*)malloc(sizeof(ValueNodeString));
    KeyValuePair kvp2 = create_key_value_pair(key2, value2);

    SimpleMap* result = set(sm, &kvp2);

    assert(result != NULL);
    assert(sm->top == 1);
    assert(sm->keys[1]->key == key2);
    assert(sm->values[1]->value == value2);
    assert(sm->capacity == 2); // Capacity should have doubled

    free(key1);
    free(value1);
    free(key2);
    free(value2);
    free_simple_map(sm);
}

// Test 16: Set a key-value pair when memory allocation fails
void test_set_memory_allocation_fails() {
    SimpleMap* sm = create_simple_map();
    assert(sm != NULL);

    // Force memory allocation to fail for keys
    KeyNode* key = (KeyNode*)malloc(sizeof(KeyNode));
    ValueNodeString* value = (ValueNodeString*)malloc(sizeof(ValueNodeString));
    KeyValuePair kvp = create_key_value_pair(key, value);

    // Simulate memory allocation failure
    sm->keys = NULL;

    SimpleMap* result = set(sm, &kvp);

    assert(result == NULL); // Function should return NULL if memory allocation fails

    free(key);
    free(value);
    free_simple_map(sm);
}

// Test 17: Set a key-value pair in a NULL map
void test_set_null_map() {
    SimpleMap* sm = NULL;

    KeyNode* key = (KeyNode*)malloc(sizeof(KeyNode));
    ValueNodeString* value = (ValueNodeString*)malloc(sizeof(ValueNodeString));
    KeyValuePair kvp = create_key_value_pair(key, value);

    SimpleMap* result = set(sm, &kvp);

    assert(result == NULL); // Function should return NULL for a NULL map

    free(key);
    free(value);
}

// Test 18: Set a key-value pair with a NULL key
void test_set_null_key() {
    SimpleMap* sm = create_simple_map();
    assert(sm != NULL);

    // Test with NULL key
    ValueNodeString* value = (ValueNodeString*)malloc(sizeof(ValueNodeString));
    KeyValuePair kvp1 = create_key_value_pair(NULL, value);

    SimpleMap* result1 = set(sm, &kvp1);

    assert(result1 == NULL); // Function should return NULL if key is NULL

    free(value);
    free_simple_map(sm);
}
// Comparison function for KeyNode
void* compare_key_node(const void* a, const void* b) {
    const KeyNode* key_a = (const KeyNode*)a;
    const KeyNode* key_b = (const KeyNode*)b;
    if (!key_a || !key_b) {
        return NULL;
    }
    if (key_a->size != key_b->size) {
        return NULL;
    }
    if (memcmp(key_a->content, key_b->content, key_a->size) == 0) {
        return (void*)key_a;
    }
    return NULL;
}

// Test 19: Find a key in an empty map
void test_find_empty_map() {
    SimpleMap* sm = create_simple_map();
    assert(sm != NULL);

    KeyNode* key_to_find = create_key_node("key1", 0, 0);

    const void* result = find(sm, key_to_find, compare_key_node);

    assert(result == NULL); // Should return NULL since the map is empty

    free(key_to_find->content);
    free(key_to_find->input_time);
    free(key_to_find);
    free_simple_map(sm);
}

// Test 20: Find a key that exists in the map
void test_find_existing_key() {
    SimpleMap* sm = create_simple_map();
    assert(sm != NULL);

    KeyNode* key1 = create_key_node("key1", 0, 0);
    KeyNode* key2 = create_key_node("key2", 0, 0);

    // Add key1 to the map
    sm->keys[0] = (KeyWrapper*)malloc(sizeof(KeyWrapper));
    sm->keys[0]->key = key1;
    sm->top = 0;

    const void* result = find(sm, key1, compare_key_node);

    assert(result == key1); // Should return the key that was added

    free(key1->content);
    free(key1->input_time);
    free(key1);
    free(key2->content);
    free(key2->input_time);
    free(key2);
    free_simple_map(sm);
}

// Test 21: Find a key that does not exist in the map
void test_find_non_existing_key() {
    SimpleMap* sm = create_simple_map();
    assert(sm != NULL);

    KeyNode* key1 = create_key_node("key1", 0, 0);
    KeyNode* key2 = create_key_node("key2", 0, 0);

    // Add key1 to the map
    sm->keys[0] = (KeyWrapper*)malloc(sizeof(KeyWrapper));
    sm->keys[0]->key = key1;
    sm->top = 0;

    const void* result = find(sm, key2, compare_key_node);

    assert(result == NULL); // Should return NULL since key2 is not in the map

    free(key1->content);
    free(key1->input_time);
    free(key1);
    free(key2->content);
    free(key2->input_time);
    free(key2);
    free_simple_map(sm);
}

// Test 22: Find a key in a map with multiple keys
void test_find_multiple_keys() {
    SimpleMap* sm = create_simple_map();
    assert(sm != NULL);

    // Resize the map to accommodate multiple keys
    sm->keys = (KeyWrapper**)realloc(sm->keys, 2 * sizeof(KeyWrapper*));
    sm->capacity = 2;

    KeyNode* key1 = create_key_node("key1", 0, 0);
    KeyNode* key2 = create_key_node("key2", 0, 0);

    // Add keys to the map
    sm->keys[0] = (KeyWrapper*)malloc(sizeof(KeyWrapper));
    sm->keys[0]->key = key1;
    sm->keys[1] = (KeyWrapper*)malloc(sizeof(KeyWrapper));
    sm->keys[1]->key = key2;
    sm->top = 1;

    const void* result = find(sm, key2, compare_key_node);

    assert(result == key2); // Should return key2

    free(key1->content);
    free(key1->input_time);
    free(key1);
    free(key2->content);
    free(key2->input_time);
    free(key2);
    free_simple_map(sm);
}

// Test 23: Find a key with a NULL map
void test_find_null_map() {
    SimpleMap* sm = NULL;

    KeyNode* key_to_find = create_key_node("key1", 0, 0);

    const void* result = find(sm, key_to_find, compare_key_node);

    assert(result == NULL); // Should return NULL for a NULL map

    free(key_to_find->content);
    free(key_to_find->input_time);
    free(key_to_find);
}

// Test 24: Find a key with a NULL key
void test_find_null_key() {
    SimpleMap* sm = create_simple_map();
    assert(sm != NULL);

    KeyNode* key1 = create_key_node("key1", 0, 0);

    // Add key1 to the map
    sm->keys[0] = (KeyWrapper*)malloc(sizeof(KeyWrapper));
    sm->keys[0]->key = key1;
    sm->top = 0;

    const void* result = find(sm, NULL, compare_key_node);

    assert(result == NULL); // Should return NULL for a NULL key

    free(key1->content);
    free(key1->input_time);
    free(key1);
    free_simple_map(sm);
}

// Test 25: Find a key with a NULL comparison function
void test_find_null_comparison_function() {
    SimpleMap* sm = create_simple_map();
    assert(sm != NULL);

    KeyNode* key1 = create_key_node("key1", 0, 0);

    // Add key1 to the map
    sm->keys[0] = (KeyWrapper*)malloc(sizeof(KeyWrapper));
    sm->keys[0]->key = key1;
    sm->top = 0;

    const void* result = find(sm, key1, NULL);

    assert(result == NULL); // Should return NULL for a NULL comparison function

    free(key1->content);
    free(key1->input_time);
    free(key1);
    free_simple_map(sm);
}
// Clean-up function for keys and values
void* clean_up_key_value(void* key, void* value) {
    if(!key || !value){
        return NULL;
    }
    KeyNode* key_node = (KeyNode*)key;
    ValueNodeString* value_node = (ValueNodeString*)value;
    if(key_node->content){
        free(key_node->content);
    }
    if(key_node->input_time){
        free(key_node->input_time);
    }
    if(key_node){
        free(key_node);
    }
    if(value_node->content){
        free(value_node->content);
    }
    if(value_node){
        free(value_node);
    }

    return NULL;
}
// Test 26: Delete an empty map
void test_delete_map_empty_map() {
    SimpleMap* sm = create_simple_map();
    assert(sm != NULL);

    int result = delete_map(sm, clean_up_key_value);

    assert(result == 1); // Should return 1 for successful deletion
}

// Test 27: Delete a map with one key-value pair
void test_delete_map_one_element() {
    SimpleMap* sm = create_simple_map();
    assert(sm != NULL);

    KeyNode* key = create_key_node("key1", 0, 0);
    ValueNodeString* value = create_value_node_string("value1");

    sm->keys[0] = (KeyWrapper*)malloc(sizeof(KeyWrapper));
    sm->keys[0]->key = key;
    sm->values[0] = (ValueWrapper*)malloc(sizeof(ValueWrapper));
    sm->values[0]->value = value;
    sm->top = 0;

    int result = delete_map(sm, clean_up_key_value);

    assert(result == 1); // Should return 1 for successful deletion
}

// Test 28: Delete a map with multiple key-value pairs
void test_delete_map_multiple_elements() {
    SimpleMap* sm = create_simple_map();
    assert(sm != NULL);

    // Resize the map to accommodate multiple elements
    sm->keys = (KeyWrapper**)realloc(sm->keys, 2 * sizeof(KeyWrapper*));
    sm->values = (ValueWrapper**)realloc(sm->values, 2 * sizeof(ValueWrapper*));
    sm->capacity = 2;

    KeyNode* key1 = create_key_node("key1", 0, 0);
    ValueNodeString* value1 = create_value_node_string("value1");
    KeyNode* key2 = create_key_node("key2", 0, 0);
    ValueNodeString* value2 = create_value_node_string("value2");

    sm->keys[0] = (KeyWrapper*)malloc(sizeof(KeyWrapper));
    sm->keys[0]->key = key1;
    sm->values[0] = (ValueWrapper*)malloc(sizeof(ValueWrapper));
    sm->values[0]->value = value1;

    sm->keys[1] = (KeyWrapper*)malloc(sizeof(KeyWrapper));
    sm->keys[1]->key = key2;
    sm->values[1] = (ValueWrapper*)malloc(sizeof(ValueWrapper));
    sm->values[1]->value = value2;
    sm->top = 1;

    int result = delete_map(sm, clean_up_key_value);

    assert(result == 1); // Should return 1 for successful deletion
}

// Test 29: Delete a NULL map
void test_delete_map_null_map() {
    SimpleMap* sm = NULL;

    int result = delete_map(sm, clean_up_key_value);

    assert(result == 0); // Should return 0 for NULL map
}

// Test 30: Delete a map with a NULL clean-up function
void test_delete_map_null_clean_up() {
    SimpleMap* sm = create_simple_map();
    assert(sm != NULL);

    KeyNode* key = create_key_node("key1", 0, 0);
    ValueNodeString* value = create_value_node_string("value1");

    sm->keys[0] = (KeyWrapper*)malloc(sizeof(KeyWrapper));
    sm->keys[0]->key = key;
    sm->values[0] = (ValueWrapper*)malloc(sizeof(ValueWrapper));
    sm->values[0]->value = value;
    sm->top = 0;

    int result = delete_map(sm, NULL);

    assert(result == 1); // Should return 1 for successful deletion

    // Manually free the key and value since clean_up is NULL
    free(key->content);
    free(key->input_time);
    free(key);
    free(value->content);
    free(value);
}

// Test 31: Delete a map with partially allocated keys and values
void test_delete_map_partial_allocation() {
    SimpleMap* sm = create_simple_map();
    assert(sm != NULL);

    KeyNode* key = create_key_node("key1", 0, 0);
    ValueNodeString* value = create_value_node_string("value1");

    sm->keys[0] = (KeyWrapper*)malloc(sizeof(KeyWrapper));
    sm->keys[0]->key = key;
    sm->values[0] = NULL; // Simulate a partially allocated map
    sm->top = 0;

    int result = delete_map(sm, clean_up_key_value);

    assert(result == 1); // Should return 1 for successful deletion
}

// Test 32: Delete a map with NULL keys array
void test_delete_map_null_keys_array() {
    SimpleMap* sm = create_simple_map();
    assert(sm != NULL);

    free(sm->keys); // Simulate NULL keys array
    sm->keys = NULL;

    int result = delete_map(sm, clean_up_key_value);

    assert(result == 0); // Should return 0 for NULL keys array
}

// Test 33: Delete a map with NULL values array
void test_delete_map_null_values_array() {
    SimpleMap* sm = create_simple_map();
    assert(sm != NULL);

    free(sm->values); // Simulate NULL values array
    sm->values = NULL;

    int result = delete_map(sm, clean_up_key_value);

    assert(result == 0); // Should return 0 for NULL values array
}


int main() {
    test_remove_key_empty_map();
    test_remove_key_existing_key();
    test_remove_key_non_existing_key();
    test_remove_key_multiple_keys();
    test_remove_key_last_key();
    test_double_arrays_empty_map();
    test_double_arrays_one_element();
    test_double_arrays_multiple_elements();
    test_double_arrays_realloc_keys_fails();
    test_double_arrays_realloc_values_fails();
    test_double_arrays_null_map();
    test_double_arrays_zero_capacity();
    test_set_empty_map();
    test_set_one_element();
    test_set_full_map();
    test_set_memory_allocation_fails();
    test_set_null_map();
    test_set_null_key();
    test_find_empty_map();
    test_find_existing_key();
    test_find_non_existing_key();
    test_find_multiple_keys();
    test_find_null_map();
    test_find_null_key();
    test_find_null_comparison_function();
    test_delete_map_empty_map();
    test_delete_map_one_element();
    test_delete_map_multiple_elements();
    test_delete_map_null_map();
    test_delete_map_null_clean_up();
    test_delete_map_partial_allocation();
    test_delete_map_null_keys_array();
    test_delete_map_null_values_array();
    printf("All tests passed!\n");
    return 0;
}