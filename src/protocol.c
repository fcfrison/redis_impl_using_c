#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "../include/protocol.h"
#include "../include/util.h"
BaseNode* new_base_node(RedisDtype type,void* next,void* prev);
BulkStringNode* new_bulk_str(char* content, void* next, void* prev,int size);
ArrayNode* new_array(void* content, void* next, void* prev, int size);
int  is_valid_terminator(int fd);
int  get_el_size(int fd);
int read_exact_bytes(int fd, char* buf, size_t len);
BulkStringNode* parse_bulk_str(int fd);
void log_error(const char *message);


ArrayNode*
parse_array(int fd){
    // this parsing assumes perfectly crafted strings
    int arr_size = get_el_size(fd);
    char next_char;
    if(arr_size==-2){
        return NULL;
    }
    //nill
    if(arr_size==-1){
        BaseNode* node = new_base_node(NILL_ARRAY,NULL,NULL);
        return new_array(node,NULL,NULL,1);

    }
    //empty
    if(!arr_size){
        BaseNode* node = new_base_node(EMPTY_ARRAY,NULL,NULL);
        return new_array(node,NULL,NULL,1);
    }
    unsigned int inserted_el = 0;
    ArrayNode* array = NULL;
    void*  previous  = NULL;
    while(inserted_el<(unsigned int)arr_size){
        void* current;
        if(!read_exact_bytes(fd, &next_char,1)){
            delete_array(previous,1);
            return NULL;
        };
        switch (next_char){
            case DOLLAR_BYTE:
                current = parse_bulk_str(fd);
                break;
            case ASTERISK_BYTE:
                current = parse_array(fd);
                break;
            case PLUS_BYTE:
                /* code */
                break;
            default:
                break;
        }
        if(!current){
            delete_array(previous,1);
            return NULL;
        }
        if(!array){
            array = new_array(current,NULL,NULL,arr_size);
            ((GenericNode*)current)->node->prev = array;
        }else{
            ((GenericNode*)current)->node->prev = previous;
        }
        if(previous){
            ((GenericNode*)previous)->node->next = current;
        }
        previous = current;
        inserted_el++;
    }
    return array;
};

BulkStringNode* 
parse_bulk_str(int fd){
    int  str_size = get_el_size(fd);
    if(str_size==ERR_ARRAY_OVERFLOW || str_size==ERR_INV_CHAR){
        log_error("The string size is invalid.");
        return NULL;
    }
    if(!str_size){
        //empty string
        if(!is_valid_terminator(fd)){
            return NULL;
        }
        return new_bulk_str("", NULL, NULL, 0);
    }
    if(str_size==-1){
        return new_bulk_str(NULL, NULL, NULL, -1);
    }
    if(str_size>PROTO_MAX_BULK_SIZE){
        log_error("The string size is greater than the max bulk size");
        return NULL;
    }
    char* buf = calloc(str_size+1,sizeof(char));
    if(!read_exact_bytes(fd,buf,str_size)){
        free(buf);
        return NULL;
    }
    if(!is_valid_terminator(fd)){
        free(buf);
        return NULL;
    }
    return new_bulk_str(buf,NULL,NULL,str_size);
}
BaseNode* 
new_base_node(RedisDtype type,
         void* next,
         void* prev){
    BaseNode* node = (BaseNode*)calloc(1,sizeof(BaseNode));
    if(node==NULL){
        return NULL;
    }
    node->type = type;
    node->next = next;
    node->prev = prev;
    return node;
}

ArrayNode*
new_array(void* content,
          void* next,
          void* prev,
          int   size){
    BaseNode* node = new_base_node(ARRAY,next,prev);
    ArrayNode* arr = (ArrayNode*)calloc(1,sizeof(ArrayNode));
    arr->content = content;
    arr->node    = node;
    arr->size    = size;
    return arr;
         }

BulkStringNode*
new_bulk_str(char* content,
             void* next,
             void* prev,
             int   size){
    BaseNode* node = new_base_node(BULK_STR,next,prev);
    BulkStringNode* arr = (BulkStringNode*)calloc(1,sizeof(BulkStringNode));
    if(size){
        content[size] = '\0';
    }
    arr->content  = content;
    arr->node     = node;
    arr->size     = size;
    return arr;
         }





/**
* Checks if the next characters in a file descriptor's receive buffer form a valid CRLF terminator.
*
* Reads characters one at a time from the file descriptor and verifies they match the
* expected CRLF ('\r\n') sequence. Returns immediately if an invalid character is encountered
* or if there is an error reading from the file descriptor.
*
* @param fd File descriptor to read from
* @return 1 if a valid CRLF terminator was found, 0 if invalid sequence or error
*/
int 
is_valid_terminator(int fd){
    unsigned char next_char;
    unsigned char state = 0;
    ssize_t ret_val;
    while(1){
        ret_val = read_exact_bytes(fd, &next_char, 1);
        if(!ret_val){
            return 0;
        }
        switch (state){
            case 0:
                if(next_char!='\r'){
                    return 0;
                }
                state = 1;
                break;
            case 1:
                if(next_char!='\n'){
                    return 0;
                }
                return 1;
        }
    }
}

