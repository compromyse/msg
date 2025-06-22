#define _GNU_SOURCE

#include <engine.h>
#include <filehandler.h>
#include <lexer.h>
#include <list.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../config.h"

void
ingest(char **buffer)
{
  key_match_t *match;
  list_t *content_headers = list_create(sizeof(contentfor_operand_t));
  if (content_headers == NULL) {
    printf("Could not create content_headers\n");
    return;
  }

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
      char *operand = directive->operands;
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
      contentfor_operand_t *operand = directive->operands;
      list_add(content_headers, operand);

      /* printf("CONTENTFOR: %s\n", operand->key); */
      /* printf("CONTENT: %s\n", operand->content); */

      /* printf("CONTENT: %.*s\n", operand->length, *buffer + match->offset);
       */

      char *temp_buffer;
      asprintf(&temp_buffer, "%s", *buffer);

      free(*buffer);
      asprintf(buffer,
               "%.*s%s",
               match->offset,
               temp_buffer,
               temp_buffer + operand->length);

      free(temp_buffer);
      /* free(operand); */
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

  for (size_t i = 0; i < content_headers->size; i++) {
    contentfor_operand_t *op = list_get(content_headers, i);
    free(op->content);
    free(op->key);
  }

  list_delete(content_headers);
}
