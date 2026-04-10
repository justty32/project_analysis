#include "stack.h"

char* stack_push(stack_t* stack, void* data, size_t size){
    if (stack->top + size > stack->cap){
        return NULL;
    }
    for(size_t i = 0; i < size; i++) {
        stack->ptr[stack->top + i] = ((char*)data)[i];
    }
    stack->top += size;
    return stack->ptr + stack->top - size;
}

char* stack_push_str(stack_t* stack, const char* str){
    char* read = (char*)str;
    size_t original_top = stack->top;
    while(*read != '\0'){
        if (stack->top == stack->cap){
            stack->top = original_top;
            return NULL;
        }
        stack->ptr[stack->top] = *read;
        stack->top++;
        read++;
    }
    return stack->ptr + original_top;
}

int stack_pop(stack_t* stack, void* data_dst, size_t size){
    if (stack->top - size < 0){
        return -1;
    }
    for(size_t i = 0; i < size; i++) {
        ((char*)data_dst)[i] = stack->ptr[stack->top - size + i];
    }
    stack->top -= size;
    return 0;
}