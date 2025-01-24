#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/protocol.h"
#define MAX_BYTES_ARR_SIZE 20 
#define MAX_BYTES_BULK_STR 9
char* handle_echo_cmd(void* fst_nod);
char* __handle_echo_cmd(void* node, int* size);


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
                printf("%s",temp->content);
                if(!strcmp(temp->content,"ECHO")){
                    return handle_echo_cmd(node);
                }
                return NULL;
            default:
                return NULL;
        }
    }
}

//TODO: currently, the ECHO command is returning the ECHO string back.
//It should echo back only the content.
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
    switch (gnode->node->type){
        case ARRAY:
            ArrayNode* array = ((ArrayNode*) node);
            char* elements = __handle_echo_cmd(array->content, size);
            char* next     = __handle_echo_cmd(gnode->node->next, size);
            char* format   = "*%d\r\n%s%s";
            char* arr_size_s = calloc(MAX_BYTES_ARR_SIZE+1, sizeof(char));
            int   arr_size = ((ArrayNode*)gnode)->size;
            snprintf(arr_size_s,MAX_BYTES_ARR_SIZE+1,"%d",arr_size);
            *size += strlen(arr_size_s) + 3;
            char* arr_ret = calloc(*size+1,sizeof(char));
            snprintf(arr_ret,*size+1,format,arr_size,elements,next);
            free(elements);
            free(next);
            free(arr_size_s);
            return arr_ret;
        case BULK_STR:
            BulkStringNode* bulks_node = ((BulkStringNode*)node);
            char* next_nd = __handle_echo_cmd(gnode->node->next, size);
            char* content = bulks_node->content;
            char* fmt_b_str  = "%d\r\n%s\r\n%s";
            char* bs_size = calloc(MAX_BYTES_BULK_STR+1, sizeof(char));
            snprintf(bs_size,(MAX_BYTES_BULK_STR+1),"%d",bulks_node->size);
            *size+= strlen(bs_size) + 4 + strlen(content);
            char* blk_str_ret = calloc(*size+1,sizeof(char));
            snprintf(blk_str_ret,*size+1,fmt_b_str,bulks_node->size,content,next_nd);
            free(next_nd);
            free(bs_size);
            return blk_str_ret;
        default:
            break;
    }
    return NULL;
}