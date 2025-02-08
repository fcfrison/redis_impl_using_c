#ifndef SIMPLE_MAP_H
typedef struct KeyWrapper KeyWrapper;
typedef struct ValueWrapper ValueWrapper;
typedef struct SimpleMap SimpleMap;
typedef struct KeyNode KeyNode;
typedef struct ValueNode ValueNode;
typedef struct ValueNodeString ValueNodeString;
typedef struct KeyValuePair KeyValuePair;
struct SimpleMap{
    KeyWrapper**   keys;
    ValueWrapper** values;
    int               capacity;
    int               top;
};
struct KeyWrapper{
    void* key;
};
struct ValueWrapper{
    void* value;
};
struct KeyValuePair{
    void* key;
    void* value;
};

/**
 * KeyNode are application code. For a while, they are going to remain here.
 */
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



unsigned char is_full(SimpleMap* sm);
SimpleMap* double_arrays(SimpleMap* sm);
void* set(SimpleMap* sm, KeyValuePair* key_par);
const void* find(const SimpleMap* sm, const void* key, void*(cmp_fptr)(const void* a, const void* b));
SimpleMap* create_simple_map(void);
void* remove_key(SimpleMap* sm,
                 void* key,
                 void*(cmp_fptr)(const void* a, const void* b),
                 KeyValuePair* rmv_pair);

#endif