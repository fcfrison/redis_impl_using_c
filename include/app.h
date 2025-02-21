#ifndef APP_H
#define APP_H
#include "./simple_map.h"
#define DEFAULT_REDIS_RDBFILE "rdbfile.rdb" 
#define DEFAULT_REDIS_DIR     "/tmp/redis-data"



void* app_code(void* fd);

#endif