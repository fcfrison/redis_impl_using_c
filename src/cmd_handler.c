#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include "../include/protocol.h"
#include "../include/simple_map.h"
#include "../include/cmd_handler.h"
#include "../include/util.h"
#define MAX_BYTES_ARR_SIZE 20 
#define MAX_BYTES_BULK_STR 9

typedef enum {
    TYPE_VALUE_NODE,
    TYPE_KEY_NODE
} NodeType;
// Prototypes
void  cpy_str(const char* src, const size_t str_len, char** dest);
int   key_or_value_node_to_string(const void* node, char** rtn_val, NodeType nt);
void* __execute_set_check(SimpleMap* sm, KeyValuePair* kvp);
void  __validate_px_ex_content(BulkStringNode* node, char* state);
OptionType get_option_type(const char* option);
unsigned int is_it_possible_to_insert_the_str_in_array(char**arr, size_t size, char* str);


char*
parse_command(void* node, CmdParserArgs* args){
    if(!node || !args->config_dict || !args->sm){
        errno = EINVAL;
        return NULL;
    }
    SimpleMap* sm = args->sm;
    SimpleMap* cfg_dict = args->config_dict;
    GenericNode* gnode = (GenericNode*) node;
    unsigned char first_array = 0;
    while(gnode){
        switch (gnode->node->type){
            case ARRAY:
                gnode = ((ArrayNode*) gnode)->content;
                first_array++;
                break;
            case BULK_STR:
                BulkStringNode* temp = (BulkStringNode*) gnode;
                if(first_array==1){
                    switch (find_redis_cmd(temp->content)){
                        case ECHO:
                            return handle_echo_cmd(temp->node->next);
                        case SET:
                            return handle_set_cmd(temp->node->next,sm);
                        case GET:
                            return handle_get_cmd(temp->node->next,sm);
                        case CONFIG:
                            return handle_conf_cmd(temp->node->next,cfg_dict);
                        default:
                            return NULL;
                        }
                }
                return NULL;
            default:
                return NULL;
        }
    }
    return NULL;
}



char*
handle_conf_cmd(GenericNode* gnode, SimpleMap* config_dict){
    if(!gnode               ||  gnode->node->type!=BULK_STR ||
       !config_dict         || !config_dict->keys           ||
       !config_dict->values){
        return NULL;
    };
    BulkStringNode* str_node = (BulkStringNode*) gnode;
    char* cmd = str_node->content;
    if(!cmd){
        return NULL;
    };
    unsigned char i = 0;
    ConfigCmdOptionsStruct item;
    for (item = config_cmd_options[i]; item.conf_cmd_option_name; item = config_cmd_options[i++]){
        switch(does_the_strings_match(item.conf_cmd_option_name, cmd)){
            case MATCH:
                if(item.conf_cmd_option==CONF_GET){
                    return handle_conf_get(gnode->node->next, config_dict);
                }else{
                    return NULL;
                }
                break;
            case MATCH_ERROR:
                return NULL;            
            default:
                continue;
        }
        
    }
    return NULL;
};
/**
 * Retrieves configuration values from a config dictionary based on provided key(s)
 *
 * This function processes a CONF GET command in a REDIS server implementation. It searches
 * the configuration dictionary for keys that match the provided bulk string nodes and constructs
 * a response containing the matched key-value pairs.
 *
 * @param bnode A pointer to a BulkStringNode containing the configuration key(s) to retrieve.
 *              Multiple keys can be provided as a linked list through the node's next pointers.
 * @param config_dict A pointer to a SimpleMap containing the server's configuration key-value pairs.
 *
 * @return char* A pointer to a generated response string containing the matched configuration
 *               key-value pairs. Returns NULL if no configuration keys were matched or if any of
 *               the input parameters are invalid.
 *
 * The function will validate inputs, then iterate through each bulk string node, looking for
 * matching keys in the configuration dictionary. For each match found, it stores the key-value
 * pair in temporary arrays. Finally, it generates a formatted response containing all matched
 * configuration entries.
 */
