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
void handle_set_option(char* state,
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
            handle_set_option(state, *parsed_cmd, &gnode,3);  
        }else if(!strcmp(option,"GET")){
            handle_set_option(state, *parsed_cmd, &gnode,2);
        }else if(!strcmp(option,"EX")   || !strcmp(option,"PX") ||
                 !strcmp(option,"EXAT") || !strcmp(option,"PXAT")){
            handle_set_option(state, *parsed_cmd, &gnode,1);
            if(!gnode || gnode->node->type!=BULK_STR || gnode->node->next){
                *state = -1;
            }
            handle_set_option(state, *parsed_cmd, &gnode,0);
        }
        else{
            *state = -1;
            break;
        }   
    }
    return;
}
void
handle_set_option(char* state,
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
