#include <stdio.h>
#include <sys/socket.h>
#define MAX_ARRAY_SIZE (size_t)512
typedef struct{
    void* content;
    unsigned char type;
    struct ArrElem* next;
    struct ArrElem* prev;
} ArrElem;
typedef enum{
    SIMPLE_STR = 0,
    INT        = 1,
    BULK_STR   = 2,
    ARRAY      = 3
} RedisDtype;

ArrElem* parse_array(int fd);
ArrElem* parse_bulk_str(void);
int      get_arr_size(int fd);
char     get_next_char(int fd);
void     delete_array(ArrElem* el);

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
        next->type    = NULL;
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
    int n_bytes = 0;
    while(1){
        ret_val = recv(fd, buf, 1, 0);
        if(ret_val==-1 || buf[n_bytes]<48 || buf[n_bytes]>57){
            //provavelmente eu tenha que iterar sobre o vetor
            free(buf);
            return -1;
        }
        buf++;
    }
    free(buf);
    //converter de char para integer
    return fd;
    };
char     get_next_char(int fd){return '$';};
void     delete_array(ArrElem* el){return NULL;};
