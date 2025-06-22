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
    if (directive == NULL) {
      printf(
          "Unknown directive: %.*s\n", match->length, *buffer + match->offset);

      break;
    }

    switch (directive->type) {
    case INCLUDE: {
      char *operand = (char *) directive->operands;
      char *partial_path;
      asprintf(&partial_path, "%s/%s/%s", DIRECTORY, PARTIALS, operand);

      FILE *f = fopen(partial_path, "r");
      if (f == NULL) {
        printf("Could not open: %s\n", partial_path);
        return;
      }

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
      break;
    }
    case CONTENTFOR: {
      contentfor_operands_t *operand
          = (contentfor_operands_t *) directive->operands;
      printf("CONTENTFOR: %s\n", operand->key);
      printf("CONTENT: %s\n", operand->content);

      return;
      break;
    }

    /* NOTE: This will never occur */
    case ENDCONTENT:
      break;
    }

    if (directive != NULL)
      free(directive);

    if (match != NULL)
      free(match);
  }
}