char*
handle_conf_get(BulkStringNode* bnode, SimpleMap* config_dict){
    if(!bnode                       ||
       !bnode->node                 ||
       !config_dict                 ||
       !config_dict->keys           ||
       !config_dict->values         ||
        config_dict->top<0){
        return NULL;
    }
    unsigned int    possible_max_size = config_dict->top+1;
    char*           key_array[possible_max_size];
    char*           value_array[possible_max_size];
    memset(key_array, '\0', possible_max_size);
    memset(value_array, '\0', possible_max_size);
    KeyValuePair*   kvp;
    KeyNode*        key;
    unsigned int nr_inserted_el      = 0;
    unsigned int total_bytes_key_arr = 0;
    unsigned int total_bytes_val_arr = 0;
    SimpleMapWrapper smw;
    do{
        if(bnode->node->type!=BULK_STR){
            bnode = bnode->node->next;
            continue;    
        };
        init_simple_map_wrapper(config_dict,&smw);
        while((kvp = smw.next(&smw))){
            key = kvp->key;
            if(!key){
                free(kvp);
                continue;
            };
            switch (does_the_strings_match(bnode->content, key->content)){
                case MATCH:
                    insert_key_value_str_to_str_array(kvp,
                                                    value_array,
                                                    key_array,
                                                    &total_bytes_val_arr,
                                                    &total_bytes_key_arr,
                                                    &nr_inserted_el);
                    free(kvp);
                    break;
                default:
                    continue;
                }
        };
        bnode = bnode->node->next;
    }while(bnode && bnode->node);
    if(nr_inserted_el==0){
        char* empty_arr = "*0\r\n";
        char* response = (char*)calloc(strlen(empty_arr)+1,sizeof(char));
        strcpy(response,empty_arr);
        return response;
    }
    return generate_conf_get_response(key_array,
                                      value_array,
                                      total_bytes_key_arr,
                                      total_bytes_val_arr,
                                      nr_inserted_el);
};

//AI generated code
char*
generate_conf_get_response(char** keys,
                           char** values,
                           const unsigned int total_key_bytes,
                           const unsigned int total_value_bytes,
                           size_t num_elements) {
    size_t final_num_el = 2*num_elements;
    char num_elements_buf[8] = {0};
    snprintf(num_elements_buf, sizeof(num_elements_buf), "%zu", final_num_el);
    size_t num_elements_len = strlen(num_elements_buf);
    size_t total_bytes = 1 + num_elements_len + 2 + total_key_bytes + total_value_bytes + 1;
    char* response = (char*)calloc(total_bytes, sizeof(char));
    if (!response) {
        return NULL;
    }
    int pos = 0;
    pos += snprintf(response + pos, total_bytes - pos, "*%s\r\n", num_elements_buf);
    for (size_t i = 0; i < num_elements; i++) {
        if(!keys[i]){
            free(response);
            return NULL;
        }
        pos += snprintf(response + pos, total_bytes - pos, "%s%s", keys[i], values[i]);
    }
    return response;
};
void
insert_key_value_str_to_str_array(const KeyValuePair* kvp, 
                                  char**        value_array,
                                  char**        key_array,
                                  unsigned int* total_bytes_val_arr,
                                  unsigned int* total_bytes_key_arr,
                                  unsigned int* nr_inserted_el){
    ValueNode* value;
    KeyNode*   key;
    if(!kvp||!(key=kvp->key) || !(value=kvp->value)){
        return;
    };
    char* key_str = NULL, *value_str = NULL;
    if(!key->content || !value->content ||
        value->dtype != BULK_STR){
        return;
    };
    int key_arr_str_size      = key_or_value_node_to_string(key, &key_str, TYPE_KEY_NODE)-1;
    int val_arr_str_size      = key_or_value_node_to_string(value, &value_str, TYPE_VALUE_NODE)-1;
    unsigned int is_valid_key = is_it_possible_to_insert_the_str_in_array(key_array, (size_t)*nr_inserted_el, key_str);
    if(key_str && value_str && is_valid_key){
        *total_bytes_key_arr += key_arr_str_size;
        *total_bytes_val_arr += val_arr_str_size;
        key_array[*nr_inserted_el] = key_str;
        value_array[*nr_inserted_el] = value_str;
        (*nr_inserted_el)++;
        return;
    }
    free(key_str);
    free(value_str);
    return;
};
unsigned int
is_it_possible_to_insert_the_str_in_array(char**arr, size_t size, char* str){
    for (size_t i = 0; i < size; i++){
        char* value = arr[i];
        if(strcmp(str, value)==0){
            return 0;
        };
    }
    return 1;    
}
RedisCommand
find_redis_cmd(char* cmd){
    unsigned char i = 0;
    for (CommandEntry item = command_table[i]; item.name; item = command_table[i++]){
        MatchErrorState state = does_the_strings_match(item.name,cmd);
        switch(state){
            case MATCH:
                return item.cmd;
            case MATCH_ERROR:
                return NULL_CMD;
            default:
                continue;
        }
    }
    return NULL_CMD;
}



