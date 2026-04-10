#ifndef NLISP_STACK_H
#define NLISP_STACK_H

#include "util.h"

typedef struct stack_t {
    char* ptr;
    size_t top;
    size_t cap;
} stack_t;

// if success, return pushed data pos in stack
char* stack_push(stack_t* stack, void* data, size_t size);

// str must end with '\0'
char* stack_push_str(stack_t* stack, const char* str);

// if success, return 0
int stack_pop(stack_t* stack, void* data_dst, size_t size);

#endif // NLISP_STACK_H