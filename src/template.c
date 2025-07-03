#define _GNU_SOURCE

#include <engine.h>
#include <filehandler.h>
#include <lexer.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <template.h>

#include "../config.h"

template_t *
template_create(void)
{
  template_t *template = malloc(sizeof(template_t));
  template->components = list_create(sizeof(directive_t));

  FILE *base = fopen(DIRECTORY "/" BASE_TEMPLATE, "r");

  unsigned int size = fsize(base);
  char *buffer = fcontent(base, size);
  fclose(base);

  key_match_t *match;
  while (true) {
    match = find_next_key(buffer);
    if (match == NULL)
      break;

    directive_t *directive = find_directive(buffer, match);
    if (directive == NULL) {
      printf(
          "Unknown directive: %.*s\n", match->length, buffer + match->offset);

      break;
    }

    switch (directive->type) {
    case CONTENT:
    case BODY:
      break;

    /* TODO: Handle this gracefully */
    case INCLUDE:
    case CONTENTFOR:
    case ENDCONTENT:
      break;
    }

    if (directive != NULL)
      free(directive);

    if (match != NULL)
      free(match);
  }

  return template;
}