char*
handle_get_cmd(const void* gnode, SimpleMap* sm){
    if(!is_cmd_valid(gnode)){
        return NULL;
    }
    char* rtn = NULL, *nil="$-1\r\n";
    BulkStringNode* blk_s_nd = (BulkStringNode*)gnode;
    KeyNode* key = create_key_node(blk_s_nd->content, 0, 0, blk_s_nd->size);
    if(!key){
        return NULL;
    }
    KeyValuePair* kvp = get(sm, key, &compare);
    if(!kvp){
        rtn = (char*)calloc(strlen(nil)+1,sizeof(char));
        strcpy(rtn,nil);
        clean_up_kv(key,NULL);
        return rtn;
    }
    if(!kvp->value || !kvp->key){
        clean_up_kv(key,NULL);
        free(kvp);
        return NULL;
    }
    if(has_key_expired(key,kvp->key)){
        KeyValuePair removed_kvp;
        switch(remove_key(sm,key,&compare,&removed_kvp)){
            case REMOVE_ERROR:
                clean_up_kv(key,NULL);
                free(kvp);
                return NULL;
            case REMOVE_KEY_NOT_FOUND:
                clean_up_kv(key,NULL);
                free(kvp);
                rtn = (char*)calloc(strlen(nil)+1,sizeof(char));
                strcpy(rtn,nil);
                return rtn;
            case REMOVE_SUCCESS:
                clean_up_kv(key,NULL);
                clean_up_kv(removed_kvp.key,removed_kvp.value);
                free(kvp);
                rtn = (char*)calloc(strlen(nil)+1,sizeof(char));
                strcpy(rtn,nil);
                return rtn;
        }
    };

    switch (((ValueNode*)kvp->value)->dtype){
        case BULK_STR:
            key_or_value_node_to_string(kvp->value, &rtn, TYPE_VALUE_NODE);
            clean_up_kv(key,NULL);
            free(kvp);
            return rtn;
        default:
            clean_up_kv(key,NULL);
            free(kvp);
            return NULL;
    }

};
unsigned char
has_key_expired(KeyNode* new_key, KeyNode* prev_key){
    unsigned int ex, px;
    ex = prev_key->ex;
    px = prev_key->px;
    if(!ex && !px){
        return 0;
    }
    struct timespec* curr_time, *prev_time;
    float curr_t_sec, prev_t_sec, delta_sec;
    curr_time  = new_key->input_time;
    prev_time  = prev_key->input_time;
    curr_t_sec = curr_time->tv_sec + curr_time->tv_nsec/1e9;
    prev_t_sec = prev_time->tv_sec + prev_time->tv_nsec/1e9;
    delta_sec  = curr_t_sec - prev_t_sec;
    //time in seconds
    if(ex){
        return delta_sec>ex?1:0;
    }else{
        //time in miliseconds
        return delta_sec*1000>px?1:0;
    }
};

unsigned char
is_cmd_valid(const void* gnode){
    BulkStringNode* blk_s_nd = (BulkStringNode*)gnode;
    if(!blk_s_nd || !blk_s_nd->content || blk_s_nd->size<0){
        return 0;
    }
    if(!blk_s_nd->node || blk_s_nd->node->next || blk_s_nd->node->type!=BULK_STR){
        return 0;
    }
    return 1;

};

char* 
handle_echo_cmd(void* fst_nod){
    int* size = calloc(1,sizeof(int));
    char* ret_val = __handle_echo_cmd(fst_nod,size);
    free(size);
    return ret_val;
}

//TODO: it's necessary to echo back other Redis types: SIMPLE_STR, etc.
char*
__handle_echo_cmd(void* node, int* size){
    if(!node){
        return NULL;
    }
    GenericNode* gnode = (GenericNode*) node;
    char *next, *format, *rtn;
    switch (gnode->node->type){
        case ARRAY:
            ArrayNode* array = ((ArrayNode*) node);
            char* elements = __handle_echo_cmd(array->content, size);
            next     = __handle_echo_cmd(gnode->node->next, size);
            format   = "*%d\r\n%s%s";
            char* arr_size_s = calloc(MAX_BYTES_ARR_SIZE+1, sizeof(char));
            int   arr_size = ((ArrayNode*)gnode)->size;
            snprintf(arr_size_s,MAX_BYTES_ARR_SIZE+1,"%d",arr_size);
            *size += strlen(arr_size_s) + 3;
            rtn = calloc(*size+1,sizeof(char));
            snprintf(rtn,*size+1,format,arr_size,elements,next);
            free(elements);
            free(next);
            free(arr_size_s);
            return rtn;
        case BULK_STR:
            BulkStringNode* bulks_node = ((BulkStringNode*)node);
            next = __handle_echo_cmd(gnode->node->next, size);
            char* content = bulks_node->content;
            format  = "$%d\r\n%s\r\n%s";
            char* bs_size = calloc(MAX_BYTES_BULK_STR+1, sizeof(char));
            snprintf(bs_size,(MAX_BYTES_BULK_STR+1),"%d",bulks_node->size);
            *size+= strlen(bs_size) + 5 + strlen(content);
            rtn = calloc(*size+1,sizeof(char));
            snprintf(rtn,*size+1,format,bulks_node->size,content,next);
            free(next);
            free(bs_size);
            return rtn;
        default:
            break;
    }
    return NULL;
}

char*
handle_set_cmd(void* node, SimpleMap* sm){
    // The void* node is already the arguments of the set cmd
    char state;
    GenericNode** parsed_cmd = NULL;
    void* is_valid = validate_set_cmd(node, &state, &parsed_cmd);
    if(!is_valid){
        if(parsed_cmd){
            free(parsed_cmd);
        }
        return NULL;
    }
    char* rtn = execute_set_cmd(state, parsed_cmd,  sm);
    if(parsed_cmd){
        free(parsed_cmd);
    }
    return rtn;
}



