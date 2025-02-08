#ifndef SIMPLE_MAP_H
typedef struct KeyBaseStruct KeyBaseStruct;
typedef struct ValueBaseStruct ValueBaseStruct;
typedef struct SimpleMap SimpleMap; 

typedef struct KeyBaseStruct KeyBaseStruct;
typedef struct ValueBaseStruct ValueBaseStruct;
typedef struct SimpleMap SimpleMap;
typedef struct KeyNode KeyNode;
typedef struct ValueNode ValueNode;

struct SimpleMap{
    KeyBaseStruct**   keys;
    ValueBaseStruct** values;
    int               capacity;
    int               top;
};
struct KeyNode{
    char*            content;
    struct timespec* input_time;
    unsigned int     ex;
    unsigned int     px;
    int              size;
};
struct KeyBaseStruct{
    void* key;
};
struct ValueBaseStruct{
    void* value;
};
struct ValueNode{
    void*       content;
    void*       dtype;
};
struct ValueNodeString{
    void*      content;
    void*      dtype;
    int        size;
};

#endif