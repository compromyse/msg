#define _GNU_SOURCE

#include <filehandler.h>
#include <lexer.h>
#include <stdio.h>
#include <stdlib.h>
#include <template.h>

#include "../config.h"

template_t *
template_create(void)
{
  template_t *template = malloc(sizeof(template_t));

  FILE *base = fopen(DIRECTORY "/" BASE_TEMPLATE, "r");

  unsigned int size = fsize(base);
  char *contents = fcontent(base, size);
  fclose(base);

  key_match_t *match = find_next_key(contents);
  asprintf(&template->pre, "%.*s", match->offset, contents);
  asprintf(&template->post,
           "%.*s",
           size - match->offset - match->length,
           contents + match->offset + match->length);

  free(contents);
  free(match);
  return template;
}