KeyNode*
create_key_node(char* content, unsigned int ex, unsigned int px, int size){
    if(!content || size<1){
        return NULL;
    }
    KeyNode* kn = (KeyNode*)calloc(1,sizeof(KeyNode));
    if(!kn){
        return NULL;
    }
    struct timespec* ts = (struct timespec*)calloc(1,sizeof(struct timespec));
    clock_gettime(CLOCK_MONOTONIC, ts);
    // Not sure what the size must be: size or "size+1"
    kn->content = (char*)calloc(size+1,sizeof(char));
    memcpy(kn->content,content,size);
    kn->content[size] = '\0';
    kn->ex   = ex;
    kn->px   = px;
    kn->size = size;
    kn->input_time = ts;
    return kn;
}
ValueNode*
create_value_node_string(char* content, RedisDtype dtype, int size){
    if(!content || size<1){
        return NULL;
    }
    ValueNodeString* vns = (ValueNodeString*)calloc(1,sizeof(ValueNodeString));
    if(!vns){
        return NULL;
    }
    // Not sure what the size must be: size or "size+1"
    vns->content = (char*)calloc(size+1,sizeof(char));
    memcpy(vns->content,content,size);
    ((char*)vns->content)[size] = '\0';
    vns->dtype = dtype;
    vns->size  = size;
    return (ValueNode*)vns;
};
ValueNode*
create_value_node(GenericNode* gnode){
    if(!gnode || !gnode->node || !gnode->node->type){
        return NULL;
    }
    switch (gnode->node->type){
        case BULK_STR:
            BulkStringNode* node = (BulkStringNode*)gnode;
            return create_value_node_string(node->content, node->node->type, node->size);
        default:
            return NULL;
    }
}

void* 
compare(const void* a, const void* b){
    if(!a || !b){
        return NULL;
    }
    KeyNode* ka = (KeyNode*) a;
    KeyNode* kb = (KeyNode*) b;
    if(ka->size!=kb->size){
        return NULL;
    }
    for(int i = 0; i < ka->size; i++){
        if(ka->content[i]!=kb->content[i]){
            return NULL;
        }
    }
    return ka;
};

char*
execute_set_cmd(char state, GenericNode** parsed_cmd,  SimpleMap* sm){
    //char* rtn_val;
    if(!parsed_cmd || !sm){
        return NULL;
    }
    KeyNode*      key;
    ValueNode*    value;
    KeyValuePair* kvp;
    //int set_rtn;
    BulkStringNode* k = (BulkStringNode*)parsed_cmd[5];
    GenericNode*    v = (GenericNode*)parsed_cmd[4];
    if(!k || !v){
        return NULL;
    }
    key = create_key_node(k->content,0,0,k->size);
    if(!key){
        return NULL;
    }
    value = create_value_node(v);
    if(!value){
        clean_up_kv(key, NULL);
        return NULL;
    }
    kvp = create_key_val_pair(key,value);
    switch (state){
        case SET_BASIC:
            return execute_set_basic(sm, kvp);
        case SET_GET:
            return execute_set_get(sm, kvp);
        case SET_NXXX:
            return execute_set_nx_xx(sm, kvp, parsed_cmd);
        case SET_NXXX_GET:
            return execute_set_nxxx_get(sm, kvp, parsed_cmd);
        case SET_EX_PX_EXAL_PXAT:
            return execute_set_ex_px_exat_pxat(sm, kvp, parsed_cmd);
        case SET_GET_EXPX:
        case SET_NXXX_EXPX:
        default:
            break;
    }
    return NULL;
}
int
key_or_value_node_to_string(const void* node, char** rtn_val, NodeType nt){
    int content_size;
    char* content;
    if(!node){
        return -1;
    }
    if(nt==TYPE_KEY_NODE){
        content_size = ((KeyNode*)node)->size;
        content      = ((KeyNode*)node)->content; 
    }else{
        content_size = ((ValueNodeString*)node)->size;
        content      = ((ValueNodeString*)node)->content; 
    };
    char content_size_buf[16] = {0};
    sprintf(content_size_buf, "%d", content_size);
    int rtn_val_size = 1 + strlen(content_size_buf) + 2 + content_size + 3;
    *rtn_val = (char*)calloc(rtn_val_size,sizeof(char));
    if(!*rtn_val){
        return -1;
    }
    int pos = 0;
    memcpy(*rtn_val,"$",1);
    pos++;
    memcpy(*rtn_val+pos,content_size_buf,strlen(content_size_buf));
    pos+=strlen(content_size_buf);
    memcpy(*rtn_val+pos,"\r\n",2);
    pos+=2;
    memcpy(*rtn_val+pos,content,content_size);
    pos+=content_size;
    memcpy(*rtn_val+pos,"\r\n",2);
    pos+=2;
    *(*rtn_val+pos) = '\0';
    return rtn_val_size;
}

