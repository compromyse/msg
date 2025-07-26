#define _GNU_SOURCE

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util.h>

void
config_parse(char *content)
{
  char *buffer = strdup(content);

  char *line_internal = strtok(buffer, "\n");
  while (line_internal != NULL) {
    char *line = strdup(line_internal);
    char *key = strsep(&line, "=");
    line = trim(line);
    key = trim(key);

    printf("Key: %s\n", key);
    printf("Value: %s\n", line);

    line_internal = strtok(NULL, "\n");
  }

  free(buffer);
}
