#ifndef NLISP_UTIL_H
#define NLISP_UTIL_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// DATATYPE_LIST: list_node_t*, sizeof(list_node_t*)
// DATATYPE_ATOM: char*, strlen(), end with '\0'
#define DATATYPE_LIST "list"
#define DATATYPE_ATOM "atom"

typedef struct data_t {
    const char* type;
    void* ptr;
    size_t size;
} data_t;

void data_init(data_t* data);

void data_set(data_t* data, const char* type, void* ptr, size_t size);

int data_is_type(const data_t* data, const char* type);

int data_is_true(const data_t* data);

#endif // NLISP_UTIL_H