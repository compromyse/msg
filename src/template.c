#define _GNU_SOURCE

#include <filehandler.h>
#include <lexer.h>
#include <stdio.h>
#include <stdlib.h>
#include <template.h>

#include "../config.h"

void
template_initialize(char **base_pre, char **base_post)
{
  FILE *base = fopen(DIRECTORY "/" BASE_TEMPLATE, "r");

  unsigned int size = fsize(base);
  char *contents = fcontent(base, size);

  key_match_t *match = find_next_key(contents);
  asprintf(base_pre, "%.*s", match->offset, contents);
  asprintf(base_post,
           "%.*s",
           size - match->offset - match->length,
           contents + match->offset + match->length);

  free(contents);
  fclose(base);
}
