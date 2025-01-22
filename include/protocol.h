#ifndef PROTOCOL_H
#define PROTOCOL_H
#define MAX_ARRAY_SIZE (size_t)8192
#define PROTO_MAX_BULK_SIZE    512000000
#define ERR_INV_CHAR           -2
#define ERR_ARRAY_OVERFLOW     -2
#define DOLLAR_BYTE            '$'
#define ASTERISK_BYTE          '*'
#define PLUS_BYTE              '+'
#define MINUS_BYTE             '-'
#define COLON_BYTE             ':'
typedef struct BaseNode BaseNode;
typedef struct ArrayNode ArrayNode;
typedef struct StringNode StringNode;
typedef struct BulkStringNode BulkStringNode;
typedef enum{
    UNDEF_DTYPE = -1,
    SIMPLE_STR  =  0,
    INT         =  1,
    BULK_STR    =  2,
    ARRAY       =  3,
    NILL_ARRAY  =  4,
    EMPTY_ARRAY =  5
} RedisDtype;
typedef enum ArraySizeStates{
    START_STATE        = 0,
    DIGIT_STATE        = 1,
    NEG_DIGIT_STATE_I  = 2,
    NEG_DIGIT_STATE_II = 3,
    END_STATE          = 4

} ArraySizeStates;

struct BaseNode{
    RedisDtype  type;
    void*   next;
    void*   prev;
};

struct ArrayNode{
    BaseNode*   node;
    void*       content;
    int         size;
};

struct BulkStringNode{
    BaseNode*   node;
    char*       content;
    int         size;
};



ArrayNode* parse_array(int fd);
BulkStringNode* parse_bulk_str(int fd);
void delete_array(void* el, int lp_bck);
#endif