#ifndef __TEMPLATE_H
#define __TEMPLATE_H

#include <list.h>
#include <stdio.h>

typedef struct {
  list_t *components;
} template_t;

template_t *template_create(void);
void template_delete(template_t *template);

void template_write(template_t *template, list_t *content_headers, FILE *f);

#endif
