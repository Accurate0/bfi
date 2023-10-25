#include <stdio.h>
#include <stdlib.h>

#include "list.h"

list_t *list_init(void) {
  list_t *ret = malloc(sizeof(list_t));
  ret->head = NULL;
  ret->tail = NULL;
  ret->count = 0;

  return ret;
}

void list_add_end(list_t *list, void *value) {
  list_node_t *node = malloc(sizeof(list_node_t));
  node->next = NULL;
  node->value = value;

  if (list->tail == NULL) {
    list->head = node;
    list->tail = node;
  } else {
    list->tail->next = node;
    list->tail = node;
  }

  list->count++;
}

void list_free(list_t *list, fptr_free f) {
  list_node_t *c = list->head;
  while (c) {
    list_node_t *next = c->next;
    f(c->value);
    free(c);
    c = next;
  }

  free(list);
}