/**
 * Executes a Redis SET command with both NX|XX and GET options.
 *
 * This function implements the behavior of SET key value [NX|XX] GET in Redis:
 * - With NX GET: If key doesn't exist, sets the value and returns nil.
 *               If key exists, doesn't set the value and returns the current value.
 * - With XX GET: If key exists, sets the new value and returns the old value.
 *               If key doesn't exist, doesn't set anything and returns nil.
 *
 * The function returns a dynamically allocated string in RESP protocol format:
 * - If there was an old value, returns it as a bulk string: "$<length>\r\n<value>\r\n"
 * - If there was no old value or operation failed, returns nil as: "$-1\r\n"
 *
 * @param sm           Pointer to the SimpleMap data structure storing key-value pairs
 * @param kvp          Pointer to the KeyValuePair to be set
 * @param parsed_cmd   Array of GenericNode pointers representing the parsed command
 *                     parsed_cmd[2] and parsed_cmd[3] must contain the NX|XX and GET options
 *
 * @return A dynamically allocated string containing the RESP-formatted old value
 *         or nil, or NULL in the following error cases:
 *         - If __execute_set_check() fails
 *         - If parsed_cmd, parsed_cmd[2] or parsed_cmd[3] are NULL
 *         - If kvp_old exists but has invalid structure (NULL key/value/content)
 *         The caller is responsible for freeing the returned memory when not NULL.
 */
char*
execute_set_nxxx_get(SimpleMap* sm, KeyValuePair* kvp, GenericNode** parsed_cmd){
    if(!__execute_set_check(sm, kvp)){
        return NULL;
    }
    if(!parsed_cmd || !parsed_cmd[3]){
        clean_up_kv(kvp->key,kvp->value);
        free(kvp);
        return NULL;
    }
    KeyValuePair* kvp_old = get(sm,kvp->key,&compare);
    char* nil = "$-1\r\n", *rtn_val = NULL;
    if(kvp_old){
        if(!kvp_old->key || !kvp_old->value ||
           !((KeyNode*)kvp_old->key)->content){
            clean_up_kv(kvp->key,kvp->value);
            free(kvp);
            return NULL;
        }
        ValueNode* old_value = (ValueNode*)kvp_old->value;
        switch (old_value->dtype){
            case BULK_STR:
                ValueNodeString* old_value_s = (ValueNodeString*)old_value;
                if(old_value_s->content){
                    key_or_value_node_to_string(old_value_s, &rtn_val, TYPE_VALUE_NODE);
                    break;                   
                }else{
                    cpy_str(nil,strlen(nil), &rtn_val);
                    break;
                }
            default:
                cpy_str(nil,strlen(nil), &rtn_val);
                break;
        }
    }else{
        cpy_str(nil,strlen(nil), &rtn_val);
    }
    char* rtn = execute_set_nx_xx(sm, kvp, parsed_cmd);
    if(rtn){
        free(rtn);
        return rtn_val;
    }
    free(rtn_val);
    return NULL;
};

void
cpy_str(const char* src, const size_t str_len, char** dest){
    *dest = (char*)calloc(str_len+1,sizeof(char));
    strcpy(*dest,src);
    return;
};

char*
execute_set_ex_px_exat_pxat(SimpleMap* sm, KeyValuePair* kvp, GenericNode** parsed_cmd){
    if(!__execute_set_check(sm,kvp) || !parsed_cmd    || 
       !parsed_cmd[1]               || !parsed_cmd[0]){
        return NULL;
    }
    BulkStringNode* option_nd = (BulkStringNode*)parsed_cmd[1];
    BulkStringNode* time_nd   = (BulkStringNode*)parsed_cmd[0];
    KeyNode* key              = (KeyNode*)kvp->key;
    char* option = option_nd->content;
    unsigned int time = string_to_uint(time_nd->content);
    switch (get_option_type(option) ){
        case OPTION_EX:
        case OPTION_EXAT:
            key->ex = time;
            break;
        case OPTION_PX:
        case OPTION_PXAT:
            key->px = time;
            break;
        default:
            clean_up_kv(kvp->key, kvp->value);
            return NULL;
    }
    return execute_set_basic(sm, kvp);
};


