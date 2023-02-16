#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>


typedef struct{
    uint32_t size;
    uint32_t keysize;
    uint32_t used;
    uint8_t* data;
}hash_table;

uint8_t zer[100000]={0};

hash_table* hash_table_init(uint32_t tablesize, uint32_t keysize){
    hash_table* h= malloc(sizeof(hash_table));
    *h=(hash_table){.size=tablesize,.keysize=keysize,.used=0,.data=calloc(tablesize,keysize)};
    return h;
}

uint64_t hash_function(const unsigned char *key,uint32_t keysize){
    // FNV-1a hash function
    uint64_t hash = 14695981039346656037ull;
    int i;
    for (i = 0; i < keysize; i++){
        hash ^= (uint64_t)key[i];
        hash *= 1099511628211ull;
    }
    return hash;
}
bool upsert(hash_table *table, unsigned char *key) {
    uint64_t hash = hash_function(key,table->keysize);
    int i = 0;
    int index = (int)(hash%table->size);
    while (memcmp(table->data+(table->keysize*index), key, table->keysize) != 0) {
        if (memcmp(table->data+(table->keysize*index), zer, table->keysize) == 0) {
            if(table->used<(table->size*3/4)){
                memcpy(table->data+(table->keysize*index), key, table->keysize);
                table->used++;
            }

            return false;
        }
        i++;
        index = (int)((hash + i*i) % table->size);
    }
    return true;
}