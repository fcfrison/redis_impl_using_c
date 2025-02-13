#ifndef CMD_HANDLER_H
#define CMD_HANDLER_H
#include "simple_map.h"
#include "protocol.h"

typedef struct KeyNode KeyNode;
typedef struct ValueNode ValueNode;
typedef struct ValueNodeString ValueNodeString;


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
    char*      content;
    RedisDtype dtype;
    int        size;
};
enum SET_STATES{
    SET_BASIC                = 0b01110000, // 0111 0000
    SET_NXXX                 = 0b01111000, // 0111 1000
    SET_GET                  = 0b01110100, // 0111 0100
    SET_EX_PX_EXAL_PXAT      = 0b01110011, // 0111 0011
    SET_NXXX_EXPX            = 0b01111011, // 0111 1011 
    SET_GET_EXPX             = 0b01110111, // 0111 0111
    SET_NXXX_GET             = 0b01111100  // 0111 1100
};

char*         handle_echo_cmd(void* fst_nod);
ValueNode*    create_value_node(GenericNode* gnode);
void*         compare(const void* a, const void* b);
char*         parse_command(void* node, SimpleMap* sm);
char*         __handle_echo_cmd(void* node, int* size);
char*         handle_set_cmd(void* node, SimpleMap* sm);
unsigned char is_set_option_valid(char* state, char bit_pos);
void          clean_up_execute_set_cmd(KeyNode* key, ValueNode* value);
ValueNode*    create_value_node_string(char* content, RedisDtype dtype, int size);
void*         validate_set_cmd(void* node, char* state, GenericNode*** parsed_cmd);
char*         execute_set_cmd(char state, GenericNode** parsed_cmd,  SimpleMap* sm);
GenericNode*  set_cmd_stage_a(GenericNode* gnode, char* state, GenericNode** parsed_cmd);
KeyNode*      create_key_node(char* content, unsigned int ex, unsigned int px, int size);
char*         execute_set_get(SimpleMap* sm, KeyValuePair* kvp);
char*         execute_set_basic(SimpleMap* sm, KeyValuePair* kvp, KeyNode* key, ValueNode* value);
void          handle_set_options(char* state, GenericNode** parsed_cmd, GenericNode** gnode, char bit_pos);
#endif 