/**
* Executes a Redis SET command with NX or XX option.
*
* This function implements the following Redis SET behaviors:
* - NX: Only set the key if it does not already exist.
*       Returns "+OK\r\n" on successful set, "$-1\r\n" if key exists
* - XX: Only set the key if it already exists.
*       Returns "+OK\r\n" on successful update, "$-1\r\n" if key doesn't exist
*
* @param sm           Pointer to the SimpleMap data structure storing key-value pairs
* @param kvp          Pointer to the KeyValuePair to be set (will be freed by this function)
* @param parsed_cmd   Array of GenericNode pointers representing the parsed command
*                     parsed_cmd[3] must contain a BulkStringNode with "NX" or "XX" content
*
* @return A dynamically allocated string containing the RESP-formatted response,
*         or NULL in the following error cases:
*         - If __execute_set_check() fails
*         - If parsed_cmd or parsed_cmd[3] are NULL
*         - If parsed_cmd[3]'s content is NULL
*         - If option is neither "NX" nor "XX"
*         The caller is responsible for freeing the returned memory when not NULL.
*
* @note This function takes ownership of kvp and will free it appropriately.
*       Any stored key-value pair will be cleaned up using clean_up_kv() if needed.
*/
char*
execute_set_nx_xx(SimpleMap* sm, KeyValuePair* kvp, GenericNode** parsed_cmd){
    
    if(!__execute_set_check(sm,kvp) || !parsed_cmd || !parsed_cmd[3]){
        return NULL;
    }
    //NX -- Only set the key if it does not already exist.
    //XX -- Only set the key if it already exists.
    BulkStringNode* nx_xx = (BulkStringNode*)parsed_cmd[3];
    char* rtn_val = NULL, *option = nx_xx->content;
    char* nil = "$-1\r\n", *sucess="+OK\r\n";
    if(!option){
        clean_up_kv(kvp->key,kvp->value);
        free(kvp);
        return NULL;
    }
    unsigned char option_val = 0;
    if(strcmp(option,"NX")==0){
        option_val = 1;
    }else if(strcmp(option,"XX")==0){
        option_val = 2;
    }else{
        clean_up_kv(kvp->key,kvp->value);
        free(kvp);
        return NULL;
    }
    KeyValuePair* old_kvp = get(sm,kvp->key,&compare);
    // if not old_kvp --> new_key
    // if old_kvp --> already exists
    switch (option_val){
        case 1://NX
            if(!old_kvp){//chave nao existe e NX
                switch (set(sm,kvp,&compare)){
                    case SUCESS_SET:
                        rtn_val = (char*)calloc(strlen(sucess)+1,sizeof(char));
                        strcpy(rtn_val,sucess);
                        break;
                    default:
                        rtn_val = (char*)calloc(strlen(nil)+1,sizeof(char));
                        strcpy(rtn_val,nil);
                        break;
                    }
                
            }else{
                rtn_val = (char*)calloc(strlen(nil)+1,sizeof(char));
                strcpy(rtn_val,nil);
                clean_up_kv(kvp->key,kvp->value);

            }
            break;
        case 2: //XX -- Only set the key if it already exists.
            if(old_kvp){
                switch (set(sm,kvp,&compare)){
                    case SUCESS_UPGRADE:
                        rtn_val = (char*)calloc(strlen(sucess)+1,sizeof(char));
                        strcpy(rtn_val,sucess);
                        clean_up_kv(kvp->key,kvp->value);
                        break;
                    default:
                        rtn_val = (char*)calloc(strlen(nil)+1,sizeof(char));
                        strcpy(rtn_val,nil);
                        break;
                    }
            }else{
                rtn_val = (char*)calloc(strlen(nil)+1,sizeof(char));
                strcpy(rtn_val,nil);
                clean_up_kv(kvp->key,kvp->value);
            }
            break;
    }
    free(kvp);
    return rtn_val;
}

void*
__execute_set_check(SimpleMap* sm, KeyValuePair* kvp){
    if(!kvp || !kvp->key || !kvp->value
        || !sm->values || !sm->keys){
        if(kvp){
            clean_up_kv(kvp->key,kvp->value);
            free(kvp);
        }
    return NULL;
    }
    if(!((KeyNode*)kvp->key)->content){
        clean_up_kv(kvp->key,kvp->value);
        free(kvp);
        return NULL;
    }
    return sm;
}

/**
 * @brief Executes the Redis `SET <key> <value> GET` operation on a SimpleMap.
 *
 * @details This function performs the Redis-like `SET <key> <value> GET` command.  
 *          It inserts or updates a key-value pair in the given `SimpleMap` and  
 *          returns the previous value if the key already existed.
 *
 * @param sm  A pointer to the `SimpleMap` structure where the key-value pair  
 *            will be stored.
 * @param kvp A pointer to the `KeyValuePair` containing the key and value  
 *            to be set in the map.
 *
 * @return A dynamically allocated string representing the previous value  
 *         associated with the key in Redis RESP format, or `NULL` if the key  
 *         was not present before. The caller is responsible for freeing  
 *         this string.
 *
 * @note Memory Management:
 *       - The `kvp` structure is always freed before the function returns.
 *       - The `sm` structure remains unchanged.
 *       - `kvp->key` and `kvp->value` are freed if:
 *         - An error occurs during processing.
 *         - The key already exists, and an update is performed.
 *       - If a new key-value pair is inserted, ownership of `kvp->key` and  
 *         `kvp->value` is transferred to the map, and they are not freed.
 * @note Value content:
 *       Its possible to pass a string value node with its content as a NULL string. 
 *       In this case, the returned value will always be "$-1\r\n", no matter if 
 *       it's the first time the key is inserted or if it was already present.
 */
