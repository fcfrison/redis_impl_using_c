#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "../include/protocol.h"
#include "../include/simple_map.h"
#include "../include/cmd_handler.h"
#define MAX_BYTES_ARR_SIZE 20 
#define MAX_BYTES_BULK_STR 9





char*
parse_command(void* node, SimpleMap* sm){
    GenericNode* gnode = (GenericNode*) node;
    while(gnode){
        switch (gnode->node->type){
            case ARRAY:
                gnode = ((ArrayNode*) gnode)->content;
                break;
            case BULK_STR:
                BulkStringNode* temp = (BulkStringNode*) gnode;
                if(!strcmp(temp->content,"ECHO")){
                    return handle_echo_cmd(temp->node->next);
                }else if(!strcmp(temp->content,"SET")){
                    return handle_set_cmd(temp->node->next,sm);
                }
                return NULL;
            default:
                return NULL;
        }
    }
    return NULL;
}

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
    return execute_set_cmd(state, parsed_cmd,  sm);
    // Next step is decide what to do, based on the current state
    // remember that parsed_cmd must be freed;
}



KeyNode*
create_key_node(char* content,
                unsigned int ex,
                unsigned int px,
                int size){
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
    if(!gnode || !gnode->node->type){
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

void* compare(const void* a, const void* b){
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
        clean_up_execute_set_cmd(key, NULL);
        return NULL;
    }
    kvp = create_key_val_pair(key,value);
    switch (state){
        case SET_BASIC:
            return execute_set_basic(sm, kvp, key, value);
        case SET_GET:
            return execute_set_get(sm, kvp);
        case SET_EX_PX_EXAL_PXAT:
        case SET_GET_EXPX:
        case SET_NXXX:
        case SET_NXXX_EXPX:
        case SET_NXXX_GET:
        default:
            break;
    }
    return NULL;
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
execute_set_get(SimpleMap*    sm,
                KeyValuePair* kvp){
    if(!kvp || !kvp->key || !kvp->value
            || !sm->values || !sm->keys){
        if(kvp){
            clean_up_execute_set_cmd(kvp->key,kvp->value);
            free(kvp);
        }
        return NULL;
    }
    if(!((KeyNode*)kvp->key)->content){
        clean_up_execute_set_cmd(kvp->key,kvp->value);
        free(kvp);
        return NULL;
    }
    int rtn = set(sm,kvp,&compare);
    char *rtn_val = NULL, *rtn_msg = "$-1\r\n";
    if(rtn==ERROR_SET_SM_RTN){
        clean_up_execute_set_cmd(kvp->key,kvp->value);
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
        clean_up_execute_set_cmd(kvp->key, kvp->value);
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
                char content_size_buf[16] = {0};
                int   content_size = old_value_s->size;
                sprintf(content_size_buf, "%d", content_size);
                int rtn_val_size = 1 + strlen(content_size_buf) + 2 + old_value_s->size + 3;
                rtn_val = (char*)calloc(rtn_val_size,sizeof(char));
                int pos = 0;
                memcpy(rtn_val,"$",1);
                pos++;
                memcpy(rtn_val+pos,content_size_buf,strlen(content_size_buf));
                pos+=strlen(content_size_buf);
                memcpy(rtn_val+pos,"\r\n",2);
                pos+=2;
                memcpy(rtn_val+pos,old_value_s->content,old_value_s->size);
                pos+=old_value_s->size;
                memcpy(rtn_val+pos,"\r\n",2);
                pos+=2;
                rtn_val[pos] = '\0';
                clean_up_execute_set_cmd(kvp->key, kvp->value);
                free(kvp);
                return rtn_val;
            }
        default:
            clean_up_execute_set_cmd(kvp->key, kvp->value);
            free(kvp);
            rtn_val = (char*)calloc(strlen(rtn_msg)+1,sizeof(char));
            strcpy(rtn_val,rtn_msg);
            return rtn_val;
    }
}
char*
execute_set_basic(SimpleMap*    sm,
                  KeyValuePair* kvp,
                  KeyNode*      key,
                  ValueNode*    value
                ){
    int rtn = set(sm,kvp,&compare);
    char *rtn_val = NULL, *rtn_msg = "+OK\r\n";
    if(rtn==ERROR_SET_SM_RTN){
        clean_up_execute_set_cmd(key,value);
        free(kvp);
    }else if(rtn==SUCESS_SET){
        rtn_val = (char*)calloc(strlen(rtn_msg)+1,sizeof(char));
        strcpy(rtn_val,rtn_msg);
        free(kvp);
    }else{
        clean_up_execute_set_cmd(kvp->key, kvp->value);
        free(kvp);
        rtn_val = (char*)calloc(strlen(rtn_msg)+1,sizeof(char));
        strcpy(rtn_val,rtn_msg);
    }
    return rtn_val;

}

void
clean_up_execute_set_cmd(KeyNode* key, ValueNode* value){
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
    if(!state){
        return NULL;
    }
    if(!node){
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
        if(!strcmp(option,"NX") || !strcmp(option,"XX")){
            handle_set_options(state, *parsed_cmd, &next,3);  
        }else if(!strcmp(option,"GET")){
            handle_set_options(state, *parsed_cmd, &next,2);
        }else if(!strcmp(option,"EX")   || !strcmp(option,"PX") ||
                 !strcmp(option,"EXAT") || !strcmp(option,"PXAT")){
            handle_set_options(state, *parsed_cmd, &next,1);
            if(!next || next->node->type!=BULK_STR || next->node->next){
                return NULL;
            }
            handle_set_options(state, *parsed_cmd, &next,0);
        }
        else{
            return NULL;
        }   
    }
    if(*state<0){
        return NULL;
    }
    return state;
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
handle_set_options(char* state,
                  GenericNode** parsed_cmd,
                  GenericNode** gnode,
                  char bit_pos){
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
