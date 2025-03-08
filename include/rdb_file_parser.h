#ifndef RDB_FILE_PARSER_H
#define RDB_FILE_PARSER_H
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define MAX_BUF_SIZE 2048
#define MAGIC_NMB_SZ 5
#define REDIS_VER_SZ 4
#define LENGTH_MASK 0x3F
typedef enum {
    STRING_ENCODING                = 0,
    LIST_ENCODING                  = 1,
    SET_ENCODING                   = 2,
    SORTED_SET_ENCODING            = 3,
    HASH_ENCODING                  = 4,
    ZIPMAP_ENCODING                = 9,
    ZIPLIST_ENCODING               = 10,
    INTSET_ENCODING                = 11,
    SORTED_SET_IN_ZIPLIST_ENCODING = 12,
    HASHMAP_IN_ZIPLIST_ENCODING    = 13,
    LIST_IN_QUICKLIST_ENCODING     = 14,
    INVALID_ENCODING               = 15
} ValueType;

typedef enum{
    AUX          = 0xFA,
    RESIZEDB     = 0xFB,
    EXPIRETIMEMS = 0xFC,
    EXPIRETIME   = 0xFD,
    SELECTDB	 = 0xFE,
    _EOF         = 0xFF
}RedisOpCodes;

typedef enum {
    READING_KEY,
    READING_VALUE
}KeyValueStatesRdb;

typedef enum {
    LENGTH_PREF_STR,
    INTEGER_AS_STR,
    COMPRESSED_STR,
    INVALID_STR
}RdbStringEncodingType;


typedef enum{
    REDIS_RDB_6BITLEN    = 0,
    REDIS_RDB_14BITLEN   = 1,
    REDIS_RDB_32BITLEN   = 2,
    REDIS_SPECIAL_FORMAT = 3
}RdbLenEncodingTypes;
typedef struct{
    RdbLenEncodingTypes encoding_type;
    int32_t length_or_format;
}RdbLengthEncoding;
unsigned int redis_rdb_file_parser(char* filename);
void label_len_encoding_and_get_str(char* next_reading_state, char** kvp_arr, const RdbLengthEncoding rdb, unsigned char* is_rdb_file_correct, char* buf, FILE* fp);
void handle_aux_state(char* prev_state, char* next_state, FILE* fp, unsigned char* is_rdb_file_correct);
void parse_aux(unsigned char* prev, unsigned char* next, unsigned char* is_rdb_file_correct, FILE* fp);
void auxiliary_field_settings(char* key, char* value);
unsigned char is_next_state_an_opcode(unsigned char byte);
void get_str(char* string, size_t size, FILE* fp, unsigned char* is_rdb_file_correct);
unsigned char parse_rdb_header(FILE* fp);
void parse_length_encoding(unsigned char byte, RdbLengthEncoding* rdb, FILE* fp);
unsigned char is_magic_header_correct(char* data, size_t nr_rd_el);
unsigned char is_version_number_correct(char* data, size_t nr_rd_el);
unsigned char allocate_mem(char** buf, size_t size);
void deallocate_arr(char** arr, size_t size);
#endif
