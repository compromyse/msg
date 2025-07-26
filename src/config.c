#define _GNU_SOURCE

#include <config.h>
#include <list.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util.h>

void
config_parse(char *content)
{
  list_t *keys = list_create(sizeof(char *));
  list_t *values = list_create(sizeof(char *));
  list_t *array_values = list_create(sizeof(list_t));

  char *buffer = strdup(content);
  /* For free() */
  char *x = buffer;

  char *key = trim(strsep(&buffer, DELIM));

  while (buffer != NULL) {
    buffer = ltrim(buffer);

    list_add(keys, strdup(key));

    if (*buffer == '{') {
      buffer++;
      list_t *l = list_create(sizeof(char *));
      char *raw_array = remove_spaces(strsep(&buffer, "}"));

      char *value = strsep(&raw_array, DELIM_ARRAY);
      while (value != NULL) {
        list_add(l, strdup(value));
        value = strsep(&raw_array, DELIM_ARRAY);
      }

      list_add(array_values, l);
      list_add(values, "\0");
    } else {
      char *value = trim(strsep(&buffer, "\n"));

      list_add(array_values, &((list_t) { 0 }));
      list_add(values, value);
    }

    key = trim(strsep(&buffer, DELIM));
  }

  for (size_t i = 0; i < keys->size; i++) {
    char *key = list_get(keys, i);
    char *value = list_get(values, i);

    if (strcmp(value, "\0") == 0) {
      printf("%s ARRAY with VALUES ", key);
      list_t *xvalues = list_get(array_values, i);

      for (size_t y = 0; y < xvalues->size; y++) {
        char *value = list_get(xvalues, y);
        printf("%s ", value);
      }

      printf("\n");
    } else {
      printf("%s: %s\n", key, value);
    }
  }

  free(x);
}
