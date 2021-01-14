#pragma once

typedef struct list_node_t {
    void *value;
    struct list_node_t *next;
} list_node_t;

typedef struct {
    list_node_t *head;
    list_node_t *tail;
} list_t;

typedef void (*fptr_free)(void*);

list_t* list_init(void);
void list_add_end(list_t *list, void *value);
void list_free(list_t *list, fptr_free f);

#define LIST_FOREACH(list, current) for(list_node_t *current = list->head; current; current = current->next)
