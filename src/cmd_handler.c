#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/protocol.h"
#define MAX_BYTES_ARR_SIZE 20 
char* 
handle_echo_cmd(void* fst_nod);
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
                if(strcmp(temp->content,"ECHO")){
                    return handle_echo_cmd(node);
                }
            default:
                return 0;
                break;
        }
    }
}

char* 
handle_echo_cmd(void* fst_nod){
    int* size = calloc(1,sizeof(int));
    char* ret_val = __handle_echo_cmd(fst_nod,size);
    free(size);
    return ret_val;
}
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
            char* arr_std = "*%d\r\n%s%s";
            char* arr_size_s[MAX_BYTES_ARR_SIZE];
            int arr_size = ((ArrayNode*)gnode)->size;
            snprintf(arr_size_s,MAX_BYTES_ARR_SIZE-1,"%d",arr_size);
            *size += strlen(arr_size_s) + 3;
            // TODO: continue the implementation from here
            break;
        case BULK_STR:
            break;
        default:
            break;
    }
    
    
    return NULL;
}