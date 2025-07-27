#define _GNU_SOURCE

#include <copy.h>
#include <engine.h>
#include <filehandler.h>
#include <lexer.h>
#include <list.h>
#include <main.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern msg_t *msg;

void
handle_include(char **buffer, key_match_t *match, directive_t *directive)
{
  char *operand = directive->operands;
  char *partial_path;
  asprintf(&partial_path, "%s/%s/%s", msg->base_directory, PARTIALS, operand);

  FILE *f = fopen(partial_path, "r");
  if (f == NULL) {
    printf("Could not open: %s\n", partial_path);
    return;
  }

  unsigned int size = fsize(f);
  char *partial_content = fcontent(f, size);

  char *temp_buffer = strdup(*buffer);

  free(*buffer);
  asprintf(buffer,
           "%.*s%s%s\n",
           match->offset,
           temp_buffer,
           partial_content,
           temp_buffer + match->offset + match->length);

  free(temp_buffer);
}

void
handle_contentfor(char **buffer,
                  key_match_t *match,
                  directive_t *directive,
                  list_t *content_headers)
{
  contentfor_operand_t *operand = directive->operands;
  list_add(content_headers, operand);

#ifdef DEBUG
  printf("CONTENTFOR: %s\n", operand->key);
  printf("CONTENT: %s\n", operand->content);
#endif

  char *temp_buffer = strdup(*buffer);

  free(*buffer);
  asprintf(buffer,
           "%.*s%s",
           match->offset,
           temp_buffer,
           temp_buffer + operand->length);

  free(temp_buffer);
  free(operand);
}

list_t *
ingest(char **buffer)
{
  key_match_t *match;
  list_t *content_headers = list_create(sizeof(contentfor_operand_t));
  if (content_headers == NULL) {
    printf("Could not create content_headers\n");
    return NULL;
  }

  size_t skip = 0;
  while (true) {
    match = find_next_key(*buffer, skip);
    if (match == NULL)
      break;

#ifdef DEBUG
    printf("Match: %.*s LENGTH(%d) OFFSET(%d)\n",
           match->length,
           *buffer + match->offset,
           match->length,
           match->offset);
#endif

    directive_t *directive = find_directive(*buffer, match);
    if (directive == NULL) {
      printf(
          "Unknown directive: %.*s\n", match->length, *buffer + match->offset);

      break;
    }

    switch (directive->type) {
    case INCLUDE:
      handle_include(buffer, match, directive);
      break;
    case CONTENTFOR:
      handle_contentfor(buffer, match, directive, content_headers);
      break;

    case BODY:
    case CONTENT:
    case ENDCONTENT:
    case _RAW:
      skip++;
      break;
    }

    if (directive != NULL)
      free(directive);

    if (match != NULL)
      free(match);
  }

  return content_headers;
}
