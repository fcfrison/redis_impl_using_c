#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#define MAX_ARRAY_SIZE (size_t)512
typedef struct ArrElem ArrElem;
typedef enum{
    UNDEF_DTYPE = -1,
    SIMPLE_STR  =  0,
    INT         =  1,
    BULK_STR    =  2,
    ARRAY       =  3
} RedisDtype;
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

ArrElem*
parse_array(int fd){
    // parsing assuming perfectly crafted strings
    int arr_size = get_arr_size(fd);
    char next_char;
    if(arr_size==1){
        return NULL;
    }
    unsigned int inserted_el = 0;
    ArrElem* previous = NULL;
    ArrElem* first    = NULL;
    while(inserted_el<arr_size){
        ArrElem* next = (ArrElem*)calloc(1,sizeof(ArrElem));
        next->content = NULL;
        next->type    = UNDEF_DTYPE;
        next->next    = NULL;
        next->prev    = previous;
        next_char     = get_next_char(fd);
        switch (next_char){
            case '$':
                /* code */
                break;
            case '*':
                break;
            default:
                break;
        }
        if(!next->content){
            delete_array(next);
            return NULL;
        }
        if(!next->prev){
            first = next;
        }
        previous = next;
    }
    return first;
};


ArrElem* parse_bulk_str(void){};

int
get_arr_size(int fd){
    ssize_t ret_val;
    char* buf = (char*) calloc(MAX_ARRAY_SIZE, sizeof(char));
    unsigned int  arr_size, n_bytes = 0;
    unsigned char is_digit, state   = 0;
    while(1){
        ret_val = recv(fd, &buf[n_bytes], 1, 0);
        if(ret_val==-1){
            free(buf);
            return -1;
        }
        if(state==0){
            is_digit = (buf[n_bytes]>47 && buf[n_bytes]<58);
            if(is_digit){
                n_bytes++;
                continue;
            }
            if(buf[n_bytes]=='\r'){
                state = 1;
                continue;
            }
            free(buf);
            return -1;
        }
        if(buf[n_bytes]!='\n'){
            free(buf);
            return -1;
        }
        buf[n_bytes] = '\0';
        break;
    }
    arr_size = string_to_uint(buf);
    free(buf);
    return arr_size;
    };
char     get_next_char(int fd){return '$';};
void     delete_array(ArrElem* el){};

unsigned int
string_to_uint(char* string){
    ssize_t size = strlen(string);
    unsigned int num = 0;
    for(unsigned int i=0;i<size;i++){
        num = num * 10;
        num+=string[i]-48;
    }
    return num;
}