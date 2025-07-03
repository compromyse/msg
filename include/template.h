#ifndef __TEMPLATE_H
#define __TEMPLATE_H

#include <list.h>

typedef struct {
  list_t *components;
} template_t;

template_t *template_create(void);

#endif
