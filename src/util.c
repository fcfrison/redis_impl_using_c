#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <errors.h>
#include <getopt.h>
#include <fnmatch.h>
#include "../include/util.h"
#include "../include/protocol.h"
#include "../include/cmd_handler.h"
#include "../include/app.h"
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

void 
cli_parser(int    argc,
           char** argv,
           char** dir,
           char** dbfilename,
           char*  default_redis_dir,
           char*  default_redis_rdbfile){
    *dir = *dbfilename = NULL;
    struct option options[] = {
        {"dbfilename",required_argument, NULL, 'c'},
        {"dir", required_argument, NULL,'d'},
        {NULL, 0, NULL, 0}};
    int opt;
    //Reset optind
    optind = 1; 
    while((opt = getopt_long(argc, argv, "c:d:?",options, NULL)) != -1){
        if(!optarg){
            puts("Wrong syntax");
            exit(EXIT_FAILURE);
        };
        switch (opt){
            case 'c':
                *dbfilename = optarg;
                puts(*dbfilename);
                break;
            case 'd':
                *dir = optarg;
                puts(*dir);
                break;
            case '?':
            default:
                exit(EXIT_FAILURE);
        };
    };
    if(!*dir){
        *dir = (char*)calloc(strlen(default_redis_dir),sizeof(char));
        strcpy(*dir,default_redis_dir);
    };
    if(!*dbfilename){
        *dbfilename = (char*)calloc(strlen(default_redis_rdbfile),sizeof(char));
        strcpy(*dbfilename,default_redis_rdbfile);
    };
    return;
}
void*
compare_str(const void* key_a, const void* key_b){
    if(!key_a || !key_b){
        return NULL;
    }
    char* a = (char*) key_a;
    char* b = (char*) key_b;
    return strcmp(a,b)==0? a : NULL;
}
void
setup(int argc, char** argv, SimpleMap** sm, SimpleMap** config_dict){
    *sm = create_simple_map();
    *config_dict = create_simple_map();
    char* keys[2] = {"dir", "dbfilename"};
    char* dir = NULL, *dbfilename = NULL;
    char* values[2] = {dir,dbfilename};
    size_t key_len;
    KeyValuePair* kvp;
    cli_parser(argc,
               argv,
               &values[0],
               &values[1],
               DEFAULT_REDIS_DIR,
               DEFAULT_REDIS_RDBFILE);
    
    for (size_t i = 0; i < 2; i++){
        key_len = strlen(keys[i])+1;
        char* key = (char*)calloc(key_len,sizeof(char));
        strcpy(key,keys[i]);
        
        kvp = create_key_val_pair(create_key_node(key, 0, 0, strlen(keys[i])),
                                  create_value_node_string(values[i],BULK_STR,strlen(values[i]))
                                );
        if(set(*config_dict, kvp, &compare)!=SUCESS_SET){
            exit(EXIT_FAILURE);
        }
        free(kvp);
    }
    return;
};
/**
 int
 does_the_strings_matches(char* pattern, char* target){
     if(!pattern || !target){
         return MATCH_ERROR;
     };
     const int FNM_IGNORECASE = 1 << 4; //the original const wasn't recognized by the compiler
     return fnmatch(pattern,target,FNM_IGNORECASE);
     
 }
 * 
 */
MatchErrorState
does_the_strings_match(const char* pattern, const char* target){
    if(!pattern || !target){
        return MATCH_ERROR;
    };
    const int FNM_IGNORECASE = 1 << 4; //the original const wasn't recognized by the compiler
    return fnmatch(pattern,target,FNM_IGNORECASE)==MATCH?MATCH:NO_MATCH;
}
