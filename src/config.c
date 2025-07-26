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

  char *buffer = strdup(content);

  char *line_internal = strtok(buffer, "\n");
  while (line_internal != NULL) {
    char *line = strdup(line_internal);

    char *value = line;
    char *key = strsep(&value, "=");
    value = trim(value);
    key = trim(key);

    list_add(keys, strdup(key));
    list_add(values, strdup(value));

    free(line);
    line_internal = strtok(NULL, "\n");
  }

  for (size_t i = 0; i < keys->size; i++) {
    printf(
        "%s: %s\n", (char *) list_get(keys, i), (char *) list_get(values, i));
  }

  free(buffer);
}
