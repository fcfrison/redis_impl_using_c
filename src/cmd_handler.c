#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "../include/protocol.h"
#include "../include/simple_map.h"
#include "../include/cmd_handler.h"
#include "../include/util.h"
#define MAX_BYTES_ARR_SIZE 20 
#define MAX_BYTES_BULK_STR 9

// Prototypes
void  cpy_str(const char* src, const size_t str_len, char** dest);
char* from_val_nd_str_to_str(const ValueNodeString* src, char** rtn_val);
void* __execute_set_check(SimpleMap* sm, KeyValuePair* kvp);
void  __validate_px_ex_content(BulkStringNode* node, char* state);
OptionType get_option_type(const char* option);

char*
parse_command(void* node, SimpleMap* sm){
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
                if(!strcmp(temp->content,"ECHO") && first_array==1){
                    return handle_echo_cmd(temp->node->next);
                }else if(!strcmp(temp->content,"SET") && first_array==1){
                    return handle_set_cmd(temp->node->next,sm);
                }else if(!strcmp(temp->content,"GET") && first_array==1){
                    return handle_get_cmd(temp->node->next,sm);
                }return NULL;
            default:
                return NULL;
        }
    }
    return NULL;
}

char*
handle_get_cmd(const void* gnode, SimpleMap* sm){
    if(!is_get_cmd_valid(gnode)){
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
            from_val_nd_str_to_str((ValueNodeString*) kvp->value, &rtn);
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
is_get_cmd_valid(const void* gnode){
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
char*
from_val_nd_str_to_str(const ValueNodeString* src, char** rtn_val){
    char content_size_buf[16] = {0};
    int   content_size = src->size;
    sprintf(content_size_buf, "%d", content_size);
    int rtn_val_size = 1 + strlen(content_size_buf) + 2 + src->size + 3;
    *rtn_val = (char*)calloc(rtn_val_size,sizeof(char));
    if(!*rtn_val){
        return NULL;
    }
    int pos = 0;
    memcpy(*rtn_val,"$",1);
    pos++;
    memcpy(*rtn_val+pos,content_size_buf,strlen(content_size_buf));
    pos+=strlen(content_size_buf);
    memcpy(*rtn_val+pos,"\r\n",2);
    pos+=2;
    memcpy(*rtn_val+pos,src->content,src->size);
    pos+=src->size;
    memcpy(*rtn_val+pos,"\r\n",2);
    pos+=2;
    *(*rtn_val+pos) = '\0';
    return *rtn_val;
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
                    from_val_nd_str_to_str(old_value_s, &rtn_val);
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
                from_val_nd_str_to_str(old_value_s, &rtn_val);
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
    for (size_t i = 0; i < size; i++){
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
