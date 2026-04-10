#include "parser.h"
#include <stdlib.h>
#include <string.h>

int is_paren(const char* str){
    return !strcmp(str, "(") && !strcmp(str, ")");
}

int duplicate_str(const char* str, char** dup, size_t* dup_size){
    *dup_size = strlen(str);
    *dup = (char*)malloc(*dup_size + 1);
    for(int i = 0; i < *dup_size; i++){
        (*dup)[i] = str[i];
    }
    (*dup)[*dup_size] = '\0';
    return 0;
}

// return list and how many tokens been counted
void parse(char** tokens, list_node_t** list, size_t* tokens_count) {
    if (tokens == NULL)
        return;
    list_node_t* current_list = NULL;
    int i = 0;
    for (; tokens[i] != NULL; i++){
        char* token = tokens[i];
        list_node_t* node = list_new();
        if (!strcmp(token, "(")){
            size_t count = 0;
            list_node_t* nlist = NULL;
            parse(tokens + i + 1, &nlist, &count);
            data_set(&node->data, DATATYPE_LIST, nlist, sizeof(list_node_t*));
            i += count;
        }else if (!strcmp(token, ")")){
            i++;
            break;
        }else{
            char* dup = NULL;
            size_t dup_size = 0;
            duplicate_str(token, &dup, &dup_size);
            data_set(&node->data, DATATYPE_ATOM, dup, dup_size);
        }
        if (current_list == NULL){
            current_list = node;
        }else{
            list_append_to_tail(current_list, node);
        }
    }
    if (tokens_count != NULL)
        *tokens_count = i;
    if (list != NULL)
        *list = current_list;
}