char*
execute_set_get(SimpleMap* sm, KeyValuePair* kvp){
    if(!__execute_set_check(sm, kvp)){
        return NULL;
    }
    int rtn = set(sm,kvp,&compare);
    char *rtn_val = NULL, *rtn_msg = "$-1\r\n";
    if(rtn==ERROR_SET_SM_RTN){
        clean_up_kv(kvp->key,kvp->value);
        free(kvp);
        return rtn_val;
    }
    if(rtn==SUCESS_SET){
        rtn_val = (char*)calloc(strlen(rtn_msg)+1,sizeof(char));
        strcpy(rtn_val,rtn_msg);
        free(kvp);
        return rtn_val;
    }
    // In this case, the key was already in the map
    if(!kvp->value){
        clean_up_kv(kvp->key, kvp->value);
        free(kvp);
        rtn_val = (char*)calloc(strlen(rtn_msg)+1,sizeof(char));
        strcpy(rtn_val,rtn_msg);
        return rtn_val;
    }
    ValueNode* old_value = (ValueNode*)kvp->value;
    switch (old_value->dtype){
        case BULK_STR:
            ValueNodeString* old_value_s = (ValueNodeString*)old_value;
            if(old_value_s->content){
                key_or_value_node_to_string(old_value_s, &rtn_val, TYPE_VALUE_NODE);
                clean_up_kv(kvp->key, kvp->value);
                free(kvp);
                return rtn_val;
            }
        default:
            clean_up_kv(kvp->key, kvp->value);
            free(kvp);
            rtn_val = (char*)calloc(strlen(rtn_msg)+1,sizeof(char));
            strcpy(rtn_val,rtn_msg);
            return rtn_val;
    }
}
char*
execute_set_basic(SimpleMap* sm, KeyValuePair* kvp){
    if(!kvp || !kvp->key || !kvp->value
            || !sm->values || !sm->keys){
        if(kvp){
            clean_up_kv(kvp->key,kvp->value);
            free(kvp);
        }
        return NULL;
    }
    if(!((KeyNode*)kvp->key)->content){
        clean_up_kv(kvp->key,kvp->value);
        free(kvp);
        return NULL;
    }
    int rtn = set(sm,kvp,&compare);
    char *rtn_val = NULL, *rtn_msg = "+OK\r\n";
    switch (rtn){
        case ERROR_SET_SM_RTN:
            rtn_val = (char*)calloc(strlen("$-1\r\n")+1,sizeof(char));
            clean_up_kv(kvp->key,kvp->value);
            break;
        case SUCESS_SET:
            rtn_val = (char*)calloc(strlen(rtn_msg)+1,sizeof(char));
            strcpy(rtn_val,rtn_msg);
            break;
        default:
            clean_up_kv(kvp->key, kvp->value);
            rtn_val = (char*)calloc(strlen(rtn_msg)+1,sizeof(char));
            strcpy(rtn_val,rtn_msg);
            break;
    }
    free(kvp);
    return rtn_val;

}

void
clean_up_kv(KeyNode* key, ValueNode* value){
    if(key){
        if(key->content){
            free(key->content);
        }
        if(key->input_time){
            free(key->input_time);
        }
        free(key);
    }
    if(value){
        switch (value->dtype){
            case BULK_STR:
                ValueNodeString* value_ns = (ValueNodeString*)value;
                if(value_ns->content){
                    free(value_ns->content);
                }
                break;
            default:
                break;
            }
        if(value){
            free(value);
        }
    }
}
/**
 * Main entry point for SET command validation and parsing.
 * Orchestrates the entire parsing process in three stages:
 * 1. Validates and parses mandatory key-value pair (via set_cmd_stage_a)
 * 2. Processes optional arguments sequentially
 * 3. Handles option-specific validations (e.g., expiry time for EX/PX)
 * 
 * @param node Pointer to the command node to be parsed
 * @param state Pointer to parsing state (tracks progress and errors)
 * @param parsed_cmd Triple pointer to store parsed command components (array of 6 elements)
 */
void*
validate_set_cmd(void* node, char* state, GenericNode*** parsed_cmd){
    if(!state || !node){
        return NULL;
    }
    GenericNode* gnode = (GenericNode*)node;
    *parsed_cmd = calloc(6,sizeof(GenericNode*)); 
    BulkStringNode* blk_s_nd = NULL;
    char* option = NULL;
    GenericNode* next = set_cmd_stage_a(gnode, state, *parsed_cmd);
    while(next && *state>0){
        if(next->node->type!=BULK_STR){
            return NULL;
        }
        blk_s_nd = (BulkStringNode*) next;
        option   = blk_s_nd->content;
        switch (get_option_type(option)){
            case OPTION_NX:
            case OPTION_XX:
                handle_set_options(state, *parsed_cmd, &next,3); 
                break;
            case OPTION_GET:
                handle_set_options(state, *parsed_cmd, &next,2);
                break;
            case OPTION_EX:
            case OPTION_EXAT:
            case OPTION_PX:
            case OPTION_PXAT:
                handle_set_options(state, *parsed_cmd, &next,1);
                if(!next || next->node->type!=BULK_STR || next->node->next){
                    return NULL;
                }
                BulkStringNode* last_nd = (BulkStringNode*)next;
                handle_set_options(state, *parsed_cmd, &next,0);
                __validate_px_ex_content(last_nd, state);
                break;
            default:
                return NULL;
        }
    }
    if(*state<0){
        return NULL;
    }
    return state;
}

