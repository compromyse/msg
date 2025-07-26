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
    char *value = NULL;
    buffer = ltrim(buffer);

    if (*buffer == '{') {
      buffer++;
      value = trim(strsep(&buffer, "}"));
      remove_spaces(value);
    } else {
      value = trim(strsep(&buffer, "\n"));
    }

    printf("KEY %s\n", key);
    printf("VALUE %s\n", value);

    key = trim(strsep(&buffer, DELIM));
  }

  /* free(x); */
}
