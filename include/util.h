#ifndef UTIL_H
#define UTIL_H
#include "./protocol.h"
unsigned int string_to_uint(char* string);
void print_array(void* arr, int level);
void log_error(const char *message);
#endif