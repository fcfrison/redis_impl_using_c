#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/protocol.h"
char* handle_echo_cmd(void* fst_nod);
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
    return NULL;

}