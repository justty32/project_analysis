#ifndef NLISP_PARSER_H
#define NLISP_PARSER_H

#include "list.h"
#include "stack.h"

void parse(char** tokens, list_node_t** list, size_t* tokens_count);

#endif // NLISP_PARSER_H