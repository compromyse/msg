#define _GNU_SOURCE

#include <engine.h>
#include <filehandler.h>
#include <lexer.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../config.h"

void
ingest(char **buffer)
{
  key_match_t *match;

  while (true) {
    match = find_next_key(*buffer);
    if (match == NULL)
      break;

    directive_t *directive = find_directive(*buffer, match);
    if (directive == NULL)
      break;

    if (directive->type == INCLUDE) {
      char *operand = (char *) directive->operands;
      char *partial_path;
      asprintf(&partial_path, "%s/%s/%s", DIRECTORY, PARTIALS, operand);

      FILE *f = fopen(partial_path, "r");
      unsigned int size = fsize(f);
      char *partial_content = fcontent(f, size);

      char *temp_buffer;
      asprintf(&temp_buffer, "%s", *buffer);

      free(*buffer);
      asprintf(buffer,
               "%.*s%s%s\n",
               match->offset,
               temp_buffer,
               partial_content,
               temp_buffer + match->offset + match->length);

      free(temp_buffer);
    }

    free(directive);
    free(match);
  }
}
