#ifndef CMD_HANDLER_H
#define CMD_HANDLER_H
#include "simple_map.h"
char* parse_command(void* node, SimpleMap* sm);
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
#endif 