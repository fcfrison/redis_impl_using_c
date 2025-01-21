#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/util.h"
#include "../include/protocol.h"
unsigned int
string_to_uint(char* string){
    size_t size = strlen(string);
    unsigned int num = 0;
    for(unsigned int i=0;i<size;i++){
        num = num * 10;
        num+=string[i]-48;
    }
    return num;
}
void print_array(ArrElem* arr, int level) {
    int index = 0;
    for (ArrElem* temp = arr; temp; temp = temp->next, index++) {
        for (int i = 0; i < level; i++) {
            printf("  ");
        }
        printf("Node %d:\n", index);
        for (int i = 0; i < level; i++) {
            printf("  ");
        }
        printf("  ");
        switch (temp->type) {
            case BULK_STR:
            case SIMPLE_STR:
                printf("Type: STRING, Content: %s\n", (char*)temp->content);
                break;
            case ARRAY:
                printf("Type: ARRAY\n");
                print_array(temp->content, level + 1);
                break;
            case UNDEF_DTYPE:
                printf("Type: UNDEFINED\n");
                break;
            default:
                printf("Type: UNKNOWN\n");
        }
    }
}
