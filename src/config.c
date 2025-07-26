#define _GNU_SOURCE

#include <config.h>
#include <list.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util.h>

config_t *
config_parse(char *content)
{
  list_t *keys = list_create(sizeof(ptr_wrapper_t));
  list_t *values = list_create(sizeof(ptr_wrapper_t));
  list_t *array_values = list_create(sizeof(list_t));

  ptr_wrapper_t *wrapper;

  char *buffer = strdup(content);
  /* For free() */
  char *x = buffer;

  char *key = trim(strsep(&buffer, DELIM));

  while (buffer != NULL) {
    buffer = ltrim(buffer);

    wrapper = malloc(sizeof(ptr_wrapper_t));
    wrapper->ptr = strdup(key);
    list_add(keys, wrapper);

    if (*buffer == '{') {
      buffer++;
      list_t *l = list_create(sizeof(ptr_wrapper_t));
      char *raw_array = strsep(&buffer, "}");

      char *value = strsep(&raw_array, DELIM_ARRAY);
      while (value != NULL) {
        wrapper = malloc(sizeof(ptr_wrapper_t));
        wrapper->ptr = strdup(trim(value));
        list_add(l, wrapper);

        value = strsep(&raw_array, DELIM_ARRAY);
      }

      list_add(array_values, l);

      wrapper = malloc(sizeof(ptr_wrapper_t));
      wrapper->ptr = NULL;
      list_add(values, wrapper);
    } else {
      char *value = trim(strsep(&buffer, "\n"));

      wrapper = malloc(sizeof(ptr_wrapper_t));
      wrapper->ptr = NULL;
      list_add(array_values, wrapper);

      wrapper = malloc(sizeof(ptr_wrapper_t));
      wrapper->ptr = strdup(value);
      list_add(values, wrapper);
    }

    key = trim(strsep(&buffer, DELIM));
  }

  free(x);

  config_t *config = malloc(sizeof(config_t));
  config->keys = keys;
  config->values = values;
  config->array_values = array_values;
  return config;
}
