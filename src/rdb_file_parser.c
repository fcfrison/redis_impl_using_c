#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> 
#include <string.h>
#include "../include/rdb_file_parser.h"
unsigned int
redis_rdb_file_parser(char* filename){
    FILE* fp;
    int fread_rtn;
    if(!(fp=fopen(filename,"rb"))){
        perror("ERROR");
        return 0;
    };
    if(!parse_rdb_header(fp)){
        return 0;
    };
    char curr_state = 0x0F;//version state
    unsigned char next_state;
    fread(&next_state,sizeof(char),1,fp);
    if(!next_state){
        return 0;
    };
    unsigned char is_rbd_file_correct = 1;
    while(1){
        if(!is_rbd_file_correct){
            break;
        }
        switch (next_state){
            case AUX:
                handle_aux_state(&curr_state, &next_state, fp, &is_rbd_file_correct);
                break;
            case RESIZEDB:
                /** */
                break;
            case EXPIRETIMEMS:
                break;
            case EXPIRETIME:
                break;
            case SELECTDB:
                break;
            case _EOF:
                break;
        default:
        // possivelmente um key-value
            break;
        };
    };
};
// by definition, the auxiliary fields could be empty
void
handle_aux_state(char* prev_state, char* next_state, FILE* fp, unsigned char* is_rdb_file_correct){
    if(*prev_state!=0x0F || *next_state!=AUX){//prev state must be version
        *is_rdb_file_correct = 0;
        puts("ERROR: auxiliary field must be preceeded by the REDIS version.");
        return;
    };
    *prev_state = *next_state;
    if(fread(next_state, sizeof(char),1,fp)<1){
        *is_rdb_file_correct = 0;
        return;
    };
    //unsigned char state = (*next_state);
    parse_aux(prev_state, next_state, is_rdb_file_correct, fp);
};



