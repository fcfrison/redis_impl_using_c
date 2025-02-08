#ifndef SIMPLE_MAP_H
typedef struct KeyBaseStruct KeyBaseStruct;
typedef struct ValueBaseStruct ValueBaseStruct;
typedef struct SimpleMap SimpleMap; 

struct SimpleMap{
    KeyBaseStruct**   keys;
    ValueBaseStruct** values;
    int               capacity;
    int               top;
};
struct KeyBaseStruct{
    char*            content;
    struct timespec* input_time;
    unsigned int     ex;
    unsigned int     px;
    int              size;
};

struct ValueBaseStruct{
    void*       content;
    void*       dtype;
};
struct ValueStringStruct{
    void*      content;
    void*      dtype;
    int        size;
};
#endif