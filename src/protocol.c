#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "../include/protocol.h"
#include "../include/util.h"
ArrElem* new_arr_el(void* content,
                    RedisDtype type,
                    ArrElem* next,
                    ArrElem* prev);
int  is_valid_terminator(int fd);
int  get_arr_size(int fd);
char get_next_char(int fd);
void delete_array(ArrElem* el);
ArrElem*
parse_array(int fd){
    // this parse assumes perfectly crafted strings
    int arr_size = get_arr_size(fd);
    char next_char;
    if(arr_size==-2){
        return NULL;
    }
    if(arr_size==-1){
        //nill array
        return new_arr_el(NULL,NILL,NULL,NULL);
    }
    if(!arr_size){
        //empty array
        if(!is_valid_terminator(fd)){
            return NULL;
        }  
        return new_arr_el("\0",BULK_STR,NULL,NULL);
    }
    unsigned int inserted_el = 0;
    ArrElem* previous = NULL;
    ArrElem* first    = NULL;
    while(inserted_el<arr_size){
        ArrElem* next = new_arr_el(NULL,UNDEF_DTYPE,NULL,previous);
        if(!next){
            delete_array(previous);
            return NULL;
        }
        next_char = get_next_char(fd);
        switch (next_char){
            case DOLLAR_BYTE:
                /* code */
                break;
            case ASTERISK_BYTE:
                /* code */
                break;
            case PLUS_BYTE:
                /* code */
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
ArrElem* new_arr_el(void* content,
                    RedisDtype type,
                    ArrElem* next,
                    ArrElem* prev){
    ArrElem* arr_el = (ArrElem*)calloc(1,sizeof(ArrElem));
    if(arr_el==NULL){
        return NULL;
    }
    arr_el->content = content;
    arr_el->type    = type;
    arr_el->next    = next;
    arr_el->prev    = prev;
    return arr_el;
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
int is_valid_terminator(int fd){
    unsigned char next_char;
    unsigned char state = 0;
    ssize_t ret_val;
    while(1){
        ret_val = recv(fd, &next_char, 1, 0);
        if(ret_val==-1){
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

ArrElem* parse_bulk_str(void){};

int
get_arr_size(int fd){
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
            if(buf[0]='-'){
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
            break;
        }
    }
    arr_size = string_to_uint(buf);
    free(buf);
    return is_negative?(-1)*arr_size:arr_size;
    };
char     get_next_char(int fd){return '$';};
void     delete_array(ArrElem* el){};

