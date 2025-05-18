#ifndef CREN_UTILS_INCLUDED
#define CREN_UTILS_INCLUDED

#include "cren_defines.h"

// functions
#ifdef __cplusplus 
extern "C" {
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Memory management
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief allocates memory using default malloc, this is usefull if allocating functions are to be modified
/// @param size how many bytes to be allocated
/// @param empty erases all contents within specified size
/// @return the memory's address
void* crenmemory_allocate(unsigned long long size, int empty);

/// @brief dealocates memory previously allocated
/// @param ptr address to the memory's block
void crenmemory_deallocate(void* ptr);

/// @brief reallocates previouly allocated memory to fit a new size
/// @param ptr address to the memory
/// @param size new size of the memory's block
/// @return the new ptr of the memory
void* crenmemory_reallocate(void* ptr, unsigned long long size);

/// @brief erases all content exists in the given address
/// @param ptr memory's address
/// @param size how many bytes to be erased
void crenmemory_zero(void* ptr, unsigned long long size);

/// @brief copies a block of memory to a new address
/// @param dest where the memory will be moved to
/// @param src where memory previously resided
/// @param size how many bytes the memory occupies
/// @return dest's address
void* crenmemory_copy(void* dest, const void* src, unsigned long long size);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Dynamic array
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct CRenArray
{
	void** data;
	unsigned long long size;
	unsigned long long capacity;
} CRenArray;

/// @brief creates an array of data, the data should be managed outside the array struct/functions
/// @param capacity initial capacity of the array, altough it is dynamic
/// @return returns an array or NULL if couldn't be created
CREN_API CRenArray* crenarray_create(unsigned long long capacity);

/// @brief destroys an array, but not it's contents
/// @param arr the array to be destroyed
CREN_API void crenarray_destroy(CRenArray* arr);

/// @brief resizes the array
/// @param arr array to be resized
/// @param newCapacity new size of the array
/// @return 1 on success, 0 on failure
CREN_API int crenarray_resize(CRenArray* arr, unsigned long long newcapacity);

/// @brief pushes a new item at the back of the array
/// @param arr array the item will be added to
/// @param item the item to be added
/// @return 1 on success, 0 on failure
CREN_API int crenarray_push_back(CRenArray* arr, void* item);

/// @brief removes an item from the array's end
/// @param arr the array the item will be removed
/// @return the address to the removed item
CREN_API void* crenarray_pop_back(CRenArray* arr);

/// @brief inserts a new item into a given position index within the array
/// @param arr the array to add an item to
/// @param index the position to be added
/// @param item the item to be added
/// @return 0 on failure, 1 on success
CREN_API int crenarray_insert_at(CRenArray* arr, unsigned long long index, void* item);

/// @brief deletes an item from a given position
/// @param arr the array the the item will be removed
/// @param index the index to remove the item from
/// @return the removed item address or NULL if out of bounds
CREN_API void* crenarray_delete_from(CRenArray* arr, unsigned long long index);

/// @brief returns the array's current size
/// @param arr the array itself
/// @return the array's size
CREN_API unsigned long long crenarray_size(const CRenArray* arr);

/// @brief returns the array's current capacity
/// @param arr the array itself
/// @return the array's capacity
CREN_API unsigned long long crenarray_capacity(const CRenArray* arr);

/// @brief function alike std::vector::data
/// @param arr the array itself
/// @return a ptr-to-ptrs of all items within the array
CREN_API void** crenarray_data(CRenArray* arr);

/// @brief returns the address to a given item at the given index
/// @param array the array itself
/// @param index the position desired
/// @return the address of the given position or NULL if position is invalid
CREN_API void* crenarray_at(const CRenArray* array, unsigned long long index);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Stacic Hashtable
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CREN_HASHTABLE_MAXSIZE 127

typedef struct HashEntry 
{
    char* key;
    void* value;
} HashEntry;

typedef struct Hashtable
{
    HashEntry* entries[CREN_HASHTABLE_MAXSIZE]; // prime numbers are excellent for hashing
} Hashtable;

/// @brief creates an emtpy hashtable
/// @return the hashtable address
Hashtable* crenhashtable_create();

/// @brief destroys the hashtable
/// @param table the table to be destroyed
void crenhashtable_destroy(Hashtable* table);

/// @brief inserts an item into the hashtable
/// @param table the table the item will be inserted into
/// @param key unique identifier for the item within the table
/// @param value the item itself
void crenhashtable_insert(Hashtable* table, const char* key, void* value);

/// @brief returns the item the unique key points to
/// @param table the table to look the item
/// @param key unique identifier within the table
/// @return the item or NULL if not found
void* crenhashtable_lookup(Hashtable* table, const char* key);

/// @brief removes an item from the hashtable
/// @param table the table the item will be removed from
/// @param key the unique identifier within the table for the item
void crenhashtable_delete(Hashtable* table, const char* key);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// General Utility
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief compares a string with another
/// @param str1 first string
/// @param str2 second string
/// @return Negative (str1 appears before str2 in lexicographical order), Positive (str1 appears after str2 in lexicographical order). 0 (Equal strings).
int cren_strcmp(const char* str1, const char* str2);

/// @brief dupplicates a string (must be freed latter)
/// @param src the string to be dupplicated
/// @return the dupplicated string address
char* cren_strdup(char *src);

/// @brief copies a string into another string
/// @param dest destiny string
/// @param src source string
/// @param size size of the  
/// @return the dest address
char* cren_strncpy(char* dest, const char* src, unsigned long long size);

/// @brief generates an (most-likely) unique id
/// @return the generated id
unsigned long long crenid_generate();

/// @brief stringfies the id
/// @param id the id to stringfy
/// @param buffer input string buffer to fill with the stringfied id
/// @param bufferSize input string buffer's max size
/// @return the stringfied id
const char* crenid_to_cstr(unsigned long long id, char* buffer, unsigned long long bufferSize);

#ifdef __cplusplus 
}
#endif

#endif // CREN_UTILS_INCLUDED