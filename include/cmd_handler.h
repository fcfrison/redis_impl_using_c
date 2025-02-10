#ifndef CMD_HANDLER_H
#define CMD_HANDLER_H
#include "simple_map.h"

typedef KeyNode KeyNode;
typedef ValueNode ValueNode;
typedef ValueNodeString ValueNodeString;

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
    RedisDtype  dtype;
};
struct ValueNodeString{
    void*      content;
    RedisDtype dtype;
    int        size;
};
enum SET_STATES{
    SET_BASIC                = 112, // 0111 0000
    SET_EX_PX_EXAL_PXAT      = 115, // 0111 0011
    SET_GET                  = 116, // 0111 0100
    SET_GET_EXPX             = 119, // 0111 0111
    SET_NXXX                 = 120, // 0111 1000
    SET_NXXX_EXPX            = 123, // 0111 1011 
    SET_NXXX_GET             = 124  // 0111 1100
};
#endif 