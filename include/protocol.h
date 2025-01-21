#define MAX_ARRAY_SIZE (size_t)8192
#define PROTO_MAX_BULK_SIZE    512000000
#define ERR_INV_CHAR           -2
#define ERR_ARRAY_OVERFLOW     -2
#define DOLLAR_BYTE            '$'
#define ASTERISK_BYTE          '*'
#define PLUS_BYTE              '+'
#define MINUS_BYTE             '-'
#define COLON_BYTE             ':'
typedef struct ArrElem ArrElem;
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

struct ArrElem{
    void*      content;
    RedisDtype type;
    ArrElem*   next;
    ArrElem*   prev;
};

ArrElem* parse_array(int fd);
ArrElem* parse_bulk_str(int fd);
void print_array(ArrElem* arr, int level);
void     delete_array(ArrElem* el);
