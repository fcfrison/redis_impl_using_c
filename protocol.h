#define MAX_ARRAY_SIZE (size_t)8192
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
    NILL        =  4
} RedisDtype;
typedef enum ArraySizeStates{
    START_STATE        = 0,
    DIGIT_STATE        = 1,
    NEG_DIGIT_STATE_I  = 2,
    NEG_DIGIT_STATE_II = 3,
    END_STATE          = 4

};
struct ArrElem{
    void*      content;
    RedisDtype type;
    ArrElem*   next;
    ArrElem*   prev;
} ;

ArrElem* parse_array(int fd);
ArrElem* parse_bulk_str(void);
int      get_arr_size(int fd);
char     get_next_char(int fd);
void     delete_array(ArrElem* el);
unsigned int string_to_uint(char* string);
