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
  /* For free() */
  char *x = buffer;

  char *key = trim(strsep(&buffer, DELIM));

  while (buffer != NULL) {
    buffer = ltrim(buffer);

    list_add(keys, strdup(key));

    if (*buffer == '{') {
      buffer++;
      list_add(values, "\0");

      char *raw_array = remove_spaces(strsep(&buffer, "}"));
    } else {
      char *value = trim(strsep(&buffer, "\n"));
    }

    printf("KEY %s\n", key);

    key = trim(strsep(&buffer, DELIM));
  }

  /* free(x); */
}
