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
enum FindKeyStates{
    FIND_KEY_ERROR = -1,
    KEY_ARR_ERROR  = -2,
    KEY_NOT_FOUND  = -3

};

KeyValuePair* create_key_val_pair(void* key, void* value);
KeyValuePair* remove_key(SimpleMap* sm, void* key, void*(cmp_fptr)(const void* a, const void* b), KeyValuePair* rmv_pair);

unsigned char __is_full(SimpleMap* sm);
SimpleMap*    __double_arrays(SimpleMap* sm);
KeyValuePair* __set(SimpleMap* sm, KeyValuePair* key_par);
KeyValuePair* __upgrade(SimpleMap* sm, int pos, KeyValuePair* key_par);
KeyValuePair* set(SimpleMap* sm, KeyValuePair* key_par, void*(cmp_fptr)(const void* a, const void* b));
KeyValuePair* get(SimpleMap* sm, void* key, void*(cmp_fptr)(const void* a, const void* b));
int           __find(const SimpleMap* sm, const void* key, void*(cmp_fptr)(const void* a, const void* b));
SimpleMap*    create_simple_map(void);
int           delete_map(SimpleMap* sm, void* (*clean_up)(void*, void*));
#endif