#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
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
void print_array(void* arr, int level) {
    int index = 0;
    for (GenericNode* temp = (GenericNode*)arr; temp; temp = temp->node->next, index++) {
        for (int i = 0; i < level; i++) {
            printf("  ");
        }
        printf("Node %d:\n", index);
        for (int i = 0; i < level; i++) {
            printf("  ");
        }
        printf("  ");
        switch (temp->node->type) {
            case BULK_STR:
                printf("Type: STRING, Content: %s\n", ((BulkStringNode*)temp)->content);
            case SIMPLE_STR:
                break;
            case ARRAY:
                printf("Type: ARRAY\n");
                print_array(((ArrayNode*)temp)->content, level + 1);
                break;
            case UNDEF_DTYPE:
                printf("Type: UNDEFINED\n");
                break;
            default:
                printf("Type: UNKNOWN\n");
        }
    }
}
void 
log_error(const char *message) {
    fprintf(stderr, "Error: %s\n", message);
}

/**
 * Suspends program execution for specified milliseconds 
 * with retry capability on interruption.
 *
 * @param msec        Time to sleep in milliseconds
 * @param max_retries Maximum number of retry attempts if sleep is
 * interrupted
 *
 * @return  0 on successful sleep
 *         -1 on error (e.g., invalid arguments or system error)
 *
 * The function converts milliseconds to seconds and nanoseconds
 * for precise sleep timing.
 * If interrupted (EINTR), it retries up to max_retries times. For
 * other errors, it returns immediately with -1 and sets errno.
 */
int 
msleep(long msec,int max_retries){
    struct timespec ts;
    int res;
    ts.tv_sec  =  msec / 1000;            //sleep time in seconds
    ts.tv_nsec = (msec % 1000) * 1000000; // sleep time in nanoseconds
    errno      = 0;
    unsigned char retries;
    do{
        res = nanosleep(&ts, NULL);
        if(res!=-1){
            return res;
        }
        char* msg = "Error: ";
        perror(msg);
        switch (errno){
            case EINTR: 
                retries++;
                errno = 0; 
                break; 
            default: 
                return res;
        }
    }while(retries<max_retries);
    return res;
}