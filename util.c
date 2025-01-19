#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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