void
parse_aux(unsigned char* prev, unsigned char* next, unsigned char* is_rdb_file_correct, FILE* fp){
    if(!prev || !next || !is_rdb_file_correct || !fp || *prev!=AUX){
        *is_rdb_file_correct = 0;
        return;
        };
    if(is_next_state_an_opcode(*next)){
        *is_rdb_file_correct = (*next==SELECTDB)?1:0;
        return;
    };
    char next_reading_state = READING_KEY;
    RdbLengthEncoding rdb;
    char* kvp_arr[2] = {NULL, NULL};
    size_t fread_rtn;

    do{
        parse_length_encoding(*next, &rdb, fp);
        if(rdb.length_or_format==-1){
            *is_rdb_file_correct = 0;
            deallocate_arr(kvp_arr, 2);
            return;
        };
        label_len_encoding_and_get_str(&next_reading_state,
                                       kvp_arr,
                                       rdb,
                                       is_rdb_file_correct,
                                       (char*)calloc(rdb.length_or_format+1,sizeof(char)),
                                       fp);
        if(next_reading_state==READING_KEY && *is_rdb_file_correct){
            auxiliary_field_settings(kvp_arr[0],kvp_arr[1]);
            deallocate_arr(kvp_arr, 2);
        };
        if(!(*is_rdb_file_correct)){
            deallocate_arr(kvp_arr, 2);
            return;
        }
        fread_rtn = fread(next, sizeof(char), 1,fp);
        if(!fread_rtn){
            *is_rdb_file_correct = 0;
            deallocate_arr(kvp_arr, 2);
            return;
        };
    } while(!is_next_state_an_opcode(*next)); 
};
void
label_len_encoding_and_get_str(char* next_reading_state,
                               char** kvp_arr,
                               const RdbLengthEncoding rdb,
                               unsigned char* is_rdb_file_correct,
                               char* buf,
                               FILE* fp){
    switch (rdb.encoding_type){
        case REDIS_RDB_6BITLEN:
        case REDIS_RDB_14BITLEN:
        case REDIS_RDB_32BITLEN:   
            if(*next_reading_state==READING_KEY){
                kvp_arr[0] = buf;
                get_str(kvp_arr[0], rdb.length_or_format, fp, is_rdb_file_correct);
                *next_reading_state=READING_VALUE;
            }else{
                kvp_arr[1] = buf;
                get_str(kvp_arr[1], rdb.length_or_format, fp, is_rdb_file_correct);
                *next_reading_state=READING_KEY;
            };
            break;
        case REDIS_SPECIAL_FORMAT:
            *is_rdb_file_correct = 0;
            return;
    };
};
void
auxiliary_field_settings(char* key, char* value){
    if(!key){
        fprintf(stderr,"ERROR: auxiliary field key is null.");
        return;
    };
    if(!value){
        fprintf(stderr,"ERROR: auxiliary field key is null.");
        return;
    };
    char* aux_settings[] = {
        "redis-ver", "redis-bits", "ctime", "used-mem", NULL
    };
    unsigned int i = 0;
    for (char* setting = aux_settings[0]; setting; setting = aux_settings[i]){
        if(strcmp(setting,key)==0){
            printf("key: %s, value: %s\n",key,value);
        };
        i++;
    };
    return;
}
unsigned char
is_next_state_an_opcode(unsigned char byte){
    switch (byte){
        case SELECTDB:
        case AUX:
        case RESIZEDB:
        case EXPIRETIMEMS:
        case EXPIRETIME:
        case _EOF:
            return 1;
        default:
            return 0;
        }
}
void
get_str(char* string, size_t size, FILE* fp, unsigned char* is_rdb_file_correct){
    if(!string || !size){
        return;
    }
    if(!fp){
        *is_rdb_file_correct = 0;
        return;
    };
    size_t fread_rtn = fread(string, sizeof(char), size,fp);
    if(fread_rtn<size){
        *is_rdb_file_correct = 0;
        return;
    };
    string[size] = '\0';
    return;
};
void
parse_length_encoding(unsigned char byte, RdbLengthEncoding* rdb, FILE* fp){
    unsigned char temp = byte >> 6;
    size_t fread_rtn;
    switch (temp){
        case REDIS_RDB_6BITLEN:
            //next 6 bytes represent the length
            rdb->encoding_type = REDIS_RDB_6BITLEN;
            rdb->length_or_format = byte & LENGTH_MASK;
            break;
        case REDIS_RDB_14BITLEN:
            //Read one additional byte. The combined 14 bits represent the length
            //Assuming little endianess
            unsigned char extra_byte;
            fread_rtn = fread(&extra_byte,sizeof(char), 1,fp);
            rdb->encoding_type = REDIS_RDB_14BITLEN;
            if(fread_rtn<1){
                rdb->length_or_format = -1;
                return;
            };
            rdb->length_or_format = extra_byte<<8 | (byte & LENGTH_MASK);
            return;
        case REDIS_RDB_32BITLEN:
            char length[4];
            fread_rtn = fread(&length,sizeof(char), 4,fp);
            rdb->encoding_type = REDIS_RDB_32BITLEN;
            if(fread_rtn<4){
                rdb->length_or_format = -1;
                return;
            };
            // discard the remaining 6 bytes and the next 4 bytes represent the length
            // assuming the file is in little-endian
            rdb->length_or_format = length[3]<<24 | length[2]<<16 | length[1]<<8 | length[0];
            return;
        case REDIS_SPECIAL_FORMAT:
            // The next object is encoded in a special format. The remaining 6 bits indicate the format.
            rdb->encoding_type = REDIS_SPECIAL_FORMAT;
            rdb->length_or_format = byte & LENGTH_MASK; //format
            break;
        };
};
unsigned char
parse_rdb_header(FILE* fp){
    if(!fp){
        return 0;
    };
    char* buf;
    size_t fread_rtn;
    if(!allocate_mem(&buf, (size_t) MAGIC_NMB_SZ)){
        return 0;
    };
    // get first 5 bytes
    fread_rtn = fread(buf,sizeof(char),MAGIC_NMB_SZ,fp);
    if(!is_magic_header_correct(buf, fread_rtn)){
        puts("ERROR: magic number is not correct.");
        free(buf);
        return 0;    
    };
    free(buf);
    // get next 4 bytes
    if(!allocate_mem(&buf, (size_t) REDIS_VER_SZ)){
        return 0;
    };
    fread_rtn = fread(buf,sizeof(char),4,fp);
    if(!is_version_number_correct(buf, fread_rtn)){
        puts("ERROR: REDIS version is not correct.");
        free(buf);
        return 0;
    };
    free(buf);
    return 1;
};
unsigned char 
is_magic_header_correct(char* data, size_t nr_rd_el){
    if(!data || nr_rd_el<MAGIC_NMB_SZ){
        return 0;
    };
    unsigned char rtn = strcmp(data,"REDIS")==0?1:0;
    return rtn;
};
unsigned char
is_version_number_correct(char* data, size_t nr_rd_el){
    if(!data || nr_rd_el<REDIS_VER_SZ){
        return 0;
    };
    char* available_versions[] = {
        "0001", "0002", "0003", "0004",
        "0005", "0006", "0007", "0008",
        "0009", "0010", "0011", NULL
    };
    char* version = NULL;
    unsigned char count = 0;
    do{
        if(strcmp(data,available_versions[count])==0){
            return 1;
        };
        version = available_versions[++count];
    } while (version);
    return 0;
};
unsigned char
allocate_mem(char** buf, size_t size){
    // get 5 bytes
    *buf = (char*)calloc(size,sizeof(char));
    if(!buf){
        perror("ERROR");
        return 0;
    };
    return 1;
}
void
deallocate_arr(char** arr, size_t size){
    if(!arr){
        return;
    }
    for (size_t i = 0; i < size; i++){
        if(arr[i]){
            free(arr[i]);
            arr[i] = NULL;
        };
    }
    return;
}