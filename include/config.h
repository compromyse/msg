#ifndef __CONFIG_H
#define __CONFIG_H

#include <list.h>

#define DELIM "="
#define DELIM_ARRAY ","

typedef struct {
  list_t *keys;
  list_t *values;
  list_t *array_values;
} config_t;

config_t *config_parse(char *content);

#endif