OptionType get_option_type(const char* option) {
    if (!strcmp(option, "NX")) return OPTION_NX;
    if (!strcmp(option, "XX")) return OPTION_XX;
    if (!strcmp(option, "GET")) return OPTION_GET;
    if (!strcmp(option, "EX")) return OPTION_EX;
    if (!strcmp(option, "PX")) return OPTION_PX;
    if (!strcmp(option, "EXAT")) return OPTION_EXAT;
    if (!strcmp(option, "PXAT")) return OPTION_PXAT;
    return OPTION_INVALID;
}

void
__validate_px_ex_content(BulkStringNode* node, char* state){
    if(!state || *state<0){
        return;
    }
    if(!node){
        *state = -1;
        return;
    }
    const char* content = node->content;
    const size_t size   = (size_t)node->size;
    if(!content || size<1){
        *state = -1;
        return;
    }
    for (size_t i = 0; i < (size_t)size; i++){
        if(content[i]<48 ||content[i]>57){
            *state=-1;
            return;
        }
    }
    return;
    
}

/**
 * Processes individual SET command options and updates parsing state.
 * Responsible for:
 * - Validating option placement in command sequence
 * - Storing option node in appropriate parsed_cmd position
 * - Updating state bitmap to reflect processed option
 * - Advancing to next node in command sequence
 * 
 * @param state Pointer to current parsing state
 * @param parsed_cmd Array of parsed command components
 * @param gnode Pointer to current node being processed
 * @param bit_pos Position in state bitmap for current option (0-3)
 */
void
handle_set_options(char* state, GenericNode** parsed_cmd, GenericNode** gnode, char bit_pos){
    if(state==NULL){
        return;
    }
    if(!*gnode || !parsed_cmd){
        *state = -1;
        return;
    }
    if(!is_set_option_valid(state,bit_pos)){
        *state = -1;
        return;
        }
        parsed_cmd[(int)bit_pos] = *gnode;
        *state = *state | 1<<bit_pos;
        *gnode = (*gnode)->node->next;
        return;
}
/**
 * Validates option placement using bitmap state tracking.
 * Ensures:
 * - No duplicate options are present
 * - Options appear in valid sequence
 * - All prerequisite options are processed
 * 
 * Uses a walking bit check to validate all lower-order bits
 * are unset before allowing current bit position.
 * 
 * @param state Current parsing state bitmap
 * @param bit_pos Position in state bitmap to validate (0-8)
 * @return 1 if option placement is valid, 0 if invalid
 */
unsigned char
is_set_option_valid(char* state, char bit_pos){
    if((bit_pos<-1)||(bit_pos>8)){
        return 0;
    }
    unsigned char is_not_zero = 0;
    while(bit_pos>-1){
        is_not_zero = *state & 1<<bit_pos;
        if(is_not_zero){
            return 0;
        }
        bit_pos--;
    }
    return 1;
}

/**
 * Handles initial parsing stage for SET command's mandatory components.
 * Specifically:
 * 1. Validates and stores key (sets bit 6)
 * 2. Validates and stores value (sets bit 5)
 * 3. Ensures both components are bulk strings
 * 4. Verifies proper node linking
 * 
 * Uses state values:
 * - 64: Key validation stage
 * - 96: Value validation stage
 * 
 * @param gnode Pointer to command node
 * @param state Pointer to parsing state
 * @param parsed_cmd Array to store parsed components
 * @return Always returns NULL (state tracking handled via state parameter)
 */
GenericNode*
set_cmd_stage_a(GenericNode* gnode, char* state, GenericNode** parsed_cmd){
    if(!state){
        return NULL;
    }
    GenericNode* temp_gnode = gnode;    
    if(!gnode || !parsed_cmd){
        *state = -1;
        return NULL;
    }
    *state = 0b01000000;
    do{
        switch (*state){
            case 0b01000000:
                if(!temp_gnode->node ||  ((temp_gnode->node->type)!=BULK_STR) || (!temp_gnode->node->next)){
                    *state = -1;
                    return NULL;
                }
                *state = *state | (1<<5);
                parsed_cmd[5] = temp_gnode;
                temp_gnode = temp_gnode->node->next;
                break;
            case 0b01100000:
                if(!temp_gnode->node || (temp_gnode->node->type!=BULK_STR)){
                    *state=-1;
                    return NULL;
                }
                *state = *state | (1<<4);
                parsed_cmd[4] = temp_gnode;
                temp_gnode = temp_gnode->node->next;
                break;
            }
    } while((*state>0) &&(*state<112));
    return temp_gnode;
}
