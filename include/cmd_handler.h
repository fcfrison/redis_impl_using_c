#ifndef CMD_HANDLER_H
#define CMD_HANDLER_H
#include "simple_map.h"
#include "protocol.h"

typedef struct KeyNode KeyNode;
typedef struct ValueNode ValueNode;
typedef struct ValueNodeString ValueNodeString;
typedef struct CmdParserArgs CmdParserArgs; 

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
typedef enum {
    OPTION_NX,
    OPTION_XX,
    OPTION_GET,
    OPTION_EX,
    OPTION_PX,
    OPTION_EXAT,
    OPTION_PXAT,
    OPTION_INVALID
} OptionType;

struct CmdParserArgs{
    SimpleMap* sm;
    SimpleMap* config_dict;
};


char*         handle_echo_cmd(void* fst_nod);
ValueNode*    create_value_node(GenericNode* gnode);
void*         compare(const void* a, const void* b);
char*         parse_command(void* node, CmdParserArgs* args);
char*         __handle_echo_cmd(void* node, int* size);
char*         handle_set_cmd(void* node, SimpleMap* sm);
unsigned char is_set_option_valid(char* state, char bit_pos);
void          clean_up_kv(KeyNode* key, ValueNode* value);
ValueNode*    create_value_node_string(char* content, RedisDtype dtype, int size);
void*         validate_set_cmd(void* node, char* state, GenericNode*** parsed_cmd);
char*         execute_set_cmd(char state, GenericNode** parsed_cmd,  SimpleMap* sm);
GenericNode*  set_cmd_stage_a(GenericNode* gnode, char* state, GenericNode** parsed_cmd);
KeyNode*      create_key_node(char* content, unsigned int ex, unsigned int px, int size);
char*         execute_set_get(SimpleMap* sm, KeyValuePair* kvp);
char*         execute_set_basic(SimpleMap* sm, KeyValuePair* kvp);
void          handle_set_options(char* state, GenericNode** parsed_cmd, GenericNode** gnode, char bit_pos);
char*         execute_set_nx_xx(SimpleMap* sm, KeyValuePair* kvp, GenericNode** parsed_cmd);
char*         execute_set_nxxx_get(SimpleMap* sm, KeyValuePair* kvp, GenericNode** parsed_cmd);
char*         handle_get_cmd(const void* gnode, SimpleMap* sm);
unsigned char is_get_cmd_valid(const void* gnode);
char* execute_set_ex_px_exat_pxat(SimpleMap* sm, KeyValuePair* kvp, GenericNode** parsed_cmd);
unsigned char has_key_expired(KeyNode* new_key, KeyNode* prev_key);
#endif 