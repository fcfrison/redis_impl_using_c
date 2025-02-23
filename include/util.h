#ifndef UTIL_H
#define UTIL_H
#include "./protocol.h"
#include "./simple_map.h"
typedef enum {
    MATCH_ERROR = -1,
    MATCH       =  0,
    NO_MATCH    =  1
}MatchErrorState;
unsigned int string_to_uint(char* string);
void print_array(void* arr, int level);
void log_error(const char *message);
int  msleep(long msec,int max_retries);
void cli_parser(int    argc,
                char** argv,
                char** dir,
                char** dbfilename,
                char*  default_redis_dir,
                char*  default_redis_rdbfile);
void  setup(int argc, char** argv, SimpleMap** sm, SimpleMap** config_dict);
void* compare_str(const void* key_a, const void* key_b);
MatchErrorState does_the_strings_matches(const char* pattern, const char* target);
#endif