int
get_el_size(int fd){
    // It's possible *0\r\n  --> empty arrays
    // It's possible *-1\r\n --> nil arrays
    // It's not possible *\r\n   
    ssize_t ret_val;
    char* buf = (char*) calloc(MAX_ARRAY_SIZE, sizeof(char));
    unsigned int  arr_size, n_bytes = 0;
    unsigned char is_digit, state   = START_STATE;
    unsigned char is_negative       = 0;
    unsigned char should_continue   = 1;
    while(should_continue){
        if(n_bytes>MAX_ARRAY_SIZE){
            free(buf);
            return ERR_ARRAY_OVERFLOW;
        }
        ret_val = recv(fd, &buf[n_bytes], 1, 0);
        if(ret_val==-1 || ret_val>1){
            free(buf);
            return ERR_INV_CHAR;
        }
        switch (state){
        // START_STATE: reading the first char. It must be a digit or '-'
        case START_STATE:
            is_digit = (buf[0]>47 && buf[0]<58);
            if(is_digit){
                n_bytes++;
                state=DIGIT_STATE;
                break;
            }
            if(buf[0]==MINUS_BYTE){
                state=NEG_DIGIT_STATE_I;
                is_negative = 1;
                break;
            }
            free(buf);
            return ERR_INV_CHAR;
        // DIGIT_STATE: the first char was a valid digit (0,1,2,...). The next char could be a digit or '\r'
        case DIGIT_STATE:
            is_digit = (buf[n_bytes]>47 && buf[n_bytes]<58);
                if(is_digit){
                    n_bytes++;
                    break;
                }
                if(buf[n_bytes]=='\r'){
                    state=END_STATE;
                    break;
                }
                free(buf);
                return ERR_INV_CHAR;
        // NEG_DIGIT_STATE_I: the first char was '-'. The next digit must the digit '1'
        case NEG_DIGIT_STATE_I:
            if(buf[n_bytes]!='1'){
                free(buf);
                return ERR_INV_CHAR;
            }
            n_bytes++;
            state=NEG_DIGIT_STATE_II;
            break;
        // NEG_DIGIT_STATE_II: the first char was '-'. The second was 1. The next digit must the digit '\r'
        case NEG_DIGIT_STATE_II:
            if(buf[n_bytes]!='\r'){
                free(buf);
                return ERR_INV_CHAR;
            }
            state=END_STATE;
            break;
        // END_STATE: the previous char was '\r' and the next must be '\n'
        case END_STATE:
            if(buf[n_bytes]!='\n'){
                free(buf);
                return ERR_INV_CHAR;
                }
            should_continue = 0;
            buf[n_bytes]='\0';
            break;
        }
    }
    arr_size = string_to_uint(buf);
    free(buf);
    return is_negative?(-1)*arr_size:arr_size;
    };
int 
read_exact_bytes(int fd, char* buf, size_t len){
    ssize_t rtn = recv(fd,buf,len,MSG_WAITALL);
    if(!rtn){
        log_error("connection closed by the client");
        return 0;
    }
    if((size_t)rtn < len){
        log_error("less bytes than the expected were received");
        return 0;
    }
    return 1;
    };
void 
log_error(const char *message) {
    fprintf(stderr, "Error: %s\n", message);
}


/**
 * Recursively deletes a linked list of array elements and their contents.
 * Each element can be either a nested array or a bulk string that needs
 * to be freed individually.
 *
 * @param el     Pointer to the array element to start deletion from.
 *               Can be NULL, in which case the function returns immediately.
 * @param lp_bck If non-zero, the function will first traverse backwards
 *               to the start of the list before beginning deletion.
 *               If zero, deletion starts from the provided element.
 * 
 */
void
delete_array(void* el, int lp_bck){
    if(!el){
        return;
    }
    GenericNode* current = el;
    GenericNode* next;
    // looping backwards
    while(lp_bck && current->node->prev){
        current=current->node->prev;
    }
    do {
        next=current->node->next;
        switch (current->node->type){
            case ARRAY:
                ArrayNode* curr_a = (ArrayNode*)current;
                delete_array(curr_a->content, 0);
                free(curr_a->node);
                current = next;
                break;
            case BULK_STR:
                BulkStringNode* curr_s = (BulkStringNode*)current;
                free(curr_s->node);
                free(curr_s->content);
                free(curr_s);
                current = next;
                break;
            default:
                break;
        }
    } while(next);
};


