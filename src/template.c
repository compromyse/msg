#include <string.h>
#define _GNU_SOURCE

#include <filehandler.h>
#include <lexer.h>
#include <stdlib.h>
#include <template.h>
#include <util.h>

#include "../config.h"

template_t *
template_create(void)
{
  template_t *template = malloc(sizeof(template_t));

  FILE *base = fopen(DIRECTORY "/" BASE_TEMPLATE, "r");

  unsigned int size = fsize(base);
  char *buffer = fcontent(base, size);
  fclose(base);

  template->components = lex(buffer);

  free(buffer);
  return template;
}

void
template_delete(template_t *template)
{
  list_delete(template->components);
  free(template);
}

char *
template_ingest(template_t *template, list_t *content_headers, char *body)
{
  (void) body;
  char *output = malloc(1);
  strcpy(output, "");

  for (size_t i = 0; i < template->components->size; i++) {
    directive_t *match = list_get(template->components, i);

    switch (match->type) {
    case _RAW:
      xstrcat(output, match->operands);
      break;

    /* TODO: Handle this gracefully */
    default:
      break;
    }
  }

  return output;
}
