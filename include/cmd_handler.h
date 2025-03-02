#ifndef CMD_HANDLER_H
#define CMD_HANDLER_H
#include "simple_map.h"
#include "protocol.h"

typedef struct {
    char*            content;
    struct timespec* input_time;
    unsigned int     ex;
    unsigned int     px;
    int              size;
}KeyNode;

typedef struct{
    void*       content;
    RedisDtype  dtype;
}ValueNode;

typedef struct {
    char*      content;
    RedisDtype dtype;
    int        size;
}ValueNodeString;

typedef struct{
    SimpleMap* sm;
    SimpleMap* config_dict;
}CmdParserArgs;

typedef enum {
    NULL_CMD = -1,
    ECHO     =  0,
    SET      =  1,
    GET      =  2,
    CONFIG   =  3
}RedisCommand;

typedef enum{
    CONF_GET,
    CONF_SET,
    INVALID_CONF_OPTION
}ConfigCmdOptions;


typedef struct {
    const char *name;
    RedisCommand cmd;
} CommandEntry;

typedef struct{
    char* conf_cmd_option_name;
    ConfigCmdOptions conf_cmd_option;
} ConfigCmdOptionsStruct;


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


static const CommandEntry command_table[] = {
    {"ECHO",   ECHO},
    {"SET",    SET},
    {"GET",    GET},
    {"CONFIG", CONFIG},
    {NULL,     NULL_CMD}  // End mark
};

static const ConfigCmdOptionsStruct config_cmd_options[] = {
    {"GET",CONF_GET},
    {"SET",CONF_SET},
    {NULL,INVALID_CONF_OPTION}
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
unsigned char is_cmd_valid(const void* gnode);
char*         execute_set_ex_px_exat_pxat(SimpleMap* sm, KeyValuePair* kvp, GenericNode** parsed_cmd);
unsigned char has_key_expired(KeyNode* new_key, KeyNode* prev_key);
RedisCommand  find_redis_cmd(char* cmd);
char*         handle_conf_cmd(GenericNode* gnode, SimpleMap* config_dict);
void
insert_key_value_str_to_str_array(const KeyValuePair* kvp, 
                                  char**        value_array,
                                  char**        key_array,
                                  unsigned int* total_bytes_val_arr,
                                  unsigned int* total_bytes_key_arr,
                                  unsigned int* next_pos);
char*
generate_conf_get_response(char** keys,
                            char** values,
                            const unsigned int total_key_bytes,
                            const unsigned int total_value_bytes,
                            size_t num_elements);
char* handle_conf_get(BulkStringNode* bnode, SimpleMap* config_dict);
#endif 