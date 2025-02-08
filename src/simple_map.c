#include <stdlib.h>
#include "../include/simple_map.h"
unsigned char is_full(SimpleMap* sm);
SimpleMap* double_arrays(SimpleMap* sm);
void* set(SimpleMap* sm, KeyBaseStruct* key, ValueBaseStruct* value);
void* find(const SimpleMap* sm, const void* key, void*(cmp_fptr)(const void* a, const void* b));
SimpleMap* create_simple_map(void);

void*
remove(SimpleMap* sm, void* key, void*(cmp_fptr)(void* a, void* b)){
    int i = 0;
    void* rmvd_key = NULL; 
    while(i<=sm->top){
        rmvd_key = sm->keys[i];
        if(!cmp_fptr(key,rmvd_key)){
            i++;
            continue;
        };
        while(1){
            if(i==sm->top){
                sm->keys[i] = NULL;
                sm->values[i] = NULL;
                sm->top--;
                return rmvd_key;
            }
            sm->keys[i] = sm->keys[i+1];
            sm->values[i] = sm->values[i+1];
            i++;
        }
    }
}


unsigned char is_full(SimpleMap* sm){
    if(sm->top+1==sm->capacity){
        return 1;
    }
    return 0;
}
SimpleMap*
double_arrays(SimpleMap* sm){
    KeyBaseStruct**   keys   = NULL;
    ValueBaseStruct** values = NULL;
    sm->capacity = 2*sm->capacity;
    keys     = (KeyBaseStruct**)realloc(sm->keys,sm->capacity*sizeof(KeyBaseStruct*));
    if(!keys){
        return NULL;
    }
    values   = (ValueBaseStruct**)realloc(sm->values,sm->capacity*sizeof(ValueBaseStruct*));
    if(!values){
        return NULL;
    }
    sm->keys   = keys;
    sm->values = values;
    return sm;
}

void*
set(SimpleMap* sm, KeyBaseStruct* key, ValueBaseStruct* value){
    if(is_full(sm)){
        if(!double_arrays(sm)){
            return NULL;
        }
    }
    sm->top++;
    sm->keys[sm->top]   = key;
    sm->values[sm->top] = value;
    return sm;
}

void* 
compare(const void* a, const void* b){
    const KeyBaseStruct* el_a = (const KeyBaseStruct*)a;
    const KeyBaseStruct* el_b = (const KeyBaseStruct*)b;
    if(el_a->size!=el_b->size){
            return NULL;
        }
    if(memcmp(el_a->content, el_b->content,el_a->size)==0){
        return (void*)el_a;
    }
    return NULL;
}
void*
find(const SimpleMap* sm, const void* key, void*(cmp_fptr)(const void* a, const void* b)){
    int i = 0;
    while(i <= sm->top){
        const void* srch_el = cmp_fptr(sm->keys[i],key);
        if(srch_el){
            return srch_el;
        }
        i++;    
    }
    return NULL;
}
SimpleMap*
create_simple_map(void){
    SimpleMap* sm = (SimpleMap*)calloc(1,sizeof(SimpleMap));
    if(!sm){
        return NULL;
    }
    KeyBaseStruct** k_arr  = (KeyBaseStruct**)calloc(1,sizeof(KeyBaseStruct*));
    if(!k_arr){
        free(sm);
        return NULL;
    }
    ValueBaseStruct** v_arr = (ValueBaseStruct**)calloc(1,sizeof(ValueBaseStruct));
    if(!v_arr){
        free(sm);
        free(k_arr);
        return NULL;
    }
    sm->capacity = 1;
    sm->top      = -1;
    sm->keys     = k_arr;
    sm->values   = v_arr;
    return sm;
}