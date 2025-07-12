#define _GNU_SOURCE

#include <ctype.h>
#include <lexer.h>
#include <list.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

list_t *
lex(char *buffer)
{
  list_t *directives = list_create(sizeof(directive_t));
  size_t current_offset = 0;

  while (true) {
    key_match_t *key = find_next_key(buffer, 0);
    if (key == NULL)
      break;

    directive_t *directive = find_directive(buffer, key);
    /* TODO: Handle unknown directive */
    if (directive == NULL)
      break;

    current_offset += key->length + key->offset;

    if (current_offset != 0) {
      char *raw_content;
      asprintf(&raw_content, "%.*s", (int) key->offset, buffer);

      directive_t *raw_directive = malloc(sizeof(directive_t));
      raw_directive->type = _RAW;
      raw_directive->operands = raw_content;
      list_add(directives, raw_directive);
    }

    buffer += key->offset + key->length;

    list_add(directives, directive);
  }

  if (strlen(buffer) > 0) {
    char *raw_content;
    asprintf(&raw_content, "%s", buffer);

    directive_t *raw_directive = malloc(sizeof(directive_t));
    raw_directive->type = _RAW;
    raw_directive->operands = raw_content;
    list_add(directives, raw_directive);
  }

  return directives;
}

key_match_t *
find_next_key(char *buffer, size_t skip)
{
  key_match_t *match = calloc(1, sizeof(key_match_t));
  size_t count = 0;

  for (size_t i = 0; i < strlen(buffer); i++) {
    if (buffer[i] == '{' && buffer[i + 1] == '{') {
      count++;

      if (count > skip) {
        match->offset = i;
        break;
      }
    }

    if (i == strlen(buffer) - 1) {
      free(match);
      return NULL;
    }
  }

  char *subbuffer = buffer + match->offset;
  for (size_t i = 0; i < strlen(subbuffer); i++) {
    if (subbuffer[i] == '}' && subbuffer[i + 1] == '}') {
      match->length = i + 2;
      break;
    }

    if (i == strlen(buffer) - 1) {
      printf("Unterminated Key\n");
      free(match);
      return NULL;
    }
  }

  return match;
}

directive_t *
find_directive(char *content, key_match_t *match)
{
  directive_t *directive = (directive_t *) calloc(1, sizeof(directive_t));

  char *buffer = content + match->offset;
  unsigned int n = 0;

  for (size_t i = 0; i < match->length; i++)
    switch (buffer[i]) {
    case '{':
    case ' ':
    case '\t':
    case '\n':
      n++;
      break;

    default:
      goto found_start;
    }

  return NULL;

found_start:
  if (strncmp(buffer + n, "include", strlen("include")) == 0) {
    directive->type = INCLUDE;

    char *operand = NULL;
    for (size_t i = n + strlen("include"); i < match->length - n; i++)
      if (isalnum(buffer[i])) {
        sscanf(buffer + i, "%ms\"", &operand);
        operand[strlen(operand) - 1] = '\0';
        break;
      }

    directive->operands = operand;
  } else if (strncmp(buffer + n, "endcontent", strlen("endcontent")) == 0) {
    directive->type = ENDCONTENT;
    directive->operands = NULL;
  } else if (strncmp(buffer + n, "body", strlen("body")) == 0) {
    directive->type = BODY;
    directive->operands = NULL;
  } else if (strncmp(buffer + n, "contentfor", strlen("contentfor")) == 0) {
    directive->type = CONTENTFOR;
    contentfor_operand_t *operands = malloc(sizeof(contentfor_operand_t));

    for (size_t i = n + strlen("contentfor"); i < match->length; i++)
      if (isalnum(buffer[i])) {
        sscanf(buffer + i, "%ms\"", &operands->key);
        operands->key[strlen(operands->key) - 1] = '\0';
        break;
      }

    buffer = content + match->length + match->offset;

    size_t content_length = 0;

    key_match_t *new_match;
    directive_t *new_directive;

    while (true) {
      new_match = find_next_key(buffer, 0);
      if (new_match == NULL) {
        printf("Cannot find endcontent\n");
        free(new_directive);
        free(new_match);
        free(directive);
        return NULL;
      }

      new_directive = find_directive(buffer, new_match);
      if (new_directive == NULL) {
        printf("Cannot find directive: %.*s\n",
               new_match->length,
               buffer + new_match->offset);
        free(new_directive);
        free(new_match);
        free(directive);
        return NULL;
      }

      if (new_directive->type == ENDCONTENT) {
        break;
      }
    }

    asprintf(&operands->content, "%.*s", new_match->offset, buffer);
    operands->length = match->offset + match->length + new_match->offset
                       + new_match->length;

    free(new_directive);
    free(new_match);

    directive->operands = operands;
  } else if (strncmp(buffer + n, "content ", strlen("content")) == 0) {
    directive->type = CONTENT;

    char *operand = NULL;
    for (size_t i = n + strlen("content"); i < match->length - n; i++)
      if (isalnum(buffer[i])) {
        sscanf(buffer + i, "%ms\"", &operand);
        operand[strlen(operand) - 1] = '\0';
        break;
      }

    directive->operands = operand;
  } else {
    free(directive);
    return NULL;
  }

  return directive;
}

char *
find_contentfor_value(list_t *content_headers, char *key)
{
  for (size_t i = 0; i < content_headers->size; i++) {
    contentfor_operand_t *operand = list_get(content_headers, i);

    if (strcmp(key, operand->key) == 0)
      return operand->content;
  }

  return NULL;
}
