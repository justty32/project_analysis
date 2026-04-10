#include "list.h"
#include <stdlib.h>
#include <string.h>

// set list's attrs to NULL & 0
// doesn't do free()
void list_init(list_node_t* list) {
    if (list == NULL) return;
    list->prev = NULL;
    list->next = NULL;
    data_init(&list->data);
}

// return a initialized list
list_node_t* list_new() {
    list_node_t* node = (list_node_t*)malloc(sizeof(list_node_t));
    if (node == NULL) return NULL;
    list_init(node);
    return node;
}

// free list from head, if success, return 0
int list_free(list_node_t* list) {
    if (list == NULL) return 0;
    if (list->prev != NULL){
        list->prev->next = NULL;
    }
    list_node_t* head = list;
    while (head != NULL) {
        list_node_t* next = head->next;
        if (data_is_type(&head->data, "list")) {
            list_free((list_node_t*)head->data.ptr);
            data_init(&head->data);
        }
        if (head->data.ptr != NULL) {
            free(head->data.ptr);
        }
        free(head);
        head = next;
    }
    return 0;
}

// get list length
int list_get_length(list_node_t* list) {
    if (list == NULL) return 0;
    list_node_t* head = list;
    int count = 0;
    while (head != NULL) {
        count++;
        head = head->next;
    }
    return count;
}

// get list node by index
list_node_t* list_get_node(list_node_t* list, int index) {
    if (list == NULL || index < 0) return NULL;
    list_node_t* curr = list;
    int i = 0;
    while (curr != NULL) {
        if (i == index) return curr;
        curr = curr->next;
        i++;
    }
    return NULL;
}

// get head of list
list_node_t* list_get_head(list_node_t* list) {
    if (list == NULL) return NULL;
    list_node_t* curr = list;
    while (curr->prev != NULL) {
        curr = curr->prev;
    }
    return curr;
}

// get tail of list
list_node_t* list_get_tail(list_node_t* list) {
    if (list == NULL) return NULL;
    list_node_t* curr = list;
    while (curr->next != NULL) {
        curr = curr->next;
    }
    return curr;
}

// insert node between list and list's next.
// if node's next != NULL, free(node->next)
int list_insert(list_node_t* list, list_node_t* node) {
    if (list == NULL || node == NULL) return -1;
    if (list->next != NULL){
        if (node->next != NULL){
            list_free(node->next);
        }
        list->next->prev = node;
        node->next = list->next;
    }
    list->next = node;
    node->prev = list;
    return 0;
}

int list_append_to_tail(list_node_t* list, list_node_t* node) {
    list_node_t* tail = list_get_tail(list);
    if (tail != NULL){
        tail->next = node;
        node->prev = tail;
    }
    return 0;
}

// start from head
// will return while func return != 0
int list_foreach(list_node_t* list, int (*func)(list_node_t* node, int index)){
    if (list == NULL || func == NULL) return -1;
    list_node_t* head = list;
    int i = 0;
    while (head != NULL) {
        if (func(head, i) != 0) return -1;
        head = head->next;
        i++;
    }
    return 0;
}
