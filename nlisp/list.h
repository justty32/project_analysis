#ifndef NLISP_LIST_H
#define NLISP_LIST_H

#include "util.h"

typedef struct list_node_t list_node_t;

struct list_node_t {
    list_node_t* prev, *next;
    data_t data;
};

// set list's attrs to NULL & 0
// doesn't do free()
void list_init(list_node_t* list);

// return a initialized list
list_node_t* list_new();

// free list, if success, return 0
int list_free(list_node_t* list);

// get list length from head
int list_get_length(list_node_t* list);

// get list node by index from head
list_node_t* list_get_node(list_node_t* list, int index);

// get head of list
list_node_t* list_get_head(list_node_t* list);

// get tail of list
list_node_t* list_get_tail(list_node_t* list);

// add node to list
int list_insert(list_node_t* list, list_node_t* node);

// add node to tail of list
int list_append_to_tail(list_node_t* list, list_node_t* node);

// remove node
int list_remove_node(list_node_t* list, list_node_t* node);

// start from head
// will return while func return != 0
int list_foreach(list_node_t* list, int (*func)(list_node_t* node, int index));

#endif // NLISP_LIST_H