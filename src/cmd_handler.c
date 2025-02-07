#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/protocol.h"
#define MAX_BYTES_ARR_SIZE 20 
#define MAX_BYTES_BULK_STR 9
char* handle_echo_cmd(void* fst_nod);
char* __handle_echo_cmd(void* node, int* size);
char* handle_set_cmd(void* node);
void validate_set_cmd(void* node, char* state, GenericNode*** parsed_cmd);
void handle_set_options(char* state,
                       GenericNode** parsed_cmd,
                       GenericNode** gnode,
                       char bit_pos);

unsigned char is_set_option_valid(char* state, char bit_pos);
void* set_cmd_stage_a(GenericNode** gnode, char* state, GenericNode** parsed_cmd);
char*
parse_command(void* node){
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
                    return handle_set_cmd(temp->node->next);
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
handle_set_cmd(void* node){
    // The void* node is already the arguments of the set cmd
    char state;
    GenericNode** parsed_cmd = NULL;
    validate_set_cmd(node, &state, &parsed_cmd);
    if(state==-1){
        return NULL;
    }
    return NULL;
    // Next step is decide what to do, based on the current state
    // remember that parsed_cmd must be freed;
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
void
validate_set_cmd(void* node, char* state, GenericNode*** parsed_cmd){
    if(!node){
        *state = -1;
        return;
    }
    *state = 0;
    *state = 1<<6;
    GenericNode* gnode = (GenericNode*)node;
    *parsed_cmd = calloc(6,sizeof(GenericNode*)); 
    BulkStringNode* blk_s_nd = NULL;
    char* option = NULL;
    set_cmd_stage_a(&gnode, state, *parsed_cmd);
    while(gnode && *state>0){
        if(gnode->node->type!=BULK_STR){
            *state = -1;
            continue;
        }
        blk_s_nd = (BulkStringNode*) gnode;
        option   = blk_s_nd->content;
        if(!strcmp(option,"NX") || !strcmp(option,"XX")){
            handle_set_options(state, *parsed_cmd, &gnode,3);  
        }else if(!strcmp(option,"GET")){
            handle_set_options(state, *parsed_cmd, &gnode,2);
        }else if(!strcmp(option,"EX")   || !strcmp(option,"PX") ||
                 !strcmp(option,"EXAT") || !strcmp(option,"PXAT")){
            handle_set_options(state, *parsed_cmd, &gnode,1);
            if(!gnode || gnode->node->type!=BULK_STR || gnode->node->next){
                *state = -1;
            }
            handle_set_options(state, *parsed_cmd, &gnode,0);
        }
        else{
            *state = -1;
            break;
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
handle_set_options(char* state,
                  GenericNode** parsed_cmd,
                  GenericNode** gnode,
                  char bit_pos){
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
        is_not_zero = state[(int)bit_pos] & 1<<bit_pos;
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
void*
set_cmd_stage_a(GenericNode** gnode, char* state, GenericNode** parsed_cmd){
    if(!gnode){
        return NULL;
    }
    *state = 1<<6;
    do{
        switch (*state){
        case 64:
            if((((*gnode)->node->type)!=BULK_STR) || (!(*gnode)->node->next)){
                *state = -1;
                continue;
            }
            *state = *state | (1<<5) ;
            parsed_cmd[5] = *gnode;
            *gnode = (*gnode)->node->next;
            break;
        case 96:
           if(((*gnode)->node->type)!=BULK_STR){
            *state=-1;
            continue;
           }
           *state = *state | (1<<4);
           parsed_cmd[4] = *gnode;
           *gnode = (*gnode)->node->next;
           break;
        }
    } while((0<*state) &&(*state<112));
    return NULL;
}
