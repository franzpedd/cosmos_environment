#include "cren_utils.h"

#include "cren_platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Memory management
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void* crenmemory_allocate(unsigned long long size, int empty) {
    void* ptr = malloc(size);
    if(empty == 1 && ptr != NULL) memset(ptr, 0, size);

    return ptr;
}

void crenmemory_deallocate(void* ptr) {
    free(ptr);
}

void* crenmemory_reallocate(void* ptr, unsigned long long size) {
    return realloc(ptr, size);
}

void crenmemory_zero(void* ptr, unsigned long long size) {
    memset(ptr, 0, size);
}

void* crenmemory_copy(void* dest, void* src, unsigned long long size) {
    return memcpy(dest, &src, size);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Dynamic array
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CRenArray* crenarray_create(unsigned long long capacity) {
    CRenArray* arr = (CRenArray*)crenmemory_allocate(sizeof(CRenArray), 0);
    if (!arr) return NULL;

    capacity = (capacity == 0) ? 16 : capacity;
    arr->data = (void**)crenmemory_allocate(sizeof(void*) * capacity, 0);
    if (!arr->data) {
        crenmemory_deallocate(arr);
        return NULL;
    }

    arr->size = 0;
    arr->capacity = capacity;
    return arr;
}

void crenarray_destroy(CRenArray* arr) {
    if (arr) {
        crenmemory_deallocate(arr->data);
        crenmemory_deallocate(arr);
    }
}

int crenarray_resize(CRenArray* arr, unsigned long long newcapacity) {
    void** new_data = (void**)crenmemory_reallocate(arr->data, sizeof(void*) * newcapacity);
    if (!new_data) return 0;

    arr->data = new_data;
    arr->capacity = newcapacity;
    return 1;
}

int crenarray_push_back(CRenArray* arr, void* item) {
    if (arr->size >= arr->capacity) {
        unsigned long long new_capacity = arr->capacity * 2;
        if (!crenarray_resize(arr, new_capacity)) return 0;
    }

    arr->data[arr->size++] = item;
    return 1;
}

void* crenarray_pop_back(CRenArray* arr) {
    if (arr->size == 0) return NULL;
    return arr->data[--arr->size];
}

int crenarray_insert_at(CRenArray* arr, unsigned long long index, void* item) {
    if (index > arr->size) return 0; // out of bounds

    if (arr->size >= arr->capacity) {
        unsigned long long new_capacity = arr->capacity * 2;
        if (!crenarray_resize(arr, new_capacity)) return 0;
    }

    // shift elements to the right
    for (unsigned long long i = arr->size; i > index; i--) {
        arr->data[i] = arr->data[i - 1];
    }

    arr->data[index] = item;
    arr->size++;
    return 1;
}

void* crenarray_delete_from(CRenArray* arr, unsigned long long index) {
    if (index >= arr->size) return NULL; // out of bounds
    void* item = arr->data[arr->size];

    // shift elements to the left
    for (unsigned long long i = index; i < arr->size - 1; i++) {
        arr->data[i] = arr->data[i + 1];
    }

    arr->size--;
    return item;
}

unsigned long long crenarray_size(const CRenArray* arr) {
    return arr->size;
}

unsigned long long crenarray_capacity(const CRenArray* arr) {
    return arr->capacity;
}

void** crenarray_data(CRenArray* arr) {
    return arr->data;
}

void* crenarray_at(const CRenArray* array, unsigned long long index) {
    // check for NULL array or out-of-bounds index
    if (!array || index >= array->size) return NULL;
    return array->data[index];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Stacic Hashtable
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief hashes a given string using djb2 algorithm + additional mixing
/// @param str the initial string to be hashed
/// @return the index resulted by the hashing proccess
/// @details internal
static unsigned long cren_djb2_hash(const char *str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = (hash << 5) + hash + c;  // hash * 33 + c (original djb2)
        hash ^= (hash << 7) | (hash >> (sizeof(hash) * 8 - 7)); // additional mixing
    }

    return hash % CREN_HASHTABLE_MAXSIZE;
}

Hashtable* crenhashtable_create() {
    Hashtable* table = (Hashtable*)crenmemory_allocate(sizeof(Hashtable), 0);
    for (int i = 0; i < CREN_HASHTABLE_MAXSIZE; i++) {
        table->entries[i] = NULL;
    }
    return table;
}

void crenhashtable_destroy(Hashtable* table) {
    for (int i = 0; i < CREN_HASHTABLE_MAXSIZE; i++) {
        HashEntry* entry = table->entries[i];
        if (entry) {
            crenmemory_deallocate(entry->key);
            crenmemory_deallocate(entry);
        }
    }
    crenmemory_deallocate(table);
}

void crenhashtable_insert(Hashtable* table, const char* key, void* value) {
    unsigned long index = cren_djb2_hash(key);

    // Create a new entry
    HashEntry* entry = (HashEntry*)crenmemory_allocate(sizeof(HashEntry), 0);
    entry->key = cren_strdup((char*)key);
    entry->value = value;

    // Insert the entry into the table
    table->entries[index] = entry;
}

void* crenhashtable_lookup(Hashtable* table, const char* key) {
    unsigned long index = cren_djb2_hash(key);
    HashEntry* entry = table->entries[index];

    if (entry && cren_strcmp(entry->key, key) == 0) {
        return entry->value;
    }

    return NULL; // key not found
}

void crenhashtable_delete(Hashtable* table, const char* key) {
    unsigned long index = cren_djb2_hash(key);
    HashEntry* entry = table->entries[index];
    if (entry && cren_strcmp(entry->key, key) == 0) {
        crenmemory_deallocate(entry->key);
        crenmemory_deallocate(entry);
        table->entries[index] = NULL;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// General Utility
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int cren_strcmp(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

char* cren_strdup(char *src) {
    char *str;
    char *p;
    int len = 0;

    while (src[len]) {
        len++;
    }
    
    str = (char*)crenmemory_allocate(len + 1, 0);
    p = str;
    
    while (*src) {
        *p++ = *src++;
    }
    
    *p = '\0';
    return str;
}

char* cren_strncpy(char* dest, const char* src, unsigned long long size) {
    unsigned long long i;

    for (i = 0; i < size && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }

    for (; i < size; i++) {
        dest[i] = '\0';
    }

    return dest;
}


unsigned long long crenid_generate() {
    static uint64_t counter = 0;
    uint64_t new_id;

    cren_thread_lock();
    new_id = ++counter;
    cren_thread_unlock();

    return new_id;
}

const char *crenid_to_cstr(unsigned long long id, char *buffer, unsigned long long bufferSize)
{
    if (snprintf(buffer, bufferSize, "%llu", (unsigned long long)id) >= (int)bufferSize) {
        return NULL;
    }
    return buffer;
}
