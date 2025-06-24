#ifndef __TEMPLATE_H
#define __TEMPLATE_H

typedef struct {
  char *pre;
  char *post;
} template_t;

template_t *template_create(void);

#endif
