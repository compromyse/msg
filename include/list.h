#ifndef __LIST_H
#define __LIST_H

#include <stddef.h>
#include <stdint.h>

#define START_SIZE 10
#define INCREMENT_BY 5

typedef struct {
  size_t element_size;
  size_t max;
  size_t size;
  uint8_t *elements;
} list_t;

list_t *list_create(size_t element_size);
void list_add(list_t *list, void *element);
void *list_get(list_t *list, size_t i);
void list_delete(list_t *list);

#endif
