#include <stdlib.h>
#include "../include/simple_map.h"
void* remove_key(SimpleMap* sm,
                 void* key,
                 void*(cmp_fptr)(const void* a, const void* b),
                 KeyValuePair* rmv_pair);


void*
remove_key(SimpleMap* sm,
           void* key,
           void*(cmp_fptr)(const void* a, const void* b),
           KeyValuePair* rmv_pair){
    int i = 0;
    void* rmv_key = NULL, *rmv_value = NULL; 
    while(i<=sm->top){
        rmv_key = sm->keys[i]->key;
        if(!cmp_fptr(key,rmv_key)){
            i++;
            continue;
        };
        rmv_value = sm->values[i]->value;
        while(1){
            if(i==sm->top){
                sm->keys[i] = NULL;
                sm->values[i] = NULL;
                sm->top--;
                rmv_pair->key   = rmv_key;
                rmv_pair->value = rmv_value;
                return rmv_key;
            }
            sm->keys[i] = sm->keys[i+1];
            sm->values[i] = sm->values[i+1];
            i++;
        }
    }
    return NULL;
}
/**
 * 
void* 
compare(const void* a, const void* b){
    const KeyNode* el_a     = (const KeyNode*)a;
    const KeyNode* el_b     = (const KeyNode*)b;
    if(!el_a || !el_b){
        return NULL;
    }
    if(el_a->size!=el_b->size){
            return NULL;
        }
    if(memcmp(el_a->content, el_b->content,el_a->size)==0){
        return (void*)el_a;
    }
    return NULL;
}
 * 
 */

unsigned char is_full(SimpleMap* sm){
    if(sm->top+1==sm->capacity){
        return 1;
    }
    return 0;
}
SimpleMap*
double_arrays(SimpleMap* sm){
    if(!sm || !sm->keys || !sm->values){
        return NULL;
    }
    if(!sm->capacity){
        return sm;
    }
    KeyWrapper**   keys   = NULL;
    ValueWrapper** values = NULL;
    size_t new_capacity = 2 * sm->capacity;
    keys     = (KeyWrapper**)realloc(sm->keys,new_capacity*sizeof(KeyWrapper*));
    if(!keys){
        return NULL;
    }
    values   = (ValueWrapper**)realloc(sm->values,new_capacity*sizeof(ValueWrapper*));
    if(!values){
        sm->keys = realloc(keys, sm->capacity * sizeof(KeyWrapper*));
        return NULL;
    }
    sm->keys   = keys;
    sm->values = values;
    sm->capacity = new_capacity;
    return sm;
}

void*
set(SimpleMap* sm, KeyValuePair* key_par){
    if(!sm           || !key_par    || 
       !sm->keys     || !sm->values || 
       !key_par->key){
        return NULL;
    }
    KeyWrapper*   key   = (KeyWrapper*)calloc(1,sizeof(KeyWrapper));
    ValueWrapper* value = (ValueWrapper*)calloc(1,sizeof(ValueWrapper));
    key->key            = key_par->key;
    value->value        = key_par->value;
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


const void*
find(const SimpleMap* sm, const void* key, void*(cmp_fptr)(const void* a, const void* b)){
    int i = 0;
    if(!sm || !key || !cmp_fptr ||
       !sm->keys){
        return NULL;
       }
    while(i <= sm->top){
        const void* srch_el = cmp_fptr(sm->keys[i]->key,key);
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
    KeyWrapper** k_arr  = (KeyWrapper**)calloc(1,sizeof(KeyWrapper*));
    if(!k_arr){
        free(sm);
        return NULL;
    }
    ValueWrapper** v_arr = (ValueWrapper**)calloc(1,sizeof(ValueWrapper*));
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
};

int
delete_map(SimpleMap* sm, void* (*clean_up)(void*, void*)){
    if(!sm || !sm->keys || !sm->values ){
        return 0;
    }
    int i = 0;
    while(i <= sm->top){
        if(clean_up && sm->keys[i] && sm->values[i]){
            clean_up(sm->keys[i]->key,sm->values[i]->value);
        }
        if(sm->keys[i]){
            free(sm->keys[i]);
        }
        if(sm->values[i]){
            free(sm->values[i]);
        }
        i++;
    }
    free(sm->keys);
    free(sm->values);
    free(sm);
    return 1;
}